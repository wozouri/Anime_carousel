#include "GlitchEffectWidget.h"
#include <QDebug>
#include <QFontMetricsF>
#include <cmath>
#include <QPainterPath>
#include <QElapsedTimer>
#include <QCoreApplication>

GlitchRenderThread::GlitchRenderThread(QObject *parent)
    : QThread(parent),
    m_columns(0), m_rows(0),
    m_charWidth(0), m_charHeight(0), m_fontSize(0),
    m_centerVignette(false), m_outerVignette(false),
    m_widgetWidth(0), m_widgetHeight(0),
    m_isDirtyFullRedraw(false)
{
    setObjectName("GlitchRenderThread");
}

GlitchRenderThread::~GlitchRenderThread()
{
    requestInterruption();
    m_condition.wakeAll();
    wait();
}

void GlitchRenderThread::setRenderData(const std::vector<GlitchLetter>& letters, int columns, int rows,
                                       int charWidth, int charHeight, int fontSize,
                                       bool centerVignette, bool outerVignette,
                                       int widgetWidth, int widgetHeight)
{
    QMutexLocker locker(&m_mutex);
    m_letters = letters;
    m_columns = columns;
    m_rows = rows;
    m_charWidth = charWidth;
    m_charHeight = charHeight;
    m_fontSize = fontSize;
    m_centerVignette = centerVignette;
    m_outerVignette = outerVignette;
    m_widgetWidth = widgetWidth;
    m_widgetHeight = widgetHeight;
    m_isDirtyFullRedraw = true;
    m_pendingChangedIndexes.clear();
}

void GlitchRenderThread::requestRender(const QList<int>& changedIndexes)
{
    QMutexLocker locker(&m_mutex);
    if (!changedIndexes.isEmpty()) {
        m_pendingChangedIndexes.append(changedIndexes);
    }
    if (!m_isDirtyFullRedraw || !changedIndexes.isEmpty()) {
        m_condition.wakeOne();
    }
}

void GlitchRenderThread::run()
{
    qDebug() << "GlitchRenderThread started";
    while (!isInterruptionRequested()) {
        m_mutex.lock();
        if (!m_isDirtyFullRedraw && m_pendingChangedIndexes.empty()) {
            m_condition.wait(&m_mutex);
        }
        bool doFullRedraw = m_isDirtyFullRedraw;
        m_isDirtyFullRedraw = false;

        QList<int> currentChangedIndexes = m_pendingChangedIndexes;
        m_pendingChangedIndexes.clear();
        m_mutex.unlock();

        if (isInterruptionRequested()) {
            break;
        }

        if (m_widgetWidth <= 0 || m_widgetHeight <= 0 || m_letters.empty() || m_columns <= 0 || m_rows <= 0) {
            continue;
        }

        QSharedPointer<QImage> currentFrame;
        QList<QRect> dirtyRects;

        if (doFullRedraw || m_lastRenderedImage.isNull() ||
            m_lastRenderedImage->width() != m_widgetWidth || m_lastRenderedImage->height() != m_widgetHeight)
        {
            currentFrame = QSharedPointer<QImage>::create(m_widgetWidth, m_widgetHeight, QImage::Format_ARGB32_Premultiplied);
            currentFrame->fill(Qt::black);
            dirtyRects.append(currentFrame->rect());

            QPainter painter(currentFrame.data());
            painter.setRenderHint(QPainter::Antialiasing, true);
            QFont font("monospace", m_fontSize);
            painter.setFont(font);

            for (size_t i = 0; i < m_letters.size(); ++i) {
                const GlitchLetter& letter = m_letters.at(i);
                int x = (i % m_columns) * m_charWidth;
                int y = (i / m_columns) * m_charHeight;
                painter.setPen(letter.color);
                painter.drawText(x, y + m_fontSize, QString(letter.character));
            }
            drawVignettes(painter, m_widgetWidth, m_widgetHeight);

        } else {
            currentFrame = m_lastRenderedImage;
            QPainter painter(currentFrame.data());
            painter.setRenderHint(QPainter::Antialiasing, true);
            QFont font("monospace", m_fontSize);
            painter.setFont(font);

            for (int index : currentChangedIndexes) {
                if (index < 0 || index >= static_cast<int>(m_letters.size())) continue;

                const GlitchLetter& letter = m_letters.at(index);
                int x = (index % m_columns) * m_charWidth;
                int y = (index / m_columns) * m_charHeight;

                QRect charRect(x, y, m_charWidth, m_charHeight);
                painter.fillRect(charRect, Qt::black);

                painter.setPen(letter.color);
                painter.drawText(x, y + m_fontSize, QString(letter.character));

                dirtyRects.append(charRect);
            }
        }

        m_lastRenderedImage = currentFrame;

        emit frameReady(currentFrame, dirtyRects);
    }
    qDebug() << "GlitchRenderThread stopped";
}

