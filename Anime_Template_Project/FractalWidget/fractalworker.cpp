#include "fractalworker.h"
#include <cmath>
#include <complex>
#include <QVector>
#include <QThreadPool>
#include <QElapsedTimer>
#include <QDebug>

FractalWorker::FractalWorker(QObject *parent)
    : QObject(parent)
{
}

void FractalWorker::doWork(FractalType type, int currentMaxIterations, double juliaCr, double juliaCi,
                           double centerX, double centerY, double scale,
                           int imageWidth, int imageHeight, int maxAnimationSteps)
{
    m_juliaCr = juliaCr;
    m_juliaCi = juliaCi;
    m_centerX = centerX;
    m_centerY = centerY;
    m_scale = scale;
    m_maxAnimationSteps = maxAnimationSteps;

    if (!(type == Mandelbrot || type == Julia || type == BurningShip || type == Newton || type == Tricorn || type == BurningShipJulia || type == Spider)) {
        emit finished();
        return;
    }

    QElapsedTimer timer;
    timer.start();

    int numThreads = QThreadPool::globalInstance()->maxThreadCount();
    if (numThreads == 0) {
        numThreads = QThread::idealThreadCount();
    }
    if (numThreads <= 0) numThreads = 1;

    QVector<FractalChunkData> chunks;
    int rowsPerChunk = imageHeight / numThreads;
    int remainderRows = imageHeight % numThreads;

    for (int i = 0; i < numThreads; ++i) {
        FractalChunkData data;
        data.type = type;
        data.currentMaxIterations = currentMaxIterations;
        data.juliaCr = juliaCr;
        data.juliaCi = juliaCi;
        data.centerX = centerX;
        data.centerY = centerY;
        data.scale = scale;
        data.imageWidth = imageWidth;
        data.imageHeight = imageHeight;
        data.maxAnimationSteps = maxAnimationSteps;
        data.startY = i * rowsPerChunk;
        data.endY = data.startY + rowsPerChunk;
        if (i == numThreads - 1) {
            data.endY += remainderRows;
        }
        chunks.append(data);
    }

    QVector<QFuture<QImage>> futures;
    futures.reserve(chunks.size());

    for (const FractalChunkData& data : chunks) {
        futures.append(QtConcurrent::run(FractalWorker::computeFractalChunk, data));
    }

    for (QFuture<QImage>& future : futures) {
        future.waitForFinished();
    }

    QImage finalImage(imageWidth, imageHeight, QImage::Format_ARGB32);
    for (int i = 0; i < futures.size(); ++i) {
        const QImage &chunkImage = futures.at(i).result();
        const FractalChunkData &chunkData = chunks.at(i);
        for (int y = 0; y < chunkImage.height(); ++y) {
            for (int x = 0; x < chunkImage.width(); ++x) {
                finalImage.setPixel(x, chunkData.startY + y, chunkImage.pixel(x, y));
            }
        }
    }

    qDebug() << "分形计算完成，耗时" << timer.elapsed() << "ms，使用" << numThreads << "个线程。";
    emit imageReady(finalImage);
    emit finished();
}

