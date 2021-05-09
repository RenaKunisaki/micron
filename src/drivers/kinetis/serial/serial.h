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

extern uint8_t uart_interrupt_priority;
extern MicronUartState *_uartState[NUM_UART];

//serial.c
int kinetis_serialSetBaud(uint32_t port, uint32_t baud);
int kinetis_serialInit(uint32_t port, uint32_t baud);
int kinetis_serialShutdown(uint32_t port);
int kinetis_serialSend(uint32_t port, const void *data, uint32_t len);
int kinetis_serialReceive(uint32_t port, char *data, uint32_t len);
int kinetis_serialFlush(uint32_t port);
int kinetis_serialClear(uint32_t port);

//serial-isr.c
void isrUartStatus(int port);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_SERIAL_H_
