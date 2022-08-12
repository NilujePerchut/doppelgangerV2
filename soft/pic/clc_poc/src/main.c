#define NO_BIT_DEFINES
#include <pic14regs.h>
#include <stdint.h>

#define IN 1
#define OUT 0

#define DIGITAL 0
#define ANALOG  1

#define LED_PORT PORTAbits.RA0
#define LED_TRIS TRISAbits.TRISA0
#define LED_ANSEL ANSELAbits.ANSA0

#define IO_SRC_PORT PORTBbits.RB5
#define IO_SRC_TRIS TRISBbits.TRISB5
#define IO_SRC_ANSEL ANSELBbits.ANSB5

#define IO_CLCIN_PORT PORTCbits.RC3
#define IO_CLCIN_TRIS TRISCbits.TRISC3
#define IO_CLCIN_ANSEL ANSELCbits.ANSC3

#define IO_CLCOUT_PORT PORTCbits.RC7
#define IO_CLCOUT_TRIS TRISCbits.TRISC7
#define IO_CLCOUT_ANSEL ANSELCbits.ANSC7

#define CLC_SEL_CLCINPPS0 0x00

static void setup_clc(void)
{
	/* Setup CLC1  to copy input from RB0 to RC7 */

	CLC1CONbits.LC1EN = 0; /* Disable CLC during configuration */
	CLC1CONbits.LC1MODE = 2; /* 4-INPUT AND */
	CLC1POL = 0; /* Polarity untouched */

	/* Sets all inputs to CLCIN0PPS */
	CLC1SEL0 = CLC_SEL_CLCINPPS0;
	CLC1SEL1 = CLC_SEL_CLCINPPS0;
	CLC1SEL2 = CLC_SEL_CLCINPPS0;
	CLC1SEL3 = CLC_SEL_CLCINPPS0;

	IO_CLCIN_ANSEL = DIGITAL;
	IO_CLCIN_TRIS = IN;
	CLCIN0PPS = 0x13; /* Input from RC3 */

	CLC1GLS0 = 0xAA; /* Get all non-inverting inputs */

	RC7PPS = 0x01; /* Set RC7 PPS to CLC1 output */
	IO_CLCOUT_ANSEL = DIGITAL;
	IO_CLCOUT_TRIS = OUT;

	CLC1CONbits.LC1EN = 1; /* Finally enable the CLC block */
}

static void setup(void)
{
	LED_PORT = 0;
	LED_ANSEL = DIGITAL;
	LED_TRIS = OUT;

	IO_SRC_PORT = 0;
	IO_SRC_ANSEL = DIGITAL;
	IO_SRC_TRIS = OUT;

	setup_clc();
}

static void tempo(uint16_t n)
{
	uint16_t i;
	for (i=0;i<n;i++)
		__asm nop __endasm;
}

void main(void)
{
	setup();
	while(1) {
		LED_PORT = 1;
		IO_SRC_PORT = 1;
		tempo(3000);
		LED_PORT = 0;
		IO_SRC_PORT = 0;
		tempo(3000);
	}
}
