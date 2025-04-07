#include "login_button.h"

Login_button::Login_button(QWidget* parent) : QPushButton{ parent }
{


	this->resize(392, 57);
    this->setCursor(Qt::PointingHandCursor);

    setMouseTracking(true); // 启用持续鼠标跟踪（无需按下按钮）

    this->start_animation();
}

void Login_button::start_animation() //构建动画
{
    animation = new QPropertyAnimation(this, "color_opacity");
    animation->setDuration(200); // 设置动画时长为200毫秒
    animation->setStartValue(this->color_opacity()); // 设置动画起始值为255
    animation->setEndValue(188); // 设置动画结束值为100
    animation->setEasingCurve(QEasingCurve::Linear);
}


void Login_button::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(0, 0, width(), height());

    // 裁剪路径
    QPainterPath path;
    path.addRoundedRect(0, 0, width(), height(), 10, 10);
    painter.setClipPath(path);
    painter.setPen(Qt::NoPen);

    // 绘制背景
    QBrush Brush(QColor(123, 150, 228, m_color_opacity));
    painter.setBrush(Brush);
    painter.drawRect(0, 0, width(), height());

    this->draw_text();

}


void Login_button::draw_text() //绘制居中文本
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QRect rect1(0, 0, 0, 0);
    QFont   font1;
    font1.setPointSize(13);
    font1.setBold(true);
    font1.setWordSpacing(1);
    painter.setFont(font1);

    QColor semiTransparent(255, 255, 255, 255);
    painter.setPen(semiTransparent);

    QRect actualRect = painter.boundingRect(rect1, Qt::AlignCenter, m_center_text);  //计算高度
    rect1.setHeight(actualRect.height());
    rect1.setWidth(actualRect.width());
    rect1.moveCenter(QPoint(width() / 2, height() / 2)); //居中
    painter.drawText(rect1, m_center_text);
}



bool Login_button::event(QEvent* e)
{
    if (e->type() == QEvent::Enter)
    {
        animation->setDirection(QPropertyAnimation::Forward); // 设置动画方向为向前
        animation->start(); // 启动动画
        qDebug() << "鼠标进入";
        qDebug() << this->pos();
        qDebug() << this->geometry();
    }
    else if (e->type() == QEvent::Leave)
    {
        animation->setDirection(QPropertyAnimation::Backward); // 设置动画方向为向后
        animation->start(); // 启动动画
        qDebug() << "鼠标离开";
        update();
    }

    return QPushButton::event(e);
}



void Login_button::mousePressEvent(QMouseEvent* event)
{
    // 按下鼠标左键发射动画启动信号
    if (event->button() == Qt::LeftButton) emit execute_animation_signal(AnimationState::Execute);

    QWidget::mousePressEvent(event);

}
void Login_button::mouseReleaseEvent(QMouseEvent* event)
{
    // 释放鼠标左键发射动画恢复信号
    if (event->button() == Qt::LeftButton) emit execute_animation_signal(AnimationState::Restore);

    QWidget::mouseReleaseEvent(event);
}




















int Login_button::color_opacity() const
{
    return m_color_opacity;
}

void Login_button::setColor_opacity(int newColor_opacity)
{
    m_color_opacity = newColor_opacity;
    update();
}

QString Login_button::center_text() const
{
    return m_center_text;
}

void Login_button::setCenter_text(const QString &newCenter_text)
{
    m_center_text = newCenter_text;
}
