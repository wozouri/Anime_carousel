#include "wave_button.h"

Wave_button::Wave_button(QWidget* parent) : QPushButton{ parent }
{
    this->resize(147, 55);
    this->setCursor(Qt::PointingHandCursor);
    m_wave_position = sqrt(pow(width(), 2) + pow(width(), 2)) / 2;

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 16);
    shadow->setBlurRadius(50);
    shadow->setColor(QColor(0, 0, 0, 255));
    this->setGraphicsEffect(shadow);

}

double Wave_button::triangle_position()
{
    return sqrt(pow(width(), 2) + pow(width(), 2)) / 2;;
}


void Wave_button::rotating_rounded_rectangle()
{
    QPainter painter(this); 
    painter.setRenderHint(QPainter::Antialiasing, true);


    QPainterPath path;
    path.addRoundedRect(0, 0, width(), height(), 26, 26);
    painter.setClipPath(path);


    color = QColor(0, 0, 222, m_wave_transparency);


    QRect rect(0 - width() / 5 * 4, m_wave_position, width() * 2, width() * 2);
    painter.translate(rect.center());
    painter.rotate(m_angle);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(-rect.width() / 2, -rect.height() / 2, rect.width(), rect.height(), width() * 0.9, width() * 0.9);

}

void Wave_button::rotating_rounded_rectangle1()
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);


    QPainterPath path;
    path.addRoundedRect(0, 0, width(), height(), 26, 26);
    painter.setClipPath(path);

    color = QColor(0, 0, 222, m_wave_transparency);

    QRect rect1(0 - width() / 20, m_wave_position, width() * 2, width() * 2);
    painter.translate(rect1.center());
    painter.rotate(m_right_angle);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(-rect1.width() / 2, -rect1.height() / 2, rect1.width(), rect1.height(), width() * 0.9, width() * 0.9);

}


void Wave_button::draw_border()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(0, 0, width(), height(), 26, 26);
    painter.setClipPath(path);

    QPen pen;
    pen.setWidth(8);
    pen.setColor(QColor(0, 0, 255, 255));
    painter.setPen(pen);
    painter.drawRoundedRect(0, 0, width(), height(), 26, 26);

}


void Wave_button::draw_text()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(0, 0, width(), height());

    QRect rect1(0, 0, width(), height());
    QFont   font1;

    font1.setPointSize(13);
    font1.setLetterSpacing(QFont::AbsoluteSpacing, 2);

    font1.setBold(true);
    painter.setFont(font1);
    QColor semiTransparent(255, 255, 255, 255);
    painter.setPen(semiTransparent);
    painter.drawText(rect1, Qt::AlignCenter, QString("Click"));


}


void Wave_button::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(0, 0, width(), height());


    this->rotating_rounded_rectangle();

    this->rotating_rounded_rectangle1();

    this->draw_border(); 

    this->draw_text(); 

}


void Wave_button::execute_animation()
{
    QPropertyAnimation* animation1 = new QPropertyAnimation(this, "angle");
    animation1->setDuration(execution_time);
    animation1->setStartValue(this->m_angle);
    animation1->setEndValue(1080);
    animation1->setEasingCurve(QEasingCurve::Linear);
    animation1->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* animation4 = new QPropertyAnimation(this, "right_angle");
    animation4->setDuration(execution_time);
    animation4->setStartValue(this->m_right_angle);
    animation4->setEndValue(25);
    animation4->setEasingCurve(QEasingCurve::Linear);
    animation4->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* animation2 = new QPropertyAnimation(this, "wave_position");
    animation2->setDuration(execution_time);
    animation2->setStartValue(this->m_wave_position);
    animation2->setEndValue(-height());
    animation2->setEasingCurve(QEasingCurve::Linear);
    animation2->start(QAbstractAnimation::DeleteWhenStopped);


    QPropertyAnimation* animation3 = new QPropertyAnimation(this, "wave_transparency");
    animation3->setDuration(execution_time);
    animation3->setStartValue(this->m_wave_transparency);
    animation3->setEndValue(255);
    animation3->setEasingCurve(QEasingCurve::Linear);
    animation3->start(QAbstractAnimation::DeleteWhenStopped);
}

void Wave_button::restore_animation()
{
    QPropertyAnimation* animation1 = new QPropertyAnimation(this, "angle");
    animation1->setDuration(execution_time);
    animation1->setStartValue(this->m_angle);
    animation1->setEndValue(25);
    animation1->setEasingCurve(QEasingCurve::Linear);
    animation1->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* animation4 = new QPropertyAnimation(this, "right_angle");
    animation4->setDuration(execution_time);
    animation4->setStartValue(this->m_right_angle);
    animation4->setEndValue(1080);
    animation4->setEasingCurve(QEasingCurve::Linear);
    animation4->start(QAbstractAnimation::DeleteWhenStopped);


    QPropertyAnimation* animation2 = new QPropertyAnimation(this, "wave_position");
    animation2->setDuration(execution_time);
    animation2->setStartValue(this->m_wave_position);
    animation2->setEndValue(this->triangle_position());
    animation2->setEasingCurve(QEasingCurve::Linear);
    animation2->start(QAbstractAnimation::DeleteWhenStopped);



    QPropertyAnimation* animation3 = new QPropertyAnimation(this, "wave_transparency");
    animation3->setDuration(execution_time);
    animation3->setStartValue(this->m_wave_transparency);
    animation3->setEndValue(100);
    animation3->setEasingCurve(QEasingCurve::Linear);
    animation3->start(QAbstractAnimation::DeleteWhenStopped);

}




void Wave_button::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (click_status == true)
        {
            execute_animation();
            click_status = false;
        }
        else if (click_status == false)
        {
            restore_animation();
            click_status = true;
        }

    }

}















int Wave_button::angle() const
{
    return m_angle;
}

void Wave_button::setAngle(int newAngle)
{
    if (m_angle == newAngle)
        return;
    m_angle = newAngle;
    update();
    emit angleChanged();
}

int Wave_button::wave_position() const
{
    return m_wave_position;
}

void Wave_button::setWave_position(int newWave_position)
{
    if (m_wave_position == newWave_position)
        return;
    m_wave_position = newWave_position;
    update();
    emit wave_positionChanged();
}

int Wave_button::wave_transparency() const
{
    return m_wave_transparency;
}

void Wave_button::setWave_transparency(int newWave_transparency)
{
    if (m_wave_transparency == newWave_transparency)
        return;
    m_wave_transparency = newWave_transparency;
    update();
    emit wave_transparencyChanged();
}

int Wave_button::right_angle() const
{
    return m_right_angle;
}

void Wave_button::setRight_angle(int newRight_angle)
{
    if (m_right_angle == newRight_angle)
        return;
    m_right_angle = newRight_angle;
    update();
    emit right_angleChanged();
}
