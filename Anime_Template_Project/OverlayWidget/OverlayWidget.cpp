#include "OverlayWidget.h"
#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include <QMessageBox>
#include <QEvent>
#include <QTimer>
#include <qDebug>

OverlayWidget::OverlayWidget(QWidget* targetWidget, QWidget* parent)
    : QWidget(parent)
    , m_targetWidget(targetWidget)
    , m_penColor(Qt::red)
    , m_penWidth(3)
    , m_drawing(false)
    , m_textEdit(nullptr)
    , m_textMode(false)
    , m_editingTextIndex(-1)
    , m_eraserMode(false)      // 初始化橡皮擦模式
    , m_eraserSize(20)         // 初始化橡皮擦大小
    , m_erasing(false)         // 初始化擦除状态
    , m_currentMousePos(0, 0)  // 初始化鼠标位置
    , m_toolbarCollapsed(false)
    , m_draggingToolbar(false)
{
    if (!m_targetWidget) {
        qWarning("OverlayWidget: targetWidget cannot be null");
        return;
    }

    // 记录初始大小
    m_lastTargetSize = m_targetWidget->size();

    // 设置窗口属性 - 透明遮罩
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);

    // 设置焦点策略
    setFocusPolicy(Qt::StrongFocus);

    setupUI();
    updatePosition();

    // 安装事件过滤器监听目标widget变化
    installEventFilters();

    // 隐藏遮罩，需要时调用showOverlay()
    hide();
}

OverlayWidget::~OverlayWidget()
{
    // 清理事件过滤器
    removeEventFilters();

    // 清理文字输入框
    if (m_textEdit) {
        m_textEdit->deleteLater();
        m_textEdit = nullptr;
    }
}

