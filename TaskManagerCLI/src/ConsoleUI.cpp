#include "ConsoleUI.hpp"
#include "TaskRepository.hpp"
#include "Task.h"

#include <memory>

void ConsoleUI::start(){
    int menuIndex;
    TaskRepository concreteRepo;
    repo = &concreteRepo;
    repo -> readFile(taskList);
    
    while (true){
        std::cout<<"Enter the Number of options \n"<<"1. Add Task \n2. List Tasks\n3. Mark Task as Done\n4. delete task\n5. Filter tasks \n6. Edit Task Title\n7. Exit\n"<<std::flush;
        
        std::cin>>menuIndex;
        if (menuIndex < 1 || menuIndex > 7){
            std::cout<<"Invalid index. Please enter a valid number between 1 to 7\n"<<std::flush;
        }
        if (std::cin.fail()) {
            std::cin.clear(); // clear error state
            std::cin.ignore(1000, '\n'); // discard bad input
            std::cout << "Invalid input. Please enter a number.\n"<<std::flush;
        }
        else{
            switch (menuIndex) {
                case 1:
                    addTask();
                    break;
                case 2:
                    listTasks();
                    break;
                case 3:
                    markTaskDone();
                    break;
                case 4:
                    deleteTask();
                    break;
                case 5:
                    filterTasks();
                    break;
                case 6:
                    editTaskTitle();
                    break;
                case 7:
                    repo -> saveToFile(taskList);
                    return 0;
                
                default:
                    break;
            }
        
        }

    }
}

void ConsoleUI::promptTitle(std::string title) const {
}

void ConsoleUI::promptPriority() const{
}

void ConsoleUI::showTasks(const std::vector<Task>&) const {
}