// waveswidget.cpp
#include "waveswidget.h"                                                                    // 包含 WavesWidget 类的头文件
#include <QDebug> // For console logging                                                    // 包含 QDebug，用于控制台日志输出
#include <random> // For std::mt19937_64 and std::uniform_real_distribution                 // 包含 random 库，用于随机数生成器 std::mt19937_64 和均匀实数分布 std::uniform_real_distribution
#include <chrono> // For std::chrono::system_clock                                          // 包含 chrono 库，用于获取系统时间，例如 std::chrono::system_clock

WavesWidget::WavesWidget(QWidget *parent)                                                   // WavesWidget 类的构造函数
    : QWidget(parent),                                                                      // 调用 QWidget 基类的构造函数，并传入父组件 parent
    animationTimer(new QTimer(this))                                                       // animationTimer: 初始化动画定时器，其父对象为当前 WavesWidget 实例
{
    qDebug() << " 输出调试信息";                                            // 输出调试信息
    // 设置默认配置参数
    config.lineColor = Qt::white;                                                           // lineColor: 设置波浪线的默认颜色为黑色
    config.waveSpeedX = 0.02;                                                             // waveSpeedX: 设置波浪在 X 方向上的默认移动速度
    config.waveSpeedY = 0.01;                                                              // waveSpeedY: 设置波浪在 Y 方向上的默认移动速度
    config.waveAmpX = 40;                                                                   // waveAmpX: 设置波浪在 X 方向上的默认振幅
    config.waveAmpY = 20;                                                                   // waveAmpY: 设置波浪在 Y 方向上的默认振幅
    config.xGap = 12;                                                                       // xGap: 设置波浪线之间在 X 方向上的默认间距
    config.yGap = 36;                                                                       // yGap: 设置波浪线之间在 Y 方向上的默认间距
    config.friction = 0.90;                                                                // friction: 设置鼠标交互引起的波浪点移动的默认摩擦力
    config.tension = 0.01;                                                                 // tension: 设置鼠标交互引起的波浪点移动的默认张力
    config.maxCursorMove = 120;                                                             // maxCursorMove: 设置鼠标交互引起的波浪点默认最大移动距离

    // 初始化鼠标状态
    mouse.x = -10; mouse.y = 0;                                                             // x, y: 初始化鼠标的 X, Y 坐标
    mouse.lx = 0; mouse.ly = 0;                                                             // lx, ly: 初始化上次鼠标的 X, Y 坐标
    mouse.sx = 0; mouse.sy = 0;                                                             // sx, sy: 初始化平滑鼠标的 X, Y 坐标
    mouse.v = 0; mouse.vs = 0;                                                              // v, vs: 初始化鼠标速度和平滑鼠标速度
    mouse.a = 0;                                                                            // a: 初始化鼠标移动角度
    mouse.set = false;                                                                      // set: 标记鼠标位置未初始化

    // 使用当前时间作为种子初始化Perlin噪声生成器
    std::mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count());       // rng: 创建一个 Mersenne Twister 64 位随机数生成器，使用当前时间作为种子
    std::uniform_real_distribution<> dist(0.0, 1.0);                                        // dist: 创建一个均匀实数分布，范围在 0.0 到 1.0 之间
    noiseGenerator = new Noise(dist(rng));                                                  // noiseGenerator: 使用随机生成的种子值创建 Noise 实例

    // 连接定时器到tick槽函数
    connect(animationTimer, &QTimer::timeout, this, &WavesWidget::tick);                   // 将 animationTimer 的 timeout 信号连接到当前 WavesWidget 实例的 tick 槽函数
    animationTimer->start(500); // 大约60帧/秒 (1000ms / 60 = 16.66ms)                       // 启动 animationTimer，每 16 毫秒触发一次 timeout 信号，实现大约 60 帧/秒的动画效果

    // 启用鼠标跟踪，即使不按住鼠标按钮也能接收鼠标移动事件
    setMouseTracking(true);                                                                 // 启用鼠标跟踪，使得即使不按住鼠标按钮也能接收 mouseMoveEvent 事件

    // 启动计时器，用于获取动画时间戳
    timeElapsed.start();                                                                    // timeElapsed: 启动计时器，用于测量动画的运行时间

    qDebug() << "已加载";                                                      
}