void OverlayWidget::setupUI()
{
    // 创建主工具栏容器
    m_toolbar = new QWidget(this);
    m_toolbar->setStyleSheet(
        "QWidget#toolbar { background-color: rgba(50, 50, 50, 230); border-radius: 8px; border: 1px solid rgba(100, 100, 100, 100); }"
    );
    m_toolbar->setObjectName("toolbar");

    QVBoxLayout* toolbarLayout = new QVBoxLayout(m_toolbar);
    toolbarLayout->setSpacing(0);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);

    // 创建工具栏标题栏（拖动区域）
    m_toolbarHeader = new QWidget(m_toolbar);
    m_toolbarHeader->setFixedHeight(30);
    m_toolbarHeader->setStyleSheet(
        "QWidget { background-color: rgba(70, 70, 70, 200); border-radius: 6px 6px 0 0; }"
        "QPushButton { color: white; border: none; padding: 4px; font-size: 12px; background: transparent; }"
        "QPushButton:hover { background-color: rgba(100, 100, 100, 100); }"
    );
    m_toolbarHeader->setCursor(Qt::SizeAllCursor);

    QHBoxLayout* headerLayout = new QHBoxLayout(m_toolbarHeader);
    headerLayout->setContentsMargins(8, 4, 4, 4);

    // 标题标签
    QLabel* titleLabel = new QLabel("标注工具", m_toolbarHeader);
    titleLabel->setStyleSheet("color: white; font-size: 12px; font-weight: bold;");
    titleLabel->setCursor(Qt::SizeAllCursor);

    // 收起/展开按钮
    m_collapseButton = new QPushButton("−", m_toolbarHeader);
    m_collapseButton->setFixedSize(20, 20);
    m_collapseButton->setToolTip("收起工具栏");
    connect(m_collapseButton, &QPushButton::clicked, this, &OverlayWidget::toggleToolbarCollapse);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_collapseButton);

    // 创建工具栏内容区域
    m_toolbarContent = new QWidget(m_toolbar);
    m_toolbarContent->setStyleSheet(
        "QWidget { background-color: rgba(50, 50, 50, 230); border-radius: 0 0 6px 6px; }"
        "QPushButton { color: white; border: 1px solid gray; padding: 4px 8px; margin: 2px; border-radius: 3px; font-size: 12px; }"
        "QPushButton:hover { background-color: rgba(100, 100, 100, 150); }"
        "QSpinBox { color: white; background-color: rgba(70, 70, 70, 200); border: 1px solid gray; padding: 2px; }"
        "QCheckBox { color: white; font-size: 12px; }"
        "QLabel { color: white; font-size: 12px; }"
    );

    // 创建两行布局
    QVBoxLayout* contentLayout = new QVBoxLayout(m_toolbarContent);
    contentLayout->setSpacing(3);
    contentLayout->setContentsMargins(8, 6, 8, 6);

    // 第一行：绘制工具
    QHBoxLayout* drawingToolsLayout = new QHBoxLayout();
    drawingToolsLayout->setSpacing(5);

    // 颜色选择按钮
    m_colorButton = new QPushButton("颜色", m_toolbarContent);
    m_colorButton->setFixedSize(50, 28);
    m_colorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(m_penColor.name()));
    connect(m_colorButton, &QPushButton::clicked, this, &OverlayWidget::changePenColor);

    // 画笔宽度
    QLabel* widthLabel = new QLabel("宽度:", m_toolbarContent);
    m_widthSpinBox = new QSpinBox(m_toolbarContent);
    m_widthSpinBox->setRange(1, 20);
    m_widthSpinBox->setValue(m_penWidth);
    m_widthSpinBox->setFixedSize(60, 28);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &OverlayWidget::changePenWidth);

    // 文字模式复选框
    m_textModeCheckBox = new QCheckBox("文字", m_toolbarContent);
    connect(m_textModeCheckBox, &QCheckBox::toggled, this, &OverlayWidget::toggleTextMode);

    // 橡皮擦模式复选框
    m_eraserModeCheckBox = new QCheckBox("橡皮擦", m_toolbarContent);
    connect(m_eraserModeCheckBox, &QCheckBox::toggled, this, &OverlayWidget::toggleEraserMode);

    // 橡皮擦大小
    QLabel* eraserSizeLabel = new QLabel("擦除:", m_toolbarContent);
    m_eraserSizeSpinBox = new QSpinBox(m_toolbarContent);
    m_eraserSizeSpinBox->setRange(5, 50);
    m_eraserSizeSpinBox->setValue(m_eraserSize);
    m_eraserSizeSpinBox->setFixedSize(60, 28);
    m_eraserSizeSpinBox->setEnabled(false);  // 初始禁用
    connect(m_eraserSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &OverlayWidget::changeEraserSize);

    drawingToolsLayout->addWidget(m_colorButton);
    drawingToolsLayout->addWidget(widthLabel);
    drawingToolsLayout->addWidget(m_widthSpinBox);
    drawingToolsLayout->addWidget(m_textModeCheckBox);
    drawingToolsLayout->addWidget(m_eraserModeCheckBox);
    drawingToolsLayout->addWidget(eraserSizeLabel);
    drawingToolsLayout->addWidget(m_eraserSizeSpinBox);
    drawingToolsLayout->addStretch();

    // 第二行：操作按钮
    QHBoxLayout* actionButtonsLayout = new QHBoxLayout();
    actionButtonsLayout->setSpacing(5);

    // 撤销按钮
    m_undoButton = new QPushButton("撤销", m_toolbarContent);
    m_undoButton->setFixedSize(50, 28);
    m_undoButton->setEnabled(false);
    connect(m_undoButton, &QPushButton::clicked, this, &OverlayWidget::undoLastAction);

    // 重做按钮
    m_redoButton = new QPushButton("重做", m_toolbarContent);
    m_redoButton->setFixedSize(50, 28);
    m_redoButton->setEnabled(false);
    connect(m_redoButton, &QPushButton::clicked, this, &OverlayWidget::redoLastAction);

    // 清除按钮
    m_clearButton = new QPushButton("清除", m_toolbarContent);
    m_clearButton->setFixedSize(50, 28);
    connect(m_clearButton, &QPushButton::clicked, this, &OverlayWidget::clearCanvas);

    // 保存按钮
    m_saveButton = new QPushButton("保存", m_toolbarContent);
    m_saveButton->setFixedSize(50, 28);
    connect(m_saveButton, &QPushButton::clicked, this, &OverlayWidget::saveImage);

    // 完成按钮
    m_finishButton = new QPushButton("完成", m_toolbarContent);
    m_finishButton->setFixedSize(50, 28);
    connect(m_finishButton, &QPushButton::clicked, this, &OverlayWidget::finishEditing);

    actionButtonsLayout->addWidget(m_undoButton);
    actionButtonsLayout->addWidget(m_redoButton);
    actionButtonsLayout->addStretch();
    actionButtonsLayout->addWidget(m_clearButton);
    actionButtonsLayout->addWidget(m_saveButton);
    actionButtonsLayout->addWidget(m_finishButton);

    // 添加到内容布局
    contentLayout->addLayout(drawingToolsLayout);
    contentLayout->addLayout(actionButtonsLayout);

    // 添加到主工具栏布局
    toolbarLayout->addWidget(m_toolbarHeader);
    toolbarLayout->addWidget(m_toolbarContent);

    // 定位工具栏到初始位置
    m_toolbar->move(10, 10);
    updateToolbarLayout();
}

void OverlayWidget::toggleEraserMode(bool enabled)
{
    // 切换到橡皮擦模式时，先完成当前的文字输入
    if (m_textEdit && enabled) {
        finishTextInput();
    }

    m_eraserMode = enabled;
    m_eraserSizeSpinBox->setEnabled(enabled);

    // 如果启用橡皮擦模式，禁用文字模式
    if (enabled && m_textMode) {
        m_textMode = false;
        m_textModeCheckBox->setChecked(false);
    }

    // 设置鼠标光标和跟踪
    if (enabled) {
        setCursor(createEraserCursor());  // 使用自定义橡皮擦光标
        setMouseTracking(true);  // 启用鼠标跟踪
        m_currentMousePos = mapFromGlobal(QCursor::pos());
    }
    else {
        setCursor(Qt::ArrowCursor);  // 恢复默认光标
        setMouseTracking(false);  // 禁用鼠标跟踪
    }

    update();  // 重绘以显示/隐藏橡皮擦预览
}

