#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

SECTION(".bss") MicronSpiState *_spiState[NUM_SPI];

int spiInit(uint32_t port, uint32_t pinCS, uint32_t speed,
MicronSpiModeEnum mode) {
    /** Set up SPI.
     *  @param port Which SPI port to use.
     *  @param pinCS Which pin to use for CS.
     *  @param speed Baud rate.
     *  @param mode Mode flags.
     *  @return 0 on success, or a negative error code on failure.
     *  @note SPI begins in paused state; use `spiPause` to begin after setting
     *   frame size, etc as needed.
     */
    if(port > NUM_SPI) return -ENODEV;
    if(_spiState[port] != NULL) return 0; //already init

	//init state
	_spiState[port] = (MicronSpiState*)malloc(sizeof(MicronSpiState));
	if(_spiState[port] == NULL) return -ENOMEM;
    memset(_spiState[port], 0, sizeof(MicronSpiState));
    _spiState[port]->pinCS = pinCS;

    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiInit(port, pinCS, speed, mode);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int spiShutdown(uint32_t port) {
    /** Shut down specified SPI, if it's enabled.
     *  @param port Which SPI.
     *  @return 0 on success, or negative error code.
     */
    if(port >= NUM_SPI) return -ENODEV; //No such device
    if(_spiState[port] == NULL) return 0; //already shutdown

    int err = 0;
    #if defined(MCU_BASE_KINETIS)
        err = kinetis_spiShutdown(port);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif

    if(err) return err;
    free(_spiState[port]);
    _spiState[port] = NULL;
    return 0;
}

int spiPause(uint32_t port, bool pause) {
    /** Pause or resume SPI operations.
     *  @param port Which SPI port to use.
     *  @param pause Whether to pause.
     *  @return 1 if previously paused, 0 if not,
     *   or a negative error code on failure.
     */
    if(port > NUM_SPI) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiPause(port, pause);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int spiSetMode(uint32_t port, MicronSpiModeEnum mode) {
    /** Set the mode for SPI.
     *  @param port Which SPI port to use.
     *  @param mode Mode flags.
     *  @return 0 on success, or a negative error code on failure.
     */
    if(port > NUM_SPI) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiSetMode(port, mode);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int spiSetSpeed(uint32_t port, uint32_t speed) {
    /** Set the SPI baud rate.
     *  @param port Which SPI port to use.
     *  @param speed Baud rate.
     *  @return 0 on success, or a negative error code on failure.
     */
    if(port > NUM_SPI) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiSetSpeed(port, speed);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int spiSetFrameSize(uint32_t port, uint32_t size) {
    /** Set the SPI frame size.
     *  @param port Which SPI port to use.
     *  @param size Frame size in bits.
     *  @return 0 on success, or a negative error code on failure.
     */
    if(port > NUM_SPI) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiSetFrameSize(port, size);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int spiWriteDummy(uint32_t port, uint32_t data, uint32_t count, bool cs) {
    /** Send dummy frames to SPI.
     *  @param port Which SPI port to use.
     *  @param data Data to send.
     *  @param count Number of frames to send.
     *  @return Number of frames (not bytes) queued (which may be as low as
     *   zero if the buffer is full), or negative error code.
     *  @note This is the same as `spiWrite`, but the same data is used for
     *   every frame, and you have the option to not assert CS.
     */
    if(port > NUM_SPI) return -ENODEV;
    //printf("spiWriteDummy(0x%X, %d)\r\n", data, count);
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiWriteDummy(port, data, count, cs);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int spiWrite(uint32_t port, const void *data, uint32_t len, bool cont) {
    /** Send data to SPI.
     *  @param port Which SPI port to use.
     *  @param data Data to send.
     *  @param len Number of frames (not bytes) to send.
     *  @param cont Whether to keep CS asserted.
     *  @return Number of frames (not bytes) queued (which may be as low as
     *   zero if the buffer is full), or negative error code.
     */
    if(port > NUM_SPI) return -ENODEV;
    //printf("spiWrite(0x%X, %d)\r\n", data, len);
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiWrite(port, data, len, cont);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int spiWriteBlocking(uint32_t port, const void *data, uint32_t len, bool cont,
uint32_t timeout) {
    //printf("spiWriteBlocking(0x%X, %d)\r\n", data, len);
    const uint8_t *d = (const uint8_t*)data;
    uint32_t i = 0;
    timeout += millis();
    while(millis() < timeout && i < len) {
        int r = spiWrite(port, &d[i], len-i, cont);
        if(r < 0) return r;
        i += r;
        irqWait();
    }
    spiWaitTxDone(port, timeout);
    //printf("spiWriteBlocking %d/%d\r\n", i, len);
    if(i >= len) return len;
    return -ETIMEDOUT;
}

int spiRead(uint32_t port, void *out, uint32_t len, uint32_t timeout) {
    /** Receive data from SPI.
     *  @param port Which SPI port to use.
     *  @param out Destination buffer.
     *  @param len Maximum frames (not bytes) to receive.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return Number of frames (not bytes) received (which may be as low as
     *   zero if the buffer is empty), or negative error code.
     */
    if(port > NUM_SPI) return -ENODEV;
    //printf("spiRead(%d)\r\n", len);
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiRead(port, out, len, timeout);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int spiReadBlocking(uint32_t port, void *out, uint32_t len, uint32_t timeout) {
    //printf("spiReadBlocking(%d)\r\n", len);
    uint8_t *d = (uint8_t*)out;
    uint32_t limit = millis() + timeout;
    uint32_t i = 0;
    while(millis() < limit && i < len) {
        //XXX proper timeout handling
        int r = spiRead(port, &d[i], len-i, timeout);
        if(r < 0) return r;
        i += r;
        irqWait();
    }
    //printf("spiReadBlocking %d/%d\r\n", i, len);
    if(i >= len) return len;
    return -ETIMEDOUT;
}

int spiWaitTxDone(uint32_t port, uint32_t timeout) {
    /** Wait for SPI transmission to finish.
     *  @param port Which SPI port to use.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or a negative error code on failure.
     */
    if(port > NUM_SPI) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiWaitTxDone(port, timeout);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int spiClear(uint32_t port) {
    /** Clear all transmit and receive SPI buffers.
     *  @param port Which SPI port to use.
     *  @return 0 on success, or a negative error code on failure.
     */
    //XXX do something with buffers, if we even need this?
    if(port > NUM_SPI) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiClear(port);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

#ifdef __cplusplus
    } //extern "C"
#endif
