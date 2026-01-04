#include "developer.h"
#include "ui_developer.h"
#include "common_defines.h"
#include "common_types.h"

#include <QSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <Qdebug>
#include <QLineEdit>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QKeyEvent>
#include <QApplication>
#include <QEvent>
#include <algorithm>
#include "adv-settings/text_fit_helpers.h"

Developer::Developer(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::Developer)
{
    ui->setupUi(this);
    setDefaultSpinRanges();
    initConnections();
    // Bind every Set button named "btnSet_*"
    bindSetButtons();

    // Set default PGA values - all channels PGA 8 for differential mode
    // ComboBox indices: 0=PGA1, 1=PGA2, 2=PGA4, 3=PGA8, 4=PGA16
    ui->comboBox_PGA_Ch0->setCurrentIndex(3);  // PGA 8 (±0.512V) for Ch0
    ui->comboBox_PGA_Ch1->setCurrentIndex(3);  // PGA 8 (±0.512V) for Ch1
    ui->comboBox_PGA_Ch2->setCurrentIndex(3);  // PGA 8 (±0.512V) for Ch2
    ui->comboBox_PGA_Ch3->setCurrentIndex(3);  // PGA 8 (±0.512V) for Ch3

    // Set default Mode to Differential
    // ComboBox indices: 0=Single-Ended, 1=Differential
    ui->comboBox_ADCMode->setCurrentIndex(1);  // Differential

    auto prepLine = [](QLineEdit* le){
        if (!le) return;
        le->setReadOnly(true);
        le->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QFont f = le->font(); f.setStyleHint(QFont::Monospace); f.setFamily("Consolas");
        le->setFont(f);
    };
    prepLine(ui->lineEdit_RawX);
    prepLine(ui->lineEdit_RawY);

    // start at 0
    updateRawReadouts();
}

Developer::~Developer()
{
    delete ui;
}

void Developer::initConnections()
{

    connect(ui->btnAnchorsRead,   &QPushButton::clicked, this, &Developer::onRead);
    connect(ui->btnAnchorsWrite,  &QPushButton::clicked, this, &Developer::onWrite);
    connect(ui->btnAnchorsLock,   &QPushButton::clicked, this, &Developer::onLock);
    connect(ui->btnAnchorsImport, &QPushButton::clicked, this, &Developer::onImport);
    connect(ui->btnAnchorsExport, &QPushButton::clicked, this, &Developer::onExport);
    connect(ui->btnDeviceInfoWrite, &QPushButton::clicked, this, &Developer::onWriteDeviceInfo);
    connect(ui->btnPGARead, &QPushButton::clicked, this, &Developer::onReadPGA);
    connect(ui->btnPGAWrite, &QPushButton::clicked, this, &Developer::onWritePGA);

}

void Developer::setDefaultSpinRanges()
{
    const int min = -32768, max = 32767;
    // Only touch the known spin boxes; no renames
    QSpinBox* spins[] = {
        ui->spRL_100, ui->spRL_75, ui->spRL_50,
        ui->spRR_100, ui->spRR_75, ui->spRR_50,
        ui->spPD_100, ui->spPD_75, ui->spPD_50,
        ui->spPUD_100, ui->spPUD_75, ui->spPUD_50,
        ui->spPUA_100, ui->spPUA_75, ui->spPUA_50
    };
    for (QSpinBox* sp : spins) {
        if (!sp) continue;
        sp->setRange(min, max);
        sp->setSingleStep(1);
        sp->setAlignment(Qt::AlignCenter);
    }
}

/* ===================== CRC32 (little-endian, poly 0xEDB88320) ===================== */
quint32 Developer::crc32_le(const QByteArray& data) const
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


/* ===================== UI <-> struct ===================== */
void Developer::uiSet(const Anchors& a)
{
    // Roll Left
    ui->spRL_100->setValue(a.rl_17.adc100);
    ui->spRL_75 ->setValue(a.rl_17.adc75);
    ui->spRL_50 ->setValue(a.rl_17.adc50);

    // Roll Right
    ui->spRR_100->setValue(a.rr_17.adc100);
    ui->spRR_75 ->setValue(a.rr_17.adc75);
    ui->spRR_50 ->setValue(a.rr_17.adc50);

    // Pitch Down
    ui->spPD_100->setValue(a.pd_17.adc100);
    ui->spPD_75 ->setValue(a.pd_17.adc75);
    ui->spPD_50 ->setValue(a.pd_17.adc50);

    // Pitch Up Digital (25 lbf)
    ui->spPUD_100->setValue(a.pu25.adc100);
    ui->spPUD_75 ->setValue(a.pu25.adc75);
    ui->spPUD_50 ->setValue(a.pu25.adc50);

    // Pitch Up Analog (40 lbf)
    ui->spPUA_100->setValue(a.pu40.adc100);
    ui->spPUA_75 ->setValue(a.pu40.adc75);
    ui->spPUA_50 ->setValue(a.pu40.adc50);

    // Set device identification fields
    ui->lineEdit_SerialNumber->setText(a.serialNumber);
    ui->lineEdit_Model->setText(a.modelNumber);
    ui->lineEdit_ManufactureDate->setText(a.manufactureDate);
}

