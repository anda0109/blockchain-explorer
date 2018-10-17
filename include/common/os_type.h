

#ifndef _PLAT_OS_TYPE_H_
#define _PLAT_OS_TYPE_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


#ifdef PLAT_EXPORTS
	#define PLATAPI PLAT_EXPORTS
#else
	#define PLATAPI
#endif


// WIN32_LEAN_AND_MEAN

// ��̬�����
#ifdef _WIN32
#define DLL_EXPORT extern "C" _declspec(dllexport)
#pragma warning(disable: 4251)
#pragma warning(disable: 4284)
#pragma warning(disable:4996)
#else
#define DLL_EXPORT extern "C"
#define __stdcall
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

// ���������ֽ���
#if defined(__x86_64) || defined(_M_X64)
//#define ARCH_AMD64	
#define ARCH_X86_64
#define ARCH_X64
#define ARCH_LITTLE_ENDIAN
#elif defined(_M_IX86) || defined(__i386__) || defined(__i386) || defined(_X86_)
#define ARCH_X86
#define ARCH_LITTLE_ENDIAN
#elif defined(_M_IA64) || defined(__ia64) || defined(__ia64__)
#define ARCH_IA64
#define ARCH_LITTLE_ENDIAN
#elif defined(__powerpc__) || defined(_ARCH_PPC)
#define ARCH_PPC
#define ARCH_BIG_ENDIAN
#elif defined(__sparc__)
#define ARCH_SPARC
#define ARCH_BIG_ENDIAN
#elif defined(__hppa__)
#define ARCH_HPPA
#define ARCH_BIG_ENDIAN
#elif defined(__arm__) || defined(_ARM)
#define ARCH_ARM
#define ARCH_LITTLE_ENDIAN
#else
#error unsupported architecture
#endif

// ����ϵͳ
#if defined(_AIX)
#define OS_AIX
#elif defined(__FreeBSD__)
#define OS_FreeBSD
#elif defined(_hpux)
#define OS_HPUX
#elif defined(linux)
#define OS_LINUX
#elif defined(macintosh)
#define OS_MAC
#elif defined(_WIN32)
#define OS_WINDOWS
#define OS_WIN
#elif defined(_WIN32_WCE)
#define OS_WINDOWS_CE
#define OS_WINCE
#elif defined(sun)
#define OS_SOLARIS
#else
#error unsupported operation system
#endif

// ������
#if defined(__GNUC__)
#define COMP_GCC
#elif defined(_MSC_VER)
#define COMP_MSC
#elif defined(__xlc__) || defined(__xlC__)
#define COMP_XLC
#elif defined(__INTEL_COMPILER)
#define	COMP_ICC
#elif defined(__HP_cc) || defined(__HP_aCC)
#define	COMP_aCC
#else
#error unsupported compiler
#endif

// �ֳ�
#ifdef ARCH_X86_64
#define __WORDSIZE  64
#elif defined(ARCH_PPC)
#define __WORDSIZE  64
#elif defined(ARCH_X86)
#define __WORDSIZE  32
#elif defined(ARCH_ARM)
#define __WORDSIZE  32
#endif

// �������
#define PLAT_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name

// ����
PLATAPI void _os_assert(IN const char *_sFile, IN int _iLine, int exp, IN const char *fmt);
#define  os_assert(exp) if(!(bool)(exp)) _os_assert(__FILE__, __LINE__, (exp), #exp)

// ������������
#if defined(OS_WIN)
typedef char			int8;
typedef unsigned char	uint8;
typedef short			int16;
typedef unsigned short	uint16;
typedef int			int32;
typedef unsigned int		uint32;
typedef __int64             int64;
typedef unsigned __int64    uint64;
#elif defined(OS_LINUX)
typedef char			int8;
typedef unsigned char	uint8;
typedef short			int16;
typedef unsigned short	uint16;
typedef int			    int32;
typedef unsigned int	uint32;
typedef long long           int64;
typedef unsigned long long           uint64;

#elif defined(OS_AIX)
typedef u_int8   uint8;
typedef u_int16   uint16;
typedef u_int32   uint32;
typedef u_int64   uint64;
#endif

typedef int64 longint;
typedef uint64 ulongint;

typedef void* pvoid;
typedef unsigned int	uint;
#ifndef NULL
#define NULL 0
#endif

#if __WORDSIZE==64
typedef long unsigned int size_t;
#else
	typedef uint size_t;
#endif

#define SIZEOF (int)sizeof

// swab
inline uint16 os_swab16(uint16 x)
{
	return x<<8 | x>>8;
}

