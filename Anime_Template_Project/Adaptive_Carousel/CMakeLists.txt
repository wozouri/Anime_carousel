cmake_minimum_required(VERSION 3.16)
project(DaenNoKado VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt5或Qt6
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui)

# 设置Qt相关选项
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 源文件
set(SOURCES
    main.cpp
    daen_no_kado.cpp
    latticed_circle_button.cpp
)

# 头文件
set(HEADERS
    daen_no_kado.h
    latticed_circle_button.h
)

# 资源文件（如果存在）
set(RESOURCES
    img.qrc
    # resources.qrc  # 取消注释如果你有资源文件
)

# 创建可执行文件
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DaenNoKado
        MANUAL_FINALIZATION
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
    )
else()
    add_executable(DaenNoKado
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
    )
endif()

# 链接Qt库 - 使用关键字格式
target_link_libraries(DaenNoKado PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
)

# 设置编译器特定选项
if(WIN32)
    set_target_properties(DaenNoKado PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Qt6特定设置
if(${QT_VERSION_MAJOR} EQUAL 6)
    qt_finalize_executable(DaenNoKado)
endif()

# 设置输出目录
set_target_properties(DaenNoKado PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 调试信息
message(STATUS "Qt version: ${Qt${QT_VERSION_MAJOR}_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# 安装规则（可选）
install(TARGETS DaenNoKado
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)