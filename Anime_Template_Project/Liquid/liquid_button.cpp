#include "liquid_button.h"

Liquid_Button::Liquid_Button(QWidget *parent)
    : QWidget{parent}
{
    this->setFixedSize(200, 70);
    m_animation = new QPropertyAnimation(this, "color1");
    m_animation->setDuration(400);
    m_animation->setStartValue(m_color1);
    m_animation->setEndValue(QColor(255, 0, 0));

    m_animation2 = new QPropertyAnimation(this, "textColor");
    m_animation2->setDuration(400);
    m_animation2->setStartValue(m_textColor);
    m_animation2->setEndValue(QColor(255, 255, 255));
}

void Liquid_Button::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 20, 20);
    painter.setClipPath(path);

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_color1);
    painter.drawRoundedRect(rect(), 20, 20);

    painter.setPen(m_textColor);
    QFont font;
    font.setPixelSize(this->height() / 3);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter, m_text);
}

bool Liquid_Button::event(QEvent* event)
{
    if (event->type() == QEvent::Enter) {
        m_animation->setDirection(QAbstractAnimation::Forward);
        m_animation->start();
        m_animation2->setDirection(QAbstractAnimation::Forward);
        m_animation2->start();
    } else if (event->type() == QEvent::Leave) {
        m_animation->setDirection(QAbstractAnimation::Backward);
        m_animation->start();
        m_animation2->setDirection(QAbstractAnimation::Backward);
        m_animation2->start();
    }
    return QWidget::event(event);
}

QColor Liquid_Button::color1() const
{
    return m_color1;
}

void Liquid_Button::setColor1(const QColor& color1)
{
    m_color1 = color1;
    update();
}

QColor Liquid_Button::textColor() const
{
    return m_textColor;
}

void Liquid_Button::setTextColor(const QColor& textColor)
{
    m_textColor = textColor;
    update();
}
