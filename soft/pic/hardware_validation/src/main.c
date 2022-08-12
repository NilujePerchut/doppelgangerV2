#define NO_LEGACY_NAMES
#define NO_BIT_DEFINES
#include <pic14regs.h>
#include <stdint.h>

#define IN 1
#define OUT 0

#define DIGITAL 0
#define ANALOG  1

#define DBG_IOA_PORT PORTAbits.RA0
#define DBG_IOA_LAT LATAbits.LATA0
#define DBG_IOA_TRIS TRISAbits.TRISA0
#define DBG_IOA_ANSEL ANSELAbits.ANSA0

#define DBG_IOB_PORT PORTAbits.RA1
#define DBG_IOB_LAT LATAbits.LATA1
#define DBG_IOB_TRIS TRISAbits.TRISA1
#define DBG_IOB_ANSEL ANSELAbits.ANSA1

#define DBG_IOC_PORT PORTAbits.RA2
#define DBG_IOC_LAT LATAbits.LATA2
#define DBG_IOC_TRIS TRISAbits.TRISA2
#define DBG_IOC_ANSEL ANSELAbits.ANSA2

#define DBG_IOD_PORT PORTAbits.RA3
#define DBG_IOD_LAT LATAbits.LATA3
#define DBG_IOD_TRIS TRISAbits.TRISA3
#define DBG_IOD_ANSEL ANSELAbits.ANSA3

#define PIC_GPIO1_PORT PORTBbits.RB2
#define PIC_GPIO1_LAT LATBbits.LATB2
#define PIC_GPIO1_TRIS TRISBbits.TRISB2
#define PIC_GPIO1_ANSEL ANSELBbits.ANSB2

#define PIC_GPIO0_PORT PORTBbits.RB3
#define PIC_GPIO0_LAT LATBbits.LATB3
#define PIC_GPIO0_TRIS TRISBbits.TRISB3
#define PIC_GPIO0_ANSEL ANSELBbits.ANSB3

#define SNES_TERM_PORT PORTBbits.RB4
#define SNES_TERM_LAT LATBbits.LATB4
#define SNES_TERM_TRIS TRISBbits.TRISB4
#define SNES_TERM_ANSEL ANSELBbits.ANSB4

#define PSX_TERM_PORT PORTBbits.RB5
#define PSX_TERM_LAT LATBbits.LATB5
#define PSX_TERM_TRIS TRISBbits.TRISB5
#define PSX_TERM_ANSEL ANSELBbits.ANSB5

#define PPS_TXCK_OUTPUT 0x10
#define PPS_RB1_INPUT 0x09

/* Configuration bits */
static __code uint16_t __at (_CONFIG1) configword1 = _FCMEN_OFF & _RSTOSC_HFINT32 & _FEXTOSC_OFF;
static __code uint16_t __at (_CONFIG3) configword3 = _WDTE_OFF;

static void tempo(uint16_t n)
{
	uint16_t i;
	for (i=0;i<n;i++)
		__asm nop __endasm;
}

static void setup_debug_ios(void)
{
	/* Uses Pins
	 *  - DBG_A pin 2(RA0)
	 *  - DBG B pin 3(RA1)
	 *  - DBG C pin 4(RA2)
	 *  - DBG D pin 5(RA3) */
	 /* Put everything in output for now */
	 DBG_IOA_LAT = 0;
	 DBG_IOA_ANSEL = DIGITAL;
	 DBG_IOA_TRIS = OUT;

	 DBG_IOB_LAT = 0;
	 DBG_IOB_ANSEL = DIGITAL;
	 DBG_IOB_TRIS = OUT;

	 DBG_IOC_LAT = 0;
	 DBG_IOC_ANSEL = DIGITAL;
	 DBG_IOC_TRIS = OUT;

	 DBG_IOD_LAT = 0;
	 DBG_IOD_ANSEL = DIGITAL;
	 DBG_IOD_TRIS = OUT;
}

static void setup_term_commands(void)
{
	 SNES_TERM_LAT = 0;
	 SNES_TERM_ANSEL = DIGITAL;
	 SNES_TERM_TRIS = OUT;

	 PSX_TERM_LAT = 0;
	 PSX_TERM_ANSEL = DIGITAL;
	 PSX_TERM_TRIS = OUT;
}

