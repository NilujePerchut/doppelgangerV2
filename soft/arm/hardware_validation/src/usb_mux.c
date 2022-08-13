#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "usb_mux.h"

static inline void gpio_write(uint32_t gpioport, uint16_t gpio, int value)
{
	if (value)
		gpio_set(gpioport, gpio);
	else
		gpio_clear(gpioport, gpio);
}

void set_usb_mux(int mux_index)
{
	gpio_write(USBMUX_PORT, USBMUX_SEL0, mux_index&0x01);
	gpio_write(USBMUX_PORT, USBMUX_SEL1, mux_index&0x02);
}

void init_usb_mux(void)
{
	rcc_periph_clock_enable(USBMUX_RCC);
	gpio_mode_setup(USBMUX_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
					USBMUX_SEL0 | USBMUX_SEL1);
	set_usb_mux(MUX_ARM_USB);
}
