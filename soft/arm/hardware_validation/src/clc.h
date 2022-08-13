#pragma once

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
 * channel0 => CLC0 => RJ_SPI_CLK  => data0
 * channel1 => CLC1 => RJ_SPI_MOSI => data1
 * channel2 => CLC2 => RJ_SPI_CS   => data2
 * channel3 => CLC3 => RJ_SPI_MISO => data3
 *
 */

#define RJ45_2_ARM 0
#define ARM_2_RJ45 1


#define CLC_PORT GPIOC
#define CLC_CHANNEL_0 RC0
#define CLC_CHANNEL_1 RC1
#define CLC_CHANNEL_2 RC2
#define CLC_CHANNEL_3 RC3
