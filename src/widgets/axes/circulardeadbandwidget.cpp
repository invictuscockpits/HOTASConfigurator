#include "circulardeadbandwidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>

CircularDeadbandWidget::CircularDeadbandWidget(QWidget *parent)
    : QWidget(parent)
    , m_deadbandRadius(15)
    , m_currentPosition(0, 0)
    , m_xAxisName("X")
    , m_yAxisName("Y")
{
    setMinimumSize(150, 150);
}

void CircularDeadbandWidget::setDeadbandRadius(int radius)
{
    m_deadbandRadius = radius;
    update();
}

void CircularDeadbandWidget::setAxisValues(int x, int y)
{
    // Scale from axis range (-32767 to 32767) to widget coordinates
    int centerX = width() / 2;
    int centerY = height() / 2;
    int scale = qMin(width(), height()) / 2 - 20;  // Leave margin

    m_currentPosition.setX(centerX + (x * scale / 32767));
    m_currentPosition.setY(centerY - (y * scale / 32767));  // Invert Y
    update();
}

void CircularDeadbandWidget::setAxisNames(const QString &xName, const QString &yName)
{
    m_xAxisName = xName;
    m_yAxisName = yName;
    update();
}

void CircularDeadbandWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fill background
    painter.fillRect(rect(), QColor(36, 39, 49));  // Match app dark theme

    drawGrid(painter);
    drawDeadbandCircle(painter);
    drawCurrentPosition(painter);
    drawAxisLabels(painter);
}

void CircularDeadbandWidget::drawGrid(QPainter &painter)
{
    QPen gridPen(QColor(60, 60, 60), 1, Qt::DashLine);
    painter.setPen(gridPen);

    int centerX = width() / 2;
    int centerY = height() / 2;

    // Draw crosshairs
    painter.drawLine(0, centerY, width(), centerY);  // Horizontal
    painter.drawLine(centerX, 0, centerX, height()); // Vertical

    // Draw outer boundary circle
    int radius = qMin(width(), height()) / 2 - 10;
    painter.drawEllipse(QPoint(centerX, centerY), radius, radius);
}

void CircularDeadbandWidget::drawDeadbandCircle(QPainter &painter)
{
    int centerX = width() / 2;
    int centerY = height() / 2;
    int scale = qMin(width(), height()) / 2 - 20;

    // Calculate deadband radius in pixels (scale from 0-127 to pixel space)
    int deadbandPixels = (m_deadbandRadius * scale) / 127;

    // Fill deadband area
    painter.setBrush(QColor(5, 170, 61, 50));  // Invictus green, semi-transparent
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPoint(centerX, centerY), deadbandPixels, deadbandPixels);

    // Draw deadband boundary
    QPen boundaryPen(QColor(5, 170, 61), 2);  // Invictus green
    painter.setPen(boundaryPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QPoint(centerX, centerY), deadbandPixels, deadbandPixels);
}

void CircularDeadbandWidget::drawCurrentPosition(QPainter &painter)
{
    // Draw current stick position as red dot
    painter.setBrush(QColor(255, 50, 50));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(m_currentPosition, 5, 5);

    // Draw line from center to current position
    int centerX = width() / 2;
    int centerY = height() / 2;
    QPen linePen(QColor(255, 50, 50, 150), 1);
    painter.setPen(linePen);
    painter.drawLine(centerX, centerY, m_currentPosition.x(), m_currentPosition.y());
}

void CircularDeadbandWidget::drawAxisLabels(QPainter &painter)
{
    painter.setPen(QColor(200, 200, 200));
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    int centerX = width() / 2;
    int centerY = height() / 2;

    // X axis label (right)
    painter.drawText(width() - 30, centerY + 5, m_xAxisName);

    // Y axis label (top)
    painter.drawText(centerX - 10, 15, m_yAxisName);
}
