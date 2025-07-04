// Copyright (C) 2016 The Qt Company Ltd. // 版权所有 (C) 2016 Qt 公司。
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only // SPDX-许可证标识符：LicenseRef-Qt-Commercial 或 LGPL-3.0-only 或 GPL-2.0-only 或 GPL-3.0-only

#include "hexagonwidget.h" // 包含 HexagonWidget 的头文件。
#include <QPainter> // 包含 QPainter 类，用于在部件上进行绘制。
#include <QtMath> // 包含 Qt 数学函数，例如 qSqrt, qDegreesToRadians。
#include <QMouseEvent> // 确保包含 QMouseEvent 头文件，用于处理鼠标事件。

// HexagonWidget 类的构造函数。
HexagonWidget::HexagonWidget(QWidget *parent)
    : QWidget(parent) // 调用基类 QWidget 的构造函数。
    , m_hexagonSize(50.0) // 初始化六边形边长。
    , m_hexagonSpacing(5.0) // 初始化六边形间距。
    , m_isHovering(false) // 初始状态为未悬停。
{
    setMouseTracking(true); // 启用鼠标跟踪，即使没有按键也会触发 mouseMoveEvent。
    calculateHexagonDimensions(); // 计算六边形的尺寸（宽度和高度）。
}

// 设置六边形边长的方法。
void HexagonWidget::setHexagonSize(double size)
{
    if (size > 0 && m_hexagonSize != size) // 如果新尺寸有效且与当前尺寸不同。
    {
        m_hexagonSize = size; // 更新六边形边长。
        calculateHexagonDimensions(); // 重新计算六边形尺寸。
        update(); // 触发重绘以更新显示。
    }
}

// 设置六边形间距的方法。
void HexagonWidget::setHexagonSpacing(double spacing)
{
    if (spacing >= 0 && m_hexagonSpacing != spacing) // 如果新间距有效且与当前间距不同。
    {
        m_hexagonSpacing = spacing; // 更新六边形间距。
        calculateHexagonDimensions(); // 重新计算六边形尺寸。
        update(); // 触发重绘以更新显示。
    }
}

void HexagonWidget::startAnimation()
{
    QPropertyAnimation* m_animation1 = new QPropertyAnimation(this, "Center_Hexagon"); // 创建一个新的动画，目标属性为 Center_Hexagon。
    m_animation1->setDuration(350); // 设置动画持续时间为 1000 毫秒（1 秒）。
    m_animation1->setStartValue(1.0); // 设置动画起始值为 1.0。
    m_animation1->setEndValue(1.2); // 设置动画结束值为 2.0。
    m_animation1->setEasingCurve(QEasingCurve::InOutQuad); // 设置缓动曲线为 InOutQuad，使动画平滑过渡。


    QPropertyAnimation * m_animation2 = new QPropertyAnimation(this, "Edge_Hexagon"); // 创建另一个动画，目标属性为 Edge_Hexagon。
    m_animation2->setDuration(350); // 设置动画持续时间为 1000 毫秒（1 秒）。
    m_animation2->setStartValue(1.0); // 设置动画起始值为 1.0。
    m_animation2->setEndValue(0.6); // 设置动画结束值为 0.7。
    m_animation2->setEasingCurve(QEasingCurve::InOutQuad); // 设置缓动曲线为 InOutQuad，使动画平滑过渡。


    m_animation1->start(QAbstractAnimation::DeleteWhenStopped); // 启动动画。
    m_animation2->start(QAbstractAnimation::DeleteWhenStopped); // 启动第二个动画。



}



// 计算六边形宽度和高度的方法。
void HexagonWidget::calculateHexagonDimensions()
{
    m_hexagonWidth = m_hexagonSize * qSqrt(3.0); // 计算六边形宽度（对边距离）。
    m_hexagonHeight = 2.0 * m_hexagonSize; // 计算六边形高度（顶点到对边顶点的距离）。
}

