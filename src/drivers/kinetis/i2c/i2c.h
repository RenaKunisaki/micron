#ifndef _MICRON_I2C_H_
#define _MICRON_I2C_H_

#ifdef __cplusplus
	extern "C" {
#endif

#include "i2c-regs.h"

//XXX improve this.
//see https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf#d2100e3a1310_d2092e71
//have to figure out how to compute that table on the fly from the CPU clock...
#if F_BUS == 60 MHZ
	#define I2C_CLOCK  0x2C
	#define I2C_FILTER 4

#elif F_BUS == 56 MHZ
	#define I2C_CLOCK  0x2B
	#define I2C_FILTER 4

#elif F_BUS == 48 MHZ
	#define I2C_CLOCK  0x27
	#define I2C_FILTER 4

#elif F_BUS == 40 MHZ
	#define I2C_CLOCK  0x29
	#define I2C_FILTER 4

#elif F_BUS == 36 MHZ
	#define I2C_CLOCK  0x28
	#define I2C_FILTER 3

#elif F_BUS == 24 MHZ
	#define I2C_CLOCK  0x1F
	#define I2C_FILTER 2

#elif F_BUS == 16 MHZ
	#define I2C_CLOCK  0x20
	#define I2C_FILTER 1

#elif F_BUS == 8 MHZ
	#define I2C_CLOCK  0x14
	#define I2C_FILTER 1

#elif F_BUS == 4 MHZ
	#define I2C_CLOCK  0x07
	#define I2C_FILTER 1

#elif F_BUS == 2 MHZ
	#define I2C_CLOCK  0x00
	#define I2C_FILTER 1
#endif

//i2c.c
extern i2cRegs_t *i2cRegs[NUM_I2C];

//void i2cDefaultSlaveRx(uint8_t data);
//void i2cSlaveRx(uint8_t data);
//void i2cDefaultSlaveTx(uint8_t data);
//void i2cSlaveTx(uint8_t data);

int kinetis_i2cWaitForBus(uint8_t port, uint32_t timeout);
int kinetis_i2cInit(uint8_t port, int address);
int kinetis_i2cShutdown(uint8_t port);
int kinetis_i2cGetStatus(uint8_t port);
int kinetis_i2cSendByte(uint8_t port, uint8_t data);
int kinetis_i2cEndTx(uint8_t port, uint8_t stop);
int kinetis_i2cRead(micronI2cState *state, uint8_t *buffer, uint32_t length,
    uint32_t timeout);
int kinetis_i2cRequest(micronI2cState *state, uint8_t address, uint32_t length,
    uint8_t stop, uint8_t *buffer, uint32_t timeout);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_I2C_H_
