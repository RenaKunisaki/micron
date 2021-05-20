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
        uint8_t resp = 0xBB; //arbitrary dummy value
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
        uint8_t resp = 0xBB;
        err = _sdGetRespR1(state, &resp, timeout);
        if(err) return err;
        if(resp == 0xFE) ok = 1;
    } while(!ok);

    //Receive the data
    uint8_t *d = (uint8_t*)dest;
    for(size_t i=0; i<size; i++) {
        if(millis() >= limit) return -ETIMEDOUT;

        uint32_t r = 0xDEADBEEF;
        int err = spiRead(state->port, &r, timeout);
        if(err) return err;
        d[i] = r & 0xFF;
        _sdSendDummyBytes(state, 1, 100);
    }

    return 0;
}


int sdReadBlock(MicronSdCardState *state, uint32_t block, void *dest,
uint32_t timeout) {
    /** Read one block from SD card.
     *  @param state Card state.
     *  @param block Block number to read.
     *  @param dest Destination buffer. Must be at least SD_BLOCK_SIZE bytes.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     */
    uint32_t limit = millis() + timeout;
    int ok, err;

    //Send CMD17 and wait for 0x00 response
    do {
        if(millis() >= limit) return -ETIMEDOUT;
        ok = 0;
        uint8_t resp = 0xBB; //arbitrary dummy value
        err = sdcardSendCommand(state, SD_CMD_READ_BLOCK, block, &resp, 1, timeout);
        if(err) return err;
        if(resp == 0x00) ok = 1;
        if(resp & SD_RESP_PARAM_ERR) return -ERANGE;
    } while(!ok);

    return _sdWaitForData(state, dest, SD_BLOCK_SIZE, timeout);
}


int sdReadBlocks(MicronSdCardState *state, uint32_t firstBlock,
MicronSdCardReadBlocksCb callback, uint32_t timeout) {
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
    //We can actually do less than 512
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
            SPI0_SR = SPI_SR_EOQF;
            SPI0_PUSHR = 0xFF;
            int err = spiRead(state->port, &r, timeout);
            if(err) return err;
            if((r & 0xFF) == 0xFE) break;
        }

        //data followed by 2 byte CRC
        //not sure what we can do if the CRC is wrong...
        uint8_t data[SD_BLOCK_SIZE+2];
        for(int i=0; i<SD_BLOCK_SIZE+2; i++)  {
            //_sdSendDummyBytes(state, 1, 1);
            //too slow, use this instead...
            gpioSetPinOutput(state->pinCS, 0);
            SPI0_SR = SPI_SR_EOQF;
            SPI0_PUSHR = 0xFF;
            int err = spiRead(state->port, &r, timeout);
            if(err) return err;
            data[i] = r & 0xFF;
        }

        if(err) return err;
        int stop = callback(state, data);
        if(stop) break;
    }

    uint8_t resp=0; //don't care about the value
    return sdcardSendCommand(state, SD_CMD_STOP_READ, 0, &resp, 1, timeout);
}
