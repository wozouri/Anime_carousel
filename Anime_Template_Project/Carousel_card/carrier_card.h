#ifndef CARRIER_CARD_H
#define CARRIER_CARD_H

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation> //动画类
#include <QMouseEvent>
#include <QPainterPath>
#include <QTimer>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <qlabel.h>


#include "card_text.h"
#include "Card_button.h"

class Carrier_card : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int carrier_card_id READ carrier_card_id WRITE setcarrier_card_id NOTIFY carrier_card_idChanged FINAL)
    Q_PROPERTY(int opacity READ opacity WRITE setOpacity  FINAL)
    Q_PROPERTY(int circle_degrees READ circle_degrees WRITE setCircle_degrees FINAL)


public:
    explicit Carrier_card(QPixmap m_carrier_card, QString main_title, QString sub_title, QWidget *parent = nullptr);


protected:
    void paintEvent(QPaintEvent *event);

    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    bool event(QEvent* e);  


    
public:
    QTimer m_clickTimer;
    QTimer timer4s;
    int TimerTimeout = 0;
    bool display_circle = false;
    void restore_disc();


    void onClickTimerTimeout();
    void circle_reset();
    //void restore_circle();
    QPoint mousePos; // 鼠标位置
    bool isClicked = false; // 鼠标是否按下
    void draw_radial_gradient_circle();

public://字体相关

    Card_text* Anime_small_text;
    int position_small_after_enlargement = 267;
    int basic_small_position = 245;



    void font_related_construction(QString main_title, QString sub_title);


    //按钮
    Card_button* Anime_button;

public:
    int carrier_card_id() const;  //获取卡片id
    
    //颜色一和二
    QColor m_color1;
    QColor m_color2;
    QColor m_color3;

    int opacity() const;
    void setOpacity(int newOpacity);

    int circle_degrees() const;
    void setCircle_degrees(int newCircle_degrees);

    void Anime_card_1();                             //第一卡片视图
    void setcarrier_card_id(int newCarrier_card_id); //设置卡片id

    QColor getMostFrequentColor(const QPixmap& pixmap);

    void set_gradient_color();

    void draw_sector_circle();

public slots:
    void Anime_card_transformation(int animation_duration);                //卡片变化

signals:
    void carrier_card_idChanged(int m_carrier_card_id);


private:
    QGraphicsDropShadowEffect* shadow;


    QPixmap m_carrier_card; //卡片图片
    int m_carrier_card_id = 0; //卡片id

    int m_opacity = 90;
    int m_circle_degrees = 360;
};

#endif // CARRIER_CARD_H













