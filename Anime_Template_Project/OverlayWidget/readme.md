# Widget遮罩标注工具

一个功能丰富的Widget遮罩工具，可以在任意QWidget上添加透明遮罩层进行绘制、文字标注和橡皮擦擦除，类似Snipaste贴图后的完整标注功能。

## 🆕 最新更新 - 精确橡皮擦

### 核心改进
- **🎯 自定义橡皮擦光标**: 专业的橡皮擦图标，热点精确定位
- **🔧 准确预览反馈**: 红色圆形预览与实际擦除范围完全一致
- **✅ 视觉偏差修复**: 解决了光标位置与擦除位置不一致的问题
- **🚀 用户体验提升**: 精确、直观、专业的橡皮擦工具
- **🛡️ 智能边界检查**: 橡皮擦预览和操作仅在有效区域内生效

### 技术亮点
- 动态生成橡皮擦光标图标
- 基于鼠标点的精确碰撞检测
- 实时预览与鼠标跟踪
- 智能边界检查，确保操作仅在有效区域内执行
- 完整的撤销重做支持

## 功能特性

- **Widget遮罩**: 在指定的widget上覆盖透明遮罩层
- **自由绘制**: 在遮罩上进行鼠标拖拽绘制
- **文字标注**: 点击位置输入文字标注，支持编辑已有文字
- **橡皮擦功能**: 🆕 智能橡皮擦，自定义光标精确指示擦除点，红色预览圆形准确显示擦除范围，智能边界检查保护有效区域
- **颜色选择**: 可以更改画笔和文字颜色  
- **画笔设置**: 可调节画笔宽度(1-20像素)
- **橡皮擦设置**: 🆕 可调节橡皮擦大小(5-50像素)
- **完整撤销重做**: 🆕 支持所有操作的撤销重做，包括橡皮擦
- **保存功能**: 将原widget内容+标注保存为完整图片
- **快捷键支持**: 
  - `ESC`: 退出标注模式
  - `Delete/Backspace`: 清除所有标注内容
  - `Ctrl+Z`: 撤销操作
  - `Ctrl+Y` 或 `Ctrl+Shift+Z`: 重做操作
- **实时预览**: 透明遮罩可以看到下面的widget内容

## 编译和使用

### 编译要求

- Qt 6.0 或更高版本
- CMake 3.16 或更高版本  
- 支持C++17的编译器

## 编译方法

### 使用CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 使用qmake

创建 `OverlayWidget.pro` 文件:

```pro
QT += core widgets
CONFIG += c++17

TARGET = OverlayWidget
TEMPLATE = app

SOURCES += main.cpp OverlayWidget.cpp
HEADERS += OverlayWidget.h
```

然后编译:

```bash
qmake OverlayWidget.pro
make  # Linux/macOS
# 或 nmake (Windows MSVC)
# 或 mingw32-make (Windows MinGW)
```

## 使用方法

### 作为独立应用程序

1. **运行程序**: 启动编译好的可执行文件
2. **加载内容**: 
   - 点击"打开图片"加载图片文件
   - 或点击"显示演示图片"使用内置演示
3. **开始标注**: 点击"开始标注"按钮
4. **标注操作**:
   - **绘制**: 直接拖拽鼠标进行绘制
   - **文字**: 勾选"文字"模式后点击输入文字，点击已有文字可编辑
   - **橡皮擦**: 🆕 勾选"橡皮擦"模式后拖拽鼠标擦除内容
   - 使用工具栏更改颜色、画笔宽度和橡皮擦大小
5. **撤销重做**: 🆕 使用撤销/重做按钮或快捷键恢复操作
6. **完成标注**: 按ESC或点击"完成"按钮退出标注模式
7. **保存结果**: 点击"保存"保存带标注的完整图片

### 集成到现有项目

```cpp
#include "OverlayWidget.h"

// 在你的代码中
QLabel *imageLabel = new QLabel();
imageLabel->setPixmap(somePixmap);

// 创建遮罩
OverlayWidget *overlay = new OverlayWidget(imageLabel, parentWidget);

// 显示遮罩开始标注
overlay->showOverlay();

// 隐藏遮罩
overlay->hideOverlay();
```

## 工具栏功能

工具栏采用可拖动设计，包含标题栏和内容区域，现在支持两行布局：

### 拖动和收起
- **拖动**: 点击标题栏"标注工具"区域可拖拽工具栏到任意位置
- **收起/展开**: 点击右上角"−"/"+"按钮收起或展开工具栏
- **智能约束**: 工具栏始终保持在遮罩范围内

