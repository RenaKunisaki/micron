#include <micron.h>

#ifdef __cplusplus
	extern "C" {
#endif

/* Pin mapping - Teensy pin numbers to port labels/numbers:
 * Top of Teensy board:
 *    	[USB port]
 *  0 = B16
 *  1 = B17
 *  2 = D0    C2 = 23
 *  3 = A12   C1 = 22
 *  4 = A13   D6 = 21
 *  5 = D7    D5 = 20
 *  6 = D4    B2 = 19
 *  7 = D2    B3 = 18
 *  8 = D3    B1 = 17
 *  9 = C3    B0 = 16
 * 10 = C4    C0 = 15
 * 11 = C6    D1 = 14
 * 12 = C7    C5 = 13
 *
 * Bottom (inside):
 * 28 = C8    C10 = 29
 * 27 = C9    C11 = 30
 * 26 = E1    E0  = 31
 * 25 = B19   B18 = 32
 * 24 = A5    A4  = 33
 * [...other things...]
 *      [USB port]
 */

//Base address for each pin's I/O registers.
#define _PBR(n) &CORE_PIN ## n ## _PDOR
volatile uint32_t* const pinBaseAddr[] = {
	_PBR( 0), _PBR( 1), _PBR( 2), _PBR( 3), _PBR( 4), _PBR( 5), _PBR( 6),
	_PBR( 7), _PBR( 8), _PBR( 9), _PBR(10), _PBR(11), _PBR(12), _PBR(13),
	_PBR(14), _PBR(15), _PBR(16), _PBR(17), _PBR(18), _PBR(19), _PBR(20),
	_PBR(21), _PBR(22), _PBR(23), _PBR(24), _PBR(25), _PBR(26), _PBR(27),
	_PBR(28), _PBR(29), _PBR(30), _PBR(31), _PBR(32), _PBR(33),
};
#undef _PBR


//config register for each pin.
#define _PCR(n) &CORE_PIN ## n ## _CONFIG
volatile uint32_t* const pinConfigAddr[] = {
	_PCR( 0), _PCR( 1), _PCR( 2), _PCR( 3), _PCR( 4), _PCR( 5), _PCR( 6),
	_PCR( 7), _PCR( 8), _PCR( 9), _PCR(10), _PCR(11), _PCR(12), _PCR(13),
	_PCR(14), _PCR(15), _PCR(16), _PCR(17), _PCR(18), _PCR(19), _PCR(20),
	_PCR(21), _PCR(22), _PCR(23), _PCR(24), _PCR(25), _PCR(26), _PCR(27),
	_PCR(28), _PCR(29), _PCR(30), _PCR(31), _PCR(32), _PCR(33),
};
#undef _PCR


#ifdef __cplusplus
	} //extern "C"
#endif
