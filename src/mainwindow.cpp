#include <QThread>
#include <QTimer>
#include <QSettings>
#include <QFileDialog>
#include <QDesktopServices>
#include <QSpinBox>
#include <QCheckBox>
#include <QKeyEvent>
#include <QPainter>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QShowEvent>
#include <QSignalBlocker>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mousewheelguard.h"
#include "configtofile.h"
#include "selectfolder.h"
#include "advancedsettings.h"

#include "common_types.h"
#include "global.h"
#include "deviceconfig.h"
#include "version.h"
#include "pincombobox.h"
#include "converter.h"
#include "buttonconfig.h"
#include "developer.h"

//Update Check Includes:

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QTimer>
#include <QVersionNumber>
#include <QSslSocket>



#include "configmanager.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_deviceChanged(false)
//    , m_thread(new QThread)
//    , m_hidDeviceWorker(new HidDevice())
//    , m_threadGetSendConfig(new QThread)
//    , m_pinConfig(new PinConfig(this))
//    , m_buttonConfig(new ButtonConfig(this))
//    , m_ledConfig(new LedConfig(this))
//    , m_encoderConfig(new EncodersConfig(this))
//    , m_shiftRegConfig(new ShiftRegistersConfig(this))
//    , m_axesConfig(new AxesConfig(this))
//    , m_axesCurvesConfig(new AxesCurvesConfig(this))
//    , m_advSettings(new AdvancedSettings(this))


