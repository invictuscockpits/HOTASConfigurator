#ifndef PINController_H
#define PINController_H

#include <QWidget>
#include "pincombobox.h"

QT_BEGIN_NAMESPACE
class QGridLayout;
QT_END_NAMESPACE

namespace Ui {
class PinsController;
}

class PinsController : public QWidget
{
    Q_OBJECT

public:
    explicit PinsController(QWidget *parent = nullptr);
    ~PinsController();

    void addPinComboBox (QList<PinComboBox *> pinList);

private:
    Ui::PinsController *ui;
};

#endif // PINController_H
