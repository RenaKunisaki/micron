#ifndef _MICRON_HAL_GPIO_H_
#define _MICRON_HAL_GPIO_H_

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
    PIN_MODE_DISABLED,
    PIN_MODE_INPUT,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_INPUT_PULLDOWN,
    PIN_MODE_OUTPUT,
} PinMode;

typedef enum {
    PIN_SLEW_SLOW,
    PIN_SLEW_FAST,
} PinSlewRate;

typedef enum {
    PIN_DRIVE_STRENGTH_LOW,
    PIN_DRIVE_STRENGTH_HIGH,
} PinDriveStrength;

//gpio.c
int gpioSetPinMode(uint32_t pin, PinMode mode);
int gpioSetPinSlewRate(uint32_t pin, PinSlewRate rate);
int gpioSetPinDriveStrength(uint32_t pin, PinDriveStrength strength);
int gpioSetPinOutput(uint32_t pin, bool high);
int gpioGetPinInput(uint32_t pin);

//XXX interrupts, analog

//compatibility
#define digitalWrite gpioSetPinOutput
#define digitalRead gpioGetPinInput

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_HAL_GPIO_H_
