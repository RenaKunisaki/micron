/** Test/demo of SSD1306 / SSD1780 I2C OLED display.
 */
#include <micron.h>
#include <drivers/display/ssd1306/ssd1306.h>

uint8_t vram[(128 * 64) / 8];
SSD1306_State display;

void blink(int n) {
    while(--n) {
        digitalWrite(13, 1);
        delayMS(250);
        digitalWrite(13, 0);
        delayMS(250);
    }
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
        setPinAsOutput(13); //onboard LED
        blink(3);
        return err;
    }

    //init I2C
    err = i2cInit(0, -1);
    if(err) {
        printf("I2C init fail %d\n", err);
        blink(4);
        return err;
    }

    err = ssd1306_init(&display);
    if(err) {
        printf("Display init fail %d\n", err);
        blink(5);
        return err;
    }

    ssd1306_setPower(&display, 1);
    ssd1306_setTestMode(&display, 1);
    delayMS(500);
    ssd1306_setTestMode(&display, 0);

    int i = 0;
    while(1) {
        int w = display.display.width;
        int h = display.display.height;
        displayFillRect(&display.display, 0, 0, w, h, 0);
        displayDrawLine(&display.display, w/2, h-1,
            i % w, 16, 1);
        displayUpdate(&display.display);
        idle();
        i++;
    }

	return 0;
}
