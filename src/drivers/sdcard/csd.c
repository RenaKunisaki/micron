extern "C" {
    #include <micron.h>
    #include "sdcard.h"
}

//XXX move this
int quick_pow10(int n) {
    static int _pow10[10] = {1, 10, 100, 1000, 10000, 100000, 1000000,
        10000000, 100000000, 1000000000};
    if(n < 0) return 0; //fractions, which would truncate to 0
    return _pow10[n];
}


static void calcSpeeds(MicronSdCardState *state, SD_CSD *csd) {
    static const int speeds[] = { //multiplied by 10
         0, 10, 12, 13, 15, 20, 25, 30,
        35, 40, 45, 50, 55, 60, 70, 80
    };

    //subtract 1 because speeds table is multiplied by 10
    unsigned int scale = quick_pow10(csd->taacTimeunit - 1);
    state->accessSpeed = speeds[csd->taacTimevalue] * scale;

    //add 5 because speed unit is 100 kbit/s
    //subtract 1 because speeds table is multiplied by 10
    scale = quick_pow10(csd->tranSpeedTransferrateunit + 4);
    state->transferRate = speeds[csd->tranSpeedTimevalue] * scale;
}


#if SDCARD_DEBUG_PRINT
static void printCSD(SD_CSD *csd) {
    /** Print CSD structure to console.
     */
    printf("SD: CSD Ver %d\r\n", csd->csdVersion + 1);
    switch(csd->csdVersion) {
        case 0: {
            printf(
                "  taacTimevalue             = 0x%X\r\n"
                "  taacTimeunit              = 0x%X\r\n"
                "  nsac                      = 0x%X\r\n"
                "  tranSpeedTimevalue        = 0x%X\r\n"
                "  tranSpeedTransferrateunit = 0x%X\r\n"
                "  ccc                       = 0x%X\r\n"
                "  readBlLen                 = 0x%X\r\n"
                "  readBlPartial             = 0x%X\r\n"
                "  writeBlkMisalign          = 0x%X\r\n"
                "  readBlkMisalign           = 0x%X\r\n"
                "  dsrImp                    = 0x%X\r\n"
                "  cSize                     = 0x%X\r\n"
                "  vddRCurrMin               = 0x%X\r\n"
                "  vddRCurrMax               = 0x%X\r\n"
                "  vddWCurrMin               = 0x%X\r\n"
                "  vddWCurrMax               = 0x%X\r\n"
                "  cSizeMult                 = 0x%X\r\n"
                "  eraseBlkEn                = 0x%X\r\n"
                "  sectorSize                = 0x%X\r\n"
                "  wpGrpSize                 = 0x%X\r\n"
                "  wpGrpEnable               = 0x%X\r\n"
                "  r2wFactor                 = 0x%X\r\n"
                "  writeBlLen                = 0x%X\r\n"
                "  writeBlPartial            = 0x%X\r\n"
                "  fileFormatGrp             = 0x%X\r\n"
                "  copy                      = 0x%X\r\n"
                "  permWriteProtect          = 0x%X\r\n"
                "  tmpWriteProtect           = 0x%X\r\n"
                "  fileFormat                = 0x%X\r\n",
                csd->taacTimevalue,
                csd->taacTimeunit,
                csd->nsac,
                csd->tranSpeedTimevalue,
                csd->tranSpeedTransferrateunit,
                csd->ccc,
                csd->readBlLen,
                csd->readBlPartial,
                csd->writeBlkMisalign,
                csd->readBlkMisalign,
                csd->dsrImp,
                csd->cSize,
                csd->vddRCurrMin,
                csd->vddRCurrMax,
                csd->vddWCurrMin,
                csd->vddWCurrMax,
                csd->cSizeMult,
                csd->eraseBlkEn,
                csd->sectorSize,
                csd->wpGrpSize,
                csd->wpGrpEnable,
                csd->r2wFactor,
                csd->writeBlLen,
                csd->writeBlPartial,
                csd->fileFormatGrp,
                csd->copy,
                csd->permWriteProtect,
                csd->tmpWriteProtect,
                csd->fileFormat);
            break;
        }
    	case 1: {
            printf(
                "  csdVersion               = 0x%X\r\n"
                "  taacTimevalue            = 0x%X\r\n"
                "  taacTimeunit             = 0x%X\r\n"
                "  nsac                     = 0x%X\r\n"
                "  tranSpeedTimevalue       = 0x%X\r\n"
                "  tranSpeedTransferrateunit= 0x%X\r\n"
                "  ccc                      = 0x%X\r\n"
                "  readBlLen                = 0x%X\r\n"
                "  readBlPartial            = 0x%X\r\n"
                "  writeBlkMisalign         = 0x%X\r\n"
                "  readBlkMisalign          = 0x%X\r\n"
                "  dsrImp                   = 0x%X\r\n"
                "  cSize                    = 0x%X\r\n"
                "  eraseBlkEn               = 0x%X\r\n"
                "  sectorSize               = 0x%X\r\n"
                "  wpGrpSize                = 0x%X\r\n"
                "  wpGrpEnable              = 0x%X\r\n"
                "  r2wFactor                = 0x%X\r\n"
                "  writeBlLen               = 0x%X\r\n"
                "  writeBlPartial           = 0x%X\r\n"
                "  fileFormatGrp            = 0x%X\r\n"
                "  copy                     = 0x%X\r\n"
                "  permWriteProtect         = 0x%X\r\n"
                "  tmpWriteProtect          = 0x%X\r\n"
                "  fileFormat               = 0x%X\r\n",
                csd->csdVersion,
                csd->taacTimevalue,
                csd->taacTimeunit,
                csd->nsac,
                csd->tranSpeedTimevalue,
                csd->tranSpeedTransferrateunit,
                csd->ccc,
                csd->readBlLen,
                csd->readBlPartial,
                csd->writeBlkMisalign,
                csd->readBlkMisalign,
                csd->dsrImp,
                csd->cSize,
                csd->eraseBlkEn,
                csd->sectorSize,
                csd->wpGrpSize,
                csd->wpGrpEnable,
                csd->r2wFactor,
                csd->writeBlLen,
                csd->writeBlPartial,
                csd->fileFormatGrp,
                csd->copy,
                csd->permWriteProtect,
                csd->tmpWriteProtect,
                csd->fileFormat);
            break;
        }
    	default:
    		printf("Unknown CSD structure: 0x%x\r\n", csd->csdVersion);
	} //switch


    //card command classes
    static const char *classes[] = {
        "basic", "reserved", "block read", "reserved",
        "block write", "erase", "write protect", "lock card",
        "app specific", "I/O mode", "switch", "extension"};
    unsigned int ccc = csd->ccc;
    printf("  CCC: 0x%03X (class: ", ccc);
    for(int i=11; i>=0; i--) {
        if(ccc & (1 << i)) printf("%d %s, ", i, classes[i]);
    }
	printf(")\r\n");
}

