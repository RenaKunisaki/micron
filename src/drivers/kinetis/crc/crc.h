#ifndef _MICRON_CRC_H_
#define _MICRON_CRC_H_

#ifdef __cplusplus
	extern "C" {
#endif

//from kinetis.h:
//#define CRC_CRC			(*(volatile uint32_t *)0x40032000) // CRC Data register
//#define CRC_GPOLY		(*(volatile uint32_t *)0x40032004) // CRC Polynomial register
//#define CRC_CTRL		(*(volatile uint32_t *)0x40032008) // CRC Control register

//additional:
#define CRC_CRC8    (*(volatile uint8_t* )0x40032000)
#define CRC_CRC16   (*(volatile uint16_t*)0x40032000)
#define CRC_GPOLY16 (*(volatile uint16_t*)0x40032004)

//flags, with casts because lolC++11 narrowing conversion is dumb
//transpose writes
#define CRC_WR_TRS_NONE  ((uint32_t)(0 << 30)) //don't transpose
#define CRC_WR_TRS_BITS  ((uint32_t)(1 << 30)) //transpose bits
#define CRC_WR_TRS_ALL   ((uint32_t)(2 << 30)) //transpose bits and bytes
#define CRC_WR_TRS_BYTES ((uint32_t)(3 << 30)) //transpose bytes (order is weird here)
//transpose reads
#define CRC_RD_TRS_NONE  ((uint32_t)(0 << 28)) //don't transpose
#define CRC_RD_TRS_BITS  ((uint32_t)(1 << 28)) //transpose bits
#define CRC_RD_TRS_ALL   ((uint32_t)(2 << 28)) //transpose bits and bytes
#define CRC_RD_TRS_BYTES ((uint32_t)(3 << 28)) //transpose bytes
//other
#define CRC_CPL     ((uint32_t)BIT(26)) //complement result on read
#define CRC_SEED    ((uint32_t)BIT(25)) //write seed instead of data
#define CRC_SIZE_16 ((uint32_t)0)       //compute 16-bit CRC
#define CRC_SIZE_32 ((uint32_t)BIT(24)) //compute 32-bit CRC

typedef enum {
	CRC_TYPE_CRC32 = 0,
	CRC_TYPE_BZIP2,
	CRC_TYPE_CRC32C,
	CRC_TYPE_CRC32D,
	CRC_TYPE_MPEG2,
	CRC_TYPE_CKSUM,
	CRC_TYPE_CRC32Q,
	CRC_TYPE_JAMCRC,
	CRC_TYPE_XFER,
	NUM_CRC_TYPES
} CRC_TYPE;

//crc.c
uint32_t crc32Cont(uint32_t crc, const void *buf, size_t size);
uint32_t crc32(const void *buf, size_t size);
void crcHwInitCustom(uint32_t flags, uint32_t poly, uint32_t seed);
void crcHwInit(CRC_TYPE type);
void crcHwShutdown();
void crcHwInput8(uint8_t data);
void crcHwInput16(uint16_t data);
void crcHwInput32(uint32_t data);
uint8_t crcHwEnd8();
uint16_t crcHwEnd16();
uint32_t crcHwEnd32();
void crcTest();

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_CRC_H_
