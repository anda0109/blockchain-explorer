#include "common/os_log.h"
#include "common/os_ipc.h"
#include "common/os_time.h"
#include "common/os_errno.h"
#include "common/os_file.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#ifdef OS_WINDOWS
	#include <windows.h>
	#define  vsnprintf _vsnprintf
#endif


struct T_LogParam
{
	str		ms_log_file;
	int		mi_log_size;
	int     mi_log_level;
};
//使用线程局部存储，使log在每个线程中有不同的T_LogParam，即每个线程生成独立的log文件
static OsTsd	*gp_log_fsd=NULL;

void os_log_init(IN const char * _log_file, int _level, int _log_size)
{
	os_assert(NULL==gp_log_fsd);
	gp_log_fsd = new OsTsd();

	os_set_log_file(_log_file);
	os_set_log_level(_level);
	os_set_log_size(_log_size);
}

void  os_set_log_file(IN const char * _log_file)
{
	os_assert(NULL!=gp_log_fsd);
	T_LogParam *pLogParam=(T_LogParam*)gp_log_fsd->getData();
	if(NULL==pLogParam)
	{
		pLogParam = new T_LogParam;
		pLogParam->mi_log_size = 1024*1024;
		pLogParam->mi_log_level=JENC_LOG;
		gp_log_fsd->setData(pLogParam);
	}

	pLogParam->ms_log_file = _log_file;
}

void os_set_log_level(int _level)
{
	os_assert(NULL!=gp_log_fsd);
	os_assert(_level>=JENC_SYS);
	os_assert(_level<=JENC_ALARM);

	T_LogParam *pLogParam=(T_LogParam*)gp_log_fsd->getData();
	os_assert(NULL!=pLogParam);
	pLogParam->mi_log_level = _level;
}

void  os_set_log_size(int _ui32_log_size)
{
	os_assert(NULL!=gp_log_fsd);
	os_assert(_ui32_log_size>=1024);

	T_LogParam *pLogParam=(T_LogParam*)gp_log_fsd->getData();
	os_assert(NULL!=pLogParam);
	pLogParam->mi_log_size = _ui32_log_size;
}

