
#include "common/os_type.h"
//#include "plat/fw_mem.h"
#include "common/os_time.h"
#include "common/os_errno.h"
//#include "plat/os_socket.h"
#include "common/os_libc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif

#define LF '\n'
#define CR '\r'
#define CRLF "\r\n"
#define HT  0x09
#define SP  ' '

void _os_assert(IN const char *_sFile, IN int _iLine, int exp, IN const char *fmt)
{
	if(exp)
		return;
	
	FILE *fp = NULL;
	fp = fopen("alarm.log", "a");
	if (fp == NULL)
		fp = fopen( "alarm.log", "w");
	
	OsTime DateTime;
	os_time_mktm(os_time_gmttimet(), &DateTime);
	
	if(fp) 
	{
#ifdef _WIN32
		fprintf(fp, "** assert fail, %s, err:%d/%s\n    [%04d-%02d-%02d %02d:%02d:%02d %I64u][%s][%d]\n\n",
			fmt,
			os_errno(),
			os_errstr(),
			DateTime.tm_year+1900,
			DateTime.tm_mon+1,
			DateTime.tm_mday,
			DateTime.tm_hour, 
			DateTime.tm_min, 
			DateTime.tm_sec,
			os_time_tickcount(),
			_sFile,
			_iLine);
#else
		fprintf(fp, "** assert fail, %s, err:%d/%s\n    [%04d-%02d-%02d %02d:%02d:%02d %llu][%s][%d]\n\n",
			fmt,
			os_errno(),
			os_errstr(),
			DateTime.tm_year+1900,
			DateTime.tm_mon+1,
			DateTime.tm_mday,
			DateTime.tm_hour, 
			DateTime.tm_min, 
			DateTime.tm_sec,
			os_time_tickcount(),
			_sFile,
			_iLine);
#endif
		fclose(fp);
	}
	
	abort();
}


void str::_init()
{
	mi_len = 1;
	mp_str =(char*) "";
	mi_bufsize = 0;
}

void str::_free()
{
	if(mi_bufsize>0)
		delete [] mp_str;
	mi_len = 1;
	mp_str = (char*)"";
	mi_bufsize = 0;
}

str::operator const char*() const
{
	return mp_str;
}

char * str::getbuf()
{
	os_assert(mi_bufsize>0);

	return mp_str;
}


const char * str::_getbuf() const
{
	return mp_str;
}


int  str::strlen() const
{
	return mi_len-1;
}

int str::bufsize() const
{
	return mi_bufsize;
}

str::~str()
{
	_free();
}


str::str()
{
	_init();
}

str::str(int _bufsize)
{
	os_assert(_bufsize>0);
	mp_str = new char[_bufsize];
	mp_str[0]='\0';
	mi_bufsize = _bufsize;
	mi_len = 1;
}


str::str(const char* _str)
{
	os_assert(_str!=NULL);

	int len = osc_strlen(_str);

	mi_len = len+1;
	mp_str = (char*)_str;
	mi_bufsize = 0;
}

str::str(char* _str)
{
	int len = osc_strlen(_str);
	if(len==0)
	{
		mi_len = 1;
		mp_str = (char*)"";
		mi_bufsize = 0;
		return;
	}
	
	mp_str = new char[len+1];
	mi_bufsize = len+1;
	mi_len = len+1;
	osc_memcpy(mp_str, _str, mi_len);
}

str::str(const str &m)
{
	if(m.bufsize()==0)
	{
		mp_str = (char*)(const char*)m;
		mi_bufsize = 0;
		mi_len = m.strlen()+1;
	}
	else
	{
		mi_bufsize = m.strlen()+1;
		mp_str = new char[mi_bufsize];
		mi_len = mi_bufsize;
		osc_memcpy(mp_str, m._getbuf(), mi_len);
	}
}

str::str(int _maxlen, const char *_fmt, ...)
{	
	va_list args;

	os_assert(_maxlen>0);

	mp_str = new char[_maxlen+1];
	//mp_str[0]='\0';
	mi_bufsize = _maxlen+1;
	mi_len = 1;

	va_start(args, _fmt);
	int ret = vsnprintf(mp_str, _maxlen, _fmt, args);
	os_assert(ret>0);
    va_end(args);
	mi_len = ret+1;
}


