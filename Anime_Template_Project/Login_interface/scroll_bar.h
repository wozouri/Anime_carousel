#ifndef SCROLL_BAR_H
#define SCROLL_BAR_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>


class Scroll_bar : public QWidget //蓝布
{
    Q_OBJECT
public:
    explicit Scroll_bar(QWidget *parent = nullptr);

  
protected:
    void paintEvent(QPaintEvent *event);

public:
    void crop_corner(); //裁剪圆角

};

#endif // SCROLL_BAR_H
