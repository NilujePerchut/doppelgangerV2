#include <stdint.h>

#define GPIO0 (1<<0)
#define GPIO1 (1<<1)
#define GPIO2 (1<<2)
#define GPIO3 (1<<3)
#define GPIO4 (1<<4)
#define GPIO5 (1<<5)
#define GPIO6 (1<<6)
#define GPIO7 (1<<7)
#define GPIO8 (1<<8)
#define GPIO9 (1<<9)
#define GPIO10 (1<<10)
#define GPIO11 (1<<11)
#define GPIO12 (1<<12)
#define GPIO13 (1<<13)
#define GPIO14 (1<<14)
#define GPIO15 (1<<15)

#define GPIO_MODE_INPUT 0
#define GPIO_PUPD_NONE 0

#define PERIPH_BASE_AHB2 0x50000000

#define GPIO_PORT_A_BASE (PERIPH_BASE_AHB2 + 0x000)
#define GPIO_PORT_B_BASE (PERIPH_BASE_AHB2 + 0x400)
#define GPIO_PORT_C_BASE (PERIPH_BASE_AHB2 + 0x800)
#define GPIOA GPIO_PORT_A_BASE
#define GPIOB GPIO_PORT_B_BASE
#define GPIOC GPIO_PORT_C_BASE

void gpio_mode_setup(uint32_t port, uint8_t mode, uint8_t cfn, uint32_t io);
uint16_t gpio_port_read(uint32_t port);

