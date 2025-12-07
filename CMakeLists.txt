cmake_minimum_required(VERSION 3.10)

project(SmoothSlideshow VERSION 1.0.45 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets Gui Core REQUIRED)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/ConfigManager.cpp
    src/ConfigManager.h
    src/SlideshowWidget.cpp
    src/SlideshowWidget.h
    src/ThumbnailLoader.cpp
    src/ThumbnailLoader.h
    src/ImageCacheLoader.cpp
    src/ImageCacheLoader.h
)

add_executable(SmoothSlideshow ${SOURCES})

target_link_libraries(SmoothSlideshow PRIVATE Qt5::Widgets Qt5::Gui Qt5::Core)