WavesWidget::~WavesWidget()
 {                                                                                           // WavesWidget 类的析构函数
    delete noiseGenerator;                                                                  // noiseGenerator: 删除 noiseGenerator 对象，释放内存
    qDebug() << "已卸载";                                                    
}

void WavesWidget::paintEvent(QPaintEvent *event) 
{                                          // paintEvent: 重写自 QWidget，用于绘制波浪效果
    Q_UNUSED(event);                                                                        // 标记 event 参数未使用，避免编译器警告
    QPainter painter(this);                                                                 // painter: 创建一个 QPainter 对象，用于在当前 widget 上进行绘制
    painter.setRenderHint(QPainter::Antialiasing); // 启用抗锯齿，使线条更平滑             // 启用抗锯齿渲染提示，使绘制的线条更加平滑

    painter.setPen(config.lineColor);                                                       // 设置画笔颜色为 config.lineColor

    // 遍历所有波浪线并绘制
    for (const auto& points : lines) 
    {                                                      // 遍历 lines 容器中的每一条波浪线（即每个 Point 向量）
        if (points.empty()) continue;                                                       // 如果当前波浪线为空，则跳过

        QPainterPath path; // 创建一个QPainterPath对象                                       // path: 创建一个 QPainterPath 对象，用于构建复杂的图形路径
        QPointF p1 = moved(points[0], false); // 获取第一个点的初始位置（不包含鼠标偏移）   // p1: 获取当前波浪线第一个点的渲染位置，不包含鼠标偏移
        path.moveTo(p1); // 将路径的起始点移动到第一个点                                     // 将路径的当前点移动到 p1

        for (size_t idx = 0; idx < points.size(); ++idx)
        {                                  // 遍历当前波浪线中的所有点
            const Point& p = points[idx];                                                   // p: 获取当前点的引用
            bool isLast = (idx == points.size() - 1);                                       // isLast: 判断当前点是否是波浪线的最后一个点
            p1 = moved(p, !isLast); // 获取当前点的渲染位置，最后一个点不应用鼠标偏移       // p1: 获取当前点的渲染位置，如果不是最后一个点则包含鼠标偏移
            path.lineTo(p1); // 从当前位置画线到p1                                           // 从路径的当前点画线到 p1
        }
        painter.drawPath(path); // 绘制整个路径                                             // 绘制由 path 定义的整个图形路径
    }
}

void WavesWidget::mouseMoveEvent(QMouseEvent *event)
{                                      // mouseMoveEvent: 重写自 QWidget，用于处理鼠标移动事件
    updateMouse(event->pos().x(), event->pos().y());                                        // 调用 updateMouse 函数，更新鼠标在 widget 坐标系中的位置
                                                                                            // event->pos().x(): 鼠标事件的 X 坐标
                                                                                            // event->pos().y(): 鼠标事件的 Y 坐标
    QWidget::mouseMoveEvent(event); // 调用基类的mouseMoveEvent                             // 调用 QWidget 基类的 mouseMoveEvent，确保默认的鼠标事件处理行为
}

void WavesWidget::resizeEvent(QResizeEvent *event) 
{                                        // resizeEvent: 重写自 QWidget，用于处理窗口大小改变事件
    Q_UNUSED(event);                                                                        // 标记 event 参数未使用，避免编译器警告
    setLines(); // 重新设置波浪线数据                                                       // 调用 setLines 函数，根据新的窗口大小重新设置波浪线数据
    QWidget::resizeEvent(event); // 调用基类的resizeEvent                                  // 调用 QWidget 基类的 resizeEvent，确保默认的尺寸改变事件处理行为
}

