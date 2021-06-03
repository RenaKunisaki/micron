/** SD card SPI driver.
 *  Communicates with a card connected to the SPI pins.
 *  Pin 11 = card's DIN
 *  Pin 12 = card's DOUT
 *  Pin 13 = card's SCK
 *  Any other GPIO = card's CS/SS
 *  This driver doesn't deal with filesystems, only getting data into
 *  and out of the device.
 */
#ifndef _MICRON_SDCARD_H_
#define _MICRON_SDCARD_H_

#ifdef __cplusplus
	extern "C" {
#endif

#define SDCARD_DEBUG_PRINT 1
#include <drivers/kinetis/spi/spi.h>

//some cards allow to change this, others don't.
#define SD_BLOCK_SIZE 512

#define SD_CSD_SIZE 17 //size of CSD structure

//SD card commands (in decimal, lol specs)
#define SD_CMD_RESET             0
#define SD_CMD_INIT              1
#define SD_CMD_SDC_INIT         41
#define SD_CMD_SDC_CHECK_VOLTAGE 8
#define SD_CMD_READ_CSD          9
#define SD_CMD_READ_CID         10
#define SD_CMD_STOP_READ        12
#define SD_CMD_BLOCK_LEN        16
#define SD_CMD_READ_BLOCK       17 //one block
#define SD_CMD_READ_BLOCKS      18 //multiple blocks
#define SD_CMD_NUM_BLOCKS       23 //different between MMC and SDC
#define SD_CMD_WRITE_BLOCK      24
#define SD_CMD_WRITE_BLOCKS     25
#define SD_CMD_ACMD             55 //prefix for cmds 41, 23(SDC)
#define SD_CMD_READ_OCR         58

//bits of R1 response byte
#define SD_RESP_ZERO          BIT(7) //always 0
#define SD_RESP_PARAM_ERR     BIT(6)
#define SD_RESP_ADDR_ERR      BIT(5)
#define SD_RESP_ERASE_SEQ_ERR BIT(4)
#define SD_RESP_CRC_ERR       BIT(3)
#define SD_RESP_BAD_CMD       BIT(2)
#define SD_RESP_ERASE_RESET   BIT(1)
#define SD_RESP_IDLE          BIT(0)

typedef struct {
    //Mainly for internal use.
    //fields common to both v1 and v2
    //all must be unsigned int
    unsigned int cSize;                     //Device Size
    unsigned int cSizeMult;
    unsigned int ccc;                       //Card Command Classes
    unsigned int copy;                      //Copy Flag (OTP)
    unsigned int crc;                       //CRC
    unsigned int csdVersion;                //CSD Structure
    unsigned int dsrImp;                    //DSR Implemented
    unsigned int eraseBlkEn;                //Erase Single Block Enable
    unsigned int fileFormat;                //File Format
    unsigned int fileFormatGrp;             //File Format Group
    unsigned int nsac;                      //Data Read Access-time In CLK Cycles (NSAC*100)
    unsigned int permWriteProtect;          //Permanent Write Protection
    unsigned int r2wFactor;                 //Write Speed Factor
    unsigned int readBlLen;                 //Max. Read Data Block Length
    unsigned int readBlPartial;             //Partial Blocks For Read Allowed
    unsigned int readBlkMisalign;           //Read Block Misalignment
    unsigned int sectorSize;                //Erase Sector Size
    unsigned int taacTimeunit;
    unsigned int taacTimevalue;             //Data Read Access-time
    unsigned int tmpWriteProtect;           //Temporary Write Protection
    unsigned int tranSpeedTimevalue;        //Max. Data Transfer Rate
    unsigned int tranSpeedTransferrateunit;
    unsigned int vddRCurrMax;
    unsigned int vddRCurrMin;
    unsigned int vddWCurrMax;
    unsigned int vddWCurrMin;
    unsigned int wpGrpEnable;               //Write Protect Group Enable
    unsigned int wpGrpSize;                 //Write Protect Group Size
    unsigned int writeBlLen;                //Max. Write Data Block Length
    unsigned int writeBlPartial;
    unsigned int writeBlkMisalign;          //Write Block Misalignment
} SD_CSD;

typedef struct {
    uint8_t port; //which SPI port to use
    uint8_t pinCS; //which pin is card's CS/SS
    uint8_t cardVersion; //SD card protocol version
    uint64_t cardSize; //capacity in bytes
    uint64_t nSectors; //number of blocks
    uint16_t sectorSize; //size in bytes
    uint64_t accessSpeed; //in nanoseconds
    uint64_t transferRate; //in bytes/sec
    void *blockCache; // -> 0x512 * (n+1) bytes; first "block" is u32 blockId[]
    uint32_t blockCacheSize; //number of blocks
} MicronSdCardState;

#include "filecls.h"

typedef int(*MicronSdCardReadBlocksCb)(MicronSdCardState *state, const void *data);

//cmds.c
int sdcardSendCommand(MicronSdCardState *state, uint8_t cmd, uint32_t param,
    uint8_t *resp, size_t respSize, uint32_t timeout);
int _sdSendDummyBytes(MicronSdCardState *state, int count, uint32_t timeout, bool cs);
int _sdSendCmd0(MicronSdCardState *state, uint32_t timeout);
int _sdSendCmd1(MicronSdCardState *state, uint32_t timeout);
int _sdSendCmd8(MicronSdCardState *state, uint32_t timeout);
int _sdSendCmd41(MicronSdCardState *state, uint32_t timeout);
int _sdSendCmd55(MicronSdCardState *state, uint32_t timeout);

//crc.c
uint8_t sdcardCalcCrc(const void *data, size_t len);
uint16_t sdcardCalcCrc16(uint16_t crc, const uint8_t *data, size_t len);

//csd.c
int _sdGetCSD(MicronSdCardState *state, uint32_t timeout, void *csd);
int _sdParseCSD(void *csdIn, SD_CSD *out);
int sdReadInfo(MicronSdCardState *state, uint32_t timeout);

//debug.c
void _sdPrintStatus(uint8_t stat);

//io.c
int _sdSetBlockSize(MicronSdCardState *state, uint32_t size, uint32_t timeout);
int _sdWaitForData(MicronSdCardState *state, void *dest, size_t size,
    uint32_t timeout);
int sdReadBlock(MicronSdCardState *state, uint32_t block, void *dest,
    uint32_t timeout, bool checkCrc);
int sdReadBlocks(MicronSdCardState *state, uint32_t firstBlock,
    MicronSdCardReadBlocksCb callback, uint32_t timeout, bool checkCrc);

//response.c
int _sdWaitForResponse(MicronSdCardState *state, uint32_t timeout);
int _sdGetRespR1(MicronSdCardState *state, uint8_t *resp, uint32_t timeout);
int _sdGetRespR2(MicronSdCardState *state, uint8_t *resp, uint32_t timeout);
int _sdGetRespR7(MicronSdCardState *state, uint8_t *resp, uint32_t timeout);

//sdcard.c
int sdcardInit(MicronSdCardState *state);
int sdcardUpdateSpeed(MicronSdCardState *state, uint32_t timeout);
int sdcardReset(MicronSdCardState *state, uint32_t timeout);


#ifdef __cplusplus
    } //extern "C"
#endif

#endif //_MICRON_SDCARD_H_
