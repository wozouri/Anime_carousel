#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMetaType> // 引入 QMetaType
#include <QPainter>
#include <QPolygonF>
#include <QtMath> // For qDegreesToRadians
#include <QPropertyAnimation>
//动画组
#include <QParallelAnimationGroup>
//定时器
#include <QTimer>



class Widget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal outerRadius READ getOuterRadius WRITE setOuterRadius)
    Q_PROPERTY(qreal innerRadius READ getInnerRadius WRITE setInnerRadius)
    Q_PROPERTY(qreal angleStep READ getAngleStep WRITE setAngleStep)

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();


    qreal getOuterRadius() const;
    void setOuterRadius(qreal newOuterRadius);

    qreal getInnerRadius() const;
    void setInnerRadius(qreal newInnerRadius);

    qreal getAngleStep() const;
    void setAngleStep(qreal newAngleStep);


public:
    void startAnimation(qreal Start, qreal End);
    void startangleStepAnimation(qreal Start, qreal End);
    //计算边数量



protected:
    void paintEvent(QPaintEvent *event) override;




private:
    qreal outerRadius;
    qreal innerRadius;
    qreal angleStep; // 角度步长

    QTimer *timer; // 定时器
    // 计数
    int count = 0;;
    QPropertyAnimation *animation; // 属性动画
    QPropertyAnimation *animation2; // 属性动画




};
#endif // WIDGET_H
