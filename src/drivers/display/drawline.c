#include <micron.h>
#include "display.h"

INLINE void drawLineLow(MicronDisplayState *state, int x1, int y1, int x2,
int y2, uint32_t p) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int inc = 1;
    int y = y1;
    if(dy < 0) { inc = -1; dy = -dy; }
    int D = 2*dy - dx;
    for(int x=x1; x<x2; x++) {
        displaySetPixel(state, x, y, p);
        if(D > 0) { y += inc; D -= 2*dx; }
        D += 2*dy;
    }
}

INLINE void drawLineHi(MicronDisplayState *state, int x1, int y1, int x2,
int y2, uint32_t p) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int inc = 1;
    int x = x1;
    if(dx < 0) { inc = -1; dx = -dx; }
    int D = 2*dx - dy;
    for(int y=y1; y<y2; y++) {
        displaySetPixel(state, x, y, p);
        if(D > 0) { x += inc; D -= 2*dy; }
        D += 2*dx;
    }
}

int displayDrawLine(MicronDisplayState *state, int x1, int y1, int x2, int y2,
uint32_t p) {
    /** Draw a line.
     *  @param state Display state to use.
     *  @param x1 X pixel coordinate of first point.
     *  @param y1 Y pixel coordinate of first point.
     *  @param x2 X pixel coordinate of second point.
     *  @param y2 Y pixel coordinate of second point.
     *  @param p Color to draw.
     *  @return 0 on success, or negative error code.
     */

    //Bresenham's algorithm
    if(state->bitsPerPixel == 1) {
        if(x1 == x2) return displayDrawLineV1BPP(state, x1,MIN(y1,y2),abs(y2-y1),p);
        if(y1 == y2) return displayDrawLineH1BPP(state, MIN(x1,x2),y1,abs(x2-x1),p);
    }

    if(abs(y2-y1) < abs(x2-x1)) {
        if(x1 > x2) drawLineLow(state, x2, y2, x1, y1, p);
        else drawLineLow(state, x1, y1, x2, y2, p);
    }
    else {
        if(y1 > y2) drawLineHi(state, x2, y2, x1, y1, p);
        else drawLineHi(state, x1, y1, x2, y2, p);
    }
    return 0;
}
