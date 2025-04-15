#include "daen_no_kado.h"

Daen_no_Kado::Daen_no_Kado(QWidget *parent)
    : QWidget{parent}
{
    this->resize(1200, 606);
    recta = QRect(0, 0, 1200, 606);
    
    QPalette pal(this->palette());
    pal.setColor(QPalette::Window, QColor(243, 246, 253));
    this->setPalette(pal);

    button = new Latticed_Circle_Button(this);
    button2 = new Latticed_Circle_Button(this);
    button->Shikisai_no_joutai = true;
    connect(button, &Latticed_Circle_Button::Latticed_Circle_Click_Signal, this, &Daen_no_Kado::Move_Left);
    connect(button2, &Latticed_Circle_Button::Latticed_Circle_Click_Signal, this, &Daen_no_Kado::Move_Right);

    this->Get_Image();
    this->Kiso_Deta_Kouchiku();
    this->Gazou_wo_nabebae_ru();

    connect(&timerA, &QTimer::timeout, this, &Daen_no_Kado::koushin_suru);
    timerA.setInterval(16);

    connect(&timerB, &QTimer::timeout, this, &Daen_no_Kado::Progress_Bar_Data_Update);
    timerB.setInterval(16);
    timerB.start();

    this->animations();
}

void Daen_no_Kado::animations()
{
    animation = new QPropertyAnimation(this->button, "geometry");
    animation->setDuration(300);
    animation->setStartValue(this->button->geometry());
    animation->setEndValue(QRect(this->button->pos().x() - zoom_rate / 2, this->button->pos().y() - zoom_rate / 2, button->width() + zoom_rate, button->height() + zoom_rate));
    animation->setEasingCurve(QEasingCurve::Linear);

    animation2 = new QPropertyAnimation(this->button2, "geometry");
    animation2->setDuration(300);
    animation2->setStartValue(this->button2->geometry());
    animation2->setEndValue(QRect(this->button2->pos().x() - zoom_rate / 2, this->button2->pos().y() - zoom_rate / 2, button2->width() + zoom_rate, button2->height() + zoom_rate));
    animation2->setEasingCurve(QEasingCurve::Linear);

    connect(button, &Latticed_Circle_Button::execute_animation_signal, this, [this](Latticed_Circle_Button::AnimationState state) {
        if (state == Latticed_Circle_Button::Execute)
        {
            animation->setDirection(QAbstractAnimation::Forward);
            animation->start();
        }
        else if (state == Latticed_Circle_Button::Restore)
        {
            animation->setDirection(QAbstractAnimation::Backward);
            animation->start();
        }
    });

    connect(button2, &Latticed_Circle_Button::execute_animation_signal, this, [this](Latticed_Circle_Button::AnimationState state) {
        if (state == Latticed_Circle_Button::Execute)
        {
            animation2->setDirection(QAbstractAnimation::Forward);
            animation2->start();
        }
        else if (state == Latticed_Circle_Button::Restore)
        {
            animation2->setDirection(QAbstractAnimation::Backward);
            animation2->start();
        }
    });

    animation3 = new QPropertyAnimation(this, "Expand_Collapse_Height");
    animation3->setDuration(360);
    animation3->setStartValue(m_Expand_Collapse_Height);
    animation3->setEndValue(recta.height());
    animation3->setEasingCurve(QEasingCurve::Linear);

    animation4 = new QPropertyAnimation(this, "Expand_Collapse_Opacity");
    animation4->setDuration(360);
    animation4->setStartValue(m_Expand_Collapse_Opacity);
    animation4->setEndValue(0);
    animation4->setEasingCurve(QEasingCurve::Linear);
}
void Daen_no_Kado::Get_Image()
{
    qreal x = 0;
    for (int i = 1; i < 99; i++)
    {
        QPixmap pixmap(QString("://img/card_image%1.jpg").arg(i));
        if (pixmap.isNull()) return;

        Gazou_Shuu.append(pixmap);
    }
}

void Daen_no_Kado::Kiso_Deta_Kouchiku()
{
    leftBound = recta.width() * 0.3750;
    Jouhan_Daen_no_Chuushin = QPointF(recta.width() / 2.0, 0);
    Jouhan_Daen_no_X_Hankei = recta.width() * 0.56;
    Jouhan_Daen_no_Y_Hankei = recta.height() * 0.2215;

    Kahan_Daen_no_Chuushin = QPointF(recta.width() / 2.0, recta.height());
    Kahan_Daen_no_X_Hankei = recta.width() * 0.656;
    Kahan_Daen_no_Y_Hankei = recta.height() * 0.3261;

    int i = recta.width() / 30;
    button->resize(i, i);
    button2->resize(i, i);
    button->move(recta.width() * 0.45, recta.height() * 0.8);
    button2->move(recta.width() * 0.50, recta.height() * 0.8);

    Progress_Bar_Step = qreal(recta.width()) / qreal(recta.height()) * 1.6;
    m_Expand_Collapse_Height = recta.height() * 0.98;

    qDebug() << "进度条步长" << Progress_Bar_Step;
    qDebug() << "展开高度" << m_Expand_Collapse_Height;

    Kado_Suu.clear();
    this->Gazou_wo_nabebae_ru();
    this->animations();
}

