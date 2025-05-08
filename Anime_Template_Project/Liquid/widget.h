#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include "image_processing.h"
#include <QGraphicsScene>
#include <QTimer>
#include <QGraphicsBlurEffect>
#include <QApplication>
#include <QRandomGenerator>
#include <QVector>
#include "liquid_button.h"

struct Circle {
    QPointF position;
    QColor color = QColor(255, 0, 0, 255);
    qreal alpha;
    QPointF direction;
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QPropertyAnimation *animation;
    QPropertyAnimation *animation2;

    QPoint startPoint = QPoint(295, 200);
    QPoint endPoint = QPoint(400, 200);

    void process(QImage &image, int alphaMultiplier);
    void processImageTransparency(QImage &image, int alphaMultiplier, int threshold);

private slots:
    void updateCircles();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QTimer *timer;
    QVector<Circle> circles;
    Liquid_Button *button;
};

#endif // WIDGET_H
