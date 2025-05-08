#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->resize(655, 655);
    button = new Liquid_Button(this);
    button->show();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget::updateCircles);
    timer->start(16);
}

Widget::~Widget() {}

void Widget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setBrush(QColor(0, 0, 0, 255));
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, width(), height());

    QImage blur_image(this->size(), QImage::Format_ARGB32);
    blur_image.fill(QColor(0, 0, 0, 0));
    QPainter image_painter(&blur_image);
    image_painter.setRenderHints(QPainter::Antialiasing);
    image_painter.setPen(Qt::NoPen);
    image_painter.setBrush(QColor(255, 0, 0, 255));
    image_painter.drawRoundedRect(button->pos().x() + 10, button->pos().y() + 10, button->width() - 20, button->height() - 20, 10, 10);

    for (const auto& circle : circles)
    {
        QColor currentColor = circle.color;
        currentColor.setAlphaF(circle.alpha);
        image_painter.setBrush(currentColor);
        image_painter.drawEllipse(circle.position.x() - 20, circle.position.y() - 20, 40, 40);
    }
    image_painter.end();

    IMAGE_PROCESSING->qt_blurImage(blur_image, 111, true, false);
    this->processImageTransparency(blur_image, 3, 147);
    this->process(blur_image, 4);
    painter.drawImage(this->rect(), blur_image);
}

void Widget::resizeEvent(QResizeEvent* event)
{
    button->move(this->width() / 2 - button->width() / 2, this->height() / 2 - button->height() / 2);
}

void Widget::updateCircles()
{
    static int counter = 0;
    if (counter % 4 == 0) {
        Circle newCircle;
        newCircle.position = QPointF(width() / 2, height() / 2);
        newCircle.alpha = 1.01;
        qreal angle = QRandomGenerator::global()->bounded(360);
        newCircle.direction = QPointF(qCos(qDegreesToRadians(angle)), qSin(qDegreesToRadians(angle)));
        circles.append(newCircle);
    }

    for (auto it = circles.begin(); it != circles.end(); ) {
        it->position += it->direction * 2.7;
        it->alpha -= 0.007;
        if (it->alpha <= 0 || 
            it->position.x() < -20 || it->position.x() > width() + 20 ||
            it->position.y() < -20 || it->position.y() > height() + 20) {
            it = circles.erase(it);
        } else {
            ++it;
        }
    }

    update();
    counter++;
}

void Widget::processImageTransparency(QImage& image, int alphaMultiplier, int threshold)
{
    if (image.isNull()) {
        return;
    }

    if (image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    const int width = image.width();
    const int height = image.height();
    const int size = width * height;

    for (int i = 0; i < size; i++) {
        QRgb* pixel = reinterpret_cast<QRgb*>(image.bits()) + i;

        int r = qRed(*pixel) * alphaMultiplier;
        int g = qGreen(*pixel) * alphaMultiplier;
        int b = qBlue(*pixel) * alphaMultiplier;
        int a = qAlpha(*pixel) * alphaMultiplier;

        r = qBound(0, r, 255);
        g = qBound(0, g, 255);
        b = qBound(0, b, 255);
        a = qBound(0, a, 255);

        *pixel = qRgba(r, g, b, a);
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            QRgb* pixel = reinterpret_cast<QRgb*>(image.scanLine(y)) + x;

            int r = qRed(*pixel);
            int g = qGreen(*pixel);
            int b = qBlue(*pixel);
            int a = qAlpha(*pixel);

            auto softThreshold = [threshold](int value) {
                if (value <= threshold * 0.7) return 0;
                if (value >= threshold * 1.3) return value;
                return static_cast<int>((value - threshold * 0.7) * (255 / (0.8 * threshold)));
            };

            r = softThreshold(r);
            g = softThreshold(g);
            b = softThreshold(b);
            a = softThreshold(a);

            *pixel = qRgba(r, g, b, a);
        }
    }
}

void Widget::process(QImage& image, int alphaMultiplier)
{
    uint8_t* rgb = image.bits();

    if (nullptr == rgb) {
        return;
    }

    int width = image.width();
    int height = image.height();
    int size = width * height;

    for (int i = 0; i < size; i++) {
        int r = rgb[i * 4 + 0] * alphaMultiplier;
        int g = rgb[i * 4 + 1] * alphaMultiplier;
        int b = rgb[i * 4 + 2] * alphaMultiplier;
        int a = rgb[i * 4 + 3] * alphaMultiplier;

        r = qBound(0, r, 255);
        g = qBound(0, g, 255);
        b = qBound(0, b, 255);
        a = qBound(0, a, 255);
        rgb[i * 4 + 0] = r;
        rgb[i * 4 + 1] = g;
        rgb[i * 4 + 2] = b;
        rgb[i * 4 + 3] = a;
    }
}
