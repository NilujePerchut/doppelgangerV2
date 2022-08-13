#include <string.h>  // for memset
#include <stdio.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include "doppelganger.h"
#include "pic_iface.h"
#include "systick.h"

#define  CMD_LOAD_ADDRESS     0x80
#define  CMD_LATCH_DATA       0x00
#define  CMD_LATCH_DATA_IA    0x02
#define  CMD_INC_ADDR         0xF8
#define  CMD_BEGIN_PROG       0xE0
#define  CMD_BULK_ERASE       0x18

void init_pic_usart(void)
{
	rcc_periph_clock_enable(PIC_USART_RCC);

	/* Configure GPIO as alternate function */
	rcc_periph_clock_enable(PIC_USART_PORT_RCC);
	gpio_mode_setup(PIC_USART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE,
					PIC_USART_TX_GPIO|PIC_USART_RX_GPIO);
	gpio_set_af(PIC_USART_PORT, GPIO_AF7, PIC_USART_TX_GPIO|PIC_USART_RX_GPIO);

	usart_set_baudrate(PIC_USART, 9600);
	usart_set_databits(PIC_USART, 8);
	usart_set_stopbits(PIC_USART, USART_STOPBITS_1);
	usart_set_mode(PIC_USART, USART_MODE_TX_RX);
	usart_set_parity(PIC_USART, USART_PARITY_NONE);
	usart_set_flow_control(PIC_USART, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(PIC_USART);
}

void init_pic_gpio(void)
{
	rcc_periph_clock_enable(PIC_RCC);
	gpio_mode_setup(PIC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
					PIC_RESET | PIC_BOOT | PIC_ICSP_CLK | PIC_ICSP_DATA);
}

void set_pic_reset(int state)
{
	if (state)
		gpio_set(PIC_PORT, PIC_RESET);
	else
		gpio_clear(PIC_PORT, PIC_RESET);
}


/* ICSP programming */

/* LVP Stuff */

void LVP_init()
{
	rcc_periph_clock_enable(PIC_RCC);
	gpio_mode_setup(PIC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, PIC_ICSP_DATA);

	gpio_clear(PIC_PORT, PIC_ICSP_CLK);
	gpio_mode_setup(PIC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIC_ICSP_CLK);

	/* Reset is inverted clear -> 1L, set -> 0L */
	gpio_set(PIC_PORT, PIC_RESET);
	gpio_mode_setup(PIC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIC_RESET);
}

void LVP_release(void)
{
	gpio_mode_setup(PIC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
					PIC_ICSP_DATA | PIC_ICSP_CLK);

	gpio_clear(PIC_PORT, PIC_RESET); /* SLAVE_RUN = 0 on original code */
	gpio_mode_setup(PIC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIC_RESET);
}

void sendCmd(uint8_t b)
{
	uint8_t i;

	gpio_mode_setup(PIC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIC_ICSP_DATA);

	for (i=0; i<8; i++) {
		if (b & 0x80)
			gpio_set(PIC_PORT, PIC_ICSP_DATA);
		else
			gpio_clear(PIC_PORT, PIC_ICSP_DATA);

		gpio_set(PIC_PORT, PIC_ICSP_CLK);
		b <<= 1;
		delay_us(1);
		gpio_clear(PIC_PORT, PIC_ICSP_CLK);
		delay_us(1);
	}
	delay_us(1);
}

void sendData(uint16_t data)
{
	uint8_t i;
	uint32_t w = (uint32_t)data;

	w = (w << 1) & 0x7FFFFE; /* Start and stop bit */
	gpio_mode_setup(PIC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIC_ICSP_DATA);
	for (i=0; i<24; i++) {
		if ((w & 0x800000) > 0)/* MSB first */
			gpio_set(PIC_PORT, PIC_ICSP_DATA);
		else
			gpio_clear(PIC_PORT, PIC_ICSP_DATA);
		gpio_set(PIC_PORT, PIC_ICSP_CLK);
		w <<= 1;
		delay_us(1);
		gpio_clear(PIC_PORT, PIC_ICSP_CLK);
		delay_us(1);
	}
}

uint8_t getByte(void)
{
	uint8_t i;
	uint8_t b;

	gpio_mode_setup(PIC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, PIC_ICSP_DATA);
	for (i=0; i<8; i++) {
		gpio_set(PIC_PORT, PIC_ICSP_CLK);
		b <<= 1;
		delay_us(1);
		b |= gpio_get(PIC_PORT, PIC_ICSP_DATA)?1:0;
		gpio_clear(PIC_PORT, PIC_ICSP_CLK);
		delay_us(1);
	}
	return b;
}

uint16_t getData(void)
{
	uint8_t i;
	uint16_t w = 0;

	gpio_mode_setup(PIC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, PIC_ICSP_DATA);
	for (i=0; i<24; i++) {
		gpio_set(PIC_PORT, PIC_ICSP_CLK);
		w <<= 1;
		delay_us(1);
		w |= gpio_get(PIC_PORT, PIC_ICSP_DATA)?1:0;
		gpio_clear(PIC_PORT, PIC_ICSP_CLK);
		delay_us(1);
	}
	return (w >> 1);
}

void LVP_enter(void)
{
	LVP_init();
	gpio_set(PIC_PORT, PIC_RESET);
	delay_ms(10);
	sendCmd('M');
	sendCmd('C');
	sendCmd('H');
	sendCmd('P');
	delay_ms(5);
}

void LVP_exit(void)
{
	LVP_release();
}

bool LVP_inProgress(void)
{
	return (gpio_get(PIC_PORT, PIC_RESET)?0:1) == 0;
}

void LVP_bulkErase(void)
{
	sendCmd(CMD_LOAD_ADDRESS);
	sendData(0x8000);
	sendCmd(CMD_BULK_ERASE);
	delay_ms(6);
}

void LVP_skip(uint16_t count)
{
	while(count-- > 0)
		sendCmd(CMD_INC_ADDR);
}

void LVP_addressLoad(uint16_t address)
{
	sendCmd(CMD_LOAD_ADDRESS);
	sendData(address);
}

void LVP_rowWrite(uint16_t *buffer, uint8_t w)
{
	for(; w>1; w--) {
		sendCmd(CMD_LATCH_DATA_IA);
		sendData(*buffer++);
	}
	sendCmd(CMD_LATCH_DATA);
	sendData(*buffer++);
	sendCmd(CMD_BEGIN_PROG);
	delay_ms(3);
	sendCmd(CMD_INC_ADDR);
}

void LVP_cfgWrite(uint16_t *cfg, uint8_t count)
{
	sendCmd(CMD_LOAD_ADDRESS);
	sendData(0x8007);
	while(count-- > 0) {
		sendCmd(CMD_LATCH_DATA);
		sendData(*cfg++);
		sendCmd(CMD_BEGIN_PROG);
		/* delay_ms(3) is just below 3ms on Doppelganger board
		 * switch to 4ms to ensure correct configuration bits flashing*/
		delay_ms(4);
		sendCmd(CMD_INC_ADDR);
	}
	sendCmd(CMD_LOAD_ADDRESS);
	sendData(0x0000);
}

/* Hex parsing */
/*******************************************************************************
 Direct Hex File Parsing and Programming State Machine

 This is a simple state machine that parses an input stream to detect and decode
 the INTEL Hex file format produced by the MPLAB XC8 compiler
 Bytes are assembled in Words
 Words are assembled in Rows (currently supporting fixed size of 32-words)
 Rows are aligned (normalized) and written directly to the target using LVP ICSP
 Special treatment is reserved for words written to 'configuration' addresses
 ******************************************************************************/
#define ROW_SIZE     32      // for all pic16f188xx
#define CFG_ADDRESS 0x8000   // for all pic16f188xx
#define CFG_NUM      5       // number of config words for PIC16F188xx

// internal state
uint16_t row[ROW_SIZE];    // buffer containing row being formed
uint32_t row_address;       // destination address of current row
bool     lvp;               // flag: low voltage programming in progress

/*
 * State machine initialization
 */
void DIRECT_Initialize(void)
{
	memset((void*)row, 0xff, sizeof(row));    // fill buffer with blanks
	row_address = 0x8000;
	lvp = false;
	LVP_init();
}

/**
 * Test if Low Voltage - In Circuit Serial Programming in progress
 * @return  true if lvp sequence in progress
 */
bool DIRECT_ProgrammingInProgress(void)
{
	return lvp;
}

bool isDigit(char *c)
{
	if (*c < '0')
		return false;
	*c -= '0';
	if (*c > 9)
		*c-=7;
	if (*c > 0xf)
		return false;
	return true;
}

void lvpWrite(void)
{
	// check for first entry in lvp
	if (!lvp) {
		lvp = true;
		LVP_enter();
		LVP_bulkErase();
	}
	if (row_address >= CFG_ADDRESS) {
		// use the special cfg word sequence
		LVP_cfgWrite(&row[7], CFG_NUM);
	} else {
		// normal row programming sequence
		LVP_addressLoad(row_address);
		LVP_rowWrite(row, ROW_SIZE);
    }
}

void writeRow(void)
{
	// latch and program a row, skip if blank
	uint8_t i;
	uint16_t chk = 0xffff;

	for(i=0; i<ROW_SIZE; i++) {
		// blank check
		chk &= row[i];
	}
	if (chk != 0xffff) {
		lvpWrite();
		// fill buffer with blanks
		memset((void*)row, 0xff, sizeof(row));
	}
}

/**
 * Align and pack words in rows, ready for lvp programming
 * @param address       starting address
 * @param data          buffer
 * @param data_count    number of bytes
 */
void packRow(uint32_t address, uint8_t *data, uint8_t data_count)
{
	/* copy only the bytes from the current data packet up
	to the boundary of a row */
	uint8_t  index = (address & 0x3e)>>1;
	uint32_t new_row = (address & 0xfffc0)>>1;

	if (new_row != row_address) {
		writeRow();
		row_address = new_row;
	}

	// ensure data is always even (rounding up)
	data_count = (data_count+1) & 0xfe;

	// copy data up to the row boundaries
	while ((data_count > 0) && (index < ROW_SIZE)){
		uint16_t word = *data++;
		word += ((uint16_t)(*data++)<<8);
		row[index++] = word;
		data_count -= 2;
	}

	// if a complete row was filled, proceed to programming
	if (index == ROW_SIZE) {
		writeRow();
		// next consider the split row scenario
		if (data_count > 0) {   // leftover must spill into next row
			row_address += ROW_SIZE;
			index = 0;
			while (data_count > 0){
				uint16_t word = *data++;
				word += ((uint16_t)(*data++)<<8);
				row[index++] = word;
				data_count -= 2;
			}
		}
	}
}

void programLastRow(void)
{
	writeRow();
	LVP_exit();
	lvp = false;
}

// the actual state machine - Hex Machina
enum hexstate {SOL, BYTE_COUNT, ADDRESS, RECORD_TYPE, DATA, CHKSUM};

/**
 * Parser, main state machine decoding engine
 *
 * @param c     input character
 * @return      true = success, false = decoding failure/invalid file contents
 */
bool ParseHex(char c)
{
	static enum hexstate state = SOL;
	static uint8_t  bc;
	static uint8_t  data_count;
	static uint32_t address;
	static uint32_t ext_address = 0;
	static uint8_t  checksum;
	static uint8_t  record_type;
	static uint8_t  data_byte, data_index, data[16];

	switch(state){
		case SOL:
			if (c == '\r')
				break;
			if (c == '\n')
				break;
			if (c != ':')
				return false;

			state = BYTE_COUNT;
			bc = 0;
			address = 0;
			checksum = 0;
			break;

		case BYTE_COUNT:
			if (isDigit(&c) == false) {
				state = SOL;
				return false;
			}
			bc++;
			if (bc == 1)
				data_count = c;
			if (bc == 2 )  {
				data_count = (data_count << 4) + c;
				checksum += data_count;
				bc = 0;
				if (data_count > 16){
					state = SOL;
					return false;
				}
				state = ADDRESS;
			}
			break;

		case ADDRESS:
			if (isDigit(&c) == false) {
				state = SOL;
				return false;
			}
			bc++;
			if (bc == 1)
				address = c;
			else  {
				address = (address << 4) + (uint32_t)c;
				if (bc == 4) {
					checksum += (address>>8) + address;
					bc = 0;
					state = RECORD_TYPE;
				}
			}
			break;

		case RECORD_TYPE:
			if (isDigit( &c) == false){
				state = SOL;
				return false;
			}
			bc++;
			if (bc == 1)
				if (c != 0){
					state = SOL;
					return false;
				}
			if (bc == 2){
				record_type = c;
				checksum += c;
				bc = 0;
				state = DATA;  // default
				data_index = 0;
				memset(data, 0xff, sizeof(data));
				if (record_type == 0)
					break; // data record
				if (record_type == 1) {
					// EOF record
					state = CHKSUM;
					break;
				}
				if (record_type == 4)
					break; // extended address record
				state = SOL;
				return false;
			}
			break;

		case DATA:
			if (isDigit(&c) == false){
				state = SOL;
				return false;
			}
			bc++;
			if (bc == 1)
				data[data_index] = (c<<4);
			if (bc == 2){
				bc = 0;
				data[data_index] += c;
				checksum +=  data[data_index];
				data_index++;
				if (data_index == data_count) {
					state = CHKSUM;
				}
			}
			break;

		case CHKSUM:
			if (isDigit( &c) == false) {
				state = SOL;
				return false;
			}
			bc++;
			if (bc == 1)
				checksum += (c<<4);
			if (bc == 2){
				bc = 0;
				checksum += c;
				if (checksum != 0) {
					state = SOL;
					return false;
				}
				// chksum is good
				state = SOL;
				if (record_type == 0)
					packRow(ext_address + address, data, data_count);
				else if (record_type == 4)
					ext_address = ((uint32_t)(data[0]) << 24) + \
						          ((uint32_t)(data[1]) << 16);
				else if (record_type == 1){
					programLastRow();
					ext_address = 0;
				} else
					return false;
			}
			break;

		default:
			break;
	}
	return true;
}

#define FLASH_MAGIC "FLASH"
#define FLASH_MAGIC_LEN (sizeof(FLASH_MAGIC)-1)

bool flash_pic_from_uart(void)
{
	/* Protocol is:
		1) MAGIC (exclamation mark not tested): FLASH!
		2) 8-digit size (32bits)
		3) the hex file */
	char c;
	uint8_t i;
	uint8_t magic_index;
	uint32_t hex_len;
	char hex_file[4000];

	/* Asumes printf/puts is initialized */
	printf("PIC programming start\n");

	/* Wait for Magic */
	magic_index = 0;
	c = usart_recv_blocking(DBG_USART);
	while(magic_index < FLASH_MAGIC_LEN) {
		if (c == FLASH_MAGIC[magic_index])
			magic_index++;
		else
			if (c == FLASH_MAGIC[0])
				magic_index = 1;
			else
				magic_index = 0;
		c = usart_recv_blocking(DBG_USART);
	}
	delay_ms(3000);
	puts("Magic validated\n");


	/* Wait for the hex len (in char) MSB first*/
	hex_len = 0;
	for (i=0;i<8;i++) {
		c = usart_recv_blocking(DBG_USART);
		isDigit(&c);
		hex_len |= c<<(8-i-1)*4;
	}
	delay_ms(3000);
	printf("Found len: %u\n", hex_len);

	for (uint32_t i=0;i<hex_len;i++)
		hex_file[i] = usart_recv_blocking(DBG_USART);

	DIRECT_Initialize();

	for (uint32_t i=0;i<hex_len;i++) {
		if (!ParseHex(hex_file[i])) {
			printf("Hex flashing failed at char %d\n", i);
			return false;
		}
	}
	puts("Hex firmware flashed into pic\n");
	return true;
}

void pic_debug(void)
{
	char c;

	while(1) {
		usart_send_blocking(PIC_USART, 'C');
		c = usart_recv_blocking(PIC_USART);
		printf("Recv: %c\n", c);
		delay_ms(500);
	}
}


void pic_mem_write(unsigned short address, unsigned char data)
{
	usart_send_blocking(PIC_USART, 'W');
	usart_send_blocking(PIC_USART, address&0xFF);
	usart_send_blocking(PIC_USART, (address>>8)&0xFF);
	usart_send_blocking(PIC_USART, data);
	usart_recv_blocking(PIC_USART); /* Wait for the Ack */
}


unsigned char pic_mem_read(unsigned short address)
{
	unsigned char data;
	usart_send_blocking(PIC_USART, 'R');
	usart_send_blocking(PIC_USART, address&0xFF);
	usart_send_blocking(PIC_USART, (address>>8)&0xFF);
	data = usart_recv_blocking(PIC_USART);
	usart_recv_blocking(PIC_USART); /* Wait for the Ack */
	return data;
}


void set_pic_gpio(int gpio, int value)
{
	// GPIO0 -> RB3
	// GPIO1 -> RB2

	unsigned char cur;
	const unsigned char gpio_offset[] = {3, 2};

	cur = GET_PIC_REG(PIC_LATB);

	cur &= ~(1<<gpio_offset[gpio]);
	cur |= value << gpio_offset[gpio];

	SET_PIC_REG(PIC_LATB, cur);
}


void setup_clc_passthrough_channel1(unsigned char direction)
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
	SET_PIC_FIELD(PIC_CLC1CON, LC1EN, 0);

	/* 2) Data Selection: selects CLC_IN PPS value for first channel */
	SET_PIC_REG(PIC_CLC1SEL0, PIC_CLC1SEL_IN_PPS0);
	SET_PIC_REG(PIC_CLC1SEL1, 0x0);
	SET_PIC_REG(PIC_CLC1SEL2, 0x0);
	SET_PIC_REG(PIC_CLC1SEL3, 0x0);

	/* 3) Clear Associated ANSEL bits */
	SET_PIC_FIELD(PIC_ANSELC, ANSELC_RJ45_CHANNEL1, DIGITAL);
	SET_PIC_FIELD(PIC_ANSELC, ANSELC_ARM_CHANNEL1, DIGITAL);

	/* 4|5|10) Setup pin configuration */
	switch(direction) {
		case RJ45_2_ARM:
			puts("Channel 1: RJ45 -> ARM\n");
			SET_PIC_REG(PIC_CLCIN0PPS, PPSIN_RJ45_CHANNEL1);
			SET_PIC_FIELD(PIC_TRISC, TRISC_ARM_INDEX_CHANNEL1, OUTPUT);
			SET_PIC_FIELD(PIC_TRISC, TRISC_RJ45_INDEX_CHANNEL1, INPUT);
			SET_PIC_REG(OUTPIN_ARM_CLC_CHANNEL1, CLCOUT_INDEX_CHANNEL1);
			break;
		case ARM_2_RJ45:
			puts("Channel 1: ARM -> RJ45\n");
			SET_PIC_REG(PIC_CLCIN0PPS, PPSIN_ARM_CHANNEL1);
			SET_PIC_FIELD(PIC_TRISC, TRISC_ARM_INDEX_CHANNEL1, INPUT);
			SET_PIC_FIELD(PIC_TRISC, TRISC_RJ45_INDEX_CHANNEL1, OUTPUT);
			SET_PIC_REG(OUTPIN_RJ45_CLC_CHANNEL1, CLCOUT_INDEX_CHANNEL1);
			break;
		default:
			puts("Warning unknown dir\n");
			break;
	}
	unsigned char tmp = GET_PIC_REG(PIC_TRISC);
	printf("\ttrisc = 0x{%02x}\n", tmp);

	/* 6) Data Gating: Non inverted CLC gate
	 * First gate will redirect the given input signal.
	 * All other three, will propagate a fixed 1 */
	SET_PIC_REG(PIC_CLC1GLS0, CLC_GATE_ONLY_FIRST_CHANNEL);
	SET_PIC_REG(PIC_CLC1GLS1, 0);    /* Fixed 0 on all outputs*/
	SET_PIC_REG(PIC_CLC1GLS2, 0);    /* Fixed 0 on all outputs*/
	SET_PIC_REG(PIC_CLC1GLS3, 0);    /* Fixed 0 on all outputs*/

	/* 7) Gate output polarity */
	SET_PIC_FIELD(PIC_CLC1POL, LC1G1POL, 0);  /* Non-inverting */
	SET_PIC_FIELD(PIC_CLC1POL, LC1G2POL, 1);  /* Inverting */
	SET_PIC_FIELD(PIC_CLC1POL, LC1G3POL, 1);  /* Inverting */
	SET_PIC_FIELD(PIC_CLC1POL, LC1G4POL, 1);  /* Inverting */

	/* 8) Logic Function: 4-INPUT AND */
	SET_PIC_FIELD(PIC_CLC1CON, LC1MODE, LCMODE_4_INPUT_AND);

	/* 11) Interrupts */
	/* Not needed */

	/* 12) Finaly enable the CLC block */
	SET_PIC_FIELD(PIC_CLC1CON, LC1EN, 1);
}