QList<QRect> GlitchRenderThread::mergeRects(const QList<QRect>& rects) {
    if (rects.isEmpty()) return QList<QRect>();
    QRect bounding = rects.first();
    for (int i = 1; i < rects.size(); ++i) {
        bounding = bounding.united(rects.at(i));
    }
    return QList<QRect>() << bounding;
}

void GlitchRenderThread::drawVignettes(QPainter& painter, int w, int h)
{
    if (m_outerVignette) {
        QRadialGradient outerGradient(w / 2.0, h / 2.0, qMin(w, h) / 2.0);
        outerGradient.setColorAt(0.6, Qt::transparent);
        outerGradient.setColorAt(1.0, Qt::black);
        painter.setBrush(outerGradient);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, w, h);
    }

    if (m_centerVignette) {
        QRadialGradient centerGradient(w / 2.0, h / 2.0, qMin(w, h) / 2.0);
        centerGradient.setColorAt(0.0, QColor(0, 0, 0, 204));
        centerGradient.setColorAt(0.6, Qt::transparent);
        painter.setBrush(centerGradient);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, w, h);
    }
}
GlitchEffectWidget::GlitchEffectWidget(QWidget *parent)
    : QWidget(parent),
    m_glitchColors({QColor(43, 69, 57, 255), QColor(97, 220, 163, 255), QColor(97, 179, 220, 255)}),
    m_glitchSpeed(22),
    m_smoothTransitions(true),
    m_centerVignette(false),
    m_outerVignette(true),
    m_fontSize(16)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);

    m_lettersAndSymbols = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$&*()-_+=/[]{};:<>,0123456789";

    QFont font("monospace", m_fontSize);
    QFontMetricsF fm(font);
    m_charWidth = static_cast<int>(fm.averageCharWidth());
    m_charHeight = static_cast<int>(fm.height());

    m_glitchTimer = new QTimer(this);
    connect(m_glitchTimer, &QTimer::timeout, this, &GlitchEffectWidget::updateGlitch);
    m_glitchTimer->start(m_glitchSpeed);

    m_smoothTimer = new QTimer(this);
    connect(m_smoothTimer, &QTimer::timeout, this, &GlitchEffectWidget::handleSmoothColorTransitions);
    m_smoothTimer->start(16);

    int numThreads = QThread::idealThreadCount();
    if (numThreads <= 0) {
        numThreads = 1;
    }
    qDebug() << "Creating" << numThreads << "render threads.";

    for (int i = 0; i < numThreads; ++i) {
        GlitchRenderThread *thread = new GlitchRenderThread(this);
        thread->setObjectName(QString("GlitchRenderThread_%1").arg(i));
        connect(thread, &GlitchRenderThread::frameReady, this, &GlitchEffectWidget::handleFrameReady);
        m_renderThreads.append(thread);
        thread->start();
    }

    m_displayTimer = new QTimer(this);
    connect(m_displayTimer, &QTimer::timeout, this, &GlitchEffectWidget::displayNextFrame);
    m_displayTimer->start(16);
}

GlitchEffectWidget::~GlitchEffectWidget()
{
    m_displayTimer->stop();

    for (GlitchRenderThread *thread : m_renderThreads) {
        if (thread->isRunning()) {
            thread->requestInterruption();
        }
    }
    m_queueCondition.wakeAll();
    for (GlitchRenderThread *thread : m_renderThreads) {
        thread->wait();
        delete thread;
    }
    m_renderThreads.clear();
}

void GlitchEffectWidget::setGlitchColors(const QList<QColor>& colors)
{
    m_glitchColors = colors;
    QList<int> changedIndexes;
    QMutexLocker locker(&m_lettersMutex);
    for (size_t i = 0; i < m_letters.size(); ++i) {
        GlitchLetter& letter = m_letters[i];
        letter.color = getRandomColor();
        letter.targetColor = getRandomColor();
        letter.colorProgress = 1.0;
        changedIndexes.append(static_cast<int>(i));
    }
    for (GlitchRenderThread* thread : m_renderThreads) {
        thread->setRenderData(m_letters, m_columns, m_rows, m_charWidth, m_charHeight,
                              m_fontSize, m_centerVignette, m_outerVignette,
                              static_cast<int>(width() * devicePixelRatioF()), static_cast<int>(height() * devicePixelRatioF()));
        thread->requestRender(changedIndexes);
    }
}

