#include "AccordionCard.h"
#include <QDebug>
#include <QPalette>
#include <QVariant>
#include <QPainter>
#include <QResizeEvent> // 用于 resizeEvent

// AccordionCard 类的构造函数
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

    m_originalImage.load(imagePath); // 加载原始图片到 m_originalImage
    if (m_originalImage.isNull()) {
        qWarning() << "Failed to load image for AccordionCard from path:" << imagePath;
    }

    // 初始状态下清空缓存，让第一次绘制时进行缩放
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

void AccordionCard::enterEvent(QEnterEvent *event)
{
    emit hovered(this);
    QWidget::enterEvent(event);
}

void AccordionCard::leaveEvent(QEvent *event)
{
    emit left(this);
    QWidget::leaveEvent(event);
}

// 核心优化：在 paintEvent 中缓存缩放后的图片
void AccordionCard::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int w = width();
    int h = height();
    int cornerRadius = 5;

    // 裁剪圆角
    QPainterPath path;
    path.addRoundedRect(0, 0, w, h, cornerRadius, cornerRadius);
    painter.setClipPath(path);

    // 绘制背景
    painter.fillRect(rect(), QColor(100, 100, 100, 255));

    // 绘制图片
    if (!m_originalImage.isNull()) {
        // 判断当前卡片尺寸与缓存尺寸是否一致，如果不一致则重新缩放并缓存
        if (m_cachedImageSize != QSize(w, h)) {
            QSize imageSize = m_originalImage.size();
            qreal imageAspectRatio = (qreal)imageSize.width() / imageSize.height();
            qreal cardAspectRatio = (qreal)w / h;

            QSize scaledTargetSize;
            if (imageAspectRatio > cardAspectRatio) {
                // 图片更宽（更扁），按高度缩放以填充卡片高度，宽度超出
                scaledTargetSize = QSize(qRound(h * imageAspectRatio), h);
            } else {
                // 图片更高（更瘦）或比例相同，按宽度缩放以填充卡片宽度，高度超出
                scaledTargetSize = QSize(w, qRound(w / imageAspectRatio));
            }

            m_cachedScaledImage = m_originalImage.scaled(scaledTargetSize,
                                                         Qt::KeepAspectRatio,
                                                         Qt::SmoothTransformation);
            m_cachedImageSize = QSize(w, h); // 更新缓存尺寸
        }

        // 计算绘制图片的起始位置，使其在卡片内部居中
        int imageX = (w - m_cachedScaledImage.width()) / 2;
        int imageY = (h - m_cachedScaledImage.height()) / 2;

        painter.drawPixmap(imageX, imageY, m_cachedScaledImage); // 使用缓存的图片进行绘制
    }
    // QWidget::paintEvent(event); // 当完全自定义绘制时，这行通常可以省略
}

// 添加 resizeEvent，当部件尺寸改变时，强制清除图片缓存，以便下次绘制时重新缩放
void AccordionCard::resizeEvent(QResizeEvent *event)
{
    // 只有当尺寸确实改变时才清除缓存
    if (event->oldSize() != event->size()) {
        m_cachedScaledImage = QPixmap(); // 清空缓存
        m_cachedImageSize = QSize(0,0); // 重置缓存尺寸
    }
    QWidget::resizeEvent(event); // 调用基类的 resizeEvent
}
