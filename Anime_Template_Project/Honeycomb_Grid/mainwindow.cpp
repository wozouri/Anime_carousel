#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建中心Widget和布局
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 添加 HexagonWidget
    m_hexagonWidget = new HexagonWidget(this);
    mainLayout->addWidget(m_hexagonWidget);

    // 添加控制滑块
    QHBoxLayout *controlsLayout = new QHBoxLayout();

    // 边长控制
    m_sizeLabel = new QLabel("边长: 50", this);
    m_sizeSlider = new QSlider(Qt::Horizontal, this);
    m_sizeSlider->setRange(10, 100);
    m_sizeSlider->setValue(50);
    connect(m_sizeSlider, &QSlider::valueChanged, this, &MainWindow::onSizeSliderChanged);
    controlsLayout->addWidget(m_sizeLabel);
    controlsLayout->addWidget(m_sizeSlider);

    // 间距控制
    m_spacingLabel = new QLabel("间距: 5", this);
    m_spacingSlider = new QSlider(Qt::Horizontal, this);
    m_spacingSlider->setRange(0, 30);
    m_spacingSlider->setValue(5);
    connect(m_spacingSlider, &QSlider::valueChanged, this, &MainWindow::onSpacingSliderChanged);
    controlsLayout->addWidget(m_spacingLabel);
    controlsLayout->addWidget(m_spacingSlider);

    mainLayout->addLayout(controlsLayout);

    setWindowTitle("蜂巢网格");
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onSizeSliderChanged(int value)
{
    m_hexagonWidget->setHexagonSize(static_cast<double>(value));
    m_sizeLabel->setText(QString("边长: %1").arg(value));
}

void MainWindow::onSpacingSliderChanged(int value)
{
    m_hexagonWidget->setHexagonSpacing(static_cast<double>(value));
    m_spacingLabel->setText(QString("间距: %1").arg(value));
}
