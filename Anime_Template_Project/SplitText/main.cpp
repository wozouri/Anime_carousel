#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QMessageLogContext>
#include "SplitText.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 创建主窗口
    QMainWindow mainWindow;
    QWidget central;
    QHBoxLayout h_layout;
    h_layout.addWidget(new SplitText());
    central.setLayout(&h_layout);
    mainWindow.setCentralWidget(&central);

    mainWindow.resize(500, 800);

    mainWindow.show();

    return app.exec();
}