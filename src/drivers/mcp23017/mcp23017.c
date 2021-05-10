#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>
#include "mcp23017.h"

int mcp23017ReadReg(uint8_t port, uint8_t address, uint8_t reg,
uint32_t timeout) {
    //to read a register, send its address and then read one byte.
	int err;
	uint8_t response;
	err = i2cBeginTx   (port, address); if(err < 1) { return err; }
	err = i2cContinueTx(port, &reg, 1); if(err < 1) { return err; }
	err = i2cEndTx     (port, 1);       if(err < 0) { return err; }
	err = i2cRequest   (port, address, sizeof(response), 1, &response, timeout);
	if(err < 0) { return err; }
	return response;
}

int mcp23017ReadRegPair(uint8_t port, uint8_t address, uint8_t reg,
uint32_t timeout) {
    int r1 = mcp23017ReadReg(port, address, reg,   timeout);
    if(r1 < 0) return r1;
    int r2 = mcp23017ReadReg(port, address, reg+1, timeout);
    if(r2 < 0) return r2;
    return (r2 << 8) | r1;
}

int mcp23017WriteReg(uint8_t port, uint8_t address, uint8_t reg, uint8_t data) {
    //to write a register, send its address and data.
	int err;
	uint8_t packet[2] = {reg, data};
	err = i2cBeginTx   (port, address);    if(err < 1) { return err; }
	err = i2cContinueTx(port, &packet, 2); if(err < 1) { return err; }
	err = i2cEndTx     (port, 1);          if(err < 0) { return err; }
	return 0;
}

int mcp23017WriteRegPair(uint8_t port, uint8_t address, uint8_t reg,
uint16_t data) {
    int      err=mcp23017WriteReg(port, address, reg,   data & 0xFF);
    if(!err) err=mcp23017WriteReg(port, address, reg+1, data >> 8);
    return err;
}

/** Set IOCON register.
 *  port: Which I2C port to use.
 *  address: I2C device address of the MCP.
 *  val: Value to set to.
 *  Returns 0 on success or a negative error code on failure.
 */
int mcp23017SetIocon(uint8_t port, uint8_t address, uint8_t val) {
    return mcp23017WriteReg(port, address, MCP23017_REG_IOCON, val);
}

/** Set I/O direction of GPIO pins.
 *  port: Which I2C port to use.
 *  address: I2C device address of the MCP.
 *  dir: Direction bits: 0=output, 1=input
 *    bit  0 = port A pin 1
 *    bit  1 = port A pin 2
 *    bit  7 = port A pin 8
 *    bit  8 = port B pin 1
 *    bit  9 = port B pin 2
 *    bit 15 = port B pin 8
 *  Returns 0 on success or a negative error code on failure.
 */
int mcp23017SetDirection(uint8_t port, uint8_t address, uint16_t dir) {
    return mcp23017WriteRegPair(port, address, MCP23017_REG_IODIRA, dir);
}

/** Set GPIO input polarity.
 *  port: Which I2C port to use.
 *  address: I2C device address of the MCP.
 *  pol: Polarity bits. One per pin, as with SetDirection.
 *    1=invert input, 0=don't.
 *  Returns 0 on success or a negative error code on failure.
 */
int mcp23017SetPolarity(uint8_t port, uint8_t address, uint16_t pol) {
    return mcp23017WriteRegPair(port, address, MCP23017_REG_IPOLA, pol);
}

/** Set GPIO input pullups.
 *  port: Which I2C port to use.
 *  address: I2C device address of the MCP.
 *  pull: Pullup bits. One per pin, as with SetDirection.
 *    1=enable internal pullup resistor, 0=disable pullup.
 *  Returns 0 on success or a negative error code on failure.
 */
int mcp23017SetPullup(uint8_t port, uint8_t address, uint16_t pull) {
    return mcp23017WriteRegPair(port, address, MCP23017_REG_GPPUA, pull);
}

/** Set GPIO interrupt enable flags.
 *  port: Which I2C port to use.
 *  address: I2C device address of the MCP.
 *  enable: Pullup bits. One per pin, as with SetDirection.
 *    1=enable interrupt, 0=disable.
 *  Returns 0 on success or a negative error code on failure.
 */
int mcp23017SetIntEnable(uint8_t port, uint8_t address, uint16_t enable) {
    return mcp23017WriteRegPair(port, address, MCP23017_REG_GPINTENA, enable);
}

/** Set GPIO interrupt default values.
 *  port: Which I2C port to use.
 *  address: I2C device address of the MCP.
 *  val: Default value bits. One per pin, as with SetDirection.
 *  Returns 0 on success or a negative error code on failure.
 */
int mcp23017SetIntDefault(uint8_t port, uint8_t address, uint16_t val) {
    return mcp23017WriteRegPair(port, address, MCP23017_REG_DEFVALA, val);
}

/** Set GPIO interrupt mode.
 *  port: Which I2C port to use.
 *  address: I2C device address of the MCP.
 *  mode: Interrupt mode bits. One per pin, as with SetDirection.
 *    0=interrupt on any change
 *    1=interrupt when different from DEFVAL
 *  Returns 0 on success or a negative error code on failure.
 */
int mcp23017SetIntMode(uint8_t port, uint8_t address, uint16_t val) {
    return mcp23017WriteRegPair(port, address, MCP23017_REG_INTCONA, val);
}

/** Read interrupt flags.
 *  port: Which I2C port to use.
 *  address: I2C device address of the MCP.
 *  timeout: Maximum number of milliseconds to wait for response.
 *  On success, returns the 16 interrupt flags in the lowest bits of the result.
 *  On failure, returns a negative error code.
 */
int mcp23017GetIntFlag(uint8_t port, uint8_t address, uint32_t timeout) {
    return mcp23017ReadRegPair(port, address, MCP23017_REG_INTFA, timeout);
}

/** Read captured pin states at time of interrupt.
 *  port: Which I2C port to use.
 *  address: I2C device address of the MCP.
 *  timeout: Maximum number of milliseconds to wait for response.
 *  On success, returns the 16 values in the lowest bits of the result.
 *  On failure, returns a negative error code.
 */
int mcp23017GetIntValue(uint8_t port, uint8_t address, uint32_t timeout) {
    return mcp23017ReadRegPair(port, address, MCP23017_REG_INTCAPA, timeout);
}

/** Read GPIO states.
 *  port: Which I2C port to use.
 *  address: I2C device address of the MCP.
 *  timeout: Maximum number of milliseconds to wait for response.
 *  On success, returns the 16 pin values in the lowest bits of the result.
 *  On failure, returns a negative error code.
 *  If some pins are configured as outputs, this returns the output state.
 */
int mcp23017Read(uint8_t port, uint8_t address, uint32_t timeout) {
    return mcp23017ReadRegPair(port, address, MCP23017_REG_GPIOA, timeout);
}

/** Write GPIO states.
 *  port: Which I2C port to use.
 *  address: I2C device address of the MCP.
 *  data: Output data.
 *  On success, returns 0. On failure, returns a negative error code.
 *  Bits corresponding to input pins are ignored.
 */
int mcp23017Write(uint8_t port, uint8_t address, uint16_t data) {
    return mcp23017WriteRegPair(port, address, MCP23017_REG_GPIOA, data);
}

#ifdef __cplusplus
	} //extern "C"
#endif