void WavesWidget::tick()
{                                                                  // tick: 动画循环的主函数，每帧更新波浪点位置并触发重绘
    qint64 currentTime = timeElapsed.elapsed(); // 获取当前动画时间戳                       // currentTime: 获取自 timeElapsed 启动以来经过的毫秒数，作为当前动画时间戳

    // 平滑更新鼠标的平滑位置（sx, sy）
    mouse.sx += (mouse.x - mouse.sx) * 0.1;                                                 // sx: 平滑更新鼠标的 X 坐标，使其逐渐接近当前鼠标 X 坐标
    mouse.sy += (mouse.y - mouse.sy) * 0.1;                                                 // sy: 平滑更新鼠标的 Y 坐标，使其逐渐接近当前鼠标 Y 坐标

    // 计算鼠标速度和角度
    qreal dx = mouse.x - mouse.lx;                                                         // dx: 计算鼠标在 X 方向上的移动距离
    qreal dy = mouse.y - mouse.ly;                                                         // dy: 计算鼠标在 Y 方向上的移动距离
    qreal d = std::hypot(dx, dy); // 鼠标移动距离                                            // d: 计算鼠标的实际移动距离（欧几里得距离）
    mouse.v = d; // 当前鼠标速度                                                             // v: 更新当前鼠标速度为实际移动距离
    mouse.vs += (d - mouse.vs) * 0.1; // 平滑鼠标速度                                       // vs: 平滑更新鼠标速度，使其逐渐接近当前速度
    mouse.vs = std::min(100.0, mouse.vs); // 限制平滑鼠标速度的最大值                       // 限制平滑鼠标速度的最大值为 100.0
    mouse.lx = mouse.x;                                                                     // lx: 更新上次鼠标 X 坐标为当前鼠标 X 坐标
    mouse.ly = mouse.y; // 更新上次鼠标位置                                                 // ly: 更新上次鼠标 Y 坐标为当前鼠标 Y 坐标
    mouse.a = std::atan2(dy, dx); // 计算鼠标移动角度                                       // a: 计算鼠标移动的方向角度（弧度）

    movePoints(currentTime); // 更新波浪点位置                                             // 调用 movePoints 函数，根据当前时间更新所有波浪点的位置
    update(); // 请求重绘，这将触发paintEvent                                               // 请求 widget 重绘，这将触发 paintEvent 函数的调用
}

void WavesWidget::setLines() 
{                                                              // setLines: 初始化或重新设置波浪线的点数据
    lines.clear(); // 清空现有波浪线数据                                                     // 清空 lines 容器中所有现有的波浪线数据

    qreal oWidth = width() + 200; // 扩展的宽度，以确保波浪覆盖整个区域                     // oWidth: 计算一个扩展的宽度，比当前 widget 宽度多 200 像素，以确保波浪覆盖整个区域
    qreal oHeight = height() + 30; // 扩展的高度                                            // oHeight: 计算一个扩展的高度，比当前 widget 高度多 30 像素

    int totalLines = static_cast<int>(std::ceil(oWidth / config.xGap));                     // totalLines: 根据扩展宽度和 X 方向间距计算总共需要绘制的波浪线数量
    int totalPoints = static_cast<int>(std::ceil(oHeight / config.yGap));                   // totalPoints: 根据扩展高度和 Y 方向间距计算每条波浪线上的总点数

    qreal xStart = (width() - config.xGap * totalLines) / 2.0;                             // xStart: 计算 X 方向的起始偏移量，使波浪线在水平方向上居中
    qreal yStart = (height() - config.yGap * totalPoints) / 2.0;                           // yStart: 计算 Y 方向的起始偏移量，使波浪线在垂直方向上居中

    for (int i = 0; i <= totalLines; ++i) 
    {                                                 // 循环遍历，创建每一条波浪线
        std::vector<Point> pts; // 当前波浪线的点数组                                         // pts: 用于存储当前波浪线点的向量
        for (int j = 0; j <= totalPoints; ++j) 
        {                                            // 循环遍历，创建当前波浪线上的每一个点
            pts.push_back({                                                                 // 向 pts 向量中添加一个新的 Point 结构体
                xStart + config.xGap * i, // 点的原始x坐标                                   // x: 点的原始 X 坐标
                yStart + config.yGap * j, // 点的原始y坐标                                   // y: 点的原始 Y 坐标
                {0, 0}, // 波浪动画引起的偏移量                                               // wave: 初始化波浪动画引起的偏移量为 (0, 0)
                {0, 0, 0, 0} // 鼠标交互引起的偏移量和速度                                   // cursor: 初始化鼠标交互引起的偏移量和速度为 (0, 0, 0, 0)
            });
        }
        lines.push_back(pts); // 将当前波浪线添加到lines中                                  // 将完整的波浪线（Point 向量）添加到 lines 容器中
    }
    // qDebug() << "总线数: " << lines.size() << "第一条线的点数:" << (lines.empty() ? 0 : lines[0].size());   // 输出调试信息，显示总线数和第一条线的点数
}

