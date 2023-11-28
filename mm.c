// Memory manager functions
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
#include "stdbool.h"
#include "gpio.h"
#include "mpu.h"
#include "mm.h"
#include "uart0.h"

#define NVIC_MPU_ATTR_RW_ALL (0x3 << 24)
#define NVIC_MPU_ATTR_RW_KERNEL (0x1 << 24)

int16_t memoryMap [40];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// REQUIRED: add your malloc code here and update the SRD bits for the current thread
void * mallocFromHeap(uint32_t sizeInBytes, uint8_t srdMask[NUM_SRAM_REGIONS])
{
    //putsUart0("\n\nTrying to Allocate: ");
    //iToA(sizeInBytes);
    //putsUart0("\n----------------------\n");
    bool failedMemoryAccess = false;
    uint32_t index = 0;
    uint32_t startingPosition;
    bool check1024Blocks = false;
    if (sizeInBytes <= 512)
    {
        index = 0;
        while(memoryMap[index] != 0)
        {
            if (index == 6)
            {
                index = 16;
            }
            if (index == 30)
            {
                check1024Blocks = true;
                break;
            }
            index++;
        }
        if (!check1024Blocks)
        {
            startingPosition = index;
            memoryMap[index] = 1;
            setSrdBits(index, srdMask, true);

        }
    }
    if (sizeInBytes > 1024 && sizeInBytes <= 1536)
    {
        uint8_t inc = 0;
        check1024Blocks = true;
        for (inc = 0; inc < 3; inc ++)
        {
            if (inc == 0)
            {
                index = 7;
            }
            else if (inc == 1)
            {
                index = 15;
            }
            else
            {
                index = 31;
            }
            if(memoryMap[index] == 0)
            {
                memoryMap[index] = 2;
                setSrdBits(index, srdMask, true);

                memoryMap[index+1] = -1;
                setSrdBits(index+1, srdMask, true);

                check1024Blocks = false;
                break;
            }
        }
        if (!check1024Blocks)
        {
            startingPosition = index;
        }
    }
    if ((sizeInBytes > 512 && sizeInBytes <= 1024) || (sizeInBytes > 1536) || check1024Blocks)
    {
        bool foundValid = false;
        uint16_t consecutiveBytesNeeded = sizeInBytes/1024;
        if (sizeInBytes %  1024 != 0)
        {
            consecutiveBytesNeeded+=1;
        }
        uint16_t checkSP;
        bool no1024Blocks = false;
        uint16_t x = 0;
        index  = 9;
        while (!foundValid)
        {
            do
            {
                if (index > 14 && index <33)
                {
                    index = 33;
                }
                if (index > 39)
                {
                    no1024Blocks = true;
                    failedMemoryAccess = true;
                    break;
                }

            }while(memoryMap[index++] != 0);
            if (no1024Blocks)
            {
                break;
            }
            else
            {
                checkSP = index-1;
                uint16_t newPosition;
                for (x = 0; x < consecutiveBytesNeeded; x++)
                {
                    newPosition = checkSP + x;
                    if ((newPosition >= 9 && newPosition < 15) || (newPosition >=33 && newPosition < 40))
                    {
                        if (memoryMap[newPosition] == 0)
                        {
                            foundValid  = true;
                        }
                        else
                        {
                            foundValid = false;
                            break;
                        }
                    }
                    else
                    {
                        foundValid = false;
                        break;
                    }
                }
                index = newPosition;
            }
        }
        if (foundValid)
        {
            startingPosition = checkSP;
            uint8_t temp;
            for (x = 0; x < consecutiveBytesNeeded; x++)
            {
                temp = checkSP+x;
                if (x == 0)
                {
                    memoryMap[temp] = consecutiveBytesNeeded;
                }
                else
                {
                    memoryMap[temp] = -1;
                }
                setSrdBits(temp, srdMask, true);
            }
        }
    }
    if (failedMemoryAccess)
    {
        return 0;
    }
    else
    {
        if (startingPosition < 8)
        {
            return (uint32_t*)(0x20001000 + startingPosition*512);
        }
        else if (startingPosition >= 8 && startingPosition < 16)
        {
            return(uint32_t*)( 0x20002000 + (startingPosition-8)*1024);
        }
        else if (startingPosition >= 16 && startingPosition < 32)
        {
            return (uint32_t*)(0x20004000 + (startingPosition-16)*512);
        }
        else if (startingPosition >= 32 && startingPosition < 40)
        {
            return (uint32_t*)(0x20006000 + (startingPosition-32)*1024);
        }
    }
    return 0;
}
// REQUIRED: add your custom MPU functions here (eg to return the srd bits)
void initMemoryMap()
{
    uint8_t i = 0;
    for (i = 0; i < 40; i ++)
    {
        memoryMap[i] = 0;
    }
}
int8_t findMPURegion(uint32_t add)
{
    if (add >= 0x20001000 && add < 0x20002000)
    {
        return 0x3;
    }
    else if (add >= 0x20002000 && add < 0x20004000)
    {
        return 0x4;
    }
    else if (add >= 0x20004000 && add < 0x20005000)
    {
        return 0x5;
    }
    else if (add >= 0x20005000 && add < 0x20006000)
    {
        return 0x6;

    }
    else if (add >= 0x20006000 && add <= 0x20008000)
    {
        return 0x7;
    }
    else
    {
        return -1;
    }
}
void setSrdBits(uint8_t index, uint8_t srdMask[NUM_SRAM_REGIONS], bool allowRW)
{
    uint8_t temp = 0;
    if (index < 8)
    {
        temp = 1 << (index);
        if (allowRW)
            srdMask[0] |= temp;
        else
            srdMask[0] &= ~temp;
    }
    else if (index >= 8 && index < 16)
    {
        temp = 1 << (index-8);
        if (allowRW)
            srdMask[1] |= temp;
        else
            srdMask[1] &= ~temp;
    }
    else if (index >= 16 && index < 24)
    {
        temp = 1 << (index-16);
        if (allowRW)
            srdMask[2] |= temp;
        else
            srdMask[2] &= ~temp;
    }
    else if (index >= 24 && index < 32)
    {
        temp = 1 << (index-24);
        if (allowRW)
            srdMask[3] |= temp;
        else
            srdMask[3] &= ~temp;
    }
    else if (index >= 32 && index < 40)
    {
        temp = 1 << (index-32);
        if (allowRW)
            srdMask[4] |= temp;
        else
            srdMask[4] &= ~temp;
    }
}
//void setSramAccessWindow(uint32_t *baseAdd, uint32_t sizeInBytes, uint8_t srdMask[NUM_SRAM_REGIONS])
void setSramAccessWindow(uint8_t srdMask[NUM_SRAM_REGIONS])
{
    uint8_t i = 0;
    for (i = 0; i < 5; i++)
    {
        NVIC_MPU_NUMBER_R = i+3;
        NVIC_MPU_ATTR_R &= ~0x0000FF00;
        NVIC_MPU_ATTR_R |= ((uint32_t)srdMask[i] << 8);
    }
}
// REQUIRED: initialize MPU here
void initMpu(void)
{
    // REQUIRED: call your MPU functions here
    mpuAllAccessRegion();
    allowFlashAccess();
    allowPeripheralAccess();
    setupSramAccess();
    NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_ENABLE;   //enable mpu
    initMemoryMap();
}

