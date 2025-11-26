
#ifndef __COMMON_TYPES_H__
#define __COMMON_TYPES_H__

#include "stdint.h"
#include "common_defines.h"

/******************** AXIS **********************/
enum
{
    FILTER_NO = 0,
    FILTER_LEVEL_1,
    FILTER_LEVEL_2,
    FILTER_LEVEL_3,
    FILTER_LEVEL_4,
    FILTER_LEVEL_5,
    FILTER_LEVEL_6,
    FILTER_LEVEL_7,
};
typedef uint8_t filter_t;

typedef int16_t analog_data_t;

enum
{
    NO_FUNCTION = 0,
    FUNCTION_PLUS,
    FUNCTION_MINUS,
    FUNCTION_EQUAL,
};

enum
{
    AXIS_BUTTON_FUNC_EN = 0,
    AXIS_BUTTON_PRESCALER_EN,
    AXIS_BUTTON_CENTER,
    AXIS_BUTTON_RESET,
    AXIS_BUTTON_DOWN,
    AXIS_BUTTON_UP,
};

typedef struct
{
    analog_data_t  calib_min;
    analog_data_t  calib_center;
    analog_data_t  calib_max;
    uint8_t        out_enabled : 1;
    uint8_t        inverted    : 1;
    uint8_t        is_centered : 1;
    uint8_t        function    : 2;
    uint8_t        filter      : 3;

    int8_t         curve_shape[11];
    uint8_t        resolution : 4;
    uint8_t        channel    : 4;
    uint8_t        deadband_size        : 7;
    uint8_t        is_dynamic_deadband  : 1;

    int8_t         source_main;
    uint8_t        source_secondary : 3;
    uint8_t        offset_angle     : 5;

    int8_t         button1;
    int8_t         button2;
    int8_t         button3;
    uint8_t        divider;
    uint8_t        i2c_address;
    uint8_t        button1_type : 3;
    uint8_t        button2_type : 2;
    uint8_t        button3_type : 3;
    uint8_t        prescaler;
    uint8_t        reserved[1];

} axis_config_t;

enum
{
    SOURCE_I2C     = -2,
    SOURCE_NO      = -1,
};
typedef int8_t axis_source_t;

enum
{
    ANALOG = 0,
    MCP3202,
    ADS1115,
};

typedef struct
{
    uint32_t ok_cnt;
    uint32_t err_cnt;

    uint8_t  rx_complete;
    uint8_t  tx_complete;
    uint8_t  curr_channel;

    int8_t   source;
    uint8_t  type;
    uint8_t  address;

    uint8_t  data[24];

} sensor_t;

/******************** PINS **********************/
enum
{
    NOT_USED = 0,

    BUTTON_GND,
    BUTTON_VCC,
    BUTTON_ROW,
    BUTTON_COLUMN,

    AXIS_ANALOG,

    SPI_SCK,
    SPI_MOSI,
    SPI_MISO,

    MCP3202_CS,

    SHIFT_REG_LATCH,
    SHIFT_REG_DATA,

    I2C_SCL,
    I2C_SDA,

    SHIFT_REG_CLK,
};
typedef int8_t pin_t;

/******************** BUTTONS **********************/
enum
{
    BUTTON_NORMAL = 0,
    BUTTON_TOGGLE,
    TOGGLE_SWITCH,
    TOGGLE_SWITCH_ON,
    TOGGLE_SWITCH_OFF,

    POV1_UP,
    POV1_RIGHT,
    POV1_DOWN,
    POV1_LEFT,
    POV1_CENTER,

    POV2_UP,
    POV2_RIGHT,
    POV2_DOWN,
    POV2_LEFT,
    POV2_CENTER,

    POV3_UP,
    POV3_RIGHT,
    POV3_DOWN,
    POV3_LEFT,

    POV4_UP,
    POV4_RIGHT,
    POV4_DOWN,
    POV4_LEFT,

    RADIO_BUTTON1,
    RADIO_BUTTON2,
    RADIO_BUTTON3,
    RADIO_BUTTON4,

    SEQUENTIAL_TOGGLE,
    SEQUENTIAL_BUTTON,
};
typedef uint8_t button_type_t;

enum
{
    BUTTON_TIMER_OFF = 0,
    BUTTON_TIMER_1,
    BUTTON_TIMER_2,
    BUTTON_TIMER_3,
};
typedef uint8_t button_timer_t;

