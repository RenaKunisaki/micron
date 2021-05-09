//UART serial functions.

#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

WEAK uint8_t uart_interrupt_priority = 64; //0 = highest priority, 255 = lowest

static const uint8_t uart_tx_pin[] = {
    UART0_TX_PIN, UART1_TX_PIN, UART2_TX_PIN, //XXX more
};
static const uint8_t uart_rx_pin[] = {
    UART0_RX_PIN, UART1_RX_PIN, UART2_RX_PIN,
};


/** Sets up the baud rate for a UART.
 *  serialInit() calls this, so normally you'd never need to call it yourself,
 *  unless you've changed the clock speed.
 */
int kinetis_serialSetBaud(uint32_t port, uint32_t baud) {
    if(port >= NUM_UART) return -ENODEV; //No such device

	uint32_t divisor;
	//UART0 and UART1 modules operate from the core/system clock, which provides
	//higher performance level for these modules. All other UART modules
	//operate from the bus clock.
	if(port > 1) divisor = (((F_BUS * 2) + ((baud) >> 1)) / (baud));
	else         divisor = (((F_CPU * 2) + ((baud) >> 1)) / (baud));

	//handy struct in kinetis.h
	KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(port);
	regs->BDH = (divisor >> 13) & 0x1F;
	regs->BDL = (divisor >>  5) & 0xFF;
	regs->C4  =  divisor        & 0x1F;
    return 0;
}


/** Initialize specified UART at specified baud rate.
 *  Returns 0 on success, or a negative error code on failure.
 *  This function takes care of turning on the UART module, preparing buffers,
 *  and setting up interrupts.
 */
int kinetis_serialInit(uint32_t port, uint32_t baud) {
    _uartState[port]->tx_pin = uart_tx_pin[port];
	_uartState[port]->rx_pin = uart_rx_pin[port];

	//set up pins
	setPinMode(uart_tx_pin[port],
		PCR_DRIVE_STRENGTH_HI | PCR_SLEW_SLOW | PCR_MUX(3), OUTPUT);
	setPinMode(uart_rx_pin[port], PCR_PULLUP | PCR_FILTER | PCR_MUX(3), INPUT);

	//set up module
	KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(port);
	kinetis_serialSetBaud(port, baud);
	regs->C1 = UART_C1_ILT; //idle character bit count starts
        //after stop bit
    regs->PFIFO = UART_PFIFO_TXFE | UART_PFIFO_RXFE; //FIFOs on

    uint32_t szTxFifo = (regs->PFIFO >> 4) & 7; //size of HW TX FIFO
	if(szTxFifo == 0) szTxFifo = 1;
	else szTxFifo = 2 << szTxFifo; //1 -> 4, 2 -> 8, 3 -> 16...

    uint32_t szRxFifo = regs->PFIFO & 7; //size of HW RX FIFO
	if(szRxFifo == 0) szRxFifo = 1;
	else szRxFifo = 2 << szRxFifo; //1 -> 4, 2 -> 8, 3 -> 16...

    //tx/rx watermark levels, causes S1_TDRE/S1_RDRF to set
    //when FIFO level reaches them
    if(szTxFifo > 1) regs->TWFIFO = MAX(1U, szTxFifo/2);
    else regs->TWFIFO = 1;
	if(szRxFifo > 1) regs->RWFIFO = MAX(1U, szRxFifo/2);
    else regs->RWFIFO = 1;
    //enable transmitter, enable receiver, and
    //enable receive interrupt
	regs->C2 = UART_C2_TE | UART_C2_RE | UART_C2_RIE;

	//enable UART status interrupt in NVIC
	NVIC_SET_PRIORITY(IRQ_UART0_STATUS + (port*2), uart_interrupt_priority);
	NVIC_ENABLE_IRQ  (IRQ_UART0_STATUS + (port*2));

	return 0; //init OK
}


/** Shut down specified UART, if it's enabled.
 */
int kinetis_serialShutdown(uint32_t port) {
	NVIC_DISABLE_IRQ(IRQ_UART0_STATUS + (port*2));

	//disable clock for UART module. XXX use bitband register here
	SIM_SCGC4 &= ~(SIM_SCGC4_UART0 << port);
    return 0;
}


/** Transmit specified data over specified UART.
 *  On success, returns the number of bytes which were transmitted (which could
 *  be anywhere from 0 to len).
 *  On failure, returns a negative error code.
 *  A return of less than len (especially zero) indicates the transmit buffer is
 *  full and you might want to wait for an interrupt before retrying.
 */
