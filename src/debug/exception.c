//Exception handler; prints exception info to serial port.
#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

static const char *hex = "0123456789ABCDEF";
extern MicronUartState *_uartState[NUM_UART];

extern char   __heap_start;
extern char   __heap_end;
extern size_t __malloc_allocated;
extern char  *__brkval;
extern size_t __malloc_margin;

void blinkNum(int n) {
	while(n --> 0) {
		gpioSetPinOutput(13, 1);
		delayUS(250000);
		gpioSetPinOutput(13, 0);
		delayUS(250000);
	}
}

//CPU register names
//static const char *regNames[] = {
//	" r0", " r1", " r2", " r3", " r4", " r5", " r6", " r7",
//	" r8", " r9", "r10", "r11", "r12", " sp", " lr", " pc"
//};

//names of exception codes
static const char *ipsr_values[] = {
	"Thread",    "Rsvd1",    "NMI",    "HardFault",
	"MemManage", "BusFault", "Rsvd7",  "Rsvd8",
	"Rsvd9",     "Rsvd10",   "SVCall", "Debug",
	"Rsvd13",    "PendSV",   "SysTick",
};


static void serialInitIsr() {
	//init serial
	//we can't use the serial API here because it requires interrupts.
	//we also can't rely on anything already being set up properly.
    //XXX HAL
	if(SIM_SCGC4 & SIM_SCGC4_UART0) return; //already set up

	KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(0);

	//gpioSetPinMode(UART0_TX_PIN,
	//	PCR_DRIVE_STRENGTH_HI | PCR_SLEW_SLOW | PCR_MUX(3), PIN_MODE_OUTPUT);
	//gpioSetPinMode(UART0_TX_PIN, PCR_PULLUP | PCR_FILTER | PCR_MUX(3), PIN_MODE_INPUT);
	gpioSetPinOutput(UART0_TX_PIN, 1); //tx assert

	SIM_SCGC4 |= SIM_SCGC4_UART0;
	regs->C1 = UART_C1_ILT;
	regs->C2 = 0; //clear
	regs->C3 = 0;
	regs->PFIFO  = UART_PFIFO_TXFE;
	delayUS(5000);
	regs->C2 = UART_C2_TE; //disable transmit interrupt, enable transmitter

	serialSetBaud(0, 460800);
}


static void serialPutsIsr(const char *str) {
	KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(0);
	while(*str) {
		while(regs->TCFIFO >= 8) delayUS(1000);
		static UNUSED_SYMBOL uint8_t dummy = regs->S1; //clear Tx Data Reg Empty flag.
		regs->D = *str;
		str++;
		//delayUS(5000);
	}
}

static void serialPutHexIsr(const char *prefix, int digits, uint32_t data) {
	if(prefix) serialPutsIsr(prefix);

	char buf[64];
	for(size_t i=0; i<sizeof(buf); i++) buf[i] = 0;
	for(int i=digits-1; i>=0; i--) {
		buf[i] = hex[data & 0xF];
		data >>= 4;
	}
	serialPutsIsr(buf);
}

static void serialPutU32Isr(const char *prefix, uint32_t addr) {
	serialPutHexIsr(prefix, 8, *(volatile uint32_t*)addr);
}

//these might not get used, but stop warning about it kthx
UNUSED_SYMBOL static void serialPutU16Isr(const char *prefix, uint32_t addr) {
	serialPutHexIsr(prefix, 4, *(volatile uint16_t*)addr);
}

UNUSED_SYMBOL static void serialPutU8Isr(const char *prefix, uint32_t addr) {
	serialPutHexIsr(prefix, 2, *(volatile uint8_t*)addr);
}


static void dumpRegs(volatile uint32_t *sp) {
	serialPutHexIsr("\r\nR0 = ", 8, sp[0]);
	serialPutHexIsr(   " R1 = ", 8, sp[1]);
	serialPutHexIsr(   " R2 = ", 8, sp[2]);
	serialPutHexIsr(   " R3 = ", 8, sp[3]);
	serialPutHexIsr("\r\nR12= ", 8, sp[4]);
	serialPutHexIsr(   " LR = ", 8, sp[5]);
	serialPutHexIsr(   " PC = ", 8, sp[6]);
	serialPutHexIsr(   " PSR= ", 8, sp[7]);
	serialPutU32Isr("\r\nICSR=", 0xE000ED04);
	serialPutU32Isr(   " HFSR=", 0xE000ED2C);
	serialPutU32Isr(   " CFSR=", 0xE000ED28);
	serialPutU32Isr(   " AFSR=", 0xE000ED3C);
	//serialPutU32Isr(   " MMAR=", 0xE000ED34);
	//serialPutU32Isr(   " BFAR=", 0xE000ED38);
	//serialPutHexIsr (" LR=", 8, (uint32_t)lr);
}