void str::operator = (const str &m)
{
	if(m.bufsize()==0)
	{
		_free();
		mp_str = (char*)m._getbuf();
		mi_bufsize = 0;
		mi_len = m.strlen()+1;
	}
	else
	{
		int len2= m.strlen()+1;
		if(mi_bufsize < len2)
		{
			newbuf(len2);
		}
		mi_len =len2;
		osc_memcpy(mp_str, m._getbuf(),mi_len);
	}
}



void str::operator = (const char* _str)
{
/*	_free();
	int len = osc_strlen(_str);
	
	mi_len = len+1;
	mp_str = (char*)_str;
	mi_bufsize = 0;
*/

	int len = osc_strlen(_str);
	if(len+1 > mi_bufsize)
	{
		_free();
		mp_str = new char[len+1];
		mi_bufsize = len+1;
		mi_len = len+1;
		if(_str)
			osc_memcpy(mp_str, _str, mi_len);
		else
			osc_memcpy(mp_str, "", mi_len);
	}
	else
	{
		mi_len = len+1;
		if(_str)
			osc_memcpy(mp_str, _str, mi_len);
		else
			osc_memcpy(mp_str, "", mi_len);
	}
}

/*
void str::operator = (char* _str)
{
	int len = osc_strlen(_str);
	if(len+1 > mi_bufsize)
	{
		_free();
		mp_str = new char[len+1];
		mi_bufsize = len+1;
		mi_len = len+1;
		osc_memcpy(mp_str, _str, mi_len);
	}
	else
	{
		mi_len = len+1;
		osc_memcpy(mp_str, _str, mi_len);
	}
}
*/

void str::newbuf(int _size)
{
	os_assert(_size>0);
	if(_size<= mi_bufsize)
	{
		return;
	}
	
	char *pval = new char[_size];
	if(mi_bufsize)
		delete[] mp_str;
	mp_str = pval;
	mi_bufsize = _size;
	
	mi_len = 1;
	mp_str[0]='\0';
}

void str::setstrlen(int _len)
{
	os_assert(_len>=0);
	os_assert(_len < mi_bufsize);
	
	mi_len = _len+1;
}

void str::enlargebufto(int _size)
{
	os_assert(_size>0);
	if(_size<= mi_bufsize)
	{
		return;
	}
	char *pval = new char[_size];
	if(mi_len>0)
	    osc_memcpy(pval, mp_str, mi_len);
	if(mi_bufsize)
		delete[] mp_str;
	mp_str = pval;
	mi_bufsize = _size;
}

void str::copyFromCStrByLen(const char* pval, int len)
{
	os_assert(len>=0);
	if(len==0)
		return;
	if(pval==NULL)
		return;

	if(len+1 > mi_bufsize)
	{
		enlargebufto(len+1 + mi_bufsize);
	}

	osc_memcpy(mp_str, pval, len);
	mp_str[len] = '\0';
	mi_len = len+1;
}

void str::appendFromCStrByLen(const char* pval, int len)
{
	os_assert(len>0);
	os_assert(mi_len + len < mi_bufsize);
	
	if(len==0)
		return;
	if(pval==NULL)
		return;
	osc_memcpy(mp_str+mi_len, pval, len);
	mp_str[mi_len+len] = '\0';
	mi_len += len+1;
}

void str::assignRef(const char*pval, int len)
{
	if(pval==NULL && len>0)
	{
		os_assert(false);
	}

	_free();
	mp_str = (char*)pval;
	mi_bufsize = 0;
	mi_len = len;
}


void str::copyFromStrPosToEnd(const str &_str, int pos)
{
	os_assert(pos>=0);
	os_assert(pos<_str.strlen());
	int len2=_str.strlen() - pos;
	
	copyFromCStrByLen(_str._getbuf()+pos, len2);
}


void str::copyFromStrPosByLen(const str &_str, int pos, int len)
{
	os_assert(pos>=0);
	os_assert(len>=0);
	os_assert(pos+len<_str.strlen());
	
	copyFromCStrByLen(_str._getbuf()+pos, len);
}

void str::operator +=(const str &cstr2)
{
	int len2=cstr2.strlen();
	if(len2==0)
		return;
	
	if(strlen() + len2 +1 > mi_bufsize)
		enlargebufto(strlen() + len2 +1 + strlen());
	osc_memcpy(getbuf()+strlen(), cstr2._getbuf(), len2+1);
	setstrlen(strlen() + len2);
}

