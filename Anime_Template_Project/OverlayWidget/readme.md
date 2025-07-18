# 增强版 Widget 遮罩标注工具 - 完整技术文档

## 🌟 项目概述

这是一个基于 Qt 的专业级 Widget 遮罩标注工具，在原有功能基础上新增了完整的几何图形标注支持，提供了丰富的绘制工具、智能的用户界面和强大的功能扩展。

### 核心特性
- **7种专业工具** - 自由绘制、直线、矩形、椭圆、箭头、文字、橡皮擦
- **零误差缩放** - 相对坐标系统确保窗口变化时标注完美跟随
- **完整撤销重做** - 支持所有操作类型，最多50步历史记录
- **智能工具栏** - 三行布局，可拖拽、可收起、智能属性面板
- **精确橡皮擦** - 自定义光标，精确预览，智能边界检测
- **样式系统** - 4种预设主题，完全可定制外观
- **快捷键支持** - 完整的键盘快捷键系统
- **数据导入导出** - JSON格式保存和加载标注数据

## 🎯 新增功能详解

### 几何图形工具

#### 📏 直线工具
- **功能**: 绘制精确的直线段
- **操作**: 按下起点 → 拖拽到终点 → 释放完成
- **应用**: 测量标注、连接线、分割线

#### ⬜ 矩形工具
- **功能**: 绘制矩形和正方形
- **模式**: 支持空心和填充两种模式
- **操作**: 对角拖拽定义矩形区域
- **应用**: 区域框选、重点标记、遮罩区域

#### ⭕ 椭圆工具
- **功能**: 绘制椭圆和圆形
- **模式**: 支持空心和填充模式
- **操作**: 拖拽定义椭圆边界框
- **应用**: 重点突出、软性边界、装饰图形

#### ➡️ 箭头工具
- **功能**: 绘制带箭头的指示线
- **可调**: 箭头大小5-30像素可调
- **操作**: 从起点拖拽到指向目标
- **应用**: 指向标注、流程指示、方向说明

### 智能用户界面

#### 三行工具栏布局
```
第一行: [工具选择] ✏️📏⬜⭕➡️📝🧽
第二行: [属性设置] 颜色 宽度 填充 箭头 字号 擦除
第三行: [操作按钮] 撤销 重做 清除 保存 完成
```

#### 智能属性面板
- **自适应显示**: 根据选择工具自动启用/禁用相关设置
- **实时反馈**: 设置变更立即在预览中体现
- **状态保持**: 工具切换时保持上次使用的属性值

### 高级功能

#### 相对坐标系统
```cpp
// 零误差缩放核心算法
struct RelativePoint {
    double x, y;  // 0.0-1.0相对坐标
    QPoint toAbsolute(const QSize& size) const {
        return QPoint(qRound(x * size.width()), qRound(y * size.height()));
    }
};
```
- **零累积误差**: 无论多少次缩放都保持精确位置
- **自动适配**: 窗口大小变化时标注自动跟随
- **性能优化**: 智能缓存减少计算开销

#### 精确碰撞检测
```cpp
// 线段与圆形相交检测
bool isLineIntersectCircle(const QPoint& start, const QPoint& end, 
                          const QPoint& center, int radius) {
    // 点到线段最短距离算法
    // 支持椭圆、矩形、箭头等复杂图形
}
```

## 🚀 技术架构

### 核心类设计

```cpp
class OverlayWidget : public QWidget {
    // 绘制工具枚举
    enum DrawingTool {
        TOOL_FREE_DRAW, TOOL_LINE, TOOL_RECTANGLE, 
        TOOL_ELLIPSE, TOOL_ARROW, TOOL_TEXT, TOOL_ERASER
    };
    
    // 几何图形数据结构
    struct ShapeItem {
        ShapeType type;
        QPoint startPoint, endPoint;
        QColor color;
        int width;
        bool filled;
        int arrowSize;
    };
    
    // 相对坐标支持
    struct RelativeShapeItem {
        double startX, startY, endX, endY;
        // 其他属性...
    };
};
```

### 模块化设计

#### 1. 绘制引擎模块
- **drawPaths()** - 自由绘制路径渲染
- **drawTexts()** - 文字标注渲染  
- **drawShapes()** - 几何图形渲染
- **drawPreviewShape()** - 实时预览渲染

