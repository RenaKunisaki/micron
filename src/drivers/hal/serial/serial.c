//UART serial functions.
//This is the public HAL API, which at compile-time will select the appropriate
//backend for the target hardware.

#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

SECTION(".bss") MicronUartState *_uartState[NUM_UART];

int serialSetBaud(uint32_t port, uint32_t baud) {
    /** Sets up the baud rate for a UART.
     *  @param port Which UART.
     *  @param baud Baud rate.
     *  @return 0 on success, or negative error code.
     *  @note serialInit() calls this, so normally you'd never need to call it
     *   yourself, unless you've changed the clock speed.
     */
    if(port >= NUM_UART) return -ENODEV; //No such device
 	//if(_uartState[port] != NULL) return 0; //already init

    #if defined(MCU_BASE_KINETIS)
        return kinetis_serialSetBaud(port, baud);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int serialInit(uint32_t port, uint32_t baud) {
    /** Initialize UART at specified baud rate.
     *  @param port Which UART.
     *  @param baud Baud rate.
     *  @return 0 on success, or negative error code.
     */
    if(port >= NUM_UART) return -ENODEV; //No such device
	if(_uartState[port] != NULL) return 0; //already init

	//init state
	_uartState[port] = (MicronUartState*)malloc(sizeof(MicronUartState));
	if(_uartState[port] == NULL) return -ENOMEM;
	_uartState[port]->transmitting = 0;
	_uartState[port]->txbuf.head   = 0;
	_uartState[port]->txbuf.tail   = 0;
	_uartState[port]->rxbuf.head   = 0;
	_uartState[port]->rxbuf.tail   = 0;
	_uartState[port]->txCnt        = 0;
	_uartState[port]->rxCnt        = 0;
    //debug
	//memset((void*)_uartState[port]->txbuf.data, 0xAA, UART_TX_BUFSIZE);
	//memset((void*)_uartState[port]->rxbuf.data, 0xAA, UART_RX_BUFSIZE);

    #if defined(MCU_BASE_KINETIS)
        return kinetis_serialInit(port, baud);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        serialShutdown(port);
        return -ENOSYS;
    #endif
}

int serialShutdown(uint32_t port) {
    /** Shut down specified UART, if it's enabled.
     *  @param port Which UART.
     *  @return 0 on success, or negative error code.
     */
    if(port >= NUM_UART) return -ENODEV; //No such device
	if(_uartState[port] == NULL) return 0; //already shutdown

    int err = 0;
    #if defined(MCU_BASE_KINETIS)
        err = kinetis_serialShutdown(port);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif

    if(err) return err;
    free(_uartState[port]);
	_uartState[port] = NULL;
    return 0;
}

int serialSend(uint32_t port, const void *data, uint32_t len) {
    /** Transmit data over specified UART.
     *  @param port Which UART.
     *  @param data Data to send.
     *  @param len Number of bytes to send.
     *  @return Number of bytes sent (which can be zero),
     *   or negative error code.
     *  @note A return of less than len (especially zero) indicates the transmit
     *   buffer is full and you might want to wait for an interrupt before
     *   retrying.
     */
    if(len == 0) return 0;
	if(port >= NUM_UART) return -ENODEV;
    MicronUartState *uart = _uartState[port];
	if(uart == NULL) return -EBADFD;

    irqDisable();

    int r = 0;
    #if defined(MCU_BASE_KINETIS)
        r = kinetis_serialSend(port, data, len);

    #elif defined(MCU_BASE_IMX)
        r = -ENOSYS; //XXX

    #else
        r = -ENOSYS;
    #endif

    irqEnable();
    return r;
}

int serialReceive(uint32_t port, char *data, uint32_t len) {
    /** Receive from specified UART.
     *  @param port Which UART.
     *  @param data Destination buffer.
     *  @param len Maximum bytes to receive.
     *  @return Number of bytes received (which can be zero), or
     *   negative error code.
     */
    if(len == 0) return 0;
	if(port >= NUM_UART) return -ENODEV;
    MicronUartState *uart = _uartState[port];
	if(uart == NULL) return -EBADFD;

    irqDisable();

    int r = 0;
    #if defined(MCU_BASE_KINETIS)
        r = kinetis_serialReceive(port, data, len);

    #elif defined(MCU_BASE_IMX)
        r = -ENOSYS; //XXX

    #else
        r = -ENOSYS;
    #endif

    irqEnable();
    return r;
}

int serialFlush(uint32_t port) {
    /** Wait for specified UART to be finished transmitting.
     *  @param port Which UART.
     *  @return 0 on success, or negative error code.
     */
    if(port >= NUM_UART) return -ENODEV; //No such device
	MicronUartState *uart = _uartState[port];
	if(uart == NULL) return -EBADFD;

    int r = 0;
    #if defined(MCU_BASE_KINETIS)
        r = kinetis_serialFlush(port);

    #elif defined(MCU_BASE_IMX)
        r = -ENOSYS; //XXX

    #else
        r = -ENOSYS;
    #endif

    return r;
}

int serialClear(uint32_t port) {
    /** Discard all buffered input from specified UART.
     *  @param port Which UART.
     *  @return 0 on success, or negative error code.
     */
    if(port >= NUM_UART) return -ENODEV; //No such device
	MicronUartState *uart = _uartState[port];
	if(uart == NULL) return -EBADFD;

    int r = 0;
    #if defined(MCU_BASE_KINETIS)
        r = kinetis_serialClear(port);

    #elif defined(MCU_BASE_IMX)
        r = -ENOSYS; //XXX

    #else
        r = -ENOSYS;
    #endif

    return r;
}

int serialPutchr(uint32_t port, char c) {
    /** Send one character to UART.
     *  @param port Which UART.
     *  @param c Character to send.
     *  @return 0 on success, or negative error code.
     */
    int r;
	do {
		r = serialSend(port, &c, 1);
		if(r == 0) irqWait();
	} while(r <= 0);
	return r;
}

int serialPuts(uint32_t port, const char *str) {
    /** Send a null-terminated string to UART.
     *  @param port Which UART.
     *  @param str String to send.
     *  @return 0 on success, or negative error code.
     *  @note Blocks until entire string is sent or error occurs.
     */
    int r;
	int len = strlen(str);
	int slen = len; //for return value
	//XXX any better way than using strlen?
	//we could make this function mostly a duplicate of serialSend()
	//with the addition of checking for null characters...
	//also, standard puts() adds a line break.
	do {
		r = serialSend(port, str, len);
		if(r < 0) return r;
		if(r == 0) irqWait();
		len -= r;
		str += r;
	} while(len > 0);
	if(r >= 0) return slen;
	return r;
}

int serialGetchr(uint32_t port) {
    /** Receive one byte from UART.
     *  @param port Which UART.
     *  @return Received byte (0-255) on success, or negative error code.
     */
    char c = 0;
	int r = serialReceive(port, &c, 1);
	if(r <= 0) return r;
	return c;
}

int serialGets(uint32_t port, char *str, uint32_t len) {
    /** Receive a string from UART.
     *  @param port Which UART.
     *  @param str Destination buffer.
     *  @param len Maximum bytes to receive.
     *  @return Number of bytes received, or negative error code.
     *  @note Receives until a carriage return, line feed, or null character.
     *   These are not added to `str`.
     */
    char chr;
	uint32_t count = 0;
	while(count < (len-1)) {
		int r = serialReceive(port, &chr, 1); //XXX any better way?
		if(r < 0) return r;
		else if(r == 0) irqWait();
		else if(chr == '\r' || chr == '\n' || chr == '\0') break;
		else str[count++] = chr;
	}
	str[count] = '\0';
	return count;
}


#ifdef __cplusplus
	} //extern "C"
#endif