void Daen_no_Kado::Gazou_wo_nabebae_ru()
{
    qreal x = Ratio_Position * recta.width();

    for (int i = 0; i < Gazou_Shuu.size(); i++)
    {
        if (i == 0) 
            Left_Limit_Margin = x + -leftBound * 0.4444;
        else if (i == Gazou_Shuu.size() - 1) 
            Right_Limit_Margin = x + leftBound * 0.3333;

        Kado_Suu.append(qMakePair(x, leftBound * 0.6666 + x));
        x += leftBound * 0.6888;
    }

    kankaku = Kado_Suu[3].second - Kado_Suu[3].first;
}
void Daen_no_Kado::resizeEvent(QResizeEvent* event)
{
    recta = QRect(0, 0, event->size().width(), event->size().height());
    this->Kiso_Deta_Kouchiku();

    m_Progress_Bar_X = (m_Progress_Bar_X / event->oldSize().width()) * event->size().width();
    m_Progress_Bar_Width = (m_Progress_Bar_Width / event->oldSize().width()) * event->size().width();

    QWidget::resizeEvent(event);
}

void Daen_no_Kado::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (!Ima_no_joutai) return;
        this->setCursor(Qt::PointingHandCursor);

        mousePressed = true;
        startX = event->pos().x();
        accumulatedDistance = 0;
    }
    QWidget::mousePressEvent(event);
}

void Daen_no_Kado::mouseMoveEvent(QMouseEvent* event)
{
    if (mousePressed)
    {
        int currentX = event->pos().x();
        int deltaX = currentX - startX;

        if (deltaX > 0) Execution_Direction = true;
        else if (deltaX <= 0) Execution_Direction = false;

        accumulatedDistance += qAbs(deltaX);

        if (accumulatedDistance >= 9) {
            this->koushin_suru();
            accumulatedDistance -= 9;
        }

        startX = currentX;
    }
    QWidget::mouseMoveEvent(event);
}

void Daen_no_Kado::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        this->setCursor(Qt::ArrowCursor);
        mousePressed = false;
        accumulatedDistance = 0;
    }
    QWidget::mouseReleaseEvent(event);
}
bool Daen_no_Kado::event(QEvent* e)
{
    if (e->type() == QEvent::Enter)
    {
        timerB.stop();
        animation3->setDirection(QAbstractAnimation::Forward);
        animation3->start();
        animation4->setDirection(QAbstractAnimation::Forward);
        animation4->start();
    }
    else if (e->type() == QEvent::Leave)
    {
        timerB.start();
        animation3->setDirection(QAbstractAnimation::Backward);
        animation3->start();
        animation4->setDirection(QAbstractAnimation::Backward);
        animation4->start();
    }
    return QWidget::event(e);
}

void Daen_no_Kado::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) 
        this->Card_Move_Left_Behavior();
    else if (event->angleDelta().y() < 0) 
        this->Card_Move_Right_Behavior();
}

void Daen_no_Kado::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::LosslessImageRendering);

    for (int i = 0; i < Kado_Suu.size(); i++) 
        Byouga_Ryouiki(Kado_Suu[i].first, Kado_Suu[i].second, Gazou_Shuu[i]);

    this->draw_text();
    this->Draw_Progress_Bar();
}

