/*******************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions.  All rights reserved.
 * 
 * @file mpu.h
 * @author Microchip-PRO Embedded Systems Solutions
 * @brief defines and structures related to the MPU
 *
 * SVN $Revision: 10513 $
 * SVN $Date: 2018-11-06 13:40:20 +0000 (Tue, 06 Nov 2018) $
 */

#ifndef MPU_H
#define MPU_H

#include <stdint.h>

typedef struct {

	struct {
		volatile uint64_t pmp :36;
		volatile uint64_t rsrvd :20;
		volatile uint64_t mode :8;
	} CFG[16];

	volatile uint64_t STATUS;
	uint64_t gap[15];
} mpu_t;

#define MPU_MODE_READ		(1u << 0u)
#define MPU_MODE_WRITE		(1u << 1u)
#define MPU_MODE_EXEC		(1u << 2u)
#define MPU_MODE_NAPOT		(3u << 3u)

#define MPU ((mpu_t*) 0x20005000)

#define MPU_ONES(bits) ((1llu << (bits))-1)

#define MPU_SIZE_4K 	12u
#define MPU_SIZE_8K		13u
#define MPU_SIZE_16K	14u
#define MPU_SIZE_32K	15u
#define MPU_SIZE_64K	16u
#define MPU_SIZE_128K	17u
#define MPU_SIZE_256K	18u

static __inline__ uint64_t MPU_CFG(uint64_t base, unsigned xbits) {
	return ((base & ~MPU_ONES(xbits)) | MPU_ONES(xbits - 1)) >> 2u;
}

#endif
