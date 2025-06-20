#include "FuzzyTextWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QScreen>
#include <QApplication>
#include <QFontMetricsF>
#include <QRandomGenerator>

FuzzyTextWidget::FuzzyTextWidget(QWidget *parent)
    : QWidget(parent),
    m_text("Hello, Qt!"),
    m_fontSizeStr("clamp(2rem, 10vw, 10rem)"),
    m_fontWeight(QFont::Black),
    m_fontFamily("Arial"),
    m_color(Qt::white),
    m_enableHover(true),
    m_baseIntensity(0.18),
    m_hoverIntensity(0.5),
    m_isHovering(false),
    m_numericFontSize(0.0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &FuzzyTextWidget::updateFuzzyEffect);
    m_animationTimer->start(16);

    updateTextMetrics();
    initializeOffscreenBuffer();
    setMinimumSize(m_offscreenPixmap.size() + QSize(100, 0));
}

FuzzyTextWidget::~FuzzyTextWidget()
{
    m_animationTimer->stop();
    delete m_animationTimer;
}

void FuzzyTextWidget::setText(const QString &text)
{
    if (m_text != text) {
        m_text = text;
        updateTextMetrics();
        initializeOffscreenBuffer();
        update();
    }
}

void FuzzyTextWidget::setFontSize(const QString &fontSize)
{
    if (m_fontSizeStr != fontSize)
    {
        m_fontSizeStr = fontSize;
        updateTextMetrics();
        initializeOffscreenBuffer();
        update();
    }
}

void FuzzyTextWidget::setFontWeight(QFont::Weight fontWeight)
{
    if (m_fontWeight != fontWeight)
    {
        m_fontWeight = fontWeight;
        updateTextMetrics();
        initializeOffscreenBuffer();
        update();
    }
}

void FuzzyTextWidget::setFontFamily(const QString &fontFamily)
{
    if (m_fontFamily != fontFamily)
    {
        m_fontFamily = fontFamily;
        updateTextMetrics();
        initializeOffscreenBuffer();
        update();
    }
}

void FuzzyTextWidget::setColor(const QColor &color)
{
    if (m_color != color)
    {
        m_color = color;
        initializeOffscreenBuffer();
        update();
    }
}

void FuzzyTextWidget::setEnableHover(bool enable)
{
    m_enableHover = enable;
    if (!m_enableHover) {
        m_isHovering = false;
        update();
    }
}

void FuzzyTextWidget::setBaseIntensity(double intensity)
{
    m_baseIntensity = intensity;
}

void FuzzyTextWidget::setHoverIntensity(double intensity)
{
    m_hoverIntensity = intensity;
}

double FuzzyTextWidget::parseFontSize(const QString &fontSizeStr, double defaultSize)
{
    if (fontSizeStr.endsWith("px", Qt::CaseInsensitive))
    {
        return fontSizeStr.chopped(2).toDouble();
    }
    else if (fontSizeStr.endsWith("rem", Qt::CaseInsensitive))
    {
        return fontSizeStr.chopped(3).toDouble() * 16.0;
    }
    else if (fontSizeStr.endsWith("vw", Qt::CaseInsensitive))
    {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen)
        {
            return fontSizeStr.chopped(2).toDouble() / 100.0 * screen->geometry().width();
        }
        return defaultSize;
    }
    else if (fontSizeStr.startsWith("clamp(", Qt::CaseInsensitive) && fontSizeStr.endsWith(")"))
    {
        QString content = fontSizeStr.mid(6, fontSizeStr.length() - 7);
        QStringList parts = content.split(",", Qt::SkipEmptyParts);
        if (parts.size() == 3) {
            double minVal = parseFontSize(parts[0].trimmed(), defaultSize);
            double preferredVal = parseFontSize(parts[1].trimmed(), defaultSize);
            double maxVal = parseFontSize(parts[2].trimmed(), defaultSize);
            return qBound(minVal, preferredVal, maxVal);
        }
    }
    return defaultSize;
}

void FuzzyTextWidget::updateTextMetrics()
{
    QFont font;
    font.setFamily(m_fontFamily);
    font.setWeight(m_fontWeight);
    m_numericFontSize = parseFontSize(m_fontSizeStr, 48.0);
    font.setPixelSize(static_cast<int>(m_numericFontSize));

    QFontMetricsF metrics(font);
    m_textBoundingRect = metrics.boundingRect(m_text).toRect();

    int extraWidthBuffer = 10;
    int horizontalMargin = 50;
    int verticalMargin = 0;

    int offscreenWidth = m_textBoundingRect.width() + extraWidthBuffer;
    int tightHeight = m_textBoundingRect.height();

    setFixedSize(offscreenWidth + horizontalMargin * 2, tightHeight + verticalMargin * 2);

    qDebug() << "Calculated font size:" << m_numericFontSize;
    qDebug() << "Text bounding rect:" << m_textBoundingRect;
    qDebug() << "Widget fixed size:" << size();
}

void FuzzyTextWidget::initializeOffscreenBuffer()
{
    if (m_text.isEmpty())
    {
        m_offscreenPixmap = QPixmap();
        return;
    }

    updateTextMetrics();

    int offscreenWidth = m_textBoundingRect.width() + 10;
    int offscreenHeight = m_textBoundingRect.height();

    m_offscreenPixmap = QPixmap(offscreenWidth, offscreenHeight);
    m_offscreenPixmap.fill(Qt::transparent);

    QPainter painter(&m_offscreenPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font;
    font.setFamily(m_fontFamily);
    font.setWeight(m_fontWeight);
    font.setPixelSize(static_cast<int>(m_numericFontSize));
    painter.setFont(font);
    painter.setPen(m_color);

    painter.drawText(5 - m_textBoundingRect.left(), -m_textBoundingRect.top(), m_text);
}

void FuzzyTextWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_offscreenPixmap.isNull() || m_text.isEmpty())
    {
        return;
    }

    const int fuzzRange = 30;
    double intensity = m_isHovering ? m_hoverIntensity : m_baseIntensity;

    for (int j = 0; j < m_offscreenPixmap.height(); j++)
    {
        int dx = static_cast<int>(intensity * (QRandomGenerator::global()->generateDouble() - 0.5) * fuzzRange);

        painter.drawPixmap(dx + 50,
                           j + 0,
                           m_offscreenPixmap,
                           0,
                           j,
                           m_offscreenPixmap.width(),
                           1);
    }
}

void FuzzyTextWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_enableHover)
    {
        QRect interactiveArea(55, 0, m_textBoundingRect.width(), m_textBoundingRect.height());
        bool nowHovering = interactiveArea.contains(event->pos());

        if (nowHovering != m_isHovering)
        {
            m_isHovering = nowHovering;
            update();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void FuzzyTextWidget::leaveEvent(QEvent *event)
{
    if (m_enableHover && m_isHovering)
    {
        m_isHovering = false;
        update();
    }
    QWidget::leaveEvent(event);
}

void FuzzyTextWidget::resizeEvent(QResizeEvent *event)
{
    updateTextMetrics();
    initializeOffscreenBuffer();
    QWidget::resizeEvent(event);
}

void FuzzyTextWidget::updateFuzzyEffect()
{
    update();
}
