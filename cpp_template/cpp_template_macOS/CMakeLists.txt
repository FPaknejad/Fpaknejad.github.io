cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++" CACHE STRING "" FORCE)
# macOS SDK path ( get path with xcrun --show-sdk-path)
# set(MACOS_SDK_PATH "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")

project(my_cpp_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect module files
file(GLOB_RECURSE MODULE_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/modules/*.cppm"
)

# Detect normal C++ files
file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# If project has modules, create a module library
if(MODULE_FILES)
    add_library(project_modules)
    target_sources(project_modules
        PUBLIC
            FILE_SET CXX_MODULES FILES ${MODULE_FILES}
    )
endif()

# Main application
add_executable(app ${CPP_FILES})

# Link modules to the app (only if modules exist)
if(TARGET project_modules)
    target_link_libraries(app PRIVATE project_modules)
endif()

target_include_directories(app PRIVATE include)