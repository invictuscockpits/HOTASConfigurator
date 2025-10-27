#include "mousewheelguard.h"

#include <QEvent>
#include <QWidget>

MouseWheelGuard::MouseWheelGuard(QObject *parent)
    : QObject(parent)
{}

// Prevents accidental mouse wheel changes to widgets without focus
// Requires setFocusPolicy(Qt::StrongFocus) to make widget click-to-scroll
bool MouseWheelGuard::eventFilter(QObject *o, QEvent *e)
{
    const QWidget *widget = static_cast<QWidget *>(o);
    if (e->type() == QEvent::Wheel && widget && !widget->hasFocus()) {
        e->ignore();
        return true;
    }

    return QObject::eventFilter(o, e);
}