{
    qDebug()<<"main + member initialize time ="<< gEnv.pApp_start_time->elapsed() << "ms";
    QElapsedTimer timer;
    timer.start();

    ui->setupUi(this);




    qDebug() << "Qt version:" << qVersion();
    qDebug() << "SSL support:" << QSslSocket::supportsSsl();
    qDebug() << "Built against:" << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "Loaded runtime:" << QSslSocket::sslLibraryVersionString();

    configManager = new ConfigManager(this);
    QMainWindow::setWindowIcon(QIcon(":/Images/icon-32.png"));

    // Software version
    setWindowTitle(tr("Invictus HOTAS Configurator") + " v" + APP_VERSION);

    // load application config
    loadAppConfig();

    // hide Advanced Mode button by default
    ui->pushButton_ShowDebug->setVisible(false);

    // hide unused buttons and fields related to legacy config Maybe remove these elements when confirmed unneeded.
    ui->pushButton_SaveToFile->hide();
    ui->pushButton_LoadFromFile->hide();
    ui->comboBox_Configs->hide();
    ui->toolButton_ConfigsDir->hide();

    m_thread = new QThread;
    m_hidDeviceWorker = new HidDevice();
    m_threadGetSendConfig = new QThread;

    qDebug()<<"before add widgets ="<< timer.restart() << "ms";
    //////////////// ADD WIDGETS ////////////////
    // add pin widget
    m_pinConfig = new PinConfig(this);
    ui->layoutV_tabPinConfig->addWidget(m_pinConfig);
    qDebug()<<"pin config load time ="<< timer.restart() << "ms";

    // hide pin widget by default
    int pinTabIndex = ui->tabWidget->indexOf(ui->layoutV_tabPinConfig->parentWidget());
    if (pinTabIndex != -1) {
        ui->tabWidget->setTabVisible(pinTabIndex, false);
    }


    // add button widget
    m_buttonConfig = new ButtonConfig(this);
    ui->layoutV_tabButtonConfig->addWidget(m_buttonConfig);
    qDebug()<<"button config load time ="<< timer.restart() << "ms";

    // add axes widget
    m_axesConfig = new AxesConfig(this);
    ui->layoutV_tabAxesConfig->addWidget(m_axesConfig);
    qDebug()<<"axes config load time ="<< timer.restart() << "ms";

    wireForceButtons();

    // add axes curves widget
    m_axesCurvesConfig = new AxesCurvesConfig(m_axesConfig, this);
    ui->layoutV_tabAxesCurvesConfig->addWidget(m_axesCurvesConfig);
    qDebug()<<"curves config load time ="<< timer.restart() << "ms";
    connect(m_axesConfig, &AxesConfig::axisVisibilityChanged,
            m_axesCurvesConfig, &AxesCurvesConfig::handleAxisVisibility);


    // add shift registers widget
    m_shiftRegConfig = new ShiftRegistersConfig(this);
    m_shiftRegsPtrList = m_shiftRegConfig->getShiftRegisterWidgets();

    ui->layoutV_tabShiftRegistersConfig->addWidget(m_shiftRegConfig);
    qDebug()<<"shift config load time ="<< timer.restart() << "ms";
    // hide shift registers widget
    int shiftRegTabIndex = ui->tabWidget->indexOf(ui->layoutV_tabShiftRegistersConfig->parentWidget());
    if (shiftRegTabIndex != -1) {
        ui->tabWidget->setTabVisible(shiftRegTabIndex, false);
    }

    // add encoders widget
    m_encoderConfig = new EncodersConfig(this);
    ui->layoutV_tabEncodersConfig->addWidget(m_encoderConfig);
    qDebug()<<"encoder config load time ="<< timer.restart() << "ms";

    // hide encoders widget by default
    int encoderTabIndex = ui->tabWidget->indexOf(ui->layoutV_tabEncodersConfig->parentWidget());

    if (encoderTabIndex != -1) {
        ui->tabWidget->setTabVisible(encoderTabIndex, false);
    }
    // add led widget
    m_ledConfig = new LedConfig(this);
    ui->layoutV_tabLedConfig->addWidget(m_ledConfig);
    qDebug()<<"led config load time ="<< timer.restart() << "ms";
    // hide LED widget
    int ledTabIndex = ui->tabWidget->indexOf(ui->layoutV_tabLedConfig->parentWidget());
    if (ledTabIndex != -1) {
        ui->tabWidget->setTabVisible(ledTabIndex, false);
    }
    // add advanced settings widget
    m_advSettings = new AdvancedSettings(this);
    ui->layoutV_tabAdvSettings->addWidget(m_advSettings);
    qDebug()<<"advanced settings load time ="<< timer.restart() << "ms";

    //GUI Update Checker
    connect(m_advSettings, &AdvancedSettings::updateAvailable,
            this, &MainWindow::onUpdateAvailable);

    //Firmware Update Checker
    connect(m_advSettings, &AdvancedSettings::firmwareUpdateAvailable,
            this, [this](const QString& tag, const QUrl& url){
                // defer if window not ready/visible (same gating as GUI)
                if (!m_guiReady || !isVisible()) {
                    // stash and show after showEvent()
                    QTimer::singleShot(0, this, [=]{
                        QMessageBox box(this);
                        box.setWindowTitle(tr("Firmware update available"));
                        box.setText(tr("A newer firmware is available: %1").arg(tag));
                        box.setInformativeText(tr("Open the firmware release page?"));
                        box.setIcon(QMessageBox::NoIcon);
                        box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
                        QPushButton* open = box.addButton(tr("Open GitHub"), QMessageBox::AcceptRole);
                        box.addButton(QMessageBox::Cancel);
                        box.exec();
                        if (box.clickedButton() == open) QDesktopServices::openUrl(url);
                    });
                    return;
                }

                // show immediately if GUI is ready
                QMessageBox box(this);
                box.setWindowTitle(tr("Firmware update available"));
                box.setText(tr("A newer firmware is available: %1").arg(tag));
                box.setInformativeText(tr("Open the firmware release page?"));
                box.setIcon(QMessageBox::NoIcon);
                box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
                QPushButton* open = box.addButton(tr("Open GitHub"), QMessageBox::AcceptRole);
                box.addButton(QMessageBox::Cancel);
                box.exec();
                if (box.clickedButton() == open) QDesktopServices::openUrl(url);
            });

    // Create Developer panel and add it to the Developer tab
    m_developer = new Developer(this);


    // --- Developer Mode: capture and hide Developer tab (repurposed Debug Tab) at startup
    m_devTab = ui->tab_Developer;

    ui->layoutV_tabDeveloper->addWidget(m_developer);


    // Capture and hide the Developer tab at startup
    m_devTab = ui->tab_Developer;
    const int idx = ui->tabWidget->indexOf(m_devTab);
    if (idx >= 0) {
        m_devTabText = ui->tabWidget->tabText(idx);
        m_devTabIcon = ui->tabWidget->tabIcon(idx);
        ui->tabWidget->removeTab(idx);
    }
    setDeveloperMode(false);

    // Hook Developer's I/O into HID worker

    m_developer->setTransport(
        // send(op, payload)
        [this](quint8 op, const QByteArray& payload) -> bool {
            if (!m_hidDeviceWorker) return false;
            m_hidDeviceWorker->devRequest(op, payload);   // hands it to the worker
            return true;
        },
        // recv(expectOp, out)
        [this](quint8 expectOp, QByteArray* out) -> bool {
            QEventLoop loop;
            QByteArray resp;
            bool ok = false;

            // one-shot connection for the matching reply
            QMetaObject::Connection c = connect(
                m_hidDeviceWorker, &HidDevice::devPacket, this,
                [&](quint8 op, const QByteArray& data){
                    if (op == expectOp) { resp = data; ok = true; loop.quit(); }
                }
                );
            QTimer::singleShot(3000, &loop, &QEventLoop::quit); // 1s timeout
            loop.exec();
            disconnect(c);

            if (ok && out) *out = resp;
            return ok;
        }
        );
    m_developer->setFallbackProvider([this](Developer::Calib* roll, Developer::Calib* pitch) -> bool {
        if (!gEnv.pDeviceConfig) return false;

        // Heuristic: axis 0 = roll, axis 1 = pitch (adjust if your mapping differs)
        auto& axRoll  = gEnv.pDeviceConfig->config.axis_config[0];
        auto& axPitch = gEnv.pDeviceConfig->config.axis_config[1];

        roll->min   = axRoll.calib_min;
        roll->center= axRoll.calib_center;
        roll->max   = axRoll.calib_max;

        pitch->min   = axPitch.calib_min;
        pitch->center= axPitch.calib_center;
        pitch->max   = axPitch.calib_max;

        return true;
    });



    // read last chosen board (shared with PinConfig)
    gEnv.pAppSettings->beginGroup("BoardSettings");
    int saved = gEnv.pAppSettings->value("SelectedBoard", 0).toInt();  // 0=Gen3, 1=Gen4
    gEnv.pAppSettings->endGroup();

    ui->comboBox_Board->setCurrentIndex(saved);
    connect(ui->comboBox_Board, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onBoardPresetChanged);

    //Building Board Preset Selector
    ui->comboBox_Board->clear();
    ui->comboBox_Board->addItem(tr("— Select board —"), QVariant());
    ui->comboBox_Board->setItemData(0, 0, Qt::UserRole - 1);
    ui->comboBox_Board->addItem(tr("VFT Controller Gen 1-3"), int(BoardId::VftControllerGen3));
    ui->comboBox_Board->addItem(tr("VFT Controller Gen 4"), int(BoardId::VFTControllerGen4));
    ui->comboBox_Board->setCurrentIndex(0);

    const QVariant data = ui->comboBox_Board->currentData();
    if (data.isValid()) {
        applyBoardPreset(static_cast<BoardId>(data.toInt()), /*ask=*/false);
    }

    // strong focus for mouse wheel
    // without protection, when scrolling the page you can accidentally hover over a combo box and change it with the mouse wheel
    // when setFocusPolicy(Qt::StrongFocus) and protection on the combo box, you’ll have to click in order to scroll with the wheel
    for (auto &&child: this->findChildren<QSpinBox *>())
    {
        child->setFocusPolicy(Qt::StrongFocus);
        child->installEventFilter(new MouseWheelGuard(child));
    }

    for (auto &&child: this->findChildren<QComboBox *>())
    {
        child->setFocusPolicy(Qt::StrongFocus);
        child->installEventFilter(new MouseWheelGuard(child));
    }
    // not sure—this way or exclude above?
    ui->comboBox_HidDeviceList->setFocusPolicy(Qt::WheelFocus);
    ui->comboBox_Configs->setFocusPolicy(Qt::WheelFocus);
    for (auto &&comBox: m_pinConfig->findChildren<QComboBox *>())
    {
        comBox->setFocusPolicy(Qt::WheelFocus);
    }
    for (auto &&comBox: m_axesCurvesConfig->findChildren<QComboBox *>())
    {
        comBox->setFocusPolicy(Qt::WheelFocus);
    }
    for (auto &&comBox: m_ledConfig->findChildren<QComboBox *>())
    {
        comBox->setFocusPolicy(Qt::WheelFocus);
    }

    //////////////// SIGNAL-SLOTS ////////////////
    // get/send config
    connect(this, &MainWindow::getConfigDone, this, &MainWindow::configReceived);
    connect(this, &MainWindow::sendConfigDone, this, &MainWindow::configSent);


    // buttons pin changed
    connect(m_pinConfig, &PinConfig::totalButtonsValueChanged, m_buttonConfig, &ButtonConfig::setUiOnOff);
    // LEDs changed
    connect(m_pinConfig, &PinConfig::totalLEDsValueChanged, m_ledConfig, &LedConfig::spawnLeds);
    // encoder changed
    connect(m_buttonConfig, &ButtonConfig::encoderInputChanged, m_encoderConfig, &EncodersConfig::encoderInputChanged);
    // fast encoder
    connect(m_pinConfig, &PinConfig::fastEncoderSelected, m_encoderConfig, &EncodersConfig::fastEncoderSelected);
    // shift registers
    connect(m_pinConfig, &PinConfig::shiftRegSelected, m_shiftRegConfig, &ShiftRegistersConfig::shiftRegSelected);
    // a2b count
    connect(m_axesConfig, &AxesConfig::a2bCountChanged, m_pinConfig, &PinConfig::a2bCountChanged);
    // shift reg buttons count shiftRegsButtonsCount
    connect(m_shiftRegConfig, &ShiftRegistersConfig::shiftRegButtonsCountChanged,
            m_pinConfig, &PinConfig::shiftRegButtonsCountChanged);
    // buttonts/LEDs limit reached
    connect(m_pinConfig, &PinConfig::limitReached, this, &MainWindow::blockWRConfigToDevice);
    // axes source changed//axesSourceChanged
    connect(m_pinConfig, &PinConfig::axesSourceChanged, m_axesConfig, &AxesConfig::addOrDeleteMainSource);
    // language changed
    connect(m_advSettings, &AdvancedSettings::languageChanged, this, &MainWindow::languageChanged);
    // theme changed
    connect(m_advSettings, &AdvancedSettings::themeChanged, this, &MainWindow::themeChanged);
    // font changed
    connect(m_advSettings, &AdvancedSettings::fontChanged, this, &MainWindow::setFont);


    // enter flash mode clicked
    connect(m_advSettings->flasher(), &Flasher::flashModeClicked, this, &MainWindow::deviceFlasherController);
    // flasher found
    connect(m_hidDeviceWorker, &HidDevice::flasherFound, m_advSettings->flasher(), &Flasher::flasherFound);
    // start flash
    connect(m_advSettings->flasher(), &Flasher::startFlash, this, &MainWindow::deviceFlasherController);
    // flash status
    connect(m_hidDeviceWorker, &HidDevice::flashStatus, m_advSettings->flasher(), &Flasher::flashStatus);
    // set selected hid device
    connect(ui->comboBox_HidDeviceList, SIGNAL(currentIndexChanged(int)),
            this, SLOT(hidDeviceListChanged(int)));
    {
        // helper: request anchors, unpack, apply to X/Y using selected mode
        auto applyFromDevice = [this](int percent) {
            if (!m_hidDeviceWorker) return;

            // 1) Request anchors from device (same pattern as Developer transport)
            QByteArray resp;
            bool ok = false;
            QEventLoop loop;
            QMetaObject::Connection c = connect(
                m_hidDeviceWorker, &HidDevice::devPacket, this,
                [&](quint8 op, const QByteArray& data){
                    if (op == OP_GET_FACTORY_ANCHORS) { resp = data; ok = true; loop.quit(); }
                }
                );
            m_hidDeviceWorker->devRequest(OP_GET_FACTORY_ANCHORS, QByteArray());
            QTimer::singleShot(1000, &loop, &QEventLoop::quit);
            loop.exec();
            disconnect(c);
            if (!ok) { QMessageBox::warning(this, tr("Force Anchors"), tr("No reply from device.")); return; }

            // 2) Unpack (you already added MainWindow::unpackAnchors earlier)
            ForceAnchorsGUI a{};
            if (!unpackAnchors(resp, &a)) {
                QMessageBox::warning(this, tr("Force Anchors"), tr("CRC/format error reading anchors."));
                return;
            }

            // 3) Choose mode: Digital -> pu25, Analog -> pu40
            // (If you want Digital==40 and Analog==25, just swap the ternary.)
            const bool digitalMode = (ui->radioButton && ui->radioButton->isChecked());
            const ForceTriplet& pu = digitalMode ? a.pu25 : a.pu40;

            // helper to pick ADC by percent
            auto pick = [&](const ForceTriplet& t)->int {
                switch (percent) { case 75: return t.adc75; case 50: return t.adc50; default: return t.adc100; }
            };

            const int rollL = pick(a.rl_17);
            const int rollR = pick(a.rr_17);
            const int pitDn = pick(a.pd_17);
            const int pitUp = pick(pu);

            if (!gEnv.pDeviceConfig) {
                QMessageBox::warning(this, tr("Force Anchors"), tr("Device config not loaded."));
                return;
            }

            // 4) Apply to axis 0 (roll/X) and axis 1 (pitch/Y)
            auto& axX = gEnv.pDeviceConfig->config.axis_config[0];
            auto& axY = gEnv.pDeviceConfig->config.axis_config[1];

            const int xMin = qMin(rollL, rollR), xMax = qMax(rollL, rollR);
            axX.calib_min    = xMin;
            axX.calib_max    = xMax;
            axX.calib_center = 0;

            const int yMin = qMin(pitDn, pitUp), yMax = qMax(pitDn, pitUp);
            axY.calib_min    = yMin;
            axY.calib_max    = yMax;
            axY.calib_center = 0;

            if (m_axesConfig) m_axesConfig->readFromConfig();
        };

        // 5) Wire the percent radios (trigger on checked==true)
        if (ui->radio_Anchor100)
            connect(ui->radio_Anchor100, &QRadioButton::toggled, this,
                    [applyFromDevice](bool on){ if (on) applyFromDevice(100); });
        if (ui->radio_Anchor75)
            connect(ui->radio_Anchor75,  &QRadioButton::toggled, this,
                    [applyFromDevice](bool on){ if (on) applyFromDevice(75);  });
        if (ui->radio_Anchor50)
            connect(ui->radio_Anchor50,  &QRadioButton::toggled, this,
                    [applyFromDevice](bool on){ if (on) applyFromDevice(50);  });

        // 6) Mode radios (“Digital” / “Analog”): when mode changes and a % is selected, re-apply
        auto reapplyIfPercentChosen = [applyFromDevice, this]{
            if      (ui->radio_Anchor100 && ui->radio_Anchor100->isChecked()) applyFromDevice(100);
            else if (ui->radio_Anchor75  && ui->radio_Anchor75->isChecked())  applyFromDevice(75);
            else if (ui->radio_Anchor50  && ui->radio_Anchor50->isChecked())  applyFromDevice(50);
        };
        if (ui->radioButton)   // "Digital"
            connect(ui->radioButton,   &QRadioButton::toggled, this, [=](bool on){ if (on) reapplyIfPercentChosen(); });
        if (ui->radioButton_2) // "Analog"
            connect(ui->radioButton_2, &QRadioButton::toggled, this, [=](bool on){ if (on) reapplyIfPercentChosen(); });
        auto currentPercent = [this]() -> int {
            if (ui->radio_Anchor75 && ui->radio_Anchor75->isChecked()) return 75;
            if (ui->radio_Anchor50 && ui->radio_Anchor50->isChecked()) return 50;
            return 100;
        };
        auto pick = [](const ForceTriplet& t, int pct) {
            switch (pct) { case 75: return t.adc75; case 50: return t.adc50; default: return t.adc100; }
        };
        auto applyModeToPitchUpOnly = [this, currentPercent, pick](bool digitalOn) {
            if (!digitalOn) return;                    // only on the radio turning ON
            if (!gEnv.pDeviceConfig) return;

            // If we don’t have cached anchors yet, populate them by running the percent apply once.
            if (!m_cachedAnchorsValid) {
                applyAnchorsToAxes(currentPercent());
                if (!m_cachedAnchorsValid) return;
            }

            const int pct = currentPercent();
            const ForceTriplet& pu = (ui->radioButton && ui->radioButton->isChecked())
                                         ? m_cachedAnchors.pu25  // Digital
                                         : m_cachedAnchors.pu40; // Analog
            const int pitUp = pick(pu, pct);

            // Axis 1 = Pitch (Y): set max (keep min), maintain ordering
            auto& axY = gEnv.pDeviceConfig->config.axis_config[1];
            if (pitUp < axY.calib_min) axY.calib_min = pitUp; else axY.calib_max = pitUp;

            // Force Center = ON, value = 0 (per your requirement)
            axY.is_centered = 1;
            axY.calib_center = 0;

            if (m_axesConfig) m_axesConfig->readFromConfig();
        };

        // when mode changes, re-apply using the current %
        if (ui->radioButton)   // Digital
            connect(ui->radioButton,   &QRadioButton::toggled, this,
                    [this,currentPercent](bool on){ if (on) applyAnchorsToAxes(currentPercent()); });
        if (ui->radioButton_2) // Analog
            connect(ui->radioButton_2, &QRadioButton::toggled, this,
                    [this,currentPercent](bool on){ if (on) applyAnchorsToAxes(currentPercent()); });
    }


    // HID worker
    m_hidDeviceWorker->moveToThread(m_thread);
    connect(m_thread, &QThread::started, m_hidDeviceWorker, &HidDevice::processData);

    connect(m_hidDeviceWorker, &HidDevice::paramsPacketReceived, this, &MainWindow::getParamsPacket);
    connect(m_hidDeviceWorker, &HidDevice::deviceConnected, this, &MainWindow::showConnectDeviceInfo);
    connect(m_hidDeviceWorker, &HidDevice::deviceDisconnected, this, &MainWindow::hideConnectDeviceInfo);
    connect(m_hidDeviceWorker, &HidDevice::flasherConnected, this, &MainWindow::flasherConnected);
    connect(m_hidDeviceWorker, &HidDevice::hidDeviceList, this, &MainWindow::hidDeviceList);


    // read config from device
    connect(m_hidDeviceWorker, &HidDevice::configReceived, this, &MainWindow::configReceived);
    // write config to device
    connect(m_hidDeviceWorker, &HidDevice::configSent, this, &MainWindow::configSent);


    // load default config // loading will occur after loading buttons config
    // combo boxes for buttons are filled after the application starts and the config
    // should be launched by a signal from the buttons
    connect(m_buttonConfig, &ButtonConfig::logicalButtonsCreated, this, &MainWindow::finalInitialization);

    // Creating fields for comboBox_GripSelection
    ui->comboBox_GripSelection->clear();
    ui->comboBox_GripSelection->addItem(tr("— Select grip —"), QVariant());          // placeholder
    ui->comboBox_GripSelection->setItemData(0, 0, Qt::UserRole - 1);
    ui->comboBox_GripSelection->addItem("Invictus Viper", "invictus_viper.json");
    ui->comboBox_GripSelection->addItem("Thrustmaster® Warthog", "warthog.json");
    ui->comboBox_GripSelection->addItem("Thrustmaster® Cougar", "cougar.json");
    ui->comboBox_GripSelection->addItem("Tianhang F-16 26 Button", "tianhang26.json");
    ui->comboBox_GripSelection->addItem("Tianhang F-16 30 Button", "tianhang30.json");
    ui->comboBox_GripSelection->setCurrentIndex(0);

    //  Automatic loading of grip profile based on comboBox_GripSelection choice
    connect(ui->comboBox_GripSelection, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onGripSelectionChanged);

    {
        QSignalBlocker block(ui->comboBox_simSoftware);      // avoid firing currentTextChanged while we tweak

        // Insert a disabled placeholder at index 0 without clearing Designer items
        if (ui->comboBox_simSoftware->findText(tr("— Select sim software—")) == -1) {
            ui->comboBox_simSoftware->insertItem(0, tr("— Select sim software—"));
            ui->comboBox_simSoftware->setItemData(0, 0, Qt::UserRole - 1);  // disable placeholder
        }

        // Start on the placeholder unless you restore a saved selection elsewhere
        ui->comboBox_simSoftware->setCurrentIndex(0);
    }
    // Reconfiguring buttons when different sim options are chose
    connect(ui->comboBox_simSoftware, &QComboBox::currentTextChanged,
            this, &MainWindow::onSimSoftwareChanged);

    // protect key press event
    qApp->installEventFilter(this);

    // set theme
    /*gEnv.pAppSettings->beginGroup("StyleSettings");
    QString style = gEnv.pAppSettings->value("StyleSheet", "default").toString();
    gEnv.pAppSettings->endGroup();
    if (style == "dark") {
        themeChanged(true);
    } else {
        themeChanged(false);
    }*/

    themeChanged(true); // Force dark mode

    m_thread->start();

    qDebug()<<"after widgets load time ="<< timer.elapsed() << "ms";
    qDebug()<<"without style startup time ="<< gEnv.pApp_start_time->elapsed() << "ms";
}

