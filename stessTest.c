/*
 * stessTest.c
 *
 *  Created on: Oct 8, 2023
 *      Author: velum
 */
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

#include "clock.h"
#include "wait.h"
#include "uart0.h"
#include "gpio.h"
#include "nvic.h"

#include "faults.h"
#include "shell.h"
#include "asm.h"
#include "mpu.h"
#include "mm.h"

#define GREEN_LED PORTE, 1
#define YELLOW_LED PORTE, 2
#define RED_LED PORTE, 3
#define ORANGE_LED PORTA, 7
#define BLUE_LED PORTF, 2
#define ON_BOARD_GREEN_LED PORTF, 3
#define ON_BOARD_RED_LED PORTF, 1
#define SWITCH_TO_PRIVILEGED PORTF, 4

bool inPrivileged;
void checkEnter()
{
    static USER_DATA data;
    //    bool valid = true;
    //    char *tempString;
    putsUart0("\nTesting peripherals by turning on LEDs\n");
    setPinValue(GREEN_LED, 1);
    setPinValue(YELLOW_LED, 1);
    setPinValue(RED_LED, 1);
    setPinValue(ORANGE_LED, 1);
    setPinValue(BLUE_LED, 1);
    getsUart0(&data);
    parseFields(&data);
    while(data.buffer[0] != '\0');
    setPinValue(GREEN_LED, 0);
    setPinValue(YELLOW_LED, 0);
    setPinValue(RED_LED, 0);
    setPinValue(ORANGE_LED, 0);
    setPinValue(BLUE_LED, 0);

}


void switchToPrivilegedISR()
{
    putsUart0("\nSwitching Back to Privileged Mode\n");
    setPinValue(GREEN_LED, 0);
    setPinValue(YELLOW_LED, 0);
    setPinValue(RED_LED, 0);
    setPinValue(ORANGE_LED, 0);
    setPinValue(BLUE_LED, 0);
    uint8_t i = 0;
    for (i = 0; i < 2; i++)
    {
        togglePinValue(GREEN_LED);
        togglePinValue(YELLOW_LED);
        togglePinValue(RED_LED);
        togglePinValue(ORANGE_LED);
        togglePinValue(BLUE_LED);
        waitMicrosecond(100000);
    }
    setControlRegister(2);
    inPrivileged = true;
    clearPinInterrupt(SWITCH_TO_PRIVILEGED);
}
void testRegion(uint32_t *ptr, uint32_t initialValue)
{
    putsUart0("RMW variables at address: ");
    intToHex((uint32_t)ptr);
    *ptr = initialValue;
    putsUart0("\nCurrent value at address: ");
    iToA(*ptr);
    *ptr = *ptr+10;
    putsUart0("\nValue at address after adding 10: ");
    iToA(*ptr);
    putsUart0("\n");
}


void flashLeds()
{
    putsUart0("\nTesting peripherals by turning on and off LEDs\n");
    uint8_t i = 0;
    for (i = 0; i < 4; i++)
    {
        togglePinValue(GREEN_LED);
        togglePinValue(YELLOW_LED);
        togglePinValue(RED_LED);
        togglePinValue(ORANGE_LED);
        togglePinValue(BLUE_LED);
        waitMicrosecond(1000000);
    }


}

void testAllAccessRegion()
{
    setPinValue(GREEN_LED, 1);
    setPinValue(YELLOW_LED, 1);
    setPinValue(RED_LED, 1);
    setPinValue(ORANGE_LED, 1);
    setPinValue(BLUE_LED, 1);
    putsUart0("\nR/W has been given to privileged and un-prilvileged mode\nStart Address: 0x0000 0000\nSize: 4GB\n");
    putsUart0("R/W/X has been given to privileged and un-prilvileged in the Flash region\nStart Address: 0x0000 0000\nEnd Address: 0x0003 FFFF\n");
    putsUart0("Testingall access region in privileged mode\n");
    putsUart0("--------------------------------------------\n");
    putsUart0("Creating variables in all SRAM regions and executing code in flash\n");

    testRegion((uint32_t*)0x20001000, 59);
    testRegion((uint32_t*)0x20002000, 59);
    testRegion((uint32_t*)0x20004000, 59);
    testRegion((uint32_t*)0x20005000, 59);
    testRegion((uint32_t*)0x20006000, 59);
    testRegion((uint32_t*)0x20007000, 59);
    //Testing peripherals
    putsUart0("Press Enter to test All access region in un-privileged mode\n");
    checkEnter();

    putsUart0("Testing all access region in un-privileged mode\n");
    putsUart0("-----------------------------------------------\n");
    putsUart0("Creating variables in all 7 regions and executing code in flash\n");
    inPrivileged = false;
    setControlRegister(3);
    testRegion((uint32_t*)0x20001000, 59);
    testRegion((uint32_t*)0x20002000, 59);
    testRegion((uint32_t*)0x20004000, 59);
    testRegion((uint32_t*)0x20005000, 59);
    testRegion((uint32_t*)0x20006000, 59);
    testRegion((uint32_t*)0x20007000, 59);
    //Testing peripherals
    setPinValue(GREEN_LED, 0);
    setPinValue(YELLOW_LED, 0);
    setPinValue(RED_LED, 0);
    setPinValue(ORANGE_LED, 0);
    setPinValue(BLUE_LED, 0);
    while (!inPrivileged);
}

