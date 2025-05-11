#ifndef WIDGET_H
#define WIDGET_H


#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QLineEdit>


class Widget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int Gradient_Position READ Gradient_Position WRITE setGradient_Position)
    Q_PROPERTY(qreal Circle_Ratio READ Circle_Ratio WRITE setCircle_Ratio)
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QPropertyAnimation *animation;
    QPropertyAnimation* animation2;
    QPushButton *button;
    QLineEdit *lineEdit;

    void forward();
    void backward();

    int Gradient_Position() const;
    void setGradient_Position(const int&value);

    qreal Circle_Ratio() const;
    void setCircle_Ratio(const qreal&value);
protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    int m_Gradient_Position; 
    QColor Global_Color = QColor(255, 255, 255 , 255); 
    QString color_str;
    qreal m_Circle_Ratio = 0.0; 
    bool m_isForward = true; 
    
};
#endif // WIDGET_H
