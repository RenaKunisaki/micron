//SPI driver
#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>
#include "baud.h"

static uint8_t pcs[NUM_SPI];
static volatile uint32_t *regCS[NUM_SPI];

int kinetis_spiInit(uint32_t port, uint8_t pinCS, uint32_t speed,
MicronSpiModeEnum mode) {
    uint32_t ctar = speed;
    SIM_SCGC6 |= SIM_SCGC6_SPI0; //turn on SPI
    if (mode & 0x02) ctar |= SPI_CTAR_CPOL; //clock active low
    if (mode & 0x01) {
        ctar |= SPI_CTAR_CPHA; //clock phase: change on leading edge
        ctar |= (ctar & 0x0F) << 8; //wtf?
        //this is extracting BR and copying it to ASC... why?
        //ASC is After SCK delay scaler
    } else {
        //this is extracting BR and copying it to CSSCK (PCS to SCK delay scaler)
        ctar |= (ctar & 0x0F) << 12;
    }

    SPI0_MCR = SPI_MCR_HALT | //stop transfers
        //SPI_MCR_MDIS | //module disable
        //SPI_MCR_CONT_SCKE | //continuous clock
        SPI_MCR_PCSIS(0x1F) | //inactive high for all PCS
        SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF; //clear FIFOs
    if(mode & SPI_MODE_SLAVE) {
        SPI0_CTAR0_SLAVE = SPI_CTAR_FMSZ(7);
    }
    else {
        SPI0_MCR |= SPI_MCR_MSTR;
        SPI0_CTAR0 = ctar | SPI_CTAR_FMSZ(7); //default frame sizes (#bits - 1)
        SPI0_CTAR1 = ctar | SPI_CTAR_FMSZ(7);
    }

    switch(pinCS) {
        case 10: pcs[port] = 0x01; break; //PTC4
        case  2: pcs[port] = 0x01; break; //PTD0
        case  9: pcs[port] = 0x02; break; //PTC3
        case  6: pcs[port] = 0x02; break; //PTD4
        case 20: pcs[port] = 0x04; break; //PTD5
        case 23: pcs[port] = 0x04; break; //PTC2
        case 21: pcs[port] = 0x08; break; //PTD6
        case 22: pcs[port] = 0x08; break; //PTC1
        case 15: pcs[port] = 0x10; break; //PTC0
        //we can use any other digital output as CS but we'll
        //need to drive it manually.
        default: pcs[port] = 0; gpioSetPinMode(pinCS, PIN_MODE_OUTPUT);
    }

    regCS[port] = (volatile uint32_t*)&PDOR(pinCS);
    if(pcs[port]) PCR(pinCS) = PORT_PCR_MUX(2);

    CORE_PIN11_CONFIG = PORT_PCR_DSE | PORT_PCR_MUX(2); // DOUT/MOSI = 11 (PTC6)
    CORE_PIN12_CONFIG = PORT_PCR_MUX(2);  // DIN/MISO = 12 (PTC7)
    //CORE_PIN13_CONFIG = PORT_PCR_DSE | PORT_PCR_MUX(2); // SCK = 13 (PTC5)
    //XXX allow to specify this
    CORE_PIN14_CONFIG = PORT_PCR_DSE | PORT_PCR_MUX(2); // SCK = 13 (PTC5)
    //SPI0_MCR = (SPI0_MCR & ~SPI_MCR_HALT) | SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF;

    int err = kinetis_spiSetSpeed(port, speed);
    if(err) return err;

    err = kinetis_spiSetMode(port, mode);
    if(err) return err;

    //enable interrupt
    NVIC_SET_PRIORITY(IRQ_SPI0 + port, spiInterruptPriority);
	NVIC_ENABLE_IRQ  (IRQ_SPI0 + port);
    //XXX only enable TFFF when we have something to send.
    //otherwise it loops the interrupt forever.
    SPI0_RSER = 0 //SPI_RSER_TCF_RE //Transmission Complete Request Enable
        //| SPI_RSER_TFFF_RE //Tx FIFO Fill Request Enable
        | SPI_RSER_RFDF_RE //Rx FIFO Drain Request Enable
        //| SPI_RSER_EOQF_RE //Finished Request Enabled
        ;

    return 0;
}

int kinetis_spiShutdown(uint32_t port) {
    NVIC_DISABLE_IRQ(INT_SPI0 + port);
    SPI0_MCR |= SPI_MCR_HALT;
    SIM_SCGC6 &= ~SIM_SCGC6_SPI0; //turn off SPI
    return 0;
}

