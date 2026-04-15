#include "kernel.hpp"
#include "task.hpp"

Kernel::Kernel() {
    
}

Kernel::~Kernel() {
    
}

Kernel& Kernel::getInstance() {
    static Kernel instance;
    return instance;
}

Scheduler& Kernel::getScheduler() {
    return scheduler_;
}

void Kernel::start() {
    if(running_) return;

    TaskBase* firstTask = scheduler_.getNextTask();

    if(firstTask != nullptr) {
        running_ = true;
        firstTask->getEntry()(); 
    }
    
    while(true) {
        
    }
}
