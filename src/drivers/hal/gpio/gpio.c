#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

int gpioSetPinMode(uint32_t pin, PinMode mode) {
    /** Set mode of a GPIO pin.
     *  @param pin Which pin.
     *  @param mode Which mode to set.
     *  @return 0 on success, or negative error code.
     */
    if(pin > CORE_NUM_DIGITAL) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_gpioSetPinMode(pin, mode);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int gpioSetPinSlewRate(uint32_t pin, PinSlewRate rate) {
    /** Set slew rate of a GPIO pin.
     *  @param pin Which pin.
     *  @param rate Which rate.
     *  @return 0 on success, or negative error code.
     */
    if(pin > CORE_NUM_DIGITAL) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_gpioSetPinSlewRate(pin, rate);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int gpioSetPinDriveStrength(uint32_t pin, PinDriveStrength strength) {
    /** Set output drive strength of a GPIO pin.
     *  @param pin Which pin.
     *  @param strength Which strength.
     *  @return 0 on success, or negative error code.
     *  @note Behaviour is undefined if the pin isn't set up for output.
     */
    if(pin > CORE_NUM_DIGITAL) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_gpioSetPinDriveStrength(pin, strength);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int gpioSetPinOutput(uint32_t pin, bool high) {
    /** Set output state of a GPIO pin.
     *  @param pin Which pin.
     *  @param high If true, set output high, otherwise, set low.
     *  @return 0 on success, or negative error code.
     *  @note Behaviour is undefined if the pin isn't set up for digital output.
     */
    if(pin > CORE_NUM_DIGITAL) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_gpioSetPinOutput(pin, high);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

int gpioGetPinInput(uint32_t pin) {
    /** Get input state of a GPIO pin.
     *  @param pin Which pin.
     *  @return Input state (0 or 1) on success, or negative error code.
     *  @note Behaviour is undefined if the pin isn't set up for digital input.
     */
    if(pin > CORE_NUM_DIGITAL) return -ENODEV;
    #if defined(MCU_BASE_KINETIS)
        return kinetis_gpioGetPinInput(pin);

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

#ifdef __cplusplus
	} //extern "C"
#endif
