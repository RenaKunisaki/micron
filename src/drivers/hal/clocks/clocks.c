#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

int osGetClockSpeed(MicronClockEnum clock, uint64_t *out) {
    #if defined(MCU_BASE_KINETIS)
        //XXX move to kinetis/clocks
        switch(clock) {
            case MICRON_CLOCK_CPU: {
                //uint32_t outdiv1 = ((SIM_CLKDIV1 >> 27) & 0xF) + 1;
                //XXX much more complexity involving MCG
                //maybe easier to just record the speeds we set.
                //for now we'll just use the hardcoded constants and
                //assume the speed hasn't been changed.
                *out = F_CPU;
                return 0;
            }

            case MICRON_CLOCK_BUS: {
                //uint32_t outdiv2 = ((SIM_CLKDIV1 >> 23) & 0xF) + 1;
                *out = F_CPU / 2;
                return 0;
            }

            default: return -EINVAL;
        }

    #elif defined(MCU_BASE_IMX)
        return -ENOSYS; //XXX

    #else
        return -ENOSYS;
    #endif
}

#ifdef __cplusplus
	} //extern "C"
#endif
