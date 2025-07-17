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

class OverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OverlayWidget(QWidget* targetWidget, QWidget* parent = nullptr);
    ~OverlayWidget();

    void showOverlay();
    void hideOverlay();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void enterEvent(QEvent* event) override;  // 新增：鼠标进入事件
    void leaveEvent(QEvent* event) override;       // 新增：鼠标离开事件
    void wheelEvent(QWheelEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void changePenColor();
    void changePenWidth(int width);
    void changeFontSize(int size);
    void toggleTextMode(bool enabled);
    void toggleEraserMode(bool enabled);  // 新增：切换橡皮擦模式
    void changeEraserSize(int size);      // 新增：改变橡皮擦大小
    void clearCanvas();
    void saveImage();
    void finishEditing();
    void undoLastAction();  // 撤销上次操作
    void redoLastAction();  // 重做上次操作

private:
    enum ActionType {
        ACTION_DRAW_PATH,     // 绘制路径
        ACTION_ADD_TEXT,      // 添加文字
        ACTION_EDIT_TEXT,     // 编辑文字
        ACTION_DELETE_TEXT,   // 删除文字
        ACTION_ERASE          // 新增：橡皮擦操作
    };

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

    // 新增：橡皮擦删除的数据结构
    struct ErasedData {
        QVector<int> erasedPathIndices;           // 被删除的路径索引
        QVector<QVector<DrawPoint>> erasedPaths;  // 被删除的路径数据
        QVector<int> erasedTextIndices;           // 被删除的文字索引
        QVector<TextItem> erasedTexts;            // 被删除的文字数据
    };

    struct UndoAction {
        ActionType type;
        QVector<DrawPoint> pathData;  // 路径数据（用于绘制撤销）
        TextItem textData;            // 文字数据（用于文字撤销）
        int textIndex;                // 文字索引（用于编辑/删除撤销）
        QString oldText;              // 原文字内容（用于编辑撤销）
        QString newText;              // 新文字内容（用于重做）
        QColor oldColor;              // 原文字颜色（用于编辑撤销）
        QColor newColor;              // 新文字颜色（用于重做）
        ErasedData erasedData;        // 新增：橡皮擦删除的数据
    };

    void setupUI();
    void updatePosition();
    void scaleContent(const QSize& oldSize, const QSize& newSize);
    void installEventFilters();
    void removeEventFilters();
    void drawPaths(QPainter& painter);
    void drawTexts(QPainter& painter);
    void drawEraserCursor(QPainter& painter);  // 新增：绘制橡皮擦光标
    void addTextAt(const QPoint& position);
    void editTextAt(int index, const QPoint& position);
    void finishTextInput();

    // 新增：橡皮擦相关函数
    void performErase(const QPoint& position);
    bool isPointInEraserRange(const QPoint& point, const QPoint& eraserCenter);
    bool isTextInEraserRange(const TextItem& textItem, const QPoint& eraserCenter);
    QCursor createEraserCursor();  // 新增：创建橡皮擦光标

    // 撤销/重做相关函数
    void saveAction(ActionType type, const QVector<DrawPoint>& pathData = QVector<DrawPoint>(),
        const TextItem& textData = TextItem(), int textIndex = -1,
        const QString& oldText = QString(), const QString& newText = QString(),
        const QColor& oldColor = QColor(), const QColor& newColor = QColor(),
        const ErasedData& erasedData = ErasedData());  // 新增橡皮擦数据参数
    void updateUndoRedoButtons();  // 更新撤销/重做按钮状态
    void clearRedoStack();         // 清空重做栈

    // 工具栏相关函数
    void toggleToolbarCollapse();  // 切换工具栏收起状态
    void updateToolbarLayout();    // 更新工具栏布局
    void constrainToolbarPosition(); // 限制工具栏位置在遮罩内

    // 目标widget
    QWidget* m_targetWidget;
    QSize m_lastTargetSize;

    // 绘制相关
    QVector<QVector<DrawPoint>> m_paths;
    QVector<DrawPoint> m_currentPath;
    QColor m_penColor;
    int m_penWidth;
    bool m_drawing;

    // 文字相关
    QVector<TextItem> m_textItems;
    QLineEdit* m_textEdit;
    int m_fontSize;             // 字体大小
    bool m_textMode;
    QPoint m_currentTextPosition;
    int m_editingTextIndex;

    // 新增：橡皮擦相关
    bool m_eraserMode;            // 橡皮擦模式
    int m_eraserSize;             // 橡皮擦大小
    bool m_erasing;               // 是否正在擦除
    QPoint m_lastErasePos;        // 上次擦除位置（用于连续擦除）
    QPoint m_currentMousePos;     // 当前鼠标位置（用于橡皮擦光标）
    ErasedData m_currentErasedData; // 当前擦除操作的数据

    // 工具栏
    QWidget* m_toolbar;
    QWidget* m_toolbarHeader;      // 工具栏标题栏（拖动区域）
    QWidget* m_toolbarContent;     // 工具栏内容区域
    QPushButton* m_collapseButton; // 收起/展开按钮
    QPushButton* m_colorButton;
    QSpinBox* m_widthSpinBox;      // 画笔大小选择
    QSpinBox* m_fontSizeSpinBox;      // 画笔大小选择
    QCheckBox* m_textModeCheckBox;
    QCheckBox* m_eraserModeCheckBox;  // 新增：橡皮擦模式复选框
    QSpinBox* m_eraserSizeSpinBox;    // 新增：橡皮擦大小选择
    QPushButton* m_clearButton;
    QPushButton* m_saveButton;
    QPushButton* m_finishButton;
    QPushButton* m_undoButton;
    QPushButton* m_redoButton;

    // 工具栏状态
    bool m_toolbarCollapsed;       // 工具栏是否收起
    bool m_draggingToolbar;        // 是否正在拖动工具栏
    QPoint m_dragStartPos;         // 拖动开始位置
    QPoint m_toolbarDragOffset;    // 拖动偏移量

    // 撤销/重做相关
    QVector<UndoAction> m_undoStack;   // 撤销栈
    QVector<UndoAction> m_redoStack;   // 重做栈
    static const int MAX_UNDO_STEPS = 50;  // 最大撤销步数
};

#endif // OVERLAYWIDGET_H