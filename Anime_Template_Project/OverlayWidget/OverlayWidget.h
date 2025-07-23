#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QPixmap>
#include <QTimer>
#include <QFontMetrics>
#include <QEvent>
#include <QRegularExpression>
#include <QButtonGroup>
#include <QComboBox>
#include <QSlider>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

// =============================================================================
// 样式管理系统
// =============================================================================

struct OverlayStyle {
    // 工具栏样式
    QColor toolbarBackgroundColor;
    QColor toolbarBorderColor;
    QColor toolbarHeaderColor;
    QColor buttonColor;
    QColor buttonHoverColor;
    QColor buttonCheckedColor;
    QColor textColor;
    QString fontFamily;
    int fontSize;
    int borderRadius;
    int buttonPadding;

    // 绘制样式
    QColor defaultPenColor;
    int defaultPenWidth;
    int defaultFontSize;
    int defaultArrowSize;
    int defaultEraserSize;

    // 预览样式
    QColor previewColor;
    Qt::PenStyle previewStyle;
    int previewAlpha;

    OverlayStyle() {
        // 默认深色主题
        toolbarBackgroundColor = QColor(50, 50, 50, 230);
        toolbarBorderColor = QColor(100, 100, 100, 100);
        toolbarHeaderColor = QColor(70, 70, 70, 200);
        buttonColor = QColor(255, 255, 255);
        buttonHoverColor = QColor(100, 100, 100, 150);
        buttonCheckedColor = QColor(0, 120, 212, 200);
        textColor = QColor(255, 255, 255);
        fontFamily = "Microsoft YaHei";
        fontSize = 11;
        borderRadius = 6;
        buttonPadding = 4;

        defaultPenColor = Qt::red;
        defaultPenWidth = 3;
        defaultFontSize = 12;
        defaultArrowSize = 10;
        defaultEraserSize = 20;

        previewColor = Qt::gray;
        previewStyle = Qt::DashLine;
        previewAlpha = 128;
    }
};

class OverlayStyleManager
{
public:
    static OverlayStyleManager& instance() {
        static OverlayStyleManager instance;
        return instance;
    }

    enum StyleTheme {
        THEME_DARK,     // 深色主题（默认）
        THEME_LIGHT,    // 浅色主题
        THEME_BLUE,     // 蓝色主题
        THEME_GREEN,    // 绿色主题
        THEME_CUSTOM    // 自定义主题
    };

    void setTheme(StyleTheme theme);
    const OverlayStyle& getStyle() const { return currentStyle; }
    OverlayStyle& getStyle() { return currentStyle; }
    StyleTheme getCurrentTheme() const { return currentTheme; }
    QString generateToolbarStyleSheet() const;

private:
    OverlayStyle currentStyle;
    StyleTheme currentTheme = THEME_DARK;

    OverlayStyle createDarkTheme();
    OverlayStyle createLightTheme();
    OverlayStyle createBlueTheme();
    OverlayStyle createGreenTheme();
};

// =============================================================================
// 主要的OverlayWidget类
// =============================================================================

class OverlayWidget : public QWidget
{
    Q_OBJECT

public:
    // =============================================================================
    // 公共枚举和结构体
    // =============================================================================

    // 绘制工具类型
    enum DrawingTool {
        TOOL_FREE_DRAW = 0,  // 自由绘制
        TOOL_LINE,           // 直线
        TOOL_RECTANGLE,      // 矩形
        TOOL_ELLIPSE,        // 椭圆
        TOOL_ARROW,          // 箭头
        TOOL_TEXT,           // 文字
        TOOL_ERASER          // 橡皮擦
    };

    // 几何图形类型
    enum ShapeType {
        SHAPE_LINE,
        SHAPE_RECTANGLE,
        SHAPE_ELLIPSE,
        SHAPE_ARROW
    };

    // =============================================================================
    // 构造函数和析构函数
    // =============================================================================

    explicit OverlayWidget(QWidget* targetWidget, QWidget* parent = nullptr);
    ~OverlayWidget();

