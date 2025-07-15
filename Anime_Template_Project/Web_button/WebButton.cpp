#include "WebButton.h"
#include <QGraphicsDropShadowEffect>
#include <QLinearGradient>
#include <QPainterPath>
#include <QGraphicsScene>
#include <QDebug>
#include <QtMath>

WebButton::WebButton(const QString& text, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_text(text)
    , m_size(120, 40)
    , m_state(Normal)
    , m_hoverProgress(0.0)
    , m_borderRadius(8)
    , m_draggable(true)          // 默认可拖拽
    , m_isDragging(false)
    , m_dragThreshold(5)         // 5像素阈值
    , m_dragOpacity(0.8)         // 拖拽时80%透明度
{
    // 启用鼠标事件和 hover 事件
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    // 设置字体
    m_font = QFont("Arial", 10, QFont::Medium);

    // 设置默认颜色主题（仿现代 Web 按钮）
    setPrimaryColor(QColor(59, 130, 246)); // 蓝色主题

    // 创建阴影效果
    m_shadowEffect = new QGraphicsDropShadowEffect();
    m_shadowEffect->setBlurRadius(8);
    m_shadowEffect->setOffset(0, 2);
    m_shadowEffect->setColor(QColor(0, 0, 0, 80));
    setGraphicsEffect(m_shadowEffect);

    // 创建 hover 动画
    m_hoverAnimation = new QPropertyAnimation(this, "hoverProgress");
    m_hoverAnimation->setDuration(150); // 150ms 平滑过渡
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);

    updateColors();
}

WebButton::~WebButton()
{
    delete m_hoverAnimation;
}

QRectF WebButton::boundingRect() const
{
    // 为阴影效果留出额外空间
    return QRectF(-5, -5, m_size.width() + 10, m_size.height() + 10);
}

void WebButton::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHint(QPainter::Antialiasing);

    // 拖拽时设置透明度
    if (m_isDragging) {
        painter->setOpacity(m_dragOpacity);
    }

    // 按钮区域
    QRectF buttonRect(0, 0, m_size.width(), m_size.height());

    // 创建圆角路径
    QPainterPath path;
    path.addRoundedRect(buttonRect, m_borderRadius, m_borderRadius);

    // 绘制渐变背景
    QLinearGradient gradient(0, 0, 0, m_size.height());
    gradient.setColorAt(0, m_currentBgColor.lighter(110));
    gradient.setColorAt(1, m_currentBgColor.darker(110));

    painter->fillPath(path, gradient);

    // 绘制边框
    QPen borderPen(m_currentBgColor.darker(120), m_isDragging ? 2 : 1);
    if (m_isDragging) {
        borderPen.setStyle(Qt::DashLine); // 拖拽时使用虚线边框
    }
    painter->setPen(borderPen);
    painter->drawPath(path);

    // 绘制文本
    painter->setPen(m_textColor);
    painter->setFont(m_font);

    QFontMetrics metrics(m_font);
    QRect textRect = metrics.boundingRect(m_text);

    // 文本居中
    int x = (m_size.width() - textRect.width()) / 2;
    int y = (m_size.height() + textRect.height()) / 2 - metrics.descent();

    painter->drawText(x, y, m_text);

    // 拖拽时显示额外的视觉提示
    if (m_isDragging) {
        painter->setOpacity(0.3);
        QPen dragIndicator(Qt::white, 1, Qt::DotLine);
        painter->setPen(dragIndicator);
        painter->drawEllipse(buttonRect.center(), 3, 3);
    }
}

void WebButton::setText(const QString& text)
{
    m_text = text;
    update();
}

void WebButton::setSize(const QSizeF& size)
{
    prepareGeometryChange();
    m_size = size;
    update();
}

void WebButton::setPrimaryColor(const QColor& color)
{
    m_primaryColor = color;
    m_hoverColor = color.lighter(115);
    m_pressedColor = color.darker(115);
    m_draggingColor = color.lighter(130); // 拖拽时更亮的颜色
    m_textColor = Qt::white;
    updateColors();
}

void WebButton::setHoverProgress(qreal progress)
{
    m_hoverProgress = qBound(0.0, progress, 1.0);
    updateColors();
    update();
}

