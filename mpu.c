/*
 * mpu.c
 *
 *  Created on: Oct 6, 2023
 *      Author: velum
 */
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "gpio.h"
#include "mm.h"
#include "mpu.h"

#define NVIC_MPU_ATTR_RW_ALL (0x3 << 24)
#define NVIC_MPU_ATTR_RW_KERNEL (0x1 << 24)

int16_t memoryMap [40];
uint8_t srdBits[5];


void printOutRegionInfo(uint32_t mpuNumber)
{
    putsUart0("\nMPU REGION ");
    iToA(mpuNumber);
    putsUart0("\n");
    putsUart0("All Attributes: ");
    intToHex(NVIC_MPU_ATTR_R);
    putsUart0("\n");
    putsUart0("SRD Bits: ");
    intToHex(NVIC_MPU_ATTR_R & 0xFF00);
    putsUart0("\n");
}


void mpuAllAccessRegion()
{
    NVIC_MPU_CTRL_R &= ~NVIC_MPU_CTRL_ENABLE;

    //MPU All Access Region
    NVIC_MPU_NUMBER_R = 0x0;
    NVIC_MPU_BASE_R = 0x00000000;
    NVIC_MPU_ATTR_R |= (0x1F<<1) |  NVIC_MPU_ATTR_RW_ALL | NVIC_MPU_ATTR_ENABLE | NVIC_MPU_ATTR_XN; // size = Log2(4GB)-1, size = 31 == 1F == 11111
    //printOutRegionInfo(NVIC_MPU_NUMBER_R);
}
void allowFlashAccess()
{
    //FLASH
    NVIC_MPU_NUMBER_R = 0x1;
    NVIC_MPU_BASE_R = 0x00000000;
    NVIC_MPU_ATTR_R |= (0x11<<1) | NVIC_MPU_ATTR_RW_ALL | NVIC_MPU_ATTR_ENABLE | NVIC_MPU_ATTR_CACHEABLE; //log2(256*1024)-1 = 17 = 0x11
    //printOutRegionInfo(NVIC_MPU_NUMBER_R);
}
void allowPeripheralAccess()
{
    //Protect Kernel region
    // 0x0000000 - 0x20001000
    NVIC_MPU_NUMBER_R = 0x2;
    NVIC_MPU_BASE_R = 0x20000000;
    NVIC_MPU_ATTR_R |= (0xB<<1) | NVIC_MPU_ATTR_RW_KERNEL | NVIC_MPU_ATTR_ENABLE | NVIC_MPU_ATTR_CACHEABLE | NVIC_MPU_ATTR_BUFFRABLE | NVIC_MPU_ATTR_XN; //log2(0x4400 0000 - 0x4000 0000)-1 = 25 = 0x19
    //printOutRegionInfo(NVIC_MPU_NUMBER_R);

}

void setupSramAccess()
{
    //SRAM
    //4k
    NVIC_MPU_CTRL_R &= ~NVIC_MPU_CTRL_ENABLE;

    NVIC_MPU_NUMBER_R = 0x3;
    NVIC_MPU_BASE_R = 0x20001000;
    NVIC_MPU_ATTR_R |= (0xB<<1) | NVIC_MPU_ATTR_RW_KERNEL | NVIC_MPU_ATTR_ENABLE | NVIC_MPU_ATTR_CACHEABLE | NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_XN; //log2(4*1024)-1 = 11 == 0xB
   // printOutRegionInfo(NVIC_MPU_NUMBER_R);
    //8k
    NVIC_MPU_NUMBER_R = 0x4;
    NVIC_MPU_BASE_R = 0x20002000;
    NVIC_MPU_ATTR_R |= (0xC<<1) | NVIC_MPU_ATTR_RW_KERNEL | NVIC_MPU_ATTR_ENABLE | NVIC_MPU_ATTR_CACHEABLE | NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_XN; //log2(8*1024)-1 = 12 == 0xC
   // printOutRegionInfo(NVIC_MPU_NUMBER_R);
    //4k
    NVIC_MPU_NUMBER_R = 0x5;
    NVIC_MPU_BASE_R = 0x20004000;
    NVIC_MPU_ATTR_R |= (0xB<<1) | NVIC_MPU_ATTR_RW_KERNEL | NVIC_MPU_ATTR_ENABLE | NVIC_MPU_ATTR_CACHEABLE | NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_XN; //log2(4*1024)-1 = 11 == 0xB
   // printOutRegionInfo(NVIC_MPU_NUMBER_R);
    //4k
    NVIC_MPU_NUMBER_R = 0x6;
    NVIC_MPU_BASE_R = 0x20005000;
    NVIC_MPU_ATTR_R |= (0xB<<1) | NVIC_MPU_ATTR_RW_KERNEL | NVIC_MPU_ATTR_ENABLE | NVIC_MPU_ATTR_CACHEABLE | NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_XN; //log2(4*1024)-1 = 11 == 0xB
   // printOutRegionInfo(NVIC_MPU_NUMBER_R);
    //8k
    NVIC_MPU_NUMBER_R = 0x7;
    NVIC_MPU_BASE_R = 0x20006000;
    NVIC_MPU_ATTR_R |= (0xC<<1) | NVIC_MPU_ATTR_RW_KERNEL | NVIC_MPU_ATTR_ENABLE | NVIC_MPU_ATTR_CACHEABLE | NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_XN; //log2(8*1024)-1 = 12 == 0xC
}