static void dumpStack(volatile uint32_t *sp) {
	serialPutsIsr("\r\nStack Dump:\r\n");
	static char buf[4] = {0, 0, 0, 0}; //static to work around gcc bug
	for(int i=0; i<256; i += 8) {
		serialPutHexIsr(NULL, 8, (uint32_t)sp);
		serialPutsIsr(":");

		for(int j=0; j<8; j++) {
            //if in ROM, color red
			if(*sp > 0x400 && *sp < 0x7FFF) {
				serialPutsIsr("\e[1;31m");
			}
			serialPutHexIsr(j == 4 ? "  " : " ", 8, *sp);
			if(*sp > 0x400 && *sp < 0x7FFF) {
				serialPutsIsr("\e[0m");
			}
			sp++;
			if(sp >= &_estack) break;
		}

		serialPutsIsr("  ");
		volatile uint8_t *txt = (uint8_t*)(sp - 8);
		for(int j=0; j<32; j++) {
			/* if(txt >= (volatile uint8_t*)&_estack) buf[0] = ' ';
			else*/ buf[0] = *(txt++);
			if(buf[0] < 0x20 || buf[0] > 0x7E) buf[0] = '.';
			buf[1] = ((j & 0x3) == 0x3) ? ' ' : '\0';
			buf[2] = ((j & 0xF) == 0xF) ? ' ' : '\0';
			serialPutsIsr(buf);
		}

		serialPutsIsr("\r\n");
		if(sp >= &_estack) break;
	}
}


//default handler for undefined/unused ISRs
extern "C" COLD NORETURN NAKED void hardfault(volatile uint32_t *sp) {
	sp += 2; //??? XXX what's the extra 2 words pushed here?

	__disable_irq();

	//disable watchdog
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
	__asm__ volatile ("nop"); //must wait 2 cycles after unlocking
	__asm__ volatile ("nop"); //before any other watchdog operation.
	WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE;

	blinkNum(5);
	serialInitIsr();

	serialPutsIsr("\r\n *** SYSTEM FAILURE *** \r\nFault in ");
	uint32_t ICSR = *(volatile uint32_t*)0xE000ED04;
	uint8_t exc_nr = ICSR & 0xFF; //exception number
	if(exc_nr > 15) serialPutHexIsr("IRQ 0x", 2, exc_nr - 16);
	else serialPutsIsr(ipsr_values[exc_nr]);

	//serialPutHexIsr(" SP=", 8, (uint32_t)sp);

	uint16_t UFSR = *(uint16_t*)0xE000ED2A; //usage fault status register
	if(UFSR & BIT(9)) serialPutsIsr(": divide by zero");
	if(UFSR & BIT(8)) serialPutsIsr(": unaligned access");
	if(UFSR & BIT(3)) serialPutsIsr(": no coprocessor");
	if(UFSR & BIT(2)) serialPutsIsr(": invalid PC");
	if(UFSR & BIT(1)) serialPutsIsr(": invalid state");
	if(UFSR & BIT(0)) serialPutsIsr(": undefined instruction");

	uint8_t MMFSR = *(uint8_t*)0xE000ED29; //memory mgmt fault status register
	if(MMFSR & BIT(7)) {
		serialPutU32Isr("\r\nMem Fault at: ", 0xE000ED34);
		if(MMFSR & BIT(2)) serialPutsIsr(" (inexact)");
		if(MMFSR & BIT(4)) serialPutsIsr(" on exception enter");
		if(MMFSR & BIT(3)) serialPutsIsr(" on exception return");
		if(MMFSR & BIT(0)) serialPutsIsr(" on exec");
	}

	uint8_t BFSR = *(uint8_t*)0xE000ED29; //bus fault status register
	if(BFSR & BIT(7)) {
		serialPutU32Isr("\r\nBus Fault at: ", 0xE000ED38);
		if(BFSR & BIT(4)) serialPutsIsr(" on exception enter");
		if(BFSR & BIT(3)) serialPutsIsr(" on exception return");
		if(BFSR & BIT(0)) serialPutsIsr(" on exec");
	}

	serialPutHexIsr("\r\nALLOC 0x", 4, __malloc_allocated);
	serialPutHexIsr(" / 0x", 4, &__heap_end - &__heap_start);
	serialPutHexIsr("; BRK 0x", 8, (uint32_t)__brkval);
	serialPutHexIsr("\r\nHEAP  0x", 8, (uint32_t)&__heap_start);
	serialPutHexIsr(" - 0x", 8, (uint32_t)&__heap_end);
	serialPutHexIsr("; margin 0x", 8, (uint32_t)__malloc_margin);
	serialPutHexIsr("; free 0x", 8, (uint32_t)(&__heap_end - __brkval));
	serialPutHexIsr("\r\nSTACK 0x", 8, (uint32_t)&_estack);

	dumpRegs(sp);
	dumpStack(sp);

	serialPutsIsr("\r\nRebooting.\r\n");
	osBootloader();
	while(1) {
		blinkNum(1);
	}
}

//actual isrFault handler, just calls hardfault() with SP as parameter.
COLD NORETURN ISRFUNC NAKED void isrFault() {
	__asm__ volatile(
		//"TST LR, #4\n"
		//"ITE EQ\n"
		//"MRSEQ R0, MSP\n" //XXX is this correct?
		//"MRSNE R0, PSP\n"
		"mov R0, sp\n"
		"B hardfault\n"
		:  :: );
	while(1); //to convince gcc that this function really never returns
	//probably it can't deduce that it can never be returned to (since
	//hardfault() never returns) because we've written the branch in ASM.
}


COLD void* on_malloc_fail(size_t len) {
	if(stderr) {
		uint32_t heapSize = &__heap_end - &__heap_start;
		fprintf(stderr, "*** malloc(%zd) failed; alloc=%d/%ld, free %ld\r\n",
			len, __malloc_allocated, heapSize, heapSize - __malloc_allocated);
	}
	return NULL;
}

#ifdef __cplusplus
	} //extern "C"
#endif
