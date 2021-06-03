#ifndef _MICRON_SPI_H_
#define _MICRON_SPI_H_

#ifdef __cplusplus
	extern "C" {
#endif

//print each sent/received byte to serial
#define SPI_PRINT_DATA 0

//missing from kinetis.h
#define SPI_SR_TXCTR ((uint32_t)(0xF << 12))
#define SPI_SR_RXCTR ((uint32_t)(0xF <<  4))

//isr.c
extern uint8_t spiInterruptPriority;
extern uint32_t _spi_rxOverflowCount;

//spi.c
int kinetis_spiInit(uint32_t port, uint8_t pinCS, uint32_t speed,
    MicronSpiModeEnum mode);
int kinetis_spiShutdown(uint32_t port);
int kinetis_spiPause(uint32_t port, bool pause);
int kinetis_spiSetMode(uint32_t port, MicronSpiModeEnum mode);
int kinetis_spiSetSpeed(uint32_t port, uint32_t speed);
int kinetis_spiSetFrameSize(uint32_t port, uint32_t size);
int kinetis_spiWriteDummy(uint32_t port, uint32_t data, uint32_t count, bool cs);
int kinetis_spiWrite(uint32_t port, const void *data, uint32_t len, bool cont);
int kinetis_spiRead(uint32_t port, void *out, uint32_t len, uint32_t timeout);
int kinetis_spiWaitTxDone(uint32_t port, uint32_t timeout);
int kinetis_spiClear(uint32_t port);

#ifdef __cplusplus
    } //extern "C"
#endif

#endif //_MICRON_SPI_H_
