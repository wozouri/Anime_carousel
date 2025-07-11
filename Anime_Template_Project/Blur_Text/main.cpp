#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QMessageLogContext>
#include "fuzzytextwidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("Daen No Kado Demo");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Your Organization");

    // 创建主窗口
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("");//椭圆卡片展示器
    mainWindow.resize(1200, 606);

    // 创建中央widget
    QWidget* centralWidget = new QWidget(&mainWindow);
    mainWindow.setCentralWidget(centralWidget);

    // 创建布局
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    // 创建FuzzyTextWidget实例
    FuzzyTextWidget* test = new FuzzyTextWidget(centralWidget);
    test->setColor(Qt::green);
    layout->addWidget(test);

    // 显示主窗口
    mainWindow.show();

    qDebug() << "应用程序启动成功";

    return app.exec();
}