# 🔀 LooRTOS — Sequence Diagram

> Sequence diagrams describe the **interaction flow over time** between modules.  
> Read top-to-bottom = chronological order of events on the system.

---

## 1. System Boot Sequence

> Scenario: User creates 2 Tasks, then calls Kernel::start()  
> to begin multitasking.

```mermaid
sequenceDiagram
    autonumber
    participant App as 👤 User App
    participant T1 as Task A<br>(Priority High)
    participant T2 as Task B<br>(Priority Low)
    participant Sch as Scheduler
    participant Port as Port (HAL)
    participant CPU as 🔧 CPU

    App->>T1: Task<512> taskA(blinkLed, HIGH)
    Note right of T1: Constructor:<br>state_ = READY<br>stackPtr_ = top of stackMemory_

    App->>T2: Task<256> taskB(printUart, LOW)
    Note right of T2: Constructor:<br>state_ = READY<br>stackPtr_ = top of stackMemory_

    App->>Sch: addTask(&taskA)
    Sch->>Sch: Insert taskA into readyList_[HIGH]

    App->>Sch: addTask(&taskB)
    Sch->>Sch: Insert taskB into readyList_[LOW]

    App->>Sch: start()
    Sch->>Port: initTick(1000Hz)
    Port->>CPU: Configure SysTick / POSIX Timer

    Sch->>Sch: schedule()
    Note over Sch: Scan readyList_[]<br>from highest to lowest priority<br>→ Select taskA

    Sch->>T1: setState(RUNNING)
    Sch->>Port: triggerContextSwitch()
    Port->>CPU: Load taskA's stackPtr_<br>into SP register
    CPU->>T1: Jump to entry_ → blinkLed()

    Note over CPU,T1: 🟢 Task A is now running...
```

---

## 2. Context Switch on Tick Interrupt

> Scenario: SysTick Interrupt fires every 1ms.  
> Scheduler checks whether a context switch is needed.

```mermaid
sequenceDiagram
    autonumber
    participant ISR as ⚡ SysTick ISR
    participant Tick as Tick Manager
    participant Sch as Scheduler
    participant TA as Task A<br>(RUNNING)
    participant TB as Task B<br>(READY)
    participant Port as Port (HAL)
    participant CPU as 🔧 CPU

    ISR->>Tick: tick()
    Tick->>Tick: systemTicks_++

    Tick->>Sch: schedule()

    alt Task B has higher priority OR Round-Robin timeout
        Sch->>Port: disableInterrupts()
        Note over Port: Protect Critical Section

        Sch->>TA: setState(READY)
        Sch->>CPU: Save registers R4-R11<br>into Task A's stackMemory_
        Note right of TA: Context SAVED

        Sch->>TB: setState(RUNNING)
        Sch->>CPU: Restore registers R4-R11<br>from Task B's stackMemory_
        Note right of TB: Context RESTORED

        Sch->>Port: restoreInterrupts()
        CPU->>TB: Resume execution of Task B
    else Task A remains highest priority
        Note over Sch: No switch needed.<br>Task A continues running.
    end
```

---

## 3. Task Calls delay() — Self-Blocking

> Scenario: Task A calls delay(500) to sleep for 500 ticks.

```mermaid
sequenceDiagram
    autonumber
    participant TA as Task A<br>(RUNNING)
    participant Sch as Scheduler
    participant Tick as Tick Manager
    participant TB as Task B<br>(READY)
    participant Port as Port (HAL)

    TA->>Sch: delay(500)
    Sch->>TA: setState(BLOCKED)
    Sch->>Tick: Record: taskA wakes up<br>at tick = now + 500

    Sch->>Sch: schedule()
    Note over Sch: Task A removed from readyList_<br>→ Select Task B

    Sch->>TB: setState(RUNNING)
    Sch->>Port: triggerContextSwitch()

    Note over TB: 🟢 Task B is running...

    loop Every SysTick (500 times)
        Tick->>Tick: systemTicks_++
        Tick->>Tick: Check blocked task list
    end

    Tick->>TA: Timeout expired!
    Tick->>TA: setState(READY)
    Tick->>Sch: Re-insert Task A into readyList_

    Note over Sch: Next schedule() call<br>may select Task A again
```

---

## 4. Mutex Lock/Unlock — Protecting Shared Resources

> Scenario: Task A locks a Mutex, Task B also wants to lock → BLOCKED.

```mermaid
sequenceDiagram
    autonumber
    participant TA as Task A<br>(RUNNING)
    participant M as Mutex
    participant Sch as Scheduler
    participant TB as Task B<br>(READY)
    participant Port as Port (HAL)

    TA->>M: lock()
    M->>M: owner_ = Task A<br>locked_ = true
    Note right of M: 🔒 Mutex LOCKED

    Note over TA: Accessing<br>Shared Resource...

    TA-->>Sch: SysTick → Preemption
    Sch->>TB: setState(RUNNING)

    TB->>M: lock()
    M->>M: Mutex held by Task A!
    M->>TB: setState(BLOCKED)
    M->>Sch: Insert Task B into Mutex waitList

    Sch->>Sch: schedule()
    Sch->>TA: setState(RUNNING)

    Note over TA: Finishes work

    TA->>M: unlock()
    M->>M: owner_ = nullptr<br>locked_ = false
    Note right of M: 🔓 Mutex UNLOCKED

    M->>TB: setState(READY)
    M->>Sch: Re-insert Task B into readyList_

    Sch->>Sch: schedule()
    Note over Sch: Task B selected → RUNNING
    TB->>M: lock() succeeds!
```

---

## 5. Producer-Consumer via Queue

> Scenario: Producer Task pushes data into Queue,  
> Consumer Task pops data out for processing.

```mermaid
sequenceDiagram
    autonumber
    participant Prod as 📤 Producer Task
    participant Q as Queue~Data, 8~
    participant Cons as 📥 Consumer Task
    participant Sch as Scheduler

    Note over Q: Queue is empty (EMPTY)

    Cons->>Q: pop(data)
    Q->>Cons: Queue empty → return false
    Cons->>Sch: setState(BLOCKED)<br>Waiting for Queue data

    Sch->>Prod: setState(RUNNING)

    Prod->>Q: push(sensorReading)
    Q->>Q: buffer_[tail_++] = data
    Note over Q: Queue PARTIAL

    Q->>Cons: Wake up Consumer!
    Q->>Sch: setState(READY) for Consumer

    Sch->>Sch: schedule()
    Note over Sch: Consumer has higher priority?<br>→ Preempt Producer

    Sch->>Cons: setState(RUNNING)
    Cons->>Q: pop(data)
    Q->>Cons: data = sensorReading ✅
    Note over Cons: Processing data...
```
