/**
  ******************************************************************************
  * @file           : common_defines.h
  * @brief          : This file contains the common defines for the app.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_DEFINES_H__
#define __COMMON_DEFINES_H__

//#define DEBUG

#define FIRMWARE_VERSION					0x2121              // v2.1.2 (Fixed firmware flasher)
#define USED_PINS_NUM						30					// Contstant for HOTAS Control boards
#define MAX_AXIS_NUM						8					// max 8
#define MAX_BUTTONS_NUM						128					// power of 2, max 128
#define MAX_POVS_NUM						4					// max 4
#define MAX_ENCODERS_NUM					16					// max 64
#define MAX_SHIFT_REG_NUM					4					// max 4
#define MAX_LEDS_NUM						24

#define AXIS_MIN_VALUE						(-32767)
#define AXIS_MAX_VALUE						(32767)
#define AXIS_CENTER_VALUE					(AXIS_MIN_VALUE + (AXIS_MAX_VALUE-AXIS_MIN_VALUE)/2)
#define AXIS_FULLSCALE						(AXIS_MAX_VALUE - AXIS_MIN_VALUE + 1)

#define CONFIG_ADDR							(0x0800FC00)
#define FLASH_PAGE_SIZE                     0x400
#define FACTORY_ADDR                        (CONFIG_ADDR - FLASH_PAGE_SIZE)  // protected page
#define FACTORY_MAGIC                       0xF00C
#define FACTORY_VERSION                     0x02


#define INVICTUS_GREEEN "rgb(5, 170, 61)"
#define FLAT_BLACK "rgb(36, 39,49)"

/* ---- Lock bits ---- */
#ifndef LOCKBIT_SERIAL
#define LOCKBIT_SERIAL      (1u << 0)
#endif
#ifndef LOCKBIT_MODEL
#define LOCKBIT_MODEL       (1u << 1)
#endif
#ifndef LOCKBIT_DOM
#define LOCKBIT_DOM         (1u << 2)   /* optional lock for DoM */
#endif

/* ---- Dev opcodes ---- */
#ifndef CMD_GET_DEVICE_ID
#define CMD_GET_DEVICE_ID   0xA4  /* reply: model[INV_MODEL_MAX_LEN], serial[INV_SERIAL_MAX_LEN], optional 10 bytes "YYYY-MM-DD" */
#endif
#ifndef CMD_SET_DEVICE_DOM
#define CMD_SET_DEVICE_DOM  0xA7  /* payload: 10-byte ASCII "YYYY-MM-DD" (no NUL) */
#endif

/* ---- Identity lengths (already present) ---- */
#ifndef INV_MODEL_MAX_LEN
#define INV_MODEL_MAX_LEN   24
#endif
#ifndef INV_SERIAL_MAX_LEN
#define INV_SERIAL_MAX_LEN  24
#endif

/* ---- Date format ---- */
#ifndef DOM_ASCII_LEN
#define DOM_ASCII_LEN       10     /* "YYYY-MM-DD" */
#endif
enum
{
    REPORT_ID_JOY = 1,
    REPORT_ID_PARAM,
    REPORT_ID_CONFIG_IN,
    REPORT_ID_CONFIG_OUT,
    REPORT_ID_FIRMWARE,
    REPORT_ID_DEV = 6,   //For force anchors
};

enum {
    OP_GET_FACTORY_ANCHORS  = 1,
    OP_SET_FACTORY_ANCHORS  = 2,
    OP_LOCK_FACTORY_ANCHORS = 3,
    OP_UNLOCK_FACTORY_ANCHORS = 4,
    // New device info operations
    OP_GET_DEVICE_INFO = 5,
    OP_SET_DEVICE_INFO = 6,
};

#ifndef CMD_GET_DEVICE_ID
#define CMD_GET_DEVICE_ID   0xA4  /* read model/serial/[optional DoM "YYYY-MM-DD"] */
#endif

#endif 	/* __COMMON_DEFINES_H__ */
