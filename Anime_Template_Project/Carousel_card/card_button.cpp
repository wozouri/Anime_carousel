#include "card_button.h"

Card_button::Card_button(QString text, QColor button_color, QWidget* parent) : QPushButton{ parent }
{
    this->btntext = text;
    this->m_button_color = button_color;
    this->resize(122, 32);
    this->color_control();

    animation = new QPropertyAnimation(this, "color_opacity");
    animation->setDuration(100);
    animation->setStartValue(this->color_opacity());
    animation->setEndValue(0);

    connect(this, &Card_button::pressed, this, &Card_button::openWebsite);
}

void Card_button::openWebsite()
{
    QDesktopServices::openUrl(QUrl(website_url));
}

void Card_button::setWebsite_url(QString url)
{
    this->website_url = url;
}

void Card_button::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(0, 0, 122, 32);

    QPainterPath path;
    path.addRoundedRect(0, 0, 122, 32, 10, 10);
    painter.setClipPath(path);
    painter.setPen(Qt::NoPen);

    QBrush Brush(color1);
    painter.setBrush(Brush);
    painter.drawRoundedRect(0, 0, 122, 32, 8, 8);

    color2.setAlpha(m_color_opacity);
    QBrush Brush1(color2);
    painter.setBrush(Brush1);
    painter.drawRoundedRect(0, -3, 122, 32, 8, 8);

    QPen pen;
    pen.setWidth(1);
    pen.setColor(Qt::white);
    pen.setStyle(Qt::SolidLine);
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::BevelJoin);
    painter.setPen(pen);

    QPoint points[] = {
        QPoint(28, 10),
        QPoint(33, 15),
        QPoint(28, 20),
    };
    painter.drawPolyline(points, 3);

    painter.setPen(Qt::white);
    QFont font("Microsoft YaHei", 10, QFont::Normal);
    painter.setFont(font);
    painter.drawText(45, 21, btntext);
}

bool Card_button::event(QEvent* e)
{
    if (e->type() == QEvent::Enter)
    {
        animation->setDirection(QPropertyAnimation::Forward);
        animation->start();
        qDebug() << "鼠标进入";
    }
    else if (e->type() == QEvent::Leave)
    {
        animation->setDirection(QPropertyAnimation::Backward);
        animation->start();
        qDebug() << "鼠标离开";
        update();
    }

    return QPushButton::event(e);
}

QColor Card_button::getbutton_color() const
{
    return m_button_color;
}

void Card_button::setButton_color(const QColor &newButton_color)
{
    if (m_button_color == newButton_color)
        return;
    m_button_color = newButton_color;
    update();
}

void Card_button::color_control()
{
    m_button_color.setAlpha(255);
    color1 = m_button_color;
    color2 = QColor(m_button_color.red() * 0.8, m_button_color.green() * 0.8, m_button_color.blue() * 0.8);
}

int Card_button::color_opacity() const
{
    return m_color_opacity;
}

void Card_button::setcolor_opacity(int newColor_opacity)
{
    if (m_color_opacity == newColor_opacity)
        return;
    m_color_opacity = newColor_opacity;
    update();
}
