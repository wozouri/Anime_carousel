#include "jellyfishwidget.h"
#include <QtMath>
#include <QDebug>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <QEvent>

JellyfishWidget::JellyfishWidget(QWidget *parent)
    : QWidget(parent),
    x_input(0.0),
    y_input(0.0),
    t_current(0.0),
    mousePosition(width() / 2.0, height() / 2.0),
    smoothedPupilTarget(0.0, 0.0),
    mouseIsInWindow(false),
    blinkProgress(0.0),
    isBlinking(false),
    pulseScale(1.0),
    m_targetPupilScaleFactor(1.0),
    m_currentPupilScaleFactor(1.0)
{
    setWindowTitle("眼球克苏鲁");
    resize(800, 600);
    setMouseTracking(true);

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &JellyfishWidget::updateAnimation);
    animationTimer->start(15);

    for (int i = 0; i < 3; ++i) {
        TentacleShadow shadow;
        shadow.opacity = QRandomGenerator::global()->generateDouble() * 0.1 + 0.05;
        shadow.phaseOffset = QRandomGenerator::global()->generateDouble() * 100.0;
        shadow.thickness = QRandomGenerator::global()->bounded(50, 150);
        for (int j = 0; j < 5; ++j) {
            shadow.points.append(QPointF(
                QRandomGenerator::global()->bounded(-100, width() + 100),
                QRandomGenerator::global()->bounded(height() / 2, height() + 100)
                ));
        }
        backgroundTentacleShadows.append(shadow);
    }
}

JellyfishWidget::~JellyfishWidget()
{
}

QPointF JellyfishWidget::calculateJellyfishPoint(double local_x, double local_y, double current_t)
{
    double x_formula = local_x;
    double y_formula = local_y;
    double t = current_t;

    double k = 5.0 * qCos(x_formula / 14.0) * qCos(y_formula / 30.0);
    double e = y_formula / 8.0 - 13.0;
    double d = (k * k + e * e) / 59.0 + 4.0;

    double a = qAtan2(e, k);

    double d_safe = (d < 0.001 && d >= 0) ? 0.001 : d;

    double q = 60.0 - qSin(a * e) + k * (3.0 + (4.0 / d_safe) * qSin(d * d - 2.0 * t));
    double c = d / 2.0 + e / 99.0 - t / 18.0;

    double X_result = q * qSin(c);
    double Y_result = (q + 9.0 * d) * qCos(c);

    return QPointF(X_result, Y_result);
}


void JellyfishWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(20, 0, 0));
    drawBackgroundTentacleShadows(&painter);

    QPointF cthulhuCenter = QPointF(width() / 2.0, height() / 2.0);
    QPointF currentCthulhuPosition = calculateJellyfishPoint(x_input, y_input, t_current) + cthulhuCenter;

    double currentEyeballRadius = 80.0 * pulseScale;
    double currentEyeballHeight = 40.0 * pulseScale;

    drawEyeball(&painter, currentCthulhuPosition, currentEyeballRadius, currentEyeballHeight);

    drawGoreTendrils(&painter, currentCthulhuPosition, currentEyeballRadius * 0.9, 15, t_current);

    drawFallingGoreAndBlood(&painter);

    drawAcidDrips(&painter);
}

void JellyfishWidget::mouseMoveEvent(QMouseEvent *event)
{
    mousePosition = event->pos();
    mouseIsInWindow = true;
}

void JellyfishWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    mousePosition = QPointF(width() / 2.0, height() / 2.0);
    mouseIsInWindow = false;
}


