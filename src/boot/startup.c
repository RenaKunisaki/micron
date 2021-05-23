//Core startup code. Runs at power-on and initializes the hardware and memory.
//Most of this code is copied from Teensyduino mk20dx128.c
//Note, this fits into the startup section at 0x1BC-0x400 in flash (just after
//the ISR table). It's a very tight fit; currently we have about 20 bytes to
//spare. If you get errors about "can't move location backward" it means you've
//run out of space and will have to move some of the functions to .text section.
//(Be careful not to put them in RAM if RAM isn't populated yet.)
#include "startup.h"

#ifdef __cplusplus
	extern "C" {
#endif

//part of libc, used to init libraries.
WEAK void __libc_init_array(void) {}
WEAK void __libc_fini_array(void) {}
WEAK void _init(void) {};
WEAK void _fini(void) {};


// programs using the watchdog timer or needing to initialize hardware as
// early as possible can implement startup_early_hook()
// we must tell gcc not to put this in RAM, because it's called before
// we've loaded anything to RAM yet.
SECTION(".text") void startupDefaultEarlyHook() {
	WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE;
}

// late hook is called just before main()
static void startupDefaultLateHook() {}

void startupEarlyHook() WEAK ALIAS("startupDefaultEarlyHook");
void startupLateHook()  WEAK ALIAS("startupDefaultLateHook");

//void rtcSet(unsigned long t); //in init.c


//Unlock watchdog timer.
static inline void unlock_wdog() {
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
	__asm__ volatile ("nop"); //must wait 2 cycles after unlocking
	__asm__ volatile ("nop"); //before any other watchdog operation.
}

//enable clocks to always-used peripherals
static inline void enable_clocks() {
	#if defined(__MK20DX128__) /* { */
		SIM_SCGC5 = 0x00043F82;		// clocks active to all GPIO
		SIM_SCGC6 = SIM_SCGC6_RTC | SIM_SCGC6_FTM0 | SIM_SCGC6_FTM1 |
			SIM_SCGC6_ADC0 | SIM_SCGC6_FTFL;

	#elif defined(__MK20DX256__)
		SIM_SCGC3 = SIM_SCGC3_ADC1 | SIM_SCGC3_FTM2;
		SIM_SCGC5 = 0x00043F82;		// clocks active to all GPIO
		SIM_SCGC6 = SIM_SCGC6_RTC | SIM_SCGC6_FTM0 | SIM_SCGC6_FTM1 |
			SIM_SCGC6_ADC0 | SIM_SCGC6_FTFL;
	#endif /* } */
}


//if the RTC oscillator isn't enabled, get it started early
static inline void start_rtc() {
	if(!(RTC_CR & RTC_CR_OSCE)) {
		RTC_SR = 0;
		RTC_CR = RTC_CR_SC16P | RTC_CR_SC4P | RTC_CR_OSCE;
	}
}


static inline void init_power_control() {
	//release I/O pins hold, if we woke up from VLLS mode
	//Writing one to this bit when it is set releases the I/O pads and certain
	//peripherals to their normal run mode state.
	if(PMC_REGSC & PMC_REGSC_ACKISO) PMC_REGSC |= PMC_REGSC_ACKISO;

    //since this is a write once register, make it visible to all F_CPU's
    //so we can switch into other sleep modes in the future at any speed
	SMC_PMPROT = SMC_PMPROT_AVLP   //Allow very low power modes
		       | SMC_PMPROT_ALLS   //Allow low leakage stop mode
		       | SMC_PMPROT_AVLLS; //Allow very low leakage stop mode
}


static inline void init_ram() {
	//Copy text section into memory
	//XXX use memcpy and memset here?
	//Might be faster (compiler can optimize better)?
	//But only if they execute from flash.
	uint32_t *src  = &_etext;
	uint32_t *dest = &_sdata;
	while(dest < &_edata) *dest++ = *src++;

	//Clear BSS section
	dest = &_sbss;
	while(dest < &_ebss) *dest++ = 0;
}


static inline void init_isrs() {
	int i;

	//Copy ISR table into RAM
	for(i=0; i < NVIC_NUM_INTERRUPTS + 16; i++)
		_VectorsRam[i] = _VectorsFlash[i];

	//Default all interrupts to medium priority level
	for(i=0; i < NVIC_NUM_INTERRUPTS; i++) NVIC_SET_PRIORITY(i, 128);
	SCB_VTOR = (uint32_t)_VectorsRam;	//use vector table in RAM
}


static inline void init_clocks() {
	// hardware always starts in FEI mode
		//  C1[CLKS]  bits are written to 00
		//  C1[IREFS] bit  is  written to 1
		//  C6[PLLS]  bit  is  written to 0
	// MCG_SC[FCDIV] defaults to divide by two for internal ref clock
	// I tried changing MSG_SC to divide by 1, it didn't work for me
	#if F_CPU <= 2 MHZ /* { */
		volatile int n;
		MCG_C1 = MCG_C1_CLKS(1) | MCG_C1_IREFS; //use the internal oscillator

		//wait for MCGOUT to use oscillator
		while((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(1)) ;
		for (n=0; n<10; n++) ; //TODO: why do we get 2 mA extra without this delay?
		MCG_C2 = MCG_C2_IRCS;
		while (!(MCG_S & MCG_S_IRCST)) ;

		// now in FBI mode:
		//  C1[CLKS]  bits are written to 01
		//  C1[IREFS] bit  is  written to 1
		//  C6[PLLS]       is  written to 0
		//  C2[LP]         is  written to 0
		MCG_C2 = MCG_C2_IRCS | MCG_C2_LP;

		// now in BLPI mode:
		//  C1[CLKS]  bits are written to 01
		//  C1[IREFS] bit  is  written to 1
		//  C6[PLLS]  bit  is  written to 0
		//  C2[LP]    bit  is  written to 1

	#else
		OSC0_CR = OSC_SC8P | OSC_SC2P; //enable capacitors for crystal

		// enable osc, 8-32 MHz range, low power mode
		MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_EREFS;

		// switch to crystal as clock source, FLL input = 16 MHz / 512
		MCG_C1 =  MCG_C1_CLKS(2) | MCG_C1_FRDIV(4);

		// wait for crystal oscillator to begin
		while((MCG_S & MCG_S_OSCINIT0) == 0) ;

		// wait for FLL to use oscillator
		while((MCG_S & MCG_S_IREFST) != 0) ;

		// wait for MCGOUT to use oscillator
		while((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(2)) ;

		// now in FBE mode
		//  C1[CLKS]  bits are written to 10
		//  C1[IREFS] bit  is  written to 0
		//  C1[FRDIV] must be  written to divide xtal to 31.25-39 kHz
		//  C6[PLLS]  bit  is  written to 0
		//  C2[LP]         is  written to 0
		#if F_CPU <= 16 MHZ /* { */
			// if the crystal is fast enough, use it directly (no FLL or PLL)
			MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_EREFS | MCG_C2_LP;
			// BLPE mode:
			//   C1[CLKS]  bits are written to 10
			//   C1[IREFS] bit  is  written to 0
			//   C2[LP]    bit  is  written to 1
		#else
			// if we need faster than the crystal, turn on the PLL
			#if F_CPU == 72 MHZ /* { */
				//config PLL input for 16 MHz Crystal / 6 = 2.667 Hz
				MCG_C5 = MCG_C5_PRDIV0(5);
			#else
				// config PLL input for 16 MHz Crystal / 4 = 4 MHz
				MCG_C5 = MCG_C5_PRDIV0(3);
			#endif /* } */

			//config PLL for...
			#if F_CPU == 168 MHZ /* { */
				MCG_C6 = MCG_C6_PLLS | MCG_C6_VDIV0(18); //168 MHz output
			#elif F_CPU == 144 MHZ
				MCG_C6 = MCG_C6_PLLS | MCG_C6_VDIV0(12); //144 MHz output
			#elif F_CPU == 120 MHZ
				MCG_C6 = MCG_C6_PLLS | MCG_C6_VDIV0(6); //120 MHz output
			#elif F_CPU == 72 MHZ
				MCG_C6 = MCG_C6_PLLS | MCG_C6_VDIV0(3); //72 MHz output
			#else
				MCG_C6 = MCG_C6_PLLS | MCG_C6_VDIV0(0); //96 MHz output
			#endif /* } */

			// wait for PLL to start using xtal as its input
			while (!(MCG_S & MCG_S_PLLST)) ;

			// wait for PLL to lock
			while (!(MCG_S & MCG_S_LOCK0)) ;
			// now we're in PBE mode
		#endif /* } */
	#endif /* } */
}


static inline void init_clock_dividers() {
    //XXX why these speeds? it seems like we can do 72MHz CPU and 72MHz bus
    //without any problems, so why limit to 36?
    
	#if F_CPU == 168 MHZ /* { */
		// 168 MHz core, 56 MHz bus, 33.6 MHz flash, USB = 168 * 2 / 7
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(2) | SIM_CLKDIV1_OUTDIV4(4);
		SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(6) | SIM_CLKDIV2_USBFRAC;
	#elif F_CPU == 144 MHZ
		// 144 MHz core, 48 MHz bus, 28.8 MHz flash, USB = 144 / 3
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(2) | SIM_CLKDIV1_OUTDIV4(4);
		SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(2);
	#elif F_CPU == 120 MHZ
		// 120 MHz core, 60 MHz bus, 24 MHz flash, USB = 128 * 2 / 5
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(1) | SIM_CLKDIV1_OUTDIV4(4);
		SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(4) | SIM_CLKDIV2_USBFRAC;
	#elif F_CPU == 96 MHZ
		// 96 MHz core, 48 MHz bus, 24 MHz flash, USB = 96 / 2
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(1) | SIM_CLKDIV1_OUTDIV4(3);
		SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(1);
	#elif F_CPU == 72 MHZ
		// 72 MHz core, 36 MHz bus, 24 MHz flash, USB = 72 * 2 / 3
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(1) | SIM_CLKDIV1_OUTDIV4(2);
		SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(2) | SIM_CLKDIV2_USBFRAC;
	#elif F_CPU == 48 MHZ
		// 48 MHz core, 48 MHz bus, 24 MHz flash, USB = 96 / 2
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1) | SIM_CLKDIV1_OUTDIV2(1) | SIM_CLKDIV1_OUTDIV4(3);
		SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(1);
	#elif F_CPU == 24 MHZ
		// 24 MHz core, 24 MHz bus, 24 MHz flash, USB = 96 / 2
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(3) | SIM_CLKDIV1_OUTDIV2(3) | SIM_CLKDIV1_OUTDIV4(3);
		SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(1);
	#elif F_CPU == 16 MHZ
		// 16 MHz core, 16 MHz bus, 16 MHz flash
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(0) | SIM_CLKDIV1_OUTDIV4(0);
	#elif F_CPU == 8 MHZ
		// 8 MHz core, 8 MHz bus, 8 MHz flash
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1) | SIM_CLKDIV1_OUTDIV2(1) | SIM_CLKDIV1_OUTDIV4(1);
	#elif F_CPU == 4 MHZ
		// config divisors: 4 MHz core, 4 MHz bus, 2 MHz flash
		// since we are running from external clock 16MHz
		// fix outdiv too -> cpu 16/4, bus 16/4, flash 16/4
		// here we can go into vlpr?
		// config divisors: 4 MHz core, 4 MHz bus, 4 MHz flash
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(3) | SIM_CLKDIV1_OUTDIV2(3) |	 SIM_CLKDIV1_OUTDIV4(3);
	#elif F_CPU == 2 MHZ
		// since we are running from the fast internal reference clock 4MHz
		// but is divided down by 2 so we actually have a 2MHz, MCG_SC[FCDIV] default is 2
		// fix outdiv -> cpu 2/1, bus 2/1, flash 2/2
		// config divisors: 2 MHz core, 2 MHz bus, 1 MHz flash
		SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(0) |	 SIM_CLKDIV1_OUTDIV4(1);
	#else
	#error "Error, F_CPU must be 168, 144, 120, 96, 72, 48, 24, 16, 8, 4, or 2 MHz"
	#endif /* } */

	#if F_CPU > 16 MHZ /* { */
		// switch to PLL as clock source, FLL input = 16 MHz / 512
		MCG_C1 = MCG_C1_CLKS(0) | MCG_C1_FRDIV(4);

		// wait for PLL clock to be used
		while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(3)) ;

		// now we're in PEE mode
		// USB uses PLL clock, trace is CPU clock, CLKOUT=OSCERCLK0
		SIM_SOPT2 = SIM_SOPT2_USBSRC | SIM_SOPT2_PLLFLLSEL | SIM_SOPT2_TRACECLKSEL |
			SIM_SOPT2_CLKOUTSEL(6);
	#else
		SIM_SOPT2 = SIM_SOPT2_TRACECLKSEL | SIM_SOPT2_CLKOUTSEL(3);
	#endif /* } */

	#if F_CPU <= 2 MHZ /* { */
		// since we are not going into "stop mode" i removed it
		SMC_PMCTRL = SMC_PMCTRL_RUNM(2); // VLPR mode :-)
	#endif /* } */
}


