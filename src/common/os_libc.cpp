#include "common/os_libc.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef _WIN32
#define  vsnprintf _vsnprintf
#define snprintf	_snprintf
#endif


int osc_rand()
{
	return rand();
}

void osc_memset(void *m, int val, size_t len)
{
	os_assert(m!=NULL);
	os_assert(len>0);

	memset(m, val, len);
}

void osc_memcpy(void *dest, const void *src, int count)
{
	os_assert(dest != NULL);
	os_assert(src!=NULL);
	os_assert(count>=0);

	if(count==0)
		return;
	if(dest == src)
		return;
	memcpy(dest, src, (size_t)count);
}

int osc_strlen(const char *s)
{
	if(NULL==s)
		return 0;
	
	int len=0;
	const char *pos = s;
	while(*pos!='\0')
	{
		++len;
		++pos;
	}
	return len;
}


int osc_strcmp( const char *string1, const char *string2)
{
	if(string1 == string2)
	{
		return 0;
	}
	else
	{
		if(NULL==string1)
			return -1;
		else if(NULL==string2)
			return 1;
		else
			return strcmp(string1, string2);
	}
}


char *osc_strstr( const char *string, const char *_substr )
{
	if(NULL == string)
		return NULL;
	if(NULL == _substr)
		return NULL;
	if(string == _substr)
		return 0;
	return (char*)strstr(string, _substr);
}

int osc_strncmp(const char* s1, const char* s2, int maxlen)
{
	os_assert(s1!=NULL);
	os_assert(s2!=NULL);
	os_assert(maxlen>0);

	return strncmp(s1, s2, maxlen);
}

int osc_strnicmp(const char* s1, const char* s2, int maxlen)
{
	os_assert(s1!=NULL);
	os_assert(s2!=NULL);
	os_assert(maxlen>0);

#ifdef OS_WIN
	return _strnicmp(s1, s2, maxlen);
#else
	return strncasecmp(s1, s2, maxlen);
#endif
}

void osc_strncpy(char*dst, int dst_size, const char*src, int _strlen)
{
	os_assert(dst!=NULL);
	os_assert(src!=NULL);
	strncpy(dst,  src, _strlen);
}

/*
int os_snprintf( char *buffer, size_t count, const char *format, ... )
{
	va_list args;

	va_start(args, format);
	int len = os_vsnprintf(buffer, count, format, args);
	va_end(args);
	return len;
}


int os_vsnprintf(char *buffer, size_t count, const char *format, va_list argptr)
{
	os_assert(count>0);
	return vsnprintf(buffer, count, format, argptr);
}
*/
void osc_snprintf( char *buffer, size_t count, size_t max_count ,const char *format, ... )
{
	va_list args;
	
	va_start(args, format);
	int len = vsnprintf(buffer, count, format, args);
	va_end(args);

	os_assert(len>0);

	return;
}

int osc_snprintf_r( char *buffer, size_t count, size_t max_count, const char *format, ... )
{
	va_list args;
	
	va_start(args, format);
	int len = vsnprintf(buffer, count, format, args);
	va_end(args);
	return len;
}

int osc_atoi( const char *string )
{
	if(string == NULL)
		return 0;

	return atoi(string);
}


char*  osc_strdup(const char* _str)
{
	int len=osc_strlen(_str);
	char *p= new char[len+1];
	osc_memcpy(p, _str, len+1);
	return p;
}

void *osc_malloc(size_t _size)
{
	void *p=malloc(_size);
	os_assert(p!=NULL);
	return p;
}

void osc_free(void *rawMemory)
{
	free(rawMemory);
}

const char * osc_strchr(const char *string, char c)
{
	os_assert(string !=NULL);
	return strchr(string, c);
}

int osc_strtol(const char *nptr, char **endptr, int base)
{
	os_assert(nptr !=NULL);
	return strtol(nptr, endptr, base);
}

//解决不同平台uint64输出的格式化问题
str osc_print_uint64(uint64 number)
{
	str str_number;
#ifdef OS_WIN
	str_number.nfmt(20, "%I64d", number);
#else
	str_number.nfmt(20, "%lld", number);
#endif // OS_WIN
	return str_number;	
}
