#include <micron.h>
#include "sdcard.h"

int sdcardInit(MicronSdCardState *state) {
    /** Initialize SD Card driver.
     *  @param state Card state.
     *  @return 0 on success, or negative error code on failure.
     *  @note You must initialize the following fields of `state`
     *   before calling this function: port, pinCS
     */
    //the spec calls for 400 Hz but we can't necessarily do that exact rate.
    //at the default 72 MHz CPU speed, the closest is 366 Hz.
    //but going below 400 can make some cards not respond, so we'll specify
    //a bit higher to get the next-highest speed of 439, while still being
    //as close as possible on other speeds.
    int err = spiInit(state->port, state->pinCS, 420, SPI_MODE_0);
    if(err) return err;
    err = spiPause(state->port, false);
    if(err < 0) return err;
    return 0;
}

int sdcardReset(MicronSdCardState *state, uint32_t timeout) {
    /** Reset SD card.
     *  @param state Card state.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     *  @note This should normally be called after sdcardInit().
     */
    int err;

    //Send at least 74 dummy bits.
    //10 bytes = 80 bits, so that'll do.
    #if SDCARD_DEBUG_PRINT
        printf("SD: send dummy bytes...\r\n");
    #endif
    delayMS(1);
    spiWriteDummy(state->port, 0xFF, 10, false);
    uint32_t _d = 0xFF;
    spiWriteBlocking(state->port, &_d, 1, false, 1000);
    spiWaitTxDone(state->port, 1000);
    delayMS(1);

    //send CMD0
    #if SDCARD_DEBUG_PRINT
        printf("SD: Send CMD0...\r\n");
    #endif
    err = _sdSendCmd0(state, timeout);
    if(err) {
        #if SDCARD_DEBUG_PRINT
            printf("SD: CMD0 err %d\r\n", err);
        #endif
        return err;
    }

    //send CMD8
    #if SDCARD_DEBUG_PRINT
        printf("SD: Send CMD8...\r\n");
    #endif
    err = _sdSendCmd8(state, timeout);
    if(err < 0) {
        #if SDCARD_DEBUG_PRINT
            printf("SD: CMD8 err %d\r\n", err);
        #endif
        return err;
    }
    else {
        state->cardVersion = err;
        #if SDCARD_DEBUG_PRINT
            printf("SD: card version %d\r\n", state->cardVersion);
        #endif
    }

    //send CMD41.
    //may fail on old cards, just ignore that...
    #if SDCARD_DEBUG_PRINT
        printf("SD: Send CMD41...\r\n");
    #endif
    err = _sdSendCmd41(state, timeout);
    if(err == -EINVAL) { //old card doesn't support this command
        #if SDCARD_DEBUG_PRINT
            printf("SD: CMD41 not supported; send CMD1...\r\n");
        #endif
        err = _sdSendCmd1(state, timeout);
    }
    else if(err == -ETIMEDOUT) {
        #if SDCARD_DEBUG_PRINT
            //even though the spec I could find suggests 3.3V is the proper
            //voltage, I couldn't get any response until I used 5V.
            printf("SD: CMD41 timeout - card voltage too low?\r\n");
        #endif
    }
    if(err) {
        #if SDCARD_DEBUG_PRINT
            printf("SD: CMD41 err %d\r\n", err);
        #endif
        return err;
    }

    //Required for some cards
    #if SDCARD_DEBUG_PRINT
        printf("SD: set block size...\r\n");
    #endif
    err = _sdSetBlockSize(state, SD_BLOCK_SIZE, 5000);
    if(err) {
        #if SDCARD_DEBUG_PRINT
            printf("SD: setBlockSize err %d\r\n", err);
        #endif
        return err;
    }

    #if SDCARD_DEBUG_PRINT
        printf("SD: reset OK!\r\n");
    #endif
    return 0;
}
