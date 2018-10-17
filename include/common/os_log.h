#ifndef _PLAT_OS_LOG_H_
#define _PLAT_OS_LOG_H_

#include "os_type.h"

#define FL	__FILE__, __LINE__

#define JENC_SYS   0
#define JENC_DEBUG 1
#define JENC_TRACE 2
#define JENC_ERROR 3
#define JENC_LOG   4
#define JENC_ALARM 5

PLATAPI void os_log_init(IN const char * _log_file, int _level = JENC_LOG, int _log_size = 1024*1024);

PLATAPI void os_set_log_file(IN const char * _log_file);
PLATAPI void os_set_log_level(int _level);
PLATAPI void os_set_log_size(int _log_size);


PLATAPI void os_sys(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...);
PLATAPI void os_debug(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...);
PLATAPI void os_trace(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...);
PLATAPI void os_error(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...);
PLATAPI void os_log(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...);
PLATAPI void os_alarm(IN const char *_sFile, IN int _iLine, IN const char *fmt, IN ...);

#endif
