#include <QCoreApplication>
#include <QDebug>
#include <QList>

#include "core/sessionstatus.h"

namespace
{
bool parsesSessionStateLines()
{
    // Verbose lines, exactly as the core prints them: the reason is the core's
    // prose and contains spaces, the code sits at the end.
    const SessionStateLine online = parseSessionStateLine(
        "2026/10/18 13:43:01 aTrust session state: online reason=logged in code=0"
    );
    const SessionStateLine restoring = parseSessionStateLine(
        "2026/10/18 13:54:01 aTrust session state: restoring reason=session refresh code=0"
    );
    const SessionStateLine relogin = parseSessionStateLine(
        "aTrust session state: relogin-required reason=session refresh was refused code=75500002"
    );

    if (!online.valid
        || online.state != SessionMaintenanceState::Online
        || online.reason != "logged in"
        || online.code != 0)
    {
        qCritical() << "parsesSessionStateLines failed on the online line" << online.reason;
        return false;
    }
    if (!restoring.valid || restoring.state != SessionMaintenanceState::Restoring)
    {
        qCritical() << "parsesSessionStateLines failed on the restoring line";
        return false;
    }
    if (!relogin.valid
        || relogin.state != SessionMaintenanceState::ReloginRequired
        || relogin.reason != "session refresh was refused"
        || relogin.code != 75500002)
    {
        qCritical() << "parsesSessionStateLines failed on the relogin line" << relogin.reason;
        return false;
    }

    // A word this build does not know is still a valid report: the state is what
    // the GUI cannot act on, not a reason to drop the line.
    const SessionStateLine unknown = parseSessionStateLine(
        "aTrust session state: something-new reason=whatever code=1"
    );
    if (!unknown.valid || unknown.state != SessionMaintenanceState::Unknown)
    {
        qCritical() << "parsesSessionStateLines dropped an unknown state word";
        return false;
    }

    return !parseSessionStateLine("2026/10/18 13:54:01 VPN client started").valid
        && !parseSessionStateLine("aTrust session requires reauthentication (code=0)").valid
        && !parseSessionStateLine("aTrust session state: restoring").valid;
}

bool parsesKeepAliveRounds()
{
    const KeepAliveLine ok = parseKeepAliveLine(
        "KeepAlive: OK status=200 target=http://intranet.example/ routed=tunnel"
    );
    if (!ok.valid || !ok.ok)
    {
        qCritical() << "parsesKeepAliveRounds failed on the tunnel round";
        return false;
    }

    const KeepAliveLine direct = parseKeepAliveLine(
        "KeepAlive: OK status=200 target=http://intranet.example/ routed=direct: "
        "this request did not cross the tunnel and does not hold the session"
    );
    if (!direct.valid || !direct.ok)
    {
        qCritical() << "parsesKeepAliveRounds failed on the direct round";
        return false;
    }

    const KeepAliveLine perRound = parseKeepAliveLine(
        "KeepAlive: target failure target=http://intranet.example/ routed=denied "
        "err=not in the granted resources, the tunnel refuses it"
    );
    if (!perRound.valid || perRound.ok)
    {
        qCritical() << "parsesKeepAliveRounds failed on the per-round failure";
        return false;
    }

    const KeepAliveLine sessionGone = parseKeepAliveLine(
        "KeepAlive: the session behind the tunnel is gone (target=http://intranet.example/ "
        "code=10000004, 3 consecutive failed rounds)"
    );
    const KeepAliveLine transport = parseKeepAliveLine(
        "KeepAlive: transport failure for 4 consecutive rounds "
        "(target=http://intranet.example/): dial tcp: i/o timeout"
    );
    const KeepAliveLine noTarget = parseKeepAliveLine(
        "KeepAlive: no usable target for 5 consecutive rounds "
        "(target=http://intranet.example/): http status 404"
    );
    if (!sessionGone.valid || sessionGone.ok
        || !transport.valid || transport.ok
        || !noTarget.valid || noTarget.ok)
    {
        qCritical() << "parsesKeepAliveRounds failed on a failure report";
        return false;
    }

    const KeepAliveLine recovered = parseKeepAliveLine(
        "KeepAlive: recovered after 3 failed rounds (target=http://intranet.example/)"
    );
    if (!recovered.valid || !recovered.ok)
    {
        qCritical() << "parsesKeepAliveRounds failed on the recovery";
        return false;
    }

    // Notes about how targets were chosen are not results and must not overwrite
    // the last one.
    return !parseKeepAliveLine("KeepAlive: probing 2 target(s) every 60s").valid
        && !parseKeepAliveLine(
               "KeepAlive: no application target, falling back to the remote-DNS probe"
           ).valid
        && !parseKeepAliveLine("Keep alive is disabled: no application target").valid
        && !parseKeepAliveLine("ordinary output").valid;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return parsesSessionStateLines() && parsesKeepAliveRounds() ? 0 : 1;
}
