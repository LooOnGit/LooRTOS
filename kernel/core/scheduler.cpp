#include "scheduler.hpp"
#include <cstddef>
#include <cstdint>

Scheduler::Scheduler() :
    currentTask_(nullptr)
{
    readyList_.fill(nullptr);
}

Scheduler::~Scheduler() {

}

void Scheduler::addTask(TaskBase *task) {
    if(task == nullptr) return;

    if(task->getPriority() >= MAX_PRIORITIES) return;
    
    task->setNext(readyList_[task->getPriority()]);
    readyList_[task->getPriority()] = task;
}

void Scheduler::removeTask(TaskBase *task) {
    if(task == nullptr) return;

    if(task->getPriority() >= MAX_PRIORITIES) return;

    uint8_t prio = task->getPriority();

    //case 1: task is the first task in the ready list
    if(readyList_[prio] == task)
    {
        readyList_[prio] = task->getNext();
        task->setNext(nullptr);
        return;
    }

    //case 2: task is not the first task in the ready list
    TaskBase *prev = readyList_[prio];
    while(prev != nullptr && prev->getNext() != task)
    {
        prev = prev->getNext();
    }

    if(prev != nullptr)
    {
        prev->setNext(task->getNext());
        task->setNext(nullptr);
    }
}

TaskBase* Scheduler::getNextTask() {
    for(size_t i = MAX_PRIORITIES; i > 0; --i)
    {
        if(readyList_[i-1] != nullptr)
        {
            return readyList_[i-1];
        }
    }
    return nullptr;
}

void Scheduler::tick() {
    
}