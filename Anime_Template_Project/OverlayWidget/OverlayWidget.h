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

class OverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OverlayWidget(QWidget* targetWidget, QWidget* parent = nullptr);
    ~OverlayWidget();

    void showOverlay();
    void hideOverlay();

    // 相对坐标系统控制接口
    void setUseRelativeCoordinates(bool enabled);
    void setDebugMode(bool enabled);
    void setHighPrecisionMode(bool enabled);
    void setTargetMargins(const QMargins& margins);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void changePenColor();
    void changePenWidth(int width);
    void changeFontSize(int size);
    void toggleTextMode(bool enabled);
    void toggleEraserMode(bool enabled);
    void changeEraserSize(int size);
    void clearCanvas();
    void saveImage();
    void finishEditing();
    void undoLastAction();
    void redoLastAction();
    void testScalingAccuracy();  // 测试缩放精度

private:
    enum ActionType {
        ACTION_DRAW_PATH,
        ACTION_ADD_TEXT,
        ACTION_EDIT_TEXT,
        ACTION_DELETE_TEXT,
        ACTION_ERASE
    };

    // 传统绝对坐标结构（用于显示）
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

    // 相对坐标结构（用于存储和缩放）
    struct RelativePoint {
        double x, y;  // 相对坐标 (0.0-1.0)
        QColor color;
        int width;

        // 转换为绝对坐标
        QPoint toAbsolute(const QSize& containerSize) const {
            if (containerSize.isEmpty()) return QPoint(0, 0);
            int absX = qRound(x * containerSize.width());
            int absY = qRound(y * containerSize.height());
            return QPoint(absX, absY);
        }

        // 从绝对坐标创建
        static RelativePoint fromAbsolute(const QPoint& point, const QSize& containerSize,
            const QColor& color = Qt::black, int width = 1) {
            RelativePoint rp;
            if (containerSize.width() > 0 && containerSize.height() > 0) {
                rp.x = static_cast<double>(point.x()) / containerSize.width();
                rp.y = static_cast<double>(point.y()) / containerSize.height();
            }
            else {
                rp.x = 0.0;
                rp.y = 0.0;
            }
            rp.color = color;
            rp.width = width;
            return rp;
        }
    };

    struct RelativeTextItem {
        double x, y;                // 位置相对坐标 (0.0-1.0)
        QString text;
        QColor color;
        QString fontFamily;
        double relativeFontSize;    // 字体大小相对于容器高度的比例
        bool bold;
        bool italic;

        // 转换为绝对坐标和字体
        QPoint toAbsolutePosition(const QSize& containerSize) const {
            if (containerSize.isEmpty()) return QPoint(0, 0);
            int absX = qRound(x * containerSize.width());
            int absY = qRound(y * containerSize.height());
            return QPoint(absX, absY);
        }

        QFont toAbsoluteFont(const QSize& containerSize) const {
            int fontSize = 12;  // 默认字体大小
            if (containerSize.height() > 0) {
                fontSize = qMax(6, qRound(relativeFontSize * containerSize.height()));
            }

            QFont font(fontFamily, fontSize);
            font.setBold(bold);
            font.setItalic(italic);
            return font;
        }

        TextItem toAbsolute(const QSize& containerSize) const {
            TextItem item;
            item.position = toAbsolutePosition(containerSize);
            item.text = text;
            item.color = color;
            item.font = toAbsoluteFont(containerSize);
            return item;
        }

        // 从绝对坐标和字体创建
        static RelativeTextItem fromAbsolute(const TextItem& item, const QSize& containerSize) {
            RelativeTextItem relItem;

            if (containerSize.width() > 0 && containerSize.height() > 0) {
                relItem.x = static_cast<double>(item.position.x()) / containerSize.width();
                relItem.y = static_cast<double>(item.position.y()) / containerSize.height();
                relItem.relativeFontSize = static_cast<double>(item.font.pointSize()) / containerSize.height();
            }
            else {
                relItem.x = 0.0;
                relItem.y = 0.0;
                relItem.relativeFontSize = 0.02; // 默认2%的高度
            }

            relItem.text = item.text;
            relItem.color = item.color;
            relItem.fontFamily = item.font.family();
            relItem.bold = item.font.bold();
            relItem.italic = item.font.italic();

            return relItem;
        }
    };

    // 橡皮擦删除的数据结构
    struct ErasedData {
        QVector<int> erasedPathIndices;
        QVector<QVector<DrawPoint>> erasedPaths;
        QVector<int> erasedTextIndices;
        QVector<TextItem> erasedTexts;

        bool isEmpty() const {
            return erasedPaths.isEmpty() && erasedTexts.isEmpty();
        }
    };

    struct UndoAction {
        ActionType type;
        QVector<DrawPoint> pathData;
        TextItem textData;
        int textIndex;
        QString oldText;
        QString newText;
        QColor oldColor;
        QColor newColor;
        ErasedData erasedData;
    };

    // 相对坐标系统相关函数
    void initializeRelativeSystem();
    void convertToRelativeCoordinates();
    void updateAbsoluteFromRelative();
    void syncRelativeData();
    RelativePoint pointToRelative(const QPoint& point) const;
    QPoint relativeToPoint(const RelativePoint& relativePoint) const;

    // UI设置和位置更新
    void setupUI();
    void updatePosition();
    void calculatePreciseGeometry();
    void updateOverlayGeometry();
    QRect getTargetWidgetGlobalRect() ;
    QPoint getTargetWidgetGlobalPosition() const;
    void handleGeometryChange();
    bool isGeometryChanged() const;
    void scaleContent(const QSize& oldSize, const QSize& newSize);
    void installEventFilters();
    void removeEventFilters();

    // 绘制相关函数
    void drawPaths(QPainter& painter);
    void drawTexts(QPainter& painter);
    void drawEraserCursor(QPainter& painter);
    void drawDebugInfo(QPainter& painter);

    // 文字相关函数
    void addTextAt(const QPoint& position);
    void editTextAt(int index, const QPoint& position);
    void finishTextInput();

    // 橡皮擦相关函数
    void performErase(const QPoint& position);
    bool isPointInEraserRange(const QPoint& point, const QPoint& eraserCenter);
    bool isTextInEraserRange(const TextItem& textItem, const QPoint& eraserCenter);
    QCursor createEraserCursor();

    // 撤销/重做相关函数
    void saveAction(ActionType type, const QVector<DrawPoint>& pathData = QVector<DrawPoint>(),
        const TextItem& textData = TextItem(), int textIndex = -1,
        const QString& oldText = QString(), const QString& newText = QString(),
        const QColor& oldColor = QColor(), const QColor& newColor = QColor(),
        const ErasedData& erasedData = ErasedData());
    void updateUndoRedoButtons();
    void clearRedoStack();

    // 工具栏相关函数
    void toggleToolbarCollapse();
    void updateToolbarLayout();
    void constrainToolbarPosition();

    // 调试和测试函数
    void debugRelativeCoordinates() const;
    void validateCoordinateConsistency();
    bool isValidPosition(const QPoint& pos) const;

    // 目标widget相关
    QWidget* m_targetWidget;
    QSize m_lastTargetSize;
    QRect m_lastTargetGeometry;
    QPoint m_targetWidgetOffset;
    bool m_geometryUpdatePending;
    QTimer* m_updateTimer;

    // 相对坐标系统
    QVector<QVector<RelativePoint>> m_relativePaths;
    QVector<RelativePoint> m_currentRelativePath;
    QVector<RelativeTextItem> m_relativeTextItems;
    QSize m_baseSize;
    bool m_baseSizeInitialized;
    bool m_useRelativeCoordinates;

    // 传统绝对坐标（用于显示和兼容）
    QVector<QVector<DrawPoint>> m_paths;
    QVector<DrawPoint> m_currentPath;
    QColor m_penColor;
    int m_penWidth;
    bool m_drawing;

    // 文字相关
    QVector<TextItem> m_textItems;
    QLineEdit* m_textEdit;
    int m_fontSize;
    bool m_textMode;
    QPoint m_currentTextPosition;
    int m_editingTextIndex;

    // 当前编辑文字的相对坐标
    RelativeTextItem m_currentEditingRelativeText;
    bool m_hasEditingRelativeText;

    // 橡皮擦相关
    bool m_eraserMode;
    int m_eraserSize;
    bool m_erasing;
    QPoint m_lastErasePos;
    QPoint m_currentMousePos;
    ErasedData m_currentErasedData;

    // 工具栏
    QWidget* m_toolbar;
    QWidget* m_toolbarHeader;
    QWidget* m_toolbarContent;
    QPushButton* m_collapseButton;
    QPushButton* m_colorButton;
    QSpinBox* m_widthSpinBox;
    QSpinBox* m_fontSizeSpinBox;
    QCheckBox* m_textModeCheckBox;
    QCheckBox* m_eraserModeCheckBox;
    QSpinBox* m_eraserSizeSpinBox;
    QPushButton* m_clearButton;
    QPushButton* m_saveButton;
    QPushButton* m_finishButton;
    QPushButton* m_undoButton;
    QPushButton* m_redoButton;

    // 工具栏状态
    bool m_toolbarCollapsed;
    bool m_draggingToolbar;
    QPoint m_dragStartPos;
    QPoint m_toolbarDragOffset;

    // 撤销/重做相关
    QVector<UndoAction> m_undoStack;
    QVector<UndoAction> m_redoStack;
    static const int MAX_UNDO_STEPS = 50;

    // 调试和优化相关
    bool m_debugMode;
    QMargins m_targetMargins;
    bool m_useHighPrecision;
    mutable QRect m_cachedTargetRect;
    mutable bool m_rectCacheValid;
    int m_updateCount;
};

#endif // OVERLAYWIDGET_H