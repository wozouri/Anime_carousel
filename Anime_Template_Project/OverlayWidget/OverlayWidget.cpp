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
#include <QJsonParseError>
#include <QElapsedTimer>


// ============================================================================
// 性能监控和优化
// ============================================================================

class PerformanceMonitor
{
public:
    static PerformanceMonitor& instance() {
        static PerformanceMonitor instance;
        return instance;
    }

    void startTimer(const QString& operation) {
        timers[operation] = QElapsedTimer();
        timers[operation].start();
    }

    void endTimer(const QString& operation) {
        if (timers.contains(operation)) {
            qint64 elapsed = timers[operation].elapsed();
            addSample(operation, elapsed);
            timers.remove(operation);

            if (elapsed > 16) {  // 超过16ms（60fps阈值）
                qDebug() << "Performance warning:" << operation << "took" << elapsed << "ms";
            }
        }
    }

    void addSample(const QString& operation, qint64 timeMs) {
        samples[operation].append(timeMs);
        if (samples[operation].size() > 100) {
            samples[operation].removeFirst();  // 保持最近100个样本
        }
    }

    double getAverageTime(const QString& operation) const {
        if (!samples.contains(operation) || samples[operation].isEmpty()) {
            return 0.0;
        }

        qint64 total = 0;
        for (qint64 time : samples[operation]) {
            total += time;
        }
        return static_cast<double>(total) / samples[operation].size();
    }

    void printStatistics() const {
        qDebug() << "=== Performance Statistics ===";
        for (auto it = samples.begin(); it != samples.end(); ++it) {
            qDebug() << it.key() << ": avg" << getAverageTime(it.key()) << "ms";
        }
    }

private:
    QMap<QString, QElapsedTimer> timers;
    QMap<QString, QVector<qint64>> samples;
};

#define PERF_START(op) PerformanceMonitor::instance().startTimer(op)
#define PERF_END(op) PerformanceMonitor::instance().endTimer(op)



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
    // 新增几何图形相关初始化
    , m_currentTool(TOOL_FREE_DRAW)
    , m_fillMode(false)
    , m_arrowSize(10)
    , m_drawingShape(false)
    , m_toolButtonGroup(nullptr)
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

    m_relativeShapes.clear();
    for (const auto& shape : m_shapes) {
        m_relativeShapes.append(RelativeShapeItem::fromAbsolute(shape, m_baseSize));
    }

    if (m_debugMode) {
        qDebug() << "Converted to relative coordinates:" << m_relativePaths.size()
            << "paths," << m_relativeTextItems.size() << "texts,"
            << m_relativeShapes.size() << "shapes";
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
            "QLineEdit { background-color: white; border: 2px solid %1; "
            "padding: 5px; font-size: %2px; }").arg(m_penColor.name()).arg(newFont.pointSize());
        m_textEdit->setStyleSheet(styleSheet);
    }

    // 重建几何图形
    m_shapes.clear();
    for (const auto& relativeShape : m_relativeShapes) {
        m_shapes.append(relativeShape.toAbsolute(currentSize));
    }

    if (m_debugMode) {
        qDebug() << "Updated absolute coordinates for size:" << currentSize
            << "- shapes:" << m_shapes.size();
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
    return;


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
    // =============================================================================
    // 创建主工具栏容器
    // =============================================================================
    m_toolbar = new QWidget(this);
    m_toolbar->setStyleSheet(
        "QWidget#toolbar { background-color: rgba(50, 50, 50, 230); border-radius: 8px; "
        "border: 1px solid rgba(100, 100, 100, 100); }"
    );
    m_toolbar->setObjectName("toolbar");

    QVBoxLayout* toolbarLayout = new QVBoxLayout(m_toolbar);
    toolbarLayout->setSpacing(0);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);

    // =============================================================================
    // 设置工具栏标题栏（拖动区域）
    // =============================================================================
    setupToolbarHeader();

    // =============================================================================
    // 创建工具栏内容区域
    // =============================================================================
    m_toolbarContent = new QWidget(m_toolbar);
    m_toolbarContent->setStyleSheet(
        "QWidget { background-color: rgba(50, 50, 50, 230); border-radius: 0 0 6px 6px; }"
        "QPushButton { color: white; border: 1px solid gray; padding: 4px 6px; margin: 1px; "
        "border-radius: 3px; font-size: 11px; }"
        "QPushButton:hover { background-color: rgba(100, 100, 100, 150); }"
        "QPushButton:checked { background-color: rgba(0, 120, 212, 200); "
        "border-color: rgba(0, 120, 212, 255); }"
        "QSpinBox { color: white; background-color: rgba(70, 70, 70, 200); "
        "border: 1px solid gray; padding: 2px; }"
        "QCheckBox { color: white; font-size: 11px; }"
        "QLabel { color: white; font-size: 11px; }"
        "QComboBox { color: white; background-color: rgba(70, 70, 70, 200); "
        "border: 1px solid gray; padding: 2px; }"
    );

    QVBoxLayout* contentLayout = new QVBoxLayout(m_toolbarContent);
    contentLayout->setSpacing(3);
    contentLayout->setContentsMargins(6, 4, 6, 6);

    // =============================================================================
    // 第一行：工具选择按钮
    // =============================================================================
    setupToolButtons();
    contentLayout->addWidget(createToolButtonsWidget());

    // =============================================================================
    // 第二行：属性控制
    // =============================================================================
    setupAttributeControls();
    contentLayout->addLayout(createAttributeControlsLayout());

    // =============================================================================
    // 第三行：基础操作按钮
    // =============================================================================
    setupActionButtons();
    contentLayout->addLayout(createActionButtonsLayout());

    // =============================================================================
    // 第四行：高级功能按钮（新增）
    // =============================================================================
    setupAdvancedControls();
    contentLayout->addLayout(createAdvancedControlsLayout());

    // =============================================================================
    // 第五行：调试功能（可选显示）
    // =============================================================================
    if (m_debugMode) {
        setupDebugControls();
        contentLayout->addLayout(createDebugControlsLayout());
    }

    // =============================================================================
    // 添加到主工具栏布局
    // =============================================================================
    toolbarLayout->addWidget(m_toolbarHeader);
    toolbarLayout->addWidget(m_toolbarContent);

    // 定位工具栏到初始位置
    m_toolbar->move(10, 10);
    updateToolbarLayout();

    // 应用当前样式
    applyCurrentStyle();
}

void OverlayWidget::setupToolbarHeader()
{
    m_toolbarHeader = new QWidget(m_toolbar);
    m_toolbarHeader->setFixedHeight(30);
    m_toolbarHeader->setStyleSheet(
        "QWidget { background-color: rgba(70, 70, 70, 200); border-radius: 6px 6px 0 0; }"
        "QPushButton { color: white; border: none; padding: 4px; font-size: 12px; "
        "background: transparent; }"
        "QPushButton:hover { background-color: rgba(100, 100, 100, 100); }"
        "QLabel { color: white; font-size: 12px; font-weight: bold; }"
    );
    m_toolbarHeader->setCursor(Qt::SizeAllCursor);

    QHBoxLayout* headerLayout = new QHBoxLayout(m_toolbarHeader);
    headerLayout->setContentsMargins(8, 4, 4, 4);

    // 标题标签
    QLabel* titleLabel = new QLabel(tr("增强标注工具"), m_toolbarHeader);
    titleLabel->setCursor(Qt::SizeAllCursor);

    // 收起/展开按钮
    m_collapseButton = new QPushButton("−", m_toolbarHeader);
    m_collapseButton->setFixedSize(20, 20);
    m_collapseButton->setToolTip(tr("收起工具栏"));
    connect(m_collapseButton, &QPushButton::clicked, this, &OverlayWidget::toggleToolbarCollapse);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_collapseButton);
}

void OverlayWidget::setupToolButtons()
{
    // 创建工具按钮组
    m_toolButtonGroup = new QButtonGroup(this);

    // 自由绘制
    m_freeDrawButton = new QPushButton("✏️", nullptr);
    m_freeDrawButton->setFixedSize(30, 24);
    m_freeDrawButton->setCheckable(true);
    m_freeDrawButton->setChecked(true);
    m_freeDrawButton->setToolTip(tr("自由绘制 (P)"));
    m_toolButtonGroup->addButton(m_freeDrawButton, TOOL_FREE_DRAW);

    // 直线
    m_lineButton = new QPushButton("📏", nullptr);
    m_lineButton->setFixedSize(30, 24);
    m_lineButton->setCheckable(true);
    m_lineButton->setToolTip(tr("直线 (L)"));
    m_toolButtonGroup->addButton(m_lineButton, TOOL_LINE);

    // 矩形
    m_rectangleButton = new QPushButton("⬜", nullptr);
    m_rectangleButton->setFixedSize(30, 24);
    m_rectangleButton->setCheckable(true);
    m_rectangleButton->setToolTip(tr("矩形 (R)"));
    m_toolButtonGroup->addButton(m_rectangleButton, TOOL_RECTANGLE);

    // 椭圆
    m_ellipseButton = new QPushButton("⭕", nullptr);
    m_ellipseButton->setFixedSize(30, 24);
    m_ellipseButton->setCheckable(true);
    m_ellipseButton->setToolTip(tr("椭圆 (O)"));
    m_toolButtonGroup->addButton(m_ellipseButton, TOOL_ELLIPSE);

    // 箭头
    m_arrowButton = new QPushButton("➡️", nullptr);
    m_arrowButton->setFixedSize(30, 24);
    m_arrowButton->setCheckable(true);
    m_arrowButton->setToolTip(tr("箭头 (A)"));
    m_toolButtonGroup->addButton(m_arrowButton, TOOL_ARROW);

    // 文字
    m_textButton = new QPushButton("📝", nullptr);
    m_textButton->setFixedSize(30, 24);
    m_textButton->setCheckable(true);
    m_textButton->setToolTip(tr("文字 (T)"));
    m_toolButtonGroup->addButton(m_textButton, TOOL_TEXT);

    // 橡皮擦
    m_eraserButton = new QPushButton("🧽", nullptr);
    m_eraserButton->setFixedSize(30, 24);
    m_eraserButton->setCheckable(true);
    m_eraserButton->setToolTip(tr("橡皮擦 (E)"));
    m_toolButtonGroup->addButton(m_eraserButton, TOOL_ERASER);

    // 连接信号
    connect(m_toolButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
        this, &OverlayWidget::onToolButtonClicked);
}



QWidget* OverlayWidget::createToolButtonsWidget()
{
    QWidget* toolsWidget = new QWidget();
    QHBoxLayout* toolsLayout = new QHBoxLayout(toolsWidget);
    toolsLayout->setSpacing(2);
    toolsLayout->setContentsMargins(0, 0, 0, 0);

    // 添加工具提示标签
    QLabel* toolsLabel = new QLabel(tr("工具:"));
    toolsLabel->setStyleSheet("color: white; font-size: 10px; font-weight: bold;");
    toolsLayout->addWidget(toolsLabel);

    toolsLayout->addWidget(m_freeDrawButton);
    toolsLayout->addWidget(m_lineButton);
    toolsLayout->addWidget(m_rectangleButton);
    toolsLayout->addWidget(m_ellipseButton);
    toolsLayout->addWidget(m_arrowButton);
    toolsLayout->addWidget(m_textButton);
    toolsLayout->addWidget(m_eraserButton);
    toolsLayout->addStretch();

    return toolsWidget;
}

