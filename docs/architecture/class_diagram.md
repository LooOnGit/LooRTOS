# 📐 LooRTOS — Class Diagram

> Class diagrams describe the **static structure** of the system: classes, attributes,  
> methods, and the inheritance / composition relationships between them.

---

## 1. Kernel Core — Central Brain

> The most critical group of classes, determining how Tasks are created,  
> queued, and executed by the CPU.

```mermaid
classDiagram
    direction TB

    class TaskBase {
        <<Abstract Base>>
        #uint8_t priority_
        #void* stackPtr_
        #TaskBase* next_
        #TaskFunction entry_
        -volatile State currentState_
        +TaskBase(TaskFunction entry, uint8_t priority)
        +~TaskBase()
        +getState() State
        +setState(State s) void
        +getPriority() uint8_t
        +run() void
    }

    class State {
        <<enumeration>>
        READY
        RUNNING
        BLOCKED
        SUSPENDED
    }

    class Task~size_t N~ {
        <<Template Class>>
        -alignas 8 std::array~std::byte, N~ stackMemory_
        +Task(TaskFunction entry, uint8_t priority)
        +~Task()
        +getStackSize() size_t
    }

    class Scheduler {
        <<Singleton>>
        -TaskBase* readyList_[MAX_PRIORITIES]
        -TaskBase* currentTask_
        -uint32_t tickCount_
        -Scheduler()
        +instance() Scheduler&
        +addTask(TaskBase* task) void
        +removeTask(TaskBase* task) void
        +schedule() void
        +yield() void
        +tick() void
        +getCurrentTask() TaskBase*
    }

    class Tick {
        <<Static Utility>>
        -uint32_t systemTicks_
        +init(uint32_t frequency) void
        +getTickCount() uint32_t
        +delayTicks(uint32_t ticks) void
    }

    TaskBase *-- State : contains
    TaskBase <|-- Task~size_t N~ : Inheritance &#40;Type Erasure&#41;
    Scheduler "1" o-- "*" TaskBase : manages via readyList_
    Scheduler ..> Tick : uses tick count

    note for TaskBase "Non-template Base Class\n→ Allows Scheduler to manage\nall Tasks regardless of Stack Size"
    note for Task~size_t N~ "Template Class\n→ Static stack allocation\nat Compile Time (Zero Heap)"
    note for Scheduler "Meyers Singleton\n→ Only one instance exists\nacross the entire system"
```

---

## 2. Synchronization Primitives

> Classes responsible for synchronizing access between Tasks:  
> preventing Race Conditions and protecting Shared Resources.

```mermaid
classDiagram
    direction TB

    class Mutex {
        <<RAII Support>>
        -TaskBase* owner_
        -uint8_t originalPriority_
        -bool locked_
        +Mutex()
        +lock() void
        +tryLock() bool
        +unlock() void
        +isLocked() bool
    }

    class LockGuard {
        <<RAII Wrapper>>
        -Mutex& mutex_
        +LockGuard(Mutex& m)
        +~LockGuard()
    }

    class Semaphore {
        -volatile uint8_t count_
        -uint8_t maxCount_
        +Semaphore(uint8_t initial, uint8_t max)
        +give() void
        +take(uint32_t timeout) bool
        +getCount() uint8_t
    }

    class Queue~T, size_t CAP~ {
        <<Lock-free Ring Buffer>>
        -std::array~T, CAP~ buffer_
        -std::atomic~size_t~ head_
        -std::atomic~size_t~ tail_
        +Queue()
        +push(const T& item) bool
        +pop(T& item) bool
        +isFull() bool
        +isEmpty() bool
        +size() size_t
    }

    class EventGroup {
        -volatile uint32_t flags_
        +EventGroup()
        +setFlags(uint32_t mask) void
        +clearFlags(uint32_t mask) void
        +waitFlags(uint32_t mask, uint32_t timeout) uint32_t
    }

    LockGuard --> Mutex : wraps reference
    Mutex ..> TaskBase : tracks owner_

    note for LockGuard "C++ RAII Pattern\nConstructor: lock()\nDestructor:  unlock()\n→ Automatically releases Mutex\neven if an exception occurs"
    note for Queue~T, size_t CAP~ "std::atomic head/tail\n→ Lock-free: ISR and Task\ncan access concurrently\nwithout disabling interrupts"
```

---

## 3. Memory Management — Static Allocation

```mermaid
classDiagram
    direction LR

    class PoolAllocator~T, size_t N~ {
        <<No-Heap Allocator>>
        -Slot storage_[N]
        -size_t freeCount_
        +allocate(Args... args) T*
        +deallocate(T* ptr) void
        +available() size_t
    }

    class Slot {
        <<Internal>>
        +alignas T byte data[sizeof T]
        +bool used
    }

    PoolAllocator~T, size_t N~ *-- Slot : contains N slots

    note for PoolAllocator~T, size_t N~ "Placement new + aligned storage\n→ Allocates objects from static array\nNever calls malloc/new"
```

---

## 4. Hardware Abstraction Layer

```mermaid
classDiagram
    direction TB

    class PortBase {
        <<Interface>>
        +initTick(uint32_t hz)* void
        +triggerContextSwitch()* void
        +disableInterrupts()* uint32_t
        +restoreInterrupts(uint32_t mask)* void
        +initTaskStack(void* stackTop, TaskFunction entry)* void*
    }

    class PosixPort {
        <<Simulation>>
        +initTick(uint32_t hz) void
        +triggerContextSwitch() void
        +disableInterrupts() uint32_t
        +restoreInterrupts(uint32_t mask) void
        +initTaskStack(void* stackTop, TaskFunction entry) void*
    }

    class ArmCortexMPort {
        <<Production>>
        +initTick(uint32_t hz) void
        +triggerContextSwitch() void
        +disableInterrupts() uint32_t
        +restoreInterrupts(uint32_t mask) void
        +initTaskStack(void* stackTop, TaskFunction entry) void*
    }

    PortBase <|.. PosixPort : implements
    PortBase <|.. ArmCortexMPort : implements
    Scheduler ..> PortBase : depends on

    note for PosixPort "Uses ucontext_t / setjmp\nto simulate Context Switch\non PC (Windows/Linux)"
    note for ArmCortexMPort "Uses PendSV Handler\n+ SysTick Interrupt\non real ARM hardware"
```

---

## 5. Full System Overview

```mermaid
classDiagram
    direction TB

    TaskBase <|-- Task~size_t N~
    Scheduler "1" o-- "*" TaskBase
    Scheduler --> PortBase
    PortBase <|.. PosixPort
    PortBase <|.. ArmCortexMPort
    LockGuard --> Mutex
    Mutex ..> TaskBase
    Scheduler ..> Tick

    class TaskBase {
        #priority_
        #stackPtr_
        #entry_
    }
    class Task~size_t N~ {
        -stackMemory_
    }
    class Scheduler {
        -readyList_[]
        -currentTask_
        +schedule()
    }
    class PortBase {
        <<interface>>
    }
    class PosixPort {
        ucontext_t
    }
    class ArmCortexMPort {
        PendSV + SysTick
    }
    class Mutex {
        -owner_
        +lock() +unlock()
    }
    class LockGuard {
        RAII Wrapper
    }
    class Tick {
        -systemTicks_
    }
```
