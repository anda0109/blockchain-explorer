#include "common/os_file.h"

#ifdef OS_WINDOWS
	#include <direct.h>
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <unistd.h>
//	#include <sys/sendfile.h>
#endif

#include "common/os_type.h"
#include "common/os_dir.h"
#include "common/os_libc.h"
//#include "common/os_ipc.h"
#include "common/os_errno.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

int os_file_lock(const str & _filename)
{
#ifdef OS_WINDOWS
	HANDLE   hFile;

	hFile=   CreateFile((const char *)_filename, GENERIC_READ   |   GENERIC_WRITE, 
		FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(INVALID_HANDLE_VALUE==hFile)
	{
		return -1;
	}
	return (int)hFile;
#else
    int file = -1;

    //printf("lockfile=[%s]\n", (const char*)_filename);

    file = open((const char *)_filename, O_RDWR);
    if (file < 0)
    {
       printf("open lockfile fail  [%s]\n", (const char*)_filename);
       printf("ceate lock file[%s]\n", (const char*)_filename);
       file = open((const char *)_filename, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR);
       if(file>=0)
         {
          close(file);
       }
	   else
	   {
		   printf("create lock file fail %d/%s\n", os_errno(), os_errstr());
	   }
       file = open((const char *)_filename, O_RDWR);
   }
    if (file < 0)
    {
       printf("open2 lockfail error %d/%s\n", os_errno(), os_errstr());
       return -1;
    }

    if (lockf(file, F_TLOCK, 0) < 0)
    {
       	printf("lockf fail: %d/%s\n", os_errno(), os_errstr());
        close(file);
        return -1;
    }
    return file;
#endif
}

void os_file_unlock(int _lockf_handle)
{
#ifdef OS_WINDOWS
	HANDLE   hFile =(HANDLE)_lockf_handle;
	CloseHandle(hFile);
#else
    if (lockf(_lockf_handle, F_ULOCK, 0) < 0)
	{
		abort();
		return;
    }
	close(_lockf_handle);
#endif
}

int os_file_copy(const str & _sFrom, const str & _sTo)
{
#ifdef OS_WINDOWS
    int ret = CopyFile((const char*)_sFrom, (const char*)_sTo, FALSE);
    if(ret==0)
        return -1;
    else
        return 0;
#else
    char buf[1024];
    snprintf(buf, sizeof(buf)-1, "cp -f %s %s", (const char*)_sFrom, (const char*)_sTo);
    return system(buf);
#endif
}

int os_file_rename(const str & _sFrom, const str & _sTo)
{
#ifdef _WIN32
	DeleteFile(_sTo);
    int ret = MoveFile((const char*)_sFrom, (const char*)_sTo);
    if(ret==0)
        return -1;
    else
        return 0;
#else
    char buf[1024];
    snprintf(buf, sizeof(buf)-1, "mv -f %s %s", (const char*)_sFrom, (const char*)_sTo);
    return system(buf);
#endif
}


OsFile::OsFile()
{
#ifdef OS_WIN
	mh_file = INVALID_HANDLE_VALUE;
#else
	mh_file=-1;
#endif
}


OsFile::~OsFile()
{
#ifdef OS_WIN
	if(mh_file !=INVALID_HANDLE_VALUE)
		fclose();
#else
	if(mh_file !=-1)
		fclose();
#endif

}


int OsFile::open_file_read_r(const str & _filename)
{
#ifdef OS_WIN
	HANDLE hFile; 
	hFile = CreateFile((const char*)_filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	mh_file = hFile;
	if(INVALID_HANDLE_VALUE==hFile)
		return -1;
	else
		return 0;
#else
	mh_file = open((const char*)_filename, O_RDONLY);
	if(mh_file<0)
		return -1;
	else
		return 0;
#endif
}


int OsFile::open_file_write_r(const str & _filename)
{
#ifdef OS_WIN
	HANDLE hFile; 
	hFile = CreateFile(_filename,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	mh_file = hFile;
	if(INVALID_HANDLE_VALUE==hFile)
		return -1;
	else
		return 0;
#elif defined(OS_LINUX)
	mh_file = open((const char*)_filename, O_RDWR|O_CREAT,0777);
	if(mh_file<0)
		return -1;
	else
		return 0;
#elif defined(OS_AIX)
	mh_file = open((const char*)_filename, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IXOTH);
	if(mh_file<0)
		return -1;
	else
		return 0;
#else
#error Unsupported platform
#endif
}



int OsFile::open_read_excl_r(const str & _filename)
{
#ifdef OS_WIN
	HANDLE hFile; 
	hFile = CreateFile(_filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	mh_file = hFile;
	if(INVALID_HANDLE_VALUE==hFile)
		return -1;
	else
		return 0;
#else
	mh_file = open((const char*)_filename, O_RDONLY|O_EXCL,0777);
	if(mh_file<0)
		return -1;
	else
		return 0;
#endif
}


int OsFile::create_write_excl(const str & _filename)
{
#ifdef OS_WIN
	HANDLE hFile; 
	hFile = CreateFile(_filename,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	mh_file = hFile;
	if(INVALID_HANDLE_VALUE==hFile)
		return -1;
	else
		return 0;
#elif defined(OS_LINUX)
	mh_file = open((const char*)_filename, O_WRONLY|O_CREAT|O_EXCL,0777);
	if(mh_file<0)
		return -1;
	else
		return 0;
#elif defined(OS_AIX)	
	mh_file = open((const char*)_filename, O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IXOTH);
	if(mh_file<0)
		return -1;
	else
		return 0;
#else
#error Unsupported platform
#endif
}


#ifdef OS_WIN
__int64 _myFileSeek (HANDLE hf, __int64 distance, DWORD MoveMethod)
{
	LARGE_INTEGER li;
	
	li.QuadPart = distance;
	
	li.LowPart = SetFilePointer (hf, li.LowPart, &li.HighPart, MoveMethod);
	
	if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR)
	{
		li.QuadPart = -1;
	}
	
	return li.QuadPart;
}
#endif

longint OsFile::seek_set(longint _origin)
{
#ifdef OS_WIN
	return _myFileSeek((HANDLE)mh_file, _origin, SEEK_SET);
#else
	return lseek(mh_file, _origin, SEEK_SET);
#endif
}

longint OsFile::seek_cur(longint _origin)
{
#ifdef OS_WIN
	return _myFileSeek((HANDLE)mh_file, _origin, SEEK_CUR);
#else
	return lseek(mh_file, _origin, SEEK_CUR);
#endif
}

longint OsFile::seek_end(longint _origin)
{
#ifdef OS_WIN
	return _myFileSeek((HANDLE)mh_file, _origin, SEEK_END);
#else
	return lseek(mh_file, _origin, SEEK_END);
#endif
}

// 读写
int OsFile::fread_r(OUT void * obuf, IN int _buflen)
{
#ifdef OS_WIN
	DWORD len;
	BOOL ret = ReadFile((HANDLE)mh_file, obuf, _buflen, &len, NULL);
	if(ret==FALSE)
		return -1;
	else
		return len;
#else
   return (int)read(mh_file, (void *)obuf, (size_t)_buflen);	
#endif

}

void OsFile::fwrite(const void * buf, int _buflen)
{
#ifdef OS_WIN
	DWORD len=0;
	BOOL ret = WriteFile((HANDLE)mh_file, (void*)buf, _buflen, &len, NULL);
	
	if(ret==FALSE)
		os_assert(false);
	
	if(len==0)
		os_assert(false);
#else
	ssize_t ret = write(mh_file, (const void *)buf, (size_t)_buflen);
	os_assert((int)ret==_buflen);
#endif

}

#ifdef OS_WIN
#define vsnprintf _vsnprintf
#endif
void OsFile::fprintf(int _maxsize, const char*_fmt, ...)
{
	va_list args;
	int ret=-1;
	
	a<char>	buf(_maxsize, _maxsize);
	
	va_start(args, _fmt);
	//while(1)
	{
		ret = vsnprintf(buf.get(), buf.bufsize()-1, _fmt, args);
		os_assert(ret>0);
		/*if(ret<0)
		{
			buf.newbuf(buf.bufsize()*2);
		}
		else
		{
			break;
		}*/
	}
    va_end(args);
	
	fwrite(buf.get(), ret);
}

// 格式化写
void OsFile::flush()
{
#ifdef OS_WIN
	FlushFileBuffers((HANDLE)mh_file);
#else
	fsync(mh_file);
#endif
}

void OsFile::stat(OUT int *osize, OUT int *omtime)
{
#ifdef OS_WIN
	BY_HANDLE_FILE_INFORMATION fileinfo;
	BOOL ret= GetFileInformationByHandle(
		(HANDLE)mh_file,                                  // handle to file 
		&fileinfo // buffer
		);
	os_assert(ret==TRUE);
	
	if(osize)
	{
		
		if(fileinfo.nFileSizeHigh==0)
			*osize = fileinfo.nFileSizeLow;
		else
			*osize = 0;
		/*LARGE_INTEGER fsize;
		fsize.LowPart = fileinfo.nFileSizeLow;
		fsize.HighPart = fileinfo.nFileSizeHigh;
		*osize = fsize.QuadPart;
		*/
	}
	if(omtime)
	{
		LARGE_INTEGER mtime;
		mtime.LowPart = fileinfo.ftLastWriteTime.dwLowDateTime;
		mtime.HighPart = fileinfo.ftLastWriteTime.dwHighDateTime;
		//*omtime = (longint)(mtime.QuadPart / 10000 / 1000  - 11644473600);
		*omtime = (int)(mtime.QuadPart / 10000 / 1000  - 11644473600);
	}
#else
	struct    stat    file_stat;
	int ret=fstat(mh_file, &file_stat);
	os_assert(ret==0);

	if(osize)
		*osize = (int)file_stat.st_size;
	if(omtime)
		*omtime = (int)file_stat.st_mtime;

#endif
}

void OsFile::stat64(OUT longint *osize, OUT longint *omtime)
{
#ifdef OS_WIN
	BY_HANDLE_FILE_INFORMATION fileinfo;
	BOOL ret= GetFileInformationByHandle(
		(HANDLE)mh_file,                                  // handle to file 
		&fileinfo // buffer
		);
	os_assert(ret==TRUE);
	
	if(osize)
	{
		LARGE_INTEGER fsize;
		fsize.LowPart = fileinfo.nFileSizeLow;
		fsize.HighPart = fileinfo.nFileSizeHigh;
		*osize = fsize.QuadPart;
	}
	
	if(omtime)
	{
		LARGE_INTEGER mtime;
		mtime.LowPart = fileinfo.ftLastWriteTime.dwLowDateTime;
		mtime.HighPart = fileinfo.ftLastWriteTime.dwHighDateTime;
		*omtime = (longint)(mtime.QuadPart / 10000 / 1000  - 11644473600);
	}
#else
	struct    stat    file_stat;
	int ret=fstat(mh_file, &file_stat);
	os_assert(ret==0);
	
	if(osize)
		*osize = file_stat.st_size;
	if(omtime)
		*omtime = file_stat.st_mtime;
#endif
}

void OsFile::fclose()
{
#ifdef OS_WIN
	os_assert(mh_file!=INVALID_HANDLE_VALUE);
	CloseHandle((HANDLE)mh_file);
	mh_file = INVALID_HANDLE_VALUE;
#else
	os_assert(mh_file!=-1);
	close(mh_file);
	mh_file=-1;
#endif
}

/*
// mmap，返回地址
char* os_file_map(H_File _hf, longint _offset, int length, OUT H_FileMap *_hfmap)
{
#ifdef OS_WIN
	HANDLE hFileMapping = CreateFileMapping(_hf,NULL,PAGE_READWRITE, 0, 0, NULL);
	os_assert(NULL!=hFileMapping);
	*_hfmap = (H_FileMap)hFileMapping;

	// 将文件数据映射到进程的地址空间
	LARGE_INTEGER pos;
	pos.QuadPart = _offset;
	PBYTE pbFile = (PBYTE)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, pos.HighPart, pos.LowPart, length);

	os_assert(NULL!=pbFile);

	return (char*)pbFile;
#else

#endif
}

// munmap
void os_file_unmap(const char* _addr, H_FileMap _hfmap)
{
#ifdef OS_WIN
	UnmapViewOfFile(_addr);
	CloseHandle(_hfmap);
#else

#endif
}
*/

/*
int OsFile::transmit(int iSocket)
{
#ifdef _WIN32
	HANDLE hFile=(HANDLE)mh_file;

	LARGE_INTEGER li;
	DWORD dwError=0;
	
	li.LowPart = GetFileSize(hFile, (DWORD*)&li.HighPart) ; 
	
	// If we failed ... 
	if (li.LowPart == INVALID_FILE_SIZE && (dwError = GetLastError()) != NO_ERROR )
	{
		return -1;
	} // End of error handler. 
	
	longint sendlen=0;
	longint filelen=li.QuadPart;
	
	while(sendlen < filelen)
	{
		
		DWORD dwResult = SetFilePointer(hFile,(LONG)sendlen, 0,FILE_BEGIN);
		if (HFILE_ERROR == dwResult)
		{
			//os_assert(false);
			return -2;
		}
		BOOL bResult;
		if(filelen - sendlen >= 2147483647)
			bResult = TransmitFile(iSocket, hFile, 2147483647, 10240, NULL, NULL, TF_WRITE_BEHIND);
		else
			bResult = TransmitFile(iSocket, hFile, (unsigned long)(filelen - sendlen), 10240, NULL, NULL, TF_WRITE_BEHIND);
		if (!bResult)
		{
			int err=os_errno();
			const char *errmsg=os_errstr();
			//os_assert(false);
			return -3;
		}
		sendlen += 2147483647;
	}
	return 0;

#else
	//Sendfile函数说明 
	//ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
	longint fsize=0;

	stat64(&fsize, NULL);

	longint pos=0;

	while(pos < fsize)
	{
		ssize_t ret = sendfile(iSocket, mh_file, (off_t*)&pos, fsize);
		if(ret < 0)
			return -1;
		else
		{
			pos += ret;
		}
		
	}
	return 0;
#endif
}*/
