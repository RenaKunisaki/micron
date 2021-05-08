#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

static FILE usb_file[USB_MAX_ENDPOINTS];

static int usb_close(FILE *self) {
	//nothing to do
	return 0;
}

static int usb_read(FILE *self, void *dest, size_t len) {
	//return usbReceive(self->udata.u8, (char*)dest, len);
	//XXX this would require an rx buffer.
	//current USB system is event/callback-driven; you have a
	//callback when the endpoint receives data.
	//to work with this interface, we would need to have that callback
	//set a flag indicating that it received something, and check it here.
	//we might have openUSB() set up the endpoint, and allocate some rx
	//buffers that are multiples of the packet size. then in the callback
	//we mark the buffer as full and swap out the endpoint's rxbuf to a free
	//one.
	//this polling interface for USB isn't really ideal. but it's a wrapper
	//to use the standard read/write() functions. but maybe we don't need
	//that for raw USB?
	return -ENOSYS;
}

static int usb_write(FILE *self, const void *src, size_t len) {
	return usbTrySend(src, len, self->udata.u8);
}

static int usb_peek(FILE *self, void *dest, size_t len) {
	//XXX TODO
	return -ENOSYS;
}

static int usb_getWriteBuf(FILE *self) {
	//XXX TODO
	return -ENOSYS;
}

static int usb_sync(FILE *self) {
	//usbFlush(self->udata.u8);
	return 0;
}

static int usb_purge(FILE *self) {
	//usbClear(self->udata.u8);
	return 0;
}

static micronFileClass usb_class = {
	.close       = usb_close,
	.read        = usb_read,
	.write       = usb_write,
	.peek        = usb_peek,
	.getWriteBuf = usb_getWriteBuf,
	.sync        = usb_sync,
	.purge       = usb_purge,
};


FILE* openUSB(uint8_t endp, int *err) {
	if(endp >= USB_MAX_ENDPOINTS) {
		if(err) *err = -ENODEV;  //No such device
		return NULL;
	}
	if(!usbEndpCfg[endp].config) { //endpoint isn't set up
		if(err) *err = -EUNATCH; //Protocol driver not attached
		return NULL;
	}

	usb_file[endp].cls      = &usb_class;
	usb_file[endp].udata.u8 = endp;
	if(err) *err = 0;
	return &usb_file[endp];
}


#ifdef __cplusplus
	} //extern "C"
#endif
