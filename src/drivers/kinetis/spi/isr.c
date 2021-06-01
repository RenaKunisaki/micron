#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

WEAK uint8_t spiInterruptPriority = 128; //0 = highest priority, 255 = lowest

static volatile uint32_t *p_sr[NUM_SPI] = {
    &SPI0_SR,
    #if NUM_SPI > 1
        &SPI1_SR,
    #endif
    #if NUM_SPI > 2
        &SPI2_SR,
    #endif
    #if NUM_SPI > 3
        &SPI3_SR,
    #endif
    #if NUM_SPI > 4
        &SPI4_SR,
    #endif
    #if NUM_SPI > 5
        &SPI5_SR,
    #endif
    #if NUM_SPI > 6
        &SPI6_SR,
    #endif
    #if NUM_SPI > 7
        &SPI7_SR,
    #endif
    #if NUM_SPI > 8
        &SPI8_SR,
    #endif
    #if NUM_SPI > 9
        &SPI9_SR,
    #endif
};
static volatile uint32_t *p_mcr[NUM_SPI] = {
    &SPI0_MCR,
    #if NUM_SPI > 1
        &SPI1_MCR,
    #endif
    #if NUM_SPI > 2
        &SPI2_MCR,
    #endif
    #if NUM_SPI > 3
        &SPI3_MCR,
    #endif
    #if NUM_SPI > 4
        &SPI4_MCR,
    #endif
    #if NUM_SPI > 5
        &SPI5_MCR,
    #endif
    #if NUM_SPI > 6
        &SPI6_MCR,
    #endif
    #if NUM_SPI > 7
        &SPI7_MCR,
    #endif
    #if NUM_SPI > 8
        &SPI8_MCR,
    #endif
    #if NUM_SPI > 9
        &SPI9_MCR,
    #endif
};
static volatile uint32_t *p_tcr[NUM_SPI] = {
    &SPI0_TCR,
    #if NUM_SPI > 1
        &SPI1_TCR,
    #endif
    #if NUM_SPI > 2
        &SPI2_TCR,
    #endif
    #if NUM_SPI > 3
        &SPI3_TCR,
    #endif
    #if NUM_SPI > 4
        &SPI4_TCR,
    #endif
    #if NUM_SPI > 5
        &SPI5_TCR,
    #endif
    #if NUM_SPI > 6
        &SPI6_TCR,
    #endif
    #if NUM_SPI > 7
        &SPI7_TCR,
    #endif
    #if NUM_SPI > 8
        &SPI8_TCR,
    #endif
    #if NUM_SPI > 9
        &SPI9_TCR,
    #endif
};
static volatile uint32_t *p_pushr[NUM_SPI] = {
    &SPI0_PUSHR,
    #if NUM_SPI > 1
        &SPI1_PUSHR,
    #endif
    #if NUM_SPI > 2
        &SPI2_PUSHR,
    #endif
    #if NUM_SPI > 3
        &SPI3_PUSHR,
    #endif
    #if NUM_SPI > 4
        &SPI4_PUSHR,
    #endif
    #if NUM_SPI > 5
        &SPI5_PUSHR,
    #endif
    #if NUM_SPI > 6
        &SPI6_PUSHR,
    #endif
    #if NUM_SPI > 7
        &SPI7_PUSHR,
    #endif
    #if NUM_SPI > 8
        &SPI8_PUSHR,
    #endif
    #if NUM_SPI > 9
        &SPI9_PUSHR,
    #endif
};
static volatile uint32_t *p_popr[NUM_SPI] = {
    &SPI0_POPR,
    #if NUM_SPI > 1
        &SPI1_POPR,
    #endif
    #if NUM_SPI > 2
        &SPI2_POPR,
    #endif
    #if NUM_SPI > 3
        &SPI3_POPR,
    #endif
    #if NUM_SPI > 4
        &SPI4_POPR,
    #endif
    #if NUM_SPI > 5
        &SPI5_POPR,
    #endif
    #if NUM_SPI > 6
        &SPI6_POPR,
    #endif
    #if NUM_SPI > 7
        &SPI7_POPR,
    #endif
    #if NUM_SPI > 8
        &SPI8_POPR,
    #endif
    #if NUM_SPI > 9
        &SPI9_POPR,
    #endif
};
static volatile uint32_t *p_rser[NUM_SPI] = {
    &SPI0_RSER,
    #if NUM_SPI > 1
        &SPI1_RSER,
    #endif
    #if NUM_SPI > 2
        &SPI2_RSER,
    #endif
    #if NUM_SPI > 3
        &SPI3_RSER,
    #endif
    #if NUM_SPI > 4
        &SPI4_RSER,
    #endif
    #if NUM_SPI > 5
        &SPI5_RSER,
    #endif
    #if NUM_SPI > 6
        &SPI6_RSER,
    #endif
    #if NUM_SPI > 7
        &SPI7_RSER,
    #endif
    #if NUM_SPI > 8
        &SPI8_RSER,
    #endif
    #if NUM_SPI > 9
        &SPI9_RSER,
    #endif
};

