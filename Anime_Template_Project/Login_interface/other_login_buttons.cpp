#include "other_login_buttons.h"

Other_login_buttons::Other_login_buttons(QString icon, QWidget* parent) : QPushButton{ parent }
{
	this->icon = QPixmap(icon);
	this->resize(54, 54);
    this->setCursor(Qt::PointingHandCursor);
}

void Other_login_buttons::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(0, 0, width(), height());

    // 裁剪路径
    QPainterPath path;
    path.addRoundedRect(0, 0, width(), height(), height() / 4, height() / 4);
    painter.setClipPath(path);
    painter.setPen(Qt::NoPen);

    QPen pen;
    pen.setWidth(8);
    pen.setColor(QColor(220, 220, 220, 255));
    painter.setPen(pen);
    painter.drawRoundedRect(0, 0, width(), height(), height() / 4, height() / 4);

    QRect rect1(0, 0, width() / 2, width() / 2);
    rect1.moveCenter(QPoint(width() / 2, height() / 2));
    painter.drawPixmap(rect1, icon);

}






