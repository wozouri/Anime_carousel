#cmake_minimum_required(VERSION 3.16)
#
##project(Anime_carousel)
#
## 设置C++标准
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
## 查找Qt5或Qt6
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Gui)
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui)
#
## 启用Qt的自动MOC功能
#set(CMAKE_AUTOMOC ON)


#add_subdirectory(Blur_Text)
#add_subdirectory(button_class)
#add_subdirectory(Carousel_card)
#add_subdirectory(dial_class)
#add_subdirectory(Eye_of_Cthulhu)
#add_subdirectory(Flowing_Gradient_Font)
#add_subdirectory(FractalWidget)
#add_subdirectory(Generative_Lines)
#add_subdirectory(Glitch_Text)
#add_subdirectory(Honeycomb_Grid)
#add_subdirectory(img)
#add_subdirectory(json)
#add_subdirectory(Letter_Glitch)
#add_subdirectory(Liquid)
#add_subdirectory(Login_interface)
#add_subdirectory(Magnet_Lines)
#add_subdirectory(Mimic_Button)
#add_subdirectory(Physical_Text)
#add_subdirectory(PixelTransition)
#add_subdirectory(Point_Wave)
#add_subdirectory(Pressure_Block)
#add_subdirectory(SplitText)
#add_subdirectory(Star_Sky_Connection)
#add_subdirectory(TreeScene)
#add_subdirectory(utility_class)
#add_subdirectory(Wave)
#add_subdirectory(Zipper_Slider)

# 如果有主程序文件，添加可执行文件
#add_executable(Anime_carousel main.cpp)

# 链接Qt库
# target_link_libraries(Anime_carousel Qt6::Core Qt6::Widgets)

# 设置包含目录
#target_include_directories(Anime_carousel PRIVATE
    #${CMAKE_CURRENT_SOURCE_DIR}
    #${CMAKE_CURRENT_SOURCE_DIR}/Accordion
    #${CMAKE_CURRENT_SOURCE_DIR}/Adaptive_Carousel
    #${CMAKE_CURRENT_SOURCE_DIR}/Blur_Text
    #${CMAKE_CURRENT_SOURCE_DIR}/button_class
    #${CMAKE_CURRENT_SOURCE_DIR}/Carousel_card
    #${CMAKE_CURRENT_SOURCE_DIR}/dial_class
    #${CMAKE_CURRENT_SOURCE_DIR}/Eye_of_Cthulhu
    #${CMAKE_CURRENT_SOURCE_DIR}/Flowing_Gradient_Font
    #${CMAKE_CURRENT_SOURCE_DIR}/FractalWidget
    #${CMAKE_CURRENT_SOURCE_DIR}/Generative_Lines
    #${CMAKE_CURRENT_SOURCE_DIR}/Glitch_Text
    #${CMAKE_CURRENT_SOURCE_DIR}/Honeycomb_Grid
    #${CMAKE_CURRENT_SOURCE_DIR}/img
    #${CMAKE_CURRENT_SOURCE_DIR}/json
    #${CMAKE_CURRENT_SOURCE_DIR}/Letter_Glitch
    #${CMAKE_CURRENT_SOURCE_DIR}/Liquid
    #${CMAKE_CURRENT_SOURCE_DIR}/Login_interface
    #${CMAKE_CURRENT_SOURCE_DIR}/Magnet_Lines
    #${CMAKE_CURRENT_SOURCE_DIR}/Mimic_Button
    #${CMAKE_CURRENT_SOURCE_DIR}/Physical_Text
    #${CMAKE_CURRENT_SOURCE_DIR}/PixelTransition
    #${CMAKE_CURRENT_SOURCE_DIR}/Point_Wave
    #${CMAKE_CURRENT_SOURCE_DIR}/Pressure_Block
    #${CMAKE_CURRENT_SOURCE_DIR}/SplitText
    #${CMAKE_CURRENT_SOURCE_DIR}/Star_Sky_Connection
    #${CMAKE_CURRENT_SOURCE_DIR}/TreeScene
    #${CMAKE_CURRENT_SOURCE_DIR}/utility_class
    #${CMAKE_CURRENT_SOURCE_DIR}/Wave
    #${CMAKE_CURRENT_SOURCE_DIR}/Zipper_Slider
#)

# 添加子目录
add_subdirectory(Accordion)
add_subdirectory(Adaptive_Carousel)
add_subdirectory(Blur_Text)