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

uint8_t TaskBase::getPriority() const { 
  return priority_; 
}

TaskBase* TaskBase::getNext() const {
  return next_;
}

void TaskBase::setNext(TaskBase* next) {
  next_ = next;
}
