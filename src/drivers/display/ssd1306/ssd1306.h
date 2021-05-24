//SSD1306 / SSD1780 I2C OLED display module
//128 x 64 pixels @ 1 bit per pixel
//top 16 rows are yellow, rest are blue; a gap ~2px separates them.
//XXX these also exist as 128 x 32, and as SPI instead of I2C.

//as best I can tell, SSD1306 and SSD1306 are two names for the same thing.

#include "../display.h"
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_DATA_SIZE ((SSD1306_WIDTH*SSD1306_HEIGHT)/8)


//it will actually accept up to 32 bytes but this includes the
//prefix byte telling if it's command/data.
//trying to use that complicates the update function. XXX fix that.
#define SSD1306_MAX_PACKET_LEN 16

typedef struct {
    MicronDisplayState display;
    bool spi; //whether SPI or I2C
    uint8_t port; //which I2C port (0..NUM_I2C-1 or 0..NUM_SPI-1)
    union {
        uint8_t i2cAddr; //the display's I2C address
        uint8_t pinDC; //D/C GPIO pin
    };
} SSD1306_State;

int ssd1306_init(SSD1306_State *state);
int ssd1306_setPower(SSD1306_State *state, int on);
int ssd1306_setTestMode(SSD1306_State *state, int on);
int ssd1306_setContrast(SSD1306_State *state, uint8_t contrast);
int ssd1306_setStartLine(SSD1306_State *state, uint8_t line);
int ssd1306_setOffset(SSD1306_State *state, uint8_t offs);
int ssd1306_setInvert(SSD1306_State *state, int on);
int ssd1306_setHFlip(SSD1306_State *state, int on);
int ssd1306_setVFlip(SSD1306_State *state, int on);
int ssd1306_updatePartial(SSD1306_State *state, int xs, int ys, int width,
    int height);
int ssd1306_update(SSD1306_State *state);

#define SSD1306_PREFIX_CMD  0x00 //first byte of command packet
#define SSD1306_PREFIX_DATA 0x40 //first byte of data packet
//note this is reversed from what some examples say...

#define SSD1306_CMD_SET_CONTRAST             0x81 //followed by 00-FF
#define SSD1306_CMD_DISPLAY_NORMAL           0xA4 //display contents of RAM
#define SSD1306_CMD_DISPLAY_TEST             0xA5 //test mode, light all pixels
#define SSD1306_CMD_INVERT_OFF               0xA6 //do not invert pixels (0=off, 1=on)
#define SSD1306_CMD_INVERT_ON                0xA7 //invert pixels (1=off, 0=on)
#define SSD1306_CMD_DISPLAY_OFF              0xAE //turn display off
#define SSD1306_CMD_DISPLAY_ON               0xAF //turn display on
#define SSD1306_CMD_CONTINUOUS_SCROLL_RIGHT  0x26
#define SSD1306_CMD_CONTINUOUS_SCROLL_LEFT   0x27
#define SSD1306_CMD_CONTINUOUS_SCROLL_RIGHTV 0x29 //right+vertical
#define SSD1306_CMD_CONTINUOUS_SCROLL_LEFTV  0x2A //left+vertical
#define SSD1306_CMD_DISABLE_SCROLL           0x2E //stop scrolling
#define SSD1306_CMD_ENABLE_SCROLL            0x2F //start scrolling
#define SSD1306_CMD_SET_VSCROLL_AREA         0xA3

#define SSD1306_CMD_SET_ADDRESSING_MODE 0x20
#define SSD1306_ADDRESSING_HORIZONTAL 0x00
#define SSD1306_ADDRESSING_VERTICAL   0x01
#define SSD1306_ADDRESSING_PAGE       0x02

//only used for page addressing mode
#define SSD1306_CMD_SET_COL_START_LO 0x00 //low 4 bits = start addr low nybble
#define SSD1306_CMD_SET_COL_START_HI 0x10 //low 4 bits = start addr high nybble
#define SSD1306_CMD_SET_PAGE_START   0xB0 //low 3 bits = start addr

