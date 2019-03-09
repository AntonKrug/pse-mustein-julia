/*******************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved
 *
 * Code running on E51 hart 0
 ******************************************************************************/

#include <common_macros.h>
#include "mss_util.h"
#include "mss_clint.h"
#include "mss_hart_ints.h"
#include "mss_plic.h"
#include "mss_gpio.h"
#include "mss_sysreg.h"
#include "mss_uart.h"


uint8_t gpio0_bit0_or_gpio2_bit13_plic_0_IRQHandler(void) {
	MSS_UART_polled_tx_string(&g_mss_uart0_lo,
			"\r\nSetting output 0 to high\r\n");


	MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_0, 1);
	MSS_GPIO_clear_irq(GPIO0_LO, MSS_GPIO_0);
	return EXT_IRQ_KEEP_ENABLED;
}

uint8_t gpio0_bit1_or_gpio2_bit13_plic_1_IRQHandler(void) {
	MSS_UART_polled_tx_string(&g_mss_uart0_lo,
			"\r\nSetting output 1 to high\r\n");

	MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_1, 1);
	MSS_GPIO_clear_irq(GPIO0_LO, MSS_GPIO_1);
	return EXT_IRQ_KEEP_ENABLED;
}

uint8_t gpio0_bit2_or_gpio2_bit13_plic_2_IRQHandler(void) {
	MSS_UART_polled_tx_string(&g_mss_uart0_lo,
			"\r\nSetting output 2 to high\r\n");

	MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_2, 1);
	MSS_GPIO_clear_irq(GPIO0_LO, MSS_GPIO_2);
	return EXT_IRQ_KEEP_ENABLED;
}

uint8_t gpio0_non_direct_plic_IRQHandler(void) {
	return EXT_IRQ_KEEP_ENABLED;
}

uint8_t gpio1_non_direct_plic_IRQHandler(void) {
	return EXT_IRQ_KEEP_ENABLED;
}

uint8_t gpio2_non_direct_plic_IRQHandler(void) {
	return EXT_IRQ_KEEP_ENABLED;
}

void e51(void) {
	uint32_t hartid = read_csr(mhartid); // cppcheck-suppress unreadVariable

	// Making sure that the default GPIO0 and 1 are used on interrupts
	SYSREG->GPIO_INTERRUPT_FAB_CR = 0x00000000UL;

	// all clocks on
	SYSREG->SUBBLK_CLOCK_CR = 0xffffffff;

	// MMUART0
	SYSREG->SOFT_RESET_CR &= ~((1u << 0u) | (1u << 4u) | (1u << 5u)
			| (1u << 19u) | (1u << 23u) | (1u << 28u));

	// connect MMUART0 to GPIO, QSPI to pads
	SYSREG->IOMUX0_CR = 0xfffffe7f;

	// pad5,6 = mux 5 (mmuart 0)
	SYSREG->IOMUX1_CR = 0x05500000;

	// IOMUX configurations to allow QSPI pins to the pads
	SYSREG->IOMUX2_CR = 0;
	SYSREG->IOMUX3_CR = 0;
	SYSREG->IOMUX4_CR = 0;
	SYSREG->IOMUX5_CR = 0;

	/*************************************************************************/
	PLIC_init();
	PLIC_SetPriority_Threshold(0);
	PLIC_SetPriority(GPIO0_BIT0_or_GPIO2_BIT0_PLIC_0, 2);
	PLIC_SetPriority(GPIO0_BIT1_or_GPIO2_BIT1_PLIC_1, 2);
	PLIC_SetPriority(GPIO0_BIT2_or_GPIO2_BIT2_PLIC_2, 2);

	PLIC_SetPriority(GPIO0_NON_DIRECT_PLIC, 2);
	PLIC_SetPriority(GPIO1_NON_DIRECT_PLIC, 2);
	PLIC_SetPriority(GPIO2_NON_DIRECT_PLIC, 2);

	__disable_local_irq((int8_t) MMUART0_E51_INT);
	__enable_irq();

	// GPIO0
	MSS_GPIO_init(GPIO0_LO);

	MSS_GPIO_config(GPIO0_LO, MSS_GPIO_0,
	MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE);

	MSS_GPIO_config(GPIO0_LO, MSS_GPIO_1,
	MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE);

	MSS_GPIO_config(GPIO0_LO, MSS_GPIO_2,
	MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE);

	MSS_GPIO_enable_irq(GPIO0_LO, MSS_GPIO_0);
	MSS_GPIO_enable_irq(GPIO0_LO, MSS_GPIO_1);
	MSS_GPIO_enable_irq(GPIO0_LO, MSS_GPIO_2);

	// GPIO1
	MSS_GPIO_init(GPIO1_LO);
	MSS_GPIO_config(GPIO1_LO, MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(GPIO1_LO, MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(GPIO1_LO, MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE);

	// Set all outputs of GPIO1 to 0
	MSS_GPIO_set_outputs(GPIO1_LO, 0x0);

	MSS_UART_init(&g_mss_uart0_lo, MSS_UART_115200_BAUD,
	MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY);
	MSS_UART_polled_tx_string(&g_mss_uart0_lo,
			"Hello World from e51 hart0.\r\n");

	MSS_UART_polled_tx_string(&g_mss_uart0_lo,
			"Starting u54_1 (hart 1) and u54_2 (hart 2).\r\n");

	raise_soft_interrupt(1);
	raise_soft_interrupt(2);

	//asm("wfi");


	while (1) {
		// Stay in the infinite loop, never return from main
		volatile uint64_t delay_loop_sum = 0;
		uint64_t mcycle_start = readmcycle();

		MSS_UART_polled_tx_string(&g_mss_uart0_lo,
				"Setting outputs 0, 1 and 2 to high\r\n");
		MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_0, 1);
		MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_1, 1);
		MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_2, 1);


		for (uint64_t i = 0; i< 10000000; i++) {
			delay_loop_sum = delay_loop_sum + i;
		}

		MSS_UART_polled_tx_string(&g_mss_uart0_lo,
				"Setting outputs 0, 1 and 2 to low\r\n");
		MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_0, 0);
		MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_1, 0);
		MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_2, 0);


		for (uint64_t i = 0; i< 10000000; i++) {
			delay_loop_sum = delay_loop_sum + i;
		}

		uint64_t mcycle_end = readmcycle();
		uint64_t delta_mcycle = mcycle_end - mcycle_start; // cppcheck-suppress unreadVariable
	}

}


