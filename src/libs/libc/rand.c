#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

//C standard rand() implementation.
//not great, but good.
//RAND_MAX assumed to be 32767.

static unsigned long int next = 1;

int rand(void) {
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % (RAND_MAX+1);
}

void srand(unsigned int seed) {
    next = seed;
}

#ifdef __cplusplus
	} //extern "C"
#endif
