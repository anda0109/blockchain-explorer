#ifndef _PLAT_OS_ERRNO_H_
#define _PLAT_OS_ERRNO_H_

#include "os_type.h"
PLATAPI int os_errno();
PLATAPI const char *os_errstr(int _err=0);

#endif
