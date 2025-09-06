#include "reportconverter.h"

#include "deviceconfig.h"
#include "global.h"

namespace ReportConverter
{
    int8_t firmwareCompatible = -1;
//    namespace
//    {
//        int8_t firmwareCompatible = -1;
//    }
}

    int ReportConverter::paramReport(uint8_t *paramsBuf)
    {
        if (!paramsBuf) return 0;

        // Page 0: ALWAYS copy first 62 bytes so UI shows what's actually on the wire
        if (paramsBuf[1] == 0) {
            // copy raw payload (skip [0]=ID, [1]=page index)
            memcpy(reinterpret_cast<uint8_t *>(&gEnv.pDeviceConfig->paramsReport),
                   paramsBuf + 2, 62);

            // Now decide compatibility from freshly copied bytes
            const uint16_t fw_on_wire = gEnv.pDeviceConfig->paramsReport.firmware_version;
            // keep your policy (was 0xFF00 while investigating)
            firmwareCompatible = ((fw_on_wire & 0xFFF0) == (FIRMWARE_VERSION & 0xFFF0)) ? 1 : 0;

            // Return 1 so the UI refreshes the displayed version immediately
            return 1;
        }

        // Page 1: only apply if compatible
        if (paramsBuf[1] == 1) {
            if (firmwareCompatible == 1) {
                memcpy(reinterpret_cast<uint8_t *>(&gEnv.pDeviceConfig->paramsReport) + 62,
                       paramsBuf + 2, sizeof(params_report_t) - 62);
                return 1;
            } else if (firmwareCompatible == -1) {
                return -1; // haven't seen page 0 yet
            }
            return 0; // incompatible → ignore tail
        }

        return 0;
    }

void ReportConverter::resetReport()
{
    firmwareCompatible = -1;
    memset(&gEnv.pDeviceConfig->paramsReport, 0, sizeof(params_report_t));
}

void ReportConverter::getConfigFromDevice(uint8_t *hidBuf)
{
    uint8_t cfg_count = sizeof(dev_config_t) / 62;
    uint8_t last_cfg_size = sizeof(dev_config_t) % 62;
    if (last_cfg_size > 0) {
        cfg_count++;
    }

    if (hidBuf[1] == cfg_count && last_cfg_size > 0) {
        memcpy((uint8_t *)&(gEnv.pDeviceConfig->config) + 62*(hidBuf[1] - 1), hidBuf + 2, last_cfg_size);
    } else {
        memcpy((uint8_t *)&(gEnv.pDeviceConfig->config) + 62*(hidBuf[1] - 1), hidBuf + 2, 62);
    }
}

void ReportConverter::sendConfigToDevice(uint8_t *hidBuf, uint8_t requestConfigNumber)
{
    uint8_t cfg_count = sizeof(dev_config_t) / 62;
    uint8_t last_cfg_size = sizeof(dev_config_t) % 62;
    if (last_cfg_size > 0) {
        cfg_count++;
    }

    hidBuf[0] = REPORT_ID_CONFIG_OUT;
    hidBuf[1] = requestConfigNumber;

    if (requestConfigNumber == cfg_count && last_cfg_size > 0) {
        memcpy(&hidBuf[2], (uint8_t *)&(gEnv.pDeviceConfig->config) + 62*(requestConfigNumber - 1), last_cfg_size);
    } else {
        memcpy(&hidBuf[2], (uint8_t *)&(gEnv.pDeviceConfig->config) + 62*(requestConfigNumber - 1), 62);
    }
}
