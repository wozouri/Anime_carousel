#include "timer_animation.h"
#include <QDebug>
#include <QtMath>

Timer_animation::Timer_animation(QObject* target, const QByteArray& propertyName, QObject* parent)
    : QObject(parent), m_target(target), m_propertyName(propertyName), m_previousValue(QVariant())
{
    m_timer.setInterval(16);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &Timer_animation::update);

    if (m_target && !m_propertyName.isEmpty())
    {
        int propIndex = m_target->metaObject()->indexOfProperty(m_propertyName);
        if (propIndex < 0)
        {
            qWarning("属性 '%s'未找到", m_propertyName.constData());
        }
        else
        {
            QMetaProperty prop = m_target->metaObject()->property(propIndex);
            if (!prop.isWritable())
            {
                qWarning("属性 '%s' 是只读", m_propertyName.constData());
            }
        }
    }
}

void Timer_animation::calculateInterval()
{
    if (!m_target || m_propertyName.isEmpty()) return;

    QMetaProperty prop = m_target->metaObject()->property(
        m_target->metaObject()->indexOfProperty(m_propertyName)
    );
    const int type = prop.userType();

    if (type == QMetaType::Int)
    {
        const int delta = qAbs(m_endValue.toInt() - m_startValue.toInt());
        if (delta == 0) return;
        const int idealInterval = qMax(1, m_duration / delta);
        m_timer.setInterval(qBound(1, idealInterval, 16));
    }
    else if (type == QMetaType::Double)
    {
        const double delta = qAbs(m_endValue.toDouble() - m_startValue.toDouble());
        const double stepSize = 0.5;
        const int steps = qCeil(delta / stepSize);
        const int idealInterval = qMax(1, m_duration / steps);
        m_timer.setInterval(qBound(1, idealInterval, 16));
    }
    else {
        m_timer.setInterval(16);
    }
}

void Timer_animation::setStartValue(const QVariant& value) { m_startValue = value; }
void Timer_animation::setEndValue(const QVariant& value) { m_endValue = value; }
void Timer_animation::setDuration(int msecs) { if (msecs > 0) m_duration = msecs; }

void Timer_animation::setDirection(Direction dir) {
    if (m_direction == dir) return;
    m_direction = dir;

    if (m_state == Running) {
        qint64 current = QDateTime::currentMSecsSinceEpoch();
        qreal elapsed = current - m_startTime;
        progress = elapsed / m_duration;

        m_startTime = current - (m_duration * (1 - progress));
    }
}

void Timer_animation::start(bool autoDelete)
{
    if (m_state == Running) return;

    m_autoDelete = autoDelete;

    if (!m_target || m_propertyName.isEmpty()) return;

    QMetaProperty prop = m_target->metaObject()->property(
        m_target->metaObject()->indexOfProperty(m_propertyName)
    );

    if (!m_startValue.canConvert(prop.userType()) ||
        !m_endValue.canConvert(prop.userType())) return;


    m_previousValue = (m_direction == Forward) ? m_startValue : m_endValue;

    progress = 0.0;
    if (m_state == Paused)
    {
        qint64 totalElapsed = m_pausedTime - m_startTime;
        progress = qreal(totalElapsed) / m_duration;
    }

    m_startTime = QDateTime::currentMSecsSinceEpoch() - (m_duration * progress);
    calculateInterval();
    m_timer.start();
    setState(Running);

    emit started();
}

void Timer_animation::pause()
{
    if (m_state != Running) return;

    m_timer.stop();
    m_pausedTime = QDateTime::currentMSecsSinceEpoch();
    setState(Paused);
}

void Timer_animation::resume()
{
    if (m_state != Paused) return;

    qint64 pauseDuration = QDateTime::currentMSecsSinceEpoch() - m_pausedTime;
    m_startTime += pauseDuration;
    m_timer.start();
    setState(Running);
}

void Timer_animation::stop()
{
    m_timer.stop();
    if (m_autoDelete)  this->deleteLater();
    m_previousValue = QVariant();
    setState(Stopped);
}