void OverlayWidget::changeEraserSize(int size)
{
    m_eraserSize = size;
    if (m_eraserMode) {
        update();  // 重绘以显示新的橡皮擦预览大小
    }
}

void OverlayWidget::performErase(const QPoint& position)
{
    bool hasErased = false;
    ErasedData erasedData;

    // 从后往前遍历路径，这样删除时不会影响索引
    for (int i = m_paths.size() - 1; i >= 0; --i) {
        const auto& path = m_paths[i];
        bool pathErased = false;

        // 检查路径中的每个点是否在橡皮擦范围内
        for (const auto& point : path) {
            if (isPointInEraserRange(point.point, position)) {
                pathErased = true;
                break;
            }
        }

        if (pathErased) {
            erasedData.erasedPathIndices.append(i);
            erasedData.erasedPaths.append(m_paths[i]);
            m_paths.removeAt(i);
            hasErased = true;
        }
    }

    // 从后往前遍历文字，这样删除时不会影响索引
    for (int i = m_textItems.size() - 1; i >= 0; --i) {
        const TextItem& textItem = m_textItems[i];

        if (isTextInEraserRange(textItem, position)) {
            erasedData.erasedTextIndices.append(i);
            erasedData.erasedTexts.append(textItem);
            m_textItems.removeAt(i);
            hasErased = true;
        }
    }

    // 如果有内容被擦除，累积到当前擦除数据中
    if (hasErased) {
        // 合并到当前擦除操作的数据中
        m_currentErasedData.erasedPathIndices.append(erasedData.erasedPathIndices);
        m_currentErasedData.erasedPaths.append(erasedData.erasedPaths);
        m_currentErasedData.erasedTextIndices.append(erasedData.erasedTextIndices);
        m_currentErasedData.erasedTexts.append(erasedData.erasedTexts);

        update();
    }
}

bool OverlayWidget::isPointInEraserRange(const QPoint& point, const QPoint& eraserCenter)
{
    int dx = point.x() - eraserCenter.x();
    int dy = point.y() - eraserCenter.y();
    int distance = dx * dx + dy * dy;
    int radius = m_eraserSize / 2;
    return distance <= radius * radius;
}

bool OverlayWidget::isTextInEraserRange(const TextItem& textItem, const QPoint& eraserCenter)
{
    QFontMetrics fm(textItem.font);
    QRect textRect(textItem.position, fm.size(Qt::TextSingleLine, textItem.text));

    // 检查文字矩形是否与橡皮擦圆形区域相交
    int radius = m_eraserSize / 2;
    QRect eraserRect(eraserCenter.x() - radius, eraserCenter.y() - radius,
        m_eraserSize, m_eraserSize);

    return textRect.intersects(eraserRect);
}

