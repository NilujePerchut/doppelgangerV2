#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>

#include "systick.h"

volatile uint32_t system_millis; /* overflows every 49 days */


/* Milliseconds timers using systicks */


void sys_tick_handler(void)
{
	system_millis++;
}

void delay_ms(uint32_t delay)
{
	uint32_t wake = system_millis + delay;
	while (wake>system_millis);
}

void init_systick(void)
{
	/* clock rate / 1000 to get 1mS interrupt rate */
	systick_set_reload(168000);
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_counter_enable();
	/* this done last */
	systick_interrupt_enable();
}

void init_usec_delay(void)
{
	/* set up a microsecond free running timer for ... things... */
	rcc_periph_clock_enable(RCC_TIM6);
	/* microsecond counter */
	timer_set_prescaler(TIM6, rcc_apb1_frequency / 2000000 - 1);
	timer_set_period(TIM6, 0xffff);
	timer_one_shot_mode(TIM6);
}

void delay_us(uint16_t us)
{
	TIM_ARR(TIM6) = 2*us;
	TIM_EGR(TIM6) = TIM_EGR_UG;
	TIM_CR1(TIM6) |= TIM_CR1_CEN;
	while (TIM_CR1(TIM6) & TIM_CR1_CEN);
}
