#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

int crcHwInitCustom(uint32_t port, uint32_t flags, uint32_t poly, uint32_t seed) {
    /** Initialize the hardware CRC module for a custom CRC type.
     *  @param port Which hardware CRC module to use.
     *  @param flags Some of CRC_* (see crc.h).
     *  @param poly Polynomial to use.
     *  @param seed Initial seed.
     *  @return 0 on success, or negative error code.
     */
    if(port > NUM_HW_CRC) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_crcHwInitCustom(flags, poly, seed);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int crcHwInit(uint32_t port, CRC_TYPE type) {
    /** Initialize the hardware CRC module and configure for specified CRC type.
     *  @param port Which hardware CRC module to use.
     *  @param type CRC type.
     *  @return 0 on success, or negative error code.
     */
    if(port > NUM_HW_CRC) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_crcHwInit(type);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int crcHwShutdown(uint32_t port) {
    /** Shut down the hardware CRC module.
     *  @param port Which hardware CRC module to use.
     *  @return 0 on success, or negative error code.
     */
    if(port > NUM_HW_CRC) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_crcHwShutdown();

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int crcHwInput8(uint32_t port, uint8_t data) {
    /** Feed a uint8_t to the hardware CRC module.
     *  @param port Which hardware CRC module to use.
     *  @param data Data to feed.
     *  @return 0 on success, or negative error code.
     */
    if(port > NUM_HW_CRC) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_crcHwInput8(data);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int crcHwInput16(uint32_t port, uint16_t data) {
    /** Feed a uint16_t to the hardware CRC module.
     *  @param port Which hardware CRC module to use.
     *  @param data Data to feed.
     *  @return 0 on success, or negative error code.
     */
    if(port > NUM_HW_CRC) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_crcHwInput16(data);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int crcHwInput32(uint32_t port, uint32_t data) {
    /** Feed a uint32_t to the hardware CRC module.
     *  @param port Which hardware CRC module to use.
     *  @param data Data to feed.
     *  @return 0 on success, or negative error code.
     */
    if(port > NUM_HW_CRC) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_crcHwInput32(data);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int crcHwEnd8(uint32_t port, uint8_t *out) {
    /** Read 8-bit result from hardware CRC module.
     *  @param port Which hardware CRC module to use.
     *  @param out Destination to store result into.
     *  @return 0 on success, or negative error code.
     */
    if(port > NUM_HW_CRC) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_crcHwEnd8(out);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int crcHwEnd16(uint32_t port, uint16_t *out) {
    /** Read 16-bit result from hardware CRC module.
     *  @param port Which hardware CRC module to use.
     *  @param out Destination to store result into.
     *  @return 0 on success, or negative error code.
     */
    if(port > NUM_HW_CRC) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_crcHwEnd16(out);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int crcHwEnd32(uint32_t port, uint32_t *out) {
    /** Read 32-bit result from hardware CRC module.
     *  @param port Which hardware CRC module to use.
     *  @param out Destination to store result into.
     *  @return 0 on success, or negative error code.
     */
    if(port > NUM_HW_CRC) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_crcHwEnd32(out);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int crcTest(void) {
    /** Internal function to test CRC computation.
     *  @return 0 on success, or negative error code.
     *  @note May print diagnostics to stdout/stderr.
     */
    //XXX test software functions
    #if defined(MCU_BASE_KINETIS)
        return kinetis_crcTest();

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

#ifdef __cplusplus
	} //extern "C"
#endif
