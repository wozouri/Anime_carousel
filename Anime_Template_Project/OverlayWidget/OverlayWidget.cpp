#include "OverlayWidget.h"
#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include <QMessageBox>
#include <QEvent>
#include <QTimer>
#include <QDebug>
#include <algorithm>

OverlayWidget::OverlayWidget(QWidget* targetWidget, QWidget* parent)
    : QWidget(parent)
    , m_targetWidget(targetWidget)
    , m_penColor(Qt::red)
    , m_penWidth(3)
    , m_drawing(false)
    , m_textEdit(nullptr)
    , m_fontSize(12)
    , m_textMode(false)
    , m_editingTextIndex(-1)
    , m_eraserMode(false)
    , m_eraserSize(20)
    , m_erasing(false)
    , m_currentMousePos(0, 0)
    , m_toolbarCollapsed(false)
    , m_draggingToolbar(false)
    , m_geometryUpdatePending(false)
    , m_updateTimer(new QTimer(this))
    , m_baseSizeInitialized(false)
    , m_useRelativeCoordinates(true)
    , m_hasEditingRelativeText(false)
    , m_debugMode(false)
    , m_useHighPrecision(false)
    , m_rectCacheValid(false)
    , m_updateCount(0)
{
    if (!m_targetWidget) {
        qWarning("OverlayWidget: targetWidget cannot be null");
        return;
    }

    // 初始化定时器
    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(16); // 约60fps的更新频率
    connect(m_updateTimer, &QTimer::timeout, this, &OverlayWidget::updateOverlayGeometry);

    // 记录初始几何信息
    m_lastTargetGeometry = m_targetWidget->geometry();
    m_lastTargetSize = m_targetWidget->size();

    // 设置窗口属性 - 透明遮罩
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);

    // 设置焦点策略
    setFocusPolicy(Qt::StrongFocus);

    // 初始化相对坐标系统
    initializeRelativeSystem();

    setupUI();
    calculatePreciseGeometry();

    // 安装事件过滤器监听目标widget变化
    installEventFilters();

    // 隐藏遮罩，需要时调用showOverlay()
    hide();
}

OverlayWidget::~OverlayWidget()
{
    // 清理定时器
    if (m_updateTimer) {
        m_updateTimer->stop();
    }

    // 清理事件过滤器
    removeEventFilters();

    // 清理文字输入框
    if (m_textEdit) {
        m_textEdit->deleteLater();
        m_textEdit = nullptr;
    }
}

// ============================================================================
// 相对坐标系统实现
// ============================================================================

void OverlayWidget::initializeRelativeSystem()
{
    if (!m_targetWidget) return;

    m_baseSize = m_targetWidget->size();
    if (m_baseSize.isEmpty()) {
        m_baseSize = QSize(800, 600); // 默认基准尺寸
    }
    m_baseSizeInitialized = true;

    if (m_debugMode) {
        qDebug() << "Relative coordinate system initialized with base size:" << m_baseSize;
    }
}

void OverlayWidget::convertToRelativeCoordinates()
{
    if (!m_baseSizeInitialized || m_baseSize.isEmpty()) return;

    // 转换现有的绝对坐标路径为相对坐标
    m_relativePaths.clear();
    for (const auto& path : m_paths) {
        QVector<RelativePoint> relativePath;
        for (const auto& point : path) {
            relativePath.append(RelativePoint::fromAbsolute(
                point.point, m_baseSize, point.color, point.width));
        }
        m_relativePaths.append(relativePath);
    }

    // 转换当前路径
    m_currentRelativePath.clear();
    for (const auto& point : m_currentPath) {
        m_currentRelativePath.append(RelativePoint::fromAbsolute(
            point.point, m_baseSize, point.color, point.width));
    }

    // 转换文字项
    m_relativeTextItems.clear();
    for (const auto& textItem : m_textItems) {
        m_relativeTextItems.append(RelativeTextItem::fromAbsolute(textItem, m_baseSize));
    }

    if (m_debugMode) {
        qDebug() << "Converted to relative coordinates:" << m_relativePaths.size()
            << "paths," << m_relativeTextItems.size() << "texts";
    }
}

void OverlayWidget::updateAbsoluteFromRelative()
{
    if (!m_targetWidget) return;

    QSize currentSize = m_targetWidget->size();
    if (currentSize.isEmpty()) return;

    // 从相对坐标重建绝对坐标路径
    m_paths.clear();
    for (const auto& relativePath : m_relativePaths) {
        QVector<DrawPoint> absolutePath;
        for (const auto& relativePoint : relativePath) {
            DrawPoint point;
            point.point = relativePoint.toAbsolute(currentSize);
            point.color = relativePoint.color;
            point.width = relativePoint.width;
            absolutePath.append(point);
        }
        m_paths.append(absolutePath);
    }

    // 重建当前路径
    m_currentPath.clear();
    for (const auto& relativePoint : m_currentRelativePath) {
        DrawPoint point;
        point.point = relativePoint.toAbsolute(currentSize);
        point.color = relativePoint.color;
        point.width = relativePoint.width;
        m_currentPath.append(point);
    }

    // 重建文字项
    m_textItems.clear();
    for (const auto& relativeItem : m_relativeTextItems) {
        m_textItems.append(relativeItem.toAbsolute(currentSize));
    }

    // 更新正在编辑的文字位置
    if (m_textEdit && m_hasEditingRelativeText) {
        QPoint newPos = m_currentEditingRelativeText.toAbsolutePosition(currentSize);
        m_currentTextPosition = newPos;
        m_textEdit->move(newPos);

        // 更新字体大小
        QFont newFont = m_currentEditingRelativeText.toAbsoluteFont(currentSize);
        QString styleSheet = QString(
            "QLineEdit { background-color: white; border: 2px solid blue; "
            "padding: 5px; font-size: %1px; }").arg(newFont.pointSize());
        m_textEdit->setStyleSheet(styleSheet);
    }

    if (m_debugMode) {
        qDebug() << "Updated absolute coordinates for size:" << currentSize;
    }
}