MainWindow::~MainWindow()
{
    saveAppConfig();
    m_hidDeviceWorker->setIsFinish(true);
    m_hidDeviceWorker->deleteLater();
    m_thread->quit();
    m_thread->deleteLater();
    m_thread->wait();
    m_threadGetSendConfig->quit();
    m_threadGetSendConfig->deleteLater();
    m_threadGetSendConfig->wait();
    delete ui;
}




///////////////////// device reports /////////////////////
// device connected
void MainWindow::showConnectDeviceInfo()
{
    if (ui->comboBox_HidDeviceList->itemData(ui->comboBox_HidDeviceList->currentIndex()).toInt() != 1) {
        m_deviceChanged = true;
    } else {
        // for old(1.6-) firmware
        blockWRConfigToDevice(true);
        ui->label_DeviceStatus->setStyleSheet("color: white; background-color: rgb(168, 168, 0);");
    }
    m_advSettings->flasher()->deviceConnected(true);
    QSettings *settings = gEnv.pAppSettings;
    const QString suppressKey = "General/DisableForceConfigWarning";

    if (!settings->value(suppressKey, false).toBool()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Important: Save Configuration");
        msgBox.setText("Each individual device is carefully calibrated with precise force values.\n"
                       "If your device shipped with a Gen 4 control board, it has hardcoded force values.\n"
                       "If it shipped before 8/16/2025, it does not.\n"
                       "Please read your config from your device and make note of the calibration values\n"
                       "in the Axes Config Tab.  When you update your firmware, they will be overwritten.\n"
                       "Save the original values in a safe place.\n"
                       "If you lose them, they cannot be recovered.");
        msgBox.setIcon(QMessageBox::Warning);
        QCheckBox dontShow("Do not warn me again");
        msgBox.setCheckBox(&dontShow);
        msgBox.exec();

        if (dontShow.isChecked()) {
            settings->setValue(suppressKey, true);
        }
    }
}

// device disconnected
void MainWindow::hideConnectDeviceInfo()
{
    ui->label_DeviceStatus->setText(tr("Disconnected"));
    ui->label_DeviceStatus->setStyleSheet("color: white; background-color: rgb(160, 0, 0);");
    blockWRConfigToDevice(true);
    m_advSettings->flasher()->deviceConnected(false);
    // debug window
    if (m_debugWindow) {
        m_debugWindow->resetPacketsCount();
    }
    // disable curve point
    QTimer::singleShot(3000, this, [&] {   // not the best way
        if (ui->pushButton_ReadConfig->isEnabled() == false) {
            m_axesCurvesConfig->deviceStatus(false);
        }
    });
}

// flasher connected
void MainWindow::flasherConnected()
{
    ui->label_DeviceStatus->setText(tr("Connected"));
    ui->label_DeviceStatus->setStyleSheet("color: white; background-color: rgb(5, 170, 61);");
    blockWRConfigToDevice(true);
    m_advSettings->flasher()->deviceConnected(false);
    if (m_debugWindow) {
        m_debugWindow->resetPacketsCount();
    }
    if (ui->pushButton_ReadConfig->isEnabled() == false) {
        m_axesCurvesConfig->deviceStatus(false);
    }
}

// add/delete hid devices to/from combobox
void MainWindow::hidDeviceList(const QList<QPair<bool, QString>> &deviceNames)
{
    if (deviceNames.size() == 0) {
        ui->comboBox_HidDeviceList->clear();
        return;
    } else {
        ui->comboBox_HidDeviceList->clear();
        for (int i = 0; i < deviceNames.size(); ++i) {
            if (deviceNames[i].first) {
                // for old firmware
                ui->comboBox_HidDeviceList->addItem("ONLY FLASH " + deviceNames[i].second, 1);
            } else {
                ui->comboBox_HidDeviceList->addItem(deviceNames[i].second);
            }
        }
    }
}

