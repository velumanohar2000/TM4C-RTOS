/*
 * suerCommands.h
 *
 *  Created on: Nov 11, 2023
 *      Author: velum
 */

#ifndef USERCOMMANDS_H_
#define USERCOMMANDS_H_

#include <inttypes.h>

#define MAX_MUTEX_QUEUE_SIZE 2
#define MAX_SEMAPHORE_QUEUE_SIZE 2

typedef struct _MUTEX_INFO
{
    char name[64];
    bool lockStatus;
    char lockedBy[64];
    uint8_t queueSize;
    char processInQueue [MAX_MUTEX_QUEUE_SIZE][64];
}MUTEX_INFO;
extern struct MUTEX_INFO mutexInfo;

typedef struct _SEM_INFO
{
    char name[64];
    uint8_t count;
    uint8_t queueSize;
    char processInQueue [MAX_SEMAPHORE_QUEUE_SIZE][64];
}SEM_INFO;
extern struct SEM_INFO semaphoreInfo;

typedef struct _TASK_INFO
{
    char name[64];
    uint32_t pid;
    uint8_t state;
    uint32_t cpuTime;
    uint8_t taskCount;
    uint32_t totalTime;
}TASK_INFO;
extern struct TASK_INFO taskInfo;



void ps();
void ipcs();
void kill(uint32_t pid);
void Pkill(const char processName[]);
void preempt(bool on);
void sched(bool prio_on);
void pidof(const char name[]);
void run(const char processName[]);

#endif /* USERCOMMANDS_H_ */
