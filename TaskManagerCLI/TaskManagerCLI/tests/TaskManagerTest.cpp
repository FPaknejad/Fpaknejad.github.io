#include <gtest/gtest.h>
#include "../src/TaskManager.hpp"
#include "mocks/MockRepository.hpp"

TEST(TaskManagerTest, AddTaskSavesToRepo) {
    TaskManager manager;
    MockRepository mock;

    manager.setRepository(&mock);
    manager.addTask("Do the dishes");

    ASSERT_EQ(mock.capturedTasks.size(), 1);
    EXPECT_EQ(mock.capturedTasks[0].getTitle(), "Do the dishes");
    EXPECT_FALSE(mock.capturedTasks[0].isDone());
}
TEST(TaskManagerTest, ListTasks) {
    TaskManager manager;
    MockRepository mock;

    manager.setRepository(&mock);
    manager.addTask("Do the dishes");
    manager.addTask("Clean the room");

    std::ostringstream output;
    std::streambuf* oldCoutBuffer = std::cout.rdbuf(output.rdbuf());

    manager.listTasks();

    std::cout.rdbuf(oldCoutBuffer);
    
    std::string expectedOutput = "[ ] Do the dishes\n[ ] Clean the room\n";
    EXPECT_EQ(output.str(), expectedOutput);
}