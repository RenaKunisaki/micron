#include <micron.h>
#include "ssd1306.h"

int _sendPacket(SSD1306_State *state, int isCmd, const uint8_t *data, size_t len) {
    /** Send a packet to the display.
     *  @param state Display state to use.
     *  @param isCmd Whether this is a command packet or data packet.
     *  @param data Data to send.
     *  @param len Length of data.
     */
    uint8_t prefix = isCmd ? SSD1306_PREFIX_CMD : SSD1306_PREFIX_DATA;
    uint8_t port = state->i2cPort;
    int err = i2cBeginTx(port, state->i2cAddr);
    if(err >  0) err = i2cSend(port, &prefix, 1);
    if(err >= 0) err = i2cSend(port, data, len);
    if(err >= 0) err = i2cEndTx(port, 1);
    return err;
}

int _sendCmdByte(SSD1306_State *state, uint8_t cmd) {
    /** Send a one-byte command.
     *  This can also be used to send parameter bytes one at a time
     *  after the command byte, though this is slower than sending
     *  the command+params in a single packet.
     */
    return _sendPacket(state, 1, &cmd, 1);
}

int _sendDataByte(SSD1306_State *state, uint8_t d) {
    /** Send one data byte.
     */
    return _sendPacket(state, 0, &d, 1);
}

int _sendCmds(SSD1306_State *state, const int *commands) {
    /** Send multiple commands.
     *  Command list ends with -1. (since it's int, you can still send 0xFF.)
     */
    int err;
    for(int i=0; commands[i] != -1; i++) {
        err = _sendCmdByte(state, commands[i]);
        if(err) return err;
        delayMS(10); //XXX necessary?
    }
    return err;
}

int ssd1306_init(SSD1306_State *state) {
    /** Initialize the display.
     *  @param state The display state.
     *  @return 0 on success, or negative error code on failure.
     *  @note You must initialize `state` with the appropriate information
     *   before calling this method.
     */
    static const int commands[] = {
        SSD1306_CMD_DISPLAY_OFF,
        SSD1306_SET_CLOCK_DIVIDE(0x80), //XXX is this just display width?
        SSD1306_SET_MULTIPLEX(state->display.height - 1), //set multiplex
        //some examples set multiplex to 0x2F which seems to just
        //disable the yellow rows. ie this basically sets
        //the display height.
        SSD1306_SET_DISPLAY_OFFSET(0x00),
        SSD1306_SET_CHARGE_PUMP(0x14),
        SSD1306_CMD_INVERT_OFF,
        SSD1306_CMD_DISPLAY_NORMAL,
        SSD1306_SET_START_LINE(0),
        SSD1306_SET_COM_CONFIG(0x12),
        SSD1306_SET_CONTRAST(0x8F),
        SSD1306_SET_PRECHARGE(0xF1),
        SSD1306_SET_VCOMM_DESELECT(0x40),
        SSD1306_CMD_DISABLE_SCROLL,
        SSD1306_SET_ADDRESSING_PAGE,
        //we want H and V flip so that the origin is in the top left
        //corner (assuming pins at top) like most displays.
        SSD1306_CMD_SEGMENT_REMAP_ON, //H flip
        SSD1306_CMD_SET_COM_DESCENDING, //V flip
        //SSD1306_CMD_DISPLAY_ON,
        -1};

    state->display.bitsPerPixel = 1;
    state->display.setPixel = displaySetPixel1BPP;
    state->display.updatePartial = (DisplayUpdatePartialFunc)ssd1306_updatePartial;
    state->display.update = (DisplayUpdateFunc)ssd1306_update;
    return _sendCmds(state, commands);
}

int ssd1306_setPower(SSD1306_State *state, int on) {
    /** Turn the display on or off.
     */
    return _sendCmdByte(state,
        on ? SSD1306_CMD_DISPLAY_ON : SSD1306_CMD_DISPLAY_OFF);
}

int ssd1306_setTestMode(SSD1306_State *state, int on) {
    /** Put the display into test mode (all pixels on).
     */
    return _sendCmdByte(state,
        on ? SSD1306_CMD_DISPLAY_TEST : SSD1306_CMD_DISPLAY_NORMAL);
}

int ssd1306_setContrast(SSD1306_State *state, uint8_t contrast) {
    /** Set the contrast.
     */
    uint8_t cmd[] = {SSD1306_SET_CONTRAST(contrast)};
    return _sendPacket(state, 1, cmd, sizeof(cmd));
}

int ssd1306_setStartLine(SSD1306_State *state, uint8_t line) {
    /** Set the start line of the display.
     *  Effectively, sets the vertical scroll.
     */
    uint8_t cmd[] = {SSD1306_SET_START_LINE(line)};
    return _sendPacket(state, 1, cmd, sizeof(cmd));
}

int ssd1306_setOffset(SSD1306_State *state, uint8_t offs) {
    /** Set the display offset.
     *  Effectively, does the same as setting the start line.
     */
    uint8_t cmd[] = {SSD1306_SET_DISPLAY_OFFSET(offs)};
    return _sendPacket(state, 1, cmd, sizeof(cmd));
}

int ssd1306_setInvert(SSD1306_State *state, int on) {
    /** Set whether to invert pixels.
     */
    return _sendCmdByte(state,
        on ? SSD1306_CMD_INVERT_ON : SSD1306_CMD_INVERT_OFF);
}

int ssd1306_setHFlip(SSD1306_State *state, int on) {
    /** Set whether to flip the display horizontally.
     *  Note, it's flipped by default.
     */
    return _sendCmdByte(state, on ? SSD1306_CMD_SEGMENT_REMAP_ON : SSD1306_CMD_SEGMENT_REMAP_OFF);
}

int ssd1306_setVFlip(SSD1306_State *state, int on) {
    /** Set whether to flip the display vertically.
     *  Note, it's flipped by default.
     */
    return _sendCmdByte(state, on ? SSD1306_CMD_SET_COM_DESCENDING : SSD1306_CMD_SET_COM_ASCENDING);
}

int ssd1306_updatePartial(SSD1306_State *state, int xs, int ys, int width, int height) {
    /** Send block of screenData to display.
     */
    uint8_t *screenData = (uint8_t*)state->display.vram;
    int err = 0;
    int pkSize = MIN(width, SSD1306_MAX_PACKET_LEN);
    for(int y=0; y<height; y += 8) {
        int row = (y+ys)/8;
        _sendCmdByte(state, SSD1306_SET_PAGE_START(row));
        for(int x=0; x<width; x += pkSize) {
            int xp = x+xs;
            uint8_t cmd[] = {SSD1306_SET_COL_START(xp)};
            err = _sendPacket(state, 1, cmd, sizeof(cmd));
            if(err) break;

            uint8_t packet[pkSize];
            for(int i=0; i<pkSize; i++) {
                int xo = xp; //* state->display.width;
                int yo = row * state->display.width;
                packet[i] = screenData[i+xo+yo];
            }
            err = _sendPacket(state, 0, packet, sizeof(packet));
            if(err) break;
        }
    }
    return err;
}

int ssd1306_update(SSD1306_State *state) {
    /** Send entire screenData to display.
     *  This is slow and should be avoided.
     */
    return ssd1306_updatePartial(state, 0, 0, state->display.width, state->display.height);
}
