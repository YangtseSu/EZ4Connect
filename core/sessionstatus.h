#ifndef SESSIONSTATUS_H
#define SESSIONSTATUS_H

#include <QDateTime>
#include <QString>

// What aTrust session maintenance reports on stdout. The core prints these words
// on purpose so a caller can follow the session without reading prose, and the
// GUI follows them instead of guessing from the data plane: `reason` is the
// core's own text and is shown to the user verbatim.
enum class SessionMaintenanceState
{
    Unknown,
    Online,
    Renewing,
    Restoring,
    ReloginRequired,
    Offline,
};

// One "aTrust session state: <state> reason=<reason> code=<code>" line.
struct SessionStateLine
{
    bool valid = false;
    SessionMaintenanceState state = SessionMaintenanceState::Unknown;
    QString stateWord;
    QString reason;
    int code = 0;
};

// What the core's keep-alive last reported. A round whose target answered
// outside the tunnel counts as an answer but not as activity - the core says so
// with "routed=direct" - and only traffic through the tunnel holds the session
// open, so the two have to stay apart.
struct KeepAliveLine
{
    bool valid = false;
    bool ok = false;
    bool throughTunnel = false;
    // The core reports every failed round; its own streak accounting ("3
    // consecutive failed rounds") is what tells a miss from an outage.
    bool reportsStreak = false;
    int consecutiveFailures = 0;
    QString summary; // the line without its "KeepAlive: " prefix
};

SessionMaintenanceState sessionMaintenanceStateFromCore(const QString &stateWord);
QString sessionMaintenanceStateName(SessionMaintenanceState state);

SessionStateLine parseSessionStateLine(const QString &line);
KeepAliveLine parseKeepAliveLine(const QString &line);

// One notification per outage, not one per round: the core reports every failed
// round, so a warning is only worth showing once the streak has lasted a few of
// them.
constexpr int KeepAliveRoundsBeforeWarning = 3;
bool keepAliveWarrantsWarning(const KeepAliveLine &line);

// The status the GUI keeps for display: the core's last words plus when the GUI
// saw them, so a stale status is visibly stale.
struct SessionStatus
{
    SessionMaintenanceState state = SessionMaintenanceState::Unknown;
    QString reason;
    int code = 0;
    bool hasKeepAlive = false;
    KeepAliveLine keepAlive;
    QDateTime keepAliveAt;
};

#endif // SESSIONSTATUS_H
