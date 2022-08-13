#pragma once

#include <math.h>

void init_pic_usart(void);
void init_pic_gpio(void);
void LVP_enter(void);
void LVP_exit(void);
bool flash_pic_from_uart(void);
void pic_debug(void);
void pic_mem_write(unsigned short address, unsigned char data);
unsigned char pic_mem_read(unsigned short address);
void set_pic_gpio(int gpio, int value);
void setup_clc_passthrough_channel1(unsigned char direction);
void setup_clc_passthrough_channel2(unsigned char direction);
void setup_clc_passthrough_channel3(unsigned char direction);
void setup_clc_passthrough_channel4(unsigned char direction);


// Need a wrapper around all base pic macros to ensure full expantion BEFORE call

#define SET_PIC_REG(reg, val) SET_PIC_REG_(reg, val)
#define SET_PIC_REG_(reg, val) pic_mem_write(reg ## _ADDR, val)

#define GET_PIC_REG(reg) GET_PIC_REG_(reg)
#define GET_PIC_REG_(reg) pic_mem_read(reg ## _ADDR)

#define SET_PIC_FIELD(reg, field, val) SET_PIC_FIELD_(reg, field, val)
#define SET_PIC_FIELD_(reg, field, val) do { \
	unsigned char set_pic_mask; \
	unsigned char set_pic_data; \
	set_pic_mask = (unsigned char)(pow(2, (reg ## _ ## field ## _WIDTH)) - 1); \
	set_pic_data = pic_mem_read(reg ## _ADDR); \
	set_pic_data &= ~(set_pic_mask << reg ## _ ## field ## _ ## POS); \
	set_pic_data |= (val & set_pic_mask) << reg ## _ ## field ## _ ##POS; \
	pic_mem_write(reg ## _ADDR, set_pic_data); \
} while(0)

/* CLC Stuff
 *
 * Sides
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
 * Channel  ARM     PIC-ARM    PIC-RJ45
 * 1        PA15    RC0        RC4
 * 2        PC10    RC1        RC5
 * 3        PC11    RC2        RC6
 * 4        PC12    RC3        RC7
 */

#define RJ45_2_ARM 0
#define ARM_2_RJ45 1

#define INPUT 1
#define OUTPUT 0

#define DIGITAL 0
#define ANALOG 1

/* PIC registers
 * Using indirect addressing:
 * +------------------+
 * |1111|10000|0000000|
 * |5432|10987|6543210|
 * +-----+----+-------+
 * |0000| BSR | Baddr |
 * +-----+----+-------+ */

#define PIC_TRISC_ADDR 0x13
#define PIC_TRISC_TRISC0_POS   0
#define PIC_TRISC_TRISC0_WIDTH 1
#define PIC_TRISC_TRISC1_POS   1
#define PIC_TRISC_TRISC1_WIDTH 1
#define PIC_TRISC_TRISC2_POS   2
#define PIC_TRISC_TRISC2_WIDTH 1
#define PIC_TRISC_TRISC3_POS   3
#define PIC_TRISC_TRISC3_WIDTH 1
#define PIC_TRISC_TRISC4_POS   4
#define PIC_TRISC_TRISC4_WIDTH 1
#define PIC_TRISC_TRISC5_POS   5
#define PIC_TRISC_TRISC5_WIDTH 1
#define PIC_TRISC_TRISC6_POS   6
#define PIC_TRISC_TRISC6_WIDTH 1
#define PIC_TRISC_TRISC7_POS   7
#define PIC_TRISC_TRISC7_WIDTH 1

#define PIC_LATB_ADDR 0x17

/* ANSEL_C */
#define PIC_ANSELC_ADDR        0xF4E
#define PIC_ANSELC_ANSC0_POS   0
#define PIC_ANSELC_ANSC0_WIDTH 1
#define PIC_ANSELC_ANSC1_POS   1
#define PIC_ANSELC_ANSC1_WIDTH 1
#define PIC_ANSELC_ANSC2_POS   2
#define PIC_ANSELC_ANSC2_WIDTH 1
#define PIC_ANSELC_ANSC3_POS   3
#define PIC_ANSELC_ANSC3_WIDTH 1
#define PIC_ANSELC_ANSC4_POS   4
#define PIC_ANSELC_ANSC4_WIDTH 1
#define PIC_ANSELC_ANSC5_POS   5
#define PIC_ANSELC_ANSC5_WIDTH 1
#define PIC_ANSELC_ANSC6_POS   6
#define PIC_ANSELC_ANSC6_WIDTH 1
#define PIC_ANSELC_ANSC7_POS   7
#define PIC_ANSELC_ANSC7_WIDTH 1

/* CLC2SELs */
#define PIC_CLC2SEL0_ADDR        0xE1C
#define PIC_CLC2SEL1_ADDR        0xE1D
#define PIC_CLC2SEL2_ADDR        0xE1E
#define PIC_CLC2SEL3_ADDR        0xE1F

/* CLC1CON */
#define PIC_CLC1CON_ADDR          0xE10
#define PIC_CLC1CON_LC1MODE_POS   0
#define PIC_CLC1CON_LC1MODE_WIDTH 3
#define PIC_CLC1CON_LC1EN_POS     7
#define PIC_CLC1CON_LC1EN_WIDTH   1

/* CLC1POL */
#define PIC_CLC1POL_ADDR           0xE11
#define PIC_CLC1POL_LC1G1POL_POS   0
#define PIC_CLC1POL_LC1G1POL_WIDTH 1
#define PIC_CLC1POL_LC1G2POL_POS   1
#define PIC_CLC1POL_LC1G2POL_WIDTH 1
#define PIC_CLC1POL_LC1G3POL_POS   2
#define PIC_CLC1POL_LC1G3POL_WIDTH 1
#define PIC_CLC1POL_LC1G4POL_POS   3
#define PIC_CLC1POL_LC1G4POL_WIDTH 1

/* CLC1SELs */
#define PIC_CLC1SEL0_ADDR        0xE12
#define PIC_CLC1SEL1_ADDR        0xE13
#define PIC_CLC1SEL2_ADDR        0xE14
#define PIC_CLC1SEL3_ADDR        0xE15

/* CLCxGLSs */
#define PIC_CLC1GLS0_ADDR        0xE16
#define PIC_CLC1GLS1_ADDR        0xE17
#define PIC_CLC1GLS2_ADDR        0xE18
#define PIC_CLC1GLS3_ADDR        0xE19
#define PIC_CLC2GLS0_ADDR        0xE20
#define PIC_CLC2GLS1_ADDR        0xE21
#define PIC_CLC2GLS2_ADDR        0xE22
#define PIC_CLC2GLS3_ADDR        0xE23
#define PIC_CLC3GLS0_ADDR        0xE2A
#define PIC_CLC3GLS1_ADDR        0xE2B
#define PIC_CLC3GLS2_ADDR        0xE2C
#define PIC_CLC3GLS3_ADDR        0xE2D
#define PIC_CLC4GLS0_ADDR        0xE34
#define PIC_CLC4GLS1_ADDR        0xE35
#define PIC_CLC4GLS2_ADDR        0xE36
#define PIC_CLC4GLS3_ADDR        0xE37

/* CLC2CON */
#define PIC_CLC2CON_ADDR          0xE1A
#define PIC_CLC2CON_LC2MODE_POS   0
#define PIC_CLC2CON_LC2MODE_WIDTH 3
#define PIC_CLC2CON_LC2EN_POS     7
#define PIC_CLC2CON_LC2EN_WIDTH   1

/* CLC2POL */
#define PIC_CLC2POL_ADDR           0xE1B
#define PIC_CLC2POL_LC2G1POL_POS   0
#define PIC_CLC2POL_LC2G1POL_WIDTH 1
#define PIC_CLC2POL_LC2G2POL_POS   1
#define PIC_CLC2POL_LC2G2POL_WIDTH 1
#define PIC_CLC2POL_LC2G3POL_POS   2
#define PIC_CLC2POL_LC2G3POL_WIDTH 1
#define PIC_CLC2POL_LC2G4POL_POS   3
#define PIC_CLC2POL_LC2G4POL_WIDTH 1

/* CLC2CON */
#define PIC_CLC3CON_ADDR          0xE24
#define PIC_CLC3CON_LC3MODE_POS   0
#define PIC_CLC3CON_LC3MODE_WIDTH 3
#define PIC_CLC3CON_LC3EN_POS     7
#define PIC_CLC3CON_LC3EN_WIDTH   1

/* CLC3POL */
#define PIC_CLC3POL_ADDR           0xE25
#define PIC_CLC3POL_LC3G1POL_POS   0
#define PIC_CLC3POL_LC3G1POL_WIDTH 1
#define PIC_CLC3POL_LC3G2POL_POS   1
#define PIC_CLC3POL_LC3G2POL_WIDTH 1
#define PIC_CLC3POL_LC3G3POL_POS   2
#define PIC_CLC3POL_LC3G3POL_WIDTH 1
#define PIC_CLC3POL_LC3G4POL_POS   3
#define PIC_CLC3POL_LC3G4POL_WIDTH 1

/* CLC3SELs */
#define PIC_CLC3SEL0_ADDR        0xE26
#define PIC_CLC3SEL1_ADDR        0xE27
#define PIC_CLC3SEL2_ADDR        0xE28
#define PIC_CLC3SEL3_ADDR        0xE29

/* CLC4CON */
#define PIC_CLC4CON_ADDR          0xE2E
#define PIC_CLC4CON_LC4MODE_POS   0
#define PIC_CLC4CON_LC4MODE_WIDTH 3
#define PIC_CLC4CON_LC4EN_POS     7
#define PIC_CLC4CON_LC4EN_WIDTH   1

/* CLC4POL */
#define PIC_CLC4POL_ADDR           0xE2F
#define PIC_CLC4POL_LC4G1POL_POS   0
#define PIC_CLC4POL_LC4G1POL_WIDTH 1
#define PIC_CLC4POL_LC4G2POL_POS   1
#define PIC_CLC4POL_LC4G2POL_WIDTH 1
#define PIC_CLC4POL_LC4G3POL_POS   2
#define PIC_CLC4POL_LC4G3POL_WIDTH 1
#define PIC_CLC4POL_LC4G4POL_POS   3
#define PIC_CLC4POL_LC4G4POL_WIDTH 1

/* CLC4SELs */
#define PIC_CLC4SEL0_ADDR        0xE30
#define PIC_CLC4SEL1_ADDR        0xE31
#define PIC_CLC4SEL2_ADDR        0xE32
#define PIC_CLC4SEL3_ADDR        0xE33

/* CLCxINPPS */
#define PIC_CLCIN0PPS_ADDR       0xEBB
#define PIC_CLCIN1PPS_ADDR       0xEBC
#define PIC_CLCIN2PPS_ADDR       0xEBD
#define PIC_CLCIN3PPS_ADDR       0xEBE

/* RCxPPS */
#define PIC_RC0PPS_ADDR          0xF20
#define PIC_RC1PPS_ADDR          0xF21
#define PIC_RC2PPS_ADDR          0xF22
#define PIC_RC3PPS_ADDR          0xF23
#define PIC_RC4PPS_ADDR          0xF24
#define PIC_RC5PPS_ADDR          0xF25
#define PIC_RC6PPS_ADDR          0xF26
#define PIC_RC7PPS_ADDR          0xF27

// CLC defines
#define PIC_CLC1SEL_IN_PPS0         0x0
#define PIC_CLC2SEL_IN_PPS0         0x1
#define PIC_CLC3SEL_IN_PPS0         0x2
#define PIC_CLC4SEL_IN_PPS0         0x3
#define ANSELC_RJ45_CHANNEL1        ANSC4
#define ANSELC_RJ45_CHANNEL2        ANSC5
#define ANSELC_RJ45_CHANNEL3        ANSC6
#define ANSELC_RJ45_CHANNEL4        ANSC7
#define ANSELC_ARM_CHANNEL1         ANSC0
#define ANSELC_ARM_CHANNEL2         ANSC1
#define ANSELC_ARM_CHANNEL3         ANSC2
#define ANSELC_ARM_CHANNEL4         ANSC3
#define PPSIN_RJ45_CHANNEL1         0x14
#define PPSIN_RJ45_CHANNEL2         0x15
#define PPSIN_RJ45_CHANNEL3         0x16
#define PPSIN_RJ45_CHANNEL4         0x17
#define PPSIN_ARM_CHANNEL1          0x10
#define PPSIN_ARM_CHANNEL2          0x11
#define PPSIN_ARM_CHANNEL3          0x12
#define PPSIN_ARM_CHANNEL4          0x13
#define TRISC_ARM_INDEX_CHANNEL1    TRISC0
#define TRISC_ARM_INDEX_CHANNEL2    TRISC1
#define TRISC_ARM_INDEX_CHANNEL3    TRISC2
#define TRISC_ARM_INDEX_CHANNEL4    TRISC3
#define TRISC_RJ45_INDEX_CHANNEL1   TRISC4
#define TRISC_RJ45_INDEX_CHANNEL2   TRISC5
#define TRISC_RJ45_INDEX_CHANNEL3   TRISC6
#define TRISC_RJ45_INDEX_CHANNEL4   TRISC7
#define OUTPIN_ARM_CLC_CHANNEL1     PIC_RC0PPS
#define OUTPIN_ARM_CLC_CHANNEL2     PIC_RC1PPS
#define OUTPIN_ARM_CLC_CHANNEL3     PIC_RC2PPS
#define OUTPIN_ARM_CLC_CHANNEL4     PIC_RC3PPS
#define CLCOUT_INDEX_CHANNEL1       0x01
#define CLCOUT_INDEX_CHANNEL2       0x02
#define CLCOUT_INDEX_CHANNEL3       0x03
#define CLCOUT_INDEX_CHANNEL4       0x04
#define OUTPIN_RJ45_CLC_CHANNEL1    PIC_RC4PPS
#define OUTPIN_RJ45_CLC_CHANNEL2    PIC_RC5PPS
#define OUTPIN_RJ45_CLC_CHANNEL3    PIC_RC6PPS
#define OUTPIN_RJ45_CLC_CHANNEL4    PIC_RC7PPS
#define CLC_GATE_ONLY_FIRST_CHANNEL 0x02
#define CLC_GATE_POL_INVERTING      1
#define CLC_GATE_POL_NON_INVERTING  0
#define LCMODE_4_INPUT_AND          0x2
