#include "sessionstatus.h"

#include <QRegularExpression>
#include <QStringList>

namespace
{
const QString kSessionStatePrefix = QStringLiteral("aTrust session state: ");
const QString kKeepAlivePrefix = QStringLiteral("KeepAlive: ");

// The result shapes the core's keep-alive prints. Anything else with the
// "KeepAlive: " prefix is a note about how targets were chosen ("probing 2
// target(s) every 60s", "no application target, falling back to ..."), which is
// not a result and must not overwrite the last one.
struct KeepAliveShape
{
    const char *pattern;
    bool ok;
};

constexpr KeepAliveShape kKeepAliveShapes[] = {
    {"^OK status=[0-9]+ target=.* routed=", true},
    {"^recovered after [0-9]+ failed rounds", true},
    {"^target failure ", false},
    {"^(session|transport|target) failure ", false},
    {"^the session behind the tunnel is gone", false},
    {"^no usable target for [0-9]+ consecutive rounds", false},
};
}

SessionMaintenanceState sessionMaintenanceStateFromCore(const QString &stateWord)
{
    if (stateWord == QLatin1String("online"))
    {
        return SessionMaintenanceState::Online;
    }
    if (stateWord == QLatin1String("renewing"))
    {
        return SessionMaintenanceState::Renewing;
    }
    if (stateWord == QLatin1String("restoring"))
    {
        return SessionMaintenanceState::Restoring;
    }
    if (stateWord == QLatin1String("relogin-required"))
    {
        return SessionMaintenanceState::ReloginRequired;
    }
    if (stateWord == QLatin1String("offline"))
    {
        return SessionMaintenanceState::Offline;
    }
    return SessionMaintenanceState::Unknown;
}

QString sessionMaintenanceStateName(SessionMaintenanceState state)
{
    switch (state)
    {
    case SessionMaintenanceState::Online:
        return QStringLiteral("online");
    case SessionMaintenanceState::Renewing:
        return QStringLiteral("renewing");
    case SessionMaintenanceState::Restoring:
        return QStringLiteral("restoring");
    case SessionMaintenanceState::ReloginRequired:
        return QStringLiteral("relogin-required");
    case SessionMaintenanceState::Offline:
        return QStringLiteral("offline");
    case SessionMaintenanceState::Unknown:
        break;
    }
    return {};
}

SessionStateLine parseSessionStateLine(const QString &line)
{
    SessionStateLine result;
    const qsizetype marker = line.indexOf(kSessionStatePrefix);
    if (marker < 0)
    {
        return result;
    }

    // "aTrust session state: restoring reason=session refresh code=0". The reason
    // is the core's prose and contains spaces, so it is everything between
    // "reason=" and the code at the end.
    static const QRegularExpression pattern(
        QStringLiteral("^([a-z-]+) reason=(.*) code=(-?[0-9]+)$")
    );
    const QRegularExpressionMatch match = pattern.match(
        line.mid(marker + kSessionStatePrefix.size()).trimmed()
    );
    if (!match.hasMatch())
    {
        return result;
    }

    result.valid = true;
    result.stateWord = match.captured(1);
    result.state = sessionMaintenanceStateFromCore(result.stateWord);
    result.reason = match.captured(2).trimmed();
    result.code = match.captured(3).toInt();
    return result;
}

KeepAliveLine parseKeepAliveLine(const QString &line)
{
    KeepAliveLine result;
    const qsizetype marker = line.indexOf(kKeepAlivePrefix);
    if (marker < 0)
    {
        return result;
    }

    const QString summary = line.mid(marker + kKeepAlivePrefix.size()).trimmed();
    for (const KeepAliveShape &shape : kKeepAliveShapes)
    {
        if (!summary.contains(QRegularExpression(QLatin1String(shape.pattern))))
        {
            continue;
        }

        result.valid = true;
        result.ok = shape.ok;
        result.throughTunnel = shape.ok
            && !summary.contains(QLatin1String("routed=direct"));
        result.summary = summary;
        break;
    }
    if (!result.valid)
    {
        return result;
    }

    static const QRegularExpression streakPattern(
        QStringLiteral("([0-9]+) (?:consecutive failed rounds|consecutive rounds|failed rounds)")
    );
    const QRegularExpressionMatch streak = streakPattern.match(summary);
    if (streak.hasMatch())
    {
        result.reportsStreak = true;
        result.consecutiveFailures = streak.captured(1).toInt();
    }
    return result;
}

bool keepAliveWarrantsWarning(const KeepAliveLine &line)
{
    return line.valid
        && !line.ok
        && line.reportsStreak
        && line.consecutiveFailures >= KeepAliveRoundsBeforeWarning;
}