    // =============================================================================
    // 核心功能接口
    // =============================================================================

    // 显示和隐藏
    void showOverlay();
    void hideOverlay();
    void finishEditing();

    // 工具控制
    void setDrawingTool(DrawingTool tool);
    DrawingTool getCurrentTool() const { return m_currentTool; }

    // 属性设置
    void setPenColor(const QColor& color);
    void setPenWidth(int width);
    void setFontSize(int size);
    void setFillMode(bool enabled);
    void setArrowSize(int size);
    void setEraserSize(int size);

    // 获取当前属性
    QColor getPenColor() const { return m_penColor; }
    int getPenWidth() const { return m_penWidth; }
    int getFontSize() const { return m_fontSize; }
    bool getFillMode() const { return m_fillMode; }
    int getArrowSize() const { return m_arrowSize; }
    int getEraserSize() const { return m_eraserSize; }

    // =============================================================================
    // 数据管理接口
    // =============================================================================

    // 数据操作
    void clearCanvas();
    void optimizePerformance();
    void enableAutoSave(int intervalSeconds);
    void undoLastAction();
    void redoLastAction();
    bool canUndo() const { return !m_undoStack.isEmpty(); }
    bool canRedo() const { return !m_redoStack.isEmpty(); }

    // 数据导入导出
    QString exportAnnotationData() const;
    bool importAnnotationData(const QString& jsonData);
    void saveImage();

    // 统计信息
    int getPathCount() const { return m_paths.size(); }
    int getTextCount() const { return m_textItems.size(); }
    int getShapeCount() const { return m_shapes.size(); }

    // =============================================================================
    // 配置和样式接口
    // =============================================================================

    // 相对坐标系统
    void setUseRelativeCoordinates(bool enabled);
    bool getUseRelativeCoordinates() const { return m_useRelativeCoordinates; }

    // 调试和性能
    void setDebugMode(bool enabled);
    void setHighPrecisionMode(bool enabled);
    void setTargetMargins(const QMargins& margins);

    // 样式设置
    void setStyleTheme(OverlayStyleManager::StyleTheme theme);
    void applyCurrentStyle();

    // 配置保存和加载
    void saveConfiguration(const QString& filePath = "");
    void loadConfiguration(const QString& filePath = "");

    // =============================================================================
    // 高级功能接口
    // =============================================================================

    // 几何图形操作
    void moveShape(int shapeIndex, const QPoint& offset);
    void scaleShape(int shapeIndex, float scaleX, float scaleY, const QPoint& center);
    int hitTestShape(const QPoint& point, int tolerance = 5);

    // 测试和调试
    void testScalingAccuracy();
    void debugRelativeCoordinates() const;
    void validateCoordinateConsistency();

    // 帮助功能
    void showShortcutsHelp();
    void addHelpButton();
    //void showAboutDialog();

signals:
    // 核心信号
    void finished();
    void toolChanged(DrawingTool tool);
    void annotationModified();

    // 数据变化信号
    void pathAdded();
    void textAdded();
    void shapeAdded();
    void contentCleared();

    // 操作信号
    void undoPerformed();
    void redoPerformed();
    void exportRequested();

protected:
    // =============================================================================
    // 事件处理
    // =============================================================================

    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void changeEvent(QEvent* e)override;

private slots:
    // =============================================================================
    // UI控制槽函数
    // =============================================================================

    // 工具切换
    void onToolButtonClicked(int toolId);

    // 属性设置
    void changePenColor();
    void changePenColorTo(const QColor& color);
    void changePenWidth(int width);
    void changeFontSize(int size);
    void toggleTextMode(bool enabled);
    void changeArrowSize(int size);
    void toggleEraserMode(bool enabled);
    void changeEraserSize(int size);
    void toggleFillMode(bool enabled);

    // 操作按钮
    void onUndoClicked();
    void onRedoClicked();
    void onClearClicked();
    void onSaveClicked();
    void onFinishClicked();

