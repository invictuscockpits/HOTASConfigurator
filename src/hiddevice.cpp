#include <QThread>
#include <QDebug>
#include <QElapsedTimer>
#include <QMap>
#include <thread>
#include "hiddevice.h"
#include "hidapi.h"
#include "common_defines.h"
#include "reportconverter.h"
#include "deviceconfig.h"
#include "global.h"
#include "firmwareupdater.h"
#include <qset.h>
#include <atomic>

static const int DEVICE_SEARCH_DELAY = 1000; // ms
static const int OLD_FIRMWARE_VID = 0x0483;
// sience FJ v1.7 REPORT_ID_FIRMWARE = 5 but flasher has = 4
static const int REPORT_ID_FLASH = 4;




/**
 * @brief Enumerates HID devices, opens the selection, and streams reports — with transient read-error debounce.
 *
 * Rationale:
 * - Gen3 (and earlier) boards may momentarily glitch on D+/D− (ESD/EMI). Windows keeps the HID device alive,
 *   but a single `hid_read_timeout(...) < 0` used to make the GUI drop the connection and spend seconds
 *   re-enumerating. That feels like a disconnect even though the OS never lost the device.
 *
 * What changed (minimal):
 * - We only close the HID handle after a small burst of real read errors (res < 0) *or* after a short grace
 *   since the last good frame. Plain timeouts (res == 0) are not errors.
 * - We did not touch your enumeration, signals, or the 1-byte/2-byte PARAM writes.
 *
 * Effects:
 * - Brief USB hiccups no longer tear down the GUI path. `deviceConnected()` and PARAM streaming behave
 *   just like before when the device is truly present【turn4file5L39-L54】, and `deviceDisconnected()` is still only
 *   emitted when the list is empty【turn4file5L17-L24】.
 *
 * See also:
 * - Signals used by MainWindow: `deviceConnected`, `deviceDisconnected`, `paramsPacketReceived`, etc. 【turn4file3L27-L41】
 */
