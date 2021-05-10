#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>
#include "mcp23017.h"

int mcp23017ReadReg(uint8_t port, uint8_t address, uint8_t reg,
uint32_t timeout) {
    /** Read a register from MCP.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param reg Which register to read.
     *  @param timeout Maximum number of milliseconds to wait.
     *  @return Register value on success or a negative error code on failure.
     */
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
    /** Read a pair of registers from MCP.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param reg Which register to read.
     *  @param timeout Maximum number of milliseconds to wait.
     *  @return Register values on success or a negative error code on failure.
     *  @note Assumes MCP is not in banked mode.
     */
    int r1 = mcp23017ReadReg(port, address, reg,   timeout);
    if(r1 < 0) return r1;
    int r2 = mcp23017ReadReg(port, address, reg+1, timeout);
    if(r2 < 0) return r2;
    return (r2 << 8) | r1;
}

int mcp23017WriteReg(uint8_t port, uint8_t address, uint8_t reg, uint8_t data) {
    /** Write to a register on MCP.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param reg Which register to write.
     *  @param data Value to write.
     *  @return 0 on success or a negative error code on failure.
     */
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
    /** Write to a pair of registers on MCP.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param reg Which register to write.
     *  @param data Value to write.
     *  @return 0 on success or a negative error code on failure.
     *  @note Assumes MCP is not in banked mode.
     */
    int      err=mcp23017WriteReg(port, address, reg,   data & 0xFF);
    if(!err) err=mcp23017WriteReg(port, address, reg+1, data >> 8);
    return err;
}

int mcp23017SetIocon(uint8_t port, uint8_t address, uint8_t val) {
    /** Set IOCON register.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param val Value to set to.
     *  @return 0 on success or a negative error code on failure.
     */
    return mcp23017WriteReg(port, address, MCP23017_REG_IOCON, val);
}

int mcp23017SetDirection(uint8_t port, uint8_t address, uint16_t dir) {
    /** Set I/O direction of GPIO pins.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param dir Direction bits: 0=output, 1=input
     *    bit  0 = port A pin 1
     *    bit  1 = port A pin 2
     *    bit  7 = port A pin 8
     *    bit  8 = port B pin 1
     *    bit  9 = port B pin 2
     *    bit 15 = port B pin 8
     *  @return 0 on success or a negative error code on failure.
     */
    return mcp23017WriteRegPair(port, address, MCP23017_REG_IODIRA, dir);
}

int mcp23017SetPolarity(uint8_t port, uint8_t address, uint16_t pol) {
    /** Set GPIO input polarity.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param pol Polarity bits. One per pin, as with SetDirection.
     *    1=invert input, 0=don't.
     *  @return 0 on success or a negative error code on failure.
     */
    return mcp23017WriteRegPair(port, address, MCP23017_REG_IPOLA, pol);
}

int mcp23017SetPullup(uint8_t port, uint8_t address, uint16_t pull) {
    /** Set GPIO input pullups.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param pull Pullup bits. One per pin, as with SetDirection.
     *    1=enable internal pullup resistor, 0=disable pullup.
     *  @return 0 on success or a negative error code on failure.
     */
    return mcp23017WriteRegPair(port, address, MCP23017_REG_GPPUA, pull);
}

int mcp23017SetIntEnable(uint8_t port, uint8_t address, uint16_t enable) {
    /** Set GPIO interrupt enable flags.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param enable Pullup bits. One per pin, as with SetDirection.
     *    1=enable interrupt, 0=disable.
     *  @return 0 on success or a negative error code on failure.
     */
    return mcp23017WriteRegPair(port, address, MCP23017_REG_GPINTENA, enable);
}

int mcp23017SetIntDefault(uint8_t port, uint8_t address, uint16_t val) {
    /** Set GPIO interrupt default values.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param val Default value bits. One per pin, as with SetDirection.
     *  @return 0 on success or a negative error code on failure.
     */
    return mcp23017WriteRegPair(port, address, MCP23017_REG_DEFVALA, val);
}

int mcp23017SetIntMode(uint8_t port, uint8_t address, uint16_t val) {
    /** Set GPIO interrupt mode.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param mode Interrupt mode bits. One per pin, as with SetDirection.
     *    0=interrupt on any change
     *    1=interrupt when different from DEFVAL
     *  @return 0 on success or a negative error code on failure.
     */
    return mcp23017WriteRegPair(port, address, MCP23017_REG_INTCONA, val);
}

int mcp23017GetIntFlag(uint8_t port, uint8_t address, uint32_t timeout) {
    /** Read interrupt flags.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param timeout Maximum number of milliseconds to wait for response.
     *  @return the 16 interrupt flags in the lowest bits of the result,
     *   or a negative error code.
     */
    return mcp23017ReadRegPair(port, address, MCP23017_REG_INTFA, timeout);
}

int mcp23017GetIntValue(uint8_t port, uint8_t address, uint32_t timeout) {
    /** Read captured pin states at time of interrupt.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param timeout Maximum number of milliseconds to wait for response.
     *  @return the 16 values in the lowest bits of the result,
     *   or returns a negative error code.
     */
    return mcp23017ReadRegPair(port, address, MCP23017_REG_INTCAPA, timeout);
}

int mcp23017Read(uint8_t port, uint8_t address, uint32_t timeout) {
    /** Read GPIO states.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param timeout Maximum number of milliseconds to wait for response.
     *  @return the 16 pin values in the lowest bits of the result,
     *   or a negative error code.
     *  @note If some pins are configured as outputs, this returns the
     *   output state.
     */
    return mcp23017ReadRegPair(port, address, MCP23017_REG_GPIOA, timeout);
}

int mcp23017Write(uint8_t port, uint8_t address, uint16_t data) {
    /** Write GPIO states.
     *  @param port Which I2C port to use.
     *  @param address I2C device address of the MCP.
     *  @param data Output data.
     *  @return 0 on success, or a negative error code.
     *  @note Bits corresponding to input pins are ignored.
     */
    return mcp23017WriteRegPair(port, address, MCP23017_REG_GPIOA, data);
}

#ifdef __cplusplus
	} //extern "C"
#endif