static void _printCapacity(MicronSdCardState *state) {
    static const char *units = " KMGT";
    int unit = 0;
    uint64_t size = state->cardSize;
    while(size > 9999ll && units[unit]) {
        size /= 1024ll;
        unit++;
    }

    printf("SD: Capacity: %llu bytes (%llu %cB) - %llu sectors, %d bytes each\r\n",
        state->cardSize, size, units[unit], state->nSectors,
        state->sectorSize);
}

static void _printSpeed(MicronSdCardState *state) {
    static const char *timeUnits = "num ";
    static const char *sizeUnits = " KMGT";
    int unit = 0;
    uint64_t val = state->accessSpeed;
    while(val > 9999ll && timeUnits[unit]) {
        val /= 1000ll;
        unit++;
    }
    printf("SD: Access time: %llu %cs; ", val, timeUnits[unit]);

    unit = 0;
    val = state->transferRate / 8ll; //bits -> bytes
    while(val > 9999ll && sizeUnits[unit]) {
        val /= 1024ll;
        unit++;
    }
    printf("Transfer rate: %llu %cB/s\r\n", val, sizeUnits[unit]);
}
#endif //SDCARD_DEBUG_PRINT



int _sdGetCSD(MicronSdCardState *state, uint32_t timeout, void *csd) {
    /** Read the CSD register from the card.
     *  @param state Card state.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @param csd Buffer to receive response into. Must be at least
     *   SD_CSD_SIZE bytes.
     *  @return 0 on success, or negative error code on failure.
     */
    uint32_t limit = millis() + timeout;
    int ok, err;
    while(1) {
        do {
            if(millis() >= limit) return -ETIMEDOUT;
            ok = 0;
            uint8_t resp = 0xBB;
            err = sdcardSendCommand(state, SD_CMD_READ_CSD, 0, &resp, 1, timeout);
            if(err) return err;
            if(resp == 0x00) ok = 1;
        } while(!ok);

        err = _sdWaitForData(state, csd, SD_CSD_SIZE, timeout);
        if(err) return err;

        //this seems to not be the proper algorithm, it always fails
        /* uint8_t gotCrc = ((uint8_t*)csd)[16];
        uint8_t crc = sdcardCalcCrc(csd, SD_CSD_SIZE - 1);
        if(crc != gotCrc) {
            #if SDCARD_DEBUG_PRINT
                printf("CSD CRC FAIL (0x%02X, got 0x%02X)\r\n",
                    crc, gotCrc);
                for(int i=0; i<17; i++) printf("%02X ", ((uint8_t*)csd)[i] );
                printf("\r\n");
            #endif
        }
        else break; */
        break;
    }

    return 0;
}


