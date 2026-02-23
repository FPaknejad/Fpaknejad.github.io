# Build test executable
add_executable(run_tests test_main.cpp)

# Add include paths for tests
target_include_directories(run_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

# Register tests with CTest
add_test(NAME basic_tests COMMAND run_tests)