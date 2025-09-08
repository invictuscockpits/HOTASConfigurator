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
#include "mainwindow.h"

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



    m_flasher = new Flasher(this);
    ui->layoutH_Flasher->addWidget(m_flasher);

    gEnv.pAppSettings->beginGroup("FontSettings");
    ui->spinBox_FontSize->setValue(gEnv.pAppSettings->value("FontSize", "8").toInt());
    gEnv.pAppSettings->endGroup();

    gEnv.pAppSettings->beginGroup("StyleSettings");
    QString style = gEnv.pAppSettings->value("StyleSheet", "default").toString();
    gEnv.pAppSettings->endGroup();

    m_deviceInfo = new DeviceInfo(this);
    connect(m_deviceInfo, &DeviceInfo::infoUpdated, this, &AdvancedSettings::onDeviceInfoUpdated);
    connect(m_deviceInfo, &DeviceInfo::errorOccurred, this, &AdvancedSettings::onDeviceInfoError);
    // Initialize line edits as read-only by default
    ui->line_DeviceModel->setReadOnly(true);
    ui->line_DeviceSerial->setReadOnly(true);
    ui->line_DeviceDoM->setReadOnly(true);

    // Apply stored device info if any
    if (!m_deviceModel.isEmpty() && ui->line_DeviceModel) {
        ui->line_DeviceModel->setText(m_deviceModel);
        ui->line_DeviceSerial->setText(m_deviceSerial);
        ui->line_DeviceDoM->setText(m_deviceDoM);
        ui->line_DeviceFwVersion->setText(m_deviceFwVersion);
    }

#ifndef Q_OS_WIN
    ui->text_removeName->setHidden(true);
    ui->pushButton_removeName->setHidden(true);
    ui->info_removeName->hide();
#endif

}

void AdvancedSettings::onDeviceInfoUpdated()
{
    ui->line_DeviceModel->setText(m_deviceInfo->getModel());
    ui->line_DeviceSerial->setText(m_deviceInfo->getSerial());
    ui->line_DeviceDoM->setText(m_deviceInfo->getDateOfManufacture());

    // Disable editing if locked
    bool locked = m_deviceInfo->isLocked();
    ui->line_DeviceModel->setReadOnly(locked);
    ui->line_DeviceSerial->setReadOnly(locked);
    ui->line_DeviceDoM->setReadOnly(locked);



}

void AdvancedSettings::onDeviceInfoError(const QString &error)
{
    QMessageBox::warning(this, tr("Device Info Error"), error);
}

void AdvancedSettings::on_pushButton_ReadDeviceInfo_clicked()
{

}

