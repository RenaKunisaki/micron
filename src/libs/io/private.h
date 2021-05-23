/** Internals of file/stream I/O API.
 *  This works somewhat similarly to POSIX, but doesn't differentiate between
 *  files and streams.
 *  XXX rename this
 */
#ifndef _MICRON_IO_PRIVATE_H_
#define _MICRON_IO_PRIVATE_H_

struct FILE;

/** Pointers to methods for each file I/O driver.
 */
typedef struct {
	int (*close)      (FILE *self);
	int (*read)       (FILE *self, void *dest, size_t len);
	int (*write)      (FILE *self, const void *src, size_t len);
	int (*seek)       (FILE *self, long int offset, int origin);
	int (*peek)       (FILE *self, void *dest, size_t len);
	int (*getWriteBuf)(FILE *self);
	int (*sync)       (FILE *self);
	int (*purge)      (FILE *self);
} MicronFileClass;

#define MAX_FILE_CLASSES 8

extern MicronFileClass *micronFileClasses[MAX_FILE_CLASSES];

/** Individual file objects.
 */
typedef struct FILE {
	uint8_t fileCls; //index to micronFileClasses
    uint64_t offset; //seek position
	union { //file-specific data
		void*    ptr;
		 int8_t  i8;
		 int16_t i16;
		 int32_t i32;
		uint8_t  u8;
		uint16_t u16;
		uint32_t u32;
	} udata;
} FILE;

//standard file descriptors.
//these are initialized to NULL, but your program can set them to something
//so that printf() and puts() can work.
extern FILE *stdin, *stdout, *stderr;

//private.c
int osRegisterFileClass(MicronFileClass *cls);
int osUnregisterFileClass(int cls);
MicronFileClass* osGetFileClass(unsigned int cls);

#endif //_MICRON_IO_PRIVATE_H_
