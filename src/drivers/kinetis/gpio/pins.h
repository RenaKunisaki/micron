//Functions, macros, and definitions relating to Teensy GPIO pins.
//Much of this is copied from the original Teensy libraries and then reorganized
//and added on to.
#ifndef _MICRON_PINS_H_
#define _MICRON_PINS_H_

#ifdef __cplusplus
	extern "C" {
#endif

#define LOW    0
#define HIGH   1
#define INPUT  0
#define OUTPUT 1

#include "pin_bits.h"

//Arrays of pointers to the GPIO registers for each pin.
//These are used when constant folding isn't possible in the functions below.
extern volatile uint32_t* const pinBaseAddr[];
extern volatile uint32_t* const pinConfigAddr[];

//Convenient macros for accessing the GPIO registers of a given pin.
#define PDOR(n) (*(pinBaseAddr[n]       )) //Data Output Register
#define PSOR(n) (*(pinBaseAddr[n] + 0x20)) //Set Output Register
#define PCOR(n) (*(pinBaseAddr[n] + 0x40)) //Clear Output Register
#define PTOR(n) (*(pinBaseAddr[n] + 0x60)) //Toggle Output Register
#define PDIR(n) (*(pinBaseAddr[n] + 0x80)) //Data Input Register
#define PDDR(n) (*(pinBaseAddr[n] + 0xA0)) //Data Direction Register
#define PCR(n)  (*(pinConfigAddr[n]))      //Pin Control Register

//More readable names for these registers.
#define PIN_OUTPUT_REG(n)    PDOR(n)
#define PIN_SET_REG(n)       PSOR(n)
#define PIN_CLEAR_REG(n)     PCOR(n)
#define PIN_TOGGLE_REG(n)    PTOR(n)
#define PIN_INPUT_REG(n)     PDIR(n)
#define PIN_DIRECTION_REG(n) PDDR(n)
#define PIN_CONTROL_REG(n)   PCR(n)

//Bits of PCR.
#define PCR_ISF                BIT(24)//Interrupt Status Flag (write 1 to clear)
#define PCR_IRQC(n)            ((n) << 16) //Interrupt Configuration
#define PCR_IRQ_NONE           0 //Interrupt/DMA request disabled.
#define PCR_DMA_NONE           0
#define PCR_DMA_RISING         (0x01 << 16) //DMA request on rising edge.
#define PCR_DMA_FALLING        (0x02 << 16) //DMA request on falling edge.
#define PCR_DMA_EITHER         (0x03 << 16) //DMA request on either edge.
#define PCR_IRQ_ZERO           (0x08 << 16) //Interrupt when logic zero.
#define PCR_IRQ_ONE            (0x0C << 16) //Interrupt when logic one.
#define PCR_IRQ_RISING         (0x09 << 16) //Interrupt on rising edge.
#define PCR_IRQ_FALLING        (0x0A << 16) //Interrupt on falling edge.
#define PCR_IRQ_EITHER         (0x0B << 16) //Interrupt on either edge.
#define PCR_IRQ_MASK           (0x0F << 16) //All IRQC bits
#define PCR_LOCK               BIT(15) //Lock bits 0-15 until reset
#define PCR_MUX(n)             ((n) << 8) //mux control
#define PCR_DRIVE_STRENGTH_LOW 0
#define PCR_DRIVE_STRENGTH_HI  BIT(6)
#define PCR_OPEN_DRAIN         BIT(5)
#define PCR_FILTER             BIT(4)
#define PCR_SLEW_FAST          0
#define PCR_SLEW_SLOW          BIT(2)
#define PCR_PULL_ENABLE        BIT(1) //Enable pullup/down resistor for inputs
#define PCR_PULLUP             (PCR_PULL_ENABLE | BIT(0))
#define PCR_PULLDOWN           PCR_PULL_ENABLE

//These are the flags the Teensy libs use to set pins to output/input modes.
//Should be possible to use other flags if you like.
#define PCR_OUTPUT (PCR_SLEW_SLOW | PCR_DRIVE_STRENGTH_HI | PCR_MUX(1))
#define PCR_INPUT PCR_MUX(1)


//Fast inline functions to set pin I/O modes.
//These use __builtin_constant_p() to choose a faster path when possible.
//The compiler is able to condense the entire function body into a single
//inline statement at compile time, so this is a lot more efficient than
//it looks.
INLINE void setPinMode(int pin, int mode, int direction) {
	uint32_t m=mode, d=direction; //to fit statements on one line
	if(__builtin_constant_p(pin)) {
		//pin is a constant, so gcc is able to optimize all this down
		//to a single statement at compile time.
		if      (pin ==  0) { CORE_PIN0_PCR  = m;  CORE_PIN0_PDDR  = d; }
		else if (pin ==  1) { CORE_PIN1_PCR  = m;  CORE_PIN1_PDDR  = d; }
		else if (pin ==  2) { CORE_PIN2_PCR  = m;  CORE_PIN2_PDDR  = d; }
		else if (pin ==  3) { CORE_PIN3_PCR  = m;  CORE_PIN3_PDDR  = d; }
		else if (pin ==  4) { CORE_PIN4_PCR  = m;  CORE_PIN4_PDDR  = d; }
		else if (pin ==  5) { CORE_PIN5_PCR  = m;  CORE_PIN5_PDDR  = d; }
		else if (pin ==  6) { CORE_PIN6_PCR  = m;  CORE_PIN6_PDDR  = d; }
		else if (pin ==  7) { CORE_PIN7_PCR  = m;  CORE_PIN7_PDDR  = d; }
		else if (pin ==  8) { CORE_PIN8_PCR  = m;  CORE_PIN8_PDDR  = d; }
		else if (pin ==  9) { CORE_PIN9_PCR  = m;  CORE_PIN9_PDDR  = d; }
		else if (pin == 10) { CORE_PIN10_PCR = m;  CORE_PIN10_PDDR = d; }
		else if (pin == 11) { CORE_PIN11_PCR = m;  CORE_PIN11_PDDR = d; }
		else if (pin == 12) { CORE_PIN12_PCR = m;  CORE_PIN12_PDDR = d; }
		else if (pin == 13) { CORE_PIN13_PCR = m;  CORE_PIN13_PDDR = d; }
		else if (pin == 14) { CORE_PIN14_PCR = m;  CORE_PIN14_PDDR = d; }
		else if (pin == 15) { CORE_PIN15_PCR = m;  CORE_PIN15_PDDR = d; }
		else if (pin == 16) { CORE_PIN16_PCR = m;  CORE_PIN16_PDDR = d; }
		else if (pin == 17) { CORE_PIN17_PCR = m;  CORE_PIN17_PDDR = d; }
		else if (pin == 18) { CORE_PIN18_PCR = m;  CORE_PIN18_PDDR = d; }
		else if (pin == 19) { CORE_PIN19_PCR = m;  CORE_PIN19_PDDR = d; }
		else if (pin == 20) { CORE_PIN20_PCR = m;  CORE_PIN20_PDDR = d; }
		else if (pin == 21) { CORE_PIN21_PCR = m;  CORE_PIN21_PDDR = d; }
		else if (pin == 22) { CORE_PIN22_PCR = m;  CORE_PIN22_PDDR = d; }
		else if (pin == 23) { CORE_PIN23_PCR = m;  CORE_PIN23_PDDR = d; }
		else if (pin == 24) { CORE_PIN24_PCR = m;  CORE_PIN24_PDDR = d; }
		else if (pin == 25) { CORE_PIN25_PCR = m;  CORE_PIN25_PDDR = d; }
		else if (pin == 26) { CORE_PIN26_PCR = m;  CORE_PIN26_PDDR = d; }
		else if (pin == 27) { CORE_PIN27_PCR = m;  CORE_PIN27_PDDR = d; }
		else if (pin == 28) { CORE_PIN28_PCR = m;  CORE_PIN28_PDDR = d; }
		else if (pin == 29) { CORE_PIN29_PCR = m;  CORE_PIN29_PDDR = d; }
		else if (pin == 30) { CORE_PIN30_PCR = m;  CORE_PIN30_PDDR = d; }
		else if (pin == 31) { CORE_PIN31_PCR = m;  CORE_PIN31_PDDR = d; }
		else if (pin == 32) { CORE_PIN32_PCR = m;  CORE_PIN32_PDDR = d; }
		else if (pin == 33) { CORE_PIN33_PCR = m;  CORE_PIN33_PDDR = d; }
	}
	else {
		//pin is not a constant that can be folded at compile time,
		//so we need to do this the "slow" way.
		//still, the function will be optimized down to just these
		//two statements at compile time, so it's only slightly slower.
		PIN_CONTROL_REG(pin)   = m;
		PIN_DIRECTION_REG(pin) = d;
	}
}


INLINE void setPinAsInput(int pin) {
	setPinMode(pin, PCR_INPUT, INPUT);
}

INLINE void setPinAsInputPullup(int pin) {
	setPinMode(pin, PCR_INPUT | PCR_PULLUP, INPUT);
}

INLINE void setPinAsInputPulldown(int pin) {
	setPinMode(pin, PCR_INPUT | PCR_PULLDOWN, INPUT);
}

INLINE void setPinAsOutput(int pin) {
	setPinMode(pin, PCR_OUTPUT, OUTPUT);
}

INLINE void setPinAs(int pin, int mode) {
	if(mode == INPUT) setPinAsInput(pin);
	else setPinAsOutput(pin);
}

//Set pin interrupt mode.
//Mode is one of PCR_IRQ_* or PCR_DMA_*.
//Note that you also need to enable/disable the corresponding IRQ_PORT*.
//The default handlers for those IRQs call `void isrPin(int pin)`;
//the default isrPin() calls isrUnused().
INLINE void setPinInterrupt(int pin, uint32_t mode) {
	mode |= PCR_ISF; //clear interrupt flag if it's set (by writing 1 to it)
	PIN_CONTROL_REG(pin) = (PIN_CONTROL_REG(pin) & ~PCR_IRQ_MASK) | mode;
}


//Fast digital pin write.
//Copied from Teensy's digitalWriteFast() (but this version is using
//bitband registers).
INLINE void digitalWrite(uint8_t pin, int val) {
	uint8_t v = (val != 0); //Ensure val is either 0 or 1.
		//XXX could this be further optimized using
		//__builtin_constant_p(val)? The original Teensy libs check val and have
		//separate paths to store to PSOR or PCOR, but they don't use
		//__builtin_constant_p(val). Not sure why, or if that's any faster.
	if(__builtin_constant_p(pin)) {
		if      (pin ==  0) CORE_PIN0_PDOR  = v;
		else if (pin ==  1) CORE_PIN1_PDOR  = v;
		else if (pin ==  2) CORE_PIN2_PDOR  = v;
		else if (pin ==  3) CORE_PIN3_PDOR  = v;
		else if (pin ==  4) CORE_PIN4_PDOR  = v;
		else if (pin ==  5) CORE_PIN5_PDOR  = v;
		else if (pin ==  6) CORE_PIN6_PDOR  = v;
		else if (pin ==  7) CORE_PIN7_PDOR  = v;
		else if (pin ==  8) CORE_PIN8_PDOR  = v;
		else if (pin ==  9) CORE_PIN9_PDOR  = v;
		else if (pin == 10) CORE_PIN10_PDOR = v;
		else if (pin == 11) CORE_PIN11_PDOR = v;
		else if (pin == 12) CORE_PIN12_PDOR = v;
		else if (pin == 13) CORE_PIN13_PDOR = v;
		else if (pin == 14) CORE_PIN14_PDOR = v;
		else if (pin == 15) CORE_PIN15_PDOR = v;
		else if (pin == 16) CORE_PIN16_PDOR = v;
		else if (pin == 17) CORE_PIN17_PDOR = v;
		else if (pin == 18) CORE_PIN18_PDOR = v;
		else if (pin == 19) CORE_PIN19_PDOR = v;
		else if (pin == 20) CORE_PIN20_PDOR = v;
		else if (pin == 21) CORE_PIN21_PDOR = v;
		else if (pin == 22) CORE_PIN22_PDOR = v;
		else if (pin == 23) CORE_PIN23_PDOR = v;
		else if (pin == 24) CORE_PIN24_PDOR = v;
		else if (pin == 25) CORE_PIN25_PDOR = v;
		else if (pin == 26) CORE_PIN26_PDOR = v;
		else if (pin == 27) CORE_PIN27_PDOR = v;
		else if (pin == 28) CORE_PIN28_PDOR = v;
		else if (pin == 29) CORE_PIN29_PDOR = v;
		else if (pin == 30) CORE_PIN30_PDOR = v;
		else if (pin == 31) CORE_PIN31_PDOR = v;
		else if (pin == 32) CORE_PIN32_PDOR = v;
		else if (pin == 33) CORE_PIN33_PDOR = v;
	}
	else PDOR(pin) = v;
}


//Fast digital pin read.
//Copied from Teensy's digitalReadFast() (but this version is using
//bitband registers).
INLINE volatile uint8_t digitalRead(uint8_t pin) {
	if(__builtin_constant_p(pin)) {
		if      (pin ==  0) return CORE_PIN0_PDIR;
		else if (pin ==  1) return CORE_PIN1_PDIR;
		else if (pin ==  2) return CORE_PIN2_PDIR;
		else if (pin ==  3) return CORE_PIN3_PDIR;
		else if (pin ==  4) return CORE_PIN4_PDIR;
		else if (pin ==  5) return CORE_PIN5_PDIR;
		else if (pin ==  6) return CORE_PIN6_PDIR;
		else if (pin ==  7) return CORE_PIN7_PDIR;
		else if (pin ==  8) return CORE_PIN8_PDIR;
		else if (pin ==  9) return CORE_PIN9_PDIR;
		else if (pin == 10) return CORE_PIN10_PDIR;
		else if (pin == 11) return CORE_PIN11_PDIR;
		else if (pin == 12) return CORE_PIN12_PDIR;
		else if (pin == 13) return CORE_PIN13_PDIR;
		else if (pin == 14) return CORE_PIN14_PDIR;
		else if (pin == 15) return CORE_PIN15_PDIR;
		else if (pin == 16) return CORE_PIN16_PDIR;
		else if (pin == 17) return CORE_PIN17_PDIR;
		else if (pin == 18) return CORE_PIN18_PDIR;
		else if (pin == 19) return CORE_PIN19_PDIR;
		else if (pin == 20) return CORE_PIN20_PDIR;
		else if (pin == 21) return CORE_PIN21_PDIR;
		else if (pin == 22) return CORE_PIN22_PDIR;
		else if (pin == 23) return CORE_PIN23_PDIR;
		else if (pin == 24) return CORE_PIN24_PDIR;
		else if (pin == 25) return CORE_PIN25_PDIR;
		else if (pin == 26) return CORE_PIN26_PDIR;
		else if (pin == 27) return CORE_PIN27_PDIR;
		else if (pin == 28) return CORE_PIN28_PDIR;
		else if (pin == 29) return CORE_PIN29_PDIR;
		else if (pin == 30) return CORE_PIN30_PDIR;
		else if (pin == 31) return CORE_PIN31_PDIR;
		else if (pin == 32) return CORE_PIN32_PDIR;
		else if (pin == 33) return CORE_PIN33_PDIR;
		else return 0;
	}
	else return PDIR(pin);
}

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_PINS_H_
