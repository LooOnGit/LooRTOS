# 🧩 LooRTOS — Component Diagram

> Component diagrams describe the **physical module blocks** of the system  
> and how they **depend on each other** through interfaces.  
> This is the blueprint that maps directly to the project directory structure.

---

## 1. Layered Architecture Overview

```mermaid
block-beta
    columns 1

    block:app["🟦 APPLICATION LAYER"]
        columns 3
        blinky["examples/blinky"]
        producer["examples/producer_consumer"]
        preemption["examples/preemption_demo"]
    end

    block:kernel["🟩 KERNEL LAYER"]
        columns 4
        core["kernel/core"]
        sync["kernel/sync"]
        mem["kernel/mem"]
        timer["kernel/timer"]
    end

    block:hal["🟧 HARDWARE ABSTRACTION LAYER"]
        columns 2
        posix["arch/posix"]
        arm["arch/arm-cortex-m"]
    end

    block:hw["⬛ HARDWARE / SIMULATOR"]
        columns 2
        pc["PC (Windows/Linux)"]
        mcu["ARM Cortex-M MCU"]
    end

    app --> kernel
    kernel --> hal
    posix --> pc
    arm --> mcu
```

---

## 2. Detailed Module Dependencies

> Arrow `A --> B` means A **depends on** B  
> (A needs to include headers or call functions from B to operate).

```mermaid
flowchart TB
    subgraph APP ["🟦 Application Layer"]
        EX1["examples/blinky"]
        EX2["examples/producer_consumer"]
    end

    subgraph CORE ["🟩 kernel/core"]
        TASK["Task.hpp\n(TaskBase + Task<N>)"]
        SCHED["Scheduler.hpp\n(Singleton)"]
        TICK["Tick.hpp\n(System Timer)"]
    end

    subgraph SYNC ["🟩 kernel/sync"]
        MTX["Mutex.hpp\n+ LockGuard"]
        SEM["Semaphore.hpp"]
        QUE["Queue.hpp<T,N>\n(Lock-free)"]
        EVT["EventGroup.hpp"]
    end

    subgraph MEM ["🟩 kernel/mem"]
        POOL["PoolAllocator.hpp<T,N>\n(Placement new)"]
    end

    subgraph TIMER ["🟩 kernel/timer"]
        STMR["SoftTimer.hpp\n(One-shot + Periodic)"]
    end

    subgraph HAL ["🟧 arch/"]
        PORT_IF["PortBase\n(Interface)"]
        POSIX["posix/Port.hpp\n(ucontext_t)"]
        ARM["arm-cortex-m/Port.hpp\n(PendSV + SysTick)"]
        ASM["arm-cortex-m/ContextSwitch.s\n(Assembly)"]
    end

    %% Application depends on Kernel
    EX1 --> TASK
    EX1 --> SCHED
    EX2 --> TASK
    EX2 --> QUE

    %% Core internal dependencies
    SCHED --> TASK
    SCHED --> TICK
    SCHED --> PORT_IF

    %% Sync depends on Core
    MTX --> TASK
    MTX --> SCHED
    SEM --> TASK
    SEM --> SCHED
    QUE -.->|"std::atomic\n(Lock-free)"| TASK
    EVT --> TASK

    %% Timer depends on Core
    STMR --> TICK
    STMR --> SCHED

    %% HAL implementations
    PORT_IF -.-> POSIX
    PORT_IF -.-> ARM
    ARM --> ASM

    %% Styling
    style APP fill:#e3f2fd,stroke:#1565c0,stroke-width:2px
    style CORE fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px
    style SYNC fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px
    style MEM fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px
    style TIMER fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px
    style HAL fill:#fff3e0,stroke:#e65100,stroke-width:2px
```

---

## 3. Project Directory Structure (UML → Code Mapping)

```
LooRTOS/
├── 📁 kernel/                        ← 🟩 KERNEL LAYER
│   ├── 📁 core/                      ← Central brain
│   │   ├── Task.hpp                  ← TaskBase + Task<N> (Type Erasure)
│   │   ├── Scheduler.hpp/.cpp        ← Singleton Scheduler
│   │   └── Tick.hpp/.cpp             ← System Tick Counter
│   │
│   ├── 📁 sync/                      ← Synchronization primitives
│   │   ├── Mutex.hpp/.cpp            ← Binary Mutex + Priority Inheritance
│   │   ├── LockGuard.hpp             ← RAII Wrapper for Mutex
│   │   ├── Semaphore.hpp/.cpp        ← Counting Semaphore
│   │   ├── Queue.hpp                 ← Template Lock-free Ring Buffer
│   │   └── EventGroup.hpp/.cpp       ← Event Flags
│   │
│   ├── 📁 mem/                       ← Static memory management
│   │   └── PoolAllocator.hpp         ← Object Pool (Placement new)
│   │
│   └── 📁 timer/                     ← Software timers
│       └── SoftTimer.hpp/.cpp        ← One-shot + Periodic Timer
│
├── 📁 arch/                          ← 🟧 HARDWARE ABSTRACTION LAYER
│   ├── 📁 posix/                     ← Simulation Port (Dev on PC)
│   │   ├── Port.hpp/.cpp             ← ucontext_t Context Switch
│   │   └── Tick.cpp                  ← POSIX timer simulates SysTick
│   │
│   └── 📁 arm-cortex-m/              ← Production Port (Real hardware)
│       ├── Port.hpp/.cpp             ← SysTick + NVIC Configuration
│       ├── ContextSwitch.s           ← PendSV Handler (ARM Assembly)
│       └── Startup.s                 ← Vector Table
│
├── 📁 examples/                      ← 🟦 APPLICATION LAYER
│   ├── blinky/                       ← LED Blink (Hello World of RTOS)
│   ├── producer_consumer/            ← Queue + 2 Tasks
│   └── preemption_demo/              ← Priority Preemption Demo
│
├── 📁 tests/                         ← ✅ Unit Tests (Google Test)
│
├── 📁 docs/                          ← 📚 Documentation
│   └── architecture/                 ← UML Diagrams (This file!)
│
├── 📁 scripts/                       ← 🔧 Build Scripts
│   ├── build.ps1                     ← PowerShell Build
│   └── build.sh                      ← Bash Build
│
└── CMakeLists.txt                    ← Root Build Configuration
```

---

## 4. Build Flow

```mermaid
flowchart LR
    subgraph INPUT ["📝 Source Code"]
        HPP["*.hpp\n(Headers)"]
        CPP["*.cpp\n(Implementation)"]
        ASM["*.s\n(Assembly)"]
    end

    subgraph CMAKE ["⚙️ CMake"]
        ROOT["CMakeLists.txt\n(Root)"]
        KERN["kernel/CMakeLists.txt"]
        ARCH["arch/CMakeLists.txt"]
    end

    subgraph OUTPUT ["📦 Build Output"]
        LIB_K["libkernel.a\n(Static Library)"]
        LIB_A["libarch.a\n(Static Library)"]
        BIN["loortos_app.elf\n(Final Binary)"]
    end

    HPP --> ROOT
    CPP --> ROOT
    ASM --> ROOT

    ROOT --> KERN
    ROOT --> ARCH

    KERN --> LIB_K
    ARCH --> LIB_A

    LIB_K --> BIN
    LIB_A --> BIN

    style INPUT fill:#fce4ec,stroke:#c62828
    style CMAKE fill:#e8eaf6,stroke:#283593
    style OUTPUT fill:#e8f5e9,stroke:#2e7d32
```