static int _extractFields(const uint8_t *bitLengths, unsigned int **fields,
const void *data) {
    /** Used internally to parse CSD structure.
     *  XXX might be useful elsewhere?
     *  We use this because packed structs don't necessarily have the
     *  correct bit order.
     */
    int iField = 0;
    int bit = 0;
    const uint8_t *bIn = (const uint8_t*)data;
    while(bitLengths[iField]) {
        int len = bitLengths[iField];
        unsigned int *field = fields[iField];
        unsigned int val = 0;

        for(int iBit=0; iBit<len; iBit++) {
            val = (val << 1) | ((bIn[bit >> 3] >> (7-(bit & 7))) & 1);
            bit++;
        }
        if(field) *field = val;
        iField++;
    }
    return 0;
}


int _sdParseCSD(void *csdIn, SD_CSD *out) {
    /** Parse SD CSD structure.
     *  @param csdIn Raw data to parse. Should be 17 bytes.
     *  @param out Structure to fill.
     *  @return 0 on success, negative error code on failure.
     */
    memset(out, 0, sizeof(SD_CSD));

    #if SDCARD_DEBUG_PRINT
        //show raw data
        printf("CSD Data: ");
        for(int i=0; i<17; i++) printf("%02X ", ((uint8_t*)csdIn)[i]);
        printf("\r\n");
    #endif //SDCARD_DEBUG_PRINT

    uint8_t version = (*(uint8_t*)csdIn) >> 6;
    switch(version) {
        case 0: {
            static const uint8_t bitLengths[] = {
                 2,  6,  1,  4,  3,  8,  1,  4,  3, 12,  4,  1,  1,  1,  1,  2,
                12,  3,  3,  3,  3,  3,  1,  7,  7,  1,  2,  3,  4,  1,  5,  1,
                 1,  1,  1,  2,  2,  7,  1,  0};

            unsigned int *fields[] = {
                &out->csdVersion               ,
                NULL                           , //reserved field
                NULL                           ,
                &out->taacTimevalue            ,
                &out->taacTimeunit             ,
                &out->nsac                     ,
                NULL                           ,
                &out->tranSpeedTimevalue       ,
                &out->tranSpeedTransferrateunit,
                &out->ccc                      ,
                &out->readBlLen                ,
                &out->readBlPartial            ,
                &out->writeBlkMisalign         ,
                &out->readBlkMisalign          ,
                &out->dsrImp                   ,
                NULL                           ,
                &out->cSize                    ,
                &out->vddRCurrMin              ,
                &out->vddRCurrMax              ,
                &out->vddWCurrMin              ,
                &out->vddWCurrMax              ,
                &out->cSizeMult                ,
                &out->eraseBlkEn               ,
                &out->sectorSize               ,
                &out->wpGrpSize                ,
                &out->wpGrpEnable              ,
                NULL                           ,
                &out->r2wFactor                ,
                &out->writeBlLen               ,
                &out->writeBlPartial           ,
                NULL                           ,
                &out->fileFormatGrp            ,
                &out->copy                     ,
                &out->permWriteProtect         ,
                &out->tmpWriteProtect          ,
                &out->fileFormat               ,
                NULL                           ,
                &out->crc                      ,
                NULL                           ,
            };
            return _extractFields(bitLengths, fields, csdIn);
        }
        case 1: {
            static const uint8_t bitLengths[] = {
                 2,  6,  1,  4,  3,  8,  1,  4,  3, 12,  4,  1,  1,  1,  1,  6,
                22,  1,  1,  7,  7,  1,  2,  3,  4,  1,  5,  1,  1,  1,  1,  2,
                 2,  7,  1,  0};
            unsigned int *fields[] = {
                &out->csdVersion               ,
                NULL                           ,
                NULL                           ,
                &out->taacTimevalue            ,
                &out->taacTimeunit             ,
                &out->nsac                     ,
                NULL                           ,
                &out->tranSpeedTimevalue       ,
                &out->tranSpeedTransferrateunit,
                &out->ccc                      ,
                &out->readBlLen                ,
                &out->readBlPartial            ,
                &out->writeBlkMisalign         ,
                &out->readBlkMisalign          ,
                &out->dsrImp                   ,
                NULL                           ,
                &out->cSize                    ,
                NULL                           ,
                &out->eraseBlkEn               ,
                &out->sectorSize               ,
                &out->wpGrpSize                ,
                &out->wpGrpEnable              ,
                NULL                           ,
                &out->r2wFactor                ,
                &out->writeBlLen               ,
                &out->writeBlPartial           ,
                NULL                           ,
                &out->fileFormatGrp            ,
                &out->copy                     ,
                &out->permWriteProtect         ,
                &out->tmpWriteProtect          ,
                &out->fileFormat               ,
                NULL                           ,
                &out->crc                      ,
                NULL                           ,
            };
            return _extractFields(bitLengths, fields, csdIn);
        }
        default: {
            #if SDCARD_DEBUG_PRINT
                printf("Unknown CSD structure version: 0x%x\r\n", version);
            #endif
            return -ENOSYS;
        }
    }
}


