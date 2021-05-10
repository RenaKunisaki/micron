#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

MicronI2cState *i2cState[NUM_I2C];

int i2cInit(uint32_t port, int address) {
    /** Initialize specified I2C port with specified address.
     *  @param port Which I2C port to use.
     *  @param address The slave address to respond to, or -1 for none.
     *  @return 0 on success, or a negative error code on failure.
     */
    if(port > NUM_I2C) return -ENODEV;
    if(address >= 0) {
        //sanity check address
        if(address < 8 || address > 0x78) return -EADDRNOTAVAIL;
    }

    //set up state variables
	i2cState[port] = (MicronI2cState*)malloc(sizeof(MicronI2cState));
	if(i2cState[port] == NULL) return -ENOMEM;
	MicronI2cState *state = i2cState[port];
	memset(state, 0, sizeof(MicronI2cState));
    state->port = port;
    if(address >= 0) state->slaveMode = 1;

    #if defined(MCU_BASE_KINETIS)
        return kinetis_i2cInit(port, address);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int i2cShutdown(uint32_t port) {
    /** Shut down specified I2C port, if it's enabled.
     *  @param port Which I2C port to use.
     *  @return 0 on success, or a negative error code on failure.
     */
    if(port >= NUM_I2C) return -ENODEV;
	if(i2cState[port] == NULL) return 0; //already shutdown

    int r = 0;
    #if defined(MCU_BASE_KINETIS)
        r = kinetis_i2cShutdown(port);

    #elif defined(MCU_BASE_IMX)
        r = -ENOSYS; //XXX

    #else
        r = -ENOSYS;
    #endif

    free(i2cState[port]);
	i2cState[port] = NULL;
    return r;
}

int i2cBeginTx(uint32_t port, uint8_t address) {
    /** Initiate a transmission on the I2C bus.
     *  @param port Which I2C port to use.
     *  @param address Destination device address.
     *  @return 0 on success, or a negative error code on failure.
     */
    if(port >= NUM_I2C) return -ENODEV;
	MicronI2cState *state = i2cState[port];
	if(state == NULL) return -EBADFD;
	//state->transmitting = 1;
	state->doRecv = 0;
	state->state  = MICRON_I2C_STATE_TX;

    //Queue one byte: the destination address and R/W flag.
	uint8_t data = address << 1;
	int r = _i2cBufferTxP(state, &data, 1);
    if(!r) return -ENOBUFS; //no buffer space
    return r;
}

int i2cContinueTx(uint32_t port, const void *data, uint32_t len) {
    /** Queue some data to transmit on the I2C bus.
     *  @param port Which I2C port to use.
     *  @param data Data to send.
     *  @param len Number of bytes to send.
     *  @return Number of bytes queued (which may be as few as zero if the
     *   transmit buffer is full), or a negative error code.
     */
    if(port >= NUM_I2C) return -ENODEV;
	MicronI2cState *state = i2cState[port];
	if(state == NULL) return -EBADFD;

	//queue data to send.
	if(state->state == MICRON_I2C_STATE_TX || state->slaveMode) {
		return _i2cBufferTxP(state, data, len);
	}
	//we need to call i2cBeginTx() first.
	return -EDESTADDRREQ;
}

int i2cEndTx(uint32_t port, bool stop) {
    /** Transmit all queued data on the I2C bus.
     *  @param port Which I2C port to use.
     *  @param stop Whether to send a STOP condition after
     *   completing the transmission.
     *  @return 0 on success or a negative error code on failure.
     *
     *  This function actually transmits the data queued by `i2cBeginTx` and
     *  `i2cContinueTx`. It waits for the bus to be ready, makes itself the bus
     *  master, and waits for the transmission to complete (or for an error).
     *
     *  XXX support slave mode, allow to specify timeout
     */
    if(port >= NUM_I2C) return -ENODEV;
	MicronI2cState *state = i2cState[port];
	if(state == NULL) return -EBADFD;

    int err = _i2cWaitForBus(state, millis() + 100);
	if(err) return err;

	state->sendStop = stop;
	state->txbuf.tail = (state->txbuf.tail + 1) % I2C_TX_BUFSIZE;
	uint8_t byte = state->txbuf.data[state->txbuf.tail];

    #if defined(MCU_BASE_KINETIS)
        err = kinetis_i2cSendByte(port, byte);

    #elif defined(MCU_BASE_IMX)
        err = -ENOSYS; //XXX

    #else
        err = -ENOSYS;
    #endif
    if(err) return err;

	//wait for device to respond, which will trigger IRQ to send next byte.
	return _i2cWaitForIdle(state, millis() + 100);
}

int i2cSend(uint32_t port, uint8_t addr, const void *data, uint8_t len) {
    /** Send some data on the I2C bus.
     *  @param port Which I2C port to use.
     *  @param addr Destination address.
     *  @param data Data to send.
     *  @param len Length of data.
     *  @return Number of bytes sent, or negative error code.
     */
    int err = i2cBeginTx(port, addr);
    if(err >  0) err = i2cContinueTx(port, data, len);
    if(err >= 0) err = i2cEndTx(port, true);
    return err;
}

int i2cSendRecv(uint32_t port, uint8_t address, const void *txData,
uint32_t txLen, uint8_t *rxData, uint32_t rxLen, uint32_t timeout) {
    /** Send some data on the I2C bus and receive a response.
     *  @param port Which I2C port to use.
     *  @param addr Destination address.
     *  @param txData Data to send.
     *  @param txLen Length of txData.
     *  @param rxData Buffer to receive response into.
     *  @param rxLen Number of bytes to receive.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return Number of bytes sent, or negative error code.
     */
    int err = i2cSend(port, address, txData, txLen);
    if(err >= 0) err = i2cRequest(port, address, rxLen, 1, rxData, timeout);
    return err;
}

int i2cGetNumRecv(uint32_t port) {
    /** Get the number of bytes queued in the receive buffer.
     *  @param port Which I2C port to use.
     *  @return Number of bytes, or negative error code.
     */
    if(port >= NUM_I2C) return -ENODEV;
	MicronI2cState *state = i2cState[port];
	if(state == NULL) return -EBADFD;
	return state->rxbuf.idx;
}

int i2cRead(uint32_t port, uint8_t *buffer, uint32_t length, uint32_t timeout) {
    /** Receive some data from I2C bus.
     *  @param port Which I2C port to use.
     *  @param buffer Buffer to receive into.
     *  @param length Number of bytes to receive.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return Number of bytes received, or negative error code.
     */
    if(port >= NUM_I2C) return -ENODEV;
	MicronI2cState *state = i2cState[port];
	if(state == NULL) return -EBADFD;

    state->rxbuf.data = NULL;
	//state->receiving  = 0;
	timeout += millis();

    //set rx buffer
	state->rxbuf.data = buffer;
	state->rxbuf.len  = length;
	state->rxbuf.idx  = 0;
	//state->receiving  = 1;
    state->state = MICRON_I2C_STATE_RX;

    int err = 0;
    #if defined(MCU_BASE_KINETIS)
        err = kinetis_i2cRead(state, buffer, length, timeout);

    #elif defined(MCU_BASE_IMX)
        err = -ENOSYS; //XXX

    #else
        err = -ENOSYS;
    #endif
    if(err) return err;

    //state->receiving = 0;
	state->rxbuf.data = NULL;
	return state->rxbuf.idx;
}

int i2cRequest(uint32_t port, uint8_t address, uint32_t length, uint8_t stop,
uint8_t *buffer, uint32_t timeout) {
    /** Send a request to a device on the I2C bus.
     *  @param port Which I2C port to use.
     *  @param address Destination device address.
     *  @param length Number of bytes to read.
     *  @param stop Whether to send a STOP condition after completing the read.
     *  @param buffer Destination buffer to read into.
     *  @param timeout Maximum number of milliseconds to wait for response.
     *  @return 0 on success or a negative error code on failure.
     *
     * XXX make the parameters/order less silly; support slave mode; don't deadlock
     * if receiving less than expected; return # bytes received
     */
    if(port >= NUM_I2C) return -ENODEV;
	MicronI2cState *state = i2cState[port];
	if(state == NULL) return -EBADFD;
    timeout += millis();
    state->rxbuf.data = NULL;
	//state->receiving  = 0;

    //wait for the bus, then enter master transmit mode
	int err = _i2cWaitForBus(state, timeout);
	if(err) return err;

    //set rx buffer
    state->rxbuf.data = buffer;
    state->rxbuf.len  = length;
    state->rxbuf.idx  = 0;
    state->doRecv     = 0;
    //state->receiving  = 1;

    state->state = MICRON_I2C_STATE_RX;

    #if defined(MCU_BASE_KINETIS)
        return kinetis_i2cRequest(state, address, length, stop, buffer,
            timeout);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}


#ifdef __cplusplus
	} //extern "C"
#endif