QCursor OverlayWidget::createEraserCursor()
{
    // 创建橡皮擦光标图标
    int cursorSize = 8;  // 光标大小
    QPixmap cursorPixmap(cursorSize, cursorSize);
    cursorPixmap.fill(Qt::transparent);

    QPainter painter(&cursorPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制橡皮擦图标
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(QBrush(Qt::white));

    // 绘制橡皮擦主体（矩形）
    QRect eraserRect(0, 0, 8, 10);
    painter.drawRoundedRect(eraserRect, 0, 0);

    // 绘制橡皮擦上的金属部分
    painter.setBrush(QBrush(Qt::lightGray));
    QRect metalRect(0, 0, 8, 4);
    painter.drawRoundedRect(metalRect, 0, 0);

    // 绘制中心十字线指示精确位置
    //painter.setPen(QPen(Qt::red, 1));
    //painter.drawLine(15, 0, 17, 0);  // 上
    //painter.drawLine(15, 31, 17, 31);  // 下
    //painter.drawLine(0, 15, 0, 17);   // 左
    //painter.drawLine(31, 15, 31, 17); // 右

    // 创建光标，热点在中心
    return QCursor(cursorPixmap, 0, 0);
}

void OverlayWidget::drawEraserCursor(QPainter& painter)
{
    if (!m_eraserMode) return;

    // 只有当鼠标在widget范围内时才绘制预览圆形
    if (!hasMouseTracking()) return;
    if (!rect().contains(m_currentMousePos)) return;

    qDebug() << m_currentMousePos;

    // 绘制橡皮擦预览圆形，以鼠标点为圆心
    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    painter.setBrush(QBrush(QColor(0, 0, 0, 30)));  // 半透明红色填充

    int radius = m_eraserSize / 2;
    painter.drawEllipse(m_currentMousePos, radius, radius);
}

void OverlayWidget::showOverlay()
{
    if (!m_targetWidget) return;

    updatePosition();
    show();
    raise();
    activateWindow();
    setFocus();

    // 如果是橡皮擦模式，启用鼠标跟踪
    if (m_eraserMode) {
        setMouseTracking(true);
        m_currentMousePos = mapFromGlobal(QCursor::pos());
        update();
    }
}

void OverlayWidget::hideOverlay()
{
    hide();
    // 注意：不清除内容，保留用户的标注
}

void OverlayWidget::updatePosition()
{
    if (!m_targetWidget) return;

    // 检查目标widget大小是否发生变化
    QSize currentSize = m_targetWidget->size();
    if (currentSize != m_lastTargetSize && m_lastTargetSize.isValid()) {
        // 大小发生变化，需要缩放已有内容
        scaleContent(m_lastTargetSize, currentSize);
        m_lastTargetSize = currentSize;
    }
    else if (!m_lastTargetSize.isValid()) {
        m_lastTargetSize = currentSize;
    }

    // 计算目标widget在屏幕上的全局位置
    QPoint globalPos = m_targetWidget->mapToParent(QPoint(0, 0));

    // 设置遮罩的位置和大小
    setGeometry(globalPos.x(), globalPos.y(),
        m_targetWidget->width(), m_targetWidget->height());

    // 确保工具栏位置在新的遮罩范围内
    constrainToolbarPosition();
}

void OverlayWidget::scaleContent(const QSize& oldSize, const QSize& newSize)
{
    if (oldSize.isEmpty() || newSize.isEmpty()) return;

    double scaleX = (double)newSize.width() / oldSize.width();
    double scaleY = (double)newSize.height() / oldSize.height();

    // 缩放绘制路径
    for (auto& path : m_paths) {
        for (auto& point : path) {
            point.point.setX(qRound(point.point.x() * scaleX));
            point.point.setY(qRound(point.point.y() * scaleY));
        }
    }

    // 缩放当前路径
    for (auto& point : m_currentPath) {
        point.point.setX(qRound(point.point.x() * scaleX));
        point.point.setY(qRound(point.point.y() * scaleY));
    }

    // 缩放文字位置
    for (auto& textItem : m_textItems) {
        textItem.position.setX(qRound(textItem.position.x() * scaleX));
        textItem.position.setY(qRound(textItem.position.y() * scaleY));

        // 可选：也缩放字体大小
        int oldFontSize = textItem.font.pointSize();
        if (oldFontSize > 0) {
            double avgScale = (scaleX + scaleY) / 2.0;
            int newFontSize = qMax(8, qRound(oldFontSize * avgScale));
            textItem.font.setPointSize(newFontSize);
        }
    }

    // 如果正在编辑文字，也需要更新位置
    if (m_textEdit) {
        m_currentTextPosition.setX(qRound(m_currentTextPosition.x() * scaleX));
        m_currentTextPosition.setY(qRound(m_currentTextPosition.y() * scaleY));
        m_textEdit->move(m_currentTextPosition);
    }

    update();
}

void OverlayWidget::installEventFilters()
{
    if (!m_targetWidget) return;

    // 监听目标widget的事件
    m_targetWidget->installEventFilter(this);
}

void OverlayWidget::removeEventFilters()
{
    if (!m_targetWidget) return;
}

bool OverlayWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (!m_targetWidget) return QWidget::eventFilter(obj, event);

    // 监听目标widget或其父widget的几何变化
    if (obj == m_targetWidget) {
        switch (event->type()) {
        case QEvent::Resize:
        case QEvent::Move:
        case QEvent::Show:
        case QEvent::Hide:
            // 延迟更新位置，避免在快速变化时频繁计算
            QTimer::singleShot(0, this, [this]() {
                if (isVisible()) {
                    updatePosition();
                }
                });
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void OverlayWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

        QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制半透明背景以便看到遮罩区域
    painter.fillRect(rect(), QColor(0, 0, 0, 10));

    // 绘制所有路径和文字
    drawPaths(painter);
    drawTexts(painter);

    // 绘制橡皮擦光标
    drawEraserCursor(painter);
}

void OverlayWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    // 确保工具栏位置在新的尺寸范围内
    constrainToolbarPosition();
}

void OverlayWidget::enterEvent(QEvent* event)
{
    Q_UNUSED(event)
        // 鼠标进入widget时，如果是橡皮擦模式则开始跟踪
        if (m_eraserMode) {
            setMouseTracking(true);
            m_currentMousePos = mapFromGlobal(QCursor::pos());
            update();
        }
    QWidget::enterEvent(event);
}

void OverlayWidget::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
        // 鼠标离开widget时停止跟踪
        if (m_eraserMode) {
            setMouseTracking(false);
            update();  // 重绘以隐藏橡皮擦光标
        }
    QWidget::leaveEvent(event);
}

