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
        //irqDisable();
        uint32_t r = 0xDEADBEEF;
        _sdSendDummyBytes(state, 1, timeout);
        int err = spiRead(state->port, &r, timeout);
        //irqEnable();
        if(err) return err;
        //printf("%02X ", r);
        if((r & 0xFF) != 0xFF) {
            return r & 0xFF;
        }
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
    //_sdSendDummyBytes(state, 1, timeout);
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
    for(size_t i=1; i<17; i++) {
        uint32_t r = 0xDEADBEEF;
        err = spiRead(state->port, &r, timeout);
        if(err) {
            #if SDCARD_DEBUG_PRINT
                printf("SD: get R2 failed after %d bytes: ", i);
                for(size_t j=0; j<i; j++) printf("%02X ", resp[j]);
                printf("\r\n");
            #endif
            return err;
        }
        resp[i] = r & 0xFF;
    }
    _sdSendDummyBytes(state, 1, timeout);


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

    resp[0] = err & 0xFF;
    for(size_t i=1; i<5; i++) {
        uint32_t r = 0xDEADBEEF;
        err = spiRead(state->port, &r, timeout);
        if(err) return err;
        resp[i] = r & 0xFF;
    }
    _sdSendDummyBytes(state, 1, timeout);


    #if SDCARD_DEBUG_PRINT
        printf("SD: R7 Resp: %02X %02X %02X %02X %02X err %4d",
            resp[0], resp[1], resp[2], resp[3], resp[4], err);
        _sdPrintStatus(resp[0]);
    #endif
    return 0;
}