// initialize the SysTick counter
static inline void init_systick() {
	SYST_RVR = (F_CPU / 1000) - 1;
	SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE;
}


static inline void init_timers() {
	//SIM_SCGC6 |= SIM_SCGC6_FTM0;	// TODO: use bitband for atomic read-mod-write
	//SIM_SCGC6 |= SIM_SCGC6_FTM1;
	FTM0_CNT  = 0;
	FTM0_MOD  = DEFAULT_FTM_MOD;
	FTM0_C0SC = 0x28; // MSnB:MSnA = 10, ELSnB:ELSnA = 10
	FTM0_C1SC = 0x28;
	FTM0_C2SC = 0x28;
	FTM0_C3SC = 0x28;
	FTM0_C4SC = 0x28;
	FTM0_C5SC = 0x28;
	FTM0_C6SC = 0x28;
	FTM0_C7SC = 0x28;
	FTM0_SC   = FTM_SC_CLKS(1) | FTM_SC_PS(DEFAULT_FTM_PRESCALE);
	FTM1_CNT  = 0;
	FTM1_MOD  = DEFAULT_FTM_MOD;
	FTM1_C0SC = 0x28;
	FTM1_C1SC = 0x28;
	FTM1_SC   = FTM_SC_CLKS(1) | FTM_SC_PS(DEFAULT_FTM_PRESCALE);
#if defined(__MK20DX256__)
	FTM2_CNT  = 0;
	FTM2_MOD  = DEFAULT_FTM_MOD;
	FTM2_C0SC = 0x28;
	FTM2_C1SC = 0x28;
	FTM2_SC   = FTM_SC_CLKS(1) | FTM_SC_PS(DEFAULT_FTM_PRESCALE);
#endif
	delayMS(4);
}


