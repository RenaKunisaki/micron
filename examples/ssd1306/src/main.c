/** Test/demo of SSD1306 / SSD1780 I2C OLED display.
 */
#include <micron.h>
#include <drivers/display/ssd1306/ssd1306.h>
#include <drivers/display/fonts/8x8_1bpp.h>

uint8_t vram[(128 * 64) / 8];
SSD1306_State display;

void blink(int n) {
    setPinAsOutput(13); //onboard LED
    while(n--) {
        digitalWrite(13, 1);
        delayMS(250);
        digitalWrite(13, 0);
        delayMS(250);
    }
}

void fail(int code) {
    blink(code);
    osBootloader();
}

int main() {
    display.i2cPort = 0;
    display.i2cAddr = 0x3C;
    display.display.vram = vram;
    display.display.width = 128;
    display.display.height = 64;

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

    while(1) {
        //init I2C and display
        err = i2cInit(0, -1);
        if(err) {
            printf("I2C init fail %d\r\n", err);
            blink(4);
        }
        else err = ssd1306_init(&display);
        if(err) {
            printf("Display init fail %d\r\n", err);
            blink(5);
        }
        else break;

        i2cShutdown(0);
        delayMS(1000);
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
        displayDrawStr(&display.display, &font_8x8_1bpp, 4, 4, msg, 1);
        displayUpdate(&display.display);
        idle();
        frame++;
        if(frame >= 30) frame = 0;
        uint32_t tEnd = millis();
        fps = 1000 / (tEnd - tStart);
        tStart = tEnd;
    }

	return 0;
}
