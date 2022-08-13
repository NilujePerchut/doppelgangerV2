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
#include "clc.h"
#include "snes.h"

static void flush_serial_rx(uint32_t usart)
{
	while (USART_SR(usart) & USART_SR_RXNE)
		usart_recv_blocking(usart);
}

static unsigned short get_16_bits(void)
{
	unsigned short val;
	val = usart_recv_blocking(DBG_USART);
	val |= usart_recv_blocking(DBG_USART) << 8;
	val |= usart_recv_blocking(DBG_USART) << 8;
	val |= usart_recv_blocking(DBG_USART) << 8;
	return val;
}


static void pic_gpio_menu(void)
{
	char c;

	puts("\nPIC sub menu\n");
	puts("\t0 => PIC GPIO pulses\n");
	puts("\t1 => Set GPIO0\n");
	puts("\t2 => Clear GPOI0\n");
	puts("\t3 => Set GPIO1\n");
	puts("\t4 => Clear GPOI1\n");
	c = usart_recv_blocking(DBG_USART);
	switch(c) {
		case '0':
			usart_send_blocking(PIC_USART, 'G');
			break;
		case '1':
			set_pic_gpio(0, 1);
			break;
		case '2':
			set_pic_gpio(0, 0);
			break;
		case '3':
			set_pic_gpio(1, 1);
			break;
		case '4':
			set_pic_gpio(1, 0);
			break;
	}
}


static void pic_sub_menu(void)
{
	volatile char c;
	volatile unsigned int value;
	volatile unsigned int address;

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
	puts("\tc => CLC setup arm -> rj45\n");
	puts("\td => CLC setup rj45 _> arm\n");
	puts("\tg => PIC GPIO menu\n");
	puts("\tr => Read pic memory\n");
	puts("\tw => Read pic memory\n");

	fflush(stdin);
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
		case 'c':
			setup_clc_passthrough_channel1(ARM_2_RJ45);
			setup_clc_passthrough_channel2(ARM_2_RJ45);
			setup_clc_passthrough_channel3(ARM_2_RJ45);
			setup_clc_passthrough_channel4(ARM_2_RJ45);
			rcc_periph_clock_enable(RCC_GPIOA);
			rcc_periph_clock_enable(RCC_GPIOC);
			gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO15); // Channel1
			gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO10); // Channel2
			gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO11); // Channel3
			gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12); // Channel4
			while(1) {
				gpio_clear(GPIOC, GPIO11);
				gpio_set(GPIOC, GPIO12);

				gpio_set(GPIOA, GPIO15);
				gpio_clear(GPIOC, GPIO10);
				delay_ms(5);
				gpio_clear(GPIOA, GPIO15);
				gpio_set(GPIOC, GPIO10);
				delay_ms(5);

				gpio_set(GPIOC, GPIO11);
				gpio_clear(GPIOC, GPIO12);

				gpio_set(GPIOA, GPIO15);
				gpio_clear(GPIOC, GPIO10);
				delay_ms(5);
				gpio_clear(GPIOA, GPIO15);
				gpio_set(GPIOC, GPIO10);
				delay_ms(5);
			}
			break;
		case 'd':
			rcc_periph_clock_enable(RCC_GPIOA);
			rcc_periph_clock_enable(RCC_GPIOC);
			gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO15); // Channel1
			gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO10); // Channel2
			gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO11); // Channel3
			gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO12); // Channel4
			setup_clc_passthrough_channel1(RJ45_2_ARM);
			setup_clc_passthrough_channel2(RJ45_2_ARM);
			setup_clc_passthrough_channel3(RJ45_2_ARM);
			setup_clc_passthrough_channel4(RJ45_2_ARM);
			volatile unsigned char vals[4];
			while(1) {
				vals[0] = gpio_get(GPIOA, GPIO15)?1:0;
				vals[1] = gpio_get(GPIOC, GPIO10)?1:0;
				vals[2] = gpio_get(GPIOC, GPIO11)?1:0;
				vals[3] = gpio_get(GPIOC, GPIO12)?1:0;
				printf("%d %d %d %d\n", vals[0], vals[1], vals[2], vals[3]);
				delay_ms(200);
			}
			break;
		case 'g':
			pic_gpio_menu();
			break;
		case 'r':
			puts("Enter address (hexa 16 bits without 0x): \n");
			fflush(stdout);
			scanf("%x", &address);
			flush_serial_rx(PIC_USART);
			value = pic_mem_read(address);
			printf("read: 0x%02x @0x%04x\n", value, address);
			break;
		case 'w':
			puts("Enter address (hexa 16 bits without 0x): \n");
			fflush(stdout);
			scanf("%x", &address);
			puts("Enter value (hex 8 bits without 0x): \n");
			fflush(stdout);
			scanf("%x", &value);
			pic_mem_write(address, value);
			printf("wrote: 0x%02x @0x%04x\n", value, address);
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

static void clc_menu(void)
{
	char c;
	unsigned char v;
	unsigned char channel;
	puts("\nCLC test menu\n");
	puts("\n1-4 Setup CLC channel\n");
	puts("\ni Monitor input clc channel\n");
	puts("\no Toogle output clc channel\n");
	c = usart_recv_blocking(DBG_USART);
	switch(c) {
		case '1':
		case '2':
		case '3':
		case '4':
			channel = c - 0x30;
			printf("Selected channel %d\n", channel);
			usart_send_blocking(PIC_USART, 'c');
			usart_send_blocking(PIC_USART, c);
			puts("\nSet direction:");
			puts("\n\t0) RJ45_2_ARM:");
			puts("\n\t1) ARM_2_RJ45:");
			c = usart_recv_blocking(DBG_USART);
			usart_send_blocking(PIC_USART, c);
			switch (c-0x30) {
				case RJ45_2_ARM:
					puts("Set RC0 as input\n");
					gpio_mode_setup(CLC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
									channel-1);
					break;
				case ARM_2_RJ45:
					puts("Set RC0 as output\n");
					gpio_mode_setup(CLC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
									channel-1);
					break;
				default:
					puts("\nError: Invalid direction\n");
					break;
			}
			break;
		case 'i':
			/* Monitor a CLC input channel */
			puts("\nChannel to monitor 1-4:");
			c = usart_recv_blocking(DBG_USART);
			puts("\n");
			while(1) {
				v = gpio_get(CLC_PORT, c-0x30-1);
				printf("%c\n", 0x30 + v);
				delay_ms(1000);
			}
		case 'o':
			puts("\nChannel to toggle 1-4:");
			c = usart_recv_blocking(DBG_USART);
			while(1){
				gpio_toggle(CLC_PORT, c-0x30-1);
				puts("\nPress any key to toggle\n");
				usart_recv_blocking(DBG_USART);
			}
			break;
		default:
			puts("\nError: not implemented\n");
			break;
	}
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
		puts("\t7 => CLC menu\n");
		puts("\t8 => snes demo\n");

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
			case '7':
				clc_menu();
				break;
			case '8':
				snes();
				break;
			default:
				puts("\nError, choice not valid\n");
		}
	}
}
