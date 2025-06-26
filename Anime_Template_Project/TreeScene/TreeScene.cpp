#include "TreeScene.h"
#include <QPainter>
#include <QDebug>
#include <cmath>
#include <QPainterPath>

#define PI 3.14159265358979323846

TreeScene::TreeScene(QWidget *parent)
    : QWidget(parent),
    root(nullptr),
    currentGrowthIndex(0),
    currentSakuraGrowthIndex(0),
    shakeAngle(0),
    shakeDirection(1),
    maxShakeAngle(2.0),
    shakeSpeed(0.1),
    fallingSakuraSpeedFactor(0.5),
    currentStage(TreeGrowing)
{
    setFixedSize(800, 600);
    setStyleSheet("background-color: lightblue;");

    treeGrowthTimer = new QTimer(this);
    connect(treeGrowthTimer, &QTimer::timeout, this, &TreeScene::updateTreeGrowth);
    treeGrowthTimer->start(50);

    sakuraGrowthTimer = new QTimer(this);
    connect(sakuraGrowthTimer, &QTimer::timeout, this, &TreeScene::updateSakuraGrowth);

    treeShakeTimer = new QTimer(this);
    connect(treeShakeTimer, &QTimer::timeout, this, &TreeScene::updateTreeShake);

    fallingSakuraTimer = new QTimer(this);
    connect(fallingSakuraTimer, &QTimer::timeout, this, &TreeScene::updateFallingSakura);

    sakuraDropTriggerTimer = new QTimer(this);
    connect(sakuraDropTriggerTimer, &QTimer::timeout, this, &TreeScene::startSakuraDropTimer);

    QPointF startPoint(width() / 2, height() - 50);
    root = new TreeNode();
    root->startPoint = startPoint;
    root->endPoint = startPoint;
    root->length = 0;
    root->angle = -90;
    root->depth = 0;
    root->indexInAllNodes = allNodes.size();
    allNodes.append(root);

    generateTree(root, 0, root->startPoint, root->angle, 0);
    qDebug() << "Total nodes generated:" << allNodes.size();
    qDebug() << "Leaf nodes generated:" << leafNodes.size();
    qDebug() << "Flower-bearing nodes generated (depth >= 5):" << flowerBearingNodes.size();

    growthElapsedTimer.start();
}

TreeScene::~TreeScene() {
    delete root;
}

void TreeScene::generateTree(TreeNode* node, int depth, QPointF start, qreal angle, qreal length)
{
    bool shouldBeLeaf = (depth >= 9);

    node->startPoint = start;
    node->angle = angle;
    node->depth = depth;

    qreal branchLengthFactor = QRandomGenerator::global()->generateDouble() * (1.2 - 0.8) + 0.8;
    qreal branchLength = (200.0 / (depth + 1.0)) * branchLengthFactor;

    node->length = branchLength;
    node->endPoint.setX(start.x() + branchLength * std::cos(angle * PI / 180.0));
    node->endPoint.setY(start.y() + branchLength * std::sin(angle * PI / 180.0));

    node->isLeaf = shouldBeLeaf;

    if (node->depth >= 5)
    {
        flowerBearingNodes.append(node);
    }

    if (node->depth < 8) {
        qreal angleOffset1 = QRandomGenerator::global()->bounded(0, 15);
        qreal branchAngle1 = angle - 25 - angleOffset1;

        qreal angleOffset2 = QRandomGenerator::global()->bounded(0, 15);
        qreal branchAngle2 = angle + 25 + angleOffset2;

        node->left = new TreeNode();
        node->left->parent = node;
        node->left->indexInAllNodes = allNodes.size();
        allNodes.append(node->left);
        generateTree(node->left, depth + 1, node->endPoint, branchAngle1, branchLength * 0.8);

        node->right = new TreeNode();
        node->right->parent = node;
        node->right->indexInAllNodes = allNodes.size();
        allNodes.append(node->right);
        generateTree(node->right, depth + 1, node->endPoint, branchAngle2, branchLength * 0.8);
    } else {
    }
}

void TreeScene::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawTree(painter, root);

    for (int i = 0; i < growingSakuras.size(); ++i) {
        if (i < flowerBearingNodes.size() && flowerBearingNodes[i]->hasSakura) {
            drawSakura(painter, flowerBearingNodes[i]->endPoint, growingSakuras[i]);
        }
    }

    for (const auto& sakura : fallingSakuras) {
        drawFallingSakura(painter, sakura);
    }
}

