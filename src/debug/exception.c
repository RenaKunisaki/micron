//Exception handler; prints exception info to serial port.
#ifdef __cplusplus
	extern "C" {
#endif
#include "../micron.h"

static const char *hex = "0123456789ABCDEF";
extern micron_uart_state *uart_state[NUM_UART];

extern char   __heap_start;
extern char   __heap_end;
extern size_t __malloc_allocated;
extern char  *__brkval;
extern size_t __malloc_margin;

void blinkNum(int n) {
	while(n --> 0) {
		digitalWrite(13, 1);
		delayUS(250000);
		digitalWrite(13, 0);
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


static void serial_init_isr() {
	//init serial
	//we can't use the serial API here because it requires interrupts.
	//we also can't rely on anything already being set up properly.
	if(SIM_SCGC4 & SIM_SCGC4_UART0) return; //already set up

	KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(0);

	setPinMode(UART0_TX_PIN,
		PCR_DRIVE_STRENGTH_HI | PCR_SLEW_SLOW | PCR_MUX(3), OUTPUT);
	setPinMode(UART0_TX_PIN, PCR_PULLUP | PCR_FILTER | PCR_MUX(3), INPUT);
	digitalWrite(UART0_TX_PIN, 1); //tx assert

	SIM_SCGC4 |= SIM_SCGC4_UART0;
	regs->C1 = UART_C1_ILT;
	regs->C2 = 0; //clear
	regs->C3 = 0;
	regs->PFIFO  = UART_PFIFO_TXFE;
	delayUS(5000);
	regs->C2 = UART_C2_TE; //disable transmit interrupt, enable transmitter

	serialSetBaud(0, 460800);
}


static void serial_puts_isr(const char *str) {
	KINETISK_UART_t *regs = (KINETISK_UART_t*)UART_REG_BASE(0);
	while(*str) {
		while(regs->TCFIFO >= 8) delayUS(1000);
		static UNUSED uint8_t dummy = regs->S1; //clear Tx Data Reg Empty flag.
		regs->D = *str;
		str++;
		//delayUS(5000);
	}
}

static void serial_puthex_isr(const char *prefix, int digits, uint32_t data) {
	if(prefix) serial_puts_isr(prefix);

	char buf[64];
	for(size_t i=0; i<sizeof(buf); i++) buf[i] = 0;
	for(int i=digits-1; i>=0; i--) {
		buf[i] = hex[data & 0xF];
		data >>= 4;
	}
	serial_puts_isr(buf);
}

static void serial_put_u32_isr(const char *prefix, uint32_t addr) {
	serial_puthex_isr(prefix, 8, *(volatile uint32_t*)addr);
}

//these might not get used, but stop warning about it kthx
UNUSED static void serial_put_u16_isr(const char *prefix, uint32_t addr) {
	serial_puthex_isr(prefix, 4, *(volatile uint16_t*)addr);
}

UNUSED static void serial_put_u8_isr(const char *prefix, uint32_t addr) {
	serial_puthex_isr(prefix, 2, *(volatile uint8_t*)addr);
}


static void dump_regs(volatile uint32_t *sp) {
	serial_puthex_isr ("\r\nR0 = ", 8, sp[0]);
	serial_puthex_isr (   " R1 = ", 8, sp[1]);
	serial_puthex_isr (   " R2 = ", 8, sp[2]);
	serial_puthex_isr (   " R3 = ", 8, sp[3]);
	serial_puthex_isr ("\r\nR12= ", 8, sp[4]);
	serial_puthex_isr (   " LR = ", 8, sp[5]);
	serial_puthex_isr (   " PC = ", 8, sp[6]);
	serial_puthex_isr (   " PSR= ", 8, sp[7]);
	serial_put_u32_isr("\r\nICSR=", 0xE000ED04);
	serial_put_u32_isr(   " HFSR=", 0xE000ED2C);
	serial_put_u32_isr(   " CFSR=", 0xE000ED28);
	serial_put_u32_isr(   " AFSR=", 0xE000ED3C);
	//serial_put_u32_isr(   " MMAR=", 0xE000ED34);
	//serial_put_u32_isr(   " BFAR=", 0xE000ED38);
	//serial_puthex_isr (" LR=", 8, (uint32_t)lr);
}


static void dump_stack(volatile uint32_t *sp) {
	serial_puts_isr("\r\nStack Dump:\r\n");
	static char buf[4] = {0, 0, 0, 0}; //static to work around gcc bug
	for(int i=0; i<256; i += 8) {
		serial_puthex_isr(NULL, 8, (uint32_t)sp);
		serial_puts_isr(":");

		for(int j=0; j<8; j++) {
            //if in ROM, color red
			if(*sp > 0x400 && *sp < 0x7FFF) {
				serial_puts_isr("\e[1;31m");
			}
			serial_puthex_isr(j == 4 ? "  " : " ", 8, *sp);
			if(*sp > 0x400 && *sp < 0x7FFF) {
				serial_puts_isr("\e[0m");
			}
			sp++;
			if(sp >= &_estack) break;
		}

		serial_puts_isr("  ");
		volatile uint8_t *txt = (uint8_t*)(sp - 8);
		for(int j=0; j<32; j++) {
			/* if(txt >= (volatile uint8_t*)&_estack) buf[0] = ' ';
			else*/ buf[0] = *(txt++);
			if(buf[0] < 0x20 || buf[0] > 0x7E) buf[0] = '.';
			buf[1] = ((j & 0x3) == 0x3) ? ' ' : '\0';
			buf[2] = ((j & 0xF) == 0xF) ? ' ' : '\0';
			serial_puts_isr(buf);
		}

		serial_puts_isr("\r\n");
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
	serial_init_isr();

	serial_puts_isr("\r\n *** ISR FAULT *** \r\n");
	uint32_t ICSR = *(volatile uint32_t*)0xE000ED04;
	uint8_t exc_nr = ICSR & 0xFF; //exception number
	if(exc_nr > 15) serial_puthex_isr("IRQ 0x", 2, exc_nr - 16);
	else serial_puts_isr(ipsr_values[exc_nr]);

	//serial_puthex_isr(" SP=", 8, (uint32_t)sp);

	uint16_t UFSR = *(uint16_t*)0xE000ED2A; //usage fault status register
	if(UFSR & BIT(9)) serial_puts_isr(": divide by zero");
	if(UFSR & BIT(8)) serial_puts_isr(": unaligned access");
	if(UFSR & BIT(3)) serial_puts_isr(": no coprocessor");
	if(UFSR & BIT(2)) serial_puts_isr(": invalid PC");
	if(UFSR & BIT(1)) serial_puts_isr(": invalid state");
	if(UFSR & BIT(0)) serial_puts_isr(": undefined instruction");

	uint8_t MMFSR = *(uint8_t*)0xE000ED29; //memory mgmt fault status register
	if(MMFSR & BIT(7)) {
		serial_put_u32_isr("\r\nMem Fault at: ", 0xE000ED34);
		if(MMFSR & BIT(2)) serial_puts_isr(" (inexact)");
		if(MMFSR & BIT(4)) serial_puts_isr(" on exception enter");
		if(MMFSR & BIT(3)) serial_puts_isr(" on exception return");
		if(MMFSR & BIT(0)) serial_puts_isr(" on exec");
	}

	uint8_t BFSR = *(uint8_t*)0xE000ED29; //bus fault status register
	if(BFSR & BIT(7)) {
		serial_put_u32_isr("\r\nBus Fault at: ", 0xE000ED38);
		if(BFSR & BIT(4)) serial_puts_isr(" on exception enter");
		if(BFSR & BIT(3)) serial_puts_isr(" on exception return");
		if(BFSR & BIT(0)) serial_puts_isr(" on exec");
	}

	serial_puthex_isr("\r\nALLOC 0x", 4, __malloc_allocated);
	serial_puthex_isr(" / 0x", 4, &__heap_end - &__heap_start);
	serial_puthex_isr("; BRK 0x", 8, (uint32_t)__brkval);
	serial_puthex_isr("\r\nHEAP  0x", 8, (uint32_t)&__heap_start);
	serial_puthex_isr(" - 0x", 8, (uint32_t)&__heap_end);
	serial_puthex_isr("; margin 0x", 8, (uint32_t)__malloc_margin);
	serial_puthex_isr("; free 0x", 8, (uint32_t)(&__heap_end - __brkval));
	serial_puthex_isr("\r\nSTACK 0x", 8, (uint32_t)&_estack);

	dump_regs(sp);
	dump_stack(sp);

	serial_puts_isr("\r\nRebooting.\r\n");
	osBootloader();
	while(1) {
		blinkNum(1);
	}
}

//actual isrFault handler, just calls hardfault() with SP as parameter.
COLD NORETURN ISR NAKED void isrFault() {
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
