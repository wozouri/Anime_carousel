#ifndef GLITCHTEXTWIDGET_H
#define GLITCHTEXTWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QTimer>
#include <QFontMetrics>
#include <QEnterEvent>
#include <QAbstractAnimation>
#include <QFontDatabase>
#include <QDebug> 

class GlitchTextWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QRect clipRectAfter READ clipRectAfter WRITE setClipRectAfter)
    Q_PROPERTY(QRect clipRectBefore READ clipRectBefore WRITE setClipRectBefore)

public:
    explicit GlitchTextWidget(const QString& text, QWidget *parent = nullptr);

    void setText(const QString& text);
    void setSpeed(qreal speed);
    void setEnableShadows(bool enable);
    void setEnableOnHover(bool enable);
    void setCustomClassName(const QString& className);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    QSize sizeHint() const override;

private:
    QString m_text;
    qreal m_speed;
    bool m_enableShadows;
    bool m_enableOnHover;
    QString m_customClassName;
    QPropertyAnimation *m_animationAfter;
    QPropertyAnimation *m_animationBefore;
    QTimer *m_glitchTimer;
    int m_glitchFrameIndex;
    QList<QMargins> m_clipKeyframes;
    bool m_isMouseOver;

    QRect m_clipRectAfter;
    QRect m_clipRectBefore;

    QRect clipRectAfter() const;
    void setClipRectAfter(const QRect& rect);

    QRect clipRectBefore() const;
    void setClipRectBefore(const QRect& rect);

    void setupAnimations();
    void startGlitchAnimation();
    void stopGlitchAnimation();
    void updateClipKeyframes();
};

#endif // GLITCHTEXTWIDGET_H
