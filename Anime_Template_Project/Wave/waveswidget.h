// waveswidget.h
#ifndef WAVESWIDGET_H                                                                       // 如果未定义 WAVESWIDGET_H
#define WAVESWIDGET_H                                                                       // 定义 WAVESWIDGET_H

#include <QWidget>                                                                          // 包含 QWidget 类定义，用于创建用户界面组件
#include <QTimer>                                                                           // 包含 QTimer 类定义，用于定时器事件
#include <QPainter>                                                                         // 包含 QPainter 类定义，用于在 QWidget 上绘制图形
#include <QMouseEvent>                                                                      // 包含 QMouseEvent 类定义，用于处理鼠标事件
#include <QResizeEvent>                                                                     // 包含 QResizeEvent 类定义，用于处理窗口大小改变事件
#include <QElapsedTimer>                                                                    // 包含 QElapsedTimer 类定义，用于测量经过的时间
#include <cmath>                                                                            // 包含 cmath 库，用于数学函数，如 std::floor
#include <vector>                                                                           // 包含 vector 容器，用于动态数组
#include <QPainterPath> // Include QPainterPath for drawing complex shapes                 // 包含 QPainterPath 类定义，用于绘制复杂的形状
#include <iostream>

/**
 * @brief Grad 结构体：表示一个梯度向量，用于Perlin噪声计算。
 */
struct Grad 
{                                                                                           // Grad 结构体：表示一个梯度向量，用于Perlin噪声计算。
    qreal x, y, z;                                                                         // x, y, z: 梯度向量的分量

    // 默认构造函数，解决 std::vector::resize 时的默认构造问题
    Grad() : x(0.0), y(0.0), z(0.0) {}                                                      // 默认构造函数：初始化 x, y, z 为 0.0，解决 std::vector::resize 时的默认构造问题

    // 构造函数
    Grad(qreal gx, qreal gy, qreal gz) : x(gx), y(gy), z(gz) {}                          // 构造函数：使用给定的 gx, gy, gz 初始化梯度向量

    /**
     * @brief dot2 方法：计算当前梯度向量与给定二维向量的点积。
     * @param dx - 二维向量的x分量。
     * @param dy - 二维向量的y分量。
     * @returns 点积结果。
     */
    qreal dot2(qreal dx, qreal dy) const { return x * dx + y * dy; }                     // dot2 方法：计算当前梯度向量与给定二维向量的点积。
                                                                                            // dx: 二维向量的x分量。
                                                                                            // dy: 二维向量的y分量。
                                                                                            // 返回值: 点积结果。
};

/**
 * @brief Noise 类：实现Perlin噪声生成器。
 */
class Noise 
{                                                                                                  // Noise 类：实现Perlin噪声生成器。
public:
    // 构造函数
    Noise(qreal seedVal = 0) 
    {                                                                                            // 构造函数：初始化 Noise 实例，可选参数 seedVal 用于设置种子
        // 预定义的12个梯度向量
        grad3 = 
        {                                                                                        // grad3: 预定义的12个梯度向量，用于Perlin噪声计算
            Grad(1, 1, 0), Grad(-1, 1, 0), Grad(1, -1, 0), Grad(-1, -1, 0),
            Grad(1, 0, 1), Grad(-1, 0, 1), Grad(1, 0, -1), Grad(-1, 0, -1),
            Grad(0, 1, 1), Grad(0, -1, 1), Grad(0, 1, -1), Grad(0, -1, -1)
        };
        // 预定义的256个整数的排列数组
        p = {151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, // p: 预定义的256个整数的排列数组，用于Perlin噪声的置换表
             69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219,
             203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74,
             165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105,
             92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208,
             89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217,
             226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17,
             182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167,
             43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246,
             97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
             107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
             138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180};

        perm.resize(512);                                                                   // perm: Perlin噪声的置换数组，大小为512
        gradP.resize(512);                                                                  // gradP: 存储梯度向量的数组，大小为512
        seed(seedVal); // 调用seed方法初始化排列数组。                                         // 调用 seed 方法初始化排列数组。
    }

