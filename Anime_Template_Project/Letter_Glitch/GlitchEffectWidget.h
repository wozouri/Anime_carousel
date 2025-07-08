#ifndef GLITCHEFFECTWIDGET_H
#define GLITCHEFFECTWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QColor>
#include <QPainter>
#include <QResizeEvent>
#include <QRandomGenerator>
#include <vector>
#include <QImage>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QObject>
#include <QSharedPointer>
#include <QRect>
#include <QList>
#include <QQueue>

struct GlitchLetter {
    QChar character;
    QColor color;
    QColor targetColor;
    double colorProgress;
};

class GlitchRenderThread : public QThread
{
    Q_OBJECT

public:
    GlitchRenderThread(QObject *parent = nullptr);
    ~GlitchRenderThread();

    void setRenderData(const std::vector<GlitchLetter>& letters, int columns, int rows,
                       int charWidth, int charHeight, int fontSize,
                       bool centerVignette, bool outerVignette,
                       int widgetWidth, int widgetHeight);

    void requestRender(const QList<int>& changedIndexes = QList<int>());

signals:
    void frameReady(QSharedPointer<QImage> image, const QList<QRect>& dirtyRects);

protected:
    void run() override;

private:
    std::vector<GlitchLetter> m_letters;
    int m_columns;
    int m_rows;
    int m_charWidth;
    int m_charHeight;
    int m_fontSize;
    bool m_centerVignette;
    bool m_outerVignette;
    int m_widgetWidth;
    int m_widgetHeight;

    bool m_isDirtyFullRedraw;
    QList<int> m_pendingChangedIndexes;
    QMutex m_mutex;
    QWaitCondition m_condition;

    QSharedPointer<QImage> m_lastRenderedImage;

    void drawVignettes(QPainter& painter, int w, int h);
    QList<QRect> mergeRects(const QList<QRect>& rects);
};

class GlitchEffectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlitchEffectWidget(QWidget *parent = nullptr);
    ~GlitchEffectWidget();

    void setGlitchColors(const QList<QColor>& colors);
    void setGlitchSpeed(int speed);
    void setSmoothTransitions(bool smooth);
    void setCenterVignette(bool enabled);
    void setOuterVignette(bool enabled);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateGlitch();
    void handleSmoothColorTransitions();
    void handleFrameReady(QSharedPointer<QImage> image, const QList<QRect>& dirtyRects);
    void displayNextFrame();

private:
    QList<QColor> m_glitchColors;
    int m_glitchSpeed;
    bool m_smoothTransitions;
    bool m_centerVignette;
    bool m_outerVignette;

    QTimer *m_glitchTimer;
    QTimer *m_smoothTimer;
    QTimer *m_displayTimer;

    std::vector<GlitchLetter> m_letters;
    int m_columns;
    int m_rows;

    int m_fontSize;
    int m_charWidth;
    int m_charHeight;

    QString m_lettersAndSymbols;

    QColor hexToQColor(const QString& hex);
    QChar getRandomChar();
    QColor getRandomColor();
    void calculateGrid(int width, int height);
    void initializeLetters();

    QList<GlitchRenderThread*> m_renderThreads;
    QQueue<QPair<QSharedPointer<QImage>, QList<QRect>>> m_renderedFrameQueue;
    QMutex m_queueMutex;
    QWaitCondition m_queueCondition;

    QSharedPointer<QImage> m_currentDisplayedImage;
    QList<QRect> m_currentDirtyRects;

    QMutex m_lettersMutex;
};

#endif
