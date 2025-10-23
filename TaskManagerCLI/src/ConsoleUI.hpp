#include <vector>
#include "Task.h"

class ConsoleUI{

void showMenu(int choice);
void promptTitle(std::string title) const;
void promptPriority() const;
void showTasks(const std::vector<Task>&) const;

};