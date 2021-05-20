/** Test/demo of SD card driver.
 */
#include <micron.h>
#include <drivers/sdcard/sdcard.h>

//XXX remove these
#include <drivers/display/ssd1306/ssd1306.h>
#include <drivers/display/fonts/8x8_1bpp.h>
#define SSD1306_I2C_PORT 0 //which I2C port
#define SSD1306_I2C_ADDR 0x3C //default address - can be changed by
    //changing jumpers on the board
uint8_t vram[(128 * 64) / 8];
SSD1306_State display;


MicronSdCardState sdcard;


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

//XXX remove
int initDisplay() {
    display.display.vram = vram;
    display.display.width = 128;
    display.display.height = 64;
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

    err = ssd1306_init(&display);
    printf("Display init: %d\r\n", err);

    ssd1306_setPower(&display, 1);
    ssd1306_setTestMode(&display, 1);
    delayMS(500);
    ssd1306_setTestMode(&display, 0);
    //ssd1306_setPower(&display, 1);
    return 0;
}

int displayPrintf(const char *fmt, ...) {
    char buf[2048];
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buf, sizeof(buf), fmt, arg);
    va_end(arg);
    int w = display.display.width;
    int h = display.display.height;
    displayFillRect(&display.display, 0, 0, w, h, 0);
    displayDrawStr(&display.display, &font_8x8_1bpp, 0, 0, buf, 1);
    return displayUpdate(&display.display);
}

int initSD() {
    //XXX move to driver
    sdcard.port = 0;
    sdcard.pinCS = 10;
    displayPrintf("SD init...\r\n");

    //XXX shouldn't init SPI for us unless it isn't already inited
    printf("SD init... ");
    int err = sdcardInit(&sdcard);
    printf("%d\r\n", err);
    if(err) return err;

    displayPrintf("SD reset...\r\n");
    printf("SD reset... ");
    err = sdcardReset(&sdcard, 5000);
    printf("%d\r\n", err);
    if(err) return err;

    uint64_t size = 0;
    displayPrintf("SD get size...\r\n");
    printf("SD get size... ");
    err = sdGetSize(&sdcard, &size, 1000);
    printf("%d\r\n", err);
    if(err) {
        printf("SD get size fail %d\r\n", err);
        return err;
    }

    char msg[256];
    snprintf(msg, sizeof(msg), "SD size:\r\n%lld MB\r\n",
        size / 1048576LL);
    displayPrintf("%s", msg);
    printf("%s\r\n", msg);
    return 0;
}

int main() {
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
    printf("\e[0m\e[2J\e[H\r\n *** SDCARD DEMO PROGRAM *** \r\n");

    initDisplay();
    printf("Init OK\r\n");
    displayPrintf("moo");
    err = initSD();
    if(err) displayPrintf("SD FAIL %d", err);
    gpioSetPinMode(10, PIN_MODE_DISABLED);
    gpioSetPinMode(11, PIN_MODE_DISABLED);
    gpioSetPinMode(12, PIN_MODE_DISABLED);
    gpioSetPinMode(14, PIN_MODE_DISABLED);
    delayMS(3000);
    ssd1306_setPower(&display, 0);
    osBootloader();
    //osReboot();

    /* int frame = 0;
    int fps = 0;
    int w = display.display.width;
    int h = display.display.height;
    char msg[64];
    while(1) {
        gpioSetPinOutput(13, 1);
        displayFillRect(&display.display, 0, 0, w, h, 0);
        displayDrawLine(&display.display, w/2, h-1,
            (frame * w) / 30, 16, 1);
        sprintf(msg, "Howdy Doody");
        displayDrawStr(&display.display, &font_8x8_1bpp, 4, 4, msg, 1);
        err = displayUpdate(&display.display);
        gpioSetPinOutput(13, 0);
        idle();
        frame = (frame + 1) % 30;
    } */

	return 0;
}
