// Kernel functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "mm.h"
#include "kernel.h"
#include "uart0.h"
#include "asm.h"


//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// mutex
typedef struct _mutex
{
    char name[64];
    bool lock;
    uint8_t queueSize;      // 0 = nobody in queue
    uint8_t processQueue[MAX_MUTEX_QUEUE_SIZE];
    uint8_t lockedBy;    //can only be unlocked by task that locked it
} mutex;
mutex mutexes[MAX_MUTEXES];

// semaphore
typedef struct _semaphore
{
    char name[64];
    uint8_t count;
    uint8_t queueSize;
    uint8_t processQueue[MAX_SEMAPHORE_QUEUE_SIZE];
} semaphore;
semaphore semaphores[MAX_SEMAPHORES];

// task states
#define STATE_INVALID           0 // no task
#define STATE_STOPPED           1 // stopped, can be resumed
#define STATE_UNRUN             2 // task has never been run
#define STATE_READY             3 // has run, can resume at any time
#define STATE_DELAYED           4 // has run, but now awaiting timer
#define STATE_BLOCKED_MUTEX     5 // has run, but now blocked by mutex
#define STATE_BLOCKED_SEMAPHORE 6 // has run, but now blocked by semaphore


// Kernel Calls
#define YIELD                   1  // When called switches task
#define SLEEP                   2  // When called puts task to sleep
#define MUTEX_LOCK              3  // When called locks resource or adds to resource queue
#define MUTEX_UNLOCK            4  // When called unlocks resource
#define SEMAPHORE_WAIT          5  // When called uses resource if available or adds to resource queue
#define SEMAPHORE_POST          6  // When called returns resource
#define SCHEDULER_TYPE          7  // When called will either switch to Round Robin or Priority scheduler
#define GET_PID                 8  // When called will get of process name
#define GET_MUTEX_INFO          9  // When called will get mutex info name
#define GET_SEMAPHORE_INFO      10 // When called will get semaphore info
#define STOP_THREAD             11 // When called will stop thread and unblock mutexes and remove from wait of semaphores
#define RESTART_THREAD          12 // When called will restart thread
#define PREEMPTION_TOGGLE       13 // When called will toggle Preemption on or off
#define THREAD_PRIORITY         14 // When called will set priority of thread
#define REBOOT                  15 // When called will reboot hardware
// task
uint8_t taskCurrent = 0;          // index of last dispatched task
uint8_t taskCount = 0;            // total number of valid tasks
uint8_t lastTaskRun[8] = {0};


// control
bool priorityScheduler = false;    // priority (true) or round-robin (false)
bool priorityInheritance = false; // priority inheritance for mutexes
bool preemption = false;          // preemption (true) or cooperative (false)

// tcb
#define NUM_PRIORITIES   8
struct _tcb
{
    uint8_t state;                 // see STATE_ values above
    void *pid;                     // used to uniquely identify thread (add of task fn)
    void *spInit;                  // original top of stack
    void *sp;                      // current stack pointer
    uint8_t priority;              // 0=highest
    uint8_t currentPriority;       // 0=highest (needed for pi)
    uint32_t ticks;                // ticks until sleep complete
    uint8_t srd[NUM_SRAM_REGIONS]; // MPU subregion disable bits
    char name[16];                 // name of task used in ps command
    int8_t mutex;                 // index of the mutex in use or blocking the thread
    int8_t semaphore;             // index of the semaphore that is blocking the thread
} tcb[MAX_TASKS];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool initMutex(uint8_t mutex, const char name[])
{
    bool ok = (mutex < MAX_MUTEXES);
    if (ok)
    {
        stringCopy(mutexes[mutex].name, name);
        mutexes[mutex].lock = false;
        mutexes[mutex].lockedBy = 0;
    }
    return ok;
}

