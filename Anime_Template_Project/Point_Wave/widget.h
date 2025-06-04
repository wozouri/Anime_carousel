#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QResizeEvent>
#include <QTimer>
struct DotData {
    QPointF point; 
    qreal  dotRadius;
};
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void addDot();
    bool isPointInAnnulus(const QPointF &testPoint, const QPointF &centerPoint, qreal innerRadius, qreal outerRadius);

public slots:
    void Radius_Processing();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    QList<DotData> m_dots;
    qreal m_dotRadius = 3.0;

    int w = 600;
    int h = 600;

    QTimer * timer;
    QPointF Wcenter;

    qreal r;
    qreal r1 = 0;
    qreal r2;


};
#endif // WIDGET_H
