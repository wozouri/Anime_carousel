#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    setWindowTitle("Qt 分形生成器 - She Chen Zhi Ma");
    setMinimumSize(800, 700);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainHorizontalLayout = new QHBoxLayout(centralWidget);

    QGroupBox *controlGroup = new QGroupBox("控制", this);
    controlGroup->setMaximumWidth(250);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlGroup);

    m_fractalTypeComboBox = new QComboBox(this);
    m_fractalTypeComboBox->addItem("曼德布罗集", FractalWidget::Mandelbrot);
    m_fractalTypeComboBox->addItem("朱利亚集", FractalWidget::Julia);
    m_fractalTypeComboBox->addItem("燃烧船分形", FractalWidget::BurningShip);
    m_fractalTypeComboBox->addItem("牛顿分形", FractalWidget::Newton);
    m_fractalTypeComboBox->addItem("Tricorn", FractalWidget::Tricorn);
    m_fractalTypeComboBox->addItem("燃烧船朱利亚集", FractalWidget::BurningShipJulia);
    m_fractalTypeComboBox->addItem("蜘蛛分形", FractalWidget::Spider);
    m_fractalTypeComboBox->addItem("谢尔宾斯基曲线", FractalWidget::SierpinskiCurve);
    m_fractalTypeComboBox->addItem("科赫雪花", FractalWidget::KochSnowflake);
    connect(m_fractalTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFractalTypeChanged);
    controlLayout->addWidget(new QLabel("选择分形类型:"));
    controlLayout->addWidget(m_fractalTypeComboBox);

    m_juliaCrLineEdit = new QLineEdit(this);
    m_juliaCrLineEdit->setValidator(new QDoubleValidator(-1000.0, 1000.0, 5, this));
    m_juliaCrLineEdit->setText("-0.7");

    m_juliaCiLineEdit = new QLineEdit(this);
    m_juliaCiLineEdit->setValidator(new QDoubleValidator(-1000.0, 1000.0, 5, this));
    m_juliaCiLineEdit->setText("0.27015");

    m_applyJuliaButton = new QPushButton("应用朱利亚参数", this);
    connect(m_applyJuliaButton, &QPushButton::clicked, this, &MainWindow::onApplyJuliaParams);

    QHBoxLayout *juliaCrLayout = new QHBoxLayout();
    juliaCrLayout->addWidget(new QLabel("Julia Cr:"));
    juliaCrLayout->addWidget(m_juliaCrLineEdit);

    QHBoxLayout *juliaCiLayout = new QHBoxLayout();
    juliaCiLayout->addWidget(new QLabel("Julia Ci:"));
    juliaCiLayout->addWidget(m_juliaCiLineEdit);

    controlLayout->addLayout(juliaCrLayout);
    controlLayout->addLayout(juliaCiLayout);
    controlLayout->addWidget(m_applyJuliaButton);
    controlLayout->addStretch();

    mainHorizontalLayout->addWidget(controlGroup);

    m_fractalWidget = new FractalWidget(this);
    mainHorizontalLayout->addWidget(m_fractalWidget);

    m_fractalTypeComboBox->setCurrentIndex(0);
}

void MainWindow::onFractalTypeChanged(int index)
{
    Q_UNUSED(index);

    FractalWidget::FractalType type = static_cast<FractalWidget::FractalType>(
        m_fractalTypeComboBox->currentData().toInt());
    m_fractalWidget->setFractalType(type);

    bool isJuliaRelated = (type == FractalWidget::Julia || type == FractalWidget::BurningShipJulia);
    m_juliaCrLineEdit->setVisible(isJuliaRelated);
    m_juliaCiLineEdit->setVisible(isJuliaRelated);
    m_applyJuliaButton->setVisible(isJuliaRelated);

    if (type == FractalWidget::BurningShipJulia) {
        m_juliaCrLineEdit->setText("-0.1");
        m_juliaCiLineEdit->setText("0.29015");
        m_fractalWidget->setJuliaC(-0.1, 0.29015);
    } else if (type == FractalWidget::Julia) {
        m_juliaCrLineEdit->setText("-0.7");
        m_juliaCiLineEdit->setText("0.27015");
        m_fractalWidget->setJuliaC(-0.7, 0.27015);
    }
}

void MainWindow::onApplyJuliaParams()
{
    double cr = m_juliaCrLineEdit->text().toDouble();
    double ci = m_juliaCiLineEdit->text().toDouble();
    m_fractalWidget->setJuliaC(cr, ci);
}
