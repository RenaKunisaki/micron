#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

//lookup table for PBR, BR, DBR, and bus clock divisor for given SPI baud rates.
//pbr values are 2, 3, 5, 7 but the actual values written to PBR are 0, 1, 2, 3,
//which conveniently fits in 2 bits.
#define SPI_BAUD_LUT(s, pbr, br, dbr) \
    ((s) | ((pbr) << 18) | ((br) << 20) | ((dbr) << 24))

//BR register values for corresponding actual values
#define BR_VAL_2 0
#define BR_VAL_4 1
#define BR_VAL_6 2
#define BR_VAL_8 3
#define BR_VAL_16 4
#define BR_VAL_32 5
#define BR_VAL_64 6
#define BR_VAL_128 7
#define BR_VAL_256 8
#define BR_VAL_512 9
#define BR_VAL_1024 10
#define BR_VAL_2048 11
#define BR_VAL_4096 12
#define BR_VAL_8192 13
#define BR_VAL_16384 14
#define BR_VAL_32768 15

//XXX there's probably a clever way to calculate this...
const uint32_t _spiBaudLut[] = {
    //                s  PBR    BR        DBR
    SPI_BAUD_LUT(     2, 0,     BR_VAL_2, 1),
    SPI_BAUD_LUT(     3, 1,     BR_VAL_2, 1),
    SPI_BAUD_LUT(     4, 0,     BR_VAL_2, 0),
    SPI_BAUD_LUT(     5, 2,     BR_VAL_2, 1),
    SPI_BAUD_LUT(     6, 0,     BR_VAL_6, 1),
    SPI_BAUD_LUT(     7, 3,     BR_VAL_2, 1),
    SPI_BAUD_LUT(     8, 0,     BR_VAL_4, 0),
    SPI_BAUD_LUT(     9, 1,     BR_VAL_6, 1),
    SPI_BAUD_LUT(    10, 2,     BR_VAL_2, 0),
    SPI_BAUD_LUT(    12, 0,     BR_VAL_6, 0),
    SPI_BAUD_LUT(    15, 2,     BR_VAL_6, 1),
    SPI_BAUD_LUT(    14, 3,     BR_VAL_2, 0),
    SPI_BAUD_LUT(    16, 0,     BR_VAL_8, 0),
    SPI_BAUD_LUT(    18, 1,     BR_VAL_6, 0),
    SPI_BAUD_LUT(    20, 2,     BR_VAL_4, 0),
    SPI_BAUD_LUT(    21, 3,     BR_VAL_6, 1),
    SPI_BAUD_LUT(    24, 1,     BR_VAL_8, 0),
    SPI_BAUD_LUT(    28, 3,     BR_VAL_4, 0),
    SPI_BAUD_LUT(    30, 2,     BR_VAL_6, 0),
    SPI_BAUD_LUT(    32, 0,    BR_VAL_16, 0),
    SPI_BAUD_LUT(    40, 2,     BR_VAL_8, 0),
    SPI_BAUD_LUT(    42, 3,     BR_VAL_6, 0),
    SPI_BAUD_LUT(    48, 1,    BR_VAL_16, 0),
    SPI_BAUD_LUT(    56, 3,     BR_VAL_8, 0),
    SPI_BAUD_LUT(    64, 0,    BR_VAL_32, 0),
    SPI_BAUD_LUT(    80, 2,    BR_VAL_16, 0),
    SPI_BAUD_LUT(    96, 1,    BR_VAL_32, 0),
    SPI_BAUD_LUT(   112, 3,    BR_VAL_16, 0),
    SPI_BAUD_LUT(   128, 0,    BR_VAL_64, 0),
    SPI_BAUD_LUT(   160, 2,    BR_VAL_32, 0),
    SPI_BAUD_LUT(   192, 1,    BR_VAL_64, 0),
    SPI_BAUD_LUT(   224, 3,    BR_VAL_32, 0),
    SPI_BAUD_LUT(   256, 0,   BR_VAL_128, 0),
    SPI_BAUD_LUT(   320, 2,    BR_VAL_64, 0),
    SPI_BAUD_LUT(   384, 1,   BR_VAL_128, 0),
    SPI_BAUD_LUT(   448, 3,    BR_VAL_64, 0),
    SPI_BAUD_LUT(   512, 0,   BR_VAL_256, 0),
    SPI_BAUD_LUT(   640, 2,   BR_VAL_128, 0),
    SPI_BAUD_LUT(   768, 1,   BR_VAL_256, 0),
    SPI_BAUD_LUT(   896, 3,   BR_VAL_128, 0),
    SPI_BAUD_LUT(  1024, 0,   BR_VAL_512, 0),
    SPI_BAUD_LUT(  1280, 2,   BR_VAL_256, 0),
    SPI_BAUD_LUT(  1536, 1,   BR_VAL_512, 0),
    SPI_BAUD_LUT(  1792, 3,   BR_VAL_256, 0),
    SPI_BAUD_LUT(  2048, 0,  BR_VAL_1024, 0),
    SPI_BAUD_LUT(  2560, 2,   BR_VAL_512, 0),
    SPI_BAUD_LUT(  3072, 1,  BR_VAL_1024, 0),
    SPI_BAUD_LUT(  3584, 3,   BR_VAL_512, 0),
    SPI_BAUD_LUT(  4096, 0,  BR_VAL_2048, 0),
    SPI_BAUD_LUT(  5120, 2,  BR_VAL_1024, 0),
    SPI_BAUD_LUT(  6144, 1,  BR_VAL_2048, 0),
    SPI_BAUD_LUT(  7168, 3,  BR_VAL_1024, 0),
    SPI_BAUD_LUT(  8192, 0,  BR_VAL_4096, 0),
    SPI_BAUD_LUT( 10240, 2,  BR_VAL_2048, 0),
    SPI_BAUD_LUT( 12288, 1,  BR_VAL_4096, 0),
    SPI_BAUD_LUT( 14336, 3,  BR_VAL_2048, 0),
    SPI_BAUD_LUT( 16384, 0,  BR_VAL_8192, 0),
    SPI_BAUD_LUT( 20480, 2,  BR_VAL_4096, 0),
    SPI_BAUD_LUT( 24576, 1,  BR_VAL_8192, 0),
    SPI_BAUD_LUT( 28672, 3,  BR_VAL_4096, 0),
    SPI_BAUD_LUT( 32768, 0, BR_VAL_16384, 0),
    SPI_BAUD_LUT( 40960, 2,  BR_VAL_8192, 0),
    SPI_BAUD_LUT( 49152, 1, BR_VAL_16384, 0),
    SPI_BAUD_LUT( 57344, 3,  BR_VAL_8192, 0),
    SPI_BAUD_LUT( 65536, 0, BR_VAL_32768, 0),
    SPI_BAUD_LUT( 81920, 2, BR_VAL_16384, 0),
    SPI_BAUD_LUT( 98304, 1, BR_VAL_32768, 0),
    SPI_BAUD_LUT(114688, 3, BR_VAL_16384, 0),
    SPI_BAUD_LUT(163840, 2, BR_VAL_32768, 0),
    SPI_BAUD_LUT(229376, 3, BR_VAL_32768, 0),
    0 };

