//
//  Task.cpp
//  TaskManagerCLI
//
//  Created by Fatemeh Paknejad on 03.07.25.
//

#include "Task.h"
#include <iostream>

Task::Task(const std::string& title, bool done, Task::Priority priority)
    : title(title), done(done), priority(priority) {}

void Task::markDone() {
    done = true;
}

void Task::setTitle(const std::string& newTitle) {
    title = newTitle;
}

bool Task::isDone() const {
    return done;
}

void Task::printTask() const {
    std::string p;
    switch (priority) {
        case Priority::LOW:
            p = "Low";
            break;
        case Priority::MEDIUM:
            p = "Medium";
            break;
        case Priority::HIGH:
            p = "High";
            break;
    }

    std::cout << (done ? "[x] " : "[ ] ") << title << " | Priority: " << p << ")" << std::endl;
}


std::string Task::getTitle() const {
    return title;  
}

std::string Task::priorityToString(Task::Priority p){
    switch (p) {
        case Priority::LOW:
            return "Low";
        case Priority::MEDIUM:
            return "Medium";
        case Priority::HIGH:
            return "High";
        default:
            return "Unknown";
    }
}

Task::Priority Task::getPriority() const {
    return priority;
}
void Task::setPriority(Task::Priority newPriority) {
    priority = newPriority;
}