// received device report
void MainWindow::getParamsPacket(bool firmwareCompatible)
{
    if (m_deviceChanged) {
        if (firmwareCompatible == false) {
            blockWRConfigToDevice(true);
            ui->label_DeviceStatus->setStyleSheet("color: white; background-color: rgb(168, 168, 0);");
            ui->label_DeviceStatus->setText(tr("Incompatible Firmware"));
        } else {
            if (m_pinConfig->limitIsReached() == false) {
                blockWRConfigToDevice(false);
            }
            ui->label_DeviceStatus->setStyleSheet("color: white; background-color: rgb(5, 170, 61);");
            // set firmware version
            QString str = QString::number(gEnv.pDeviceConfig->paramsReport.firmware_version, 16);
            if (str.size() == 4) {
                ui->label_DeviceStatus->setText(tr("Device firmware") + " v" + str[0] + "." + str[1] + "." + str[2]);
            }
        }
        m_deviceChanged = false;
    }

    // update button state without delay. fix gamepad_report.raw_button_data[0]
    // due to delay, changes in the first 64 physical buttons or the remaining ones might not be detected.
    // For example, gamepad_report.raw_button_data[0] might consecutively be 0
    // and the remaining 64 physical buttons won't be seen.
    if(ui->tab_ButtonConfig->isVisible() == true || m_debugWindow) {
        m_buttonConfig->buttonStateChanged();
    }

    static QElapsedTimer timer;

    if (timer.isValid() == false) {
        timer.start();
    }
    else if (timer.elapsed() > 17)    // update UI every 17ms(~60fps)
    {
        if(ui->tab_LED->isVisible() == true) {
            m_ledConfig->setLedsState();
        }
        if(ui->tab_AxesConfig->isVisible() == true) {
            m_axesConfig->axesValueChanged();
        }
        if(ui->tab_AxesCurves->isVisible() == true) {
            m_axesCurvesConfig->updateAxesCurves();
        }
        timer.restart();
    }
    // debug info
    if (m_debugWindow) {
        m_debugWindow->devicePacketReceived();
    }

    // debug tab
#ifdef QT_DEBUG

#endif
}

// Flasher controller
void MainWindow::deviceFlasherController(bool isStartFlash)
{        // crap? maybe QtConcurrent::run()
    // leave it like this or like read/write and pass bool via signal?
    QEventLoop loop; // static?
    QObject context;
    context.moveToThread(m_threadGetSendConfig);
    connect(m_threadGetSendConfig, &QThread::started, &context, [&]() {
        qDebug()<<"Start flasher controller";
        if (isStartFlash == true){
            qDebug()<<"Start flash";
            m_hidDeviceWorker->flashFirmware(m_advSettings->flasher()->fileArray());
        } else {
            qDebug()<<"Enter to flash mode";
            m_hidDeviceWorker->enterToFlashMode();
        }
        qDebug()<<"Flasher controller finished";
        loop.quit();
    });
    m_threadGetSendConfig->start();
    loop.exec();
    m_threadGetSendConfig->quit();
    m_threadGetSendConfig->wait();
}


/////////////////////    CONFIG SLOTS    /////////////////////
void MainWindow::UiReadFromConfig()
{
    // read pin config
    m_pinConfig->readFromConfig();
    // read axes config
    m_axesConfig->readFromConfig();
    // read axes curves config
    m_axesCurvesConfig->readFromConfig();
    // read shift registers config
    m_shiftRegConfig->readFromConfig();
    // read encoder config
    m_encoderConfig->readFromConfig();
    // read LED config
    m_ledConfig->readFromConfig();
    // read adv.settings config
    m_advSettings->readFromConfig();
    // read button config
    m_buttonConfig->readFromConfig();
}

void MainWindow::UiWriteToConfig()
{
    // write pin config
    m_pinConfig->writeToConfig();
    // write axes config
    m_axesConfig->writeToConfig();
    // write axes curves config
    m_axesCurvesConfig->writeToConfig();
    // write shift registers config
    m_shiftRegConfig->writeToConfig();
    // write encoder config
    m_encoderConfig->writeToConfig();
    // write LED config
    m_ledConfig->writeToConfig();
    // write adv.settings config
    m_advSettings->writeToConfig();
    // write button config
    m_buttonConfig->writeToConfig();
    // remove device name from registry. sometimes windows does not update the name in gaming devices and has to be deleted in the registry
#ifdef Q_OS_WIN
    qDebug()<<"Remove device OEMName from registry";
    QString path("HKEY_CURRENT_USER\\System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_%1&PID_%2");
    QString path2("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_%1&PID_%2");
    QSettings(path.arg(QString::number(gEnv.pDeviceConfig->config.vid, 16), QString::number(gEnv.pDeviceConfig->config.pid, 16)),
              QSettings::NativeFormat).remove("OEMName");
    QSettings(path2.arg(QString::number(gEnv.pDeviceConfig->config.vid, 16), QString::number(gEnv.pDeviceConfig->config.pid, 16)),
              QSettings::NativeFormat).remove("OEMName");
#endif
}


// load default config
void MainWindow::finalInitialization()
{
    // load config files
    QStringList filesList = cfgFilesList(m_cfgDirPath);
    if (filesList.isEmpty() == false) {
        ui->comboBox_Configs->clear();
        ui->comboBox_Configs->addItems(filesList);
        gEnv.pAppSettings->beginGroup("Configs");
        QString lastCfg(gEnv.pAppSettings->value("LastCfg").toString());
        gEnv.pAppSettings->endGroup();
        bool found = false;
        for (int i = 0; i < filesList.size(); ++i) {
            if (filesList[i] == lastCfg) {
                curCfgFileChanged(lastCfg);
                ui->comboBox_Configs->setCurrentIndex(i);
                found = true;
                break;
            }
        }
        if (found == false) {
            curCfgFileChanged(ui->comboBox_Configs->currentText());
        }
    }  else {
        UiReadFromConfig();
    }

    // select config comboBox // should be after "// load config files"
    connect(ui->comboBox_Configs, &QComboBox::currentTextChanged, this, &MainWindow::curCfgFileChanged);
}

// current cfg file changed
void MainWindow::curCfgFileChanged(const QString &fileName)
{
    QString filePath = m_cfgDirPath + '/' + fileName + ".cfg";
    ConfigToFile::loadDeviceConfigFromFile(this, filePath, gEnv.pDeviceConfig->config);
    UiReadFromConfig();
}
// get config file list
QStringList MainWindow::cfgFilesList(const QString &dirPath)
{
    QDir dir(dirPath);
    QStringList cfgs = dir.entryList(QStringList() << "*.cfg", QDir::Files);
    for (auto &line : cfgs) {
        line.remove(line.size() - 4, 4);// 4 = ".cfg" characters count
    }
    cfgs.sort(Qt::CaseInsensitive);
    return cfgs;
}


// slot after receiving the config
void MainWindow::configReceived(bool success)
{
    m_buttonDefaultStyle = ui->pushButton_ReadConfig->styleSheet();
    static QString button_default_text = ui->pushButton_ReadConfig->text();    //????????????????????????

    if (success == true)
    {
        UiReadFromConfig();
        // curves pointer activated
        m_axesCurvesConfig->deviceStatus(true);

        // set firmware version
        QString str = QString::number(gEnv.pDeviceConfig->config.firmware_version, 16);
        if (str.size() == 4) {
            ui->label_DeviceStatus->setText(tr("Device firmware") + " v" + str[0] + "." + str[1] + "." + str[2] + "b" + str[3] + " ✔");
        }

        ui->pushButton_ReadConfig->setText(tr("Received"));
        ui->pushButton_ReadConfig->setStyleSheet(m_buttonDefaultStyle + "color: rgb(235, 235, 235); background-color: rgb(5, 170, 61);");
        QTimer::singleShot(1500, this, [&] {
            ui->pushButton_ReadConfig->setStyleSheet(m_buttonDefaultStyle);
            ui->pushButton_ReadConfig->setText(button_default_text);
            if (ui->comboBox_HidDeviceList->currentIndex() >= 0){
                blockWRConfigToDevice(false);
            }
        });
    } else {
        ui->pushButton_ReadConfig->setText(tr("Error"));
        ui->pushButton_ReadConfig->setStyleSheet(m_buttonDefaultStyle + "color: rgb(235, 235, 235); background-color: rgb(200, 0, 0);");
        QTimer::singleShot(1500, this, [&] {
            ui->pushButton_ReadConfig->setStyleSheet(m_buttonDefaultStyle);
            ui->pushButton_ReadConfig->setText(button_default_text);
            if (ui->comboBox_HidDeviceList->currentIndex() >= 0) {
                blockWRConfigToDevice(false);
            }
        });
    }
}

// slot after sending the config
void MainWindow::configSent(bool success)
{
    m_buttonDefaultStyle = ui->pushButton_ReadConfig->styleSheet();
    static QString button_default_text = ui->pushButton_WriteConfig->text();    //???
    // curves pointer activated
    m_axesCurvesConfig->deviceStatus(true);

    if (success == true)
    {
        ui->pushButton_WriteConfig->setText(tr("Sent"));
        ui->pushButton_WriteConfig->setStyleSheet(m_buttonDefaultStyle + "color: white; background-color: rgb(5, 170, 61);");

        QTimer::singleShot(1500, this, [&] {
            ui->pushButton_WriteConfig->setStyleSheet(m_buttonDefaultStyle);
            ui->pushButton_WriteConfig->setText(button_default_text);
            if (ui->comboBox_HidDeviceList->currentIndex() >= 0){
                blockWRConfigToDevice(false);
            }
        });
    } else {
        ui->pushButton_WriteConfig->setText(tr("Error"));
        ui->pushButton_WriteConfig->setStyleSheet(m_buttonDefaultStyle + "color: white; background-color: rgb(200, 0, 0);");

        QTimer::singleShot(1500, this, [&] {
            ui->pushButton_WriteConfig->setStyleSheet(m_buttonDefaultStyle);
            ui->pushButton_WriteConfig->setText(button_default_text);
            if (ui->comboBox_HidDeviceList->currentIndex() >= 0){
                blockWRConfigToDevice(false);
            }
        });
    }

    // remove device name from registry. sometimes windows does not update the name in gaming devices and has to be deleted in the registry
#ifdef Q_OS_WIN
    qDebug()<<"Remove device OEMName from registry";
    QString path("HKEY_CURRENT_USER\\System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_%1&PID_%2");
    QString path2("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_%1&PID_%2");
    QSettings(path.arg(QString::number(gEnv.pDeviceConfig->config.vid, 16), QString::number(gEnv.pDeviceConfig->config.pid, 16)),
              QSettings::NativeFormat).remove("OEMName");
    QSettings(path2.arg(QString::number(gEnv.pDeviceConfig->config.vid, 16), QString::number(gEnv.pDeviceConfig->config.pid, 16)),
              QSettings::NativeFormat).remove("OEMName");
#endif
}

