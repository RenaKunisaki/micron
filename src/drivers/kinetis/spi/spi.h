#ifndef _MICRON_SPI_H_
#define _MICRON_SPI_H_

#ifdef __cplusplus
	extern "C" {
#endif

int spiBegin(uint8_t pin, uint32_t speed, uint32_t mode);
int spiChangeSpeed(uint32_t speed, uint32_t mode);
int spiWrite(uint32_t b, uint32_t cont, uint32_t timeout);
int spiRead(uint32_t timeout, uint32_t *out);
int spiWaitTxDone(uint32_t timeout);
void spiClear();

#ifdef __cplusplus
    } //extern "C"
#endif

#endif //_MICRON_SPI_H_
