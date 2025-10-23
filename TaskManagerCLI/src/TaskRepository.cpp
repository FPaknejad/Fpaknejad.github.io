//
//  TaskRepository.cpp
//  TaskManagerCLI
//
//  Created by Fatemeh Paknejad on 17.07.25.
//

#include "TaskRepository.hpp"
#include "Task.h"
#include "TaskManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

void TaskRepository::readFile(std::vector<Task>& taskList){
    std::ifstream taskFile("/Users/niuklear/Portfolio/TaskManagerCLI/TaskManagerCLI/tasks.txt");
    std::string line;
    while(std::getline(taskFile,line)){
        size_t sep = line.find("|");
        std::string title = line.substr(0,sep);
        bool done = (line.substr(sep+1) == "1");
        taskList.push_back(Task(title, done));
    }
}

void TaskRepository::saveToFile(const std::vector<Task>& taskList){
    std::ofstream taskFile("/Users/niuklear/Portfolio/TaskManagerCLI/TaskManagerCLI/tasks.txt");
    for(const Task& t : taskList){
        taskFile << t.getTitle() << "|" << t.isDone()<< "\n";
    }
    taskFile.close();
    std::cout << "Tasks saved successfully." << std::endl;
}
