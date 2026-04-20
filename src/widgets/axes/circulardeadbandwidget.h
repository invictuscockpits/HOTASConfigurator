#ifndef CIRCULARDEADBANDWIDGET_H
#define CIRCULARDEADBANDWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPoint>

class CircularDeadbandWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CircularDeadbandWidget(QWidget *parent = nullptr);

    void setDeadbandRadius(int radius);  // 0-127
    void setAxisValues(int x, int y);     // Current axis positions
    void setAxisNames(const QString &xName, const QString &yName);

    QSize sizeHint() const override { return QSize(200, 200); }
    QSize minimumSizeHint() const override { return QSize(150, 150); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_deadbandRadius;
    QPoint m_currentPosition;
    QString m_xAxisName;
    QString m_yAxisName;

    void drawGrid(QPainter &painter);
    void drawDeadbandCircle(QPainter &painter);
    void drawCurrentPosition(QPainter &painter);
    void drawAxisLabels(QPainter &painter);
};

#endif // CIRCULARDEADBANDWIDGET_H
