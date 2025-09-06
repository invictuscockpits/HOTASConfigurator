#ifndef DEVELOPER_H
#define DEVELOPER_H

#include <QWidget>
#include <QByteArray>
#include <functional>

namespace Ui { class Developer; }



class Developer : public QWidget
{
    Q_OBJECT
public:
    using Calib = struct { qint16 min, center, max; };
    using FallbackFn = std::function<bool(Calib* roll, Calib* pitch)>;
    void setFallbackProvider(FallbackFn fn) { m_fallback = std::move(fn); }
    explicit Developer(QWidget *parent = nullptr);
    ~Developer();
    // Minimal transport interface (inject from main app)
    using TxFn = std::function<bool(quint8 /*op*/, const QByteArray&)>;
    using RxFn = std::function<bool(quint8 /*op*/, QByteArray*)>;
    void setTransport(TxFn tx, RxFn rx) { m_send = std::move(tx); m_recv = std::move(rx); }


public slots:
    // Hooked to the three buttons
    void onRead();
    void onWrite();
    void onLock();
    void setLiveRaw(int rawX, int rawY);

private:

    Ui::Developer* ui{};
    TxFn m_send;
    RxFn m_recv;

    // ---- Local mirror of the protected anchors block (must match device layout) ----
    struct Triplet { qint16 adc100{0}, adc75{0}, adc50{0}; };
    struct Anchors {
        quint16 magic{0xF00C};  // FACTORY_ANCHORS_MAGIC
        quint8  version{0x01};  // FACTORY_ANCHORS_VER
        quint8  sealed{0};      // 1 when locked
        quint32 crc32{0};       // CRC of entire struct (including header)

        Triplet rl_17;  // roll-left  (17 lbf full)
        Triplet rr_17;  // roll-right (17 lbf full)
        Triplet pd_17;  // pitch-down (17 lbf full)
        Triplet pu25;   // pitch-up digital (25 lbf full)
        Triplet pu40;   // pitch-up analog  (40 lbf full)

        quint8  reserved[8]{};
    };
    FallbackFn m_fallback;
    static bool isEmptyTrip(const Triplet& t) {
        return t.adc100 == 0 && t.adc75 == 0 && t.adc50 == 0;
    }

    // UI <-> struct
    void uiSet(const Anchors& a);
    Anchors uiGet() const;

    // Pack/unpack (little-endian) + CRC32
    QByteArray pack(const Anchors& a) const;
    bool       unpack(const QByteArray& buf, Anchors* out) const;
    quint32    crc32_le(const QByteArray& data) const;



    // Helpers
    void initConnections();
    void setDefaultSpinRanges(); // -32768..32767 and centered alignment

    // Mirrored from params_report_t::raw_axis_data[0/1]
    int m_rawX = 0;
    int m_rawY = 0;

    void updateRawReadouts();   // writes to lineEdit_RawX / lineEdit_RawY
    void bindSetButtons();      // connects ^btnSet_.*$ -> onAnySetClicked()

    // Decide X vs Y based on button name ("Roll" -> X, otherwise Y)
    static inline bool isRollButtonName(const QString& n) {
        return n.contains("Roll", Qt::CaseInsensitive);
    }

private slots:
    // Handles any Set button named "btnSet_*"
    void onAnySetClicked();


};

#endif // DEVELOPER_H
