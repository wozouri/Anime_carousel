#include "latticed_circle_button.h"

Latticed_Circle_Button::Latticed_Circle_Button(QWidget* parent) : QPushButton{ parent }
{
    this->resize(50, 50);

    this->setCursor(Qt::PointingHandCursor);


    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 13);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 100));
    this->setGraphicsEffect(shadow);

    animation1 = new QPropertyAnimation(this, "Tomeido");
    animation1->setDuration(300);
    animation1->setStartValue(m_Tomeido);
    animation1->setEndValue(255);
    animation1->setEasingCurve(QEasingCurve::Linear);

    animation2 = new QPropertyAnimation(this, "Current_Color");
    animation2->setDuration(300);
    animation2->setStartValue(m_Current_Color);
    animation2->setEndValue(QColor(255, 255, 255, 255));
    animation2->setEasingCurve(QEasingCurve::Linear);
}


void Latticed_Circle_Button::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), width());
    painter.setWindow(0, 0, 50, 50);

    QPen pen;
    pen.setWidth(2.00);
    pen.setColor(Qt::black);
    painter.setPen(pen);

    QBrush brush;
    brush.setColor(QColor(0, 0, 0, m_Tomeido));
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);

    painter.drawEllipse(3, 3, 44, 44 );

    QPen pen1;
    pen1.setWidth(4); 
    pen1.setColor(m_Current_Color); 
    pen1.setStyle(Qt::SolidLine); 
    pen1.setCapStyle(Qt::FlatCap); 
    pen1.setJoinStyle(Qt::MiterJoin); 
    painter.setPen(pen1);

    if (Shikisai_no_joutai == true)
    {
      QPoint points[] = {
        QPoint(28, 15),
        QPoint(18, 25),
        QPoint(28, 35),
            };
      painter.drawPolyline(points, 3);
    }
    else
    {
     QPoint points[] = {
      QPoint(23, 15),
      QPoint(33, 25),
      QPoint(23, 35),
            };
     painter.drawPolyline(points, 3);
    }

}
bool Latticed_Circle_Button::event(QEvent* e)
{
    if (e->type() == QEvent::Enter)
    {
        animation1->setDirection(QPropertyAnimation::Forward); 
        animation1->start(); 
        animation2->setDirection(QPropertyAnimation::Forward); 
        animation2->start(); 
        emit execute_animation_signal(AnimationState::Execute);
        update();
    }
    else if (e->type() == QEvent::Leave)
    {
        animation1->setDirection(QPropertyAnimation::Backward); 
        animation1->start(); 

        animation2->setDirection(QPropertyAnimation::Backward);
        animation2->start(); 
        emit execute_animation_signal(AnimationState::Restore);
        update();
    }
    return QPushButton::event(e);
}


void Latticed_Circle_Button::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) emit Latticed_Circle_Click_Signal();
}


int Latticed_Circle_Button::Tomeido() const
{
    return m_Tomeido;
}

void Latticed_Circle_Button::setTomeido(int newTomeido)
{
    if (m_Tomeido == newTomeido)
        return;
    m_Tomeido = newTomeido;
    update();

}

QColor Latticed_Circle_Button::Current_Color() const
{
    return m_Current_Color;
}

void Latticed_Circle_Button::setCurrent_Color(const QColor &newCurrent_Color)
{
    if (m_Current_Color == newCurrent_Color)
        return;
    m_Current_Color = newCurrent_Color;
    update();

}
