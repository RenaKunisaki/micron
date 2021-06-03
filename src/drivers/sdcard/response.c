extern "C" {
    #include <micron.h>
    #include "sdcard.h"
}

int _sdWaitForResponse(MicronSdCardState *state, uint32_t timeout) {
    /** Receive dummy bytes.
     *  @param state Card state.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     *  Expects to receive several 0xFF bytes followed by a byte
     *  which is not 0xFF. Returns that byte, or a negative error code.
     */
    uint32_t limit = millis() + timeout;
    while(1) { //receive dummy bytes
        if(millis() >= limit) return -ETIMEDOUT;
        uint8_t r = 0xFF;
        while(1) {
            int err = _sdSendDummyBytes(state, 1, 10, true);
            if(err < 0 && err != -ETIMEDOUT) return err;
            irqWait();

            err = spiReadBlocking(state->port, &r, 1, 50);
            if(err < 0 && err != -ETIMEDOUT) {
                #if SDCARD_DEBUG_PRINT
                    printf("_sdWaitForResponse read err %d\r\n", err);
                #endif
                return err;
            }
            //printf("resp %d %02X\r\n", err, r);
            if(err != -ETIMEDOUT) break;
            if(millis() >= limit) return -ETIMEDOUT;
        }
        if(r != 0xFF) {
            //printf("%02X ", r);
            return r;
        }
        //printf(".");
    }
}

int _sdGetRespR1(MicronSdCardState *state, uint8_t *resp, uint32_t timeout) {
    /** Receive an R1 type response, ie one status byte.
     *  @param state Card state.
     *  @param resp Destination variable.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     */
    int err = _sdWaitForResponse(state, timeout);
    if(err < 0) {
        #if SDCARD_DEBUG_PRINT
            printf("SD: R1 Error %d\r\n", err);
        #endif
        return err;
    }
    //#if SDCARD_DEBUG_PRINT
    //    printf("SD: R1 0x%02X\r\n", err & 0xFF);
    //#endif
    *resp = err & 0xFF;
    //_sdSendDummyBytes(state, 1, timeout, true);
    return 0;
}


int _sdGetRespR2(MicronSdCardState *state, uint8_t *resp, uint32_t timeout) {
    /** Receive an R2 type response (17 bytes total).
     *  @param state Card state.
     *  @param resp Destination buffer.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     */
    int err;
    err = _sdWaitForResponse(state, timeout);
    if(err < 0) {
        #if SDCARD_DEBUG_PRINT
            printf("SD: R2 Error %d\r\n", err);
        #endif
        return err;
    }

    resp[0] = err & 0xFF;
    err = spiReadBlocking(state->port, &resp[1], 16, timeout);
    if(err < 0) return err;

    _sdSendDummyBytes(state, 1, timeout, true);

    #if SDCARD_DEBUG_PRINT
        printf("SD: R2 Resp: ");
        for(size_t i=0; i<17; i++) {
            printf("%02X ", resp[i]);
        }
        _sdPrintStatus(resp[0]);
    #endif
    return 0;
}


int _sdGetRespR7(MicronSdCardState *state, uint8_t *resp, uint32_t timeout) {
    /** Receive an R7 type response, ie one status byte followed
     *  by 4 data bytes.
     *  @param state Card state.
     *  @param resp Destination buffer.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     */
    int err;
    err = _sdWaitForResponse(state, timeout);
    if(err < 0) {
        #if SDCARD_DEBUG_PRINT
            printf("SD: R7 Error %d\r\n", err);
        #endif
        return err;
    }

    _sdSendDummyBytes(state, 1, timeout, true);
    resp[0] = err & 0xFF;
    err = spiReadBlocking(state->port, &resp[1], 5, timeout);
    if(err < 0) return err;
    _sdSendDummyBytes(state, 1, timeout, true);

    #if SDCARD_DEBUG_PRINT
        printf("SD: R7 Resp: %02X %02X %02X %02X %02X err %4d",
            resp[0], resp[1], resp[2], resp[3], resp[4], err);
        _sdPrintStatus(resp[0]);
    #endif
    return 0;
}
