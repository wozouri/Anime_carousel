#include "draggableelasticobject.h"
#include <QPainter>
#include <QtMath>
#include <QRandomGenerator>
#include <QDebug>

DraggableElasticObject::DraggableElasticObject(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    if (parent) {
        resize(parent->size());
    } else {
        resize(800, 600);
    }

    m_physicsTimer = new QTimer(this);
    connect(m_physicsTimer, &QTimer::timeout, this, &DraggableElasticObject::updatePhysics);
    m_physicsTimer->start(static_cast<int>(TIME_STEP * 1000));

    setMouseTracking(true);
}

DraggableElasticObject::~DraggableElasticObject()
{
    delete m_physicsTimer;
}

void DraggableElasticObject::addBlock(const PhysicsBlockProperties& block)
{
    m_blocks.append(block);
    update();
}

void DraggableElasticObject::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const PhysicsBlockProperties& block : m_blocks)
    {
        painter.setPen(block.color);
        painter.setBrush(Qt::NoBrush);

        painter.drawRect(block.position.x(), block.position.y(),
                         block.size.width(), block.size.height());

        if (!block.name.isEmpty())
        {
            QFont font;
            font.setPointSize(15);
            font.setBold(true);
            painter.setFont(font);
            painter.drawText(QRectF(block.position.x(), block.position.y(), block.size.width(), block.size.height()), Qt::AlignCenter, block.name);
        }
    }
}

void DraggableElasticObject::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        for (int i = 0; i < m_blocks.size(); ++i) {
            PhysicsBlockProperties& block = m_blocks[i];
            QRectF blockRect(block.position, block.size);
            if (blockRect.contains(event->pos())) {
                m_isDragging = true;
                m_draggedBlockIndex = i;
                m_dragOffset = event->pos() - block.position;
                m_lastMousePos = event->pos();
                m_lastMousePressTime = QDateTime::currentMSecsSinceEpoch();
                m_lastMouseVelocity = {0.0, 0.0};
                break;
            }
        }
    }
}

void DraggableElasticObject::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && m_draggedBlockIndex != -1) {
        PhysicsBlockProperties& block = m_blocks[m_draggedBlockIndex];

        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        double deltaTime = (currentTime - m_lastMousePressTime) / 1000.0;

        if (deltaTime > 0) {
            QPointF currentMouseVelocity = (event->pos() - m_lastMousePos) / deltaTime;
            m_lastMouseVelocity = currentMouseVelocity;
        }
        m_lastMousePos = event->pos();
        m_lastMousePressTime = currentTime;

        update();
    }
}

void DraggableElasticObject::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isDragging && m_draggedBlockIndex != -1) {
        PhysicsBlockProperties& block = m_blocks[m_draggedBlockIndex];
        block.isMovable = true;

        double throwFactor = 1.5;
        block.velocity = m_lastMouseVelocity * throwFactor;

        m_isDragging = false;
        m_draggedBlockIndex = -1;
        update();
    }
}

void DraggableElasticObject::updatePhysics()
{
    for (int i = 0; i < m_blocks.size(); ++i) {
        PhysicsBlockProperties& block = m_blocks[i];
        block.isOnSomething = false;

        if (m_isDragging && m_draggedBlockIndex == i) {
            block.acceleration = {0.0, 0.0};
        } else {
            applyForces(block);
            block.velocity += block.acceleration * TIME_STEP;
        }
    }

    int collisionIterations = 5;
    for (int iter = 0; iter < collisionIterations; ++iter) {
        for (int i = 0; i < m_blocks.size(); ++i) {
            PhysicsBlockProperties& block1 = m_blocks[i];

            for (int j = i + 1; j < m_blocks.size(); ++j) {
                PhysicsBlockProperties& block2 = m_blocks[j];
                handleBlockToBlockCollision(block1, block2);
            }
        }
    }

    for (int i = 0; i < m_blocks.size(); ++i) {
        PhysicsBlockProperties& block = m_blocks[i];

        if (m_isDragging && m_draggedBlockIndex == i) {
            block.position = m_lastMousePos - m_dragOffset;
            block.velocity = {0.0, 0.0};
            block.acceleration = {0.0, 0.0};
            continue;
        }

        block.position += block.velocity * TIME_STEP;

        handleBoundaryCollision(block);

        bool onGround = (block.position.y() >= (height() - block.size.height() - EPSILON));
        applyFrictionAndCheckStop(block, onGround);
    }

    update();
}

void DraggableElasticObject::applyForces(PhysicsBlockProperties& block)
{
    if (!block.isOnSomething) {
        block.acceleration.setY(GRAVITY_ACCELERATION);
    } else {
        block.acceleration.setY(0.0);
    }
    block.acceleration.setX(0.0);
}

