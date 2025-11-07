#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTranslator>

#include "hiddevice.h"
#include "reportconverter.h"

#include "advancedsettings.h"
#include "axesconfig.h"
#include "axescurvesconfig.h"
#include "buttonconfig.h"
#include "debugwindow.h"
#include "pinconfig.h"
#include "shiftregistersconfig.h"
#include "switchbutton.h"
 #include "configmanager.h"
#include "shiftregisters.h"
#include "developer.h"
#include "board_presets.h"
#include "device_info.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Developer;
class MainWindow : public QMainWindow
{
    Q_OBJECT



public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setDefaultStyleSheet();

    //Force Anchor setup:

    struct ForceTriplet { qint16 adc100, adc75, adc50; };
    struct ForceAnchorsGUI {
        quint16 magic;
        quint8  version;
        quint8  sealed;
        quint32 crc32;
        ForceTriplet rl_17;   // roll-left 17 lbf
        ForceTriplet rr_17;   // roll-right 17 lbf
        ForceTriplet pd_17;   // pitch-down 17 lbf
        ForceTriplet pu25;    // pitch-up digital 25 lbf
        ForceTriplet pu40;    // pitch-up analog 40 lbf
        quint8   reserved[8];

    };
    bool readAnchorsFromDevice(ForceAnchorsGUI* out);
signals:
    void getConfigDone(bool success);
    void sendConfigDone(bool success);
    void advancedModeToggled(bool enabled);

private slots:
    void showConnectDeviceInfo();
    void hideConnectDeviceInfo();
    void flasherConnected();
    //void getParamsPacket(uint8_t *buffer);
    void getParamsPacket(bool firmwareCompatible);

    void configReceived(bool success);
    void configSent(bool success);
    void blockWRConfigToDevice(bool block);

    void deviceFlasherController(bool isStartFlash);

    void hidDeviceList(const QList<QPair<bool, QString>> &deviceNames);
    void hidDeviceListChanged(int index);

    void languageChanged(const QString &language);
    void setFont();

    void finalInitialization();

    void on_pushButton_ResetAllPins_clicked();

    void on_pushButton_ReadConfig_clicked();
    void on_pushButton_WriteConfig_clicked();

    void on_pushButton_SaveToFile_clicked();
    void on_pushButton_LoadFromFile_clicked();

    void on_pushButton_ShowDebug_clicked();

    void on_toolButton2_clicked();

    void on_pushButton_Wiki_clicked();
    void on_pushButton_Help_clicked();

    void themeChanged(bool dark);

    void on_toolButton_ConfigsDir_clicked();

    void onGripSelectionChanged(int index); //building modular profiles
    void setAdvancedMode(bool enabled); // triggering advanced mode with developer mode
    void onBoardPresetChanged(int index); //Building board presets for pin mapping

    void onUpdateAvailable(const QString& tag, const QUrl& url); //update available popup when appropriate
    void readDeviceInfo();




protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override; //preventskeypressevent from being blocked by adding grip profile
    void showEvent(QShowEvent* e) override; //Show popup when update is available after building GUI
    void resizeEvent(QResizeEvent *event) override; //Debug: show window size

private:
    Ui::MainWindow *ui;

    QThread *m_thread;
    HidDevice *m_hidDeviceWorker;

    QThread *m_threadGetSendConfig;

    PinConfig *m_pinConfig;
    ButtonConfig *m_buttonConfig;
    ShiftRegistersConfig *m_shiftRegConfig;
    AxesConfig *m_axesConfig;
    AxesCurvesConfig *m_axesCurvesConfig;
    AdvancedSettings *m_advSettings;

    DebugWindow *m_debugWindow = nullptr;
    bool m_debugIsEnable;

    bool m_deviceChanged;
    bool m_hasShownSaveWarning = false; //save config warning populates only once.

    QString m_buttonDefaultStyle;   // ?????????

    QString m_cfgDirPath;
    void curCfgFileChanged(const QString &fileName);
    QStringList cfgFilesList(const QString &dirPath);
    QIcon pixmapToIcon(QPixmap pixmap, const QColor &color);
    void updateColor();

    void UiReadFromConfig();
    void UiWriteToConfig();

    void loadAppConfig();
    void saveAppConfig();

    void checkForUpdates(bool silent = true); //GUI version check


    ConfigManager *configManager; //modular profiles
     QList<ShiftRegisters*> m_shiftRegsPtrList; //modular profiles

    void applyGripProfile(const QJsonObject &cfg);//modular profiles
    void remapLogicalButtonsForSim(const QString &simName);
    void onSimSoftwareChanged(const QString &simName);

    //Setup Developer Mode
    bool m_developerMode = false;
    QWidget* m_devTab = nullptr;
    QString  m_devTabText;
    QIcon    m_devTabIcon;
    void setDeveloperMode(bool on);
    Developer* m_developer = nullptr;



    void anchorsUiSet(const ForceAnchorsGUI& a);
    ForceAnchorsGUI anchorsUiGet() const;

    QByteArray packAnchors(const ForceAnchorsGUI& a) const;
    bool       unpackAnchors(const QByteArray& buf, ForceAnchorsGUI* out) const;
    quint32    crc32_le(const QByteArray& data) const;


    void applyBoardPreset(BoardId id, bool applyPinDefaults = true); //Board preset application

    //Popup when update available

    bool m_guiReady = false;
    QString m_pendingUpdateTag;
    QUrl m_pendingUpdateUrl;
    void showUpdatePopup(const QString& tag, const QUrl& url);

    // --- Anchors-driven calibration wiring ---
    enum AnchorsMode { AnchorsMode_FLCS40, AnchorsMode_Digital25 };

    void wireForceButtons();                // find buttons in the Axes tab and connect them

    void applyAnchorsToAxes(int percent);   // percent ∈ {100, 75, 50}
    int  pickForPercent(const ForceTriplet& t, int percent) const;

    AnchorsMode m_anchorsMode = AnchorsMode_FLCS40; // default “FLCS” (40 lbf up)
    int         m_anchorsPercent = 100;             // last chosen % (for UI feedback, if desired)
    ForceAnchorsGUI m_cachedAnchors{};
    bool m_cachedAnchorsValid = false;
    bool devRequestReply(quint8 op, const QByteArray& payload, QByteArray* out);

    DeviceInfo *m_deviceInfo;


};
#endif // MAINWINDOW_H