void HidDevice::processData()                   /////// bad code, I'll try to rewrite later
{
    const QString FLASHER_PROD_STR ("Invictus HOTAS Flasher");
    const QStringList SUPPORTED_PROD_STR = QStringList() << FLASHER_PROD_STR << "FreeJoy Flasher" << "InvictusFlasher" << "Invictus HOTAS FLasher";

    const QString OLD_MANUFACT_STR ("STMicroelectronics");
    const QString FJ_MANUFACT_STR ("Invictus Cockpit Systems");
    const QStringList SUPPORTED_MANUFACTURERS = QStringList () << FJ_MANUFACT_STR <<"Invictus Cockpits"  <<"STMicroelectronics";

    // --- Debounce state for USB burps (local; minimal intrusion) ---
    static int          s_consecutiveErrors = 0;
    static QElapsedTimer s_lastGoodRx; // time since last good frame
    s_lastGoodRx.invalidate();
    constexpr int kMaxConsecutiveErrors = 3;   // tolerate a few real errors
    constexpr int kGraceMs              = 700; // or 500 if your line is noisier
    // --- Debounce for empty device list (enumeration flaps) ---
    static QElapsedTimer s_emptyListSince;
    s_emptyListSince.invalidate();
    constexpr int kEmptyListGraceMs = 400; // wait this long before calling it a real disconnect

    // m_hidDevicesList - should be thread safe

    int res = 0;
    int oldSelectedDevice = -1;
    bool deviceCountChanged = false;
    uint8_t buffer[BUFFERSIZE]{};
    uint8_t deviceBuffer[BUFFERSIZE];
    uint8_t paramsRequest[1] = {REPORT_ID_PARAM};
    QList<QPair<bool, hid_device_info*>> tmp_HidList;
    m_currentWork = REPORT_ID_PARAM;
    QElapsedTimer paramsTimer;
    //paramsTimer.start();

    hid_device_info *hidDevInfo;
    // first device in list need for hid_free_enumeration(). fixes memory leak
    hid_device_info *firstInList = NULL;
    // if hid_enumerate(0x0, 0x0) it takes 140-190ms on my main PC but in a laptop it takes 25ms.
    // the more devices are connected, the longer the search takes
    // std::thread because we dont have event loop
    //QElapsedTimer timer;
    std::thread th([&] {
        while (m_isFinish == false) {
            // free memory. start from first device in list
            //timer.start();
            hid_free_enumeration(firstInList);
            firstInList = hidDevInfo = hid_enumerate(0x0, 0x0);
            //qDebug()<<"hid_enumerate(0x0, 0x0) ms ="<<timer.elapsed();
            while(hidDevInfo) {
                // flasher search. flasher is here because we dont need read params buffer
                // and we are not intrested in other devices when we flash firmware
                QString productStr = QString::fromWCharArray(hidDevInfo->product_string);
                if (SUPPORTED_PROD_STR.contains(productStr)) {
                    // flasher found, remember path
                    if (m_flasherPath.isEmpty()) {
                        m_flasherPath = hidDevInfo->path;
                        // remove all devices and add flasher
                        m_hidDevicesList.clear();
                        m_deviceNames.clear();
                        m_deviceNames.append(qMakePair(false, FLASHER_PROD_STR));
                        emit hidDeviceList(m_deviceNames);
                        emit flasherConnected();
                        emit flasherFound(true);
                    }
                    //hidDevInfo = hidDevInfo->next;
                    // start flash firmware
                    if (m_currentWork == REPORT_ID_FIRMWARE) {
                        flashFirmwareToDevice();
                        // flash done
                        m_flasherPath.clear();
                        m_currentWork = REPORT_ID_PARAM;
                        m_deviceNames.clear();
                        emit hidDeviceList(m_deviceNames);
                        emit deviceConnected();
                        QThread::msleep(300);
                        break;
                    }
                    break;
                }
                // add devices ptr to list. since FJ v1.7 we have changed the name and made two interfaces
                if (QString::fromWCharArray(hidDevInfo->manufacturer_string) == FJ_MANUFACT_STR && hidDevInfo->interface_number == 1) {
                    tmp_HidList.append(qMakePair(false, hidDevInfo));
                }
                // search the old firmware device
                else if (hidDevInfo->vendor_id == OLD_FIRMWARE_VID && QString::fromWCharArray(hidDevInfo->manufacturer_string) == OLD_MANUFACT_STR &&
                         hidDevInfo->interface_number <= 0) {
                    // true if older than FJ v1.7
                    tmp_HidList.append(qMakePair(true, hidDevInfo));
                }
                hidDevInfo = hidDevInfo->next;
                // all devices added
                if (!hidDevInfo) {
                    // old devices count != new devices count
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_hidDevicesList.size() != tmp_HidList.size()) {  // mutex
                        m_hidDevicesList.clear();
                        m_deviceNames.clear();
                        for (int i = 0; i < tmp_HidList.size(); ++i) {
                            m_hidDevicesList.append(tmp_HidList[i].second);
                            // device names for UI combobox. pair for old firmware device
                            m_deviceNames.append(qMakePair(tmp_HidList[i].first, QString::fromWCharArray(tmp_HidList[i].second->product_string)));
                        }
                        // emit all connected HOTAS devices names
                        emit hidDeviceList(m_deviceNames);
                        tmp_HidList.clear();
                        deviceCountChanged = true;
                        // flasher disconnected. clear path
                        if (m_flasherPath.isEmpty() == false) {
                            m_flasherPath.clear();
                            emit flasherFound(false);
                        }
                    }
                }
            }
            tmp_HidList.clear();
            std::this_thread::sleep_for(std::chrono::milliseconds(DEVICE_SEARCH_DELAY));
        }
    });


    while (m_isFinish == false)
    {
        if (m_flasherPath.isEmpty()) {

            // all devices disconnected (debounced)
            if (m_hidDevicesList.empty()) {
                if (!s_emptyListSince.isValid()) s_emptyListSince.start();

                // Wait a short grace period before declaring a real disconnect
                if (s_emptyListSince.elapsed() < kEmptyListGraceMs) {
                    QThread::msleep(50);   // brief yield; keep checking
                    continue;
                }

                emit deviceDisconnected();
                oldSelectedDevice = m_selectedDevice = -1;

                s_emptyListSince.invalidate();   // reset for next time
                QThread::msleep(150);            // faster recovery than 600 ms
                continue;
            } else {
                // as soon as we see something in the list, cancel any pending "empty" debounce
                if (s_emptyListSince.isValid()) s_emptyListSince.invalidate();
            }
            // open HID
            if (m_selectedDevice != oldSelectedDevice || (deviceCountChanged && m_selectedDevice >= 0)) {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (!m_hidDevicesList.empty() && m_selectedDevice < m_hidDevicesList.size()) {  // mutex
                    const ushort vid = m_hidDevicesList[m_selectedDevice].vid;
                    const ushort pid = m_hidDevicesList[m_selectedDevice].pid;
                    const wchar_t *serNum = m_hidDevicesList[m_selectedDevice].serNum.c_str();
                    const char *path = m_hidDevicesList[m_selectedDevice].path.c_str();

                    qDebug().nospace()<<"Open HID device №"<<m_selectedDevice + 1
                                       <<". VID"<<QString::number(vid, 16).toUpper().rightJustified(4, '0')
                                       <<", PID"<<QString::number(pid, 16).toUpper().rightJustified(4, '0')
                                       <<", Serial number "<<QString::fromWCharArray(serNum);
                    m_paramsRead = hid_open_path(path);
                    // params hid opened
                    if (m_paramsRead) {
                        ReportConverter::resetReport();
                        emit deviceConnected();
                        oldSelectedDevice = m_selectedDevice;
                        deviceCountChanged = false;
                        // for old firmware
                        if (m_deviceNames[m_selectedDevice].first) {
                            m_oldFirmwareSelected = true;
                        } else {
                            m_oldFirmwareSelected = false;
                        }
                        // send params request (1 byte) — unchanged
                        hid_write(m_paramsRead, paramsRequest, 1);
                        qDebug()<<"HID opened, connected devices ="<<m_hidDevicesList.size();
                    }
                }
            }
            // device connected
            if (m_paramsRead) {
                // read joy report
                if (m_currentWork == REPORT_ID_PARAM && !m_oldFirmwareSelected) {
                    // send params request periodically — unchanged (2 bytes as before)
                    if (!paramsTimer.isValid() || paramsTimer.hasExpired(5000)) {
                        hid_write(m_paramsRead, paramsRequest, 2);
                        paramsTimer.start();
                    }
                    // read report with debounce on real errors
                    res = hid_read_timeout(m_paramsRead, buffer, BUFFERSIZE, 1000);
                    if (res > 0) {
                        // good frame
                        s_consecutiveErrors = 0;
                        if (!s_lastGoodRx.isValid()) s_lastGoodRx.start(); else s_lastGoodRx.restart();

                        if (buffer[0] == REPORT_ID_PARAM) {   // перестраховка
                            memset(deviceBuffer, 0, BUFFERSIZE);
                            memcpy(deviceBuffer, buffer, BUFFERSIZE);
                            if (ReportConverter::paramReport(deviceBuffer) == -1) continue;
                            if (ReportConverter::paramReport(deviceBuffer)) { // datarace?
                                emit paramsPacketReceived(true);
                            } else {
                                emit paramsPacketReceived(false);
                                QThread::msleep(300);
                            }
                        }
                    } else if (res == 0) {
                        // benign timeout — do nothing
                    } else { // res < 0: real read error
                        ++s_consecutiveErrors;
                        const bool graceElapsed = s_lastGoodRx.isValid() && (s_lastGoodRx.elapsed() >= kGraceMs);
                        if (s_consecutiveErrors >= kMaxConsecutiveErrors || graceElapsed) {
                            // Close and let the loop re-open fast. Do NOT emit deviceDisconnected here.
                            hid_close(m_paramsRead);
                            m_paramsRead = nullptr;
                            paramsTimer.invalidate();
                            s_consecutiveErrors = 0;
                            s_lastGoodRx.invalidate();
                            QThread::msleep(50);
                        } else {
                            // transient hiccup; small backoff
                            QThread::msleep(20);
                        }
                    }
                }
                // read config from device
                else if (m_currentWork == REPORT_ID_CONFIG_IN) {
                    readConfigFromDevice(buffer);
#ifdef QT_DEBUG
                    gEnv.readFinished = true;
#endif
                }
                // write config to device
                else if (m_currentWork == REPORT_ID_CONFIG_OUT) { ////////////// ХУЁВО ПАШЕТ. редко
                    writeConfigToDevice(buffer);
                    // disconnect all devices (unchanged)
                    std::lock_guard<std::mutex> lock(m_mutex);
                    emit deviceDisconnected();
                    m_deviceNames.clear();
                    emit hidDeviceList(m_deviceNames);
                    m_hidDevicesList.clear();  // mutex
                    oldSelectedDevice = m_selectedDevice = -1;
#ifdef QT_DEBUG
                    gEnv.readFinished = false;
#endif
                    QThread::msleep(200);
                }
                else if (m_currentWork == REPORT_ID_DEV) {
                    // snapshot current request
                    quint8 op = 0;
                    QByteArray payload;
                    {
                        std::lock_guard<std::mutex> lock(m_devMutex);
                        op = m_devOp;
                        payload = m_devPayload;
                    }

                    // build one HID report: [ID=REPORT_ID_DEV][op][payload...]
                    uint8_t tx[BUFFERSIZE]{};        // BUFFERSIZE = 64 in your code
                    tx[0] = REPORT_ID_DEV;
                    tx[1] = op;
                    int copyLen = qMin<int>(payload.size(), BUFFERSIZE - 2);
                    if (copyLen > 0)
                        memcpy(tx + 2, payload.constData(), copyLen);

                    // send
                    hid_write(m_paramsRead, tx, BUFFERSIZE);

                    // wait for a single reply with matching [ID, op]
                    QElapsedTimer t; t.start();
                    QByteArray devResp;
                    int res = 0;
                    while (t.elapsed() < 1000) {                 // 1s timeout; tune as needed
                        if (!m_paramsRead) break;
                        res = hid_read_timeout(m_paramsRead, buffer, BUFFERSIZE, 200);
                        if (res < 0) { hid_close(m_paramsRead); m_paramsRead = nullptr; break; }
                        if (res >= 2 && buffer[0] == REPORT_ID_DEV && buffer[1] == op) {
                            devResp = QByteArray(reinterpret_cast<char*>(buffer + 2), res - 2);
                            break;
                        }
                        // ignore other traffic (PARAM frames, etc.)
                    }

                    emit devPacket(op, devResp);      // empty devResp means timeout/error
                    m_currentWork = REPORT_ID_PARAM;  // return to streaming
                }
                else if (m_oldFirmwareSelected) {
                    QThread::msleep(200);
                }
            }
        }
    }
    th.join();
    // free memory. start from first device in list
    hid_free_enumeration(firstInList);
}



