#ifndef _PLAT_OS_TIME_H_
#define _PLAT_OS_TIME_H_

#include "os_type.h"

struct OsTime
{
    int tm_sec;      //seconds after the minute - [0,59] 
    int tm_min;      //minutes after the hour - [0,59] 
    int tm_hour;     //hours since midnight - [0,23] 
    int tm_mday;     //day of the month - [1,31] 
    int tm_mon;      //months since January - [0,11] 
    int tm_year;     //years since 1900

    int tm_wday;     //days since Sunday - [0,6] 
    //int tm_yday;     //days since January 1 - [0,365] 
    //int tm_isdst;    //daylight savings time flag 
};

PLATAPI longint os_time_localtimet();
PLATAPI longint os_time_gmttimet();

PLATAPI void os_time_localtm(OUT OsTime *pTime);
PLATAPI void os_time_gmttm(OUT OsTime *pTime);

// 与时区无关，本地时间仍然是本地时间，UTC时间还是UTC时间。
PLATAPI longint os_time_mktimet(const OsTime &ptm); 
PLATAPI void os_time_mktm(longint _timer, OsTime *pTime);
PLATAPI longint os_time_tickcount();

str os_time_format(longint time);

str os_time_format();

#endif
