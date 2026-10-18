#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QDialog>
#include <QSettings>
#include "ui_settingwindow.h"
#include "presentation/dialogs/extrasettingwindow/extrasettingwindow.h"
#include "presentation/dialogs/authinfowindow/authinfowindow.h"

namespace Ui
{
    class SettingWindow;
}

class SettingWindow : public QDialog
{
Q_OBJECT

public:
    explicit SettingWindow(QWidget *parent = nullptr, QSettings *settings = nullptr, const QString &profileId = "");

    ~SettingWindow() override;

private:
    void loadSettings();
    void applySettings();
    void updateKeepAliveTargetState();
    bool isAuthSettingChanged();
    bool shouldCheckCredential();
    // The dialog's values as the settings window holds them between load and
    // apply; the core knobs are part of the same round trip.
    ExtraSettings extraSettings() const;
    void setExtraSettings(const ExtraSettings &values);

    Ui::SettingWindow *ui;

    QSettings *settings;
    QString profileId;

    ExtraSettingWindow *extraSettingWindow;
    AuthInfoWindow *authInfoWindow;

    QString tcpPortForwarding;
    QString udpPortForwarding;
    QString customDNS;
    QString customProxyDomain;
    QString extraArguments;
    int sessionRefreshInterval = 0;
    int keepAliveInterval = 0;
    bool autoRelogin = false;
};

#endif //SETTINGWINDOW_H
