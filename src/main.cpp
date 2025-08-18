#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <QStyleFactory>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include "infolabel.h"
#include <QCoreApplication>

#include "global.h"
GlobalEnvironment gEnv;
#include "deviceconfig.h"

#include <QSslSocket>   // for TLS diagnostics

// Get the default Qt message handler.
static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(nullptr);

// define QT_NO_DEBUG_OUTPUT - no debug info
void CustomMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (gEnv.pDebugWindow != nullptr) {
        QMetaObject::invokeMethod(gEnv.pDebugWindow, "printMsg",
                                  Qt::QueuedConnection, Q_ARG(QString, msg));
    }
    (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);
}

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // Optional: see exactly why plugins load/fail
    qputenv("QT_DEBUG_PLUGINS", "1");

    QElapsedTimer time;
    time.start();

    QApplication a(argc, argv);  // <-- create the app FIRST

    // Make Qt look for TLS plugins in .\tls next to the EXE
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/tls");

    // TLS diagnostics (should say supportsSsl: true; loaded: Schannel if qschannelbackend.dll is found)
    qDebug() << "Qt" << qVersion()
             << "supportsSsl:" << QSslSocket::supportsSsl()
             << "built:" << QSslSocket::sslLibraryBuildVersionString()
             << "loaded:" << QSslSocket::sslLibraryVersionString();
    qDebug() << "LIB PATHS:" << QCoreApplication::libraryPaths();

    // Styles (do this AFTER QApplication exists; avoid using qApp before)
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication::setStyle(new InfoProxyStyle(QApplication::style()));

    qRegisterMetaType<QList<QPair<bool, QString>> >();

    QString docLoc = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!docLoc.isEmpty()) {
        docLoc += "/HOTAS/";
    }

    QDir dir(docLoc);
    if (!dir.exists()) {
        dir.mkpath(".");
        dir.mkpath("configs");
    }

    QSettings appSettings(docLoc + "HOTASSettings.conf", QSettings::IniFormat);

    DeviceConfig deviceConfig;
    QTranslator translator;

    // global
    gEnv.pAppSettings  = &appSettings;
    gEnv.pDeviceConfig = &deviceConfig;
    gEnv.pTranslator   = &translator;

    qInstallMessageHandler(CustomMessageHandler);
    gEnv.pApp_start_time = &time;

    // set font size
    gEnv.pAppSettings->beginGroup("FontSettings");
    QFont defaultFont = QApplication::font();
    defaultFont.setPointSize(gEnv.pAppSettings->value("FontSize", "8").toInt());
    qApp->setFont(defaultFont);
    gEnv.pAppSettings->endGroup();

    // load language settings
    appSettings.beginGroup("LanguageSettings");
    bool ok = false;
    const QString lang = appSettings.value("Language", "english").toString();
    if (lang == "russian") {
        ok = gEnv.pTranslator->load(":/FreeJoyQt_ru");
    } else if (lang == "schinese") {
        ok = gEnv.pTranslator->load(":/FreeJoyQt_zh_CN");
    } else if (lang == "deutsch") {
        ok = gEnv.pTranslator->load(":/FreeJoyQt_de_DE");
    }
    if (ok) qApp->installTranslator(gEnv.pTranslator);
    appSettings.endGroup();

    MainWindow w;
    qDebug() << "Application startup time =" << gEnv.pApp_start_time->elapsed() << "ms";
    w.show();

    return a.exec();
}
