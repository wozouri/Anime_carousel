#include "responsive_form.h"

Responsive_form::Responsive_form(QWidget *parent)
    : QWidget{parent}
{
    this->setFixedSize(955, 620);      //固定大小 
    this->setWindowFlags(Qt::FramelessWindowHint);  //取消标题

    //右点击
    transparent_transition_interface2 = new Transparent_transition_interface("Welcome Back!", "Already have an account?", "Login", this);
    transparent_transition_interface2->button->animation_status(false);
    transparent_transition_interface2->move(this->width(), 0);

    //注册表单
    registration_form = new Registration_form(this);
    registration_form->move(width(), 0);


    //登录表单
    login_form = new Login_form(this);
    login_form->move(this->width() / 2, 0);


    //蓝布
    scroll_bar = new Scroll_bar(this);
    scroll_bar->move( -width() * 1.5, 0);

    //左点击
    transparent_transition_interface = new Transparent_transition_interface("Hello, Welcome!", "Don't have an account?", "Register", this);
    transparent_transition_interface->move(0, 0);
    

 
    this->setRightShow(); //设置显示层级
    this->build_animation(); //构建动画


    connect(transparent_transition_interface->button, &Hollow_button::page_changed, this, &Responsive_form::execute_animation); //左切换
    connect(transparent_transition_interface2->button, &Hollow_button::page_changed, this, &Responsive_form::execute_animation); //右切换

}



void Responsive_form::setRightShow()  //设置显示层级
{
    transparent_transition_interface2->raise(); 
    registration_form->lower();

    transparent_transition_interface->raise();
    login_form->lower();

}


void Responsive_form::build_animation() //构建动画
{
    animation2 = new QPropertyAnimation(this->scroll_bar, "pos");  //3
    animation2->setDuration(m_animation_duration);
    animation2->setStartValue(this->scroll_bar->pos());
    animation2->setEndValue(QPoint(this->width() / 2, 0));
    animation2->setEasingCurve(QEasingCurve::Linear);

    animation3 = new QPropertyAnimation(this->transparent_transition_interface, "pos"); //2
    animation3->setDuration(m_animation_duration);
    animation3->setStartValue(this->transparent_transition_interface->pos());
    animation3->setEndValue(QPoint(-this->width() / 2, 0));
    animation3->setEasingCurve(QEasingCurve::Linear);

    animation4 = new QPropertyAnimation(this->transparent_transition_interface2, "pos"); //4
    animation4->setDuration(m_animation_duration);
    animation4->setStartValue(this->transparent_transition_interface2->pos());
    animation4->setEndValue(QPoint(this->width() / 2, 0));
    animation4->setEasingCurve(QEasingCurve::Linear);

    animation5 = new QPropertyAnimation(this->registration_form, "pos"); //5
    animation5->setDuration(m_animation_duration);
    animation5->setStartValue(this->registration_form->pos());
    animation5->setEndValue(QPoint(0, 0));
    animation5->setEasingCurve(QEasingCurve::Linear);

    animation6 = new QPropertyAnimation(this->login_form, "pos"); //6
    animation6->setDuration(m_animation_duration);
    animation6->setStartValue(this->login_form->pos());
    animation6->setEndValue(QPoint(-this->width() / 2, 0));
    animation6->setEasingCurve(QEasingCurve::Linear);

    connect(animation2, &QPropertyAnimation::valueChanged, this, [this] {
        if (scroll_bar->pos().x() > -this->width() / 2 && animation4->state() == !QAbstractAnimation::Running && animation_execute_duration == true)
        { //暂停animation2，执行animation3
            animation2->pause();

            animation3->setDirection(QAbstractAnimation::Forward);
            animation3->start();

            animation_execute_duration = false;

        }
        else if (scroll_bar->pos().x() < -this->width() / 10 && animation3->state() == !QAbstractAnimation::Running && animation_restore_duration == true)
        {//暂停animation2，执行animation4
            animation2->pause();

            animation4->setDirection(QAbstractAnimation::Backward);
            animation4->start();

            animation_restore_duration = false;
        }
        });

    connect(animation3, &QPropertyAnimation::finished, this, [this] { //启动animation2
        animation2->resume(); 
        });

    connect(animation4, &QPropertyAnimation::finished, this, [this] { //启动animation2
        animation2->resume();
        });

    connect(animation3, &QPropertyAnimation::finished, this, &Responsive_form::onAnimation3Finished); //连接启动动画。避免反复触发
    connect(animation4, &QPropertyAnimation::finished, this, &Responsive_form::onAnimation4Finished); //连接还原动画。避免反复触发

}
void Responsive_form::onAnimation3Finished() //animation3 的动画
{
    if (currentSequence == 1)
    {
        animation4->setDirection(QAbstractAnimation::Forward);
        animation4->start();

        animation5->setDirection(QAbstractAnimation::Forward);
        animation5->start();
    }
    else if (currentSequence == 2);
}


