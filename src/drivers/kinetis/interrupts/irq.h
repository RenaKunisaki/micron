#ifndef _MICRON_IRQ_H_
#define _MICRON_IRQ_H_

#ifdef __cplusplus
	extern "C" {
#endif

//XXX where are these supposed to be?
#define SCB_SCR_SLEEPONEXIT_MASK  0x2u
#define SCB_SCR_SLEEPONEXIT_SHIFT 1
#define SCB_SCR_SLEEPDEEP_MASK    0x4u
#define SCB_SCR_SLEEPDEEP_SHIFT   2
#define SCB_SCR_SEVONPEND_MASK    0x10u
#define SCB_SCR_SEVONPEND_SHIFT   4

static uint8_t irqDisableDepth = 0;

/** Disable all interrupts.
 *  Use this when precise timing is important. Leaving interrupts disabled will
 *  prevent many functions from working correctly.
 *  This function can be nested. Each call to `irqDisable()` increments a
 *  counter, and each call to `irqEnable()` decrements the counter. Interrupts
 *  are disabled as long as the counter is greater than zero. This allows using
 *  these functions to delimit critical sections without having to check the
 *  enable state beforehand.
 */
inline void irqDisable() {
	__disable_irq();
	if(irqDisableDepth < 255) irqDisableDepth++;
}

/** Enable all interrupts, after disabling them with `irqDisable()`.
 */
inline void irqEnable() {
	if(irqDisableDepth >  0) irqDisableDepth--;
	if(irqDisableDepth == 0) __enable_irq();
}

/** Return the IRQ disable counter, which is nonzero if interrupts are disabled.
 */
inline uint8_t irqEnabled() {
	return irqDisableDepth == 0;
}

/** Enable the specified interrupt.
 */
inline void irqEnableInterrupt(uint8_t which) {
	NVIC_ENABLE_IRQ(which);
}

/** Disable the specified interrupt.
 */
inline void irqDisableInterrupt(uint8_t which) {
	NVIC_DISABLE_IRQ(which);
}

/** Check if the specified interrupt is enabled.
 */
inline uint8_t irqInterruptEnabled(uint8_t which) {
	return (*(NVIC_BASE + (which >> 5))) & BIT(which & 31);
}

/** Set the priority of the specified interrupt.
 *  Priority is 0 to 255, with 0 being highest. However, the granularity may be
 *  lower. On Teensy 3.x there are only 16 priority levels; the lower 4 bits are
 *  ignored.
 */
inline void irqSetPriority(uint8_t which, uint8_t priority) {
	NVIC_SET_PRIORITY(which, priority);
}

/** Get the priority of the specified interrupt.
 */
inline uint8_t irqGetPriority(uint8_t which) {
	return NVIC_GET_PRIORITY(which);
}

/** Wait for an interrupt.
 *  This can be called even when interrupts are disabled (in which case it will
 *  return, but not call the IRQ handler, when an interrupt occurs).
 *  Mind that by default, the systick interrupt fires every millisecond,
 *  so this function will wait for no more than 1ms when that's enabled.
 */
INLINE void irqWait() {
	//Clear the SLEEPDEEP bit to make sure we go into WAIT (sleep) mode
	//instead of deep sleep.
	SCB_SCR &= ~SCB_SCR_SLEEPDEEP_MASK;
	__asm__ volatile("WFI"); //Wait For Interrupt
}

/** Return the number of the currently executing ISR. Zero means no ISR is
 *  executing.
 */
INLINE int irqCurrentISR() {
    //read the ARM ICSR register (which is memory mapped).
	return SCB_ICSR & 0x1FF;
}


#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_IRQ_H_
