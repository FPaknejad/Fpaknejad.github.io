#pragma once
#include "../../src/TaskRepository.hpp"
#include <vector>

class MockRepository : public ITaskRepository {
public:
    std::vector<Task> capturedTasks;

    void saveToFile(const std::vector<Task>& tasks) override {
        capturedTasks = tasks;
    }

    void readFile(std::vector<Task>& tasks) override {
        tasks = capturedTasks;
    }
};