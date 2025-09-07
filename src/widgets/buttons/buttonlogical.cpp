#include "buttonlogical.h"
#include "ui_buttonlogical.h"

#include "widgets/debugwindow.h"
#include "converter.h"

int ButtonLogical::m_currentFocus = -1;
bool ButtonLogical::m_autoPhysButEnabled = false;

ButtonLogical::ButtonLogical(int buttonIndex, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ButtonLogical)
{
    ui->setupUi(this);
    m_buttonIndex = buttonIndex;
    m_functionPrevType = BUTTON_NORMAL;
    m_currentState = false;
    m_debugState = false;
    ui->label_LogicalButtonNumber->setNum(m_buttonIndex + 1);
    ui->spinBox_PhysicalButtonNumber->installEventFilter(this);
}

ButtonLogical::~ButtonLogical()
{
    delete ui;
}

void ButtonLogical::retranslateUi()
{
    ui->retranslateUi(this);
}

void ButtonLogical::initialization()
{
    // add gui text
    m_logicFunc_enumIndex.reserve(m_logicFunctionList.size());
    for (int i = 0; i < m_logicFunctionList.size(); i++) {
//        // Encoder only for first 30 buttons  // if uncomment DONT FORGET EDIT MainWindow::oldConfigHandler() !!!!!!!!
//        if (m_buttonNumber > 29 &&
//                (m_logicFunctionList[i].deviceEnumIndex == ENCODER_INPUT_A ||
//                 m_logicFunctionList[i].deviceEnumIndex == ENCODER_INPUT_B)) {
//            continue;
//        }
        ui->comboBox_ButtonFunction->addItem(QString(m_logicFunctionList[i].guiName));

        m_logicFunc_enumIndex.push_back(m_logicFunctionList[i].deviceEnumIndex);
    }
    for (int i = 0; i < SHIFT_COUNT; i++) {
        ui->comboBox_ShiftIndex->addItem(m_shiftList[i].guiName);
    }
    for (int i = 0; i < TIMER_COUNT; i++) {
        ui->comboBox_DelayTimerIndex->addItem(m_timerList[i].guiName);
        ui->comboBox_PressTimerIndex->addItem(m_timerList[i].guiName);
    }

    connect(ui->comboBox_ButtonFunction, SIGNAL(currentIndexChanged(int)),
            this, SLOT(functionIndexChanged(int)));
    connect(ui->spinBox_PhysicalButtonNumber, SIGNAL(valueChanged(int)),
            this, SLOT(editingOnOff(int)));
}

void ButtonLogical::setMaxPhysButtons(int maxPhysButtons)
{
    ui->spinBox_PhysicalButtonNumber->setMaximum(maxPhysButtons);
}

void ButtonLogical::setSpinBoxOnOff(int maxPhysButtons)
{
    if (maxPhysButtons > 0) {
        ui->spinBox_PhysicalButtonNumber->setEnabled(true);
    } else {
        ui->spinBox_PhysicalButtonNumber->setEnabled(false);
    }
}

void ButtonLogical::functionIndexChanged(int index)
{
    int type = m_logicFunctionList[index].deviceEnumIndex;
    emit functionTypeChanged(type, m_functionPrevType, m_buttonIndex);
    m_functionPrevType = type;
}

void ButtonLogical::editingOnOff(int value)
{
    if (value > 0 && ui->spinBox_PhysicalButtonNumber->isEnabled()) {
        ui->checkBox_IsInvert->setEnabled(true);
        ui->checkBox_IsDisable->setEnabled(true);
        ui->comboBox_ButtonFunction->setEnabled(true);
        ui->comboBox_ShiftIndex->setEnabled(true);
        ui->comboBox_DelayTimerIndex->setEnabled(true);
        ui->comboBox_PressTimerIndex->setEnabled(true);
    } else {
        ui->checkBox_IsInvert->setEnabled(false);
        ui->checkBox_IsDisable->setEnabled(false);
        ui->comboBox_ButtonFunction->setEnabled(false);
        ui->comboBox_ShiftIndex->setEnabled(false);
        ui->comboBox_DelayTimerIndex->setEnabled(false);
        ui->comboBox_PressTimerIndex->setEnabled(false);
    }
}

void ButtonLogical::setButtonState(bool state)
{
    if (state != m_currentState) {
        setAutoFillBackground(true);

        if (state) {
            QPalette pal(window()->palette());
            pal.setColor(QPalette::Window, QColor(5, 170, 61)); //changed to invictus green
            setPalette(pal);

            m_lastAct.start();
            m_currentState = state;
        } else {
            // sometimes state dont have time to render. e.g. encoder press time 10ms and monitor refresh time 17ms(60fps)
            if (m_lastAct.hasExpired(30)) {
                setPalette(window()->palette());
                m_currentState = state;
            }
        }
    }
    // debug shows real state without a timer for rendering
    if (state != m_debugState) {
        if (state) {
            if (gEnv.pDebugWindow) {
                gEnv.pDebugWindow->logicalButtonState(ui->label_LogicalButtonNumber->text().toInt(), true);
            }
        } else {
            if (gEnv.pDebugWindow) {
                gEnv.pDebugWindow->logicalButtonState(ui->label_LogicalButtonNumber->text().toInt(), false);
            }
        }
        m_debugState = state;
    }
}

void ButtonLogical::setPhysicButton(int buttonIndex)
{
    ui->spinBox_PhysicalButtonNumber->setValue(buttonIndex + 1); // +1 !!!!
}

