# LooRTOS — Lightweight C++ RTOS

Xây dựng một RTOS từ đầu bằng C++ hiện đại (C++17/20), nhắm mục tiêu ARM Cortex-M
với một **POSIX/x86 simulation port** để có thể dev/test ngay trên máy Windows/Linux mà không cần phần cứng.

> Triết lý: **Zero-cost abstractions** — mọi pattern phải compile xuống code ngang bằng C thuần.
> Không dùng: `dynamic_cast`, exceptions, `std::vector` (heap), RTTI.

---

## Mục tiêu học tập (C++ Concepts)

| Tính năng | Dùng ở đâu |
|-----------|-----------|
| Templates & Template Metaprogramming | Generic Queue, Pool allocator |
| RAII | `LockGuard`, `ScopedIRQDisable` |
| Inheritance & Virtual functions | HAL port abstraction |
| `constexpr` / `if constexpr` | Compile-time config |
| Move semantics | Task lambda capture |
| Type traits (`std::is_trivially_*`) | Memory pool safety checks |
| `std::atomic` | Lock-free ring buffer |
| Lambdas & `std::function` | Task entry points |
| Namespaces & ADL | Kernel namespace isolation |
| Operator overloading | Duration literals (`500_ms`) |

---

## Embedded Design Patterns

### 1. CRTP — Curiously Recurring Template Pattern
> **Dùng ở**: HAL Port abstraction (thay thế `virtual` functions → zero vtable overhead)

```cpp
// Thay vì virtual:
template <typename Derived>
class PortBase {
public:
    void enableTick(uint32_t hz) {
        static_cast<Derived*>(this)->enableTick_impl(hz);
    }
    void contextSwitch() {
        static_cast<Derived*>(this)->contextSwitch_impl();
    }
};

class ArmCortexMPort : public PortBase<ArmCortexMPort> {
    void enableTick_impl(uint32_t hz) { /* SysTick config */ }
    void contextSwitch_impl()         { /* trigger PendSV */ }
};

class PosixPort : public PortBase<PosixPort> {
    void enableTick_impl(uint32_t hz) { /* POSIX timer */ }
    void contextSwitch_impl()         { /* swapcontext() */ }
};
// → Compiler inlines everything, zero overhead!
```

---

### 2. Policy-Based Design
> **Dùng ở**: Scheduler — swap algorithm tại compile time

```cpp
struct RoundRobinPolicy {
    static Task* select(ReadyList& list) { /* ... */ }
};
struct PriorityPolicy {
    static Task* select(ReadyList& list) { /* highest priority first */ }
};
struct EDFPolicy {
    static Task* select(ReadyList& list) { /* earliest deadline first */ }
};

template <typename SchedPolicy, typename LockPolicy>
class Scheduler {
    Task* getNext() { return SchedPolicy::select(readyList_); }
};

// Dùng:
using MyScheduler = Scheduler<PriorityPolicy, InterruptLock>;
```

---

### 3. RAII Guards (Embedded flavor)
> **Dùng ở**: Mutex, Critical section, ISR disable

```cpp
// ISR Disable Guard — bảo vệ shared data trong ISR context
class CriticalSection {
public:
    CriticalSection()  { primask_ = __get_PRIMASK(); __disable_irq(); }
    ~CriticalSection() { __set_PRIMASK(primask_); }  // restore, không phải enable blindly
    // Non-copyable!
    CriticalSection(const CriticalSection&) = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;
private:
    uint32_t primask_;
};

// Dùng:
void someISRSafeFunc() {
    CriticalSection cs;   // disable IRQ
    sharedData++;         // safe
}                         // ~CriticalSection() restore IRQ tự động
```

---

### 4. Singleton (Meyers' Singleton)
> **Dùng ở**: `Kernel`, `Scheduler` — chỉ có 1 instance

```cpp
class Kernel {
public:
    static Kernel& instance() {
        static Kernel k;  // thread-safe trong C++11+, zero dynamic alloc
        return k;
    }
    void start();   // bắt đầu scheduling
private:
    Kernel() = default;  // private constructor
};
```

---

### 5. Object Pool (No-heap allocator)
> **Dùng ở**: `PoolAllocator<Task, MAX_TASKS>` — không bao giờ dùng `new/delete` trong kernel

```cpp
template <typename T, std::size_t N>
class ObjectPool {
public:
    template <typename... Args>
    T* allocate(Args&&... args) {
        for (auto& slot : storage_) {
            if (!slot.used) {
                slot.used = true;
                return new (&slot.data) T(std::forward<Args>(args)...); // placement new!
            }
        }
        return nullptr;  // pool full, không throw exception
    }
    void deallocate(T* ptr) {
        ptr->~T();  // explicit destructor
        // mark slot as free
    }
private:
    struct Slot {
        alignas(T) std::byte data[sizeof(T)];  // aligned raw storage
        bool used = false;
    };
    std::array<Slot, N> storage_;
};
```

