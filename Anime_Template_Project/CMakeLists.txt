cmake_minimum_required(VERSION 3.16)

project(Anime_Template_Project VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        anime_homepage.cpp
        anime_homepage.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Anime_Template_Project
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Carousel_card/carousel_card.h Carousel_card/carousel_card.cpp
        Carousel_card/carrier_card.h Carousel_card/carrier_card.cpp
        src.qrc
        Carousel_card/card_text.h Carousel_card/card_text.cpp
        Carousel_card/card_text.h Carousel_card/card_text.cpp
        Carousel_card/card_button.h Carousel_card/card_button.cpp
        button_class/diffusion_button.h button_class/diffusion_button.cpp
        button_class/wave_button.h button_class/wave_button.cpp
        Login_interface/responsive_form.h Login_interface/responsive_form.cpp
        Login_interface/scroll_bar.h Login_interface/scroll_bar.cpp
        Login_interface/login_form.h Login_interface/login_form.cpp
        Login_interface/registration_form.h Login_interface/registration_form.cpp

        Login_interface/input_box.h Login_interface/input_box.cpp
        Login_interface/login_button.h Login_interface/login_button.cpp
        Login_interface/other_login_buttons.h Login_interface/other_login_buttons.cpp
        Login_interface/transparent_transition_interface.h Login_interface/transparent_transition_interface.cpp
        Login_interface/hollow_button.h Login_interface/hollow_button.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Anime_Template_Project APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Anime_Template_Project SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Anime_Template_Project
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Anime_Template_Project PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Anime_Template_Project)
endif()
set_target_properties(Anime_Template_Project PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Anime_Template_Project
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Anime_Template_Project)
endif()