void OverlayWidget::setupAttributeControls()
{
    // 颜色选择按钮
    m_colorButton = new QPushButton(tr("颜色"));
    m_colorButton->setFixedSize(45, 24);
    m_colorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(m_penColor.name()));
    m_colorButton->setToolTip(tr("选择颜色"));
    connect(m_colorButton, &QPushButton::clicked, this, &OverlayWidget::changePenColor);

    // 画笔宽度
    m_widthSpinBox = new QSpinBox();
    m_widthSpinBox->setRange(1, 20);
    m_widthSpinBox->setValue(m_penWidth);
    m_widthSpinBox->setFixedSize(50, 24);
    m_widthSpinBox->setToolTip(tr("画笔宽度 ([/])"));
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &OverlayWidget::changePenWidth);

    // 填充模式
    m_fillModeCheckBox = new QCheckBox(tr("填充"));
    m_fillModeCheckBox->setToolTip(tr("填充模式 (F)"));
    connect(m_fillModeCheckBox, &QCheckBox::toggled, this, &OverlayWidget::toggleFillMode);

    // 箭头大小
    m_arrowSizeSpinBox = new QSpinBox();
    m_arrowSizeSpinBox->setRange(5, 30);
    m_arrowSizeSpinBox->setValue(m_arrowSize);
    m_arrowSizeSpinBox->setFixedSize(50, 24);
    m_arrowSizeSpinBox->setEnabled(false);
    m_arrowSizeSpinBox->setToolTip(tr("箭头大小"));
    connect(m_arrowSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &OverlayWidget::changeArrowSize);

    // 文字字体大小
    m_fontSizeSpinBox = new QSpinBox();
    m_fontSizeSpinBox->setRange(8, 72);
    m_fontSizeSpinBox->setValue(m_fontSize);
    m_fontSizeSpinBox->setFixedSize(50, 24);
    m_fontSizeSpinBox->setEnabled(false);
    m_fontSizeSpinBox->setToolTip(tr("字体大小"));
    connect(m_fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &OverlayWidget::changeFontSize);

    // 橡皮擦大小
    m_eraserSizeSpinBox = new QSpinBox();
    m_eraserSizeSpinBox->setRange(10, 80);
    m_eraserSizeSpinBox->setValue(m_eraserSize);
    m_eraserSizeSpinBox->setFixedSize(50, 24);
    m_eraserSizeSpinBox->setEnabled(false);
    m_eraserSizeSpinBox->setToolTip(tr("橡皮擦大小"));
    connect(m_eraserSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &OverlayWidget::changeEraserSize);
}

QHBoxLayout* OverlayWidget::createAttributeControlsLayout()
{
    QHBoxLayout* attributesLayout = new QHBoxLayout();
    attributesLayout->setSpacing(4);

    // 添加属性提示标签
    QLabel* attrLabel = new QLabel(tr("属性:"));
    attrLabel->setStyleSheet("color: white; font-size: 10px; font-weight: bold;");
    attributesLayout->addWidget(attrLabel);

    attributesLayout->addWidget(m_colorButton);

    QLabel* widthLabel = new QLabel(tr("宽度:"));
    attributesLayout->addWidget(widthLabel);
    attributesLayout->addWidget(m_widthSpinBox);

    attributesLayout->addWidget(m_fillModeCheckBox);

    QLabel* arrowLabel = new QLabel(tr("箭头:"));
    attributesLayout->addWidget(arrowLabel);
    attributesLayout->addWidget(m_arrowSizeSpinBox);

    QLabel* fontLabel = new QLabel(tr("字号:"));
    attributesLayout->addWidget(fontLabel);
    attributesLayout->addWidget(m_fontSizeSpinBox);

    QLabel* eraserLabel = new QLabel(tr("擦除:"));
    attributesLayout->addWidget(eraserLabel);
    attributesLayout->addWidget(m_eraserSizeSpinBox);

    attributesLayout->addStretch();

    return attributesLayout;
}

void OverlayWidget::setupActionButtons()
{
    // 撤销按钮
    m_undoButton = new QPushButton(tr("撤销"));
    m_undoButton->setFixedSize(40, 24);
    m_undoButton->setEnabled(false);
    m_undoButton->setToolTip(tr("撤销 (Ctrl+Z)"));
    connect(m_undoButton, &QPushButton::clicked, this, &OverlayWidget::onUndoClicked);

    // 重做按钮
    m_redoButton = new QPushButton(tr("重做"));
    m_redoButton->setFixedSize(40, 24);
    m_redoButton->setEnabled(false);
    m_redoButton->setToolTip(tr("重做 (Ctrl+Y)"));
    connect(m_redoButton, &QPushButton::clicked, this, &OverlayWidget::onRedoClicked);

    // 清除按钮
    m_clearButton = new QPushButton(tr("清除"));
    m_clearButton->setFixedSize(40, 24);
    m_clearButton->setToolTip(tr("清除所有 (Delete)"));
    connect(m_clearButton, &QPushButton::clicked, this, &OverlayWidget::onClearClicked);

    // 保存按钮
    m_saveButton = new QPushButton(tr("保存"));
    m_saveButton->setFixedSize(40, 24);
    m_saveButton->setToolTip(tr("保存图片 (Ctrl+S)"));
    connect(m_saveButton, &QPushButton::clicked, this, &OverlayWidget::onSaveClicked);

    // 完成按钮
    m_finishButton = new QPushButton(tr("完成"));
    m_finishButton->setFixedSize(40, 24);
    m_finishButton->setToolTip(tr("完成标注 (ESC)"));
    connect(m_finishButton, &QPushButton::clicked, this, &OverlayWidget::onFinishClicked);
}

QHBoxLayout* OverlayWidget::createActionButtonsLayout()
{
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(4);

    // 添加操作提示标签
    QLabel* actionLabel = new QLabel(tr("操作:"));
    actionLabel->setStyleSheet("color: white; font-size: 10px; font-weight: bold;");
    actionLayout->addWidget(actionLabel);

    actionLayout->addWidget(m_undoButton);
    actionLayout->addWidget(m_redoButton);
    actionLayout->addStretch();
    actionLayout->addWidget(m_clearButton);
    actionLayout->addWidget(m_saveButton);
    actionLayout->addWidget(m_finishButton);

    return actionLayout;
}

void OverlayWidget::setupAdvancedControls()
{
    // 导入按钮
    m_importButton = new QPushButton(tr("导入"));
    m_importButton->setFixedSize(40, 24);
    m_importButton->setToolTip(tr("导入标注数据"));
    connect(m_importButton, &QPushButton::clicked, this, &OverlayWidget::onImportClicked);

    // 导出按钮
    m_exportButton = new QPushButton(tr("导出"));
    m_exportButton->setFixedSize(40, 24);
    m_exportButton->setToolTip(tr("导出标注数据"));
    connect(m_exportButton, &QPushButton::clicked, this, &OverlayWidget::onExportClicked);

    // 配置保存按钮
    m_configSaveButton = new QPushButton(tr("存配置"));
    m_configSaveButton->setFixedSize(50, 24);
    m_configSaveButton->setToolTip(tr("保存当前配置"));
    connect(m_configSaveButton, &QPushButton::clicked, this, &OverlayWidget::onConfigSaveClicked);

    // 配置加载按钮
    m_configLoadButton = new QPushButton(tr("读配置"));
    m_configLoadButton->setFixedSize(50, 24);
    m_configLoadButton->setToolTip(tr("加载配置"));
    connect(m_configLoadButton, &QPushButton::clicked, this, &OverlayWidget::onConfigLoadClicked);

    // 主题选择
    m_themeComboBox = new QComboBox();
    m_themeComboBox->addItem(tr("深色主题"), OverlayStyleManager::THEME_DARK);
    m_themeComboBox->addItem(tr("浅色主题"), OverlayStyleManager::THEME_LIGHT);
    m_themeComboBox->addItem(tr("蓝色主题"), OverlayStyleManager::THEME_BLUE);
    m_themeComboBox->addItem(tr("绿色主题"), OverlayStyleManager::THEME_GREEN);
    m_themeComboBox->setFixedSize(80, 24);
    m_themeComboBox->setToolTip(tr("选择主题"));
    connect(m_themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &OverlayWidget::onThemeChanged);

    // 帮助按钮
    m_helpButton = new QPushButton("❓");
    m_helpButton->setFixedSize(24, 24);
    m_helpButton->setToolTip(tr("显示快捷键帮助"));
    connect(m_helpButton, &QPushButton::clicked, this, &OverlayWidget::onShowHelpClicked);

    // 关于按钮
    m_aboutButton = new QPushButton("ℹ️");
    m_aboutButton->setFixedSize(24, 24);
    m_aboutButton->setToolTip(tr("关于此工具"));
    connect(m_aboutButton, &QPushButton::clicked, this, &OverlayWidget::onShowAboutClicked);
}

QHBoxLayout* OverlayWidget::createAdvancedControlsLayout()
{
    QHBoxLayout* advancedLayout = new QHBoxLayout();
    advancedLayout->setSpacing(4);

    // 添加高级功能提示标签
    QLabel* advancedLabel = new QLabel(tr("高级:"));
    advancedLabel->setStyleSheet("color: white; font-size: 10px; font-weight: bold;");
    advancedLayout->addWidget(advancedLabel);

    advancedLayout->addWidget(m_importButton);
    advancedLayout->addWidget(m_exportButton);
    advancedLayout->addWidget(m_configSaveButton);
    advancedLayout->addWidget(m_configLoadButton);

    QLabel* themeLabel = new QLabel(tr("主题:"));
    advancedLayout->addWidget(themeLabel);
    advancedLayout->addWidget(m_themeComboBox);

    advancedLayout->addStretch();
    advancedLayout->addWidget(m_helpButton);
    advancedLayout->addWidget(m_aboutButton);

    return advancedLayout;
}

void OverlayWidget::setupDebugControls()
{
    // 调试模式复选框
    m_debugModeCheckBox = new QCheckBox(tr("调试模式"));
    m_debugModeCheckBox->setChecked(m_debugMode);
    m_debugModeCheckBox->setToolTip(tr("启用调试信息显示"));
    connect(m_debugModeCheckBox, &QCheckBox::toggled, this, &OverlayWidget::onDebugModeToggled);

    // 测试缩放按钮
    m_testScalingButton = new QPushButton(tr("测试缩放"));
    m_testScalingButton->setFixedSize(60, 24);
    m_testScalingButton->setToolTip(tr("测试缩放精度"));
    connect(m_testScalingButton, &QPushButton::clicked, this, &OverlayWidget::onTestScalingClicked);

    // 性能统计按钮
    m_performanceStatsButton = new QPushButton(tr("性能统计"));
    m_performanceStatsButton->setFixedSize(60, 24);
    m_performanceStatsButton->setToolTip(tr("显示性能统计信息"));
    connect(m_performanceStatsButton, &QPushButton::clicked, this, &OverlayWidget::onShowPerformanceStats);
}

QHBoxLayout* OverlayWidget::createDebugControlsLayout()
{
    QHBoxLayout* debugLayout = new QHBoxLayout();
    debugLayout->setSpacing(4);

    // 添加调试功能提示标签
    QLabel* debugLabel = new QLabel(tr("调试:"));
    debugLabel->setStyleSheet("color: yellow; font-size: 10px; font-weight: bold;");
    debugLayout->addWidget(debugLabel);

    debugLayout->addWidget(m_debugModeCheckBox);
    debugLayout->addWidget(m_testScalingButton);
    debugLayout->addWidget(m_performanceStatsButton);
    debugLayout->addStretch();

    return debugLayout;
}