void OverlayWidget::drawPaths(QPainter& painter)
{
    // 绘制所有完成的路径
    for (const auto& path : m_paths) {
        if (path.isEmpty()) continue;

        QPen pen(path.first().color, path.first().width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);

        if (path.size() == 1) {
            painter.drawPoint(path.first().point);
        }
        else {
            for (int i = 1; i < path.size(); ++i) {
                painter.drawLine(path[i - 1].point, path[i].point);
            }
        }
    }

    // 绘制当前正在绘制的路径
    if (!m_currentPath.isEmpty()) {
        QPen pen(m_currentPath.first().color, m_currentPath.first().width,
            Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);

        if (m_currentPath.size() == 1) {
            painter.drawPoint(m_currentPath.first().point);
        }
        else {
            for (int i = 1; i < m_currentPath.size(); ++i) {
                painter.drawLine(m_currentPath[i - 1].point, m_currentPath[i].point);
            }
        }
    }
}

void OverlayWidget::drawTexts(QPainter& painter)
{
    for (const auto& textItem : m_textItems) {
        painter.setPen(textItem.color);
        painter.setFont(textItem.font);
        painter.drawText(textItem.position, textItem.text);
    }
}

void OverlayWidget::mousePressEvent(QMouseEvent* event)
{
    // 更新鼠标位置
    if (m_eraserMode) {
        m_currentMousePos = event->pos();
    }

    if (event->button() == Qt::LeftButton) {
        // 检查是否点击在工具栏标题栏上（开始拖动）
        if (m_toolbarHeader && m_toolbarHeader->geometry().translated(m_toolbar->pos()).contains(event->pos())) {
            m_draggingToolbar = true;
            m_dragStartPos = event->pos();
            m_toolbarDragOffset = event->pos() - m_toolbar->pos();
            return;
        }

        // 检查是否点击在工具栏上（但不是标题栏）
        if (m_toolbar->geometry().contains(event->pos())) {
            return;
        }

        if (m_eraserMode) {
            // 橡皮擦模式
            m_erasing = true;
            m_lastErasePos = event->pos();

            // 清空当前擦除数据
            m_currentErasedData = ErasedData();

            // 执行擦除
            performErase(event->pos());
        }
        else if (m_textMode) {
            // 检查是否点击在已有文字上（可以编辑）
            bool clickedOnText = false;
            for (int i = m_textItems.size() - 1; i >= 0; --i) {
                const TextItem& item = m_textItems[i];
                QFontMetrics fm(item.font);
                QRect textRect(item.position, fm.size(Qt::TextSingleLine, item.text));
                textRect.adjust(-5, -5, 5, 5); // 扩大点击区域

                if (textRect.contains(event->pos())) {
                    // 点击在已有文字上，编辑该文字
                    editTextAt(i, event->pos());
                    clickedOnText = true;
                    break;
                }
            }

            if (!clickedOnText) {
                // 点击在空白处，添加新文字
                addTextAt(event->pos());
            }
        }
        else {
            // 绘制模式
            m_drawing = true;
            m_currentPath.clear();

            DrawPoint point;
            point.point = event->pos();
            point.color = m_penColor;
            point.width = m_penWidth;
            m_currentPath.append(point);

            update();
        }
    }
}

void OverlayWidget::mouseMoveEvent(QMouseEvent* event)
{
    // 在橡皮擦模式下，始终更新鼠标位置
    if (m_eraserMode) {
        m_currentMousePos = event->pos();
        update();  // 重绘以更新橡皮擦光标位置
    }

    if (event->buttons() & Qt::LeftButton) {
        if (m_draggingToolbar) {
            // 拖动工具栏
            QPoint newPos = event->pos() - m_toolbarDragOffset;
            m_toolbar->move(newPos);
            constrainToolbarPosition();
            return;
        }

        if (m_erasing) {
            // 橡皮擦连续擦除
            performErase(event->pos());
            m_lastErasePos = event->pos();
        }
        else if (m_drawing) {
            DrawPoint point;
            point.point = event->pos();
            point.color = m_penColor;
            point.width = m_penWidth;
            m_currentPath.append(point);

            update();
        }
    }
}

void OverlayWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_draggingToolbar) {
            // 结束工具栏拖动
            m_draggingToolbar = false;
            constrainToolbarPosition();
            return;
        }

        if (m_erasing) {
            // 结束擦除操作
            m_erasing = false;

            // 如果有内容被擦除，保存撤销信息
            if (!m_currentErasedData.erasedPaths.isEmpty() || !m_currentErasedData.erasedTexts.isEmpty()) {
                saveAction(ACTION_ERASE, QVector<DrawPoint>(), TextItem(), -1,
                    QString(), QString(), QColor(), QColor(), m_currentErasedData);
            }
        }
        else if (m_drawing) {
            m_drawing = false;

            if (!m_currentPath.isEmpty()) {
                m_paths.append(m_currentPath);

                // 保存撤销信息
                saveAction(ACTION_DRAW_PATH, m_currentPath);

                m_currentPath.clear();
            }

            update();
        }
    }
}

void OverlayWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        finishEditing();
    }
    else if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        clearCanvas();
    }
    else if (event->key() == Qt::Key_Z && (event->modifiers() & Qt::ControlModifier)) {
        if (event->modifiers() & Qt::ShiftModifier) {
            // Ctrl+Shift+Z 重做
            redoLastAction();
        }
        else {
            // Ctrl+Z 撤销
            undoLastAction();
        }
    }
    else if (event->key() == Qt::Key_Y && (event->modifiers() & Qt::ControlModifier)) {
        // Ctrl+Y 重做（另一种常用快捷键）
        redoLastAction();
    }

    QWidget::keyPressEvent(event);
}

void OverlayWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (m_textMode) {
        addTextAt(event->pos());
    }
}

void OverlayWidget::addTextAt(const QPoint& position)
{
    if (m_textEdit) {
        // 如果已有文字框，先保存当前的文字
        finishTextInput();
    }

    m_editingTextIndex = -1;  // 标记为新建文字
    m_currentTextPosition = position;

    m_textEdit = new QLineEdit(this);
    m_textEdit->setStyleSheet(
        "QLineEdit { background-color: white; border: 2px solid blue; padding: 5px; font-size: 12px; }"
    );
    m_textEdit->move(position);
    m_textEdit->resize(200, 25);
    m_textEdit->show();
    m_textEdit->setFocus();

    connect(m_textEdit, &QLineEdit::returnPressed, [this]() {
        finishTextInput();
        });

    connect(m_textEdit, &QLineEdit::editingFinished, [this]() {
        // 延迟一点执行，避免在某些情况下立即删除
        QTimer::singleShot(100, this, [this]() {
            if (m_textEdit && !m_textEdit->hasFocus()) {
                finishTextInput();
            }
            });
        });
}

void OverlayWidget::editTextAt(int index, const QPoint& position)
{
    if (index < 0 || index >= m_textItems.size()) return;

    if (m_textEdit) {
        // 如果已有文字框，先保存当前的文字
        finishTextInput();
    }

    const TextItem& item = m_textItems[index];
    m_editingTextIndex = index;
    m_currentTextPosition = item.position;

    m_textEdit = new QLineEdit(this);
    m_textEdit->setStyleSheet(
        "QLineEdit { background-color: lightyellow; border: 2px solid orange; padding: 5px; font-size: 12px; }"
    );
    m_textEdit->setText(item.text);  // 设置原有文字
    m_textEdit->selectAll();         // 全选文字便于编辑
    m_textEdit->move(item.position);
    m_textEdit->resize(200, 25);
    m_textEdit->show();
    m_textEdit->setFocus();

    connect(m_textEdit, &QLineEdit::returnPressed, [this]() {
        finishTextInput();
        });

    connect(m_textEdit, &QLineEdit::editingFinished, [this]() {
        QTimer::singleShot(100, this, [this]() {
            if (m_textEdit && !m_textEdit->hasFocus()) {
                finishTextInput();
            }
            });
        });
}

void OverlayWidget::finishTextInput()
{
    if (m_textEdit) {
        QString text = m_textEdit->text().trimmed();

        if (m_editingTextIndex >= 0 && m_editingTextIndex < m_textItems.size()) {
            // 编辑已有文字
            QString oldText = m_textItems[m_editingTextIndex].text;
            QColor oldColor = m_textItems[m_editingTextIndex].color;

            if (text.isEmpty()) {
                // 如果文字为空，删除该文字项
                TextItem deletedItem = m_textItems[m_editingTextIndex];
                m_textItems.removeAt(m_editingTextIndex);

                // 保存删除操作的撤销信息
                saveAction(ACTION_DELETE_TEXT, QVector<DrawPoint>(), deletedItem, m_editingTextIndex);
            }
            else {
                // 更新文字内容
                m_textItems[m_editingTextIndex].text = text;
                m_textItems[m_editingTextIndex].color = m_penColor;

                // 保存编辑操作的撤销信息（包含新旧信息）
                saveAction(ACTION_EDIT_TEXT, QVector<DrawPoint>(), m_textItems[m_editingTextIndex],
                    m_editingTextIndex, oldText, text, oldColor, m_penColor);
            }
        }
        else {
            // 新建文字
            if (!text.isEmpty()) {
                TextItem item;
                item.position = m_currentTextPosition;
                item.text = text;
                item.color = m_penColor;
                item.font = QFont("Microsoft YaHei", 12);
                m_textItems.append(item);

                // 保存添加操作的撤销信息
                saveAction(ACTION_ADD_TEXT, QVector<DrawPoint>(), item);
            }
        }

        m_editingTextIndex = -1;  // 重置编辑索引
        update();

        m_textEdit->deleteLater();
        m_textEdit = nullptr;
    }
}

void OverlayWidget::changePenColor()
{
    QColor newColor = QColorDialog::getColor(m_penColor, this, "选择颜色");
    if (newColor.isValid()) {
        m_penColor = newColor;
        m_colorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(m_penColor.name()));
    }
}