inline uint32 os_swab32(uint32 x)
{
	return x<<24 | x>>24 |
		(x & (uint32)0x0000ff00)<<8 |
		(x & (uint32)0x00ff0000)>>8;
}

inline uint64 os_swab64(uint64 x)
{
#ifdef OS_WIN
	return x<<56 | x>>56 |
		(x & (uint64)0x000000000000ff00)<<40 |
		(x & (uint64)0x0000000000ff0000)<<24 |
		(x & (uint64)0x00000000ff000000)<< 8 |
		(x & (uint64)0x000000ff00000000)>> 8 |
		(x & (uint64)0x0000ff0000000000)>>24 |
		(x & (uint64)0x00ff000000000000)>>40;
#else
	return x<<56 | x>>56 |
		(x & (uint64)0x000000000000ff00ULL)<<40 |
		(x & (uint64)0x0000000000ff0000ULL)<<24 |
		(x & (uint64)0x00000000ff000000ULL)<< 8 |
		(x & (uint64)0x000000ff00000000ULL)>> 8 |
		(x & (uint64)0x0000ff0000000000ULL)>>24 |
		(x & (uint64)0x00ff000000000000ULL)>>40;
#endif
}

// �����ֽ��� �� Сͷ�ֽ��� ��ת
inline uint16 os_host_little_uint16(uint16 _len)
{
#ifdef ARCH_BIG_ENDIAN
		return os_swab16(_len);
#else
		return _len;
#endif
}

inline uint32 os_host_little_uint32(uint32 _len)
{
#ifdef ARCH_BIG_ENDIAN
	return os_swab32(_len);
#else
	return _len;
#endif
}

inline uint64 os_host_little_uint64(uint64 _len)
{
#ifdef ARCH_BIG_ENDIAN
	return os_swab64(_len);
#else
	return _len;
#endif
}

// �����ֽ��� �� ��ͷ�ֽ��������ֽ��򣩻�ת
inline uint16 os_host_big_uint16(uint16 _len)
{
#ifdef ARCH_LITTLE_ENDIAN
	return os_swab16(_len);
#else
	return _len;
#endif
}

inline uint32 os_host_big_uint32(uint32 _len)
{
#ifdef ARCH_LITTLE_ENDIAN
	return os_swab32(_len);
#else
	return _len;
#endif
}

inline uint64 os_host_big_uint64(uint64 _len)
{
#ifdef ARCH_LITTLE_ENDIAN
	return os_swab64(_len);
#else
	return _len;
#endif
}

// ����ָ�룬������
template <typename Type>
class ptr
{
private:
	Type *mp_val;
	int  *count;
	typedef Type * PType;
public:
	ptr()
	{
		mp_val=NULL;
		count=NULL;
	}
	void _init()
	{
		mp_val=NULL;
		count=NULL;
	}
    ptr(Type *p)
	{
		mp_val = p;
		if(mp_val)
		{
			count = new int;
			*count=1;
		}
		else
		{
			count = NULL;
		}
	}
	ptr(const ptr<Type> &_ptr)
	{
		mp_val = _ptr.mp_val;
		count = _ptr.count;
		if(count)
			++(*count);
	}
	~ptr()
	{
		if(count)
		{
			--(*count);
			if(*count==0)
			{
				delete count;
				delete mp_val;
			}
		}
	}

	bool operator ==(const ptr<Type>& _ptr) const 
	{
		if(_ptr.get() == mp_val)
			return true;
		else
			return false;
	}
	bool operator !=(const ptr<Type>& _ptr) const 
	{
		if(_ptr.get() != mp_val)
			return true;
		else
			return false;
	}

	bool operator ==(void* _zero) const 
	{
		os_assert(NULL==_zero);
		if(mp_val)
			return false;
		else
			return true;
	}

	bool operator !=(void* _zero) const 
	{
		os_assert(NULL==_zero);
		if(mp_val)
			return true;
		else
			return false;
	}
	void operator= (const ptr<Type>& _ptr)
	{
		// ���Ƚ����ü�����1, Ȼ�����ж��Ƿ�С��0, ���С��0, ��delete.			
		if (count)
		{
			--(*count);
			if(*count==0)
			{
				delete count;
				delete mp_val;
			}
		}
		
		//
		mp_val = _ptr.mp_val;
		count = _ptr.count;
		if(count)
			++(*count);
	}