void JellyfishWidget::updateAnimation()
{
    t_current += 0.02;

    QPointF cthulhuCenter = QPointF(width() / 2.0, height() / 2.0);
    QPointF currentCthulhuPosition = calculateJellyfishPoint(x_input, y_input, t_current) + cthulhuCenter;

    double eyeball_radius = 80.0;
    double maxPupilTravel = eyeball_radius * 0.4;

    QPointF targetPupilOffset;

    double basePupilRatio = 0.3;
    double minPupilScale = 0.8;
    double maxPupilScale = 1.2;
    double influenceDistance = 200.0;

    if (mouseIsInWindow) {
        QPointF vecToMouse = mousePosition - currentCthulhuPosition;
        double distToMouse = qSqrt(vecToMouse.x() * vecToMouse.x() + vecToMouse.y() * vecToMouse.y());
        if (distToMouse > 0) {
            targetPupilOffset = vecToMouse / distToMouse * qMin(distToMouse, maxPupilTravel);
        } else {
            targetPupilOffset = QPointF(0, 0);
        }

        double normalizedDist = qMin(distToMouse, influenceDistance) / influenceDistance;

        m_targetPupilScaleFactor = minPupilScale + (1.0 - normalizedDist) * (maxPupilScale - minPupilScale);

    } else {
        QPointF wanderOffset(qSin(t_current * 0.7) * 20, qCos(t_current * 0.9) * 15);

        double wander_magnitude = qSqrt(wanderOffset.x()*wanderOffset.x() + wanderOffset.y()*wanderOffset.y());
        if (wander_magnitude > maxPupilTravel) {
            wanderOffset = wanderOffset / wander_magnitude * maxPupilTravel;
        }
        targetPupilOffset = wanderOffset;

        m_targetPupilScaleFactor = 1.0;
    }

    double smoothness_factor = 0.05;
    smoothedPupilTarget = smoothedPupilTarget * (1.0 - smoothness_factor) + targetPupilOffset * smoothness_factor;

    double pupil_smoothness = 0.1;
    m_currentPupilScaleFactor += (m_targetPupilScaleFactor - m_currentPupilScaleFactor) * pupil_smoothness;


    if (isBlinking) {
        blinkProgress += 0.06;
        if (blinkProgress >= 1.0) {
            isBlinking = false;
            blinkProgress = 0.0;
        }
    } else {
        if (QRandomGenerator::global()->generateDouble() < 0.003) {
            isBlinking = true;
            blinkProgress = 0.0;
        }
    }

    double targetPulse = 1.0 + qSin(t_current * 5.0) * 0.02;
    pulseScale += (targetPulse - pulseScale) * 0.1;


    ParticleUpdateRunnable *runnable = new ParticleUpdateRunnable(
        t_current, height(), width(), currentCthulhuPosition, 80.0,
        fallingGoreParticles, acidDrips, backgroundTentacleShadows
        );
    connect(runnable, &ParticleUpdateRunnable::particlesUpdated,
            this, &JellyfishWidget::handleParticlesUpdated, Qt::QueuedConnection);
    QThreadPool::globalInstance()->start(runnable);

    update();
}

void JellyfishWidget::handleParticlesUpdated(QVector<GoreParticle> newGore,
                                             QVector<AcidDrip> newAcid,
                                             QVector<TentacleShadow> newShadows)
{
    fallingGoreParticles = newGore;
    acidDrips = newAcid;
    backgroundTentacleShadows = newShadows;

}


