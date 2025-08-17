// board_presets.h
#pragma once
#include <array>
#include "common_types.h"
#include "common_defines.h"

struct BoardPins {
    std::array<pin_t, USED_PINS_NUM> pins;
};


enum class BoardId : int {
    VftControllerGen3 = 0,
    VFTControllerGen4 = 1
};

const BoardPins& boardPins(BoardId id);
const char* boardPresetName(BoardId id);
