#ifndef _MICRON_MALLOC_H_
#define _MICRON_MALLOC_H_

#ifdef __cplusplus
	extern "C" {
#endif

#include "string.h"
extern size_t __malloc_margin;
extern char *__brkval;

//gcc expects malloc() to return an address aligned to 8 bytes
//(16 on a 64-bit system) and makes optimizations based on that
//assumption, which will cause bugs if it's not met.
//This must be a power of two!
#define MALLOC_ALIGN ((CPU_BITS) >> 2)

WEAK COLD void* on_malloc_fail(size_t len);
MALLOC MUST_CHECK void* malloc(size_t len);
MALLOC MUST_CHECK void* calloc(size_t num, size_t size);
                  void  free(void *p);
       MUST_CHECK void* realloc(void *ptr, size_t len);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_MALLOC_H_