auto Developer::uiGet() const -> Anchors
{
    Anchors a;
    // Roll Left
    a.rl_17.adc100 = (qint16)ui->spRL_100->value();
    a.rl_17.adc75  = (qint16)ui->spRL_75 ->value();
    a.rl_17.adc50  = (qint16)ui->spRL_50 ->value();

    // Roll Right
    a.rr_17.adc100 = (qint16)ui->spRR_100->value();
    a.rr_17.adc75  = (qint16)ui->spRR_75 ->value();
    a.rr_17.adc50  = (qint16)ui->spRR_50 ->value();

    // Pitch Down
    a.pd_17.adc100 = (qint16)ui->spPD_100->value();
    a.pd_17.adc75  = (qint16)ui->spPD_75 ->value();
    a.pd_17.adc50  = (qint16)ui->spPD_50 ->value();

    // Pitch Up Digital (25 lbf)
    a.pu25.adc100  = (qint16)ui->spPUD_100->value();
    a.pu25.adc75   = (qint16)ui->spPUD_75 ->value();
    a.pu25.adc50   = (qint16)ui->spPUD_50 ->value();

    // Pitch Up Analog (40 lbf)
    a.pu40.adc100  = (qint16)ui->spPUA_100->value();
    a.pu40.adc75   = (qint16)ui->spPUA_75 ->value();
    a.pu40.adc50   = (qint16)ui->spPUA_50 ->value();

    // Get device identification fields
    a.serialNumber = ui->lineEdit_SerialNumber->text();
    a.modelNumber = ui->lineEdit_Model->text();
    a.manufactureDate = ui->lineEdit_ManufactureDate->text();
    return a;
}

/* ===================== pack / unpack (match device struct layout) ===================== */
QByteArray Developer::pack(const Anchors& a) const
{
    QByteArray b; b.reserve(sizeof(Anchors));

    auto put16 = [&](quint16 v){ b.append(char(v & 0xFF)); b.append(char(v >> 8)); };
    auto put32 = [&](quint32 v){ for(int i=0;i<4;++i) b.append(char((v>>(8*i)) & 0xFF)); };

    put16(a.magic);
    b.append(char(a.version));
    b.append(char(a.sealed));
    put32(0); // CRC placeholder

    auto putTrip = [&](const Triplet& t){
        put16((quint16)t.adc100);
        put16((quint16)t.adc75);
        put16((quint16)t.adc50);
    };

    putTrip(a.rl_17);
    putTrip(a.rr_17);
    putTrip(a.pd_17);
    putTrip(a.pu25);
    putTrip(a.pu40);

    for (int i=0;i<8;i++) b.append(char(0));

    // compute CRC over entire struct (with crc field included)
    quint32 crc = crc32_le(b);
    b[4] = char(crc & 0xFF);
    b[5] = char((crc >> 8) & 0xFF);
    b[6] = char((crc >> 16) & 0xFF);
    b[7] = char((crc >> 24) & 0xFF);

    // Pack device identification strings - MUST be exact lengths
    auto packString = [&](const QString& str, int maxLen) {
        QByteArray bytes = str.toUtf8();
        // Ensure we always write exactly maxLen bytes
        for (int i = 0; i < maxLen; i++) {
            if (i < bytes.size()) {
                b.append(bytes[i]);
            } else {
                b.append(char(0));  // pad with zeros
            }
        }
    };

    for (int i=0;i<8;i++) b.append(char(0));  // reserved bytes

    return b;
}

bool Developer::unpack(const QByteArray& b, Anchors* out) const
{
    if (!out) return false;

    constexpr int kLen = 46;
    if (b.size() < kLen) return false;

    // Use only the first 46 bytes, ignore HID padding
    const QByteArray view = b.left(kLen);

    auto get16 = [&](int off){ return quint16(quint8(view[off])) | (quint16(quint8(view[off+1]))<<8); };
    auto get32 = [&](int off){ quint32 v=0; for (int i=0;i<4;++i) v |= (quint32(quint8(view[off+i]))<<(8*i)); return v; };

    out->magic   = get16(0);
    out->version = quint8(view[2]);
    out->sealed  = quint8(view[3]);
    out->crc32   = get32(4);

    QByteArray tmp = view;
    tmp[4] = tmp[5] = tmp[6] = tmp[7] = char(0);
    const quint32 calc = crc32_le(tmp);

    if (calc != out->crc32) {
        qWarning().noquote() << "[Anchors] CRC mismatch: got=0x"
                             << QString::number(out->crc32,16)
                             << " calc=0x" << QString::number(calc,16)
                             << " len=" << b.size();
        // Soft-accept if header looks right:
        if (out->magic != 0xF00C || (out->version != 0x01 && out->version != 0x02))
            return false;
    }

    int off = 8;
    auto getTrip = [&](Triplet* t){
        t->adc100 = qint16(get16(off)); off += 2;
        t->adc75  = qint16(get16(off)); off += 2;
        t->adc50  = qint16(get16(off)); off += 2;
    };
    getTrip(&out->rl_17);
    getTrip(&out->rr_17);
    getTrip(&out->pd_17);
    getTrip(&out->pu25);
    getTrip(&out->pu40);

    // Skip the 8 reserved bytes (already consumed by getTrip calls)
    // Total should be 46 bytes: 8 header + 30 triplets + 8 reserved

    return true;
}

