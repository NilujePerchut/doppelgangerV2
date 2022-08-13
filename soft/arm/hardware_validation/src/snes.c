#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include "doppelganger.h"
#include "snes.h"
#include "clc.h"
#include "pic_iface.h"
#include "keys.h"


/* Quick and dirty snes test
 *
 * | RJ45 pin |  PU  | Function |    Signal   | PIC PORT | CLC channel |     dir     | ARM PORT | ARM DIR |
 * +----------+------+----------+-------------+----------+-------------+-------------+----------+---------+
 * | 2        | 3.5K | Clock    | RJ_SPI_CLK  |   RC4    | 1           | RJ45 -> ARM | GPIOA-15 | In      |
 * | 4        |      | Data     | RJ_SPI_MISO |   RC7    | 4           | ARM -> RJ45 | GPIOC-12 | Out     |
 * | 7        | 3.5K | Latch    | RJ_SPI_CS   |   RC6    | 3           | RJ45 -> ARM | GPIOC-11 | In      |
 * +----------+------+----------+-------------+----------+-------------+-------------+----------+---------+
 *
 */

extern volatile struct keys key_buff;

#define SNES_PORT GPIO15
#define SNES_SCK GPIO15

static inline unsigned char snes_sck(void)
{
	return gpio_get(GPIOA, GPIO15)?1:0;
}

static inline unsigned char snes_cs(void)
{
	return gpio_get(GPIOC, GPIO11)?1:0;
}

static inline void snes_do(int val)
{
	if (val)
		gpio_set(GPIOC, GPIO12);
	else
		gpio_clear(GPIOC, GPIO12);
}

void snes_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Setup CLC */
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO15); // Channel1
	gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO11); // Channel3
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12); // Channel4
	setup_clc_passthrough_channel1(RJ45_2_ARM);
	setup_clc_passthrough_channel3(RJ45_2_ARM);
	setup_clc_passthrough_channel4(ARM_2_RJ45);

	/* Set pullups */
	usart_send_blocking(PIC_USART, 'P');

}

void snes(void)
{
	init_keys();
	snes_init();
	snes_do(1);
	while(1) {
		update_key_buff();

		/* Idle */
		snes_do(1);

		/* Wait for latch */
		while(!snes_cs());
		while(snes_cs());

		snes_do(key_buff.lk);
		while(snes_sck());
		while(!snes_sck());

		snes_do(key_buff.lp);
		while(snes_sck());
		while(!snes_sck());

		snes_do(key_buff.select);
		while(snes_sck());
		while(!snes_sck());

		snes_do(key_buff.start);
		while(snes_sck());
		while(!snes_sck());

		snes_do(key_buff.up);
		while(snes_sck());
		while(!snes_sck());

		snes_do(key_buff.down);
		while(snes_sck());
		while(!snes_sck());

		snes_do(key_buff.left);
		while(snes_sck());
		while(!snes_sck());

		snes_do(key_buff.right);
		while(snes_sck());
		while(!snes_sck());

		snes_do(key_buff.mk);
		while(snes_sck());
		while(!snes_sck());

		snes_do(key_buff.mp);
		while(snes_sck());
		while(!snes_sck());

		snes_do(key_buff.hp);
		while(snes_sck());
		while(!snes_sck());

		snes_do(key_buff.hk);
		while(snes_sck());
		while(!snes_sck());
		snes_do(1);
	}
}
