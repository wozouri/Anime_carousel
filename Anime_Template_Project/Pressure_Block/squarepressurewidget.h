#ifndef SQUAREPRESSUREWIDGET_H
#define SQUAREPRESSUREWIDGET_H

#include <QtWidgets/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QCursor>
#include <QtCore/QTimer>
#include <QtCore/QVector>
#include <QtCore/QPointF>
#include <QtCore/QRectF>

struct SquareProperties
{
    QChar character;
    QRectF rect;
    double currentWeight;
    double currentAlpha;
    QColor currentColor;
    QPointF initialCenter;
};

class SquarePressureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SquarePressureWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateAnimation();

private:
    void calculateSquareProperties();
    double dist(const QPointF &p1, const QPointF &p2);

    QString m_text;
    double m_minSquareSize;
    QPointF m_mousePos;
    QPointF m_cursorPos;
    QTimer *m_timer;
    QVector<SquareProperties> m_squareProps;
};

#endif // SQUAREPRESSUREWIDGET_H
