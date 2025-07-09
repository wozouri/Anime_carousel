cmake_minimum_required(VERSION 3.19)
project(untitled4 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)

qt_standard_project_setup()

qt_add_executable(untitled4
    WIN32 MACOSX_BUNDLE
    main.cpp
    GlitchEffectWidget.cpp
    GlitchEffectWidget.h


)

target_link_libraries(untitled4
    PRIVATE
        Qt::Core
        Qt::Widgets
)
target_link_libraries(untitled4 PRIVATE Qt6::Widgets)
target_link_libraries(untitled4 PRIVATE Qt6::OpenGLWidgets)

include(GNUInstallDirs)

install(TARGETS untitled4
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET untitled4
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