void str::operator +=(const char *_str)
{
	int len2=osc_strlen(_str);
	if(len2==0)
		return;
	
	if(strlen() + len2 +1 > mi_bufsize)
		enlargebufto(strlen() + len2 +1 + strlen());
	osc_memcpy(getbuf()+strlen(), _str, len2+1);
	setstrlen(strlen() + len2);
}

void str::operator +=(const int num)
{
	char buf[16];
	osc_snprintf(buf, sizeof(buf)-1, sizeof(buf) - 1, "%d", num);
	*this += buf;
}

void str::operator +=(const char ch)
{
	const char stmp[2]={ch, '\0'};
	*this += stmp;
}

bool str::operator ==(const char *_str)  const 
{
	if(strlen()==0)
	{
		if(NULL==_str || _str[0]=='\0')
			return true;
		else
			return false;
	}
	else
	{
		if(NULL==_str || _str[0]=='\0')
			return false;
	}
	
	if(osc_strcmp(_getbuf(), _str)==0)
		return true;
	else
		return false;
}

bool  str::operator ==(const str& cstr2) const 
{
	return *this==cstr2._getbuf();
}


bool  str::operator !=(const char *_str) const 
{
	return !(*this==_str);
}

bool  str::operator !=(const str &_cstr2) const 
{
	return !(*this==_cstr2);
}

int str::find(const char*substr) const 
{
	char *p=osc_strstr(_getbuf(), substr);
	
	if(NULL==p)
		return -1;
	int ret=p-_getbuf();
	return ret;
}

int str::find(int iPos, const char* substr) const 
{
	os_assert(iPos>=0);
	os_assert(iPos<strlen());
	
	char *p=osc_strstr(_getbuf() + iPos, substr);
	
	if(NULL==p)
		return -1;
	int ret=p-_getbuf();
	return ret;
}

int str::findskip(int pos, const char*substr)  const
{
	int ret = find(pos, substr);
	if(ret<0)
		return ret;
	else
		return ret + osc_strlen(substr);
}

int str::rfind(const char* substr) const
{
	if(substr == NULL)
		return -1;
	int len2=osc_strlen(substr);
	int len1=  mi_len;
	if(len1==0 || len1 < len2)
		return -1;
	
	const char* p = _getbuf() + mi_len - 1;
	if(NULL == p)
		return -1;
	const char* q = NULL;
	int ret = -1;
	while (p >= _getbuf())
	{
		q = osc_strstr(p, substr);
		if (q != NULL)
		{
			ret = q -  _getbuf();
			return ret;
		}
		p--;
	}
	
	return -1;
}

char & str::operator[](int index)
{
	os_assert(index>=0 && index < mi_len);	// 数组越界
	return mp_str[index];
}


const char & str::operator[](int index) const 
{
	os_assert(index>=0 && index < mi_len);	// 数组越界
	return mp_str[index];
}

int str::skipSPTAB(int pos) const
{
	os_assert(mi_len>0);
	os_assert(pos>=0);
	os_assert(pos<=mi_len);

	if(pos==mi_len)	// 已经结束了。
		return pos;

	char *p=mp_str + pos;
	int retpos=pos;

    for( ; (*p!='\0') && (retpos < mi_len) && (*p==SP || *p==HT); ++p, ++retpos);

	return retpos;
}

int str::skipSPTABCRLF(int pos) const 
{
	os_assert(mi_len>0);
	os_assert(pos>=0);
	os_assert(pos<=mi_len);
	
	if(pos==mi_len)	// 已经结束了。
		return pos;

	char *p=mp_str + pos;
	int retpos=pos;
	
    for( ; (*p!='\0') && (retpos < mi_len) && (*p==SP || *p==HT || *p==CR || *p==LF); ++p, ++retpos);
	
	return retpos;
}

int str::_getstr(int _pos, const char*sp, OUT const char **begin, OUT int *len) const 
{
	int pos=_pos;
	os_assert(pos >=0);
	if(pos>mi_len)
		return -1;
	//os_assert(pos <= mi_len);


	if(pos == mi_len)
	{
		return -1;
	}

	pos=skipSPTABCRLF(pos);
	*begin = mp_str + pos;
	*len=0;

	const char *p=_getbuf() + pos;
	for( ; pos < mi_len; ++pos,++(*len),++p)
	{
		if(*p==SP || *p==HT || *p==CR || *p==LF || *p=='\0')
		{
			break;
		}
		if(osc_strchr(sp, *p))
			break;
	}

	++pos;	// 跳过分隔符
	return pos;
}

