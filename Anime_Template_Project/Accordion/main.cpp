#include <QApplication>
#include <QMainWindow>

#include "AccordionCard.h"
#include "AccordionWidget.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QMainWindow window;
    window.setWindowTitle("手风琴");
    window.setMinimumSize(800, 300);

    AccordionWidget* accordion = new AccordionWidget(&window);

    accordion->addCard(new AccordionCard("卡片 1", ":/img/FvCpS5naYAA06ej.jpg"));
    accordion->addCard(new AccordionCard("卡片 2", ":/img/GS6v-JebIAIMmNY.jpg"));
    accordion->addCard(new AccordionCard("卡片 3", ":/img/GniQBbrbUAAq-HX.jpg"));
    accordion->addCard(new AccordionCard("卡片 4", ":/img/GGE5u19acAAa7OA.jpg"));
    accordion->addCard(new AccordionCard("卡片 5", ":/img/GGIv8piaUAA_myh.jpg"));

    window.setCentralWidget(accordion);
    window.show();

    return a.exec();
}
