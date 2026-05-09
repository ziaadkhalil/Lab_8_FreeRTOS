# Lab 8 — Introduction to FreeRTOS
### ATmega169PA | Microchip Studio Setup Guide

---

## Part 1 — FreeRTOS Overview

### 1.1 What is FreeRTOS?

FreeRTOS is a real-time operating system kernel for microcontrollers. It lets you write multiple independent tasks that appear to run simultaneously, managed by a scheduler that decides which task gets the CPU at any moment.

**Without FreeRTOS — bare metal:**
```c
while (1) {
    read_sensor();   // blocks everything else
    blink_led();     // nothing runs during this
    send_uart();
}
```

**With FreeRTOS — multitasking:**
```
Task1: blink LED every 500ms      // independent
Task2: read sensor every 1s       // independent
Task3: send UART every 2s         // independent
```


### 1.2 Core Task API

| Function | Purpose |
|---|---|
| `xTaskCreate(func, name, stack, param, priority, handle)` | Create a new task |
| `vTaskDelay(pdMS_TO_TICKS(ms))` | Block task for given milliseconds |
| `vTaskStartScheduler()` | Start the scheduler — never returns |
| `vTaskDelete(handle)` | Delete a task |
| `vTaskSuspend(handle)` | Pause a task |
| `vTaskResume(handle)` | Resume a paused task |
| `vTaskPrioritySet(handle, priority)` | Change task priority at runtime |

---

### 1.3 Scheduling & Priorities

FreeRTOS uses preemptive scheduling. The highest-priority READY task always runs first. Equal-priority tasks share the CPU round-robin.

- Priority 0 = lowest (IDLE task runs here)
- Higher number = higher priority
- A high-priority task immediately preempts a lower-priority task the moment it becomes READY

---

## Part 2 — Mutex

### 2.1 What is a Mutex?

A mutex (Mutual Exclusion) ensures only one task can access a shared resource at a time. Think of it as a bathroom key — only one person can have it.

**Without mutex — race condition:**
```
Task1 starts printing "[Task1] Hello..."
Scheduler switches to Task2 mid-print
Task2 also prints "[Task2] Hello..."
Result: "[Task1] He[Task2] Hello...llo..."  ← garbled!
```

**With mutex — safe access:**
```
Task1 takes mutex → prints fully → gives mutex
Task2 tries to take mutex → BLOCKS until Task1 gives it
Result: clean, complete messages every time
```

---

### 2.2 Mutex API

| Function | Purpose |
|---|---|
| `xSemaphoreCreateMutex()` | Create a mutex — returns a handle |
| `xSemaphoreTake(mutex, portMAX_DELAY)` | Lock the mutex — blocks if already taken |
| `xSemaphoreGive(mutex)` | Release the mutex — unblocks waiting tasks |

---

### 2.3 Mutex Usage Pattern

```c
SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();

// In any task that uses the shared resource:
if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
{
    // safe to use shared resource here
    xSemaphoreGive(xMutex);   // ALWAYS give it back!
}
```

> ⚠️ **Always call `xSemaphoreGive()` after `xSemaphoreTake()`.**  
> Forgetting it causes deadlock — every task waiting for the mutex blocks forever.

> ⚠️ **Create the mutex BEFORE creating tasks.**  
> Tasks starting before the mutex exists will try to take a NULL handle and crash.

---

### 2.4 Priority Inheritance

FreeRTOS mutexes support priority inheritance. If a low-priority task holds the mutex and a high-priority task is waiting, FreeRTOS temporarily raises the low-priority task's priority so it finishes and releases the mutex as fast as possible. This prevents priority inversion.

---

## Part 3 — Queue

### 3.1 What is a Queue?

A queue is a FIFO buffer for passing data safely between tasks. Unlike sharing a global variable, a queue is thread-safe by design — no mutex needed for the queue itself.

```
Task1 (Producer)  →  Queue [A|B|C]  →  Task2 (Consumer)
sends data             mailbox           receives data
```

- If queue is **full** → producer **BLOCKS** until space is available
- If queue is **empty** → consumer **BLOCKS** until data arrives
- Tasks use zero CPU while blocked on a queue

