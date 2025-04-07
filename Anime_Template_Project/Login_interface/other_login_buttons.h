#ifndef OTHER_LOGIN_BUTTONS_H
#define OTHER_LOGIN_BUTTONS_H

#include <QPushButton>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>



class Other_login_buttons : public QPushButton //小图标
{
    Q_OBJECT
public:
    explicit Other_login_buttons(QString icon, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);



private:
    QPixmap icon; // 图标


};

#endif // OTHER_LOGIN_BUTTONS_H
