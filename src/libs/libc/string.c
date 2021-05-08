#include <micron.h>

//reimplementations of some basic C string functions because libc is bloated
//and gcc's builtins are also bloated.
//TODO: make these faster.


PURE MUST_CHECK int memcmp(const void *str1, const void *str2, size_t num) {
	unsigned char *strA = (uint8_t*)str1;
	unsigned char *strB = (uint8_t*)str2;
	while(num --> 0) {
		unsigned char a = *strA++;
		unsigned char b = *strB++;
		if(a > b) return  1;
		if(a < b) return -1;
	}
	return 0;
}


void* memcpy(void *dest, const void *source, size_t num) {
	uint8_t *src = (uint8_t*)source;
	uint8_t *dst = (uint8_t*)dest;
	while(num --> 0) {
		*dst++ = *src++;
	}
	return dest;
}


void* memset(void *ptr, int value, size_t num) {
	unsigned char *dst = (uint8_t*)ptr;
	unsigned char  val = (unsigned char)value;
	while(num --> 0) {
		*dst++ = val;
	}
	return ptr;
}


char* strcat(char *dst, const char *src) {
	char *end = strchr(dst, 0);
	strcpy(end, src);
	return dst;
}


PURE MUST_CHECK char* strchr(char *str, int chr) {
	char c = (char)chr;
	while(1) {
		if(*str == c) return str;
		if(*str == 0) return NULL;
		str++;
	}
}


PURE MUST_CHECK int strcmp(const char *str1, const char *str2) {
	while(1) {
		char a = *str1++;
		char b = *str2++;
		if(a > b) return  1;
		if(a < b) return -1;
		if(!a)    return 0;
	}
}


char* strcpy(char *dst, const char *src) {
	char c, *ret = dst;
	do {
		c = *src++;
		*dst++ = c;
	} while(c);
	return ret;
}


size_t strlcpy(char *dst, const char *src, size_t dstsize) {
	size_t len = 0;
	char c;
	do {
		c = *src++;
		if(len++ < dstsize) *dst++ = c;
	} while(c);
	return len;
}


size_t strlcat(char *dst, const char *src, size_t dstsize) {
	size_t len = 0;
	char c;
	dst--;
	while(*++dst) len++; //find end of dst
	do {
		c = *src++;
		if(len++ < dstsize) *dst++ = c;
	} while(c);
	return len;
}


PURE MUST_CHECK size_t strlen(const char *str) {
	size_t len = 0;
	while(*str++) len++;
	return len;
}


char* strncat(char *dst, const char *src, size_t num) {
	char *end = strchr(dst, 0);
	strncpy(end, src, num);
	return dst;
}


PURE MUST_CHECK int strncmp(const char *str1, const char *str2, size_t num) {
	while(num --> 0) {
		char a = *str1++;
		char b = *str2++;
		if(a > b) return  1;
		if(a < b) return -1;
		if(!a)    return 0;
	}
	return 0;
}


char* strncpy(char *dst, const char *src, size_t num) {
	char *ret = dst;
	while(num --> 0) {
		char c = *src++;
		*dst++ = c;
		if(c == 0) break;
	}
	return ret;
}


PURE MUST_CHECK char* strpbrk(char *str1, const char *str2) {
	char c;
	str1--;
	do {
		c = *(++str1);
		const char *s = str2;
		while(*s++) {
			if(*s == c) return str1;
		}
	} while(c);
	return NULL;
}


PURE MUST_CHECK char* strrchr(char *str, int character) {
	char c = (char)character;
	char *last = NULL;
	while(1) {
		if(*str == c) last = str;
		if(*str == 0) break;
		str++;
	}
	return last;
}