void OverlayWidget::setDrawingTool(int toolType)
{
    // 先完成正在进行的操作
    if (m_textEdit) {
        finishTextInput();
    }

    m_currentTool = static_cast<DrawingTool>(toolType);

    // 重置各种模式状态
    m_drawing = false;
    m_drawingShape = false;
    m_textMode = false;
    m_eraserMode = false;

    // 根据工具类型设置状态
    switch (m_currentTool) {
    case TOOL_TEXT:
        m_textMode = true;
        setCursor(Qt::IBeamCursor);
        setMouseTracking(false);
        break;
    case TOOL_ERASER:
        m_eraserMode = true;
        setCursor(createEraserCursor());
        setMouseTracking(true);
        break;
    default:
        setCursor(Qt::CrossCursor);
        setMouseTracking(false);
        break;
    }

    updateToolButtonStates();
    update();
}

void OverlayWidget::toggleFillMode(bool enabled)
{
    m_fillMode = enabled;
}

void OverlayWidget::changeArrowSize(int size)
{
    m_arrowSize = size;
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
        painter.fillRect(rect(), QColor(255, 0, 0, 30));
    }
    else {
        painter.fillRect(rect(), QColor(0, 0, 0, 10));
    }

    // 绘制所有内容
    drawPaths(painter);
    drawTexts(painter);
    drawShapes(painter);  // 新增：绘制几何图形
    drawPreviewShape(painter);  // 新增：绘制预览图形

    // 绘制橡皮擦光标
    drawEraserCursor(painter);

    // 调试模式下绘制额外信息
    if (m_debugMode) {
        drawDebugInfo(painter);
    }
    PERF_START("paintEvent");

    Q_UNUSED(event)
    //QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    PERF_START("drawBackground");
    if (m_debugMode) {
        painter.fillRect(rect(), QColor(255, 0, 0, 30));
    }
    else {
        painter.fillRect(rect(), QColor(0, 0, 0, 10));
    }
    PERF_END("drawBackground");

    // 绘制内容
    PERF_START("drawContent");
    drawPaths(painter);
    drawTexts(painter);
    drawShapes(painter);
    drawPreviewShape(painter);
    PERF_END("drawContent");

    // 绘制UI元素
    PERF_START("drawUI");
    drawEraserCursor(painter);
    if (m_debugMode) {
        drawDebugInfo(painter);
    }
    PERF_END("drawUI");

    PERF_END("paintEvent");

}

void OverlayWidget::drawShapes(QPainter& painter)
{
    for (const auto& shape : m_shapes) {
        switch (shape.type) {
        case SHAPE_LINE:
            drawLine(painter, shape);
            break;
        case SHAPE_RECTANGLE:
            drawRectangle(painter, shape);
            break;
        case SHAPE_ELLIPSE:
            drawEllipse(painter, shape);
            break;
        case SHAPE_ARROW:
            drawArrow(painter, shape);
            break;
        }
    }
}

void OverlayWidget::drawPreviewShape(QPainter& painter)
{
    if (!m_drawingShape) return;

    // 设置预览样式（半透明）
    QPen pen(m_currentShape.color, m_currentShape.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);

    if (m_currentShape.filled) {
        QColor fillColor = m_currentShape.color;
        fillColor.setAlpha(50);
        painter.setBrush(fillColor);
    }
    else {
        painter.setBrush(Qt::NoBrush);
    }

    // 绘制预览图形
    switch (m_currentShape.type) {
    case SHAPE_LINE:
        drawLine(painter, m_currentShape);
        break;
    case SHAPE_RECTANGLE:
        drawRectangle(painter, m_currentShape);
        break;
    case SHAPE_ELLIPSE:
        drawEllipse(painter, m_currentShape);
        break;
    case SHAPE_ARROW:
        drawArrow(painter, m_currentShape);
        break;
    }
}

void OverlayWidget::drawLine(QPainter& painter, const ShapeItem& shape)
{
    QPen pen(shape.color, shape.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(shape.startPoint, shape.endPoint);
}

void OverlayWidget::drawRectangle(QPainter& painter, const ShapeItem& shape)
{
    QPen pen(shape.color, shape.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);

    if (shape.filled) {
        QColor fillColor = shape.color;
        fillColor.setAlpha(100);
        painter.setBrush(fillColor);
    }
    else {
        painter.setBrush(Qt::NoBrush);
    }

    QRect rect(shape.startPoint, shape.endPoint);
    painter.drawRect(rect.normalized());
}

void OverlayWidget::drawEllipse(QPainter& painter, const ShapeItem& shape)
{
    QPen pen(shape.color, shape.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);

    if (shape.filled) {
        QColor fillColor = shape.color;
        fillColor.setAlpha(100);
        painter.setBrush(fillColor);
    }
    else {
        painter.setBrush(Qt::NoBrush);
    }

    QRect rect(shape.startPoint, shape.endPoint);
    painter.drawEllipse(rect.normalized());
}

void OverlayWidget::drawArrow(QPainter& painter, const ShapeItem& shape)
{
    QPen pen(shape.color, shape.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(shape.color);

    // 绘制箭头主体线条
    painter.drawLine(shape.startPoint, shape.endPoint);

    // 绘制箭头头部
    QPolygonF arrowHead = createArrowHead(shape.startPoint, shape.endPoint, shape.arrowSize);
    painter.drawPolygon(arrowHead);
}

QPolygonF OverlayWidget::createArrowHead(const QPoint& start, const QPoint& end, int size)
{
    QPolygonF arrowHead;

    // 计算箭头方向
    QVector2D direction(end - start);
    if (direction.length() == 0) return arrowHead;

    direction.normalize();

    // 计算箭头的两个侧翼点
    QVector2D perpendicular(-direction.y(), direction.x());

    QPointF tip = end;
    QPointF base = end - direction.toPointF() * size;
    QPointF left = base + perpendicular.toPointF() * (size * 0.5);
    QPointF right = base - perpendicular.toPointF() * (size * 0.5);

    arrowHead << tip << left << right;
    return arrowHead;
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
        // 检查是否点击在工具栏上
        if (m_toolbarHeader && m_toolbarHeader->geometry().translated(m_toolbar->pos()).contains(event->pos())) {
            m_draggingToolbar = true;
            m_dragStartPos = event->pos();
            m_toolbarDragOffset = event->pos() - m_toolbar->pos();
            return;
        }

        if (m_toolbar->geometry().contains(event->pos())) {
            return;
        }

        // 根据当前工具执行对应操作
        switch (m_currentTool) {
        case TOOL_FREE_DRAW:
        {
            // 自由绘制模式
            m_drawing = true;
            if (m_useRelativeCoordinates) {
                m_currentRelativePath.clear();
                RelativePoint relativePoint = pointToRelative(event->pos());
                m_currentRelativePath.append(relativePoint);
            }
            m_currentPath.clear();
            DrawPoint point;
            point.point = event->pos();
            point.color = m_penColor;
            point.width = m_penWidth;
            m_currentPath.append(point);
            break;
        }


        case TOOL_LINE:
        case TOOL_RECTANGLE:
        case TOOL_ELLIPSE:
        case TOOL_ARROW:
            // 几何图形模式
            m_drawingShape = true;
            m_shapeStartPoint = event->pos();
            m_currentShape = ShapeItem();
            m_currentShape.startPoint = event->pos();
            m_currentShape.endPoint = event->pos();
            m_currentShape.color = m_penColor;
            m_currentShape.width = m_penWidth;
            m_currentShape.filled = m_fillMode;
            m_currentShape.arrowSize = m_arrowSize;

            // 设置几何图形类型
            switch (m_currentTool) {
            case TOOL_LINE: m_currentShape.type = SHAPE_LINE; break;
            case TOOL_RECTANGLE: m_currentShape.type = SHAPE_RECTANGLE; break;
            case TOOL_ELLIPSE: m_currentShape.type = SHAPE_ELLIPSE; break;
            case TOOL_ARROW: m_currentShape.type = SHAPE_ARROW; break;
            default: break;
            }
            break;

        case TOOL_TEXT:
            // 文字模式
            handleTextClick(event->pos());
            break;

        case TOOL_ERASER:
            // 橡皮擦模式
            m_erasing = true;
            m_lastErasePos = event->pos();
            m_currentErasedData = ErasedData();
            performErase(event->pos());
            break;
        }

        update();
    }
}


void OverlayWidget::mouseMoveEvent(QMouseEvent* event)
{
    // 在橡皮擦模式下，始终更新鼠标位置
    if (m_eraserMode) {
        m_currentMousePos = event->pos();
        update();
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
            // 自由绘制
            if (m_useRelativeCoordinates) {
                RelativePoint relativePoint = pointToRelative(event->pos());
                m_currentRelativePath.append(relativePoint);
            }

            DrawPoint point;
            point.point = event->pos();
            point.color = m_penColor;
            point.width = m_penWidth;
            m_currentPath.append(point);

            update();
        }
        else if (m_drawingShape) {
            // 更新几何图形
            m_currentShape.endPoint = event->pos();
            update();
        }
    }
}

void OverlayWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_draggingToolbar) {
            m_draggingToolbar = false;
            constrainToolbarPosition();
            return;
        }

        if (m_erasing) {
            // 结束擦除操作
            m_erasing = false;
            if (!m_currentErasedData.isEmpty()) {
                saveAction(ACTION_ERASE, QVector<DrawPoint>(), TextItem(), -1,
                    QString(), QString(), QColor(), QColor(), m_currentErasedData);
                if (m_useRelativeCoordinates) {
                    syncRelativeData();
                }
            }
        }
        else if (m_drawing) {
            // 结束自由绘制
            m_drawing = false;
            if (!m_currentPath.isEmpty()) {
                if (m_useRelativeCoordinates) {
                    m_relativePaths.append(m_currentRelativePath);
                    m_currentRelativePath.clear();
                }
                m_paths.append(m_currentPath);
                saveAction(ACTION_DRAW_PATH, m_currentPath);
                m_currentPath.clear();
            }
            update();
        }
        else if (m_drawingShape) {
            // 完成几何图形绘制
            m_drawingShape = false;

            // 检查是否有效（起点和终点不能太接近）
            QPoint diff = m_currentShape.endPoint - m_currentShape.startPoint;
            if (qAbs(diff.x()) > 3 || qAbs(diff.y()) > 3) {
                m_shapes.append(m_currentShape);

                // 同步到相对坐标
                if (m_useRelativeCoordinates && m_targetWidget) {
                    QSize currentSize = m_targetWidget->size();
                    RelativeShapeItem relativeShape = RelativeShapeItem::fromAbsolute(m_currentShape, currentSize);
                    m_relativeShapes.append(relativeShape);
                }

                saveAction(ACTION_ADD_SHAPE, QVector<DrawPoint>(), TextItem(), -1,
                    QString(), QString(), QColor(), QColor(), ErasedData(), m_currentShape);
            }

            m_currentShape = ShapeItem();
            update();
        }
    }
}


