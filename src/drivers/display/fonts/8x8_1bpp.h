#ifndef FONT_8x8_1bpp_H
#define FONT_8x8_1bpp_H
#include "font.h"

/* arduino #include <avr/pgmspace.h> */

/* font_h Width 128, Height 128 */
const Font font_8x8_1bpp = {
    .width = 128,
    .height = 128,
    .chrWidth = 8,
    .chrHeight = 8,
    .bitsPerPixel = 1,
    .data = {
0x00, 0x7E, 0x7E, 0x6C, 0x10, 0x38, 0x10, 0x00, 0xFF, 0x00, 0xFF, 0x0F, 0x3C, 0x3F, 0x7F, 0x99, /* scanline 1 */
0x00, 0x81, 0xFF, 0xFE, 0x38, 0x7C, 0x10, 0x00, 0xFF, 0x3C, 0xC3, 0x07, 0x66, 0x33, 0x63, 0x5A, /* scanline 2 */
0x00, 0xA5, 0xDB, 0xFE, 0x7C, 0x38, 0x38, 0x18, 0xE7, 0x66, 0x99, 0x0F, 0x66, 0x3F, 0x7F, 0x3C, /* scanline 3 */
0x00, 0x81, 0xFF, 0xFE, 0xFE, 0xFE, 0x7C, 0x3C, 0xC3, 0x42, 0xBD, 0x7D, 0x66, 0x30, 0x63, 0xE7, /* scanline 4 */
0x00, 0xBD, 0xC3, 0x7C, 0x7C, 0xFE, 0xFE, 0x3C, 0xC3, 0x42, 0xBD, 0xCC, 0x3C, 0x30, 0x63, 0xE7, /* scanline 5 */
0x00, 0x99, 0xE7, 0x38, 0x38, 0xD6, 0x7C, 0x18, 0xE7, 0x66, 0x99, 0xCC, 0x18, 0x70, 0x67, 0x3C, /* scanline 6 */
0x00, 0x81, 0xFF, 0x10, 0x10, 0x10, 0x10, 0x00, 0xFF, 0x3C, 0xC3, 0xCC, 0x7E, 0xF0, 0xE6, 0x5A, /* scanline 7 */
0x00, 0x7E, 0x7E, 0x00, 0x00, 0x38, 0x38, 0x00, 0xFF, 0x00, 0xFF, 0x78, 0x18, 0xE0, 0xC0, 0x99, /* scanline 8 */
0x80, 0x02, 0x18, 0x66, 0x7F, 0x7E, 0x00, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* scanline 9 */
0xE0, 0x0E, 0x3C, 0x66, 0xDB, 0xC3, 0x00, 0x3C, 0x3C, 0x18, 0x18, 0x30, 0x00, 0x24, 0x18, 0xFF, /* scanline 10 */
0xF8, 0x3E, 0x7E, 0x66, 0xDB, 0x78, 0x00, 0x7E, 0x7E, 0x18, 0x0C, 0x60, 0xC0, 0x66, 0x3C, 0xFF, /* scanline 11 */
0xFE, 0xFE, 0x18, 0x66, 0x7B, 0xCC, 0x00, 0x18, 0x18, 0x18, 0xFE, 0xFE, 0xC0, 0xFF, 0x7E, 0x7E, /* scanline 12 */
0xF8, 0x3E, 0x18, 0x66, 0x1B, 0xCC, 0x7E, 0x7E, 0x18, 0x7E, 0x0C, 0x60, 0xC0, 0x66, 0xFF, 0x3C, /* scanline 13 */
0xE0, 0x0E, 0x7E, 0x00, 0x1B, 0x78, 0x7E, 0x3C, 0x18, 0x3C, 0x18, 0x30, 0xFE, 0x24, 0xFF, 0x18, /* scanline 14 */
0x80, 0x02, 0x3C, 0x66, 0x1B, 0x8C, 0x7E, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* scanline 15 */
0x00, 0x00, 0x18, 0x00, 0x00, 0xF8, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* scanline 16 */
0x00, 0x30, 0x6C, 0x6C, 0x30, 0x00, 0x38, 0x60, 0x18, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, /* scanline 17 */
0x00, 0x78, 0x6C, 0x6C, 0x7C, 0xC6, 0x6C, 0x60, 0x30, 0x30, 0x66, 0x30, 0x00, 0x00, 0x00, 0x0C, /* scanline 18 */
0x00, 0x78, 0x6C, 0xFE, 0xC0, 0xCC, 0x38, 0xC0, 0x60, 0x18, 0x3C, 0x30, 0x00, 0x00, 0x00, 0x18, /* scanline 19 */
0x00, 0x30, 0x00, 0x6C, 0x78, 0x18, 0x76, 0x00, 0x60, 0x18, 0xFF, 0xFC, 0x00, 0xFC, 0x00, 0x30, /* scanline 20 */
0x00, 0x30, 0x00, 0xFE, 0x0C, 0x30, 0xDC, 0x00, 0x60, 0x18, 0x3C, 0x30, 0x00, 0x00, 0x00, 0x60, /* scanline 21 */
0x00, 0x00, 0x00, 0x6C, 0xF8, 0x66, 0xCC, 0x00, 0x30, 0x30, 0x66, 0x30, 0x70, 0x00, 0x30, 0xC0, /* scanline 22 */
0x00, 0x30, 0x00, 0x6C, 0x30, 0xC6, 0x76, 0x00, 0x18, 0x60, 0x00, 0x00, 0x30, 0x00, 0x30, 0x80, /* scanline 23 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, /* scanline 24 */
0x78, 0x30, 0x78, 0x78, 0x1C, 0xFC, 0x38, 0xFC, 0x78, 0x78, 0x00, 0x00, 0x18, 0x00, 0x60, 0x78, /* scanline 25 */
0xCC, 0xF0, 0xCC, 0xCC, 0x3C, 0xC0, 0x60, 0xCC, 0xCC, 0xCC, 0x00, 0x00, 0x30, 0x00, 0x30, 0xCC, /* scanline 26 */
0xDC, 0x30, 0x0C, 0x0C, 0x6C, 0xF8, 0xC0, 0x0C, 0xCC, 0xCC, 0x30, 0x30, 0x60, 0xFC, 0x18, 0x0C, /* scanline 27 */
0xFC, 0x30, 0x38, 0x38, 0xCC, 0x0C, 0xF8, 0x18, 0x78, 0x7C, 0x30, 0x30, 0xC0, 0x00, 0x0C, 0x18, /* scanline 28 */
0xEC, 0x30, 0x60, 0x0C, 0xFE, 0x0C, 0xCC, 0x30, 0xCC, 0x0C, 0x00, 0x00, 0x60, 0xFC, 0x18, 0x30, /* scanline 29 */
0xCC, 0x30, 0xCC, 0xCC, 0x0C, 0xCC, 0xCC, 0x60, 0xCC, 0x18, 0x30, 0x70, 0x30, 0x00, 0x30, 0x00, /* scanline 30 */
0x78, 0xFC, 0xFC, 0x78, 0x0C, 0x78, 0x78, 0x60, 0x78, 0x70, 0x30, 0x30, 0x18, 0x00, 0x60, 0x30, /* scanline 31 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, /* scanline 32 */
0x7C, 0x30, 0xFC, 0x3C, 0xFC, 0xFE, 0xFE, 0x3C, 0xCC, 0x78, 0x1E, 0xE6, 0xF0, 0xC6, 0xC6, 0x38, /* scanline 33 */
0xC6, 0x78, 0x66, 0x66, 0x6C, 0x62, 0x62, 0x66, 0xCC, 0x30, 0x0C, 0x66, 0x60, 0xEE, 0xE6, 0x6C, /* scanline 34 */
0xDE, 0xCC, 0x66, 0xC0, 0x66, 0x68, 0x68, 0xC0, 0xCC, 0x30, 0x0C, 0x6C, 0x60, 0xFE, 0xF6, 0xC6, /* scanline 35 */
0xDE, 0xCC, 0x7C, 0xC0, 0x66, 0x78, 0x78, 0xC0, 0xFC, 0x30, 0x0C, 0x78, 0x60, 0xD6, 0xDE, 0xC6, /* scanline 36 */
0xDE, 0xFC, 0x66, 0xC0, 0x66, 0x68, 0x68, 0xCE, 0xCC, 0x30, 0xCC, 0x6C, 0x62, 0xC6, 0xCE, 0xC6, /* scanline 37 */
0xC0, 0xCC, 0x66, 0x66, 0x6C, 0x62, 0x60, 0x66, 0xCC, 0x30, 0xCC, 0x66, 0x66, 0xC6, 0xC6, 0x6C, /* scanline 38 */
0x78, 0xCC, 0xFC, 0x3C, 0xFC, 0xFE, 0xF0, 0x3E, 0xCC, 0x78, 0x78, 0xE6, 0xFE, 0xC6, 0xC6, 0x38, /* scanline 39 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* scanline 40 */
0xFC, 0x78, 0xFC, 0x78, 0xFC, 0xCC, 0xCC, 0xC6, 0xC6, 0xCC, 0xFE, 0x78, 0xC0, 0x78, 0x10, 0x00, /* scanline 41 */
0x66, 0xCC, 0x66, 0xCC, 0xB4, 0xCC, 0xCC, 0xC6, 0xC6, 0xCC, 0xCC, 0x60, 0x60, 0x18, 0x38, 0x00, /* scanline 42 */
0x66, 0xCC, 0x66, 0xE0, 0x30, 0xCC, 0xCC, 0xC6, 0x6C, 0xCC, 0x98, 0x60, 0x30, 0x18, 0x6C, 0x00, /* scanline 43 */
0x7C, 0xCC, 0x7C, 0x38, 0x30, 0xCC, 0xCC, 0xD6, 0x38, 0x78, 0x30, 0x60, 0x18, 0x18, 0xC6, 0x00, /* scanline 44 */
0x60, 0xDC, 0x78, 0x1C, 0x30, 0xCC, 0xCC, 0xFE, 0x6C, 0x30, 0x62, 0x60, 0x0C, 0x18, 0x00, 0x00, /* scanline 45 */
0x60, 0x78, 0x6C, 0xCC, 0x30, 0xCC, 0x78, 0xEE, 0xC6, 0x30, 0xC6, 0x60, 0x06, 0x18, 0x00, 0x00, /* scanline 46 */
0xF0, 0x1C, 0xE6, 0x78, 0x78, 0xFC, 0x30, 0xC6, 0xC6, 0x78, 0xFE, 0x78, 0x02, 0x78, 0x00, 0x00, /* scanline 47 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, /* scanline 48 */
0x30, 0x00, 0xE0, 0x00, 0x1C, 0x00, 0x38, 0x00, 0xE0, 0x30, 0x18, 0xE0, 0x70, 0x00, 0x00, 0x00, /* scanline 49 */
0x30, 0x00, 0x60, 0x00, 0x0C, 0x00, 0x6C, 0x00, 0x60, 0x00, 0x00, 0x60, 0x30, 0x00, 0x00, 0x00, /* scanline 50 */
0x18, 0x78, 0x7C, 0x78, 0x0C, 0x78, 0x60, 0x76, 0x6C, 0x70, 0x78, 0x66, 0x30, 0xEC, 0xF8, 0x78, /* scanline 51 */
0x00, 0x0C, 0x66, 0xCC, 0x7C, 0xCC, 0xF0, 0xCC, 0x76, 0x30, 0x18, 0x6C, 0x30, 0xFE, 0xCC, 0xCC, /* scanline 52 */
0x00, 0x7C, 0x66, 0xC0, 0xCC, 0xFC, 0x60, 0xCC, 0x66, 0x30, 0x18, 0x78, 0x30, 0xD6, 0xCC, 0xCC, /* scanline 53 */
0x00, 0xCC, 0x66, 0xCC, 0xCC, 0xC0, 0x60, 0x7C, 0x66, 0x30, 0x18, 0x6C, 0x30, 0xC6, 0xCC, 0xCC, /* scanline 54 */
0x00, 0x76, 0xBC, 0x78, 0x76, 0x78, 0xF0, 0x0C, 0xE6, 0x78, 0xD8, 0xE6, 0x78, 0xC6, 0xCC, 0x78, /* scanline 55 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, /* scanline 56 */
0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x18, 0xE0, 0x76, 0x10, /* scanline 57 */
0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x18, 0x30, 0xDC, 0x38, /* scanline 58 */
0xDC, 0x76, 0xD8, 0x7C, 0x7C, 0xCC, 0xCC, 0xC6, 0xC6, 0xCC, 0xFC, 0x30, 0x18, 0x30, 0x00, 0x6C, /* scanline 59 */
0x66, 0xCC, 0x6C, 0xC0, 0x30, 0xCC, 0xCC, 0xC6, 0x6C, 0xCC, 0x98, 0xE0, 0x00, 0x1C, 0x00, 0xC6, /* scanline 60 */
0x66, 0xCC, 0x6C, 0x78, 0x30, 0xCC, 0xCC, 0xD6, 0x38, 0xCC, 0x30, 0x30, 0x18, 0x30, 0x00, 0xC6, /* scanline 61 */
0x7C, 0x7C, 0x60, 0x0C, 0x34, 0xCC, 0x78, 0xFE, 0x6C, 0x7C, 0x64, 0x30, 0x18, 0x30, 0x00, 0xC6, /* scanline 62 */
0x60, 0x0C, 0xF0, 0xF8, 0x18, 0x76, 0x30, 0x6C, 0xC6, 0x0C, 0xFC, 0x1C, 0x18, 0xE0, 0x00, 0xFE, /* scanline 63 */
0xF0, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* scanline 64 */
0x78, 0x00, 0x1C, 0x7E, 0xCC, 0xE0, 0x30, 0x00, 0x7E, 0xCC, 0xE0, 0xCC, 0x7C, 0xE0, 0xCC, 0x30, /* scanline 65 */
0xCC, 0xCC, 0x00, 0xC3, 0x00, 0x00, 0x30, 0x00, 0xC3, 0x00, 0x00, 0x00, 0xC6, 0x00, 0x30, 0x30, /* scanline 66 */
0xC0, 0x00, 0x78, 0x3C, 0x78, 0x78, 0x78, 0x7C, 0x3C, 0x78, 0x78, 0x70, 0x38, 0x70, 0x78, 0x00, /* scanline 67 */
0xCC, 0xCC, 0xCC, 0x06, 0x0C, 0x0C, 0x0C, 0xC0, 0x66, 0xCC, 0xCC, 0x30, 0x18, 0x30, 0xCC, 0x78, /* scanline 68 */
0x78, 0xCC, 0xFC, 0x3E, 0x7C, 0x7C, 0x7C, 0xC0, 0x7E, 0xFC, 0xFC, 0x30, 0x18, 0x30, 0xCC, 0xCC, /* scanline 69 */
0x18, 0xCC, 0xC0, 0x66, 0xCC, 0xCC, 0xCC, 0x7C, 0x60, 0xC0, 0xC0, 0x30, 0x18, 0x30, 0xFC, 0xFC, /* scanline 70 */
0x0C, 0x7E, 0x78, 0x3F, 0x7E, 0x7E, 0x7E, 0x06, 0x3C, 0x78, 0x78, 0x78, 0x3C, 0x78, 0xCC, 0xCC, /* scanline 71 */
0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* scanline 72 */
0x1C, 0x00, 0x3E, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00, 0xC6, 0xCC, 0x18, 0x38, 0xCC, 0xF0, 0x0E, /* scanline 73 */
0x00, 0x00, 0x6C, 0xCC, 0xCC, 0xE0, 0xCC, 0xE0, 0xCC, 0x38, 0x00, 0x18, 0x6C, 0xCC, 0xD8, 0x1B, /* scanline 74 */
0xFC, 0x7F, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xCC, 0x7E, 0x64, 0x78, 0xD8, 0x18, /* scanline 75 */
0x60, 0x0C, 0xFE, 0x78, 0x78, 0x78, 0xCC, 0xCC, 0xCC, 0xC6, 0xCC, 0xC0, 0xF0, 0xFC, 0xF4, 0x7E, /* scanline 76 */
0x78, 0x7F, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xC6, 0xCC, 0xC0, 0x60, 0x30, 0xCC, 0x18, /* scanline 77 */
0x60, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFC, 0x7C, 0xCC, 0x7E, 0xE6, 0xFC, 0xDE, 0x18, /* scanline 78 */
0xFC, 0x7F, 0xCE, 0x78, 0x78, 0x78, 0x7E, 0x7E, 0x0C, 0x38, 0x78, 0x18, 0xFC, 0x30, 0xCC, 0xD8, /* scanline 79 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x18, 0x00, 0x00, 0x0E, 0x70, /* scanline 80 */
0x1C, 0x38, 0x00, 0x00, 0x00, 0xFC, 0x3C, 0x3C, 0x30, 0x00, 0x00, 0xC6, 0xC6, 0x00, 0x00, 0x00, /* scanline 81 */
0x00, 0x00, 0x1C, 0x1C, 0xF8, 0x00, 0x6C, 0x66, 0x00, 0x00, 0x00, 0xCC, 0xCC, 0x18, 0x33, 0xCC, /* scanline 82 */
0x78, 0x70, 0x00, 0x00, 0x00, 0xCC, 0x6C, 0x66, 0x30, 0x00, 0x00, 0xD8, 0xD8, 0x00, 0x66, 0x66, /* scanline 83 */
0x0C, 0x30, 0x78, 0xCC, 0xF8, 0xEC, 0x3E, 0x3C, 0x60, 0xFC, 0xFC, 0x3E, 0xF3, 0x18, 0xCC, 0x33, /* scanline 84 */
0x7C, 0x30, 0xCC, 0xCC, 0xCC, 0xFC, 0x00, 0x00, 0xC0, 0xC0, 0x0C, 0x63, 0x67, 0x18, 0x66, 0x66, /* scanline 85 */
0xCC, 0x30, 0xCC, 0xCC, 0xCC, 0xDC, 0x7E, 0x7E, 0xCC, 0xC0, 0x0C, 0xCE, 0xCF, 0x3C, 0x33, 0xCC, /* scanline 86 */
0x7E, 0x78, 0x78, 0x7E, 0xCC, 0xCC, 0x00, 0x00, 0x78, 0x00, 0x00, 0x98, 0x9F, 0x3C, 0x00, 0x00, /* scanline 87 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x03, 0x18, 0x00, 0x00, /* scanline 88 */
0x22, 0x55, 0xDD, 0x18, 0x18, 0x18, 0x36, 0x00, 0x00, 0x36, 0x36, 0x00, 0x36, 0x36, 0x18, 0x00, /* scanline 89 */
0x88, 0xAA, 0x77, 0x18, 0x18, 0x18, 0x36, 0x00, 0x00, 0x36, 0x36, 0x00, 0x36, 0x36, 0x18, 0x00, /* scanline 90 */
0x22, 0x55, 0xDD, 0x18, 0x18, 0xF8, 0x36, 0x00, 0xF8, 0xF6, 0x36, 0xFE, 0xF6, 0x36, 0xF8, 0x00, /* scanline 91 */
0x88, 0xAA, 0x77, 0x18, 0x18, 0x18, 0x36, 0x00, 0x18, 0x06, 0x36, 0x06, 0x06, 0x36, 0x18, 0x00, /* scanline 92 */
0x22, 0x55, 0xDD, 0x18, 0xF8, 0xF8, 0xF6, 0xFE, 0xF8, 0xF6, 0x36, 0xF6, 0xFE, 0xFE, 0xF8, 0xF8, /* scanline 93 */
0x88, 0xAA, 0x77, 0x18, 0x18, 0x18, 0x36, 0x36, 0x18, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x18, /* scanline 94 */
0x22, 0x55, 0xDD, 0x18, 0x18, 0x18, 0x36, 0x36, 0x18, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x18, /* scanline 95 */
0x88, 0xAA, 0x77, 0x18, 0x18, 0x18, 0x36, 0x36, 0x18, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x18, /* scanline 96 */
0x18, 0x18, 0x00, 0x18, 0x00, 0x18, 0x18, 0x36, 0x36, 0x00, 0x36, 0x00, 0x36, 0x00, 0x36, 0x18, /* scanline 97 */
0x18, 0x18, 0x00, 0x18, 0x00, 0x18, 0x18, 0x36, 0x36, 0x00, 0x36, 0x00, 0x36, 0x00, 0x36, 0x18, /* scanline 98 */
0x18, 0x18, 0x00, 0x18, 0x00, 0x18, 0x1F, 0x36, 0x37, 0x3F, 0xF7, 0xFF, 0x37, 0xFF, 0xF7, 0xFF, /* scanline 99 */
0x18, 0x18, 0x00, 0x18, 0x00, 0x18, 0x18, 0x36, 0x30, 0x30, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, /* scanline 100 */
0x1F, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0x1F, 0x37, 0x3F, 0x37, 0xFF, 0xF7, 0x37, 0xFF, 0xF7, 0xFF, /* scanline 101 */
0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x36, 0x00, 0x36, 0x00, 0x36, 0x36, 0x00, 0x36, 0x00, /* scanline 102 */
0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x36, 0x00, 0x36, 0x00, 0x36, 0x36, 0x00, 0x36, 0x00, /* scanline 103 */
0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x36, 0x00, 0x36, 0x00, 0x36, 0x36, 0x00, 0x36, 0x00, /* scanline 104 */
0x36, 0x00, 0x00, 0x36, 0x18, 0x00, 0x00, 0x36, 0x18, 0x18, 0x00, 0xFF, 0x00, 0xF0, 0x0F, 0xFF, /* scanline 105 */
0x36, 0x00, 0x00, 0x36, 0x18, 0x00, 0x00, 0x36, 0x18, 0x18, 0x00, 0xFF, 0x00, 0xF0, 0x0F, 0xFF, /* scanline 106 */
0x36, 0xFF, 0x00, 0x36, 0x1F, 0x1F, 0x00, 0x36, 0xFF, 0x18, 0x00, 0xFF, 0x00, 0xF0, 0x0F, 0xFF, /* scanline 107 */
0x36, 0x00, 0x00, 0x36, 0x18, 0x18, 0x00, 0x36, 0x00, 0x18, 0x00, 0xFF, 0x00, 0xF0, 0x0F, 0xFF, /* scanline 108 */
0xFF, 0xFF, 0xFF, 0x3F, 0x1F, 0x1F, 0x3F, 0xF7, 0xFF, 0xF8, 0x1F, 0xFF, 0xFF, 0xF0, 0x0F, 0x00, /* scanline 109 */
0x00, 0x18, 0x36, 0x00, 0x00, 0x18, 0x36, 0x36, 0x18, 0x00, 0x18, 0xFF, 0xFF, 0xF0, 0x0F, 0x00, /* scanline 110 */
0x00, 0x18, 0x36, 0x00, 0x00, 0x18, 0x36, 0x36, 0x18, 0x00, 0x18, 0xFF, 0xFF, 0xF0, 0x0F, 0x00, /* scanline 111 */
0x00, 0x18, 0x36, 0x00, 0x00, 0x18, 0x36, 0x36, 0x18, 0x00, 0x18, 0xFF, 0xFF, 0xF0, 0x0F, 0x00, /* scanline 112 */
0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0xFC, 0x38, 0x38, 0x1C, 0x00, 0x06, 0x3C, 0x78, /* scanline 113 */
0x00, 0x78, 0xFE, 0xFE, 0x66, 0x00, 0x66, 0x76, 0x30, 0x6C, 0x6C, 0x30, 0x00, 0x0C, 0x60, 0xCC, /* scanline 114 */
0x76, 0xCC, 0xC6, 0x6C, 0x30, 0x7E, 0x66, 0xDC, 0x78, 0xC6, 0xC6, 0x18, 0x7E, 0x7E, 0xC0, 0xCC, /* scanline 115 */
0xDC, 0xF8, 0xC0, 0x6C, 0x18, 0xCC, 0x66, 0x18, 0xCC, 0xFE, 0xC6, 0x7C, 0xDB, 0xDB, 0xFC, 0xCC, /* scanline 116 */
0xC8, 0xCC, 0xC0, 0x6C, 0x30, 0xCC, 0x66, 0x18, 0xCC, 0xC6, 0x6C, 0xCC, 0xDB, 0xDB, 0xC0, 0xCC, /* scanline 117 */
0xDC, 0xF8, 0xC0, 0x6C, 0x66, 0xCC, 0x7C, 0x18, 0x78, 0x6C, 0x6C, 0xCC, 0x7E, 0x7E, 0x60, 0xCC, /* scanline 118 */
0x76, 0xC0, 0xC0, 0x6C, 0xFE, 0x78, 0x60, 0x18, 0x30, 0x38, 0xEE, 0x78, 0x00, 0x60, 0x3C, 0xCC, /* scanline 119 */
0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, /* scanline 120 */
0x00, 0x30, 0x60, 0x18, 0x0E, 0x18, 0x30, 0x00, 0x38, 0x00, 0x00, 0x0F, 0x78, 0x78, 0x00, 0x00, /* scanline 121 */
0xFC, 0x30, 0x30, 0x30, 0x1B, 0x18, 0x30, 0x72, 0x6C, 0x00, 0x00, 0x0C, 0x6C, 0x0C, 0x00, 0x00, /* scanline 122 */
0x00, 0xFC, 0x18, 0x60, 0x1B, 0x18, 0x00, 0x9C, 0x6C, 0x00, 0x00, 0x0C, 0x6C, 0x38, 0x3C, 0x00, /* scanline 123 */
0xFC, 0x30, 0x30, 0x30, 0x18, 0x18, 0xFC, 0x00, 0x38, 0x18, 0x00, 0x0C, 0x6C, 0x60, 0x3C, 0x00, /* scanline 124 */
0x00, 0x30, 0x60, 0x18, 0x18, 0x18, 0x00, 0x72, 0x00, 0x18, 0x18, 0xEC, 0x6C, 0x7C, 0x3C, 0x00, /* scanline 125 */
0xFC, 0x00, 0x00, 0x00, 0x18, 0xD8, 0x30, 0x9C, 0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x3C, 0x00, /* scanline 126 */
0x00, 0xFC, 0xFC, 0xFC, 0x18, 0xD8, 0x30, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, /* scanline 127 */
0x00, 0x00, 0x00, 0x00, 0x18, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, /* scanline 128 */
}};
#endif /* FONT_8x8_1bpp_H */
