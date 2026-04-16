#ifndef ARCH_POSIX_PORT_HPP
#define ARCH_POSIX_PORT_HPP

#include <thread>
#include <mutex>
#include <condition_variable>

class TaskBase;

using TaskFunction = void (*)();

struct TaskContext {
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    bool running;
};

namespace Port {

    extern TaskBase* currentTask;
    extern TaskBase* nextTask;
}



#endif