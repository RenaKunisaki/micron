#ifndef _MICRON_HAL_SPI_H_
#define _MICRON_HAL_SPI_H_

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
    //SPI standard modes
    SPI_MODE_0 = 0, //Clock active high, change on falling edge (normal mode)
    SPI_MODE_1 = 1, //Clock active high, change on rising  edge
    SPI_MODE_2 = 2, //Clock active low,  change on falling edge
    SPI_MODE_3 = 3, //Clock active low,  change on rising  edge
    //OR those with any of these
    SPI_MODE_SLAVE = BIT(2), //slave mode
    SPI_MODE_CONT_SCK = BIT(3), //continuous clock
} MicronSpiModeEnum;

int spiInit(uint32_t port, uint32_t pinCS, uint32_t speed, MicronSpiModeEnum mode);
int spiPause(uint32_t port, bool pause);
int spiSetMode(uint32_t port, MicronSpiModeEnum mode);
int spiSetSpeed(uint32_t port, uint32_t speed);
int spiSetFrameSize(uint32_t port, uint32_t size);
int spiWrite(uint32_t port, uint32_t data, bool cont, uint32_t timeout);
int spiWriteDummy(uint32_t port, uint32_t data, uint32_t timeout);
int spiRead(uint32_t port, uint32_t *out, uint32_t timeout);
int spiWaitTxDone(uint32_t port, uint32_t timeout);
int spiClear(uint32_t port);

#ifdef __cplusplus
    } //extern "C"
#endif

#endif //_MICRON_HAL_SPI_H_
