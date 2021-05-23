/** Internals of file/stream I/O API.
 */
#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

SECTION(".bss") FILE *stdin, *stdout, *stderr;
static FILE** std_fds[] = {&stdin, &stdout, &stderr};

/** Called at shutdown to close all open files.
 */
/* __attribute__((destructor)) static void closeFileTable() {
	for(int i=0; i<MAX_FD; i++) {
		if(micronCurOpenFiles[i].cls) close(i);
	}
} */


/** These methods are documented in io.h.
 */

int close(FILE *self) {
    MicronFileClass *cls = osGetFileClass(self->fileCls);
	return cls->close(self);
}


int read(FILE *self, void *dest, size_t len) {
    MicronFileClass *cls = osGetFileClass(self->fileCls);
	size_t count = 0;
	char *destp = (char*)dest;
	while(count < len) {
		int r = cls->read(self, destp, len - count);
		if(r < 0) return r;
		count += r;
		//if(r == 0) irqWait(); //XXX use a semaphore?
		if(r == 0) cls->sync(self);
		if(destp) destp += r;
	}
	return count;
}


int write(FILE *self, const void *src, size_t len) {
    MicronFileClass *cls = osGetFileClass(self->fileCls);
	const uint8_t *srcp = (const uint8_t*)src;
	size_t count = 0;
	while(count < len) {
		int r = cls->write(self, srcp, len - count);
		if(r < 0) return r;
		count += r;
		//if(r == 0) irqWait(); //XXX use a semaphore?
		if(r == 0) cls->sync(self);
		if(srcp) srcp += r;
	}
	return count;
}

int fseek(FILE *self, long int offset, int origin) {
    MicronFileClass *cls = osGetFileClass(self->fileCls);
    return cls->seek(self, offset, origin);
}


int tryRead(FILE *self, void *dest, size_t len) {
    MicronFileClass *cls = osGetFileClass(self->fileCls);
	return cls->read(self, dest, len);
}

int tryWrite(FILE *self, const void *src, size_t len) {
    MicronFileClass *cls = osGetFileClass(self->fileCls);
	return cls->write(self, src, len);
}

int peek(FILE *self, void *dest, size_t len) {
    MicronFileClass *cls = osGetFileClass(self->fileCls);
	return cls->peek(self, dest, len);
}

int getWriteBuf(FILE *self) {
    MicronFileClass *cls = osGetFileClass(self->fileCls);
	return cls->getWriteBuf(self);
}

int sync(FILE *self) {
    MicronFileClass *cls = osGetFileClass(self->fileCls);
	return cls->sync(self);
}

int purge(FILE *self) {
    MicronFileClass *cls = osGetFileClass(self->fileCls);
	return cls->purge(self);
}


int readUntil(FILE *self, void *buf, size_t len, const char *chrs) {
    MicronFileClass *cls = osGetFileClass(self->fileCls);
    
	//create a table of which characters to stop at.
	char stop[256];
	memset(stop, 0, sizeof(stop));
	const unsigned char *ch = (const unsigned char*)chrs;
	do {
		stop[*ch] = 1;
	} while(*(ch++));

	//read into buffer until it's full, or we find one of these chars,
	//or something goes wrong.
	size_t count = 0;
	char *dst = (char*)buf;
	while(count < len-1) {
		//XXX better way to do this without reading one byte at a time or
		//reading and discarding extra bytes?
		char c=0;
		int r = cls->read(self, &c, 1);
		if(r < 0) { //read failed, r is error code
			*dst = '\0';
			return r;
		}
		else if(r > 0) { //read succeeded
			count += r;
			*(dst++) = c;
			if(stop[(unsigned char)c]) break;
		}
		//else irqWait(); //nothing was read. XXX use a semaphore?
		else cls->sync(self);
	}
	*dst = '\0';
	return count;
}


int readLine(FILE *self, void *buf, size_t len) {
	return readUntil(self, buf, len, "\n");
}

int fputs(const char *str, FILE *self) {
	return write(self, str, strlen(str));
}

//wrapper for printf() to work when gcc translates it to _write()
int _write(int fd, const void *buf, unsigned int len) {
	return write(*std_fds[fd], buf, len);
}

int puts(const char *str) {
	return fputs(str, stdout) + fputs("\n", stdout);
}

int putc(int c, FILE *f) {
	char buf[2] = {(char)c, '\0'};
	return write(f, buf, 1);
}

int putchar(int c) {
	return putc(c, stdout);
}


#ifdef __cplusplus
	} //extern "C"
#endif