/* ===================== Buttons ===================== */
void Developer::onRead()
{
    Anchors a{};
    bool haveAnchors = false;


    if (m_send && m_recv) {
        QByteArray req, resp;
        if (m_send(OP_GET_FACTORY_ANCHORS, QByteArray()) && m_recv(OP_GET_FACTORY_ANCHORS, &resp)) {
            // Debug output
            qDebug() << "[Developer] Received anchors:";
            //qDebug() << "  - Response size:" << resp.size();
            //qDebug() << "  - First 16 bytes:" << resp.left(16).toHex(' ');

            haveAnchors = unpack(resp, &a);
            // consider anchors "missing" only if unpack failed OR all trips are zero
            if (haveAnchors &&
                isEmptyTrip(a.rl_17) && isEmptyTrip(a.rr_17) &&
                isEmptyTrip(a.pd_17)  && isEmptyTrip(a.pu25)  && isEmptyTrip(a.pu40)) {
                haveAnchors = false;
            }
        }
    }

    if (!haveAnchors && m_fallback) {
        Calib roll{}, pitch{};
        if (m_fallback(&roll, &pitch)) {
            auto lerp = [](qint16 c, qint16 e, float f){ return qint16(c + (e - c) * f); };

            a.magic   = 0xF00C;  // FACTORY_MAGIC
            a.version = 0x02;    // FACTORY_VERSION
            a.sealed  = 0;

            // Roll: left=min, right=max
            a.rl_17 = { roll.min,  lerp(roll.center, roll.min,  0.75f), lerp(roll.center, roll.min,  0.5f) };
            a.rr_17 = { roll.max,  lerp(roll.center, roll.max,  0.75f), lerp(roll.center, roll.max,  0.5f) };

            // Pitch: down=min, up=max (we use same triplet for both pitch-up paths)
            a.pd_17 = { pitch.min, lerp(pitch.center, pitch.min, 0.75f), lerp(pitch.center, pitch.min, 0.5f) };
            a.pu25  = { pitch.max, lerp(pitch.center, pitch.max, 0.75f), lerp(pitch.center, pitch.max, 0.5f) };
            a.pu40  = a.pu25;

            haveAnchors = true;

            QMessageBox box(window());  // Use main window as parent
            box.setWindowTitle(tr("Force Anchors"));
            box.setText(tr("Factory anchors missing; using calibration defaults."));

            box.setIcon(QMessageBox::NoIcon); // prevent style's default PNG
            box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48)); // crisp at any DPI
            box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                             "QMessageBox QLabel { background-color: transparent; }"
                             "QMessageBox QFrame { background-color: transparent; }");
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();
        }
    }

    if (!haveAnchors) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle(tr("Force Anchors"));
        box.setText(tr("Unable to read anchors and no fallback available."));

        box.setIcon(QMessageBox::NoIcon); // prevent style's default PNG
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48)); // crisp at any DPI
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();

    }



    uiSet(a);

    // Update lock button text based on sealed state
    if (ui->btnAnchorsLock) {
        if (a.sealed == 1) {
            ui->btnAnchorsLock->setText(tr("Unlock Force Anchors"));
            ui->btnAnchorsLock->setEnabled(true);
            if (ui->btnAnchorsWrite) ui->btnAnchorsWrite->setEnabled(false);
        } else {
            ui->btnAnchorsLock->setText(tr("Lock Force Anchors"));
            ui->btnAnchorsLock->setEnabled(true);
            if (ui->btnAnchorsWrite) ui->btnAnchorsWrite->setEnabled(true);
        }
    }
}
static QString hexDump(const QByteArray& ba, int headBytes = 16) {
    const int n = qMin(headBytes, ba.size());
    QString s; s.reserve(3*n);
    for (int i=0;i<n;++i) s += QString("%1 ").arg(quint8(ba.at(i)),2,16,QLatin1Char('0'));
    return s.trimmed().toUpper();
}

static int firstDiff(const QByteArray& a, const QByteArray& b) {
    const int n = qMin(a.size(), b.size());
    for (int i=0; i<n; ++i) if (a.at(i) != b.at(i)) return i;
    return (a.size() == b.size()) ? -1 : n; // -1 means equal
}

