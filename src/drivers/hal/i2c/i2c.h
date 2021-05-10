#ifndef _MICRON_HAL_I2C_H_
#define _MICRON_HAL_I2C_H_

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef I2C_TX_BUFSIZE
	#define I2C_TX_BUFSIZE 32
#endif
//#ifndef I2C_RX_BUFSIZE
//	#define I2C_RX_BUFSIZE 32
//#endif

typedef enum {
	MICRON_I2C_STATE_IDLE = 0,
	MICRON_I2C_STATE_TX,
	MICRON_I2C_STATE_RX,
	MICRON_I2C_STATE_STOP,
	MICRON_I2C_STATE_WAIT,
} MicronI2cStateEnum;


typedef struct {
    uint8_t port; //which port this is
	uint8_t scl_pin, sda_pin; //which pins this port uses
	//volatile unsigned int transmitting : 1; //transmission in progress?
	//volatile unsigned int receiving : 1; //expecting more data?
	volatile unsigned int sendStop  : 1; //send STOP after Tx finishes?
	volatile unsigned int doRecv    : 1; //enter RX state after Tx finishes?
	volatile unsigned int slaveMode : 1; //device in slave mode?
	volatile MicronI2cStateEnum state;
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
} MicronI2cState;


//i2c.c
extern MicronI2cState *i2cState[NUM_I2C];

int i2cInit(uint32_t port, int address);
int i2cShutdown(uint32_t port);
int i2cBeginTx(uint32_t port, uint8_t address);
int i2cContinueTx(uint32_t port, const void *data, uint32_t len);
int i2cEndTx(uint32_t port, bool stop);
int i2cSend(uint32_t port, uint8_t addr, const void *data, uint8_t len);
int i2cSendRecv(uint32_t port, uint8_t address, const void *txData,
    uint32_t txLen, uint8_t *rxData, uint32_t rxLen, uint32_t timeout);
int i2cGetNumRecv(uint32_t port);
int i2cRead(uint32_t port, uint8_t *buffer, uint32_t length, uint32_t timeout);
int i2cRequest(uint32_t port, uint8_t address, uint32_t length, uint8_t stop,
    uint8_t *buffer, uint32_t timeout);

//internal.c
int _i2cBufferTxP(MicronI2cState *state, const void *data, uint32_t len);
int _i2cWaitForBus(MicronI2cState *state, uint32_t timeout);
int _i2cWaitForIdle(MicronI2cState *state, uint32_t timeout);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_HAL_I2C_H_
