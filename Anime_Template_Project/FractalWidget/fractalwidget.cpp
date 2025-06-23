#include "fractalwidget.h"
#include <QPainter>
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <QRandomGenerator>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QElapsedTimer>

FractalWidget::FractalWidget(QWidget *parent)
    : QWidget(parent),
    m_currentFractalType(Mandelbrot),
    m_juliaCr(-0.7),
    m_juliaCi(0.27015),
    m_animationTimer(new QTimer(this)),
    m_animationStep(0),
    m_maxAnimationSteps(500),
    m_centerX(0.0),
    m_centerY(0.0),
    m_scale(2.0),
    m_defaultCenterX(0.0),
    m_defaultCenterY(0.0),
    m_defaultScale(2.0),
    m_lineFractalOrder(1),
    m_defaultLineFractalOrder(1),
    m_workerThread(new QThread(this)),
    m_fractalWorker(new FractalWorker()),
    m_isCalculating(false),
    m_isDragging(false),
    m_accumulatedDragDelta(0, 0),
    m_imageTopLeft(0, 0)
{
    resize(600, 600);

    connect(m_animationTimer, &QTimer::timeout, this, &FractalWidget::animateFractal);

    m_fractalWorker->moveToThread(m_workerThread);

    connect(m_fractalWorker, &FractalWorker::imageReady, this, &FractalWidget::onImageReady);
    connect(m_workerThread, &QThread::finished, m_fractalWorker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
    connect(m_fractalWorker, &FractalWorker::finished, this, &FractalWidget::onWorkerFinished);

    m_workerThread->start();
}

FractalWidget::~FractalWidget()
{
    if (m_workerThread->isRunning())
    {
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

void FractalWidget::setFractalType(FractalType type)
{
    bool typeChanged = (m_currentFractalType != type);
    m_currentFractalType = type;

    m_imageTopLeft = QPointF(0, 0);

    if (m_currentFractalType == Mandelbrot ||
        m_currentFractalType == Julia ||
        m_currentFractalType == BurningShip ||
        m_currentFractalType == Newton ||
        m_currentFractalType == Tricorn ||
        m_currentFractalType == BurningShipJulia ||
        m_currentFractalType == Spider) {
        if (typeChanged || (std::abs(m_centerX - m_defaultCenterX) > 1e-9 ||
                            std::abs(m_centerY - m_defaultCenterY) > 1e-9 ||
                            std::abs(m_scale - m_defaultScale) > 1e-9)) {
            m_centerX = m_defaultCenterX;
            m_centerY = m_defaultCenterY;
            m_scale = m_defaultScale;
            qDebug() << "迭代型分形视图已重置到默认。";
        }
    }
    else
    {
        if (typeChanged || m_lineFractalOrder != m_defaultLineFractalOrder)
        {
            m_lineFractalOrder = m_defaultLineFractalOrder;
            qDebug() << "线条分形阶数已重置到默认。";
        }
    }
    startAnimation();
}

void FractalWidget::setJuliaC(double cr, double ci)
{
    m_juliaCr = cr;
    m_juliaCi = ci;
    if (m_currentFractalType == Julia || m_currentFractalType == BurningShipJulia)
    {
        startAnimation();
    }
}

void FractalWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    if (!m_image.isNull() && (m_currentFractalType == Mandelbrot ||
                              m_currentFractalType == Julia ||
                              m_currentFractalType == BurningShip ||
                              m_currentFractalType == Newton ||
                              m_currentFractalType == Tricorn ||
                              m_currentFractalType == BurningShipJulia ||
                              m_currentFractalType == Spider)) {
        if (m_isDragging)
        {
            painter.drawImage(m_imageTopLeft, m_image);
        }
        else
        {
            painter.drawImage(0, 0, m_image);
        }
    }
    else if (m_currentFractalType == KochSnowflake ||
             m_currentFractalType == SierpinskiCurve)
    {
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::white);

        qreal minSide = std::min(width(), height()) * 0.8;

        if (m_currentFractalType == KochSnowflake)
        {
            QPointF p1(width() / 2.0 - minSide / 2.0, height() / 2.0 + minSide * std::sqrt(3.0) / 6.0);
            QPointF p2(width() / 2.0 + minSide / 2.0, height() / 2.0 + minSide * std::sqrt(3.0) / 6.0);
            QPointF p3(width() / 2.0, height() / 2.0 - minSide * std::sqrt(3.0) / 3.0);

            generateKochSnowflake(painter, m_animationStep, p1, p2);
            generateKochSnowflake(painter, m_animationStep, p2, p3);
            generateKochSnowflake(painter, m_animationStep, p3, p1);
        } else if (m_currentFractalType == SierpinskiCurve)
        {
            qreal side = minSide * 0.8;
            QPointF p1(width() / 2.0 - side / 2.0, height() / 2.0 + side * std::sqrt(3.0) / 6.0);
            QPointF p2(width() / 2.0 + side / 2.0, height() / 2.0 + side * std::sqrt(3.0) / 6.0);
            QPointF p3(width() / 2.0, height() / 2.0 - side * std::sqrt(3.0) / 3.0);
            generateSierpinskiCurve(painter, m_animationStep, p1, p2, p3);
        }
    }

    if (m_isCalculating && (m_currentFractalType == Mandelbrot ||
                            m_currentFractalType == Julia ||
                            m_currentFractalType == BurningShip ||
                            m_currentFractalType == Newton ||
                            m_currentFractalType == Tricorn ||
                            m_currentFractalType == BurningShipJulia ||
                            m_currentFractalType == Spider)) {
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 24));
        painter.drawText(rect(), Qt::AlignCenter, "计算中...");
    }
}

void FractalWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    setFractalType(m_currentFractalType);
}

void FractalWidget::wheelEvent(QWheelEvent *event)
{
    if (!(m_currentFractalType == Mandelbrot ||
          m_currentFractalType == Julia ||
          m_currentFractalType == BurningShip ||
          m_currentFractalType == Newton ||
          m_currentFractalType == Tricorn ||
          m_currentFractalType == BurningShipJulia ||
          m_currentFractalType == Spider))
    {
        event->ignore();
        return;
    }

    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 4;

    qreal zoomFactor = 1.0;
    if (!numPixels.isNull())
    {
        zoomFactor = std::pow(1.0015, numPixels.y());
    } else if (!numDegrees.isNull())
    {
        zoomFactor = std::pow(1.1, numDegrees.y() / 15.0);
    }

    if (zoomFactor == 1.0)
    {
        event->ignore();
        return;
    }

    QPoint mousePos = event->position().toPoint();
    double mouseX = mousePos.x();
    double mouseY = mousePos.y();

    double oldReal = (mouseX - width() / 2.0) * m_scale / width() + m_centerX;
    double oldImag = (mouseY - height() / 2.0) * m_scale / height() + m_centerY;

    m_scale /= zoomFactor;

    m_centerX = oldReal - (mouseX - width() / 2.0) * m_scale / width();
    m_centerY = oldImag - (mouseY - height() / 2.0) * m_scale / height();

    m_scale = std::max(1e-10, std::min(1000.0, m_scale));

    startAnimation();
    event->accept();
}

void FractalWidget::mousePressEvent(QMouseEvent *event)
{
    if (!(m_currentFractalType == Mandelbrot ||
          m_currentFractalType == Julia ||
          m_currentFractalType == BurningShip ||
          m_currentFractalType == Newton ||
          m_currentFractalType == Tricorn ||
          m_currentFractalType == BurningShipJulia ||
          m_currentFractalType == Spider)) {
        event->ignore();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_lastMousePos = event->position().toPoint();
        m_accumulatedDragDelta = QPoint(0, 0);
        m_imageTopLeft = QPointF(0, 0);

        updateLoadingState(true);

        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void FractalWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && (m_currentFractalType == Mandelbrot ||
                         m_currentFractalType == Julia ||
                         m_currentFractalType == BurningShip ||
                         m_currentFractalType == Newton ||
                         m_currentFractalType == Tricorn ||
                         m_currentFractalType == BurningShipJulia ||
                         m_currentFractalType == Spider)) {
        QPoint currentMousePos = event->position().toPoint();
        QPoint delta = currentMousePos - m_lastMousePos;

        m_imageTopLeft += delta;

        m_centerX -= delta.x() * m_scale / width();
        m_centerY -= delta.y() * m_scale / height();

        m_lastMousePos = currentMousePos;

        m_accumulatedDragDelta += delta;

        if (std::abs(m_accumulatedDragDelta.x()) >= 5 || std::abs(m_accumulatedDragDelta.y()) >= 5) {
            update();
            m_accumulatedDragDelta = QPoint(0, 0);
        }

        event->accept();
    } else {
        event->ignore();
    }
}

void FractalWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isDragging) {
            m_isDragging = false;
            startAnimation();
        }
        event->accept();
    } else {
        event->ignore();
    }
}

void FractalWidget::startAnimation()
{
    stopAnimation();

    m_animationStep = 0;

    if (m_currentFractalType == Mandelbrot ||
        m_currentFractalType == Julia ||
        m_currentFractalType == BurningShip ||
        m_currentFractalType == Newton ||
        m_currentFractalType == Tricorn ||
        m_currentFractalType == BurningShipJulia ||
        m_currentFractalType == Spider) {
        m_maxAnimationSteps = 500;

        m_animationTimer->start(25);
    } else {
        int orderVariation = QRandomGenerator::global()->bounded(-2, 3);
        m_maxAnimationSteps = m_lineFractalOrder + orderVariation;

        m_maxAnimationSteps = std::max(4, m_maxAnimationSteps);

        m_animationTimer->start(1);
    }
}

