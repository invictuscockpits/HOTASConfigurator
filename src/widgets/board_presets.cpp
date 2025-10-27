#include "board_presets.h"

static const BoardPins kControllerGen3 = { std::array<pin_t, USED_PINS_NUM>{
    /* 0..29 match PA0..PC15 */
    NOT_USED,        // PA0
    MCP3202_CS,      // PA1
    NOT_USED,        // PA2
    NOT_USED,        // PA3
    NOT_USED,        // PA4
    NOT_USED,        // PA5
    NOT_USED,        // PA6
    NOT_USED,        // PA7
    NOT_USED,        // PA8
    NOT_USED,        // PA9
    NOT_USED,        // PA10
    NOT_USED,        // PA15
    NOT_USED,        // PB0
    NOT_USED,        // PB1
    SPI_SCK,         // PB3
    SPI_MISO,        // PB4
    SPI_MOSI,        // PB5
    NOT_USED,        // PB6
    SHIFT_REG_LATCH, // PB7
    NOT_USED,        // PB8
    NOT_USED,        // PB9
    NOT_USED,        // PB10
    NOT_USED,        // PB11
    NOT_USED,        // PB12
    SHIFT_REG_CLK,   // PB13
    SHIFT_REG_DATA,  // PB14
    NOT_USED,        // PB15
    NOT_USED,        // PC13
    NOT_USED,        // PC14
    NOT_USED         // PC15
} };

static const BoardPins kControllerGen4 = { std::array<pin_t, USED_PINS_NUM>{
    NOT_USED,        // PA0
    NOT_USED,        // PA1
    NOT_USED,        // PA2
    NOT_USED,        // PA3
    NOT_USED,        // PA4
    NOT_USED,        // PA5
    NOT_USED,        // PA6
    NOT_USED,        // PA7
    NOT_USED,        // PA8
    NOT_USED,        // PA9
    NOT_USED,        // PA10
    NOT_USED,        // PA15
    NOT_USED,        // PB0
    NOT_USED,        // PB1
    SPI_SCK,         // PB3
    SPI_MISO,        // PB4
    SPI_MOSI,        // PB5
    NOT_USED,        // PB6
    SHIFT_REG_LATCH, // PB7
    NOT_USED,        // PB8
    NOT_USED,        // PB9
    I2C_SCL,         // PB10
    I2C_SDA,         // PB11
    NOT_USED,        // PB12
    SHIFT_REG_CLK,   // PB13
    SHIFT_REG_DATA,  // PB14
    NOT_USED,        // PB15
    NOT_USED,        // PC13
    NOT_USED,        // PC14
    NOT_USED         // PC15
} };

const BoardPins& boardPins(BoardId id) {
    return (id == BoardId::VftControllerGen3) ? kControllerGen3 : kControllerGen4;
}
