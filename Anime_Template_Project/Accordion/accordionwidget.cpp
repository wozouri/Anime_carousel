#include "AccordionWidget.h" // 包含 AccordionWidget 的头文件，通常包含类的声明
#include <QDebug> // 包含 QDebug，用于调试输出

// AccordionWidget 类的构造函数
// @param parent: 父部件指针，默认为 nullptr
AccordionWidget::AccordionWidget(QWidget *parent)
    : QWidget(parent) // 调用父类 QWidget 的构造函数
{
    // 创建一个水平布局管理器
    m_layout = new QHBoxLayout(this);
    // 设置布局中部件之间的间距为5像素
    m_layout->setSpacing(5); // 设置卡片间的间距为0
    // 将当前部件的布局设置为这个水平布局
    setLayout(m_layout);
}

// addCard 方法
// 功能：向手风琴部件中添加一个卡片
// @param card: 指向要添加的 AccordionCard 对象的指针
void AccordionWidget::addCard(AccordionCard *card)
{
    m_cards.append(card); // 将卡片添加到内部管理的卡片列表中
    m_layout->addWidget(card); // 将卡片添加到水平布局中

    // 初始设置卡片的 currentStretch 为 1.0
    card->setCurrentStretch(1.0);
    // 在映射中记录该卡片的当前伸缩比例为 1.0
    m_cardCurrentStretches.insert(card, 1.0);

    // 连接卡片的信号到容器的槽
    // 当鼠标悬停在卡片上时，触发 onCardHovered 槽函数
    connect(card, &AccordionCard::hovered, this, &AccordionWidget::onCardHovered);
    // 当鼠标离开卡片时，触发 onCardLeft 槽函数
    connect(card, &AccordionCard::left, this, &AccordionWidget::onCardLeft);
    // 连接卡片内部 stretch 变化的信号到容器的槽
    // 当卡片的 currentStretch 属性通过动画或直接设置发生变化时，触发 onCardStretchChanged 槽函数
    connect(card, &AccordionCard::stretchChanged, this, &AccordionWidget::onCardStretchChanged);

    // 每次添加卡片后，更新布局以确保初始状态正确
    updateLayoutStretches();
}

// onCardHovered 槽函数
// 功能：处理鼠标悬停在某个卡片上的事件
// @param hoveredCard: 被鼠标悬停的 AccordionCard 对象指针
void AccordionWidget::onCardHovered(AccordionCard *hoveredCard)
{
    qreal expandedStretch = 5.0; // 鼠标悬停卡片的放大倍数，例如放大5倍
    qreal defaultStretch = 1.0;  // 其他卡片的默认倍数，例如保持原始大小

    // 遍历所有卡片
    for (AccordionCard *card : m_cards)
    {
        if (card == hoveredCard)
        {
            // 如果是当前被悬停的卡片，则设置其目标伸缩比例为放大倍数
            card->setTargetStretch(expandedStretch);
        }
        else
        {
            // 如果是其他卡片，则设置其目标伸缩比例为默认倍数（收缩）
            card->setTargetStretch(defaultStretch);
        }
    }
}

// onCardLeft 槽函数
// 功能：处理鼠标离开某个卡片上的事件
// @param leftCard: 鼠标离开的 AccordionCard 对象指针 (此参数未使用，但保留)
void AccordionWidget::onCardLeft(AccordionCard *leftCard)
{
    Q_UNUSED(leftCard); // 标记参数未使用，避免编译器警告
    qreal defaultStretch = 1.0; // 鼠标离开时所有卡片恢复的默认倍数

    // 遍历所有卡片，将它们的伸缩比例都恢复到默认值
    for (AccordionCard *card : m_cards)
    {
        card->setTargetStretch(defaultStretch);
    }
}

// onCardStretchChanged 槽函数
// 功能：处理卡片内部伸缩比例变化的事件
// @param card: 发生伸缩比例变化的 AccordionCard 对象指针
// @param stretchValue: 该卡片新的伸缩比例值
void AccordionWidget::onCardStretchChanged(AccordionCard *card, qreal stretchValue)
{
    // 更新映射中对应卡片的当前伸缩比例
    m_cardCurrentStretches.insert(card, stretchValue); //insert的
    // 每次有卡片 stretch 变化时，重新更新所有布局 stretch
    updateLayoutStretches();
}

// updateLayoutStretches 方法
// 功能：根据所有卡片当前的伸缩比例更新 QHBoxLayout 的 stretch 因子
void AccordionWidget::updateLayoutStretches()
{
    // 如果没有卡片，直接返回
    if (m_cards.isEmpty()) return;

    // 遍历所有卡片及其在布局中的索引
    for (int i = 0; i < m_cards.size(); ++i)
    {
        AccordionCard *card = m_cards.at(i);
        // QHBoxLayout 的 stretch 因子需要整数。
        // 我们将 qreal 类型的 currentStretch 乘以一个因子（例如1000）转换为整数，
        // 以保留动画的平滑度，同时确保总和足够大以避免精度损失。
        // 从映射中获取卡片的当前伸缩比例，如果不存在则默认为1.0
        int layoutStretch = static_cast<int>(m_cardCurrentStretches.value(card, 1.0) * 1000);
        qDebug() << "Card" << i << "current stretch:" << m_cardCurrentStretches.value(card, 1.0)
                 << "layout stretch factor:" << layoutStretch;
        // 设置布局中对应卡片的伸缩因子，这会影响卡片在布局中占用的空间比例
        m_layout->setStretch(i, layoutStretch);
    }
}
