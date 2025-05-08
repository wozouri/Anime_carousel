#ifndef LIQUID_BUTTON_H
#define LIQUID_BUTTON_H

#include <QWidget>
#include <QPainter>
#include <QEvent>
#include <QPainterPath>
#include <QPropertyAnimation>

class Liquid_Button : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color1 READ color1 WRITE setColor1)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)

public:
    explicit Liquid_Button(QWidget *parent = nullptr);
    QPropertyAnimation *m_animation;
    QPropertyAnimation *m_animation2;

    QColor color1() const;
    void setColor1(const QColor &color1);
    QColor textColor() const;
    void setTextColor(const QColor &textColor);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

signals:

private:
    QColor m_color1 = QColor(0, 0, 0);
    QColor m_textColor = QColor(255, 0, 0);
    QString m_text = "液 体";
};

#endif // LIQUID_BUTTON_H
