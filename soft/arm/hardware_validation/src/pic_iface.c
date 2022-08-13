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
	char hex_file[3000];

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
