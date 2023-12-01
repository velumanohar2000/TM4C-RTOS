// Tasks
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
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "wait.h"
#include "kernel.h"
#include "tasks.h"


#define GREEN_LED PORTE, 1          // Off Board GREEN LED
#define YELLOW_LED PORTE, 2         // Off Board YELLOW LED
#define RED_LED PORTE, 3            // Off Board Red LED
#define ORANGE_LED PORTA, 7         // Off Board ORANGE LED

#define ON_BOARD_RED_LED PORTF, 1   // On Board RED LED
#define ON_BOARD_BLUE_LED PORTF, 2  // On Board BLUE LED
#define ON_BOARD_GREEN_LED PORTF, 3 // On Board GREEN LED

//External Push Buttons 0-5 
#define PB0 PORTB, 1                   
#define PB1 PORTE, 4
#define PB2 PORTE, 5
#define PB3 PORTB, 4
#define PB4 PORTA, 5
#define PB5 PORTA, 6

// #define SWITCH_TO_PRIVILEGED PORTF, 4
// #define interrupt PORTB, 5

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
// Initialization for blue, orange, red, green, and yellow LEDs
// Initialization for 6 push buttons
bool idleUnlock = false;


void initHw(void)
{
    // Setup LEDs and push buttons
    enablePort(PORTA);
    enablePort(PORTB);
    enablePort(PORTE);
    enablePort(PORTF);

    selectPinPushPullOutput(GREEN_LED);
    selectPinPushPullOutput(YELLOW_LED);
    selectPinPushPullOutput(RED_LED);
    selectPinPushPullOutput(ORANGE_LED);
    selectPinPushPullOutput(ON_BOARD_BLUE_LED);
    selectPinPushPullOutput(ON_BOARD_GREEN_LED);
    selectPinPushPullOutput(ON_BOARD_RED_LED);

    selectPinDigitalInput(PB0);
    selectPinDigitalInput(PB1);
    selectPinDigitalInput(PB2);
    selectPinDigitalInput(PB3);
    selectPinDigitalInput(PB4);
    selectPinDigitalInput(PB5);

    enablePinPullup(PB0);
    enablePinPullup(PB1);
    enablePinPullup(PB2);
    enablePinPullup(PB3);
    enablePinPullup(PB4);
    enablePinPullup(PB5);

    
    setPinValue(ON_BOARD_BLUE_LED, 0);
    setPinValue(ON_BOARD_RED_LED, 0);
    setPinValue(ON_BOARD_GREEN_LED, 0);
    setPinValue(GREEN_LED, 0);
    setPinValue(YELLOW_LED, 0);
    setPinValue(RED_LED, 0);
    setPinValue(ORANGE_LED, 0);



    // Power-up flash
    setPinValue(ON_BOARD_GREEN_LED, 1);
    waitMicrosecond(250000);
    setPinValue(ON_BOARD_GREEN_LED, 0);
    waitMicrosecond(250000);
}

// Return a value from 0-63 indicating which of 6 PBs are pressed
uint8_t readPbs(void)
{
    uint8_t pbValue = 0;
    if (!getPinValue(PB0)) pbValue |= 1;
    if (!getPinValue(PB1)) pbValue |= 2;
    if (!getPinValue(PB2)) pbValue |= 4;
    if (!getPinValue(PB3)) pbValue |= 8;
    if (!getPinValue(PB4)) pbValue |= 16;
    if (!getPinValue(PB5)) pbValue |= 32;
    return pbValue;
}
// one task must be ready at all times or the scheduler will fail
// the idle task is implemented for this purpose
void idle(void)
{
    while(true)
    {
        setPinValue(ORANGE_LED, 1);
        waitMicrosecond(1000);
        setPinValue(ORANGE_LED, 0);
        yield();
    }
}

//void idle2(void)
//{
//    while(true)
//    {
//        setPinValue(ON_BOARD_GREEN_LED, 1);
//        waitMicrosecond(5e5);
//        setPinValue(ON_BOARD_GREEN_LED, 0);
//        waitMicrosecond(1000);
//        sleep(1000);
//    }
//}


void flash4Hz(void)
{
    while(true)
    {
        setPinValue(GREEN_LED, !getPinValue(GREEN_LED));
        sleep(125);
    }
}

void oneshot(void)
{
    while(true)
    {
        wait(flashReq);
        setPinValue(YELLOW_LED, 1);
        sleep(1000);
        setPinValue(YELLOW_LED, 0);
    }
}

void partOfLengthyFn(void)
{
    // represent some lengthy operation
    waitMicrosecond(990);
    // give another process a chance to run
    yield();
}

void lengthyFn(void)
{
    uint16_t i;
    while(true)
    {
        lock(resource);
        for (i = 0; i < 5000; i++)
        {
            partOfLengthyFn();
        }
        setPinValue(RED_LED, !getPinValue(RED_LED));
        unlock(resource);
    }
}

void readKeys(void)
{
    uint8_t buttons;
    while(true)
    {
        wait(keyReleased);
        buttons = 0;
        while (buttons == 0)
        {
            buttons = readPbs();
            yield();
        }
        post(keyPressed);
        if ((buttons & 1) != 0)
        {
            setPinValue(RED_LED, 1);
            setPinValue(YELLOW_LED, !getPinValue(YELLOW_LED));
        }
        if ((buttons & 2) != 0)
        {
            post(flashReq);
            setPinValue(RED_LED, 0);
        }
        if ((buttons & 4) != 0)
        {
            restartThread(flash4Hz);
        }
        if ((buttons & 8) != 0)
        {
            stopThread(flash4Hz);
        }
        if ((buttons & 16) != 0)
        {
            setThreadPriority(lengthyFn, 4);
        }
        yield();
    }
}

void debounce(void)
{
    uint8_t count;
    while(true)
    {
        wait(keyPressed);
        count = 10;
        while (count != 0)
        {
            sleep(10);
            if (readPbs() == 0)
                count--;
            else
                count = 10;
        }
        post(keyReleased);
    }
}

void uncooperative(void)
{
    while(true)
    {
        while (readPbs() == 8)
        {
        }
        yield();
    }
}

void errant(void)
{
    uint32_t* p = (uint32_t*)0x20000000;
    while(true)
    {
        while (readPbs() == 32)
        {
            *p = 0;
        }
        yield();
    }
}

void important(void)
{
    while(true)
    {
        lock(resource);
        setPinValue(ON_BOARD_BLUE_LED, 1);
        sleep(1000);
        setPinValue(ON_BOARD_BLUE_LED, 0);
        unlock(resource);
    }
}
