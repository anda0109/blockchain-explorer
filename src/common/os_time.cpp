
#include "common/os_time.h"
#include <time.h>

#ifdef COMP_MSC
	#include <windows.h>
#else
#include <sys/time.h>
	#include <sys/times.h>
#endif



#ifdef OS_WIN
longint _win_mktimet(const SYSTEMTIME &ptm)
{
	unsigned int year = ptm.wYear ;
	unsigned int mon = ptm.wMonth;
    unsigned int day =  ptm.wDay;
	unsigned int hour= ptm.wHour;
    unsigned int min = ptm.wMinute;
	unsigned int sec = ptm.wSecond;
	
	if (0 >= (int) (mon -= 2)) {    /**//* 1..12 -> 11,12,1..10 */
		mon += 12;      /**//* Puts Feb last since it has leap day */
		year -= 1;
    }
	
    return (((
		(longint) (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		year*365 - 719499
		)*24 + hour /**//* now have hours */
		)*60 + min /**//* now have minutes */
		)*60 + sec; /**//* finally seconds */
}
#endif



longint os_time_localtimet()
{
#ifdef OS_WIN
	SYSTEMTIME localtime;
	GetLocalTime(&localtime);
	longint tt2 = _win_mktimet(localtime);
	return tt2;
#else
	return (time(NULL)-timezone);
#endif
}

longint os_time_gmttimet()
{
#ifdef OS_WIN
	SYSTEMTIME systime;
	GetSystemTime(&systime);
	longint tt2 = _win_mktimet(systime);
	return tt2;
#else
	return time(NULL);
#endif
}

void os_time_gmttm(OsTime *pTime)
{
#ifdef OS_WIN
	SYSTEMTIME systime;
	GetSystemTime(&systime);
	pTime->tm_year = systime.wYear-1900;
	pTime->tm_mon = systime.wMonth -1;
	pTime->tm_mday = systime.wDay;
	pTime->tm_hour = systime.wHour;
	pTime->tm_min = systime.wMinute;
	pTime->tm_sec = systime.wSecond;
	pTime->tm_wday = systime.wDayOfWeek;
#else
	os_time_mktm(os_time_gmttimet(), pTime);
	
#endif

}

void os_time_localtm(OsTime *pTime)
{
#ifdef OS_WIN
	SYSTEMTIME localtime;
	GetLocalTime(&localtime);
	pTime->tm_year = localtime.wYear-1900;
	pTime->tm_mon = localtime.wMonth -1;
	pTime->tm_mday = localtime.wDay;
	pTime->tm_hour = localtime.wHour;
	pTime->tm_min = localtime.wMinute;
	pTime->tm_sec = localtime.wSecond;
	pTime->tm_wday = localtime.wDayOfWeek;
#else
	os_time_mktm(os_time_localtimet(), pTime);
#endif

}


longint os_time_mktimet(const OsTime &ptm)
{
	unsigned int year = ptm.tm_year + 1900;
	unsigned int mon = ptm.tm_mon+1;
    unsigned int day =  ptm.tm_mday;
	unsigned int hour= ptm.tm_hour;
    unsigned int min = ptm.tm_min;
	unsigned int sec = ptm.tm_sec;
	
	if (0 >= (int) (mon -= 2)) {    /**//* 1..12 -> 11,12,1..10 */
		mon += 12;      /**//* Puts Feb last since it has leap day */
		year -= 1;
    }
	
    return (((
		(longint) (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		year*365 - 719499
		)*24 + hour /**//* now have hours */
		)*60 + min /**//* now have minutes */
		)*60 + sec; /**//* finally seconds */
}


static unsigned short int mon_lengths[2][12] =
{
	/* Normal years.  */
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
		/* Leap years.  */
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
#define	SECS_PER_HOUR	(60 * 60)
#define	SECS_PER_DAY	(SECS_PER_HOUR * 24)
#define	isleap(year)	((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

void os_time_mktm(longint _timer, OsTime *pTime)
{
	//struct tm tbuf;
	int days, rem;
	int y;
	register unsigned short *ip;

	days = (int)_timer / SECS_PER_DAY;
	rem = (int)_timer % SECS_PER_DAY;

	// 计算时分秒
	pTime->tm_hour = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	pTime->tm_min = rem / 60;
	pTime->tm_sec = rem % 60;

	// 计算年和 天数
	/* January 1, 1970 was a Thursday.  */
	pTime->tm_wday = (4 + days) % 7; // 星期几
	y = 1970;
	while (days >= (rem = isleap(y) ? 366 : 365))
	{
		++y;
		days -= rem;
	}
	pTime->tm_year = y - 1900;
	//pTime->tm_yday = days;

	// 计算月和日
	ip = mon_lengths[isleap(y)];
	for (y = 0; days >= ip[y]; ++y)
		days -= ip[y];
	pTime->tm_mon = y;
	pTime->tm_mday = days + 1;
	//pTime->tm_isdst = -1;
	return ;
}

longint os_time_tickcount()
{
#ifdef OS_WINDOWS
    return (longint)GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (longint)(tv.tv_sec*1000 + tv.tv_usec/1000);
#endif
}

str os_time_format(longint time)
{
	OsTime osTime;
	os_time_mktm(time+8*60*60, &osTime);
	str strTime;
	strTime.nfmt(64, "%02d-%02d-%02d %02d:%02d:%02d", osTime.tm_year+1900, osTime.tm_mon+1, osTime.tm_mday, osTime.tm_hour,
		osTime.tm_min, osTime.tm_sec);
	return strTime;
}

str os_time_format()
{
	longint time = os_time_gmttimet();
	return os_time_format(time);
}