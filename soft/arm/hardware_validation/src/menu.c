#include <stdio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include "doppelganger.h"
#include "pic_iface.h"
#include "systick.h"
#include "keys.h"
#include "usb_mux.h"
#include "arm_gpios.h"
#include "adc.h"
#include "config.h"
#include "menu.h"

static void pic_sub_menu(void)
{
	char c;

	puts("\nPIC sub menu\n");
	puts("\t0 => Assert PIC reset\n");
	puts("\t1 => Deassert PIC reset\n");
	puts("\t2 => Program PIC from ttl serial\n");
	puts("\t3 => PIC serial terminal\n");
	puts("\t4 => Set SNES terms\n");
	puts("\t5 => Reset SNES terms\n");
	puts("\t6 => Set PSX terms\n");
	puts("\t7 => Reset PSX terms\n");
	puts("\t8 => PIC debug\n");
	puts("\t9 => PIC GPIO pulses\n");

	c = usart_recv_blocking(DBG_USART);
	switch(c) {
		case '0':
			gpio_set(PIC_PORT, PIC_RESET);
			break;
		case '1':
			gpio_clear(PIC_PORT, PIC_RESET);
			break;
		case '2':
			flash_pic_from_uart();
			break;
		case '8':
			pic_debug();
			break;
		case '4':
			usart_send_blocking(PIC_USART, 'S');
			break;
		case '5':
			usart_send_blocking(PIC_USART, 's');
			break;
		case '6':
			usart_send_blocking(PIC_USART, 'P');
			break;
		case '7':
			usart_send_blocking(PIC_USART, 'p');
			break;
		case '9':
			usart_send_blocking(PIC_USART, 'G');
			break;
		case '3':
		default:
			puts("\nError: not implemented\n");
			break;
	}
}

static void leds_sub_menu(void)
{
	char c;

	puts("\nLEDs sub menu\n");
	puts("\t0 => Set LED_L\n");
	puts("\t1 => Clear LED_L\n");
	puts("\t2 => Set LED_R\n");
	puts("\t3 => Clear LED_R\n");

	c = usart_recv_blocking(DBG_USART);
	switch(c) {
		case '0':
			gpio_set(LEDS_PORT, LED_L_GPIO);
			break;
		case '1':
			gpio_clear(LEDS_PORT, LED_L_GPIO);
			break;
		case '2':
			gpio_set(LEDS_PORT, LED_R_GPIO);
			break;
		case '3':
			gpio_clear(LEDS_PORT, LED_R_GPIO);
			break;
		default:
			puts("\nError: not implemented\n");
			break;
	}
}

static void brook_sub_menu(void)
{
	char c;

	puts("\nBrook sub menu\n");
	puts("\t0 => Set Brook power command\n");
	puts("\t1 => Clear brook power command\n");

	c = usart_recv_blocking(DBG_USART);
	switch(c) {
		case '0':
			gpio_set(BROOK_PORT, BROOK_CMD);
			break;
		case '1':
			gpio_clear(BROOK_PORT, BROOK_CMD);
			break;
		default:
			puts("\nError: not implemented\n");
			break;
	}
}

static inline void gpio_write(uint32_t gpioport, uint16_t gpio, int value)
{
	if (value)
		gpio_set(gpioport, gpio);
	else
		gpio_clear(gpioport, gpio);
}

static void send_arm_gpio_pulses(void)
{
	// Just send some pulses on AMR GPIOs

	// init
	rcc_periph_clock_enable(ARM_GPIO_RCC);
	gpio_mode_setup(ARM_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
					ARM_GPIO0 | ARM_GPIO1);

	// Pattern:
	//	- GPIO0=0 GPIO1=0
	//	- wait 100ms
	//	- GPIO0=1 GPIO1=0
	//	- wait 40ms
	//	- GPIO0=0 GPIO1=1
	//	- wait 60ms
	//	- GPIO0=1 GPIO1=1
	//	- wait 20ms
	//	- GPIO0=0 GPIO1=0
	delay_ms(10);
	gpio_write(ARM_GPIO_PORT, ARM_GPIO0, 0);
	gpio_write(ARM_GPIO_PORT, ARM_GPIO1, 0);
	delay_ms(100);
	gpio_write(ARM_GPIO_PORT, ARM_GPIO0, 1);
	gpio_write(ARM_GPIO_PORT, ARM_GPIO1, 0);
	delay_ms(40);
	gpio_write(ARM_GPIO_PORT, ARM_GPIO0, 0);
	gpio_write(ARM_GPIO_PORT, ARM_GPIO1, 1);
	delay_ms(60);
	gpio_write(ARM_GPIO_PORT, ARM_GPIO0, 1);
	gpio_write(ARM_GPIO_PORT, ARM_GPIO1, 1);
	delay_ms(20);
	gpio_write(ARM_GPIO_PORT, ARM_GPIO0, 0);
	gpio_write(ARM_GPIO_PORT, ARM_GPIO1, 0);
	delay_ms(100);
}

static void usbmux_sub_menu(void)
{
	char c;

	puts("\nUSB mux sub menu\n");
	puts("\t0 => Set mux to PIC IOs\n");
	puts("\t1 => Set mux to ARM IOs\n");
	puts("\t2 => Set mux to BROOK USB\n");
	puts("\t3 => Set mux to ARM USB\n");
	puts("\t4 => Send pulses on ARM GPIOs\n");

	c = usart_recv_blocking(DBG_USART);
	if ((c>='0') && (c<='3'))
		set_usb_mux(c-'0');
	else if (c=='4')
		send_arm_gpio_pulses();
	else
		puts("\nError: not implemented\n");
}

void clear_screen(void)
{
	puts("\033[2J");
}

void hv_menu(void)
{
	char c;
	uint16_t vio;

	while (1) {

		puts("****************************\n");
		puts("** Doppelganger V2          \n");
		puts("** Hardware validation menu \n");
		puts("****************************\n");
		puts("\t0 => test keys\n");
		puts("\t1 => pic sub menu\n");
		puts("\t2 => leds sub menu\n");
		puts("\t3 => brook sub menu\n");
		puts("\t4 => usb mux sub menu\n");
		puts("\t5 => get vio\n");
		puts("\t6 => dump config\n");

		c = usart_recv_blocking(DBG_USART);
		switch(c) {
			case '0':
				while(1) {
					clear_screen();
					dump_keys();
					delay_ms(1000);
				}
				break;
			case '1':
				pic_sub_menu();
				break;
			case '2':
				leds_sub_menu();
				break;
			case '3':
				brook_sub_menu();
				break;
			case '4':
				usbmux_sub_menu();
				break;
			case '5':
				vio = read_adc_vio();
				printf("VIO=%fV (0x%X)\n", vio*3.3/0xFFF, vio);
				break;
			case '6':
				display_config_info();
				break;
			default:
				puts("\nError, choice not valid\n");
		}
	}
}