void OverlayWidget::syncRelativeData()
{
    if (!m_useRelativeCoordinates) return;

    // 确保相对坐标数据与绝对坐标数据同步
    if (!m_baseSizeInitialized) {
        initializeRelativeSystem();
    }
    convertToRelativeCoordinates();
}

OverlayWidget::RelativePoint OverlayWidget::pointToRelative(const QPoint& point) const
{
    if (!m_targetWidget) return RelativePoint();

    QSize currentSize = m_targetWidget->size();
    return RelativePoint::fromAbsolute(point, currentSize, m_penColor, m_penWidth);
}

QPoint OverlayWidget::relativeToPoint(const RelativePoint& relativePoint) const
{
    if (!m_targetWidget) return QPoint();

    QSize currentSize = m_targetWidget->size();
    return relativePoint.toAbsolute(currentSize);
}

// ============================================================================
// 位置和几何更新
// ============================================================================

void OverlayWidget::calculatePreciseGeometry()
{
    if (!m_targetWidget) return;

    // 获取目标widget的全局矩形
    QRect globalRect = getTargetWidgetGlobalRect();

    // 如果overlay有父widget，需要转换坐标
    if (parentWidget()) {
        QPoint parentGlobalPos = parentWidget()->mapToGlobal(QPoint(0, 0));
        globalRect.translate(-parentGlobalPos);
    }

    // 设置overlay的几何
    setGeometry(globalRect);

    // 更新工具栏位置约束
    constrainToolbarPosition();

    // 使缓存失效
    m_rectCacheValid = false;
}

QRect OverlayWidget::getTargetWidgetGlobalRect() 
{
    if (!m_targetWidget) return QRect();

    // 使用缓存提高性能
    if (m_rectCacheValid && m_useHighPrecision) {
        return m_cachedTargetRect;
    }

    // 获取目标widget的实际显示区域
    QPoint globalPos = m_targetWidget->mapToGlobal(QPoint(0, 0));
    QSize size = m_targetWidget->size();

    // 应用边距调整
    QRect rect(globalPos, size);
    rect = rect.marginsRemoved(m_targetMargins);

    // 确保矩形有效
    if (rect.width() < 1) rect.setWidth(1);
    if (rect.height() < 1) rect.setHeight(1);

    // 缓存结果
    if (m_useHighPrecision) {
        m_cachedTargetRect = rect;
        m_rectCacheValid = true;
    }

    if (m_debugMode) {
        qDebug() << "Target rect:" << rect << "Update count:" << ++m_updateCount;
    }

    return rect;
}

QPoint OverlayWidget::getTargetWidgetGlobalPosition() const
{
    if (!m_targetWidget) return QPoint();
    return m_targetWidget->mapToGlobal(QPoint(0, 0));
}

void OverlayWidget::updatePosition()
{
    if (!m_targetWidget) return;

    // 检查几何是否发生变化
    if (isGeometryChanged()) {
        handleGeometryChange();
    }

    // 延迟更新以避免频繁计算
    if (!m_updateTimer->isActive()) {
        m_updateTimer->start();
    }
}

bool OverlayWidget::isGeometryChanged() const
{
    if (!m_targetWidget) return false;

    QRect currentGeometry = m_targetWidget->geometry();
    QPoint currentGlobalPos = getTargetWidgetGlobalPosition();

    return (currentGeometry != m_lastTargetGeometry) ||
        (currentGlobalPos != m_targetWidgetOffset);
}

void OverlayWidget::handleGeometryChange()
{
    if (!m_targetWidget) return;

    QRect currentGeometry = m_targetWidget->geometry();
    QSize currentSize = m_targetWidget->size();

    // 检查大小是否发生变化
    if (currentSize != m_lastTargetSize && m_lastTargetSize.isValid()) {
        // 大小发生变化，需要处理缩放
        if (m_useRelativeCoordinates) {
            // 使用相对坐标系统，无需累积缩放
            updateAbsoluteFromRelative();
        }
        else {
            // 使用传统缩放方法
            scaleContent(m_lastTargetSize, currentSize);
        }
    }

    // 更新记录的几何信息
    m_lastTargetGeometry = currentGeometry;
    m_lastTargetSize = currentSize;
    m_targetWidgetOffset = getTargetWidgetGlobalPosition();
}

void OverlayWidget::updateOverlayGeometry()
{
    if (!m_targetWidget) return;

    calculatePreciseGeometry();

    // 如果overlay可见，确保它在正确位置
    if (isVisible()) {
        raise();
        update();
    }
}

void OverlayWidget::scaleContent(const QSize& oldSize, const QSize& newSize)
{
    if (oldSize.isEmpty() || newSize.isEmpty()) return;

    if (m_useRelativeCoordinates) {
        // 使用相对坐标恢复（零误差）
        updateAbsoluteFromRelative();
    }
    else {
        // 传统缩放方法（会有累积误差）
        double scaleX = static_cast<double>(newSize.width()) / oldSize.width();
        double scaleY = static_cast<double>(newSize.height()) / oldSize.height();

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

        // 缩放文字位置和大小
        for (auto& textItem : m_textItems) {
            textItem.position.setX(qRound(textItem.position.x() * scaleX));
            textItem.position.setY(qRound(textItem.position.y() * scaleY));

            int oldFontSize = textItem.font.pointSize();
            if (oldFontSize > 0) {
                double avgScale = (scaleX + scaleY) / 2.0;
                int newFontSize = qMax(6, qRound(oldFontSize * avgScale));
                textItem.font.setPointSize(newFontSize);
            }
        }

        // 处理正在编辑的文字
        if (m_textEdit) {
            m_currentTextPosition.setX(qRound(m_currentTextPosition.x() * scaleX));
            m_currentTextPosition.setY(qRound(m_currentTextPosition.y() * scaleY));
            m_textEdit->move(m_currentTextPosition);

            // 更新编辑框字体大小
            QString currentStyle = m_textEdit->styleSheet();
            QRegularExpression regex("font-size:\\s*(\\d+)px");
            QRegularExpressionMatch match = regex.match(currentStyle);
            if (match.hasMatch()) {
                int oldFontSize = match.captured(1).toInt();
                double avgScale = (scaleX + scaleY) / 2.0;
                int newFontSize = qMax(8, qRound(oldFontSize * avgScale));
                currentStyle.replace(regex, QString("font-size: %1px").arg(newFontSize));
                m_textEdit->setStyleSheet(currentStyle);
            }
        }

        // 同步到相对坐标
        syncRelativeData();
    }

    update();
}