typedef struct button_t
{
    int8_t         physical_num;
    button_type_t  type            : 5;
    uint8_t        shift_modificator : 3;

    uint8_t        is_inverted : 1;
    uint8_t        is_disabled : 1;
    button_timer_t delay_timer : 3;
    button_timer_t press_timer : 3;

} button_t;

typedef struct physical_buttons_state_t
{
    uint32_t time_last;
    uint8_t  pin_state        : 1;
    uint8_t  prev_pin_state   : 1;
    uint8_t  current_state    : 1;
    uint8_t  changed          : 1;

} physical_buttons_state_t;

enum
{
    BUTTON_ACTION_IDLE = 0,
    BUTTON_ACTION_DELAY,
    BUTTON_ACTION_PRESS,
    BUTTON_ACTION_BLOCK,
};
typedef uint8_t button_action_t;

typedef struct logical_buttons_state_t
{
    uint32_t time_last;
    uint8_t  curr_physical_state : 1;
    uint8_t  prev_physical_state : 1;
    uint8_t  on_state            : 1;
    uint8_t  off_state           : 1;
    uint8_t  current_state       : 1;
    uint8_t  delay_act           : 2;

} logical_buttons_state_t;

/******************** AXIS TO BUTTONS **********************/
typedef struct
{
    uint8_t points[13];
    uint8_t buttons_cnt;

} axis_to_buttons_t;

/******************** SHIFT REGISTERS **********************/
enum
{
    HC165_PULL_DOWN = 0,
    CD4021_PULL_DOWN,
    HC165_PULL_UP,
    CD4021_PULL_UP,
};
typedef uint8_t shift_reg_config_type_t;

typedef struct
{
    uint8_t type;
    uint8_t button_cnt;
    int8_t  pin_latch;
    int8_t  pin_data;
    int8_t  pin_clk;

} shift_reg_t;

typedef struct
{
    uint8_t type;
    uint8_t button_cnt;
    int8_t  reserved[2];

} shift_reg_config_t;

/******************** SHIFT MODIFICATORS **********************/
typedef struct
{
    int8_t button;

} shift_modificator_t;

/******************** FORCE PROFILES & FACTORY ANCHORS ********************/
/* NOTE: The factory anchors block is stored OUTSIDE dev_config_t
 * (separate flash page) so it survives config flashes/firmware updates.
 * This header only defines the layout.
 */

/*typedef enum : uint8_t {
    FORCE_LEVEL_100 = 0,  // “Full”: 17 lbf (roll L/R, pitch down), 25 lbf (PU-digital), 40 lbf (PU-analog)
    FORCE_LEVEL_75  = 1,  // e.g., 12.75 / 18.75 / 30 lbf equivalents (nonlinear handled by measured ADCs)
    FORCE_LEVEL_50  = 2,  // e.g., 8.5 / 12.5 / 20 lbf equivalents
} force_level_t;*/

//* Per-direction triplet of ADC counts for 100/75/50 */
#pragma pack(push, 1)
typedef struct {
    int16_t adc_100;
    int16_t adc_75;
    int16_t adc_50;
} force_triplet_t;
#pragma pack(pop)

/* Factory anchors (protected flash page). GUI Dev tab writes these;
 * normal end-users cannot modify. CRC covers the whole struct.
 *
 * Directions / specs:
 *  - Roll Left      : 17 lbf
 *  - Roll Right     : 17 lbf
 *  - Pitch Down     : 17 lbf
 *  - Pitch Up (DIG) : 25 lbf   (digital detent path)
 *  - Pitch Up (AN)  : 40 lbf   (analog path)
 */
#pragma pack(push, 1)
typedef struct {
    /* housekeeping / integrity */
    uint16_t magic;     /* 0xF00C */
    uint8_t  version;   /* 1 */
    uint8_t  sealed;    /* 1 = locked against non-Dev writes */
    uint32_t crc32;     /* CRC of the entire struct (magic..end), sealed included */

    /* anchors: each has ADC at 100/75/50% of the DIRECTION'S spec force */
    force_triplet_t roll_left_17lbf;
    force_triplet_t roll_right_17lbf;
    force_triplet_t pitch_down_17lbf;
    force_triplet_t pitch_up_25lbf_digital;
    force_triplet_t pitch_up_40lbf_analog;

    /* future extension space */
    uint8_t  reserved[8];
} force_factory_anchors_t;
#pragma pack(pop)
/* Device identification info - stored separately from force anchors */
#pragma pack(push, 1)
typedef struct {
    uint16_t magic;
    uint8_t  version;
    uint8_t  locked;
    uint32_t crc32;
    char     model_number[INV_MODEL_MAX_LEN];      // 16 bytes
    char     serial_number[INV_SERIAL_MAX_LEN];    // 16 bytes
    char     manufacture_date[DOM_ASCII_LEN + 1];  // 11 bytes
    char     device_name[26];                       // 26 bytes (USB device name)
    uint8_t  adc_pga[4];                            // 4 bytes - PGA gain for ADS1115 channels 0-3
    uint8_t  adc_mode[4];                           // 4 bytes - 0=single-ended, 1=differential (pairs: 0-1, 2-3)
} device_info_t;
#pragma pack(pop)

