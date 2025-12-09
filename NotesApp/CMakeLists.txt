cmake_minimum_required(VERSION 3.16)

project(HandNoteApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.10.1/msvc2022_64/lib/cmake")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(HandNoteApp
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/HandwritingCanvas.cpp
    src/HandwritingCanvas.h
)


target_link_libraries(HandNoteApp PRIVATE Qt6::Widgets)
