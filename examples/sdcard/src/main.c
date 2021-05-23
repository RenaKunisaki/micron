/** Test/demo of SD card driver.
 */
#include <micron.h>
#include <drivers/sdcard/sdcard.h>

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

int resetSD() {
    printf("SD reset... ");
    int err = sdcardReset(&sdcard, 5000);
    printf("%d\r\n", err);
    if(err) return err;

    printf("SD get info... ");
    err = sdReadInfo(&sdcard, 1000);
    printf("%d\r\n", err);
    if(err) {
        printf("SD get info fail %d\r\n", err);
        return err;
    }

    printf("SD size: %lld MB\r\n", sdcard.cardSize / 1048576LL);
    return 0;
}

int initSD() {
    //XXX move to driver
    sdcard.port = 0;
    sdcard.pinCS = 10;

    //XXX shouldn't init SPI for us unless it isn't already inited
    printf("SD init... ");
    int err = sdcardInit(&sdcard);
    printf("%d\r\n", err);
    if(err) return err;

    return resetSD();
}

int init() {
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

    uint64_t val = 0;
    osGetClockSpeed(MICRON_CLOCK_CPU, &val);
    printf("CPU: %lld MHz  ", val / 1000000LL);
    osGetClockSpeed(MICRON_CLOCK_BUS, &val);
    printf("Bus: %lld MHz  ", val / 1000000LL);
    osGetMemorySize(MICRON_MEM_MAIN_RAM, &val);
    printf("RAM: %lld KB  ", val / 1024LL);
    osGetMemorySize(MICRON_MEM_MAIN_ROM, &val);
    printf("ROM: %lld KB\r\n", val / 1024LL);

    printf("Starting...\r\n");
    err = initSD();
    printf("SD init: %d\r\n", err);
    return err;
}

void hexdump(const uint8_t *data, uint32_t len) {
    printf("\r\n");
    for(uint32_t i=0; i<len; i += 16) {
        printf("%04X", i);
        for(uint32_t j=0; j<16; j++) {
            printf("%s%02X", (j&3) ? " " : "  ", data[i+j]);
        }
        printf(" ");
        for(uint32_t j=0; j<16; j++) {
            uint8_t c = data[i+j];
            printf("%s%c", (j&3) ? "" : " ",
                (c >= 0x20 && c <= 0x7E) ? c : '.');
        }
        printf("\r\n");
    }
}

void shutdown(const char *param) {
    printf("Shutting down...\r\n");
    gpioSetPinMode(10, PIN_MODE_DISABLED);
    gpioSetPinMode(11, PIN_MODE_DISABLED);
    gpioSetPinMode(12, PIN_MODE_DISABLED);
    gpioSetPinMode(14, PIN_MODE_DISABLED);
    delayMS(3000);
    osBootloader();
    //osReboot();
}

void cmd_readSector(const char *param) {
    uint32_t sector = strtoul(param, (char**)&param, 0);
    uint8_t buf[512]; //XXX check card sector size
    int err = sdReadBlock(&sdcard, sector, buf, 10000);
    if(err) printf("Error %d\r\n", err);
    else hexdump(buf, 512);
}

void cmd_setBaud(const char *param) {
    uint32_t baud = strtoul(param, (char**)&param, 0);
    if(!baud) {
        printf("Invalid entry\r\n");
        return;
    }
    int err = spiSetSpeed(sdcard.port, baud);
    if(err) printf("Error %d\r\n");
    else printf("OK\r\n");
}

