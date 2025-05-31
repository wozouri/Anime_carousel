#ifndef PIXEL_TRANSITION_H
#define PIXEL_TRANSITION_H

#include <QWidget>
#include <QList>
#include <QRandomGenerator>
#include <QPixmap>
#include <QPainter>
#include <QEvent>
#include <QPropertyAnimation>
#include <QPainterPath>


class Pixel_Transition : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int Display_Progress READ Display_Progress WRITE setDisplay_Progress   FINAL)

public:
    explicit Pixel_Transition(int count, QWidget *parent = nullptr);

    int Display_Progress() const;
    void setDisplay_Progress(int newDisplay_Progress);
    void adjustDisplaySize(int width, int height);
    void startAnimation();
    void resetAnimation();
    void setPixelColor(const QColor &color) { this->PixelColor = color; }
public slots:
    void updatePixelCount(int count);
    void updateAnimationDuration(float duration);
protected:
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QList<int> White_pixel; 
    int count; 
    QColor PixelColor = QColor(255,255,255);
    qreal PixelSize; 
    qreal AnimationTimeRatio = 0.4; 
    bool Display_Background = true; 
    QPixmap PixelPixmap;
    int m_Display_Progress = 0; 
};

#endif // PIXEL_TRANSITION_H
