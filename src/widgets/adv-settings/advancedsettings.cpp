#include "advancedsettings.h"
#include "ui_advancedsettings.h"

//#include <QFile>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <qicon.h>
#include <QSettings>
#include <QTextStream>
#include <QTimer>
#include <QProcess>

#include "version.h"
#include "deviceconfig.h"
#include "global.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QVersionNumber>
#include <QCheckBox>
#include <QPushButton>
#include <QSslSocket>
#include <QJsonArray>
#include <QApplication>
#include <QUrl>


#ifdef Q_OS_WIN
#include <qt_windows.h>  // safer with Qt than <windows.h>
#include <winhttp.h>
#endif

static inline void showInfoBox(QWidget* parent, const QString& title, const QString& text)
{
    QMessageBox box(parent);
    box.setWindowTitle(title);
    box.setText(text);
    box.setIcon(QMessageBox::NoIcon);
    box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

AdvancedSettings::AdvancedSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AdvancedSettings)
{
    ui->setupUi(this);

    qDebug() << "Qt" << qVersion()
             << "supportsSsl:" << QSslSocket::supportsSsl()
             << "built:" << QSslSocket::sslLibraryBuildVersionString()
             << "loaded:" << QSslSocket::sslLibraryVersionString();

    m_flasher = new Flasher(this);
    ui->layoutH_Flasher->addWidget(m_flasher);

    gEnv.pAppSettings->beginGroup("FontSettings");
    ui->spinBox_FontSize->setValue(gEnv.pAppSettings->value("FontSize", "8").toInt());
    gEnv.pAppSettings->endGroup();

    gEnv.pAppSettings->beginGroup("StyleSettings");
    QString style = gEnv.pAppSettings->value("StyleSheet", "default").toString();
    gEnv.pAppSettings->endGroup();
    if (style == "dark") {
        ui->widget_StyleSwitch->setChecked(true);
    } else {
        ui->widget_StyleSwitch->setChecked(false);
    }
    connect(ui->widget_StyleSwitch, &SwitchButton::stateChanged, this, &AdvancedSettings::themeChanged);

#ifndef Q_OS_WIN
    ui->text_removeName->setHidden(true);
    ui->pushButton_removeName->setHidden(true);
    ui->info_removeName->hide();
#endif




    ui->layoutG_Lang->setAlignment(Qt::AlignCenter);
}
void AdvancedSettings::checkForUpdatesSilent()
{
#ifdef Q_OS_WIN
    // silent = true → no popups unless an update is available
    checkForUpdatesWinHTTP(true);
#else
    // Non-Windows: no-op
#endif
}
AdvancedSettings::~AdvancedSettings()
{
    delete ui;
}

void AdvancedSettings::retranslateUi()
{
    ui->retranslateUi(this);
    m_flasher->retranslateUi();
}

Flasher *AdvancedSettings::flasher() const
{
    return m_flasher;
}

void AdvancedSettings::on_pushButton_LangEnglish_clicked()
{
    gEnv.pAppSettings->beginGroup("LanguageSettings");
    gEnv.pAppSettings->setValue("Language", "english");
    gEnv.pAppSettings->endGroup();

    emit languageChanged("english");
}

void AdvancedSettings::on_pushButton_LangRussian_clicked()
{
    gEnv.pAppSettings->beginGroup("LanguageSettings");
    gEnv.pAppSettings->setValue("Language", "russian");
    gEnv.pAppSettings->endGroup();

    emit languageChanged("russian");
}

void AdvancedSettings::on_pushButton_LangSChinese_clicked()
{
    gEnv.pAppSettings->beginGroup("LanguageSettings");
    gEnv.pAppSettings->setValue("Language", "schinese");
    gEnv.pAppSettings->endGroup();

    emit languageChanged("schinese");
}

void AdvancedSettings::on_pushButton_LangDeutsch_clicked()
{
    gEnv.pAppSettings->beginGroup("LanguageSettings");
    gEnv.pAppSettings->setValue("Language", "deutsch");
    gEnv.pAppSettings->endGroup();

    emit languageChanged("deutsch");
}

