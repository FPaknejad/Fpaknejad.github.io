//
//  TaskManager.cpp
//  TaskManagerCLI
//
//  Created by Fatemeh Paknejad on 05.07.25.
//

#include "TaskManager.hpp"
#include "Task.h"
#include "TaskRepository.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

void TaskManager::setRepository(ITaskRepository* customRepo) {
    repo = customRepo;
}

void TaskManager::addTask(const std::string& title, bool done) {
    taskList.emplace_back(title, done);
    repo->saveToFile(taskList);
}

void TaskManager::addTask(){
    std::string taskTitle;
    std::cout<< "Enter the Task title\n"<<std::flush;
    std::cin.ignore();
    std::getline(std::cin,taskTitle);
    taskList.push_back(Task(taskTitle,false));

    std::cout<<"Set priority (1. Low, 2. Medium, 3. High): "<<std::flush;
    int priorityInput;
    std::cin>>priorityInput;
    if (std::cin.fail()) {
        std::cin.clear(); // clear error state
        std::cin.ignore(1000, '\n'); // discard bad input
        std::cout << "Invalid input. Please enter a number.\n"<<std::flush;
        return; 
    }
    else if (priorityInput < 1|| priorityInput > 3){
        std::cout<<"Please enter valid priority number 1 - 2\n"<<std::flush;
    }


    Task::setPriority(priorityInput);
}

void TaskManager::listTasks() const{
    for(const Task &t : taskList){
        t.printTask();
    }
}

void TaskManager::markTaskDone(){
    for (size_t i = 0; i < taskList.size(); ++i) {
        std::cout << i + 1 << ". ";
        taskList[i].printTask();
    }
    size_t taskIndex;
    std::cout<<"Enter task Index: "<<std::flush;
    std::cin>>taskIndex;
    if (std::cin.fail()) {
        std::cin.clear(); // clear error state
        std::cin.ignore(1000, '\n'); // discard bad input
        std::cout << "Invalid input. Please enter a number.\n"<<std::flush;
        return;
    }
    if (taskIndex < 1 || taskIndex > static_cast<size_t>(taskList.size())) {
        std::cout << "invalid Index\n"<<std::flush;
        return;
    }
    taskList[taskIndex-1].markDone();

}

void TaskManager::deleteTask(){
    std::cout<<"Enter the index of tasks from the following list to delete\n"<<std::flush;
    for (size_t i = 0; i < taskList.size(); ++i) {
        std::cout << i + 1 << ". ";
        taskList[i].printTask();
    }
    size_t taskIndex;
    std::cin >> taskIndex;
    if (std::cin.fail()) {
        std::cin.clear(); // clear error state
        std::cin.ignore(1000, '\n'); // discard bad input
        std::cout << "Invalid input. Please enter a number.\n"<<std::flush;
        
        return;
    }
    if (taskIndex < 1 || taskIndex > static_cast<size_t>(taskList.size())) {
        std::cout << "Please enter a valid index number" << std::endl;
        return;
    }
    taskList.erase(taskList.begin() + taskIndex - 1);
}
void TaskManager::filterTasks() const{
    std::cout<<"Enter the filter option:\n1. Show all\n2. Show completed\n3. Show incomplete\n"<<std::flush;
    
    int filterOption;
    
    std::cin>>filterOption;
    if (std::cin.fail()) {
        std::cin.clear(); // clear error state
        std::cin.ignore(1000, '\n'); // discard bad input
        std::cout << "Invalid input. Please enter a number.\n"<<std::flush;
        return;
    }

    switch (filterOption) {
        case 1:
            TaskManager::listTasks();
            break;
            
        case 2:
            for(const Task &t:taskList){
                if( t.isDone()){
                    t.printTask();
                }
                
            }
            break;
        case 3:
            for(const Task &t : taskList){
                if(t.isDone() == false){
                    t.printTask();
                }
            }
            break;
            
        default:
            break;
    }
    
}

void TaskManager::editTaskTitle(){
    std::cout<<"Enter index of the task to edit: \n"<<std::flush;
    for (size_t i = 0; i < taskList.size(); ++i) {
        std::cout << i + 1 << ". ";
        taskList[i].printTask();
    }
    size_t taskIndex;
    std::cin>>taskIndex;
    if (taskIndex < 1 || taskIndex > static_cast<size_t>(taskList.size())) {
        std::cout << "Invalid index.\n" << std::flush;
        return;
    }
    if (std::cin.fail()) {
        std::cin.clear(); // clear error state
        std::cin.ignore(1000, '\n'); // discard bad input
        std::cout << "Invalid input. Please enter a number.\n"<<std::flush;
        return;
    }
    std::cout<<"Enter new title:\n"<<std::flush;
    
    std::string newTitle;
    std::cin.ignore();
    std::getline(std::cin,newTitle);
    
    taskList[taskIndex-1].setTitle(newTitle);
}

int TaskManager:: runMenu(){
    
    
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
