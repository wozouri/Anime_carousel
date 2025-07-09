#include <QApplication>
#include <QMainWindow>
#include "GlitchEffectWidget.h"
#include <QVBoxLayout>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QMainWindow window;
    window.resize(800, 600);

    QWidget *centralWidget = new QWidget(&window);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    GlitchEffectWidget *glitchWidget = new GlitchEffectWidget(centralWidget);
    layout->addWidget(glitchWidget);

    QPushButton *smoothToggleBtn = new QPushButton("平滑过渡开关按钮", centralWidget);
    QObject::connect(smoothToggleBtn, &QPushButton::clicked, [glitchWidget]() {
        glitchWidget->setSmoothTransitions(!glitchWidget->property("smoothTransitions").toBool());
        glitchWidget->setProperty("smoothTransitions", !glitchWidget->property("smoothTransitions").toBool());
    });
    glitchWidget->setProperty("smoothTransitions", true);

    QPushButton *centerVignetteToggleBtn = new QPushButton("中心渐晕开关按钮", centralWidget);
    QObject::connect(centerVignetteToggleBtn, &QPushButton::clicked, [glitchWidget]() {
        glitchWidget->setCenterVignette(!glitchWidget->property("centerVignette").toBool());
        glitchWidget->setProperty("centerVignette", !glitchWidget->property("centerVignette").toBool());
    });
    glitchWidget->setProperty("centerVignette", false);

    QPushButton *outerVignetteToggleBtn = new QPushButton("外部渐晕开关按钮", centralWidget);
    QObject::connect(outerVignetteToggleBtn, &QPushButton::clicked, [glitchWidget]() {
        glitchWidget->setOuterVignette(!glitchWidget->property("outerVignette").toBool());
        glitchWidget->setProperty("outerVignette", !glitchWidget->property("outerVignette").toBool());
    });
    glitchWidget->setProperty("outerVignette", true);

    layout->addWidget(smoothToggleBtn);
    layout->addWidget(centerVignetteToggleBtn);
    layout->addWidget(outerVignetteToggleBtn);

    window.setCentralWidget(centralWidget);
    window.show();

    return a.exec();
}
