// Copyright (C) 2016 The Qt Company Ltd. // 版权所有 (C) 2016 Qt 公司。
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only // SPDX-许可证标识符：LicenseRef-Qt-Commercial 或 LGPL-3.0-only 或 GPL-2.0-only 或 GPL-3.0-only

#ifndef HEXAGONWIDGET_H // 如果未定义 HEXAGONWIDGET_H。
#define HEXAGONWIDGET_H // 定义 HEXAGONWIDGET_H。

#include <QWidget> // 包含 QWidget 类，它是所有用户界面对象的基类。
#include <QVector> // 包含 QVector 类，提供动态数组。
#include <QPointF> // 包含 QPointF 类，表示浮点精确度的点。
#include <QMap> // 用于存储六边形中心点和其对应的多边形。

//动画类
#include <QPropertyAnimation>


class HexagonWidget : public QWidget // HexagonWidget 类声明，继承自 QWidget。
{
    Q_OBJECT // 宏，用于启用 Qt 的元对象系统，支持信号与槽。
    //放大
    Q_PROPERTY(qreal Center_Hexagon READ Center_Hexagon WRITE setCenter_Hexagon NOTIFY Center_HexagonChanged FINAL)
    Q_PROPERTY(qreal Edge_Hexagon READ Edge_Hexagon WRITE setEdge_Hexagon NOTIFY Edge_HexagonChanged FINAL)

public:
    explicit HexagonWidget(QWidget *parent = nullptr); // 显式构造函数，允许指定父部件。
    void setHexagonSize(double size); // 设置六边形的基准边长。
    void setHexagonSpacing(double spacing); // 设置六边形之间的间距。

    //执行动画
    void startAnimation(); // 启动六边形放大动画。



    qreal Center_Hexagon() const;
    void setCenter_Hexagon(qreal newCenter_Hexagon);

    qreal Edge_Hexagon() const;
    void setEdge_Hexagon(qreal newEdge_Hexagon);

signals:
    void Center_HexagonChanged();

    void Edge_HexagonChanged();

protected:
    void paintEvent(QPaintEvent *event) override; // 覆盖 paintEvent，处理部件的绘制事件。
    void mouseMoveEvent(QMouseEvent *event) override; // 新增：鼠标移动事件处理函数。
    void leaveEvent(QEvent *event) override;          // 新增：鼠标离开部件事件处理函数。

    QPolygonF createHexagon(QPointF center, double currentHexagonSize) const; // 修改：增加一个参数来控制当前绘制的六边形大小。根据中心点和给定大小创建六边形多边形。

private:
    double m_hexagonSize;    // 六边形基准边长。
    double m_hexagonSpacing; // 六边形间距。
    double m_hexagonWidth;   // 六边形宽度 (两相对边距离)。
    double m_hexagonHeight;  // 六边形高度 (两相对顶点距离)。

    QPointF m_hoveredHexagonCenter; // 存储当前鼠标悬停的六边形中心点。
    bool m_isHovering;               // 标记是否正在悬停在某个六边形上。


    void calculateHexagonDimensions(); // 计算六边形的内部尺寸（宽度和高度）。
    // 根据鼠标位置查找哪个六边形被悬停。
    QPointF getHoveredHexagonCenter(const QPoint &pos);
    // 获取指定中心点的六边形的所有相邻六边形的中心点。
    QVector<QPointF> getNeighborHexagonCenters(QPointF center) const;


    qreal m_Center_Hexagon; // 当前中心六边形的大小。
    qreal m_Edge_Hexagon; // 当前边缘六边形的大小。
};

#endif // HEXAGONWIDGET_H // 结束 HEXAGONWIDGET_H 头文件保护。
