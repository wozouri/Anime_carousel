#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    // 设置窗口大小
    resize(600, 600);

    // 初始化半径和角度步长
    outerRadius = 200.0;
    innerRadius = 200.0;
    angleStep = 15; // 每15度一个点

    animation = new QPropertyAnimation(this, "innerRadius");
    animation->setDuration(700); // 动画持续时间为5秒
    animation->setEasingCurve(QEasingCurve::InOutCirc); // 使用缓动曲线

    animation2 = new QPropertyAnimation(this, "angleStep");
    animation2->setDuration(700); // 动画持续时间为5秒
    animation2->setEasingCurve(QEasingCurve::InOutCirc); // 使用缓动曲线



    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this,[this]{
        count += 1;
        switch (count)
        {
        case 1:
            startangleStepAnimation(5,15);
            startAnimation(200,100);
            break;
        case 2:
            startAnimation(100,220);
            break;
        case 3:
            startAnimation(220,200);
            break;
        case 4:
            startangleStepAnimation(15,120);
            startAnimation(200,150);
            break;
        case 5:
            startangleStepAnimation(120,15);
            startAnimation(150,180);
            break;
        case 6:
            startangleStepAnimation(15,90);
            startAnimation(180,60);
            break;
        case 7:
            startAnimation(60,200);
            startangleStepAnimation(90,15);
            break;
        case 8:
            startAnimation(200,100);
            startangleStepAnimation(15,5);
            break;
        case 9:
            startAnimation(100,170);
            startangleStepAnimation(5,180);
            break;
        case 10:
            startAnimation(170,200);
            startangleStepAnimation(180,5);
            count = 0; // 重置计数器
            break;

        }




    }); // 每次定时器超时更新界面

    connect(animation, &QPropertyAnimation::valueChanged, this, [=]{
        timer->start(100); // 启动定时器，每100毫秒触发一次

    }); // 当动画值变化时更新界面

    timer->start(100); // 启动定时器，每100毫秒触发一次




}


Widget::~Widget() {}

void Widget::startAnimation(qreal Start, qreal End)
{
    timer->stop();


    // 设置动画的起始和结束值
    animation->setStartValue(Start);
    animation->setEndValue(End);
    // 启动动画
    animation->start();






}

void Widget::startangleStepAnimation(qreal Start, qreal End)
{
    // 设置动画的起始和结束值
    animation2->setStartValue(Start);
    animation2->setEndValue(End);
    // 启动动画
    animation2->start();



}



void Widget::paintEvent(QPaintEvent *event)
{

    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿，使线条更平滑

    // 将原点移动到窗口中心
    painter.translate(width() / 2, height() / 2);

    QPen pen(QColor(115,236,206), 2); // 设置画笔颜色为蓝色，线宽为2
    painter.setPen(pen);

    painter.setBrush(Qt::NoBrush);

    QPolygonF polygon;

    // 计算多边形的顶点
    for (qreal angle = 0; angle < 360; angle += angleStep) {
        // 计算外半径上的点
        qreal xOuter = outerRadius * qCos(qDegreesToRadians(angle));
        qreal yOuter = outerRadius * qSin(qDegreesToRadians(angle));
        polygon << QPointF(xOuter, yOuter);

        // 计算内半径上的点
        // 注意：下一个点是内半径上的点，角度与外半径点相同
        // 这样可以确保内外半径点交替出现，形成星形或锯齿状
        qreal xInner = innerRadius * qCos(qDegreesToRadians(angle + angleStep / 2)); // 调整内半径点的角度，使其在内外点之间
        qreal yInner = innerRadius * qSin(qDegreesToRadians(angle + angleStep / 2));
        polygon << QPointF(xInner, yInner);
    }


    // 绘制多边形
    painter.drawPolygon(polygon);









}

qreal Widget::getOuterRadius() const
{
    return outerRadius;
}

void Widget::setOuterRadius(qreal newOuterRadius)
{
    outerRadius = newOuterRadius;
    update(); // 更新界面以反映新的外半径
}

qreal Widget::getInnerRadius() const
{
    return innerRadius;
}

void Widget::setInnerRadius(qreal newInnerRadius)
{
    innerRadius = newInnerRadius;
    update(); // 更新界面以反映新的内半径
}


qreal Widget::getAngleStep() const
{
    return angleStep;
}

void Widget::setAngleStep(qreal newAngleStep)
{
    angleStep = newAngleStep;
    update(); // 更新界面以反映新的角度步长
}
