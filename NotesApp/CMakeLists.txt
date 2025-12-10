cmake_minimum_required(VERSION 3.16)
cmake_minimum_required(VERSION 3.16)

project(HandNoteApp LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)


qt_add_executable(HandNoteApp
    src/main.cpp

    src/MainWindow.hpp
    src/MainWindow.cpp

    src/core/NoteModel.hpp
    src/core/NoteModel.cpp

    src/core/NoteRepository.hpp

    src/core/LocalFileNoteRepository.hpp
    src/core/LocalFileNoteRepository.cpp

    src/HandwritingCanvas.hpp
    src/HandwritingCanvas.cpp
)

target_link_libraries(HandNoteApp 
    PRIVATE
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
)

target_include_directories(HandNoteApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (APPLE)
    set_target_properties(HandNoteApp PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.handnote"
        MACOSX_BUNDLE_BUNDLE_NAME "HandNoteApp"
    )
endif()