bool initSemaphore(uint8_t semaphore, uint8_t count, const char name[])
{
    bool ok = (semaphore < MAX_SEMAPHORES);
    {
        stringCopy(semaphores[semaphore].name, name);
        semaphores[semaphore].count = count;
    }
    return ok;
}

// REQUIRED: initialize systick for 1ms system timer
void initRtos(void)
{
    uint8_t i;
    // no tasks running
    taskCount = 0;
    // clear out tcb records
    for (i = 0; i < MAX_TASKS; i++)
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
        tcb[i].mutex = -1;
        tcb[i].semaphore = -1;
    }
}

// REQUIRED: Implement prioritization to NUM_PRIORITIES
uint8_t rtosScheduler(void)
{
    bool ok;
    static uint8_t task = 0xFF;
    ok = false;
    uint8_t i = 0;
    if (priorityScheduler)
    {
        uint8_t highestPriorityLevel = 0xFF;
        uint8_t checkTask;
        //find highest priority level (lowest number)
        for (i = 0; i < MAX_TASKS; i++)
        {
            if (tcb[i].priority < highestPriorityLevel && (tcb[i].state == STATE_READY || tcb[i].state == STATE_UNRUN))
            {
                highestPriorityLevel = tcb[i].priority;
            }
            if (highestPriorityLevel == 0)
                break;
        }

        checkTask = lastTaskRun[highestPriorityLevel];
        while(!ok)
        {
            checkTask++;
            if (checkTask >= MAX_TASKS)
                checkTask = 0;

            if (tcb[checkTask].priority == highestPriorityLevel && (tcb[checkTask].state == STATE_READY || tcb[checkTask].state == STATE_UNRUN))
            {
                lastTaskRun[highestPriorityLevel] = checkTask;
                ok = true;
            }
        }
        return checkTask;


    }
    else //ROUND ROBIN
    {
        while (!ok)
        {
            task++;
            if (task >= MAX_TASKS)
                task = 0;
            ok = (tcb[task].state == STATE_READY || tcb[task].state == STATE_UNRUN);
        }
        return task;
    }

}

// REQUIRED: modify this function to start the operating system
// by calling scheduler, set srd bits, setting PSP, ASP bit, TMPL bit, and PC
void startRtos(void)
{
    taskCurrent = rtosScheduler();
    tcb[taskCurrent].state = STATE_READY;


    setSramAccessWindow(tcb[taskCurrent].srd);
    setPSP((uint32_t)tcb[taskCurrent].spInit);
    setControlRegister(2);

    _fn fn = (_fn)tcb[taskCurrent].pid;

    setControlRegister(3);

    fn();
}

// REQUIRED:
// add task if room in task list
// store the thread name
// allocate stack space and store top of stack in sp and spInit
// set the srd bits based on the memory allocation
bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes)
{
    bool ok = false;
    uint8_t i = 0;
    uint8_t j;
    bool found = false;
    if (taskCount < MAX_TASKS)
    {
        // make sure fn not already in list (prevent re-entrancy)
        while (!found && (i < MAX_TASKS))
        {
            found = (tcb[i++].pid ==  fn);
        }
        if (!found)
        {
            // find first available tcb record
            i = 0;
            while (tcb[i].state != STATE_INVALID) {i++;}
            tcb[i].state = STATE_UNRUN;
            tcb[i].pid = fn;           //set pid to function address

            // allocate stack space and store top of stack in sp and spInit
            // set the srd bits based on the memory allocation
            for (j = 0; j < NUM_SRAM_REGIONS; j++)
                tcb[i].srd[j] = 0;

            uint32_t *ptr1 = mallocFromHeap(stackBytes, tcb[i].srd);
            tcb[i].sp = (uint32_t*)((uint32_t)ptr1 + stackBytes-1);
            tcb[i].spInit = (uint32_t*)((uint32_t)ptr1 + stackBytes-1);

            j = 0;
            stringCopy(tcb[i].name, name);

            tcb[i].priority = priority;
            // increment task count
            taskCount++;
            ok = true;
        }
    }
    return ok;
}