void AdvancedSettings::on_pushButton_RestartApp_clicked()
{
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void AdvancedSettings::on_spinBox_FontSize_valueChanged(int fontSize)
{
    QFont defaultFont = QApplication::font();
    defaultFont.setPointSize(fontSize);
    qApp->setFont(defaultFont);

    emit fontChanged();
}


// about
void AdvancedSettings::on_pushButton_About_clicked()
{
    QDialog *aboutDialog = new QDialog(this);
    aboutDialog->setWindowTitle("About Invictus HOTAS Configurator");
    aboutDialog->setModal(true);
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
    aboutDialog->setStyleSheet(this->styleSheet());

    QVBoxLayout *layout = new QVBoxLayout(aboutDialog);
    layout->setContentsMargins(12, 12, 12, 20); // bottom margin
    layout->setSpacing(12);

    QLabel *label = new QLabel(aboutDialog);
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);

    const QString version = QStringLiteral(
                                R"(<div style="max-width: 450px; margin: 0 auto; text-align: center;">
            <p><b>Invictus HOTAS Configurator v%1</b></p>)").arg(APP_VERSION);

    const QString source = tr(R"(
        <p>Source code available under GPLv3 on
        <a style="color: #14B307; text-decoration:none;" href="https://github.com/invictuscockpits/HOTASConfigurator">GitHub</a>.</p>
        <p>This software and firmware are based on
        <a style="color: #14B307; text-decoration:none;" href="https://github.com/FreeJoy-Team/FreeJoyConfiguratorQt">FreeJoy</a>.
        We highly recommend starting there if you're building something similar.</p>)");

    const QString wiki = tr(R"(
        <p>Visit
        <a style="color: #14B307; text-decoration:none;" href="https://invictuscockpits.com">Invictus Cockpit Systems</a>
        for detailed instructions.</p>
        </div>)");

    label->setText(version + source + wiki);
    layout->addWidget(label, 0, Qt::AlignCenter);

    // Spacer: adds 20px space between text and button
    QSpacerItem *spacer = new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);
    layout->addSpacerItem(spacer);

    QPushButton *okButton = new QPushButton("OK", aboutDialog);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, aboutDialog, &QDialog::accept);
    layout->addWidget(okButton, 0, Qt::AlignHCenter);

    aboutDialog->setLayout(layout);
    aboutDialog->adjustSize();
    aboutDialog->resize(aboutDialog->width() + 75, aboutDialog->height()); // widen it
    aboutDialog->exec();
}


