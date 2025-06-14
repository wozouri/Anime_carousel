#pragma once
#include <QWidget>
#include <QPropertyAnimation>
#include <QPainter>

class Single_Text : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int color_progress READ get_color_progress WRITE set_color_progress)
    Q_PROPERTY(int rotate_degree READ get_rotate_degree WRITE set_rotate_degree)

public:
    explicit Single_Text(QString text,QWidget *parent = nullptr);

    int get_color_progress() const;
    void set_color_progress(int value);
    int get_rotate_degree() const;
    void set_rotate_degree(int value);




    void start_color_animation();
    void reset_color_animation();
    void start_rotate_animation();
    void reset_rotate_animation();

Q_SIGNALS:
    void rotate_finished();




protected:
    void paintEvent(QPaintEvent *event) override;


private:
    QString m_text;
    QColor m_color = QColor(255, 255, 255, 255);
    int m_color_progress = 255;
    int m_rotate_degree = 0;

    QPropertyAnimation *m_color_animation = nullptr;
    QPropertyAnimation *m_rotate_animation = nullptr;


    qreal centerX  ;
    qreal centerY ;

};