### 第一行：绘制工具
- **颜色**: 点击选择画笔/文字颜色
- **宽度**: 调节画笔宽度(1-20)
- **文字**: 开启后可以点击输入文字，点击已有文字可编辑
- **橡皮擦**: 🆕 开启橡皮擦模式，自定义光标精确指示擦除点，红色圆形预览准确显示擦除范围，智能边界检查保护有效区域
- **擦除**: 🆕 调节橡皮擦大小(5-50像素)

### 第二行：操作按钮
- **撤销**: 撤销上次操作(绘制/文字/橡皮擦)，支持Ctrl+Z快捷键
- **重做**: 重做上次撤销的操作，支持Ctrl+Y或Ctrl+Shift+Z快捷键
- **清除**: 清除所有标注内容
- **保存**: 将原widget内容+标注保存为图片
- **完成**: 退出标注模式

## 🆕 橡皮擦功能详解

### 橡皮擦特性
- **智能擦除**: 自动识别并擦除绘制路径和文字
- **🎯 精确光标**: 自定义橡皮擦光标，准确指示擦除中心点
- **🔧 准确预览**: 红色圆形预览精确显示擦除范围，消除视觉误差
- **🛡️ 边界保护**: 橡皮擦预览和操作仅在有效的targetwidget区域内生效
- **精确控制**: 可调节橡皮擦大小适应不同擦除需求
- **连续擦除**: 支持鼠标拖拽连续擦除操作
- **完整撤销**: 橡皮擦操作完全支持撤销重做

### 橡皮擦使用方法
1. **启用橡皮擦**: 勾选工具栏中的"橡皮擦"复选框
2. **调整大小**: 使用"擦除"数值框调节橡皮擦大小(5-50像素)
3. **开始擦除**: 在需要擦除的区域按下左键并拖拽
4. **擦除模式下**:
   - 🎯 **自定义光标**: 鼠标光标变为橡皮擦图标，精确指示擦除点
   - 🔧 **精确预览**: 红色圆形预览准确显示擦除范围（以鼠标点为圆心）
   - 🛡️ **边界保护**: 预览和操作仅在有效的targetwidget区域内生效
   - 移动时实时显示擦除范围预览
   - 按下并拖拽开始擦除操作
5. **完成擦除**: 释放鼠标完成一次擦除操作
6. **撤销擦除**: 使用撤销按钮或Ctrl+Z恢复被擦除的内容

### 橡皮擦工作原理
- **🎯 精确定位**: 使用自定义橡皮擦光标图标，光标热点准确指示擦除中心
- **🛡️ 边界检查**: 智能检测鼠标位置，确保预览和操作仅在有效区域内执行
- **路径擦除**: 检测橡皮擦范围内的绘制路径点，删除包含这些点的完整路径
- **文字擦除**: 检测橡皮擦区域与文字矩形的交集，删除相交的文字
- **🔧 视觉一致性**: 红色预览圆形与实际擦除范围完全一致，消除视觉误差
- **批量操作**: 一次拖拽可以擦除多条路径和多个文字，作为单个撤销单元

## 技术实现

- **透明遮罩**: 使用 `Qt::WA_TranslucentBackground` 创建透明遮罩
- **位置跟随**: 遮罩自动跟随目标widget的大小和位置
- **鼠标事件**: 处理鼠标按下、移动、释放事件实现绘制和擦除
- **自定义绘制**: 重写 `paintEvent` 在遮罩上绘制标注内容和橡皮擦光标
- **widget渲染**: 使用 `QWidget::render()` 获取目标widget内容
- **合成图片**: 将原widget内容与标注合成为完整图片
- **可拖动工具栏**: 标题栏拖动区域，智能位置约束
- **工具栏状态管理**: 收起/展开状态，双行布局自动调整
- **🆕 双栈撤销重做系统**: 撤销栈+重做栈，支持完整的操作历史管理
- **🆕 智能分支清空**: 新操作时自动清空重做历史，保持逻辑一致性
- **🆕 几何碰撞检测**: 橡皮擦使用圆形区域与路径点、文字矩形的碰撞检测
- **🆕 自定义光标**: 动态生成橡皮擦光标图标，提供精确的视觉反馈
- **🆕 精确预览**: 红色圆形预览与实际擦除范围完全一致，消除视觉偏差
- **🆕 智能边界检查**: 确保橡皮擦操作仅在有效区域内执行，避免误操作

## 项目结构

```
OverlayWidget/
├── OverlayWidget.h      # 遮罩类头文件
├── OverlayWidget.cpp    # 遮罩类实现
├── main.cpp             # 演示程序入口
├── CMakeLists.txt       # CMake配置文件
└── README.md            # 说明文档
```

## 核心设计

### 遮罩原理

