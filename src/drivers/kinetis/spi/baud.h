#ifndef _MICRON_KINETIS_SPI_BAUD_H_
#define _MICRON_KINETIS_SPI_BAUD_H_

#ifdef __cplusplus
	extern "C" {
#endif

int kinetis_spiGetParamsForBaud(uint32_t baud, uint32_t *outPBR,
uint32_t *outBR, uint32_t *outDBR, uint32_t *outActualBaud);

#ifdef __cplusplus
    } //extern "C"
#endif

#endif //_MICRON_KINETIS_SPI_BAUD_H_
