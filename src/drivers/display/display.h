//Generic routines for driving displays.
#include "fonts/font.h"

struct MicronDisplayState; //declare

//Function pointer type for display set pixel
typedef int (*DisplaySetPixelFunc)(MicronDisplayState *state, int x, int y,
uint32_t p);

//Function pointer type for display partial update
typedef int (*DisplayUpdatePartialFunc)(MicronDisplayState *state, int xs,
int ys, int w, int h);

//Function pointer type for display full update
typedef int (*DisplayUpdateFunc)(MicronDisplayState *state);

typedef struct MicronDisplayState {
    uint16_t width, height; //dimensions in pixels
    uint8_t bitsPerPixel;
    void *vram; //image data buffer
    //function pointers
    DisplaySetPixelFunc setPixel;
    DisplayUpdatePartialFunc updatePartial;
    DisplayUpdateFunc update;
} MicronDisplayState;

#define DISPLAY_1BPP_BYTE_AT_COORDS(s, x, y) ((x) + (((y)/8) * s->width))
#define DISPLAY_1BPP_BIT_AT_COORDS(s, x, y) ((y)&7)

//1bpp.c
int displaySetPixel1BPP(MicronDisplayState *state, int x, int y, uint32_t p);
int displayDrawLineH1BPP(MicronDisplayState *state, int xs, int ys, int w,
    uint8_t p);
int displayDrawLineV1BPP(MicronDisplayState *state, int xs, int ys, int h,
    uint8_t p);

//display.c
int displayUpdatePartial(MicronDisplayState *state, int xs, int ys, int width,
    int height);
int displayUpdate(MicronDisplayState *state);
int displaySetPixel(MicronDisplayState *state, int x, int y, uint32_t p);
int displayDrawRect(MicronDisplayState *state, int xs, int ys, int w, int h,
    uint32_t p);
int displayFillRect(MicronDisplayState *state, int xs, int ys, int w, int h,
    uint32_t p);

//drawline.c
int displayDrawLine(MicronDisplayState *state, int x1, int y1, int x2, int y2,
    uint32_t p);

//drawtext.c
int displayDrawChr(MicronDisplayState *state, const Font *font, int xpos,
    int ypos, int chr, uint32_t p);
int displayDrawStr(MicronDisplayState *state, const Font *font, int xpos,
    int ypos, const char *str, uint32_t p);
