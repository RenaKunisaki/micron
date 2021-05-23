#ifndef _MICRON_HAL_CLOCKS_H_
#define _MICRON_HAL_CLOCKS_H_

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
    MICRON_CLOCK_CPU = 0,
    MICRON_CLOCK_BUS,
    MICRON_NUM_CLOCKS
} MicronClockEnum;

//clocks.c
int osGetClockSpeed(MicronClockEnum clock, uint64_t *out);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_HAL_CLOCKS_H_