void GlitchEffectWidget::setGlitchSpeed(int speed)
{
    m_glitchSpeed = speed;
    m_glitchTimer->start(m_glitchSpeed);
}

void GlitchEffectWidget::setSmoothTransitions(bool smooth)
{
    m_smoothTransitions = smooth;
    if (m_smoothTransitions) {
        m_smoothTimer->start(16);
    } else {
        m_smoothTimer->stop();
        QList<int> changedIndexes;
        QMutexLocker locker(&m_lettersMutex);
        for (size_t i = 0; i < m_letters.size(); ++i) {
            GlitchLetter& letter = m_letters[i];
            if (letter.colorProgress < 1.0) {
                letter.color = letter.targetColor;
                letter.colorProgress = 1.0;
                changedIndexes.append(static_cast<int>(i));
            }
        }
        for (GlitchRenderThread* thread : m_renderThreads) {
            thread->setRenderData(m_letters, m_columns, m_rows, m_charWidth, m_charHeight,
                                  m_fontSize, m_centerVignette, m_outerVignette,
                                  static_cast<int>(width() * devicePixelRatioF()), static_cast<int>(height() * devicePixelRatioF()));
            thread->requestRender(changedIndexes);
        }
    }
}



void GlitchEffectWidget::setCenterVignette(bool enabled)
{
    m_centerVignette = enabled;
    for (GlitchRenderThread* thread : m_renderThreads) {
        thread->setRenderData(m_letters, m_columns, m_rows, m_charWidth, m_charHeight,
                              m_fontSize, m_centerVignette, m_outerVignette,
                              static_cast<int>(width() * devicePixelRatioF()), static_cast<int>(height() * devicePixelRatioF()));
        thread->requestRender();
    }
}

void GlitchEffectWidget::setOuterVignette(bool enabled)
{
    m_outerVignette = enabled;
    for (GlitchRenderThread* thread : m_renderThreads) {
        thread->setRenderData(m_letters, m_columns, m_rows, m_charWidth, m_charHeight,
                              m_fontSize, m_centerVignette, m_outerVignette,
                              static_cast<int>(width() * devicePixelRatioF()), static_cast<int>(height() * devicePixelRatioF()));
        thread->requestRender();
    }
}

QColor GlitchEffectWidget::hexToQColor(const QString& hex) {
    QString actualHex = hex;
    if (actualHex.startsWith("#")) {
        actualHex = actualHex.mid(1);
    }
    if (actualHex.length() == 3) {
        actualHex = QString(actualHex.at(0)) + actualHex.at(0) +
                    QString(actualHex.at(1)) + actualHex.at(1) +
                    QString(actualHex.at(2)) + actualHex.at(2);
    }
    return QColor("#" + actualHex);
}

QChar GlitchEffectWidget::getRandomChar()
{
    return m_lettersAndSymbols.at(QRandomGenerator::global()->bounded(m_lettersAndSymbols.length()));
}

QColor GlitchEffectWidget::getRandomColor()
{
    return m_glitchColors.at(QRandomGenerator::global()->bounded(m_glitchColors.length()));
}

void GlitchEffectWidget::calculateGrid(int width, int height)
{
    QFont font("monospace", m_fontSize);
    QFontMetricsF fm(font);
    m_charWidth = static_cast<int>(fm.averageCharWidth());
    m_charHeight = static_cast<int>(fm.height());

    qreal dpr = devicePixelRatioF();
    int actualWidth = static_cast<int>(width * dpr);
    int actualHeight = static_cast<int>(height * dpr);

    m_columns = qCeil(static_cast<qreal>(actualWidth) / m_charWidth);
    m_rows = qCeil(static_cast<qreal>(actualHeight) / m_charHeight);
}

void GlitchEffectWidget::initializeLetters()
{
    QMutexLocker locker(&m_lettersMutex);
    m_letters.clear();
    int totalLetters = m_columns * m_rows;
    m_letters.reserve(totalLetters);

    for (int i = 0; i < totalLetters; ++i) {
        m_letters.push_back({getRandomChar(), getRandomColor(), getRandomColor(), 1.0});
    }

    for (GlitchRenderThread* thread : m_renderThreads) {
        thread->setRenderData(m_letters, m_columns, m_rows, m_charWidth, m_charHeight,
                              m_fontSize, m_centerVignette, m_outerVignette,
                              static_cast<int>(width() * devicePixelRatioF()), static_cast<int>(height() * devicePixelRatioF()));
        thread->requestRender();
    }
}

void GlitchEffectWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    if (!m_currentDisplayedImage.isNull()) {
        if (!m_currentDirtyRects.isEmpty()) {
            qreal dpr = devicePixelRatioF();
            for (const QRect& dirtyRect : m_currentDirtyRects) {
                QRect actualDirtyRect(dirtyRect.x() * dpr, dirtyRect.y() * dpr,
                                      dirtyRect.width() * dpr, dirtyRect.height() * dpr);
                painter.drawImage(dirtyRect.topLeft(), *m_currentDisplayedImage, actualDirtyRect);
            }
        } else {
            painter.drawImage(0, 0, *m_currentDisplayedImage);
        }
    }
}

void GlitchEffectWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    calculateGrid(event->size().width(), event->size().height());
    initializeLetters();
}

void GlitchEffectWidget::updateGlitch()
{
    QMutexLocker locker(&m_lettersMutex);
    if (m_letters.empty()) {
        return;
    }

    int updateCount = qMax(1, static_cast<int>(m_letters.size() * 0.05));
    QList<int> changedIndexes;

    for (int i = 0; i < updateCount; ++i) {
        int index = QRandomGenerator::global()->bounded(static_cast<quint32>(m_letters.size()));
        GlitchLetter& letter = m_letters[index];

        letter.character = getRandomChar();
        letter.targetColor = getRandomColor();

        if (!m_smoothTransitions) {
            letter.color = letter.targetColor;
            letter.colorProgress = 1.0;
        } else {
            letter.colorProgress = 0.0;
        }
        changedIndexes.append(index);
    }
    for (GlitchRenderThread* thread : m_renderThreads) {
        thread->setRenderData(m_letters, m_columns, m_rows, m_charWidth, m_charHeight,
                              m_fontSize, m_centerVignette, m_outerVignette,
                              static_cast<int>(width() * devicePixelRatioF()), static_cast<int>(height() * devicePixelRatioF()));
        thread->requestRender(changedIndexes);
    }
}

void GlitchEffectWidget::handleSmoothColorTransitions()
{
    if (!m_smoothTransitions) {
        return;
    }

    QList<int> changedIndexes;
    QMutexLocker locker(&m_lettersMutex);
    for (size_t i = 0; i < m_letters.size(); ++i) {
        GlitchLetter& letter = m_letters[i];
        if (letter.colorProgress < 1.0) {
            letter.colorProgress += 0.05;
            if (letter.colorProgress > 1.0) {
                letter.colorProgress = 1.0;
            }

            int startR = letter.color.red();
            int startG = letter.color.green();
            int startB = letter.color.blue();

            int endR = letter.targetColor.red();
            int endG = letter.targetColor.green();
            int endB = letter.targetColor.blue();

            int r = qRound(startR + (endR - startR) * letter.colorProgress);
            int g = qRound(startG + (endG - startG) * letter.colorProgress);
            int b = qRound(startB + (endB - startB) * letter.colorProgress);

            letter.color = QColor(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255));
            changedIndexes.append(static_cast<int>(i));
        }
    }

    if (!changedIndexes.isEmpty()) {
        for (GlitchRenderThread* thread : m_renderThreads) {
            thread->setRenderData(m_letters, m_columns, m_rows, m_charWidth, m_charHeight,
                                  m_fontSize, m_centerVignette, m_outerVignette,
                                  static_cast<int>(width() * devicePixelRatioF()), static_cast<int>(height() * devicePixelRatioF()));
            thread->requestRender(changedIndexes);
        }
    }
}

void GlitchEffectWidget::handleFrameReady(QSharedPointer<QImage> image, const QList<QRect>& dirtyRects)
{
    QMutexLocker locker(&m_queueMutex);
    m_renderedFrameQueue.enqueue(qMakePair(image, dirtyRects));
    m_queueCondition.wakeOne();
}

void GlitchEffectWidget::displayNextFrame()
{
    QMutexLocker locker(&m_queueMutex);
    if (!m_renderedFrameQueue.isEmpty()) {
        QPair<QSharedPointer<QImage>, QList<QRect>> nextFrame = m_renderedFrameQueue.dequeue();
        m_currentDisplayedImage = nextFrame.first;
        m_currentDirtyRects = nextFrame.second;

        if (!m_currentDirtyRects.isEmpty()) {
            QRect unitedRect = m_currentDirtyRects.first();
            for (int i = 1; i < m_currentDirtyRects.size(); ++i) {
                unitedRect = unitedRect.united(m_currentDirtyRects.at(i));
            }
            update(unitedRect);
        } else {
            update();
        }
    }
}
