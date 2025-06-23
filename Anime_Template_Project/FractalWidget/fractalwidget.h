#ifndef FRACTALWIDGET_H
#define FRACTALWIDGET_H

#include <QWidget>
#include <QImage>
#include <QTimer>
#include <QThread>
#include <QPoint>
#include <QPointF>

#include "fractalworker.h"

class FractalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FractalWidget(QWidget *parent = nullptr);
    ~FractalWidget();

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
    Q_ENUM(FractalType)

    void setFractalType(FractalType type);
    void setJuliaC(double cr, double ci);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void animateFractal();
    void onImageReady(const QImage &image);
    void onWorkerFinished();

private:
    FractalType m_currentFractalType;
    double m_juliaCr;
    double m_juliaCi;

    QTimer *m_animationTimer;
    int m_animationStep;
    int m_maxAnimationSteps;

    double m_centerX;
    double m_centerY;
    double m_scale;
    double m_defaultCenterX;
    double m_defaultCenterY;
    double m_defaultScale;

    int m_lineFractalOrder;
    int m_defaultLineFractalOrder;

    QThread *m_workerThread;
    FractalWorker *m_fractalWorker;
    QImage m_image;
    bool m_isCalculating;

    bool m_isDragging;
    QPoint m_lastMousePos;
    QPoint m_accumulatedDragDelta;
    QPointF m_imageTopLeft;


    void startAnimation();
    void stopAnimation();
    void startFractalCalculationThread(int currentMaxIterations);
    void updateLoadingState(bool loading);

    void generateKochSnowflake(QPainter &painter, int order, QPointF p1, QPointF p2);
    void generateSierpinskiCurve(QPainter &painter, int order, QPointF p1, QPointF p2, QPointF p3);
};

#endif // FRACTALWIDGET_H
