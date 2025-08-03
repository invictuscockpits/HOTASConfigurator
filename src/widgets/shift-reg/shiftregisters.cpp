#include "shiftregisters.h"
#include "ui_shiftregisters.h"
#include <cmath>
#include <QJsonObject>
#include <QJsonArray>

QString ShiftRegisters::m_notDefined = nullptr;

ShiftRegisters::ShiftRegisters(int shiftRegNumber, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShiftRegisters)
{
    ui->setupUi(this);

    // To perform translation at application startup, the translator must be initialized after the application starts.
    if (m_notDefined == nullptr) {
        m_notDefined = tr("Not defined");
    }

    m_buttonsCount = 0;
    m_latchPin = 0;
    m_dataPin = 0;
    m_shiftRegNumber = shiftRegNumber;
    ui->label_ShiftIndex->setNum(shiftRegNumber + 1);

    for (int i = 0; i < SHIFT_REG_TYPES; ++i) {
        ui->comboBox_ShiftRegType->addItem(m_shiftRegistersList[i].guiName);
        ui->label_DataPin->setText(m_notDefined);
        ui->label_ClkPin->setText(m_notDefined);
        ui->label_LatchPin->setText(m_notDefined);
    }

    connect(ui->spinBox_ButtonCount, SIGNAL(valueChanged(int)), this, SLOT(calcRegistersCount(int)));
}

ShiftRegisters::~ShiftRegisters()
{
    delete ui;
}

void ShiftRegisters::retranslateUi()
{
    ui->retranslateUi(this);
}

void ShiftRegisters::calcRegistersCount(int count)
{
    ui->label_RegistersCount->setNum(ceil(count / 8.0));

    if (ui->spinBox_ButtonCount->isEnabled() == true) {
        emit buttonCountChanged(count, m_buttonsCount);
        m_buttonsCount = count;
    }
}

void ShiftRegisters::setLatchPin(int latchPin, QString pinGuiName)
{
    if (latchPin != 0) {
        m_latchPin = latchPin;
        ui->label_LatchPin->setText(pinGuiName);
    } else {
        m_latchPin = 0;
        ui->label_LatchPin->setText(m_notDefined);
    }
    setUiOnOff();
}

void ShiftRegisters::setClkPin(int clkPin, QString pinGuiName)
{
    if (clkPin != 0) {
        m_clkPin = clkPin;
        ui->label_ClkPin->setText(pinGuiName);
    } else {
        m_clkPin = 0;
        ui->label_ClkPin->setText(m_notDefined);
    }
    setUiOnOff();
}

void ShiftRegisters::setDataPin(int dataPin, QString pinGuiName)
{
    if (dataPin != 0) {
        m_dataPin = dataPin;
        ui->label_DataPin->setText(pinGuiName);
    } else {
        m_dataPin = 0;
        ui->label_DataPin->setText(m_notDefined);
    }
    setUiOnOff();
}

void ShiftRegisters::setUiOnOff()
{
    if (m_latchPin > 0 && m_clkPin > 0 && m_dataPin > 0) {
        for (auto &&child : this->findChildren<QWidget *>()) {
            child->setEnabled(true);
        }
    } else {
        ui->spinBox_ButtonCount->setValue(0);
        for (auto &&child : this->findChildren<QWidget *>()) {
            child->setEnabled(false);
        }
    }
}
void ShiftRegisters::applyJsonConfig(const QJsonObject &json) {
    if (json.contains("type")) {
        QString typeStr = json["type"].toString();
        int index = ui->comboBox_ShiftRegType->findText(typeStr);
        if (index >= 0)
            ui->comboBox_ShiftRegType->setCurrentIndex(index);
    }

    if (json.contains("button_count"))
        ui->spinBox_ButtonCount->setValue(json["button_count"].toInt());
}
const QString &ShiftRegisters::defaultText() const
{
    return m_notDefined;
}

void ShiftRegisters::readFromConfig()
{
    ui->comboBox_ShiftRegType->setCurrentIndex(gEnv.pDeviceConfig->config.shift_registers[m_shiftRegNumber].type);
    ui->spinBox_ButtonCount->setValue(gEnv.pDeviceConfig->config.shift_registers[m_shiftRegNumber].button_cnt);
}

void ShiftRegisters::writeToConfig()
{
    gEnv.pDeviceConfig->config.shift_registers[m_shiftRegNumber].type = ui->comboBox_ShiftRegType->currentIndex();
    gEnv.pDeviceConfig->config.shift_registers[m_shiftRegNumber].button_cnt = ui->spinBox_ButtonCount->value();
}
