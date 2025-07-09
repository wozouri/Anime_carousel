#include "RotatableLinesWidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <cmath>
#include <QCursor>

RotatableLinesWidget::RotatableLinesWidget(QWidget *parent)
    : QWidget(parent),
    m_linesPerRow(10),
    m_linesPerCol(10),
    m_baseRotationAngle(0),
    m_globalMousePos(-1, -1),
    m_padding(20)
{
    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, QColor(0,0,0,255));
    this->setPalette(pal);

    setMinimumSize(100, 100);

    m_mousePollTimer = new QTimer(this);
    connect(m_mousePollTimer, &QTimer::timeout, this, &RotatableLinesWidget::updateMousePosition);
    m_mousePollTimer->start(5);
}

RotatableLinesWidget::~RotatableLinesWidget()
{
    if (m_mousePollTimer->isActive())
    {
        m_mousePollTimer->stop();
    }
}

void RotatableLinesWidget::setLinesPerRow(int count)
{
    if (m_linesPerRow != count && count > 0) {
        m_linesPerRow = count;
        update();
    }
}

void RotatableLinesWidget::setLinesPerCol(int count)
{
    if (m_linesPerCol != count && count > 0) {
        m_linesPerCol = count;
        update();
    }
}

void RotatableLinesWidget::setRotationAngle(int angle)
{
    if (m_baseRotationAngle != angle) {
        m_baseRotationAngle = angle;
    }
}

void RotatableLinesWidget::updateMousePosition()
{
    QPoint globalPos = QCursor::pos();
    if (globalPos != m_globalMousePos) {
        m_globalMousePos = globalPos;
        update();
    }
}

void RotatableLinesWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    update();
    QWidget::resizeEvent(event);
}

void RotatableLinesWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QColor("#00bcd4"));
    pen.setWidth(2);
    painter.setPen(pen);

    qreal usableWidth = width() - 2 * m_padding;
    qreal usableHeight = height() - 2 * m_padding;

    if (usableWidth <= 0 || usableHeight <= 0) {
        return;
    }

    qreal cellHeight = usableHeight / m_linesPerCol;
    qreal actualLineLength = cellHeight * 0.7;
    if (actualLineLength < 5) actualLineLength = 5;

    qreal hSpacing = (m_linesPerRow > 1) ? usableWidth / (m_linesPerRow - 1) : 0;
    if (m_linesPerRow == 1) hSpacing = 0;

    for (int col = 0; col < m_linesPerRow; ++col) {
        for (int row = 0; row < m_linesPerCol; ++row) {
            qreal centerX = m_padding + col * hSpacing;
            qreal centerY = m_padding + row * cellHeight + (actualLineLength / 2.0);

            QPointF lineCenterLocal(centerX, centerY);

            qreal currentRotationAngle = m_baseRotationAngle;

            if (m_globalMousePos != QPoint(-1, -1))
            {
                QPoint lineCenterGlobal = mapToGlobal(QPoint(lineCenterLocal.x(), lineCenterLocal.y()));

                qreal dx = m_globalMousePos.x() - lineCenterGlobal.x();
                qreal dy = m_globalMousePos.y() - lineCenterGlobal.y();

                qreal angleRad = std::atan2(dy, dx);
                currentRotationAngle = qRadiansToDegrees(angleRad) - 180;
            }

            painter.save();
            painter.translate(lineCenterLocal);
            painter.rotate(currentRotationAngle);
            painter.drawLine(0, -actualLineLength / 2, 0, actualLineLength / 2);
            painter.restore();
        }
    }
}