void OverlayWidget::handleTextClick(const QPoint& position)
{
    // 检查是否点击在已有文字上（可以编辑）
    bool clickedOnText = false;
    for (int i = m_textItems.size() - 1; i >= 0; --i) {
        const TextItem& item = m_textItems[i];
        QFontMetrics fm(item.font);
        QRect textRect(item.position, fm.size(Qt::TextSingleLine, item.text));
        textRect.adjust(-5, -5, 5, 5); // 扩大点击区域

        if (textRect.contains(position)) {
            editTextAt(i, position);
            clickedOnText = true;
            break;
        }
    }

    if (!clickedOnText) {
        addTextAt(position);
    }
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
        QString(
            "QLineEdit { background-color: white; border: 2px solid %1; "
            "padding: 5px; font-size: %2px; }").arg(m_penColor.name()).arg(m_fontSize));
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
        //m_textModeCheckBox->setChecked(false);
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

    // 擦除路径（原有逻辑）
    for (int i = m_paths.size() - 1; i >= 0; --i) {
        const auto& path = m_paths[i];
        bool pathErased = false;

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

            if (m_useRelativeCoordinates && i < m_relativePaths.size()) {
                m_relativePaths.removeAt(i);
            }
            hasErased = true;
        }
    }

    // 擦除文字（原有逻辑）
    for (int i = m_textItems.size() - 1; i >= 0; --i) {
        const TextItem& textItem = m_textItems[i];

        if (isTextInEraserRange(textItem, position)) {
            m_currentErasedData.erasedTextIndices.append(i);
            m_currentErasedData.erasedTexts.append(textItem);
            m_textItems.removeAt(i);

            if (m_useRelativeCoordinates && i < m_relativeTextItems.size()) {
                m_relativeTextItems.removeAt(i);
            }
            hasErased = true;
        }
    }

    // 新增：擦除几何图形
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        const ShapeItem& shape = m_shapes[i];

        if (isShapeInEraserRange(shape, position)) {
            m_currentErasedData.erasedShapeIndices.append(i);
            m_currentErasedData.erasedShapes.append(shape);
            m_shapes.removeAt(i);

            if (m_useRelativeCoordinates && i < m_relativeShapes.size()) {
                m_relativeShapes.removeAt(i);
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

bool OverlayWidget::isShapeInEraserRange(const ShapeItem& shape, const QPoint& eraserCenter)
{
    int radius = m_eraserSize / 2;
    QRect eraserRect(eraserCenter.x() - radius, eraserCenter.y() - radius,
        m_eraserSize, m_eraserSize);

    // 对于不同类型的图形，使用不同的碰撞检测方法
    switch (shape.type) {
    case SHAPE_LINE:
        // 检查线段是否与橡皮擦圆形相交
        return isLineIntersectCircle(shape.startPoint, shape.endPoint, eraserCenter, radius);

    case SHAPE_RECTANGLE:
    case SHAPE_ELLIPSE:
        // 检查矩形/椭圆边界是否与橡皮擦相交
    {
        QRect shapeRect(shape.startPoint, shape.endPoint);
        shapeRect = shapeRect.normalized();
        return shapeRect.intersects(eraserRect);
    }

    case SHAPE_ARROW:
        // 箭头包含线条和箭头头部
        if (isLineIntersectCircle(shape.startPoint, shape.endPoint, eraserCenter, radius)) {
            return true;
        }
        // 检查箭头头部
        QPolygonF arrowHead = createArrowHead(shape.startPoint, shape.endPoint, shape.arrowSize);
        return arrowHead.boundingRect().intersects(eraserRect);
    }

    return false;
}

bool OverlayWidget::isLineIntersectCircle(const QPoint& lineStart, const QPoint& lineEnd,
    const QPoint& circleCenter, int radius)
{
    // 计算点到线段的最短距离
    QVector2D line(lineEnd - lineStart);
    QVector2D toStart(lineStart - circleCenter);

    if (line.lengthSquared() == 0) {
        // 线段退化为点
        return (lineStart - circleCenter).manhattanLength() <= radius;
    }

    float t = -QVector2D::dotProduct(toStart, line) / line.lengthSquared();
    t = qBound(0.0f, t, 1.0f);

    QPointF closest = lineStart + t * line.toPointF();
    QVector2D toClosest(closest - circleCenter);

    return toClosest.lengthSquared() <= radius * radius;
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
    if (m_textEdit) {
        m_textEdit->setStyleSheet(
            QString(
                "QLineEdit { background-color: white; border: 2px solid %1; "
                "padding: 5px; font-size: %2px; }").arg(m_penColor.name()).arg(m_fontSize));
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
        //m_eraserModeCheckBox->setChecked(false);
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
    const ErasedData& erasedData, const ShapeItem& shapeData)
{
    // 进行新操作时清空重做栈（历史分支改变）
    clearRedoStack();

    UndoAction action;
    action.type = type;
    action.pathData = pathData;
    action.textData = textData;
    action.shapeData = shapeData;  // 新增几何图形数据
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

//void OverlayWidget::updateUndoRedoButtons()
//{
//    if (m_undoButton) {
//        m_undoButton->setEnabled(!m_undoStack.isEmpty());
//    }
//    if (m_redoButton) {
//        m_redoButton->setEnabled(!m_redoStack.isEmpty());
//    }
//}

void OverlayWidget::clearRedoStack()
{
    m_redoStack.clear();
    updateUndoRedoButtons();
}

void OverlayWidget::undoLastAction()
{
    if (m_undoStack.isEmpty()) return;

    // 先完成正在进行的操作
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
        // 撤销添加文字：移除匹配的文字
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

    case ACTION_ADD_SHAPE:
        // 撤销添加几何图形：移除最后添加的图形
        if (!m_shapes.isEmpty()) {
            // 查找匹配的图形并移除
            for (int i = m_shapes.size() - 1; i >= 0; --i) {
                const ShapeItem& shape = m_shapes[i];
                if (shape.startPoint == lastAction.shapeData.startPoint &&
                    shape.endPoint == lastAction.shapeData.endPoint &&
                    shape.type == lastAction.shapeData.type) {
                    m_shapes.removeAt(i);
                    if (m_useRelativeCoordinates && i < m_relativeShapes.size()) {
                        m_relativeShapes.removeAt(i);
                    }
                    break;
                }
            }
        }
        break;

    case ACTION_ERASE:
        // 撤销橡皮擦操作：恢复被擦除的内容
        undoEraseAction(lastAction.erasedData);
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

    // 先完成正在进行的操作
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

    case ACTION_ADD_SHAPE:
        // 重做添加几何图形：重新添加图形
        m_shapes.append(lastRedoAction.shapeData);
        if (m_useRelativeCoordinates && m_targetWidget) {
            QSize currentSize = m_targetWidget->size();
            RelativeShapeItem relativeItem = RelativeShapeItem::fromAbsolute(lastRedoAction.shapeData, currentSize);
            m_relativeShapes.append(relativeItem);
        }
        break;

    case ACTION_ERASE:
        // 重做橡皮擦操作：重新执行擦除
        redoEraseAction(lastRedoAction.erasedData);
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

void OverlayWidget::undoEraseAction(const ErasedData& erasedData)
{
    // 恢复被擦除的路径
    for (int i = 0; i < erasedData.erasedPathIndices.size(); ++i) {
        int index = erasedData.erasedPathIndices[i];
        const auto& path = erasedData.erasedPaths[i];
        
        // 插入到原来的位置
        if (index <= m_paths.size()) {
            m_paths.insert(index, path);
            
            // 同步相对坐标
            if (m_useRelativeCoordinates && m_targetWidget) {
                QSize currentSize = m_targetWidget->size();
                QVector<RelativePoint> relativePath;
                for (const auto& point : path) {
                    relativePath.append(RelativePoint::fromAbsolute(point.point, currentSize, point.color, point.width));
                }
                if (index <= m_relativePaths.size()) {
                    m_relativePaths.insert(index, relativePath);
                }
            }
        }
    }

    // 恢复被擦除的文字
    for (int i = 0; i < erasedData.erasedTextIndices.size(); ++i) {
        int index = erasedData.erasedTextIndices[i];
        const auto& textItem = erasedData.erasedTexts[i];
        
        if (index <= m_textItems.size()) {
            m_textItems.insert(index, textItem);
            
            // 同步相对坐标
            if (m_useRelativeCoordinates && m_targetWidget) {
                QSize currentSize = m_targetWidget->size();
                RelativeTextItem relativeItem = RelativeTextItem::fromAbsolute(textItem, currentSize);
                if (index <= m_relativeTextItems.size()) {
                    m_relativeTextItems.insert(index, relativeItem);
                }
            }
        }
    }

    // 恢复被擦除的几何图形
    for (int i = 0; i < erasedData.erasedShapeIndices.size(); ++i) {
        int index = erasedData.erasedShapeIndices[i];
        const auto& shape = erasedData.erasedShapes[i];
        
        if (index <= m_shapes.size()) {
            m_shapes.insert(index, shape);
            
            // 同步相对坐标
            if (m_useRelativeCoordinates && m_targetWidget) {
                QSize currentSize = m_targetWidget->size();
                RelativeShapeItem relativeShape = RelativeShapeItem::fromAbsolute(shape, currentSize);
                if (index <= m_relativeShapes.size()) {
                    m_relativeShapes.insert(index, relativeShape);
                }
            }
        }
    }
}

void OverlayWidget::redoEraseAction(const ErasedData& erasedData)
{
    // 重新执行擦除操作，按索引从大到小删除以避免索引错乱

    // 创建排序后的索引向量
    QVector<QPair<int, int>> sortedPathIndices;
    for (int i = 0; i < erasedData.erasedPathIndices.size(); ++i) {
        sortedPathIndices.append(qMakePair(erasedData.erasedPathIndices[i], i));
    }
    std::sort(sortedPathIndices.begin(), sortedPathIndices.end(),
        [](const QPair<int, int>& a, const QPair<int, int>& b) {
            return a.first > b.first; // 从大到小排序
        });

    // 删除路径
    for (const auto& pair : sortedPathIndices) {
        int index = pair.first;
        if (index < m_paths.size()) {
            m_paths.removeAt(index);
            if (m_useRelativeCoordinates && index < m_relativePaths.size()) {
                m_relativePaths.removeAt(index);
            }
        }
    }

    // 类似地处理文字
    QVector<QPair<int, int>> sortedTextIndices;
    for (int i = 0; i < erasedData.erasedTextIndices.size(); ++i) {
        sortedTextIndices.append(qMakePair(erasedData.erasedTextIndices[i], i));
    }
    std::sort(sortedTextIndices.begin(), sortedTextIndices.end(),
        [](const QPair<int, int>& a, const QPair<int, int>& b) {
            return a.first > b.first;
        });

    for (const auto& pair : sortedTextIndices) {
        int index = pair.first;
        if (index < m_textItems.size()) {
            m_textItems.removeAt(index);
            if (m_useRelativeCoordinates && index < m_relativeTextItems.size()) {
                m_relativeTextItems.removeAt(index);
            }
        }
    }

    // 处理几何图形
    QVector<QPair<int, int>> sortedShapeIndices;
    for (int i = 0; i < erasedData.erasedShapeIndices.size(); ++i) {
        sortedShapeIndices.append(qMakePair(erasedData.erasedShapeIndices[i], i));
    }
    std::sort(sortedShapeIndices.begin(), sortedShapeIndices.end(),
        [](const QPair<int, int>& a, const QPair<int, int>& b) {
            return a.first > b.first;
        });

    for (const auto& pair : sortedShapeIndices) {
        int index = pair.first;
        if (index < m_shapes.size()) {
            m_shapes.removeAt(index);
            if (m_useRelativeCoordinates && index < m_relativeShapes.size()) {
                m_relativeShapes.removeAt(index);
            }
        }
    }
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
        QPixmap pixmap(m_targetWidget->size());
        m_targetWidget->render(&pixmap);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        drawPaths(painter);
        drawTexts(painter);
        drawShapes(painter);  // 新增：保存几何图形

        if (pixmap.save(fileName)) {
            QMessageBox::information(this, tr("保存成功"), tr("增强标注图片已保存到: ") + fileName);
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




// 移动几何图形
void OverlayWidget::moveShape(int shapeIndex, const QPoint& offset)
{
    if (shapeIndex >= 0 && shapeIndex < m_shapes.size()) {
        m_shapes[shapeIndex].startPoint += offset;
        m_shapes[shapeIndex].endPoint += offset;

        // 同步相对坐标
        if (m_useRelativeCoordinates && m_targetWidget) {
            QSize currentSize = m_targetWidget->size();
            m_relativeShapes[shapeIndex] = RelativeShapeItem::fromAbsolute(m_shapes[shapeIndex], currentSize);
        }

        update();
    }
}

// 缩放几何图形
void OverlayWidget::scaleShape(int shapeIndex, float scaleX, float scaleY, const QPoint& center)
{
    if (shapeIndex >= 0 && shapeIndex < m_shapes.size()) {
        ShapeItem& shape = m_shapes[shapeIndex];

        // 以center为中心进行缩放
        QPoint start = shape.startPoint - center;
        QPoint end = shape.endPoint - center;

        start.setX(qRound(start.x() * scaleX));
        start.setY(qRound(start.y() * scaleY));
        end.setX(qRound(end.x() * scaleX));
        end.setY(qRound(end.y() * scaleY));

        shape.startPoint = start + center;
        shape.endPoint = end + center;

        // 同步相对坐标
        if (m_useRelativeCoordinates && m_targetWidget) {
            QSize currentSize = m_targetWidget->size();
            m_relativeShapes[shapeIndex] = RelativeShapeItem::fromAbsolute(shape, currentSize);
        }

        update();
    }
}

// 检查点是否在几何图形上
int OverlayWidget::hitTestShape(const QPoint& point, int tolerance)
{
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        const ShapeItem& shape = m_shapes[i];

        switch (shape.type) {
        case SHAPE_LINE:
            if (isLineIntersectCircle(shape.startPoint, shape.endPoint, point, tolerance)) {
                return i;
            }
            break;

        case SHAPE_RECTANGLE:
        {
            QRect rect(shape.startPoint, shape.endPoint);
            rect = rect.normalized();
            QRect hitRect = rect.adjusted(-tolerance, -tolerance, tolerance, tolerance);
            if (shape.filled) {
                if (rect.contains(point)) return i;
            }
            else {
                if (hitRect.contains(point) && !rect.adjusted(tolerance, tolerance, -tolerance, -tolerance).contains(point)) {
                    return i;
                }
            }
        }
        break;

        case SHAPE_ELLIPSE:
        {
            QRect rect(shape.startPoint, shape.endPoint);
            rect = rect.normalized();
            QPointF center = rect.center();
            float rx = rect.width() / 2.0f;
            float ry = rect.height() / 2.0f;

            if (rx > 0 && ry > 0) {
                float dx = (point.x() - center.x()) / rx;
                float dy = (point.y() - center.y()) / ry;
                float distance = dx * dx + dy * dy;

                if (shape.filled) {
                    if (distance <= 1.0f) return i;
                }
                else {
                    float toleranceRatio = tolerance / qMin(rx, ry);
                    if (distance <= (1.0f + toleranceRatio) * (1.0f + toleranceRatio) &&
                        distance >= (1.0f - toleranceRatio) * (1.0f - toleranceRatio)) {
                        return i;
                    }
                }
            }
        }
        break;

        case SHAPE_ARROW:
            // 箭头包含线条和箭头头部
            if (isLineIntersectCircle(shape.startPoint, shape.endPoint, point, tolerance)) {
                return i;
            }
            // 检查箭头头部
            QPolygonF arrowHead = createArrowHead(shape.startPoint, shape.endPoint, shape.arrowSize);
            if (arrowHead.containsPoint(point, Qt::OddEvenFill)) {
                return i;
            }
            break;
        }
    }

    return -1; // 未找到
}

// ============================================================================
// 导出功能
// ============================================================================

#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QElapsedTimer>


// 导出标注数据为JSON格式
QString OverlayWidget::exportAnnotationData() const
{
    QJsonObject root;

    // 导出路径数据
    QJsonArray pathsArray;
    for (const auto& path : m_paths) {
        QJsonArray pathArray;
        for (const auto& point : path) {
            QJsonObject pointObj;
            pointObj["x"] = point.point.x();
            pointObj["y"] = point.point.y();
            pointObj["color"] = point.color.name();
            pointObj["width"] = point.width;
            pathArray.append(pointObj);
        }
        pathsArray.append(pathArray);
    }
    root["paths"] = pathsArray;

    // 导出文字数据
    QJsonArray textsArray;
    for (const auto& text : m_textItems) {
        QJsonObject textObj;
        textObj["x"] = text.position.x();
        textObj["y"] = text.position.y();
        textObj["text"] = text.text;
        textObj["color"] = text.color.name();
        textObj["fontFamily"] = text.font.family();
        textObj["fontSize"] = text.font.pointSize();
        textObj["bold"] = text.font.bold();
        textObj["italic"] = text.font.italic();
        textsArray.append(textObj);
    }
    root["texts"] = textsArray;

    // 导出几何图形数据
    QJsonArray shapesArray;
    for (const auto& shape : m_shapes) {
        QJsonObject shapeObj;
        shapeObj["type"] = static_cast<int>(shape.type);
        shapeObj["startX"] = shape.startPoint.x();
        shapeObj["startY"] = shape.startPoint.y();
        shapeObj["endX"] = shape.endPoint.x();
        shapeObj["endY"] = shape.endPoint.y();
        shapeObj["color"] = shape.color.name();
        shapeObj["width"] = shape.width;
        shapeObj["filled"] = shape.filled;
        shapeObj["arrowSize"] = shape.arrowSize;
        shapesArray.append(shapeObj);
    }
    root["shapes"] = shapesArray;

    // 添加元数据
    QJsonObject metadata;
    metadata["version"] = "2.0";
    metadata["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    if (m_targetWidget) {
        metadata["targetWidth"] = m_targetWidget->width();
        metadata["targetHeight"] = m_targetWidget->height();
    }
    root["metadata"] = metadata;

    QJsonDocument doc(root);
    return doc.toJson();
}

// 从JSON导入标注数据
bool OverlayWidget::importAnnotationData(const QString& jsonData)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析错误：" << error.errorString();
        return false;
    }

    QJsonObject root = doc.object();

    // 清除现有数据
    clearCanvas();

    try {
        // 导入路径数据
        QJsonArray pathsArray = root["paths"].toArray();
        for (const auto& pathValue : pathsArray) {
            QJsonArray pathArray = pathValue.toArray();
            QVector<DrawPoint> path;
            for (const auto& pointValue : pathArray) {
                QJsonObject pointObj = pointValue.toObject();
                DrawPoint point;
                point.point.setX(pointObj["x"].toInt());
                point.point.setY(pointObj["y"].toInt());
                point.color = QColor(pointObj["color"].toString());
                point.width = pointObj["width"].toInt();
                path.append(point);
            }
            if (!path.isEmpty()) {
                m_paths.append(path);
            }
        }

        // 导入文字数据
        QJsonArray textsArray = root["texts"].toArray();
        for (const auto& textValue : textsArray) {
            QJsonObject textObj = textValue.toObject();
            TextItem text;
            text.position.setX(textObj["x"].toInt());
            text.position.setY(textObj["y"].toInt());
            text.text = textObj["text"].toString();
            text.color = QColor(textObj["color"].toString());
            text.font.setFamily(textObj["fontFamily"].toString());
            text.font.setPointSize(textObj["fontSize"].toInt());
            text.font.setBold(textObj["bold"].toBool());
            text.font.setItalic(textObj["italic"].toBool());
            m_textItems.append(text);
        }

        // 导入几何图形数据
        QJsonArray shapesArray = root["shapes"].toArray();
        for (const auto& shapeValue : shapesArray) {
            QJsonObject shapeObj = shapeValue.toObject();
            ShapeItem shape;
            shape.type = static_cast<ShapeType>(shapeObj["type"].toInt());
            shape.startPoint.setX(shapeObj["startX"].toInt());
            shape.startPoint.setY(shapeObj["startY"].toInt());
            shape.endPoint.setX(shapeObj["endX"].toInt());
            shape.endPoint.setY(shapeObj["endY"].toInt());
            shape.color = QColor(shapeObj["color"].toString());
            shape.width = shapeObj["width"].toInt();
            shape.filled = shapeObj["filled"].toBool();
            shape.arrowSize = shapeObj["arrowSize"].toInt();
            m_shapes.append(shape);
        }

        // 同步到相对坐标系统
        if (m_useRelativeCoordinates) {
            syncRelativeData();
        }

        update();
        return true;

    }
    catch (...) {
        qWarning() << "导入标注数据时发生错误";
        clearCanvas();
        return false;
    }
}



// 快捷键和样式配置系统

// ============================================================================
// 快捷键支持扩展
// ============================================================================

void OverlayWidget::keyPressEvent(QKeyEvent* event)
{
    // 基础快捷键
    if (event->key() == Qt::Key_Escape) {
        finishEditing();
        return;
    }
    else if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        clearCanvas();
        return;
    }
    else if (event->key() == Qt::Key_Z && (event->modifiers() & Qt::ControlModifier)) {
        if (event->modifiers() & Qt::ShiftModifier) {
            redoLastAction();  // Ctrl+Shift+Z 重做
        }
        else {
            undoLastAction();  // Ctrl+Z 撤销
        }
        return;
    }
    else if (event->key() == Qt::Key_Y && (event->modifiers() & Qt::ControlModifier)) {
        redoLastAction();  // Ctrl+Y 重做
        return;
    }
    else if (event->key() == Qt::Key_S && (event->modifiers() & Qt::ControlModifier)) {
        saveImage();  // Ctrl+S 保存
        return;
    }

    // 工具切换快捷键
    switch (event->key()) {
    case Qt::Key_P:  // Pen - 自由绘制
        setDrawingTool(TOOL_FREE_DRAW);
        break;
    case Qt::Key_L:  // Line - 直线
        setDrawingTool(TOOL_LINE);
        break;
    case Qt::Key_R:  // Rectangle - 矩形
        setDrawingTool(TOOL_RECTANGLE);
        break;
    case Qt::Key_O:  // Oval/Ellipse - 椭圆
        setDrawingTool(TOOL_ELLIPSE);
        break;
    case Qt::Key_A:  // Arrow - 箭头
        setDrawingTool(TOOL_ARROW);
        break;
    case Qt::Key_T:  // Text - 文字
        setDrawingTool(TOOL_TEXT);
        break;
    case Qt::Key_E:  // Eraser - 橡皮擦
        setDrawingTool(TOOL_ERASER);
        break;
    case Qt::Key_F:  // Fill - 切换填充模式
        if (m_currentTool == TOOL_RECTANGLE || m_currentTool == TOOL_ELLIPSE) {
            m_fillMode = !m_fillMode;
            if (m_fillModeCheckBox) {
                m_fillModeCheckBox->setChecked(m_fillMode);
            }
        }
        break;
    }

    // 颜色快捷键
    if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_1: changePenColorTo(Qt::red); break;
        case Qt::Key_2: changePenColorTo(Qt::green); break;
        case Qt::Key_3: changePenColorTo(Qt::blue); break;
        case Qt::Key_4: changePenColorTo(Qt::yellow); break;
        case Qt::Key_5: changePenColorTo(Qt::magenta); break;
        case Qt::Key_6: changePenColorTo(Qt::cyan); break;
        case Qt::Key_7: changePenColorTo(Qt::black); break;
        case Qt::Key_8: changePenColorTo(Qt::white); break;
        }
    }

    // 大小调整快捷键
    if (event->key() == Qt::Key_BracketLeft) {  // [ - 减小
        if (m_currentTool == TOOL_ERASER) {
            changeEraserSize(qMax(10, m_eraserSize - 5));
        } else if (m_currentTool == TOOL_ARROW) {
            changeArrowSize(qMax(5, m_arrowSize - 2));
        } else if (m_currentTool == TOOL_TEXT) {
            changeFontSize(qMax(8, m_fontSize - 2));
        } else {
            changePenWidth(qMax(1, m_penWidth - 1));
        }
    }
    else if (event->key() == Qt::Key_BracketRight) {  // ] - 增大
        if (m_currentTool == TOOL_ERASER) {
            changeEraserSize(qMin(80, m_eraserSize + 5));
        } else if (m_currentTool == TOOL_ARROW) {
            changeArrowSize(qMin(30, m_arrowSize + 2));
        } else if (m_currentTool == TOOL_TEXT) {
            changeFontSize(qMin(72, m_fontSize + 2));
        } else {
            changePenWidth(qMin(20, m_penWidth + 1));
        }
    }

    // 工具栏控制
    if (event->key() == Qt::Key_Space) {
        toggleToolbarCollapse();  // 空格键切换工具栏显示
    }

    QWidget::keyPressEvent(event);
}

void OverlayWidget::changePenColorTo(const QColor& color)
{
    m_penColor = color;
    if (m_colorButton) {
        m_colorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(color.name()));
    }
}

