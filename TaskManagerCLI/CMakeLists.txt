cmake_minimum_required(VERSION 3.10)
project(TaskManagerCLI)

set(CMAKE_CXX_STANDARD 17)

add_executable(app
    src/Main.cpp
    src/Task.cpp
    src/TaskManager.cpp
    src/TaskRepository.cpp
)

enable_testing()
add_subdirectory(tests)