#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QMenuBar>
#include <QAction>
#include <QPainter>
#include "OverlayWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        m_overlay = nullptr;
    }

private slots:
    void openImage()
    {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            "打开图片",
            "",
            "Image Files (*.png *.jpg *.jpeg *.bmp *.gif);;All Files (*)"
            );

        if (!fileName.isEmpty()) {
            QPixmap pixmap(fileName);
            if (!pixmap.isNull()) {
                // 缩放图片以适应显示
                QPixmap scaled = pixmap.scaled(m_imageLabel->size(),
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
                m_imageLabel->setPixmap(scaled);
                m_imageLabel->setAlignment(Qt::AlignCenter);

                m_startAnnotationBtn->setEnabled(true);
            } else {
                QMessageBox::warning(this, "错误", "无法加载图片文件");
            }
        }
    }

    void startAnnotation()
    {
        if (!m_overlay) {
            // 创建遮罩，覆盖在图片label上
            //m_imageLabel->setStyleSheet("background-color:black;");
            m_overlay = new OverlayWidget(m_imageLabel, this);
        }
        m_overlay->setStyleTheme(OverlayStyleManager::THEME_LIGHT);
        m_overlay->showOverlay();
    }

    void showDemoImage()
    {
        // 创建一个演示图片
        QPixmap demo(400, 300);
        demo.fill(QColor(200, 220, 240));

        QPainter painter(&demo);
        painter.setPen(QPen(Qt::darkBlue, 2));
        painter.setFont(QFont("Arial", 16));
        painter.drawText(demo.rect(), Qt::AlignCenter,
                         "这是一个演示图片\n点击'开始标注'在上面绘制");

        // 绘制一些装饰
        painter.setPen(QPen(Qt::blue, 3));
        painter.drawRoundedRect(20, 20, demo.width()-40, demo.height()-40, 10, 10);

        m_imageLabel->setPixmap(demo);
        m_imageLabel->setAlignment(Qt::AlignCenter);
        m_startAnnotationBtn->setEnabled(true);
    }

private:
    void setupUI()
    {
        setWindowTitle("Widget遮罩标注演示");
        setMinimumSize(600, 500);

        // 创建菜单栏
        QMenuBar *menuBar = this->menuBar();
        QMenu *fileMenu = menuBar->addMenu("文件");

        QAction *openAction = new QAction("打开图片", this);
        openAction->setShortcut(QKeySequence::Open);
        connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
        fileMenu->addAction(openAction);

        QAction *demoAction = new QAction("显示演示图片", this);
        connect(demoAction, &QAction::triggered, this, &MainWindow::showDemoImage);
        fileMenu->addAction(demoAction);

        fileMenu->addSeparator();

        QAction *exitAction = new QAction("退出", this);
        exitAction->setShortcut(QKeySequence::Quit);
        connect(exitAction, &QAction::triggered, this, &QWidget::close);
        fileMenu->addAction(exitAction);

        // 创建中央widget
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        // 图片显示区域
        m_imageLabel = new QLabel(this);
        m_imageLabel->setMinimumSize(400, 300);
        m_imageLabel->setStyleSheet(
            "QLabel { "
            "  border: 2px dashed #aaa; "
            "  background-color: #f5f5f5; "
            "  border-radius: 5px; "
            "}"
            );
        m_imageLabel->setText("请打开图片或点击'显示演示图片'");
        m_imageLabel->setAlignment(Qt::AlignCenter);
        m_imageLabel->setWordWrap(true);

        // 按钮区域
        QHBoxLayout *buttonLayout = new QHBoxLayout();

        QPushButton *openBtn = new QPushButton("打开图片", this);
        connect(openBtn, &QPushButton::clicked, this, &MainWindow::openImage);

        QPushButton *demoBtn = new QPushButton("显示演示图片", this);
        connect(demoBtn, &QPushButton::clicked, this, &MainWindow::showDemoImage);

        m_startAnnotationBtn = new QPushButton("开始标注", this);
        m_startAnnotationBtn->setEnabled(false);
        m_startAnnotationBtn->setStyleSheet(
            "QPushButton { "
            "  background-color: #4CAF50; "
            "  color: white; "
            "  border: none; "
            "  padding: 8px 16px; "
            "  border-radius: 4px; "
            "  font-weight: bold; "
            "}"
            "QPushButton:hover { background-color: #45a049; }"
            "QPushButton:disabled { background-color: #cccccc; }"
            );
        connect(m_startAnnotationBtn, &QPushButton::clicked, this, &MainWindow::startAnnotation);

        buttonLayout->addWidget(openBtn);
        buttonLayout->addWidget(demoBtn);
        buttonLayout->addStretch();
        buttonLayout->addWidget(m_startAnnotationBtn);

        // 添加说明文字
        QLabel *helpLabel = new QLabel(
            "使用说明：\n"
            "1. 打开图片或使用演示图片\n"
            "2. 点击'开始标注'启动遮罩\n"
            "3. 在遮罩上绘制或添加文字\n"
            "4. 按ESC或点击'完成'退出标注\n"
            "5. 可以保存带标注的图片",
            this
            );
        helpLabel->setStyleSheet(
            "QLabel { "
            "  background-color: #e3f2fd; "
            "  border: 1px solid #bbdefb; "
            "  border-radius: 4px; "
            "  padding: 8px; "
            "  color: #1976d2; "
            "}"
            );

        // 布局
        mainLayout->addWidget(m_imageLabel, 1);
        mainLayout->addLayout(buttonLayout);
        mainLayout->addWidget(helpLabel);
    }

private:
    QLabel *m_imageLabel;
    QPushButton *m_startAnnotationBtn;
    OverlayWidget *m_overlay;
};

#include <windows.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AllocConsole();

    // 重定向标准输入/输出/错误流到新控制台
    FILE* fp;
    freopen_s(&fp, "CONIN$", "r", stdin);
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);

    // 设置无缓冲模式
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    app.setApplicationName("Widget遮罩标注工具");
    app.setApplicationVersion("1.0");

    MainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