void FractalWidget::stopAnimation()
{
    if (m_animationTimer->isActive()) {
        m_animationTimer->stop();
    }
    if (m_isCalculating) {
        qDebug() << "动画停止，但分形计算仍在进行中。当前帧将完成。";
    }
    updateLoadingState(false);
}

void FractalWidget::animateFractal()
{
    if (m_animationStep > m_maxAnimationSteps) {
        stopAnimation();
        return;
    }

    QElapsedTimer timer;
    timer.start();

    if (m_currentFractalType == Mandelbrot ||
        m_currentFractalType == Julia ||
        m_currentFractalType == BurningShip ||
        m_currentFractalType == Newton ||
        m_currentFractalType == Tricorn ||
        m_currentFractalType == BurningShipJulia ||
        m_currentFractalType == Spider) {
        if (!m_isCalculating) {
            updateLoadingState(true);
            startFractalCalculationThread(m_animationStep);
        }
    } else {
        update();
        qDebug() << "线条分形 动画步进:" << m_animationStep << "耗时:" << timer.elapsed() << "ms";
    }

    if (m_currentFractalType == KochSnowflake ||
        m_currentFractalType == SierpinskiCurve) {
        m_animationStep += 1;
    } else {
        m_animationStep += 10;
    }
    m_animationStep = std::min(m_animationStep, m_maxAnimationSteps + 1);
}

void FractalWidget::onImageReady(const QImage &image)
{
    m_image = image;
    update();
    qDebug() << "收到工作线程图像。更新UI。";
    updateLoadingState(false);
}

void FractalWidget::onWorkerFinished()
{
}

void FractalWidget::startFractalCalculationThread(int currentMaxIterations)
{
    QMetaObject::invokeMethod(m_fractalWorker, "doWork",
                              Qt::QueuedConnection,
                              Q_ARG(FractalWorker::FractalType, (FractalWorker::FractalType)m_currentFractalType),
                              Q_ARG(int, currentMaxIterations),
                              Q_ARG(double, m_juliaCr),
                              Q_ARG(double, m_juliaCi),
                              Q_ARG(double, m_centerX),
                              Q_ARG(double, m_centerY),
                              Q_ARG(double, m_scale),
                              Q_ARG(int, width()),
                              Q_ARG(int, height()),
                              Q_ARG(int, m_maxAnimationSteps));

    m_isCalculating = true;
}

void FractalWidget::updateLoadingState(bool loading) {
    m_isCalculating = loading;
    update();
}

void FractalWidget::generateKochSnowflake(QPainter &painter, int order, QPointF p1, QPointF p2)
{
    if (order == 0) {
        painter.drawLine(p1, p2);
        return;
    }

    QPointF p_a = p1;
    QPointF p_b(p1.x() + (p2.x() - p1.x()) / 3.0, p1.y() + (p2.y() - p1.y()) / 3.0);
    QPointF p_c(p1.x() + 2.0 * (p2.x() - p1.x()) / 3.0, p1.y() + 2.0 * (p2.y() - p1.y()) / 3.0);
    QPointF p_d = p2;

    qreal dx = p2.x() - p1.x();
    qreal dy = p2.y() - p1.y();

    QPointF p_e(p_b.x() + dx * std::cos(M_PI / 3.0) - dy * std::sin(M_PI / 3.0),
                p_b.y() + dx * std::sin(M_PI / 3.0) + dy * std::cos(M_PI / 3.0));

    generateKochSnowflake(painter, order - 1, p_a, p_b);
    generateKochSnowflake(painter, order - 1, p_b, p_e);
    generateKochSnowflake(painter, order - 1, p_e, p_c);
    generateKochSnowflake(painter, order - 1, p_c, p_d);
}

void FractalWidget::generateSierpinskiCurve(QPainter &painter, int order, QPointF p1, QPointF p2, QPointF p3)
{
    if (order == 0) {
        painter.drawLine(p1, p2);
        painter.drawLine(p2, p3);
        painter.drawLine(p3, p1);
        return;
    }

    QPointF p12((p1.x() + p2.x()) / 2.0, (p1.y() + p2.y()) / 2.0);
    QPointF p23((p2.x() + p3.x()) / 2.0, (p2.y() + p3.y()) / 2.0);
    QPointF p31((p3.x() + p1.x()) / 2.0, (p1.y() + p3.y()) / 2.0);

    generateSierpinskiCurve(painter, order - 1, p1, p12, p31);
    generateSierpinskiCurve(painter, order - 1, p12, p2, p23);
    generateSierpinskiCurve(painter, order - 1, p31, p23, p3);
}