void DraggableElasticObject::updateVelocityAndPredictedPosition(PhysicsBlockProperties& block, QPointF& predictedPosition) const
{
    predictedPosition = block.position + block.velocity * TIME_STEP;
}

void DraggableElasticObject::handleBlockToBlockCollision(PhysicsBlockProperties& blockA, PhysicsBlockProperties& blockB)
{
    QRectF rectA(blockA.position, blockA.size);
    QRectF rectB(blockB.position, blockB.size);

    if (!rectA.intersects(rectB)) {
        return;
    }

    bool isBlockADragged = (m_isDragging && m_draggedBlockIndex != -1 && &blockA == &m_blocks[m_draggedBlockIndex]);
    bool isBlockBDragged = (m_isDragging && m_draggedBlockIndex != -1 && &blockB == &m_blocks[m_draggedBlockIndex]);

    if (!blockA.isMovable && !blockB.isMovable && !isBlockADragged && !isBlockBDragged) {
        return;
    }

    if (isBlockADragged && isBlockBDragged) {
        return;
    }

    double overlapX = 0.0;
    double overlapY = 0.0;

    double blockARight = blockA.position.x() + blockA.size.width();
    double blockALeft = blockA.position.x();
    double blockABottom = blockA.position.y() + blockA.size.height();
    double blockATop = blockA.position.y();

    double blockBRight = blockB.position.x() + blockB.size.width();
    double blockBLeft = blockB.position.x();
    double blockBBottom = blockB.position.y() + blockB.size.height();
    double blockBTop = blockB.position.y();

    if (blockARight > blockBLeft && blockALeft < blockBRight) {
        overlapX = qMin(blockARight - blockBLeft, blockBRight - blockALeft);
    }

    if (blockABottom > blockBTop && blockATop < blockBBottom) {
        overlapY = qMin(blockABottom - blockBTop, blockBBottom - blockATop);
    }

    if (qAbs(overlapX) < qAbs(overlapY)) {
        PhysicsBlockProperties* movableBlock = nullptr;
        PhysicsBlockProperties* fixedBlock = nullptr;

        if (isBlockADragged || !blockA.isMovable) {
            fixedBlock = &blockA;
            movableBlock = &blockB;
        } else if (isBlockBDragged || !blockB.isMovable) {
            fixedBlock = &blockB;
            movableBlock = &blockA;
        } else {
            double totalMovableMass = blockA.mass + blockB.mass;
            if (totalMovableMass < EPSILON) return;

            double separationAmount = overlapX;
            if (blockALeft < blockBLeft) {
                blockA.position.setX(blockA.position.x() - separationAmount * (blockB.mass / totalMovableMass));
                blockB.position.setX(blockB.position.x() + separationAmount * (blockA.mass / totalMovableMass));
            } else {
                blockA.position.setX(blockA.position.x() + separationAmount * (blockB.mass / totalMovableMass));
                blockB.position.setX(blockB.position.x() - separationAmount * (blockA.mass / totalMovableMass));
            }
            goto velocity_resolution_x;
        }

        if (fixedBlock && movableBlock && movableBlock->isMovable) {
            double separationAmount = overlapX;
            if (movableBlock->position.x() < fixedBlock->position.x()) {
                movableBlock->position.setX(movableBlock->position.x() - separationAmount);
            } else {
                movableBlock->position.setX(movableBlock->position.x() + separationAmount);
            }
            goto velocity_resolution_x;
        } else {
            return;
        }

    } else {
        PhysicsBlockProperties* movableBlock = nullptr;
        PhysicsBlockProperties* fixedBlock = nullptr;

        if (isBlockADragged || !blockA.isMovable) {
            fixedBlock = &blockA;
            movableBlock = &blockB;
        } else if (isBlockBDragged || !blockB.isMovable) {
            fixedBlock = &blockB;
            movableBlock = &blockA;
        } else {
            double totalMovableMass = blockA.mass + blockB.mass;
            if (totalMovableMass < EPSILON) return;

            double separationAmount = overlapY;
            if (blockATop < blockBTop) {
                blockA.position.setY(blockA.position.y() - separationAmount * (blockB.mass / totalMovableMass));
                blockB.position.setY(blockB.position.y() + separationAmount * (blockA.mass / totalMovableMass));
                blockA.isOnSomething = true;
            } else {
                blockA.position.setY(blockA.position.y() + separationAmount * (blockB.mass / totalMovableMass));
                blockB.position.setY(blockB.position.y() - separationAmount * (blockA.mass / totalMovableMass));
                blockB.isOnSomething = true;
            }
            goto velocity_resolution_y;
        }

        if (fixedBlock && movableBlock && movableBlock->isMovable) {
            double separationAmount = overlapY;
            if (movableBlock->position.y() < fixedBlock->position.y()) {
                movableBlock->position.setY(movableBlock->position.y() - separationAmount);
                movableBlock->isOnSomething = true;
            } else {
                movableBlock->position.setY(movableBlock->position.y() + separationAmount);
            }
            goto velocity_resolution_y;
        } else {
            return;
        }
    }

velocity_resolution_x:
    if (!blockA.isMovable && !blockB.isMovable && !isBlockADragged && !isBlockBDragged) return;

    if (isBlockADragged && blockB.isMovable) {
        double vRelX = blockA.velocity.x() - blockB.velocity.x();
        if ((blockALeft < blockBLeft && vRelX > 0) || (blockALeft > blockBLeft && vRelX < 0)) {
            double e = qMin(blockA.restitutionCoefficient, blockB.restitutionCoefficient);
            double j = -(1 + e) * vRelX * blockB.mass;
            blockB.velocity.setX(blockB.velocity.x() - j / blockB.mass);
        }
    } else if (isBlockBDragged && blockA.isMovable) {
        double vRelX = blockA.velocity.x() - blockB.velocity.x();
        if ((blockALeft < blockBLeft && vRelX > 0) || (blockALeft > blockBLeft && vRelX < 0)) {
            double e = qMin(blockA.restitutionCoefficient, blockB.restitutionCoefficient);
            double j = -(1 + e) * vRelX * blockA.mass;
            blockA.velocity.setX(blockA.velocity.x() + j / blockA.mass);
        }
    } else {
        double vRelX = blockA.velocity.x() - blockB.velocity.x();

        if ((blockALeft < blockBLeft && vRelX > 0) || (blockALeft > blockBLeft && vRelX < 0)) {
            double e = qMin(blockA.restitutionCoefficient, blockB.restitutionCoefficient);
            double j = -(1 + e) * vRelX;

            double invMassA = blockA.isMovable ? (1.0 / blockA.mass) : 0.0;
            double invMassB = blockB.isMovable ? (1.0 / blockB.mass) : 0.0;
            double totalInvMass = invMassA + invMassB;

            if (totalInvMass > EPSILON) {
                j /= totalInvMass;

                if (blockA.isMovable) {
                    blockA.velocity.setX(blockA.velocity.x() + j * invMassA);
                    if (qAbs(blockA.velocity.x()) < EPSILON) blockA.velocity.setX(0.0);
                }
                if (blockB.isMovable) {
                    blockB.velocity.setX(blockB.velocity.x() - j * invMassB);
                    if (qAbs(blockB.velocity.x()) < EPSILON) blockB.velocity.setX(0.0);
                }
            }
        }
    }
    return;

velocity_resolution_y:
    if (!blockA.isMovable && !blockB.isMovable && !isBlockADragged && !isBlockBDragged) return;

    if (isBlockADragged && blockB.isMovable) {
        double vRelY = blockA.velocity.y() - blockB.velocity.y();
        if ((blockATop < blockBTop && vRelY > 0) || (blockATop > blockBTop && vRelY < 0)) {
            double e = qMin(blockA.restitutionCoefficient, blockB.restitutionCoefficient);
            double j = -(1 + e) * vRelY * blockB.mass;
            blockB.velocity.setY(blockB.velocity.y() - j / blockB.mass);
            blockB.isOnSomething = true;
        }
    } else if (isBlockBDragged && blockA.isMovable) {
        double vRelY = blockA.velocity.y() - blockB.velocity.y();
        if ((blockATop < blockBTop && vRelY > 0) || (blockATop > blockBTop && vRelY < 0)) {
            double e = qMin(blockA.restitutionCoefficient, blockB.restitutionCoefficient);
            double j = -(1 + e) * vRelY * blockA.mass;
            blockA.velocity.setY(blockA.velocity.y() + j / blockA.mass);
            blockA.isOnSomething = true;
        }
    } else {
        double vRelY = blockA.velocity.y() - blockB.velocity.y();

        if ((blockATop < blockBTop && vRelY > 0) || (blockATop > blockBTop && vRelY < 0)) {
            double e = qMin(blockA.restitutionCoefficient, blockB.restitutionCoefficient);
            double j = -(1 + e) * vRelY;

            double invMassA = blockA.isMovable ? (1.0 / blockA.mass) : 0.0;
            double invMassB = blockB.isMovable ? (1.0 / blockB.mass) : 0.0;
            double totalInvMass = invMassA + invMassB;

            if (totalInvMass > EPSILON) {
                j /= totalInvMass;

                if (blockA.isMovable) {
                    blockA.velocity.setY(blockA.velocity.y() + j * invMassA);
                    if (qAbs(blockA.velocity.y()) < EPSILON) blockA.velocity.setY(0.0);
                }
                if (blockB.isMovable) {
                    blockB.velocity.setY(blockB.velocity.y() - j * invMassB);
                    if (qAbs(blockB.velocity.y()) < EPSILON) blockB.velocity.setY(0.0);
                }
            }
        }
    }

    if (!isBlockADragged && !blockA.isMovable && (qAbs(blockA.velocity.x()) > EPSILON || qAbs(blockA.velocity.y()) > EPSILON)) {
        blockA.isMovable = true;
    }
    if (!isBlockBDragged && !blockB.isMovable && (qAbs(blockB.velocity.x()) > EPSILON || qAbs(blockB.velocity.y()) > EPSILON)) {
        blockB.isMovable = true;
    }
}

