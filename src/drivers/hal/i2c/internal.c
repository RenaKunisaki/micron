#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

int _i2cBufferTxP(micronI2cState *state, const void *data, uint32_t len) {
    /** Internal function to add some data to an I2C transmit buffer.
     *  @param state I2C state.
     *  @param data Data to send.
     *  @param len Length of data.
     *  @return Number of bytes added to buffer.
     */
	uint8_t *dat = (uint8_t*)data;
	uint32_t i = 0;
	while(i < len) { //while more bytes to send
		uint32_t head = state->txbuf.head;
		uint32_t tail = state->txbuf.tail;
		uint32_t next = (head + 1) % I2C_TX_BUFSIZE;
		if(next == tail) { //buffer is full
			break;
		}
		state->txbuf.data[next] = dat[i++];
		state->txbuf.head = next;
	}
	state->state = MICRON_I2C_STATE_TX;
	return i;
}

int _i2cWaitForBus(micronI2cState *state, uint32_t timeout) {
    /** Internal function to wait for I2C bus ownership.
     *  @param state I2C state.
     *  @param timeout Time at which to stop waiting.
     *  @return 0 on success, or negative error code.
     */
    #if defined(MCU_BASE_KINETIS)
        return kinetis_i2cWaitForBus(state->port, timeout);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int _i2cWaitForIdle(micronI2cState *state, uint32_t timeout) {
    /** Internal function to wait for I2C bus to be idle.
     *  @param state I2C state.
     *  @param timeout Time at which to stop waiting.
     *  @return 0 on success, or negative error code.
     */
	while(state->state != MICRON_I2C_STATE_IDLE) {
		if(millis() >= timeout) return -EBUSY;

        int r = 0;
        #if defined(MCU_BASE_KINETIS)
            r = kinetis_i2cGetStatus(state->port);

        #elif defined(MCU_BASE_IMX)
            r = -ENOSYS; //XXX

        #else
            r = -ENOSYS;
        #endif

        if(r) return r;
	}
	return 0;
}

#ifdef __cplusplus
	} //extern "C"
#endif
