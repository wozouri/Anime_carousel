cmake_minimum_required(VERSION 3.16)

project(Accordion VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt库
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 定义项目源文件（统一文件列表，避免重复）
set(PROJECT_SOURCES
    AccordionCard.cpp
    AccordionCard.h
    accordionwidget.h
    accordionwidget.cpp
    AccordionWidget.h
    AccordionWidget.cpp
)

# 定义资源文件
set(PROJECT_RESOURCES
    img.qrc
)

# 创建库（而不是可执行文件，因为这是子项目）
add_library(Accordion STATIC
    ${PROJECT_SOURCES}
    ${PROJECT_RESOURCES}
)

# 链接Qt库
target_link_libraries(Accordion PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# 设置目标属性
set_target_properties(Accordion PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# 如果需要创建独立的可执行文件用于测试（可选）
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        qt_add_executable(Accordion_demo
            MANUAL_FINALIZATION
            main.cpp
        )
    else()
        add_executable(Accordion_demo main.cpp  img.qrc)
    endif()
    
    # 链接库和Qt
    target_link_libraries(Accordion_demo PRIVATE 
        Accordion 
        Qt${QT_VERSION_MAJOR}::Widgets
    )
    
    # 设置可执行文件属性
    set_target_properties(Accordion_demo PROPERTIES
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )
    
    # Qt6需要finalize
    if(QT_VERSION_MAJOR EQUAL 6)
        qt_finalize_executable(Accordion_demo)
    endif()
endif()

# 导出包含目录，供父项目使用
target_include_directories(Accordion PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# 安装规则（可选）
include(GNUInstallDirs)
install(TARGETS Accordion
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 安装头文件
install(FILES
    AccordionCard.h
    accordionwidget.h
    AccordionWidget.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Accordion
)