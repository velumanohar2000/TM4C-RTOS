// Memory manager functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef MM_H_
#define MM_H_

#define NUM_SRAM_REGIONS 5

#include <stdbool.h>
#include <inttypes.h>
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void * mallocFromHeap(uint32_t size_in_bytes, uint8_t srdMask[NUM_SRAM_REGIONS]);
void initMpu(void);
void initMemoryMap();
// void setSramAccessWindow(uint32_t *baseAdd, uint32_t sizeInBytes, uint8_t srdMask[NUM_SRAM_REGIONS]);
void setSramAccessWindow(uint8_t srdMask[NUM_SRAM_REGIONS]);
void setSrdBits(uint8_t index, uint8_t srdMask[NUM_SRAM_REGIONS], bool allowRW);
int8_t findMPURegion(uint32_t add);

#endif