    // 高级功能
    void onImportClicked();
    void onExportClicked();
    void onConfigSaveClicked();
    void onConfigLoadClicked();
    void onThemeChanged(int themeIndex);
    void onShowHelpClicked();
    void onShowAboutClicked();

    // 工具栏管理
    void toggleToolbarCollapse();
    void onToolbarDragBegin();
    void onToolbarDragEnd();

    // 调试功能
    void onDebugModeToggled(bool enabled);
    void onTestScalingClicked();
    void onShowPerformanceStats();

private:
    // =============================================================================
    // 内部数据结构
    // =============================================================================

    struct DrawPoint {
        QPoint point;
        QColor color;
        int width;
    };

    struct TextItem {
        QPoint position;
        QString text;
        QColor color;
        QFont font;
    };

    struct ShapeItem {
        ShapeType type;
        QPoint startPoint;
        QPoint endPoint;
        QColor color;
        int width;
        bool filled;
        int arrowSize;

        ShapeItem() : type(SHAPE_LINE), filled(false), arrowSize(10) {}
    };

    // 相对坐标结构
    struct RelativePoint {
        double x, y;
        QColor color;
        int width;

        QPoint toAbsolute(const QSize& containerSize) const;
        static RelativePoint fromAbsolute(const QPoint& point, const QSize& containerSize,
            const QColor& color = Qt::black, int width = 1);
    };

    struct RelativeTextItem {
        double x, y;
        QString text;
        QColor color;
        QString fontFamily;
        double relativeFontSize;
        bool bold;
        bool italic;

        QPoint toAbsolutePosition(const QSize& containerSize) const;
        QFont toAbsoluteFont(const QSize& containerSize) const;
        TextItem toAbsolute(const QSize& containerSize) const;
        static RelativeTextItem fromAbsolute(const TextItem& item, const QSize& containerSize);
    };

    struct RelativeShapeItem {
        ShapeType type;
        double startX, startY;
        double endX, endY;
        QColor color;
        int width;
        bool filled;
        int arrowSize;

        ShapeItem toAbsolute(const QSize& containerSize) const;
        static RelativeShapeItem fromAbsolute(const ShapeItem& item, const QSize& containerSize);
    };

    // 撤销重做相关
    enum ActionType {
        ACTION_DRAW_PATH,
        ACTION_ADD_TEXT,
        ACTION_EDIT_TEXT,
        ACTION_DELETE_TEXT,
        ACTION_ERASE,
        ACTION_ADD_SHAPE
    };

    struct ErasedData {
        QVector<int> erasedPathIndices;
        QVector<QVector<DrawPoint>> erasedPaths;
        QVector<int> erasedTextIndices;
        QVector<TextItem> erasedTexts;
        QVector<int> erasedShapeIndices;
        QVector<ShapeItem> erasedShapes;

        bool isEmpty() const;
    };

    struct UndoAction {
        ActionType type;
        QVector<DrawPoint> pathData;
        TextItem textData;
        ShapeItem shapeData;
        int textIndex;
        QString oldText;
        QString newText;
        QColor oldColor;
        QColor newColor;
        ErasedData erasedData;
    };

    // 主题相关
    struct ThemeItem {
        QString key;
        QString text;
        QVariant data;
    };

    // =============================================================================
    // UI管理函数
    // =============================================================================

    void setupUI();
    void setupToolbarHeader();
    //void setupToolbar();
    void setupToolButtons();
    QWidget* createToolButtonsWidget();
    void setDrawingTool(int toolType);
    void setupAttributeControls();
    QHBoxLayout* createAttributeControlsLayout();
    void setupActionButtons();
    QHBoxLayout* createActionButtonsLayout();
    void setupAdvancedControls();
    QHBoxLayout* createAdvancedControlsLayout();
    void setupDebugControls();
    QHBoxLayout* createDebugControlsLayout();
    void updateToolbarLayout();
    void updateToolButtonStates();
    void updateToolTips();
    void constrainToolbarPosition();

