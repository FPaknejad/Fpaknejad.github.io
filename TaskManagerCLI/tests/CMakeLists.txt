add_executable(TaskTest
    TaskTest.cpp
    ../src/Task.cpp
)
target_include_directories(TaskTest PRIVATE ../src)
target_link_libraries(TaskTest gtest gtest_main pthread)
add_test(NAME TaskTest COMMAND TaskTest)

add_executable(TaskManagerTest
    TaskManagerTest.cpp
    ../src/Task.cpp
    ../src/TaskManager.cpp
    ../src/TaskRepository.cpp  
)
target_include_directories(TaskManagerTest PRIVATE ../src)
target_link_libraries(TaskManagerTest gtest gtest_main pthread)
add_test(NAME TaskManagerTest COMMAND TaskManagerTest)