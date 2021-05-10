#ifndef _MICRON_HAL_SERIAL_H_
#define _MICRON_HAL_SERIAL_H_

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef UART_TX_BUFSIZE
#define UART_TX_BUFSIZE 64
#endif
#ifndef UART_RX_BUFSIZE
#define UART_RX_BUFSIZE 64
#endif

typedef struct {
    uint32_t txCnt, rxCnt; //debug counters
	uint8_t txPin, rxPin; //which pins this port uses
	volatile int transmitting : 1; //is this port currently transmitting?
	struct {
		volatile uint8_t data[UART_TX_BUFSIZE];
		#if UART_TX_BUFSIZE > 255
			volatile uint16_t head, tail;
		#else
			volatile uint8_t  head, tail;
		#endif
	} txbuf;
	struct {
		volatile uint8_t data[UART_RX_BUFSIZE];
		#if UART_RX_BUFSIZE > 255
			volatile uint16_t head, tail;
		#else
			volatile uint8_t  head, tail;
		#endif
	} rxbuf;
} MicronUartState;

extern MicronUartState *_uartState[NUM_UART];

//serial.c
int serialSetBaud(uint32_t port, uint32_t baud);
int serialInit(uint32_t port, uint32_t baud);
int serialShutdown(uint32_t port);
int serialSend(uint32_t port, const void *data, uint32_t len);
int serialReceive(uint32_t port, char *data, uint32_t len);
int serialFlush(uint32_t port);
int serialClear(uint32_t port);
int serialPutchr(uint32_t port, char c);
int serialPuts(uint32_t port, const char *str);
int serialGetchr(uint32_t port);
int serialGets(uint32_t port, char *str, uint32_t len);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_HAL_SERIAL_H_