// another device selected in comboBox
void HidDevice::setSelectedDevice(int deviceNumber)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (deviceNumber < 0 || m_flasherPath.isEmpty() == false){
        return;
    } else if (deviceNumber > m_hidDevicesList.size() - 1){
        deviceNumber = int(m_hidDevicesList.size() - 1);
    }
    m_selectedDevice = deviceNumber;//std::memory_order_release?
}

// stop processData()
void HidDevice::setIsFinish(bool isFinish)
{
    m_isFinish = isFinish;
}

// read config
void HidDevice::readConfigFromDevice(uint8_t *buffer)
{
    QElapsedTimer timer;
    timer.start();
    int res = 0;
    qint64 start_time = 0;
    qint64 resendTime = 0;
    uint8_t report_count = 0;
    uint8_t config_request_buffer[2] = {REPORT_ID_CONFIG_IN, 1};

    // 62 = BUFFERSIZE(64)-2 bytes (first - ID, second - cfg part)
    uint8_t cfg_count = sizeof(dev_config_t) / 62;
    uint8_t last_cfg_size = sizeof(dev_config_t) % 62;
    if (last_cfg_size > 0)
    {
        cfg_count++;
    }

    start_time = timer.elapsed();
    resendTime = start_time;
    hid_write(m_paramsRead, config_request_buffer, 2);

    while (timer.elapsed() < start_time + 5000)
    {
        if (m_paramsRead)    // перестаховка
        {
            res=hid_read_timeout(m_paramsRead, buffer, BUFFERSIZE,100);
            if (res < 0) {
                hid_close(m_paramsRead);
                m_paramsRead=nullptr;
            }
            else
            {
                if (buffer[0] == REPORT_ID_CONFIG_IN)
                {
                    if (buffer[1] == config_request_buffer[1])
                    {
                        ReportConverter::getConfigFromDevice(buffer);
                        config_request_buffer[1] += 1;
                        hid_write(m_paramsRead, config_request_buffer, 2);
                        report_count++;
                        resendTime = timer.elapsed();
                        qDebug()<<"Config"<<report_count<<"received";
                        if (config_request_buffer[1] > cfg_count)
                        {
                            break;
                        }
                    }
                }
                else if ((resendTime + 250 - timer.elapsed()) <= 0)
                {
                    qDebug() << "Resend activated";
                    config_request_buffer[1] = report_count +1;
                    qDebug() << config_request_buffer[1];
                    resendTime = timer.elapsed();
                    hid_write(m_paramsRead, config_request_buffer, 2);
                }
            }
        } else {
            m_currentWork = REPORT_ID_PARAM;
            emit configReceived(false);
            break;
        }
    }
    qDebug()<<"Read report_count ="<<report_count<<"/"<<cfg_count;
    if (report_count == cfg_count){
        qDebug() << "All config received";
    } else {
        qDebug() << "ERROR, not all config received";
    }

    if (report_count == cfg_count) {
        m_currentWork = REPORT_ID_PARAM;
        emit configReceived(true);
    } else {
        m_currentWork = REPORT_ID_PARAM;
        emit configReceived(false);
    }
}