    /**
     * @brief seed 方法：根据给定的种子初始化Perlin噪声的排列数组。
     * @param seedVal - 用于初始化的种子值。
     */
    void seed(qreal seedVal)
     {                                                                                      // seed 方法：根据给定的种子初始化Perlin噪声的排列数组。
                                                                                            // seedVal: 用于初始化的种子值。
        if (seedVal > 0 && seedVal < 1) seedVal *= 65536;                                   // 如果种子值在 (0, 1) 之间，则乘以 65536
        seedVal = std::floor(seedVal);                                                      // 将种子值向下取整
        if (seedVal < 256) seedVal = int(seedVal) | (int(seedVal) << 8);                    // 如果种子值小于 256，则进行位操作以扩展种子值

        for (int i = 0; i < 256; i++) 
        {                                                                                    // 循环遍历 256 次以初始化 perm 和 gradP 数组
            int v = (i & 1) ? (p[i] ^ (int(seedVal) & 255)) : (p[i] ^ ((int(seedVal) >> 8) & 255)); // 根据 i 的奇偶性，使用种子值对 p[i] 进行异或操作
            perm[i] = perm[i + 256] = v;                                                    // 将计算出的 v 赋值给 perm[i] 和 perm[i + 256]
            gradP[i] = gradP[i + 256] = grad3[v % 12];                                      // 将 grad3 中对应索引的梯度向量赋值给 gradP[i] 和 gradP[i + 256]

            // qDebug() << "p[" << i << "] = " << p[i] << " gradP[" << i << "] = " << gradP[i].x << ", " << gradP[i].y << ", " << gradP[i].z; // 输出排列数组和梯度向量
        }
    }

    /**
     * @brief fade 方法：一个平滑曲线函数（6t^5 - 15t^4 + 10t^3），用于Perlin噪声的插值。
     * @param t - 输入值，通常在0到1之间。
     * @returns 平滑后的值。
     */
    static qreal fade(qreal t) { return t * t * t * (t * (t * 6 - 15) + 10); }            // fade 方法：一个平滑曲线函数（6t^5 - 15t^4 + 10t^3），用于Perlin噪声的插值。
                                                                                            // t: 输入值，通常在0到1之间。
                                                                                            // 返回值: 平滑后的值。

    /**
     * @brief lerp 方法：线性插值函数。
     * @param a - 起始值。
     * @param b - 结束值。
     * @param t - 插值因子，通常在0到1之间。
     * @returns 插值结果。
     */
    static qreal lerp(qreal a, qreal b, qreal t) { return (1 - t) * a + t * b; }        // lerp 方法：线性插值函数。
                                                                                            // a: 起始值。
                                                                                            // b: 结束值。
                                                                                            // t: 插值因子，通常在0到1之间。
                                                                                            // 返回值: 插值结果。

    /**
     * @brief perlin2 方法：生成二维Perlin噪声值。
     * @param x - x坐标。
     * @param y - y坐标。
     * @returns 对应的Perlin噪声值。
     */
    qreal perlin2(qreal x, qreal y) const 
    {                                                                                     // perlin2 方法：生成二维Perlin噪声值。
                                                                                            // x: x坐标。
                                                                                            // y: y坐标。
                                                                                            // 返回值: 对应的Perlin噪声值。
        int X = int(std::floor(x));                                            // X: x 坐标的整数部分
        int Y = int(std::floor(y));                                            // Y: y 坐标的整数部分
        x -= X; y -= Y; X &= 255; Y &= 255;                                                 // x, y: x, y 坐标的小数部分；X, Y: 限制在 0-255 范围内

        const Grad& n00 = gradP[X + perm[Y]];                                               // n00: 左下角网格点的梯度向量
        const Grad& n01 = gradP[X + perm[Y + 1]];                                           // n01: 左上角网格点的梯度向量
        const Grad& n10 = gradP[X + 1 + perm[Y]];                                           // n10: 右下角网格点的梯度向量
        const Grad& n11 = gradP[X + 1 + perm[Y + 1]];                                       // n11: 右上角网格点的梯度向量

        qreal u = fade(x);                                                                 // u: x 坐标经过平滑函数处理后的值
        return lerp(                                                                        // 返回插值后的 Perlin 噪声值
            lerp(n00.dot2(x, y), n10.dot2(x - 1, y), u),                                     // 对底部两个点进行插值
            lerp(n01.dot2(x, y - 1), n11.dot2(x - 1, y - 1), u),                            // 对顶部两个点进行插值
            fade(y)                                                                         // 使用 y 坐标经过平滑函数处理后的值进行最终插值
            );
    }

private:
    std::vector<Grad> grad3;                                                                // grad3: 存储预定义梯度向量的容器
    std::vector<int> p;                                                                     // p: 存储置换表的容器
    std::vector<int> perm;                                                                  // perm: 存储 Perlin 噪声排列数组的容器
    std::vector<Grad> gradP;                                                                // gradP: 存储梯度向量的排列数组的容器
};