// 修改 createHexagon 以便接收当前绘制的六边形大小。
// 根据中心点和六边形大小创建六边形的 QPolygonF 对象。
QPolygonF HexagonWidget::createHexagon(QPointF center, double currentHexagonSize) const
{
    QPolygonF hexagon; // 创建一个 QPolygonF 对象。
    for (int i = 0; i < 6; ++i) // 循环 6 次，为六边形的每个顶点。
    {
        double angle_deg = 60 * i + 30; // 计算当前顶点的角度（度），加30度是为了使六边形平放。
        double angle_rad = qDegreesToRadians(angle_deg); // 将角度从度转换为弧度。
        hexagon << QPointF(center.x() + currentHexagonSize * qCos(angle_rad), // 计算顶点 X 坐标。
                           center.y() + currentHexagonSize * qSin(angle_rad)); // 计算顶点 Y 坐标。
    }
    return hexagon; // 返回构建好的六边形多边形。
}

// 新增：查找哪个六边形被悬停。
// 根据鼠标位置查找被悬停的六边形的中心点。
QPointF HexagonWidget::getHoveredHexagonCenter(const QPoint &pos)
{
    // 计算实际的六边形宽度和高度，考虑间距。
    double hexGridWidth = m_hexagonWidth + m_hexagonSpacing; // 计算网格中六边形的水平总宽度（包括间距）。
    double hexGridHeight = m_hexagonHeight * 0.75 + m_hexagonSpacing; // 计算网格中六边形的垂直总高度（包括间距）。

    // 初始偏移量。
    double xOffset = m_hexagonWidth / 2.0; // 六边形第一列的 X 偏移。
    double yOffset = m_hexagonSize; // 六边形第一行的 Y 偏移。

    // 我们可以反向计算鼠标点可能落在哪一行哪一列。
    // 这是一个简化的方法，更精确的需要考虑六边形形状。
    // 粗略判断鼠标点在哪个“单元格”内。

    // 考虑y轴偏移来找到最近的行。
    int estimatedRow = qRound((pos.y() - yOffset) / hexGridHeight); // 估算鼠标所在行。
    if (estimatedRow < 0) estimatedRow = 0; // 确保行号非负。

    // 根据行数判断x轴偏移。
    double currentXOffset = xOffset; // 默认 X 偏移。
    if (estimatedRow % 2 != 0) { // 如果是奇数行。
        currentXOffset += hexGridWidth / 2.0; // 奇数行会向右偏移半个网格宽度。
    }

    // 考虑x轴偏移来找到最近的列。
    int estimatedCol = qRound((pos.x() - currentXOffset) / hexGridWidth); // 估算鼠标所在列。
    if (estimatedCol < 0) estimatedCol = 0; // 确保列号非负。

    // 遍历鼠标点附近的几个六边形，判断精确的悬停。
    // 遍历周围的 3x3 区域，以确保找到正确的六边形。
    for (int r_offset = -1; r_offset <= 1; ++r_offset) {
        for (int c_offset = -1; c_offset <= 1; ++c_offset) {
            int row = estimatedRow + r_offset; // 计算实际行。
            int col = estimatedCol + c_offset; // 计算实际列。

            if (row < 0 || col < 0) continue; // 避免负索引，跳过无效的行或列。

            double x = col * hexGridWidth + xOffset; // 计算当前六边形的 X 坐标。
            double y = row * hexGridHeight + yOffset; // 计算当前六边形的 Y 坐标。

            if (row % 2 != 0) { // 奇数行交错。
                x += hexGridWidth / 2.0; // 如果是奇数行，X 坐标需要额外偏移。
            }

            QPointF center(x, y); // 构成当前六边形的中心点。
            // 检查鼠标点是否在当前计算出的六边形内部。
            QPolygonF hexPolygon = createHexagon(center, m_hexagonSize); // 根据中心点和默认大小创建六边形多边形。
            if (hexPolygon.containsPoint(pos, Qt::OddEvenFill)) { // 判断鼠标点是否在该多边形内。
                return center; // 找到被悬停的六边形中心，并返回。
            }
        }
    }
    return QPointF(); // 没有六边形被悬停，返回空 QPointF。
}

