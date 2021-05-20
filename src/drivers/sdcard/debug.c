extern "C" {
    #include <micron.h>
    #include "sdcard.h"
}

static const char *respBits[] = {
    "Idle", //bit 7
    "ErRst",
    "BadCmd",
    "CrcErr",
    "ErSqErr",
    "AddrErr",
    "ParamErr",
    "Zero", //bit 0
};

void _sdPrintStatus(uint8_t stat) {
    for(int i=7; i>=0; i--) {
        printf(" \x1B[38;5;%dm%s",
            stat & BIT(i) ? 9 : 15,
            respBits[i]);
    }
    printf("\x1B[0m\r\n");
}