void Timer_animation::update()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qreal elapsed = currentTime - m_startTime;
    qreal progress = elapsed / m_duration;

    if (m_direction == Backward) progress = 1.0 - progress;

    if ((m_direction == Forward && progress >= 1.0) ||
        (m_direction == Backward && progress <= 0.0))
    {
        progress = qBound(0.0, progress, 1.0);
        QVariant finalValue = (m_direction == Forward) ? m_endValue : m_startValue;
        m_target->setProperty(m_propertyName, finalValue);
        emit valueChanged(finalValue);

        if (m_previousValue.isValid() && m_previousValue.userType() == finalValue.userType()) {
            QVariant finalIncrement = calculateIncrement(m_previousValue, finalValue);
            emit incrementChanged(finalIncrement);
        }

        stop();
        emit finished();
        return;
    }

    QVariant value = interpolate(progress);
    if (value.isValid()) {
        m_target->setProperty(m_propertyName, value);
        emit valueChanged(value);

        if (m_previousValue.isValid() && m_previousValue.userType() == value.userType()) {
            QVariant increment = calculateIncrement(m_previousValue, value);
            emit incrementChanged(increment);
        }
        m_previousValue = value;
    }
}
QVariant Timer_animation::calculateIncrement(const QVariant &prev, const QVariant &curr) const
{
    if (prev.userType() != curr.userType())
        return QVariant();

    switch (prev.userType()) {
    case QMetaType::Int:
        return curr.toInt() - prev.toInt();
    case QMetaType::Double:
        return curr.toDouble() - prev.toDouble();
    case QMetaType::QColor: {
        QColor prevColor = prev.value<QColor>();
        QColor currColor = curr.value<QColor>();
        return QColor(
            currColor.red() - prevColor.red(),
            currColor.green() - prevColor.green(),
            currColor.blue() - prevColor.blue(),
            currColor.alpha() - prevColor.alpha()
        );
    }
    case QMetaType::QPoint:
        return curr.toPoint() - prev.toPoint();
    case QMetaType::QPointF:
        return curr.toPointF() - prev.toPointF();
    case QMetaType::QSize: {
        QSize prevSize = prev.toSize();
        QSize currSize = curr.toSize();
        return QSize(
            currSize.width() - prevSize.width(),
            currSize.height() - prevSize.height()
        );
    }
    case QMetaType::QSizeF: {
        QSizeF prevSize = prev.toSizeF();
        QSizeF currSize = curr.toSizeF();
        return QSizeF(
            currSize.width() - prevSize.width(),
            currSize.height() - prevSize.height()
        );
    }
    case QMetaType::QRect: {
        QRect prevRect = prev.toRect();
        QRect currRect = curr.toRect();
        return QRect(
            currRect.x() - prevRect.x(),
            currRect.y() - prevRect.y(),
            currRect.width() - prevRect.width(),
            currRect.height() - prevRect.height()
        );
    }
    case QMetaType::QRectF: {
        QRectF prevRect = prev.toRectF();
        QRectF currRect = curr.toRectF();
        return QRectF(
            currRect.x() - prevRect.x(),
            currRect.y() - prevRect.y(),
            currRect.width() - prevRect.width(),
            currRect.height() - prevRect.height()
        );
    }
    default:
        return QVariant();
    }
}

void Timer_animation::setState(State newState) {
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

QVariant Timer_animation::interpolate(qreal progress) const
{
    qreal actualProgress = (m_direction == Forward) ? progress : (1.0 - progress);

    if (m_startValue.userType() != m_endValue.userType())
        return QVariant();

    switch (m_startValue.userType()) {
    case QMetaType::Int: {
        int start = m_startValue.toInt();
        int end = m_endValue.toInt();
        return start + (end - start) * progress;
    }
    case QMetaType::Double: {
        double start = m_startValue.toDouble();
        double end = m_endValue.toDouble();
        return start + (end - start) * progress;
    }
    case QMetaType::QColor: {
        QColor start = m_startValue.value<QColor>();
        QColor end = m_endValue.value<QColor>();
        return QColor(
            qBound(0, static_cast<int>(start.red() + (end.red() - start.red()) * actualProgress), 255),
            qBound(0, static_cast<int>(start.green() + (end.green() - start.green()) * progress), 255),
            qBound(0, static_cast<int>(start.blue() + (end.blue() - start.blue()) * progress), 255),
            qBound(0, static_cast<int>(start.alpha() + (end.alpha() - start.alpha()) * progress), 255)
        );
    }
    case QMetaType::QPoint: {
        QPoint start = m_startValue.toPoint();
        QPoint end = m_endValue.toPoint();
        return QPoint(
            start.x() + (end.x() - start.x()) * progress,
            start.y() + (end.y() - start.y()) * progress
        );
    }
    case QMetaType::QPointF: {
        QPointF start = m_startValue.toPointF();
        QPointF end = m_endValue.toPointF();
        return QPointF(
            start.x() + (end.x() - start.x()) * progress,
            start.y() + (end.y() - start.y()) * progress
        );
    }
    case QMetaType::QSize: {
        QSize start = m_startValue.toSize();
        QSize end = m_endValue.toSize();
        return QSize(
            start.width() + (end.width() - start.width()) * progress,
            start.height() + (end.height() - start.height()) * progress
        );
    }
    case QMetaType::QSizeF: {
        QSizeF start = m_startValue.toSizeF();
        QSizeF end = m_endValue.toSizeF();
        return QSizeF(
            start.width() + (end.width() - start.width()) * progress,
            start.height() + (end.height() - start.height()) * progress
        );
    }
    case QMetaType::QRect: {
        QRect start = m_startValue.toRect();
        QRect end = m_endValue.toRect();
        return QRect(
            start.x() + (end.x() - start.x()) * progress,
            start.y() + (end.y() - start.y()) * progress,
            start.width() + (end.width() - start.width()) * progress,
            start.height() + (end.height() - start.height()) * progress
        );
    }
    case QMetaType::QRectF: {
        QRectF start = m_startValue.toRectF();
        QRectF end = m_endValue.toRectF();
        return QRectF(
            start.x() + (end.x() - start.x()) * progress,
            start.y() + (end.y() - start.y()) * progress,
            start.width() + (end.width() - start.width()) * progress,
            start.height() + (end.height() - start.height()) * progress
        );
    }
    default:
        return QVariant();
    }
}
