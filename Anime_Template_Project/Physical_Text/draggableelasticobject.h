#ifndef DRAGGABLEELASTICOBJECT_H
#define DRAGGABLEELASTICOBJECT_H

#include <QWidget>
#include <QTimer>
#include <QPointF>
#include <QSizeF>
#include <QColor>
#include <QPainter>
#include <QList>
#include <QMouseEvent>
#include <QDateTime>

struct PhysicsBlockProperties {
    QPointF position = {0.0, 0.0};
    QSizeF size = {50.0, 50.0};
    QString name;

    double mass = 1.0;
    QPointF velocity = {0.0, 0.0};
    QPointF acceleration = {0.0, 0.0};
    double frictionCoefficient = 0.5;
    double restitutionCoefficient = 0.7;

    bool isMovable = true;
    QColor color = Qt::blue;
    bool isOnSomething = false;
};


class DraggableElasticObject : public QWidget
{
    Q_OBJECT

public:
    explicit DraggableElasticObject(QWidget *parent = nullptr);
    ~DraggableElasticObject() override;

    void addBlock(const PhysicsBlockProperties& block);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void updatePhysics();

private:
    void applyForces(PhysicsBlockProperties& block);
    void updateVelocityAndPredictedPosition(PhysicsBlockProperties& block, QPointF& predictedPosition) const;
    void handleBlockToBlockCollision(PhysicsBlockProperties& blockA, PhysicsBlockProperties& blockB);
    void handleBoundaryCollision(PhysicsBlockProperties& block);
    void applyFrictionAndCheckStop(PhysicsBlockProperties& block, bool onGround);

private:
    QList<PhysicsBlockProperties> m_blocks;
    QTimer *m_physicsTimer;
    const double GRAVITY_ACCELERATION = 981.0;
    const double TIME_STEP = 0.001;
    const double EPSILON = 0.1;

    bool m_isDragging = false;
    int m_draggedBlockIndex = -1;
    QPointF m_dragOffset;

    QPointF m_lastMousePos;
    qint64 m_lastMousePressTime;
    QPointF m_lastMouseVelocity;
};

#endif // DRAGGABLEELASTICOBJECT_H