void AdvancedSettings::on_pushButton_WriteDeviceInfo_clicked()
{


    // Confirm before writing
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              tr("Write Device Info"),
                                                              tr("Are you sure you want to write device info to flash?"),
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    QString model = ui->line_DeviceModel->text();
    QString serial = ui->line_DeviceSerial->text();
    QString dom = ui->line_DeviceDoM->text();

    // Validate date format if provided
    if (!dom.isEmpty() && !QRegExp("\\d{4}-\\d{2}-\\d{2}").exactMatch(dom)) {
        QMessageBox::warning(this, tr("Error"), tr("Date must be in YYYY-MM-DD format"));
        return;
    }


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
void AdvancedSettings::checkForFirmwareUpdatesSilent()
{
#ifdef Q_OS_WIN
    checkForFirmwareUpdatesWinHTTP(true);
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
 * It performs a HTTPS GET to:
 *   https://api.github.com/repos/InvictusCockpits/HOTASConfigurator/releases/latest
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
    getPath(L"/repos/invictuscockpits/HOTASConfigurator/releases/latest", &body, &status);
    if (status == 404) {
        body.clear();
        status = 0;
        getPath(L"/repos/invictuscockpits/HOTASConfigurator/releases/", &body, &status);
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
/**
 * \brief Check GitHub for the latest release using Windows WinHTTP (Schannel).
 *
 * It performs a HTTPS GET to:
 *   https://api.github.com/repos/InvictusCockpits/invictusssc/releases/latest
 *
 * Behavior:
 *  - Adds a required User-Agent and Accept headers for the GitHub API.
 *  - Requests identity encoding to avoid dealing with gzip/deflate.
 *  - Parses JSON for "tag_name" and "html_url".
 *  - Compares against FIRMWARE_VERSION (normalized to N.N.N).
 *  - If newer, shows a dialog with an “Open GitHub” button and an
 *    “Ignore this version” checkbox stored under Update/IgnoreTag.
 *
 * \param silent If true, suppresses “up to date” / network error popups.
 */
#ifdef Q_OS_WIN
void AdvancedSettings::checkForFirmwareUpdatesWinHTTP(bool silent)
{
    const bool guiReady = qApp && qApp->property("invcs.guiReady").toBool();
    const bool effectiveSilent = silent || !guiReady;

    // ---- Open session/connection (identical headers as GUI checker) ----
    HINTERNET hSession = WinHttpOpen(L"InvictusHOTASConfigurator/1.0",
                                     WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        if (!effectiveSilent) showInfoBox(this, tr("Firmware Update"), tr("WinHTTP initialization failed (error %1).").arg((qulonglong)GetLastError()));
        return;
    }
    HINTERNET hConnect = WinHttpConnect(hSession, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); if (!effectiveSilent) showInfoBox(this, tr("Firmware Update"), tr("Connect failed (error %1).").arg((qulonglong)GetLastError())); return; }

    auto getPath = [&](const std::wstring& path, QByteArray* out, DWORD* outStatus)->bool {
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        if (!hRequest) return false;
        WinHttpAddRequestHeaders(hRequest, L"User-Agent: Invictus-SSC-Firmware\r\n", (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
        WinHttpAddRequestHeaders(hRequest, L"Accept: application/vnd.github+json\r\n", (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);

        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
            !WinHttpReceiveResponse(hRequest, nullptr)) { WinHttpCloseHandle(hRequest); return false; }

        DWORD status=0, len=sizeof(status);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &len, WINHTTP_NO_HEADER_INDEX);
        if (outStatus) *outStatus = status;

        QByteArray body;
        for (;;) {
            DWORD avail=0; if (!WinHttpQueryDataAvailable(hRequest, &avail) || !avail) break;
            QByteArray chunk; chunk.resize((int)avail);
            DWORD read=0; if (!WinHttpReadData(hRequest, chunk.data(), avail, &read)) break;
            chunk.resize((int)read); body += chunk;
        }
        WinHttpCloseHandle(hRequest);
        if (out) *out = body;
        return true;
    };

    // ---- Hit /releases/latest then fallback to /releases (same flow as GUI) ----
    QByteArray body; DWORD status = 0;
    std::wstring latest = L"/repos/" + QString(FW_REPO_OWNER).toStdWString() + L"/" + QString(FW_REPO_NAME).toStdWString() + L"/releases/latest";
    std::wstring list   = L"/repos/" + QString(FW_REPO_OWNER).toStdWString() + L"/" + QString(FW_REPO_NAME).toStdWString() + L"/releases";
    getPath(latest, &body, &status);
    if (status == 404) { body.clear(); status = 0; getPath(list, &body, &status); }

    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (status != 200 || body.isEmpty()) {
        if (!effectiveSilent) showInfoBox(this, tr("Firmware Update"), tr("GitHub returned HTTP %1.").arg(status));
        return;
    }

    // ---- Parse JSON to latest tag/url (same as GUI) ----
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
        if (!effectiveSilent) showInfoBox(this, tr("Firmware Update"), tr("No releases found."));
        return;
    }

    // Optional: enforce a prefix like "fw-v2.0.3"
    if (*FW_TAG_PREFIX && !latestTag.startsWith(QString::fromLatin1(FW_TAG_PREFIX), Qt::CaseInsensitive)) {
        // Ignore unrelated app releases in a shared repo
        return;
    }

    // ---- Compare device firmware vs tag (normalize just like GUI) ----
    auto normTag = [](QString v){
        v = v.trimmed();

        // Drop any leading non-digit/non-'v' garbage (handles fw-, release-, etc.)
        int start = 0;
        while (start < v.size() && !v[start].isDigit() && v[start].toLower() != 'v')
            ++start;
        v.remove(0, start);

        // If we now start with 'v' (e.g. "v2.1.1" or "v.2.1.1"), drop it
        if (v.startsWith('v', Qt::CaseInsensitive))
            v.remove(0, 1);

        // Also handle "v.2.1.1" → strip the extra dot
        if (v.startsWith('.'))
            v.remove(0, 1);

        // Keep only digits and dots until the first invalid char
        int i = 0;
        while (i < v.size() && (v[i].isDigit() || v[i] == '.'))
            ++i;

        return v.left(i);   // e.g. "2.1.1"
    };
    QString tagForCompare = latestTag;
    if (*FW_TAG_PREFIX && tagForCompare.startsWith(QString::fromLatin1(FW_TAG_PREFIX), Qt::CaseInsensitive))
        tagForCompare.remove(0, int(strlen(FW_TAG_PREFIX)));

    const QVersionNumber latestV = QVersionNumber::fromString(normTag(tagForCompare));

    // Convert the device’s hex version (shown as X.Y.Z) into QVersionNumber
    // You already build the dotted string from firmware_version; reuse the same rule here.
    const quint16 fwHex =
        gEnv.pDeviceConfig->paramsReport.firmware_version
            ? gEnv.pDeviceConfig->paramsReport.firmware_version
            : gEnv.pDeviceConfig->config.firmware_version;
    QString hex = QString::number(fwHex, 16).rightJustified(3, '0'); // e.g. "203"
    const QString devStr = QString("%1.%2.%3").arg(hex[0]).arg(hex[1]).arg(hex[2]);
    const QVersionNumber devV = QVersionNumber::fromString(devStr);

    if (latestV.isNull() || devV.isNull()) return;

    if (QVersionNumber::compare(latestV, devV) <= 0) {
        if (!effectiveSilent) showInfoBox(this, tr("Firmware Update"), tr("Your device firmware is up to date (%1).").arg(devStr));
        return;
    }

    // Newer firmware available
    if (effectiveSilent) {
        emit firmwareUpdateAvailable(latestTag, QUrl(releaseUrl));
        return;
    }

    QMessageBox box(this);
    box.setWindowTitle(tr("Firmware update available"));
    box.setText(tr("A newer firmware is available: %1\nDevice has: %2").arg(latestTag, devStr));
    box.setInformativeText(tr("Open the firmware release page?"));
    box.setIcon(QMessageBox::NoIcon);
    box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
    QPushButton* open = box.addButton(tr("Open GitHub"), QMessageBox::AcceptRole);
    box.addButton(QMessageBox::Cancel);
    box.exec();
    if (box.clickedButton() == open) QDesktopServices::openUrl(QUrl(releaseUrl));
}
#endif
void AdvancedSettings::on_pushButton_CheckFirmware_clicked()
{
#ifdef Q_OS_WIN
    // Call your firmware checker (see temp stub below).
    checkForFirmwareUpdatesWinHTTP(false);
#else
    QMessageBox::information(this, tr("Firmware Update"),
                             tr("Firmware update check is only implemented for Windows in this build."));
#endif
}

void AdvancedSettings::readFromConfig()
{

    // usb exchange period
    ui->spinBox_USBExchangePeriod->setValue(gEnv.pDeviceConfig->config.exchange_period_ms);
    // Read device identification from force anchors
    // Request this when the Read Config button is pressed
    ui->line_DeviceSerial->clear();
    ui->line_DeviceModel->clear();
    ui->line_DeviceDoM->clear();
}

void AdvancedSettings::writeToConfig()
{

    // usb exchange period
    gEnv.pDeviceConfig->config.exchange_period_ms = uint8_t(ui->spinBox_USBExchangePeriod->value());
}
void AdvancedSettings::applyDeviceIdentity(const params_report_t& r)
{


    // Firmware version: vX.Y.Z (first 3 nybbles, matching your status label)
    if (ui->line_DeviceFwVersion) {
        const quint16 v = r.firmware_version;                              // e.g. 0x2121
        const QString s = QString("%1").arg(v, 4, 16, QChar('0')).toUpper(); // "2121"
        ui->line_DeviceFwVersion->setText(
            (s.size() >= 4)
                ? QStringLiteral("v%1.%2.%3.b%4").arg(s[0]).arg(s[1]).arg(s[2]).arg(s[3])
                : QString() /* unexpected */);
    }

    // Date of Manufacture (not in params yet) — leave empty for now
    if (ui->line_DeviceDoM)
        ui->line_DeviceDoM->clear();
}
void AdvancedSettings::showDeviceInfo(const QString& model,
                                     const QString& serial,
                                     const QString& domISO,
                                     quint16 fwRaw)
{
    // Always store the values
    m_deviceModel = model;
    m_deviceSerial = serial;
    m_deviceDoM = domISO;

    const QString s = QString("%1").arg(fwRaw, 4, 16, QChar('0')).toUpper();
    m_deviceFwVersion = (s.size() >= 4) ? QStringLiteral("v%1.%2.%3.b%4").arg(s[0]).arg(s[1]).arg(s[2]).arg(s[3])
                                        : QString();

    // Update UI if it exists
    if (ui->line_DeviceModel) {
        ui->line_DeviceModel->setText(m_deviceModel);
        ui->line_DeviceSerial->setText(m_deviceSerial);
        ui->line_DeviceDoM->setText(m_deviceDoM);
        ui->line_DeviceFwVersion->setText(m_deviceFwVersion);
    }
}