---

### 6. Active Object Pattern
> **Dùng ở**: High-level task abstraction — mỗi task CÓ queue riêng, xử lý messages
> Đây là nền tảng của Actor Model / event-driven embedded firmware

```cpp
template <typename MsgType, std::size_t QueueSize>
class ActiveObject {
public:
    void post(const MsgType& msg) { queue_.push(msg); }
protected:
    virtual void onMessage(const MsgType& msg) = 0;
private:
    void run() {                          // task entry point
        MsgType msg;
        while (true) {
            queue_.pop(msg);              // blocking wait
            onMessage(msg);              // dispatch
        }
    }
    Queue<MsgType, QueueSize> queue_;
    Task task_{[this]{ run(); }, Priority::Normal};
};

// Dùng:
struct LedMsg { bool on; };
class LedController : public ActiveObject<LedMsg, 4> {
    void onMessage(const LedMsg& msg) override {
        gpio_write(LED_PIN, msg.on);
    }
};
```

---

### 7. State Machine (via `std::variant` hoặc enum + table)
> **Dùng ở**: Task state transitions, Protocol handlers

```cpp
enum class TaskState : uint8_t {
    Ready, Running, Blocked, Suspended, Deleted
};

// Transition table approach (data-driven, dễ extend)
constexpr bool validTransition[5][5] = {
//  Ready  Run  Block  Susp  Del
    {0,    1,   0,     1,    1},  // from Ready
    {1,    0,   1,     1,    1},  // from Running
    {1,    0,   0,     1,    1},  // from Blocked
    {1,    0,   0,     0,    1},  // from Suspended
    {0,    0,   0,     0,    0},  // from Deleted
};
```

---

### 8. Observer / Callback (type-erased, no heap)
> **Dùng ở**: Soft Timer callbacks, Event group notifications

```cpp
// Tránh std::function (có thể alloc heap)
// Dùng inplace_function (fixed-size storage)
template <typename Sig, std::size_t StorageSize = 32>
class Callback;  // custom implementation

// Hoặc đơn giản hơn với raw function pointer + void* context:
struct TimerCallback {
    void (*fn)(void* ctx);
    void* ctx;
    void invoke() { if (fn) fn(ctx); }
};
```

---

### Pattern Summary — Phân bổ theo component

| Component | Patterns áp dụng |
|-----------|------------------|
| `Kernel` | Singleton, CRTP |
| `Scheduler` | Policy-based, Strategy |
| `Mutex/Semaphore` | RAII, Null Object (NullMutex) |
| `Queue<T,N>` | Object Pool, Lock-free |
| `PoolAllocator` | Object Pool, Placement new |
| `SoftTimer` | Observer/Callback, Sorted list |
| `Task` | State Machine, Active Object |
| `Port (HAL)` | CRTP, Abstract Factory |

---

## Architecture

```
LooRTOS/
├── kernel/
│   ├── core/
│   │   ├── Task.hpp / Task.cpp          # Task control block (TCB)
│   │   ├── Scheduler.hpp / .cpp         # Priority preemptive scheduler
│   │   ├── Kernel.hpp / .cpp            # Kernel init & main loop
│   │   └── Tick.hpp / .cpp              # System tick management
│   ├── sync/
│   │   ├── Mutex.hpp / .cpp             # Binary mutex + priority inheritance
│   │   ├── Semaphore.hpp / .cpp         # Counting semaphore
│   │   ├── EventGroup.hpp / .cpp        # Event flags (như FreeRTOS)
│   │   └── Queue.hpp                    # Template message queue
│   ├── mem/
│   │   ├── PoolAllocator.hpp            # Static fixed-size block pool
│   │   └── HeapAllocator.hpp            # Simple first-fit heap
│   └── timer/
│       └── SoftTimer.hpp / .cpp         # One-shot & periodic software timers
│
├── arch/                                # Port-specific code
│   ├── arm-cortex-m/
│   │   ├── ContextSwitch.s              # PendSV handler (assembly)
│   │   ├── Port.hpp / .cpp              # SysTick, NVIC, WFI
│   │   └── Startup.s                   # Vector table
│   └── posix/
│       ├── Port.hpp / .cpp              # POSIX threads simulate context switch
│       └── Tick.cpp                     # POSIX timer để sim SysTick
│
├── drivers/                             # Optional thin HAL (học thêm)
│   ├── UART.hpp
│   └── GPIO.hpp
│
├── examples/
│   ├── blinky/                          # LED blink = hello world của RTOS
│   ├── producer_consumer/               # Queue + 2 tasks
│   └── preemption_demo/                 # Priority preemption demo
│
├── tests/
│   └── (Google Test unit tests chạy trên host)
│
├── CMakeLists.txt                       # CMake build system
└── README.md
```

