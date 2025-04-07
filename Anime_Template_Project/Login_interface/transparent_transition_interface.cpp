#include "transparent_transition_interface.h"

Transparent_transition_interface::Transparent_transition_interface(QString large_text, QString small_text, QString btn_text, QWidget *parent)
    : QWidget{parent}
{
    this->large_text = large_text;
    this->small_text = small_text;

    this->resize(477, 620);


    //居中按钮，  万能居中法
    button = new Hollow_button(this);
    button->setCenter_text(btn_text);
    QRect rect1(0, 0, 180, 55);
    rect1.moveCenter(QPoint(width() / 2, height() * 0.59)); //居中
    button->setGeometry(rect1);

}


void Transparent_transition_interface::paintEvent(QPaintEvent* event)
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setViewport(0, 0, 477, 620);
    painter.setWindow(0, 0, 477, 620);

    this->draw_text();
    this->draw_text2();

}


void Transparent_transition_interface::draw_text() // 万能居中法 绘制文本
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QRect rect1(0, 0, 0, 0);
    QFont   font1;
    font1.setPointSize(29);
    //黑体

    font1.setBold(true);
    font1.setWordSpacing(1);
    painter.setFont(font1);
    QColor semiTransparent(255, 255, 255, 255);
    painter.setPen(semiTransparent);

    QRect actualRect = painter.boundingRect(rect1, Qt::AlignCenter, large_text);  //计算高度
    rect1.setHeight(actualRect.height());
    rect1.setWidth(actualRect.width());
    rect1.moveCenter(QPoint(width() / 2, height() * 0.41));
    painter.drawText(rect1, large_text);


}

void Transparent_transition_interface::draw_text2()// 万能居中法 绘制文本
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QRect rect1(0, 0, 0, 0);
    QFont   font1;
    font1.setPointSize(11);
    //黑体

    font1.setBold(true);
    font1.setWordSpacing(1);
    painter.setFont(font1);
    QColor semiTransparent(255, 255, 255, 255);
    painter.setPen(semiTransparent);

    QRect actualRect = painter.boundingRect(rect1, Qt::AlignCenter, small_text);  //计算高度
    rect1.setHeight(actualRect.height());
    rect1.setWidth(actualRect.width());
    rect1.moveCenter(QPoint(width() / 2, height() * 0.49));
    painter.drawText(rect1, small_text);

}