//UART serial functions.
//This is the public API. See also serial-isr.c for the
//behind-the-scenes interrupt handlers.

#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

static const uint8_t uart_tx_pin[] = {UART0_TX_PIN, UART1_TX_PIN, UART2_TX_PIN};
static const uint8_t uart_rx_pin[] = {UART0_RX_PIN, UART1_RX_PIN, UART2_RX_PIN};
WEAK uint8_t uart_interrupt_priority = 64; //0 = highest priority, 255 = lowest
SECTION(".bss") micron_uart_state *uart_state[NUM_UART];

/** Sets up the baud rate for a UART.
 *  serialInit() calls this, so normally you'd never need to call it yourself,
 *  unless you've changed the clock speed.
 */
void serialSetBaud(uint8_t port, uint32_t baud) {
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
}


/** Initialize specified UART at specified baud rate.
 *  Returns 0 on success, or a negative error code on failure.
 *  This function takes care of turning on the UART module, preparing buffers,
 *  and setting up interrupts.
 */
int serialInit(uint8_t port, uint32_t baud) {
	if(port >= NUM_UART) return -ENODEV; //No such device
	if(uart_state[port] != NULL) return 0; //already init

	//init state
	uart_state[port] = (micron_uart_state*)malloc(sizeof(micron_uart_state));
	if(uart_state[port] == NULL) return -ENOMEM;
	uart_state[port]->tx_pin = uart_tx_pin[port];
	uart_state[port]->rx_pin = uart_rx_pin[port];
	uart_state[port]->transmitting = 0;
	uart_state[port]->txbuf.head   = 0;
	uart_state[port]->txbuf.tail   = 0;
	uart_state[port]->rxbuf.head   = 0;
	uart_state[port]->rxbuf.tail   = 0;
	uart_state[port]->txCnt        = 0;
	uart_state[port]->rxCnt        = 0;
    //debug
	//memset((void*)uart_state[port]->txbuf.data, 0xAA, UART_TX_BUFSIZE);
	//memset((void*)uart_state[port]->rxbuf.data, 0xAA, UART_RX_BUFSIZE);

	//enable clock for UART module.
	SIM_SCGC4 |= (SIM_SCGC4_UART0 << port);

	//set up pins
	setPinMode(uart_tx_pin[port],
		PCR_DRIVE_STRENGTH_HI | PCR_SLEW_SLOW | PCR_MUX(3), OUTPUT);
	setPinMode(uart_rx_pin[port], PCR_PULLUP | PCR_FILTER | PCR_MUX(3), INPUT);

	//set up module
	KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(port);
	serialSetBaud(port, baud);
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
void serialShutdown(uint8_t port) {
	if(port >= NUM_UART) return;
	if(uart_state[port] == NULL) return; //already shutdown
	NVIC_DISABLE_IRQ(IRQ_UART0_STATUS + (port*2));

	//disable clock for UART module. XXX use bitband register here
	SIM_SCGC4 &= ~(SIM_SCGC4_UART0 << port);
	free(uart_state[port]);
	uart_state[port] = NULL;
}


/** Transmit specified data over specified UART.
 *  On success, returns the number of bytes which were transmitted (which could
 *  be anywhere from 0 to len).
 *  On failure, returns a negative error code.
 *  A return of less than len (especially zero) indicates the transmit buffer is
 *  full and you might want to wait for an interrupt before retrying.
 */
int serialSend(uint8_t port, const void *data, uint32_t len) {
	if(len == 0) return 0;
	if(port >= NUM_UART) return -ENODEV;

	micron_uart_state *uart = uart_state[port];
	if(uart == NULL) return -EBADFD;
    irqDisable();
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
    #else
    while(i < len) {
        while(regs->TCFIFO) idle();
        //avail = fifoSize - regs->TCFIFO;
        //while(avail --> 0 && i < len) {
            regs->D = dat[i++];
        //}
    }
    #endif

	regs->C2 = UART_C2_TX_ACTIVE; //enable transmit interrupt
    irqEnable();
	return i; //return number of bytes we sent
}


/** Receive from specified UART.
 *  On success, returns number of bytes received (which could be zero).
 *  On failure, returns a negative error code.
 */
int serialReceive(uint8_t port, char *data, uint32_t len) {
	if(port >= NUM_UART) return -ENODEV;

	micron_uart_state *uart = uart_state[port];
	if(uart == NULL) return -EBADFD;
    irqDisable();

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
    //irqEnable();
    #else

    unsigned int i = 0;
	while(i < len && uart->rxbuf.tail != uart->rxbuf.head) {
        data[i++] = uart->rxbuf.data[uart->rxbuf.tail++];
        uart->rxbuf.tail %= UART_RX_BUFSIZE;
    }

    #endif
    irqEnable();
	return i;
}


/** Wait for specified UART to be finished transmitting.
 */
void serialFlush(uint8_t port) {
	if(port >= NUM_UART) return;
	micron_uart_state *uart = uart_state[port];
	if(uart == NULL) return;

	if(irqCurrentISR() == 0 && irqEnabled()) {
		//if we're not in an ISR already, we can wait for one.
		while(uart->transmitting) irqWait();
	}
	else {
		//otherwise, we must busy-wait and run the handler manually.
		while(uart->transmitting) isrUartStatus(port);
	}
}


/** Discard all buffered input from specified UART.
 */
void serialClear(uint8_t port) {
	if(port >= NUM_UART) return;
	micron_uart_state *uart = uart_state[port];
	if(uart == NULL) return;
	KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(port);

	regs->C2 &= ~(UART_C2_RE | UART_C2_RIE | UART_C2_ILIE);
	regs->CFIFO = UART_CFIFO_RXFLUSH;
	uart->rxbuf.head = uart->rxbuf.tail;
	regs->C2 |=  (UART_C2_RE | UART_C2_RIE | UART_C2_ILIE);
}


int serialPutchr(uint8_t port, char c) {
	int r;
	do {
		r = serialSend(port, &c, 1);
		if(r == 0) irqWait();
	} while(r <= 0);
	return r;
}

int serialPuts(uint8_t port, const char *str) {
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

int serialGetchr(uint8_t port) {
	char c = 0;
	int r = serialReceive(port, &c, 1);
	if(r <= 0) return r;
	return c;
}

int serialGets(uint8_t port, char *str, uint32_t len) {
	//works a little different from standard gets() because that sucks anyway.
	char chr;
	uint32_t count = 0;
	while(count < (len-1)) {
		int r = serialReceive(port, &chr, 1); //XXX any better way?
		if(r < 0) return r;
		else if(r == 0) irqWait();
		else if(chr == '\r' || chr == '\n') break;
		else str[count++] = chr;
	}
	str[count] = '\0';
	return count;
}


#ifdef __cplusplus
	} //extern "C"
#endif