void MainWindow::blockWRConfigToDevice(bool block)
{
    ui->pushButton_ReadConfig->setDisabled(block);
    ui->pushButton_WriteConfig->setDisabled(block);
}



/////////////////////    APP SETTINGS   /////////////////////

// slot language change
void MainWindow::languageChanged(const QString &language)
{
    qDebug()<<"Retranslate UI";

    auto trFunc = [&](const QString &file) {
        if (gEnv.pTranslator->load(file) == false) {
            qWarning()<<"failed to load translate file";
            return false;
        }
        qApp->installTranslator(gEnv.pTranslator);
        ui->retranslateUi(this);
        return true;
    };

    if (language == "english")
    {
        if (gEnv.pTranslator->load(":/NO_FILE_IS_OK!!_DEFAULT_TRANSLATE") == true) {
            qWarning()<<"failed to load translate file";
            return;
        }
        qApp->installTranslator(gEnv.pTranslator);
        ui->retranslateUi(this);
    }
    else if (language == "russian")
    {
        if (trFunc(":/FreeJoyQt_ru") == false) return;
    }
    else if (language == "schinese")
    {
        if (trFunc(":/FreeJoyQt_zh_CN") == false) return;
    }
    else if (language == "deutsch")
    {
        if (trFunc(":/FreeJoyQt_de_DE") == false) return;
    }
    else
    {
        return;
    }

    m_pinConfig->retranslateUi();
    m_buttonConfig->retranslateUi();
    m_ledConfig->retranslateUi();
    m_encoderConfig->retranslateUi();
    m_shiftRegConfig->retranslateUi();
    m_axesConfig->retranslateUi();
    m_axesCurvesConfig->retranslateUi();
    m_advSettings->retranslateUi();
    if(m_debugWindow){
        m_debugWindow->retranslateUi();
        ui->pushButton_ShowDebug->setText(tr("Hide debug"));
    }
    qDebug()<<"done";
}

// set font
void MainWindow::setFont()
{
    QWidgetList list = QApplication::allWidgets();
    for (QWidget *widget : list) {
        widget->setFont(QApplication::font());
        widget->update();
    }
}

// load app config
void MainWindow::loadAppConfig()
{
    QSettings *appS = gEnv.pAppSettings;
    qDebug()<<"Loading application config";
    // load window settings
    appS->beginGroup("WindowSettings");
    this->restoreGeometry(appS->value("Geometry").toByteArray());
    appS->endGroup();
    // load tab index
    appS->beginGroup("TabIndexSettings");
    ui->tabWidget->setCurrentIndex(appS->value("CurrentIndex", 0).toInt());
    appS->endGroup();
    // load debug window
    appS->beginGroup("OtherSettings");
    m_debugIsEnable = appS->value("DebugEnable", "false").toBool();
    if (m_debugIsEnable){
        on_pushButton_ShowDebug_clicked();
        if (this->isMaximized() == false){
            resize(width(), height() - 120 - ui->layoutG_MainLayout->verticalSpacing());
        }
    }
    appS->endGroup();
    // load configs dir path
    appS->beginGroup("Configs");
    m_cfgDirPath = appS->value("Path", gEnv.pAppSettings->fileName().remove("FreeJoySettings.conf") + "configs").toString();
    appS->endGroup();

// debug tab, only for debug build
#ifdef QT_DEBUG
#else
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tab_Developer));
#endif
    qDebug()<<"Finished loading application config";
}

// save app config
void MainWindow::saveAppConfig()
{
    QSettings *appS = gEnv.pAppSettings;
    qDebug()<<"Saving application config";
    // save tab index
    appS->beginGroup("TabIndexSettings");
    appS->setValue("CurrentIndex",    ui->tabWidget->currentIndex());
    appS->endGroup();
    // save window settings
    appS->beginGroup("WindowSettings");
    appS->setValue("Geometry",   this->saveGeometry());
    appS->endGroup();
    // save font settings
    appS->beginGroup("FontSettings");
    appS->setValue("FontSize", QApplication::font().pointSize());
    appS->endGroup();
    // save debug
    appS->beginGroup("OtherSettings");
    appS->setValue("DebugEnable", m_debugIsEnable);
    appS->endGroup();
    // save configs dir path
    appS->beginGroup("Configs");
    appS->setValue("Path", m_cfgDirPath);
    appS->setValue("LastCfg", ui->comboBox_Configs->currentText());
    appS->endGroup();
    qDebug()<<"done";
}


////////////////// buttons //////////////////
// comboBox selected device
void MainWindow::hidDeviceListChanged(int index)
{
    m_hidDeviceWorker->setSelectedDevice(index);
}

// reset all pins
void MainWindow::on_pushButton_ResetAllPins_clicked()
{
    qDebug()<<"Reset all started";
    gEnv.pDeviceConfig->resetConfig();

    UiReadFromConfig();

    m_pinConfig->resetAllPins();
    qDebug()<<"done";
}

// read config from device
void MainWindow::on_pushButton_ReadConfig_clicked()
{
    qDebug()<<"Read config started";
    blockWRConfigToDevice(true);

    m_hidDeviceWorker->getConfigFromDevice();
}

// write config to device
void MainWindow::on_pushButton_WriteConfig_clicked()
{
    qDebug()<<"Write config started";
    blockWRConfigToDevice(true);  // doesn’t have time to block? timer

    UiWriteToConfig();
    m_hidDeviceWorker->sendConfigToDevice();
}

// load from file
void MainWindow::on_pushButton_LoadFromFile_clicked()
{
    qDebug()<<"Load from file started";
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Config"), m_cfgDirPath + "/", tr("Config Files (*.cfg)"));

    ConfigToFile::loadDeviceConfigFromFile(this, fileName, gEnv.pDeviceConfig->config);
    UiReadFromConfig();
    qDebug()<<"done";
}

// save to file
void MainWindow::on_pushButton_SaveToFile_clicked()
{
    qDebug()<<"Save to file started";

    QString tmpStr(ui->comboBox_Configs->currentText());
    if (tmpStr == "") {
        tmpStr = gEnv.pDeviceConfig->config.device_name;
    }

    QFileInfo file(QFileDialog::getSaveFileName(this, tr("Save Config"),
                                                m_cfgDirPath + "/" + tmpStr, tr("Config Files (*.cfg)")));
    UiWriteToConfig();
    ConfigToFile::saveDeviceConfigToFile(file.absoluteFilePath(), gEnv.pDeviceConfig->config);

    QTimer::singleShot(200, this, [this, file]{
        QSignalBlocker bl(ui->comboBox_Configs);
        ui->comboBox_Configs->clear();
        ui->comboBox_Configs->addItems(cfgFilesList(m_cfgDirPath));
        bl.unblock();

        QString fileName(file.fileName());
        fileName.remove(fileName.size() - 4, 4); // 4 = ".cfg" characters count
        ui->comboBox_Configs->setCurrentText(fileName);
    });
    qDebug()<<"done";
}

// select configs dir path
void MainWindow::on_toolButton_ConfigsDir_clicked()
{
    SelectFolder dialog(m_cfgDirPath, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_cfgDirPath = dialog.folderPath();
        QSignalBlocker bl(ui->comboBox_Configs);
        ui->comboBox_Configs->clear();
        bl.unblock();
        ui->comboBox_Configs->addItems(cfgFilesList(m_cfgDirPath));
    }
}

/**
 * @brief Enables or disables Advanced Mode in the main window.
 *
 * Advanced Mode exposes additional configuration tabs and tools
 * intended for experienced users or developers. When enabled, it:
 * - Expands the UI to show the DebugWindow widget.
 * - Reveals normally hidden tabs such as Pin Config and Shift Registers.
 * - Updates the Advanced Mode toggle button text.
 *
 * When disabled, these UI elements are hidden again and the layout
 * is reduced to its normal size.
 *
 * @param enabled
 *   - `true`  → Enter Advanced Mode and reveal advanced UI elements.
 *   - `false` → Exit Advanced Mode and hide advanced UI elements.
 *
 * @note
 * - This function changes only the current UI state. It does not persist
 *   the Advanced Mode setting; persistence is handled separately in
 *   the application config save/load routines.
 * - This is the central implementation for toggling Advanced Mode;
 *   both the "Activate Advanced Mode" button and Developer Mode
 *   (`setDeveloperMode()`) should call into this function to ensure
 *   consistent behavior.
 */