void WebButton::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_state = Pressed;

        // 记录拖拽相关信息
        m_dragStartPos = event->scenePos();
        m_dragStartItemPos = pos();
        m_lastMousePos = event->scenePos();
        m_isDragging = false;

        updateColors();
        update();
        event->accept();
        return;
    }
    QGraphicsObject::mousePressEvent(event);
}

void WebButton::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_state == Pressed || m_isDragging) {
        if (!m_isDragging && m_draggable && shouldStartDrag(event->scenePos())) {
            // 开始拖拽
            m_isDragging = true;
            m_state = Dragging;

            // 增强阴影效果
            m_shadowEffect->setBlurRadius(16);
            m_shadowEffect->setOffset(0, 6);
            m_shadowEffect->setColor(QColor(0, 0, 0, 120));

            updateColors();
            emit dragStarted(m_dragStartPos);

            // 将按钮提升到最前面
            if (scene()) {
                setZValue(scene()->items().size() + 1);
            }
        }

        if (m_isDragging) {
            // 计算移动增量
            QPointF delta = event->scenePos() - m_lastMousePos;
            QPointF newPos = pos() + delta;

            setPos(newPos);
            emit dragging(event->scenePos(), delta);

            m_lastMousePos = event->scenePos();
            update();
        }

        event->accept();
        return;
    }

    QGraphicsObject::mouseMoveEvent(event);
}

void WebButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && (m_state == Pressed || m_isDragging)) {
        if (m_isDragging) {
            // 结束拖拽
            m_isDragging = false;
            m_state = Normal;

            // 恢复正常阴影
            m_shadowEffect->setBlurRadius(8);
            m_shadowEffect->setOffset(0, 2);
            m_shadowEffect->setColor(QColor(0, 0, 0, 80));

            // 恢复正常层级
            setZValue(0);

            updateColors();
            emit dragFinished(m_dragStartPos, event->scenePos());

        } else {
            // 这是一个点击操作
            m_state = Hovered;
            updateColors();

            // 检查是否在按钮范围内释放
            QRectF buttonArea(0, 0, m_size.width(), m_size.height());
            if (buttonArea.contains(event->pos())) {
                emit clicked();
            }
        }

        update();
        event->accept();
        return;
    }

    QGraphicsObject::mouseReleaseEvent(event);
}

void WebButton::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event)

    if (!m_isDragging) {
        m_state = Hovered;

        // 启动 hover 动画
        m_hoverAnimation->stop();
        m_hoverAnimation->setStartValue(m_hoverProgress);
        m_hoverAnimation->setEndValue(1.0);
        m_hoverAnimation->start();

        // 增强阴影效果
        if (!m_isDragging) {
            m_shadowEffect->setBlurRadius(12);
            m_shadowEffect->setOffset(0, 4);
        }
    }
}

void WebButton::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event)

    if (!m_isDragging) {
        m_state = Normal;

        // 启动 hover 退出动画
        m_hoverAnimation->stop();
        m_hoverAnimation->setStartValue(m_hoverProgress);
        m_hoverAnimation->setEndValue(0.0);
        m_hoverAnimation->start();

        // 恢复正常阴影
        if (!m_isDragging) {
            m_shadowEffect->setBlurRadius(8);
            m_shadowEffect->setOffset(0, 2);
        }
    }
}

void WebButton::updateColors()
{
    switch (m_state) {
    case Normal:
        m_currentBgColor = interpolateColor(m_primaryColor, m_hoverColor, m_hoverProgress);
        break;
    case Hovered:
        m_currentBgColor = interpolateColor(m_primaryColor, m_hoverColor, m_hoverProgress);
        break;
    case Pressed:
        m_currentBgColor = m_pressedColor;
        break;
    case Dragging:
        m_currentBgColor = m_draggingColor;
        break;
    }
}

QColor WebButton::interpolateColor(const QColor& from, const QColor& to, qreal progress)
{
    int r = from.red() + (to.red() - from.red()) * progress;
    int g = from.green() + (to.green() - from.green()) * progress;
    int b = from.blue() + (to.blue() - from.blue()) * progress;
    int a = from.alpha() + (to.alpha() - from.alpha()) * progress;

    return QColor(r, g, b, a);
}

bool WebButton::shouldStartDrag(const QPointF& currentPos) const
{
    QPointF delta = currentPos - m_dragStartPos;
    qreal distance = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
    return distance > m_dragThreshold;
}

#include "WebButton.moc"
