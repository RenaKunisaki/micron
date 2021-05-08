#ifndef _MICRON_I2C_H_
#define _MICRON_I2C_H_

#ifdef __cplusplus
	extern "C" {
#endif

#include "i2c-regs.h"

#ifndef I2C_TX_BUFSIZE
	#define I2C_TX_BUFSIZE 32
#endif
//#ifndef I2C_RX_BUFSIZE
//	#define I2C_RX_BUFSIZE 32
//#endif

//XXX do we use this?
//#define I2C_BUFFER_IS_FULL(buf, size) ((((buf).head +1) % (size)) == (buf).tail)

//XXX improve this.
//see https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf#d2100e3a1310_d2092e71
//have to figure out how to compute that table on the fly from the CPU clock...
#if F_BUS == 60 MHZ
	#define I2C_CLOCK  0x2C
	#define I2C_FILTER 4

#elif F_BUS == 56 MHZ
	#define I2C_CLOCK  0x2B
	#define I2C_FILTER 4

#elif F_BUS == 48 MHZ
	#define I2C_CLOCK  0x27
	#define I2C_FILTER 4

#elif F_BUS == 40 MHZ
	#define I2C_CLOCK  0x29
	#define I2C_FILTER 4

#elif F_BUS == 36 MHZ
	#define I2C_CLOCK  0x28
	#define I2C_FILTER 3

#elif F_BUS == 24 MHZ
	#define I2C_CLOCK  0x1F
	#define I2C_FILTER 2

#elif F_BUS == 16 MHZ
	#define I2C_CLOCK  0x20
	#define I2C_FILTER 1

#elif F_BUS == 8 MHZ
	#define I2C_CLOCK  0x14
	#define I2C_FILTER 1

#elif F_BUS == 4 MHZ
	#define I2C_CLOCK  0x07
	#define I2C_FILTER 1

#elif F_BUS == 2 MHZ
	#define I2C_CLOCK  0x00
	#define I2C_FILTER 1
#endif


typedef enum {
	MICRON_I2C_STATE_IDLE = 0,
	MICRON_I2C_STATE_TX,
	MICRON_I2C_STATE_RX,
	MICRON_I2C_STATE_STOP,
	MICRON_I2C_STATE_WAIT,
} _micron_i2c_state;


typedef struct {
	uint8_t scl_pin, sda_pin; //which pins this port uses
	//volatile unsigned int transmitting : 1; //transmission in progress?
	//volatile unsigned int receiving : 1; //expecting more data?
	volatile unsigned int sendStop  : 1; //send STOP after Tx finishes?
	volatile unsigned int doRecv    : 1; //enter RX state after Tx finishes?
	volatile unsigned int slaveMode : 1; //device in slave mode?
	volatile _micron_i2c_state state;
	struct {
		volatile uint8_t data[UART_TX_BUFSIZE];
		#if UART_TX_BUFSIZE > 255
			volatile uint16_t head, tail;
		#else
			volatile uint8_t  head, tail;
		#endif
	} txbuf;
	struct {
		uint8_t *data;
		uint32_t idx, len;
	} rxbuf;
} micron_i2c_state;


//i2c.c
extern i2c_regs_t *i2c_regs[NUM_I2C];
extern micron_i2c_state *i2c_state[NUM_I2C];

void i2cDefaultSlaveRx(uint8_t data);
void i2cSlaveRx(uint8_t data);
void i2cDefaultSlaveTx(uint8_t data);
void i2cSlaveTx(uint8_t data);

int i2cInit(uint8_t port, int address);
void i2cShutdown(uint8_t port);
int i2cBeginTx(uint8_t port, uint8_t address);
int i2cSend(uint8_t port, const void *data, uint32_t len);
int i2cEndTx(uint8_t port, uint8_t stop);
int i2cSendRecv(uint8_t port, uint8_t address, uint32_t txLen, const void *src,
uint32_t rxLen, uint8_t *dest, uint32_t timeout);
int i2cGetNumRecv(uint8_t port);
int i2cRead(uint8_t port, uint32_t length, uint8_t *buffer, uint32_t timeout);
int i2cRequest(uint8_t port, uint8_t address, uint32_t length, uint8_t stop,
	uint8_t *buffer, uint32_t timeout);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_I2C_H_
