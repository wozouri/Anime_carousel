#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    resize(w,h);
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&Widget::Radius_Processing);
    timer->setTimerType(Qt::PreciseTimer);
    timer->start(3);

}

Widget::~Widget() {}

void Widget::addDot()
{
    this->w = width();
    this->h = height();

    m_dots.clear(); 
    qreal startX = w / 15;
    qreal startY = h / 15;;
    qreal spacing =  h / 14; 
    r = spacing;
    r2 = spacing;
    Wcenter = QPointF(w/2,w/2);

    int numRows = 13;
    int numCols = 13;

    int x = 4;
    int j = numRows;
    int i = 8;

    for (int row = 0; row < numRows; ++row)
    {
        for (int col = 0; col < numCols; ++col)
        {
            if (col < x) continue; 
            if (col > i) continue;
            m_dots.append({QPointF(startX + col * spacing, startY + row * spacing),qreal(3)});
        }

        if (row < numRows *0.3)
        {
            qDebug() << numRows *0.3;
            x -=1;
            i +=1;
        }
        else if (row > numRows *0.61)
        {
            qDebug() << numRows *0.61;

            x +=1;
            i -=1;

        }
    }

}

void Widget::Radius_Processing()
{
    for (DotData& Data : m_dots)
    {
        if (this->isPointInAnnulus(Data.point, Wcenter, r1, r2))
        {
            qDebug() << "存在: " << Data.point;
            Data.dotRadius += 0.75;
        }
        else
        {
            if (Data.dotRadius < 3) continue;
            Data.dotRadius -= 0.3;
        }
    }
    if ( r1>w && r2 >2)
    {
        r1 = 0;
        r2 = r;
    }
    else
    {
        r1 += 1.0;
        r2 += 1.0;
    }

    update();

}

void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event); 

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(33,33,33));
    painter.setPen(Qt::NoPen); 
    painter.setBrush(QBrush(QColor(146,252,193)));
    for (const DotData& Data : m_dots) {
        painter.drawEllipse(Data.point.x() - Data.dotRadius, Data.point.y() - Data.dotRadius, 2 * Data.dotRadius, 2 * Data.dotRadius);
    }
    painter.setPen(QColor(146,252,193));
    painter.setBrush(Qt::NoBrush); 
    painter.drawEllipse(Wcenter,r2,r2);
    painter.drawEllipse(Wcenter,r1,r1);
}
void Widget::resizeEvent(QResizeEvent *event)
{
    resize(event->size().width(),event->size().width());
    addDot();
}
bool Widget::isPointInAnnulus(const QPointF& testPoint, const QPointF& centerPoint, qreal innerRadius, qreal outerRadius)
{
    qreal dx = testPoint.x() - centerPoint.x();
    qreal dy = testPoint.y() - centerPoint.y();
    qreal distanceSquared = qPow(dx, 2) + qPow(dy, 2);
    qreal distance = qSqrt(distanceSquared);
    return (distance >= innerRadius && distance <= outerRadius);
}






 