// remove name from registry
void AdvancedSettings::on_pushButton_removeName_clicked()
{
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

void AdvancedSettings::on_pushButton_CheckUpdates_clicked()
{
#ifdef Q_OS_WIN
    checkForUpdatesWinHTTP(false);
#else
    QMessageBox::information(this, tr("Update Check"),
                             tr("Update check is only implemented for Windows in this build."));
#endif
}

/**
 * \brief Check GitHub for the latest release using Windows WinHTTP (Schannel).
 *
 * This function avoids QtNetwork/OpenSSL entirely and relies on the OS TLS stack.
 * It performs a HTTPS GET to:
 *   https://api.github.com/repos/InvictusCockpits/InvictusHOTASConfigurator/releases/latest
 *
 * Behavior:
 *  - Adds a required User-Agent and Accept headers for the GitHub API.
 *  - Requests identity encoding to avoid dealing with gzip/deflate.
 *  - Parses JSON for "tag_name" and "html_url".
 *  - Compares against APP_VERSION (normalized to N.N.N).
 *  - If newer, shows a dialog with an “Open GitHub” button and an
 *    “Ignore this version” checkbox stored under Update/IgnoreTag.
 *
 * \param silent If true, suppresses “up to date” / network error popups.
 */

void AdvancedSettings::checkForUpdatesWinHTTP(bool silent)
{
    // Treat any pre-GUI call as silent, regardless of the caller.
    const bool guiReady = qApp && qApp->property("invcs.guiReady").toBool();
    const bool effectiveSilent = silent || !guiReady;

    // 1) Open session
    HINTERNET hSession = WinHttpOpen(L"InvictusHOTASConfigurator/1.0",
                                     WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        if (!effectiveSilent)
            showInfoBox(this, tr("Update Check"),
                        tr("WinHTTP initialization failed (error %1).")
                            .arg((qulonglong)GetLastError()));
        return;
    }

    // 2) Connect
    HINTERNET hConnect = WinHttpConnect(hSession, L"api.github.com",
                                        INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        if (!effectiveSilent)
            showInfoBox(this, tr("Update Check"),
                        tr("Network connect failed (error %1).")
                            .arg((qulonglong)GetLastError()));
        WinHttpCloseHandle(hSession);
        return;
    }

    // Helper to send GET and read body
    auto getPath = [&](const wchar_t* path, QByteArray* out, DWORD* outStatus)->bool {
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path, NULL,
                                                WINHTTP_NO_REFERER,
                                                WINHTTP_DEFAULT_ACCEPT_TYPES,
                                                WINHTTP_FLAG_SECURE);
        if (!hRequest) return false;

        // Required headers
        const wchar_t* baseHdrs =
            L"User-Agent: InvictusHOTASConfigurator\r\n"
            L"Accept: application/vnd.github+json\r\n"
            L"Accept-Encoding: identity\r\n"
            L"X-GitHub-Api-Version: 2022-11-28\r\n";
        WinHttpAddRequestHeaders(hRequest, baseHdrs, (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);

        // Optional token (env var or QSettings)
        QString token = qEnvironmentVariable("INVCS_GH_TOKEN");
        if (token.isEmpty()) {
            QSettings* st = gEnv.pAppSettings;
            st->beginGroup("Update");
            token = st->value("GitHubToken").toString();
            st->endGroup();
        }
        if (!token.isEmpty()) {
            const std::wstring auth = L"Authorization: Bearer " + token.toStdWString() + L"\r\n";
            WinHttpAddRequestHeaders(hRequest, auth.c_str(), (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);
        }

        bool ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                     WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
                  && WinHttpReceiveResponse(hRequest, NULL);
        if (!ok) { WinHttpCloseHandle(hRequest); return false; }

        DWORD status = 0, sz = sizeof(status);
        WinHttpQueryHeaders(hRequest,
                            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);

        QByteArray body;
        for (;;) {
            DWORD avail = 0; if (!WinHttpQueryDataAvailable(hRequest, &avail) || !avail) break;
            QByteArray chunk; chunk.resize((int)avail);
            DWORD read = 0; if (!WinHttpReadData(hRequest, chunk.data(), avail, &read)) break;
            chunk.resize((int)read); body += chunk;
        }
        WinHttpCloseHandle(hRequest);
        if (out) *out = body;
        if (outStatus) *outStatus = status;
        return true;
    };

    // 3) Try /releases/latest first; if 404, fall back to /releases
    QByteArray body;
    DWORD status = 0;
    getPath(L"/repos/InvictusCockpits/InvictusHOTASConfigurator/releases/latest", &body, &status);
    if (status == 404) {
        body.clear();
        status = 0;
        getPath(L"/repos/InvictusCockpits/InvictusHOTASConfigurator/releases", &body, &status);
    }

    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (status != 200 || body.isEmpty()) {
        if (!effectiveSilent)
            showInfoBox(this, tr("Update Check"),
                        tr("GitHub returned HTTP %1.").arg(status));
        return; // never pop in silent or pre-GUI
    }

    // 4) Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(body);
    QString latestTag, releaseUrl;

    if (doc.isObject()) {
        const auto obj = doc.object();
        latestTag  = obj.value("tag_name").toString();
        releaseUrl = obj.value("html_url").toString();
    } else if (doc.isArray()) {
        const auto arr = doc.array();
        for (const auto& v : arr) {
            const auto o = v.toObject();
            if (!o.value("draft").toBool() && !o.value("prerelease").toBool()) {
                latestTag  = o.value("tag_name").toString();
                releaseUrl = o.value("html_url").toString();
                break;
            }
        }
        if (latestTag.isEmpty() && !arr.isEmpty()) {
            const auto o = arr.first().toObject();
            latestTag  = o.value("tag_name").toString();
            releaseUrl = o.value("html_url").toString();
        }
    }

    if (latestTag.isEmpty()) {
        if (!effectiveSilent)
            showInfoBox(this, tr("Update Check"), tr("No releases found."));
        return;
    }

    // Respect “ignore this tag”
    QSettings* s = gEnv.pAppSettings;
    s->beginGroup("Update");
    const QString ignored = s->value("IgnoreTag").toString();
    s->endGroup();
    if (ignored == latestTag) return;

    auto norm = [](QString v){ v=v.trimmed(); if(v.startsWith('v',Qt::CaseInsensitive)) v.remove(0,1);
        int i=0; while(i<v.size() && (v[i].isDigit()||v[i]=='.')) ++i; return v.left(i); };
    const QVersionNumber cur = QVersionNumber::fromString(norm(QString::fromLatin1(APP_VERSION)));
    const QVersionNumber lat = QVersionNumber::fromString(norm(latestTag));
    if (lat.isNull() || cur.isNull()) return;

    if (QVersionNumber::compare(lat, cur) <= 0) {
        if (!effectiveSilent)
            showInfoBox(this, tr("Up to date"),
                        tr("You’re on the latest version (%1).")
                            .arg(QString::fromLatin1(APP_VERSION)));
        return;
    }

    // Newer version available
    if (effectiveSilent) {
        emit updateAvailable(latestTag, QUrl(releaseUrl)); // MainWindow will show it when ready
        return;
    }

    // Manual check (not silent) -> show dialog now
    QMessageBox box(this);
    box.setWindowTitle(tr("Update available"));
    box.setText(tr("A newer version is available: %1\nYou have: %2")
                    .arg(latestTag, QString::fromLatin1(APP_VERSION)));
    box.setInformativeText(tr("Open the release page to download?"));
    box.setIcon(QMessageBox::NoIcon);
    box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
    QCheckBox dontShow(tr("Don’t remind me again for %1").arg(latestTag));
    box.setCheckBox(&dontShow);
    QPushButton* open = box.addButton(tr("Open GitHub"), QMessageBox::AcceptRole);
    box.addButton(QMessageBox::Cancel);
    box.exec();

    if (dontShow.isChecked()) {
        s->beginGroup("Update");
        s->setValue("IgnoreTag", latestTag);
        s->endGroup();
    }
    if (box.clickedButton() == open)
        QDesktopServices::openUrl(QUrl(releaseUrl));
}



void AdvancedSettings::readFromConfig()
{
    // PID
    ui->lineEdit_VID->setText(QString::number(gEnv.pDeviceConfig->config.vid, 16).toUpper().rightJustified(4, '0'));
    // PID
    //ui->lineEdit_PID->setInputMask("HHHH");
    ui->lineEdit_PID->setText(QString::number(gEnv.pDeviceConfig->config.pid, 16).toUpper().rightJustified(4, '0'));
    // device name
    ui->lineEdit_DeviceUSBName->setText(gEnv.pDeviceConfig->config.device_name);
    // usb exchange period
    ui->spinBox_USBExchangePeriod->setValue(gEnv.pDeviceConfig->config.exchange_period_ms);
}

void AdvancedSettings::writeToConfig()
{
    // VID
    gEnv.pDeviceConfig->config.vid = uint16_t(ui->lineEdit_VID->text().toInt(nullptr, 16));
    // PID
    gEnv.pDeviceConfig->config.pid = uint16_t(ui->lineEdit_PID->text().toInt(nullptr, 16));
    // device name
    std::string tmp_string = ui->lineEdit_DeviceUSBName->text().toStdString();
    for (uint i = 0; i < sizeof(gEnv.pDeviceConfig->config.device_name); i++) {
        if (i < tmp_string.size()) {
            gEnv.pDeviceConfig->config.device_name[i] = tmp_string[i];
        } else {
            gEnv.pDeviceConfig->config.device_name[i] = '\0';
        }
    }
    // usb exchange period
    gEnv.pDeviceConfig->config.exchange_period_ms = uint8_t(ui->spinBox_USBExchangePeriod->value());
}
