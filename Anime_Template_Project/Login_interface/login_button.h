#ifndef LOGIN_BUTTON_H
#define LOGIN_BUTTON_H

#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QPropertyAnimation>


class Login_button : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int color_opacity READ color_opacity WRITE setColor_opacity  FINAL) //色彩透明度
    Q_PROPERTY(QString center_text READ center_text WRITE setCenter_text  FINAL) //居中的文本




public:
    explicit Login_button(QWidget* parent = nullptr);

    QPropertyAnimation *animation;
    // 定义枚举类型，表示动画状态
    enum AnimationState {
        Execute,  // 执行动画
        Restore   // 还原动画
    };

    void draw_text(); //绘制居中文本

    void start_animation(); //构建动画
    int color_opacity() const;
    void setColor_opacity(int newColor_opacity);

    QString center_text() const;
    void setCenter_text(const QString &newCenter_text);

protected:
    void paintEvent(QPaintEvent *event);

    bool event(QEvent* e);  //重新实现event()函数
    
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);


signals:
    void execute_animation_signal(Login_button::AnimationState state); // 发送动画状态信号



private:
    int m_color_opacity = 255;
    QString m_center_text;
};

#endif // LOGIN_BUTTON_H
