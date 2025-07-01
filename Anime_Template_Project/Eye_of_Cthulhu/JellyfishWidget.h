#ifndef JELLYFISHWIDGET_H
#define JELLYFISHWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QDateTime>
#include <QPointF>
#include <QVector>
#include <QColor>
#include <QMouseEvent>
#include <QEvent>
#include <QPainterPath>
#include <QThreadPool>
#include <QRunnable>
#include <QRandomGenerator>

struct GoreParticle {
    QPointF position;
    QPointF velocity;
    double size;
    double opacity;
    QColor color;
    bool active;
};

struct AcidDrip {
    QPointF position;
    QPointF velocity;
    double size;
    double opacity;
    double trailOpacity;
    QColor color;
    bool active;
};

struct TentacleShadow {
    QVector<QPointF> points;
    double opacity;
    double phaseOffset;
    double thickness;
};

class ParticleUpdateRunnable : public QObject, public QRunnable {
    Q_OBJECT
public:
    ParticleUpdateRunnable(double t_current, int widgetHeight, int widgetWidth,
                           const QPointF& cthulhuCenter, double eyeball_radius_base,
                           QVector<GoreParticle> currentGore,
                           QVector<AcidDrip> currentAcid,
                           QVector<TentacleShadow> currentShadows)
        : m_t_current(t_current),
        m_widgetHeight(widgetHeight),
        m_widgetWidth(widgetWidth),
        m_cthulhuCenter(cthulhuCenter),
        m_eyeball_radius_base(eyeball_radius_base),
        m_goreParticles(currentGore),
        m_acidDrips(currentAcid),
        m_tentacleShadows(currentShadows) {
        setAutoDelete(true);
    }

    void run() override {
        for (int i = 0; i < m_goreParticles.size(); ) {
            GoreParticle &p = m_goreParticles[i];
            p.position += p.velocity;
            p.velocity.setY(p.velocity.y() + 0.2);
            p.opacity -= 0.01;
            p.size *= 0.99;

            if (p.position.y() > m_widgetHeight + 50 || p.opacity <= 0.01) {
                m_goreParticles.removeAt(i);
            } else {
                ++i;
            }
        }
        if (QRandomGenerator::global()->generateDouble() < 0.2) {
            GoreParticle newParticle;
            newParticle.position = QPointF(m_cthulhuCenter.x() + QRandomGenerator::global()->bounded(-50, 51),
                                           m_cthulhuCenter.y() + m_eyeball_radius_base + QRandomGenerator::global()->bounded(10, 31));

            newParticle.velocity = QPointF(QRandomGenerator::global()->generateDouble() * 2.0 - 1.0,
                                           QRandomGenerator::global()->generateDouble() * 3.0 + 1.0);

            newParticle.size = QRandomGenerator::global()->bounded(3, 11);
            newParticle.opacity = QRandomGenerator::global()->generateDouble() * 0.2 + 0.8;

            if (QRandomGenerator::global()->generateDouble() < 0.7) {
                newParticle.color = QColor(150, 0, 0);
            } else {
                newParticle.color = QColor(80, 20, 20);
            }
            m_goreParticles.append(newParticle);
        }

        for (int i = 0; i < m_acidDrips.size(); ) {
            AcidDrip &d = m_acidDrips[i];
            d.position += d.velocity;
            d.velocity.setY(d.velocity.y() + 0.1);
            d.opacity -= 0.008;
            d.trailOpacity -= 0.012;
            d.size *= 0.995;

            if (d.position.y() > m_widgetHeight + 50 || d.opacity <= 0.01) {
                m_acidDrips.removeAt(i);
            } else {
                ++i;
            }
        }
        if (QRandomGenerator::global()->generateDouble() < 0.05) {
            AcidDrip newDrip;
            newDrip.position = QPointF(m_cthulhuCenter.x() + QRandomGenerator::global()->bounded(-60, 61),
                                       m_cthulhuCenter.y() + m_eyeball_radius_base * 0.8 + QRandomGenerator::global()->bounded(0, 20));

            newDrip.velocity = QPointF(QRandomGenerator::global()->generateDouble() * 1.0 - 0.5,
                                       QRandomGenerator::global()->generateDouble() * 1.5 + 0.5);

            newDrip.size = QRandomGenerator::global()->bounded(4, 8);
            newDrip.opacity = QRandomGenerator::global()->generateDouble() * 0.3 + 0.7;
            newDrip.trailOpacity = 0.5;
            newDrip.color = QColor(50, 150, 50, 255);
            m_acidDrips.append(newDrip);
        }

        for (TentacleShadow &shadow : m_tentacleShadows) {
            for (int i = 0; i < shadow.points.size(); ++i) {
                shadow.points[i].setX(shadow.points[i].x() + qSin(m_t_current * 0.1 + shadow.phaseOffset + i * 0.5) * 0.5);
                shadow.points[i].setY(shadow.points[i].y() + qCos(m_t_current * 0.08 + shadow.phaseOffset + i * 0.7) * 0.5);

                if (shadow.points[i].x() < -200) shadow.points[i].setX(m_widgetWidth + 200);
                else if (shadow.points[i].x() > m_widgetWidth + 200) shadow.points[i].setX(-200);
                if (shadow.points[i].y() < -200) shadow.points[i].setY(m_widgetHeight + 200);
                else if (shadow.points[i].y() > m_widgetHeight + 200) shadow.points[i].setY(-200);
            }
        }

        emit particlesUpdated(m_goreParticles, m_acidDrips, m_tentacleShadows);
    }

signals:
    void particlesUpdated(QVector<GoreParticle> newGore, QVector<AcidDrip> newAcid, QVector<TentacleShadow> newShadows);

private:
    double m_t_current;
    int m_widgetHeight;
    int m_widgetWidth;
    QPointF m_cthulhuCenter;
    double m_eyeball_radius_base;
    QVector<GoreParticle> m_goreParticles;
    QVector<AcidDrip> m_acidDrips;
    QVector<TentacleShadow> m_tentacleShadows;
};


class JellyfishWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JellyfishWidget(QWidget *parent = nullptr);
    ~JellyfishWidget();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void updateAnimation();
    void handleParticlesUpdated(QVector<GoreParticle> newGore,
                                QVector<AcidDrip> newAcid,
                                QVector<TentacleShadow> newShadows);

private:
    double x_input;
    double y_input;

    double t_current;
    QTimer *animationTimer;

    QPointF mousePosition;
    QPointF smoothedPupilTarget;

    bool mouseIsInWindow;

    double blinkProgress;
    bool isBlinking;

    QVector<GoreParticle> fallingGoreParticles;
    QVector<AcidDrip> acidDrips;
    QVector<TentacleShadow> backgroundTentacleShadows;

    double pulseScale;
    double m_targetPupilScaleFactor;
    double m_currentPupilScaleFactor;

    QPen eyeballBorderPen;
    QBrush eyeballGradientBrush;
    QPen irisBorderPen;
    QBrush irisGradientBrush;
    QPen pupilPen;
    QBrush pupilBrush;
    QBrush eyelidBrush;
    QPen goreTendrilPen;
    QBrush goreTendrilBrush;
    QPen fallingGorePen;
    QBrush fallingGoreBrush;
    QPen acidDripPen;
    QBrush acidDripBrush;
    QPen acidDripTrailPen;
    QPen backgroundShadowPen;

    QPointF calculateJellyfishPoint(double local_x, double local_y, double current_t);

    void drawEyeball(QPainter *painter, const QPointF& center, double radius, double eyeball_height);
    void drawGoreTendrils(QPainter *painter, const QPointF& center, double radius, int num_tentacles, double current_t);
    void drawFallingGoreAndBlood(QPainter *painter);
    void drawAcidDrips(QPainter *painter);
    void drawBackgroundTentacleShadows(QPainter *painter);
};

#endif
