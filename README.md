# Temperature-Based Motor Control System using FreeRTOS

## 📌 Project Overview
This project implements a Real-Time Temperature-Based Motor Control System using **FreeRTOS** on the **ATmega169PA** microcontroller. The system continuously monitors ambient temperature and automatically drives a DC motor if the temperature exceeds a predefined threshold (25°C). It also features an emergency hardware override mechanism using a push button with the highest execution priority.

## 🛠️ Hardware Components
* **Microcontroller:** ATmega169PA
* **Temperature Sensor:** LM35
* **Motor Driver:** L293D
* **Actuator:** DC Motor
* **User Input:** Push Button (Active-Low)
* **Debugging:** Virtual Terminal (UART at 9600 Baud Rate)

## 💻 Software & Tools
* **IDE:** Microchip Studio
* **RTOS:** FreeRTOS
* **Simulation:** Proteus 8 Professional

## ⚙️ System Architecture & RTOS Tasks
The system is designed using a preemptive scheduling approach with 3 main tasks, completely optimized to run stably on a limited 1KB SRAM environment.

### 1. Button Monitor Task (`vTask_ButtonMonitor`)
* **Priority:** Highest (3)
* **Description:** Continuously polls the emergency push button. If pressed, it immediately preempts other tasks and sends a `MOTOR_STATE_STOP` command to the Queue, forcing an override over the temperature readings.

### 2. Motor Control Task (`vTask_MotorControl`)
* **Priority:** Medium (2)
* **Description:** Listens to the `xMotorStateQueue`. It physically controls the L293D driver pins to either run the motor `FORWARD` or `STOP` based on the received commands.

### 3. Temperature Monitor Task (`vTask_TemperatureMonitor`)
* **Priority:** Lowest (1)
* **Description:** Reads the analog voltage from the LM35 sensor, converts it to Celsius without using hardware-intensive division/modulo operations (to prevent Stack Overflow), and sends the desired state to the Queue.

## 🔄 Inter-Process Communication (IPC)
* **Queue (`xMotorStateQueue`):** Used to safely pass motor states (`MOTOR_STATE_FORWARD` / `MOTOR_STATE_STOP`) between tasks.
* **Mutex (`xUART_Mutex`):** Used to protect the UART transmission resource, ensuring that terminal logs from different tasks are not mixed or corrupted during context switching.

## 🚀 How to Run the Simulation
1. Clone this repository.
2. Open the Proteus simulation file located in the `Simulation` folder.
3. Ensure the ATmega169PA is loaded with the latest `.hex` file found in the `Code/Debug` directory.
4. Click **Play** on Proteus.
5. Watch the Virtual Terminal for real-time logs. Adjust the LM35 temperature arrows or press the Push Button to observe the real-time priority-based preemption!