void TreeScene::drawTree(QPainter& painter, TreeNode* node)
{
    if (!node) return;

    QPen pen(Qt::darkGreen);
    pen.setWidth(qMax(1, 10 - node->depth * 1));
    painter.setPen(pen);

    painter.save();

    if (currentStage == TreeShakingAndSakuraFalling)
    {
        qreal maxTreeDepth = 9.0;
        qreal depthShakeFactor = static_cast<qreal>(node->depth) / maxTreeDepth;
        qreal currentBranchShakeAngle = shakeAngle * depthShakeFactor;

        painter.translate(node->startPoint);
        painter.rotate(currentBranchShakeAngle);
        painter.translate(-node->startPoint);
    }

    if (currentStage == TreeGrowing)
    {
        int nodeIndex = node->indexInAllNodes;

        if (nodeIndex <= currentGrowthIndex)
        {
            qreal progress = 1.0;
            if (nodeIndex == currentGrowthIndex)
            {
                qint64 elapsedMs = growthElapsedTimer.elapsed();
                qreal growthDurationPerNode = 50.0;
                progress = qMin(1.0, elapsedMs / growthDurationPerNode);
            }
            QPointF actualEndPoint;
            actualEndPoint.setX(node->startPoint.x() + node->length * progress * std::cos(node->angle * PI / 180.0));
            actualEndPoint.setY(node->startPoint.y() + node->length * progress * std::sin(node->angle * PI / 180.0));
            painter.drawLine(node->startPoint, actualEndPoint);
        }
    }
    else
    {
        painter.drawLine(node->startPoint, node->endPoint);
    }

    painter.restore();

    drawTree(painter, node->left);
    drawTree(painter, node->right);
}

void TreeScene::drawSakura(QPainter& painter, const QPointF& center, const QVector<SakuraPetal>& petals)
{
    painter.setBrush(SAKURA_COLOR);
    painter.setPen(Qt::NoPen);

    QPainterPath petalPath;
    qreal baseWidth = 10.0;
    qreal baseHeight = 15.0;

    petalPath.moveTo(0, baseHeight / 2.0);
    petalPath.cubicTo(baseWidth * 0.4, baseHeight * 0.3,
                      baseWidth * 0.4, -baseHeight * 0.3,
                      0, -baseHeight / 2.0);
    petalPath.cubicTo(-baseWidth * 0.4, -baseHeight * 0.3,
                      -baseWidth * 0.4, baseHeight * 0.3,
                      0, baseHeight / 2.0);
    petalPath.closeSubpath();

    for (const auto& petal : petals)
    {
        painter.save();
        painter.translate(center);
        painter.rotate(petal.rotation);
        painter.setOpacity(petal.opacity);

        painter.scale(petal.sizeFactor, petal.sizeFactor);

        painter.translate(petal.relativePos);

        painter.drawPath(petalPath);
        painter.restore();
    }
    painter.setOpacity(1.0);
}

void TreeScene::drawFallingSakura(QPainter& painter, const FallingSakura& sakura)
{
    painter.setBrush(SAKURA_COLOR);
    painter.setPen(Qt::NoPen);
    painter.setOpacity(sakura.opacity);

    QPainterPath petalPath;
    qreal baseWidth = 10.0;
    qreal baseHeight = 15.0;
    petalPath.moveTo(0, baseHeight / 2.0);
    petalPath.cubicTo(baseWidth * 0.4, baseHeight * 0.3, baseWidth * 0.4, -baseHeight * 0.3, 0, -baseHeight / 2.0);
    petalPath.cubicTo(-baseWidth * 0.4, -baseHeight * 0.3, -baseWidth * 0.4, baseHeight * 0.3, 0, baseHeight / 2.0);
    petalPath.closeSubpath();

    for (const auto& petal : sakura.petals)
    {
        painter.save();
        painter.translate(sakura.position);
        painter.rotate(sakura.rotation + petal.rotation);

        painter.drawPath(petalPath);
        painter.restore();
    }
    painter.setOpacity(1.0);
}

QVector<SakuraPetal> TreeScene::generateSakuraPetals()
{
    QVector<SakuraPetal> petals;
    int numPetals = 5;
    qreal petalAngleStep = 360.0 / numPetals;

    for (int i = 0; i < numPetals; ++i)
    {
        SakuraPetal petal;
        petal.relativePos = QPointF(0, 0);
        petal.rotation = i * petalAngleStep;
        petal.opacity = 1.0;
        petal.sizeFactor = 0.3;
        petals.append(petal);
    }
    return petals;
}

void TreeScene::updateTreeGrowth()
{
    if (currentGrowthIndex < allNodes.size())
    {
        TreeNode* currentNode = allNodes[currentGrowthIndex];
        qint64 elapsedMs = growthElapsedTimer.elapsed();
        qreal growthDurationPerNode = 50.0;

        if (elapsedMs >= growthDurationPerNode)
        {
            currentGrowthIndex++;
            growthElapsedTimer.restart();
        }
        update();
    }
    else
    {
        treeGrowthTimer->stop();
        qDebug() << "Tree growth finished.";
        currentStage = SakuraGrowing;
        if (!flowerBearingNodes.isEmpty())
        {
            for(int i = 0; i < flowerBearingNodes.size(); ++i)
            {
                growingSakuras.append(generateSakuraPetals());
            }
            sakuraGrowthTimer->start(30);
            qDebug() << "Sakura growth started. Growing " << growingSakuras.size() << " sakuras.";
        }
        else
        {
            qDebug() << "No flower-bearing nodes found, skipping sakura growth.";
            currentStage = TreeShakingAndSakuraFalling;
            treeShakeTimer->start(20);
            sakuraDropTriggerTimer->start(QRandomGenerator::global()->bounded(500, 1500));
            fallingSakuraTimer->start(30);
            qDebug() << "Tree shaking and sakura falling started (no sakuras grown).";
        }
    }
}

