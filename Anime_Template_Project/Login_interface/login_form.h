#ifndef LOGIN_FORM_H
#define LOGIN_FORM_H

#include <QWidget>
#include <QPainter>
#include <QIntValidator>
#include <QRegularExpressionValidator>
#include <QPropertyAnimation>

#include "input_box.h"
#include "login_button.h"
#include "other_login_buttons.h"


class Login_form : public QWidget
{
    Q_OBJECT
public:
    explicit Login_form(QWidget *parent = nullptr);

    void crop_corner();//绘制纯白矩形
     
    void draw_text(); //绘制文本

     
    void animations(); //动画

    QPropertyAnimation* animation;
    int zoom_rate = 20;
public slots:
    void execute_animation(Login_button::AnimationState State);
    //绘制
protected:
    void paintEvent(QPaintEvent *event);

public:
    Input_box* username;
    Input_box* password;

    Login_button* login_button;

    Other_login_buttons* other_login_buttons1;
    Other_login_buttons* other_login_buttons2;
    Other_login_buttons* other_login_buttons3;
    Other_login_buttons* other_login_buttons4;
};


#endif // LOGIN_FORM_H
