#ifndef _MICRON_H_
#define _MICRON_H_

//use these as a suffix, e.g.
//#if F_CPU == 72 MHZ
//if using them in complex expressions, keep in mind how they'll expand,
//and use parentheses as needed to ensure the result is what you intend.
#define KHZ * 1000
#define MHZ * 1000000

#ifndef BIT
	#define BIT(n) (1 << (n))
#endif

//XXX pull these addresses from link script.
#define ADDR_IS_ROM(a) ((uint32_t)(a) < 0x08000000)
#define ADDR_IS_RAM(a) ((uint32_t)(a) >= 0x1C000000 \
	&& (uint32_t)(a) < 0x20100000)
    //XXX where do these numbers come from?

//Symbols defined in the link script.
extern unsigned long _stext;
extern unsigned long _etext;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sbss;
extern unsigned long _ebss;
extern unsigned long _estack;

//XXX why is thie necessary? gcc should be providing these already.
//#define snprintf __builtin_snprintf
//#define sprintf  __builtin_sprintf
#define sscanf   __builtin_sscanf
#define strcspn  __builtin_strcspn
#define strspn   __builtin_strspn
#define strstr   __builtin_strstr
//#define vsprintf __builtin_vsprintf

#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>

#if defined(BOARD_TYPE_TEENSY30)
#include "hw/teensy30.h"
#elif defined(BOARD_TYPE_TEENSY31)
#include "hw/teensy31.h"
#else
#error No valid BOARD_TYPE_* defined
#endif

#include "gcc-macros.h"
#include "errors.h"
#include "string.h"
#include "libs/math.h"
#include "drivers/kinetis/main.h"
#include "libs/io/io.h"
#include "libs/libc/libc.h"

#endif //_MICRON_H_