/**
 * @brief Point 结构体：表示波浪线上的一个点。
 */
struct Point 
{                                                                                             // Point 结构体：表示波浪线上的一个点。
    qreal x, y; // 原始坐标                                                                  // x, y: 原始坐标
    struct Wave { qreal x, y; } wave; // 波浪动画引起的偏移量                               // wave: 波浪动画引起的偏移量，包含 x 和 y 分量
    struct Cursor { qreal x, y, vx, vy; } cursor; // 鼠标交互引起的偏移量和速度             // cursor: 鼠标交互引起的偏移量和速度，包含 x, y 偏移量和 vx, vy 速度分量
};

/**
 * @brief MouseState 结构体：存储鼠标的当前位置、上次位置、平滑位置、速度、角度等信息。
 */
struct MouseState 
{                                                                                             // MouseState 结构体：存储鼠标的当前位置、上次位置、平滑位置、速度、角度等信息。
    qreal x, y; // 当前鼠标位置                                                              // x, y: 当前鼠标的 x, y 坐标
    qreal lx, ly; // 上次鼠标位置                                                            // lx, ly: 上次鼠标的 x, y 坐标
    qreal sx, sy; // 平滑鼠标位置                                                            // sx, sy: 平滑后的鼠标 x, y 坐标
    qreal v; // 当前鼠标速度                                                                  // v: 当前鼠标移动的速度
    qreal vs; // 平滑鼠标速度                                                                // vs: 平滑后的鼠标移动速度
    qreal a; // 鼠标移动角度                                                                  // a: 鼠标移动的角度
    bool set; // 鼠标位置是否已初始化                                                          // set: 鼠标位置是否已经初始化
};

/**
 * @brief Config 结构体：存储波浪效果的各种配置参数。
 */
struct Config 
{                                                                                            // Config 结构体：存储波浪效果的各种配置参数。
    QColor lineColor;                                                                       // lineColor: 波浪线的颜色
    qreal waveSpeedX;                                                                      // waveSpeedX: 波浪在 X 方向上的移动速度
    qreal waveSpeedY;                                                                      // waveSpeedY: 波浪在 Y 方向上的移动速度
    qreal waveAmpX;                                                                        // waveAmpX: 波浪在 X 方向上的振幅
    qreal waveAmpY;                                                                        // waveAmpY: 波浪在 Y 方向上的振幅
    qreal xGap;                                                                            // xGap: 波浪线之间在 X 方向上的间距
    qreal yGap;                                                                            // yGap: 波浪线之间在 Y 方向上的间距
    qreal friction;                                                                        // friction: 鼠标交互引起的波浪点移动的摩擦力
    qreal tension;                                                                         // tension: 鼠标交互引起的波浪点移动的张力
    qreal maxCursorMove;                                                                   // maxCursorMove: 鼠标交互引起的波浪点最大移动距离
};

/**
 * @brief WavesWidget 类：一个Qt Widget，用于创建动态波浪背景效果。
 */
class WavesWidget : public QWidget 
{                                                                                           // WavesWidget 类：一个Qt Widget，用于创建动态波浪背景效果。
    Q_OBJECT                                                                                // 启用 Qt 的元对象系统特性，如信号、槽和属性

public:
    explicit WavesWidget(QWidget *parent = nullptr);                                        // 构造函数：创建一个 WavesWidget 实例
                                                                                            // parent: 父组件，如果指定，则此对象将成为父组件的子对象
    ~WavesWidget(); 

