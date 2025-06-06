#include "GlitchTextWidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QAbstractAnimation>

GlitchTextWidget::GlitchTextWidget(const QString& text, QWidget *parent)
    : QWidget(parent),
    m_text(text),
    m_speed(1.0),
    m_enableShadows(true),
    m_enableOnHover(false),
    m_glitchFrameIndex(0),
    m_isMouseOver(false)
{
    QFontDatabase::addApplicationFont("://Cyberpunk.ttf");
    setFont(QFont("Cyberpunk", 72, QFont::Black));

    setMouseTracking(true);

    m_animationAfter = new QPropertyAnimation(this, "clipRectAfter");
    m_animationBefore = new QPropertyAnimation(this, "clipRectBefore");

    m_animationAfter->setLoopCount(-1);
    m_animationBefore->setLoopCount(-1);

    m_animationAfter->setEasingCurve(QEasingCurve::Linear);
    m_animationBefore->setEasingCurve(QEasingCurve::Linear);

    m_glitchTimer = new QTimer(this);
    connect(m_glitchTimer, &QTimer::timeout, this, [this](){
        if (m_clipKeyframes.isEmpty()) {
            updateClipKeyframes();
        }

        if (!m_clipKeyframes.isEmpty()) {
            QMargins margins = m_clipKeyframes[m_glitchFrameIndex];

            int clipY = (height() * margins.top()) / 100;
            int clipHeight = height() - clipY - (height() * margins.bottom()) / 100;

            m_clipRectAfter = QRect(0, clipY, width(), clipHeight);
            m_clipRectBefore = QRect(0, clipY, width(), clipHeight);

            m_glitchFrameIndex = (m_glitchFrameIndex + 1) % m_clipKeyframes.size();
            update();
        }
    });

    setupAnimations();
    if (!m_enableOnHover) {
        startGlitchAnimation();
    }
}

void GlitchTextWidget::setText(const QString& text) {
    if (m_text != text) {
        m_text = text;
        update();
        updateGeometry();
    }
}

void GlitchTextWidget::setSpeed(qreal speed) {
    if (m_speed != speed) {
        m_speed = speed;
        setupAnimations();
        if (!m_enableOnHover && m_glitchTimer->isActive()) {
            startGlitchAnimation();
        } else if (m_enableOnHover && m_isMouseOver) {
            startGlitchAnimation();
        }
    }
}

void GlitchTextWidget::setEnableShadows(bool enable) {
    if (m_enableShadows != enable) {
        m_enableShadows = enable;
        update();
    }
}

void GlitchTextWidget::setEnableOnHover(bool enable) {
    if (m_enableOnHover != enable) {
        m_enableOnHover = enable;
        if (m_enableOnHover) {
            stopGlitchAnimation();
        } else {
            startGlitchAnimation();
        }
        update();
    }
}

void GlitchTextWidget::setCustomClassName(const QString& className) {
    m_customClassName = className;
}

void GlitchTextWidget::updateClipKeyframes() {
    m_clipKeyframes.clear();
    m_clipKeyframes.append(QMargins(0, 20, 0, 50));
    m_clipKeyframes.append(QMargins(0, 10, 0, 60));
    m_clipKeyframes.append(QMargins(0, 15, 0, 55));
    m_clipKeyframes.append(QMargins(0, 25, 0, 35));
    m_clipKeyframes.append(QMargins(0, 30, 0, 40));
    m_clipKeyframes.append(QMargins(0, 40, 0, 20));
    m_clipKeyframes.append(QMargins(0, 10, 0, 60));
    m_clipKeyframes.append(QMargins(0, 15, 0, 55));
    m_clipKeyframes.append(QMargins(0, 25, 0, 35));
    m_clipKeyframes.append(QMargins(0, 00, 0, 40));
    m_clipKeyframes.append(QMargins(0, 20, 0, 50));
    m_clipKeyframes.append(QMargins(0, 10, 0, 60));
    m_clipKeyframes.append(QMargins(0, 15, 0, 55));
    m_clipKeyframes.append(QMargins(0, 25, 0, 35));
    m_clipKeyframes.append(QMargins(0, 30, 0, 111));
    m_clipKeyframes.append(QMargins(0, 40, 0, 20));
    m_clipKeyframes.append(QMargins(0, 20, 0, 50));
    m_clipKeyframes.append(QMargins(0, 10, 0, 60));
    m_clipKeyframes.append(QMargins(0, 15, 0, 55));
    m_clipKeyframes.append(QMargins(0, 25, 0, 35));
    m_clipKeyframes.append(QMargins(0, 30, 0, 40));
}

void GlitchTextWidget::setupAnimations() {
    updateClipKeyframes();

    int totalAfterDurationMs = static_cast<int>(m_speed * 3000);
    int totalBeforeDurationMs = static_cast<int>(m_speed * 2000);

    if (m_clipKeyframes.isEmpty()) {
        m_glitchTimer->setInterval(30);
    } else {
        m_glitchTimer->setInterval(totalAfterDurationMs / m_clipKeyframes.size());
    }

    m_animationAfter->setDuration(totalAfterDurationMs);
    m_animationBefore->setDuration(totalBeforeDurationMs);
}

void GlitchTextWidget::startGlitchAnimation() {
    if (!m_glitchTimer->isActive()) {
        m_glitchFrameIndex = 0;
        m_glitchTimer->start();
    }
}

void GlitchTextWidget::stopGlitchAnimation() {
    if (m_glitchTimer->isActive()) {
        m_glitchTimer->stop();
        m_clipRectAfter = rect();
        m_clipRectBefore = rect();
        update();
    }
}

void GlitchTextWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(font());

    painter.fillRect(rect(), QColor("#060606"));

    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(m_text);
    int textHeight = fm.height();
    int textX = (width() - textWidth) / 2;
    int textY = (height() - textHeight) / 2 + fm.ascent();

    painter.setPen(QColor(Qt::white));
    painter.drawText(textX, textY, m_text);

    if (!m_enableOnHover || (m_enableOnHover && m_isMouseOver)) {
        painter.save();
        painter.setPen(QColor(Qt::red));
        painter.setClipRect(m_clipRectAfter);
        painter.drawText(textX - 10, textY, m_text);
        painter.restore();

        painter.save();
        painter.setPen(QColor(Qt::cyan));
        painter.setClipRect(m_clipRectBefore);
        painter.drawText(textX + 10, textY, m_text);
        painter.restore();
    }
}

void GlitchTextWidget::enterEvent(QEnterEvent *event) {
    m_isMouseOver = true;
    if (m_enableOnHover) {
        startGlitchAnimation();
    }
    QWidget::enterEvent(event);
}

void GlitchTextWidget::leaveEvent(QEvent *event) {
    m_isMouseOver = false;
    if (m_enableOnHover) {
        stopGlitchAnimation();
    }
    QWidget::leaveEvent(event);
}

QSize GlitchTextWidget::sizeHint() const {
    QFontMetrics fm(font());
    int widthHint = fm.horizontalAdvance(m_text) + 20;
    int heightHint = fm.height() + 20;
    return QSize(widthHint, heightHint);
}

QRect GlitchTextWidget::clipRectAfter() const { return m_clipRectAfter; }
void GlitchTextWidget::setClipRectAfter(const QRect& rect) { m_clipRectAfter = rect; update(); }

QRect GlitchTextWidget::clipRectBefore() const { return m_clipRectBefore; }
void GlitchTextWidget::setClipRectBefore(const QRect& rect) { m_clipRectBefore = rect; update(); }