void cmd_speedTest(const char *param) {
    static const char *units = " KMG";

    //read sector at low speed
    printf("Read sector 0 at low speed (wait!)...\r\n");
    uint8_t buf[512]; //XXX check card sector size
    gpioSetPinOutput(13, 1);
    int err = sdReadBlock(&sdcard, 0, buf, 20000);
    gpioSetPinOutput(13, 0);
    if(err) {
        printf("Error %d\r\n", err);
        return;
    }
    bool ok = false;
    for(int i=1; i<sizeof(buf); i++) {
        if(buf[i-1] != buf[i]) {
            ok = true;
            break;
        }
    }
    hexdump(buf, sizeof(buf));
    if(!ok) printf("Warning: sector 0 is empty; results may be inaccurate\r\n");

    uint32_t baud = 1000;
    uint8_t buf2[512];
    while(baud < 2000000000) {
        baud *= 2;

        uint32_t val=baud, unit=0;
        while(val > 9999 && units[unit]) {
            val /= 1000;
            unit++;
        }

        printf("Try %9d Hz (%4d %cHz)... ", baud, val, units[unit]);
        int err = spiSetSpeed(sdcard.port, baud);
        if(err) { printf("spiSetSpeed error %d\r\n"); break; }

        for(int tries=0; tries<3; tries++) {
            //resetSD();
            //delayMS(250);

            gpioSetPinOutput(13, 1);
            memset(buf2, 0xEE, sizeof(buf2));
            uint32_t time = millis();
            err = sdReadBlock(&sdcard, 0, buf2, 10000);
            time = millis() - time;
            gpioSetPinOutput(13, 0);
            if(err) {
                printf("sdReadBlock error %d\r\n", err);
            }
            else {
                ok = true;
                for(int i=0; i<512; i++) {
                    if(buf[i] != buf2[i]) {
                        printf("Data corruption at byte %d (0x%02X, should be 0x%02X)\r\n",
                            i, buf2[i], buf[i]);
                        ok = false;
                        break;
                    }
                }
                if(ok) {
                    printf("OK, %6dms, %d B/s\r\n", time, time / 512000);
                    break;
                }
            }
        }
        if(!ok) break;
    }

    baud /= 2;
    uint32_t val=baud, unit=0;
    while(val > 9999 && units[unit]) {
        val /= 1000;
        unit++;
    }
    printf("Fastest available speed for this card: %d %cHz (%d Hz)\r\n",
        val, units[unit], baud);
    spiSetSpeed(sdcard.port, baud);
}

void cmd_reset(const char *param) {
    resetSD();
}

static struct {
	const char *cmd;
	void(*func)(const char*);
} commands[] = {
	{"shutdown",  shutdown},
	{"read",      cmd_readSector},
	{"baud",      cmd_setBaud},
    {"speedtest", cmd_speedTest},
    {"reset",     cmd_reset},
	{NULL, NULL}
};

const char* strcmpCmd(const char *s1, const char *s2) {
	for(size_t j=0; j < 1024; j++) {
		char c = s1[j];
		if(c == ' ') c = '\0';
		if(c != s2[j]) return NULL;
		if(c == '\0') return &s1[j];
	}
	return NULL;
}

void doCommand(const char *buf) {
    //printf("cmd = \"%s\"\r\n", buf);
	for(int i=0; commands[i].cmd != NULL; i++) {
		const char *param = strcmpCmd(buf, commands[i].cmd);
		if(param) {
			commands[i].func(param);
			return;
		}
	}
	printf("Unknown command \"%s\"\r\n", buf);
}

int main() {
    init();

    char cmd[1024];
    memset(cmd, 0, sizeof(cmd));

    int cmdPos = 0;
    bool redraw = true;
    while(1) {
        idle();
        if(redraw) {
            //clear entire line
            printf("\r\x1B[2K>: %s", cmd);
            sync(stdout);
            redraw = false;
        }
        int chr = serialGetchr(0);
        if(chr <= 0) continue;
        switch(chr) {
            case '\r': case '\n': //enter
                printf("\r\n");
                if(cmdPos) doCommand(cmd);
                cmdPos = 0;
                cmd[cmdPos] = 0;
                redraw = true;
                break;

            case '\b': case '\x7F': //backspace
                if(cmdPos) cmd[--cmdPos] = 0;
                redraw = true;
                break;

            default:
                if(cmdPos+1 < sizeof(cmd)) {
                    cmd[cmdPos++] = chr;
                    cmd[cmdPos] = 0;
                }
                redraw = true;
        }
    }

    shutdown(NULL);
    return 0;
}