#ifndef _MICRON_REBOOT_H_
#define _MICRON_REBOOT_H_

#ifdef __cplusplus
	extern "C" {
#endif


//Reboot into bootloader, to download new program.
INLINE COLD NORETURN void osBootloader() {
	__asm__ volatile("bkpt");
	UNREACHABLE;
	while(1);
}


//Reset, but don't return to bootloader.
INLINE COLD NORETURN void osReboot() {
	//this is some ARM magic register to request a reboot.
	((*(volatile uint32_t*)0xE000ED0C) = 0x05FA0004);
	UNREACHABLE;
	while(1);
}


#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_REBOOT_H_