// ============================================================================
// 事件过滤器
// ============================================================================

void OverlayWidget::installEventFilters()
{
    if (!m_targetWidget) return;

    // 监听目标widget的事件
    m_targetWidget->installEventFilter(this);

    // 也监听目标widget的父widget事件（用于检测父widget的变化）
    QWidget* parent = m_targetWidget->parentWidget();
    while (parent) {
        parent->installEventFilter(this);
        parent = parent->parentWidget();
    }
}

void OverlayWidget::removeEventFilters()
{
    if (!m_targetWidget) return;

    m_targetWidget->removeEventFilter(this);

    // 移除父widget的事件过滤器
    QWidget* parent = m_targetWidget->parentWidget();
    while (parent) {
        parent->removeEventFilter(this);
        parent = parent->parentWidget();
    }
}

bool OverlayWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (!m_targetWidget) return QWidget::eventFilter(obj, event);

    // 监听目标widget及其父widget的几何变化
    bool shouldUpdate = false;

    switch (event->type()) {
    case QEvent::Resize:
    case QEvent::Move:
        shouldUpdate = true;
        break;
    case QEvent::Show:
    case QEvent::Hide:
        shouldUpdate = true;
        break;
    case QEvent::ParentChange:
        // 父widget发生变化时重新安装事件过滤器
        removeEventFilters();
        installEventFilters();
        shouldUpdate = true;
        break;
    default:
        break;
    }

    if (shouldUpdate && isVisible()) {
        // 延迟更新以避免频繁计算
        updatePosition();
    }

    return QWidget::eventFilter(obj, event);
}

// ============================================================================
// UI设置
// ============================================================================

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
    QLabel* titleLabel = new QLabel(tr("标注工具"), m_toolbarHeader);
    titleLabel->setStyleSheet("color: white; font-size: 12px; font-weight: bold;");
    titleLabel->setCursor(Qt::SizeAllCursor);

    // 收起/展开按钮
    m_collapseButton = new QPushButton("−", m_toolbarHeader);
    m_collapseButton->setFixedSize(20, 20);
    m_collapseButton->setToolTip(tr("收起工具栏"));
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
    m_colorButton = new QPushButton(tr("颜色"), m_toolbarContent);
    m_colorButton->setFixedSize(50, 28);
    m_colorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(m_penColor.name()));
    connect(m_colorButton, &QPushButton::clicked, this, &OverlayWidget::changePenColor);

    // 画笔宽度
    QLabel* widthLabel = new QLabel(tr("宽度:"), m_toolbarContent);
    m_widthSpinBox = new QSpinBox(m_toolbarContent);
    m_widthSpinBox->setRange(1, 20);
    m_widthSpinBox->setValue(m_penWidth);
    m_widthSpinBox->setFixedSize(60, 28);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &OverlayWidget::changePenWidth);

    // 文字模式复选框
    m_textModeCheckBox = new QCheckBox(tr("文字"), m_toolbarContent);
    connect(m_textModeCheckBox, &QCheckBox::toggled, this, &OverlayWidget::toggleTextMode);

    // 文字字体大小
    m_fontSizeSpinBox = new QSpinBox(m_toolbarContent);
    m_fontSizeSpinBox->setRange(8, 72);
    m_fontSizeSpinBox->setValue(m_fontSize);
    m_fontSizeSpinBox->setFixedSize(60, 28);
    m_fontSizeSpinBox->setEnabled(false);
    connect(m_fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &OverlayWidget::changeFontSize);

    // 橡皮擦模式复选框
    m_eraserModeCheckBox = new QCheckBox(tr("橡皮擦"), m_toolbarContent);
    connect(m_eraserModeCheckBox, &QCheckBox::toggled, this, &OverlayWidget::toggleEraserMode);

    // 橡皮擦大小
    QLabel* eraserSizeLabel = new QLabel(tr("擦除:"), m_toolbarContent);
    m_eraserSizeSpinBox = new QSpinBox(m_toolbarContent);
    m_eraserSizeSpinBox->setRange(10, 80);
    m_eraserSizeSpinBox->setValue(m_eraserSize);
    m_eraserSizeSpinBox->setFixedSize(60, 28);
    m_eraserSizeSpinBox->setEnabled(false);
    connect(m_eraserSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &OverlayWidget::changeEraserSize);

    drawingToolsLayout->addWidget(m_colorButton);
    drawingToolsLayout->addWidget(widthLabel);
    drawingToolsLayout->addWidget(m_widthSpinBox);
    drawingToolsLayout->addWidget(m_textModeCheckBox);
    drawingToolsLayout->addWidget(m_fontSizeSpinBox);
    drawingToolsLayout->addWidget(m_eraserModeCheckBox);
    drawingToolsLayout->addWidget(eraserSizeLabel);
    drawingToolsLayout->addWidget(m_eraserSizeSpinBox);
    drawingToolsLayout->addStretch();

    // 第二行：操作按钮
    QHBoxLayout* actionButtonsLayout = new QHBoxLayout();
    actionButtonsLayout->setSpacing(5);

    // 撤销按钮
    m_undoButton = new QPushButton(tr("撤销"), m_toolbarContent);
    m_undoButton->setFixedSize(50, 28);
    m_undoButton->setEnabled(false);
    connect(m_undoButton, &QPushButton::clicked, this, &OverlayWidget::undoLastAction);

    // 重做按钮
    m_redoButton = new QPushButton(tr("重做"), m_toolbarContent);
    m_redoButton->setFixedSize(50, 28);
    m_redoButton->setEnabled(false);
    connect(m_redoButton, &QPushButton::clicked, this, &OverlayWidget::redoLastAction);

    // 清除按钮
    m_clearButton = new QPushButton(tr("清除"), m_toolbarContent);
    m_clearButton->setFixedSize(50, 28);
    connect(m_clearButton, &QPushButton::clicked, this, &OverlayWidget::clearCanvas);

    // 保存按钮
    m_saveButton = new QPushButton(tr("保存"), m_toolbarContent);
    m_saveButton->setFixedSize(50, 28);
    connect(m_saveButton, &QPushButton::clicked, this, &OverlayWidget::saveImage);

    // 完成按钮
    m_finishButton = new QPushButton(tr("完成"), m_toolbarContent);
    m_finishButton->setFixedSize(50, 28);
    connect(m_finishButton, &QPushButton::clicked, this, &OverlayWidget::finishEditing);

    actionButtonsLayout->addWidget(m_undoButton);
    actionButtonsLayout->addWidget(m_redoButton);
    actionButtonsLayout->addStretch();
    actionButtonsLayout->addWidget(m_clearButton);
    actionButtonsLayout->addWidget(m_saveButton);
    actionButtonsLayout->addWidget(m_finishButton);

    // 调试按钮（仅在调试模式下显示）
    if (m_debugMode) {
        QPushButton* testButton = new QPushButton(tr("测试"), m_toolbarContent);
        testButton->setFixedSize(50, 28);
        connect(testButton, &QPushButton::clicked, this, &OverlayWidget::testScalingAccuracy);
        actionButtonsLayout->addWidget(testButton);
    }

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

