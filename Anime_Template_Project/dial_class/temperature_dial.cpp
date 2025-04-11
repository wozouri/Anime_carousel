#include "temperature_dial.h"

Temperature_dial::Temperature_dial(QWidget *parent) : QWidget(parent) 
{
    resize(260, 260);
    setMouseTracking(true);
    window_width = QRectF(0, 0, 260, 260);
    center = window_width.center();
    radius = window_width.height() * 0.41;
    circleCenter = center + QPointF(0, radius);
    angle = getAngleInDegrees();

    shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 19);
    shadow->setBlurRadius(50);
    shadow->setColor(QColor(213, 224, 254, 255));
    this->setGraphicsEffect(shadow);
    setMouseTracking(true);
    this->startAnimation();
}

void Temperature_dial::startAnimation()
{
    timer_animation = new Timer_animation(this->shadow, "blurRadius");
    timer_animation->setDuration(400);
    timer_animation->setStartValue(50);
    timer_animation->setEndValue(150);

    connect(timer_animation, &Timer_animation::started, this, [this] {
        is_animation_running = false;
        m_previousAngle = atan2(circleCenter.y() - center.y(), circleCenter.x() - center.x());
    });
}

double Temperature_dial::getAngleInDegrees() const
{
    double dx = circleCenter.x() - center.x();
    double dy = circleCenter.y() - center.y();
    double angle = atan2(dy, dx);
    double degrees = angle * (180.0 / M_PI);
    if (degrees < 0) {
        degrees += 360.0;
    }
    return degrees;
}

void Temperature_dial::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(0, 0, window_width.width(), window_width.height());

    QPainterPath path;
    path.addRoundedRect(0, 0, window_width.width(), window_width.height(), window_width.width() / 10, window_width.height() / 10);
    painter.setClipPath(path);
    painter.fillRect(rect(), Qt::white);
    this->draw_empty_circle();
    this->draw_crosshair();
    this->draw_circular_gradient_shadow();
    this->draw_circle();
    this->draw_full_circle();
    this->draw_text();
}

void Temperature_dial::draw_circle()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(window_width.x(), window_width.y(), window_width.width(), window_width.height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawEllipse(circleCenter, 9, 9);
}

void Temperature_dial::draw_empty_circle()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(window_width.x(), window_width.y(), window_width.width(), window_width.height());
    QPen pen;
    pen.setWidth(3);
    pen.setColor(QColor(242, 242, 242, 255));
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, int(radius), int(radius));
}

void Temperature_dial::draw_circular_gradient_shadow()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(window_width.x(), window_width.y(), window_width.width(), window_width.height());
    painter.setPen(QPen(Qt::NoPen));
    QColor Color = getColorForAngle(getAngleInDegrees());
    QRadialGradient radialGrad(window_width.width() / 2, window_width.height() / 2, window_width.width() * 0.6, window_width.width() * 0.35, window_width.height() * 0.65);
    Color.setAlpha(255);
    radialGrad.setColorAt(0, Color);
    Color.setAlpha(222);
    radialGrad.setColorAt(0.3, Color);
    Color.setAlpha(111);
    radialGrad.setColorAt(0.5, Color);
    Color.setAlpha(0);
    radialGrad.setColorAt(0.7, Color);
    radialGrad.setColorAt(1, Color);
    painter.setBrush(radialGrad);
    painter.drawRect(window_width);
}
void Temperature_dial::draw_full_circle()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(window_width.x(), window_width.y(), window_width.width(), window_width.height());
    painter.setPen(QPen(Qt::NoPen));
    painter.setBrush(Qt::white);
    painter.drawEllipse(center, int(window_width.width() * 0.28), int(window_width.width() * 0.28));
}

void Temperature_dial::draw_crosshair()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(window_width.x(), window_width.y(), window_width.width(), window_width.height());
    QPen pen(Qt::black, 2);
    painter.setPen(pen);
    painter.drawLine(center, center + QPoint(0, window_width.height() * 0.31));
    painter.drawLine(center, center + QPoint(0, -window_width.height() * 0.31));
    painter.drawLine(center, center + QPoint(window_width.height() * 0.31, 0));
    painter.drawLine(center, center + QPoint(-window_width.height() * 0.31, 0));
}

