#include "device_info.h"
#include "common_defines.h"
#include <cstring>
#include <QDebug>

DeviceInfo::DeviceInfo(QObject *parent) :
    QObject(parent),
    m_locked(false)
{
}

DeviceInfo::~DeviceInfo()
{
}

bool DeviceInfo::sendDeviceInfoCommand(hid_device *dev, uint8_t op, const uint8_t *data, int len, uint8_t *response, int &response_len)
{
    if (!dev) return false;

    uint8_t buffer[65] = {0};
    buffer[0] = 0;  // Report ID 0 for Windows
    buffer[1] = REPORT_ID_DEV;
    buffer[2] = op;

    if (data && len > 0) {
        memcpy(&buffer[3], data, len);
    }

    // Send command
    if (hid_write(dev, buffer, 65) < 0) {
        emit errorOccurred("Failed to write device info command");
        return false;
    }

    // Read response
    int res = hid_read_timeout(dev, buffer, 65, 1000);
    if (res < 0) {
        emit errorOccurred("Failed to read device info response");
        return false;
    }

    // Verify response
    if (buffer[0] != REPORT_ID_DEV || buffer[1] != op) {
        emit errorOccurred("Invalid device info response");
        return false;
    }

    // Copy response data
    response_len = res - 2;  // Subtract report ID and op code
    if (response_len > 0 && response) {
        memcpy(response, &buffer[2], response_len);
    }

    return true;
}

bool DeviceInfo::readFromDevice(hid_device *dev)
{
    if (!dev) return false;

    device_info_t info;
    int response_len;

    if (!sendDeviceInfoCommand(dev, OP_GET_DEVICE_INFO, nullptr, 0,
                               (uint8_t*)&info, response_len)) {
        return false;
    }

    if (response_len != sizeof(device_info_t)) {
        emit errorOccurred("Invalid device info size");
        return false;
    }

    // Extract data from structure
    m_model = QString::fromLatin1(info.model_number, strnlen(info.model_number, INV_MODEL_MAX_LEN));
    m_serial = QString::fromLatin1(info.serial_number, strnlen(info.serial_number, INV_SERIAL_MAX_LEN));
    m_dateOfManufacture = QString::fromLatin1(info.manufacture_date, strnlen(info.manufacture_date, DOM_ASCII_LEN));
    m_locked = info.locked;

    emit infoUpdated();
    return true;
}

bool DeviceInfo::writeToDevice(hid_device *dev, const QString &model, const QString &serial, const QString &dom)
{
    if (!dev) return false;

    if (m_locked) {
        emit errorOccurred("Device info is locked and cannot be modified");
        return false;
    }

    // Prepare device_info_t structure
    device_info_t info;
    memset(&info, 0, sizeof(info));

    // Copy strings (ensure null termination)
    strncpy(info.model_number, model.toLatin1().data(), INV_MODEL_MAX_LEN - 1);
    strncpy(info.serial_number, serial.toLatin1().data(), INV_SERIAL_MAX_LEN - 1);
    strncpy(info.manufacture_date, dom.toLatin1().data(), DOM_ASCII_LEN - 1);

    // Send to device
    uint8_t response;
    int response_len;

    if (!sendDeviceInfoCommand(dev, OP_SET_DEVICE_INFO, (uint8_t*)&info, sizeof(info),
                               &response, response_len)) {
        return false;
    }

    if (response_len != 1 || response != 1) {
        emit errorOccurred("Failed to write device info");
        return false;
    }

    // Update local copy
    m_model = model;
    m_serial = serial;
    m_dateOfManufacture = dom;

    emit infoUpdated();
    return true;
}
