
#include "clc.h"


static void setup_clc_passthrough_channel1(unsigned char direction)
{
	/* According to Datasheet chap22.6, programming CLC is performed by
	 * configuring the 12 stages:
	 *	1) Disable CLC
	 *	2) Select desired inputs
	 *	3) Clear any associated ANSEL bits
	 *	4) Set TRIS bit as input for the input pin
	 *	5) Set TRIS bit as ouput for the ouput pin
	 *	6) Enable choosen input through the four gates
	 *	7) Gate ouput polarity
	 *	8) Select the desired logic function
	 *	9) Select ouptut polarity
	 *	10) If ouput id PPS drived, configure output PPS
	 *	11) If interrupts are needed -> setup interrupts
	 *	12) Enables CLC
	 */

	/* 1) Disable CLC during configuration */
	CLC1CONbits.LC1EN = 0;

	/* 2) Data Selection: selects CLC_IN PPS value for first channel */
	CLC1SEL0 = CLC1SEL_IN_PPS0;
	CLC1SEL1 = 0x0;
	CLC1SEL2 = 0x0;
	CLC1SEL3 = 0x0;

	/* 3) Clear Associated ANSEL bits */
	SET_BIT(ANSELC, ANSELC_OFFSET_RJ45_CHANNEL1, DIGITAL);
	SET_BIT(ANSELC, ANSELC_OFFSET_ARM_CHANNEL1, DIGITAL);

	/* 4|5|10) Setup pin configuration */
	switch(direction) {
		case RJ45_2_ARM:
			CLCIN0PPS = PPSIN_RJ45_CHANNEL1;
			SET_BIT(TRISC, TRISC_ARM_INDEX_CHANNEL1, OUTPUT);
			SET_BIT(TRISC, TRISC_RJ45_INDEX_CHANNEL1, INPUT);
			OUTPIN_ARM_CLC_CHANNEL1 = CLCOUT_INDEX_CHANNEL1;
			break;
		case ARM_2_RJ45:
			CLCIN0PPS = PPSIN_ARM_CHANNEL1;
			SET_BIT(TRISC, TRISC_ARM_INDEX_CHANNEL1, INPUT);
			SET_BIT(TRISC, TRISC_RJ45_INDEX_CHANNEL1, OUTPUT);
			OUTPIN_RJ45_CLC_CHANNEL1 = CLCOUT_INDEX_CHANNEL1;
			break;
		default:
			break;
	}
	/* 6) Data Gating: Non inverted CLC gate
	 * First gate will redirect the given input signal.
	 * All other three, will propagate a fixed 1 */
	D1T = 1;	     /* Or */
	CLC1GLS1 = 0;    /* Fixed 0 on all outputs*/
	CLC1GLS2 = 0;    /* Fixed 0 on all outputs*/
	CLC1GLS3 = 0;    /* Fixed 0 on all outputs*/

	/* 7) Gate output polarity */
	G1POL = 0;       /* Non-inverting */
	G2POL = 1;       /* Inverting */
	G3POL = 1;       /* Inverting */
	G4POL = 1;       /* Inverting */

	/* 8) Logic Function: 4-INPUT AND */
	CLC1CONbits.LC1MODE = LCMODE_4_INPUT_AND;

	/* 9) Output Polarity:  untouched */
	CLC1POL = 0;

	/* 12) Finaly enable the CLC block */
	CLC1CONbits.LC1EN = 1;
}

void setup_clc_passthrough(unsigned char channel, unsigned char direction)
{
	switch(channel) {
		case 1:
			setup_clc_passthrough_channel1(direction);
			break;
		default:
			break;
	 }
}

void reset_clcs(void)
{
	CLC1CON = 0;
	CLC1POL = 0;
	CLC1SEL0 = 0;
	CLC1SEL1 = 0;
	CLC1SEL2 = 0;
	CLC1SEL3 = 0;
	CLC1GLS0 = 0;
	CLC1GLS1 = 0;
	CLC1GLS2 = 0;
	CLC1GLS3 = 0;
}
