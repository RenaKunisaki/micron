#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

//static _serial_state fd_state[NUM_UART];
static FILE serial_file[NUM_UART];

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

static micronFileClass serial_class = {
	.close       = serial_close,
	.read        = serial_read,
	.write       = serial_write,
	.peek        = serial_peek,
	.getWriteBuf = serial_getWriteBuf,
	.sync        = serial_sync,
	.purge       = serial_purge,
};


FILE* openSerial(uint8_t port, int *err) {
	if(port >= NUM_UART) {
		if(err) *err = -ENODEV;  //No such device
		return NULL;
	}
	//if(!uart_state[port]) return -EUNATCH; //Protocol driver not attached
		//(i.e. port isn't set up)

	serial_file[port].cls      = &serial_class;
	serial_file[port].udata.u8 = port;
	if(err) *err = 0;
	return &serial_file[port];
}


#ifdef __cplusplus
	} //extern "C"
#endif
