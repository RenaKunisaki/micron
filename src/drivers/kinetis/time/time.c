#include <micron.h>

/** Returns number of microseconds that have elapsed.
 *  Note that this counter will overflow approximately once every 71 minutes.
 *  Therefore, in code such as this:
 *    uint32_t start = micros();
 *    do_something_real_quick();
 *    uint32_t end = micros();
 *    uint32_t elapsed = end - start;
 *  It's possible that end < start; however, elapsed will still be valid, as
 *  long as fewer than 71 minutes passed between the two calls to micros().
 */
volatile uint32_t micros() {
	uint32_t count, current, istatus;

	irqDisable();
	current = SYST_CVR;
	count = systick_millis_count;
	istatus = SCB_ICSR;	// bit 26 indicates if systick exception pending
	irqEnable();

	if ((istatus & SCB_ICSR_PENDSTSET) && current > 50) count++;
	current = ((F_CPU / 1000) - 1) - current;
	return count * 1000 + current / (F_CPU / 1000000);
}


/** Returns number of milliseconds that have elapsed.
 *  The same caveats as micros() apply here with regard to overflow,
 *  except that this counter only overflows once every ~49.7 days.
 */
volatile uint32_t systick_millis_count;
volatile uint32_t millis() {
	return systick_millis_count;
}


/** Delay for (approximately) a number of milliseconds.
 *  This will be more accurate if you disable interrupts first.
 *  (Otherwise, a slow ISR can delay it beyond the intended time.)
 */
void delayMS(uint32_t ms) {
	uint32_t start = micros();
	while(ms > 0) {
		if((micros() - start) >= 1000) {
			ms--;
			start += 1000;
		}
		//yield();
	}
}

/** Initialize the real-time clock.
 */
WEAK int rtcInit() {
    SIM_SCGC6 |= SIM_SCGC6_RTC; //enable module
    RTC_CR = RTC_CR_SWR; //reset
    delayMS(1);
    RTC_CR = 0; //not reset
    delayMS(1); //wait for RTC init before setting it up.
    //according to the datasheet, RTC_CR_CLKO *disables* the 32KHz clock output.
    //this seems strange and might be a typo. anyway I don't think that output
    //is used or accessible on the Teensy.
    RTC_CR = RTC_CR_SUP | RTC_CR_OSCE | RTC_CR_CLKO; // | RTC_CR_SC16P;
    RTC_RAR = 0xFF; //read access: all
    RTC_WAR = 0xFF; //write access: all
    //delayMS(20);
    //RTC_SR  = 0; //disable counter (allow writing)
    //RTC_TPR = 0; //clear prescalar (fractional seconds)
    //RTC_TSR = 42; //clear counter (seconds)
    RTC_SR  = RTC_SR_TCE; //start counting (disable writing)
    return 0;
}

/** Read the RTC.
 */
WEAK int rtcGet(uint32_t *outSecs, uint32_t *outUsecs) {
    uint32_t stat = RTC_SR;
    if(stat & RTC_SR_TIF) return -ENODATA;   //timer not set
    if(stat & RTC_SR_TOF) return -EOVERFLOW; //timer overflowed

    //Read the RTC in a loop until we read the same TSR value twice.
    //This avoids a race condition where TSR changes between reading
    //it and TPR, which would cause readings off by up to a second.
    uint32_t secs1, secs2, frac;
    do {
        secs1 = RTC_TSR;
        frac  = RTC_TPR;
        secs2 = RTC_TSR;
    } while(secs1 != secs2);

    if(outSecs)  *outSecs  = secs2;
    if(outUsecs) *outUsecs = frac * RTC_PRESCALAR_RATIO;
    return 0;
}

/** Set the RTC.
 */
WEAK void rtcSet(unsigned long secs, unsigned int usecs) {
	RTC_SR  = 0; //stop the clock so we can set it
    RTC_TPR = usecs / RTC_PRESCALAR_RATIO;
	RTC_TSR = secs;
	RTC_SR  = RTC_SR_TCE; //start the clock
}
