#ifndef KERNEL_CORE_TASK_HPP
#define KERNEL_CORE_TASK_HPP

#include <array>
#include <cstddef>
#include <cstdint>

// Define TaskFunction represents the entry point of a task
using TaskFunction = void (*)();

class TaskBase {
public:
  TaskBase();
  ~TaskBase();

  enum class State : uint8_t {
    READY,
    RUNNING,
    BLOCKED,
    SUSPENDED
  };

  State getState() const;

protected:
  uint8_t priority_;
  void *stackPtr_;
  TaskBase *next_;
  TaskFunction entry_;

private:
  volatile State currentState_;
};

template <size_t N> class Task : public TaskBase {
public:
  Task();
  ~Task();

private:
  alignas(8) std::array<std::byte, N> stackMemory_;
};

#endif // KERNEL_CORE_TASK_HPP