// 新增：获取指定六边形的所有相邻六边形中心点。
// 根据给定的六边形中心点，计算并返回其所有相邻六边形的中心点列表。
QVector<QPointF> HexagonWidget::getNeighborHexagonCenters(QPointF center) const
{
    QVector<QPointF> neighbors; // 用于存储相邻六边形中心的向量。
    double hexGridWidth = m_hexagonWidth + m_hexagonSpacing; // 计算网格中六边形的水平总宽度。
    double hexGridHeight = m_hexagonHeight * 0.75 + m_hexagonSpacing; // 计算网格中六边形的垂直总高度。

    // 相邻偏移量 (相对一个六边形的中心)。
    // 这是一个简化的表示，需要根据六边形中心推算其相邻六边形中心。
    // 考虑六边形网格的两种相邻模式：水平方向和对角线方向。

    // 直接水平相邻。
    neighbors.append(QPointF(center.x() + hexGridWidth, center.y())); // 右侧相邻。
    neighbors.append(QPointF(center.x() - hexGridWidth, center.y())); // 左侧相邻。

    // 上下对角线相邻 (左上, 右上, 左下, 右下)。
    // 这里的偏移量需要精确计算。
    double x_offset_half = hexGridWidth / 2.0; // 水平偏移量的一半。
    double y_offset_quarter_height = m_hexagonHeight * 0.75; // 六边形堆叠的垂直步长。

    // 上方两相邻。
    neighbors.append(QPointF(center.x() + x_offset_half, center.y() - y_offset_quarter_height - m_hexagonSpacing)); // 右上方相邻。
    neighbors.append(QPointF(center.x() - x_offset_half, center.y() - y_offset_quarter_height - m_hexagonSpacing)); // 左上方相邻。

    // 下方两相邻。
    neighbors.append(QPointF(center.x() + x_offset_half, center.y() + y_offset_quarter_height + m_hexagonSpacing)); // 右下方相邻。
    neighbors.append(QPointF(center.x() - x_offset_half, center.y() + y_offset_quarter_height + m_hexagonSpacing)); // 左下方相邻。

    return neighbors; // 返回所有相邻六边形的中心点列表。
}

