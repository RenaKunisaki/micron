extern "C" {
    #include <micron.h>
    #include "private.h"
}

SECTION(".bss") MicronFileClass *micronFileClasses[MAX_FILE_CLASSES];

int osRegisterFileClass(MicronFileClass *cls) {
    for(int i=0; i<MAX_FILE_CLASSES; i++) {
        if(!micronFileClasses[i]) {
            micronFileClasses[i] = cls;
            return i;
        }
    }
    return -ENFILE;
}

int osUnregisterFileClass(int cls) {
    micronFileClasses[cls] = NULL;
    return 0;
}

MicronFileClass* osGetFileClass(unsigned int cls) {
    if(cls >= MAX_FILE_CLASSES) return NULL;
    return micronFileClasses[cls];
}
