#include "card_text.h"

Card_text::Card_text(QString text, QString text1, QWidget *parent)
    : QWidget{parent}
{
    this->text_main = text;
    this->text_sub = text1;
}


void Card_text::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect rect(0, 0, width(), height());
    painter.setViewport(rect);          //设置Viewport
    painter.setWindow(0, 0, m_shrink_width, height());  //设置窗口
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);


    QRect rect1(0, 0, m_shrink_width, 0);
    QFont   font1;
    font1.setPointSize(m_font_size);
    font1.setBold(true);
    painter.setFont(font1);
    QColor semiTransparent(255, 255, 255, 255);
    painter.setPen(semiTransparent);
    QRect actualRect = painter.boundingRect(rect1, Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignBottom, text_main);  //计算高度
    rect1.setHeight(actualRect.height());
    painter.drawText(rect1, text_main);


    QRect rect2(2, actualRect.height() + 7, width() - width()/4, 0);
    QFont   font;
    font.setPointSize(m_font_size2);
    font.setBold(true);
    painter.setFont(font);
    QColor semiTransparent1(255, 255, 255, m_enlarge_width);
    painter.setPen(semiTransparent1);
    QRect actualRect1 = painter.boundingRect(rect2, Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignBottom, text_sub);  //计算高度
    rect2.setHeight(actualRect1.height());
    painter.drawText(rect2, text_sub);


    this->resize(m_shrink_width, actualRect1.height() + actualRect.height() + 7);
    this->move(38, 480 - actualRect1.height() - actualRect.height() - m_top_margin);
}



//执行动画
void Card_text::start_animation(int animation_duration)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "shrink_width");
    animation->setDuration(animation_duration);
    animation->setStartValue(this->m_shrink_width);
    animation->setEndValue(280);
    animation->setEasingCurve(QEasingCurve::Linear);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* animation1 = new QPropertyAnimation(this, "enlarge_width");
    animation1->setDuration(animation_duration);
    animation1->setStartValue(this->m_enlarge_width);
    animation1->setEndValue(255);
    animation1->setEasingCurve(QEasingCurve::Linear);
    animation1->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* animation2 = new QPropertyAnimation(this, "font_size");
    animation2->setDuration(animation_duration);
    animation2->setStartValue(this->m_font_size);
    animation2->setEndValue(23);
    animation2->setEasingCurve(QEasingCurve::Linear);
    animation2->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* animation3 = new QPropertyAnimation(this, "font_size2");
    animation3->setDuration(animation_duration);
    animation3->setStartValue(this->m_font_size2);
    animation3->setEndValue(10);
    animation3->setEasingCurve(QEasingCurve::Linear);
    animation3->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* animation4 = new QPropertyAnimation(this, "top_margin");
    animation4->setDuration(animation_duration);
    animation4->setStartValue(this->m_top_margin);
    animation4->setEndValue(232);
    animation4->setEasingCurve(QEasingCurve::Linear);
    animation4->start(QAbstractAnimation::DeleteWhenStopped);


}
//还原动画
void Card_text::reset_animation(int animation_duration)
{

    QPropertyAnimation *animation = new QPropertyAnimation(this, "shrink_width");
    animation->setDuration(animation_duration);
    animation->setStartValue(this->m_shrink_width);
    animation->setEndValue(200);
    animation->setEasingCurve(QEasingCurve::Linear);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* animation1 = new QPropertyAnimation(this, "enlarge_width");
    animation1->setDuration(animation_duration);
    animation1->setStartValue(this->m_enlarge_width);
    animation1->setEndValue(0);
    animation1->setEasingCurve(QEasingCurve::Linear);
    animation1->start(QAbstractAnimation::DeleteWhenStopped);


    QPropertyAnimation* animation2 = new QPropertyAnimation(this, "font_size");
    animation2->setDuration(animation_duration);
    animation2->setStartValue(this->m_font_size);
    animation2->setEndValue(15);
    animation2->setEasingCurve(QEasingCurve::Linear);
    animation2->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* animation3 = new QPropertyAnimation(this, "font_size2");
    animation3->setDuration(animation_duration);
    animation3->setStartValue(this->m_font_size2);
    animation3->setEndValue(1);
    animation3->setEasingCurve(QEasingCurve::Linear);
    animation3->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* animation4 = new QPropertyAnimation(this, "top_margin");
    animation4->setDuration(animation_duration);
    animation4->setStartValue(this->m_top_margin);
    animation4->setEndValue(210);
    animation4->setEasingCurve(QEasingCurve::Linear);
    animation4->start(QAbstractAnimation::DeleteWhenStopped);

}

int Card_text::shrink_width() const
{
    return m_shrink_width;
}

void Card_text::setShrink_width(int newShrink_width)
{
    m_shrink_width = newShrink_width;
    update();
}

int Card_text::enlarge_width() const
{
    return m_enlarge_width;
}

void Card_text::setEnlarge_width(int newEnlarge_width)
{
    m_enlarge_width = newEnlarge_width;
    update();
}

int Card_text::font_size() const
{
    return m_font_size;
}

void Card_text::setFont_size(int newFont_size)
{
    m_font_size = newFont_size;
    update();
}

int Card_text::font_size2() const
{
    return m_font_size2;
}

void Card_text::setFont_size2(int newFont_size2)
{
    m_font_size2 = newFont_size2;
    update();
}

int Card_text::top_margin() const
{
    return m_top_margin;
}

void Card_text::setTop_margin(int newTop_margin)
{
    m_top_margin = newTop_margin;
    update();
}
