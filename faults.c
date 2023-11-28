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
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "gpio.h"
#include "asm.h"
#include "faults.h"
#include "kernel.h"

#define ON_BOARD_RED_LED PORTF, 1

uint32_t *pspValue = 0;
uint32_t *mspValue = 0;
extern uint32_t getPID();
extern void stopThreadHelper();
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initFaultHandlers()
{
    NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_USAGE | NVIC_SYS_HND_CTRL_BUS | NVIC_SYS_HND_CTRL_MEM;
}

// TODO: Delete later
void faultInfiniteLoop()
{
    setPinValue(ON_BOARD_RED_LED, 1);
    while(true);
}

void mpuFaultIsr(void)
{
    pspValue = getPSP();
    mspValue = getMSP();
    putsUart0("\n***Memory Management Fault Occurred!***\n");
    putsUart0("PSP: ");
    intToHex((uint32_t)pspValue);
    putsUart0("\nMSP: ");
    intToHex((uint32_t)mspValue);
    uint8_t i = 0;

    for (i = 0; i<8; i++)
    {
        if (i < 4)
        {
            putsUart0("\nR");
            iToA(i);
            putsUart0(": ");
            intToHex(*(pspValue+i));
        }
        else if (i == 4)
        {
            putsUart0("\nR12: ");
            intToHex(*(pspValue+i));
        }
        else if (i == 5)
        {
            putsUart0("\nLR: ");
            intToHex(*(pspValue+i));
        }
        else if (i == 6)
        {
            putsUart0("\nPC: ");
            intToHex(*(pspValue+i));
        }
        else if (i == 7)
        {
            putsUart0("\nxPSR: ");
            intToHex(*(pspValue+i));
        }
    }
    putsUart0("\nMemory Management Fault Flags: ");
    intToHex((uint32_t)(NVIC_FAULT_STAT_R & 0xFF));
    putsUart0("\nMemory Management Fault Address: ");
    intToHex((uint32_t)NVIC_MM_ADDR_R);
    putsUart0("\nOffending Instruction: ");
    intToHex(*(pspValue+6));
    putsUart0("\n");

    // Clear the MPU fault pending bit and trigger a pendsv ISR call.
    NVIC_SYS_HND_CTRL_R &= ~(NVIC_SYS_HND_CTRL_MEMA);
    if (NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_IERR || NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_DERR)
    {
        NVIC_FAULT_STAT_R &= ~(NVIC_FAULT_STAT_IERR | NVIC_FAULT_STAT_DERR);
        putsUart0("\n***PendSV Fault Occurred from MPU!***\n");
    }
    stopThreadHelper(getPID());

}

void hardFaultIsr(void)
{
    putsUart0("\n***Hard Fault Occurred!***\n");
    uint32_t *pspValue = getPSP();
    uint32_t *mspValue = getMSP();
    putsUart0("PSP: ");
    intToHex((uint32_t)pspValue);
    putsUart0("\n");
    putsUart0("MSP: ");
    intToHex((uint32_t)mspValue);
    putsUart0("\n");
    putsUart0("Hard Fault Flags: ");
    intToHex((uint32_t)NVIC_HFAULT_STAT_R);
    putsUart0("\n");
    putsUart0("All Status Flags: ");
    intToHex((uint32_t)NVIC_FAULT_STAT_R);
    faultInfiniteLoop();
}

void busFaultIsr(void)
{
    putsUart0("\n***Bus Fault Occurred!***\n");
    faultInfiniteLoop();
}

void usageFaultIsr(void)
{
    putsUart0("\n***Usage Fault Occurred!***\n");
    faultInfiniteLoop();
}

