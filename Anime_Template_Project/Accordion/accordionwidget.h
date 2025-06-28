#ifndef ACCORDIONWIDGET_H 
#define ACCORDIONWIDGET_H 

#include <QWidget>
#include <QHBoxLayout>
#include <QList> 
#include <QMap> 
#include "AccordionCard.h"

class AccordionWidget : public QWidget
{
    Q_OBJECT

public:

    explicit AccordionWidget(QWidget *parent = nullptr);

    void addCard(AccordionCard *card);

private slots:

    void onCardHovered(AccordionCard *hoveredCard);

    void onCardLeft(AccordionCard *leftCard);

    void onCardStretchChanged(AccordionCard *card, qreal stretchValue);

private:
    QHBoxLayout *m_layout;
    QList<AccordionCard *> m_cards; 
    QMap<AccordionCard *, qreal> m_cardCurrentStretches; 

    void updateLayoutStretches();
};

#endif // ACCORDIONWIDGET_H // 预处理指令：结束条件编译块
