// RTOS Framework - Fall 2023
// J Losh

// Student Name:
// TO DO: Velu Manohar
//        Do not include your ID number(s) in the file.

// Please do not change any function name in this code or the thread priorities

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// 6 Pushbuttons and 5 LEDs, UART
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1
// Memory Protection Unit (MPU):
//   Region to control access to flash, peripherals, and bitbanded areas
//   4 or more regions to allow SRAM access (RW or none for task)

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------
#include <stdbool.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "gpio.h"
#include "uart0.h"
#include "wait.h"
#include "mm.h"
#include "kernel.h"
#include "faults.h"
#include "tasks.h"
#include "shell.h"
#include "asm.h"
//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------


int main(void)
{

    bool ok = false;

    // Initialize hardware
    initSystemClockTo40Mhz();
    initHw();    //in tasks.h because each task will have different hardware configs
    initUart0();
    initMpu();  //in mm.h
    initRtos(); //in kernel.h
    initSysTickIsr();
    initFaultHandlers();

    // Setup UART0 baud rate
    setUart0BaudRate(115200, 40e6);

    // Initialize mutexes and semaphores
     initMutex(resource, "Resource");
     initSemaphore(keyPressed, 1, "Key Pressed");
     initSemaphore(keyReleased, 0, "Key Released");
     initSemaphore(flashReq, 5, "Flash Req");

    // // Add required idle process at lowest priority
     ok =  createThread(idle, "Idle", 7, 512);
     //ok =  createThread(idle2, "Idle2", 7, 512);


     // Add other processes
     ok &= createThread(lengthyFn, "LengthyFn", 6, 1024);
     ok &= createThread(flash4Hz, "Flash4Hz", 4, 1024);
     ok &= createThread(oneshot, "OneShot", 2, 1024);
     ok &= createThread(readKeys, "ReadKeys", 6, 1024);
     ok &= createThread(debounce, "Debounce", 6, 1024);
     ok &= createThread(important, "Important", 0, 1024);
     ok &= createThread(uncooperative, "Uncoop", 6, 1024);
     ok &= createThread(errant, "Errant", 6, 1024);
     ok &= createThread(shell, "Shell", 6, 4096);


    // Start up RTOS
    if (ok)
        startRtos(); // never returns
    else
        while(true);
}
