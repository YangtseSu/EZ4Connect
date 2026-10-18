#include "coreexecutable.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QRegularExpression>
#include <QStringList>
#include <QSysInfo>

#include <stdexcept>

namespace
{
// A core answers its own help immediately; the timeout only bounds a core that
// does something else entirely.
constexpr int probeTimeoutMs = 5000;
}

QString CoreExecutable::path()
{
    const QString fileName =
        QSysInfo::productType() == "windows" ? "zju-connect.exe" : "zju-connect";
    const QString bundledPath = QCoreApplication::applicationDirPath() + "/" + fileName;
    return QFileInfo::exists(bundledPath) ? bundledPath : fileName;
}

QString CoreExecutable::version(QObject *parent)
{
    QProcess process(parent);
    process.start(path(), {"-version"});
    if (!process.waitForStarted())
    {
        throw std::runtime_error("核心无法启动");
    }
    if (!process.waitForFinished())
    {
        throw std::runtime_error("核心运行超时");
    }

    const QByteArray errorOutput = process.readAllStandardError();
    if (!errorOutput.isEmpty())
    {
        throw std::runtime_error(errorOutput);
    }

    const QString output = process.readAllStandardOutput();
    const QString prefix("ZJU Connect v");
    if (!output.startsWith(prefix))
    {
        throw std::runtime_error("无法解析核心版本号");
    }
    return output.mid(prefix.size()).trimmed();
}

QSet<QString> CoreExecutable::parseSupportedFlags(const QByteArray &helpOutput)
{
    // One flag per indented line: "      --session-refresh-interval int     ...".
    // The core's flag library prints two dashes and Go's own flag package prints
    // one, so both forms are read; the name is the first token after them.
    static const QRegularExpression flagPattern(
        QStringLiteral("^[ \\t]+-{1,2}([A-Za-z0-9][A-Za-z0-9-]*)(?:[ \\t]|$)")
    );

    QSet<QString> flags;
    const QString text = QString::fromUtf8(helpOutput);
    for (const QString &line : text.split(QLatin1Char('\n')))
    {
        const QRegularExpressionMatch match = flagPattern.match(line);
        if (match.hasMatch())
        {
            flags.insert(match.captured(1));
        }
    }
    return flags;
}

QSet<QString> CoreExecutable::supportedFlags(QObject *parent)
{
    // One probe per core binary per run: the answer cannot change while this
    // process lives, and starting a core for every connection would be waste.
    static QHash<QString, QSet<QString>> cache;

    const QString corePath = path();
    const QHash<QString, QSet<QString>>::const_iterator cached = cache.constFind(corePath);
    if (cached != cache.constEnd())
    {
        return cached.value();
    }

    QSet<QString> flags;
    QProcess process(parent);
    process.start(corePath, {"-h"});
    if (process.waitForStarted() && process.waitForFinished(probeTimeoutMs))
    {
        // The flag package writes usage to stderr; a core that writes it to
        // stdout is read the same way.
        flags = parseSupportedFlags(
            process.readAllStandardError() + process.readAllStandardOutput()
        );
    }
    else
    {
        // A core that neither answers nor exits is killed rather than left
        // behind, and the empty answer keeps every optional flag out of the
        // command line.
        process.kill();
        process.waitForFinished(probeTimeoutMs);
    }

    cache.insert(corePath, flags);
    return flags;
}
