#ifndef KERNEL_CORE_SCHEDULER_HPP
#define KERNEL_CORE_SCHEDULER_HPP

#include "task.hpp"

class Scheduler {
    public:
        Scheduler();
        ~Scheduler();

        //kernel
        void addTask(TaskBase *task);
        void removeTask(TaskBase *task);
        TaskBase* getNextTask();

        //tick
        void tick();

    protected:

    private:
        static constexpr size_t MAX_PRIORITIES = 8U;
        std::array<TaskBase*, MAX_PRIORITIES> readyList_;

        TaskBase* currentTask_;
};
    

#endif // KERNEL_CORE_SCHEDULER_HPP