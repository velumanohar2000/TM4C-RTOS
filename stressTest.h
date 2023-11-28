/*
 * stressTest.h
 *
 *  Created on: Oct 8, 2023
 *      Author: velum
 */

#ifndef STRESSTEST_H_
#define STRESSTEST_H_

#include <inttypes.h>

void testAllAccessRegion();
void stressTestMalloc();
void flashLeds();
void testRegion(uint32_t *ptr, uint32_t initialValue);
void switchToPrivilegedISR();
void testSRAMRegion();

#endif /* STRESSTEST_H_ */