void testSRAMRegion()
{
    putsUart0("R/W has been taken away from un-prilvileged in the SRAM Region\nStart Address: 0x2000 1000\nEnd Address: 0x2000 7FFF\n");
    putsUart0("Testing SRAM region in privileged mode\n");
    putsUart0("--------------------------------------------\n");
    putsUart0("Creating variables in all SRAM regions and executing code in flash\n");

    testRegion((uint32_t*)0x20001000, 59);
    testRegion((uint32_t*)0x20002000, 59);
    testRegion((uint32_t*)0x20004000, 59);
    testRegion((uint32_t*)0x20005000, 59);
    testRegion((uint32_t*)0x20006000, 59);
    testRegion((uint32_t*)0x20007000, 59);
    //Testing peripherals
    putsUart0("\nPress Enter to test SRAM region in un-privileged mode\n");
    putsUart0("***If un-privileged mode has not been given R/W access this will cause a fault***\n");

    checkEnter();

    putsUart0("Testing All access region in un-privileged mode\n");
    putsUart0("-----------------------------------------------\n");
    putsUart0("Creating variables in all SRAM regions and executing code in flash\n");
    inPrivileged = false;
    setControlRegister(3);
    testRegion((uint32_t*)0x20001000, 59);
    testRegion((uint32_t*)0x20002000, 59);
    testRegion((uint32_t*)0x20004000, 59);
    testRegion((uint32_t*)0x20005000, 59);
    testRegion((uint32_t*)0x20006000, 59);
    testRegion((uint32_t*)0x20007000, 59);
    //Testing peripherals

    while (!inPrivileged);
}

//void stressTestMalloc()
//{
//
//    uint32_t *ptr1;
//
//    uint32_t i = 0;
//    ptr1 = mallocFromHeap(512);
//    if (ptr1 == 0)
//    {
//        putsUart0("Failed Memory Access!\n");
//    }
//    else
//    {
//        setSramAccessWindow(ptr1, 512);
//        putsUart0("Memory Allocated at Address: ");
//        intToHex((uint32_t)ptr1);
//        putsUart0("\n");
//    }
//    for (i = 0; i < 2; i++)
//    {
//        ptr1 = mallocFromHeap(1500);
//        if (ptr1 == 0)
//        {
//            putsUart0("Failed Memory Access!\n");
//        }
//        else
//        {
//            setSramAccessWindow(ptr1, 1500);
//            putsUart0("Memory Allocated at Address: ");
//            intToHex((uint32_t)ptr1);
//            putsUart0("\n");
//        }
//    }
//    for (i = 0; i < 20; i++)
//    {
//        ptr1 = mallocFromHeap(1024 * 3);
//        if (ptr1 == 0)
//        {
//            putsUart0("Failed Memory Access!\n");
//        }
//        else
//        {
//            setSramAccessWindow(ptr1, 1024 * 3);
//            putsUart0("Memory Allocated at Address: ");
//            intToHex((uint32_t)ptr1);
//            putsUart0("\n");
//        }
//    }
//    for (i = 0; i < 10; i++)
//    {
//        ptr1 = mallocFromHeap(200);
//        if (ptr1 == 0)
//        {
//            putsUart0("Failed Memory Access!\n");
//        }
//        else
//        {
//            setSramAccessWindow(ptr1, 200);
//            putsUart0("Memory Allocated at Address: ");
//            intToHex((uint32_t)ptr1);
//            putsUart0("\n");
//        }
//    }
//    ptr1 = mallocFromHeap(1500);
//    if (ptr1 == 0)
//    {
//        putsUart0("Failed Memory Access!\n");
//    }
//    else
//    {
//        setSramAccessWindow(ptr1, 1500);
//        putsUart0("Memory Allocated at Address: ");
//        intToHex((uint32_t)ptr1);
//        putsUart0("\n");
//    }
//    for (i = 0; i < 7; i++)
//    {
//        ptr1 = mallocFromHeap(200);
//        if (ptr1 == 0)
//        {
//            putsUart0("Failed Memory Access!\n");
//        }
//        else
//        {
//            setSramAccessWindow(ptr1, 200);
//            putsUart0("Memory Allocated at Address: ");
//            intToHex((uint32_t)ptr1);
//            putsUart0("\n");
//
//        }
//    }
//    for (i = 0; i < 7; i++)
//    {
//        ptr1 = mallocFromHeap(1024);
//        if (ptr1 == 0)
//        {
//            putsUart0("Failed Memory Access!\n");
//        }
//        else
//        {
//            setSramAccessWindow(ptr1, 1024);
//            putsUart0("Memory Allocated at Address: ");
//            intToHex((uint32_t)ptr1);
//            putsUart0("\n");
//        }
//    }
//    for (i = 0; i < 10; i++)
//    {
//        ptr1 = mallocFromHeap(200);
//        if (ptr1 == 0)
//        {
//            putsUart0("Failed Memory Access!\n");
//        }
//        else
//        {
//            setSramAccessWindow(ptr1, 200);
//            putsUart0("Memory Allocated at Address: ");
//            intToHex((uint32_t)ptr1);
//            putsUart0("\n");
//        }
//    }
//    // freeMalloc((uint32_t*)0x20001000);
//    // freeMalloc((uint32_t*)0x20001E00);
//    // freeMalloc((uint32_t*)0x20003C00);
//    // freeMalloc((uint32_t*)0x20001200);
//}
