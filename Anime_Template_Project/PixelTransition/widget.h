#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter.h>
#include <QResizeEvent>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QColorDialog>
#include "pixel_transition.h"



class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void chooseColor();
private:
    Pixel_Transition* pixel_transition; 
    QSlider *slider; 
    QSlider *timerslider; 
    QLabel *label1;
    QLabel *label2;
    QLabel *label3;
    QLabel *label4;
    QPushButton *label5;
    QPushButton *colorButton;
};
#endif // WIDGET_H