void JellyfishWidget::drawEyeball(QPainter *painter, const QPointF& center, double radius, double bell_height)
{
    QRadialGradient eyeballGradient(center.x(), center.y() - bell_height / 2, radius * 1.5, center.x(), center.y());
    eyeballGradient.setColorAt(0.0, QColor(255, 50, 50, 200));
    eyeballGradient.setColorAt(0.4, QColor(180, 0, 0, 180));
    eyeballGradient.setColorAt(0.8, QColor(80, 0, 0, 150));
    eyeballGradient.setColorAt(1.0, QColor(30, 0, 0, 100));

    painter->setBrush(eyeballGradient);
    painter->setPen(QPen(QColor(50, 0, 0, 180), 3));
    painter->drawEllipse(QRectF(center.x() - radius, center.y() - bell_height, radius * 2, bell_height * 2));


    painter->setPen(Qt::NoPen);
    for (int i = 0; i < 20; ++i) {
        double angle_start = QRandomGenerator::global()->generateDouble() * 2 * M_PI;
        double vein_start_radius = radius * (0.8 + QRandomGenerator::global()->generateDouble() * 0.1);
        QPointF veinStart = center + QPointF(qCos(angle_start) * vein_start_radius, qSin(angle_start) * vein_start_radius * (bell_height / radius));

        double angle_end = angle_start + (QRandomGenerator::global()->generateDouble() * 0.5 - 0.25) * M_PI;
        double vein_end_radius = radius * (0.3 + QRandomGenerator::global()->generateDouble() * 0.3);
        QPointF veinEnd = center + QPointF(qCos(angle_end) * vein_end_radius, qSin(angle_end) * vein_end_radius * (bell_height / radius));

        QColor veinColor(QRandomGenerator::global()->bounded(100, 201), 0, 0, QRandomGenerator::global()->bounded(100, 201));
        int veinThickness = QRandomGenerator::global()->bounded(1, 3);

        painter->setPen(QPen(veinColor, veinThickness));
        painter->drawLine(veinStart, veinEnd);
    }


    painter->save();

    QPainterPath eyeballOutline;
    eyeballOutline.addEllipse(QRectF(center.x() - radius, center.y() - bell_height, radius * 2, bell_height * 2));
    painter->setClipPath(eyeballOutline);

    double iris_radius = radius * 0.6;
    QPointF irisCenter = center + smoothedPupilTarget;

    QRadialGradient irisGradient(irisCenter, iris_radius);
    irisGradient.setColorAt(0.0, QColor(80, 150, 80, 200));
    irisGradient.setColorAt(0.5, QColor(30, 80, 30, 220));
    irisGradient.setColorAt(1.0, QColor(10, 40, 10, 255));

    painter->setBrush(irisGradient);
    painter->setPen(QPen(QColor(0, 0, 0, 150), 2));
    painter->drawEllipse(irisCenter, iris_radius, iris_radius);

    painter->setPen(QPen(QColor(0, 0, 0, 100), 1));
    for (int i = 0; i < 8; ++i) {
        double angle = i * M_PI / 4.0 + t_current * 0.1;
        QPointF crackStart = irisCenter + QPointF(qCos(angle) * iris_radius * 0.5, qSin(angle) * iris_radius * 0.5);
        QPointF crackEnd = irisCenter + QPointF(qCos(angle) * iris_radius * 0.9, qSin(angle) * iris_radius * 0.9);
        painter->drawLine(crackStart, crackEnd);
    }

    double pupil_base_radius = radius * 0.3;
    double pupil_radius_adjusted = pupil_base_radius * m_currentPupilScaleFactor;

    QPointF pupilOffsetFinal = smoothedPupilTarget;
    pupilOffsetFinal.setX(pupilOffsetFinal.x() + qSin(t_current * 2.5) * 2);
    pupilOffsetFinal.setY(pupilOffsetFinal.y() + qCos(t_current * 2.8) * 2);

    QPointF pupilCenter = center + pupilOffsetFinal;

    double pupil_width = pupil_radius_adjusted * (1.0 + qSin(t_current * 1.5) * 0.05);
    double pupil_height = pupil_radius_adjusted * (1.0 + qCos(t_current * 1.7) * 0.05);

    painter->setBrush(QColor(0, 0, 0, 255));
    painter->setPen(QPen(QColor(20, 0, 0, 200), 2));
    painter->drawEllipse(QRectF(pupilCenter.x() - pupil_width / 2, pupilCenter.y() - pupil_height / 2, pupil_width, pupil_height));

    painter->restore();


    if (blinkProgress > 0.0) {
        double closure_amount;
        if (blinkProgress <= 0.5) {
            closure_amount = blinkProgress * 2.0;
        } else {
            closure_amount = (1.0 - blinkProgress) * 2.0;
        }
        closure_amount = qMax(0.0, qMin(1.0, closure_amount));

        double eyelid_vertical_coverage = bell_height * closure_amount;

        painter->setBrush(QColor(20, 0, 0));
        painter->setPen(Qt::NoPen);

        painter->drawRect(QRectF(center.x() - radius, center.y() - bell_height, radius * 2, eyelid_vertical_coverage));

        painter->drawRect(QRectF(center.x() - radius, center.y() + bell_height - eyelid_vertical_coverage, radius * 2, eyelid_vertical_coverage));
    }
}


