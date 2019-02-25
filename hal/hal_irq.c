/***************************************************************************//**
 * (c) Copyright 2007-2018 Microsemi SoC Products Group. All rights reserved.
 * 
 * Legacy interrupt control functions for the Microsemi driver library hardware
 * abstraction layer.
 *
 * SVN $Revision: 10516 $
 * SVN $Date: 2018-11-08 18:09:23 +0000 (Thu, 08 Nov 2018) $
 */
#include <stdio.h>

#include "hal.h"
#include "atomic.h"
#include "encoding.h"
#include "mcall.h"
#include "mtrap.h"

/*------------------------------------------------------------------------------
 * 
 */
void HAL_enable_interrupts(void) {
    __enable_irq();
}

/*------------------------------------------------------------------------------
 * 
 */
psr_t HAL_disable_interrupts(void) {
    psr_t psr;
    psr = read_csr(mstatus);
    __disable_irq();
    return(psr);
}

/*------------------------------------------------------------------------------
 * 
 */
void HAL_restore_interrupts(psr_t saved_psr) {
    write_csr(mstatus, saved_psr);
}