void DraggableElasticObject::handleBoundaryCollision(PhysicsBlockProperties& block)
{
    if (qAbs(block.velocity.x()) < EPSILON && qAbs(block.velocity.y()) < EPSILON && !block.isMovable && !(m_isDragging && m_draggedBlockIndex != -1 && &block == &m_blocks[m_draggedBlockIndex])) {
        return;
    }

    double parentWidth = width();
    double parentHeight = height();

    bool collided = false;

    double groundY = parentHeight - block.size.height();
    if (block.position.y() >= groundY - EPSILON) {
        block.position.setY(groundY);
        block.velocity.setY(-block.velocity.y() * block.restitutionCoefficient);
        if (qAbs(block.velocity.y()) < EPSILON) {
            block.velocity.setY(0.0);
        }
        collided = true;
        block.isOnSomething = true;
    }

    double topY = 0.0;
    if (block.position.y() <= topY + EPSILON) {
        block.position.setY(topY);
        block.velocity.setY(-block.velocity.y() * block.restitutionCoefficient);
        if (qAbs(block.velocity.y()) < EPSILON) {
            block.velocity.setY(0.0);
        }
        collided = true;
    }

    double leftX = 0.0;
    if (block.position.x() <= leftX + EPSILON) {
        block.position.setX(leftX);
        block.velocity.setX(-block.velocity.x() * block.restitutionCoefficient);
        if (qAbs(block.velocity.x()) < EPSILON) {
            block.velocity.setX(0.0);
        }
        collided = true;
    }

    double rightX = parentWidth - block.size.width();
    if (block.position.x() >= rightX - EPSILON) {
        block.position.setX(rightX);
        block.velocity.setX(-block.velocity.x() * block.restitutionCoefficient);
        if (qAbs(block.velocity.x()) < EPSILON) {
            block.velocity.setX(0.0);
        }
        collided = true;
    }

    if (collided && !block.isMovable && (qAbs(block.velocity.x()) > EPSILON || qAbs(block.velocity.y()) > EPSILON) && !(m_isDragging && m_draggedBlockIndex != -1 && &block == &m_blocks[m_draggedBlockIndex])) {
        block.isMovable = true;
    }
}

