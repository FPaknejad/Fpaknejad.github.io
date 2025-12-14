cmake_minimum_required(VERSION 3.16)
cmake_minimum_required(VERSION 3.16)

project(HandNoteApp LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.1/msvc2022_64")
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)
# find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

set(SOURCES
    src/main.cpp

    src/core/LocalFileNoteRepository.cpp
    src/core/NoteRepository.cpp

    src/ui/MainWindow.cpp
    src/ui/NoteTreePanel.cpp
    src/ui/PageWorkspace.cpp
    src/ui/SidebarTreeView.cpp
    src/ui/TextEditorWidget.cpp
    src/ui/HandwritingCanvas.cpp
    src/ui/PageToolbar.cpp

    src/controller/NoteController.cpp

    src/domain/NoteModel.cpp
    src/domain/NoteTypes.cpp
    src/domain/Sorting.cpp
    src/domain/NoteCommands.cpp

    src/persistence/JsonSerializer.cpp

    src/services/TreeSyncService.cpp
    src/services/NoteService.cpp
    src/services/HandwritingService.cpp
    src/services/AuthService.cpp
)

add_executable(HandNoteApp ${SOURCES})

target_link_libraries(HandNoteApp 
    PRIVATE
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
)

target_include_directories(HandNoteApp PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/ui
        ${CMAKE_SOURCE_DIR}/src/controller
        ${CMAKE_SOURCE_DIR}/src/domain
        ${CMAKE_SOURCE_DIR}/src/persistence
        ${CMAKE_SOURCE_DIR}/src/services
)



if (APPLE)
    set_target_properties(HandNoteApp PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.handnote"
        MACOSX_BUNDLE_BUNDLE_NAME "HandNoteApp"
    )
endif()
