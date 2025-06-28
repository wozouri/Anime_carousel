#ifndef ACCORDIONCARD_H
#define ACCORDIONCARD_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QEnterEvent>

class AccordionCard : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal currentStretch READ currentStretch WRITE setCurrentStretch)

public:
    explicit AccordionCard(const QString &text, const QString &imagePath, QWidget *parent = nullptr);

    qreal currentStretch() const;
    void setCurrentStretch(qreal stretch);

    void setTargetStretch(qreal targetStretch);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    // 添加 resizeEvent 以便在部件大小改变时清除缓存
    void resizeEvent(QResizeEvent *event) override;

signals:
    void hovered(AccordionCard *card);
    void left(AccordionCard *card);
    void stretchChanged(AccordionCard *card, qreal stretchValue);

private:
    QLabel *m_label;
    qreal m_currentStretch;
    QPropertyAnimation *m_stretchAnimation;
    QPixmap m_originalImage; // 存储原始图片
    QPixmap m_cachedScaledImage; // 缓存缩放后的图片
    QSize m_cachedImageSize; // 缓存缩放图片时的目标尺寸，用于判断是否需要重新缩放
};

#endif // ACCORDIONCARD_H
