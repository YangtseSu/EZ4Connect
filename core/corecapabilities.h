#ifndef CORECAPABILITIES_H
#define CORECAPABILITIES_H

#include <QSet>
#include <QString>

// The optional session and keep-alive knobs the GUI knows how to pass. They go
// only to a core that advertises them: a core that does not know a flag exits
// instead of ignoring it, so guessing here breaks the connection rather than
// degrading.
struct CoreCapabilities
{
    bool sessionRefreshInterval = false;
    bool keepAliveInterval = false;
    bool autoRelogin = false;
};

inline CoreCapabilities coreCapabilitiesFromFlags(const QSet<QString> &flags)
{
    CoreCapabilities capabilities;
    capabilities.sessionRefreshInterval =
        flags.contains(QStringLiteral("session-refresh-interval"));
    capabilities.keepAliveInterval = flags.contains(QStringLiteral("keep-alive-interval"));
    capabilities.autoRelogin = flags.contains(QStringLiteral("auto-relogin"));
    return capabilities;
}

#endif // CORECAPABILITIES_H