void MainWindow::setAdvancedMode(bool enabled)
{
    if (m_debugIsEnable == enabled) {
        return; // already in desired state
    }

    if (enabled) {
        if (m_debugWindow == nullptr)
        {
            m_debugWindow = new DebugWindow(this);
            gEnv.pDebugWindow = m_debugWindow;
            ui->layoutV_DebugWindow->addWidget(m_debugWindow);
            m_debugWindow->hide();
        }

        m_debugWindow->setMinimumHeight(120);
        if (!this->isMaximized()) {
            resize(width(), height() + 120 + ui->layoutG_MainLayout->verticalSpacing());
        }
        m_debugWindow->setVisible(true);
        m_debugIsEnable = true;
        ui->pushButton_ShowDebug->setText(tr("Deactivate Advanced Mode"));

        // reveal hidden tabs useful in Advanced Mode


        int shiftRegIndex = ui->tabWidget->indexOf(ui->layoutV_tabShiftRegistersConfig->parentWidget());
        if (shiftRegIndex != -1) ui->tabWidget->setTabVisible(shiftRegIndex, true);
    }
    else {
        if (m_debugWindow) {
            m_debugWindow->setVisible(false);
            m_debugWindow->setMinimumHeight(0);
        }
        if (!this->isMaximized()) {
            resize(width(), height() - 120 - ui->layoutG_MainLayout->verticalSpacing());
        }
        m_debugIsEnable = false;
        ui->pushButton_ShowDebug->setText(tr("Activate Advanced Mode"));

        int shiftRegIndex = ui->tabWidget->indexOf(ui->layoutV_tabShiftRegistersConfig->parentWidget());
        if (shiftRegIndex != -1) ui->tabWidget->setTabVisible(shiftRegIndex, false);
    }
}

/**
 * @brief Slot triggered when the "Activate/Deactivate Advanced Mode" button is clicked.
 *
 * Toggles the current Advanced Mode state by calling `setAdvancedMode()`
 * with the opposite of the current `m_debugIsEnable` value.
 *
 * This provides the user-facing control for entering or exiting Advanced Mode.
 * Internally, all UI changes are handled by `setAdvancedMode()` to ensure
 * consistent behavior across different triggers (e.g., Developer Mode toggle).
 *
 * @see setAdvancedMode()
 */

void MainWindow::on_pushButton_ShowDebug_clicked()
{
    setAdvancedMode(!m_debugIsEnable);
}

void MainWindow::on_toolButton2_clicked()
{
    QMessageBox box(this);
    box.setWindowTitle(tr("What Board Do I Have?"));
    box.setText(tr("Identify your board by connector count on the left side."));
    box.setInformativeText(tr("Gen 1–2: Two 3-pin headers\n"
                              "Gen 3: One 4-pin header\n"
                              "Gen 4: One 6-pin header"));
    box.setIcon(QMessageBox::NoIcon); // prevent style’s default PNG
    box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(64,64)); // crisp at any DPI
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}


// Wiki Modified to Invictus logo and routing to invictuscockpits.com
void MainWindow::on_pushButton_Wiki_clicked()
{
    QDesktopServices::openUrl(QUrl("https://invictuscockpits.com"));
}


/**
    * @brief Applies a modular grip configuration to the UI.
         *
             * Sets logical button count and shift register config
                 * based on the given grip profile JSON. This assumes the profile was selected
                     * via comboBox_GripSelection and JSON loaded via configManager.
                         *
                             * @param cfg The QJsonObject representing the grip profile.
                                 */
void MainWindow::applyGripProfile(const QJsonObject &cfg)
{
    if (!m_shiftRegsPtrList.isEmpty()) {
        ShiftRegisters* sr = m_shiftRegsPtrList[0];

        // Set button count
        if (cfg.contains("buttons")) {
            int buttonCount = cfg["buttons"].toInt();
            qDebug() << "Setting button count to:" << buttonCount;

            QSpinBox* buttonCountSpin = sr->findChild<QSpinBox*>("spinBox_ButtonCount");
            if (buttonCountSpin) {
                buttonCountSpin->setValue(buttonCount);
            } else {
                qWarning() << "spinBox_ButtonCount not found!";
            }
        }

        // Set shift register type
        if (cfg.contains("shift_register_type")) {
            QString type = cfg["shift_register_type"].toString();
            qDebug() << "Setting shift register type to:" << type;

            QComboBox* regTypeCombo = sr->findChild<QComboBox*>("comboBox_ShiftRegType");
            if (regTypeCombo) {
                int index = regTypeCombo->findText(type);
                if (index >= 0)
                    regTypeCombo->setCurrentIndex(index);
                else
                    qWarning() << "Shift register type not found in comboBox:" << type;
            } else {
                qWarning() << "comboBox_ShiftRegType not found!";
            }
        }
    } else {
        qWarning() << "m_shiftRegsPtrList is empty!";
    }

    // Mapping physical buttons to logical buttons
    if (cfg.contains("logical_button_map")) {
        QJsonArray buttonMap = cfg["logical_button_map"].toArray();
        for (int i = 0; i < buttonMap.size() && i < MAX_LOGICAL_BUTTONS_GUI; ++i) {
            QJsonObject entry = buttonMap[i].toObject();

            int bit = entry.value("bit").toInt(-1);
            QString typeStr = entry.value("type").toString("BUTTON_NORMAL");
            QString nameStr = entry.value("name").toString();

            if (bit < 0) continue;

            ButtonLogical* btn = m_buttonConfig->logicButtons()[i];

            // Set function and physical mapping
            button_type_t typeEnum = Converter::StringToButtonType(typeStr);
            btn->setPhysicButton(bit);
            btn->setFunctionType(typeEnum);
            btn->setJsonName(nameStr);

            // Optional: Apply rename for BUTTON_NORMAL only
            if (typeEnum == BUTTON_NORMAL) {
                btn->renameButtonNormalLabel(nameStr);
            }

            btn->rememberGripOriginal();
            // <- store original name

        }
    }


}

/**
    * @brief Loads and applies the grip profile corresponding to the selected item.
    *
    * Triggered when the user changes the grip selection via comboBox_GripSelection.
    * Assumes that each item in the combo box has its associated JSON file set as userData.
    *
    * @param index The selected index in the combo box.
    */
void MainWindow::onGripSelectionChanged(int index)
{
    const QVariant v = ui->comboBox_GripSelection->itemData(index);
    if (!v.isValid()) return; // placeholder selected -> do nothing

    const QString filename = v.toString();
    qDebug() << "Grip profile filename:" << filename;

    const QString path = QCoreApplication::applicationDirPath()
                         + "/profiles/grips/" + filename;
    qDebug() << "Grip profile path:" << path;

    if (!configManager->loadGripProfile(path)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to load grip profile: %1").arg(filename));
        return;
    }
    applyGripProfile(configManager->gripConfig());
}

/**
 * @brief Handles global keyboard shortcuts and modifier state tracking.
 *
 * - Pressing Shift + A toggles visibility of the hidden "Advanced Mode"  built from existing debug button).
 * - Pressing Shift+ D toggles visibility of the hidden "Developer Mode" directly.  No button.
 * - Control key press is from existing code.  Unsure what it does
 * @param event The key press event.
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Shift + D -> toggle Developer Mode (show/hide Developer tab)
    if ((event->modifiers() & Qt::ShiftModifier) && event->key() == Qt::Key_D && !event->isAutoRepeat()) {
        setDeveloperMode(!m_developerMode);
        qDebug() << "Developer Mode:" << (m_developerMode ? "ON" : "OFF");
        return; // handled
    }

    // Shift + A -> toggle the Advanced Mode button visibility
    if ((event->modifiers() & Qt::ShiftModifier) && event->key() == Qt::Key_A && !event->isAutoRepeat()) {
        ui->pushButton_ShowDebug->setVisible(!ui->pushButton_ShowDebug->isVisible());
        qDebug() << "Key event: Shift + A";
        return; // handled
    }

    if (event->key() == Qt::Key_Control) {
        m_axesCurvesConfig->setExclusive(false);
    }

    QMainWindow::keyPressEvent(event);


    if (event->key() == Qt::Key_Control) {
        m_axesCurvesConfig->setExclusive(false);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        m_axesCurvesConfig->setExclusive(true);
    }
}

/**
 * @brief Remaps logical button functions depending on selected simulator.
 *
 * For Falcon BMS, remaps POV2–POV4 buttons to BUTTON_NORMAL since BMS only supports one POV hat.
 * For DCS and MSFS, restores original grip-defined logical button functions.
 *
 * This function assumes each ButtonLogical instance has already remembered its grip-assigned function
 * using `rememberGripOriginal()`.
 *
 * @param simName The name of the currently selected simulator (e.g., "DCS", "Falcon BMS", "MSFS").
 */

// --- POV buttons that should be downgraded in Falcon BMS ---
static const QList<int> falconPovRemapTargets = {
    POV2_UP, POV2_DOWN, POV2_LEFT, POV2_RIGHT,
    POV3_UP, POV3_DOWN, POV3_LEFT, POV3_RIGHT,
    POV4_UP, POV4_DOWN, POV4_LEFT, POV4_RIGHT
};
void MainWindow::remapLogicalButtonsForSim(const QString &simName)
{
    if (!m_buttonConfig)
        return;

    for (ButtonLogical* btn : m_buttonConfig->logicButtons()) {
        int currentType = btn->currentButtonType();

        if (simName == "Falcon BMS" && falconPovRemapTargets.contains(currentType)) {
            btn->setFunctionType(BUTTON_NORMAL);
            btn->renameButtonNormalLabel(btn->jsonName());  // <- reapply label
        }

        else if (simName == "DCS" || simName == "MSFS") {
            btn->restoreGripOriginal();  // Revert back to grip-defined function
        }
    }
}

/**
 * @brief Slot called when the selected simulator changes in the UI.
 *
 * Triggers remapping of logical buttons based on sim-specific rules.
 *
 * @param simName The name of the selected simulator (e.g., "DCS", "Falcon BMS").
 */
void MainWindow::onSimSoftwareChanged(const QString &simName)
{
    // Ignore the placeholder at index 0
    if (ui->comboBox_simSoftware->currentIndex() == 0)
        return;

    // (Optional: also ignore if empty text)
    if (simName.trimmed().isEmpty())
        return;

    remapLogicalButtonsForSim(simName);
}