void Daen_no_Kado::Byouga_Ryouiki(qreal& Gazou_Zen_XJiku, qreal& Gazou_Go_XJiku, QPixmap& Ehon)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::LosslessImageRendering);

    if (Gazou_Zen_XJiku > recta.width() || Gazou_Go_XJiku < 0) return;

    qreal Haba = Gazou_Go_XJiku - Gazou_Zen_XJiku;
    QRectF Kari_no_saizu(Gazou_Zen_XJiku, 0, Haba, recta.height());

    QPainterPath targetPath1;
    targetPath1.addEllipse(Kahan_Daen_no_Chuushin, Kahan_Daen_no_X_Hankei, Kahan_Daen_no_Y_Hankei);
    QPainterPath targetPath2;
    targetPath2.addEllipse(Jouhan_Daen_no_Chuushin, Jouhan_Daen_no_X_Hankei, Jouhan_Daen_no_Y_Hankei);

    QPainterPath mergedTargetPath = targetPath1.united(targetPath2);
    QPainterPath entireArea;
    entireArea.addRect(recta);
    QPainterPath clipPath = entireArea.subtracted(mergedTargetPath);
    QPainterPath targetPath3;
    targetPath3.addRect(Kari_no_saizu);
    QPainterPath targetPath4 = clipPath.intersected(targetPath3);
    painter.setClipPath(targetPath4);

    qreal targetHeight = Kari_no_saizu.height() * Keisan_suru_shukusetsu_no_takasa(Gazou_Zen_XJiku);
    qreal aspectRatio = Ehon.width() / qreal(Ehon.height());
    qreal targetWidth = targetHeight * aspectRatio;
    QRectF targetRect;
    targetRect.setSize(QSizeF(targetWidth, targetHeight));
    targetRect.moveCenter(QPointF(Kari_no_saizu.center().rx(), Kari_no_saizu.height() * 0.45));

    QTransform transform;
    QPointF center = targetRect.center();
    transform.translate(center.x(), center.y());
    transform.rotate(calculateRectRotation(Gazou_Zen_XJiku), Qt::YAxis, 2048.0);
    transform.translate(-center.x(), -center.y());

    painter.setWorldTransform(transform);
    painter.drawPixmap(targetRect, Ehon, Ehon.rect());

    QRect rect1(0, 0, recta.width(), recta.height());
    QFont font1;
    font1.setPixelSize(recta.width() / 45);
    font1.setBold(true);
    font1.setWordSpacing(4);
    font1.setStyleHint(QFont::Decorative);
    painter.setFont(font1);
    QColor semiTransparent(255, 177, 248, 255);
    painter.setPen(semiTransparent);
    painter.drawText(Kari_no_saizu, Qt::AlignCenter, QString("夹心假面"));
}
void Daen_no_Kado::draw_text()
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setViewport(recta);
    painter.setWindow(recta);

    QRect rect1(0, 0, recta.width(), recta.height());
    QFont font1;
    font1.setPixelSize(recta.width() / 35);
    font1.setBold(true);
    font1.setWordSpacing(4);
    font1.setStyleHint(QFont::Decorative);
    painter.setFont(font1);

    QColor semiTransparent(0, 0, 0, 255);
    painter.setPen(semiTransparent);

    QRect actualRect = painter.boundingRect(rect1, Qt::AlignCenter, QString("Hello_World"));
    rect1.setHeight(actualRect.height());
    rect1.setWidth(actualRect.width());
    rect1.moveCenter(QPoint(width() / 2, height() / 10));
    painter.drawText(rect1, QString("Hello_World"));
}

void Daen_no_Kado::Draw_Progress_Bar()
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setViewport(recta);
    painter.setWindow(recta);

    painter.setPen(Qt::NoPen);
    QBrush brush;
    brush.setColor(QColor(129, 249, 255, m_Expand_Collapse_Opacity));
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);
    painter.drawRect(QRectF(m_Progress_Bar_X, m_Expand_Collapse_Height, m_Progress_Bar_Width, recta.height() * 0.04));
}

void Daen_no_Kado::Progress_Bar_Data_Update()
{
    if (Is_Execution_Step_Width)
    {
        m_Progress_Bar_Width += Progress_Bar_Step;
        m_Progress_Bar_X = 0;

        if (m_Progress_Bar_Width >= recta.width())
        {
            Is_Execution_Step_Width = false;
            Is_Execution_Step_X = true;

            this->Card_Move_Right_Behavior();
        }
    }
    else if (Is_Execution_Step_X)
    {
        m_Progress_Bar_X += Progress_Bar_Step;
        m_Progress_Bar_Width -= Progress_Bar_Step;

        if (m_Progress_Bar_X >= recta.width())
        {
            m_Progress_Bar_Width = recta.width();
            Is_Execution_Step_X = false;

            this->Card_Move_Right_Behavior();
        }
    }
    else if (m_Progress_Bar_X > 0)
    {
        m_Progress_Bar_X -= Progress_Bar_Step;

        if (m_Progress_Bar_X <= 0)
        {
            m_Progress_Bar_X = 0;

            this->Card_Move_Right_Behavior();
        }
    }
    else
    {
        m_Progress_Bar_Width -= Progress_Bar_Step;

        if (m_Progress_Bar_Width <= 0)
        {
            m_Progress_Bar_Width = 0;
            Is_Execution_Step_Width = true;

            this->Card_Move_Right_Behavior();
        }
    }

    update();
}
void Daen_no_Kado::koushin_suru()
{
    for (auto& pair : Kado_Suu)
    {
        if (Execution_Direction == false)
        {
            pair.first -= Time_Step;
            pair.second -= Time_Step;
        }
        else if (Execution_Direction == true)
        {
            pair.first += Time_Step;
            pair.second += Time_Step;
        }
    }

    if (Kado_Suu[0].first < Left_Limit_Margin)
    {
        Kado_Suu.append(Kado_Suu.takeFirst());
        Gazou_Shuu.append(Gazou_Shuu.takeFirst());
        Kado_Suu[Kado_Suu.size() - 1].first = Kado_Suu[Kado_Suu.size() - 2].first + leftBound * 0.6888;
        Kado_Suu[Kado_Suu.size() - 1].second = Kado_Suu[Kado_Suu.size() - 2].second + leftBound * 0.6888;
    }
    else if (Kado_Suu[Kado_Suu.size() - 1].first > Right_Limit_Margin)
    {
        Kado_Suu.prepend(Kado_Suu.takeLast());
        Gazou_Shuu.prepend(Gazou_Shuu.takeLast());
        Kado_Suu[0].first = Kado_Suu[1].first - leftBound * 0.6888;
        Kado_Suu[0].second = Kado_Suu[1].second - leftBound * 0.6888;
    }

    if (mousePressed == true)
    {
        timerA.stop();
        Ratio_Position = Kado_Suu[0].first / recta.width();
        update();
        return;
    }

    Kari_no_kankaku += Time_Step;
    if (Kari_no_kankaku >= leftBound * 0.6888)
    {
        timerA.stop();
        Ima_no_joutai = true;
        Ratio_Position = Kado_Suu[0].first / recta.width();
    }
    update();
}