    // 设置波浪线的颜色
    void setLineColor(const QColor& color) { config.lineColor = color; update(); }         // setLineColor: 设置波浪线的颜色
                                                                                            // color: 新的波浪线颜色
    // 设置波浪在x方向的移动速度
    void setWaveSpeedX(qreal speed) { config.waveSpeedX = speed; }                         // setWaveSpeedX: 设置波浪在x方向的移动速度
                                                                                            // speed: 波浪在x方向的新速度
    // 设置波浪在y方向的移动速度
    void setWaveSpeedY(qreal speed) { config.waveSpeedY = speed; }                         // setWaveSpeedY: 设置波浪在y方向的移动速度
                                                                                            // speed: 波浪在y方向的新速度
    // 设置波浪在x方向的振幅
    void setWaveAmpX(qreal amp) { config.waveAmpX = amp; }                                 // setWaveAmpX: 设置波浪在x方向的振幅
                                                                                            // amp: 波浪在x方向的新振幅
    // 设置波浪在y方向的振幅
    void setWaveAmpY(qreal amp) { config.waveAmpY = amp; }                                 // setWaveAmpY: 设置波浪在y方向的振幅
                                                                                            // amp: 波浪在y方向的新振幅
    // 设置波浪线之间在x方向的间距
    void setXGap(qreal gap) { config.xGap = gap; setLines(); update(); }                   // setXGap: 设置波浪线之间在x方向的间距
                                                                                            // gap: 波浪线之间在x方向的新间距
    // 设置波浪线之间在y方向的间距
    void setYGap(qreal gap) { config.yGap = gap; setLines(); update(); }                   // setYGap: 设置波浪线之间在y方向的间距
                                                                                            // gap: 波浪线之间在y方向的新间距
    // 设置鼠标交互引起的波浪点移动的摩擦力
    void setFriction(qreal f) { config.friction = f; }                                     // setFriction: 设置鼠标交互引起的波浪点移动的摩擦力
                                                                                            // f: 新的摩擦力值
    // 设置鼠标交互引起的波浪点移动的张力
    void setTension(qreal t) { config.tension = t; }                                       // setTension: 设置鼠标交互引起的波浪点移动的张力
                                                                                            // t: 新的张力值
    // 设置鼠标交互引起的波浪点最大移动距离
    void setMaxCursorMove(qreal move) { config.maxCursorMove = move; }                     // setMaxCursorMove: 设置鼠标交互引起的波浪点最大移动距离
                                                                                            // move: 新的最大移动距离

protected:
    // 重写 paintEvent，用于绘制波浪
    void paintEvent(QPaintEvent *event) override;                                           // paintEvent: 重写自 QWidget，用于绘制波浪效果
                                                                                            // event: 绘制事件对象
    // 重写 mouseMoveEvent，用于处理鼠标移动
    void mouseMoveEvent(QMouseEvent *event) override;                                       // mouseMoveEvent: 重写自 QWidget，用于处理鼠标移动事件
                                                                                            // event: 鼠标事件对象
    // 重写 resizeEvent，用于处理窗口大小改变
    void resizeEvent(QResizeEvent *event) override;                                         // resizeEvent: 重写自 QWidget，用于处理窗口大小改变事件
                                                                                            // event: 尺寸改变事件对象

private slots:
    // 动画循环的主函数，每帧更新和绘制波浪
    void tick();                                                                            // tick: 动画循环的主函数，每帧更新波浪点位置并触发重绘

private:
    QTimer *animationTimer; // 动画定时器                                                      // animationTimer: 指向 QTimer 对象的指针，用于控制动画的帧率
    Noise *noiseGenerator = nullptr; // Perlin噪声生成器实例                                             // noiseGenerator: 指向 Noise 对象的指针，用于生成 Perlin 噪声
    std::vector<std::vector<Point>> lines; // 存储波浪线的点数据                             // lines: 存储波浪线的点数据，是一个二维向量，每个内层向量代表一条波浪线
    MouseState mouse; // 存储鼠标状态                                                          // mouse: 存储鼠标当前状态的结构体实例
    Config config; // 存储组件的配置属性                                                        // config: 存储 WavesWidget 组件的各种配置属性的结构体实例
    QElapsedTimer timeElapsed; // 用于获取动画时间戳                                           // timeElapsed: 用于获取动画时间戳的 QElapsedTimer 实例

    // 设置波浪线的点数据
    void setLines();                                                                        // setLines: 初始化或重新设置波浪线的点数据，根据当前窗口大小和配置参数
    // 根据Perlin噪声和鼠标交互更新每个波浪点的位置
    void movePoints(qint64 time);                                                           // movePoints: 根据 Perlin 噪声和鼠标交互，更新每个波浪点的位置
                                                                                            // time: 当前动画时间戳
    // 计算一个点的最终渲染位置，包括波浪和鼠标交互引起的偏移
    QPointF moved(const Point& point, bool withCursor = true);                              // moved: 计算一个点的最终渲染位置，包括波浪动画和鼠标交互引起的偏移
                                                                                            // point: 要计算位置的 Point 结构体
                                                                                            // withCursor: 是否包含鼠标交互引起的偏移量，默认为 true
    // 更新鼠标在widget坐标系中的位置
    void updateMouse(int x, int y);                                                         // updateMouse: 更新鼠标在 widget 坐标系中的位置，并计算其速度和角度
                                                                                            // x: 鼠标的 x 坐标
                                                                                            // y: 鼠标的 y 坐标
};

#endif // WAVESWIDGET_H
