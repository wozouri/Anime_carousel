#ifndef FUZZYTEXTWIDGET_H
#define FUZZYTEXTWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QColor>
#include <QFont>

class FuzzyTextWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FuzzyTextWidget(QWidget *parent = nullptr);
    ~FuzzyTextWidget();

    void setText(const QString &text);
    void setFontSize(const QString &fontSize);
    void setFontWeight(QFont::Weight fontWeight);
    void setFontFamily(const QString &fontFamily);
    void setColor(const QColor &color);
    void setEnableHover(bool enable);
    void setBaseIntensity(double intensity);
    void setHoverIntensity(double intensity);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateFuzzyEffect();

private:
    void initializeOffscreenBuffer();
    void updateTextMetrics();
    double parseFontSize(const QString &fontSizeStr, double defaultSize);

    QString m_text;
    QString m_fontSizeStr;
    QFont::Weight m_fontWeight;
    QString m_fontFamily;
    QColor m_color;
    bool m_enableHover;
    double m_baseIntensity;
    double m_hoverIntensity;

    QTimer *m_animationTimer;
    QPixmap m_offscreenPixmap;
    bool m_isHovering;
    QRect m_textBoundingRect;
    double m_numericFontSize;
};

#endif // FUZZYTEXTWIDGET_H