/**
 * @brief Filters global Qt events before they reach their target.
 *
 * This override allows the MainWindow to intercept keyboard input
 * regardless of which widget currently has focus. It is used to
 * globally detect and handle the Shift+A shortcut to toggle the
 * visibility of the advanced mode debug button.
 *
 * @param obj The object receiving the event.
 * @param event The event being processed.
 * @return true if the event was handled and should not propagate further;
 *         false if the event should continue to its intended destination.
 */
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->modifiers() == Qt::ShiftModifier && keyEvent->key() == Qt::Key_A) {
            ui->pushButton_ShowDebug->setVisible(!ui->pushButton_ShowDebug->isVisible());
            qDebug() << "Shift+A toggled advanced mode button";
            return true; // event handled
        }        // NEW: Shift + D -> toggle Developer Mode *globally*
        if (keyEvent->modifiers() == Qt::ShiftModifier && keyEvent->key() == Qt::Key_D && !keyEvent->isAutoRepeat()) {
            setDeveloperMode(!m_developerMode);
            qDebug() << "Developer Mode:" << (m_developerMode ? "ON" : "OFF");
            return true; // handled
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

/**
 * @brief Enables or disables Developer Mode in the main window.
 *
 * Developer Mode reveals developer-only UI elements — currently the
 * "Developer" tab in the main tab widget — without requiring a debug build.
 * When turned on, the tab is dynamically inserted; when turned off, the tab
 * is removed. Other developer-only widgets can also be toggled here.
 *
 * This function is intended to be triggered via a special keypress
 * (e.g. Shift+D) rather than a visible menu or button, so end users
 * cannot easily access it.
 *
 * @param on
 *        - `true`  → Enable Developer Mode and insert the developer tab.
 *        - `false` → Disable Developer Mode and remove the developer tab.
 *
 * @note This does not persist the Developer Mode state between sessions.
 *       If persistence is needed, integrate with the app settings save/load
 *       mechanism.
 */

void MainWindow::setDeveloperMode(bool on)
{
    if (m_developerMode == on) return;
    m_developerMode = on;

    const int existing = ui->tabWidget->indexOf(m_devTab);

    if (on) {
        if (existing < 0 && m_devTab) {
            ui->tabWidget->addTab(m_devTab, m_devTabIcon, m_devTabText.isEmpty() ? tr("Developer") : m_devTabText);
        }
        ui->tabWidget->setCurrentWidget(m_devTab);
    } else {
        if (existing >= 0) {
            ui->tabWidget->removeTab(existing);
        }
    }
    // Show Pin Config tab in Developer Mode
    int pinTabIndex = ui->tabWidget->indexOf(ui->layoutV_tabPinConfig->parentWidget());
    if (pinTabIndex != -1) {
        ui->tabWidget->setTabVisible(pinTabIndex, on);
    }


    setAdvancedMode(on);
}
void MainWindow::onBoardPresetChanged(int index)
{
    const QVariant v = ui->comboBox_Board->itemData(index);
    if (!v.isValid()) return; // placeholder -> do nothing

    applyBoardPreset(static_cast<BoardId>(v.toInt()), /*ask=*/true);
    // persist if you want:
    QSettings s; s.setValue("BoardSettings/SelectedBoard", v);
}


/**
 * @brief Apply hardware-generation defaults (pins + axis sources) in one shot.
 *
 * This is the single entry point for switching between Gen 3 and Gen 4 boards.
 * It performs, in order:
 *  1) Copy the per-board **pin preset** into config (pins[]).
 *  2) Refresh the Pins UI (so pin-derived axis sources like "A1 – MCP3202" exist).
 *  3) Set **axis defaults** for X/Y:
 *     - Gen 3: source_main = A1 (MCP3202), channel X=0 / Y=1, resolution = 12-bit.
 *     - Gen 4: source_main = I2C, i2c_address = ADS1115_00 (0x48), channel X=0 / Y=1, resolution = 16-bit.
 *  4) Refresh the Axes UI to reflect the new selections.
 *
 * The function optionally asks for confirmation before overwriting the current mapping.
 *
 * @param id  Which board generation to apply (e.g., VftControllerGen3, VFTControllerGen4).
 * @param ask If true, shows a confirmation dialog before applying changes.
 *
 * @note Order matters: pins must be applied before axis presets so the
 *       Axis Source dropdowns contain the correct pin/I2C entries.
 * @sa boardPins(), PinConfig::readFromConfig(), AxesConfig::readFromConfig()
 */

void MainWindow::applyBoardPreset(BoardId id, bool ask)
{
    const QString name = (id == BoardId::VftControllerGen3)
    ? tr("VFT Controller Gen 1-3")
    : tr("VFT Controller Gen 4");

    if (ask) {
        QMessageBox box(this);
        box.setWindowTitle(tr("Apply board defaults?"));
        box.setText(tr("Replace current pin mapping with %1 defaults?").arg(name));
        box.setIcon(QMessageBox::NoIcon); // avoid the style’s PNG
        box.setIconPixmap(QIcon(":/Images/question_icon.svg").pixmap(64,64));
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.setDefaultButton(QMessageBox::Yes);

        if (box.exec() != QMessageBox::Yes)
            return;
    }

    // 1) Copy pins from presets into the live config
    // (1) Copy pins from preset
    const BoardPins& preset = boardPins(id);
    for (int i = 0; i < PINS_COUNT; ++i)
        gEnv.pDeviceConfig->config.pins[i] = preset.pins[i];

    // (2) Rebuild the Pin tab so AxisSource list gets updated (A1 – MCP3202 is added)
    if (m_pinConfig) m_pinConfig->readFromConfig();

    // (3) Apply axis defaults required by legacy behavior
    auto& cfg = gEnv.pDeviceConfig->config;

    constexpr int AXIS_X = 0;
    constexpr int AXIS_Y = 1;

    // Axes:: enum values (private there), mirrored here:
    constexpr int SRC_ENCODER = -3;
    constexpr int SRC_I2C     = -2;
    constexpr int SRC_NONE    = -1;
    constexpr int SRC_A1      = 1;   // None=-1, Encoder=-3, I2C=-2, A0=0, A1=1, ...

    if (id == BoardId::VftControllerGen3) {
        // Must explicitly select "A1 – MCP3202" for both X and Y
        cfg.axis_config[AXIS_X].source_main = SRC_A1;
        cfg.axis_config[AXIS_Y].source_main = SRC_A1;
        // Channels: X=0, Y=1
        cfg.axis_config[AXIS_X].channel = 0;
        cfg.axis_config[AXIS_Y].channel = 1;
        // Resolution: 12-bit (stored as bits-1)
        cfg.axis_config[AXIS_X].resolution = 12 - 1;
        cfg.axis_config[AXIS_Y].resolution = 12 - 1;

        // (optional) make sure “I2C” isn’t lingering in the list
        // if (m_axesConfig) m_axesConfig->addOrDeleteMainSource(SRC_I2C, QString(), false);

    } else { // Gen 4 (ADS1115 over I2C)
        // Ensure “I2C” exists in the Axis Source dropdowns
        if (m_axesConfig) m_axesConfig->addOrDeleteMainSource(SRC_I2C, tr("I2C"), true);

        cfg.axis_config[AXIS_X].source_main = SRC_I2C;
        cfg.axis_config[AXIS_Y].source_main = SRC_I2C;

        // ADS1115 default address 0x48 → matches AxesExtended::ADS1115_00 (enum = 1)
        cfg.axis_config[AXIS_X].i2c_address = AxesExtended::ADS1115_00;  // shows “ADS 1115_00”
        cfg.axis_config[AXIS_Y].i2c_address = AxesExtended::ADS1115_00;  // change to _01/_10/_11 if strapped


        // Channels X=0, Y=1
        cfg.axis_config[AXIS_X].channel = 0;
        cfg.axis_config[AXIS_Y].channel = 1;

        // Resolution: 16-bit (stored as bits-1)
        cfg.axis_config[AXIS_X].resolution = 16 - 1;
        cfg.axis_config[AXIS_Y].resolution = 16 - 1;
    }

    // (4) Paint Axes UI from config so the combos/spinboxes show the new selections
    if (m_axesConfig) m_axesConfig->readFromConfig();

    // persist board selection
    gEnv.pAppSettings->beginGroup("BoardSettings");
    gEnv.pAppSettings->setValue("SelectedBoard", id == BoardId::VftControllerGen3 ? 0 : 1);
    gEnv.pAppSettings->endGroup();
}

void MainWindow::showEvent(QShowEvent* e)
{
    QMainWindow::showEvent(e);

    // Mark GUI as ready (for AdvancedSettings gating)
    m_guiReady = true;
    qApp->setProperty("invcs.guiReady", true);

#ifdef Q_OS_WIN
    static bool s_ran = false;
    if (!s_ran) {
        s_ran = true;
        // Run after paint so any popup matches theme and doesn't pre-empt the window
        QTimer::singleShot(700, this, [this]{
            if (m_advSettings) m_advSettings->checkForUpdatesSilent();
        });
    }
#endif

    // Flush any queued “update available” signal that arrived pre-GUI
    if (!m_pendingUpdateTag.isEmpty()) {
        const QString tag = m_pendingUpdateTag;
        const QUrl    url = m_pendingUpdateUrl;
        m_pendingUpdateTag.clear();
        m_pendingUpdateUrl = QUrl();
        QTimer::singleShot(0, this, [this, tag, url]{ showUpdatePopup(tag, url); });
    }
}




void MainWindow::onUpdateAvailable(const QString& tag, const QUrl& url)
{
    if (!m_guiReady || !isVisible()) {
        m_pendingUpdateTag = tag;
        m_pendingUpdateUrl = url;
        return;
    }
    showUpdatePopup(tag, url);
}


void MainWindow::showUpdatePopup(const QString& tag, const QUrl& url)
{
    QMessageBox box(this);
    box.setWindowTitle(tr("Update available"));
    box.setText(tr("A newer version is available: %1\nYou have: %2")
                    .arg(tag, QString::fromLatin1(APP_VERSION)));
    box.setInformativeText(tr("Open the release page to download?"));
    box.setIcon(QMessageBox::NoIcon); // avoid style’s default PNG
    box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48)); // themed SVG
    QPushButton* open = box.addButton(tr("Open GitHub"), QMessageBox::AcceptRole);
    box.addButton(QMessageBox::Cancel);
    box.exec();
    if (box.clickedButton() == open)
        QDesktopServices::openUrl(url);
}