// ============================================================================
// 绘制相关
// ============================================================================

void OverlayWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

        QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制半透明背景
    if (m_debugMode) {
        painter.fillRect(rect(), QColor(255, 0, 0, 30)); // 红色调试背景
    }
    else {
        painter.fillRect(rect(), QColor(0, 0, 0, 10));   // 正常半透明背景
    }

    // 绘制所有路径和文字
    drawPaths(painter);
    drawTexts(painter);

    // 绘制橡皮擦光标
    drawEraserCursor(painter);

    // 调试模式下绘制额外信息
    if (m_debugMode) {
        drawDebugInfo(painter);
    }
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

void OverlayWidget::drawEraserCursor(QPainter& painter)
{
    if (!m_eraserMode) return;

    // 只有当鼠标在widget范围内时才绘制预览圆形
    if (!hasMouseTracking()) return;
    if (!rect().contains(m_currentMousePos)) return;

    // 绘制橡皮擦预览圆形，以鼠标点为圆心
    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    painter.setBrush(QBrush(QColor(255, 0, 0, 30)));  // 半透明红色填充

    int radius = m_eraserSize / 2;
    painter.drawEllipse(m_currentMousePos, radius, radius);
}

void OverlayWidget::drawDebugInfo(QPainter& painter)
{
    painter.setPen(QPen(Qt::yellow, 2));
    painter.setBrush(Qt::NoBrush);

    // 绘制overlay边界
    painter.drawRect(rect().adjusted(1, 1, -1, -1));

    // 显示坐标信息
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));

    QString info = QString("Overlay: %1x%2 at (%3,%4)")
        .arg(width()).arg(height())
        .arg(x()).arg(y());

    if (m_targetWidget) {
        info += QString("\nTarget: %1x%2")
            .arg(m_targetWidget->width())
            .arg(m_targetWidget->height());
    }

    info += QString("\nPaths: %1, Texts: %2")
        .arg(m_paths.size())
        .arg(m_textItems.size());

    if (m_useRelativeCoordinates) {
        info += QString("\nRel Paths: %1, Rel Texts: %2")
            .arg(m_relativePaths.size())
            .arg(m_relativeTextItems.size());
    }

    painter.drawText(10, 20, info);
}