//only used for horizontal/vertical addressing mode
#define SSD1306_CMD_SET_COLUMN_ADDR 0x21
#define SSD1306_CMD_SET_PAGE_ADDR   0x22

#define SSD1306_CMD_SET_START_LINE     0x40 //low 6 bits = start line
#define SSD1306_CMD_SEGMENT_REMAP_OFF  0xA0
#define SSD1306_CMD_SEGMENT_REMAP_ON   0xA1 //effectively horizontal flip
#define SSD1306_CMD_SET_MULTIPLEX      0xA8
#define SSD1306_CMD_SET_COM_ASCENDING  0xC0
#define SSD1306_CMD_SET_COM_DESCENDING 0xC8 //effectively vertical flip
#define SSD1306_CMD_SET_DISPLAY_OFFSET 0xD3
#define SSD1306_CMD_SET_COM_CONFIG     0xDA
#define SSD1306_CMD_SET_CLOCK_DIVIDE   0xD5
#define SSD1306_CMD_SET_PRECHARGE      0xD9
#define SSD1306_CMD_SET_VCOMM_DESELECT 0xDB
#define SSD1306_CMD_SET_CHARGE_PUMP    0x8D //undocumented!?
#define SSD1306_CMD_NOP                0xE3 //no operation

//wrapper macros for commands
#define SSD1306_SET_CONTRAST(c) SSD1306_CMD_SET_CONTRAST, (c)
#define SSD1306_SET_ADDRESSING_MODE(m) SSD1306_CMD_SET_ADDRESSING_MODE, (m)
#define SSD1306_SET_ADDRESSING_HORIZONTAL SSD1306_SET_ADDRESSING_MODE(SSD1306_ADDRESSING_HORIZONTAL)
#define SSD1306_SET_ADDRESSING_VERTICAL SSD1306_SET_ADDRESSING_MODE(SSD1306_ADDRESSING_VERTICAL)
#define SSD1306_SET_ADDRESSING_PAGE \
    SSD1306_SET_ADDRESSING_MODE(SSD1306_ADDRESSING_PAGE)

#define SSD1306_SET_COL_START(a) \
    (uint8_t)(SSD1306_CMD_SET_COL_START_LO | (((uint8_t)a) & 0x0F)), \
    (uint8_t)(SSD1306_CMD_SET_COL_START_HI | ((((uint8_t)a) >> 4) & 0x0F))
#define SSD1306_SET_PAGE_START(a) (uint8_t)(SSD1306_CMD_SET_PAGE_START | ((uint8_t)a))
#define SSD1306_SET_START_LINE(a) (uint8_t)(SSD1306_CMD_SET_START_LINE | ((uint8_t)a))
#define SSD1306_SET_MULTIPLEX(a) (uint8_t)SSD1306_CMD_SET_MULTIPLEX, ((uint8_t)a)
#define SSD1306_SET_DISPLAY_OFFSET(a) (uint8_t)SSD1306_CMD_SET_DISPLAY_OFFSET, ((uint8_t)a)
#define SSD1306_SET_COM_CONFIG(a) (uint8_t)SSD1306_CMD_SET_COM_CONFIG, ((uint8_t)a)
#define SSD1306_SET_CLOCK_DIVIDE(a) (uint8_t)SSD1306_CMD_SET_CLOCK_DIVIDE, ((uint8_t)a)
#define SSD1306_SET_PRECHARGE(a) (uint8_t)SSD1306_CMD_SET_PRECHARGE, ((uint8_t)a)
#define SSD1306_SET_VCOMM_DESELECT(a) (uint8_t)SSD1306_CMD_SET_VCOMM_DESELECT, ((uint8_t)a)
#define SSD1306_SET_CHARGE_PUMP(v) (uint8_t)SSD1306_CMD_SET_CHARGE_PUMP, ((uint8_t)v)