void _os_write_log(T_LogParam *pLogParam, IN const char *_sFile, IN int _iLine, int _level, const char* levlename, IN const char *_str)
{
#ifdef _WIN32
	HANDLE   hFile;
	
	hFile=   CreateFile((const char*)pLogParam->ms_log_file, GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(INVALID_HANDLE_VALUE==hFile)
	{
		return;
	}
	DWORD dwSize   =   GetFileSize   (hFile,   NULL);
	if((int)dwSize >= pLogParam->mi_log_size)
	{
		str filebackup = pLogParam->ms_log_file;
		filebackup += ".old";
		
		CloseHandle(hFile);
		os_file_rename(pLogParam->ms_log_file, filebackup);
		hFile=   CreateFile((const char*)pLogParam->ms_log_file, GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(INVALID_HANDLE_VALUE==hFile)
		{
			return;
		}
	}
	SetFilePointer   (hFile,   0,   NULL,   FILE_END); 

	OsTime DateTime;
	char *buff = new char[4096];
	os_time_mktm(os_time_localtimet(), &DateTime);
	int len = _snprintf_s(buff, 4096-1, 4096 - 1, "%s [%s][%04d-%02d-%02d %02d:%02d:%02d %I64d][%s][%d]\r\n",
		_str,
		levlename,
		DateTime.tm_year+1900,
		DateTime.tm_mon+1,
		DateTime.tm_mday,
		DateTime.tm_hour, 
		DateTime.tm_min, 
		DateTime.tm_sec,
		os_time_tickcount(),
		_sFile,
		_iLine);
	DWORD writelen=0;
	WriteFile(hFile, buff, len, &writelen,	NULL);
	CloseHandle(hFile);
	delete [] buff;
#else
	FILE *fp = fopen((const char*)pLogParam->ms_log_file, "a");
	if (fp == NULL)
	{
		return;
	}
	fpos_t pos;
	fseek(fp, 0, SEEK_END);
	if(fgetpos(fp, &pos) == 0)
	{
#ifdef _WIN32
		if (pos > pLogParam->mi_log_size)
		{
#elif !defined(__linux__) 
		if (pos > pLogParam->mi_log_size)
		{
#else 
		if (pos.__pos > (__off_t)pLogParam->mi_log_size)
		{
#endif	
			str filebackup = pLogParam->ms_log_file;
			filebackup += ".old";
			
			fclose(fp);
			os_file_rename(pLogParam->ms_log_file, filebackup);
			fp = fopen((const char*)pLogParam->ms_log_file, "w");
		}
	}
	if(fp==NULL)
	{
		//int err=os_errno();
		//const char *errstr=os_errstr();
		return;
	}
	OsTime DateTime;
	os_time_mktm(os_time_localtimet(), &DateTime);
	
	//int len = 
		fprintf(fp, "%s [%s][%04d-%02d-%02d %02d:%02d:%02d %lld][%s][%d]\n",
		_str,
		levlename,
		DateTime.tm_year+1900,
		DateTime.tm_mon+1,
		DateTime.tm_mday,
		DateTime.tm_hour, 
		DateTime.tm_min, 
		DateTime.tm_sec,
		os_time_tickcount(),
		_sFile,
		_iLine);
	fclose(fp);
#endif
}
		
void os_sys(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...)
{
	os_assert(NULL!=gp_log_fsd);
	va_list args;
	T_LogParam *pLogParam=(T_LogParam*)gp_log_fsd->getData();
	if(NULL==pLogParam)
	{
		return;		
	}
	if(pLogParam->mi_log_level > JENC_SYS)
		return;
	
	char *buff = new char[2048];
	va_start(args, fmt);
	vsnprintf(buff, 2048-1,fmt, args);
	va_end(args);
	_os_write_log(pLogParam, _sFile, _iLine, JENC_SYS, "SYS", buff);
	delete[] buff;

}

void os_debug(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...)
{
	os_assert(NULL!=gp_log_fsd);
	va_list args;
	T_LogParam *pLogParam=(T_LogParam*)gp_log_fsd->getData();
	if(NULL==pLogParam)
	{
		return;		
	}
	if(pLogParam->mi_log_level > JENC_DEBUG)
		return;
	
	char *buff = new char[2048];
	va_start(args, fmt);
	vsnprintf(buff, 2048-1, fmt, args);
	va_end(args);
	_os_write_log(pLogParam, _sFile, _iLine, JENC_DEBUG, "DEBUG", buff);
	delete[] buff;
	
}

void os_trace(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...)
{
	os_assert(NULL!=gp_log_fsd);
	va_list args;
	T_LogParam *pLogParam=(T_LogParam*)gp_log_fsd->getData();
	if(NULL==pLogParam)
	{
		return;		
	}
	if(pLogParam->mi_log_level > JENC_TRACE)
		return;

	char *buff = new char[2048];
	va_start(args, fmt);
	vsnprintf(buff, 2048-1,  fmt, args);
	va_end(args);
	_os_write_log(pLogParam, _sFile, _iLine, JENC_TRACE, "TRACE", buff);
	delete[] buff;
}

void os_error(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...)
{
	os_assert(NULL!=gp_log_fsd);
	va_list args;
	T_LogParam *pLogParam=(T_LogParam*)gp_log_fsd->getData();
	if(NULL==pLogParam)
	{
		return;		
	}
	if(pLogParam->mi_log_level > JENC_ERROR)
		return;
	
	char *buff = new char[2048];
	va_start(args, fmt);
	vsnprintf(buff, 2048-1,  fmt, args);
	va_end(args);
	_os_write_log(pLogParam, _sFile, _iLine, JENC_ERROR, "ERROR", buff);
	delete[] buff;
}


void os_log(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...)
{
	os_assert(NULL!=gp_log_fsd);
	va_list args;
	T_LogParam *pLogParam=(T_LogParam*)gp_log_fsd->getData();
	if(NULL==pLogParam)
	{
		return;		
	}
	if(pLogParam->mi_log_level > JENC_LOG)
		return;
	
	char *buff = new char[2048];
	va_start(args, fmt);
	vsnprintf(buff, 2048-1, fmt, args);
	va_end(args);
	_os_write_log(pLogParam, _sFile, _iLine, JENC_LOG, "LOG", buff);
	delete[] buff;
}


void os_alarm(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...)
{
	os_assert(NULL!=gp_log_fsd);
	va_list args;
	T_LogParam *pLogParam=(T_LogParam*)gp_log_fsd->getData();
	if(NULL==pLogParam)
	{
		return;		
	}
	if(pLogParam->mi_log_level > JENC_ALARM)
		return;
	
	char *buff = new char[2048];
	va_start(args, fmt);
	vsnprintf(buff, 2048-1, fmt, args);
	va_end(args);
	_os_write_log(pLogParam, _sFile, _iLine, JENC_ALARM, "ALARM", buff);
	delete[] buff;
}

void os_prompt(const char *fmt, ...)
{
	os_assert(NULL!=gp_log_fsd);
	va_list args;
	char buff[2048];
	
	va_start(args, fmt);
	vsprintf(buff, fmt, args);
	va_end(args);

#ifdef OS_WINDOWS
	MessageBox(NULL, buff, "bkc", MB_OK);
#else
	printf("%s\n", buff);
#endif
}