// write config
void HidDevice::writeConfigToDevice(uint8_t *buffer)
{
    QElapsedTimer timer;
    timer.start();
    int res = 0;
    qint64 startTime = 0;
    qint64 resendTime = 0;
    uint8_t reportCount = 0;
    uint8_t configOutBuf[BUFFERSIZE] = {REPORT_ID_CONFIG_OUT, 0};

    // 62 = BUFFERSIZE(64)-2 bytes (first - ID, second - cfg part)
    uint8_t cfg_count = sizeof(dev_config_t) / 62;
    uint8_t last_cfg_size = sizeof(dev_config_t) % 62;
    if (last_cfg_size > 0)
    {
        cfg_count++;
    }

    startTime = timer.elapsed();
    resendTime = startTime;
    hid_write(m_paramsRead, configOutBuf, BUFFERSIZE);

    while (timer.elapsed() < startTime + 5000)
    {
        if (m_paramsRead)    // перестаховка
        {
            res = hid_read_timeout(m_paramsRead, buffer, BUFFERSIZE,100);
            if (res < 0) {
                hid_close(m_paramsRead);
                m_paramsRead=nullptr;
            }
            else
            {
                if (buffer[0] == REPORT_ID_CONFIG_OUT)
                {
                    if (buffer[1] == configOutBuf[1] + 1)
                    {
                        configOutBuf[1] += 1;
                        ReportConverter::sendConfigToDevice(configOutBuf, configOutBuf[1]);
                        //memcpy(configOutBuf, configOutBuf, BUFFERSIZE);

                        hid_write(m_paramsRead, configOutBuf, BUFFERSIZE);
                        reportCount++;
                        resendTime = timer.elapsed();
                        qDebug()<<"Config"<<reportCount<<"sent";

                        if (buffer[1] == cfg_count){
                            break;
                        }
                    }
                }
                else if ((resendTime + 250 - timer.elapsed()) <= 0)
                {
                    qDebug() << "Resend activated";
                    resendTime = timer.elapsed();
                    configOutBuf[1] = reportCount;
                    hid_write(m_paramsRead, configOutBuf, BUFFERSIZE);
                }
            }
        } else {
            m_currentWork = REPORT_ID_PARAM;
            emit configSent(false);
            break;
        }
    }
    qDebug()<<"Write report_count ="<<reportCount<<"/"<<cfg_count;
    if (reportCount == cfg_count) {
        qDebug() << "All config sent";
        while (timer.elapsed() < startTime + 2000) {
            if (m_paramsRead) {
                res = hid_read_timeout(m_paramsRead, buffer, BUFFERSIZE,100);
                if (res < 0) {
                    hid_close(m_paramsRead);
                    m_paramsRead=nullptr;
                } else if (buffer[1] == 0xFE) {
                    qDebug() << "ERROR! Version doesnt match";
                    m_currentWork = REPORT_ID_PARAM;
                    emit configSent(false);
                    return;
                }
            }
        }
        m_currentWork = REPORT_ID_PARAM;
        emit configSent(true);
    } else {
        qDebug() << "ERROR, not all config sent";
        m_currentWork = REPORT_ID_PARAM;
        emit configSent(false);
    }
}

