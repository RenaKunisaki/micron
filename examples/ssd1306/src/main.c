/** Test/demo of SSD1306 / SSD1780 I2C OLED display.
 */
#include <micron.h>
#include <drivers/display/ssd1306/ssd1306.h>
#include <drivers/display/fonts/8x8_1bpp.h>

#define USE_SPI 0 //else use I2C

#if USE_SPI
    #define SSD1306_SPI_PORT 0 //which SPI port (most MCUs only have one)
    #define SSD1306_PIN_DC 15 //any GPIO will do
    #define SSD1306_PIN_CS 10 //any GPIO will do, but ones marked CS on the
        //MCU's spec sheet will have better performance.
    #define SSD1306_PIN_RESET 9
#else //not USE_SPI
    #define SSD1306_I2C_PORT 0 //which I2C port
    #define SSD1306_I2C_ADDR 0x3C //default address - can be changed by
        //changing jumpers on the board
#endif //USE_SPI

uint8_t vram[(128 * 64) / 8];
SSD1306_State display;

void blink(int n) {
    gpioSetPinMode(13, PIN_MODE_OUTPUT); //onboard LED
    while(n--) {
        gpioSetPinOutput(13, 1);
        delayMS(250);
        gpioSetPinOutput(13, 0);
        delayMS(250);
    }
}

void fail(int code) {
    blink(code);
    osBootloader();
}

#if USE_SPI
int init_spi() {
    display.port  = SSD1306_SPI_PORT;
    display.pinDC = SSD1306_PIN_DC;
    display.spi   = true;

    #ifdef SSD1306_PIN_RESET
        //reset the display
        gpioSetPinMode(SSD1306_PIN_RESET, PIN_MODE_OUTPUT);
        gpioSetPinOutput(SSD1306_PIN_RESET, 0);
        delayMS(50);
        gpioSetPinOutput(SSD1306_PIN_RESET, 1);
        delayMS(50);
    #endif //SSD1306_PIN_RESET

    int err = spiInit(display.port, SSD1306_PIN_CS, 2, SPI_MODE_0);
    if(err) {
        printf("SPI init fail %d\r\n", err);
        return err;
    }

    err = spiSetFrameSize(display.port, 8);
    if(err) {
        printf("spiSetFrameSize fail %d\r\n", err);
        return err;
    }

    //XXX shouldn't be needed
    err = spiSetMode(display.port, SPI_MODE_0);
    if(err) {
        printf("spiSetMode fail %d\r\n", err);
        return err;
    }

    err = spiPause(display.port, false); //unpause
    if(err < 0) {
        printf("spiPause fail %d\r\n", err);
        return err;
    }

    return 0;
}

#else //not USE_SPI

int init_i2c() {
    display.port    = SSD1306_I2C_PORT;
    display.i2cAddr = SSD1306_I2C_ADDR;
    display.spi     = false;

    int err = i2cInit(display.port, -1);
    if(err) {
        printf("I2C init fail %d\r\n", err);
        blink(4);
        i2cShutdown(0);
        return err;
    }
    return 0;
}
#endif //USE_SPI

int main() {
    display.display.vram = vram;
    display.display.width = 128;
    display.display.height = 64;

    gpioSetPinMode(13, PIN_MODE_OUTPUT); //onboard LED
    blink(1);

    //init serial
    int err = 0;
    //print startup message
	serialInit(0, 460800);
    stdout = openSerial(0, &err);
    stderr = stdout;
    if(err) {
        //not using 1 or 2 because those could be mistaken
        //for normal behaviour or bad connection
        fail(3);
    }
    printf("\e[0m\e[2J\e[H\r\n *** SSD1306 DEMO PROGRAM *** \r\n");

    while(1) { //init I2C/SPI and display
        #if USE_SPI
            err = init_spi();
        #else
            err = init_i2c();
        #endif
        if(!err) {
            //SPI/I2C init OK, now init display itself.
            err = ssd1306_init(&display);
            if(err) {
                printf("Display init fail %d\r\n", err);
                blink(5);
            }
            else break; //display init OK
        }
        else blink(4);
    }
    printf("Init OK\r\n");

    ssd1306_setPower(&display, 1);
    ssd1306_setTestMode(&display, 1);
    delayMS(500);
    ssd1306_setTestMode(&display, 0);

    int frame = 0;
    int fps = 0;
    int w = display.display.width;
    int h = display.display.height;
    char msg[64];
    uint32_t tStart = millis();
    while(1) {
        displayFillRect(&display.display, 0, 0, w, h, 0);
        displayDrawLine(&display.display, w/2, h-1,
            (frame * w) / 30, 16, 1);
        sprintf(msg, "%3d FPS", fps);
        printf("\r%s  Err %d     ", msg, err);
        displayDrawStr(&display.display, &font_8x8_1bpp, 4, 4, msg, 1);
        gpioSetPinOutput(13, 1);
        err = displayUpdate(&display.display);
        gpioSetPinOutput(13, 0);
        idle();
        frame++;
        if(frame >= 30) frame = 0;
        uint32_t tEnd = millis();
        fps = 1000 / (tEnd - tStart);
        tStart = tEnd;
    }

	return 0;
}