	void operator= (Type *p)
	{
		// ���Ƚ����ü�����1, Ȼ�����ж��Ƿ�С��0, ���С��0, ��delete.			
		if (count)
		{
			--(*count);
			if(*count==0)
			{
				delete count;
				delete mp_val;
			}
		}
		
		mp_val = p;
		if(mp_val)
		{
			count = new int;
			*count=1;
		}
		else
		{
			count = NULL;
		}
	}

	Type *operator-> (void)  
	{
		os_assert(mp_val!=NULL);
		return mp_val; 
	}
	const Type& operator *(void) const 
	{ 
		os_assert(mp_val!=NULL);
		return *mp_val; 
	}
	Type& operator *(void) 
	{ 
		os_assert(mp_val!=NULL);
		return *mp_val; 
	}

	Type* get() const 
	{
		return mp_val;
	}
};


// ���飬���ݴ�ͳ�������顣
template <typename Type>
class a
{
private:
	Type *ma_val;
	int	 mi_len;
	int  mi_bufsize;	// ��ǰ����
public:
	a()
	{
		mi_len = 0;
		ma_val = NULL;
		mi_bufsize = 0;
	}
	a(int _bufsize, int _arrsize)
	{
		os_assert(_bufsize>0);
		os_assert(_bufsize >= _arrsize);
		ma_val = new Type[_bufsize];
		mi_bufsize = _bufsize;
		mi_len = _arrsize;
	}
	
	a(const a<Type> &arr)
	{
		if(arr.len()==0)
		{
			mi_len = 0;
			ma_val = NULL;
			return;
		}

		ma_val = new Type[arr.len()];
		mi_bufsize = arr.len();

		mi_len = arr.len();
		for(int i=0; i<mi_len; ++i)
			ma_val[i] = arr[i];
	}
	void operator = (const a<Type> &arr)
	{
		int len2=arr.len();

		if(ma_val)
			delete [] ma_val;
		if(len2>0)
		{
			ma_val = new Type[len2];
			mi_bufsize = len2;
			mi_len = len2;
			for(int i=0; i<mi_len; ++i)
				ma_val[i] = arr[i];
		}
		else
		{
			mi_len = 0;
			ma_val = NULL;
			mi_bufsize = 0;
		}
	}
	~a()
	{
		if(ma_val)
			delete[] ma_val;
		mi_len = 0;
		ma_val = NULL;
		mi_bufsize = 0;
	}

	void newbuf(int _bufsize)
	{
		os_assert(_bufsize>0);
		
		if(ma_val)
			delete [] ma_val;
		if(_bufsize>0)
		{
			ma_val = new Type[_bufsize];
			mi_bufsize = _bufsize;
			mi_len = 0;
		}
		else
		{
			mi_len = 0;
			ma_val = NULL;
			mi_bufsize = 0;
		}
		mi_len = 0;
	}
	void setlen(int _size)
	{
		os_assert(_size>=0);
		os_assert(_size<=mi_bufsize);
		mi_len = _size;
	}
	
	void enlargeBufAtLeast(int _size)
	{
		os_assert(_size>0);

		int size=mi_bufsize *2 + 1;
		while(size-mi_bufsize < _size)
		{
			size *=2;
		}

		Type *pval = new Type[size];
		for(int i=0; i<mi_len; ++i)
			pval[i] = ma_val[i];
		if(ma_val)
			delete[] ma_val;
		ma_val = pval;
		mi_bufsize = size;
	}


	Type & operator[](int index)
	{
		os_assert(index>=0 && index < mi_len);	// ����Խ��
		return ma_val[index];
	}
	const Type & operator[](int index) const 
	{
		os_assert(index>=0 && index < mi_len);	// ����Խ��
		return ma_val[index];
	}

	int len() const
	{
		return mi_len;
	}
	int bufsize() const
	{
		return mi_bufsize;
	}
	
	void push(const Type & val)
	{
		if(mi_len == mi_bufsize)
			enlargeBufAtLeast(1);
		ma_val[mi_len] = val;
		++mi_len;
	}

	Type * get()
	{
		return ma_val;
	};

	Type *get_end()
	{
		return &ma_val[mi_len];
	}

	void addlen(int len)
	{
		os_assert(mi_len + len <= mi_bufsize);
		mi_len += len;
	}
};

// ��̬���飬������
template <typename Type>
class Vec
{
public:
	typedef ptr<Type> PType;
	a<PType>		ma_ptype;
public:
	Vec(const Vec<Type> &_vec):ma_ptype(_vec.ma_ptype)
	{
	}
	
	Vec():ma_ptype(8, 0)
	{
	}
	void push(const PType &pval)
	{
		ma_ptype.push(pval);
	}
	
