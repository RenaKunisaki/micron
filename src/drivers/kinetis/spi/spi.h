#ifndef _MICRON_SPI_H_
#define _MICRON_SPI_H_

#ifdef __cplusplus
	extern "C" {
#endif

int kinetis_spiInit(uint32_t port, uint8_t pinCS, uint32_t speed,
    MicronSpiModeEnum mode);
int kinetis_spiPause(uint32_t port, bool pause);
int kinetis_spiSetMode(uint32_t port, MicronSpiModeEnum mode);
int kinetis_spiSetSpeed(uint32_t port, uint32_t speed);
int kinetis_spiSetFrameSize(uint32_t port, uint32_t size);
int kinetis_spiWriteDummy(uint32_t port, uint32_t data, uint32_t count);
int kinetis_spiWrite(uint32_t port, const void *data, uint32_t len, bool cont);
int kinetis_spiRead(uint32_t port, void *out, uint32_t len, uint32_t timeout);
int kinetis_spiWaitTxDone(uint32_t port, uint32_t timeout);
int kinetis_spiClear(uint32_t port);

#ifdef __cplusplus
    } //extern "C"
#endif

#endif //_MICRON_SPI_H_
