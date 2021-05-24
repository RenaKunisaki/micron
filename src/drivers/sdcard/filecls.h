#ifndef _MICRON_DRIVERS_SDCARD_FILECLS_H_
#define _MICRON_DRIVERS_SDCARD_FILECLS_H_

extern int8_t sdFileClsIdx;
FILE* sdOpenCard(MicronSdCardState *state, int *outErr);

#endif //_MICRON_DRIVERS_SDCARD_FILECLS_H_