void isrSpi(int which) {
    volatile uint32_t *sr    = p_sr     [which];
    volatile uint32_t *mcr   = p_mcr    [which];
    volatile uint32_t *tcr   = p_tcr    [which];
    volatile uint32_t *pushr = p_pushr  [which];
    volatile uint32_t *popr  = p_popr   [which];
    volatile uint32_t *rser  = p_rser   [which];
    MicronSpiState *state    = _spiState[which];
    if(!state) return;
    gpioSetPinOutput(13, 1);
    state->irqCnt++;
    if((*sr) & SPI_SR_TFFF) state->irqFillCnt++;
    if((*sr) & SPI_SR_RFDF) state->irqEmptyCnt++;

    static uint32_t lastT = 0;
    static uint32_t lastC = 0;

    uint32_t now = millis();
    if(now == lastT) {
        lastC++;
        if(lastC >= 100) {
            printf(" *** ISR LOOP, sr=%08X, rser=%08X, buf %04X/%04X %04X/%04X\r\n",
                *sr, *rser,
                state->txbuf.head, state->txbuf.tail,
                state->rxbuf.head, state->rxbuf.tail);
            osBootloader();
        }
    }
    else {
        lastT = now;
        lastC = 0;
    }

    //while tx buffer is not empty, and FIFO is not full,
    //funnel from the buffer into the FIFO.
    while((*sr) & SPI_SR_TFFF //FIFO not full
    && (state->txbuf.head != state->txbuf.tail)) { //buffer not empty
        *pushr = state->txbuf.data[state->txbuf.tail++];
        if(state->txbuf.tail >= SPI_TX_BUFSIZE) state->txbuf.tail = 0;
        *sr |= SPI_SR_TFFF; //clear (remains set if not full)
        state->txCount++;
    }

    if(state->txbuf.head == state->txbuf.tail) {
        //no more to send, so disable Tx FIFO Not Full interrupt
        *rser &= ~SPI_RSER_TFFF_RE;
    }

    //while rx buffer is not full, and FIFO is not empty,
    //funnel from the FIFO into the buffer.
    if((*sr) & SPI_SR_RFDF) {
        while((*sr) & SPI_SR_RXCTR) { //more to receive
            unsigned int next = state->rxbuf.head + 1;
            if(next >= SPI_RX_BUFSIZE) next = 0;
            if(next == state->rxbuf.tail) { //buffer is full
                //disable Rx FIFO Not Empty interrupt if Rx buffer is full.
                //otherwise we get stuck in a loop, being unable to empty the
                //Rx FIFO, and thus triggering the interrupt again before we
                //have any chance to empty the buffer.
                *rser &= ~SPI_RSER_RFDF_RE;
                break;
            }
            state->rxbuf.data[state->rxbuf.head] = *popr;
            state->rxbuf.head = next;
            state->rxCount++;
        }
    }

    //all IRQ bits are write-1-to-clear so this will acknowledge them all
    *sr = *sr;

    // gpioSetPinOutput(15, ((*sr) & SPI_SR_TFFF) ? 1 : 0); //XXX remove
    // gpioSetPinOutput(16, ((*sr) & SPI_SR_RFDF) ? 1 : 0);
    // gpioSetPinOutput(17, ((*sr) & SPI_SR_TCF ) ? 1 : 0);
    // gpioSetPinOutput(18, ((*sr) & SPI_SR_EOQF) ? 1 : 0);
    // gpioSetPinOutput(19, ((*sr) & SPI_SR_TFUF) ? 1 : 0);
    // gpioSetPinOutput(20, ((*sr) & SPI_SR_RFOF) ? 1 : 0);
    gpioSetPinOutput(13, 0);
}

ISRFUNC void isrSpi0(void) {
    isrSpi(0);
}
#if NUM_SPI > 1
    ITCRFUNC void isrSpi1(void) {
        isrSpi(1);
    }
#endif
#if NUM_SPI > 2
    ITCRFUNC void isrSpi2(void) {
        isrSpi(2);
    }
#endif
#if NUM_SPI > 3
    ITCRFUNC void isrSpi3(void) {
        isrSpi(3);
    }
#endif
#if NUM_SPI > 4
    ITCRFUNC void isrSpi4(void) {
        isrSpi(4);
    }
#endif
#if NUM_SPI > 5
    ITCRFUNC void isrSpi5(void) {
        isrSpi(5);
    }
#endif
#if NUM_SPI > 6
    ITCRFUNC void isrSpi6(void) {
        isrSpi(6);
    }
#endif
#if NUM_SPI > 7
    ITCRFUNC void isrSpi7(void) {
        isrSpi(7);
    }
#endif
#if NUM_SPI > 8
    ITCRFUNC void isrSpi8(void) {
        isrSpi(8);
    }
#endif
#if NUM_SPI > 9
    ITCRFUNC void isrSpi9(void) {
        isrSpi(9);
    }
#endif

#ifdef __cplusplus
    } //extern "C"
#endif
