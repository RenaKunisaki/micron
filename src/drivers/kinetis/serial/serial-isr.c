//UART serial interrupt service routines.
//These do the behind-the-scenes buffer management for the serial API.

#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

//called from default UART ISRs to handle receiving
void uart_isr_rx_default(micron_uart_state *uart, KINETISK_UART_t *regs) {
	uint8_t avail = regs->RCFIFO;
	if (avail == 0) { //Rx FIFO empty
		/* The only way to clear the IDLE interrupt flag is
		 * to read the data register.  But reading with no
		 * data causes a FIFO underrun, which causes the
		 * FIFO to return corrupted data.  If anyone from
		 * Freescale reads this, what a poor design!  There
		 * write should be a write-1-to-clear for IDLE.
		 */
		UNUSED uint8_t dummy = regs->D;
		/* flushing the fifo recovers from the underrun,
		 * but there's a possible race condition where a
		 * new character could be received between reading
		 * RCFIFO == 0 and flushing the FIFO.  To minimize
		 * the chance, interrupts are disabled so a higher
		 * priority interrupt (hopefully) doesn't delay.
		 */
		regs->CFIFO = UART_CFIFO_RXFLUSH;
	} else {
		//receive bytes into Rx buffer.
		uint32_t head = uart->rxbuf.head;
		uint32_t tail = uart->rxbuf.tail;
		while(avail) {
            uint32_t data = regs->D; //receive data
			//if (use9Bits && (regs->C3 & 0x80)) n |= 0x100; //receive 9th bit
			uint32_t next = (head + 1) % UART_RX_BUFSIZE;
			if (next != tail) {
				uart->rxbuf.data[head] = data;
				head = next;
			}
            avail--;
            uart->rxCnt++;
		}
		uart->rxbuf.head = head;
	}
}

static void uart_isr_tx_default(micron_uart_state *uart, KINETISK_UART_t *regs) {
    if(uart->txbuf.head == uart->txbuf.tail) {
        //we don't have any more to send.
        if(regs->S1 & UART_S1_TC) { //last char has finished sending.
            uart->transmitting = 0;
            digitalWrite(uart->tx_pin, 0); //tx deassert
        }
        //else wait for Transmit Complete interrupt
        else regs->C2 |= UART_C2_TCIE | UART_C2_TE;
    }
    else {
        //we have more to send, and the buffer isn't full
        //(as implied by TC and/or TDRE being set).
        //TDRE is a bit misleading; it indicates that the buffer
        //hasn't reached the watermark point, not that it's
        //completely empty, so we can send more than one byte here.
        uint32_t head = uart->txbuf.head;
		uint32_t tail = uart->txbuf.tail;
        //apparerntly TDRE is just useless.
        //while((head != tail) && (regs->S1 & UART_S1_TDRE)) {
        while((head != tail) && (regs->SFIFO & UART_SFIFO_TXEMPT)) {
            uint32_t data = uart->txbuf.data[tail++];
            //if (use9Bits) regs->C3 = (regs->C3 & ~0x40) | ((n & 0x100) >> 2);
            tail %= UART_TX_BUFSIZE;
            uart->txCnt++;
            regs->D = data;
        }
        uart->txbuf.tail = tail;
        //enable transmit interrupt
        regs->C2 |= UART_C2_TIE | UART_C2_TCIE | UART_C2_TE | UART_C2_ILIE;
    }
}

void uart_isr_rx(micron_uart_state *uart, KINETISK_UART_t *regs)
    WEAK ALIAS("uart_isr_rx_default");

void uart_isr_tx(micron_uart_state *uart, KINETISK_UART_t *regs)
    WEAK ALIAS("uart_isr_tx_default");

//called from default UART ISRs
void isrUartStatus(int port) {
	//This interrupt fires on the following events:
	//  Event                          Flag
	//  Transmit data below watermark  UART_S1_TDRE
	//  Transmit complete              UART_S1_TC
	//  Idle line                      UART_S1_IDLE
	//  Receive data above watermark   UART_S1_RDRF
	//  LIN break detect               UART_S2_LBKDIF
	//  RxD pin active edge            UART_S2_RXEDGIF
    //  see UARTx_BDH and UARTx_BDL for the last one.
    irqDisable(); //avoid race condition (see uart_isr_rx_default)
    micron_uart_state *uart = uart_state[port];
	KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(port);
    //disable all interrupts while we handle them.
    //leave transmitter and receiver enabled.
    regs->C2 = UART_C2_TE | UART_C2_RE;
    uart_isr_rx(uart, regs);
    uart_isr_tx(uart, regs);
    regs->C2 |= UART_C2_RIE; //enable receiver interrupt.
    //Cortex-M4 ARM errata 838869: "Store immediate overlapping
    //exception return operation might vector to incorrect interrupt"
    //XXX this is only needed for Cortex-M4
    __asm__ volatile ("DSB");
    irqEnable();
}

#ifdef __cplusplus
	} //extern "C"
#endif
