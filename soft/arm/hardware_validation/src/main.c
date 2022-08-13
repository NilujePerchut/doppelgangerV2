#include <stdio.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include "doppelganger.h"
#include "dbg.h"
#include "pic_iface.h"
#include "systick.h"
#include "keys.h"
#include "usb_mux.h"
#include "menu.h"
#include "adc.h"

static void init_pll(void)
{
	rcc_periph_clock_enable(LEDS_RCC);
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
}

static void init_leds(void)
{
	rcc_periph_clock_enable(LEDS_RCC);
	gpio_mode_setup(LEDS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
					LED_L_GPIO | LED_R_GPIO);

	gpio_mode_setup(BROOK_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BROOK_CMD);

}

int main(void)
{
	int i;
	uint16_t c;

	init_pll();
	init_leds();
	init_keys();
	init_systick();
	init_usec_delay();
	init_pic_gpio();
	init_dbg_usart();
	init_pic_usart();
	init_usb_mux();
	init_adc();

	while (1) {
		hv_menu();
		delay_ms(100);
	}
	return 0;
}
