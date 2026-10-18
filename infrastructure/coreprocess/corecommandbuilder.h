#ifndef CORECOMMANDBUILDER_H
#define CORECOMMANDBUILDER_H

#include <QMap>
#include <QStringList>

#include "core/connectionprofile.h"
#include "core/corecapabilities.h"

struct CoreRuntimePaths
{
    QString graphCodeFile;
    QString clientDataFile;
    QString debugPcapFile;
    QString debugTlsLogFile;
};

struct CoreCommand
{
    QStringList arguments;
    QStringList loggableArguments;
    QStringList clearedEnvironmentVariables;
    QMap<QString, QString> environmentVariables;

    QString loggableCommandLine() const;
};

class CoreCommandBuilder
{
public:
    // `capabilities` is what the core being started advertises; the optional
    // knobs are only passed when the corresponding capability is set, and the
    // default (nothing) keeps every caller that does not probe safe.
    static CoreCommand build(
        const ConnectionProfile &profile,
        const CoreRuntimePaths &runtimePaths = {},
        const CoreCapabilities &capabilities = {}
    );
};

#endif // CORECOMMANDBUILDER_H
