#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QMessageLogContext>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 创建FuzzyTextWidget实例
    MainWindow mainWindow;


    // 显示主窗口
    mainWindow.show();

    qDebug() << "应用程序启动成功";

    return app.exec();
}