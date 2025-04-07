#ifndef HOLLOW_BUTTON_H
#define HOLLOW_BUTTON_H

#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QMouseEvent>

class Hollow_button : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QString center_text READ center_text WRITE setCenter_text  FINAL)
    Q_PROPERTY(int radius READ radius WRITE setRadius  FINAL)
    Q_PROPERTY(int opacity READ opacity WRITE setOpacity   FINAL)

public:
    explicit Hollow_button(QWidget* parent = nullptr);

    //动画执行还原枚举
    enum AnimationState {
        ANIMATION_STATE_EXECUTING, // 执行动画
        ANIMATION_STATE_RESET // 还原动画
    };


    void draw_disappearing_circle(); // 绘制扩散圆

    void draw_border(); // 绘制边框

    void draw_text(); //绘制居中文本


    QPoint mouse_coordinates;

    QPropertyAnimation* animation1;
    QPropertyAnimation* animation3;


    QString center_text() const;
    void setCenter_text(const QString &newCenter_text);

    int radius() const;
    void setRadius(int newRadius);

    int opacity() const;
    void setOpacity(int newOpacity);

    void reset_animation();  //重置动画参数

    void execute_animation();  //执行动画

    void animation_status(bool status); //设置动画状态

signals:
    //换页信号
    void page_changed(AnimationState status);



protected:
    //绘制
    void paintEvent(QPaintEvent* event);

    void mousePressEvent(QMouseEvent* event);
    //释放鼠标
    void mouseReleaseEvent(QMouseEvent* event);


private:
    QString m_center_text;
    int m_radius = 0;
    int m_opacity = 255;
    bool status = true;
};

#endif // HOLLOW_BUTTON_H
