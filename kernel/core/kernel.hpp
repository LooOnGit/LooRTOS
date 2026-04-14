#ifndef KERNEL_CORE_KERNEL_HPP
#define KERNEL_CORE_KERNEL_HPP

#include "scheduler.hpp"
#include "task.hpp" 

class Kernel {
    public:
        Kernel(const Kernel&) = delete; // Delete copy constructor
        Kernel& operator=(const Kernel&) = delete; // Delete copy assignment operator
        static Kernel& getInstance();
        
        Scheduler& getScheduler();
        void start();
    private:
        Kernel();
        ~Kernel();

        Scheduler scheduler_;
        bool running_ = false;
};

#endif // KERNEL_CORE_KERNEL_HPP