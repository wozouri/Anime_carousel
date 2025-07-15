#ifndef WEBBUTTON_H
#define WEBBUTTON_H

#include <QGraphicsObject>
#include <QPropertyAnimation>
#include <QFont>
#include <QColor>
#include <QSizeF>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>

class WebButton : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress)

public:
    enum ButtonState {
        Normal,
        Hovered,
        Pressed,
        Dragging  // 新增拖拽状态
    };

    explicit WebButton(const QString& text = "Button", QGraphicsItem* parent = nullptr);
    ~WebButton();

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // 属性设置
    void setText(const QString& text);
    void setSize(const QSizeF& size);
    void setPrimaryColor(const QColor& color);

    // 拖拽相关设置
    void setDraggable(bool draggable) { m_draggable = draggable; }
    bool isDraggable() const { return m_draggable; }
    void setDragThreshold(int threshold) { m_dragThreshold = threshold; }
    int dragThreshold() const { return m_dragThreshold; }

    // 动画相关
    qreal hoverProgress() const { return m_hoverProgress; }
    void setHoverProgress(qreal progress);

signals:
    void clicked();
    void dragStarted(const QPointF& startPos);
    void dragging(const QPointF& currentPos, const QPointF& delta);
    void dragFinished(const QPointF& startPos, const QPointF& endPos);

protected:
    // 鼠标事件
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    // 悬停事件
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    void updateColors();
    QColor interpolateColor(const QColor& from, const QColor& to, qreal progress);
    bool shouldStartDrag(const QPointF& currentPos) const;

private:
    // 基本属性
    QString m_text;
    QSizeF m_size;
    QFont m_font;
    ButtonState m_state;
    int m_borderRadius;

    // 颜色
    QColor m_primaryColor;
    QColor m_hoverColor;
    QColor m_pressedColor;
    QColor m_draggingColor;  // 新增拖拽时的颜色
    QColor m_textColor;
    QColor m_currentBgColor;

    // 动画
    QPropertyAnimation* m_hoverAnimation;
    qreal m_hoverProgress;

    // 视觉效果
    QGraphicsDropShadowEffect* m_shadowEffect;

    // 拖拽相关
    bool m_draggable;           // 是否可拖拽
    bool m_isDragging;          // 当前是否在拖拽中
    QPointF m_dragStartPos;     // 拖拽开始位置（场景坐标）
    QPointF m_dragStartItemPos; // 拖拽开始时按钮的位置
    QPointF m_lastMousePos;     // 上次鼠标位置
    int m_dragThreshold;        // 拖拽阈值（像素）
    qreal m_dragOpacity;        // 拖拽时的透明度
};

#endif // WEBBUTTON_H