void Temperature_dial::draw_text()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0, 0, width(), height());
    painter.setWindow(window_width.x(), window_width.y(), window_width.width(), window_width.height());
    if (is_animation_running == true)
    {
        integerPart = QString::number(int(angle));
        decimalPart = QString::number(angle - int(angle), 'f', 1).mid(2);
        angle = getAngleInDegrees();
    }
    draw_text_template(integerPart, 0.12, QPointF(0.31, 0.54), Qt::AlignRight | Qt::AlignHCenter);
    draw_text_template(".", 0.1, QPointF(0.34, 0.56), Qt::AlignRight | Qt::AlignHCenter);
    draw_text_template(decimalPart, 0.12, QPointF(0.41, 0.54), Qt::AlignRight | Qt::AlignHCenter);
    draw_text_template("°", 0.1, QPointF(0.45, 0.54), Qt::AlignRight | Qt::AlignHCenter);
}

void Temperature_dial::draw_text_template(const QString& text, float fontScale, QPointF positionRatio, const int& alignment)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int baseSize = qMin(width(), height());
    int dynamicFontSize = baseSize * fontScale;
    QFont font("Arial");
    font.setPixelSize(dynamicFontSize);
    font.setWeight(QFont::DemiBold);
    painter.setFont(font);
    QRect textRect;
    textRect.setSize(QSize(width() * 0.4, height() * 0.2));
    textRect.moveTo(
        width() * positionRatio.x() - textRect.width() / 2,
        height() * positionRatio.y() - textRect.height() / 2
    );
    painter.setPen(Qt::black);
    painter.drawText(textRect, alignment, text);
}

QColor Temperature_dial::getColorForAngle(int angle)
{
    if (angle <= 120) {
        return interpolateColor(QColor(235, 255, 188), QColor(255, 245, 188), angle / 120.0);
    }
    else if (angle <= 240) {
        return interpolateColor(QColor(255, 245, 188), QColor(255, 216, 188), (angle - 120) / 120.0);
    }
    else {
        return interpolateColor(QColor(255, 216, 188), QColor(235, 255, 188), (angle - 240) / 120.0);
    }
}

QColor Temperature_dial::interpolateColor(const QColor& start, const QColor& end, double t)
{
    int r = static_cast<int>(start.red() + t * (end.red() - start.red()));
    int g = static_cast<int>(start.green() + t * (end.green() - start.green()));
    int b = static_cast<int>(start.blue() + t * (end.blue() - start.blue()));
    int a = static_cast<int>(start.alpha() + t * (end.alpha() - start.alpha()));
    return QColor(r, g, b, a);
}

void Temperature_dial::set_size(QRect size)
{
    this->window_width = size;
}
void Temperature_dial::mousePressEvent(QMouseEvent *event) 
{
    if (event->button() == Qt::LeftButton) 
    {
        double distance = sqrt(pow(event->pos().x() - circleCenter.x(), 2) +
                               pow(event->pos().y() - circleCenter.y(), 2));
        if (distance <= 20) 
        {
            this->setCursor(Qt::PointingHandCursor);
            isDragging = true;
            is_animation_running = true;
        }
    }
}

void Temperature_dial::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging)
    {
        QPoint newPos = event->pos();
        m_previousAngle = atan2(newPos.y() - center.y(), newPos.x() - center.x());
        circleCenter.setX(center.x() + radius * cos(m_previousAngle));
        circleCenter.setY(center.y() + radius * sin(m_previousAngle));
        update();
    }
}

void Temperature_dial::resizeEvent(QResizeEvent* event)
{
    window_width = QRect(0, 0, event->size().width(), event->size().height());
    center = window_width.center();
    radius = window_width.height() * 0.41;
    circleCenter.setX(center.x() + radius * cos(m_previousAngle));
    circleCenter.setY(center.y() + radius * sin(m_previousAngle));
    fontSize = static_cast<int>(window_width.height() * 0.1);
    update();
    QWidget::resizeEvent(event);
}

void Temperature_dial::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        this->setCursor(Qt::ArrowCursor);
    }
}

bool Temperature_dial::event(QEvent* e)
{
    if (e->type() == QEvent::Enter)
    {
        execute_animation_signal(AnimationState::Execute);
        timer_animation->setDirection(Timer_animation::Forward);
        timer_animation->start();
    }
    else if (e->type() == QEvent::Leave)
    {
        execute_animation_signal(AnimationState::Restore);
        timer_animation->setDirection(Timer_animation::Backward);
        timer_animation->start();
    }
    return QWidget::event(e);
}

int Temperature_dial::shadow_scale() const
{
    return m_shadow_scale;
}

void Temperature_dial::setShadow_scale(int newShadow_scale)
{
    if (m_shadow_scale == newShadow_scale) return;
    m_shadow_scale = newShadow_scale;
    emit shadow_scaleChanged();
}
