#ifndef EXTRASETTINGWINDOW_H
#define EXTRASETTINGWINDOW_H

#include <QDialog>

#include "ui_extrasettingwindow.h"

namespace Ui
{
    class ExtraSettingWindow;
}

// What the "其他设置" dialog carries: options that are passed as the core's own
// flags without a second consumer in the GUI.
struct ExtraSettings
{
    QString tcpPortForwarding;
    QString udpPortForwarding;
    QString customDns;
    QString customProxyDomains;
    QString extraArguments;
};

class ExtraSettingWindow : public QDialog
{
Q_OBJECT

public:
    explicit ExtraSettingWindow(QDialog *parent = nullptr);

    ~ExtraSettingWindow() override;

    void setup(const ExtraSettings &settings);

signals:
    void applied(const ExtraSettings &settings);

private:
    Ui::ExtraSettingWindow *ui;
};

#endif //EXTRASETTINGWINDOW_H
