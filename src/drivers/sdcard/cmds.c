extern "C" {
    #include <micron.h>
    #include "sdcard.h"
}

int sdcardSendCommand(MicronSdCardState *state, uint8_t cmd, uint32_t param,
uint8_t *resp, size_t respSize, uint32_t timeout) {
    /** Send raw SD command.
     *  @param state Card state.
     *  @param cmd Command to send.
     *  @param param Command parameter.
     *  @param resp Buffer to store response.
     *  @param respSize Size of resp buffer in bytes.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     */
    int err;

    //Build packet.
    uint8_t data[6] = {
        cmd + 64,
        (param >> 24),
        (param >> 16) & 0xFF,
        (param >>  8) & 0xFF,
         param        & 0xFF,
        0, //CRC
    };
    data[5] = sdcardCalcCrc(data, 5);
    //if(cmd == 16) data[5] = 0xFF;

    //dummy bytes for timing
    spiClear(state->port);
    //err = _sdSendDummyBytes(state, 3, timeout);
    //if(err) return err;

    //Send command.
    //#if SDCARD_DEBUG_PRINT
    //    printf("SD: CMD%2d: ", data[0] - 64);
    //    for(int i=0; i<6; i++) printf("%02X ", data[i]);
    //    printf("  ");
    //#endif

    for(int i=0; i<6; i++) {
        err = spiWrite(state->port, data[i], 1, timeout);
        if(err) {
            //#if SDCARD_DEBUG_PRINT
            //    printf("Byte %d err %d\r\n", i, err);
            //#endif
            return err;
        }
    }

    //Send dummy bytes and wait for transmission to finish.
    err = _sdSendDummyBytes(state, 5, timeout);
    if(!err) err = spiWaitTxDone(state->port, timeout);
    if(err) return err;

    //Get response
    int respType;
    switch(cmd) {
        //no idea the significance of these numbers...
        //0x1B is really "1b" but oh well
        case SD_CMD_SDC_CHECK_VOLTAGE: respType = 0x07; break;
        case SD_CMD_STOP_READ:         respType = 0x1B; break;
        case SD_CMD_READ_OCR:          respType = 0x03; break;
        default:                       respType = 0x01; break;
    }
    switch(respType) {
        case 0x01: return _sdGetRespR1(state, resp, timeout);
        case 0x02: return _sdGetRespR2(state, resp, timeout);
        //XXX how is R7 different from R3?
        case 0x03: return _sdGetRespR7(state, resp, timeout);
        case 0x07: return _sdGetRespR7(state, resp, timeout);
        case 0x1B: //R1 followed by busy flag (wait until not FF)
            return _sdGetRespR1(state, resp, timeout);
        default:
            #if SDCARD_DEBUG_PRINT
                printf("SD: Unknown RespType 0x%02X\r\n", respType);
            #endif
            return -ENOSYS;
    }

    return 0;
}


int _sdSendDummyBytes(MicronSdCardState *state, int count, uint32_t timeout) {
    /** Send some dummy bytes for timing.
     *  @param state Card state.
     *  @param count Number of dummy bytes to send.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     */
    while(count --> 0) {
        int err = spiWrite(state->port, 0xFF, count, timeout);
        if(err) return err;
    }
    return 0;
}


int _sdSendCmd0(MicronSdCardState *state, uint32_t timeout) {
    /** Send CMD0, ie RESET.
     *  @param state Card state.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     *  @note Like most SD card init commands, this may have to be repeated
     *   several times before it responds correctly.
     */
    uint32_t limit = millis() + timeout;
    int ok, err;
    do {
        if(millis() >= limit) return -ETIMEDOUT;
        ok = 0;
        uint8_t resp = 0xBB; //arbitrary dummy value
        //XXX passing `timeout` here means we might end up waiting
        //longer than the actual timeout, eg if we already waited
        //90% of that time on previous attempts.
        err = sdcardSendCommand(state, SD_CMD_RESET, 0, &resp, 1, 200);
        //if(err) return err;
        if(resp == 0x01) ok = 1;
    } while(!ok);
    return 0;
}


