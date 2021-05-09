#ifndef _MICRON_HAL_CRC_H_
#define _MICRON_HAL_CRC_H_

#ifdef __cplusplus
	extern "C" {
#endif

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

//softcrc32.c
uint32_t crc32(const void *buf, size_t size);
uint32_t crc32Cont(uint32_t crc, const void *buf, size_t size);

//crc.c
int crcHwInitCustom(uint32_t port, uint32_t flags, uint32_t poly, uint32_t seed);
int crcHwInit(uint32_t port, CRC_TYPE type);
int crcHwShutdown(uint32_t port);
int crcHwInput8(uint32_t port, uint8_t data);
int crcHwInput16(uint32_t port, uint16_t data);
int crcHwInput32(uint32_t port, uint32_t data);
uint8_t crcHwEnd8(uint32_t port);
uint16_t crcHwEnd16(uint32_t port);
uint32_t crcHwEnd32(uint32_t port);
int crcTest(void);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_HAL_CRC_H_