// REQUIRED: modify this function to yield execution back to scheduler using pendsv
void yield(void)
{
    __asm("   SVC #1");
}

// REQUIRED: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses using pendsv
void sleep(uint32_t tick)
{
    __asm("   SVC #2");
}

// REQUIRED: modify this function to lock a mutex using pendsv
void lock(int8_t mutex)
{
    __asm("   SVC #3");
}

// REQUIRED: modify this function to unlock a mutex using pendsv
void unlock(int8_t mutex)
{
    __asm("   SVC #4");
}

// REQUIRED: modify this function to wait a semaphore using pendsv
void wait(int8_t semaphore)
{
    __asm("   SVC #5");
}

// REQUIRED: modify this function to signal a semaphore is available using pendsv
void post(int8_t semaphore)
{
    __asm("   SVC #6");
}

void schedSelect(bool sched)
{
    __asm("   SVC #7");
}
int getPid(const char processName[])
{
    __asm("   SVC #8");
}

void getMutexInfo(MUTEX_INFO *mutexInfo, uint8_t mutexNumber)
{
    __asm("   SVC #9");
}
void getSemaphoreInfo(SEM_INFO *semaphoreInfo, uint8_t semNumber)
{
    __asm("   SVC #10");
}
// REQUIRED: modify this function to stop a thread
// REQUIRED: remove any pending semaphore waiting, unlock any mutexes
void stopThread(_fn fn)
{
    __asm("   SVC #11");
}
// REQUIRED: modify this function to restart a thread
void restartThread(_fn fn)
{
    __asm("   SVC #12");
}
void togglePreemption(bool preempt)
{
    __asm("   SVC #13");
}
// REQUIRED: modify this function to set a thread priority
void setThreadPriority(_fn fn, uint8_t priority)
{
    __asm("   SVC #14");
}

void rebootSystem()
{
    __asm("   SVC #15");
}

void initSysTickIsr()
{
    //FIXME: Need to check if load value is correct
    NVIC_ST_RELOAD_R = 40000-1;     //40 clocks per microsecond == 40000 clocks per millisecond
    NVIC_ST_CURRENT_R = 0;
    NVIC_ST_CTRL_R = 0x7;  // Use system clock, Enable interrupt, enable SysTick
}

// REQUIRED: modify this function to add support for the system timer
// REQUIRED: in preemptive code, add code to request task switch
void systickIsr(void)
{
    uint8_t i = 0;
    for (i = 0; i < taskCount; i++)
    {
        if (tcb[i].state == STATE_DELAYED)
        {
            tcb[i].ticks -=1;
            if (tcb[i].ticks == 0)
            {
                tcb[i].state = STATE_READY;
            }
        }
    }
    if (preemption)
        NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;

}

