#ifndef _PLAT_OS_LIBC_H_
#define _PLAT_OS_LIBC_H_

#include "os_type.h"

PLATAPI int osc_rand();

PLATAPI void osc_memset(void *m, int val, size_t len);

PLATAPI void osc_memcpy(void *dest, const void *src, int count);

PLATAPI int  osc_strlen(const char *s);
PLATAPI int osc_strcmp( const char *string1, const char *string2);
PLATAPI char *osc_strstr( const char *string, const char *_substr);
PLATAPI int osc_strncmp(const char* s1, const char* s2, int maxlen);
PLATAPI int osc_strnicmp(const char* s1, const char* s2, int maxlen);

PLATAPI void osc_snprintf( char *buffer, size_t count, size_t max_count, const char *format, ... );
PLATAPI int osc_snprintf_r( char *buffer, size_t count, size_t max_count, const char *format, ... );

/*typedef char *  va_list;
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )
PLATAPI int os_vsnprintf(char *buffer, size_t count, const char *format, va_list argptr);
*/
PLATAPI void osc_strncpy(char*dst, int dst_size, const char*src, int _strlen);


PLATAPI int osc_atoi( const char *string );

//PLATAPI char*osc_strdup(const char* _str);

PLATAPI void *osc_malloc(size_t _size);
PLATAPI void osc_free(void *rawMemory);
PLATAPI const char * osc_strchr(const char *string, char c);
PLATAPI int osc_strtol( const char *nptr, char **endptr, int base );

str osc_print_uint64(uint64 number);

#endif
