/*
 * asm.h
 *
 *  Created on: Sep 30, 2023
 *      Author: velum
 */
#include <inttypes.h>

#ifndef ASM_H_
#define ASM_H_

extern void setPSP(uint32_t pspStackPointer);
extern void setControlRegister(uint8_t controlRegisterValue);
extern uint32_t* getPSP();
extern uint32_t* getMSP();
extern uint8_t dividebyzero();
extern uint32_t getR0();
extern void getRegisters(uint32_t *array);

extern void pushToPsp();
extern void popFromPsp();
extern void writeToLr(uint32_t exceptionReturn);
extern void unreadyPush(uint32_t xPSR, void* newFnAddress, uint32_t exceptionReturn);


#endif /* ASM_H_ */
