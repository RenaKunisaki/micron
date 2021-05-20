/** Test/demo of MCP23017 16-bit I2C I/O Expander.
 *  Port A will cycle through bits, while port B is used as an input
 *  whose state is shown over UART.
 *  If you connect LEDs to each port A pin you should see them "chasing".
 */
#include <micron.h>
#include <drivers/mcp23017/mcp23017.h>

//which I2C port to use
static const int mcpPort = 0;

//I2C address of MCP chip
//low 3 bits can be changed using the A0, A1, A2 pins
static const int mcpAddr = 0x20;

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

int main() {
    gpioSetPinMode(13, PIN_MODE_OUTPUT); //onboard LED
    blink(1);

    //init serial
    int err = 0;
    serialInit(0, 460800);
	stdout = openSerial(0, &err);
	stderr = stdout;
    if(err) fail(3);
    printf("\e[0m\e[2J\e[H\r\n *** MCP23017 DEMO PROGRAM *** \r\n");

    err = i2cInit(mcpPort, -1);
    if(err) {
        printf("I2C init fail %d\r\n", err);
        fail(4);
    }
    printf("Init OK\r\n");

    //port A: all output (0); port B: all input (1)
    mcp23017SetDirection(mcpPort, mcpAddr, 0xFF00);
    mcp23017SetPullup(mcpPort, mcpAddr, 0xFF00); //pullup on all port B pins

    uint16_t lastState = 0;
    int count = 0;
    while(true) {
        delayMS(100);
        int state = mcp23017Read(mcpPort, mcpAddr, 100);
        if(state >= 0) {
            lastState = state;
            err = 0;
        }
        else {
            err = state;
            //printf("ERROR %d  \r\n", err);
            //osReboot();
        }

        printf(
            "\rPortA: %c%c%c%c %c%c%c%c PortB: %c%c%c%c %c%c%c%c Err: %d     ",
            ((lastState >>  0) & 1) + '0',
            ((lastState >>  1) & 1) + '0',
            ((lastState >>  2) & 1) + '0',
            ((lastState >>  3) & 1) + '0',
            ((lastState >>  4) & 1) + '0',
            ((lastState >>  5) & 1) + '0',
            ((lastState >>  6) & 1) + '0',
            ((lastState >>  7) & 1) + '0',
            ((lastState >>  8) & 1) + '0',
            ((lastState >>  9) & 1) + '0',
            ((lastState >> 10) & 1) + '0',
            ((lastState >> 11) & 1) + '0',
            ((lastState >> 12) & 1) + '0',
            ((lastState >> 13) & 1) + '0',
            ((lastState >> 14) & 1) + '0',
            ((lastState >> 15) & 1) + '0',
            err);

        mcp23017Write(mcpPort, mcpAddr, 1 << count);
        count++;
        if(count >= 8) count = 0;
    }

	return 0;
}