void WavesWidget::movePoints(qint64 time) 
{                                                  // movePoints: 根据 Perlin 噪声和鼠标交互更新每个波浪点的位置
                                                                                            // time: 当前动画时间戳
    for (auto& pts : lines) 
    {                                                               // 遍历 lines 容器中的每一条波浪线
        for (auto& p : pts) 
        {                                                               // 遍历当前波浪线中的所有点
            // 根据Perlin噪声计算波浪偏移量
            qreal move = noiseGenerator->perlin2(                                          // move: 根据 Perlin 噪声计算一个移动值
                                  (p.x + time * config.waveSpeedX) * 0.002,                 // 第一个噪声输入：基于点的 X 坐标和时间（乘以波浪速度 X）
                                  (p.y + time * config.waveSpeedY) * 0.0015                 // 第二个噪声输入：基于点的 Y 坐标和时间（乘以波浪速度 Y）
                                  ) * 12;                                                   // 噪声值乘以一个缩放因子 12

            // qDebug() << "move: " << move;

            p.wave.x = std::cos(move) * config.waveAmpX;                                    // wave.x: 计算 X 方向的波浪偏移量，使用 move 的余弦值乘以 X 方向振幅
            p.wave.y = std::sin(move) * config.waveAmpY;                                    // wave.y: 计算 Y 方向的波浪偏移量，使用 move 的正弦值乘以 Y 方向振幅

            // qDebug() << "wave: " << p.wave.x << " " << p.wave.y;

            // 计算点到鼠标位置的距离
            qreal dx = p.x - mouse.sx;                                                     // dx: 点的原始 X 坐标与平滑鼠标 X 坐标的差值
            qreal dy = p.y - mouse.sy;                                                     // dy: 点的原始 Y 坐标与平滑鼠标 Y 坐标的差值
            qreal dist = std::hypot(dx, dy);                                               // dist: 计算点到平滑鼠标位置的距离
            qreal l = std::max(175.0, mouse.vs); // dist为距离，l为影响范围，受鼠标速度影响 // l: 鼠标影响范围，至少为 175.0，并受平滑鼠标速度影响

            if (dist < l) 
            { // 如果点在鼠标影响范围内                                        // 如果当前点到鼠标的距离小于影响范围 l
                qreal s = 1.0 - dist / l; // 影响强度，距离越近强度越大                     // s: 计算影响强度，距离越近强度越大（0 到 1 之间）
                qreal f = std::cos(dist * 0.001) * s; // 影响因子，带有余弦波纹效果         // f: 计算影响因子，带有余弦波纹效果，并受影响强度 s 影响
                // 根据鼠标移动方向和速度，施加力到点的cursor偏移量上
                p.cursor.vx += std::cos(mouse.a) * f * l * mouse.vs * 0.00065;              // cursor.vx: 根据鼠标移动角度、影响因子、影响范围、平滑鼠标速度，计算并增加 X 方向的速度
                p.cursor.vy += std::sin(mouse.a) * f * l * mouse.vs * 0.00065;              // cursor.vy: 根据鼠标移动角度、影响因子、影响范围、平滑鼠标速度，计算并增加 Y 方向的速度
            }

            // 应用张力（将点拉回原位）和摩擦力
            p.cursor.vx += (0.0 - p.cursor.x) * config.tension;                             // cursor.vx: 应用张力，将点在 X 方向上拉回原始位置 (0,0)
            p.cursor.vy += (0.0 - p.cursor.y) * config.tension;                             // cursor.vy: 应用张力，将点在 Y 方向上拉回原始位置 (0,0)
            p.cursor.vx *= config.friction;                                                 // cursor.vx: 应用摩擦力，减小 X 方向的速度
            p.cursor.vy *= config.friction;                                                 // cursor.vy: 应用摩擦力，减小 Y 方向的速度
            p.cursor.x += p.cursor.vx * 2;                                                  // cursor.x: 更新 X 方向的偏移量，乘以 2 调整步长
            p.cursor.y += p.cursor.vy * 2;                                                  // cursor.y: 更新 Y 方向的偏移量，乘以 2 调整步长

            // 限制cursor偏移量在最大范围内
            p.cursor.x = std::min(config.maxCursorMove, std::max(-config.maxCursorMove, p.cursor.x)); // cursor.x: 限制 X 方向的偏移量在 [-maxCursorMove, maxCursorMove] 范围内
            p.cursor.y = std::min(config.maxCursorMove, std::max(-config.maxCursorMove, p.cursor.y)); // cursor.y: 限制 Y 方向的偏移量在 [-maxCursorMove, maxCursorMove] 范围内
        }
    }
}