static void setup_uart(void)
{
	/* Setup the serial link with the ARM processor */
	/* Only TX side for now */

	/* Follow procedure of datasheet page 557 (33.1.1.7)  */

	/* Baud rate generator */
	SP1BRGH = 0;
	SP1BRGL = 51;
	TX1STAbits.BRGH = 0;
	BAUD1CONbits.BRG16 = 0;

	/* Enable transmitter */
	TX1STAbits.SYNC = 0;
	TX1STAbits.TXEN = 1;
	RC1STAbits.SPEN = 1;

	/* ATTENTION: bug inside
	 * The RA4 pin in not suitable for USART RX operation
	 * (due to PPS restrictions).
	 * The pin RB1 pin 22 will used instead.
	 * To avoid cutting tracks on the PCB, RA4 (pin 6) will be connected to
	 * RB1 (pin 22). Thus RA4 must be configured as an input.*/
	TRISAbits.TRISA4 = IN;
	ANSELAbits.ANSA4 = DIGITAL;

	TRISBbits.TRISB1 = IN;
	ANSELBbits.ANSB1 = DIGITAL;
	RXPPS = PPS_RB1_INPUT;

	/* Receiver */
	/* ATTENTION: bug inside
	 * The RA5 pin in not suitable for USART TX operation
	 * (due to PPS restrictions).
	 * The pin RB0 pin 21 will used instead.
	 * To avoid cutting tracks on the PCB, RA5 (pin 7) will be connected to
	 * RB0 (pin 21). Thus RA5 must be configured as an input.*/
	TRISAbits.TRISA5 = IN;
	ANSELAbits.ANSA5 = DIGITAL;

	ANSELBbits.ANSB0 = DIGITAL;
	TRISBbits.TRISB0 = OUT;
	RC1STAbits.CREN = 1;
	RB0PPS = PPS_TXCK_OUTPUT;
}

static void setup_pic_gpio(void)
{
	 PIC_GPIO0_LAT = 0;
	 PIC_GPIO0_ANSEL = DIGITAL;
	 PIC_GPIO0_TRIS = OUT;

	 PIC_GPIO1_LAT = 0;
	 PIC_GPIO1_ANSEL = DIGITAL;
	 PIC_GPIO1_TRIS = OUT;
}

static void pic_gpio_pulses(void)
{
	// Test pattern
	PIC_GPIO0_LAT = 0;	
	PIC_GPIO1_LAT = 0;	
	tempo(10000);
	PIC_GPIO0_LAT = 1;	
	PIC_GPIO1_LAT = 0;	
	tempo(60000);
	PIC_GPIO0_LAT = 0;	
	PIC_GPIO1_LAT = 1;	
	tempo(40000);
	PIC_GPIO0_LAT = 1;	
	PIC_GPIO1_LAT = 1;	
	tempo(80000);
	PIC_GPIO0_LAT = 0;	
	PIC_GPIO1_LAT = 0;	
}

static void usart_tx_blocking(unsigned short c)
{
	while (!TX1STAbits.TRMT);
	TX1REG = (unsigned char)c;
	while (!TX1STAbits.TRMT);
}

static unsigned char usart_rx_blocking(void)
{
	while (!PIR3bits.RCIF);
	return RCREG;
}

/* Main goal is to validate the following points:
 * - Serial connection with the ARM microcontoller (DONE)
 * - CLC configurations on demand (via serial commands)
 * - Terminations command (SNES & PSX)
 * - GPIOs from USB mux
*/

void main(void)
{
	char c;

	setup_debug_ios();
	setup_term_commands();
	setup_uart();
	setup_pic_gpio();
	while(1) {
		c = usart_rx_blocking();
		DBG_IOA_LAT = 1;
		switch (c) {
			case 'S':
				/* Switch on Snes terminations */
				SNES_TERM_LAT = 1;
				break;
			case 's':
				/* Switch off Snes terminations */
				SNES_TERM_LAT = 0;
				break;
			case 'P':
				/* Switch on PSX terminations */
				PSX_TERM_LAT = 1;
				break;
			case 'p':
				/* Switch off PSX terminations */
				PSX_TERM_LAT = 0;
				break;
			case 'G':
				/* Send GPIOs pulses */
				pic_gpio_pulses();
			default:
				c = 'u';
				break;
		}
		usart_tx_blocking(c);
		DBG_IOA_LAT = 0;
	}
}
