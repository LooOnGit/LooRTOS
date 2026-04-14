#ifndef KERNEL_CORE_KERNEL_HPP
#define KERNEL_CORE_KERNEL_HPP

#include "scheduler.hpp"
#include "task.hpp" 

class Kernel {
    public:
        static Kernel getInstance();
        
        
    private:
        Kernel();
        ~Kernel();
};

#endif // KERNEL_CORE_KERNEL_HPP