QPointF WavesWidget::moved(const Point& point, bool withCursor) 
{                           // moved: 计算一个点的最终渲染位置，包括波浪和鼠标交互引起的偏移
                                                                                            // point: 要计算位置的 Point 结构体
                                                                                            // withCursor: 是否包含鼠标交互引起的偏移量，默认为 true
    qreal x = point.x + point.wave.x + (withCursor ? point.cursor.x : 0);                  // x: 计算最终的 X 坐标，包括原始坐标、波浪偏移和可选的鼠标偏移
    qreal y = point.y + point.wave.y + (withCursor ? point.cursor.y : 0);                  // y: 计算最终的 Y 坐标，包括原始坐标、波浪偏移和可选的鼠标偏移
    return QPointF(std::round(x * 10) / 10.0, std::round(y * 10) / 10.0);                   // 返回四舍五入到一位小数的 QPointF 坐标，用于平滑渲染
}

void WavesWidget::updateMouse(int x, int y) 
{                                               // updateMouse: 更新鼠标在 widget 坐标系中的位置
                                                                                            // x: 鼠标的 X 坐标
                                                                                            // y: 鼠标的 Y 坐标
    mouse.x = x;                                                                            // mouse.x: 更新鼠标的 X 坐标
    mouse.y = y;                                                                            // mouse.y: 更新鼠标的 Y 坐标

    if (!mouse.set) 
    { // 如果鼠标位置尚未初始化                                              // 如果 mouse.set 为 false (即鼠标位置尚未初始化)
        mouse.sx = mouse.x; mouse.sy = mouse.y;                                             // sx, sy: 初始化平滑鼠标位置为当前鼠标位置
        mouse.lx = mouse.x; mouse.ly = mouse.y;                                             // lx, ly: 初始化上次鼠标位置为当前鼠标位置
        mouse.set = true; // 标记鼠标位置已设置                                               // set: 将 mouse.set 设置为 true，表示鼠标位置已初始化
    }
}
