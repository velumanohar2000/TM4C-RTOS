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
#include <stdio.h>

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

int stringLength(const char string[])
{
    uint8_t i = 0;
    while(string[i] != '\0')
        i++;
    return i;
}

void dashLine(uint16_t dashCount)
{
    uint8_t i = 0;
    for (i = 0; i < dashCount; i++)
        putsUart0("-");
    putsUart0("\n");
}

void starTitlePad(uint16_t titleSize, const char string[])
{
    uint16_t i = 0;
    uint16_t starPad = (titleSize - stringLength(string))/2;
    for (i = 0; i < starPad; i++)
    {
        putsUart0("*");
    }
    putsUart0((char*)string);
    for (i = 0; i < starPad; i++)
    {
        putsUart0("*");
    }
    putsUart0("\n");

}

void format(uint16_t numSpace)
{
    uint8_t i = 0;
    for (i = 0; i < numSpace-1; i++)
    {
        putsUart0(" ");
    }
}

void ps()
{
    TASK_INFO taskInfo;
    uint8_t taskCount;
    uint8_t i = 0;
    starTitlePad(70, "TASK INFO");
    putsUart0("NAME"); //starts at 0
    format(15);
    putsUart0("PID"); // starts at 19 (15+4)
    format(15);
    putsUart0("STATE"); //starts at 37 (19 + 3 + 15)
    format(15);
    putsUart0("CPU TIME"); //starts at 57(37 + 5 + 15)
    //format(15);
    putsUart0("\n");
    dashLine(70);
    uint16_t strLength;
    uint32_t totalTime;
    float totalTaskPercent = 0;
    float totalTaskTime = 0;
    float kernelTime = 0;
    char *strState;

    getTaskInfo(&taskInfo, i);
    taskCount = taskInfo.taskCount;
    totalTime = taskInfo.totalTime;
    for (i = 0; i < taskCount; i ++)
    {
        if (i != 0)
            getTaskInfo(&taskInfo, i);

        putsUart0(taskInfo.name);
        strLength = stringLength(taskInfo.name);
        format(19-strLength);

        iToA(taskInfo.pid);
        strLength = 5 + 19;
        format(37-strLength);

        switch (taskInfo.state)
        {
        case 0:
        {
            strState = "INVALID";
            break;
        }
        case 1:
        {
            strState = "STOPPED";
            break;
        }
        case 2:
        {
            strState = "UNRUN";
            break;
        }
        case 3:
        {
            strState = "READY";
            break;
        }case 4:
        {
            strState = "DELAYED";
            break;
        }
        case 5:
        {
            strState = "BLOCKED MUTEX";
            break;
        }
        case 6:
        {
            strState = "BLOCKED SEMAPHORE";
            break;
        }
        }
        putsUart0(strState);
        strLength = stringLength(strState) + 37;
        format(57-strLength);
        if (taskInfo.state != 0)
        {

            float cpuPercent = ((float)taskInfo.cpuTime/120e6)*100;
            totalTaskTime += taskInfo.cpuTime;
            totalTaskPercent += cpuPercent;
            fToA(cpuPercent);
            putsUart0("%");
        }
        else
            putsUart0("N/A");
        putsUart0("\n");
    }
    kernelTime = (120e6-totalTaskPercent)/(120e6);
    putsUart0("Kernel");
    format(57-8);
    fToA(100 - totalTaskPercent);
    putsUart0("%");
    putsUart0("\n");




}
void ipcs()
{
    MUTEX_INFO mutexInfo;
    SEM_INFO semaphoreInfo;
    uint8_t i = 0;
    uint8_t j = 0;
    starTitlePad(70, "MUTEX INFO");
    putsUart0("NAME"); //4
    format(15);
    putsUart0("STATUS"); //starts at 19 (15+4)
    format(15);
    putsUart0("Locked By"); //starts at 40 (19 + 6 + 15)
    format(15);
    putsUart0("QUEUE"); //starts at 63 (40 + 9 + 15)
    putsUart0("\n");
    uint16_t strLength;

    for (i = 0; i < MAX_MUTEXES; i++)
    {
        j = 0;
        getMutexInfo(&mutexInfo, i);
        putsUart0(mutexInfo.name);
        strLength = stringLength(mutexInfo.name);
        format(19-strLength);
        if (mutexInfo.lockStatus)
        {
            putsUart0("Locked");
            strLength = stringLength("Locked") + 19;
            format(40-strLength);
            putsUart0(mutexInfo.lockedBy);
            strLength = stringLength(mutexInfo.lockedBy)+40;
            format(64-strLength);
            //            putsUart0("\nProcesses in Queue:\n");
            for(j = 0; j < mutexInfo.queueSize; j++)
            {
                if (j != 0)
                    putsUart0(", ");
                putsUart0(mutexInfo.processInQueue[j]);
            }
        }
        else
        {
            putsUart0("Unlocked");
            strLength = stringLength("Unlocked") + 19;
            format(40-strLength);
            putsUart0("N/A");
            strLength = stringLength("N/A") + 39;
            format(63-strLength);
            putsUart0("N/A");
        }
        putsUart0("\n");

    }

    starTitlePad(70, "SEMAPHORE INFO");
    putsUart0("NAME"); //4
    format(15);
    putsUart0("COUNT"); //6
    format(16);
    putsUart0("QUEUE"); //5
    format(15);
    putsUart0("\n");

    for (i = 0; i < MAX_SEMAPHORES; i++)
    {
        j = 0;
        getSemaphoreInfo(&semaphoreInfo, i);
        //putsUart0("Name: ");
        putsUart0(semaphoreInfo.name);
        strLength = stringLength(semaphoreInfo.name);
        format(19-strLength);
        //putsUart0("\nCount: ");
        iToA(semaphoreInfo.count);
        strLength = stringLength("1") + 19;
        format(40-strLength);

        if (semaphoreInfo.count == 0 && semaphoreInfo.queueSize)
        {
            for(j = 0; j < semaphoreInfo.queueSize; j++)
            {
                if (j != 0)
                    putsUart0(", ");
                putsUart0(semaphoreInfo.processInQueue[j]);
            }
        }
        else
        {
            putsUart0("N/A");

        }
        putsUart0("\n");
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
        restartThread((_fn)pid);
    }
    else
    {
        putsUart0("Invalid Name\n");
    }
}