int _sdSendCmd1(MicronSdCardState *state, uint32_t timeout) {
    /** Send CMD1, ie INIT for old cards.
     *  @param state Card state.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     *  @note Only sent if the card doesn't understand CMD41.
     */
    uint32_t limit = millis() + timeout;
    int ok, err;
    do {
        if(millis() >= limit) return -ETIMEDOUT;
        ok = 0;
        uint8_t resp = 0xBB; //arbitrary dummy value
        err = sdcardSendCommand(state, SD_CMD_INIT, 0, &resp, 1, 1000);
        if(err) return err;
        if(resp == 0x00 || resp == 0x01) ok = 1;
    } while(!ok);
    return 0;
}


int _sdSendCmd8(MicronSdCardState *state, uint32_t timeout) {
    /** Send CMD8, ie CHECK_VOLTAGE.
     *  @param state Card state.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return The card version, which is 2 if it understands this command or
     *  1 if it doesn't, or a negative error code on failure.
     */
    uint32_t limit = millis() + timeout;
    int ok, err, version;
    do {
        if(millis() >= limit) return -ETIMEDOUT;
        ok = 0;
        uint8_t resp[5];
        err = sdcardSendCommand(state, SD_CMD_SDC_CHECK_VOLTAGE, 0x000001AA,
            resp, 4, timeout);
        if(err) return err;

        switch(resp[0]) {
            case 0x00:
            case 0x01: {
                uint32_t p = (resp[1] << 24) |
                    (resp[2] << 16) | (resp[3] << 8) | resp[4];
                if(p == 0x000001AA) {
                    ok = 1;
                    version = 2;
                }
                else if(p == 0x000000AA) {
                    #if SDCARD_DEBUG_PRINT
                        printf("SD: Incorrect voltage!!\r\n");
                    #endif
                    return -ENETDOWN;
                }
                //else, try again.
                //else {
                //    printf("SD: Unknown CMD8 response 0x%08X\r\n", p);
                //}
                break;
            }

            case 0x05: //old card doesn't understand this command
                ok = 1;
                version = 1;
                break;
        }
    } while(!ok);
    return version;
}


int _sdSendCmd41(MicronSdCardState *state, uint32_t timeout) {
    /** Send CMD41 (aka ACMD41), ie INIT for new cards.
     *  @param state Card state.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     *  @note Automatically sends CMD55 first.
     */
    uint32_t limit = millis() + timeout;
    int ok, err;
    do {
        if(millis() >= limit) return -ETIMEDOUT;
        err = _sdSendCmd55(state, 1000);
        if(err) {
            #if SDCARD_DEBUG_PRINT
                printf("SD: CMD55 err %d\r\n", err);
            #endif
            return err;
        }

        ok = 0;
        uint8_t resp = 0xBB; //arbitrary dummy value
        //set bit 30 here to indicate SDHC support
        err = sdcardSendCommand(state, SD_CMD_SDC_INIT, 0x40000000,
            &resp, 1, timeout);
        if(err) {
            #if SDCARD_DEBUG_PRINT
                printf("SD: CMD41 err %d\r\n", err);
            #endif
            return err;
        }

        switch(resp) {
            case 0x00: ok = 1; break;
            //case 0x01: ok = 1; break;
            case 0x05: return -EINVAL;
        }
    } while(!ok);
    #if SDCARD_DEBUG_PRINT
        printf("SD: CMD41 OK\r\n");
    #endif
    return 0;
}


int _sdSendCmd55(MicronSdCardState *state, uint32_t timeout) {
    /** Send CMD55 (aka ACMD55), ie the prefix for ACMDs.
     *  @param state Card state.
     *  @param timeout Maximum time to wait, in milliseconds.
     *  @return 0 on success, or negative error code on failure.
     */
    uint32_t limit = millis() + timeout;
    int ok, err;
    do {
        if(millis() >= limit) return -ETIMEDOUT;
        ok = 0;
        uint8_t resp = 0xBB; //arbitrary dummy value
        err = sdcardSendCommand(state, SD_CMD_ACMD, 0, &resp, 1, timeout);
        if(err) return err;
        if(resp == 0x00 || resp == 0x01) ok = 1;
    } while(!ok);
    return 0;
}
