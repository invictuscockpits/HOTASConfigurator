#include "converter.h"
#include <QDebug>

Converter::Converter() {}

//! Returns the index of the item containing the deviceEnum ; otherwise returns -1
int Converter::EnumToIndex(const int deviceEnum, const QVector<deviceEnum_guiName_t> &list)
{
    for (int i = 0; i < list.size(); ++i) {
        if (deviceEnum == list[i].deviceEnumIndex) {
            return i;
        }
    }
    qCritical() << "Converter::EnumToIndex returns -1";
    return -1;
}

//! Returns the index of the item containing the deviceEnum ; otherwise returns -1
int Converter::EnumToIndex(const int deviceEnum, const QVector<int> &list)
{
    for (int i = 0; i < list.size(); ++i) {
        if (deviceEnum == list[i]) {
            return i;
        }
    }
    qCritical() << "Converter::FindIndex returns -1";
    return -1;
}

button_type_t Converter::StringToButtonType(const QString& name)
{
    static QMap<QString, button_type_t> typeMap = {
        {"BUTTON_NORMAL", BUTTON_NORMAL},
        {"BUTTON_TOGGLE", BUTTON_TOGGLE},
        {"TOGGLE_SWITCH", TOGGLE_SWITCH},
        {"TOGGLE_SWITCH_ON", TOGGLE_SWITCH_ON},
        {"TOGGLE_SWITCH_OFF", TOGGLE_SWITCH_OFF},

        {"POV1_UP", POV1_UP}, {"POV1_RIGHT", POV1_RIGHT}, {"POV1_DOWN", POV1_DOWN}, {"POV1_LEFT", POV1_LEFT}, {"POV1_CENTER", POV1_CENTER},
        {"POV2_UP", POV2_UP}, {"POV2_RIGHT", POV2_RIGHT}, {"POV2_DOWN", POV2_DOWN}, {"POV2_LEFT", POV2_LEFT}, {"POV2_CENTER", POV2_CENTER},
        {"POV3_UP", POV3_UP}, {"POV3_RIGHT", POV3_RIGHT}, {"POV3_DOWN", POV3_DOWN}, {"POV3_LEFT", POV3_LEFT},
        {"POV4_UP", POV4_UP}, {"POV4_RIGHT", POV4_RIGHT}, {"POV4_DOWN", POV4_DOWN}, {"POV4_LEFT", POV4_LEFT},

        {"ENCODER_INPUT_A", ENCODER_INPUT_A},
        {"ENCODER_INPUT_B", ENCODER_INPUT_B},

        {"RADIO_BUTTON1", RADIO_BUTTON1}, {"RADIO_BUTTON2", RADIO_BUTTON2},
        {"RADIO_BUTTON3", RADIO_BUTTON3}, {"RADIO_BUTTON4", RADIO_BUTTON4},

        {"SEQUENTIAL_TOGGLE", SEQUENTIAL_TOGGLE},
        {"SEQUENTIAL_BUTTON", SEQUENTIAL_BUTTON}
    };

    return typeMap.value(name.trimmed().toUpper(), BUTTON_NORMAL);
}