void setup_clc_passthrough_channel2(unsigned char direction)
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
	SET_PIC_FIELD(PIC_CLC2CON, LC2EN, 0);

	/* 2) Data Selection: selects CLC_IN PPS value for first channel */
	SET_PIC_REG(PIC_CLC2SEL0, PIC_CLC2SEL_IN_PPS0);
	SET_PIC_REG(PIC_CLC2SEL1, 0x0);
	SET_PIC_REG(PIC_CLC2SEL2, 0x0);
	SET_PIC_REG(PIC_CLC2SEL3, 0x0);

	/* 3) Clear Associated ANSEL bits */
	SET_PIC_FIELD(PIC_ANSELC, ANSELC_RJ45_CHANNEL2, DIGITAL);
	SET_PIC_FIELD(PIC_ANSELC, ANSELC_ARM_CHANNEL2, DIGITAL);

	/* 4|5|10) Setup pin configuration */
	switch(direction) {
		case RJ45_2_ARM:
			puts("Channel 2: RJ45 -> ARM\n");
			SET_PIC_REG(PIC_CLCIN1PPS, PPSIN_RJ45_CHANNEL2);
			SET_PIC_FIELD(PIC_TRISC, TRISC_ARM_INDEX_CHANNEL2, OUTPUT);
			SET_PIC_FIELD(PIC_TRISC, TRISC_RJ45_INDEX_CHANNEL2, INPUT);
			SET_PIC_REG(OUTPIN_ARM_CLC_CHANNEL2, CLCOUT_INDEX_CHANNEL2);
			break;
		case ARM_2_RJ45:
			puts("Channel 2: ARM -> RJ45\n");
			SET_PIC_REG(PIC_CLCIN1PPS, PPSIN_ARM_CHANNEL2);
			SET_PIC_FIELD(PIC_TRISC, TRISC_ARM_INDEX_CHANNEL2, INPUT);
			SET_PIC_FIELD(PIC_TRISC, TRISC_RJ45_INDEX_CHANNEL2, OUTPUT);
			SET_PIC_REG(OUTPIN_RJ45_CLC_CHANNEL2, CLCOUT_INDEX_CHANNEL2);
			break;
		default:
			puts("Warning unknown dir\n");
			break;
	}
	unsigned char tmp = GET_PIC_REG(PIC_TRISC);
	printf("\ttrisc = 0x{%02x}\n", tmp);

	/* 6) Data Gating: Non inverted CLC gate
	 * First gate will redirect the given input signal.
	 * All other three, will propagate a fixed 1 */
	SET_PIC_REG(PIC_CLC2GLS0, CLC_GATE_ONLY_FIRST_CHANNEL);
	SET_PIC_REG(PIC_CLC2GLS1, 0);    /* Fixed 0 on all outputs*/
	SET_PIC_REG(PIC_CLC2GLS2, 0);    /* Fixed 0 on all outputs*/
	SET_PIC_REG(PIC_CLC2GLS3, 0);    /* Fixed 0 on all outputs*/

	/* 7) Gate output polarity */
	SET_PIC_FIELD(PIC_CLC2POL, LC2G1POL, 0);  /* Non-inverting */
	SET_PIC_FIELD(PIC_CLC2POL, LC2G2POL, 1);  /* Inverting */
	SET_PIC_FIELD(PIC_CLC2POL, LC2G3POL, 1);  /* Inverting */
	SET_PIC_FIELD(PIC_CLC2POL, LC2G4POL, 1);  /* Inverting */

	/* 8) Logic Function: 4-INPUT AND */
	SET_PIC_FIELD(PIC_CLC2CON, LC2MODE, LCMODE_4_INPUT_AND);

	/* 11) Interrupts */
	/* Not needed */

	/* 12) Finaly enable the CLC block */
	SET_PIC_FIELD(PIC_CLC2CON, LC2EN, 1);
}

