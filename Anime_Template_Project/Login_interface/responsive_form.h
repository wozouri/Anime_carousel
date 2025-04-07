#ifndef RESPONSIVE_FORM_H
#define RESPONSIVE_FORM_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QResizeEvent>



#include "scroll_bar.h"
#include "transparent_transition_interface.h"
#include "registration_form.h"
#include "login_form.h"

class Responsive_form : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int animation_duration READ animation_duration WRITE setAnimation_duration   FINAL)


public:
    explicit Responsive_form(QWidget *parent = nullptr);


    Scroll_bar* scroll_bar; 
    Transparent_transition_interface *transparent_transition_interface;
    Transparent_transition_interface* transparent_transition_interface2;
    Registration_form* registration_form;
    Login_form* login_form;


    void build_animation(); //构建动画
    QPropertyAnimation* animation2;
    QPropertyAnimation* animation3;
    QPropertyAnimation* animation4;
    QPropertyAnimation* animation5;
    QPropertyAnimation* animation6;


    int animation_duration() const;
    void setAnimation_duration(int newAnimation_duration);



    void updateMask();//更新遮罩
    void createRoundPath(QPainterPath& path); //创建遮罩
    void crop_corner(); //裁剪圆角

protected:
    void paintEvent(QPaintEvent *event);


    void mousePressEvent(QMouseEvent* event); // 鼠标按下事件
    void mouseMoveEvent(QMouseEvent* event);  // 鼠标移动事件

    void resizeEvent(QResizeEvent* event);  //窗口大小变化时更新遮罩


public slots:

    void setRightShow(); //设置显示层级

    void execute_animation(Hollow_button::AnimationState status); //执行动画

    void onAnimation3Finished(); //第三层动画
    void onAnimation4Finished(); //第四层动画

private:
    QPoint m_dragStartPosition;    // 鼠标按下时的全局位置
    QPoint m_startWindowPosition;  // 窗口初始位置
    int currentSequence = 1; // 当前动画序列，默认为 1
    bool animation_execute_duration = false;
    bool animation_restore_duration = false;
    int m_animation_duration = 600;
};

#endif // RESPONSIVE_FORM_H