int kinetis_spiPause(uint32_t port, bool pause) {
    bool paused = SPI0_MCR & SPI_MCR_HALT;
    if(pause) SPI0_MCR |= SPI_MCR_HALT;
    else SPI0_MCR &= ~SPI_MCR_HALT;
    return paused;
}

int kinetis_spiSetMode(uint32_t port, MicronSpiModeEnum mode) {
    uint32_t flags = 0;
    if(mode & 1) flags |= SPI_CTAR_CPHA;
    if(mode & 2) flags |= SPI_CTAR_CPOL;

    bool paused = kinetis_spiPause(port, true); //required to change other bits
    if(mode & SPI_MODE_SLAVE) {
        SPI0_MCR &= ~SPI_MCR_MSTR;
        SPI0_CTAR0_SLAVE = (SPI0_CTAR0_SLAVE &
            ~(SPI_CTAR_CPOL | SPI_CTAR_CPHA)) | flags;
    }
    else {
        SPI0_MCR |= SPI_MCR_MSTR;
        uint32_t ctar0 = SPI0_CTAR0 & ~(SPI_CTAR_CPOL | SPI_CTAR_CPHA);
        uint32_t ctar1 = SPI0_CTAR1 & ~(SPI_CTAR_CPOL | SPI_CTAR_CPHA);

        //XXX what's this about?
        if (mode & 0x01) {
            ctar0 |= (ctar0 & 0x0F) << 8;
            ctar1 |= (ctar1 & 0x0F) << 8;
        } else {
            ctar0 |= (ctar0 & 0x0F) << 12;
            ctar1 |= (ctar1 & 0x0F) << 12;
        }

        SPI0_CTAR0 = ctar0 | flags;
        SPI0_CTAR1 = ctar1 | flags;
    }
    if(!paused) kinetis_spiPause(port, false);
    return 0;
}

int kinetis_spiSetSpeed(uint32_t port, uint32_t speed) {

    uint32_t pbr, br, dbr, abr;
    int err = kinetis_spiGetParamsForBaud(speed, &pbr, &br, &dbr, &abr);
    if(err) return err;

    bool paused = kinetis_spiPause(port, true); //required to change other bits
    uint32_t mask = ~(SPI_CTAR_PBR(  3) | SPI_CTAR_BR(15) | SPI_CTAR_DBR);
    uint32_t val  =   SPI_CTAR_PBR(pbr) | SPI_CTAR_BR(br) |
        (dbr ? SPI_CTAR_DBR : 0);
    //printf("SPI: %d Hz => PBR=%d BR=%d DBR=%d = 0x%08X (actual speed %d Hz)\r\n",
    //    speed, pbr, br, dbr, val, abr);

    uint32_t ctar0 = (SPI0_CTAR0 & mask) | val;
    uint32_t ctar1 = (SPI0_CTAR1 & mask) | val;

    /* if(SPI0_MCR & SPI_CTAR_CPHA) {
        ctar0 |= (ctar0 & 0x0F) << 8;
        ctar1 |= (ctar1 & 0x0F) << 8;
    } else {
        ctar0 |= (ctar0 & 0x0F) << 12;
        ctar1 |= (ctar1 & 0x0F) << 12;
    } */

    SPI0_CTAR0 = ctar0;
    SPI0_CTAR1 = ctar1;

    if(!paused) kinetis_spiPause(port, false);
    return 0;
}

int kinetis_spiSetFrameSize(uint32_t port, uint32_t size) {
    bool paused = kinetis_spiPause(port, true); //required to change other bits
    uint32_t mask = ~SPI_CTAR_FMSZ(15);
    if(SPI0_MCR & SPI_MCR_MSTR) {
        SPI0_CTAR0 = (SPI0_CTAR0 & mask) | SPI_CTAR_FMSZ(size - 1);
        SPI0_CTAR1 = (SPI0_CTAR1 & mask) | SPI_CTAR_FMSZ(size - 1);
    }
    else {
        SPI0_CTAR0_SLAVE = (SPI0_CTAR0_SLAVE & mask) | SPI_CTAR_FMSZ(size - 1);
    }
    if(!paused) kinetis_spiPause(port, false);
    return 0;
}

