#include <QCoreApplication>
#include <QDebug>
#include <QSet>

#include "infrastructure/coreprocess/coreexecutable.h"

namespace
{
bool readsFlagsOutOfAUsageDump()
{
    // The shape the core's flag library prints, plus one line in the shape Go's
    // own flag package prints, because both are cores this GUI starts.
    const QByteArray help =
        "Usage of zju-connect:\n"
        "      --add-route                        Add route from rules for TUN interface\n"
        "      --auto-relogin                     Repeat a full aTrust login when a session dies\n"
        "      --keep-alive-interval int          Keep alive interval in seconds (minimum 10) (default 60)\n"
        "      --session-refresh-interval int     aTrust session refresh interval in seconds (0 disables)\n"
        "      --socks-bind string                The address SOCKS5 server listens on\n"
        "  -version\n"
        "    \tshow the version\n";

    const QSet<QString> flags = CoreExecutable::parseSupportedFlags(help);
    const QSet<QString> expected{
        "add-route",
        "auto-relogin",
        "keep-alive-interval",
        "session-refresh-interval",
        "socks-bind",
        "version"
    };
    if (flags != expected)
    {
        qCritical() << "readsFlagsOutOfAUsageDump failed:" << flags;
        return false;
    }

    // A description that mentions a flag must not become one, and a header line
    // is not a flag either.
    const QSet<QString> falsePositives = CoreExecutable::parseSupportedFlags(
        "Usage of zju-connect:\n"
        "      --keep-alive-url string   Keep alive URLs, comma separated or repeated\n"
        "     -h, --help\n"
        "          Print usage\n"
    );
    if (falsePositives.contains("help") || falsePositives.contains("Print"))
    {
        qCritical() << "readsFlagsOutOfAUsageDump invented a flag:" << falsePositives;
        return false;
    }

    return CoreExecutable::parseSupportedFlags("").isEmpty()
        && CoreExecutable::parseSupportedFlags("no flags here\n").isEmpty();
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return readsFlagsOutOfAUsageDump() ? 0 : 1;
}
