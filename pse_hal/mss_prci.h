 /*******************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions.  All rights reserved.
 *
 * @file mss_clint.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief PSE PLIC and PRCI access data structures and functions.
 *
 * PRCI: Power, Reset, Clock, Interrupt
 *
 * SVN $Revision: 10597 $
 * SVN $Date: 2018-11-23 15:48:29 +0000 (Fri, 23 Nov 2018) $
 */

#ifndef MSS_PRCI_H
#define MSS_PRCI_H

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "encoding.h"


#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * PRCI: Power, Reset, Clock, Interrupt
 */
//#define PRCI_BASE   0x44000000UL
#define PRCI_BASE   0x10000000UL	/* FU540-C000 on unleashed board- 0x10000000UL	*/


#ifdef __cplusplus
}
#endif


#endif /* MSS_PRCI_H_ */