void OverlayWidget::changePenWidth(int width)
{
    m_penWidth = width;
}

void OverlayWidget::toggleTextMode(bool enabled)
{
    // 切换模式时先完成当前的文字输入
    if (m_textEdit && !enabled) {
        finishTextInput();
    }

    m_textMode = enabled;

    // 如果启用文字模式，禁用橡皮擦模式
    if (enabled && m_eraserMode) {
        m_eraserMode = false;
        m_eraserModeCheckBox->setChecked(false);
        m_eraserSizeSpinBox->setEnabled(false);
        setCursor(Qt::ArrowCursor);
    }
}

void OverlayWidget::clearCanvas()
{
    // 先完成正在进行的文字输入
    finishTextInput();

    m_paths.clear();
    m_currentPath.clear();
    m_textItems.clear();

    // 清除撤销和重做栈
    m_undoStack.clear();
    m_redoStack.clear();
    updateUndoRedoButtons();

    update();
}

void OverlayWidget::saveAction(ActionType type, const QVector<DrawPoint>& pathData,
    const TextItem& textData, int textIndex,
    const QString& oldText, const QString& newText,
    const QColor& oldColor, const QColor& newColor,
    const ErasedData& erasedData)
{
    // 进行新操作时清空重做栈（历史分支改变）
    clearRedoStack();

    UndoAction action;
    action.type = type;
    action.pathData = pathData;
    action.textData = textData;
    action.textIndex = textIndex;
    action.oldText = oldText;
    action.newText = newText;
    action.oldColor = oldColor;
    action.newColor = newColor;
    action.erasedData = erasedData;

    m_undoStack.append(action);

    // 限制撤销栈大小
    if (m_undoStack.size() > MAX_UNDO_STEPS) {
        m_undoStack.removeFirst();
    }

    updateUndoRedoButtons();
}

void OverlayWidget::updateUndoRedoButtons()
{
    if (m_undoButton) {
        m_undoButton->setEnabled(!m_undoStack.isEmpty());
    }
    if (m_redoButton) {
        m_redoButton->setEnabled(!m_redoStack.isEmpty());
    }
}

void OverlayWidget::clearRedoStack()
{
    m_redoStack.clear();
    updateUndoRedoButtons();
}

void OverlayWidget::undoLastAction()
{
    if (m_undoStack.isEmpty()) return;

    // 先完成正在进行的文字输入
    finishTextInput();

    UndoAction lastAction = m_undoStack.takeLast();

    switch (lastAction.type) {
    case ACTION_DRAW_PATH:
        // 撤销绘制路径：移除最后一条路径
        if (!m_paths.isEmpty()) {
            m_paths.removeLast();
        }
        break;

    case ACTION_ADD_TEXT:
        // 撤销添加文字：移除最后添加的文字
        for (int i = m_textItems.size() - 1; i >= 0; --i) {
            const TextItem& item = m_textItems[i];
            if (item.position == lastAction.textData.position &&
                item.text == lastAction.textData.text) {
                m_textItems.removeAt(i);
                break;
            }
        }
        break;

    case ACTION_EDIT_TEXT:
        // 撤销编辑文字：恢复原文字内容
        if (lastAction.textIndex >= 0 && lastAction.textIndex < m_textItems.size()) {
            m_textItems[lastAction.textIndex].text = lastAction.oldText;
            m_textItems[lastAction.textIndex].color = lastAction.oldColor;
        }
        break;

    case ACTION_DELETE_TEXT:
        // 撤销删除文字：重新插入文字
        if (lastAction.textIndex >= 0 && lastAction.textIndex <= m_textItems.size()) {
            m_textItems.insert(lastAction.textIndex, lastAction.textData);
        }
        break;

    case ACTION_ERASE:
        // 撤销橡皮擦操作：恢复被擦除的内容
    {
        const ErasedData& erasedData = lastAction.erasedData;

        // 恢复被擦除的路径（按原来的索引顺序）
        for (int i = 0; i < erasedData.erasedPathIndices.size(); ++i) {
            int originalIndex = erasedData.erasedPathIndices[i];
            const auto& erasedPath = erasedData.erasedPaths[i];

            // 插入到适当位置（需要考虑其他路径的变化）
            if (originalIndex <= m_paths.size()) {
                m_paths.insert(originalIndex, erasedPath);
            }
            else {
                m_paths.append(erasedPath);
            }
        }

        // 恢复被擦除的文字（按原来的索引顺序）
        for (int i = 0; i < erasedData.erasedTextIndices.size(); ++i) {
            int originalIndex = erasedData.erasedTextIndices[i];
            const TextItem& erasedText = erasedData.erasedTexts[i];

            // 插入到适当位置
            if (originalIndex <= m_textItems.size()) {
                m_textItems.insert(originalIndex, erasedText);
            }
            else {
                m_textItems.append(erasedText);
            }
        }
    }
    break;
    }

    // 将撤销的操作放入重做栈
    m_redoStack.append(lastAction);

    // 限制重做栈大小
    if (m_redoStack.size() > MAX_UNDO_STEPS) {
        m_redoStack.removeFirst();
    }

    updateUndoRedoButtons();
    update();
}

