#include "knob_page.h"

Knob_page::Knob_page(QWidget *parent)
    : QWidget{parent}
{
    QPalette pal(this->palette());
    pal.setColor(QPalette::Window, QColor(243, 246, 253));
    this->setPalette(pal);
    this->resize(1000, 1000);
    temp_dial = new Temperature_dial(this);
    temp_dial->show();
    QPoint Point = this->rect().center();
    temp_dial->move(Point.x() - temp_dial->width() / 2, Point.y() - temp_dial->height() / 2);
    this->animations();
}

void Knob_page::animations()
{
    animation = new Timer_animation(this->temp_dial, "geometry");
    animation->setDuration(400);
    animation->setStartValue(this->temp_dial->geometry());
    animation->setEndValue(QRect(this->temp_dial->pos().x() - zoom_rate / 2,
        this->temp_dial->pos().y() - zoom_rate / 2,
        temp_dial->width() + zoom_rate,
        temp_dial->height() + zoom_rate));
    connect(temp_dial, &Temperature_dial::execute_animation_signal, this, &Knob_page::execute_animation);
}

void Knob_page::execute_animation(Temperature_dial::AnimationState State)
{
    if (State == Temperature_dial::Execute)
    {
        animation->setDirection(Timer_animation::Forward);
        animation->start();
    }
    else if (State == Temperature_dial::Restore)
    {
        animation->setDirection(Timer_animation::Backward);
        animation->start();
    }
}