// ============================================================================
// 鼠标事件处理
// ============================================================================

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
            m_currentErasedData = ErasedData();
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

            if (m_useRelativeCoordinates) {
                // 直接存储相对坐标
                m_currentRelativePath.clear();
                RelativePoint relativePoint = pointToRelative(event->pos());
                m_currentRelativePath.append(relativePoint);
            }

            // 同时维护绝对坐标用于显示
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
            if (m_useRelativeCoordinates) {
                // 添加相对坐标点
                RelativePoint relativePoint = pointToRelative(event->pos());
                m_currentRelativePath.append(relativePoint);
            }

            // 同时维护绝对坐标用于显示
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
            if (!m_currentErasedData.isEmpty()) {
                saveAction(ACTION_ERASE, QVector<DrawPoint>(), TextItem(), -1,
                    QString(), QString(), QColor(), QColor(), m_currentErasedData);

                // 同步相对坐标
                if (m_useRelativeCoordinates) {
                    syncRelativeData();
                }
            }
        }
        else if (m_drawing) {
            m_drawing = false;

            if (!m_currentPath.isEmpty()) {
                if (m_useRelativeCoordinates) {
                    // 保存相对坐标路径
                    m_relativePaths.append(m_currentRelativePath);
                    m_currentRelativePath.clear();
                }

                // 保存绝对坐标路径
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

void OverlayWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    // 确保工具栏位置在新的尺寸范围内
    constrainToolbarPosition();

    // 检查是否需要更新目标widget的跟踪
    if (m_targetWidget && isVisible()) {
        QTimer::singleShot(0, this, [this]() {
            calculatePreciseGeometry();
            });
    }
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

void OverlayWidget::wheelEvent(QWheelEvent* event)
{
    // 获取滚动的方向和数量
    int delta = event->angleDelta().y();

    if (m_eraserMode) {
        m_eraserSize += delta / 40;
        m_eraserSize = qBound(10, m_eraserSize, 80);
        m_eraserSizeSpinBox->setValue(m_eraserSize);
    }
    else if (m_textMode) {
        m_fontSize += delta / 120;
        m_fontSize = qBound(8, m_fontSize, 72);
        m_fontSizeSpinBox->setValue(m_fontSize);
        if (m_textEdit) {
            QString styleSheet = QString(
                "QLineEdit { background-color: white; border: 2px solid blue; padding: 5px; font-size: %1px; }").arg(m_fontSize);
            m_textEdit->setStyleSheet(styleSheet);
        }
    }
    else {
        m_penWidth += delta / 120;
        m_penWidth = qBound(1, m_penWidth, 20);
        m_widthSpinBox->setValue(m_penWidth);
    }

    QWidget::wheelEvent(event);
}

// ============================================================================
// 文字处理
// ============================================================================

void OverlayWidget::addTextAt(const QPoint& position)
{
    if (m_textEdit) {
        finishTextInput();
    }

    m_editingTextIndex = -1;
    m_currentTextPosition = position;

    // 记录当前编辑文字的相对坐标
    if (m_useRelativeCoordinates && m_targetWidget) {
        QSize currentSize = m_targetWidget->size();
        if (!currentSize.isEmpty()) {
            m_currentEditingRelativeText.x = static_cast<double>(position.x()) / currentSize.width();
            m_currentEditingRelativeText.y = static_cast<double>(position.y()) / currentSize.height();
            m_currentEditingRelativeText.relativeFontSize = static_cast<double>(m_fontSize) / currentSize.height();
            m_currentEditingRelativeText.color = m_penColor;
            m_currentEditingRelativeText.fontFamily = "Microsoft YaHei";
            m_currentEditingRelativeText.bold = false;
            m_currentEditingRelativeText.italic = false;
            m_hasEditingRelativeText = true;

            if (m_debugMode) {
                qDebug() << "Recorded editing text relative coords:" << m_currentEditingRelativeText.x
                    << m_currentEditingRelativeText.y;
            }
        }
    }

    m_textEdit = new QLineEdit(this);
    m_textEdit->setStyleSheet(
        QString("QLineEdit { background-color: white; border: 2px solid blue; "
            "padding: 5px; font-size: %1px; }").arg(m_fontSize));
    m_textEdit->move(position);
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

void OverlayWidget::editTextAt(int index, const QPoint& position)
{
    if (index < 0 || index >= m_textItems.size()) return;

    if (m_textEdit) {
        finishTextInput();
    }

    const TextItem& item = m_textItems[index];
    m_editingTextIndex = index;
    m_currentTextPosition = item.position;

    // 记录当前编辑文字的相对坐标
    if (m_useRelativeCoordinates && m_targetWidget) {
        QSize currentSize = m_targetWidget->size();
        if (!currentSize.isEmpty()) {
            m_currentEditingRelativeText = RelativeTextItem::fromAbsolute(item, currentSize);
            m_hasEditingRelativeText = true;

            if (m_debugMode) {
                qDebug() << "Recorded editing text relative coords (edit mode):"
                    << m_currentEditingRelativeText.x << m_currentEditingRelativeText.y;
            }
        }
    }

    m_textEdit = new QLineEdit(this);
    m_textEdit->setStyleSheet(
        "QLineEdit { background-color: lightyellow; border: 2px solid orange; padding: 5px; font-size: 12px; }"
    );
    m_textEdit->setText(item.text);
    m_textEdit->selectAll();
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
                // 删除文字
                TextItem deletedItem = m_textItems[m_editingTextIndex];
                m_textItems.removeAt(m_editingTextIndex);

                // 同步相对坐标
                if (m_useRelativeCoordinates) {
                    if (m_editingTextIndex < m_relativeTextItems.size()) {
                        m_relativeTextItems.removeAt(m_editingTextIndex);
                    }
                }

                saveAction(ACTION_DELETE_TEXT, QVector<DrawPoint>(), deletedItem, m_editingTextIndex);
            }
            else {
                // 更新文字内容
                m_textItems[m_editingTextIndex].text = text;
                m_textItems[m_editingTextIndex].color = m_penColor;

                // 同步相对坐标
                if (m_useRelativeCoordinates && m_editingTextIndex < m_relativeTextItems.size()) {
                    m_relativeTextItems[m_editingTextIndex].text = text;
                    m_relativeTextItems[m_editingTextIndex].color = m_penColor;
                }

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
                item.font = QFont("Microsoft YaHei", m_fontSize);
                m_textItems.append(item);

                // 同步相对坐标
                if (m_useRelativeCoordinates && m_targetWidget) {
                    QSize currentSize = m_targetWidget->size();
                    RelativeTextItem relativeItem = RelativeTextItem::fromAbsolute(item, currentSize);
                    m_relativeTextItems.append(relativeItem);
                }

                saveAction(ACTION_ADD_TEXT, QVector<DrawPoint>(), item);
            }
        }

        m_editingTextIndex = -1;
        m_hasEditingRelativeText = false;
        update();

        m_textEdit->deleteLater();
        m_textEdit = nullptr;
    }
}

// ============================================================================
// 橡皮擦功能
// ============================================================================

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
        setCursor(createEraserCursor());
        setMouseTracking(true);
        m_currentMousePos = mapFromGlobal(QCursor::pos());
    }
    else {
        setCursor(Qt::ArrowCursor);
        setMouseTracking(false);
    }

    update();
}

void OverlayWidget::changeEraserSize(int size)
{
    m_eraserSize = size;
    if (m_eraserMode) {
        update();
    }
}

