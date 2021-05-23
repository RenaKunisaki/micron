#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

//static _serial_state fd_state[NUM_UART];
static FILE serial_file[NUM_UART];
static int8_t serialFileClsIdx = -1;

static int serial_close(FILE *self) {
	//nothing to do
	return 0;
}

static int serial_read(FILE *self, void *dest, size_t len) {
	return serialReceive(self->udata.u8, (char*)dest, len);
}

static int serial_write(FILE *self, const void *src, size_t len) {
	return serialSend(self->udata.u8, src, len);
}

static int serial_seek(FILE *self, long int offset, int origin) {
	return -ENOTBLK;
}

static int serial_peek(FILE *self, void *dest, size_t len) {
	//XXX TODO
	return -ENOSYS;
}

static int serial_getWriteBuf(FILE *self) {
	//XXX TODO
	return -ENOSYS;
}

static int serial_sync(FILE *self) {
	serialFlush(self->udata.u8);
	return 0;
}

static int serial_purge(FILE *self) {
	serialClear(self->udata.u8);
	return 0;
}

static MicronFileClass serial_class = {
	.close       = serial_close,
	.read        = serial_read,
	.write       = serial_write,
	.seek        = serial_seek,
	.peek        = serial_peek,
	.getWriteBuf = serial_getWriteBuf,
	.sync        = serial_sync,
	.purge       = serial_purge,
};


FILE* openSerial(uint8_t port, int *outErr) {
	if(port >= NUM_UART) {
		if(outErr) *outErr = -ENODEV;  //No such device
		return NULL;
	}
	//if(!uart_state[port]) return -EUNATCH; //Protocol driver not attached
		//(i.e. port isn't set up)

    if(serialFileClsIdx < 0) {
        int err = osRegisterFileClass(&serial_class);
        if(err < 0) {
            if(outErr) *outErr = err;
            return NULL;
        }
        serialFileClsIdx = err;
    }

	serial_file[port].fileCls  = serialFileClsIdx;
	serial_file[port].udata.u8 = port;
	if(outErr) *outErr = 0;
	return &serial_file[port];
}


#ifdef __cplusplus
	} //extern "C"
#endif
