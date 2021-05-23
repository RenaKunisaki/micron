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

int kinetis_spiWriteDummy(uint32_t port, uint32_t data, uint32_t timeout) {
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
    return 0;
}

int kinetis_spiWrite(uint32_t port, uint32_t data, bool cont, uint32_t timeout){
    uint32_t t = millis() + timeout;
    uint32_t pcsbits = pcs[port] << 16;
    uint32_t b = data & 0xFFFF;

    //wait if FIFO full (TXCTR > 3)
    while(((SPI0_SR) & (15 << 12)) > (3 << 12)) {
        if(millis() >= t) return -ETIMEDOUT;
        idle();
    }

    if(pcsbits) {
        SPI0_PUSHR = b | pcsbits | (cont ? SPI_PUSHR_CONT : 0);
    } else {
        *regCS[port] = 0;
        SPI0_SR = SPI_SR_EOQF;
        SPI0_PUSHR = b | (cont ? 0 : SPI_PUSHR_EOQ) | SPI_PUSHR_CONT;
        if(cont) {
            while(((SPI0_SR) & (15 << 12)) > (3 << 12)) {
                if(millis() >= t) return -ETIMEDOUT;
                idle();
            }
        } else {
            while(!(SPI0_SR & SPI_SR_EOQF)) {
                if(millis() >= t) return -ETIMEDOUT;
                idle();
            }
            *regCS[port] = 1;
        }
    }
    return 0;
}

int kinetis_spiRead(uint32_t port, uint32_t *out, uint32_t timeout) {
    uint32_t t = millis() + timeout;
    //Wait for receive
    if(regCS[port]) *regCS[port] = 0;
    while((SPI0_SR & (15 << 4)) == 0) { //RXCTR == 0
        if(millis() >= t) return -ETIMEDOUT;
        idle();
    }
    *out = SPI0_POPR;
    if(regCS[port]) *regCS[port] = 1;
    return 0;
}


int kinetis_spiWaitTxDone(uint32_t port, uint32_t timeout) {
    //Wait until transmission is finished
    uint32_t t = millis() + timeout;
    while(!(SPI0_SR & SPI_SR_TCF)) {
        if(millis() >= t) return -ETIMEDOUT;
        idle();
    }
    SPI0_SR = SPI_SR_TCF; //clear
    return 0;
}

int kinetis_spiClear(uint32_t port) {
    bool paused = kinetis_spiPause(port, true); //required to change other bits
    SPI0_MCR |= SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF;
    if(!paused) kinetis_spiPause(port, false);
    return 0;
}

#ifdef __cplusplus
    } //extern "C"
#endif
