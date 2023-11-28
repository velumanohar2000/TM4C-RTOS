/*
 * userCommands.c
 *
 *  Created on: Nov 11, 2023
 *      Author: velum
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "userCommands.h"
#include "kernel.h"

#define GREEN_LED PORTE, 1
#define YELLOW_LED PORTE, 2
#define RED_LED PORTE, 3
#define ORANGE_LED PORTA, 7
#define BLUE_LED PORTF, 2
#define ON_BOARD_GREEN_LED PORTF, 3
#define ON_BOARD_RED_LED PORTF, 1

#define PB0 PORTB, 1
#define PB1 PORTE, 4
#define PB2 PORTE, 5
#define PB3 PORTB, 4
#define PB4 PORTA, 5
#define PB5 PORTA, 6



void ps()
{
    putsUart0("PS called\n");
}
void ipcs()
{
    MUTEX_INFO mutexInfo;
    SEM_INFO semaphoreInfo;
    uint8_t i = 0;
    uint8_t j = 0;

    putsUart0("\n*********Mutex Info*********\n");
    for (i = 0; i < MAX_MUTEXES; i++)
    {
        j = 0;
        getMutexInfo(&mutexInfo, i);
        putsUart0("Name: ");
        putsUart0(mutexInfo.name);
        putsUart0("\nLock Status: ");
        if (mutexInfo.lockStatus)
        {
            putsUart0("Locked\n");
            putsUart0("Locked By: ");
            putsUart0(mutexInfo.lockedBy);
            putsUart0("\nProcesses in Queue:\n");
            for(j = 0; j < mutexInfo.queueSize; j++)
            {
                putsUart0(mutexInfo.processInQueue[j]);
                putsUart0("\n");
            }
        }
        else
        {
            putsUart0("Unlocked\n");
            putsUart0("No processes in queue\n");
        }
        putsUart0("--------------------------------\n");
    }

    putsUart0("\n*********Semaphore Info*********\n");
    for (i = 0; i < MAX_SEMAPHORES; i++)
    {
        j = 0;
        getSemaphoreInfo(&semaphoreInfo, i);
        putsUart0("Name: ");
        putsUart0(semaphoreInfo.name);
        putsUart0("\nCount: ");
        iToA(semaphoreInfo.count);
        if (semaphoreInfo.count == 0)
        {
            putsUart0("\nProcesses in Queue: \n");
            for(j = 0; j < semaphoreInfo.queueSize; j++)
            {
                putsUart0(semaphoreInfo.processInQueue[j]);
                putsUart0("\n");
            }
        }
        else
        {
            putsUart0("\nNo processes in Queue\n");
        }
        putsUart0("--------------------------------\n");
    }
}
void kill(uint32_t pid)
{
    stopThread((_fn)pid);
}
void Pkill(const char processName[])
{
    int32_t pid = getPid(processName);
    if (pid >= 0)
    {
        kill(pid);
    }
    else
    {
        putsUart0("Invalid Name\n");
    }
}
void preempt(bool on)
{
    if (on)
    {
        putsUart0("Preemption On\n");
    }
    else
    {
        putsUart0("Preemption Off\n");
    }
    togglePreemption(on);
}
void sched(bool prio_on)
{
    if (prio_on)
    {
        putsUart0("Priority Scheduling\n");
    }
    else
    {
        putsUart0("Round-Robin Scheduling Enabled\n");
    }
    schedSelect(prio_on);

}

void pidof(const char name[])
{
    int32_t pid = getPid(name);
    if (pid >= 0)
    {
        putsUart0("PID of ");
        putsUart0((char*)name);
        putsUart0(": ");
        iToA((uint32_t)pid);
        putsUart0("\n");
    }
    else
    {
        putsUart0("Invalid Name\n");
    }
}

void run(const char processName[])
{

    int32_t pid = getPid(processName);
    if (pid >= 0)
    {
        putsUart0("Process ");
        putsUart0((char*)processName);
        putsUart0(" is running\n");
        restartThread((_fn)pid);
    }
    else
    {
        putsUart0("Invalid Name\n");
    }
}