// ============================================================================
// 样式配置系统
// ============================================================================


// 在OverlayWidget中添加样式应用方法
void OverlayWidget::applyCurrentStyle()
{
    const OverlayStyle& style = OverlayStyleManager::instance().getStyle();
    
    // 应用工具栏样式
    if (m_toolbarContent) {
        m_toolbarContent->setStyleSheet(OverlayStyleManager::instance().generateToolbarStyleSheet());
    }
    
    // 应用默认值
    if (m_penColor == Qt::red) {  // 只在还是默认颜色时才更改
        m_penColor = style.defaultPenColor;
        if (m_colorButton) {
            m_colorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(m_penColor.name()));
        }
    }
}

void OverlayWidget::setStyleTheme(OverlayStyleManager::StyleTheme theme)
{
    OverlayStyleManager::instance().setTheme(theme);
    applyCurrentStyle();
}


// ============================================================================
// 帮助和快捷键显示
// ============================================================================

void OverlayWidget::showShortcutsHelp()
{
    QString helpText = tr(
        "快捷键帮助\n\n"
        "工具切换:\n"
        "P - 自由绘制  L - 直线  R - 矩形\n"
        "O - 椭圆  A - 箭头  T - 文字  E - 橡皮擦\n\n"
        "操作:\n"
        "Ctrl+Z - 撤销  Ctrl+Y - 重做\n"
        "Ctrl+S - 保存  ESC - 退出标注\n"
        "Delete - 清除所有  Space - 切换工具栏\n\n"
        "颜色 (Ctrl+数字):\n"
        "1-红 2-绿 3-蓝 4-黄 5-品红 6-青 7-黑 8-白\n\n"
        "大小调整:\n"
        "[ - 减小  ] - 增大\n"
        "F - 切换填充模式（矩形/椭圆）\n\n"
        "鼠标操作:\n"
        "滚轮 - 调整当前工具大小\n"
        "拖拽 - 绘制图形\n"
        "双击 - 添加文字"
    );

    QMessageBox::information(this, tr("快捷键帮助"), helpText);
}