void DraggableElasticObject::applyFrictionAndCheckStop(PhysicsBlockProperties& block, bool onGround)
{
    if (m_isDragging && m_draggedBlockIndex != -1 && &block == &m_blocks[m_draggedBlockIndex]) {
        return;
    }

    bool isSupported = onGround || block.isOnSomething;

    if (isSupported && qAbs(block.velocity.x()) > EPSILON) {
        double normalForce = block.mass * GRAVITY_ACCELERATION;
        double frictionForceMagnitude = block.frictionCoefficient * normalForce;
        double frictionAccelerationMagnitude = frictionForceMagnitude / block.mass;

        if (block.velocity.x() > 0) {
            block.velocity.setX(qMax(0.0, block.velocity.x() - frictionAccelerationMagnitude * TIME_STEP));
        } else {
            block.velocity.setX(qMin(0.0, block.velocity.x() + frictionAccelerationMagnitude * TIME_STEP));
        }

        if (qAbs(block.velocity.x()) < EPSILON) {
            block.velocity.setX(0.0);
        }
    }

    if (isSupported && qAbs(block.velocity.y()) < EPSILON && qAbs(block.acceleration.y()) < EPSILON) {
        block.velocity.setY(0.0);
        block.acceleration.setY(0.0);
        if (qAbs(block.velocity.x()) < EPSILON) {
            block.velocity.setX(0.0);
            block.acceleration.setX(0.0);
            block.isMovable = false;
        }
    } else {
        if (qAbs(block.velocity.x()) > EPSILON || qAbs(block.velocity.y()) > EPSILON || !isSupported) {
            block.isMovable = true;
        }
    }
}