void MainWindow::wireForceButtons()
{
    // Buttons live in the Axes tab (.ui). Try several likely objectNames so this works
    // even if the .ui labels differ.
    auto tryConnect = [&](const char* name, auto slot) {
        if (QPushButton* b = this->findChild<QPushButton*>(name)) {
            connect(b, &QPushButton::clicked, this, slot);
            return true;
        }
        return false;
    };

    // Force level buttons (100 / 75 / 50)
    // Adjust names here to match your .ui if needed:
    tryConnect("pushButton_Force100", [this]{ applyAnchorsToAxes(100); });
    tryConnect("pushButton_Force75",  [this]{ applyAnchorsToAxes(75);  });
    tryConnect("pushButton_Force50",  [this]{ applyAnchorsToAxes(50);  });

    // Mode buttons (Digital = 25 lbf up; FLCS = 40 lbf up)
    // Again, tweak names to your .ui:
    tryConnect("pushButton_ModeDigital", [this]{
        m_anchorsMode = AnchorsMode_Digital25;
        // optionally reflect selection in the UI (checkable buttons, styles, etc.)
    });
    tryConnect("pushButton_ModeFLCS", [this]{
        m_anchorsMode = AnchorsMode_FLCS40;
    });
}

bool MainWindow::readAnchorsFromDevice(ForceAnchorsGUI* out)
{
    if (!out || !m_hidDeviceWorker) return false;

    QByteArray resp;
    bool ok = false;

    // Pattern copied from your Developer transport wiring: devRequest() then wait for devPacket(op) reply
    QEventLoop loop;
    QMetaObject::Connection c = connect(
        m_hidDeviceWorker, &HidDevice::devPacket, this,
        [&](quint8 op, const QByteArray& data){
            if (op == OP_GET_FACTORY_ANCHORS) { resp = data; ok = true; loop.quit(); }
        }
        );

    // Request anchors
    m_hidDeviceWorker->devRequest(OP_GET_FACTORY_ANCHORS, QByteArray());

    QTimer::singleShot(1000, &loop, &QEventLoop::quit); // 1s timeout is plenty here
    loop.exec();
    disconnect(c);

    return ok && unpackAnchors(resp, out);
}

int MainWindow::pickForPercent(const ForceTriplet& t, int percent) const
{
    switch (percent) {
    case 100: return t.adc100;
    case 75:  return t.adc75;
    case 50:  return t.adc50;
    default:  return t.adc100;
    }
}

void MainWindow::applyAnchorsToAxes(int percent)
{
    m_anchorsPercent = percent;

    // 1) Read anchors from device
    ForceAnchorsGUI a{};
    // after unpack + apply to axX/axY...
    m_cachedAnchors = a;
    m_cachedAnchorsValid = true;

   /* if (!readAnchorsFromDevice(&a)) {
        QMessageBox::warning(this, tr("Force Anchors"), tr("Unable to read anchors from device."));
        return;
    }*/

    // 2) Pick per-direction ADCs based on % and mode
    const int rollL = pickForPercent(a.rl_17, percent);  // roll-left @ % of 17 lbf
    const int rollR = pickForPercent(a.rr_17, percent);  // roll-right @ % of 17 lbf
    const int pitDn = pickForPercent(a.pd_17, percent);  // pitch-down @ % of 17 lbf
    const ForceTriplet& pu = (m_anchorsMode == AnchorsMode_Digital25) ? a.pu25 : a.pu40;
    const int pitUp = pickForPercent(pu, percent);       // pitch-up @ % of (25/40) lbf

    // 3) Write into config: min/center/max per axis
    // Axis indices: 0 = Roll (X), 1 = Pitch (Y)
    auto& cfgX = gEnv.pDeviceConfig->config.axis_config[0];
    auto& cfgY = gEnv.pDeviceConfig->config.axis_config[1];

    // Roll: ensure proper min/max ordering regardless of sensor polarity
    const int xMin = std::min(rollL, rollR);
    const int xMax = std::max(rollL, rollR);
    cfgX.calib_min    = xMin;
    cfgX.calib_max    = xMax;
    cfgX.calib_center = 0;

    // Pitch: down vs up; order them
    const int yMin = std::min(pitDn, pitUp);
    const int yMax = std::max(pitDn, pitUp);
    cfgY.calib_min    = yMin;
    cfgY.calib_max    = yMax;
    cfgY.calib_center = 0;

    // 4) Refresh the Axes tab from config so the UI reflects new calibration
    if (m_axesConfig) m_axesConfig->readFromConfig();


}
quint32 MainWindow::crc32_le(const QByteArray& data) const
{
    quint32 crc = 0xFFFFFFFFu;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(data.constData());
    int n = data.size();
    while (n--) {
        crc ^= *p++;
        for (int k = 0; k < 8; ++k)
            crc = (crc >> 1) ^ (0xEDB88320u & (-(qint32)(crc & 1)));
    }
    return ~crc;
}

QByteArray MainWindow::packAnchors(const ForceAnchorsGUI& a) const
{
    QByteArray b; b.reserve(46);

    auto put16 = [&](quint16 v){ b.append(char(v & 0xFF)); b.append(char(v >> 8)); };
    auto put32 = [&](quint32 v){ for (int i=0;i<4;++i) b.append(char((v>>(8*i)) & 0xFF)); };

    // header
    put16(a.magic);
    b.append(char(a.version));
    b.append(char(a.sealed));
    put32(0); // CRC placeholder

    // body (triplets)
    auto putTrip = [&](const ForceTriplet& t){
        put16(quint16(t.adc100));
        put16(quint16(t.adc75));
        put16(quint16(t.adc50));
    };
    putTrip(a.rl_17);
    putTrip(a.rr_17);
    putTrip(a.pd_17);
    putTrip(a.pu25);
    putTrip(a.pu40);

    // reserved (8 bytes)
    for (int i=0;i<8;++i) b.append(char(0));

    // write CRC over the whole struct as sent (CRC bytes were zero during compute)
    const quint32 crc = crc32_le(b);
    b[4] = char(crc & 0xFF);
    b[5] = char((crc >> 8) & 0xFF);
    b[6] = char((crc >> 16) & 0xFF);
    b[7] = char((crc >> 24) & 0xFF);

    return b;
}

bool MainWindow::unpackAnchors(const QByteArray& buf, ForceAnchorsGUI* out) const
{
    if (!out) return false;
    constexpr int kLen = 46;                 // exact payload length
    if (buf.size() < kLen) return false;

    const QByteArray view = buf.left(kLen);  // ignore any HID padding
    auto get16 = [&](int off){
        return quint16(quint8(view[off])) | (quint16(quint8(view[off+1])) << 8);
    };
    auto get32 = [&](int off){
        quint32 v = 0;
        for (int i=0;i<4;++i) v |= (quint32(quint8(view[off+i])) << (8*i));
        return v;
    };

    out->magic   = get16(0);
    out->version = quint8(view[2]);
    out->sealed  = quint8(view[3]);
    out->crc32   = get32(4);

    // verify CRC (zero the CRC field before recomputing)
    QByteArray tmp = view;
    tmp[4] = tmp[5] = tmp[6] = tmp[7] = char(0);


    int off = 8;
    auto getTrip = [&](ForceTriplet* t){
        t->adc100 = qint16(get16(off)); off += 2;
        t->adc75  = qint16(get16(off)); off += 2;
        t->adc50  = qint16(get16(off)); off += 2;
    };
    getTrip(&out->rl_17);
    getTrip(&out->rr_17);
    getTrip(&out->pd_17);
    getTrip(&out->pu25);
    getTrip(&out->pu40);

    return true;
}

//! QPixmap gray-scale image (an alpha map) to colored QIcon
QIcon MainWindow::pixmapToIcon(QPixmap pixmap, const QColor &color)
{
    // initialize painter to draw on a pixmap and set composition mode
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    // set color
    painter.setBrush(color);
    painter.setPen(color);
    // paint rect
    painter.drawRect(pixmap.rect());
    // here is our new colored icon
    return QIcon(pixmap);
}

void MainWindow::updateColor()
{
    QColor col = QApplication::palette().color(QPalette::Text);
    ui->toolButton_ConfigsDir->setIcon(pixmapToIcon(QPixmap(":/Images/setings.png"), col));
}





////////////////////////////////////////////////// debug tab //////////////////////////////////////////////////
// test buttons in debug tab
#ifdef QT_DEBUG
static dev_config_t testCfg;
#endif
void MainWindow::on_pushButton_TestButton_clicked()
{
#ifdef QT_DEBUG
    on_pushButton_LoadFromFile_clicked();
    testCfg = gEnv.pDeviceConfig->config;
    on_pushButton_WriteConfig_clicked();
#endif
}


void MainWindow::on_pushButton_TestButton_2_clicked()
{
#ifdef QT_DEBUG
    on_pushButton_ReadConfig_clicked();
    QElapsedTimer timer;
    timer.start();
    while (3000 > timer.elapsed()) {
        if (gEnv.readFinished) {
            qDebug()<<"compare before write to file";
            int cmp = memcmp(&testCfg, &gEnv.pDeviceConfig->config, sizeof(dev_config_t));
            if (cmp == 0) {
                qDebug()<<"equal";
            } else {
                qDebug()<<"ERROR. NOT EQUAL! memcmp ="<< cmp;
                return;
            }
            on_pushButton_SaveToFile_clicked();
            on_pushButton_LoadFromFile_clicked();
            qDebug()<<"final compare";
            cmp = memcmp(&testCfg, &gEnv.pDeviceConfig->config, sizeof(dev_config_t));
            if (cmp == 0) {
                qDebug()<<"equal";
            } else {
                qDebug()<<"ERROR. NOT EQUAL! memcmp ="<< cmp;
            }
            break;
        }
        QThread::msleep(50);
    }
#endif
}


