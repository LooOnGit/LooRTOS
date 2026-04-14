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
    running_ = true;
    TaskBase* firstTask = scheduler_.getNextTask();

    if(firstTask != nullptr) {
        while(running_) {
            
        }
    }
    firstTask->getEntry()();
}
