#include <QApplication>
#include "draggableelasticobject.h"
#include <QMainWindow>
#include <QRandomGenerator>
#include <QFontMetrics>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QMainWindow window;
    window.resize(1000, 700);

    DraggableElasticObject *physicsWidget = new DraggableElasticObject(&window);
    window.setCentralWidget(physicsWidget);

    QString sentence = "命运石之门 胆大党 败犬女主太多了 哭泣少女乐队 我心里危险的东西 葬送的芙莉莲 咒术回战 无职转生 为美好的世界献上祝福 夏日重现 鬼灭之刃 JOJO的奇妙冒险 86-不存在的战区";
    QStringList words = sentence.split(" ", Qt::SkipEmptyParts);

    QFont font;
    font.setPointSize(15);
    font.setBold(true);

    qreal currentX = 50.0;
    qreal currentY = 50.0;
    qreal maxHeightInRow = 0.0;

    for (const QString& word : words)
    {
        PhysicsBlockProperties block;
        block.name = word;

        QFontMetrics fm(font);
        int textWidth = fm.horizontalAdvance(word) + 20;
        int lineHeight = fm.height() + 10;

        block.size = {qreal(textWidth), qreal(lineHeight)};

        if (currentX + block.size.width() > window.width() - 50)
        {
            currentX = 50.0;
            currentY += maxHeightInRow + 20;
            maxHeightInRow = 0.0;
        }

        block.position = {currentX, currentY};
        block.color = QColor(QRandomGenerator::global()->bounded(255),
                             QRandomGenerator::global()->bounded(255),
                             QRandomGenerator::global()->bounded(255));
        block.mass = 1.0 + QRandomGenerator::global()->bounded(100) / 50.0;
        block.restitutionCoefficient = 0.6 + QRandomGenerator::global()->bounded(40) / 100.0;

        physicsWidget->addBlock(block);

        currentX += block.size.width() + 10;
        maxHeightInRow = qMax(maxHeightInRow, block.size.height());
    }

    PhysicsBlockProperties groundBlock;
    groundBlock.position = {100.0, 500.0};
    groundBlock.size = {800.0, 30.0};
    groundBlock.color = Qt::gray;
    groundBlock.isMovable = false;
    groundBlock.restitutionCoefficient = 0.5;
    physicsWidget->addBlock(groundBlock);

    window.show();

    return a.exec();
}
