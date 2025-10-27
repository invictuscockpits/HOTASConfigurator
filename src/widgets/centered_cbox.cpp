#include "centered_cbox.h"

#include <QComboBox>
#include <QFontMetricsF>
#include <QItemDelegate>
#include <QStylePainter>

CenteredCBox::CenteredCBox(QWidget *parent)
    : QComboBox(parent)
{
    m_arrowWidth = 0;
}

// setting text approximately centered
void CenteredCBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    QStyleOptionComboBox option;
    initStyleOption(&option);
    painter.drawComplexControl(QStyle::CC_ComboBox, option);

    option.direction = Qt::LeftToRight;

    if (style()) {
        QRect rect = style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxArrow, this);
        m_arrowWidth = rect.width();
    }

    QFontMetricsF font_metric(property("font").value<QFont>());
    // Font width calculation (note: may be measured at character centers)
    qreal font_width = font_metric.horizontalAdvance(option.currentText);
    font_metric.averageCharWidth();

    // Calculate offset to optically center text accounting for arrow on right
    // Using arrow_width/3.2 provides better visual balance than pure centering
    int offset = option.rect.center().x() - m_arrowWidth / 3.2f - font_width / 2;

    // Reduce left offset if text is wider than available space
    if (offset + font_width*1.1f > option.rect.right() - m_arrowWidth) {
        offset -= ((offset + font_width*1.1f) - (option.rect.right() - m_arrowWidth));
        if (offset < 0)
            offset = 0;
    }
    option.rect.setLeft(offset);

    painter.drawControl(QStyle::CE_ComboBoxLabel, option);
}
