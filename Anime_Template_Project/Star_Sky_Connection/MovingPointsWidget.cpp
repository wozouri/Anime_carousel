#include "MovingPointsWidget.h"



MovingPoint::MovingPoint(const QPointF& pos, const QPointF& direction, double speed, const QColor& color)
    : m_position(pos),
    m_speed(speed),
    m_color(color)
{
    qreal length = qSqrt(direction.x() * direction.x() + direction.y() * direction.y());
    if (length > 0)
    {
        m_direction = direction / length;
    } else {
        m_direction = QPointF(0, 0);
    }
}

void MovingPoint::updatePosition(double deltaTime)
{
    m_position += m_direction * (m_speed * deltaTime);
}

void MovingPoint::setDirection(const QPointF& newDirection)
{
    qreal length = qSqrt(newDirection.x() * newDirection.x() + newDirection.y() * newDirection.y());
    if (length > 0)
    {
        m_direction = newDirection / length;
    }
    else
    {
        m_direction = QPointF(0, 0);
    }
}

QPointF MovingPoint::getPosition() const
{
    return m_position;
}

QColor MovingPoint::getColor() const
{
    return m_color;
}

QPointF MovingPoint::getDirection() const
{
    return m_direction;
}

bool MovingPoint::isOffscreen(const QSizeF& windowSize) const
{
    return m_position.x() < -10 || m_position.x() > windowSize.width() + 10 ||
           m_position.y() < -10 || m_position.y() > windowSize.height() + 10;
}






MovingPointsWidget::MovingPointsWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);

    QPalette pal = this->palette();
    pal.setColor(QPalette::ColorRole::Window, QColor(0, 0, 0,255));
    this->setPalette(pal);

    resize(1200, 800);

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MovingPointsWidget::updatePoints);
    m_updateTimer->start(m_timerInterval);

    for (int i = 0; i < m_maxPoints / 2; ++i)
    {
        generateRandomPoint();
    }
}

MovingPointsWidget::~MovingPointsWidget()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
        delete m_updateTimer;
        m_updateTimer = nullptr;
    }
}

void MovingPointsWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < m_points.size(); ++i)
    {
        for (int j = i + 1; j < m_points.size(); ++j)
        {
            const QPointF& pos1 = m_points[i].getPosition();
            const QPointF& pos2 = m_points[j].getPosition();

            qreal distance = qSqrt(qPow(pos1.x() - pos2.x(), 2) + qPow(pos1.y() - pos2.y(), 2));

            if (distance < 230)
            {
                int alpha = 0;

                if (distance < 100)
                {
                    alpha = 255;
                }
                else
                {
                    alpha = qRound(255.0 * (1.0 - (distance - 100.0) / 130.0));
                    alpha = qMax(0, qMin(255, alpha));
                }

                painter.setPen(QPen(QColor(255, 255, 255, alpha)));
                painter.drawLine(pos1, pos2);
            }
        }
    }

    for (const auto& point : m_points)
    {
        painter.setBrush(QBrush(point.getColor()));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(point.getPosition(), 2, 2);
    }
}

void MovingPointsWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePosition = event->pos();
}

void MovingPointsWidget::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    m_mouseInWidget = true;
}

void MovingPointsWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_mouseInWidget = false;
}

void MovingPointsWidget::updatePoints()
{
    double deltaTime = static_cast<double>(m_timerInterval) / 1000.0;

    for (int i = 0; i < m_points.size(); ++i)
    {
        QPointF currentPointPos = m_points[i].getPosition();
        QPointF currentPointDir = m_points[i].getDirection();

        if (m_mouseInWidget)
        {
            qreal distToMouse = qSqrt(qPow(currentPointPos.x() - m_mousePosition.x(), 2) +
                                      qPow(currentPointPos.y() - m_mousePosition.y(), 2));

            if (distToMouse < m_attractionRadius)
            {
                QPointF targetPoint = calculateHexagonPoint(m_mousePosition, i % 6, 80.0);

                QPointF attractionVector = targetPoint - currentPointPos;

                qreal attractionLength = qSqrt(qPow(attractionVector.x(), 2) + qPow(attractionVector.y(), 2));
                if (attractionLength > 0)
                {
                    attractionVector /= attractionLength;
                }
                else
                {
                    attractionVector = QPointF(0,0);
                }

                double attractionFactor = 1.0 - (distToMouse / m_attractionRadius);
                attractionFactor = qMax(0.0, qMin(1.0, attractionFactor));

                QPointF newDirection = currentPointDir * (1.0 - attractionFactor) + attractionVector * attractionFactor;

                qreal newDirLength = qSqrt(qPow(newDirection.x(), 2) + qPow(newDirection.y(), 2));
                if (newDirLength > 0)
                {
                    m_points[i].setDirection(newDirection / newDirLength);
                }
            }
        }

        m_points[i].updatePosition(deltaTime);
    }

    for (int i = 0; i < m_points.size(); )
    {
        if (m_points[i].isOffscreen(size()))
        {
            m_points.removeAt(i);
            if (m_points.size() < m_maxPoints)
            {
                generateRandomPoint();
            }
        }
        else
        {
            ++i;
        }
    }

    while (m_points.size() < m_maxPoints)
    {
        generateRandomPoint();
    }

    update();
}

void MovingPointsWidget::generateRandomPoint()
{
    QPointF startPos;
    QPointF direction;
    int edge = QRandomGenerator::global()->bounded(4);

    switch (edge)
    {
    case 0:
        startPos = QPointF(QRandomGenerator::global()->bounded(width()), -5.0);
        direction = generateRandomDirection();
        if (direction.y() < 0) direction.setY(-direction.y());
        break;
    case 1:
        startPos = QPointF(width() + 5.0, QRandomGenerator::global()->bounded(height()));
        direction = generateRandomDirection();
        if (direction.x() > 0) direction.setX(-direction.x());
        break;
    case 2:
        startPos = QPointF(QRandomGenerator::global()->bounded(width()), height() + 5.0);
        direction = generateRandomDirection();
        if (direction.y() > 0) direction.setY(-direction.y());
        break;
    case 3:
        startPos = QPointF(-5.0, QRandomGenerator::global()->bounded(height()));
        direction = generateRandomDirection();
        if (direction.x() < 0) direction.setX(-direction.x());
        break;
    }

    QColor color(QRandomGenerator::global()->bounded(256),
                 QRandomGenerator::global()->bounded(256),
                 QRandomGenerator::global()->bounded(256));

    m_points.append(MovingPoint(startPos, direction, m_pointSpeed, color));
}

QPointF MovingPointsWidget::generateRandomDirection() const
{
    double angle = QRandomGenerator::global()->bounded(360.0) * M_PI / 180.0;
    return QPointF(qCos(angle), qSin(angle));
}

QPointF MovingPointsWidget::calculateHexagonPoint(const QPointF& center, int index, double radius) const
{
    double angle_rad = M_PI / 3.0 * index;

    angle_rad += M_PI / 6.0;

    return QPointF(center.x() + radius * qCos(angle_rad),
                   center.y() + radius * qSin(angle_rad));
}
