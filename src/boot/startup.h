#ifndef _MICRON_STARTUP_H_
#define _MICRON_STARTUP_H_

#include <micron.h>

#ifdef __cplusplus
	extern "C" {
#endif

extern int main(void);
void ResetHandler(void);


// Automatically initialize the RTC.  When the build defines the compile
// time, and the user has added a crystal, the RTC will automatically
// begin at the time of the first upload.
#ifndef TIME_T
#define TIME_T 0
#endif


#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_STARTUP_H_
