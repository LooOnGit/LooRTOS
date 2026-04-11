#include "task.hpp"

TaskBase::TaskBase(TaskFunction entry, uint8_t priority)
    : priority_(priority),
      stackPtr_(nullptr),
      next_(nullptr),
      entry_(entry),
      currentState_(State::READY) 
{

}

TaskBase::~TaskBase() 
{

}

TaskBase::State TaskBase::getState() const 
{
  return currentState_;
}