int str::scanf_str(int _pos, const char*sp, OUT str *_str) const
{
	int pos = _pos;
	const char *begin=NULL;
	int len=0;

	pos = _getstr(pos, sp, &begin, &len);

	if(len==0)
	{
		*_str = "";
		return pos;
	}

	_str->copyFromCStrByLen(begin, len);
	return pos;
}


int str::scanf_int(int pos, int *val)  const
{
	os_assert(val!=NULL);
	os_assert(pos>=0);
	os_assert(pos <=mi_len);
	if(pos == mi_len)
		return -1;

	int total=0;
	int sign=1;
	
	pos=skipSPTABCRLF(pos);
	const char *p=_getbuf()+pos;

	// 读取符号
	if(*p=='-')
	{
		sign = -1;
		++p;
		++pos;
	}
	else if(*p=='+')
	{
		sign = 1;
		++p;
		++pos;
	}
	if(pos == mi_len)
		return -1;

	total = 0;
	for( ; (*p>='0') && (*p<='9') && (pos < mi_len); ++pos,++p)
	{
		total = 10 * total + (*p - '0');
	}
	
	if (sign == -1)
		*val = -total;
	else
		*val = total;

	return pos;
}

int str::ncmp(int pos, const char* _str, int count) const
{
	os_assert(count>0);
	os_assert(pos >=0);
	os_assert(pos + count < mi_len);

	if(count == 0 && mi_len==0)
		return 0;

	return osc_strncmp(mp_str + pos, _str, count);
}

int str::cmp(const char*_str) const
{
	return osc_strcmp(mp_str, _str);
}

int str::cmp(const str &_str) const
{
	return osc_strcmp(mp_str, _str._getbuf());
}


int str::asint() const
{
	int val=0;
	//int pos=
		scanf_int(0, &val);
	return val;
}


bool str::asbool() const
{
	if(*this=="true")
		return true;
	else
		return false;
}

int str::subnum(const char* substr) const
{
	if(NULL == substr)
		return -1;
	int iNum = 0;
	int iPos = 0;

	while (true)
	{
		iPos = findskip(iPos, substr);
		if (iPos < 0)
		{
			break;
		}
		else 
		{
			iNum++;
		}
	}
	return iNum;
}

// 字符串赋值
void str::nfmt(int _maxstrlen, const char*_fmt, ...)
{
	va_list args;
	
	os_assert(_maxstrlen>0);
	os_assert(_fmt!=NULL);
	
	newbuf(_maxstrlen+1);
	
	va_start(args, _fmt);
	int len = vsnprintf(getbuf(), _maxstrlen, _fmt, args);
	va_end(args);
	
	setstrlen(len);
}

void str::appendnfmt(int _maxstrlen, const char*_fmt, ...)
{
	va_list args;
	
	os_assert(_maxstrlen>0);
	os_assert(_fmt!=NULL);
	
	newbuf(mi_len + 1 + _maxstrlen);
	
	va_start(args, _fmt);
	int len = vsnprintf(getbuf() + strlen(), _maxstrlen, _fmt, args);
	va_end(args);
	
	setstrlen(strlen() + len+1);
}


int str::copyLineToStr(int pos, OUT str *_str) const
{
	os_assert(pos>=0);
	os_assert(_str!=NULL);
	
	os_assert(pos<mi_len);
	
	char *p0=mp_str + pos;
	char *endch=mp_str + mi_len;
	
	char *p=p0;
	while(true)
	{
		if(*p!=CR && *p!=LF && p!=endch)
		{
			++p;
		}
		else
		{
			break;
		}
	}
	_str->copyFromCStrByLen(p0, p-p0);
	
	if(p==endch)
	{
		return 0;
	}
	
	if(*p==CR || *p==LF)
	{
		++p;
		if(p==endch)
		{
			return 0;
		}
		if(*p==CR || *p==LF)
		{
			++p;
		}
	}
	if(p==endch)
	{
		return 0;
	}
	return p-mp_str;
}