static int _decodeTableEntry(uint32_t raw, uint32_t *outPBR, uint32_t *outBR,
uint32_t *outDBR, uint32_t *outActualBaud, uint64_t f_sys) {
    // ((s) | ((pbr) << 18) | ((br) << 20) | ((dbr) << 24))
    uint32_t pbr = (raw >> 18) & 0x3;
    uint32_t br  = (raw >> 20) & 0xF;
    uint32_t dbr = (raw >> 24) & 0x1;
    *outPBR = pbr;
    *outBR  = br;
    *outDBR = dbr;

    static uint8_t pbrVals[] = {2, 3, 5, 7};
    if(br > 2) br = 1 << br;
    else if(br == 2) br = 6; //wtf
    else br = 2 << br;

    //slightly rearranged to avoid using floats
    *outActualBaud = (f_sys / pbrVals[pbr]) / (br * (dbr+1));
    //printf("SPI: actual baud = %d\r\n", *outActualBaud);
    return 0;
}

int kinetis_spiGetParamsForBaud(uint32_t baud, uint32_t *outPBR,
uint32_t *outBR, uint32_t *outDBR, uint32_t *outActualBaud) {
    uint64_t f_sys = 0;
    int err = osGetClockSpeed(MICRON_CLOCK_BUS, &f_sys);
    if(err) return err;

    uint32_t sPrev = 0;
    for(int i=0; _spiBaudLut[i]; i++) {
        uint32_t raw = _spiBaudLut[i];
        uint32_t s = f_sys / (raw & 0x0003FFFF);
        //printf("s=%d prev=%d\r\n", s, sPrev);
        if(baud == s || !_spiBaudLut[i+1]) {
            //exact match, or lower than lowest supported
            //printf("Exact match\r\n");
            return _decodeTableEntry(raw, outPBR, outBR, outDBR, outActualBaud,
                f_sys);
        }
        if(baud > s && baud < sPrev) { //use closest match
            //this seems like it should be < instead, but we're comparing
            //divisors, so it works.
            //printf("diff s=%d p=%d\r\n", baud - s, sPrev - baud);
            if((baud - s) > (sPrev - baud)) { //s is closer
                return _decodeTableEntry(raw, outPBR, outBR, outDBR,
                    outActualBaud, f_sys);
            }
            return _decodeTableEntry(_spiBaudLut[i-1], outPBR, outBR, outDBR,
                outActualBaud, f_sys);
        }
        sPrev = s;
    }

    //given rate is too low or too high
    printf("SPI: can't find params for baud %d\r\n", baud);
    return -ERANGE;
}

#ifdef __cplusplus
    } //extern "C"
#endif
