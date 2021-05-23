#ifndef _MICRON_HAL_MEMORY_H_
#define _MICRON_HAL_MEMORY_H_

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
    MICRON_MEM_MAIN_RAM = 0,
    MICRON_MEM_MAIN_ROM,
    MICRON_NUM_MEMORIES
} MicronMemEnum;

//memory.c
int osGetMemorySize(MicronMemEnum mem, uint64_t *out);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_HAL_MEMORY_H_
