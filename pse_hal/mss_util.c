/***************************************************************************
 * (c) Copyright 2008 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * Utility functions
 *
 * SVN $Revision: 10515 $
 * SVN $Date: 2018-11-08 18:00:02 +0000 (Thu, 08 Nov 2018) $
 */

#include "mss_util.h"
#include "mss_coreplex.h"
#include "mss_hart_ints.h"


/*------------------------------------------------------------------------------
 * Disable all interrupts.
 */
void __disable_irq(void)
{
    clear_csr(mstatus, MSTATUS_MPIE);
    clear_csr(mstatus, MSTATUS_MIE);
}

/*------------------------------------------------------------------------------
 * Enabler all interrupts.
 */
void __enable_irq(void)
{
    set_csr(mstatus, MSTATUS_MIE);  /* mstatus Register- Machine Interrupt Enable */
}

/*------------------------------------------------------------------------------
 * Enable particular local interrupt
 */
void __enable_local_irq(int8_t local_interrupt)
{
    set_csr(mie, (1LLu << (local_interrupt + 16)));  /* mie Register- Machine Interrupt Enable Register */
}

/*------------------------------------------------------------------------------
 * Disable particular local interrupt
 */
void __disable_local_irq(int8_t local_interrupt)
{
	clear_csr(mie, (0x01 << (local_interrupt + 16)));  /* mie Register- Machine Interrupt Enable Register */
}


/*
 * Functions 
 */
uint64_t readmtime(void)
{
  volatile uint32_t hartid = read_csr(mhartid);
  volatile uint64_t * mtime_hart = 0U;
  uint64_t mtime = 0U;

  switch(hartid) {
  case 0:
    mtime_hart = (volatile uint64_t *)U5CP_MTIME_H0;
    break;

  case 1:
    mtime_hart = (volatile uint64_t *)U5CP_MTIME_H1;
    break;

  case 2:
    mtime_hart = (volatile uint64_t *)U5CP_MTIME_H2;
    break;
 
  case 3:
    mtime_hart = (volatile uint64_t *)U5CP_MTIME_H3;
    break;

  case 4:
    mtime_hart = (volatile uint64_t *)U5CP_MTIME_H4;
    break;

  default:
    return 0U;
    break;
  }

  mtime = *mtime_hart;
  return mtime;
}


uint64_t readmcycle(void)
{
  return read_csr(mcycle);
}

void sleep_ms(uint64_t msecs)
{
  uint64_t starttime = readmtime();
  volatile uint64_t endtime = 0U;

  while(endtime < (starttime+msecs)) {
    endtime = readmtime();
  }
}

void sleep_cycles(uint64_t ncycles)
{
  uint64_t starttime = readmcycle();
  volatile uint64_t endtime = 0U;

  while(endtime < (starttime + ncycles)) {
    endtime = readmcycle();
  }
}

void exit_simulation(void) {
  uint32_t hartid = read_csr(mhartid);
  volatile uint32_t * exit_simulation = (uint32_t *)0x60000000U;
  
  
  *exit_simulation = 1;
}

__attribute__((aligned(16))) uint64_t get_program_counter(void)
{
  uint64_t prog_counter;
  asm volatile ("auipc %0, 0" : "=r"(prog_counter));
  return prog_counter;
}




uint64_t get_stack_pointer(void)
{
  uint64_t prog_counter;
  asm volatile ("addi %0, sp, 0" : "=r"(prog_counter));
  return prog_counter;
}


#ifdef PRINTF_DEBUG_SUPPORTED
void display_address_of_interest(uint64_t * address_of_interest, int nb_locations) {
  uint64_t * p_addr_of_interest = address_of_interest;
  int inc;
  pse_printf(" Displaying address of interest: 0x%lx\n", p_addr_of_interest);

  for (inc = 0U; inc < nb_locations; ++inc) {
    pse_printf("  address of interest: 0x%lx: 0x%lx\n", p_addr_of_interest, *p_addr_of_interest);
    p_addr_of_interest = p_addr_of_interest + 8;
  }
}
#endif




