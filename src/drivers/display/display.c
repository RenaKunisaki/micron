#include <micron.h>
#include "display.h"

int displayUpdatePartial(MicronDisplayState *state, int xs, int ys, int width,
int height) {
    /** Send block of VRAM to display.
     *  @param state Display state to use.
     *  @param xs X pixel of top left corner of region to update.
     *  @param ys Y pixel of top left corner of region to update.
     *  @param width Width of region to update.
     *  @param height Height of region to update.
     *  @return 0 on success, or negative error code.
     *  @note May update more than the specified region, depending on the
     *   granularity of the display's protocol.
     */
    if(state->updatePartial) {
        return state->updatePartial(state, xs, ys, width, height);
    }
    else return state->update(state);
}

int displayUpdate(MicronDisplayState *state) {
    /** Send all of VRAM to display.
     *  @param state Display state to use.
     *  @return 0 on success, or negative error code.
     */
    return state->update(state);
}

int displaySetPixel(MicronDisplayState *state, int x, int y, uint32_t p) {
    /** Update one pixel of display.
     *  @param state Display state to use.
     *  @param x X pixel coordinate to update.
     *  @param y Y pixel coordinate to update.
     *  @param p New pixel value.
     *  @return 0 on success, or negative error code.
     */
    return state->setPixel(state, x, y, p);
}

int displayDrawRect(MicronDisplayState *state, int xs, int ys, int w, int h,
uint32_t p) {
    /** Draw an outlined rectangle.
     *  @param state Display state to use.
     *  @param xs X pixel coordinate of top left corner.
     *  @param ys Y pixel coordinate of top left corner.
     *  @param w Width of rectangle.
     *  @param h Height of rectangle.
     *  @param p Outline color.
     *  @return 0 on success, or negative error code.
     */
    displayDrawLine(state, xs,     ys,     w, ys, p);
    displayDrawLine(state, xs,     ys+h-1, w, ys, p);
    displayDrawLine(state, xs,     ys,     xs, h, p);
    displayDrawLine(state, xs+w-1, ys,     xs, h, p);
    return 0;
}

int displayFillRect(MicronDisplayState *state, int xs, int ys, int w, int h,
uint32_t p) {
    /** Fill a rectangular region.
     *  @param state Display state to use.
     *  @param xs X pixel coordinate of top left corner.
     *  @param ys Y pixel coordinate of top left corner.
     *  @param w Width of rectangle.
     *  @param h Height of rectangle.
     *  @param p Fill color.
     *  @return 0 on success, or negative error code.
     */
    int err = 0;
    for(int y=0; y<h; y++) {
        for(int x=0; x<w; x++) {
            int px = x+xs;
            int py = y+ys;
            err = displaySetPixel(state, px, py, p);
            if(err) return err;
        }
    }
    return err;
}
