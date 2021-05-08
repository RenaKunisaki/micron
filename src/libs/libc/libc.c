//Defines some shims to get libc functions working (if you use them)
//and to allow basic C++ to work without the 80KB standard library.
#include <micron.h>
#include <sys/stat.h>

#ifdef __cplusplus
	extern "C" {
#endif


//char *__brkval = (char *)&_ebss;

void * _sbrk(int incr) {
	char *prev = __brkval;
	__brkval += incr;
	return prev;
}

WEAK int _read(int file, char *ptr, int len) {
	return 0;
}

WEAK int _close(int fd) {
	return -ENOSYS;
}

WEAK int _fstat(int fd, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

WEAK int _isatty(int fd) {
	return 1;
}

WEAK int _lseek(int fd, long long offset, int whence) {
	return -ENOSYS;
}

WEAK void _exit(int status) {
	while (1);
}

WEAK void __cxa_pure_virtual() {
	while (1);
}

WEAK int __cxa_guard_acquire (char *g) {
	return !(*g);
}

WEAK void __cxa_guard_release(char *g) {
	*g = 1;
}


//these are needed when using -funwind-tables
WEAK int _kill(int pid, int sig) {
	return -ENOSYS;
}

WEAK int _getpid() { return 1; }


#ifdef __cplusplus
	} //extern "C"

	void* operator new(unsigned int n) {
		return malloc(n);
	}

	void operator delete(void* p) {
		free(p);
	}

	extern "C" int _purecall(void) {
		return 0;
	}
#endif