qreal Daen_no_Kado::Keisan_suru_shukusetsu_no_takasa(qreal& xLeft)
{
    if (xLeft < leftBound)
    {
        qreal overflow = leftBound - xLeft;
        return overflow / leftBound / 4 + maxRadians;
    }
    else if (xLeft > leftBound)
    {
        qreal overflow = xLeft - leftBound;
        return overflow / leftBound / 4 + maxRadians;
    }
    else
    {
        return maxRadians;
    }
}

qreal Daen_no_Kado::calculateRectRotation(qreal& xLeft)
{

    // 计算左侧影响
    if (xLeft < leftBound)
    {
        qreal overflow = leftBound - xLeft;
        return -maxRadians2 * (overflow / leftBound);
    }
    else if (xLeft > leftBound)
    {

        qreal overflow = xLeft - leftBound;
        return maxRadians2 * (overflow / leftBound);
    }
    else return 0.0;

}


void Daen_no_Kado::Move_Left()
{
    if (Ima_no_joutai != true) return;
    this->Card_Move_Left_Behavior();
}

void Daen_no_Kado::Move_Right()
{
    if (Ima_no_joutai != true) return;
    this->Card_Move_Right_Behavior();
}

void Daen_no_Kado::Card_Move_Right_Behavior() 
{
    Ima_no_joutai = false;
    timerA.start();
    Kari_no_kankaku = 0;
    Execution_Direction = true;
}
void Daen_no_Kado::Card_Move_Left_Behavior() 
{
    Ima_no_joutai = false;
    timerA.start();
    Kari_no_kankaku = 0;
    Execution_Direction = false;
}


qreal Daen_no_Kado::Progress_Bar_X() const
{
    return m_Progress_Bar_X;
}

void Daen_no_Kado::setProgress_Bar_X(qreal newProgress_Bar_X)
{
    if (m_Progress_Bar_X == newProgress_Bar_X)
        return;
    m_Progress_Bar_X = newProgress_Bar_X;

}

qreal Daen_no_Kado::Progress_Bar_Width() const
{
    return m_Progress_Bar_Width;
}

void Daen_no_Kado::setProgress_Bar_Width(qreal newProgress_Bar_Width)
{
    if (m_Progress_Bar_Width == newProgress_Bar_Width)
        return;
    m_Progress_Bar_Width = newProgress_Bar_Width;

}

qreal Daen_no_Kado::Expand_Collapse_Height() const
{
    return m_Expand_Collapse_Height;
}

void Daen_no_Kado::setExpand_Collapse_Height(qreal newExpand_Collapse_Height)
{
    if (qFuzzyCompare(m_Expand_Collapse_Height, newExpand_Collapse_Height))
        return;
    m_Expand_Collapse_Height = newExpand_Collapse_Height;
    update();

}

int Daen_no_Kado::Expand_Collapse_Opacity() const
{
    return m_Expand_Collapse_Opacity;
}

void Daen_no_Kado::setExpand_Collapse_Opacity(int newExpand_Collapse_Opacity)
{
    if (m_Expand_Collapse_Opacity == newExpand_Collapse_Opacity)
        return;
    m_Expand_Collapse_Opacity = newExpand_Collapse_Opacity;
    update();

}