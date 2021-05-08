#include <micron.h>
#include "display.h"

//XXX support other BPP

int displayDrawChr(MicronDisplayState *state, const Font *font, int xpos,
int ypos, int chr, uint32_t p) {
    for(int y=0; y<font->chrHeight; y++) {
        int sx = chr & 0xF;
        int sy = (chr >> 4) * 8;
        uint8_t line = font->data[((sy+y)*16)+sx];
        for(int x=font->chrWidth-1; x>=0; x--) {
            uint16_t c = line & (1 << ((font->chrWidth-1)-x));
            if(c) displaySetPixel(state, xpos+x, ypos+y, p);
        }
    }
    return 0;
}

int displayDrawStr(MicronDisplayState *state, const Font *font, int xpos,
int ypos, const char *str, uint32_t p) {
    int x = xpos, y = ypos, count = 0;
    while(*str) {
        char c = *(str++);
        count++;
        switch(c) {
            case '\r': x = xpos; break;
            case '\n': y += font->chrHeight; break;
            case '\t': {
                static const int t = font->chrWidth*4;
                int a = x % t;
                if(a == 0) a = t;
                x += a;
                break;
            }
            default:
                displayDrawChr(state, font, x, y, c, p);
                x += font->chrWidth;
        }
        if(x + font->chrWidth > state->width) {
            x = xpos;
            y += font->chrHeight;
        }
    }
    return count;
}
