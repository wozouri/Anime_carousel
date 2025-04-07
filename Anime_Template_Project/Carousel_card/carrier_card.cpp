#include "carrier_card.h"

Carrier_card::Carrier_card(QPixmap m_carrier_card, QString main_title, QString sub_title,QWidget *parent)
    : QWidget{parent}
{
     
    this->setMaximumSize(860, 480);
    this->resize(320, 480);

    this->m_carrier_card = m_carrier_card;
    this->font_related_construction(main_title, sub_title);

    this->set_gradient_color();
     
    setMouseTracking(true);
     
    connect(&m_clickTimer, &QTimer::timeout, this, &Carrier_card::onClickTimerTimeout);
    connect(&timer4s, &QTimer::timeout, this, &Carrier_card::circle_reset);


    shadow = new QGraphicsDropShadowEffect(Anime_button);
    shadow->setOffset(0, 0);
    shadow->setBlurRadius(150);
    shadow->setColor(Qt::black);
    Anime_button->setGraphicsEffect(shadow);
}

void Carrier_card::font_related_construction(QString main_title, QString sub_title)
{
    Anime_small_text = new Card_text(main_title, sub_title,this);
    Anime_small_text->reset_animation(0);
    qDebug() << "Card_text::" << Anime_small_text->height();
    
}

void Carrier_card::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (!m_clickTimer.isActive())
        {
            emit carrier_card_idChanged(this->m_carrier_card_id);
            m_clickTimer.start(500);
        }
    }
    
    QWidget::mousePressEvent(event);
}

void Carrier_card::mouseMoveEvent(QMouseEvent* event)
{
    if (this->isClicked == true)
    {
        mousePos = event->pos();
        update();
    }
}
bool Carrier_card::event(QEvent* e)
{

    if (e->type() == QEvent::Enter)
    {
        this->isClicked = true;

        if (this->m_carrier_card_id == 2) timer4s.stop();

    }
    else if (e->type() == QEvent::Leave)
    {
        this->isClicked = false;

        if (this->m_carrier_card_id == 2) timer4s.start(100);
        update();
    }
    return QWidget::event(e);
}


void Carrier_card::draw_radial_gradient_circle()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRadialGradient gradient1(mousePos.rx(), mousePos.ry(), qMax(width() , height() ) / 2); 
    gradient1.setColorAt(0.0, QColor(255, 255, 255, 15));
    gradient1.setColorAt(0.8, QColor(255, 255, 255, 0));
    gradient1.setColorAt(1.0, QColor(255, 255, 255, 0));

    painter.setBrush(gradient1);
    painter.setPen(Qt::NoPen);

    painter.drawRect(rect());
}

void Carrier_card::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::LosslessImageRendering);

    QPainterPath path;
    path.addRoundedRect(0, 0, this->width(), this->height(), 10, 10);
    painter.setClipPath(path);

    int windowWidth = width();
    int windowHeight = height();
    double aspectRatio = static_cast<double>(m_carrier_card.width()) / m_carrier_card.height();
    int newWidth = static_cast<int>(windowHeight * aspectRatio);
    int newHeight = windowHeight;
    int posX = (windowWidth - newWidth) / 2;
    int posY = 0;
    painter.drawPixmap(posX, posY, newWidth, newHeight, m_carrier_card, 0, 0, m_carrier_card.width(), m_carrier_card.height());

    QLinearGradient gradient(0, 480, 860, 480);
    gradient.setColorAt(0, m_color1); 
    gradient.setColorAt(0.35, m_color2); 
    gradient.setColorAt(0.55, QColor(0, 0, 0, 0)); 
    gradient.setColorAt(1, QColor(0, 0, 0, 0)); 
    gradient.setSpread(QGradient::PadSpread);
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen); 
    painter.drawRect(0, 0, width(), height());

    QColor semiTransparentRed(0, 0, 0, m_opacity);
    painter.setBrush(semiTransparentRed);
    painter.drawRect(0, 0, width(), height()); 

    if (isClicked == true) this->draw_radial_gradient_circle();

    if (m_carrier_card_id == 2 && display_circle == true) draw_sector_circle();
}


void Carrier_card::draw_sector_circle()
{
    QPainter    painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 140));

    QRect   rect(20, 450, 15, 15);
    int startAngle = 0 * 16;
    int spanAngle = m_circle_degrees * 16;
    painter.drawPie(rect, startAngle, spanAngle);
}
void Carrier_card::circle_reset()
{
    TimerTimeout++;
    update();
    if (TimerTimeout == 15)
    {
        display_circle = true;
    }
    if (TimerTimeout < 87 && display_circle == true)
    {
        m_circle_degrees -= 5;
    }

    if (TimerTimeout == 87)
    {
        restore_disc();
        emit carrier_card_idChanged(3);
    }
}