static int _writeTxBuf(MicronSpiState *state, uint32_t data, uint32_t i,
uint32_t len, bool cont, uint32_t pcsbits) {
    //this exists mainly to avoid duplicating most of this code
    //for the cases where frame size > 8 and <= 8.

    if(cont) data |= SPI_PUSHR_CONT;
    data |= pcsbits;
    //if((i+1) == len) data |= SPI_PUSHR_EOQ; //end of queue
    //that halts SPI until we start it again, so let's not...
    state->txBufCnt++;

    int next = state->txbuf.head + 1;
    if(next >= SPI_TX_BUFSIZE) next = 0;
    if(next == state->txbuf.tail) return 0;
    state->txbuf.data[state->txbuf.head] = data;
    state->txbuf.head = next;

    //if Tx FIFO is empty but Tx buffer isn't, enable the "Tx FIFO Not Full"
    //interrupt so that the ISR will begin the transmission.
    //if it's already enabled, this does nothing.
    SPI0_RSER |= SPI_RSER_TFFF_RE;
    SPI0_SR   |= SPI_SR_TFFF;
    return 1;
}

int kinetis_spiWriteDummy(uint32_t port, uint32_t data, uint32_t count,
bool cs) {
    MicronSpiState *state = _spiState[port];
    if(!state) return -EBADFD;
    //irqDisable();

    //if we let SPI run while we're buffering data, then some of it will get
    //put into the FIFO, and some will get put into the buffer, meaning it will
    //get sent out of order.
    bool paused = spiPause(port, true);
    uint32_t pcsbits = pcs[port] << 16;
    if(!cs) pcsbits = 0;
    uint32_t i = 0;
    while(i < count) {
        //printf("\x1B[36m%02X \x1B[0m", data);
        if(!_writeTxBuf(state, data & 0xFFFF, i, count, false, pcsbits)) break;
        i++;
    }
    if(!paused) spiPause(port, false);
    irqEnable();
    return i;

    /*
    uint32_t t = millis() + timeout;
    uint32_t pcsbits = pcs[port] << 16;
    uint32_t b = data & 0xFFFF;
    if(pcsbits) {
        SPI0_PUSHR = b; //| pcsbits;
        //wait if FIFO full (TXCTR > 3)
        while(((SPI0_SR) & (15 << 12)) > (3 << 12)) {
            if(millis() >= t) return -ETIMEDOUT;
            idle();
        }
    } else {
        SPI0_SR = SPI_SR_EOQF;
        SPI0_PUSHR = b | SPI_PUSHR_EOQ;
        while(!(SPI0_SR & SPI_SR_EOQF)) {
            if(millis() >= t) return -ETIMEDOUT;
            idle();
        }
    }
    */
    return 0;
}

int kinetis_spiWrite(uint32_t port, const void *data, uint32_t len, bool cont) {
    MicronSpiState *state = _spiState[port];
    if(!state) return -EBADFD;
    //printf("spiWrite(%ld, %p, %ld, %d)\r\n", port, data, len, cont);

    uint32_t pcsbits = pcs[port] << 16;
    uint32_t i = 0;
    //irqDisable(); {
        bool paused = spiPause(port, true);
        uint32_t frameSize = (SPI0_CTAR0 & SPI_CTAR_FMSZ(15));
        if(frameSize > (7 << 27)) {
            //SPI_CTAR_FMSZ is bits 27-30, and is the size minus one.
            //so, this is checking if the frame size is > 8 bits, without
            //unnecessary shifting and addition.
            //the maximum supported size is 16 bits, so no further
            //checking is needed.
            const uint16_t *dIn = (const uint16_t*)data;
            while(i < len) {
                //printf("\x1B[32m%04X \x1B[0m", *dIn);
                if(!_writeTxBuf(state, *(dIn++), i, len, cont, pcsbits)) break;
                i++;
            }
        }
        else { //frame size is 8 bits or fewer
            const uint8_t *dIn = (const uint8_t*)data;
            while(i < len) {
                //printf("\x1B[32m%02X \x1B[0m", *dIn);
                if(!_writeTxBuf(state, *(dIn++), i, len, cont, pcsbits)) break;
                i++;
            }
        }

        //DEBUG
        #if 0
            uint32_t ntx = (SPI0_SR & SPI_SR_TXCTR) >> 12;
            const uint32_t *fifo = (const uint32_t*)&SPI0_TXFR0;

            irqEnable();
            printf("wrote %d/%d frames\r\n", i, len);

            printf("TX FIFO[%ld] (h%04X t%04X): ", ntx, state->txbuf.head,
                state->txbuf.tail);
            for(uint32_t n=0; n<ntx; n++) {
                printf("%08X ", fifo[n]);
            }

            printf(" | ");
            int tail = state->txbuf.tail;
            while(tail != state->txbuf.head) {
                printf("%08X ", state->txbuf.data[tail++]);
                if(tail >= SPI_TX_BUFSIZE) tail = 0;
            }

            printf("\r\n");

            irqDisable();
        #endif

        if(!paused) spiPause(port, false);
    //}
    //irqEnable();

    return i;
}