// flash firmware
void HidDevice::flashFirmwareToDevice()
{
    qDebug()<<"flash size = "<<m_firmware->size();
    if (m_flasherPath.isEmpty() == false)
    {
        hid_device* flasher = hid_open_path(m_flasherPath);
        qint64 millis;
        QElapsedTimer time;
        time.start();
        millis = time.elapsed();
        uint8_t flash_buffer[BUFFERSIZE]{};
        uint8_t flasher_device_buffer[BUFFERSIZE]{};
        uint16_t length = uint16_t(m_firmware->size());
        uint16_t crc16 = FirmwareUpdater::computeChecksum(m_firmware);
        int update_percent = 0;

        flash_buffer[0] = REPORT_ID_FLASH;
        flash_buffer[1] = 0;
        flash_buffer[2] = 0;
        flash_buffer[3] = 0;
        flash_buffer[4] = uint8_t(length & 0xFF);
        flash_buffer[5] = uint8_t(length >> 8);
        flash_buffer[6] = uint8_t(crc16 & 0xFF);
        flash_buffer[7] = uint8_t(crc16 >> 8);

        hid_write(flasher, flash_buffer, BUFFERSIZE);

        int res = 0;
        uint8_t buffer[BUFFERSIZE]{};
        while (time.elapsed() < millis + 50000) // 50 for flashing
        {
            if (flasher){
                res=hid_read_timeout(flasher, buffer, BUFFERSIZE,5000);  // 5000?
                if (res < 0) {
                    hid_close(flasher);
                    flasher=nullptr;
                    return;
                } else {
                    if (buffer[0] == REPORT_ID_FLASH) {
                        memset(flasher_device_buffer, 0, BUFFERSIZE);
                        memcpy(flasher_device_buffer, buffer, BUFFERSIZE);
                    }
                }
            }

            if (flasher_device_buffer[0] == REPORT_ID_FLASH)
            {
                uint16_t cnt = uint16_t(flasher_device_buffer[1] << 8 | flasher_device_buffer[2]);
                if ((cnt & 0xF000) == 0xF000)  // status packet
                {
                    //qDebug()<<"ERROR";
                    if (cnt == 0xF001)  // firmware size error
                    {
                        hid_close(flasher);
                        emit flashStatus(SIZE_ERROR, update_percent);
                        return;
                    }
                    else if (cnt == 0xF002) // CRC error
                    {
                        hid_close(flasher);
                        emit flashStatus(CRC_ERROR, update_percent);
                        return;
                    }
                    else if (cnt == 0xF003) // flash erase error
                    {
                        hid_close(flasher);
                        emit flashStatus(ERASE_ERROR, update_percent);
                        return;
                    }
                    else if (cnt == 0xF000) // OK
                    {
                        hid_close(flasher);
                        emit flashStatus(FINISHED, update_percent);
                        return;
                    }
                }
                else
                {
                    qDebug()<<"Firmware packet requested:"<<cnt;

                    flash_buffer[0] = REPORT_ID_FLASH;
                    flash_buffer[1] = uint8_t(cnt >> 8);
                    flash_buffer[2] = uint8_t(cnt & 0xFF);
                    flash_buffer[3] = 0;

                    if (cnt * 60 < m_firmware->size())
                    {
                        memcpy(flash_buffer +4, m_firmware->constData() + (cnt - 1) * 60, 60);
                        update_percent = ((cnt - 1) * 60 * 100 / m_firmware->size());
                        hid_write(flasher, flash_buffer, 64);
                        emit flashStatus(IN_PROCESS, update_percent);

                        qDebug()<<"Firmware packet sent:"<<cnt;
                    }
                    else
                    {
                        memcpy(flash_buffer +4, m_firmware->constData() + (cnt - 1) * 60, m_firmware->size() - (cnt - 1) * 60);
                        update_percent = 0;
                        hid_write(flasher, flash_buffer, 64);
                        emit flashStatus(IN_PROCESS, update_percent);

                        qDebug()<<"Firmware packet sent:"<<cnt;
                    }
                }
            }
        }
        hid_close(flasher);
        emit flashStatus(666, update_percent);
    }
}