// 在构造函数或setupUI中添加帮助按钮
void OverlayWidget::addHelpButton()
{
    if (!m_toolbarContent) return;

    QPushButton* helpButton = new QPushButton("?", m_toolbarContent);
    helpButton->setFixedSize(24, 24);
    helpButton->setToolTip(tr("显示快捷键帮助"));
    helpButton->setStyleSheet(
        "QPushButton { "
        "  background-color: rgba(100, 100, 100, 150); "
        "  color: white; "
        "  border: 1px solid gray; "
        "  border-radius: 12px; "
        "  font-weight: bold; "
        "}"
        "QPushButton:hover { "
        "  background-color: rgba(120, 120, 120, 180); "
        "}"
    );
    
    connect(helpButton, &QPushButton::clicked, this, &OverlayWidget::showShortcutsHelp);
    
    // 将帮助按钮添加到工具栏的某个位置
    // 这里需要根据实际的布局来确定位置
}


// =============================================================================
// 新增的公共接口实现
// =============================================================================

void OverlayWidget::setPenColor(const QColor& color)
{
    m_penColor = color;
    if (m_colorButton) {
        m_colorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(color.name()));
    }
    // 如果正在编辑文字，更新文字编辑框样式
    if (m_textEdit) {
        QString styleSheet = QString(
            "QLineEdit { background-color: white; border: 2px solid %1; "
            "padding: 5px; font-size: %2px; }").arg(color.name()).arg(m_fontSize);
        m_textEdit->setStyleSheet(styleSheet);
    }
}

void OverlayWidget::setPenWidth(int width)
{
    m_penWidth = qBound(1, width, 20);
    if (m_widthSpinBox) {
        m_widthSpinBox->setValue(m_penWidth);
    }
}

void OverlayWidget::setFontSize(int size)
{
    m_fontSize = qBound(8, size, 72);
    if (m_fontSizeSpinBox) {
        m_fontSizeSpinBox->setValue(m_fontSize);
    }
}

void OverlayWidget::setFillMode(bool enabled)
{
    m_fillMode = enabled;
    if (m_fillModeCheckBox) {
        m_fillModeCheckBox->setChecked(enabled);
    }
}

void OverlayWidget::setArrowSize(int size)
{
    m_arrowSize = qBound(5, size, 30);
    if (m_arrowSizeSpinBox) {
        m_arrowSizeSpinBox->setValue(m_arrowSize);
    }
}

void OverlayWidget::setEraserSize(int size)
{
    m_eraserSize = qBound(10, size, 80);
    if (m_eraserSizeSpinBox) {
        m_eraserSizeSpinBox->setValue(m_eraserSize);
    }
}

// =============================================================================
// 工具栏状态更新函数
// =============================================================================

void OverlayWidget::updateToolButtonStates()
{
    // 更新属性控件的可用状态
    bool isArrowTool = (m_currentTool == TOOL_ARROW);
    bool isTextTool = (m_currentTool == TOOL_TEXT);
    bool isEraserTool = (m_currentTool == TOOL_ERASER);
    bool isShapeTool = (m_currentTool == TOOL_RECTANGLE || m_currentTool == TOOL_ELLIPSE);

    if (m_arrowSizeSpinBox) {
        m_arrowSizeSpinBox->setEnabled(isArrowTool);
    }
    if (m_fontSizeSpinBox) {
        m_fontSizeSpinBox->setEnabled(isTextTool);
    }
    if (m_eraserSizeSpinBox) {
        m_eraserSizeSpinBox->setEnabled(isEraserTool);
    }
    if (m_fillModeCheckBox) {
        m_fillModeCheckBox->setEnabled(isShapeTool);
    }

    // 更新工具提示信息
    updateToolTips();
}

void OverlayWidget::updateToolTips()
{
    if (!m_toolButtonGroup) return;

    // 根据当前工具更新提示信息
    QString currentToolInfo;
    switch (m_currentTool) {
    case TOOL_FREE_DRAW:
        currentToolInfo = tr("当前: 自由绘制 - 按住鼠标拖拽绘制");
        break;
    case TOOL_LINE:
        currentToolInfo = tr("当前: 直线 - 点击起点拖拽到终点");
        break;
    case TOOL_RECTANGLE:
        currentToolInfo = tr("当前: 矩形 - 对角拖拽定义矩形");
        break;
    case TOOL_ELLIPSE:
        currentToolInfo = tr("当前: 椭圆 - 拖拽定义椭圆边界");
        break;
    case TOOL_ARROW:
        currentToolInfo = tr("当前: 箭头 - 从起点拖拽到指向目标");
        break;
    case TOOL_TEXT:
        currentToolInfo = tr("当前: 文字 - 点击位置添加文字");
        break;
    case TOOL_ERASER:
        currentToolInfo = tr("当前: 橡皮擦 - 点击或拖拽擦除内容");
        break;
    }

    // 更新工具栏标题
    if (m_toolbarHeader) {
        QLabel* titleLabel = m_toolbarHeader->findChild<QLabel*>();
        if (titleLabel) {
            titleLabel->setToolTip(currentToolInfo);
        }
    }
}

// =============================================================================
// 配置管理函数完善
// =============================================================================

QString OverlayWidget::getDefaultConfigPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
        "/overlay_widget_config.ini";
}

void OverlayWidget::saveConfiguration(const QString& filePath)
{
    QString configPath = filePath.isEmpty() ? getDefaultConfigPath() : filePath;

    // 确保目录存在
    QFileInfo fileInfo(configPath);
    QDir().mkpath(fileInfo.absolutePath());

    QSettings settings(configPath, QSettings::IniFormat);

    settings.beginGroup("Tools");
    settings.setValue("currentTool", static_cast<int>(m_currentTool));
    settings.setValue("penColor", m_penColor.name());
    settings.setValue("penWidth", m_penWidth);
    settings.setValue("fontSize", m_fontSize);
    settings.setValue("fillMode", m_fillMode);
    settings.setValue("arrowSize", m_arrowSize);
    settings.setValue("eraserSize", m_eraserSize);
    settings.endGroup();

    settings.beginGroup("Appearance");
    settings.setValue("styleTheme", static_cast<int>(OverlayStyleManager::instance().getCurrentTheme()));
    settings.endGroup();

    settings.beginGroup("Behavior");
    settings.setValue("useRelativeCoordinates", m_useRelativeCoordinates);
    settings.setValue("debugMode", m_debugMode);
    settings.setValue("highPrecisionMode", m_useHighPrecision);
    settings.endGroup();

    settings.beginGroup("UI");
    settings.setValue("toolbarCollapsed", m_toolbarCollapsed);
    if (m_toolbar) {
        settings.setValue("toolbarPosition", m_toolbar->pos());
    }
    settings.endGroup();

    settings.beginGroup("Margins");
    settings.setValue("left", m_targetMargins.left());
    settings.setValue("top", m_targetMargins.top());
    settings.setValue("right", m_targetMargins.right());
    settings.setValue("bottom", m_targetMargins.bottom());
    settings.endGroup();
}

