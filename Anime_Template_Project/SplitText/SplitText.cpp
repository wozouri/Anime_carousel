#include "SplitText.h"
#include <qdebug.h>

SplitText::SplitText(QWidget* parent)
    : QMainWindow(parent)
{
    this->resize(700, 700);
    run_animation_button = new QPushButton(this);
    run_animation_button->resize(100, 30);
    run_animation_button->setText("Run Animation");


    input_text_edit = new QLineEdit(this);
    input_text_edit->resize(200, 30);
    connect(input_text_edit,&QLineEdit::textChanged,this,&SplitText::add_single_text);
    connect(run_animation_button, &QPushButton::clicked, this, &SplitText::run_animation);



    

}




SplitText::~SplitText()
{
}

void SplitText::resizeEvent(QResizeEvent* event)
{
    int mw = event->size().width() / 2;
    int mh = event->size().height();

    start_y = mh - 200;
    end_y = mh - 700;


    run_animation_button->move(mw - run_animation_button->width() * 2, mh - run_animation_button->height() - 10);
    input_text_edit->move(mw - input_text_edit->width() / 2, mh - input_text_edit->height() - 10);

    int start_x = (this->width() - single_text_list.size() * 80) / 2;


    for (int i = 0; i < single_text_list.size(); i++)
    {
        single_text_list[i]->move(start_x + i * 80, height() - 200);

    }

}


void SplitText::add_single_text(QString text)
{

    for (Single_Text* single_text : single_text_list)
    {
        delete single_text;
    }

    single_text_list.clear(); 

    int text_length = text.length() * 80;



    int start_x = (this->width() - text_length) / 2;

    for (int i = 0; i < text.length(); i++)
    {
        QString single_text = text.mid(i, 1);
        Single_Text* single_text_obj = new Single_Text(single_text, this);
        single_text_obj->move(start_x + i * 80, height() - 200);
        single_text_obj->show();
        single_text_list.push_back(single_text_obj);
        

    }

    
}


void SplitText::run_animation()
{
    for (int i = 0; i < single_text_list.size(); i++)
    {
        QPropertyAnimation* animation = new QPropertyAnimation(single_text_list[i], "pos");
        animation->setDuration(350 + i * 20);
        animation->setStartValue(QPoint(single_text_list[i]->pos().x(), start_y));
        animation->setEndValue(QPoint(single_text_list[i]->pos().x(), end_y));
        animation->setEasingCurve(QEasingCurve::InBack);
        animation->start(QAbstractAnimation::DeleteWhenStopped);

/////////////////////////
        connect(animation, &QPropertyAnimation::finished, this,[=]{
            single_text_list[i]->start_rotate_animation();

/////////////////////////////
                connect(single_text_list[i], &Single_Text::rotate_finished, this, [=]{
                    qDebug() << "动画结束";
                    QPropertyAnimation* animation2 = new QPropertyAnimation(single_text_list[i], "pos");
                    animation2->setDuration(350 + i * 20);
                    animation2->setStartValue(QPoint(single_text_list[i]->pos().x(), end_y));
                    animation2->setEndValue(QPoint(single_text_list[i]->pos().x(), start_y));
                    animation2->setEasingCurve(QEasingCurve::OutElastic);
                    animation2->start(QAbstractAnimation::DeleteWhenStopped);
            });
        });
 

    }




}