void Developer::onWrite()
{
    if (!m_send || !m_recv) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle("Anchors");
        box.setText(tr("Transport not set"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Pre-read: block if sealed
    QByteArray resp;
    Anchors current{};
    if (m_send(OP_GET_FACTORY_ANCHORS, QByteArray()) &&
        m_recv(OP_GET_FACTORY_ANCHORS, &resp) &&
        unpack(resp, &current) && current.sealed == 1)
    {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle("Anchors");
        box.setText(tr("Anchors are locked; writing is not allowed."));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    Anchors a = uiGet();
    QByteArray blob = pack(a), ack;

    // Debug output
    qDebug() << "[Developer] Sending anchors:";
    //qDebug() << "  - Blob size:" << blob.size();
    //qDebug() << "  - First 16 bytes:" << blob.left(16).toHex(' ');

    if (!m_send(OP_SET_FACTORY_ANCHORS, blob) || !m_recv(OP_SET_FACTORY_ANCHORS, &ack)) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle("Anchors");
        box.setText(tr("Write failed"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    QMessageBox box(window());  // Use main window as parent
    box.setWindowTitle("Anchors");
    box.setText(tr("Anchors written."));
    box.setIcon(QMessageBox::NoIcon);
    box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
    box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                     "QMessageBox QLabel { background-color: transparent; }");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}
void Developer::onLock()
{
    if (!m_send || !m_recv) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle(tr("Force Anchors"));
        box.setText(tr("Transport not set."));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // First, read current state to determine if we should lock or unlock
    QByteArray resp;
    Anchors current{};
    if (!m_send(OP_GET_FACTORY_ANCHORS, QByteArray()) ||
        !m_recv(OP_GET_FACTORY_ANCHORS, &resp) ||
        !unpack(resp, &current)) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle(tr("Force Anchors"));
        box.setText(tr("Unable to read current anchor state."));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    const bool isCurrentlyLocked = (current.sealed == 1);
    const quint8 operation = isCurrentlyLocked ? OP_UNLOCK_FACTORY_ANCHORS : OP_LOCK_FACTORY_ANCHORS;
    const QString actionName = isCurrentlyLocked ? tr("unlock") : tr("lock");

    // qDebug() << "[Developer] Lock/Unlock operation:";
    // qDebug() << "  - Current sealed state:" << current.sealed;
    // qDebug() << "  - Operation:" << (isCurrentlyLocked ? "UNLOCK" : "LOCK");
    // qDebug() << "  - OpCode:" << operation;

    // Send lock or unlock command
    QByteArray ack;
    const bool sent = m_send(operation, QByteArray());
    const bool got = m_recv(operation, &ack);

    // qDebug() << "  - Send result:" << sent;
    // qDebug() << "  - Recv result:" << got;
    // qDebug() << "  - ACK size:" << ack.size();
    // if (!ack.isEmpty()) {
    //     qDebug() << "  - ACK data:" << ack.toHex(' ');
    // }

    if (!sent || !got) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle(tr("Force Anchors"));
        box.setText(tr("Failed to %1 anchors (no reply).").arg(actionName));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    const quint8 status = ack.isEmpty() ? 1 : quint8(ack.at(0)); // 1=OK, 0=fail
    // qDebug() << "  - Status byte:" << status;

    if (status != 1) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle(tr("Force Anchors"));
        box.setText(tr("Device refused %1.").arg(actionName));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Read back and verify the sealed bit changed
    Anchors a{};
    if (m_send(OP_GET_FACTORY_ANCHORS, QByteArray()) && m_recv(OP_GET_FACTORY_ANCHORS, &resp) && unpack(resp, &a)) {
        uiSet(a);

        const bool nowLocked = (a.sealed == 1);

        if (isCurrentlyLocked && !nowLocked) {
            // Successfully unlocked
            if (ui->btnAnchorsLock) ui->btnAnchorsLock->setText(tr("Lock Force Anchors"));
            if (ui->btnAnchorsWrite) ui->btnAnchorsWrite->setEnabled(true);
            QMessageBox box(window());  // Use main window as parent, not widget
            box.setWindowTitle(tr("Force Anchors"));
            box.setText(tr("Anchors unlocked."));
            box.setIcon(QMessageBox::NoIcon);
            box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
            box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                             "QMessageBox QLabel { background-color: transparent; }");
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();
            return;
        } else if (!isCurrentlyLocked && nowLocked) {
            // Successfully locked
            if (ui->btnAnchorsLock) ui->btnAnchorsLock->setText(tr("Unlock Force Anchors"));
            if (ui->btnAnchorsWrite) ui->btnAnchorsWrite->setEnabled(false);
            QMessageBox box(window());  // Use main window as parent, not widget
            box.setWindowTitle(tr("Force Anchors"));
            box.setText(tr("Anchors locked."));
            box.setIcon(QMessageBox::NoIcon);
            box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
            box.setStandardButtons(QMessageBox::Ok);
            box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                             "QMessageBox QLabel { background-color: transparent; }");
            box.exec();
            return;
        } else {
            QMessageBox box(window());  // Use main window as parent
            box.setWindowTitle(tr("Force Anchors"));
            box.setText(tr("Operation acknowledged but sealed flag did not change."));
            box.setIcon(QMessageBox::NoIcon);
            box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
            box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                             "QMessageBox QLabel { background-color: transparent; }");
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();
            return;
        }
    }

    QMessageBox box(window());  // Use main window as parent
    box.setWindowTitle(tr("Force Anchors"));
    box.setText(tr("Operation acknowledged but unable to read/parse anchors."));
    box.setIcon(QMessageBox::NoIcon);
    box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
    box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                     "QMessageBox QLabel { background-color: transparent; }");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}


void Developer::bindSetButtons()
{
    const auto buttons = findChildren<QPushButton*>(QRegularExpression("^btnSet_.*$"));

    // Sort buttons in the correct order:
    // Roll Left (100, 75, 50), Roll Right (100, 75, 50),
    // Pitch Down (100, 75, 50), Pitch Up Digital (100, 75, 50), Pitch Up Analog (100, 75, 50)
    auto getSortKey = [](const QString& name) -> int {
        // Define order: axis type * 1000 + percentage priority
        int axisOrder = 0;
        if (name.contains("RollLeft")) axisOrder = 0;
        else if (name.contains("RollRight")) axisOrder = 1;
        else if (name.contains("_PD")) axisOrder = 2;  // Pitch Down
        else if (name.contains("_DPU")) axisOrder = 3; // Digital Pitch Up
        else if (name.contains("_APU")) axisOrder = 4; // Analog Pitch Up

        int percentOrder = 0;
        if (name.contains("100")) percentOrder = 0;
        else if (name.contains("75")) percentOrder = 1;
        else if (name.contains("50")) percentOrder = 2;

        return axisOrder * 1000 + percentOrder;
    };

    QList<QPushButton*> sortedButtons = buttons;
    std::sort(sortedButtons.begin(), sortedButtons.end(),
              [&](QPushButton* a, QPushButton* b) {
                  return getSortKey(a->objectName()) < getSortKey(b->objectName());
              });

    for (auto* b : sortedButtons) {
        connect(b, &QPushButton::clicked, this, &Developer::onAnySetClicked);
        m_setButtons.append(b);

        // Install event filter to intercept key presses
        b->installEventFilter(this);

        // Add stylesheet for Invictus green focus indicator
        QString style = b->styleSheet();
        if (!style.isEmpty()) style += "\n";
        style += "QPushButton:focus { background-color: #00ff00; }";
        b->setStyleSheet(style);
    }
}

