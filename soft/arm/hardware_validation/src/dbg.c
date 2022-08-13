#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include "doppelganger.h"
#include "dbg.h"


void init_dbg_usart(void)
{
	rcc_periph_clock_enable(DBG_USART_RCC);

	/* Configure GPIO as alternate function */
	rcc_periph_clock_enable(DBG_USART_PORT_RCC);
	gpio_mode_setup(DBG_USART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE,
					DBG_USART_TX_GPIO|DBG_USART_RX_GPIO);
	gpio_set_af(DBG_USART_PORT, GPIO_AF7, DBG_USART_TX_GPIO|DBG_USART_RX_GPIO);

	usart_set_baudrate(DBG_USART, 115200);
	usart_set_databits(DBG_USART, 8);
	usart_set_stopbits(DBG_USART, USART_STOPBITS_1);
	usart_set_mode(DBG_USART, USART_MODE_TX_RX);
	usart_set_parity(DBG_USART, USART_PARITY_NONE);
	usart_set_flow_control(DBG_USART, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(DBG_USART);
}


/**
 * Use USART_CONSOLE as a console.
 * This is a syscall for newlib
 * @param file
 * @param ptr
 * @param len
 * @return
 */
int _write(int file, char *ptr, int len)
{
	int i;

	if (file == STDOUT_FILENO || file == STDERR_FILENO) {
		for (i = 0; i < len; i++) {
			if (ptr[i] == '\n') {
				usart_send_blocking(DBG_USART, '\r');
			}
			usart_send_blocking(DBG_USART, ptr[i]);
		}
		return i;
	}
	errno = EIO;
	return -1;
}