void OverlayWidget::performErase(const QPoint& position)
{
    bool hasErased = false;

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
            m_currentErasedData.erasedPathIndices.append(i);
            m_currentErasedData.erasedPaths.append(m_paths[i]);
            m_paths.removeAt(i);

            // 同步删除相对坐标
            if (m_useRelativeCoordinates && i < m_relativePaths.size()) {
                m_relativePaths.removeAt(i);
            }

            hasErased = true;
        }
    }

    // 从后往前遍历文字，这样删除时不会影响索引
    for (int i = m_textItems.size() - 1; i >= 0; --i) {
        const TextItem& textItem = m_textItems[i];

        if (isTextInEraserRange(textItem, position)) {
            m_currentErasedData.erasedTextIndices.append(i);
            m_currentErasedData.erasedTexts.append(textItem);
            m_textItems.removeAt(i);

            // 同步删除相对坐标
            if (m_useRelativeCoordinates && i < m_relativeTextItems.size()) {
                m_relativeTextItems.removeAt(i);
            }

            hasErased = true;
        }
    }

    if (hasErased) {
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
    int cursorSize = 16;
    QPixmap cursorPixmap(cursorSize, cursorSize);
    cursorPixmap.fill(Qt::transparent);

    QPainter painter(&cursorPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制橡皮擦图标
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(QBrush(Qt::white));

    // 绘制橡皮擦主体
    QRect eraserRect(2, 2, 12, 10);
    painter.drawRoundedRect(eraserRect, 1, 1);

    // 绘制橡皮擦上的金属部分
    painter.setBrush(QBrush(Qt::lightGray));
    QRect metalRect(2, 2, 12, 3);
    painter.drawRoundedRect(metalRect, 1, 1);

    return QCursor(cursorPixmap, 8, 8);
}

// ============================================================================
// 工具栏相关
// ============================================================================

void OverlayWidget::changePenColor()
{
    QColor newColor = QColorDialog::getColor(m_penColor, this, tr("选择颜色"));
    if (newColor.isValid()) {
        m_penColor = newColor;
        m_colorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(m_penColor.name()));
    }
}

void OverlayWidget::changePenWidth(int width)
{
    m_penWidth = width;
}

void OverlayWidget::changeFontSize(int size)
{
    m_fontSize = size;
}

void OverlayWidget::toggleTextMode(bool enabled)
{
    // 切换模式时先完成当前的文字输入
    if (m_textEdit && !enabled) {
        finishTextInput();
    }

    m_textMode = enabled;
    m_fontSizeSpinBox->setEnabled(enabled);

    // 如果启用文字模式，禁用橡皮擦模式
    if (enabled && m_eraserMode) {
        m_eraserMode = false;
        m_eraserModeCheckBox->setChecked(false);
        m_eraserSizeSpinBox->setEnabled(false);
        setCursor(Qt::ArrowCursor);
    }
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
        m_collapseButton->setToolTip(tr("展开工具栏"));
    }
    else {
        // 展开状态：显示内容区域
        m_toolbarContent->show();
        m_collapseButton->setText("−");
        m_collapseButton->setToolTip(tr("收起工具栏"));
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

// ============================================================================
// 撤销重做功能
// ============================================================================

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
        if (m_useRelativeCoordinates && !m_relativePaths.isEmpty()) {
            m_relativePaths.removeLast();
        }
        break;

    case ACTION_ADD_TEXT:
        // 撤销添加文字：移除最后添加的文字
        for (int i = m_textItems.size() - 1; i >= 0; --i) {
            const TextItem& item = m_textItems[i];
            if (item.position == lastAction.textData.position &&
                item.text == lastAction.textData.text) {
                m_textItems.removeAt(i);
                if (m_useRelativeCoordinates && i < m_relativeTextItems.size()) {
                    m_relativeTextItems.removeAt(i);
                }
                break;
            }
        }
        break;

    case ACTION_EDIT_TEXT:
        // 撤销编辑文字：恢复原文字内容
        if (lastAction.textIndex >= 0 && lastAction.textIndex < m_textItems.size()) {
            m_textItems[lastAction.textIndex].text = lastAction.oldText;
            m_textItems[lastAction.textIndex].color = lastAction.oldColor;

            if (m_useRelativeCoordinates && lastAction.textIndex < m_relativeTextItems.size()) {
                m_relativeTextItems[lastAction.textIndex].text = lastAction.oldText;
                m_relativeTextItems[lastAction.textIndex].color = lastAction.oldColor;
            }
        }
        break;

    case ACTION_DELETE_TEXT:
        // 撤销删除文字：重新插入文字
        if (lastAction.textIndex >= 0 && lastAction.textIndex <= m_textItems.size()) {
            m_textItems.insert(lastAction.textIndex, lastAction.textData);

            if (m_useRelativeCoordinates && m_targetWidget) {
                QSize currentSize = m_targetWidget->size();
                RelativeTextItem relativeItem = RelativeTextItem::fromAbsolute(lastAction.textData, currentSize);
                if (lastAction.textIndex <= m_relativeTextItems.size()) {
                    m_relativeTextItems.insert(lastAction.textIndex, relativeItem);
                }
            }
        }
        break;

    case ACTION_ERASE:
        // 撤销橡皮擦操作：恢复被擦除的内容
        // TODO: 实现橡皮擦撤销（需要更复杂的数据结构支持）
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
        if (m_useRelativeCoordinates && m_targetWidget) {
            QSize currentSize = m_targetWidget->size();
            QVector<RelativePoint> relativePath;
            for (const auto& point : lastRedoAction.pathData) {
                relativePath.append(RelativePoint::fromAbsolute(point.point, currentSize, point.color, point.width));
            }
            m_relativePaths.append(relativePath);
        }
        break;

    case ACTION_ADD_TEXT:
        // 重做添加文字：重新添加文字
        m_textItems.append(lastRedoAction.textData);
        if (m_useRelativeCoordinates && m_targetWidget) {
            QSize currentSize = m_targetWidget->size();
            RelativeTextItem relativeItem = RelativeTextItem::fromAbsolute(lastRedoAction.textData, currentSize);
            m_relativeTextItems.append(relativeItem);
        }
        break;

    case ACTION_EDIT_TEXT:
        // 重做编辑文字：应用新文字内容
        if (lastRedoAction.textIndex >= 0 && lastRedoAction.textIndex < m_textItems.size()) {
            m_textItems[lastRedoAction.textIndex].text = lastRedoAction.newText;
            m_textItems[lastRedoAction.textIndex].color = lastRedoAction.newColor;

            if (m_useRelativeCoordinates && lastRedoAction.textIndex < m_relativeTextItems.size()) {
                m_relativeTextItems[lastRedoAction.textIndex].text = lastRedoAction.newText;
                m_relativeTextItems[lastRedoAction.textIndex].color = lastRedoAction.newColor;
            }
        }
        break;

    case ACTION_DELETE_TEXT:
        // 重做删除文字：重新删除文字
        if (lastRedoAction.textIndex >= 0 && lastRedoAction.textIndex < m_textItems.size()) {
            m_textItems.removeAt(lastRedoAction.textIndex);
            if (m_useRelativeCoordinates && lastRedoAction.textIndex < m_relativeTextItems.size()) {
                m_relativeTextItems.removeAt(lastRedoAction.textIndex);
            }
        }
        break;

    case ACTION_ERASE:
        // 重做橡皮擦操作：重新执行擦除
        // TODO: 实现橡皮擦重做
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

// ============================================================================
// 主要操作函数
// ============================================================================

void OverlayWidget::showOverlay()
{
    if (!m_targetWidget) return;

    // 确保目标widget可见
    if (!m_targetWidget->isVisible()) {
        qWarning("OverlayWidget: Target widget is not visible");
        return;
    }

    // 计算精确位置
    calculatePreciseGeometry();

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

void OverlayWidget::clearCanvas()
{
    // 先完成正在进行的文字输入
    finishTextInput();

    m_paths.clear();
    m_currentPath.clear();
    m_textItems.clear();

    // 清空相对坐标数据
    if (m_useRelativeCoordinates) {
        m_relativePaths.clear();
        m_currentRelativePath.clear();
        m_relativeTextItems.clear();
    }

    // 清除撤销和重做栈
    m_undoStack.clear();
    m_redoStack.clear();
    updateUndoRedoButtons();

    update();
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
            QMessageBox::information(this, tr("保存成功"), tr("标注图片已保存到: ") + fileName);
        }
        else {
            QMessageBox::warning(this, tr("保存失败"), tr("无法保存图片"));
        }
    }
}