/******************** RUNTIME FORCE PROFILE (safe to reset) *******************
 * Lives in dev_config_t. If the user overwrites config without reading first,
 * only this selector changes—factory anchors remain intact on their own page.
 ******************************************************************************/

typedef enum force_direction_t {
    FORCE_DIR_ROLL_LEFT = 0,
    FORCE_DIR_ROLL_RIGHT,
    FORCE_DIR_PITCH_DOWN,
    FORCE_DIR_PITCH_UP_DIGITAL, /* 25 lbf path */
    FORCE_DIR_PITCH_UP_ANALOG,  /* 40 lbf path */
    FORCE_DIR_COUNT
} force_direction_t;

typedef struct {
    /* For each direction pick: 100/75/50 */
    uint8_t selected_level[FORCE_DIR_COUNT];  /* values from force_level_t */

    /* Which pitch-up path to use in the UI when "Pitch Up" is targeted by radio:
       0 = digital (25 lbf), 1 = analog (40 lbf). */

    uint8_t pitch_up_mode; /* 0=digital, 1=analog */

    uint8_t grip_type;     /* Selected grip profile index (0=none, 1=Invictus Viper, 2=Warthog, etc.) */
    uint8_t board_type;    /* Board type: 0=Gen1-3, 1=Gen4 */
    uint8_t sim_software;  /* Selected simulator: 0=none, 1=DCS, 2=Falcon BMS, 3=MSFS */
} force_profile_runtime_t;



/******************** DEVICE CONFIGURATION **********************/
typedef struct
{
    // config 1
    uint16_t        firmware_version;
    char            device_name[26];
    uint16_t        button_debounce_ms;
    uint8_t         exchange_period_ms;
    pin_t           pins[USED_PINS_NUM];

    // config 2-5
    axis_config_t   axis_config[MAX_AXIS_NUM];

    // config 6-7-8-9-10-11-12
    button_t        buttons[MAX_BUTTONS_NUM];
    uint16_t        button_timer1_ms;             // config packet 6
    uint16_t        button_timer2_ms;             // config packet 7
    uint16_t        button_timer3_ms;             // config packet 8
    uint16_t        a2b_debounce_ms;              // config packet 9

    // config 12-13-14
    axis_to_buttons_t axes_to_buttons[MAX_AXIS_NUM];

    // config 14
    shift_reg_config_t shift_registers[4];
    shift_modificator_t shift_config[5];
    uint16_t        vid;
    uint16_t        pid;

    // config 15
    force_profile_runtime_t force_profile_rt;

} dev_config_t;

/******************** APPLICATION CONFIGURATION **********************/
typedef struct
{
    uint8_t axis;
    uint8_t axis_cnt;
    uint8_t buttons_cnt;
    uint8_t pov;
    uint8_t pov_cnt;
    uint8_t slow_encoder_cnt;
    uint8_t fast_encoder_cnt;
    uint8_t pwm_cnt;

} app_config_t;

/******************** HID REPORT CONFIGURATION **********************/
typedef struct
{
    analog_data_t axis_data[MAX_AXIS_NUM];
    uint8_t       pov_data[MAX_POVS_NUM];
    uint8_t       button_data[MAX_BUTTONS_NUM/8];

} joy_report_t;

typedef struct
{
    uint16_t     firmware_version;
    analog_data_t raw_axis_data[MAX_AXIS_NUM];
    analog_data_t axis_data[MAX_AXIS_NUM];
    uint8_t       phy_button_data[MAX_BUTTONS_NUM/8];
    uint8_t       log_button_data[MAX_BUTTONS_NUM/8];
    uint8_t       shift_button_data;

} params_report_t;

#endif /* __COMMON_TYPES_H__ */