void OverlayWidget::redoLastAction()
{
    if (m_redoStack.isEmpty()) return;

    // 先完成正在进行的文字输入
    finishTextInput();

    UndoAction lastRedoAction = m_redoStack.takeLast();

    switch (lastRedoAction.type) {
    case ACTION_DRAW_PATH:
        // 重做绘制路径：重新添加路径
        m_paths.append(lastRedoAction.pathData);
        break;

    case ACTION_ADD_TEXT:
        // 重做添加文字：重新添加文字
        m_textItems.append(lastRedoAction.textData);
        break;

    case ACTION_EDIT_TEXT:
        // 重做编辑文字：应用新文字内容
        if (lastRedoAction.textIndex >= 0 && lastRedoAction.textIndex < m_textItems.size()) {
            m_textItems[lastRedoAction.textIndex].text = lastRedoAction.newText;
            m_textItems[lastRedoAction.textIndex].color = lastRedoAction.newColor;
        }
        break;

    case ACTION_DELETE_TEXT:
        // 重做删除文字：重新删除文字
        if (lastRedoAction.textIndex >= 0 && lastRedoAction.textIndex < m_textItems.size()) {
            m_textItems.removeAt(lastRedoAction.textIndex);
        }
        break;

    case ACTION_ERASE:
        // 重做橡皮擦操作：重新执行擦除
    {
        const ErasedData& erasedData = lastRedoAction.erasedData;

        // 重新删除路径（从后往前删除以保持索引正确）
        QVector<int> sortedPathIndices = erasedData.erasedPathIndices;
        std::sort(sortedPathIndices.begin(), sortedPathIndices.end(), std::greater<int>());
        for (int index : sortedPathIndices) {
            if (index >= 0 && index < m_paths.size()) {
                m_paths.removeAt(index);
            }
        }

        // 重新删除文字（从后往前删除以保持索引正确）
        QVector<int> sortedTextIndices = erasedData.erasedTextIndices;
        std::sort(sortedTextIndices.begin(), sortedTextIndices.end(), std::greater<int>());
        for (int index : sortedTextIndices) {
            if (index >= 0 && index < m_textItems.size()) {
                m_textItems.removeAt(index);
            }
        }
    }
    break;
    }

    // 将重做的操作放回撤销栈
    m_undoStack.append(lastRedoAction);

    // 限制撤销栈大小
    if (m_undoStack.size() > MAX_UNDO_STEPS) {
        m_undoStack.removeFirst();
    }

    updateUndoRedoButtons();
    update();
}

void OverlayWidget::toggleToolbarCollapse()
{
    m_toolbarCollapsed = !m_toolbarCollapsed;
    updateToolbarLayout();
}

void OverlayWidget::updateToolbarLayout()
{
    if (!m_toolbar || !m_toolbarContent || !m_collapseButton) return;

    if (m_toolbarCollapsed) {
        // 收起状态：隐藏内容区域
        m_toolbarContent->hide();
        m_collapseButton->setText("+");
        m_collapseButton->setToolTip("展开工具栏");
    }
    else {
        // 展开状态：显示内容区域
        m_toolbarContent->show();
        m_collapseButton->setText("−");
        m_collapseButton->setToolTip("收起工具栏");
    }

    // 调整工具栏大小
    m_toolbar->adjustSize();
    constrainToolbarPosition();
}

void OverlayWidget::constrainToolbarPosition()
{
    if (!m_toolbar) return;

    QPoint pos = m_toolbar->pos();
    QSize toolbarSize = m_toolbar->size();

    // 确保工具栏不会超出遮罩边界
    int maxX = width() - toolbarSize.width();
    int maxY = height() - toolbarSize.height();

    pos.setX(qMax(0, qMin(pos.x(), maxX)));
    pos.setY(qMax(0, qMin(pos.y(), maxY)));

    m_toolbar->move(pos);
}

void OverlayWidget::finishEditing()
{
    // 先完成可能正在进行的文字输入
    finishTextInput();
    hideOverlay();
}

void OverlayWidget::saveImage()
{
    if (!m_targetWidget) return;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存标注图片",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) +
        "/annotation_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".png",
        "PNG Files (*.png);;JPEG Files (*.jpg);;All Files (*)"
    );

    if (!fileName.isEmpty()) {
        // 创建合成图片：目标widget + 标注
        QPixmap pixmap(m_targetWidget->size());

        // 先绘制目标widget的内容
        m_targetWidget->render(&pixmap);

        // 再在上面绘制标注
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        drawPaths(painter);
        drawTexts(painter);

        if (pixmap.save(fileName)) {
            QMessageBox::information(this, "保存成功", "标注图片已保存到: " + fileName);
        }
        else {
            QMessageBox::warning(this, "保存失败", "无法保存图片");
        }
    }
}