// REQUIRED: in coop and preemptive, modify this function to add support for task switching
// REQUIRED: process UNRUN and READY tasks differently
__attribute__ ((naked))void pendSvIsr(void)
{


//    else
//    {
        __asm("   MRS R0, PSP");
        __asm("   STMDB R0!, {R4-R11, LR}");   //store register to psp
        __asm("   MSR PSP, R0");

        //putsUart0("\n***PendSV Called from OS!***\n");

        tcb[taskCurrent].sp = getPSP();
  //  }
        taskCurrent = rtosScheduler();
        setPSP((uint32_t)tcb[taskCurrent].sp);
        setSramAccessWindow(tcb[taskCurrent].srd);

        if (tcb[taskCurrent].state == STATE_READY)
        {
            popFromPsp();
        }
        else
        {
            tcb[taskCurrent].state = STATE_READY;
            unreadyPush(0x81000000, tcb[taskCurrent].pid, 0xFFFFFFFD);
            __asm("   BX R10");
        }
   // }
}
void stopThreadHelper(uint32_t pid)
{
    uint8_t i = 0;
    bool found = false;
    for (i = 0; i < taskCount; i++)
    {
        if ((uint32_t)tcb[i].pid == pid)
        {
            found = true;
            break;
        }
    }
    if (found)
    {
        uint8_t taskTokill = i;
        putsUart0("Killed ");
        putsUart0(tcb[taskTokill].name);
        putsUart0(" (");
        iToA(pid);
        putsUart0(")\n");
        tcb[taskTokill].sp = tcb[taskTokill].spInit;
        int8_t mutexIndex = tcb[taskTokill].mutex;
        int8_t semIndex = tcb[taskTokill].semaphore;
        //unlock mutex and remove from queue
        if (mutexIndex != -1)
        {
            if (tcb[taskTokill].state == STATE_BLOCKED_MUTEX)
            {
                for (i = 0; i <= mutexes[mutexIndex].queueSize; i++)
                {
                    if (mutexes[mutexIndex].processQueue[i] == taskTokill)
                    {
                        if (i == MAX_MUTEX_QUEUE_SIZE)
                        {
                            mutexes[mutexIndex].processQueue[i] = 0;
                        }
                        else
                        {
                            for (;i < mutexes[mutexIndex].queueSize; i++)
                            {
                                mutexes[mutexIndex].processQueue[i] = mutexes[mutexIndex].processQueue[i+1];
                            }

                        }
                        mutexes[mutexIndex].queueSize--;
                        break;
                    }
                }
            }
            else
            {
                mutexes[mutexIndex].lock = false;
                //if there is a queue then ready next task, and increment queue
                if (mutexes[mutexIndex].queueSize > 0)
                {
                    mutexes[mutexIndex].lock = true;
                    mutexes[mutexIndex].lockedBy = mutexes[mutexIndex].processQueue[0];
                    tcb[mutexes[mutexIndex].processQueue[0]].state = STATE_READY;
                    for (i = 0; i < mutexes[mutexIndex].queueSize; i++)
                    {
                        mutexes[mutexIndex].processQueue[i] = mutexes[mutexIndex].processQueue[i+1];
                    }
                    mutexes[mutexIndex].queueSize--;
                }
            }
            tcb[taskTokill].mutex = -1;
        }

        if (semIndex != -1)
        {
            if (tcb[taskTokill].state == STATE_BLOCKED_SEMAPHORE)
            {
                for (i = 0; i <= MAX_SEMAPHORE_QUEUE_SIZE; i++)
                {
                    if (semaphores[semIndex].processQueue[i] == taskTokill)
                    {
                        if (i == semaphores[semIndex].queueSize)
                        {
                            semaphores[semIndex].processQueue[i] = 0;
                        }
                        else
                        {
                            for (;i < semaphores[semIndex].queueSize; i++)
                            {
                                semaphores[semIndex].processQueue[i] = semaphores[semIndex].processQueue[i+1];
                            }

                        }
                        semaphores[semIndex].queueSize--;
                        break;
                    }
                }
            }
            else
            {
                semaphores[semIndex].count++;
                if (semaphores[semIndex].queueSize > 0)
                {
                    semaphores[semIndex].count--;
                    tcb[semaphores[semIndex].processQueue[0]].state = STATE_READY;
                    tcb[semaphores[semIndex].processQueue[0]].semaphore = semIndex;

                    for (i = 0; i < semaphores[semIndex].queueSize; i++)
                    {
                        semaphores[semIndex].processQueue[i] = semaphores[semIndex].processQueue[i+1];
                    }
                    semaphores[semIndex].queueSize--;
                }
            }
            tcb[taskTokill].semaphore = -1;
        }
        tcb[taskTokill].state = STATE_STOPPED;
        NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
    }
    else
    {
        putsUart0("PID not found\n");
    }
}
// REQUIRED: modify this function to add support for the service call
// REQUIRED: in preemptive code, add code to handle synchronization primitives
void svCallIsr(void)
{
    uint32_t *psp = (uint32_t*)getPSP();
    uint32_t r0 = (uint32_t)*getPSP();
    uint32_t *pc = (uint32_t*)*(getPSP() + 6);

    uint32_t svcValueNPtr = (uint32_t)pc-2; //& 0xFFFF;
    uint32_t *svcValuePtr = (uint32_t*)svcValueNPtr;
    uint32_t svcValue = *svcValuePtr & 0xFF;
    uint8_t i = 0;

    switch (svcValue){
    case YIELD:
    {
        NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
        break;
    }
    case SLEEP:
    {
        uint32_t ticks = r0;
        tcb[taskCurrent].state = STATE_DELAYED;
        tcb[taskCurrent].ticks = ticks;
        NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
        break;
    }
    case MUTEX_LOCK:
    {
        uint8_t mutexIndex = (uint8_t)r0;
        if (mutexes[mutexIndex].lock == false)
        {
            mutexes[mutexIndex].lock = true;
            mutexes[mutexIndex].lockedBy = taskCurrent;
            tcb[taskCurrent].mutex = mutexIndex;
        }
        else
        {
            if (mutexes[mutexIndex].queueSize <= MAX_MUTEX_QUEUE_SIZE)
            {
                mutexes[mutexIndex].processQueue[mutexes[mutexIndex].queueSize] = taskCurrent;
                mutexes[mutexIndex].queueSize++;
                tcb[taskCurrent].state = STATE_BLOCKED_MUTEX;
                tcb[taskCurrent].mutex = mutexIndex;
            }

            NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;

            //taskSwitch = true;
        }
        break;
    }
    case MUTEX_UNLOCK:
    {
        uint8_t mutexIndex = (uint8_t)r0;

        if (mutexes[mutexIndex].lock == true && mutexes[mutexIndex].lockedBy == taskCurrent)
        {
            // unlock task
            mutexes[mutexIndex].lock = false;
            tcb[taskCurrent].mutex = -1;
            //if there is a queue then ready next task, and increment queue
            if (mutexes[mutexIndex].queueSize > 0)
            {
                mutexes[mutexIndex].lock = true;
                mutexes[mutexIndex].lockedBy = mutexes[mutexIndex].processQueue[0];
                tcb[mutexes[mutexIndex].processQueue[0]].state = STATE_READY;
                for (i = 0; i < mutexes[mutexIndex].queueSize; i++)
                {
                    mutexes[mutexIndex].processQueue[i] = mutexes[mutexIndex].processQueue[i+1];
                }
                mutexes[mutexIndex].queueSize--;
            }
        }
        else
        {
            putsUart0("\n***MUTEX WAS NOT LOCKED BY YOU!***\n");
            putsUart0("\n\t***SHUTTING DOWN TASK and SWITCHING TO NEXT TASK!***\n");
            NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;

            // taskSwitch = true;
        }
        break;
    }
    case SEMAPHORE_WAIT:
    {
        uint8_t semaphoreIndex = (uint8_t)r0;
        if (semaphores[semaphoreIndex].count > 0)
        {
            semaphores[semaphoreIndex].count--;
            tcb[taskCurrent].semaphore = semaphoreIndex;
        }
        else
        {
            if (semaphores[semaphoreIndex].queueSize <= MAX_SEMAPHORE_QUEUE_SIZE)
            {
                semaphores[semaphoreIndex].processQueue[semaphores[semaphoreIndex].queueSize] = taskCurrent;
                semaphores[semaphoreIndex].queueSize++;
                tcb[taskCurrent].semaphore = semaphoreIndex;
                tcb[taskCurrent].state = STATE_BLOCKED_SEMAPHORE;
            }
            NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
        }
        break;
    }
    case SEMAPHORE_POST:
    {
        uint8_t semaphoreIndex = (uint8_t)r0;
        semaphores[semaphoreIndex].count++;
        tcb[taskCurrent].semaphore = -1;
        if (semaphores[semaphoreIndex].queueSize > 0)
        {
            semaphores[semaphoreIndex].count--;
            tcb[semaphores[semaphoreIndex].processQueue[0]].state = STATE_READY;
            tcb[semaphores[semaphoreIndex].processQueue[0]].semaphore = semaphoreIndex;

            for (i = 0; i < semaphores[semaphoreIndex].queueSize; i++)
            {
                semaphores[semaphoreIndex].processQueue[i] = semaphores[semaphoreIndex].processQueue[i+1];
            }
            semaphores[semaphoreIndex].queueSize--;
        }
        break;
    }
    case SCHEDULER_TYPE:
    {
        priorityScheduler = (bool)r0;
        break;
    }
    case GET_PID:
    {
        char *processName = (char*)r0;
        uint8_t i = 0;
        *psp = -1;
        for (i = 0; i < taskCount; i++)
        {
            if (strCmp(processName, tcb[i].name))
            {
                *psp = tcb[i].pid;   //store pid at R0
                break;
            }
        }
        break;
    }
    case GET_MUTEX_INFO:
    {
        //TODO: Check if pointer is valid

        MUTEX_INFO *mutexInfo = (MUTEX_INFO*)r0;
        uint8_t mutexNum = (uint8_t)*(psp+1);
        stringCopy(mutexInfo->name,mutexes[mutexNum].name);
        mutexInfo->lockStatus = mutexes[mutexNum].lock;
        uint8_t i = 0;
        if (mutexInfo->lockStatus)
        {
            stringCopy(mutexInfo->lockedBy,tcb[mutexes[mutexNum].lockedBy].name);
            mutexInfo->queueSize = mutexes[mutexNum].queueSize;
            for (i = 0; i < mutexes[mutexNum].queueSize; i++)
            {
                stringCopy(mutexInfo->processInQueue[i], tcb[mutexes[mutexNum].processQueue[i]].name);
            }
        }
        break;
    }
    case GET_SEMAPHORE_INFO:
    {
        //TODO: Check if pointer is valid
        SEM_INFO *semInfo = (SEM_INFO*)r0;
        uint8_t semNum = (uint8_t)*(psp+1);

        stringCopy(semInfo->name,semaphores[semNum].name);
        semInfo->count = semaphores[semNum].count;
        uint8_t i = 0;
        if (semInfo->count == 0)
        {
            semInfo->queueSize = semaphores[semNum].queueSize;
            for (i = 0; i < semaphores[semNum].queueSize; i++)
            {
                stringCopy(semInfo->processInQueue[i], tcb[semaphores[semNum].processQueue[i]].name);
            }
        }
        break;
    }
    case STOP_THREAD:
    {
        uint32_t pid = (uint32_t)r0;
        stopThreadHelper(pid);
        break;
    }
    case RESTART_THREAD:
    {
        uint32_t pid = (uint32_t)r0;
        uint8_t i = 0;
        bool found = false;
        for (i = 0; i < taskCount; i++)
        {
            if ((uint32_t)tcb[i].pid == pid)
            {
                found = true;
                break;
            }
        }
        if (found)
        {
            uint8_t taskToRestart = i;
            tcb[taskToRestart].state = STATE_UNRUN;
        }
        else
        {
            putsUart0("PID not found\n");
        }
        break;
    }
    case PREEMPTION_TOGGLE:
    {
        preemption = (bool)r0;
        break;
    }
    case THREAD_PRIORITY:
    {
        uint32_t pid = (uint32_t)r0;
        uint8_t prio = (uint8_t)*(psp+1);
        uint8_t i = 0;
        for (i = 0; i < taskCount; i++)
        {
            if ((uint32_t)tcb[i].pid == pid)
            {
                tcb[i].priority = prio;
                break;
            }
        }
        break;
    }
    case REBOOT:
    {
        NVIC_APINT_R = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
        break;
    }
    }
}

uint32_t getPID()
{
    return (uint32_t)tcb[taskCurrent].pid;
}