    // =============================================================================
    // 绘制相关函数
    // =============================================================================

    void drawPaths(QPainter& painter);
    void drawTexts(QPainter& painter);
    void drawShapes(QPainter& painter);
    void drawPreviewShape(QPainter& painter);
    void drawEraserCursor(QPainter& painter);
    void drawDebugInfo(QPainter& painter);

    // 几何图形绘制
    void drawLine(QPainter& painter, const ShapeItem& shape);
    void drawRectangle(QPainter& painter, const ShapeItem& shape);
    void drawEllipse(QPainter& painter, const ShapeItem& shape);
    void drawArrow(QPainter& painter, const ShapeItem& shape);
    QPolygonF createArrowHead(const QPoint& start, const QPoint& end, int size);

    // =============================================================================
    // 交互处理函数
    // =============================================================================

    void handleTextClick(const QPoint& position);
    void addTextAt(const QPoint& position);
    void editTextAt(int index, const QPoint& position);
    void finishTextInput();

    // 橡皮擦功能
    void performErase(const QPoint& position);
    bool isShapeInEraserRange(const ShapeItem& shape, const QPoint& eraserCenter);
    bool isPointInEraserRange(const QPoint& point, const QPoint& eraserCenter);
    bool isTextInEraserRange(const TextItem& textItem, const QPoint& eraserCenter);
    bool isLineIntersectCircle(const QPoint& lineStart, const QPoint& lineEnd,
        const QPoint& circleCenter, int radius);
    QCursor createEraserCursor();

    // =============================================================================
    // 位置和几何管理
    // =============================================================================

    void updatePosition();
    void calculatePreciseGeometry();
    void updateOverlayGeometry();
    QRect getTargetWidgetGlobalRect();
    QPoint getTargetWidgetGlobalPosition() const;
    void handleGeometryChange();
    bool isGeometryChanged() const;
    void scaleContent(const QSize& oldSize, const QSize& newSize);
    void installEventFilters();
    void removeEventFilters();

    // =============================================================================
    // 相对坐标系统
    // =============================================================================

    void initializeRelativeSystem();
    void convertToRelativeCoordinates();
    void updateAbsoluteFromRelative();
    void syncRelativeData();
    RelativePoint pointToRelative(const QPoint& point) const;
    QPoint relativeToPoint(const RelativePoint& relativePoint) const;

    // =============================================================================
    // 撤销重做系统
    // =============================================================================

    void saveAction(ActionType type, const QVector<DrawPoint>& pathData = QVector<DrawPoint>(),
        const TextItem& textData = TextItem(), int textIndex = -1,
        const QString& oldText = QString(), const QString& newText = QString(),
        const QColor& oldColor = QColor(), const QColor& newColor = QColor(),
        const ErasedData& erasedData = ErasedData(),
        const ShapeItem& shapeData = ShapeItem());
    void updateUndoRedoButtons();
    QString getActionName(ActionType actionType) const;
    bool hasUnsavedChanges() const;
    QString getStatusText() const;
    QString getToolName(DrawingTool tool) const;
    void clearRedoStack();
    void undoEraseAction(const ErasedData& erasedData);
    void redoEraseAction(const ErasedData& erasedData);

    // =============================================================================
    // 实用工具函数
    // =============================================================================

    bool isValidPosition(const QPoint& pos) const;
    QString getDefaultConfigPath() const;
    void emitModificationSignal();


    // =============================================================================
    // 国际化
    // =============================================================================

    void retranslateUi();

    // =============================================================================
    // 成员变量
    // =============================================================================

    // 目标widget相关
    QWidget* m_targetWidget;
    QSize m_lastTargetSize;
    QRect m_lastTargetGeometry;
    QPoint m_targetWidgetOffset;
    bool m_geometryUpdatePending;
    QTimer* m_updateTimer;
    QMargins m_targetMargins;

