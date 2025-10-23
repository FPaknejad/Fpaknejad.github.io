//
//  TaskRepository.hpp
//  TaskManagerCLI
//
//  Created by Fatemeh Paknejad on 17.07.25.
//

#ifndef TaskRepository_hpp
#define TaskRepository_hpp

#include <stdio.h>
#include <fstream>
#include "Task.h"
#include <vector>

class ITaskRepository{
public:
    virtual void saveToFile(const std::vector<Task>& taskList) = 0;
    
    virtual void readFile(std::vector<Task>& taskList) =0;
    
    virtual ~ITaskRepository() = default;
};

class TaskRepository : public ITaskRepository{
private:
    
public:
    void saveToFile(const std::vector<Task>& taskList);
    void readFile(std::vector<Task>& taskList);
};
#endif /* TaskRepository_hpp */
