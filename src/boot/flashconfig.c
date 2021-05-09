/** Flash config settings for the MK20 chip.
 *
 *******************************************************************************
 *                                     _     //` `\                            *
 *                              _,-"\%   // /``\`\              DANGER!        *
 *                         ~^~ >__^  |% // /  } `\`\                           *
 *     HERE                       )  )%// / }  } }`\`\                         *
 *      BE                       /  (%/'/.\_/\_/\_/\`/                         *
 *   DRAGONS!                    (    '         `-._`                          *
 *                               \   ,     (  \   _`-.__.-;%>                  *
 *                              /_`\ \      `\ \." `-..-'`                     *
 *                             ``` /_/`"-=-'`/_/                               *
 *                                ```       ```  art by jgs                    *
 *******************************************************************************
 * This section defines the flash protection (security) settings, which can be
 * used to lock sectors of flash memory, rendering them unreadable by external
 * means and unprivileged code. Incorrect settings could render your flash
 * memory unusable!
 *
 * DO NOT CHANGE THESE SETTINGS UNLESS YOU'RE SURE YOU KNOW WHAT YOU'RE DOING.
 */
#include "startup.h"

#ifdef __cplusplus
	extern "C" {
#endif

SECTION(".flashconfig") USED_SYMBOL const struct {
	uint8_t KEY[8]; //secret Backdoor Key that can be used (when enabled in
		//FSEC) to reset flash security settings, if using those.
	uint8_t FPROT[4]; //Program Flash Protection bytes
		//each bit corresponds to 1/32 of Program Flash memory
		//0 = protected (read-only)
		//1 = not protected (rewritable/erasable)
		//in privileged mode (NVM Special mode), the protection flags can be
		//changed (at memory address 0x4002001n if I'm reading this correctly), so it's possible to unprotect
		//so it's possible to unprotect and rewrite the Flash Config area.
	uint8_t FSEC; //Flash Security Byte
		//bits 7-6: KEYEN: Backdoor Key Enable
		//  00 = disable
		//  01 = disable (preferred value)
		//  10 = enable
		//  11 = disable
		//bits 5-4: MEEN: Mass Erase Enable
		//  10 = disable, else = enable
		//bits 3-2: FSLACC (Freescale Failure Analysis Access Code)
		//  00 or 11 = granted, else = denied
		//bits 1-0: SEC (Flash Security)
		//  10 = unsecure, else = secure
	uint8_t FOPT; //Flash Nonvolatile Option Byte
		//bits 7-3: reserved
		//bit 2: NMI_DIS: 0=disable NMI interrupts
		//bit 1: EZPORT_DIS: 0=disable EzPort
		//bit 0: LPBOOT: whether to boot in low-power mode. controls default
		//states of some clock dividers (SIM_CLKDIV1) at reset:
		//  0 (low power mode):
		//    OUTDIV1 (core  clock divider) = 0x7 (divide by 8)
		//    OUTDIV2 (bus   clock divider) = 0x7 (divide by 8)
		//    OUTDIV4 (flash clock divider) = 0xF (divide by 16)
		//  1 (normal mode):
		//    OUTDIV1 (core  clock divider) = 0x0 (divide by 1)
		//    OUTDIV2 (bus   clock divider) = 0x0 (divide by 1)
		//    OUTDIV4 (flash clock divider) = 0x1 (divide by 2)
	uint8_t FEPROT; //EEPROM Protection Byte (same as FPROT but for EEPROM)
	uint8_t FDPROT; //Data Flash Protection byte (FPROT but for Data Flash)
} PACKED flashConfig = {
	//default settings: disable all security features.
	//you might change these if you're building some gadget that you want to
	//try to protect the contents of.
	.KEY    = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	.FPROT  = {0xFF, 0xFF, 0xFF, 0xFF},
	.FSEC   = 0xFE,
	.FOPT   = 0xFF,
	.FEPROT = 0xFF,
	.FDPROT = 0xFF,
};

#ifdef __cplusplus
	} //extern "C"
#endif
