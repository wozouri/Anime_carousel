#include "login_form.h"

Login_form::Login_form(QWidget *parent)
    : QWidget{parent}
{
    this->resize(477, 620);

    //账户
    username = new Input_box("://img/account.png", this);
    username->move(46, 161);
    username->setPlaceholderText("Username");
    username->setMaxLength(11); //设置最大字符数，设置只能为数字
    username->setValidator(new QIntValidator(0, 9999999999, this));

     //密码
    password = new Input_box("://img/password.png", this);
    password->move(46, 253);
    password->setPlaceholderText("Password");
    password->setEchoMode(QLineEdit::Password);
    password->setMaxLength(16);
    password->setValidator(new QRegularExpressionValidator(QRegularExpression("[a-zA-Z0-9]+$"), this));

     //登录按钮
    login_button = new Login_button(this);
    login_button->setCenter_text("Login");
    login_button->move(46, 371);


    other_login_buttons1 = new Other_login_buttons("://img/logo_google.png", this);
    other_login_buttons1->move(110, 480);

    other_login_buttons2 = new Other_login_buttons("://img/facebook.png", this);
    other_login_buttons2->move(178, 480);

    other_login_buttons3 = new Other_login_buttons("://img/github-fill.png", this);
    other_login_buttons3->move(250, 480);

    other_login_buttons4 = new Other_login_buttons("://img/instagram.png", this);
    other_login_buttons4->move(320, 480);


    this->animations();
    connect(login_button, &Login_button::execute_animation_signal, this, &Login_form::execute_animation); 


}

void Login_form::animations() //按钮弹性动画
{
    animation = new QPropertyAnimation(this->login_button, "geometry");
    animation->setDuration(250);
    animation->setStartValue(this->login_button->geometry());
    animation->setEndValue(QRect(this->login_button->pos().x() + zoom_rate, this->login_button->pos().y() + zoom_rate /2, login_button->width() - zoom_rate * 2, login_button->height() - zoom_rate));
    animation->setEasingCurve(QEasingCurve::Linear);

}

void Login_form::execute_animation(Login_button::AnimationState State) //执行动画
{
    if (State == Login_button::Execute) 
    {
        animation->setDirection(QAbstractAnimation::Forward);

        animation->start();
    }
    else if (State == Login_button::Restore) //恢复动画
    {
        animation->setDirection(QAbstractAnimation::Backward);

        animation->start();
    }
}




void Login_form::paintEvent(QPaintEvent* event)
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setViewport(0, 0, 477, 620);
    painter.setWindow(0, 0, 477, 620);


    this->crop_corner(); //绘制纯白矩形

    this->draw_text();
}


void Login_form::crop_corner() //绘制纯白矩形
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    // 绘制纯白矩形
    QBrush Brush(QColor(255, 255, 255, 255));
    painter.setBrush(Brush);
    painter.drawRect(0, 0, width(), height());

}

void Login_form::draw_text() //绘制文本
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QRect rect1(0, 0, 0, 0);
    QFont   font1;
    font1.setPointSize(30);
    font1.setBold(true);
    font1.setWordSpacing(1);
    painter.setFont(font1);

    QColor semiTransparent(0, 0, 0, 255);
    painter.setPen(semiTransparent);

    QRect actualRect = painter.boundingRect(rect1, Qt::AlignCenter, "Login");  //计算高度
    rect1.setHeight(actualRect.height());
    rect1.setWidth(actualRect.width());
    rect1.moveCenter(QPoint(width() / 2, height() / 6));
    painter.drawText(rect1, "Login");

}
