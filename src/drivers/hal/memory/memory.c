#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

int osGetMemorySize(MicronMemEnum mem, uint64_t *out) {
    #if defined(MCU_BASE_KINETIS)
        //XXX move to kinetis/memory
        switch(mem) {
            case MICRON_MEM_MAIN_RAM: {
                uint32_t ramsize = (SIM_SOPT1 >> 12) & 0xF;
                switch(ramsize) {
                    case 1: *out =  8 * 1024; return 0;
                    case 3: *out = 16 * 1024; return 0;
                    case 5: *out = 32 * 1024; return 0;
                    case 7: *out = 64 * 1024; return 0;
                    default: return -ENOSYS;
                }
            }

            case MICRON_MEM_MAIN_ROM: {
                *out = 256 * 1024; //XXX don't hardcode
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
