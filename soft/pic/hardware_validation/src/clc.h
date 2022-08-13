#pragma once
//#define NO_BIT_DEFINES
//#include <pic14regs.h>
#include <pic16f18855.h>

/* Sides
 * =====
 *
 * RJ45
 * ----
 * RJ_SPI_CLK:  RC4
 * RJ_SPI_MOSI: RC5
 * RJ_SPI_CS:   RC6
 * RJ_SPI_MISO: RC7
 *
 * ARM
 * ---
 * data0: RC0
 * data1: RC1
 * data2: RC2
 * data3: RC3
 *
 * Direction:
 * ==========
 * Input = RJ45 to ARM
 * Output = ARM to RJ45
 *
 * Default Mapping
 * ===============
 *
 * channel0 => CLC0 => RJ_SPI_CLK  => data0
 * channel1 => CLC1 => RJ_SPI_MOSI => data1
 * channel2 => CLC2 => RJ_SPI_CS   => data2
 * channel3 => CLC3 => RJ_SPI_MISO => data3
 *
 */

#define RJ45_2_ARM 0
#define ARM_2_RJ45 1

#define CLCS 4

#define SET_BIT(r, b, v) do {r &= ~(1u << b); \
							 r |= v << b; \
							} while(0)

#define SET_VAL_MASK(r, o, m, v) do {r &= ~(m << o); \
									 r |= v << o; \
									} while(0)

#define DIGITAL 0
#define ANALOG 1

#define INPUT 1
#define OUTPUT 0

/* CLC1 Data in selection */
#define LCSEL_MASK 0x3F
#define LCSEL_OFFSET 0x0
#define LCMODE_4_INPUT_AND 0x2
#define CLC1SEL_IN_PPS0 0x0

#define PPSIN_RJ45_CHANNEL1 0x14
#define TRISC_ARM_INDEX_CHANNEL1 0x0
#define TRISC_RJ45_INDEX_CHANNEL1 0x4
#define OUTPIN_ARM_CLC_CHANNEL1 RC0PPS
#define CLCOUT_INDEX_CHANNEL1 0x01
#define PPSIN_ARM_CHANNEL1 0x10
#define OUTPIN_RJ45_CLC_CHANNEL1 RC4PPS
#define ANSELC_OFFSET_RJ45_CHANNEL1 0x4
#define ANSELC_OFFSET_ARM_CHANNEL1 0x0


void setup_clc_passthrough(unsigned char channel, char direction);
void reset_clcs(void);
