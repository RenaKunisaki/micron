//SPI driver
#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

static uint8_t pcs;
static volatile uint32_t *reg;

int spiBegin(uint8_t pin, uint32_t speed, uint32_t mode) {
    mode <<= 2;
    uint32_t ctar = speed;
    SIM_SCGC6 |= SIM_SCGC6_SPI0; //turn on SPI
    if (mode & 0x08) ctar |= SPI_CTAR_CPOL; //clock active low
    if (mode & 0x04) {
        ctar |= SPI_CTAR_CPHA; //clock phase: change on leading edge
        ctar |= (ctar & 0x0F) << 8;
    } else {
        ctar |= (ctar & 0x0F) << 12;
    }
    SPI0_CTAR0 = ctar | SPI_CTAR_FMSZ(7); //frame size
    SPI0_CTAR1 = ctar | SPI_CTAR_FMSZ(15);
    SPI0_CTAR0_SLAVE = SPI_CTAR_FMSZ(3);
    SPI0_MCR = //SPI_MCR_MSTR | //master mode
        //SPI_MCR_CONT_SCKE | //continuous clock
        //SPI_MCR_MDIS | //module disable
        SPI_MCR_HALT | //stop transfers
        SPI_MCR_PCSIS(0x1F); //inactive high for all PCS

    switch(pin) {
        case 10: pcs = 0x01; break; //PTC4
        case  2: pcs = 0x01; break; //PTD0
        case  9: pcs = 0x02; break; //PTC3
        case  6: pcs = 0x02; break; //PTD4
        case 20: pcs = 0x04; break; //PTD5
        case 23: pcs = 0x04; break; //PTC2
        case 21: pcs = 0x08; break; //PTD6
        case 22: pcs = 0x08; break; //PTC1
        case 15: pcs = 0x10; break; //PTC0
        default: pcs = 0; gpioSetPinMode(pin, PIN_MODE_OUTPUT);
    }

    reg = (volatile uint32_t*)&PDOR(pin);
    if(pcs) PCR(pin) = PORT_PCR_MUX(2);

    CORE_PIN11_CONFIG = PORT_PCR_DSE | PORT_PCR_MUX(2); // DOUT/MOSI = 11 (PTC6)
    CORE_PIN12_CONFIG = PORT_PCR_MUX(2);  // DIN/MISO = 12 (PTC7)
    CORE_PIN13_CONFIG = PORT_PCR_DSE | PORT_PCR_MUX(2); // SCK = 13 (PTC5)
    //SPI0_MCR = (SPI0_MCR & ~SPI_MCR_HALT) | SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF;

    return 0;
}

int spiChangeSpeed(uint32_t speed, uint32_t mode) {
    //XXX mode
    uint32_t ctar = speed;
    SPI0_CTAR0 = ctar | SPI_CTAR_FMSZ(7); //frame size
    SPI0_CTAR1 = ctar | SPI_CTAR_FMSZ(15);
    return 0;
}

int spiWrite(uint32_t b, uint32_t cont, uint32_t timeout) {
    uint32_t t = millis() + timeout;
    uint32_t pcsbits = pcs << 16;
    if (pcsbits) {
        SPI0_PUSHR = (b&0xFF)|pcsbits|(cont ? SPI_PUSHR_CONT : 0);
        //wait if FIFO full
        while(((SPI0_SR) & (15 << 12)) > (3 << 12)) {
            if(millis() >= t) return -ETIMEDOUT;
            idle();
        }
    } else {
        *reg = 0;
        SPI0_SR = SPI_SR_EOQF;
        SPI0_PUSHR = (b&0xFF)|(cont ? 0 : SPI_PUSHR_EOQ)|SPI_PUSHR_CONT;
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
            *reg = 1;
        }
    }
    return 0;
}

int spiRead(uint32_t timeout, uint32_t *out) {
    uint32_t t = millis() + timeout;
    //Wait for receive
    if(reg) *reg = 0;
    while((SPI0_SR & (15 << 4)) == 0) { //RXCTR == 0
        if(millis() >= t) return -ETIMEDOUT;
        idle();
    }
    *out = SPI0_POPR;
    if(reg) *reg = 1;
    return 0;
}


int spiWaitTxDone(uint32_t timeout) {
    //Wait until transmission is finished
    uint32_t t = millis() + timeout;
    while(!(SPI0_SR & SPI_SR_TCF)) {
        if(millis() >= t) return -ETIMEDOUT;
        idle();
    }
    SPI0_SR = SPI_SR_TCF; //clear
    return 0;
}

void spiClear() {
    SPI0_MCR = /*SPI_MCR_MSTR |*/ SPI_MCR_PCSIS(0x1F) | SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF;
}

#ifdef __cplusplus
    } //extern "C"
#endif