#### 2. 事件处理模块
- **鼠标事件处理** - 支持7种不同工具的交互逻辑
- **键盘快捷键** - 完整的快捷键映射系统
- **工具栏交互** - 拖拽、收起、属性设置

#### 3. 数据管理模块
- **双坐标系统** - 绝对坐标+相对坐标并行维护
- **撤销重做栈** - 完整的操作历史管理
- **数据导入导出** - JSON格式序列化

#### 4. 样式配置模块
- **主题管理器** - 4种预设主题+自定义
- **样式生成器** - 动态CSS样式表生成
- **配置持久化** - INI格式配置保存

## 📋 完整API参考

### 核心接口

```cpp
// 工具控制
void setDrawingTool(DrawingTool tool);
void setFillMode(bool enabled);
void changeArrowSize(int size);
void changePenColor(const QColor& color);
void changePenWidth(int width);

// 几何图形管理
int getShapeCount() const;
void clearShapes(ShapeType type);
QVector<ShapeItem> getShapesByType(ShapeType type) const;
void setShapeStyle(int index, const QColor& color, int width, bool filled);
void moveShape(int index, const QPoint& offset);
int hitTestShape(const QPoint& point, int tolerance = 5) const;

// 数据操作
QString exportAnnotationData() const;
bool importAnnotationData(const QString& jsonData);
void saveConfiguration(const QString& filePath);
void loadConfiguration(const QString& filePath);

// 样式和主题
void setStyleTheme(OverlayStyleManager::StyleTheme theme);
void applyCurrentStyle();

// 高级配置
void setUseRelativeCoordinates(bool enabled);
void setDebugMode(bool enabled);
void setHighPrecisionMode(bool enabled);
void setTargetMargins(const QMargins& margins);
```

### 信号接口

```cpp
signals:
    void finished();                    // 标注完成
    void toolChanged(DrawingTool tool); // 工具切换
    void shapeAdded(const ShapeItem& shape);    // 添加图形
    void shapeRemoved(int index);       // 删除图形
    void exportRequested();             // 导出请求
```

## ⌨️ 快捷键完整列表

### 工具切换
| 按键 | 功能 | 按键 | 功能 |
|------|------|------|------|
| `P` | 自由绘制 | `L` | 直线工具 |
| `R` | 矩形工具 | `O` | 椭圆工具 |
| `A` | 箭头工具 | `T` | 文字工具 |
| `E` | 橡皮擦 | `F` | 切换填充 |

### 操作快捷键
| 组合键 | 功能 | 组合键 | 功能 |
|--------|------|--------|------|
| `Ctrl+Z` | 撤销 | `Ctrl+Y` | 重做 |
| `Ctrl+S` | 保存 | `ESC` | 退出标注 |
| `Delete` | 清除所有 | `Space` | 切换工具栏 |

### 颜色快捷键
| 组合键 | 颜色 | 组合键 | 颜色 |
|--------|------|--------|------|
| `Ctrl+1` | 红色 | `Ctrl+2` | 绿色 |
| `Ctrl+3` | 蓝色 | `Ctrl+4` | 黄色 |
| `Ctrl+5` | 品红 | `Ctrl+6` | 青色 |
| `Ctrl+7` | 黑色 | `Ctrl+8` | 白色 |

### 大小调整
| 按键 | 功能 |
|------|------|
| `[` | 减小当前工具大小 |
| `]` | 增大当前工具大小 |
| `滚轮` | 根据工具调整对应大小 |

## 🎨 样式主题系统

### 预设主题

#### 1. 深色主题（默认）
```cpp
OverlayStyleManager::THEME_DARK
```
- 背景：深灰色半透明
- 按钮：白色文字，灰色背景
- 适用：大部分场景，专业外观

#### 2. 浅色主题
```cpp
OverlayStyleManager::THEME_LIGHT  
```
- 背景：浅灰色半透明
- 按钮：深色文字，浅色背景
- 适用：明亮环境，简洁风格

#### 3. 蓝色主题
```cpp
OverlayStyleManager::THEME_BLUE
```
- 背景：深蓝色调
- 按钮：蓝色系配色
- 适用：科技感，专业应用

#### 4. 绿色主题
```cpp
OverlayStyleManager::THEME_GREEN
```
- 背景：深绿色调
- 按钮：绿色系配色
- 适用：自然风格，环保主题

### 自定义主题

