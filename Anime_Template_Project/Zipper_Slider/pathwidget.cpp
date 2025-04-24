#include "PathWidget.h"

PathWidget::PathWidget(QWidget *parent)
    : QWidget(parent)
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor(224, 213, 185, 255));
    this->setPalette(palette);
    resize(1200, 1200);
}

void PathWidget::setOffset(qreal offset)
{
    yy3 -= offset;
    xx = (yy3 - y1) / 2;
    yy = yy3 - y1;
    update();
}

void PathWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    QPainterPath path;
    path.moveTo(xx3, yy3);
    path.cubicTo(xx3 + xx * 0.05, yy3 - yy * 0.1, xx3 + xx * 0.15, yy3 - yy * 0.5, xx3 + xx, y1);
    painter.drawPath(path);

    QPainterPath path1;
    path1.moveTo(xx3, yy3);
    path1.cubicTo(xx3 - xx * 0.05, yy3 - yy * 0.1, xx3 - xx * 0.15, yy3 - yy * 0.5, xx3 - xx, y1);
    painter.drawPath(path1);

    painter.drawLine(xxx3, yyy3, xx3, yy3);
    painter.setBrush(QColor(100, 104, 79, 255));

    QPainterPath linePath;
    linePath.moveTo(xxx3, yyy3);
    linePath.lineTo(xx3, yy3);
    Draw_Straight_Path_Square(painter, linePath);
    Draw_Curve_Path_Square(painter, path);
    Draw_Curve_Path_Square(painter, path1);
    Draw_Rectangle(painter);
}

void PathWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragStartPosition = event->globalPos();
        m_startWindowPosition = this->pos();
        m_lastMovePosition = event->globalPos();
        event->accept();
    }
    else QWidget::mousePressEvent(event);
}

void PathWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        QPoint delta = event->globalPos() - m_lastMovePosition;
        m_totalMoveDistance += delta;

        if (m_totalMoveDistance.y() > 30)
        {
            this->setOffset(-10);
            if (yy3 > yyy3 - 10) yy3 = 600;
            m_totalMoveDistance = QPoint(0, 0);
            m_lastMovePosition = event->globalPos();
        }
        else if (m_totalMoveDistance.y() < -30)
        {
            this->setOffset(10);
            if (yy3 < y1 + 10) yy3 = 310;
            m_totalMoveDistance = QPoint(0, 0);
            m_lastMovePosition = event->globalPos();
        }

        event->accept();
    }
    else QWidget::mouseMoveEvent(event);
}

void PathWidget::Draw_Straight_Path_Square(QPainter& painter, const QPainterPath& path)
{
    const qreal step = 10.0;
    const qreal totalLength = path.length();
    if (totalLength <= 0) return;

    for (qreal d = 0; d <= totalLength; d += step) {
        const qreal t = path.percentAtLength(d);
        const QPointF posx = path.pointAtPercent(t);
        const QPointF posy(xxx3, yyy3 - d);
        painter.drawRect(posx.x() - 5, posy.y() - 1, 10, 2);
    }
}

void PathWidget::Draw_Curve_Path_Square(QPainter& painter, const QPainterPath& path)
{
    const qreal step = 10.0;
    const qreal totalLength = yy;
    if (totalLength <= 0) return;

    for (qreal d = 0; d <= totalLength; d += step)
    {
        const qreal t = path.percentAtLength(d);
        const QPointF posx = path.pointAtPercent(t);
        const QPointF posy(xxx3, yy3 - d);
        painter.drawRect(posx.x() - 5, posy.y() - 2, 10, 2);
    }
}

void PathWidget::Draw_Rectangle(QPainter &painter)
{
    painter.setBrush(QColor(0, 0, 0, 255));
    painter.drawRect(xx3 - 15, yy3, 30, 10);
}
