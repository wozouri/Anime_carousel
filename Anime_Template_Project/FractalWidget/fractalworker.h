#ifndef FRACTALWORKER_H
#define FRACTALWORKER_H

#include <QObject>
#include <QImage>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

class FractalWorker : public QObject
{
    Q_OBJECT

public:
    explicit FractalWorker(QObject *parent = nullptr);

    enum FractalType {
        Mandelbrot,
        Julia,
        BurningShip,
        Newton,
        Tricorn,
        BurningShipJulia,
        Spider,
        SierpinskiCurve,
        KochSnowflake
    };

signals:
    void imageReady(const QImage &image);
    void finished();

public slots:
    void doWork(FractalType type, int currentMaxIterations, double juliaCr, double juliaCi,
                double centerX, double centerY, double scale,
                int imageWidth, int imageHeight, int maxAnimationSteps);

private:
    struct FractalChunkData {
        FractalType type;
        int currentMaxIterations;
        double juliaCr;
        double juliaCi;
        double centerX;
        double centerY;
        double scale;
        int imageWidth;
        int imageHeight;
        int maxAnimationSteps;
        int startY;
        int endY;
    };

    static QImage computeFractalChunk(const FractalChunkData &data);

    static QRgb getColor(int iterations, int maxIterationsUsed);
    static QRgb getNewtonColor(int iterations, int maxIterationsUsed, double finalZr, double finalZi);

    double m_juliaCr;
    double m_juliaCi;
    double m_centerX;
    double m_centerY;
    double m_scale;
    int m_maxAnimationSteps;
};

#endif // FRACTALWORKER_H
