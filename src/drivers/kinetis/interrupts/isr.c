//Interrupt handlers.
#include <micron.h>

#ifdef __cplusplus
	extern "C" {
#endif


//Default handler for ISRs that aren't used
//XXX these two should be WEAK COLD NORETURN but that gives us warnings:
//warning: 'void isrHardFault()' specifies less restrictive attributes than
//its target 'void isrUnused()': 'cold', 'noreturn' [-Wmissing-attributes]
//as best I can tell this is safe to ignore, but it's annoying.
//the only other solution I can see is to add those attributes to all of the
//ISRs that are unused by default, but that would be incorrect because those
//aren't necessarily cold/noreturn.
WEAK void isrFault(void) {
	//PDDR(13) = 1;
	//PCR (13) = PCR_OUTPUT;
	//PDOR(13) = 1;
	while (1) {
		// keep polling some communication while in fault
		// mode, so we don't completely die.
		#if isrUsb != isrUnused //XXX does this work?
			if (SIM_SCGC4 & SIM_SCGC4_USBOTG) isrUsb();
		#endif

		#if isrUart0Status != isrUnused
			if (SIM_SCGC4 & SIM_SCGC4_UART0) isrUart0Status();
		#endif

		#if isrUart1Status != isrUnused
			if (SIM_SCGC4 & SIM_SCGC4_UART1) isrUart1Status();
		#endif

		#if isrUart2Status != isrUnused
			if (SIM_SCGC4 & SIM_SCGC4_UART2) isrUart2Status();
		#endif
	}
}

WEAK void isrUnused(void) {
	isrFault();
}

//systick interrupt handler.
ISRFUNC void isrDefaultSystick(void) {
	systick_millis_count++;
}


//GPIO interrupt handlers
WEAK void isrPin(int pin) { isrUnused(); }

ISRFUNC void isrDefaultPortA(void) {
	uint32_t isfr = PORTA_ISFR;
	if(isfr & CORE_PIN3_BITMASK ) isrPin( 3);
	if(isfr & CORE_PIN4_BITMASK ) isrPin( 4);
	if(isfr & CORE_PIN24_BITMASK) isrPin(24);
	if(isfr & CORE_PIN33_BITMASK) isrPin(33);
    PORTA_ISFR = isfr;
}

ISRFUNC void isrDefaultPortB(void) {
	uint32_t isfr = PORTB_ISFR;
	if(isfr & CORE_PIN0_BITMASK ) isrPin( 0);
	if(isfr & CORE_PIN1_BITMASK ) isrPin( 1);
	if(isfr & CORE_PIN16_BITMASK) isrPin(16);
	if(isfr & CORE_PIN17_BITMASK) isrPin(17);
	if(isfr & CORE_PIN18_BITMASK) isrPin(18);
	if(isfr & CORE_PIN19_BITMASK) isrPin(19);
	if(isfr & CORE_PIN25_BITMASK) isrPin(25);
	if(isfr & CORE_PIN32_BITMASK) isrPin(32);
    PORTB_ISFR = isfr;
}

ISRFUNC void isrDefaultPortC(void) {
	uint32_t isfr = PORTC_ISFR;
	if(isfr & CORE_PIN9_BITMASK ) isrPin( 9);
	if(isfr & CORE_PIN10_BITMASK) isrPin(10);
	if(isfr & CORE_PIN11_BITMASK) isrPin(11);
	if(isfr & CORE_PIN12_BITMASK) isrPin(12);
	if(isfr & CORE_PIN13_BITMASK) isrPin(13);
	if(isfr & CORE_PIN15_BITMASK) isrPin(15);
	if(isfr & CORE_PIN22_BITMASK) isrPin(22);
	if(isfr & CORE_PIN23_BITMASK) isrPin(23);
	if(isfr & CORE_PIN27_BITMASK) isrPin(27);
	if(isfr & CORE_PIN28_BITMASK) isrPin(28);
	if(isfr & CORE_PIN29_BITMASK) isrPin(29);
	if(isfr & CORE_PIN30_BITMASK) isrPin(30);
    PORTC_ISFR = isfr;
}

ISRFUNC void isrDefaultPortD(void) {
	uint32_t isfr = PORTD_ISFR;
	if(isfr & CORE_PIN2_BITMASK ) isrPin( 2);
	if(isfr & CORE_PIN5_BITMASK ) isrPin( 5);
	if(isfr & CORE_PIN6_BITMASK ) isrPin( 6);
	if(isfr & CORE_PIN7_BITMASK ) isrPin( 7);
	if(isfr & CORE_PIN8_BITMASK ) isrPin( 8);
	if(isfr & CORE_PIN14_BITMASK) isrPin(14);
	if(isfr & CORE_PIN20_BITMASK) isrPin(20);
	if(isfr & CORE_PIN21_BITMASK) isrPin(21);
    PORTD_ISFR = isfr;
}

ISRFUNC void isrDefaultPortE(void) {
	uint32_t isfr = PORTE_ISFR;
	if(isfr & CORE_PIN26_BITMASK) isrPin(26);
	if(isfr & CORE_PIN31_BITMASK) isrPin(31);
    PORTE_ISFR = isfr;
}

WEAK void isrI2C(int port) { isrUnused(); }
ISRFUNC void isrDefaultI2c0(void) { isrI2C(0); }
ISRFUNC void isrDefaultI2c1(void) { isrI2C(1); }
ISRFUNC void isrDefaultI2c2(void) { isrI2C(2); } //XXX aren't there only two?

WEAK void isrUartStatus(int port) { isrUnused(); }
ISRFUNC void isrDefaultUart0Status(void) { isrUartStatus(0); }
ISRFUNC void isrDefaultUart1Status(void) { isrUartStatus(1); }
ISRFUNC void isrDefaultUart2Status(void) { isrUartStatus(2); }
ISRFUNC void isrDefaultUart3Status(void) { isrUartStatus(3); }
ISRFUNC void isrDefaultUart4Status(void) { isrUartStatus(4); }
ISRFUNC void isrDefaultUart5Status(void) { isrUartStatus(5); }


ISRFUNC void isrDefaultUsb() {
	uint8_t status;
	while((status = USB0_ISTAT) & 0x9F) {                       //bit
		if(status & USB_ISTAT_USBRST) { isrUsbReset(); break; } // 0
		if(status & USB_ISTAT_ERROR ) { isrUsbError();        } // 1
		if(status & USB_ISTAT_SOFTOK) { isrUsbSof();          } // 2
		if(status & USB_ISTAT_TOKDNE) { isrUsbToken();        } // 3
		if(status & USB_ISTAT_SLEEP ) { isrUsbSleep();        } // 4
		if(status & USB_ISTAT_RESUME) { isrUsbResume();       } // 5
		if(status & USB_ISTAT_ATTACH) { isrUsbAttach();       } // 6
		if(status & USB_ISTAT_STALL ) { isrUsbStall();        } // 7
	}
}



//Default interrupt handlers.
//These are weak aliases, so they're automatically overridden
//if your program defines the same functions.
void isrNmi()             WEAK ALIAS("isrUnused");
void isrHardFault()       WEAK ALIAS("isrUnused");
void isrMemFault()        WEAK ALIAS("isrUnused");
void isrBusFault()        WEAK ALIAS("isrUnused");
void isrUsageFault()      WEAK ALIAS("isrUnused");
void isrSvcall()          WEAK ALIAS("isrUnused");
void isrDebugmonitor()    WEAK ALIAS("isrUnused");
void isrPendablesrvreq()  WEAK ALIAS("isrUnused");
void isrSystick()         WEAK ALIAS("isrDefaultSystick");
void isrDmaCh0()          WEAK ALIAS("isrUnused");
void isrDmaCh1()          WEAK ALIAS("isrUnused");
void isrDmaCh2()          WEAK ALIAS("isrUnused");
void isrDmaCh3()          WEAK ALIAS("isrUnused");
void isrDmaCh4()          WEAK ALIAS("isrUnused");
void isrDmaCh5()          WEAK ALIAS("isrUnused");
void isrDmaCh6()          WEAK ALIAS("isrUnused");
void isrDmaCh7()          WEAK ALIAS("isrUnused");
void isrDmaCh8()          WEAK ALIAS("isrUnused");
void isrDmaCh9()          WEAK ALIAS("isrUnused");
void isrDmaCh10()         WEAK ALIAS("isrUnused");
void isrDmaCh11()         WEAK ALIAS("isrUnused");
void isrDmaCh12()         WEAK ALIAS("isrUnused");
void isrDmaCh13()         WEAK ALIAS("isrUnused");
void isrDmaCh14()         WEAK ALIAS("isrUnused");
void isrDmaCh15()         WEAK ALIAS("isrUnused");
void isrDmaError()        WEAK ALIAS("isrUnused");
void isrMcm()             WEAK ALIAS("isrUnused");
void isrFlashCmd()        WEAK ALIAS("isrUnused");
void isrFlashError()      WEAK ALIAS("isrUnused");
void isrLowVoltage()      WEAK ALIAS("isrUnused");
void isrWakeup()          WEAK ALIAS("isrUnused");
void isrWatchdog()        WEAK ALIAS("isrUnused");
void isrI2c0()            WEAK ALIAS("isrDefaultI2c0");
void isrI2c1()            WEAK ALIAS("isrDefaultI2c1");
void isrI2c2()            WEAK ALIAS("isrDefaultI2c2");
void isrSpi0()            WEAK ALIAS("isrUnused");
void isrSpi1()            WEAK ALIAS("isrUnused");
void isrSpi2()            WEAK ALIAS("isrUnused");
void isrSdhc()            WEAK ALIAS("isrUnused");
void isrCan0Message()     WEAK ALIAS("isrUnused");
void isrCan0BusOff()      WEAK ALIAS("isrUnused");
void isrCan0Error()       WEAK ALIAS("isrUnused");
void isrCan0TxWarn()      WEAK ALIAS("isrUnused");
void isrCan0RxWarn()      WEAK ALIAS("isrUnused");
void can0_isrWakeup()     WEAK ALIAS("isrUnused");
void isrI2s0Tx()          WEAK ALIAS("isrUnused");
void isrI2s0Rx()          WEAK ALIAS("isrUnused");
void isrUart0Lon()        WEAK ALIAS("isrUnused");
void isrUart0Status()     WEAK ALIAS("isrDefaultUart0Status");
void isrUart0Error()      WEAK ALIAS("isrUnused");
void isrUart1Status()     WEAK ALIAS("isrDefaultUart1Status");
void isrUart1Error()      WEAK ALIAS("isrUnused");
void isrUart2Status()     WEAK ALIAS("isrDefaultUart2Status");
void isrUart2Error()      WEAK ALIAS("isrUnused");
void isrUart3Status()     WEAK ALIAS("isrDefaultUart3Status");
void isrUart3Error()      WEAK ALIAS("isrUnused");
void isrUart4Status()     WEAK ALIAS("isrDefaultUart4Status");
void isrUart4Error()      WEAK ALIAS("isrUnused");
void isrUart5Status()     WEAK ALIAS("isrDefaultUart5Status");
void isrUart5Error()      WEAK ALIAS("isrUnused");
void isrAdc0()            WEAK ALIAS("isrUnused");
void isrAdc1()            WEAK ALIAS("isrUnused");
void isrCmp0()            WEAK ALIAS("isrUnused");
void isrCmp1()            WEAK ALIAS("isrUnused");
void isrCmp2()            WEAK ALIAS("isrUnused");
void isrFtm0()            WEAK ALIAS("isrUnused");
void isrFtm1()            WEAK ALIAS("isrUnused");
void isrFtm2()            WEAK ALIAS("isrUnused");
void isrFtm3()            WEAK ALIAS("isrUnused");
void isrCmt()             WEAK ALIAS("isrUnused");
void isrRtcAlarm()        WEAK ALIAS("isrUnused");
void isrRtcSeconds()      WEAK ALIAS("isrUnused");
void isrPit0()            WEAK ALIAS("isrUnused");
void isrPit1()            WEAK ALIAS("isrUnused");
void isrPit2()            WEAK ALIAS("isrUnused");
void isrPit3()            WEAK ALIAS("isrUnused");
void isrPdb()             WEAK ALIAS("isrUnused");
void isrUsb()             WEAK ALIAS("isrDefaultUsb");
void isrUsbCharge()       WEAK ALIAS("isrUnused");
void isrDac0()            WEAK ALIAS("isrUnused");
void isrDac1()            WEAK ALIAS("isrUnused");
void isrTsi0()            WEAK ALIAS("isrUnused");
void isrMcg()             WEAK ALIAS("isrUnused");
void isrLptmr()           WEAK ALIAS("isrUnused");
void isrPorta()           WEAK ALIAS("isrDefaultPortA");
void isrPortb()           WEAK ALIAS("isrDefaultPortB");
void isrPortc()           WEAK ALIAS("isrDefaultPortC");
void isrPortd()           WEAK ALIAS("isrDefaultPortD");
void isrPorte()           WEAK ALIAS("isrDefaultPortE");
void isrSoftware()        WEAK ALIAS("isrUnused");

#ifdef __cplusplus
	} //extern "C"
#endif
