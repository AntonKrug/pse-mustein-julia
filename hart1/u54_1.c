/***********************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * Code running on U54 hart 1
 */

#include <common_macros.h>
#include <fractal_display.h>


void u54_1(void) {
	juliaMain(0x10100000);
}

