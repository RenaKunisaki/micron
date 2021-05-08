#include <micron.h>
#include "display.h"

int displaySetPixel1BPP(MicronDisplayState *state, int x, int y, uint32_t p) {
    /** DisplaySetPixelFunc suitable for 1-bit-per-pixel displays.
     */
    if(x < 0 || y < 0 || x >= state->width || y >= state->height) {
        return -ERANGE;
    }
    uint8_t *screenData = (uint8_t*)state->vram;
    int byte = DISPLAY_1BPP_BYTE_AT_COORDS(state, x, y);
    int bit  = DISPLAY_1BPP_BIT_AT_COORDS(state, x, y);
    uint8_t data = 1 << bit;
    if(p) screenData[byte] |=  data;
    else  screenData[byte] &= ~data;
    return 0;
}

//XXX do these work with generic displays or only the odd ordering of SSD1306?
int displayDrawLineH1BPP(MicronDisplayState *state, int xs, int ys, int w,
uint8_t p) {
    //fast function for drawing horizontal lines on 1-bit-per-pixel displays.
    //if(w < 0) { xs += w; w = -w; }
    if(ys < 0 || ys >= state->height) return -ERANGE;
    if(xs < 0) {
        w += xs;
        xs = 0;
    }
    if(xs+w >= state->width) w = state->width - xs;

    uint8_t *screenData = (uint8_t*)state->vram;
    int byte = DISPLAY_1BPP_BYTE_AT_COORDS(state, xs, ys);
    int bit  = DISPLAY_1BPP_BIT_AT_COORDS(state, xs, ys);
    uint8_t data = 1 << bit;

    if(p) {
        for(int x=0; x<w; x++) {
            screenData[byte++] |= data;
        }
    }
    else {
        for(int x=0; x<w; x++) {
            screenData[byte++] &= ~data;
        }
    }
    return 0;
}

int displayDrawLineV1BPP(MicronDisplayState *state, int xs, int ys, int h,
uint8_t p) {
    //fast function for drawing vertical lines on 1-bit-per-pixel displays.
    //if(h < 0) { ys += h; h = -h; }
    if(xs < 0 || xs >= state->width) return -ERANGE;
    if(ys < 0) { h += ys; ys = 0; }
    if(ys+h >= state->height) h = state->height - ys;

    uint8_t d = 0;
    int byte;
    uint8_t *screenData = (uint8_t*)state->vram;

    //fill in bits of partial columns at top
    //lowest bit = lowest Y coord
    int top = ys & 7;
    if(top) {
        for(int i=top; i<MIN(8, h); i++) {
            d |= (1 << i);
        }
        byte = DISPLAY_1BPP_BYTE_AT_COORDS(state, xs, ys);
        if(p) screenData[byte] |= d;
        else  screenData[byte] &= ~d;
        h -= (8-top);
        ys = (ys | 7) + 1;
    }

    //fill in whole-byte columns
    while(h > 7) {
        byte = DISPLAY_1BPP_BYTE_AT_COORDS(state, xs, ys);
        screenData[byte] = p ? 0xFF : 0;
        ys += 8;
        h -= 8;
    }

    //fill in partial columns at bottom
    d = 0;
    int bot = ys & 7;
    for(int i=bot; i<MIN(8, h); i++) {
        d |= (1 << i);
    }
    byte = DISPLAY_1BPP_BYTE_AT_COORDS(state, xs, ys);
    if(p) screenData[byte] |= d;
    else  screenData[byte] &= ~d;

    return 0;
}
