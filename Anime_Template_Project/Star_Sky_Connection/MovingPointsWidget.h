#ifndef MOVINGPOINTSWIDGET_H
#define MOVINGPOINTSWIDGET_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QRandomGenerator>
#include <QPointF>
#include <QColor>
#include <QSizeF>
#include <QPainter>
#include <QtMath>
#include <QDateTime>
#include <QMouseEvent>
#include <QObject>


class MovingPoint
{
public:
    explicit MovingPoint(const QPointF& pos, const QPointF& direction, double speed, const QColor& color);

    void updatePosition(double deltaTime);
    void setDirection(const QPointF& newDirection);
    QPointF getPosition() const;
    QColor getColor() const;
    QPointF getDirection() const;

    bool isOffscreen(const QSizeF& windowSize) const;

private:
    QPointF m_position;
    QPointF m_direction;
    double m_speed;
    QColor m_color;
};


class MovingPointsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovingPointsWidget(QWidget *parent = nullptr);
    ~MovingPointsWidget();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void updatePoints();

private:
    QVector<MovingPoint> m_points;
    QTimer *m_updateTimer;
    int m_maxPoints = 150;
    double m_pointSpeed = 90;
    int m_timerInterval = 15;

    bool m_mouseInWidget = false;
    QPointF m_mousePosition;
    double m_attractionRadius = 250.0;

    void generateRandomPoint();
    QPointF generateRandomDirection() const;
    QPointF calculateHexagonPoint(const QPointF& center, int index, double radius) const;
};

#endif
