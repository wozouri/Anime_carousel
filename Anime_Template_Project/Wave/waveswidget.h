#ifndef WAVESWIDGET_H
#define WAVESWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QElapsedTimer>
#include <cmath>
#include <vector>
#include <QPainterPath>
#include <iostream>
#include <QDebug>

struct Grad
{
    qreal x, y, z;

    Grad() : x(0.0), y(0.0), z(0.0) {}

    Grad(qreal gx, qreal gy, qreal gz) : x(gx), y(gy), gz(gz) {}

    qreal dot2(qreal dx, qreal dy) const { return x * dx + y * dy; }
};

class Noise
{
public:
    Noise(qreal seedVal = 0)
    {
        grad3 =
        {
            Grad(1, 1, 0), Grad(-1, 1, 0), Grad(1, -1, 0), Grad(-1, -1, 0),
            Grad(1, 0, 1), Grad(-1, 0, 1), Grad(1, 0, -1), Grad(-1, 0, -1),
            Grad(0, 1, 1), Grad(0, -1, 1), Grad(0, 1, -1), Grad(0, -1, -1)
        };
        p = {151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30,
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

        perm.resize(512);
        gradP.resize(512);
        seed(seedVal);
    }

    void seed(qreal seedVal)
    {
        if (seedVal > 0 && seedVal < 1) seedVal *= 65536;
        seedVal = std::floor(seedVal);
        if (seedVal < 256) seedVal = static_cast<int>(seedVal) | (static_cast<int>(seedVal) << 8);

        for (int i = 0; i < 256; i++)
        {
            int v = (i & 1) ? (p[i] ^ (static_cast<int>(seedVal) & 255)) : (p[i] ^ ((static_cast<int>(seedVal) >> 8) & 255));
            perm[i] = perm[i + 256] = v;
            gradP[i] = gradP[i + 256] = grad3[v % 12];
        }
    }

    static qreal fade(qreal t) { return t * t * t * (t * (t * 6 - 15) + 10); }

    static qreal lerp(qreal a, qreal b, qreal t) { return (1 - t) * a + t * b; }

    qreal perlin2(qreal x, qreal y) const
    {
        int X = static_cast<int>(std::floor(x));
        int Y = static_cast<int>(std::floor(y));
        x -= X; y -= Y; X &= 255; Y &= 255;

        const Grad& n00 = gradP[X + perm[Y]];
        const Grad& n01 = gradP[X + perm[Y + 1]];
        const Grad& n10 = gradP[X + 1 + perm[Y]];
        const Grad& n11 = gradP[X + 1 + perm[Y + 1]];

        qreal u = fade(x);
        return lerp(
            lerp(n00.dot2(x, y), n10.dot2(x - 1, y), u),
            lerp(n01.dot2(x, y - 1), n11.dot2(x - 1, y - 1), u),
            fade(y)
            );
    }

private:
    std::vector<Grad> grad3;
    std::vector<int> p;
    std::vector<int> perm;
    std::vector<Grad> gradP;
};

struct Point
{
    qreal x, y;
    struct Wave { qreal x, y; } wave;
    struct Cursor { qreal x, y, vx, vy; } cursor;
};

struct MouseState
{
    qreal x, y;
    qreal lx, ly;
    qreal sx, sy;
    qreal v;
    qreal vs;
    qreal a;
    bool set;
};

struct Config
{
    QColor lineColor;
    qreal waveSpeedX;
    qreal waveSpeedY;
    qreal waveAmpX;
    qreal waveAmpY;
    qreal xGap;
    qreal yGap;
    qreal friction;
    qreal tension;
    qreal maxCursorMove;
};

class WavesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WavesWidget(QWidget *parent = nullptr);
    ~WavesWidget();

    void setLineColor(const QColor& color) { config.lineColor = color; update(); }
    void setWaveSpeedX(qreal speed) { config.waveSpeedX = speed; }
    void setWaveSpeedY(qreal speed) { config.waveSpeedY = speed; }
    void setWaveAmpX(qreal amp) { config.waveAmpX = amp; }
    void setWaveAmpY(qreal amp) { config.waveAmpY = amp; }
    void setXGap(qreal gap) { config.xGap = gap; setLines(); update(); }
    void setYGap(qreal gap) { config.yGap = gap; setLines(); update(); }
    void setFriction(qreal f) { config.friction = f; }
    void setTension(qreal t) { config.tension = t; }
    void setMaxCursorMove(qreal move) { config.maxCursorMove = move; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void tick();

private:
    QTimer *animationTimer;
    Noise *noiseGenerator = nullptr;
    std::vector<std::vector<Point>> lines;
    MouseState mouse;
    Config config;
    QElapsedTimer timeElapsed;

    void setLines();
    void movePoints(qint64 time);
    QPointF moved(const Point& point, bool withCursor = true);
    void updateMouse(int x, int y);
};

#endif // WAVESWIDGET_H