bool Developer::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // Check if the object is one of our Set buttons
        QPushButton* btn = qobject_cast<QPushButton*>(obj);
        if (btn && m_setButtons.contains(btn)) {

            // Enter key: trigger the focused Set button
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                btn->click();
                return true; // Event handled
            }

            // Period key: move focus to next Set button
            if (keyEvent->key() == Qt::Key_Period) {
                if (m_setButtons.isEmpty()) {
                    return false;
                }

                int currentIndex = m_setButtons.indexOf(btn);
                // Move to next button (wrap around to first if at end)
                int nextIndex = (currentIndex + 1) % m_setButtons.size();
                m_setButtons[nextIndex]->setFocus();
                return true; // Event handled
            }
        }
    }

    // Pass event to base class
    return QWidget::eventFilter(obj, event);
}

void Developer::setLiveRaw(int rawX, int rawY)
{
    if (rawX == m_rawX && rawY == m_rawY) return; // avoid needless repaints
    m_rawX = rawX;
    m_rawY = rawY;
    updateRawReadouts();
}

void Developer::updateRawReadouts()
{
    if (ui->lineEdit_RawX) ui->lineEdit_RawX->setText(QString::number(m_rawX));
    if (ui->lineEdit_RawY) ui->lineEdit_RawY->setText(QString::number(m_rawY));
}

