#include "extrasettingwindow.h"
#include "ui_extrasettingwindow.h"

ExtraSettingWindow::ExtraSettingWindow(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::ExtraSettingWindow)
{
    ui->setupUi(this);

    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(this, &QDialog::accepted,
            [&]()
            {
                QStringList tcpPortForwardingList;
                QStringList udpPortForwardingList;

                QStringList portForwardingList = ui->forwardTextEdit->toPlainText().split("\n");

                for (const auto &forwarding: portForwardingList)
                {
                    QStringList portForwardingItem = forwarding.split(",");

                    if (portForwardingItem.size() == 3)
                    {
                        if (portForwardingItem[0].simplified() == "TCP")
                        {
                            tcpPortForwardingList.append(
                                portForwardingItem[1].simplified() + "-" + portForwardingItem[2].simplified()
                            );
                        }
                        else if (portForwardingItem[0].simplified() == "UDP")
                        {
                            udpPortForwardingList.append(
                                portForwardingItem[1].simplified() + "-" + portForwardingItem[2].simplified()
                            );
                        }
                    }
                }

				QStringList customDnsList;

				for (const auto& hosts : ui->hostsTextEdit->toPlainText().split("\n"))
				{
					QStringList hostsItem = hosts.split(" ");
					if (hostsItem.size() == 2)
					{
						customDnsList.append(
							hostsItem[1].simplified() + ":" + hostsItem[0].simplified()
						);
					}
				}

				QStringList customProxyDomainsList = ui->proxyTextEdit->toPlainText().split("\n");

                ExtraSettings settings;
                settings.tcpPortForwarding = tcpPortForwardingList.join(",");
                settings.udpPortForwarding = udpPortForwardingList.join(",");
                settings.customDns = customDnsList.join(",");
                settings.customProxyDomains = customProxyDomainsList.join(",");
                settings.extraArguments =
                    ui->extraArgTextEdit->toPlainText().replace("\n", " ");
                settings.sessionRefreshInterval = ui->sessionRefreshSpinBox->value();
                settings.keepAliveInterval = ui->keepAliveSpinBox->value();
                settings.autoRelogin = ui->autoReloginCheckBox->isChecked();

                emit applied(settings);
            });

}

ExtraSettingWindow::~ExtraSettingWindow()
{
    delete ui;
}

void ExtraSettingWindow::setup(
    const ExtraSettings &settings,
    const CoreCapabilities &capabilities
)
{
    QStringList tcpPortForwardingList = settings.tcpPortForwarding.split(",");

    for (const auto &forwarding: tcpPortForwardingList)
    {
        QStringList tcpPortForwardingItem = forwarding.split("-");

        if (tcpPortForwardingItem.size() == 2)
        {
            ui->forwardTextEdit->append(
                "TCP," + tcpPortForwardingItem[0].simplified() + "," + tcpPortForwardingItem[1].simplified()
            );
        }
    }

    QStringList udpPortForwardingList = settings.udpPortForwarding.split(",");

    for (const auto &forwarding: udpPortForwardingList)
    {
        QStringList udpPortForwardingItem = forwarding.split("-");

        if (udpPortForwardingItem.size() == 2)
        {
            ui->forwardTextEdit->append(
                "UDP," + udpPortForwardingItem[0].simplified() + "," + udpPortForwardingItem[1].simplified()
            );
        }
    }

    QStringList customDnsList = settings.customDns.split(",");

	for (const auto& hosts : customDnsList)
	{
		QStringList hostsItem = hosts.split(":");

		if (hostsItem.size() == 2)
		{
			ui->hostsTextEdit->append(
				hostsItem[1].simplified() + " " + hostsItem[0].simplified()
			);
		}
	}

	QStringList customProxyDomainsList = settings.customProxyDomains.split(",");

	for (const auto &domain : customProxyDomainsList)
	{
		ui->proxyTextEdit->append(domain.trimmed());
	}

	ui->extraArgTextEdit->setPlainText(settings.extraArguments);

    ui->sessionRefreshSpinBox->setValue(settings.sessionRefreshInterval);
    ui->keepAliveSpinBox->setValue(settings.keepAliveInterval);
    ui->autoReloginCheckBox->setChecked(settings.autoRelogin);

    // A knob the running core does not advertise cannot be passed, so its control
    // is shown disabled with the reason instead of silently doing nothing. The
    // values stay as they are: another core may support them.
    const QString unsupported = "当前核心不支持该参数";
    for (const QPair<QWidget *, bool> &control : QList<QPair<QWidget *, bool>>{
             {ui->sessionRefreshSpinBox, capabilities.sessionRefreshInterval},
             {ui->keepAliveSpinBox, capabilities.keepAliveInterval},
             {ui->autoReloginCheckBox, capabilities.autoRelogin},
         })
    {
        control.first->setEnabled(control.second);
        if (!control.second)
        {
            control.first->setToolTip(unsupported);
        }
    }
}
