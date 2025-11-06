#ifndef BUTTONLOGICAL_H
#define BUTTONLOGICAL_H

#include "common_types.h"
#include <QWidget>

#include "deviceconfig.h"
#include "global.h"
//#include <QThread>

#define TIMER_COUNT 4 // "NO timer" + count
#define SHIFT_COUNT 6

namespace Ui {
class ButtonLogical;
}

class ButtonLogical : public QWidget
{
    Q_OBJECT

public:
    explicit ButtonLogical(int buttonIndex, QWidget *parent = nullptr);
    ~ButtonLogical();
    void readFromConfig();
    void writeToConfig();

    void initialization();

    void setMaxPhysButtons(int maxPhysButtons);
    void setSpinBoxOnOff(int maxPhysButtons);

    void setButtonState(bool setState);

    void setPhysicButton(int buttonIndex);
    void setAutoPhysBut(bool enabled);
    int currentFocus() const;

    void disableButtonType(button_type_t type, bool disable);
    button_type_t currentButtonType();

    void retranslateUi();

    void rememberGripOriginal();
    void restoreGripOriginal();
    void setFunctionType(button_type_t newType); //modular config building
    void renameButtonNormalLabel(const QString &customText); //renaming button_normals to F-16 button names
    void setJsonName(const QString &name);     // Stores button names for repopulating when switching to Falcon BMS
    QString jsonName() const;                  // optional getter

signals:
    void functionTypeChanged(button_type_t current, button_type_t previous, int buttonIndex);

private slots:
    void editingOnOff(int value);
    void functionIndexChanged(int index);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::ButtonLogical *ui;
    int m_functionPrevType;
    bool m_currentState;
    bool m_debugState;
    int m_buttonIndex;
    QString m_jsonName; //Preserving json names to repopulate when switching to BMS
    static int m_currentFocus;
    static bool m_autoPhysButEnabled;
    QElapsedTimer m_lastAct;

    QVector<int> m_logicFunc_enumIndex;
    button_type_t m_originalGripType;//modular config building


    const deviceEnum_guiName_t m_timerList[TIMER_COUNT] = // order must be as in common_types.h!!!!!!!!!!!          // static ?
    {
        {BUTTON_TIMER_OFF,      tr("-")},
        {BUTTON_TIMER_1,        tr("Timer 1")},
        {BUTTON_TIMER_2,        tr("Timer 2")},
        {BUTTON_TIMER_3,        tr("Timer 3")},
    };

    const deviceEnum_guiName_t m_shiftList[SHIFT_COUNT] = // order must be as in common_types.h!!!!!!!!!!!          // static ?
    {
        {0,        tr("-")},
        {1,        tr("Shift 1")},
        {2,        tr("Shift 2")},
        {3,        tr("Shift 3")},
        {4,        tr("Shift 4")},
        {5,        tr("Shift 5")},
    };
    //static deviceEnum_guiName_t logical_function_list_[LOGICAL_FUNCTION_COUNT];

    const QVector <deviceEnum_guiName_t> m_logicFunctionList = // any order          // static ?
    {{
        {BUTTON_NORMAL,        tr("Button normal")},
        {BUTTON_TOGGLE,        tr("Button toggle")},
        {TOGGLE_SWITCH,        tr("Toggle switch ON/OFF")},
        {TOGGLE_SWITCH_ON,     tr("Toggle switch ON")},
        {TOGGLE_SWITCH_OFF,    tr("Toggle switch OFF")},
        {POV1_UP,              tr("CMS Forward")},
        {POV1_RIGHT,           tr("CMS Right")},
        {POV1_DOWN,            tr("CMS Aft")},
        {POV1_LEFT,            tr("CMS Left")},
        {POV1_CENTER,          tr("CMS Depress")},
        {POV2_UP,              tr("Nose Down")},
        {POV2_RIGHT,           tr("R Wing Dwn")},
        {POV2_DOWN,            tr("Nose Up")},
        {POV2_LEFT,            tr("L Wing Dwn")},
        {POV2_CENTER,          tr("Trim Center Depress")},
        {POV3_UP,              tr("DMS Up")},
        {POV3_RIGHT,           tr("DMS Right")},
        {POV3_DOWN,            tr("DMS Down")},
        {POV3_LEFT,            tr("DMS Left")},
        {POV4_UP,              tr("TMS Up")},
        {POV4_RIGHT,           tr("TMS Right")},
        {POV4_DOWN,            tr("TMS Down")},
        {POV4_LEFT,            tr("TMS Left")},
        {RADIO_BUTTON1,        tr("Radio button 1")},
        {RADIO_BUTTON2,        tr("Radio button 2")},
        {RADIO_BUTTON3,        tr("Radio button 3")},
        {RADIO_BUTTON4,        tr("Radio button 4")},
        {SEQUENTIAL_TOGGLE,    tr("Sequential toggle")},
        {SEQUENTIAL_BUTTON,    tr("Sequential button")},
    }};
};



#endif // BUTTONLOGICAL_H