void setup_clc_passthrough_channel3(unsigned char direction)
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
	SET_PIC_FIELD(PIC_CLC3CON, LC3EN, 0);

	/* 2) Data Selection: selects CLC_IN PPS value for first channel */
	SET_PIC_REG(PIC_CLC3SEL0, PIC_CLC3SEL_IN_PPS0);
	SET_PIC_REG(PIC_CLC3SEL1, 0x0);
	SET_PIC_REG(PIC_CLC3SEL2, 0x0);
	SET_PIC_REG(PIC_CLC3SEL3, 0x0);

	/* 3) Clear Associated ANSEL bits */
	SET_PIC_FIELD(PIC_ANSELC, ANSELC_RJ45_CHANNEL3, DIGITAL);
	SET_PIC_FIELD(PIC_ANSELC, ANSELC_ARM_CHANNEL3, DIGITAL);

	/* 4|5|10) Setup pin configuration */
	switch(direction) {
		case RJ45_2_ARM:
			puts("Channel 3: RJ45 -> ARM\n");
			SET_PIC_REG(PIC_CLCIN2PPS, PPSIN_RJ45_CHANNEL3);
			SET_PIC_FIELD(PIC_TRISC, TRISC_ARM_INDEX_CHANNEL3, OUTPUT);
			SET_PIC_FIELD(PIC_TRISC, TRISC_RJ45_INDEX_CHANNEL3, INPUT);
			SET_PIC_REG(OUTPIN_ARM_CLC_CHANNEL3, CLCOUT_INDEX_CHANNEL3);
			break;
		case ARM_2_RJ45:
			puts("Channel 3: ARM -> RJ45\n");
			SET_PIC_REG(PIC_CLCIN2PPS, PPSIN_ARM_CHANNEL3);
			SET_PIC_FIELD(PIC_TRISC, TRISC_ARM_INDEX_CHANNEL3, INPUT);
			SET_PIC_FIELD(PIC_TRISC, TRISC_RJ45_INDEX_CHANNEL3, OUTPUT);
			SET_PIC_REG(OUTPIN_RJ45_CLC_CHANNEL3, CLCOUT_INDEX_CHANNEL3);
			break;
		default:
			puts("Warning unknown dir\n");
			break;
	}
	unsigned char tmp = GET_PIC_REG(PIC_TRISC);
	printf("\ttrisc = 0x{%02x}\n", tmp);

	/* 6) Data Gating: Non inverted CLC gate
	 * First gate will redirect the given input signal.
	 * All other three, will propagate a fixed 1 */
	SET_PIC_REG(PIC_CLC3GLS0, CLC_GATE_ONLY_FIRST_CHANNEL);
	SET_PIC_REG(PIC_CLC3GLS1, 0);    /* Fixed 0 on all outputs*/
	SET_PIC_REG(PIC_CLC3GLS2, 0);    /* Fixed 0 on all outputs*/
	SET_PIC_REG(PIC_CLC3GLS3, 0);    /* Fixed 0 on all outputs*/

	/* 7) Gate output polarity */
	SET_PIC_FIELD(PIC_CLC3POL, LC3G1POL, 0);  /* Non-inverting */
	SET_PIC_FIELD(PIC_CLC3POL, LC3G2POL, 1);  /* Inverting */
	SET_PIC_FIELD(PIC_CLC3POL, LC3G3POL, 1);  /* Inverting */
	SET_PIC_FIELD(PIC_CLC3POL, LC3G4POL, 1);  /* Inverting */

	/* 8) Logic Function: 4-INPUT AND */
	SET_PIC_FIELD(PIC_CLC3CON, LC3MODE, LCMODE_4_INPUT_AND);

	/* 11) Interrupts */
	/* Not needed */

	/* 12) Finaly enable the CLC block */
	SET_PIC_FIELD(PIC_CLC3CON, LC3EN, 1);
}