void OverlayWidget::finishEditing()
{
    // 先完成可能正在进行的文字输入
    finishTextInput();
    hideOverlay();
}

// ============================================================================
// 配置接口
// ============================================================================

void OverlayWidget::setUseRelativeCoordinates(bool enabled)
{
    m_useRelativeCoordinates = enabled;
    if (enabled) {
        syncRelativeData();
        if (m_debugMode) {
            qDebug() << "Enabled relative coordinate system";
        }
    }
    else {
        if (m_debugMode) {
            qDebug() << "Disabled relative coordinate system";
        }
    }
}

void OverlayWidget::setDebugMode(bool enabled)
{
    m_debugMode = enabled;
    if (enabled) {
        qDebug() << "OverlayWidget Debug Mode Enabled";
        // 在调试模式下显示边框
        setStyleSheet("border: 2px solid red;");
    }
    else {
        setStyleSheet("");
    }
    update();
}

void OverlayWidget::setHighPrecisionMode(bool enabled)
{
    m_useHighPrecision = enabled;
    if (enabled) {
        m_updateTimer->setInterval(8); // 120fps更新频率
    }
    else {
        m_updateTimer->setInterval(16); // 60fps更新频率
    }
}

void OverlayWidget::setTargetMargins(const QMargins& margins)
{
    m_targetMargins = margins;
    m_rectCacheValid = false;
    updatePosition();
}

// ============================================================================
// 调试和测试功能
// ============================================================================

