//
//  Task.h
//  TaskManagerCLI
//
//  Created by Fatemeh Paknejad on 03.07.25.
//

#ifndef TASK_H
#define TASK_H

#include <string>

class Task {

public:
    enum class Priority {
        LOW,
        MEDIUM,
        HIGH
    };

private:
    std::string title;
    bool done;
    Task::Priority priority;

public:
    Task(const std::string& title, bool done, Priority priority = Priority::MEDIUM);
    void markDone();
    void printTask() const;
    bool isDone() const;

    std::string getTitle() const;
    void setTitle(const std::string& newTitle);
    
    

    Priority getPriority() const;
    void setPriority(Task::Priority newPriority);
    static std::string priorityToString(Priority p);


};

#endif // TASK_H
