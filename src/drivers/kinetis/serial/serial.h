#ifndef _MICRON_SERIAL_H_
#define _MICRON_SERIAL_H_

#ifdef __cplusplus
	extern "C" {
#endif

#define UART_C2_ENABLE (                           \
	UART_C2_TE  | /* Transmitter Enable */         \
	UART_C2_RE  | /* Receiver Enable */            \
	UART_C2_RIE | /* Rx Interrupt Enable */        \
    UART_C2_ILIE)
#define UART_C2_TX_ACTIVE	  (UART_C2_ENABLE | UART_C2_TIE)
#define UART_C2_TX_COMPLETING (UART_C2_ENABLE | UART_C2_TCIE)
#define UART_C2_TX_INACTIVE	  (UART_C2_ENABLE)

#ifndef UART_TX_BUFSIZE
#define UART_TX_BUFSIZE 64
#endif
#ifndef UART_RX_BUFSIZE
#define UART_RX_BUFSIZE 64
#endif

typedef struct {
    uint32_t txCnt, rxCnt; //debug counters
	uint8_t tx_pin, rx_pin; //which pins this port uses
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
} micron_uart_state;

extern uint8_t uart_interrupt_priority;
extern micron_uart_state *uart_state[NUM_UART];

//serial.c
void serialSetBaud(uint8_t port, uint32_t baud);
int  serialInit(uint8_t port, uint32_t baud);
void serialShutdown(uint8_t port);
int  serialSend(uint8_t port, const void *data, uint32_t len);
int  serialReceive(uint8_t port, char *data, uint32_t len);
void serialFlush(uint8_t port);
void serialClear(uint8_t port);
int  serialPutchr(uint8_t port, char c);
int  serialPuts(uint8_t port, const char *str);
int  serialGetchr(uint8_t port);
int  serialGets(uint8_t port, char *str, uint32_t len);

//serial-isr.c
void isrUartStatus(int port);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_SERIAL_H_
