#ifndef ADVANCEDSETTINGS_H
#define ADVANCEDSETTINGS_H

#include "flasher.h"
#include <QWidget>
#include "common_types.h"
#include "device_info.h"

QT_BEGIN_NAMESPACE
class QFile;
class QPushButton;
QT_END_NAMESPACE


namespace Ui {
class AdvancedSettings;
}

class AdvancedSettings : public QWidget
{
    Q_OBJECT

public:
    explicit AdvancedSettings(QWidget *parent = nullptr);
    ~AdvancedSettings();

    void setStyle(bool isDark);

    void readFromConfig();
    void writeToConfig();

    void retranslateUi();

    void setConfigDirPath(const QString &path);

    Flasher *flasher() const; // const?

    void checkForUpdatesSilent();   // GUI
    void checkForFirmwareUpdatesSilent(); // Firmware

signals:
    void languageChanged(const QString &language);
    void themeChanged(bool dark);

    void fontChanged();

    void updateAvailable(const QString& tag, const QUrl& url); //GUI
    void firmwareUpdateAvailable(const QString& tag, const QUrl& url); //firmware

    void configImportRequested();
    void configExportRequested();


private slots:
    void on_comboBox_Language_currentIndexChanged(int index);

    void on_spinBox_FontSize_valueChanged(int fontSize);
    void on_pushButton_About_clicked();

    void on_pushButton_RestartApp_clicked();

    void on_pushButton_CheckUpdates_clicked();
    void on_pushButton_CheckFirmware_clicked();

    void on_pushButton_ImportConfig_clicked();
    void on_pushButton_ExportConfig_clicked();

    void onDeviceInfoUpdated();
    void onDeviceInfoError(const QString &error);


private:
    Ui::AdvancedSettings *ui;

    Flasher *m_flasher;

    QString m_default_text;
    QString m_default_style;
    DeviceInfo *m_deviceInfo;
    // Device info storage
    QString m_deviceModel;
    QString m_deviceSerial;
    QString m_deviceDoM;
    QString m_deviceFwVersion;

    QString m_configDirPath;

public slots:
    void showDeviceInfo(const QString& model,
                        const QString& serial,
                        const QString& domISO,
                        quint16 fwRaw);
    void applyDeviceIdentity(const params_report_t& r);

#ifdef Q_OS_WIN
    void checkForUpdatesWinHTTP(bool silent); // GUI WinHTTP Helper
    void checkForFirmwareUpdatesWinHTTP(bool silent); // Firmware
#endif
};

#endif // ADVANCEDSETTINGS_H
