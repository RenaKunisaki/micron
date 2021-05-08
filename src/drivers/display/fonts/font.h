#ifndef _FONT_H_
#define _FONT_H_

typedef struct {
    uint16_t width, height;
    uint16_t chrWidth, chrHeight;
    uint8_t bitsPerPixel;
    const uint8_t data[];
} Font;

#endif //_FONT_H_
