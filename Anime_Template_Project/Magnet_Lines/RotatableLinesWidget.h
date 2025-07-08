#ifndef ROTATABLELINESWIDGET_H
#define ROTATABLELINESWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPoint>
#include <QTimer>

class RotatableLinesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RotatableLinesWidget(QWidget *parent = nullptr);
    ~RotatableLinesWidget();

    void setLinesPerRow(int count);
    void setLinesPerCol(int count);
    void setRotationAngle(int angle);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateMousePosition();

private:
    int m_linesPerRow;
    int m_linesPerCol;
    int m_baseRotationAngle;

    QPoint m_globalMousePos;
    QTimer *m_mousePollTimer;

    qreal m_padding;
};

#endif