static inline void enable_pin_interrupts() {
	NVIC_ENABLE_IRQ(IRQ_PORTA);
	NVIC_ENABLE_IRQ(IRQ_PORTB);
	NVIC_ENABLE_IRQ(IRQ_PORTC);
	NVIC_ENABLE_IRQ(IRQ_PORTD);
	NVIC_ENABLE_IRQ(IRQ_PORTE);
}


static inline void set_clock() {
	if (RTC_SR & RTC_SR_TIF) {
		// TODO: this should probably set the time more agressively, if
		// we could reliably detect the first reboot after programming.
		//rtcSet(TIME_T);
	}
}


//Run global initializers that the compiler generates.
//Without this, we'll find some variables mysteriously not initialized
//correctly and go crazy trying to figure out why.
extern "C" {
	static inline void run_global_initializers() {
		extern void (*__init_array_start)();
		extern void (*__init_array_end)();
		for(void (**func)() = &__init_array_start;
		func < &__init_array_end; func++) {
			(*func)();
		}
	}
}


//Entry point at power on.
//XXX shrink this somehow
SECTION(".startup") NAKED NORETURN void ResetHandler(void) {
	unlock_wdog();
	startupEarlyHook();
	enable_clocks();
	start_rtc();
	init_power_control();
	init_ram(); //TODO: do this while the PLL is waiting to lock....
	init_isrs();
	init_clocks();
	init_clock_dividers();
	init_systick();
	__enable_irq();
	//enable_pin_interrupts(); //don't, because we haven't attached any ISR.
	init_timers();
	set_clock();
	run_global_initializers();
	_init();
	__libc_init_array(); //do all libc init stuff
	startupLateHook();
	main();
	__libc_fini_array(); //do all libc shutdown stuff
	_fini();
	while(1) {
		//XXX go into stop/low power mode?
		__asm__ volatile("wfe");
	}
}

#ifdef __cplusplus
	} //extern "C"
#endif
