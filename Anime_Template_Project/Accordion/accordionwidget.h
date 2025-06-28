#ifndef ACCORDIONWIDGET_H // 预处理指令：如果未定义 ACCORDIONWIDGET_H 宏，则继续编译
#define ACCORDIONWIDGET_H // 定义 ACCORDIONWIDGET_H 宏，防止头文件被重复包含

#include <QWidget> // 包含 QWidget 头文件，它是所有用户界面对象的基类
#include <QHBoxLayout> // 包含 QHBoxLayout 头文件，用于水平布局管理
#include <QList> // 包含 QList 头文件，用于存储卡片指针的列表
#include <QMap> // 包含 QMap 头文件，用于存储卡片及其当前伸缩值的映射
#include "AccordionCard.h" // 包含 AccordionCard 的头文件，因为 AccordionWidget 将管理 AccordionCard 对象

// AccordionWidget 类：一个手风琴式布局的容器部件
// 继承自 QWidget，用于管理多个 AccordionCard 对象，并根据鼠标悬停事件调整它们的布局伸缩比例
class AccordionWidget : public QWidget
{
    Q_OBJECT // 宏，必须添加到所有声明了信号、槽或属性的类中

public:
    // AccordionWidget 类的构造函数
    // 功能：创建并初始化一个手风琴容器部件
    // @param parent: 父部件指针，默认为 nullptr
    explicit AccordionWidget(QWidget *parent = nullptr);
    // addCard 方法：向手风琴容器中添加一个卡片
    // 功能：将一个 AccordionCard 对象添加到容器的布局中，并建立必要的信号-槽连接
    // @param card: 指向要添加的 AccordionCard 对象的指针
    void addCard(AccordionCard *card);

private slots:
    // onCardHovered 槽函数：处理鼠标悬停在卡片上的事件
    // 功能：当有 AccordionCard 发出 hovered 信号时被调用，用于调整卡片的伸缩比例
    // @param hoveredCard: 指向被鼠标悬停的 AccordionCard 对象的指针
    void onCardHovered(AccordionCard *hoveredCard);
    // onCardLeft 槽函数：处理鼠标离开卡片上的事件
    // 功能：当有 AccordionCard 发出 left 信号时被调用，用于将卡片恢复到默认伸缩比例
    // @param leftCard: 指向鼠标离开的 AccordionCard 对象的指针
    void onCardLeft(AccordionCard *leftCard);
    // onCardStretchChanged 槽函数：处理卡片伸缩比例变化的事件
    // 功能：当 AccordionCard 的 currentStretch 属性发生变化时被调用，用于更新内部记录并触发整体布局更新
    // @param card: 指向伸缩比例发生变化的 AccordionCard 对象的指针
    // @param stretchValue: 该卡片新的伸缩比例值 (qreal 类型)
    void onCardStretchChanged(AccordionCard *card, qreal stretchValue); // 新增槽函数

private:
    QHBoxLayout *m_layout; // 私有成员变量，指向用于排列卡片的水平布局管理器
    QList<AccordionCard *> m_cards; // 私有成员变量，存储所有 AccordionCard 对象的指针列表
    QMap<AccordionCard *, qreal> m_cardCurrentStretches; // 私有成员变量，存储每个卡片当前动画中的伸缩值（用于布局权重）

    // updateLayoutStretches 方法：统一更新布局的伸缩因子
    // 功能：根据 m_cardCurrentStretches 中记录的每个卡片的伸缩值，更新 QHBoxLayout 中对应部件的伸缩因子，从而实现布局的动态调整
    void updateLayoutStretches();
};

#endif // ACCORDIONWIDGET_H // 预处理指令：结束条件编译块