void Developer::onAnySetClicked()
{
    auto* b = qobject_cast<QPushButton*>(sender());
    if (!b) return;

    const QString name = b->objectName();            // e.g. "btnSet_RollLeft100"
    const int value = isRollButtonName(name) ? m_rawX : m_rawY;

    // Build the target spinbox objectName:
    //  RollLeftNN -> spRL_NN
    //  RollRightNN -> spRR_NN
    //  PDNN -> spPD_NN
    //  DPUNN -> spPUD_NN
    //  APUNN -> spPUA_NN
    QString tail = name.mid(QStringLiteral("btnSet_").size()); // e.g. "RollLeft100"
    QString spinName;

    // percentage suffix = last 3 (100/075/050) or last 2 (75/50)
    // we can just capture ending digits:
    QRegularExpression reSuffix("(\\d+)$");
    QRegularExpressionMatch m = reSuffix.match(tail);
    const QString pct = m.hasMatch() ? m.captured(1) : QString();

    if (tail.startsWith("RollLeft", Qt::CaseInsensitive)) {
        spinName = "spRL_" + pct;
    } else if (tail.startsWith("RollRight", Qt::CaseInsensitive)) {
        spinName = "spRR_" + pct;
    } else if (tail.startsWith("PD", Qt::CaseInsensitive)) {
        spinName = "spPD_" + pct;
    } else if (tail.startsWith("DPU", Qt::CaseInsensitive)) {
        spinName = "spPUD_" + pct;
    } else if (tail.startsWith("APU", Qt::CaseInsensitive)) {
        spinName = "spPUA_" + pct;
    } else {
        // Unknown pattern; nothing to set.
        return;
    }

    if (auto* sb = findChild<QSpinBox*>(spinName)) {
        sb->setValue(value);
    } else if (auto* le = findChild<QLineEdit*>(spinName)) {
        le->setText(QString::number(value));
    }
}
void Developer::onWriteDeviceInfo()
{


    if (!m_send || !m_recv) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle(tr("Device Info"));
        box.setText(tr("Transport not set"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Get values from UI
    QString serial = ui->lineEdit_SerialNumber->text();
    QString model = ui->lineEdit_Model->text();
    QString date = ui->lineEdit_ManufactureDate->text();
    QString deviceName = ui->comboBox_DeviceName->currentText();

    // Validate date format if provided
    if (!date.isEmpty() && !QRegExp("\\d{4}-\\d{2}-\\d{2}").exactMatch(date)) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle(tr("Device Info"));
        box.setText(tr("Date must be in YYYY-MM-DD format"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Prepare device_info_t structure
    // Prepare device_info_t structure
    device_info_t info;
    memset(&info, 0, sizeof(info));

    // Set the required header fields - ADD THESE LINES
    info.magic = 0xDEF0;  // DEVICE_INFO_MAGIC
    info.version = 1;
    info.locked = 0;
    info.crc32 = 0;  // Will be calculated by firmware


    // Use safer string copying
    QByteArray modelBytes = model.toLatin1();
    QByteArray serialBytes = serial.toLatin1();
    QByteArray dateBytes = date.toLatin1();
    QByteArray deviceNameBytes = deviceName.toLatin1();

    memcpy(info.model_number, modelBytes.constData(),
           qMin(modelBytes.size(), (int)(INV_MODEL_MAX_LEN - 1)));
    memcpy(info.serial_number, serialBytes.constData(),
           qMin(serialBytes.size(), (int)(INV_SERIAL_MAX_LEN - 1)));
    memcpy(info.manufacture_date, dateBytes.constData(),
           qMin(dateBytes.size(), (int)DOM_ASCII_LEN));
    memcpy(info.device_name, deviceNameBytes.constData(),
           qMin(deviceNameBytes.size(), (int)(sizeof(info.device_name) - 1)));

    // Get PGA values from combo boxes and convert to numeric gain values
    // ComboBox indices: 0=PGA1, 1=PGA2, 2=PGA4, 3=PGA8, 4=PGA16
    const uint8_t pgaValues[] = {1, 2, 4, 8, 16};
    info.adc_pga[0] = pgaValues[ui->comboBox_PGA_Ch0->currentIndex()];
    info.adc_pga[1] = pgaValues[ui->comboBox_PGA_Ch1->currentIndex()];
    info.adc_pga[2] = pgaValues[ui->comboBox_PGA_Ch2->currentIndex()];
    info.adc_pga[3] = pgaValues[ui->comboBox_PGA_Ch3->currentIndex()];

    // Send to device using 2-packet protocol (device_info_t is 85 bytes, USB HID max payload is 62 bytes)
    // Part 1: First 62 bytes
    QByteArray part1((const char*)&info, 62);
    QByteArray ack1;

    if (!m_send(OP_SET_DEVICE_INFO, part1) || !m_recv(OP_SET_DEVICE_INFO, &ack1)) {
        QMessageBox box(window());
        box.setWindowTitle(tr("Device Info"));
        box.setText(tr("Write failed (Part 1)"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    if (ack1.isEmpty() || ack1.at(0) != 1) {
        QMessageBox box(window());
        box.setWindowTitle(tr("Device Info"));
        box.setText(tr("Device refused write Part 1"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Part 2: Remaining 23 bytes (bytes 62-84)
    QByteArray part2((const char*)&info + 62, 23);
    QByteArray ack2;

    if (!m_send(OP_SET_DEVICE_INFO_PART2, part2) || !m_recv(OP_SET_DEVICE_INFO_PART2, &ack2)) {
        QMessageBox box(window());
        box.setWindowTitle(tr("Device Info"));
        box.setText(tr("Write failed (Part 2)"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Check Part 2 response (this is where flash write happens)
    if (ack2.isEmpty() || ack2.at(0) == 0) {
        QMessageBox box(window());
        box.setWindowTitle(tr("Device Info"));
        box.setText(tr("Flash write failed"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    } else if (ack2.at(0) == 1) {
        // Emit signal to trigger UI refresh in MainWindow AFTER flash write completes
        emit deviceInfoWritten();

        QMessageBox box(window());
        box.setWindowTitle(tr("Device Info"));
        box.setText(tr("Device info written successfully"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    } else {
        QMessageBox box(window());
        box.setWindowTitle(tr("Device Info"));
        box.setText(tr("Unknown error writing device info (code: %1)").arg((int)ack2.at(0)));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }
}

void Developer::onReadPGA()
{
    if (!m_send || !m_recv) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Transport not set"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Read device info using 2-packet protocol
    // Part 1: Read first 62 bytes
    QByteArray readAck1;
    if (!m_send(OP_GET_DEVICE_INFO, QByteArray()) || !m_recv(OP_GET_DEVICE_INFO, &readAck1)) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Failed to read device info"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Part 2: Read remaining 23 bytes
    QByteArray readAck2;
    if (!m_send(OP_GET_DEVICE_INFO_PART2, QByteArray()) || !m_recv(OP_GET_DEVICE_INFO_PART2, &readAck2)) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Failed to read device info part 2"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    if (readAck2.size() < 23) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Invalid device info part 2\nReceived: %1 bytes\nExpected: 23 bytes").arg(readAck2.size()));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Combine both parts into complete device_info_t (85 bytes total)
    device_info_t info;
    memset(&info, 0, sizeof(info));
    memcpy((uint8_t*)&info, readAck1.constData(), 62);
    memcpy((uint8_t*)&info + 62, readAck2.constData(), 23);

    // Update UI with PGA values from device
    // Map PGA values to combo box indices: 0=PGA1, 1=PGA2, 2=PGA4, 3=PGA8, 4=PGA16
    auto pgaToIndex = [](uint8_t pga) -> int {
        switch(pga) {
            case 1: return 0;
            case 2: return 1;
            case 4: return 2;
            case 8: return 3;
            case 16: return 4;
            default: return 2; // Default to PGA 4
        }
    };

    ui->comboBox_PGA_Ch0->setCurrentIndex(pgaToIndex(info.adc_pga[0]));
    ui->comboBox_PGA_Ch1->setCurrentIndex(pgaToIndex(info.adc_pga[1]));
    ui->comboBox_PGA_Ch2->setCurrentIndex(pgaToIndex(info.adc_pga[2]));
    ui->comboBox_PGA_Ch3->setCurrentIndex(pgaToIndex(info.adc_pga[3]));

    // Update mode - assume all channels have same mode (we only check channel 0)
    ui->comboBox_ADCMode->setCurrentIndex(info.adc_mode[0]);

    QMessageBox box(window());
    box.setWindowTitle(tr("PGA Settings"));
    box.setText(tr("PGA settings read successfully"));
    box.setIcon(QMessageBox::NoIcon);
    box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
    box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                     "QMessageBox QLabel { background-color: transparent; }");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void Developer::onWritePGA()
{
    if (!m_send || !m_recv) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Transport not set"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Read current device info from device using 2-packet protocol
    // Part 1: Get first 62 bytes
    QByteArray readAck1;
    if (!m_send(OP_GET_DEVICE_INFO, QByteArray()) || !m_recv(OP_GET_DEVICE_INFO, &readAck1)) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Failed to read device info part 1"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    if (readAck1.size() < 62) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Invalid device info part 1\nReceived: %1 bytes\nExpected: 62 bytes").arg(readAck1.size()));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Part 2: Get remaining 23 bytes
    QByteArray readAck2;
    if (!m_send(OP_GET_DEVICE_INFO_PART2, QByteArray()) || !m_recv(OP_GET_DEVICE_INFO_PART2, &readAck2)) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Failed to read device info part 2"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    if (readAck2.size() < 23) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Invalid device info part 2\nReceived: %1 bytes\nExpected: 23 bytes").arg(readAck2.size()));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Combine both parts into complete device_info_t (85 bytes total)
    device_info_t info;
    memset(&info, 0, sizeof(info));
    memcpy((uint8_t*)&info, readAck1.constData(), 62);
    memcpy((uint8_t*)&info + 62, readAck2.constData(), 23);

    // Update only PGA and Mode values from combo boxes
    const uint8_t pgaValues[] = {1, 2, 4, 8, 16};
    info.adc_pga[0] = pgaValues[ui->comboBox_PGA_Ch0->currentIndex()];
    info.adc_pga[1] = pgaValues[ui->comboBox_PGA_Ch1->currentIndex()];
    info.adc_pga[2] = pgaValues[ui->comboBox_PGA_Ch2->currentIndex()];
    info.adc_pga[3] = pgaValues[ui->comboBox_PGA_Ch3->currentIndex()];

    // Set all channels to same mode: 0=Single-Ended, 1=Differential
    uint8_t mode = ui->comboBox_ADCMode->currentIndex();
    info.adc_mode[0] = mode;
    info.adc_mode[1] = mode;
    info.adc_mode[2] = mode;
    info.adc_mode[3] = mode;

    // Send to device using 2-packet protocol
    // Part 1: Send first 62 bytes
    QByteArray payload1((const char*)&info, 62);
    QByteArray ack1;

    if (!m_send(OP_SET_DEVICE_INFO, payload1) || !m_recv(OP_SET_DEVICE_INFO, &ack1)) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Write part 1 failed"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    if (ack1.isEmpty() || ack1.at(0) != 1) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Device refused write part 1"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Part 2: Send remaining 23 bytes
    QByteArray payload2((const char*)&info + 62, 23);
    QByteArray ack2;

    if (!m_send(OP_SET_DEVICE_INFO_PART2, payload2) || !m_recv(OP_SET_DEVICE_INFO_PART2, &ack2)) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Write part 2 failed"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // Check final response
    if (ack2.isEmpty() || ack2.at(0) == 0) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Device refused write (flash write failed or locked)"));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    } else if (ack2.at(0) == 1) {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("PGA settings written successfully.\nPower cycle device to apply changes."));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    } else {
        QMessageBox box(window());
        box.setWindowTitle(tr("PGA Settings"));
        box.setText(tr("Error code: %1").arg((int)(uint8_t)ack2.at(0)));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }
}

void Developer::onExport()
{
    // Get current anchors from UI
    Anchors a = uiGet();

    // Open file dialog
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Force Anchors"),
        QString(),
        tr("JSON Files (*.json);;All Files (*)")
    );

    if (fileName.isEmpty()) {
        return;  // User cancelled
    }

    // Create JSON object
    QJsonObject json;
    json["version"] = 1;
    json["magic"] = QString::number(a.magic, 16);
    json["sealed"] = a.sealed;

    // Helper to create triplet object
    auto tripletToJson = [](const Triplet& t) -> QJsonObject {
        QJsonObject obj;
        obj["adc100"] = t.adc100;
        obj["adc75"] = t.adc75;
        obj["adc50"] = t.adc50;
        return obj;
    };

    // Add all anchors
    json["roll_left_17lbf"] = tripletToJson(a.rl_17);
    json["roll_right_17lbf"] = tripletToJson(a.rr_17);
    json["pitch_down_17lbf"] = tripletToJson(a.pd_17);
    json["pitch_up_digital_25lbf"] = tripletToJson(a.pu25);
    json["pitch_up_analog_40lbf"] = tripletToJson(a.pu40);

    // Add device identification if present
    if (!a.serialNumber.isEmpty()) json["serial_number"] = a.serialNumber;
    if (!a.modelNumber.isEmpty()) json["model_number"] = a.modelNumber;
    if (!a.manufactureDate.isEmpty()) json["manufacture_date"] = a.manufactureDate;

    // Add PGA settings (factory calibration data)
    const uint8_t pgaValues[] = {1, 2, 4, 8, 16};
    QJsonObject pgaSettings;
    pgaSettings["ch0"] = (int)pgaValues[ui->comboBox_PGA_Ch0->currentIndex()];
    pgaSettings["ch1"] = (int)pgaValues[ui->comboBox_PGA_Ch1->currentIndex()];
    pgaSettings["ch2"] = (int)pgaValues[ui->comboBox_PGA_Ch2->currentIndex()];
    pgaSettings["ch3"] = (int)pgaValues[ui->comboBox_PGA_Ch3->currentIndex()];
    json["pga_settings"] = pgaSettings;

    // Add ADC mode (0=Single-Ended, 1=Differential)
    json["adc_mode"] = ui->comboBox_ADCMode->currentIndex();

    // Write to file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle(tr("Export Failed"));
        box.setText(tr("Could not open file for writing: %1").arg(file.errorString()));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    QJsonDocument doc(json);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox box(window());  // Use main window as parent
    box.setWindowTitle(tr("Export Successful"));
    box.setText(tr("Force anchors and PGA settings exported successfully."));
    box.setIcon(QMessageBox::NoIcon);
    box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
    box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                     "QMessageBox QLabel { background-color: transparent; }");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void Developer::onImport()
{
    // Open file dialog
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Import Force Anchors"),
        QString(),
        tr("JSON Files (*.json);;All Files (*)")
    );

    if (fileName.isEmpty()) {
        return;  // User cancelled
    }

    // Read file
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle(tr("Import Failed"));
        box.setText(tr("Could not open file for reading: %1").arg(file.errorString()));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    // Parse JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (doc.isNull()) {
        QMessageBox box(window());  // Use main window as parent
        box.setWindowTitle(tr("Import Failed"));
        box.setText(tr("Failed to parse JSON: %1").arg(parseError.errorString()));
        box.setIcon(QMessageBox::NoIcon);
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48));
        box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                         "QMessageBox QLabel { background-color: transparent; }");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    QJsonObject json = doc.object();

    // Helper to extract triplet from JSON
    auto jsonToTriplet = [](const QJsonObject& obj) -> Triplet {
        Triplet t;
        t.adc100 = (qint16)obj["adc100"].toInt();
        t.adc75 = (qint16)obj["adc75"].toInt();
        t.adc50 = (qint16)obj["adc50"].toInt();
        return t;
    };

    // Create anchors structure from JSON
    Anchors a;
    a.magic = 0xF00C;  // Use correct magic regardless of file
    a.version = 0x02;  // Use current version
    a.sealed = json["sealed"].toInt();

    // Load all triplets
    if (json.contains("roll_left_17lbf"))
        a.rl_17 = jsonToTriplet(json["roll_left_17lbf"].toObject());
    if (json.contains("roll_right_17lbf"))
        a.rr_17 = jsonToTriplet(json["roll_right_17lbf"].toObject());
    if (json.contains("pitch_down_17lbf"))
        a.pd_17 = jsonToTriplet(json["pitch_down_17lbf"].toObject());
    if (json.contains("pitch_up_digital_25lbf"))
        a.pu25 = jsonToTriplet(json["pitch_up_digital_25lbf"].toObject());
    if (json.contains("pitch_up_analog_40lbf"))
        a.pu40 = jsonToTriplet(json["pitch_up_analog_40lbf"].toObject());

    // Load device identification
    if (json.contains("serial_number"))
        a.serialNumber = json["serial_number"].toString();
    if (json.contains("model_number"))
        a.modelNumber = json["model_number"].toString();
    if (json.contains("manufacture_date"))
        a.manufactureDate = json["manufacture_date"].toString();

    // Set UI from imported data
    uiSet(a);

    // Load and set PGA settings if present
    if (json.contains("pga_settings")) {
        QJsonObject pgaSettings = json["pga_settings"].toObject();

        // Map PGA values to combo box indices: 0=PGA1, 1=PGA2, 2=PGA4, 3=PGA8, 4=PGA16
        auto pgaToIndex = [](int pga) -> int {
            switch(pga) {
                case 1: return 0;
                case 2: return 1;
                case 4: return 2;
                case 8: return 3;
                case 16: return 4;
                default: return 2; // Default to PGA 4
            }
        };

        if (pgaSettings.contains("ch0"))
            ui->comboBox_PGA_Ch0->setCurrentIndex(pgaToIndex(pgaSettings["ch0"].toInt()));
        if (pgaSettings.contains("ch1"))
            ui->comboBox_PGA_Ch1->setCurrentIndex(pgaToIndex(pgaSettings["ch1"].toInt()));
        if (pgaSettings.contains("ch2"))
            ui->comboBox_PGA_Ch2->setCurrentIndex(pgaToIndex(pgaSettings["ch2"].toInt()));
        if (pgaSettings.contains("ch3"))
            ui->comboBox_PGA_Ch3->setCurrentIndex(pgaToIndex(pgaSettings["ch3"].toInt()));
    }

    // Load and set ADC mode if present
    if (json.contains("adc_mode")) {
        ui->comboBox_ADCMode->setCurrentIndex(json["adc_mode"].toInt());
    }

    QMessageBox box(window());  // Use main window as parent
    box.setWindowTitle(tr("Import Successful"));
    box.setText(tr("Force anchors and PGA settings imported successfully.\nRemember to write to device if you want to save these values."));
    box.setIcon(QMessageBox::NoIcon);
    box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48));
    box.setStyleSheet("QMessageBox { background-color: rgb(36, 39, 49); }"
                     "QMessageBox QLabel { background-color: transparent; }");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void Developer::retranslateUi()
{
    if (ui) {
        ui->retranslateUi(this);
        autoAdjustAllWidgetsForTranslation(this);
    }
}