---

### 3.2 Queue API

| Function | Purpose |
|---|---|
| `xQueueCreate(length, itemSize)` | Create a queue |
| `xQueueSend(queue, &data, timeout)` | Send a copy of data to the queue |
| `xQueueOverwrite(queue, &data)` | Send and overwrite if full — for single-slot queues |
| `xQueueReceive(queue, &buffer, timeout)` | Receive data — blocks if empty |

---

### 3.3 Queue Usage Pattern

```c
QueueHandle_t xQueue = xQueueCreate(5, sizeof(uint8_t));

// Producer task:
uint8_t value = 42;
xQueueSend(xQueue, &value, portMAX_DELAY);

// Consumer task:
uint8_t received;
xQueueReceive(xQueue, &received, portMAX_DELAY);
```

> **Tip:** Use `xQueueOverwrite()` when your queue has size 1 and you only care about the latest value (e.g. current button state or motor command). It never blocks.


## Part 4 — Microchip Studio Setup

### 4.1 Hardware Wiring

| Signal | ATmega169PA Pin | Notes |
|---|---|---|
| LED | PB0 | 220Ω resistor to GND |
| UART TX | PE1 | Connect to **TX** of USB-to-TTL adapter |
| UART RX | PE0 | Connect to **RX** of USB-to-TTL adapter |
| GND | GND | Common ground with USB-TTL adapter |

**Serial Terminal Settings:** `9600 baud | 8-bit | No parity | 1 stop bit`


### 4.2 Download FreeRTOS

