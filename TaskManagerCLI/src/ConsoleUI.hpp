// user interface
// shows menu, reads user input
// calls TaskManager functions

#include <vector>
#include "Task.h"
#include <iostream>

class ConsoleUI{
private:
    ITaskRepository *repo;

public:
    void ConsoleUI::start();
    void ConsoleUI::displayMenu() const;
    std::string ConsoleUI::promptTitle() const;
    Task::Priority ConsoleUI::promptPriority() const;

};