//see https://git.kernel.org/pub/scm/linux/kernel/git/cjb/mmc-utils.git/tree/lsmmc.c#n647
//for more info on the meaning of other fields.
int sdReadInfo(MicronSdCardState *state, uint32_t timeout) {
    /** Read SD card info.
     *  @param state Card state.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     *  @note fills in state->cardVersion, state->cardSize, state->nSectors,
     *   state->sectorSize, state->accessSpeed, state->transferRate.
     */
    int err;
    uint8_t buf[SD_CSD_SIZE];
    memset(&buf, 0xEE, SD_CSD_SIZE); //debug
    err = _sdGetCSD(state, timeout, buf);
    if(err) return err;

    SD_CSD csd;
    err = _sdParseCSD(buf, &csd);
    if(err) {
        #if SDCARD_DEBUG_PRINT
            printf("SD: CSD parse failed: %d\r\n", err);
        #endif
        return err;
    }
    #if SDCARD_DEBUG_PRINT
        printCSD(&csd);
    #endif

    uint32_t blockSize;
    uint64_t blocks, memoryCapacity;

	switch(csd.csdVersion) {
        case 0: {
            int mult = 1 << (csd.cSizeMult + 2);
			blocks = (csd.cSize + 1) * mult;
			blockSize = 1 << csd.readBlLen;
            memoryCapacity = blocks * blockSize;
            break;
        }
    	case 1: {
            memoryCapacity = (csd.cSize + 1) * 512ull * 1024ull;
			blockSize = 512;
			blocks = memoryCapacity / blockSize;
            break;
        }
    	default: return -ENOSYS;
	}

    state->cardSize = memoryCapacity;
    state->nSectors = blocks;
    state->sectorSize = blockSize;
    calcSpeeds(state, &csd);

    #if SDCARD_DEBUG_PRINT
        _printCapacity(state);
        _printSpeed(state);
    #endif

    return 0;
}