1. Go to: https://www.freertos.org/a00104.html
2. Download the latest zip (e.g. `FreeRTOSv10.x.x.zip`)
3. Extract to a memorable location (e.g. `C:\FreeRTOS\`)

---

### 4.3 Create Project in Microchip Studio

1. **File → New → Project**
2. Select: `GCC C Executable Project`
3. Name the project → click **OK**
4. Device selector → search `ATmega169PA` → select → **OK**

---

### 4.4 Project Folder Structure

```
YourProject/
├── main.c
├── FreeRTOSConfig.h
└── FreeRTOS/Source/
    ├── tasks.c
    ├── queue.c
    ├── list.c
    ├── include/              ← copy entire folder from zip
    └── portable/
        ├── GCC/ATMega323/
        │   ├── port.c        ← requires manual fix (see Section 4.6)
        │   └── portmacro.h
        └── MemMang/
            └── heap_1.c
```

---

### 4.5 Files to Copy from FreeRTOS zip

| Copy from zip | Destination |
|---|---|
| `FreeRTOS/Source/tasks.c` | `FreeRTOS/Source/` |
| `FreeRTOS/Source/queue.c` | `FreeRTOS/Source/` |
| `FreeRTOS/Source/list.c` | `FreeRTOS/Source/` |
| `FreeRTOS/Source/include/` (entire folder) | `FreeRTOS/Source/include/` |
| `FreeRTOS/Source/portable/GCC/ATMega323/port.c` | `FreeRTOS/Source/portable/GCC/ATMega323/` |
| `FreeRTOS/Source/portable/GCC/ATMega323/portmacro.h` | `FreeRTOS/Source/portable/GCC/ATMega323/` |
| `FreeRTOS/Source/portable/MemMang/heap_1.c` | `FreeRTOS/Source/portable/MemMang/` |

After copying: right-click project → **Add → Existing Item** → add all `.c` files.

---

### 4.6 ⚠️ Critical Fix — port.c for ATmega169PA

> **This fix is REQUIRED.** Without it the FreeRTOS tick interrupt never fires.  
> Symptom: LED turns ON but never toggles again.

Open `port.c` and make exactly **two changes:**

**Fix 1 — Wrong interrupt bit (top of file)**

Find:
```c
#define portCOMPARE_MATCH_A_INTERRUPT_ENABLE    ( ( uint8_t ) 0x10 )
```
Change to:
```c
#define portCOMPARE_MATCH_A_INTERRUPT_ENABLE    ( ( uint8_t ) 0x02 )
```

**Fix 2 — Wrong timer register (inside `prvSetupTimerInterrupt()`)**

Find:
```c
ucLowByte = TIMSK0;
ucLowByte |= portCOMPARE_MATCH_A_INTERRUPT_ENABLE;
TIMSK0 = ucLowByte;
```
Change to:
```c
ucLowByte = TIMSK1;
ucLowByte |= portCOMPARE_MATCH_A_INTERRUPT_ENABLE;
TIMSK1 = ucLowByte;
```

> This fix is only needed for ATmega169PA. ATmega32 works with the original port.c.

---

### 4.7 Set Include Paths

1. Right-click project → **Properties**
2. **Toolchain → AVR/GNU C Compiler → Directories**
3. Add: `../FreeRTOS/Source/include`
4. Add: `../FreeRTOS/Source/portable/GCC/ATMega323`
5. Click **OK**

---

### 4.8 Set CPU Frequency

1. Right-click project → **Properties**
2. **Toolchain → AVR/GNU C Compiler → Symbols**
3. Add symbol: `F_CPU=8000000UL`
4. Also set in `FreeRTOSConfig.h`: `configCPU_CLOCK_HZ = 8000000UL`

> ⚠️ **CLKDIV8 Fuse:** ATmega169PA has CLKDIV8 programmed by default, dividing the clock by 8 (8MHz becomes 1MHz). This breaks UART baud rate and FreeRTOS timing.  
> In Proteus: right-click MCU → Edit Properties → CLKDIV8 → set to **(1) Unprogrammed**.

---

### 4.9 FreeRTOSConfig.h — Recommended Values

```c
#define configUSE_PREEMPTION              1
#define configCPU_CLOCK_HZ                8000000UL
#define configTICK_RATE_HZ                1000
#define configMAX_PRIORITIES              4
#define configMINIMAL_STACK_SIZE          60     // keep small — controls IDLE task
#define configTOTAL_HEAP_SIZE             700    // tune per number of tasks
#define configUSE_16_BIT_TICKS            1
#define configUSE_MUTEXES                 1
#define configUSE_COUNTING_SEMAPHORES     1
#define INCLUDE_vTaskDelay                1
#define INCLUDE_vTaskDelete               1
```

> ⚠️ **ATmega169PA has only 1KB RAM.** Each task costs `(stack words × 2) + ~60 bytes` overhead. Mutex costs ~80 bytes. Queue costs ~40 bytes. Exceeding 1KB gives a `.bss section overflow` linker error.

---

### 4.10 Build and Flash

1. **Build → Build Solution (F7)** → confirm *Build succeeded*
2. **Tools → Device Programming**
3. Tool: `Atmel-ICE` | Device: `ATmega169PA` | Interface: `JTAG`
4. JTAG Clock: **1 MHz** (default 7.5MHz will fail)
5. Click **Apply** → Device Signature should read `0x1E 0x94 0x05`
6. **Memories tab** → browse to `.hex` file → click **Erase** then **Program**

> ⚠️ **Always click Program, not just Verify.** Verify only checks what is already on the chip — it does not flash your code.

---

### 4.11 Open UART Terminal

1. Connect USB-to-TTL adapter to PE0/PE1 as shown in wiring table
2. Install CH340 driver if needed: https://www.wch-ic.com/downloads/CH341SER_EXE.html *(run as Administrator)*
3. Open Device Manager → Ports (COM & LPT) → note your COM number
4. **Tools → Data Visualizer → Serial Port** → select COM port → 9600 baud → Connect

---

### 4.12 Proteus Simulation Setup

1. Place `ATmega169PA` component
2. Right-click → Edit Properties:
   - **CLKDIV8** → `(1) Unprogrammed`
   - **CKSEL Fuses** → `Int. RC Osc. 8MHz`
3. Connect **AVCC** pin to VCC (5V) — if floating, ADC reads garbage values
4. Connect **AREF** pin to VCC through 100nF capacitor to GND
5. Virtual Terminal: **RXD → PE1 (MCU TX)**, **TXD → PE0 (MCU RX)**
6. Virtual Terminal settings: 9600 baud | 8-bit | No parity | 1 stop bit

---