void setup_clc_passthrough_channel4(unsigned char direction)
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
	SET_PIC_FIELD(PIC_CLC4CON, LC4EN, 0);

	/* 2) Data Selection: selects CLC_IN PPS value for first channel */
	SET_PIC_REG(PIC_CLC4SEL0, PIC_CLC4SEL_IN_PPS0);
	SET_PIC_REG(PIC_CLC4SEL1, 0x0);
	SET_PIC_REG(PIC_CLC4SEL2, 0x0);
	SET_PIC_REG(PIC_CLC4SEL3, 0x0);

	/* 3) Clear Associated ANSEL bits */
	SET_PIC_FIELD(PIC_ANSELC, ANSELC_RJ45_CHANNEL4, DIGITAL);
	SET_PIC_FIELD(PIC_ANSELC, ANSELC_ARM_CHANNEL4, DIGITAL);

	/* 4|5|10) Setup pin configuration */
	switch(direction) {
		case RJ45_2_ARM:
			puts("Channel 4: RJ45 -> ARM\n");
			SET_PIC_REG(PIC_CLCIN3PPS, PPSIN_RJ45_CHANNEL4);
			SET_PIC_FIELD(PIC_TRISC, TRISC_ARM_INDEX_CHANNEL4, OUTPUT);
			SET_PIC_FIELD(PIC_TRISC, TRISC_RJ45_INDEX_CHANNEL4, INPUT);
			SET_PIC_REG(OUTPIN_ARM_CLC_CHANNEL4, CLCOUT_INDEX_CHANNEL4);
			break;
		case ARM_2_RJ45:
			puts("Channel 4: ARM -> RJ45\n");
			SET_PIC_REG(PIC_CLCIN3PPS, PPSIN_ARM_CHANNEL4);
			SET_PIC_FIELD(PIC_TRISC, TRISC_ARM_INDEX_CHANNEL4, INPUT);
			SET_PIC_FIELD(PIC_TRISC, TRISC_RJ45_INDEX_CHANNEL4, OUTPUT);
			SET_PIC_REG(OUTPIN_RJ45_CLC_CHANNEL4, CLCOUT_INDEX_CHANNEL4);
			break;
		default:
			puts("Warning unknown dir\n");
			break;
	}
	unsigned char tmp = GET_PIC_REG(PIC_TRISC);
	printf("\ttrisc = 0x{%02x}\n", tmp);

	/* 6) Data Gating: Non inverted CLC gate
	 * First gate will redirect the given input signal.
	 * All other three, will propagate a fixed 1 */
	SET_PIC_REG(PIC_CLC4GLS0, CLC_GATE_ONLY_FIRST_CHANNEL);
	SET_PIC_REG(PIC_CLC4GLS1, 0);    /* Fixed 0 on all outputs*/
	SET_PIC_REG(PIC_CLC4GLS2, 0);    /* Fixed 0 on all outputs*/
	SET_PIC_REG(PIC_CLC4GLS3, 0);    /* Fixed 0 on all outputs*/

	/* 7) Gate output polarity */
	SET_PIC_FIELD(PIC_CLC4POL, LC4G1POL, 0);  /* Non-inverting */
	SET_PIC_FIELD(PIC_CLC4POL, LC4G2POL, 1);  /* Inverting */
	SET_PIC_FIELD(PIC_CLC4POL, LC4G3POL, 1);  /* Inverting */
	SET_PIC_FIELD(PIC_CLC4POL, LC4G4POL, 1);  /* Inverting */

	/* 8) Logic Function: 4-INPUT AND */
	SET_PIC_FIELD(PIC_CLC4CON, LC4MODE, LCMODE_4_INPUT_AND);

	/* 11) Interrupts */
	/* Not needed */

	/* 12) Finaly enable the CLC block */
	SET_PIC_FIELD(PIC_CLC4CON, LC4EN, 1);
}