static int _waitForData(MicronSpiState *state, uint32_t limit) {
    SPI0_RSER |= SPI_RSER_RFDF_RE;
    while(state->rxbuf.tail == state->rxbuf.head) {
        if(millis() >= limit) return 0;

        //enable Rx FIFO Not Empty interrupt.
        //ISR will disable it again if buffer is full.
        SPI0_RSER |= SPI_RSER_RFDF_RE;
        irqWait();
    }
    return 1;
}

int kinetis_spiRead(uint32_t port, void *out, uint32_t len, uint32_t timeout) {
    MicronSpiState *state = _spiState[port];
    if(!state) return -EBADFD;

    uint32_t limit = millis() + timeout;
    uint32_t count = 0;

    //assert CS
    //if(regCS[port]) *regCS[port] = 0;

    uint32_t frameSize = (SPI0_CTAR0 & SPI_CTAR_FMSZ(15));
    if(frameSize > (7 << 27)) { // > 8-bit frames
        uint16_t *dOut = (uint16_t*)out;
        while(count < len && state->rxbuf.tail != state->rxbuf.head) {
            if(!_waitForData(state, limit)) break;
            irqDisable(); {
                unsigned int next = state->rxbuf.tail + 1;
                if(next >= SPI_RX_BUFSIZE) next = 0;
                //printf("\x1B[31m%04X \x1B[0m", state->rxbuf.data[state->rxbuf.tail]);
                (*(dOut++)) = state->rxbuf.data[state->rxbuf.tail];
                state->rxbuf.tail = next;
                state->rxBufCnt++;
                count++;
            } irqEnable();
        }
    }
    else { // <= 8-bit frames
        uint8_t *dOut = (uint8_t*)out;
        while(count < len && state->rxbuf.tail != state->rxbuf.head) {
            if(!_waitForData(state, limit)) break;
            irqDisable(); {
                unsigned int next = state->rxbuf.tail + 1;
                if(next >= SPI_RX_BUFSIZE) next = 0;
                //printf("\x1B[31m%02X \x1B[0m", state->rxbuf.data[state->rxbuf.tail]);
                (*(dOut++)) = state->rxbuf.data[state->rxbuf.tail];
                state->rxbuf.tail = next;
                state->rxBufCnt++;
                count++;
            } irqEnable();
        }
    }
    //if(regCS[port]) *regCS[port] = 1;
    return count;
}


int kinetis_spiWaitTxDone(uint32_t port, uint32_t timeout) {
    //Wait until transmission is finished
    MicronSpiState *state = _spiState[port];
    if(!state) return -EBADFD;
    //printf("waitTxDone buf %d %d\r\n",
    //    state->txbuf.head, state->txbuf.tail);

    uint32_t limit = millis() + timeout;

    //wait for tx buffer to be empty
    while(state->txbuf.head != state->txbuf.tail) {
        if(millis() >= limit) {
            //printf("waitTxDone failed 1, buf %d %d\r\n",
            //    state->txbuf.head, state->txbuf.tail);
            return -ETIMEDOUT;
        }
        irqWait();
    }

    //wait for end-of-transmission flag
    //while(!(SPI0_SR & SPI_SR_TCF)) {
    while(SPI0_SR & SPI_SR_TXCTR) {
        if(millis() >= limit) {
            //printf("waitTxDone failed 2, buf %d %d\r\n",
            //    state->txbuf.head, state->txbuf.tail);
            return -ETIMEDOUT;
        }
        irqWait();
    }
    SPI0_SR = SPI_SR_TCF; //clear
    return 0;
}

int kinetis_spiClear(uint32_t port) {
    //XXX do we need this?
    bool paused = kinetis_spiPause(port, true); //required to change other bits
    SPI0_MCR |= SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF;
    if(!paused) kinetis_spiPause(port, false);
    return 0;
}

#ifdef __cplusplus
    } //extern "C"
#endif
