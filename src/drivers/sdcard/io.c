extern "C" {
    #include <micron.h>
    #include "sdcard.h"
}

int _sdSetBlockSize(MicronSdCardState *state, uint32_t size, uint32_t timeout) {
    /** Set block size for read/write.
     *  @param state Card state.
     *  @return 0 on success, or negative error code on failure.
     *  Some cards ignore this and always use 512; other cards
     *  might require it. So you should always send it with size 512,
     *  which we do in sdReset().
     */
    uint32_t limit = millis() + timeout;
    int ok, err;
    do {
        if(millis() >= limit) return -ETIMEDOUT;
        ok = 0;
        uint8_t resp = 0xFF; //arbitrary dummy value
        err = sdcardSendCommand(state, SD_CMD_BLOCK_LEN, size, &resp, 1, timeout);
        if(err) return err;
        if(resp == 0x00) ok = 1;
    } while(!ok);
    return 0;
}


int _sdWaitForData(MicronSdCardState *state, void *dest, size_t size,
uint32_t timeout) {
    /** Wait until data arrives, and receive it.
     *  @param state Card state.
     *  @param dest Destination buffer.
     *  @param size Destination buffer size.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     */
    //Wait for 0xFE response
    uint32_t limit = millis() + timeout;
    int err, ok = 0;
    do {
        if(millis() >= limit) return -ETIMEDOUT;
        uint8_t resp = 0x00; //arbitrary dummy value
        err = _sdGetRespR1(state, &resp, timeout);
        if(err) return err;
        if(resp == 0xFE) ok = 1;
    } while(!ok);

    //Receive the data
    uint8_t *d = (uint8_t*)dest;
    for(size_t i=0; i<size; ) {
        if(millis() >= limit) return -ETIMEDOUT;

        //use rx buf size here, since each dummy byte sent
        //equals one byte received.
        //frame size is 8 bits, so no need to worry about
        //whether sizes are in bytes or in frames.
        size_t n = MIN(SPI_RX_BUFSIZE, size-i);
        _sdSendDummyBytes(state, n, 100, true);
        int err = spiRead(state->port, d, n, timeout);
        if(err < 0) return err;
        //printf("\x1B[31m%02X\x1B[0m ", r & 0xFF);
        d += err;
        i += err;
    }
    //_sdSendDummyBytes(state, 1, 100, true);

    return 0;
}

int _getBlockCrc(MicronSdCardState *state, void *dest, uint32_t timeout,
bool checkCrc) {
    //after the data is 2 bytes CRC.
    //we must receive it even if we ignore it.
    uint16_t crc = 0;
    int err = _sdSendDummyBytes(state, 2, 100, true);
    if(err < 0) return err;
    err = spiRead(state->port, &crc, 2, timeout);
    if(err < 0) return err;

    if(checkCrc) {
        crc = (crc << 8) | (crc >> 8); //byteswap (XXX check platform endian)
        uint16_t crc2 = sdcardCalcCrc16(0, (const uint8_t*)dest, SD_BLOCK_SIZE);
        if(crc != crc2) {
            #if SDCARD_DEBUG_PRINT
                printf("SD: Block CRC mismatch (0x%04X, expected 0x%04X)\r\n",
                    crc2, crc);
            #endif
            return -EIO;
        }
    }

    return 0;
}

int _getBlockFromCache(MicronSdCardState *state, uint32_t block, void *dest) {
    //return cache entry index, or 0 if not found
    uint32_t *cache = (uint32_t*)state->blockCache;
    if(!cache) return 0; //no cache
    for(int i=0; i<state->blockCacheSize; i++) {
        //first "block" is the block ID array,
        //so the actual cache index is 1 + this array index.
        if(cache[i] == block) {
            void *src = state->blockCache + (SD_BLOCK_SIZE * (i+1));
            memcpy(dest, src, SD_BLOCK_SIZE);
            return i+1;
        }
    }
    return 0; //not found
}

int _addBlockToCache(MicronSdCardState *state, uint32_t block, void *data) {
    //return new cache entry index, or 0 if not added
    uint32_t *cache = (uint32_t*)state->blockCache;
    if(!cache) return 0; //no cache
    int i;
    for(i=0; i<state->blockCacheSize; i++) {
        if(cache[i] == block) return i+1; //already in cache
        if(cache[i] == 0xFFFFFFFF) break; //empty slot
    }
    if(i >= state->blockCacheSize) { //no empty slot
        //XXX implement something smarter like LRU
        //for now, pick at random. (this isn't purely random as the modulo
        //introduces some bias, but it's good enough for now.)
        i = rand() % state->blockCacheSize;
    }
    cache[i] = block;
    void *dest = state->blockCache + (SD_BLOCK_SIZE * (i+1));
    memcpy(dest, data, SD_BLOCK_SIZE);
    return i+1;
}

