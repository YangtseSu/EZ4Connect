#ifndef EXTRASETTINGWINDOW_H
#define EXTRASETTINGWINDOW_H

#include <QDialog>

#include "core/corecapabilities.h"

#include "ui_extrasettingwindow.h"

namespace Ui
{
    class ExtraSettingWindow;
}

// What the "其他设置" dialog carries. The last three are the core's own knobs and
// are passed through as its flags; 0 and false mean "leave it to the core", which
// is also what a core that does not know them does.
struct ExtraSettings
{
    QString tcpPortForwarding;
    QString udpPortForwarding;
    QString customDns;
    QString customProxyDomains;
    QString extraArguments;
    int sessionRefreshInterval = 0;
    int keepAliveInterval = 0;
    bool autoRelogin = false;
};

class ExtraSettingWindow : public QDialog
{
Q_OBJECT

public:
    explicit ExtraSettingWindow(QDialog *parent = nullptr);

    ~ExtraSettingWindow() override;

    void setup(const ExtraSettings &settings, const CoreCapabilities &capabilities);

signals:
    void applied(const ExtraSettings &settings);

private:
    Ui::ExtraSettingWindow *ui;
};

#endif //EXTRASETTINGWINDOW_H