void TreeScene::updateSakuraGrowth()
{
    if (currentSakuraGrowthIndex < flowerBearingNodes.size())
    {
        TreeNode* currentFlowerNode = flowerBearingNodes[currentSakuraGrowthIndex];
        currentFlowerNode->hasSakura = true;

        QVector<SakuraPetal>& petals = growingSakuras[currentSakuraGrowthIndex];
        bool allPetalsGrown = true;
        for (auto& petal : petals)
        {
            if (petal.sizeFactor < 1.0)
            {
                petal.sizeFactor += 0.2;
                petal.relativePos.setX(petal.relativePos.x() + (QRandomGenerator::global()->generateDouble() - 0.5));
                petal.relativePos.setY(petal.relativePos.y() + (QRandomGenerator::global()->generateDouble() - 0.5));
                allPetalsGrown = false;
            }
        }
        update();

        if (allPetalsGrown) {
            currentSakuraGrowthIndex++;
        }
    } else {
        sakuraGrowthTimer->stop();
        qDebug() << "Sakura growth finished.";
        currentStage = TreeShakingAndSakuraFalling;
        treeShakeTimer->start(20);
        sakuraDropTriggerTimer->start(QRandomGenerator::global()->bounded(500, 1500));
        fallingSakuraTimer->start(30);
        qDebug() << "Tree shaking and sakura falling started.";
    }
}

void TreeScene::updateTreeShake() {
    shakeAngle += shakeDirection * shakeSpeed;
    if (std::abs(shakeAngle) >= maxShakeAngle) {
        shakeDirection *= -1;
    }
    update();
}

void TreeScene::startSakuraDropTimer()
{
    FallingSakura newSakura;
    if (!flowerBearingNodes.isEmpty())
    {
        int flowerNodeIndex = QRandomGenerator::global()->bounded(flowerBearingNodes.size());
        if (flowerNodeIndex < growingSakuras.size())
        {
            newSakura.position = flowerBearingNodes[flowerNodeIndex]->endPoint;
            newSakura.petals = growingSakuras[flowerNodeIndex];
        }
        else
        {
            newSakura.position = QPointF(QRandomGenerator::global()->bounded(width()), QRandomGenerator::global()->bounded(height() / 4));
            newSakura.petals = generateSakuraPetals();
            for(auto& petal : newSakura.petals) petal.sizeFactor = 1.0;
        }
    }
    else
    {
        newSakura.position = QPointF(QRandomGenerator::global()->bounded(width()), QRandomGenerator::global()->bounded(height() / 2));
        newSakura.petals = generateSakuraPetals();
        for(auto& petal : newSakura.petals) petal.sizeFactor = 1.0;
    }

    qreal speedFactor = QRandomGenerator::global()->generateDouble() * (3.0 - 1.0) + 1.0;
    newSakura.speedY = speedFactor * fallingSakuraSpeedFactor;

    qreal rotationSpeedFactor = QRandomGenerator::global()->generateDouble() * (5.0 - (-5.0)) + (-5.0);
    newSakura.rotationSpeed = rotationSpeedFactor;

    newSakura.opacity = 1.0;
    fallingSakuras.append(newSakura);

    sakuraDropTriggerTimer->start(QRandomGenerator::global()->bounded(300, 1000));
}

void TreeScene::updateFallingSakura()
{
    for (int i = 0; i < fallingSakuras.size(); ++i)
    {
        FallingSakura& sakura = fallingSakuras[i];
        sakura.position.setY(sakura.position.y() + sakura.speedY);
        qreal windOffsetX = QRandomGenerator::global()->generateDouble() - 0.5;
        sakura.position.setX(sakura.position.x() + windOffsetX);
        sakura.rotation += sakura.rotationSpeed;

        if (sakura.position.y() > height() * 0.75)
        {
            sakura.opacity -= 0.02;
            if (sakura.opacity < 0)
            {
                sakura.opacity = 0;
            }
        }
    }

    fallingSakuras.erase(std::remove_if(fallingSakuras.begin(), fallingSakuras.end(),
                                        [](const FallingSakura& s){ return s.opacity <= 0; }),
                         fallingSakuras.end());
    update();
}