    // 相对坐标系统
    QVector<QVector<RelativePoint>> m_relativePaths;
    QVector<RelativePoint> m_currentRelativePath;
    QVector<RelativeTextItem> m_relativeTextItems;
    QVector<RelativeShapeItem> m_relativeShapes;
    QSize m_baseSize;
    bool m_baseSizeInitialized;
    bool m_useRelativeCoordinates;

    // 绘制数据（绝对坐标）
    QVector<QVector<DrawPoint>> m_paths;
    QVector<DrawPoint> m_currentPath;
    QVector<TextItem> m_textItems;
    QVector<ShapeItem> m_shapes;

    // 当前工具和属性
    DrawingTool m_currentTool;
    QColor m_penColor;
    int m_penWidth;
    int m_fontSize;
    bool m_fillMode;
    int m_arrowSize;
    int m_eraserSize;

    // 操作状态
    bool m_drawing;
    bool m_drawingShape;
    bool m_textMode;
    bool m_eraserMode;
    bool m_erasing;
    ShapeItem m_currentShape;
    QPoint m_shapeStartPoint;
    QPoint m_currentMousePos;
    QPoint m_lastErasePos;
    ErasedData m_currentErasedData;

    // 文字编辑
    QLineEdit* m_textEdit;
    QPoint m_currentTextPosition;
    int m_editingTextIndex;
    RelativeTextItem m_currentEditingRelativeText;
    bool m_hasEditingRelativeText;

    // 撤销重做
    QVector<UndoAction> m_undoStack;
    QVector<UndoAction> m_redoStack;
    static const int MAX_UNDO_STEPS = 50;

    // =============================================================================
    // UI组件 - 重新整理
    // =============================================================================

    // 主工具栏
    QWidget* m_toolbar;
    QWidget* m_toolbarHeader;
    QWidget* m_toolbarContent;
    QPushButton* m_collapseButton;
    bool m_toolbarCollapsed;
    bool m_draggingToolbar;
    QPoint m_dragStartPos;
    QPoint m_toolbarDragOffset;

    // 标签
    QLabel* m_titleLabel;
    QLabel* m_toolsLabel;
    QLabel* m_attrLabel;
    QLabel* m_widthLabel;
    QLabel* m_arrowLabel;
    QLabel* m_fontLabel;
    QLabel* m_eraserLabel;
    QLabel* m_actionLabel;
    QLabel* m_advancedLabel;
    QLabel* m_themeLabel;
    QLabel* m_debugLabel;


    // 工具选择区域
    QButtonGroup* m_toolButtonGroup;
    QPushButton* m_freeDrawButton;
    QPushButton* m_lineButton;
    QPushButton* m_rectangleButton;
    QPushButton* m_ellipseButton;
    QPushButton* m_arrowButton;
    QPushButton* m_textButton;
    QPushButton* m_eraserButton;

    // 属性控制区域
    QPushButton* m_colorButton;
    QSpinBox* m_widthSpinBox;
    QSpinBox* m_fontSizeSpinBox;
    QSpinBox* m_arrowSizeSpinBox;
    QSpinBox* m_eraserSizeSpinBox;
    QCheckBox* m_fillModeCheckBox;

    // 操作按钮区域
    QPushButton* m_undoButton;
    QPushButton* m_redoButton;
    QPushButton* m_clearButton;
    QPushButton* m_saveButton;
    QPushButton* m_finishButton;

    // 高级功能区域（新增）
    QPushButton* m_importButton;
    QPushButton* m_exportButton;
    QPushButton* m_configSaveButton;
    QPushButton* m_configLoadButton;
    QComboBox* m_themeComboBox;
    QPushButton* m_helpButton;
    QPushButton* m_aboutButton;

    // 调试功能区域（新增）
    QCheckBox* m_debugModeCheckBox;
    QPushButton* m_testScalingButton;
    QPushButton* m_performanceStatsButton;

    // 性能和调试
    bool m_debugMode;
    bool m_useHighPrecision;
    mutable QRect m_cachedTargetRect;
    mutable bool m_rectCacheValid;
    int m_updateCount;
};

#endif // OVERLAYWIDGET_H