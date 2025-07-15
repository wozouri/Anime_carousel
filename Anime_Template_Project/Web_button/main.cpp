#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include "WebButton.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 创建主窗口
    QWidget window;
    window.setWindowTitle("Web Button Demo");
    window.resize(400, 300);

    // 创建 Graphics Scene 和 View
    QGraphicsScene* scene = new QGraphicsScene(&window);
    QGraphicsView* view = new QGraphicsView(scene, &window);

    // 设置背景
    scene->setBackgroundBrush(QBrush(QColor(248, 250, 252)));
    view->setFrameStyle(QFrame::NoFrame);



    // 创建几个不同样式的按钮
    WebButton* button1 = new WebButton("click me");
    button1->setPos(50, 50);
    scene->addItem(button1);

    WebButton* button2 = new WebButton("登录");
    button2->setPrimaryColor(QColor(34, 197, 94)); // 绿色
    button2->setPos(200, 50);
    scene->addItem(button2);

    WebButton* button3 = new WebButton("删除");
    button3->setPrimaryColor(QColor(239, 68, 68)); // 红色
    button3->setSize(QSizeF(100, 35));
    button3->setPos(50, 120);
    scene->addItem(button3);

    WebButton* button4 = new WebButton("更多选项");
    button4->setPrimaryColor(QColor(168, 85, 247)); // 紫色
    button4->setSize(QSizeF(140, 45));
    button4->setPos(180, 120);
    scene->addItem(button4);

    // 连接信号
    QObject::connect(button1, &WebButton::clicked, []() {
        qDebug() << "button 1 is clicked";
    });

    QObject::connect(button2, &WebButton::clicked, []() {
        qDebug() << "login button is clicked";
    });

    QObject::connect(button3, &WebButton::clicked, []() {
        qDebug() << "delete button is clicked";
    });

    QObject::connect(button4, &WebButton::clicked, []() {
        qDebug() << "setting button is clicked";
    });

    // 布局
    QVBoxLayout* layout = new QVBoxLayout(&window);
    layout->addWidget(view);
    window.setLayout(layout);

    // 设置场景大小
    scene->setSceneRect(0, 0, 380, 200);

    window.show();
    return app.exec();
}
