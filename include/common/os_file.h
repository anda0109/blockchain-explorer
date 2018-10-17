#ifndef _PLAT_OS_FILE_H_
#define _PLAT_OS_FILE_H_

#include "os_type.h"

PLATAPI int os_file_lock(const str & _filename);
PLATAPI void os_file_unlock(int _lockf_handle);

PLATAPI int os_file_copy(const str & _sFrom, const str & _sTo);
PLATAPI int os_file_rename(const str & _sFrom, const str & _sTo);

// mmap，返回地址
//PLATAPI char* os_file_map(H_File _hf, longint _offset, int length, OUT H_FileMap *_hfmap);

// munmap
//PLATAPI void os_file_unmap(const char* _addr, H_FileMap _hfmap);

class PLATAPI OsFile
{
public:
#ifdef OS_WIN
	void*	mh_file;
#else
	int		mh_file;
#endif
public:
	OsFile();
	~OsFile();
public:
	int open_file_read_r(const str & _filename);
	int open_file_write_r(const str & _filename);
	int open_read_excl_r(const str & _filename);
	int create_write_excl(const str & _filename);
public:
	longint seek_set(longint _origin);
	longint seek_cur(longint _origin);
	longint seek_end(longint _origin);
public:
	// 读写
	int fread_r(OUT void * obuf, IN int _buflen);
	void fwrite(const void * buf, int _buflen);
	// 格式化写
	void fprintf(int _maxsize, const char*_fmt, ...);
	void flush();
public:
	void stat(OUT int *_size, OUT int *_mutctime);	// 文件大小和修改时间
	void stat64(OUT longint *_size, OUT longint *_mutctime);	// 文件大小和修改时间
public:
	void fclose();
public:
	int transmit(int iSocket);
};

#endif