```cpp
// 获取样式管理器
OverlayStyle& style = OverlayStyleManager::instance().getStyle();

// 自定义颜色
style.toolbarBackgroundColor = QColor(100, 50, 150, 200);
style.buttonCheckedColor = QColor(200, 100, 50, 180);

// 应用自定义样式
overlay->setStyleTheme(OverlayStyleManager::THEME_CUSTOM);
```

## 📊 性能优化

### 渲染优化
- **分层绘制**: 路径、文字、图形分别优化渲染
- **增量更新**: 只重绘变化区域
- **缓存机制**: 几何计算结果智能缓存
- **反走样控制**: 根据内容类型选择性启用

### 内存管理
- **智能清理**: 撤销栈大小限制，防止内存泄漏
- **对象池**: 重复使用图形对象减少分配
- **延迟初始化**: 按需创建UI组件

### 响应性能
- **事件节流**: 鼠标移动事件智能过滤
- **异步更新**: 几何变换异步处理
- **帧率控制**: 60fps/120fps可选更新频率

### 性能监控

```cpp
// 启用性能监控
PERF_START("paintEvent");
// ... 执行代码 ...
PERF_END("paintEvent");

// 查看统计信息
PerformanceMonitor::instance().printStatistics();
```

## 📦 部署和集成

### 编译要求
- **Qt版本**: Qt 6.0+ （推荐Qt 6.5+）
- **编译器**: 支持C++17的现代编译器
- **CMake**: 3.16+ 或 qmake
- **平台**: Windows、macOS、Linux

### CMake配置示例

```cmake
cmake_minimum_required(VERSION 3.16)
project(EnhancedOverlayWidget)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    main.cpp
    OverlayWidget.cpp
    # 其他源文件...
)

set(HEADERS
    OverlayWidget.h
    # 其他头文件...
)

add_executable(EnhancedOverlayWidget ${SOURCES} ${HEADERS})
target_link_libraries(EnhancedOverlayWidget Qt6::Core Qt6::Widgets)
```

### 快速集成

```cpp
// 1. 基础集成
QLabel* targetWidget = new QLabel();
OverlayWidget* overlay = new OverlayWidget(targetWidget);
overlay->showOverlay();

// 2. 高级配置
overlay->setUseRelativeCoordinates(true);
overlay->setStyleTheme(OverlayStyleManager::THEME_BLUE);
overlay->setDrawingTool(OverlayWidget::TOOL_ARROW);

// 3. 信号连接
connect(overlay, &OverlayWidget::finished, [=]() {
    // 处理标注完成
});
```

## 🔧 故障排除

### 常见问题

#### 1. 工具栏不显示
**原因**: 目标widget尺寸过小或工具栏被遮挡
**解决**: 检查目标widget大小，调用constrainToolbarPosition()

#### 2. 橡皮擦预览不显示
**原因**: 鼠标跟踪未启用或超出有效区域
**解决**: 确保调用setMouseTracking(true)，检查isValidPosition()

#### 3. 缩放后标注位置错误
**原因**: 相对坐标系统未启用或基准尺寸未初始化
**解决**: 调用setUseRelativeCoordinates(true)，检查initializeRelativeSystem()

#### 4. 撤销重做功能异常
**原因**: 操作未正确保存到撤销栈
**解决**: 确保每个操作都调用saveAction()

### 调试工具

```cpp
// 启用调试模式
overlay->setDebugMode(true);

// 查看详细信息
overlay->debugRelativeCoordinates();
overlay->validateCoordinateConsistency();

// 性能分析
PerformanceMonitor::instance().printStatistics();
```

## 🚀 未来扩展

### 计划功能
- **多边形工具** - 任意多边形绘制
- **贝塞尔曲线** - 平滑曲线工具
- **图层管理** - 分层组织标注内容
- **协作标注** - 多用户实时协作
- **AI辅助** - 智能图形识别和标注建议

### 技术改进
- **GPU加速** - OpenGL渲染提升性能
- **矢量导出** - SVG、PDF格式支持
- **插件系统** - 可扩展的工具插件架构
- **云端同步** - 标注数据云端存储

## 📄 许可证

本项目采用MIT许可证，允许商业和非商业用途。

## 🤝 贡献指南

欢迎提交Issue和Pull Request！请确保：
1. 代码符合项目风格
2. 添加适当的测试
3. 更新相关文档
4. 提交详细的变更说明

---

**增强版Widget遮罩标注工具** - 让标注工作更专业、更高效！