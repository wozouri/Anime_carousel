#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QSlider>
#include <QGroupBox>
#include <QCheckBox>
#include "RotatableLinesWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QMainWindow window;
    window.resize(800, 600);
    QPalette pal = window.palette();
    pal.setColor(QPalette::Window, QColor(0,0,0,255));
    window.setPalette(pal);

    QWidget *centralWidget = new QWidget(&window);
    window.setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    RotatableLinesWidget *linesWidget = new RotatableLinesWidget(centralWidget);
    mainLayout->addWidget(linesWidget);

    QGroupBox *controlGroup = new QGroupBox("控制面板", centralWidget);
    controlGroup->setMaximumHeight(200);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);

    controlLayout->addWidget(new QLabel("行数:", controlGroup));
    QSpinBox *rowsSpinBox = new QSpinBox(controlGroup);
    rowsSpinBox->setRange(1, 99);
    rowsSpinBox->setValue(10);
    QObject::connect(rowsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                     linesWidget, &RotatableLinesWidget::setLinesPerCol);
    controlLayout->addWidget(rowsSpinBox);

    controlLayout->addWidget(new QLabel("列数:", controlGroup));
    QSpinBox *colsSpinBox = new QSpinBox(controlGroup);
    colsSpinBox->setRange(1, 99);
    colsSpinBox->setValue(10);
    QObject::connect(colsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                     linesWidget, &RotatableLinesWidget::setLinesPerRow);
    controlLayout->addWidget(colsSpinBox);


    controlLayout->addStretch(1);

    mainLayout->addWidget(controlGroup);

    window.show();

    return a.exec();
}