int kinetis_serialSend(uint32_t port, const void *data, uint32_t len) {
	MicronUartState *uart = _uartState[port];
	KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(port);

	digitalWrite(uart->tx_pin, 1); //tx assert

	uint32_t fifoSize = (regs->PFIFO & 0x70) >> 4; //size of HW TX FIFO
	if(fifoSize == 0) fifoSize = 1;
	else fifoSize = 2 << fifoSize; //1 -> 4, 2 -> 8, 3 -> 16...

	uint8_t *dat = (uint8_t*)data;
	regs->C2 = UART_C2_TX_INACTIVE; //disable transmit interrupt
	uart->transmitting = 1;

	//int avail;
	uint32_t i = 0;
	uint32_t head = uart->txbuf.head;
	uint32_t tail = uart->txbuf.tail;

    #if 1
	/* if(head == tail) { //tx buf is empty
		//send to hardware FIFO, transmit immediately, as many bytes as we
		//can fit here.
		avail = fifoSize - regs->TCFIFO;
		while(avail --> 0 && i < len) {
			regs->D = dat[i++];
		}
	} */

	while(i < len) {
		//try to send to software buffer, and transmit in next interrupt.
		//note, one byte of the buffer goes unused here. this seems to be
		//the best way to avoid the ambiguity (when all slots are full,
		//head == tail, same as when empty, so never fill all slots).
		//any other method would require more than one byte overhead.
		uint32_t next = (head + 1) % UART_TX_BUFSIZE;
		if(next == tail) break; //buffer is full
		uart->txbuf.data[head] = dat[i++];
		head = next;
	}
	uart->txbuf.head = head;
    #else //XXX what is this?
    while(i < len) {
        while(regs->TCFIFO) idle();
        //avail = fifoSize - regs->TCFIFO;
        //while(avail --> 0 && i < len) {
            regs->D = dat[i++];
        //}
    }
    #endif

	regs->C2 = UART_C2_TX_ACTIVE; //enable transmit interrupt
	return i; //return number of bytes we sent
}


/** Receive from specified UART.
 *  On success, returns number of bytes received (which could be zero).
 *  On failure, returns a negative error code.
 */
int kinetis_serialReceive(uint32_t port, char *data, uint32_t len) {
	MicronUartState *uart = _uartState[port];

    #if 0
	KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(port);

	uint32_t fifoSize = regs->PFIFO & 7; //size of HW RX FIFO
	if(fifoSize == 0) fifoSize = 1;
	else fifoSize = 2 << fifoSize; //1 -> 4, 2 -> 8, 3 -> 16...

	//XXX use bitband here
	//disable Rx interrupt to avoid race condition,
	//where interrupt might drain FIFO while we read it.
	regs->C2 &= ~(UART_C2_RIE | UART_C2_ILIE);
	uint32_t i = 0;
	while(i < len) {
		uint32_t tail = uart->rxbuf.tail;
		if(tail != uart->rxbuf.head) { //rx buf not empty
			data[i] = (char)uart->rxbuf.data[tail];
			uart->rxbuf.tail = (tail + 1) % UART_RX_BUFSIZE;
		}
		else { //rx buf is empty, check FIFO
			if(regs->RCFIFO > 0) data[i] = (char)regs->D;
			else break; //nothing to receive
            //irqWait();
		}
		//else break;
		i++;
	}

	regs->C2 |= (UART_C2_RIE | UART_C2_ILIE); //re-enable Rx interrupt
    #else

    unsigned int i = 0;
	while(i < len && uart->rxbuf.tail != uart->rxbuf.head) {
        data[i++] = uart->rxbuf.data[uart->rxbuf.tail++];
        uart->rxbuf.tail %= UART_RX_BUFSIZE;
    }

    #endif
    return i;
}


/** Wait for specified UART to be finished transmitting.
 */
int kinetis_serialFlush(uint32_t port) {
    MicronUartState *uart = _uartState[port];
    if(irqCurrentISR() == 0 && irqEnabled()) {
		//if we're not in an ISR already, we can wait for one.
		while(uart->transmitting) irqWait();
	}
	else {
		//otherwise, we must busy-wait and run the handler manually.
		while(uart->transmitting) isrUartStatus(port);
	}
    return 0;
}


/** Discard all buffered input from specified UART.
 */
int kinetis_serialClear(uint32_t port) {
	MicronUartState *uart = _uartState[port];
    KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(port);

	regs->C2 &= ~(UART_C2_RE | UART_C2_RIE | UART_C2_ILIE);
	regs->CFIFO = UART_CFIFO_RXFLUSH;
	uart->rxbuf.head = uart->rxbuf.tail;
	regs->C2 |=  (UART_C2_RE | UART_C2_RIE | UART_C2_ILIE);
    return 0;
}

#ifdef __cplusplus
	} //extern "C"
#endif
