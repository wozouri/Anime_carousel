#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->resize(1200,500);

    pixel_transition = new Pixel_Transition(18,this);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(2, 150); 
    slider->setValue(18); 
    slider->resize(200, 30); 
    timerslider = new QSlider(Qt::Horizontal, this);
    timerslider->setRange(1, 200); 
    timerslider->setValue(40); 
    timerslider->resize(200, 30); 

    label1 = new QLabel("网格尺寸:", this);
    label1->resize(100,30);
    label1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label2 = new QLabel("动画持续时间:", this);
    label2->resize(100,30);
    label2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    label3 = new QLabel(this);
    label3->resize(40,30);
    label3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    label4 = new QLabel(this);
    label4->resize(40,30);
    label4->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    label5 = new QPushButton(this);
    label5->resize(100,30);


    label3->setText(QString::number(slider->value()));
    label4->setText(QString::number(float(timerslider->value()) / 100.00));

    colorButton = new QPushButton("选择颜色", this);
    colorButton->resize(100, 30); 
    QPalette palette = label5->palette();
    palette.setColor(QPalette::Button, Qt::white);  
    label5->setPalette(palette); 

    connect(colorButton, &QPushButton::clicked, this, &Widget::chooseColor);
    connect(slider, &QSlider::valueChanged, label3, [this](int value) {
        label3->setText(QString::number(value));
        pixel_transition->updatePixelCount(value); 
    });
    connect(timerslider, &QSlider::valueChanged, label4, [this](int value) {
        label4->setText(QString::number(float(value) / 100.00));
        pixel_transition->updateAnimationDuration(float(value) / 100.00); 
    });

}

Widget::~Widget()
{

}


void Widget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event); 
    int width = event->size().width();
    int height = event->size().height();


    pixel_transition->adjustDisplaySize(width, height -100);
    pixel_transition->move((width - pixel_transition->width()) / 2, 0);

    label1->move(width /2 - slider->width() - label1->width(), height - 100);
    slider->move(width /2 - slider->width(),height -100);
    label3->move(width /2,height -100);


    label2->move(width /2 - timerslider->width() - label2->width(), height - 65);
    timerslider->move(width /2 - slider->width(),height -65);
    label4->move(width /2,height -65);

    colorButton->move(width /2 -  slider->width(), height - 35); 
    label5->move(width /2 -  slider->width() - label1->width(),height -35);

}

void Widget::chooseColor()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "选择颜色");
    if (color.isValid())
    {
        QPalette palette = label5->palette();
        palette.setColor(QPalette::Button, color); 
        label5->setPalette(palette); 

        pixel_transition->setPixelColor(color); 
    }




}
