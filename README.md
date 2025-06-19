# TM4C RTOS

This repository contains an example real-time operating system targeted for the **TM4C123GH6PM** microcontroller.  The project is designed for use with Code Composer Studio and demonstrates how to build a small preemptive kernel together with drivers and shell utilities.

## Features

- **Thread Management** – Supports creation, termination and restart of threads with configurable stack sizes.  Threads can `yield`, `sleep`, and change priority using service calls defined in `kernel.c`.
- **Scheduling** – Implements both round-robin and priority-based scheduling.  Preemption can be toggled on or off via the shell.
- **Synchronization Primitives** – Provides mutexes and semaphores to coordinate access to resources.  Functions such as `lock`, `unlock`, `wait` and `post` are available to tasks.
- **Memory Protection & Allocation** – The Memory Protection Unit is configured in `mpu.c` and `mm.c` to control access to SRAM regions.  A simple heap allocator (`mallocFromHeap`) maps memory blocks to MPU sub-regions.
- **Fault Handling** – `faults.c` installs handlers for memory, usage, bus and hard faults.  Diagnostic information is printed over UART when a fault occurs.
- **Command Shell** – A UART based shell (`shell.c`) accepts commands such as `ps`, `ipcs`, `kill`, `preempt`, `sched` and `pidof`.  These commands are implemented in `userCommands.c` and allow runtime inspection and control of tasks.
- **Example Tasks** – `tasks.c` defines demo threads that flash LEDs, read push buttons and perform lengthy operations to show RTOS behavior.
- **Hardware Support Libraries** – Drivers for GPIO, UART0, NVIC control and busy-wait delays are provided in the `gpio.c`, `uart0.c`, `nvic.c` and `wait.c` modules.

## Building

The project files (`.cproject`, `.ccsproject`, `tm4c123gh6pm.cmd`) are set up for Code Composer Studio.  Import the repository into CCS and build the project to produce firmware for the EK-TM4C123GXL LaunchPad.

## Usage

On startup the `main` function in `rtos.c` initializes hardware, creates the demo threads and starts the RTOS.  Connect to the UART at **115200 8N1** to interact with the shell.  Example commands:

```
ps          # Show task list and CPU usage
ipcs        # Display mutex and semaphore status
kill <pid>  # Terminate a task by PID
sched PRIO  # Use priority scheduler (or `sched RR` for round-robin)
preempt ON  # Enable preemptive switching
```

These commands allow observing the scheduler, toggling preemption and interacting with the running tasks.
