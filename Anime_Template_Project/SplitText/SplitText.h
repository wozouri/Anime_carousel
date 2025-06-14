#pragma once
#include <QMainWindow>
#include "Single_Text.h"
//动态列表
#include <vector>
//按钮
#include <QPushButton>
//输入框
#include <QLineEdit>
#include <QResizeEvent>
//动画
#include <QPropertyAnimation>

class SplitText : public QMainWindow 
{
    Q_OBJECT

public:
    SplitText(QWidget* parent = nullptr);
    ~SplitText();


protected:
    void resizeEvent(QResizeEvent* event) override;

public slots:
    void add_single_text(QString text );

    void run_animation();


private:


    std::vector<Single_Text*> single_text_list;

    
    QPushButton* run_animation_button;
    QLineEdit* input_text_edit;
    int start_y;
    int end_y;






};