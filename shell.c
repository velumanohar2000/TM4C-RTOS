// Shell functions
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
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "shell.h"
#include "gpio.h"

// REQUIRED: Add header files here for your strings functions, ...
#include "uart0.h"
#include "userCommands.h"
#include "kernel.h"


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

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

// for testing purposes
#define SWITCH_TO_PRIVILEGED PORTF, 4
#define interrupt PORTB, 5
#define interruptMask 32



// REQUIRED: add processing for the shell commands through the UART here
void shell(void)
{
    putsUart0("Enter Command:\n");
    while(true)
    {
        if (kbhitUart0())
        {
            USER_DATA data;
            bool valid = true;
            char *tempString;

            getsUart0(&data);
            parseFields(&data);

            // Command evaluation
            if (isCommand(&data, "reboot", 0))
            {
                rebootSystem();
            }
            else if (isCommand(&data, "ps", 0))
            {
                ps();
            }
            else if (isCommand(&data, "ipcs", 0))
            {
                ipcs();
            }
            else if (isCommand(&data, "kill", 1))
            {
                tempString = getFieldString(&data, 1);
                int32_t pidToKill = getFieldInteger(&data, 1);
                if (pidToKill < 0)
                {
                    valid =  false;
                }
                else
                {
                    kill(pidToKill);
                }
            }
            else if (isCommand(&data, "Pkill", 1) || isCommand(&data, "pkill", 1))
            {

                tempString = getFieldString(&data, 1);
                Pkill(tempString);
            }
            else if (isCommand(&data, "preempt", 1))
            {

                tempString = getFieldString(&data, 1);
                if (strCmp(tempString, "ON") || strCmp(tempString, "on"))
                {
                    preempt(true);
                }
                else if (strCmp(tempString, "OFF") || strCmp(tempString, "off"))
                {
                    preempt(false);
                }
                else
                {
                    valid = false;
                }
            }
            else if (isCommand(&data, "sched", 1))
            {

                tempString = getFieldString(&data, 1);
                if (strCmp(tempString, "PRIO") || strCmp(tempString, "prio"))
                {
                    sched(true);
                }
                else if (strCmp(tempString, "RR") || strCmp(tempString, "rr") || strCmp(tempString, "Round Robin") || strCmp(tempString, "round robin"))
                {
                    sched(false);
                }
                else
                {
                    valid = false;
                }
            }
            else if (isCommand(&data, "pidof", 1))
            {
                tempString = getFieldString(&data, 1);
                pidof(tempString);
            }
            else if (isCommand(&data, "run", 1))
            {
                tempString = getFieldString(&data, 1);
                run(tempString);
            }

            else if (data.buffer[0] == '\0')
            {
                valid = false;
            }
            else
            {
                valid = false;
            }
            if (!valid)
            {
                putsUart0("Invalid Command");
                putsUart0("\n\nEnter Command:\n");
            }
            else
            {
                putsUart0("\n\nEnter Command:\n");
            }
            yield();
        }
        yield();
    }
}
