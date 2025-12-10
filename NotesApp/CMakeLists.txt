cmake_minimum_required(VERSION 3.24)
project(HandNoteApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt auto-moc/uic/rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# If CMAKE_PREFIX_PATH is not set, use QT_ROOT from environment
if(NOT CMAKE_PREFIX_PATH AND DEFINED ENV{QT_ROOT})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_ROOT}")
endif()

# Find Qt (same on all platforms)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(HandNoteApp
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/HandwritingCanvas.cpp
    src/HandwritingCanvas.h
)

target_link_libraries(HandNoteApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)