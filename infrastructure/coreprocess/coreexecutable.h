#ifndef COREEXECUTABLE_H
#define COREEXECUTABLE_H

#include <QByteArray>
#include <QSet>
#include <QString>

class QObject;

namespace CoreExecutable
{
QString path();
QString version(QObject *parent);

// The flags the core lists in its own help output. An empty set means the probe
// could not run, and then no optional flag is passed - a core that does not know
// a flag exits instead of ignoring it.
QSet<QString> supportedFlags(QObject *parent = nullptr);

// Pure: reads the flag names out of a flag-usage dump, so the probe above can be
// tested without a core at hand.
QSet<QString> parseSupportedFlags(const QByteArray &helpOutput);
}

#endif // COREEXECUTABLE_H