void OverlayWidget::testScalingAccuracy()
{
    if (!m_targetWidget) return;

    qDebug() << "=== Complete Scaling Accuracy Test ===";

    // 记录当前状态
    QSize originalSize = m_targetWidget->size();
    int originalPathCount = m_paths.size();
    int originalTextCount = m_textItems.size();

    QVector<QPoint> originalFirstPoints;
    QVector<QPoint> originalTextPositions;
    QVector<int> originalFontSizes;

    if (!m_paths.isEmpty() && !m_paths[0].isEmpty()) {
        originalFirstPoints.append(m_paths[0][0].point);
    }

    for (const auto& textItem : m_textItems) {
        originalTextPositions.append(textItem.position);
        originalFontSizes.append(textItem.font.pointSize());
    }

    // 执行极端缩放测试
    QVector<QSize> testSizes = {
        QSize(100, 80),     // 缩小10倍
        QSize(10, 8),       // 缩小100倍
        QSize(1, 1),        // 极端缩小
        QSize(10, 8),       // 回到100倍缩小
        QSize(100, 80),     // 回到10倍缩小
        originalSize,       // 回到原始大小
        QSize(2000, 1600),  // 放大2倍
        originalSize        // 回到原始
    };

    for (const QSize& testSize : testSizes) {
        m_targetWidget->resize(testSize);
        updatePosition();  // 触发缩放

        qDebug() << "Size:" << testSize
            << "Paths:" << m_paths.size()
            << "Texts:" << m_textItems.size();

        if (!m_paths.isEmpty() && !m_paths[0].isEmpty()) {
            qDebug() << "First draw point:" << m_paths[0][0].point;
        }

        if (!m_textItems.isEmpty()) {
            qDebug() << "First text position:" << m_textItems[0].position
                << "font size:" << m_textItems[0].font.pointSize();
        }
    }

    // 验证最终结果
    bool testPassed = true;

    // 验证数量
    if (m_paths.size() != originalPathCount) {
        qWarning() << "Path count changed!";
        testPassed = false;
    }

    if (m_textItems.size() != originalTextCount) {
        qWarning() << "Text count changed!";
        testPassed = false;
    }

    // 验证绘制点精度
    if (!originalFirstPoints.isEmpty() && !m_paths.isEmpty() && !m_paths[0].isEmpty()) {
        QPoint finalPoint = m_paths[0][0].point;
        QPoint originalPoint = originalFirstPoints[0];
        int distance = (finalPoint - originalPoint).manhattanLength();

        if (distance > 2) {
            qWarning() << "Draw point accuracy failed! Original:" << originalPoint
                << "Final:" << finalPoint << "Distance:" << distance;
            testPassed = false;
        }
        else {
            qDebug() << "Draw point accuracy test PASSED! Distance:" << distance;
        }
    }

    // 验证文字位置精度
    for (int i = 0; i < qMin(originalTextPositions.size(), m_textItems.size()); ++i) {
        QPoint originalPos = originalTextPositions[i];
        QPoint finalPos = m_textItems[i].position;
        int distance = (finalPos - originalPos).manhattanLength();

        if (distance > 2) {
            qWarning() << "Text position accuracy failed at index" << i
                << "Original:" << originalPos << "Final:" << finalPos
                << "Distance:" << distance;
            testPassed = false;
        }
        else {
            qDebug() << "Text position" << i << "accuracy test PASSED! Distance:" << distance;
        }
    }

    // 验证字体大小精度
    for (int i = 0; i < qMin(originalFontSizes.size(), m_textItems.size()); ++i) {
        int originalSize = originalFontSizes[i];
        int finalSize = m_textItems[i].font.pointSize();
        int difference = qAbs(finalSize - originalSize);

        if (difference > 1) {
            qWarning() << "Font size accuracy failed at index" << i
                << "Original:" << originalSize << "Final:" << finalSize
                << "Difference:" << difference;
            testPassed = false;
        }
        else {
            qDebug() << "Font size" << i << "accuracy test PASSED! Difference:" << difference;
        }
    }

    qDebug() << "Overall test result:" << (testPassed ? "PASSED" : "FAILED");
    qDebug() << "Relative coordinate mode:" << (m_useRelativeCoordinates ? "ENABLED" : "DISABLED");
}

void OverlayWidget::debugRelativeCoordinates() const
{
    if (!m_debugMode) return;

    qDebug() << "=== Relative Coordinates Debug ===";
    qDebug() << "Base size:" << m_baseSize;
    qDebug() << "Current size:" << (m_targetWidget ? m_targetWidget->size() : QSize());
    qDebug() << "Relative paths:" << m_relativePaths.size();
    qDebug() << "Relative texts:" << m_relativeTextItems.size();
    qDebug() << "Absolute paths:" << m_paths.size();
    qDebug() << "Absolute texts:" << m_textItems.size();

    for (int i = 0; i < m_relativeTextItems.size(); ++i) {
        const auto& item = m_relativeTextItems[i];
        qDebug() << QString("Text %1: (%2, %3) size:%4 '%5'")
            .arg(i)
            .arg(item.x, 0, 'f', 4)
            .arg(item.y, 0, 'f', 4)
            .arg(item.relativeFontSize, 0, 'f', 4)
            .arg(item.text);
    }
}

void OverlayWidget::validateCoordinateConsistency()
{
    if (!m_debugMode || !m_useRelativeCoordinates) return;

    // 验证绝对坐标和相对坐标的一致性
    QSize currentSize = m_targetWidget ? m_targetWidget->size() : size();

    if (currentSize.isEmpty()) return;

    bool hasInconsistency = false;

    // 验证路径
    for (int i = 0; i < qMin(m_paths.size(), m_relativePaths.size()); ++i) {
        const auto& absPath = m_paths[i];
        const auto& relPath = m_relativePaths[i];

        if (absPath.size() != relPath.size()) {
            qWarning() << "Path size mismatch at index" << i;
            hasInconsistency = true;
            continue;
        }

        for (int j = 0; j < absPath.size(); ++j) {
            QPoint expectedAbs = relPath[j].toAbsolute(currentSize);
            QPoint actualAbs = absPath[j].point;

            if ((expectedAbs - actualAbs).manhattanLength() > 2) {
                qWarning() << "Path coordinate mismatch at" << i << j
                    << "expected" << expectedAbs << "actual" << actualAbs;
                hasInconsistency = true;
            }
        }
    }

    // 验证文字
    for (int i = 0; i < qMin(m_textItems.size(), m_relativeTextItems.size()); ++i) {
        const auto& absText = m_textItems[i];
        const auto& relText = m_relativeTextItems[i];

        QPoint expectedPos = relText.toAbsolutePosition(currentSize);
        QFont expectedFont = relText.toAbsoluteFont(currentSize);

        if ((expectedPos - absText.position).manhattanLength() > 2) {
            qWarning() << "Text position mismatch at index" << i
                << "expected" << expectedPos << "actual" << absText.position;
            hasInconsistency = true;
        }

        if (qAbs(expectedFont.pointSize() - absText.font.pointSize()) > 1) {
            qWarning() << "Font size mismatch at index" << i
                << "expected" << expectedFont.pointSize()
                << "actual" << absText.font.pointSize();
            hasInconsistency = true;
        }

        if (relText.text != absText.text) {
            qWarning() << "Text content mismatch at index" << i;
            hasInconsistency = true;
        }
    }

    if (!hasInconsistency) {
        qDebug() << "Coordinate consistency validation PASSED";
    }
}

bool OverlayWidget::isValidPosition(const QPoint& pos) const
{
    if (!m_targetWidget) return false;

    QRect validRect(QPoint(0, 0), m_targetWidget->size());
    validRect = validRect.marginsRemoved(m_targetMargins);

    return validRect.contains(pos);
}