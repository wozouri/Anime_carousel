#include "Single_Text.h"

Single_Text::Single_Text(QString text,QWidget *parent)
    : QWidget(parent)
{
    this->setFixedSize(160, 160);

    this->m_text = text;
    QPalette palette = this->palette();
    palette.setColor(QPalette::Base, QColor(5, 0, 15));
    this->setPalette(palette);

    centerX = this->width() / 2.0;
    centerY = this->height() / 3.0;

    m_color_animation = new QPropertyAnimation(this, "color_progress");
    m_color_animation->setDuration(200);
    m_color_animation->setStartValue(0);
    m_color_animation->setEndValue(255);

    m_rotate_animation = new QPropertyAnimation(this, "rotate_degree");
    m_rotate_animation->setDuration(400);
    m_rotate_animation->setStartValue(0);
    m_rotate_animation->setEndValue(360);
    m_rotate_animation->setEasingCurve(QEasingCurve::OutBack);
    connect(m_rotate_animation, &QPropertyAnimation::finished, this, &Single_Text::rotate_finished);



}

void Single_Text::start_color_animation()
{
    m_color_animation->setDirection(QAbstractAnimation::Forward);
    m_color_animation->start();


}

void Single_Text::reset_color_animation()
{
    m_color_animation->setDirection(QAbstractAnimation::Backward);
    m_color_animation->start();
}

void Single_Text::start_rotate_animation()
{
    m_rotate_animation->setDirection(QAbstractAnimation::Forward);
    m_rotate_animation->start();



}

void Single_Text::reset_rotate_animation()
{
    m_rotate_animation->setDirection(QAbstractAnimation::Backward);
    m_rotate_animation->start();
}











void Single_Text::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);


    QConicalGradient gradient(0, 0, this->pos().x() * 1.5);
    gradient.setColorAt(0, QColor( 255, 167, 69, 255));
    gradient.setColorAt(0.25, QColor(254, 134, 159, 255));
    gradient.setColorAt(0.5, QColor(239, 122, 200, 255));
    gradient.setColorAt(0.75, QColor(160, 131, 237, 255));
    gradient.setColorAt(1.0, QColor(67, 174, 255, 255));
    painter.setBrush(gradient);





    QFont font;
    font.setPixelSize(this->height() / 2);
    font.setFamily("Arial");
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QPen(QBrush(gradient), 2));

    QTransform transform;
    transform.translate(centerX, centerY);
    transform.rotate(m_rotate_degree);
    transform.translate(-centerX, -centerY);
    painter.setTransform(transform);

    painter.drawText(0, 0, this->width() / 2, this->height() / 2,  Qt::AlignCenter, m_text);




}













int  Single_Text::get_color_progress() const
{
    return m_color_progress;
}
void  Single_Text::set_color_progress(int value)
{
    m_color_progress = value;
    update();
}
int  Single_Text::get_rotate_degree() const
{
    return m_rotate_degree;
}
void  Single_Text::set_rotate_degree(int value)
{
    m_rotate_degree = value;
    update();   
}