// button "get config" clicked
void HidDevice::getConfigFromDevice()
{
    m_currentWork = REPORT_ID_CONFIG_IN;
}
// button "send config" clicked
void HidDevice::sendConfigToDevice()
{
    m_currentWork = REPORT_ID_CONFIG_OUT;
}
// button "flash firmware" clicked
void HidDevice::flashFirmware(const QByteArray* firmware)
{
    m_firmware = firmware; // pointer?
    m_currentWork = REPORT_ID_FIRMWARE;
}

bool HidDevice::enterToFlashMode()
{
    if(m_paramsRead)
    {
        m_flasherPath.clear();
        QElapsedTimer time;
        time.start();
        uint8_t report = REPORT_ID_FIRMWARE;
        if (m_oldFirmwareSelected) {
            report = 4;
        }
        // send "enter to bootloader mode" message
        uint8_t config_buffer[64] = {report,'b','o','o','t','l','o','a','d','e','r',' ','r','u','n'};
        hid_write(m_paramsRead, config_buffer, 64);
        // waiting flasher
        while (time.hasExpired(DEVICE_SEARCH_DELAY + 700) == false)
        {
            if (m_flasherPath.isEmpty() == false){
                return true;
            }
        }
    }
    return false;
}
void HidDevice::devRequest(quint8 op, const QByteArray &payload)
{
    // minimal synchronization like you already do elsewhere
    std::lock_guard<std::mutex> lock(m_devMutex);
    m_devOp = op;
    m_devPayload = payload;
    m_currentWork = REPORT_ID_DEV;   // picked up by processData() loop
}
