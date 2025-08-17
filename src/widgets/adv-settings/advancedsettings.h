#ifndef ADVANCEDSETTINGS_H
#define ADVANCEDSETTINGS_H

#include "flasher.h"
#include <QWidget>

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

    Flasher *flasher() const; // const?

    void checkForUpdatesSilent();   // GUI
    void checkForFirmwareUpdatesSilent(); // Firmware

signals:
    void languageChanged(const QString &language);
    void themeChanged(bool dark);

    void fontChanged();

    void updateAvailable(const QString& tag, const QUrl& url); //GUI
    void firmwareUpdateAvailable(const QString& tag, const QUrl& url); //firmware


private slots:
    void on_pushButton_LangEnglish_clicked();
    void on_pushButton_LangRussian_clicked();
    void on_pushButton_LangSChinese_clicked();

    void on_spinBox_FontSize_valueChanged(int fontSize);
    void on_pushButton_About_clicked();

    void on_pushButton_removeName_clicked();

    void on_pushButton_RestartApp_clicked();

    void on_pushButton_LangDeutsch_clicked();

    void on_pushButton_CheckUpdates_clicked();
    void on_pushButton_CheckFirmware_clicked();


private:
    Ui::AdvancedSettings *ui;

    Flasher *m_flasher;

    QString m_default_text;
    QString m_default_style;


#ifdef Q_OS_WIN
    void checkForUpdatesWinHTTP(bool silent); // GUI WinHTTP Helper
    void checkForFirmwareUpdatesWinHTTP(bool silent); // Firmware
#endif
};

#endif // ADVANCEDSETTINGS_H