```cpp
// 创建透明遮罩覆盖在目标widget上
OverlayWidget *overlay = new OverlayWidget(targetWidget);

// 设置遮罩位置和大小与目标widget相同
overlay->setGeometry(targetWidget->geometry());

// 透明背景，可以看到下面的内容
setAttribute(Qt::WA_TranslucentBackground, true);
```

### 🆕 橡皮擦实现

```cpp
// 检查位置是否在有效区域内
bool isPositionInTargetWidget(const QPoint& pos) const {
    if (!m_targetWidget) return false;
    QRect targetRect = QRect(QPoint(0, 0), m_targetWidget->size());
    return targetRect.contains(pos);
}

// 创建自定义橡皮擦光标
QCursor createEraserCursor() {
    QPixmap cursorPixmap(8, 8);
    cursorPixmap.fill(Qt::transparent);
    QPainter painter(&cursorPixmap);
    // 绘制橡皮擦图标...
    return QCursor(cursorPixmap, 0, 0);  // 热点在中心
}

// 橡皮擦碰撞检测
bool isPointInEraserRange(const QPoint& point, const QPoint& eraserCenter) {
    int dx = point.x() - eraserCenter.x();
    int dy = point.y() - eraserCenter.y();
    int distance = dx * dx + dy * dy;
    int radius = m_eraserSize / 2;
    return distance <= radius * radius;
}

// 绘制精确预览（仅在有效区域内显示）
void drawEraserCursor(QPainter& painter) {
    if (!m_eraserMode || !isPositionInTargetWidget(m_currentMousePos)) return;
    
    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    painter.setBrush(QBrush(QColor(0, 0, 0, 30)));
    int radius = m_eraserSize / 2;
    painter.drawEllipse(m_currentMousePos, radius, radius);
}

// 执行擦除操作（仅在有效区域内）
void performErase(const QPoint& position) {
    if (!isPositionInTargetWidget(position)) return;
    // 检测并删除范围内的路径和文字
    // 记录删除的内容用于撤销重做
}
```

### 保存机制

```cpp
// 先渲染目标widget
targetWidget->render(&pixmap);

// 再在上面绘制标注
QPainter painter(&pixmap);
drawPaths(painter);    // 绘制路径
drawTexts(painter);    // 绘制文字
```

## 应用场景

- **图片标注**: 在图片查看器中添加完整的标注和编辑功能
- **UI设计评审**: 为UI元素添加标注，支持修改和擦除
- **教学演示**: 在演示内容上添加重点标记，可以动态调整
- **文档编辑**: 为文档预览添加批注功能，支持编辑和删除
- **错误标记**: 标记问题区域，支持撤销重做操作
- **🆕 精确编辑**: 使用精确的橡皮擦工具去除不需要的标注部分，视觉反馈准确，边界保护避免误操作

## 🎯 完整功能列表

### ✅ 完成功能
- **Widget遮罩**: 在指定的widget上覆盖透明遮罩层
- **自由绘制**: 在遮罩上进行鼠标拖拽绘制，支持颜色和宽度调节
- **文字标注**: 点击位置输入文字标注，支持编辑已有文字
- **🎯 精确橡皮擦**: 自定义光标 + 准确预览 + 智能擦除 + 边界保护
- **完整撤销重做**: 支持所有操作的撤销重做，包括橡皮擦
- **智能工具栏**: 可拖动、可收起的两行工具栏
- **保存功能**: 将原widget内容+标注保存为完整图片
- **实时预览**: 透明遮罩可以看到下面的widget内容
- **快捷键支持**: 完整的快捷键系统

### 🚀 扩展功能建议

### 🚀 扩展功能建议

- 添加更多绘制工具(矩形、圆形、箭头等)
- 添加图层管理功能
- 添加更多文字样式选项(字体、大小、粗体等)
- 支持导入/导出标注数据
- 添加标注历史记录管理
- 支持标注模板和预设
- 部分路径擦除功能（当前为整条路径擦除）
- 橡皮擦形状选择（圆形、方形、自定义）

## 注意事项

- 遮罩会覆盖在目标widget上，可能影响原widget的交互
- 标注内容保存在内存中，程序关闭后会丢失
- 保存的图片包含完整的widget内容+标注
- 目标widget必须是可见的才能正确显示遮罩
- 🆕 橡皮擦会删除整条路径，不支持部分路径擦除
- 🆕 橡皮擦使用自定义光标和精确预览，确保视觉反馈与实际擦除一致
- 🆕 橡皮擦具有智能边界检查，仅在有效区域内显示预览和执行操作
- 🆕 撤销重做操作会影响所有后续操作，请谨慎使用清除功能
- 🎯 **重要**: 新的橡皮擦功能大幅提升了精确度和用户体验