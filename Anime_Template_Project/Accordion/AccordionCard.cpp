#include "AccordionCard.h"
#include <QDebug>
#include <QPalette>
#include <QVariant>
#include <QPainter>
#include <QResizeEvent>
AccordionCard::AccordionCard(const QString &text, const QString &imagePath, QWidget *parent)
    : QWidget(parent),
    m_currentStretch(1.0)
{
    m_label = new QLabel(text, this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->hide();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_label);
    setLayout(layout);

    setMinimumWidth(50);

    m_stretchAnimation = new QPropertyAnimation(this, "currentStretch", this);
    m_stretchAnimation->setDuration(280);
    m_stretchAnimation->setEasingCurve(QEasingCurve::OutQuad);

    connect(m_stretchAnimation, &QPropertyAnimation::valueChanged,
            [this](const QVariant &value){
                emit stretchChanged(this, value.toReal());
            });

    m_originalImage.load(imagePath); 
    if (m_originalImage.isNull()) {
        qWarning() << "Failed to load image for AccordionCard from path:" << imagePath;
    }

    m_cachedScaledImage = QPixmap();
    m_cachedImageSize = QSize(0,0);
}

qreal AccordionCard::currentStretch() const
{
    return m_currentStretch;
}

void AccordionCard::setCurrentStretch(qreal stretch)
{
    if (qFuzzyCompare(m_currentStretch, stretch))
        return;

    m_currentStretch = stretch;
    updateGeometry();
}

void AccordionCard::setTargetStretch(qreal targetStretch)
{
    if (m_stretchAnimation->state() == QAbstractAnimation::Running) {
        m_stretchAnimation->stop();
    }
    m_stretchAnimation->setStartValue(m_currentStretch);
    m_stretchAnimation->setEndValue(targetStretch);
    m_stretchAnimation->start();
}

#if QT_VERSION_MAJOR >= 6
void AccordionCard::enterEvent(QEnterEvent *event)
{
    emit hovered(this);
    QWidget::enterEvent(event);
}
#elif QT_VERSION_MAJOR >= 5
void AccordionCard::enterEvent(QEvent* event)
{
    emit hovered(this);
    QWidget::enterEvent(event);
}
#endif


void AccordionCard::leaveEvent(QEvent *event)
{
    emit left(this);
    QWidget::leaveEvent(event);
}

void AccordionCard::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int w = width();
    int h = height();
    int cornerRadius = 5;

    QPainterPath path;
    path.addRoundedRect(0, 0, w, h, cornerRadius, cornerRadius);
    painter.setClipPath(path);

    painter.fillRect(rect(), QColor(100, 100, 100, 255));

    if (!m_originalImage.isNull())
    {
        if (m_cachedImageSize != QSize(w, h)) 
        {
            QSize imageSize = m_originalImage.size();
            qreal imageAspectRatio = (qreal)imageSize.width() / imageSize.height();
            qreal cardAspectRatio = (qreal)w / h;

            QSize scaledTargetSize;
            if (imageAspectRatio > cardAspectRatio) 
            {
                scaledTargetSize = QSize(qRound(h * imageAspectRatio), h);
            } 
            else 
            {
                scaledTargetSize = QSize(w, qRound(w / imageAspectRatio));
            }

            m_cachedScaledImage = m_originalImage.scaled(scaledTargetSize,
                                                         Qt::KeepAspectRatio,
                                                         Qt::SmoothTransformation);
            m_cachedImageSize = QSize(w, h); 
        }

        int imageX = (w - m_cachedScaledImage.width()) / 2;
        int imageY = (h - m_cachedScaledImage.height()) / 2;

        painter.drawPixmap(imageX, imageY, m_cachedScaledImage);
    }
}

void AccordionCard::resizeEvent(QResizeEvent *event)
{
    if (event->oldSize() != event->size()) {
        m_cachedScaledImage = QPixmap();
        m_cachedImageSize = QSize(0,0);
    }
    QWidget::resizeEvent(event);
}
