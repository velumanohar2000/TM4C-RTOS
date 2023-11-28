/*
 * mpu.h
 *
 *  Created on: Oct 6, 2023
 *      Author: velum
 */
#include <inttypes.h>

#ifndef MPU_H_
#define MPU_H_

void printOutRegionInfo(uint32_t mpuNumber);
void mpuAllAccessRegion();
void allowFlashAccess();
void allowPeripheralAccess();
void setupSramAccess();
//void freeMalloc(uint32_t *address);



// uint8_t srdBitLimits(uint32_t add, uint8_t regionNumber);
// void setSramAccessWindow(uint32_t *baseAdd, uint32_t sizeInBytes, bool allowRW);



#endif /* MPU_H_ */
