//this file must be in micron src directory to override system's libc
#ifndef _MICRON_STRING_H_
#define _MICRON_STRING_H_

PURE MUST_CHECK   int  memcmp (const void *str1, const void *str2, size_t num);
                 void* memcpy (void *dest, const void *source, size_t num);
                 void* memset (void *ptr, int value, size_t num);
                 char* strcat (char *dst, const char *src);
PURE MUST_CHECK  char* strchr (const char *str, int chr);
PURE MUST_CHECK   int  strcmp (const char *str1, const char *str2);
                 char* strcpy (char *dst, const char *src);
                size_t strlcpy(char *dst, const char *src, size_t dstsize);
                size_t strlcat(char *dst, const char *src, size_t dstsize);
PURE MUST_CHECK size_t strlen (const char *str);
                 char* strncat(char *dst, const char *src, size_t num);
PURE MUST_CHECK   int  strncmp(const char *str1, const char *str2, size_t num);
                 char* strncpy(char *dst, const char *src, size_t num);
PURE MUST_CHECK  char* strpbrk(const char *str1, const char *str2);
PURE MUST_CHECK  char* strrchr(const char *str, int character);

#define MEMSET_ALL(dst, val) memset((void*)(dst), (val), sizeof(dst))

#endif //_MICRON_STRING_H_
