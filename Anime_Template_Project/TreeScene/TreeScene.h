#pragma once

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QPointF>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QRandomGenerator>
#include <algorithm>

const QColor SAKURA_COLOR = QColor(255, 192, 203);

struct TreeNode {
    QPointF startPoint;
    QPointF endPoint;
    qreal length;
    qreal angle;
    TreeNode* parent;
    TreeNode* left;
    TreeNode* right;
    int depth;
    int indexInAllNodes;

    bool isLeaf;
    bool hasSakura;

    TreeNode() : parent(nullptr), left(nullptr), right(nullptr), depth(0), indexInAllNodes(-1), isLeaf(true), hasSakura(false) {}
    ~TreeNode() {
        delete left;
        delete right;
    }
};

struct SakuraPetal {
    QPointF relativePos;
    qreal rotation;
    qreal opacity;
    qreal sizeFactor;
};

struct FallingSakura {
    QPointF position;
    qreal rotation;
    qreal rotationSpeed;
    qreal speedY;
    qreal opacity;
    QVector<SakuraPetal> petals;

    FallingSakura() : rotation(0), rotationSpeed(0), speedY(0), opacity(1.0) {}
};

class TreeScene : public QWidget {
    Q_OBJECT

public:
    explicit TreeScene(QWidget *parent = nullptr);
    ~TreeScene();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateTreeGrowth();
    void updateSakuraGrowth();
    void updateTreeShake();
    void updateFallingSakura();
    void startSakuraDropTimer();

private:
    TreeNode* root;
    QVector<TreeNode*> allNodes;
    QVector<TreeNode*> leafNodes;
    QVector<TreeNode*> flowerBearingNodes;

    int currentGrowthIndex;
    QTimer* treeGrowthTimer;
    QElapsedTimer growthElapsedTimer;

    QTimer* sakuraGrowthTimer;
    int currentSakuraGrowthIndex;
    QVector<QVector<SakuraPetal>> growingSakuras;

    QTimer* treeShakeTimer;
    qreal shakeAngle;
    qreal shakeDirection;
    qreal maxShakeAngle;
    qreal shakeSpeed;

    QTimer* fallingSakuraTimer;
    QTimer* sakuraDropTriggerTimer;
    QVector<FallingSakura> fallingSakuras;
    qreal fallingSakuraSpeedFactor;

    enum AnimationStage {
        TreeGrowing,
        SakuraGrowing,
        TreeShakingAndSakuraFalling
    };
    AnimationStage currentStage;

    void generateTree(TreeNode* node, int depth, QPointF start, qreal angle, qreal length);
    void drawTree(QPainter& painter, TreeNode* node);
    void drawSakura(QPainter& painter, const QPointF& center, const QVector<SakuraPetal>& petals);
    void drawFallingSakura(QPainter& painter, const FallingSakura& sakura);
    QVector<SakuraPetal> generateSakuraPetals();
};