void Carrier_card::set_gradient_color()
{
    QColor clr = this->getMostFrequentColor(m_carrier_card);
    Anime_button = new Card_button(QString("查看详情"), clr, this);
    Anime_button->move(38, 315);

    clr.setAlpha(100);
    m_color1 = clr;

    clr.setAlpha(68);
    m_color2 = clr;

    clr.setAlpha(255);
    m_color3 = clr;
}


QColor Carrier_card::getMostFrequentColor(const QPixmap& pixmap)
{

    QImage image = pixmap.toImage();

    if (image.isNull()) {
        return QColor();
    }


    image = image.convertToFormat(QImage::Format_RGB32);

    QHash<QRgb, int> colorCounts;
    const uchar* bits = image.constBits();
    const int width = image.width();
    const int height = image.height();
    const int bytesPerLine = image.bytesPerLine();


    const qreal minSaturation = 8.1;
    const qreal maxLightnessForWhite = 0.99;

    for (int y = 0; y < height; ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(bits + y * bytesPerLine);
        for (int x = 0; x < width; ++x) {
            QRgb rgb = line[x];
            QColor color = QColor::fromRgb(rgb);


            float h, s, l;
            color.getHslF(&h, &s, &l);


            if (s < minSaturation && l > maxLightnessForWhite) {
                continue;
            }

            colorCounts[rgb]++;
        }
    }

    if (colorCounts.isEmpty()) {
        return QColor();
    }


    QHashIterator<QRgb, int> it(colorCounts);
    QRgb maxColor = it.peekNext().key();
    int maxCount = it.peekNext().value();

    while (it.hasNext()) {
        it.next();
        if (it.value() > maxCount) {
            maxCount = it.value();
            maxColor = it.key();
        }
    }
    return QColor::fromRgb(maxColor);
}


void Carrier_card::Anime_card_transformation(int animation_duration)
{
    if (m_carrier_card_id != 2)
    {
        timer4s.stop();
        restore_disc();

        Anime_small_text->reset_animation(animation_duration);

        QPropertyAnimation* animation1 = new QPropertyAnimation(this, "size");
        animation1->setDuration(animation_duration);
        animation1->setStartValue(this->size());
        animation1->setEndValue(QSize(320, 480));
        animation1->setEasingCurve(QEasingCurve::Linear);

        QPropertyAnimation* animation3 = new QPropertyAnimation(this, "opacity");
        animation3->setDuration(animation_duration);
        animation3->setStartValue(this->opacity());
        animation3->setEndValue(90);
        animation3->setEasingCurve(QEasingCurve::Linear);

        animation1->start(QAbstractAnimation::DeleteWhenStopped);
        animation3->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else
    {
        timer4s.start(100);

        Anime_small_text->start_animation(animation_duration);

        QPropertyAnimation* animation1 = new QPropertyAnimation(this, "size");
        animation1->setDuration(animation_duration);
        animation1->setStartValue(this->size());
        animation1->setEndValue(QSize(860, 480));
        animation1->setEasingCurve(QEasingCurve::Linear);

        QPropertyAnimation* animation3 = new QPropertyAnimation(this, "opacity");
        animation3->setDuration(animation_duration);
        animation3->setStartValue(this->opacity());
        animation3->setEndValue(0);
        animation3->setEasingCurve(QEasingCurve::Linear);

        animation1->start(QAbstractAnimation::DeleteWhenStopped);
        animation3->start(QAbstractAnimation::DeleteWhenStopped);
    }
}


void Carrier_card::Anime_card_1()
{
    this->resize(860, 480);
    Anime_small_text->start_animation(0);
    this->setOpacity(0);
    timer4s.start(100);
}
void Carrier_card::onClickTimerTimeout()
{
    m_clickTimer.stop();
}

void Carrier_card::restore_disc()
{
    display_circle = false;
    TimerTimeout = 0;
    m_circle_degrees = 360;
}

int Carrier_card::carrier_card_id() const
{
    return m_carrier_card_id;
}

void Carrier_card::setcarrier_card_id(int newCarrier_card_id)
{
    if (m_carrier_card_id == newCarrier_card_id)
        return;
    m_carrier_card_id = newCarrier_card_id;
    update();
}

int Carrier_card::opacity() const
{
    return m_opacity;
}

void Carrier_card::setOpacity(int newOpacity)
{
    m_opacity = newOpacity;
    update();
}

int Carrier_card::circle_degrees() const
{
    return m_circle_degrees;
}

void Carrier_card::setCircle_degrees(int newCircle_degrees)
{
    if (m_circle_degrees == newCircle_degrees)
        return;
    m_circle_degrees = newCircle_degrees;
    qDebug() << "circle_degrees changed to:" << m_circle_degrees;
    update();
}
