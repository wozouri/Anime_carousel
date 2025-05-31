#include "pixel_transition.h"

Pixel_Transition::Pixel_Transition(int count,QWidget *parent)
    : QWidget{parent}
{
    PixelPixmap = QPixmap(":/new/Ggdkoima8AAFzsN.jpg"); 

    this->updatePixelCount(count);

}

void Pixel_Transition::paintEvent(QPaintEvent *event)
{
     Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform ); 

    int widgetWidth = width();
    int widgetHeight = height();
    int imageWidth = PixelPixmap.width();
    int imageHeight = PixelPixmap.height();

    double widthRatio = qreal(widgetWidth) / imageWidth;
    double heightRatio = qreal(widgetHeight) / imageHeight;
    double scale = std::max(widthRatio, heightRatio);

    int scaledWidth = int(imageWidth * scale);
    int scaledHeight = int(imageHeight * scale);

    int x = (widgetWidth - scaledWidth) / 2;
    // int y = (widgetHeight - scaledHeight) / 2;

    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, widgetWidth, widgetHeight), 10, 10); 
    painter.setClipPath(path); 

    // 绘制图片
    if (Display_Background) painter.drawPixmap(x, 0, PixelPixmap.scaled(scaledWidth, scaledHeight, Qt::KeepAspectRatio));

    //绘制第二背景
    if (!Display_Background)
    {
        painter.setBrush( QColor(17,17,17));

        painter.drawRect(0, 0, widgetWidth, widgetWidth); 

        QFont font;
        font.setFamily("微软雅黑");
        font.setPixelSize(widgetWidth / 5); 
        font.setBold(true);
        painter.setFont(font); 

        QLinearGradient gradient(widgetWidth * 0.3, widgetHeight * 0.3, widgetWidth * 0.6, widgetHeight * 0.6);
        gradient.setColorAt(0, QColor(87, 36, 60)); 
        gradient.setColorAt(0.5, QColor(53, 119, 177)); 
        gradient.setColorAt(1, QColor(228, 252, 255));
        painter.setPen(QPen(QBrush(gradient), 2));
        painter.drawText(QRectF(0, 0, widgetWidth, widgetHeight), Qt::AlignCenter, "星见雅"); 
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(PixelColor); 

    for (int x = 0; x < m_Display_Progress; x++)
    {
        int xpos = White_pixel[x] % count; 
        int ypos = White_pixel[x] / count; 

        painter.drawRect(QRectF(qreal(xpos) * PixelSize - 1, qreal(ypos) * PixelSize - 1, PixelSize + 1, PixelSize + 1)); 

    }

    QPen borderPen(Qt::white, 3); 
    painter.setPen(borderPen); 
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(QRectF(0, 0, widgetWidth, widgetHeight), 10, 10); 

}

bool Pixel_Transition::event(QEvent *event)
{
    if (event->type() == QEvent::Enter) this->startAnimation();
    else if (event->type() == QEvent::Leave) this->resetAnimation(); 

    return QWidget::event(event); 
}

void Pixel_Transition::resizeEvent(QResizeEvent *event)
{
    this->PixelSize = qreal(width()) / qreal(count); 
}

void Pixel_Transition::adjustDisplaySize(int width, int height)
{
    if (height > width) this->resize(width, width);
    else this->resize(height, height);

    update();
}


void Pixel_Transition::startAnimation()
{
    if (Display_Background)
    {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "Display_Progress");
        animation->setDuration(1000 *AnimationTimeRatio); 
        animation->setStartValue(0); 
        animation->setEndValue(count * count); 

        animation->start(QAbstractAnimation::DeleteWhenStopped); 

        connect(animation, &QPropertyAnimation::finished, this,[=]{
            this->Display_Background = false; 

            QPropertyAnimation* animation2 = new QPropertyAnimation(this, "Display_Progress");
            animation2->setDuration(1000 *AnimationTimeRatio); 
            animation2->setStartValue(m_Display_Progress); 
            animation2->setEndValue(0); 

            animation2->start(QAbstractAnimation::DeleteWhenStopped); 

            connect(animation2,&QPropertyAnimation::finished, this,[=]{
                this->updatePixelCount(count);
            });

        });
    }
    else
    {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "Display_Progress");
        animation->setDuration(1000 *AnimationTimeRatio); 
        animation->setStartValue(m_Display_Progress); 
        animation->setEndValue(0); 

        animation->start(QAbstractAnimation::DeleteWhenStopped); 
        connect(animation,&QPropertyAnimation::finished, this,[=]{
            this->updatePixelCount(count);
        });

    }

}


void Pixel_Transition::resetAnimation()
{

    if (Display_Background)
    {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "Display_Progress");
        animation->setDuration(1000 *AnimationTimeRatio); 
        animation->setStartValue(m_Display_Progress);  
        animation->setEndValue(0); 

        animation->start(QAbstractAnimation::DeleteWhenStopped); 
        connect(animation,&QPropertyAnimation::finished, this,[=]{
            this->updatePixelCount(count);
        });

    }
    else
    {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "Display_Progress");
        animation->setDuration(1000 *AnimationTimeRatio); 
        animation->setStartValue(m_Display_Progress); 
        animation->setEndValue(count * count); 

        animation->start(QAbstractAnimation::DeleteWhenStopped); 

        connect(animation, &QPropertyAnimation::finished, this,[=]{
            this->Display_Background = true; 

            QPropertyAnimation* animation2 = new QPropertyAnimation(this, "Display_Progress");
            animation2->setDuration(1000 *AnimationTimeRatio); 
            animation2->setStartValue(m_Display_Progress); 
            animation2->setEndValue(0);  
            animation2->start(QAbstractAnimation::DeleteWhenStopped);  

            connect(animation2,&QPropertyAnimation::finished, this,[=]{
                this->updatePixelCount(count);


            });


        }); 
    }

}


void Pixel_Transition::updatePixelCount(int count)
{
    White_pixel.clear();

    this->count = count; 
    this->PixelSize = qreal(width()) / qreal(count); 

    for (int var = 0; var < count * count; ++var) White_pixel.append(var);

    QRandomGenerator *rgb = QRandomGenerator::global();
    std::shuffle(White_pixel.begin(), White_pixel.end(), *rgb);

    qDebug() << "White_pixel size:" << White_pixel.size();

}

void Pixel_Transition::updateAnimationDuration(float duration)
{
    this->AnimationTimeRatio = duration; 

}


int Pixel_Transition::Display_Progress() const
{
    return m_Display_Progress;
}

void Pixel_Transition::setDisplay_Progress(int newDisplay_Progress)
{

    m_Display_Progress = newDisplay_Progress;
    qDebug() << "Display_Progress:" << m_Display_Progress;
    update();

}