void JellyfishWidget::drawGoreTendrils(QPainter *painter, const QPointF& center, double radius, int num_tendrils, double current_t)
{
    for (int i = 0; i < num_tendrils; ++i) {
        double base_angle = i * 2 * M_PI / num_tendrils;
        QPointF startPoint(center.x() + radius * qCos(base_angle + qSin(current_t * 0.7 + i * 0.3) * 0.05),
                           center.y() + radius * qSin(base_angle + qCos(current_t * 0.7 + i * 0.3) * 0.05));

        int segments = 12;
        double tendril_length = 100.0 + 80.0 * qSin(current_t * 1.0 + i * 0.6);

        QPointF prevPoint = startPoint;
        for (int j = 1; j <= segments; ++j) {
            double segment_progress = (double)j / segments;
            double current_segment_length_factor = 0.5 + 0.5 * qCos(current_t * 2.0 + j * 0.9 + i * 0.4);
            double current_segment_length = tendril_length * segment_progress * current_segment_length_factor;

            double wave_amplitude = 25.0 * qSin(current_t * 2.5 + j * 1.0 + i * 0.3) * (1.0 - segment_progress * 0.5);
            double wave_angle_offset = qCos(base_angle + M_PI / 2.0 + qSin(current_t * 1.8));

            QPointF dir(qCos(base_angle), qSin(base_angle));
            QPointF normal_dir(-dir.y(), dir.x());

            QPointF nextPoint(startPoint.x() + dir.x() * current_segment_length + normal_dir.x() * wave_amplitude,
                              startPoint.y() + dir.y() * current_segment_length + normal_dir.y() * wave_amplitude);

            int alpha = 200 - j * 15;
            int pen_thickness = 2 + (segments - j) * 0.2;
            painter->setPen(QPen(QColor(180, 20, 20, qMax(0, alpha)), pen_thickness));

            painter->drawLine(prevPoint, nextPoint);
            prevPoint = nextPoint;

            if (j % 3 == 0) {
                painter->setBrush(QColor(100, 0, 0, qMax(0, alpha - 50)));
                painter->setPen(Qt::NoPen);
                painter->drawEllipse(nextPoint, pen_thickness + 1, pen_thickness + 1);
                painter->setBrush(Qt::NoBrush);
            }
        }
    }
}

void JellyfishWidget::drawFallingGoreAndBlood(QPainter *painter) {
    for (const GoreParticle &p : fallingGoreParticles) {
        QColor particleColor = p.color;
        particleColor.setAlphaF(p.opacity);

        painter->setBrush(particleColor);
        painter->setPen(Qt::NoPen);

        painter->drawEllipse(p.position, p.size, p.size);
    }
}

void JellyfishWidget::drawAcidDrips(QPainter *painter) {
    for (const AcidDrip &d : acidDrips) {
        QColor dripColor = d.color;
        dripColor.setAlphaF(d.opacity);
        painter->setBrush(dripColor);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(d.position, d.size, d.size);

        if (d.trailOpacity > 0.01) {
            QColor trailColor = d.color;
            trailColor.setAlphaF(d.trailOpacity * 0.5);
            QPen trailPen(trailColor, d.size / 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(trailPen);
            painter->drawLine(d.position - d.velocity * 3.0, d.position);
            painter->setPen(Qt::NoPen);
        }
    }
}

void JellyfishWidget::drawBackgroundTentacleShadows(QPainter *painter) {
    for (const TentacleShadow &shadow : backgroundTentacleShadows) {
        if (shadow.points.size() < 2) continue;

        QColor shadowColor = QColor(10, 0, 0, static_cast<int>(shadow.opacity * 255));
        QPen shadowPen(shadowColor, shadow.thickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(shadowPen);
        painter->setBrush(Qt::NoBrush);

        QPainterPath path;
        path.moveTo(shadow.points.first());

        for (int i = 1; i < shadow.points.size(); ++i) {
            path.lineTo(shadow.points[i]);
        }
        painter->drawPath(path);
    }
}
