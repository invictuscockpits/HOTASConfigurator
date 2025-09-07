#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QObject>
#include <QString>
#include "hidapi.h"
#include "common_types.h"

class DeviceInfo : public QObject
{
    Q_OBJECT

public:
    explicit DeviceInfo(QObject *parent = nullptr);
    ~DeviceInfo();

    // Read device info from device
    bool readFromDevice(hid_device *dev);

    // Write device info to device
    bool writeToDevice(hid_device *dev, const QString &model, const QString &serial, const QString &dom);

    // Get current values
    QString getModel() const { return m_model; }
    QString getSerial() const { return m_serial; }
    QString getDateOfManufacture() const { return m_dateOfManufacture; }
    bool isLocked() const { return m_locked; }

signals:
    void infoUpdated();
    void errorOccurred(const QString &error);

private:
    QString m_model;
    QString m_serial;
    QString m_dateOfManufacture;
    bool m_locked;

    bool sendDeviceInfoCommand(hid_device *dev, uint8_t op, const uint8_t *data, int len, uint8_t *response, int &response_len);
};

#endif // DEVICEINFO_H
