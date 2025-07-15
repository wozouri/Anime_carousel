#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QMessageLogContext>
#include "Diffusion_button.h"
#include "wave_button.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 创建主窗口
    QMainWindow mainWindow;
    QWidget central;
    QVBoxLayout v_layout;
    v_layout.addWidget(new Wave_button());
    v_layout.addWidget(new Diffusion_button());
    central.setLayout(&v_layout);
    mainWindow.setCentralWidget(&central);

    mainWindow.resize(mainWindow.sizeHint());

    mainWindow.show();

    return app.exec();
}