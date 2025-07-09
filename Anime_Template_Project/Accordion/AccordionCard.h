#ifndef ACCORDIONCARD_H
#define ACCORDIONCARD_H
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

// Qt版本适配宏
#if QT_VERSION_MAJOR >= 6
#include <QEnterEvent>
#endif

class AccordionCard : public QWidget
{
    Q_OBJECT
        Q_PROPERTY(qreal currentStretch READ currentStretch WRITE setCurrentStretch)
public:
    explicit AccordionCard(const QString& text, const QString& imagePath, QWidget* parent = nullptr);
    qreal currentStretch() const;
    void setCurrentStretch(qreal stretch);
    void setTargetStretch(qreal targetStretch);
protected:
#if QT_VERSION_MAJOR >= 6
    void enterEvent(QEnterEvent* event) override;
#elif QT_VERSION_MAJOR >= 5
    void enterEvent(QEvent* event) override;
#endif

    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    // 添加 resizeEvent 以便在部件大小改变时清除缓存
    void resizeEvent(QResizeEvent* event) override;
signals:
    void hovered(AccordionCard* card);
    void left(AccordionCard* card);
    void stretchChanged(AccordionCard* card, qreal stretchValue);
private:
    QLabel* m_label;
    qreal m_currentStretch;
    QPropertyAnimation* m_stretchAnimation;
    QPixmap m_originalImage;
    QPixmap m_cachedScaledImage;
    QSize m_cachedImageSize;
};
#endif // ACCORDIONCARD_H