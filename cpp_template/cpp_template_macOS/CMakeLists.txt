cmake_minimum_required(VERSION 3.28)

project(cpp20_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.cppm"
)

add_executable(app ${SRC_FILES})

# Enable module support on macOS (Clang)
target_compile_options(app PRIVATE -std=c++20)

target_include_directories(app PRIVATE include)

# Tests
enable_testing()
add_subdirectory(tests)