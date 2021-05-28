#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

static volatile uint32_t *p_sr[NUM_SPI] = {
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

void isrSpi(int which) {
    volatile uint32_t *sr    = p_sr     [which];
    volatile uint32_t *mcr   = p_mcr    [which];
    volatile uint32_t *tcr   = p_tcr    [which];
    volatile uint32_t *pushr = p_pushr  [which];
    volatile uint32_t *popr  = p_popr   [which];
    MicronSpiState *state    = _spiState[which];
    if(!state) return;

    //while tx buffer is not empty, and FIFO is not full,
    //funnel from the buffer into the FIFO.
    while(state->txbuf.head != state->txbuf.tail
    && ((*sr) & SPI_SR_TFFF)) {
        *pushr = val = state->txbuf.data[state->txbuf.head++];
        if(state->txbuf.head >= SPI_TX_BUFSIZE) state->txbuf.head = 0;
    }

    //while rx buffer is not full, and FIFO is not empty,
    //funnel from the FIFO into the buffer.
    while((*sr) & SPI_SR_RFDF) {
        unsigned int next = state->rxbuf.head + 1;
        if(next >= SPI_RX_BUFSIZE) next = 0;
        if(next == state->rxbuf.tail) break; //buffer full
        state->rxbuf.data[state->rxbuf.head] = *popr;
        state->rxbuf.head = next;
    }
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
