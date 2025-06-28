#include "AccordionWidget.h" 
#include <QDebug> 

AccordionWidget::AccordionWidget(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(5);
    setLayout(m_layout);
}


void AccordionWidget::addCard(AccordionCard *card)
{
    m_cards.append(card); 
    m_layout->addWidget(card); 

    card->setCurrentStretch(1.0);
    m_cardCurrentStretches.insert(card, 1.0);

    connect(card, &AccordionCard::hovered, this, &AccordionWidget::onCardHovered);
    connect(card, &AccordionCard::left, this, &AccordionWidget::onCardLeft);
    connect(card, &AccordionCard::stretchChanged, this, &AccordionWidget::onCardStretchChanged);

    updateLayoutStretches();
}


void AccordionWidget::onCardHovered(AccordionCard *hoveredCard)
{
    qreal expandedStretch = 5.0; 
    qreal defaultStretch = 1.0;  

    for (AccordionCard *card : m_cards)
    {
        if (card == hoveredCard)
        {
            card->setTargetStretch(expandedStretch);
        }
        else
        {
            card->setTargetStretch(defaultStretch);
        }
    }
}


void AccordionWidget::onCardLeft(AccordionCard *leftCard)
{
    Q_UNUSED(leftCard); 
    qreal defaultStretch = 1.0; 

    for (AccordionCard *card : m_cards)
    {
        card->setTargetStretch(defaultStretch);
    }
}


void AccordionWidget::onCardStretchChanged(AccordionCard *card, qreal stretchValue)
{
    m_cardCurrentStretches.insert(card, stretchValue); 
    updateLayoutStretches();
}


void AccordionWidget::updateLayoutStretches()
{
    if (m_cards.isEmpty()) return;
    for (int i = 0; i < m_cards.size(); ++i)
    {
        AccordionCard *card = m_cards.at(i);

        int layoutStretch = static_cast<int>(m_cardCurrentStretches.value(card, 1.0) * 1000);
        qDebug() << "Card" << i << "current stretch:" << m_cardCurrentStretches.value(card, 1.0)
                 << "layout stretch factor:" << layoutStretch;
        m_layout->setStretch(i, layoutStretch);
    }
}
