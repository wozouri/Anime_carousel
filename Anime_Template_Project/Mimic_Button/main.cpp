#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QMessageLogContext>
#include "widget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 创建主窗口
    QMainWindow mainWindow;
    Widget central;
    

    mainWindow.setCentralWidget(&central);

    mainWindow.resize(500, 800);

    mainWindow.show();

    return app.exec();
}