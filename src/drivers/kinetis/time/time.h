#ifndef _MICRON_TIME_H_
#define _MICRON_TIME_H_

#ifdef __cplusplus
	extern "C" {
#endif

// create a default PWM at the same 488.28 Hz as Arduino Uno
#if F_BUS == 60 MHZ
	#define DEFAULT_FTM_MOD (61440 - 1)
	#define DEFAULT_FTM_PRESCALE 1
#elif F_BUS == 56 MHZ
	#define DEFAULT_FTM_MOD (57344 - 1)
	#define DEFAULT_FTM_PRESCALE 1
#elif F_BUS == 48 MHZ
	#define DEFAULT_FTM_MOD (49152 - 1)
	#define DEFAULT_FTM_PRESCALE 1
#elif F_BUS == 40 MHZ
	#define DEFAULT_FTM_MOD (40960 - 1)
	#define DEFAULT_FTM_PRESCALE 1
#elif F_BUS == 36 MHZ
	#define DEFAULT_FTM_MOD (36864 - 1)
	#define DEFAULT_FTM_PRESCALE 1
#elif F_BUS == 24 MHZ
	#define DEFAULT_FTM_MOD (49152 - 1)
	#define DEFAULT_FTM_PRESCALE 0
#elif F_BUS == 16 MHZ
	#define DEFAULT_FTM_MOD (32768 - 1)
	#define DEFAULT_FTM_PRESCALE 0
#elif F_BUS == 8 MHZ
	#define DEFAULT_FTM_MOD (16384 - 1)
	#define DEFAULT_FTM_PRESCALE 0
#elif F_BUS == 4 MHZ
	#define DEFAULT_FTM_MOD (8192 - 1)
	#define DEFAULT_FTM_PRESCALE 0
#elif F_BUS == 2 MHZ
	#define DEFAULT_FTM_MOD (4096 - 1)
	#define DEFAULT_FTM_PRESCALE 0
#endif

#define SCB_SCR_SLEEPONEXIT_MASK  0x2u
#define SCB_SCR_SLEEPONEXIT_SHIFT 1
#define SCB_SCR_SLEEPDEEP_MASK    0x4u
#define SCB_SCR_SLEEPDEEP_SHIFT   2
#define SCB_SCR_SEVONPEND_MASK    0x10u
#define SCB_SCR_SEVONPEND_SHIFT   4


/** Delay a precise number of microseconds.
 *  Note that this won't be precise if interrupts are enabled.
 *  This has only been tested at 72mhz, and in particular might not work
 *  at 3mhz and under. (TODO)
 *  It also won't work correctly if you change the CPU speed at runtime.
 */
#define DELAY_CYCLES (F_CPU / 3000000)
#if 1
USED_SYMBOL static int dummy;
#define delayUS(n) __asm__ volatile( \
	"1: subs %0, #1       \n" \
	"bne 1b               \n" \
	: "=r" (dummy) /* no outputs */ \
	: "0" (n * DELAY_CYCLES) /* input */ \
	: "0" /* clobbers */ \
)
#else
#define delayUS(n) __asm__ volatile( \
	"1: subs %0, #1       \n" \
	"bne 1b               \n" \
	: /* no outputs */ \
	: "r" (n * DELAY_CYCLES) /* input */ \
	: "0" /* clobbers */ \
)
#endif

//isr.c
extern volatile uint32_t systick_millis_count;

//time.c
volatile uint32_t micros();
volatile uint32_t millis();
void delayMS(uint32_t ms);
WEAK int rtcInit();
WEAK int rtcGet(uint32_t *outSecs, uint32_t *outUsecs);
WEAK void rtcSet(unsigned long secs, unsigned int usecs);


/** Wait for an interrupt or external event.
 *  The details on how exactly this differs from wfi are vague, but it seems
 *  wfi will always wait for an interrupt, whereas wfe can be implemented as a
 *  nop and is only intended for power saving when you have nothing else to do.
 */
INLINE void idle() { __asm__ volatile("WFE"); }


/** Go into deep sleep mode.
 */
INLINE void powerSleep() {
	//Set the SLEEPDEEP bit to enable deep sleep mode (STOP)
	SCB_SCR |= SCB_SCR_SLEEPDEEP_MASK;

	//WFI instruction will start entry into STOP mode
	__asm__ volatile("WFI"); //Wait For Interrupt
}

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_TIME_H_