	Type & operator [] (int index) 
	{
		os_assert(index>=0);
		os_assert(index<ma_ptype.len());
		
		return *ma_ptype[index];
	}
	const Type & operator [] (int index) const 
	{
		os_assert(index>=0);
		os_assert(index<ma_ptype.len());

		return *ma_ptype[index];
	}
	
	int len() const 
	{
		return ma_ptype.len();
	}
};

// 
class PLATAPI str
{
private:
	char *mp_str;
	int	 mi_len;		// ����0������
	int  mi_bufsize;	// ��ǰ������Ϊ0��ʾû�������ڴ档
						// mi_bufsizeΪ0��mp_str��mi_len��Ϊ0ʱ����ʾmp_strָ����ڴ治��д��str����ʱҲ����Ҫ�ͷ�mp_str��
public:
	void _init();
	void _free();
public:
	operator const char*() const;
	const char * _getbuf() const;
	int  strlen() const;
	int  bufsize() const;
	const char & operator[](int index) const;
	char * getbuf();
	char & operator[](int index);
public:
	~str();
	str();
	str(int _bufsize);
	str(const char* _str);
	str(char* _str);
	str(const str &m);
	str(int _maxlen, const char *_fmt, ...);

	void operator = (const str &m);
	void operator = (const char* _str);
	//void operator = (char* _str);
public:
	void newbuf(int _size);	// ���ݶ�ʧ
	void setstrlen(int _len);
	void enlargebufto(int _size); // �������ݲ���ʧ
	//void enlargeBufAtLeast(int _size); // ��������

public:
	// ��ֵ ******** 
	void copyFromCStrByLen(const char* pval, int len); // len������0������
	void appendFromCStrByLen(const char* pval, int len);
	void assignRef(const char*pval, int len);
	void copyFromStrPosToEnd(const str &_str, int pos);
	void copyFromStrPosByLen(const str &_str, int pos, int len);

	void nfmt(int _maxstrlen, const char*_fmt, ...);
	void appendnfmt(int _maxstrlen, const char*_fmt, ...);

	void operator +=(const str &_dstr2);
	void operator +=(const char *_str);
	void operator +=(const char ch);
	void operator +=(const int num);
	//void operator +=(const uint num);
	//void operator +=(const longint num);
	//void operator +=(const ulongint num);

	bool  operator ==(const char *_str) const;
	bool  operator ==(const str& cstr2) const;
	bool  operator !=(const char *_str) const;
	bool  operator !=(const str &_cstr2) const;

	// ���ң�<0 fail, >=0 �Ӵ�λ�ã�
	int find(const char* substr)  const ;
	int find(int pos, const char*substr)  const ;
	int findskip(int pos, const char*substr)  const ;
	int rfind(const char* substr) const;
	// �Ӻ��濪ʼ���ң��ӹ��Ӵ�
	//int rfindskip(const char* substr) const;
	//! ͳ���ַ������Ӵ��ĸ���
	int subnum(const char* substr) const;


	// �Ӵ� ******** 
	// ���˿ո��TAB�����ع��˺��λ�á�
	int skipSPTAB(int pos) const;

	// ���˿ո�TAB��CRLF�����ع��˺��λ�á�
	int skipSPTABCRLF(int pos) const;

	// ��ȡ�ַ���, pos ��ʼλ�ã�����sp�ָ�����",|<" ��SP,TAB,CRLF��sp�����طָ��������λ��
	int _getstr(int pos, const char*sp, OUT const char **begin, OUT int *len) const;

	// ��ȡ�ַ���, pos ��ʼλ�ã�����sp�ָ�����SP,TAB,CRLF��sp�����طָ��������λ��
	int scanf_str(int pos, const char*sp, OUT str *_str) const;

	// ��pos��ʼscanf(%d)�����������������λ�ã�-1ʧ��
	int scanf_int(int pos, int *val)  const ;
	//int scanf_hexint(int pos, int *val)  const;

	// �Ƚ� ********* 
	// ��pos��ʼ���Ƚ�count����
	int ncmp(int pos, const char* _str, int count) const;
	int cmp(const char*_str) const;
	int cmp(const str &_str) const;

	int asint() const;		// ����ʱ������0
	//	longint aslongint() const;	// ����ʱ����0
	bool asbool() const;

	// ������һ��λ��
	int copyLineToStr(int pos, OUT str *_str) const;
};

typedef a<str> astr;
typedef a<int> aint;
//typedef a<char>	Buf;

#endif

