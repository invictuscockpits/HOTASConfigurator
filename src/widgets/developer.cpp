#include "developer.h"
#include "ui_developer.h"
#include "common_defines.h"
#include "common_types.h"

#include <QSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <Qdebug>
#include <QLineEdit>

Developer::Developer(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::Developer)
{
    ui->setupUi(this);
    setDefaultSpinRanges();
    initConnections();
    // Bind every Set button named "btnSet_*"
    bindSetButtons();


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

    connect(ui->btnAnchorsRead,  &QPushButton::clicked, this, &Developer::onRead);
    connect(ui->btnAnchorsWrite, &QPushButton::clicked, this, &Developer::onWrite);
    connect(ui->btnAnchorsLock,  &QPushButton::clicked, this, &Developer::onLock);
    connect(ui->btnDeviceInfoWrite, &QPushButton::clicked, this, &Developer::onWriteDeviceInfo);

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


    return b;

    for (int i=0;i<8;i++) b.append(char(0));  // reserved bytes
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
            qDebug() << "  - Response size:" << resp.size();
            qDebug() << "  - First 16 bytes:" << resp.left(16).toHex(' ');

            haveAnchors = unpack(resp, &a);
            // consider anchors “missing” only if unpack failed OR all trips are zero
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

            QMessageBox box(this);
            box.setWindowTitle(tr("Force Anchors"));
            box.setText(tr("Factory anchors missing; using calibration defaults."));

            box.setIcon(QMessageBox::NoIcon); // prevent style’s default PNG
            box.setIconPixmap(QIcon(":/Images/Info_icon.svg").pixmap(48,48)); // crisp at any DPI
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();
        }
    }

    if (!haveAnchors) {
        QMessageBox box(this);
        box.setWindowTitle(tr("Force Anchors"));
        box.setText(tr("Unable to read anchors and no fallback available."));

        box.setIcon(QMessageBox::NoIcon); // prevent style’s default PNG
        box.setIconPixmap(QIcon(":/Images/warning_icon.svg").pixmap(48,48)); // crisp at any DPI
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();

    }



    uiSet(a);
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
    if (!m_send || !m_recv) { QMessageBox::warning(this, "Anchors", "Transport not set"); return; }

    // Pre-read: block if sealed
    QByteArray resp;
    Anchors current{};
    if (m_send(OP_GET_FACTORY_ANCHORS, QByteArray()) &&
        m_recv(OP_GET_FACTORY_ANCHORS, &resp) &&
        unpack(resp, &current) && current.sealed == 1)
    {
        QMessageBox::warning(this, "Anchors", "Anchors are locked; writing is not allowed.");
        return;
    }

    Anchors a = uiGet();
    QByteArray blob = pack(a), ack;

    // Debug output
    qDebug() << "[Developer] Sending anchors:";
    qDebug() << "  - Blob size:" << blob.size();
    qDebug() << "  - First 16 bytes:" << blob.left(16).toHex(' ');

    if (!m_send(OP_SET_FACTORY_ANCHORS, blob) || !m_recv(OP_SET_FACTORY_ANCHORS, &ack)) {
        QMessageBox::warning(this, "Anchors", "Write failed"); return;
    }
    QMessageBox::information(this, "Anchors", "Anchors written.");
}
void Developer::onLock()
{
    if (!m_send || !m_recv) {
        QMessageBox::warning(this, tr("Force Anchors"), tr("Transport not set."));
        return;
    }
    QByteArray ack;
    const bool sent = m_send(OP_LOCK_FACTORY_ANCHORS, QByteArray());
    qDebug() << "[Anchors] m_send(LOCK) returned" << sent;

    const bool got = m_recv(OP_LOCK_FACTORY_ANCHORS, &ack);
    qDebug().noquote() << "[Anchors] m_recv(LOCK) returned" << got
                       << " ackLen=" << ack.size()
                       << " ackHex=" << hexDump(ack, 8);

    if (!sent || !got) {
        QMessageBox::warning(this, tr("Force Anchors"), tr("Locking anchors failed (no reply)."));
        return;
    }

    const quint8 status = ack.isEmpty() ? 1 : quint8(ack.at(0)); // 1=OK, 0=fail
    if (status != 1) {
        QMessageBox::warning(this, tr("Force Anchors"), tr("Device refused lock."));
        return;
    }

    // Read back and verify the sealed bit
    QByteArray resp;
    Anchors a{};
    if (m_send(OP_GET_FACTORY_ANCHORS, QByteArray()) && m_recv(OP_GET_FACTORY_ANCHORS, &resp) && unpack(resp, &a)) {
        qDebug().noquote() << "[Anchors] GET after LOCK: len=" << resp.size()
        << " head=" << hexDump(resp, 12);
        uiSet(a);
        if (a.sealed == 1) {
            // Optional: disable editing after lock
            if (ui->btnAnchorsWrite) ui->btnAnchorsWrite->setEnabled(false);
            if (ui->btnAnchorsLock)  ui->btnAnchorsLock->setEnabled(false);
            QMessageBox::information(this, tr("Force Anchors"), tr("Anchors locked."));
            return;
        } else {
            QMessageBox::warning(this, tr("Force Anchors"), tr("Lock ACK OK but sealed flag is still 0."));
            return;
        }
    }

    QMessageBox::warning(this, tr("Force Anchors"), tr("Lock ACK OK but unable to read/parse anchors."));
}


