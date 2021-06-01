#ifndef _MICRON_HAL_SPI_H_
#define _MICRON_HAL_SPI_H_

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
    //SPI standard modes
    SPI_MODE_0 = 0, //Clock active high, change on falling edge (normal mode)
    SPI_MODE_1 = 1, //Clock active high, change on rising  edge
    SPI_MODE_2 = 2, //Clock active low,  change on falling edge
    SPI_MODE_3 = 3, //Clock active low,  change on rising  edge
    //OR those with any of these
    SPI_MODE_SLAVE = BIT(2), //slave mode
    SPI_MODE_CONT_SCK = BIT(3), //continuous clock
} MicronSpiModeEnum;

#ifndef SPI_TX_BUFSIZE
#define SPI_TX_BUFSIZE 64
#endif
#ifndef SPI_RX_BUFSIZE
#define SPI_RX_BUFSIZE 64
#endif

typedef struct {
    uint8_t pinCS;
    //volatile int transmitting : 1; //is this port currently transmitting?
	struct {
        //for simplicity's sake this is just the entire PUSHR value.
        //that lets us control CS and such, and do up to 16-bit frame size
        //(the maximum supported by hardware).
        //maybe we'll need to modify this somewhat if other hardware doesn't
        //have the same features, but application code shouldn't need to be
        //concerned with the details of this structure.
		volatile uint32_t data[SPI_TX_BUFSIZE];
		#if SPI_TX_BUFSIZE > 255
			volatile uint16_t head, tail;
		#else
			volatile uint8_t  head, tail;
		#endif
	} txbuf;
	struct {
		volatile uint16_t data[SPI_RX_BUFSIZE];
		#if SPI_RX_BUFSIZE > 255
			volatile uint16_t head, tail;
		#else
			volatile uint8_t  head, tail;
		#endif
	} rxbuf;
    //debug
    uint32_t txCount, rxCount; //number of frames sent/rxd on wire
    uint32_t txBufCnt, rxBufCnt; //num frames put into each buffer
    uint32_t irqFillCnt, irqEmptyCnt, irqCnt; //Fill/Empty/Total IRQ count
} MicronSpiState;

extern MicronSpiState *_spiState[NUM_SPI];

int spiInit(uint32_t port, uint32_t pinCS, uint32_t speed, MicronSpiModeEnum mode);
int spiPause(uint32_t port, bool pause);
int spiSetMode(uint32_t port, MicronSpiModeEnum mode);
int spiSetSpeed(uint32_t port, uint32_t speed);
int spiSetFrameSize(uint32_t port, uint32_t size);
int spiWriteDummy(uint32_t port, uint32_t data, uint32_t count, bool cs);
int spiWrite(uint32_t port, const void *data, uint32_t len, bool cont);
int spiWriteBlocking(uint32_t port, const void *data, uint32_t len, bool cont,
uint32_t timeout);
int spiRead(uint32_t port, void *out, uint32_t len, uint32_t timeout);
int spiReadBlocking(uint32_t port, void *out, uint32_t len, uint32_t timeout);
int spiWaitTxDone(uint32_t port, uint32_t timeout);
int spiClear(uint32_t port);

#ifdef __cplusplus
    } //extern "C"
#endif

#endif //_MICRON_HAL_SPI_H_
