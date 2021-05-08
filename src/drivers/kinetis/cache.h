#ifndef _MICRON_CACHE_H_
#define _MICRON_CACHE_H_

#ifdef __cplusplus
	extern "C" {
#endif


/** Instruction Synchronization Barrier flushes the pipeline in the processor,
 *  so that all instructions following the ISB are fetched from cache or
 *  memory, after the instruction has been completed.
 */
INLINE void InstructionSync() {
	__asm__ volatile("isb");
}

/** Wait for all memory accesses to complete.
 */
INLINE void DataSync() {
	__asm__ volatile("dsb");
}

/* Ensures the apparent order of the explicit memory operations before
 * and after the instruction, without ensuring their completion.
 */
INLINE void MemoryBarrier() {
	__asm__ volatile("dmb");
}


#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_CACHE_H_
