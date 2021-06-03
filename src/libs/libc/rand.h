#ifndef _MICRON_LIBC_RAND_H_
#define _MICRON_LIBC_RAND_H_

#ifdef __cplusplus
	extern "C" {
#endif

#define RAND_MAX 32767
int rand(void);
void srand(unsigned int seed);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_LIBC_RAND_H_
