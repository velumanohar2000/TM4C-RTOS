// Kernel functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef KERNEL_H_
#define KERNEL_H_

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <userCommands.h>

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// function pointer
typedef void (*_fn)();

// mutex
#define MAX_MUTEXES 1
#define MAX_MUTEX_QUEUE_SIZE 2  //change in userCammands.h as well
#define resource 0

// semaphore
#define MAX_SEMAPHORES 3
#define MAX_SEMAPHORE_QUEUE_SIZE 2 //change in userCammands.h as well
#define keyPressed 0
#define keyReleased 1
#define flashReq 2

// tasks
#define MAX_TASKS 12

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

//called from Main
bool initMutex(uint8_t mutex, const char name[]);
bool initSemaphore(uint8_t semaphore, uint8_t count, const char name[]);

void initRtos(void);
void startRtos(void);
void initSysTickIsr();


//Called from tasks
bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes);
void restartThread(_fn fn);
void stopThread(_fn fn);
void setThreadPriority(_fn fn, uint8_t priority);

void yield(void);
void sleep(uint32_t tick);
void lock(int8_t mutex);
void unlock(int8_t mutex);
void wait(int8_t semaphore);
void post(int8_t semaphore);
void schedSelect(bool sched);
int getPid(const char processName[]);
void getMutexInfo(MUTEX_INFO *mutexInfo, uint8_t mutexNumber);
void getSemaphoreInfo(SEM_INFO *semaphoreInfo, uint8_t semNumber);
void togglePreemption(bool preempt);
void rebootSystem();
void getTaskInfo(TASK_INFO *taskInfo, uint8_t taskNum);
void initWideTimer();

//Called from ccs
void systickIsr(void);
void pendSvIsr(void);
void svCallIsr(void);

#endif
