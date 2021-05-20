#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

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
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiInit(port, pinCS, speed, mode);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
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

int spiWriteDummy(uint32_t port, uint32_t data, uint32_t timeout) {
    /** Send a dummy frame to SPI.
     *  @param port Which SPI port to use.
     *  @param data Data to send.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or a negative error code on failure.
     *  @note This is the same as `spiWrite`, but does not assert CS.
     */
    if(port > NUM_SPI) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiWriteDummy(port, data, timeout);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int spiWrite(uint32_t port, uint32_t data, bool cont, uint32_t timeout) {
    /** Send a frame to SPI.
     *  @param port Which SPI port to use.
     *  @param data Data to send.
     *  @param cont Whether to keep CS asserted.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or a negative error code on failure.
     */
    if(port > NUM_SPI) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiWrite(port, data, cont, timeout);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int spiRead(uint32_t port, uint32_t *out, uint32_t timeout) {
    /** Receive a frame from SPI.
     *  @param port Which SPI port to use.
     *  @param out Destination buffer.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or a negative error code on failure.
     *  @note The amount of data written to `out` depends on the frame size.
     */
    if(port > NUM_SPI) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_spiRead(port, out, timeout);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
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
