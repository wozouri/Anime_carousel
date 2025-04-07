#include "input_box.h"

Input_box::Input_box(QString icon, QWidget* parent) : QLineEdit{ parent }
{
    this->icon = QPixmap(icon);

	this->resize(388, 58);
    setTextMargins(25, 0, 25, 0); // 设置文本与边框的内间距（左、上、右、下）

    QFont font = this->font();
    font.setPointSize(15);
    setFont(font);

    QPalette palette = this->palette();
    palette.setColor(QPalette::PlaceholderText, Qt::gray);     // 设置 输入框 占位文本颜色为灰色
    this->setPalette(palette);

    palette.setColor(QPalette::Text, Qt::black);     // 设置输入文本颜色为黑
    this->setPalette(palette);

    palette.setColor(QPalette::Base, Qt::transparent); // 设置 输入框。背景色为透明
    this->setPalette(palette);


    this->setFrame(QFrame::NoFrame); // 不显示框架

}


void Input_box::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(0, 0, width(), height());

    crop_corner(); //裁剪圆角
    draw_ico_image(); //绘制居中图标


    QLineEdit::paintEvent(event);

}




void Input_box::crop_corner() //裁剪圆角
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // 裁剪路径
    QPainterPath path;
    path.addRoundedRect(0, 0, width(), height(), 10, 10);
    painter.setClipPath(path);
    painter.setPen(Qt::NoPen);
    // 绘制纯白矩形
    QBrush Brush(QColor(238, 238, 238, 255));
    painter.setBrush(Brush);
    painter.drawRect(0, 0, width(), height());
}


void Input_box::draw_ico_image() //绘制居中图标
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect   rect(width()*0.85, height() / 4, height() / 2, height() / 2);
    painter.drawPixmap(rect, icon);

}
