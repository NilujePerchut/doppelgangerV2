#include <stdio.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "keys.h"


/* Keys definition */

/* UP */
#define BT_UP_PORT GPIOA
#define BT_UP GPIO10

/* DOWN */
#define BT_DOWN_PORT GPIOC
#define BT_DOWN GPIO1

/* LEFT */
#define BT_LEFT_PORT GPIOA
#define BT_LEFT GPIO4

/* RIGHT */
#define BT_RIGHT_PORT GPIOA
#define BT_RIGHT GPIO1

/* LP (SQUARE) */
#define BT_LP_PORT GPIOC
#define BT_LP GPIO5

/* MP (TRIANGLE) */
#define BT_MP_PORT GPIOA
#define BT_MP GPIO5

/* HP (R1) */
#define BT_HP_PORT GPIOA
#define BT_HP GPIO6

/* AP (L1) */
#define BT_AP_PORT GPIOA
#define BT_AP GPIO3

/* LK (CROSS) */
#define BT_LK_PORT GPIOB
#define BT_LK GPIO0

/* MK (CIRCLE) */
#define BT_MK_PORT GPIOC
#define BT_MK GPIO4

/* HK (R2) */
#define BT_HK_PORT GPIOB
#define BT_HK GPIO1

/* AK (L2) */
#define BT_AK_PORT GPIOA
#define BT_AK GPIO2

/* L3 */
#define BT_L3_PORT GPIOA
#define BT_L3 GPIO0

/* R3 */
#define BT_R3_PORT GPIOA
#define BT_R3 GPIO7

/* START */
#define BT_START_PORT GPIOC
#define BT_START GPIO15

/* SELECT */
#define BT_SELECT_PORT GPIOC
#define BT_SELECT GPIO13

/* HOME */
#define BT_HOME_PORT GPIOC
#define BT_HOME GPIO14


volatile struct keys key_buff;

void init_keys(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
					BT_UP | BT_LEFT | BT_RIGHT | BT_MP | BT_HP | BT_AP |\
		            BT_AK | BT_L3 | BT_R3);
	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
					BT_LK | BT_HK);
	gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
					BT_DOWN | BT_LP | BT_MK | BT_START | BT_SELECT |\
					BT_HOME);

}

void update_key_buff(void)
{
	/* Directions */
	key_buff.up = gpio_get(BT_UP_PORT, BT_UP);
	key_buff.down = gpio_get(BT_DOWN_PORT, BT_DOWN);
	key_buff.left = gpio_get(BT_LEFT_PORT, BT_LEFT);
	key_buff.right = gpio_get(BT_RIGHT_PORT, BT_RIGHT);

	/* Punches */
	key_buff.lp = gpio_get(BT_LP_PORT, BT_LP);
	key_buff.mp = gpio_get(BT_MP_PORT, BT_MP);
	key_buff.hp = gpio_get(BT_HP_PORT, BT_HP);
	key_buff.ap = gpio_get(BT_AP_PORT, BT_AP);

	/* Kicks */
	key_buff.lk = gpio_get(BT_LK_PORT, BT_LK);
	key_buff.mk = gpio_get(BT_MK_PORT, BT_MK);
	key_buff.hk = gpio_get(BT_HK_PORT, BT_HK);
	key_buff.ak = gpio_get(BT_AK_PORT, BT_AK);

	key_buff.l3 = gpio_get(BT_L3_PORT, BT_L3);
	key_buff.r3 = gpio_get(BT_R3_PORT, BT_R3);

	key_buff.start = gpio_get(BT_START_PORT, BT_START);
	key_buff.select = gpio_get(BT_SELECT_PORT, BT_SELECT);
	key_buff.home = gpio_get(BT_HOME_PORT, BT_HOME);
}

void dump_keys(void)
{
	update_key_buff();
	printf(" U  D  L  R ");
	printf("LP MP HP 3P ");
	printf("LK MK HK 3K ");
	printf("L3 R3 ");
	printf("ST SL HM ");
	printf("\n");
	printf(" %c  %c  %c  %c ",
		   key_buff.up ? 'O': 'X',
		   key_buff.down ? 'O': 'X',
		   key_buff.left ? 'O': 'X',
		   key_buff.right ? 'O': 'X'
		);
	printf(" %c  %c  %c  %c ",
		   key_buff.lp ? 'O': 'X',
		   key_buff.mp ? 'O': 'X',
		   key_buff.hp ? 'O': 'X',
		   key_buff.ap ? 'O': 'X'
		);
	printf(" %c  %c  %c  %c ",
		   key_buff.lk ? 'O': 'X',
		   key_buff.mk ? 'O': 'X',
		   key_buff.hk ? 'O': 'X',
		   key_buff.ak ? 'O': 'X'
		);
	printf(" %c  %c ",
		   key_buff.l3 ? 'O': 'X',
		   key_buff.r3 ? 'O': 'X'
		);
	printf(" %c  %c  %c",
		   key_buff.start ? 'O': 'X',
		   key_buff.select ? 'O': 'X',
		   key_buff.home ? 'O': 'X'
		);
	printf("\n");
}
