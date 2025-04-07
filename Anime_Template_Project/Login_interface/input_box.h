#ifndef INPUT_BOX_H
#define INPUT_BOX_H

#include <QLineEdit>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>




class Input_box : public QLineEdit
{
    Q_OBJECT
public:
    explicit Input_box(QString icon,QWidget* parent = nullptr);



//绘画
protected:
    void paintEvent(QPaintEvent *event) ;

public:
    void crop_corner(); //裁剪圆角
    void draw_ico_image(); //绘制居中图标

private:
    QPixmap icon; // 图标

};

#endif // INPUT_BOX_H