QImage FractalWorker::computeFractalChunk(const FractalChunkData &data)
{
    QImage chunkImage(data.imageWidth, data.endY - data.startY, QImage::Format_ARGB32);

    for (int y = 0; y < (data.endY - data.startY); ++y) {
        for (int x = 0; x < data.imageWidth; ++x) {
            double globalY = data.startY + y;
            double real = (x - data.imageWidth / 2.0) * data.scale / data.imageWidth + data.centerX;
            double imag = (globalY - data.imageHeight / 2.0) * data.scale / data.imageHeight + data.centerY;

            int iterations = 0;
            QRgb pixelColor = qRgb(0, 0, 0);

            if (data.type == Mandelbrot) {
                double zr = 0.0;
                double zi = 0.0;
                while (zr * zr + zi * zi < 4.0 && iterations < data.currentMaxIterations) {
                    double tempZr = zr * zr - zi * zi + real;
                    zi = 2.0 * zr * zi + imag;
                    zr = tempZr;
                    iterations++;
                }
                pixelColor = getColor(iterations, data.maxAnimationSteps);
            } else if (data.type == Julia) {
                double zr = real;
                double zi = imag;
                while (zr * zr + zi * zi < 4.0 && iterations < data.currentMaxIterations) {
                    double tempZr = zr * zr - zi * zi + data.juliaCr;
                    zi = 2.0 * zr * zi + data.juliaCi;
                    zr = tempZr;
                    iterations++;
                }
                pixelColor = getColor(iterations, data.maxAnimationSteps);
            } else if (data.type == BurningShip) {
                double zr = 0.0;
                double zi = 0.0;
                while (zr * zr + zi * zi < 4.0 && iterations < data.currentMaxIterations) {
                    double absZr = std::abs(zr);
                    double absZi = std::abs(zi);
                    double tempZr = absZr * absZr - absZi * absZi + real;
                    zi = 2.0 * absZr * absZi + imag;
                    zr = tempZr;
                    iterations++;
                }
                pixelColor = getColor(iterations, data.maxAnimationSteps);
            } else if (data.type == Newton) {
                std::complex<double> root1(1.0, 0.0);
                std::complex<double> root2(-0.5, std::sqrt(3.0) / 2.0);
                std::complex<double> root3(-0.5, -std::sqrt(3.0) / 2.0);
                const double tolerance = 1e-6;

                std::complex<double> z(real, imag);
                std::complex<double> finalZ = z;

                while (iterations < data.currentMaxIterations) {
                    std::complex<double> f_z = z * z * z - 1.0;
                    std::complex<double> f_prime_z = 3.0 * z * z;

                    if (std::abs(f_prime_z) < 1e-10) {
                        iterations = data.currentMaxIterations;
                        break;
                    }
                    std::complex<double> z_next = z - f_z / f_prime_z;

                    if (std::abs(z_next - z) < tolerance) {
                        finalZ = z_next;
                        break;
                    }
                    z = z_next;
                    iterations++;
                }
                pixelColor = getNewtonColor(iterations, data.maxAnimationSteps, finalZ.real(), finalZ.imag());
            } else if (data.type == Tricorn) {
                double zr = 0.0;
                double zi = 0.0;
                while (zr * zr + zi * zi < 4.0 && iterations < data.currentMaxIterations) {
                    double tempZr = zr * zr - zi * zi + real;
                    zi = -2.0 * zr * zi + imag;
                    zr = tempZr;
                    iterations++;
                }
                pixelColor = getColor(iterations, data.maxAnimationSteps);
            } else if (data.type == BurningShipJulia) {
                double zr = real;
                double zi = imag;
                while (zr * zr + zi * zi < 4.0 && iterations < data.currentMaxIterations) {
                    double absZr = std::abs(zr);
                    double absZi = std::abs(zi);
                    double tempZr = absZr * absZi - absZi * absZi + data.juliaCr;
                    zi = 2.0 * absZr * absZi + data.juliaCi;
                    zr = tempZr;
                    iterations++;
                }
                pixelColor = getColor(iterations, data.maxAnimationSteps);
            } else if (data.type == Spider) {
                double zr = 0.0;
                double zi = 0.0;
                double cr = real;
                double ci = imag;

                while (zr * zr + zi * zi < 4.0 && iterations < data.currentMaxIterations) {
                    double tempZr = zr * zr - zi * zi + cr;
                    double tempZi = 2.0 * zr * zi + ci;
                    cr = zr;
                    ci = zi;
                    zr = tempZr;
                    zi = tempZi;
                    iterations++;
                }
                pixelColor = getColor(iterations, data.maxAnimationSteps);
            }
            chunkImage.setPixel(x, y, pixelColor);
        }
    }
    return chunkImage;
}

QRgb FractalWorker::getColor(int iterations, int maxIterationsUsed)
{
    if (iterations == maxIterationsUsed) {
        return qRgb(0, 0, 0);
    }
    double t = (double)iterations / maxIterationsUsed;
    int r = (int)(9 * (1 - t) * t * t * t * 255);
    int g = (int)(15 * (1 - t) * (1 - t) * t * t * 255);
    int b = (int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
    return qRgb(r, g, b);
}

QRgb FractalWorker::getNewtonColor(int iterations, int maxIterationsUsed, double finalZr, double finalZi)
{
    if (iterations == maxIterationsUsed) {
        return qRgb(0, 0, 0);
    }

    QRgb baseColor;
    std::complex<double> finalZ(finalZr, finalZi);
    std::complex<double> root1(1.0, 0.0);
    std::complex<double> root2(-0.5, std::sqrt(3.0) / 2.0);
    std::complex<double> root3(-0.5, -std::sqrt(3.0) / 2.0);

    if (std::abs(finalZ - root1) < 1e-5) {
        baseColor = qRgb(255, 0, 0);
    } else if (std::abs(finalZ - root2) < 1e-5) {
        baseColor = qRgb(0, 255, 0);
    } else if (std::abs(finalZ - root3) < 1e-5) {
        baseColor = qRgb(0, 0, 255);
    } else {
        baseColor = qRgb(128, 128, 128);
    }

    double brightness = 1.0 - (double)iterations / maxIterationsUsed;
    int r = qRed(baseColor) * brightness;
    int g = qGreen(baseColor) * brightness;
    int b = qBlue(baseColor) * brightness;

    return qRgb(r, g, b);
}