void OverlayWidget::loadConfiguration(const QString& filePath)
{
    QString configPath = filePath.isEmpty() ? getDefaultConfigPath() : filePath;

    if (!QFile::exists(configPath)) {
        qDebug() << "Config file not found:" << configPath;
        return;
    }

    QSettings settings(configPath, QSettings::IniFormat);

    settings.beginGroup("Tools");
    setDrawingTool(static_cast<DrawingTool>(settings.value("currentTool", TOOL_FREE_DRAW).toInt()));
    setPenColor(QColor(settings.value("penColor", "#FF0000").toString()));
    setPenWidth(settings.value("penWidth", 3).toInt());
    setFontSize(settings.value("fontSize", 12).toInt());
    setFillMode(settings.value("fillMode", false).toBool());
    setArrowSize(settings.value("arrowSize", 10).toInt());
    setEraserSize(settings.value("eraserSize", 20).toInt());
    settings.endGroup();

    settings.beginGroup("Appearance");
    auto theme = static_cast<OverlayStyleManager::StyleTheme>(
        settings.value("styleTheme", OverlayStyleManager::THEME_DARK).toInt());
    setStyleTheme(theme);
    if (m_themeComboBox) {
        m_themeComboBox->setCurrentIndex(static_cast<int>(theme));
    }
    settings.endGroup();

    settings.beginGroup("Behavior");
    setUseRelativeCoordinates(settings.value("useRelativeCoordinates", true).toBool());
    setDebugMode(settings.value("debugMode", false).toBool());
    setHighPrecisionMode(settings.value("highPrecisionMode", false).toBool());
    settings.endGroup();

    settings.beginGroup("UI");
    m_toolbarCollapsed = settings.value("toolbarCollapsed", false).toBool();
    updateToolbarLayout();

    if (m_toolbar) {
        QPoint pos = settings.value("toolbarPosition", QPoint(10, 10)).toPoint();
        m_toolbar->move(pos);
        constrainToolbarPosition();
    }
    settings.endGroup();

    settings.beginGroup("Margins");
    QMargins margins(
        settings.value("left", 0).toInt(),
        settings.value("top", 0).toInt(),
        settings.value("right", 0).toInt(),
        settings.value("bottom", 0).toInt()
    );
    setTargetMargins(margins);
    settings.endGroup();
}

// =============================================================================
// 信号发射函数
// =============================================================================

void OverlayWidget::emitModificationSignal()
{
    emit annotationModified();

    // 同时更新撤销重做按钮状态
    updateUndoRedoButtons();
}

// =============================================================================
// 增强的工具切换函数
// =============================================================================

void OverlayWidget::setDrawingTool(DrawingTool tool)
{
    if (m_currentTool == tool) return;

    // 先完成正在进行的操作
    if (m_textEdit) {
        finishTextInput();
    }

    DrawingTool oldTool = m_currentTool;
    m_currentTool = tool;

    // 重置各种模式状态
    m_drawing = false;
    m_drawingShape = false;
    m_textMode = false;
    m_eraserMode = false;

    // 根据工具类型设置状态
    switch (m_currentTool) {
    case TOOL_TEXT:
        m_textMode = true;
        setCursor(Qt::IBeamCursor);
        setMouseTracking(false);
        break;
    case TOOL_ERASER:
        m_eraserMode = true;
        setCursor(createEraserCursor());
        setMouseTracking(true);
        break;
    default:
        setCursor(Qt::CrossCursor);
        setMouseTracking(false);
        break;
    }

    // 更新UI状态
    updateToolButtonStates();

    // 更新工具按钮选中状态
    if (m_toolButtonGroup) {
        QAbstractButton* button = m_toolButtonGroup->button(static_cast<int>(tool));
        if (button) {
            button->setChecked(true);
        }
    }

    // 发射工具变更信号
    emit toolChanged(tool);

    update();

    if (m_debugMode) {
        qDebug() << "Tool changed from" << oldTool << "to" << tool;
    }
}

// =============================================================================
// 增强的撤销重做按钮更新
// =============================================================================

void OverlayWidget::updateUndoRedoButtons()
{
    bool canUndo = !m_undoStack.isEmpty();
    bool canRedo = !m_redoStack.isEmpty();

    if (m_undoButton) {
        m_undoButton->setEnabled(canUndo);
        if (canUndo) {
            // 显示将要撤销的操作类型
            ActionType lastAction = m_undoStack.last().type;
            QString actionName = getActionName(lastAction);
            m_undoButton->setToolTip(tr("撤销: %1 (Ctrl+Z)").arg(actionName));
        }
        else {
            m_undoButton->setToolTip(tr("撤销 (Ctrl+Z)"));
        }
    }

    if (m_redoButton) {
        m_redoButton->setEnabled(canRedo);
        if (canRedo) {
            // 显示将要重做的操作类型
            ActionType nextAction = m_redoStack.last().type;
            QString actionName = getActionName(nextAction);
            m_redoButton->setToolTip(tr("重做: %1 (Ctrl+Y)").arg(actionName));
        }
        else {
            m_redoButton->setToolTip(tr("重做 (Ctrl+Y)"));
        }
    }
}

QString OverlayWidget::getActionName(ActionType actionType) const
{
    switch (actionType) {
    case ACTION_DRAW_PATH: return tr("绘制路径");
    case ACTION_ADD_TEXT: return tr("添加文字");
    case ACTION_EDIT_TEXT: return tr("编辑文字");
    case ACTION_DELETE_TEXT: return tr("删除文字");
    case ACTION_ADD_SHAPE: return tr("添加图形");
    case ACTION_ERASE: return tr("擦除内容");
    default: return tr("未知操作");
    }
}

// =============================================================================
// 状态查询函数
// =============================================================================

bool OverlayWidget::hasUnsavedChanges() const
{
    return !m_paths.isEmpty() || !m_textItems.isEmpty() || !m_shapes.isEmpty();
}

QString OverlayWidget::getStatusText() const
{
    return QString(tr("路径: %1 | 文字: %2 | 图形: %3 | 工具: %4"))
        .arg(getPathCount())
        .arg(getTextCount())
        .arg(getShapeCount())
        .arg(getToolName(m_currentTool));
}

QString OverlayWidget::getToolName(DrawingTool tool) const
{
    switch (tool) {
    case TOOL_FREE_DRAW: return tr("自由绘制");
    case TOOL_LINE: return tr("直线");
    case TOOL_RECTANGLE: return tr("矩形");
    case TOOL_ELLIPSE: return tr("椭圆");
    case TOOL_ARROW: return tr("箭头");
    case TOOL_TEXT: return tr("文字");
    case TOOL_ERASER: return tr("橡皮擦");
    default: return tr("未知工具");
    }
}

// =============================================================================
// 增强的清除功能
// =============================================================================

void OverlayWidget::clearCanvas()
{
    if (m_textEdit) {
        finishTextInput();
    }

    // 保存清除前的状态用于撤销
    if (hasUnsavedChanges()) {
        // TODO: 实现批量撤销功能来支持清除操作的撤销
    }

    m_paths.clear();
    m_currentPath.clear();
    m_textItems.clear();
    m_shapes.clear();

    if (m_useRelativeCoordinates) {
        m_relativePaths.clear();
        m_currentRelativePath.clear();
        m_relativeTextItems.clear();
        m_relativeShapes.clear();
    }

    m_undoStack.clear();
    m_redoStack.clear();
    updateUndoRedoButtons();

    emit contentCleared();
    emit annotationModified();

    update();

    if (m_debugMode) {
        qDebug() << "Canvas cleared";
    }
}

// =============================================================================
// 性能优化相关
// =============================================================================

void OverlayWidget::optimizePerformance()
{
    // 清理过大的撤销栈
    while (m_undoStack.size() > MAX_UNDO_STEPS) {
        m_undoStack.removeFirst();
    }
    while (m_redoStack.size() > MAX_UNDO_STEPS) {
        m_redoStack.removeFirst();
    }

    // 使缓存失效以强制重新计算
    m_rectCacheValid = false;

    // 压缩空的路径
    for (int i = m_paths.size() - 1; i >= 0; --i) {
        if (m_paths[i].isEmpty()) {
            m_paths.removeAt(i);
            if (i < m_relativePaths.size()) {
                m_relativePaths.removeAt(i);
            }
        }
    }

    if (m_debugMode) {
        qDebug() << "Performance optimization completed";
    }
}

// =============================================================================
// 自动保存功能
// =============================================================================

void OverlayWidget::enableAutoSave(int intervalSeconds)
{
    static QTimer* autoSaveTimer = nullptr;

    if (autoSaveTimer) {
        autoSaveTimer->stop();
        delete autoSaveTimer;
        autoSaveTimer = nullptr;
    }

    if (intervalSeconds > 0) {
        autoSaveTimer = new QTimer(this);
        autoSaveTimer->setInterval(intervalSeconds * 1000);
        autoSaveTimer->setSingleShot(false);

        connect(autoSaveTimer, &QTimer::timeout, [this]() {
            if (hasUnsavedChanges()) {
                QString autoSavePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
                    QString("/overlay_autosave_%1.json")
                    .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

                QFile file(autoSavePath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(exportAnnotationData().toUtf8());
                    if (m_debugMode) {
                        qDebug() << "Auto-saved to:" << autoSavePath;
                    }
                }
            }
            });

        autoSaveTimer->start();

        if (m_debugMode) {
            qDebug() << "Auto-save enabled with interval:" << intervalSeconds << "seconds";
        }
    }
}

// 在OverlayWidget.cpp中补充以下实现

void OverlayStyleManager::setTheme(StyleTheme theme)
{
    currentTheme = theme;

    switch (theme) {
    case THEME_DARK:
        currentStyle = createDarkTheme();
        break;
    case THEME_LIGHT:
        currentStyle = createLightTheme();
        break;
    case THEME_BLUE:
        currentStyle = createBlueTheme();
        break;
    case THEME_GREEN:
        currentStyle = createGreenTheme();
        break;
    case THEME_CUSTOM:
        // 保持当前自定义样式
        break;
    }
}

QString OverlayStyleManager::generateToolbarStyleSheet() const
{
    const OverlayStyle& style = currentStyle;

    return QString(
        "QWidget#toolbar { "
        "  background-color: rgba(%1, %2, %3, %4); "
        "  border-radius: %5px; "
        "  border: 1px solid rgba(%6, %7, %8, %9); "
        "} "
        "QWidget { "
        "  background-color: rgba(%1, %2, %3, %4); "
        "  border-radius: 0 0 %5px %5px; "
        "} "
        "QPushButton { "
        "  color: rgb(%10, %11, %12); "
        "  border: 1px solid gray; "
        "  padding: %13px %14px; "
        "  margin: 1px; "
        "  border-radius: 3px; "
        "  font-size: %15px; "
        "  font-family: '%16'; "
        "} "
        "QPushButton:hover { "
        "  background-color: rgba(%17, %18, %19, %20); "
        "} "
        "QPushButton:checked { "
        "  background-color: rgba(%21, %22, %23, %24); "
        "  border-color: rgba(%21, %22, %23, 255); "
        "} "
        "QSpinBox { "
        "  color: rgb(%10, %11, %12); "
        "  background-color: rgba(70, 70, 70, 200); "
        "  border: 1px solid gray; "
        "  padding: 2px; "
        "} "
        "QCheckBox { "
        "  color: rgb(%10, %11, %12); "
        "  font-size: %15px; "
        "} "
        "QLabel { "
        "  color: rgb(%10, %11, %12); "
        "  font-size: %15px; "
        "} "
        "QComboBox { "
        "  color: rgb(%10, %11, %12); "
        "  background-color: rgba(70, 70, 70, 200); "
        "  border: 1px solid gray; "
        "  padding: 2px; "
        "}"
    )
        .arg(style.toolbarBackgroundColor.red()).arg(style.toolbarBackgroundColor.green())
        .arg(style.toolbarBackgroundColor.blue()).arg(style.toolbarBackgroundColor.alpha())
        .arg(style.borderRadius)
        .arg(style.toolbarBorderColor.red()).arg(style.toolbarBorderColor.green())
        .arg(style.toolbarBorderColor.blue()).arg(style.toolbarBorderColor.alpha())
        .arg(style.textColor.red()).arg(style.textColor.green()).arg(style.textColor.blue())
        .arg(style.buttonPadding).arg(style.buttonPadding)
        .arg(style.fontSize).arg(style.fontFamily)
        .arg(style.buttonHoverColor.red()).arg(style.buttonHoverColor.green())
        .arg(style.buttonHoverColor.blue()).arg(style.buttonHoverColor.alpha())
        .arg(style.buttonCheckedColor.red()).arg(style.buttonCheckedColor.green())
        .arg(style.buttonCheckedColor.blue()).arg(style.buttonCheckedColor.alpha());
}

