//File/stream I/O API.
#ifndef _MICRON_IO_H_
#define _MICRON_IO_H_

#ifdef __cplusplus
	extern "C" {
#endif

//#define MAX_FD 8 //max files that can be open at once.
#include "private.h"

/** Open a serial UART as a file. The port must have been previously configured
 *  by calling serialInit().
 *  port: Which UART to use (0, 1, 2).
 *  On success, returns a file handle.
 *  On failure, returns NULL, and sets the value pointed to by err (if it's not
 *  NULL) to a negative error code.
 */
FILE* openSerial(uint8_t port, int *err);

/** Open a USB endpoint as a file. The endpoint must have been previously
 *  configured by calling usbConfigureEndpoint().
 *  endp: Which endpoint to use.
 *  On success, returns a file handle.
 *  On failure, returns NULL, and sets the value pointed to by err (if it's not
 *  NULL) to a negative error code.
 */
FILE* openUSB(uint8_t endp, int *err);

/** Close a file.
 *  self: file to close.
 *  On success, returns zero.
 *  On failure, returns a negative error code.
 *  This frees any resources associated with the file. After this, the FILE*
 *  is no longer valid and should not be used.
 */
int close(FILE *self);

/** Read from a file.
 *  self: file to read.
 *  dest: buffer to read into.
 *  len:  max bytes to read.
 *  On success, returns number of bytes read (which may be zero).
 *  On failure, returns a negative error code.
 *  Notes:
 *   -This function blocks until the read completes or an error occurs.
 *   -dest can be NULL; in that case, it will just block until `len` bytes
 *    are available to read.
 */
int read(FILE *self, void *dest, size_t len);

/** Write to a file.
 *  self: File to write to.
 *  src:  buffer to write.
 *  len:  max bytes to write.
 *  On success, returns number of bytes written (which may be zero).
 *  On failure, returns a negative error code.
 *  Notes:
 *   -This function blocks until the write completes or an error occurs.
 *   -src can be NULL; in that case, it will just block until `len` bytes
 *    can be written without blocking.
 */
int write(FILE *self, const void *src, size_t len);

/** Read from a file without blocking.
 *  self: file to read.
 *  dest: buffer to read into.
 *  len:  max bytes to read.
 *  On success, returns number of bytes read (which may be zero).
 *  On failure, returns a negative error code.
 */
int tryRead(FILE *self, void *dest, size_t len);

/** Write to a file without blocking.
 *  self: File to write to.
 *  src:  buffer to write.
 *  len:  max bytes to write.
 *  On success, returns number of bytes written (which may be zero).
 *  On failure, returns a negative error code.
 */
int tryWrite(FILE *self, const void *src, size_t len);

/** Read from a file without blocking, but don't remove the data
 *  from the input buffers.
 *  self: file to read.
 *  dest: buffer to read into.
 *  len:  max bytes to read.
 *  On success, returns number of bytes read (which may be zero).
 *  On failure, returns a negative error code.
 *  Notes:
 *   -dest can be NULL; in that case, it will return the number of bytes
 *    available to read without blocking (which may be greater than len).
 */
int peek(FILE *self, void *dest, size_t len);

/** Check how many bytes can be written to a file without blocking.
 *  self: file to query.
 *  On success, returns number of bytes that can be written to this file
 *  without blocking (which may be zero).
 *  On failure, returns a negative error code.
 */
int getWriteBuf(FILE *self);

/** Wait for all pending I/O on a file to complete.
 *  self: file to sync.
 *  On success, returns zero.
 *  On failure, returns a negative error code.
 */
int sync(FILE *self);

/** Discard any buffered data waiting to be read or written.
 *  self: file to discard.
 *  On success, returns zero.
 *  On failure, returns a negative error code.
 */
int purge(FILE *self);

/** Read from a file until one of the specified characters.
 *  self: file to read.
 *  buf:  buffer to read into.
 *  len:  max bytes to read.
 *  chrs: characters to stop at.
 *  On success, returns number of bytes read (which may be zero).
 *  On failure, returns a negative error code.
 *
 *  This function continues reading until any of the following are true:
 *  -The byte read is present in `chrs`.
 *  -The number of bytes read equals `len`.
 *  -An error occurs during reading.
 *  Since `chrs` is a null-terminated string, this function will always stop
 *  reading when it finds a null byte. The end character is written to the
 *  buffer, followed by a null character (even if the end character was also
 *  a null character).
 */
int readUntil(FILE *self, void *buf, size_t len, const char *chrs);

/** Read from a file until line break or null character.
 *  self: File to read.
 *  dest: buffer to read into.
 *  len:  max bytes to read.
 *  On success, returns number of bytes read (which may be zero).
 *  On failure, returns a negative error code.
 *  This function includes the line break character in the buffer and adds a
 *  null terminator at the end.
 *  This function blocks until an entire line is read, the buffer is full, or
 *  an error occurs.
 */
int readLine(FILE *self, void *buf, size_t len);

/** Write a null-terminated string to a file.
 *  str:  String to write.
 *  self: File to read.
 *  Returns the number of bytes written.
 *  The silly parameter order is because this is a standard function.
 *  Blame whomever originally wrote it.
 */
int fputs(const char *str, FILE *self);

/** Write a null-terminated string, followed by a line break, to stdout.
 *  str:  String to write.
 *  Returns the number of bytes written.
 */
int puts(const char *str);

int putc(int c, FILE *f);
int putchar(int c);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_IO_H_