// 绘制事件处理函数，负责绘制六边形网格。
void HexagonWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event); // 标记 event 未使用，避免编译警告。
    QPainter painter(this); // 创建 QPainter 对象，在当前部件上绘制。
    painter.setRenderHint(QPainter::Antialiasing); // 启用抗锯齿，使图形更平滑。
    painter.setPen(QPen(Qt::black, 1)); // 设置画笔为黑色，宽度为 1。

    double hexGridWidth = m_hexagonWidth + m_hexagonSpacing; // 计算网格中六边形的水平总宽度。
    double hexGridHeight = m_hexagonHeight * 0.75 + m_hexagonSpacing; // 计算网格中六边形的垂直总高度。

    double xOffset = m_hexagonWidth / 2.0; // 六边形第一列的 X 偏移。
    double yOffset = m_hexagonSize; // 六边形第一行的 Y 偏移。

    // 获取相邻六边形的中心列表。
    QVector<QPointF> neighborCenters; // 存储相邻六边形中心点的向量。
    if (m_isHovering && !m_hoveredHexagonCenter.isNull())
    { // 如果鼠标悬停且悬停中心点有效。
        neighborCenters = getNeighborHexagonCenters(m_hoveredHexagonCenter); // 获取悬停六边形的相邻中心点。
    }

    // 增加绘制范围，确保覆盖放大和缩小的六边形。
    // 计算需要绘制的列数和行数，增加一些冗余以确保覆盖。
    int numCols = qCeil((width() + m_hexagonSize * 2) / hexGridWidth) + 3;
    int numRows = qCeil((height() + m_hexagonSize * 2) / hexGridHeight) + 3;

    // 遍历所有可能的六边形位置。
    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            double x = col * hexGridWidth + xOffset; // 计算当前六边形的 X 坐标。
            double y = row * hexGridHeight + yOffset; // 计算当前六边形的 Y 坐标。

            if (row % 2 != 0) { // 如果是奇数行。
                x += hexGridWidth / 2.0; // 奇数行向右偏移。
            }

            QPointF currentHexagonCenter(x, y); // 当前六边形的中心点。

            // 只有当六边形完全或部分在可见区域内时才绘制，并考虑放大后的尺寸。
            // 检查六边形是否在可见区域内。
            if (x - m_hexagonSize * 2 < width() && x + m_hexagonSize * 2 > 0 &&
                y - m_hexagonSize * 2 < height() && y + m_hexagonSize * 2 > 0) {

                double currentDrawSize = m_hexagonSize; // 默认绘制大小为 m_hexagonSize。
                QBrush currentBrush = QBrush(Qt::cyan); // 默认填充颜色为青色。

                if (m_isHovering) // 如果正在悬停。
                {
                    if (currentHexagonCenter == m_hoveredHexagonCenter) { // 如果是悬停的六边形。
                        currentDrawSize = m_hexagonSize * m_Center_Hexagon; // 放大1.3倍。
                        currentBrush = QBrush(Qt::red); // 悬停六边形颜色为红色。
                    } else if (neighborCenters.contains(currentHexagonCenter)) { // 如果是悬停六边形的相邻六边形。
                        currentDrawSize = m_hexagonSize * m_Edge_Hexagon; // 缩小0.7倍。
                        currentBrush = QBrush(Qt::darkCyan); // 相邻六边形颜色为深青色。
                    }
                }
                painter.setBrush(currentBrush); // 设置画刷。
                painter.drawPolygon(createHexagon(currentHexagonCenter, currentDrawSize)); // 绘制六边形。
            }
        }
    }
}

// 新增：鼠标移动事件处理。
void HexagonWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPointF oldHoverCenter = m_hoveredHexagonCenter; // 记录旧的悬停中心。
    m_hoveredHexagonCenter = getHoveredHexagonCenter(event->pos()); // 获取新的悬停中心。

    if (m_hoveredHexagonCenter.isNull() && m_isHovering)
    {
        // 鼠标移出所有六边形，或从一个六边形移到空白区域。
        m_isHovering = false; // 设置未悬停状态。
        update(); // 触发重绘，恢复所有六边形到默认大小。
    }
    else if (!m_hoveredHexagonCenter.isNull() && m_hoveredHexagonCenter != oldHoverCenter)
    {
        // 鼠标移到新的六边形上。
        m_isHovering = true; // 设置悬停状态。

        startAnimation(); // 启动放大动画。

    }
    QWidget::mouseMoveEvent(event); // 调用基类的事件处理。
}

// 新增：鼠标离开控件事件处理。
void HexagonWidget::leaveEvent(QEvent *event)
{
    if (m_isHovering)
    { // 如果之前处于悬停状态。
        m_isHovering = true; // 设置未悬停状态。
        m_hoveredHexagonCenter = QPointF(); // 清空悬停中心。

    }
    QWidget::leaveEvent(event); // 调用基类的事件处理。
}

qreal HexagonWidget::Center_Hexagon() const
{
    return m_Center_Hexagon;
}

void HexagonWidget::setCenter_Hexagon(qreal newCenter_Hexagon)
{
    if (qFuzzyCompare(m_Center_Hexagon, newCenter_Hexagon))
        return;
    m_Center_Hexagon = newCenter_Hexagon;
    update(); // 更新绘制以反映新的中心六边形大小。
    emit Center_HexagonChanged();
}

qreal HexagonWidget::Edge_Hexagon() const
{
    return m_Edge_Hexagon;
}

void HexagonWidget::setEdge_Hexagon(qreal newEdge_Hexagon)
{
    if (qFuzzyCompare(m_Edge_Hexagon, newEdge_Hexagon))
        return;
    m_Edge_Hexagon = newEdge_Hexagon;
    emit Edge_HexagonChanged();
}