int ButtonLogical::currentFocus() const
{
    return m_currentFocus;
}

void ButtonLogical::setAutoPhysBut(bool enabled)
{
    m_autoPhysButEnabled = enabled;
}


void ButtonLogical::disableButtonType(button_type_t type, bool disable)
{
    int CBoxIndex = Converter::EnumToIndex(type, m_logicFunc_enumIndex);
    if (disable == false){
        ui->comboBox_ButtonFunction->setItemData(CBoxIndex, 1 | 32, Qt::UserRole - 1);
    } else {
        ui->comboBox_ButtonFunction->setItemData(CBoxIndex, 0, Qt::UserRole - 1);
    }
}

button_type_t ButtonLogical::currentButtonType()
{
    return m_logicFunc_enumIndex[ui->comboBox_ButtonFunction->currentIndex()];
}


bool ButtonLogical::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)
    if (m_autoPhysButEnabled) {
        if (event->type() == QEvent::FocusIn) {
            ui->spinBox_PhysicalButtonNumber->setStyleSheet(

                "color: palette(highlighted-text);"
                "border: 1px solid palette(highlight);"
                );
            m_currentFocus = m_buttonIndex;
        } else if (event->type() == QEvent::FocusOut){
            ui->spinBox_PhysicalButtonNumber->setStyleSheet("");
            m_currentFocus = -1;
        }
    }
    return false;
}

/**
 * @brief Stores the grip-defined logical function for later restoration.
 *
 * This should be called after the grip profile is initially applied,
 * to allow remapping (e.g., for Falcon BMS) and future restoration.
 */
void ButtonLogical::rememberGripOriginal()
{
    m_originalGripType = m_logicFunc_enumIndex[ui->comboBox_ButtonFunction->currentIndex()];
}


/**
 * @brief Restores the original logical function as defined by the grip profile.
 *
 * This reverses any temporary remapping done for simulator compatibility (e.g., Falcon BMS).
 */
void ButtonLogical::restoreGripOriginal()
{
    setFunctionType(m_originalGripType);
}


/**
 * @brief Sets the function type for this logical button explicitly.
 *
 * Changes the currently selected entry in the function type combo box
 * to the specified `button_type_t` value.
 *
 * @param newType The function type to apply (e.g., BUTTON_NORMAL, POV2_UP).
 */
void ButtonLogical::setFunctionType(button_type_t newType)
{
    int index = Converter::EnumToIndex(newType, m_logicFunc_enumIndex);
    if (index >= 0) {
        ui->comboBox_ButtonFunction->setCurrentIndex(index);
    }
}
//Renaming button normals to match F-16 buttons with grip json
void ButtonLogical::renameButtonNormalLabel(const QString &customText)
{
    int index = ui->comboBox_ButtonFunction->currentIndex();

    // Only rename if the selected function is BUTTON_NORMAL
    if (m_logicFunc_enumIndex[index] == BUTTON_NORMAL) {
        ui->comboBox_ButtonFunction->setItemText(index, customText);
    }
}
//Doing the same after switching to Falcon BMS
void ButtonLogical::setJsonName(const QString &name)
{
    m_jsonName = name;
}

QString ButtonLogical::jsonName() const
{
    return m_jsonName;
}

void ButtonLogical::readFromConfig()
{
    button_t *button = &gEnv.pDeviceConfig->config.buttons[m_buttonIndex];
    // physical
    ui->spinBox_PhysicalButtonNumber->setValue(button->physical_num + 1); // +1 !!!!
    // isDisable
    ui->checkBox_IsDisable->setChecked(button->is_disabled);
    // isInvert
    ui->checkBox_IsInvert->setChecked(button->is_inverted);

    // logical button function
    ui->comboBox_ButtonFunction->setCurrentIndex(Converter::EnumToIndex(button->type, m_logicFunc_enumIndex));
    // shift
    for (int i = 0; i < SHIFT_COUNT; i++) {
        if (button->shift_modificator == m_shiftList[i].deviceEnumIndex) {
            ui->comboBox_ShiftIndex->setCurrentIndex(i);
            break;
        }
    }
    // delay timer
    for (int i = 0; i < TIMER_COUNT; i++) {
        if (button->delay_timer == m_timerList[i].deviceEnumIndex) {
            ui->comboBox_DelayTimerIndex->setCurrentIndex(i);
            break;
        }
    }
    // toggle timer
    for (int i = 0; i < TIMER_COUNT; i++) {
        if (button->press_timer == m_timerList[i].deviceEnumIndex) {
            ui->comboBox_PressTimerIndex->setCurrentIndex(i);
            break;
        }
    }
}

void ButtonLogical::writeToConfig()
{
    button_t *button = &gEnv.pDeviceConfig->config.buttons[m_buttonIndex];

    button->physical_num = ui->spinBox_PhysicalButtonNumber->value() - 1; // -1 !!!!
    button->is_disabled = ui->checkBox_IsDisable->isChecked();
    button->is_inverted = ui->checkBox_IsInvert->isChecked();

    button->type = m_logicFunc_enumIndex[ui->comboBox_ButtonFunction->currentIndex()];
    button->shift_modificator = m_shiftList[ui->comboBox_ShiftIndex->currentIndex()].deviceEnumIndex;
    button->delay_timer = m_timerList[ui->comboBox_DelayTimerIndex->currentIndex()].deviceEnumIndex;
    button->press_timer = m_timerList[ui->comboBox_PressTimerIndex->currentIndex()].deviceEnumIndex;
}
