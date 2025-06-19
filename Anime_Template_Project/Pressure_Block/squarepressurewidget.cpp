// squarepressurewidget.cpp
#include "squarepressurewidget.h"
#include <algorithm>
#include <cmath>

SquarePressureWidget::SquarePressureWidget(QWidget *parent)
    : QWidget(parent),
    m_text("COMPRESSA"),
    m_minSquareSize(24),
    m_mousePos(0, 0)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SquarePressureWidget::updateAnimation);
    m_timer->start(5);
}

void SquarePressureWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const auto &prop : m_squareProps)
    {
        painter.setOpacity(prop.currentAlpha);
        painter.fillRect(prop.rect, prop.currentColor);
    }
}

void SquarePressureWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    calculateSquareProperties();
    update();
}

void SquarePressureWidget::updateAnimation()
{
    m_cursorPos = mapFromGlobal(QCursor::pos());

    m_mousePos.setX(m_mousePos.x() + (m_cursorPos.x() - m_mousePos.x()) / 5.0);
    m_mousePos.setY(m_mousePos.y() + (m_cursorPos.y() - m_mousePos.y()) / 5.0);

    double maxDist = std::max(rect().width() / 2.0, rect().height() / 2.0) * 1.5;

    for (auto &prop : m_squareProps)
    {
        double d = dist(m_mousePos, prop.initialCenter);

        auto getAttr = [&](double distance, double minVal, double maxVal)
        {
            double normalizedDistance = std::min(distance, maxDist) / maxDist;
            double val = maxVal - (normalizedDistance * (maxVal - minVal));
            return std::max(minVal, val);
        };

        double maxDynamicSquareSize = std::max(rect().width(), rect().height()) * 0.15;
        maxDynamicSquareSize = std::max(m_minSquareSize, maxDynamicSquareSize);

        double currentSquareSize = getAttr(d, m_minSquareSize, maxDynamicSquareSize);

        double maxDisplacement = maxDynamicSquareSize * 0.5;
        double displacementStrength = getAttr(d, 0.0, maxDisplacement);

        QPointF direction = prop.initialCenter - m_mousePos;
        if (direction.isNull())
        {
            direction = QPointF(1.0, 0.0);
        }
        double angle = std::atan2(direction.y(), direction.x());

        QPointF displacementVector(std::cos(angle) * displacementStrength, std::sin(angle) * displacementStrength);

        QPointF finalCenter = prop.initialCenter + displacementVector;

        prop.rect.setSize(QSizeF(currentSquareSize, currentSquareSize));
        prop.rect.moveCenter(finalCenter);

        prop.currentAlpha = getAttr(d, 0.2, 1.0);

        int colorVal = static_cast<int>(getAttr(d, 0.0, 255.0));
        prop.currentColor = QColor(colorVal, colorVal, colorVal);
    }

    update();
}

void SquarePressureWidget::calculateSquareProperties()
{
    m_squareProps.clear();
    if (m_text.isEmpty())
    {
        return;
    }

    double charCount = m_text.length();

    double idealSquareSize = std::min(width() / charCount, (double)height());
    double initialSquareSize = std::max(m_minSquareSize, idealSquareSize);

    double currentX = (width() - (charCount * initialSquareSize)) / 2.0;

    for (int i = 0; i < charCount; ++i)
    {
        SquareProperties prop;
        prop.character = m_text.at(i);
        prop.rect = QRectF(currentX, (height() - initialSquareSize) / 2.0, initialSquareSize, initialSquareSize);
        prop.initialCenter = prop.rect.center();
        prop.currentWeight = 100;
        prop.currentAlpha = 1.0;
        prop.currentColor = Qt::white;
        m_squareProps.append(prop);
        currentX += initialSquareSize;
    }
}

double SquarePressureWidget::dist(const QPointF &p1, const QPointF &p2) {
    return std::sqrt(std::pow(p2.x() - p1.x(), 2) + std::pow(p2.y() - p1.y(), 2));
}