void Responsive_form::onAnimation4Finished() //animation4 的动画
{
    if (currentSequence == 1) return;

    else if (currentSequence == 2)
    {
        animation3->setDirection(QAbstractAnimation::Backward);
        animation3->start();

        animation6->setDirection(QAbstractAnimation::Backward);
        animation6->start();
    }
}


void Responsive_form::execute_animation(Hollow_button::AnimationState status) //执行动画
{
    if (status == Hollow_button::AnimationState::ANIMATION_STATE_EXECUTING)
    { //启动动画
        animation_execute_duration = true;
        currentSequence = 1;

        animation2->setDirection(QAbstractAnimation::Forward);
        animation2 ->start();

        animation6->setDirection(QAbstractAnimation::Forward);
        animation6->start();

    }
    else if (status == Hollow_button::AnimationState::ANIMATION_STATE_RESET)
    { //还原动画
        animation_restore_duration = true;
        currentSequence = 2;

        animation2->setDirection(QAbstractAnimation::Backward);
        animation2->start();
    }
}

void Responsive_form::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(0, 0, width(), height());

    this->crop_corner(); // 裁剪圆角


}

void Responsive_form::crop_corner() // 裁剪圆角
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 裁剪路径
    QPainterPath path;
    path.addRoundedRect(0, 0, width(), height(), 34, 34);
    painter.setClipPath(path);
    painter.setPen(Qt::NoPen);

    // 绘制纯白矩形
    QBrush Brush(QColor(255, 255, 255, 255));
    painter.setBrush(Brush);
    painter.drawRect(0, 0, width(), height());
}




void Responsive_form::resizeEvent(QResizeEvent* event) //窗口大小变化时更新遮罩
{
    QWidget::resizeEvent(event);
    updateMask(); // 窗口大小变化时更新遮罩
}
void Responsive_form::updateMask() //更新遮罩
{
    QPainterPath path;
    createRoundPath(path);
    QRegion region = QRegion(path.toFillPolygon().toPolygon());
    setMask(region); // 设置遮罩区域
}
void Responsive_form::createRoundPath(QPainterPath& path) //创建遮罩
{
    path.addRoundedRect(0, 0, width(), height(), 35, 35);
    path.closeSubpath(); // 闭合路径
}




void Responsive_form::mousePressEvent(QMouseEvent* event) //鼠标按下事件
{
    if (event->button() == Qt::LeftButton) {
        // 记录鼠标按下时的全局位置和窗口当前位置
        m_dragStartPosition = event->globalPos();
        m_startWindowPosition = this->pos();
        event->accept(); // 接受事件，阻止进一步传递
    }
    else {
        QWidget::mousePressEvent(event); // 其他按钮事件交给基类处理
    }
}

void Responsive_form::mouseMoveEvent(QMouseEvent* event) //鼠标移动事件
{
    if (event->buttons() & Qt::LeftButton) {
        // 计算鼠标移动的偏移量，并更新窗口位置
        QPoint delta = event->globalPos() - m_dragStartPosition;
        this->move(m_startWindowPosition + delta);
        event->accept(); // 接受事件
    }
    else {
        QWidget::mouseMoveEvent(event); // 非左键移动事件交给基类处理
    }
}



int Responsive_form::animation_duration() const
{
    return m_animation_duration;
}

void Responsive_form::setAnimation_duration(int newAnimation_duration)
{
    m_animation_duration = newAnimation_duration;
}