OverlayStyle OverlayStyleManager::createDarkTheme()
{
    OverlayStyle style;
    style.toolbarBackgroundColor = QColor(50, 50, 50, 230);
    style.toolbarBorderColor = QColor(100, 100, 100, 100);
    style.toolbarHeaderColor = QColor(70, 70, 70, 200);
    style.buttonColor = QColor(255, 255, 255);
    style.buttonHoverColor = QColor(100, 100, 100, 150);
    style.buttonCheckedColor = QColor(0, 120, 212, 200);
    style.textColor = QColor(255, 255, 255);
    style.defaultPenColor = Qt::red;
    return style;
}

OverlayStyle OverlayStyleManager::createLightTheme()
{
    OverlayStyle style;
    style.toolbarBackgroundColor = QColor(240, 240, 240, 230);
    style.toolbarBorderColor = QColor(180, 180, 180, 150);
    style.toolbarHeaderColor = QColor(220, 220, 220, 200);
    style.buttonColor = QColor(60, 60, 60);
    style.buttonHoverColor = QColor(200, 200, 200, 150);
    style.buttonCheckedColor = QColor(0, 120, 212, 200);
    style.textColor = QColor(60, 60, 60);
    style.defaultPenColor = Qt::blue;
    return style;
}

OverlayStyle OverlayStyleManager::createBlueTheme()
{
    OverlayStyle style;
    style.toolbarBackgroundColor = QColor(25, 35, 80, 230);
    style.toolbarBorderColor = QColor(50, 70, 120, 150);
    style.toolbarHeaderColor = QColor(35, 45, 90, 200);
    style.buttonColor = QColor(220, 230, 255);
    style.buttonHoverColor = QColor(70, 90, 140, 150);
    style.buttonCheckedColor = QColor(0, 150, 255, 200);
    style.textColor = QColor(220, 230, 255);
    style.defaultPenColor = QColor(0, 150, 255);
    return style;
}

OverlayStyle OverlayStyleManager::createGreenTheme()
{
    OverlayStyle style;
    style.toolbarBackgroundColor = QColor(25, 80, 35, 230);
    style.toolbarBorderColor = QColor(50, 120, 70, 150);
    style.toolbarHeaderColor = QColor(35, 90, 45, 200);
    style.buttonColor = QColor(220, 255, 230);
    style.buttonHoverColor = QColor(70, 140, 90, 150);
    style.buttonCheckedColor = QColor(0, 200, 100, 200);
    style.textColor = QColor(220, 255, 230);
    style.defaultPenColor = QColor(0, 180, 80);
    return style;
}




void OverlayWidget::onToolButtonClicked(int toolId)
{
    setDrawingTool(static_cast<DrawingTool>(toolId));
}

void OverlayWidget::onUndoClicked()
{
    undoLastAction();
}

void OverlayWidget::onRedoClicked()
{
    redoLastAction();
}

void OverlayWidget::onClearClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("确认清除"), tr("确定要清除所有标注内容吗？"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        clearCanvas();
    }
}

void OverlayWidget::onSaveClicked()
{
    saveImage();
}

void OverlayWidget::onFinishClicked()
{
    finishEditing();
}

void OverlayWidget::onImportClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("导入标注数据"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("JSON Files (*.json);;All Files (*)")
    );

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QString jsonData = QString::fromUtf8(file.readAll());
            if (importAnnotationData(jsonData)) {
                QMessageBox::information(this, tr("导入成功"), tr("标注数据已成功导入"));
            }
            else {
                QMessageBox::warning(this, tr("导入失败"), tr("无法解析标注数据文件"));
            }
        }
        else {
            QMessageBox::warning(this, tr("导入失败"), tr("无法读取文件"));
        }
    }
}

void OverlayWidget::onExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("导出标注数据"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
        "/annotation_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".json",
        tr("JSON Files (*.json);;All Files (*)")
    );

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            QString jsonData = exportAnnotationData();
            file.write(jsonData.toUtf8());
            QMessageBox::information(this, tr("导出成功"), tr("标注数据已保存到: ") + fileName);
        }
        else {
            QMessageBox::warning(this, tr("导出失败"), tr("无法写入文件"));
        }
    }
}

void OverlayWidget::onConfigSaveClicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("保存配置"),
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
        "/overlay_config.ini",
        tr("Config Files (*.ini);;All Files (*)")
    );

    if (!fileName.isEmpty()) {
        saveConfiguration(fileName);
        QMessageBox::information(this, tr("保存成功"), tr("配置已保存"));
    }
}

void OverlayWidget::onConfigLoadClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("加载配置"),
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation),
        tr("Config Files (*.ini);;All Files (*)")
    );

    if (!fileName.isEmpty()) {
        loadConfiguration(fileName);
        QMessageBox::information(this, tr("加载成功"), tr("配置已加载"));
    }
}

void OverlayWidget::onThemeChanged(int themeIndex)
{
    auto theme = static_cast<OverlayStyleManager::StyleTheme>(themeIndex);
    setStyleTheme(theme);
}

void OverlayWidget::onShowHelpClicked()
{
    showShortcutsHelp();
}

void OverlayWidget::onShowAboutClicked()
{
    QMessageBox::about(this, tr("关于增强标注工具"),
        tr("<h3>增强版Widget遮罩标注工具 v2.0</h3>"
            "<p>这是一个功能强大的Qt标注工具，支持:</p>"
            "<ul>"
            "<li>7种专业绘制工具</li>"
            "<li>零误差缩放系统</li>"
            "<li>完整撤销重做功能</li>"
            "<li>智能工具栏界面</li>"
            "<li>多种主题样式</li>"
            "</ul>"
            "<p>版权所有 © 2024</p>"
            "<p>基于MIT许可证发布</p>"));
}

void OverlayWidget::onToolbarDragBegin()
{
    // 工具栏拖拽开始时的处理
    if (m_toolbar) {
        m_toolbar->setCursor(Qt::ClosedHandCursor);
    }
}

void OverlayWidget::onToolbarDragEnd()
{
    // 工具栏拖拽结束时的处理
    if (m_toolbar) {
        m_toolbar->setCursor(Qt::SizeAllCursor);
        constrainToolbarPosition();
    }
}

void OverlayWidget::onDebugModeToggled(bool enabled)
{
    setDebugMode(enabled);
}

void OverlayWidget::onTestScalingClicked()
{
    testScalingAccuracy();
}

void OverlayWidget::onShowPerformanceStats()
{
    PerformanceMonitor::instance().printStatistics();

    QString statsText = tr("性能统计信息已输出到控制台\n\n");
    statsText += tr("平均绘制时间: %1ms\n").arg(
        PerformanceMonitor::instance().getAverageTime("paintEvent"));
    statsText += tr("平均更新时间: %1ms\n").arg(
        PerformanceMonitor::instance().getAverageTime("updateOverlayGeometry"));

    QMessageBox::information(this, tr("性能统计"), statsText);
}


// RelativePoint 方法实现
QPoint OverlayWidget::RelativePoint::toAbsolute(const QSize& containerSize) const
{
    if (containerSize.isEmpty()) return QPoint();
    return QPoint(qRound(x * containerSize.width()), qRound(y * containerSize.height()));
}

OverlayWidget::RelativePoint OverlayWidget::RelativePoint::fromAbsolute(
    const QPoint& point, const QSize& containerSize, const QColor& color, int width)
{
    RelativePoint relPoint;
    if (!containerSize.isEmpty()) {
        relPoint.x = static_cast<double>(point.x()) / containerSize.width();
        relPoint.y = static_cast<double>(point.y()) / containerSize.height();
    }
    else {
        relPoint.x = relPoint.y = 0.0;
    }
    relPoint.color = color;
    relPoint.width = width;
    return relPoint;
}

// RelativeTextItem 方法实现
QPoint OverlayWidget::RelativeTextItem::toAbsolutePosition(const QSize& containerSize) const
{
    if (containerSize.isEmpty()) return QPoint();
    return QPoint(qRound(x * containerSize.width()), qRound(y * containerSize.height()));
}

QFont OverlayWidget::RelativeTextItem::toAbsoluteFont(const QSize& containerSize) const
{
    QFont font(fontFamily);
    if (!containerSize.isEmpty()) {
        int absoluteFontSize = qMax(6, qRound(relativeFontSize * containerSize.height()));
        font.setPointSize(absoluteFontSize);
    }
    else {
        font.setPointSize(12); // 默认字体大小
    }
    font.setBold(bold);
    font.setItalic(italic);
    return font;
}

OverlayWidget::TextItem OverlayWidget::RelativeTextItem::toAbsolute(const QSize& containerSize) const
{
    TextItem item;
    item.position = toAbsolutePosition(containerSize);
    item.text = text;
    item.color = color;
    item.font = toAbsoluteFont(containerSize);
    return item;
}

OverlayWidget::RelativeTextItem OverlayWidget::RelativeTextItem::fromAbsolute(
    const TextItem& item, const QSize& containerSize)
{
    RelativeTextItem relItem;
    if (!containerSize.isEmpty()) {
        relItem.x = static_cast<double>(item.position.x()) / containerSize.width();
        relItem.y = static_cast<double>(item.position.y()) / containerSize.height();
        relItem.relativeFontSize = static_cast<double>(item.font.pointSize()) / containerSize.height();
    }
    else {
        relItem.x = relItem.y = relItem.relativeFontSize = 0.0;
    }
    relItem.text = item.text;
    relItem.color = item.color;
    relItem.fontFamily = item.font.family();
    relItem.bold = item.font.bold();
    relItem.italic = item.font.italic();
    return relItem;
}


OverlayWidget::ShapeItem OverlayWidget::RelativeShapeItem::toAbsolute(const QSize& containerSize) const
{
    ShapeItem item;
    if (!containerSize.isEmpty()) {
        item.startPoint = QPoint(qRound(startX * containerSize.width()),
            qRound(startY * containerSize.height()));
        item.endPoint = QPoint(qRound(endX * containerSize.width()),
            qRound(endY * containerSize.height()));
    }
    item.type = type;
    item.color = color;
    item.width = width;
    item.filled = filled;
    item.arrowSize = arrowSize;
    return item;
}

OverlayWidget::RelativeShapeItem OverlayWidget::RelativeShapeItem::fromAbsolute(
    const ShapeItem& item, const QSize& containerSize)
{
    RelativeShapeItem relItem;
    if (!containerSize.isEmpty()) {
        relItem.startX = static_cast<double>(item.startPoint.x()) / containerSize.width();
        relItem.startY = static_cast<double>(item.startPoint.y()) / containerSize.height();
        relItem.endX = static_cast<double>(item.endPoint.x()) / containerSize.width();
        relItem.endY = static_cast<double>(item.endPoint.y()) / containerSize.height();
    }
    else {
        relItem.startX = relItem.startY = relItem.endX = relItem.endY = 0.0;
    }
    relItem.type = item.type;
    relItem.color = item.color;
    relItem.width = item.width;
    relItem.filled = item.filled;
    relItem.arrowSize = item.arrowSize;
    return relItem;
}


bool OverlayWidget::ErasedData::isEmpty() const
{
    return erasedPathIndices.isEmpty() &&
        erasedTextIndices.isEmpty() &&
        erasedShapeIndices.isEmpty();
}