# 🔄 LooRTOS — State Machine Diagram

> State machine diagrams describe the **dynamic lifecycle** of entities in the system.  
> These diagrams are mandatory to understand before writing the Scheduler  
> to prevent **Race Conditions** and **Deadlocks**.

---

## 1. Task State Machine — Task Lifecycle

> This is the most critical state diagram in the entire RTOS.  
> The variable `volatile State currentState_` in `TaskBase` must follow  
> exactly the transitions (arrows) shown below.  
> Any transition NOT shown in this diagram is a **BUG**.

```mermaid
stateDiagram-v2
    direction TB

    [*] --> READY : Task created\n(Constructor complete)

    state "🟢 READY" as READY
    state "🔵 RUNNING" as RUNNING
    state "🔴 BLOCKED" as BLOCKED
    state "⚪ SUSPENDED" as SUSPENDED

    READY --> RUNNING : Scheduler::schedule()\nSelects highest priority Task

    RUNNING --> READY : Preempted by higher priority\nor Task calls yield()

    RUNNING --> BLOCKED : Task calls delay()\nor Mutex::lock() waits\nor Semaphore::take() waits\nor Queue::pop() empty

    BLOCKED --> READY : Timeout expired\nor Mutex unlocked\nor Semaphore::give()\nor Queue::push() has data

    RUNNING --> SUSPENDED : Task::suspend() called
    READY --> SUSPENDED : Task::suspend() called
    BLOCKED --> SUSPENDED : Task::suspend() called

    SUSPENDED --> READY : Task::resume() called
```

### ⚠️ Valid Transition Table

Cross-reference table — `✅` = Valid, `❌` = FORBIDDEN (If occurs = BUG)

| From ╲ To | READY | RUNNING | BLOCKED | SUSPENDED |
|-----------|-------|---------|---------|-----------|
| **READY** | ❌ | ✅ Scheduler | ❌ | ✅ suspend() |
| **RUNNING** | ✅ Preempt/Yield | ❌ | ✅ Wait/Delay | ✅ suspend() |
| **BLOCKED** | ✅ Event/Timeout | ❌ | ❌ | ✅ suspend() |
| **SUSPENDED** | ✅ resume() | ❌ | ❌ | ❌ |

> **Critical Note**: No transition goes **directly** to `RUNNING`.  
> Only the Scheduler has the authority to move a Task from `READY` → `RUNNING`.  
> A Task cannot run itself!

---

## 2. Mutex State Machine — Lock Lifecycle

> Mutex (Mutual Exclusion) ensures that only **one Task at a time**  
> can access a Shared Resource.

```mermaid
stateDiagram-v2
    direction LR

    state "🔓 UNLOCKED" as UNLOCKED
    state "🔒 LOCKED" as LOCKED

    [*] --> UNLOCKED : Mutex initialized

    UNLOCKED --> LOCKED : Task A calls lock()\n→ owner_ = Task A

    LOCKED --> UNLOCKED : Task A calls unlock()\n→ owner_ = nullptr

    LOCKED --> LOCKED : Task B calls lock()\n→ Task B becomes BLOCKED\n(Waits for Task A to unlock)
```

### Priority Inheritance Protocol

```mermaid
stateDiagram-v2
    direction TB

    state "Task Low (Priority 200)\nHolding Mutex" as LOW_HOLDS
    state "Task High (Priority 10)\nWaiting for Mutex → BLOCKED" as HIGH_WAITS
    state "Task Low inherits Priority 10\n(Boosted from Task High)" as INHERITANCE
    state "Task Low unlocks → Priority 200\nTask High acquires Mutex → RUNNING" as RESOLVED

    LOW_HOLDS --> HIGH_WAITS : Task High calls lock()
    HIGH_WAITS --> INHERITANCE : OS detects\nPriority Inversion
    INHERITANCE --> RESOLVED : Task Low completes\nand calls unlock()
```

---

## 3. Semaphore State Machine — Signal Lifecycle

```mermaid
stateDiagram-v2
    direction LR

    state "COUNT > 0\n(Resource available)" as AVAILABLE
    state "COUNT = 0\n(Resource exhausted)" as EMPTY

    [*] --> AVAILABLE : Initialized with count = N

    AVAILABLE --> AVAILABLE : take() succeeds\ncount-- (count still > 0)
    AVAILABLE --> EMPTY : take() succeeds\ncount-- (count reaches 0)

    EMPTY --> AVAILABLE : give()\ncount++ (wakes blocked Task)

    EMPTY --> EMPTY : take() fails\n→ Task becomes BLOCKED
```

---

## 4. Queue State Machine — Buffer Lifecycle

```mermaid
stateDiagram-v2
    direction LR

    state "EMPTY\n(head == tail)" as EMPTY
    state "PARTIAL\n(has data, not full)" as PARTIAL
    state "FULL\n(head == tail + CAP)" as FULL

    [*] --> EMPTY : Queue initialized

    EMPTY --> PARTIAL : push() succeeds
    EMPTY --> EMPTY : pop() fails → Task BLOCKED

    PARTIAL --> PARTIAL : push() or pop()
    PARTIAL --> EMPTY : pop() → last element
    PARTIAL --> FULL : push() → capacity reached

    FULL --> PARTIAL : pop() succeeds
    FULL --> FULL : push() fails → Task BLOCKED
```

---

## 5. SoftTimer State Machine — Timer Lifecycle

```mermaid
stateDiagram-v2
    direction TB

    state "⏸️ STOPPED" as STOPPED
    state "▶️ RUNNING" as RUNNING
    state "🔔 EXPIRED" as EXPIRED

    [*] --> STOPPED : Timer initialized

    STOPPED --> RUNNING : start(period)
    RUNNING --> EXPIRED : tickCount >= expiryTime

    EXPIRED --> STOPPED : One-shot Timer\n→ Callback fires + Auto-stop
    EXPIRED --> RUNNING : Periodic Timer\n→ Callback fires + Reset expiryTime

    RUNNING --> STOPPED : stop() called manually
```