void Developer::bindSetButtons()
{
    const auto buttons = findChildren<QPushButton*>(QRegularExpression("^btnSet_.*$"));
    for (auto* b : buttons)
        connect(b, &QPushButton::clicked, this, &Developer::onAnySetClicked);
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
        QMessageBox::warning(this, tr("Device Info"), tr("Transport not set"));
        return;
    }

    // Get values from UI
    QString serial = ui->lineEdit_SerialNumber->text();
    QString model = ui->lineEdit_Model->text();
    QString date = ui->lineEdit_ManufactureDate->text();

    // Debug output
    qDebug() << "[Developer] Writing device info:";
    qDebug() << "  - Serial:" << serial;
    qDebug() << "  - Model:" << model;
    qDebug() << "  - Date:" << date;
    // Debug: show structure offsets
    qDebug() << "  - magic offset:" << offsetof(device_info_t, magic);
    qDebug() << "  - version offset:" << offsetof(device_info_t, version);
    qDebug() << "  - locked offset:" << offsetof(device_info_t, locked);
    qDebug() << "  - crc32 offset:" << offsetof(device_info_t, crc32);
    qDebug() << "  - model offset:" << offsetof(device_info_t, model_number);
    qDebug() << "  - serial offset:" << offsetof(device_info_t, serial_number);

    // Validate date format if provided
    if (!date.isEmpty() && !QRegExp("\\d{4}-\\d{2}-\\d{2}").exactMatch(date)) {
        QMessageBox::warning(this, tr("Device Info"), tr("Date must be in YYYY-MM-DD format"));
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

    memcpy(info.model_number, modelBytes.constData(),
           qMin(modelBytes.size(), (int)(INV_MODEL_MAX_LEN - 1)));
    memcpy(info.serial_number, serialBytes.constData(),
           qMin(serialBytes.size(), (int)(INV_SERIAL_MAX_LEN - 1)));
    memcpy(info.manufacture_date, dateBytes.constData(),
           qMin(dateBytes.size(), (int)DOM_ASCII_LEN));

    // Send to device
    QByteArray payload((const char*)&info, sizeof(info));
    QByteArray ack;

    qDebug() << "  - Payload size:" << payload.size();
    qDebug() << "  - Expected size:" << sizeof(device_info_t);
    qDebug() << "  - First 16 bytes of payload:" << payload.left(16).toHex(' ');

    if (!m_send(OP_SET_DEVICE_INFO, payload) || !m_recv(OP_SET_DEVICE_INFO, &ack)) {
        QMessageBox::warning(this, tr("Device Info"), tr("Write failed"));
        return;
    }

    // Check response
    qDebug() << "  - Response size:" << ack.size();
    if (!ack.isEmpty()) {
        qDebug() << "  - Full response:" << ack.left(10).toHex(' ');
    }

    // Parse the echo response to see what firmware received
    if (ack.size() >= 9 && ack.at(0) == (char)0xAA) {
        qDebug() << "  - Echo: ReportID=" << (int)(uint8_t)ack.at(1)
        << " OpCode=" << (int)(uint8_t)ack.at(2)
        << " Payload starts:" << ack.mid(3, 4).toHex(' ');
        return;  // Don't show error for echo test
    }

    // Handle normal responses
    if (ack.isEmpty() || ack.at(0) == 0) {
        QMessageBox::warning(this, tr("Device Info"), tr("Device refused write (unknown error)"));
        return;
    } else if (ack.at(0) == 1) {
        QMessageBox::information(this, tr("Device Info"), tr("Device info written successfully"));
        return;
    } else if (ack.at(0) == 2) {
        QMessageBox::warning(this, tr("Device Info"), tr("Device info is locked"));
        return;
    } else if (ack.at(0) == 3) {
        QMessageBox::warning(this, tr("Device Info"), tr("Invalid payload size"));
        return;
    } else if (ack.at(0) == 4) {
        QMessageBox::warning(this, tr("Device Info"), tr("Flash write failed"));
        return;
    } else {
        QMessageBox::warning(this, tr("Device Info"), tr("Unknown error code: %1").arg((int)(uint8_t)ack.at(0)));
        return;
    }
}
