#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

static struct {
	uint32_t poly;
	uint32_t seed;
	uint32_t flags;
} crc_types[] = {

	//CRC_TYPE_CRC32
	{0x04C11DB7, 0xFFFFFFFF,
		CRC_WR_TRS_ALL | CRC_RD_TRS_ALL | CRC_CPL | CRC_SIZE_32},

	//CRC_TYPE_BZIP2
	{0x04C11DB7, 0xFFFFFFFF,
		CRC_WR_TRS_NONE | CRC_RD_TRS_NONE | CRC_CPL | CRC_SIZE_32},

	//CRC_TYPE_CRC32C
	{0x1EDC6F41, 0xFFFFFFFF,
		CRC_WR_TRS_ALL | CRC_RD_TRS_ALL | CRC_CPL | CRC_SIZE_32},

	//CRC_TYPE_CRC32D
	{0xA833982B, 0xFFFFFFFF,
		CRC_WR_TRS_ALL | CRC_RD_TRS_ALL | CRC_CPL | CRC_SIZE_32},

	//CRC_TYPE_MPEG2
	{0x04C11DB7, 0xFFFFFFFF,
		CRC_WR_TRS_NONE | CRC_RD_TRS_NONE | CRC_SIZE_32},

	//CRC_TYPE_CKSUM
	{0x04C11DB7, 0x00000000,
		CRC_WR_TRS_NONE | CRC_RD_TRS_NONE | CRC_CPL | CRC_SIZE_32},
		//note, posix `cksum` program appends an 0x09 byte to the input data

	//CRC_TYPE_CRC32Q
	{0x814141AB, 0x00000000,
		CRC_WR_TRS_NONE | CRC_RD_TRS_NONE | CRC_SIZE_32},

	//CRC_TYPE_JAMCRC
	{0x04C11DB7, 0xFFFFFFFF,
		CRC_WR_TRS_ALL | CRC_RD_TRS_ALL | CRC_SIZE_32},

	//CRC_TYPE_XFER
	{0x000000AF, 0x00000000,
		CRC_WR_TRS_NONE | CRC_RD_TRS_NONE | CRC_SIZE_32},
};


int kinetis_crcHwInitCustom(uint32_t flags, uint32_t poly, uint32_t seed) {
	SIM_SCGC6 |= SIM_SCGC6_CRC; //turn on CRC module
	CRC_CTRL = flags | CRC_SEED;
	if(flags & CRC_SIZE_32) {
		CRC_GPOLY = poly;
		CRC_CRC   = seed;
	}
	else {
		CRC_GPOLY16 = poly;
		CRC_CRC16   = seed;
	}
	CRC_CTRL &= ~CRC_SEED; //not writing a seed
    return 0;
}

/** Initialize the hardware CRC module and configure for specified CRC type.
 */
int kinetis_crcHwInit(CRC_TYPE type) {
	return kinetis_crcHwInitCustom(crc_types[type].flags, crc_types[type].poly,
		crc_types[type].seed);
}


/** Shut down the hardware CRC module.
 */
int kinetis_crcHwShutdown() {
	SIM_SCGC6 &= ~SIM_SCGC6_CRC; //turn off CRC module
    return 0;
}

/** Feed a uint8_t to the hardware CRC module.
 */
int kinetis_crcHwInput8(uint8_t data) {
	CRC_CRC8 = data;
    return 0;
}

/** Feed a uint16_t to the hardware CRC module.
 */
int kinetis_crcHwInput16(uint16_t data) {
	CRC_CRC16 = data;
    return 0;
}

/** Feed a uint32_t to the hardware CRC module.
 */
int kinetis_crcHwInput32(uint32_t data) {
	CRC_CRC = data;
    return 0;
}

/** Read 8-bit result from hardware CRC module.
 */
int kinetis_crcHwEnd8(uint8_t *out) {
	*out = CRC_CRC8;
    return 0;
}

/** Read 16-bit result from hardware CRC module.
 */
int kinetis_crcHwEnd16(uint16_t *out) {
	*out = CRC_CRC16;
    return 0;
}

/** Read 32-bit result from hardware CRC module.
 */
int kinetis_crcHwEnd32(uint32_t *out) {
	*out = CRC_CRC;
    return 0;
}


int kinetis_crcTest(void) {
	//Internal function to test CRC computation.
	//data from:
	//http://reveng.sourceforge.net/crc-catalogue/17plus.htm#crc.cat.crc-32c

	printf("CRC test start.\r\n");
	struct {
		const char *name;
		CRC_TYPE type;
		uint32_t expect;
	} tests[] = {
		{"CRC-32",        CRC_TYPE_CRC32,  0xCBF43926},
		{"CRC-32/BZIP2",  CRC_TYPE_BZIP2,  0xFC891918},
		{"CRC-32C",       CRC_TYPE_CRC32C, 0xE3069283},
		{"CRC-32D",       CRC_TYPE_CRC32D, 0x87315576},
		{"CRC-32/MPEG-2", CRC_TYPE_MPEG2,  0x0376E6E7},
		{"POSIX CKSUM",   CRC_TYPE_CKSUM,  0x765E7680},
		{"CRC-32Q",       CRC_TYPE_CRC32Q, 0x3010BF7F},
		{"JAMCRC",        CRC_TYPE_JAMCRC, 0x340BC6D9},
		{"XFER",          CRC_TYPE_XFER,   0xBD0BE338},
		{NULL, CRC_TYPE_CRC32, 0} //end of list
	};

	static const char *data = "123456789";
	for(int i=0; tests[i].name; i++) {
		crcHwInit(tests[i].type);
		for(int i=0; i<9; i++) crcHwInput8(data[i]);

		uint32_t res = crcHwEnd32();
		if(res == tests[i].expect)
			 printf("%-13s: %08lX (OK)\r\n", tests[i].name, res);
		else printf("%-13s: %08lX (%08lX, err: %08lX)\r\n",
			tests[i].name, res, tests[i].expect, res ^ tests[i].expect);

		//XXX test software functions
	}

	crcHwShutdown();
	printf("CRC test finished.\r\n");
    return 0;
}


#ifdef __cplusplus
	} //extern "C"
#endif
