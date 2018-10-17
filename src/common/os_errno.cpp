
#include "common/os_errno.h"

#if defined(OS_WINDOWS)
	#include <windows.h>
#endif

 #include <string.h>
#include <errno.h>


#if defined(OS_WINDOWS)
	static __declspec(thread) char* gs_errstr=NULL;
#endif

int os_errno()
{
#ifdef OS_WINDOWS
    return GetLastError();
#else
    return errno;
#endif
}

const char *os_errstr(int _err)
{
	if(_err==0)
	{
		_err = os_errno();
	}
	
#ifdef OS_WINDOWS
    if(gs_errstr)
    {
        LocalFree((LPVOID)gs_errstr);
    }
    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        _err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &gs_errstr,
        0,
        NULL 
        );
    return gs_errstr;
#else
	return strerror(_err);
    //return strerror_r(_err);
#endif        
}
