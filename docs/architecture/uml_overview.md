# 🏗️ LooRTOS — UML Architecture Overview

> **Purpose**: This document serves as the central architecture map for LooRTOS,  
> providing a high-level visualization of the entire system before writing code.

## 📁 UML Diagram Index

| # | Diagram | File | Answers the Question |
|---|---------|------|---------------------|
| 1 | **Class Diagram** | [class_diagram.md](class_diagram.md) | *"What data structures exist and how are they related?"* |
| 2 | **State Machine Diagram** | [state_machine.md](state_machine.md) | *"How does a Task transition through its lifecycle?"* |
| 3 | **Sequence Diagram** | [sequence_diagram.md](sequence_diagram.md) | *"How do modules interact with each other over time?"* |
| 4 | **Component Diagram** | [component_diagram.md](component_diagram.md) | *"What are the major building blocks and their dependencies?"* |

---

## 🎯 Design Principles

```
┌─────────────────────────────────────────────────┐
│              LooRTOS Design Principles          │
├─────────────────────────────────────────────────┤
│  ✦  Zero-cost Abstraction (C++17)               │
│  ✦  No Heap Allocation (Static Memory Only)     │
│  ✦  Type Erasure (TaskBase → Task<N>)           │
│  ✦  BARR Group Coding Standard                  │
│  ✦  MISRA-C:2012 Compliance (Safety-Critical)   │
│  ✦  POSIX Simulation Port (Dev on PC)           │
│  ✦  ARM Cortex-M Target (Production)            │
└─────────────────────────────────────────────────┘
```

## 🗺️ Layered Architecture

```
┌─────────────────────────────────────────────┐
│            User Application Layer           │
│     (blinky, producer_consumer, demo...)    │
├─────────────────────────────────────────────┤
│              Kernel API Layer               │
│   Task │ Scheduler │ Mutex │ Queue │ Timer  │
├─────────────────────────────────────────────┤
│        Hardware Abstraction Layer           │
│        (arch/posix  │  arch/arm-cortex-m)   │
├─────────────────────────────────────────────┤
│           Hardware / Simulator              │
│      (ARM Cortex-M MCU  │  Linux/Windows)   │
└─────────────────────────────────────────────┘
```