int sdReadBlock(MicronSdCardState *state, uint32_t block, void *dest,
uint32_t timeout, bool checkCrc) {
    /** Read one block from SD card.
     *  @param state Card state.
     *  @param block Block number to read.
     *  @param dest Destination buffer. Must be at least SD_BLOCK_SIZE bytes.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @param checkCrc Whether to verify the data CRC or ignore it.
     *  @return 0 on success, or negative error code on failure.
     */
    uint32_t limit = millis() + timeout;
    int ok, err, len;

    err = _getBlockFromCache(state, block, dest);
    if(err) {
        #if SDCARD_DEBUG_PRINT
            printf("SD: Block 0x%X in cache at 0x%X\r\n", block, err);
        #endif
        return 0;
    }

    //Send CMD17 and wait for 0x00 response
    do {
        if(millis() >= limit) return -ETIMEDOUT;
        ok = 0;
        uint8_t resp = 0xFF; //arbitrary dummy value
        err = sdcardSendCommand(state, SD_CMD_READ_BLOCK, block, &resp, 1, timeout);
        if(err) return err;
        if(resp == 0x00) ok = 1;
        //printf("ReadBlock resp %02X\r\n", resp);
        if(resp & SD_RESP_PARAM_ERR) return -ERANGE;
    } while(!ok);

    //receive data (will wait for 0xFE token)
    err = _sdWaitForData(state, dest, SD_BLOCK_SIZE, timeout);
    if(err < 0) return err;
    len = err;
    err = _getBlockCrc(state, dest, timeout, checkCrc);
    if(err) return err;

    //add to cache
    _addBlockToCache(state, block, dest);

    return len;
}


int sdReadBlocks(MicronSdCardState *state, uint32_t firstBlock,
MicronSdCardReadBlocksCb callback, uint32_t timeout, bool checkCrc) {
    /** Read multiple blocks from SD card.
     *  @param state Card state.
     *  @param firstBlock Block number to start at.
     *  @param callback Callback to receive blocks.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     *  @note Reads until last block or until callback returns true.
     */
    uint32_t limit = millis() + timeout;
    int ok, err;

    //Send CMD16 to set read block size
    //We can actually do less than 512, but I think not all cards support that.
    do {
        if(millis() >= limit) return -ETIMEDOUT;
        ok = 0;
        uint8_t resp = 0xBB; //arbitrary dummy value
        err = sdcardSendCommand(state, SD_CMD_BLOCK_LEN,
            SD_BLOCK_SIZE, &resp, 1, timeout);
        if(err) return err;
        if(resp == 0x00) ok = 1;
    } while(!ok);

    //Send CMD18 and wait for 0x00 response
    do {
        if(millis() >= limit) return -ETIMEDOUT;
        ok = 0;
        uint8_t resp = 0xBB; //arbitrary dummy value
        err = sdcardSendCommand(state, SD_CMD_READ_BLOCKS,
            firstBlock, &resp, 1, timeout);
        if(err) return err;
        if(resp == 0x00) ok = 1;
        if(resp & SD_RESP_PARAM_ERR) return -ERANGE;
    } while(!ok);

    //receive data
    while(1) {
        uint32_t r = 0xDEADBEEF;

        //wait for 0xFE response
        while(1) {
            gpioSetPinOutput(state->pinCS, 0);
            //SPI0_SR = SPI_SR_EOQF;
            SPI0_PUSHR = 0xFF;
            int err = spiRead(state->port, &r, sizeof(uint32_t), timeout);
            if(err) return err;
            if((r & 0xFF) == 0xFE) break;
        }

        //data followed by 2 byte CRC
        //not sure what we can do if the CRC is wrong...
        uint8_t data[SD_BLOCK_SIZE+2];
        int err = spiRead(state->port, data, sizeof(data), timeout);

        /* for(int i=0; i<SD_BLOCK_SIZE+2; i++)  {
            //_sdSendDummyBytes(state, 1, 1, true);
            //too slow, use this instead...
            gpioSetPinOutput(state->pinCS, 0);
            SPI0_SR = SPI_SR_EOQF;
            SPI0_PUSHR = 0xFF;
            int err = spiRead(state->port, &r, timeout);
            if(err) return err;
            data[i] = r & 0xFF;
        } */

        if(err) return err;
        int stop = callback(state, data);
        if(stop) break;
    }

    uint8_t resp=0; //don't care about the value
    return sdcardSendCommand(state, SD_CMD_STOP_READ, 0, &resp, 1, timeout);
}
