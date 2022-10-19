#include <stdint.h>

#define RCC_GPIOA ((0x30 << 5) + 0)
#define RCC_GPIOB ((0x30 << 5) + 1)
#define RCC_GPIOC ((0x30 << 5) + 2)

void rcc_periph_clock_enable(uint32_t port);