---

## Proposed Changes (Roadmap theo Phase)

### Phase 1 — Foundation (Tuần 1–2)

#### [NEW] `kernel/core/Task.hpp`
- Class `Task` với TCB: stack pointer, priority, state (`Ready/Running/Blocked/Suspended`)
- Stack allocation tĩnh dùng template: `Task<STACK_SIZE>`
- Task entry là lambda hoặc function pointer

#### [NEW] `kernel/core/Scheduler.hpp/.cpp`
- Priority-based preemptive scheduler
- Ready list dùng `std::array` của linked lists, 1 list/priority
- `schedule()` chọn task highest priority trong Ready list
- `yield()` / `delay(ticks)` để task tự nhường CPU

#### [NEW] `arch/posix/Port.hpp/.cpp`
- Dùng `ucontext_t` (POSIX) hoặc `setjmp/longjmp` để simulate context switch
- POSIX `timer_create` → SysTick interrupt giả

#### [NEW] `CMakeLists.txt`
- Build target: `posix` (simulation) và `arm-cortex-m` (cross-compile)

---

### Phase 2 — Synchronization (Tuần 3–4)

#### [NEW] `kernel/sync/Mutex.hpp/.cpp`
- **Binary mutex** với priority inheritance cơ bản
- RAII wrapper: `LockGuard<Mutex>` (học C++ RAII)

#### [NEW] `kernel/sync/Semaphore.hpp/.cpp`
- Counting semaphore
- `give()` / `take(timeout)` với blocking

#### [NEW] `kernel/sync/Queue.hpp`
- **Template class**: `Queue<T, CAPACITY>`
- Lock-free với `std::atomic` head/tail (học lockless programming)

---

### Phase 3 — Memory & Timers (Tuần 5–6)

#### [NEW] `kernel/mem/PoolAllocator.hpp`
- Template: `PoolAllocator<T, N>` — N blocks cố định
- Không dùng heap (`new/delete`) trong kernel
- Học: placement new, aligned storage

#### [NEW] `kernel/timer/SoftTimer.hpp/.cpp`
- One-shot & periodic timers
- Timer list sorted by expiry
- Callback dùng `std::function<void()>` hoặc raw function pointer

---

### Phase 4 — ARM Port (Tuần 7–8)

#### [NEW] `arch/arm-cortex-m/ContextSwitch.s`
- PendSV handler: save/restore R4–R11, PSP
- SVC handler: kernel entry

#### [NEW] `arch/arm-cortex-m/Port.hpp/.cpp`
- SysTick config, `__WFI`, `__disable_irq`
- Targets: STM32F4 hoặc RP2040

---

### Phase 5 — Polish (Tuần 9–10)

#### [MODIFY] `kernel/core/Scheduler.hpp`
- Thêm **EDF (Earliest Deadline First)** scheduler option
- Compile-time switch via `if constexpr`

#### [NEW] `tests/`
- Google Test: test Semaphore, Queue, PoolAllocator trên host
- Valgrind để check memory leaks

---

## C++ Idioms được học qua từng phase

```
Phase 1: Classes, constructors, destructors, templates cơ bản, namespaces
Phase 2: RAII, move semantics, std::atomic, template specialization
Phase 3: Placement new, aligned_storage, std::function, variadic templates
Phase 4: extern "C", volatile, memory-mapped I/O với reinterpret_cast
Phase 5: constexpr, if constexpr, type_traits, concepts (C++20)
```

---

## Verification Plan

### Build & Test (Host)
```bash
cmake -B build -DPORT=posix
cmake --build build
./build/tests/rtos_tests          # Google Test
./build/examples/producer_consumer
```

### Hardware Test
- Flash onto STM32F4 Discovery hoặc Raspberry Pi Pico
- UART log task switching + semaphore timings

---

## Open Questions

> [!IMPORTANT]
> **Target hardware?** STM32F4, RP2040, hay chỉ cần chạy trên POSIX simulation?
> Nếu chỉ học C++ thì POSIX port là đủ, không cần mua thêm board.

> [!NOTE]
> **Build toolchain**: Dùng GCC + CMake hay Meson? Đề xuất CMake vì phổ biến trong embedded.

> [!NOTE]
> **C++ Standard**: C++17 là an toàn nhất (supported bởi arm-none-eabi-g++ mới). C++20 nếu muốn học Concepts.
