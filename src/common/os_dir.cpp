
#include "common/os_file.h"

#ifdef OS_WINDOWS
#include <direct.h>
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include    <dirent.h>
#endif

#include "common/os_type.h"
#include "common/os_dir.h"
#include "common/os_libc.h"
#include "common/os_errno.h"
//#include "common/os_ipc.h"

static str *gps_home=NULL;

void os_home_init()
{
	os_assert(NULL==gps_home);
	gps_home = new str();
}

void _os_set_home(const str &_home)
{
	os_assert(NULL!=gps_home);
	*gps_home = _home;	
}

const char* os_home()
{
	os_assert(NULL!=gps_home);
	return gps_home->getbuf();
}

void os_get_cwd(OUT str *_curDir)
{
	os_assert(_curDir!=NULL);
	_curDir->newbuf(300);
#ifdef OS_WIN
	char *p = _getcwd(_curDir->getbuf(), 300);
#else
	char *p = getcwd(_curDir->getbuf(), 300);
#endif // DEBUG

	
	os_assert(NULL!=p);
	_curDir->setstrlen(osc_strlen(_curDir->getbuf()));
}

void os_get_selfexe_fullpath(const char* _selfexename, OUT str *_curDir)
{
#ifdef OS_WIN
	char szFull[_MAX_PATH];
	::GetModuleFileName(NULL, szFull, sizeof(szFull));
	
	int len=osc_strlen(szFull);
	int i;
	for(i=len-1; i>=0; --i)
	{
		if(szFull[i]=='/' || szFull[i]=='\\')
		{
			szFull[i] = '\0';
			break;
		}
	}
	for(i=0; szFull[i]!='\0'; ++i)
	{
		if(szFull[i]=='\\')
			szFull[i]='/';
    }
	
	*_curDir = szFull;
#elif defined(OS_LINUX)
    char sysfile[15] = "/proc/self/exe";
    int  namelen = 300;
    char selfname[300];
	
    if ( -1 == readlink( sysfile,selfname,namelen) )
    {
        os_assert(false);
    }
	int len=osc_strlen(selfname);
	int i;
	for(i=len-1; i>=0; --i)
	{
		if(selfname[i]=='/' || selfname[i]=='\\')
		{
			selfname[i] = '\0';
			break;
		}
	}
	*_curDir = selfname;
#elif defined(OS_AIX)
	// ?¡§¡§?¨¢?which?¡§1¡§¡é?
        char stmp[300];
	osc_snprintf(stmp, sizeof(stmp)-1, "which %s > /tmp/authd_which", _selfexename);
	system(stmp);
	OsFile f;
	f.open_file_read_r("/tmp/authd_which");
	char stmp2[300];
	int len=f.fread_r(stmp2, sizeof(stmp2));
	os_assert(len>0);	
	stmp2[len]='\0';
	for(int i=len-1; i>=0; --i)
	{
		if(stmp2[i]=='/')
		{
			stmp2[i]='\0';
			break;
		}
	}
	if(stmp2[0]=='/')
	{
		*_curDir = stmp2;
		return;
	}
	else
	{
		printf("stmp2=[%s]\n", stmp2);
		int rc=os_chdir(stmp2);
		printf("chdir ret=[%d], error:%d/%s\n", rc, os_errno(), os_errstr());
		os_assert(rc==0);
		os_get_cwd(_curDir);
		return;
	}
#else
#error Unsupported  platform
#endif
}

int os_chdir(const char * _dir)
{
#ifdef _WIN32
	return _chdir(_dir);
#else
	return chdir(_dir);
#endif
}


#if 0
OsDir::OsDir()
{
#ifdef _WIN32
	hFile = -1;
	mcfile = new _finddata_t;
	mb_first = false;
#else
	mp_dir = NULL;
#endif
}


OsDir::~OsDir()
{
#ifdef _WIN32
	if(hFile!=-1)
		CloseDir();
	delete (struct _finddata_t*)mcfile;
#else
	if(mp_dir)
		CloseDir();
#endif
}

int OsDir::OpenDir_r(const str &_dir)
{
	ms_dir = _dir;
#ifdef _WIN32
	struct _finddata_t * cfile = (struct _finddata_t*)mcfile;

	str stmp=_dir;
	stmp += "/*";
	hFile = _findfirst(stmp, cfile);
	if(hFile <0 )
	{
		int err=os_errno();
		return -1;
	}

	ms_name = cfile->name;
	if(cfile->attrib & _A_SUBDIR)
		mb_dir = true;
	else
		mb_dir = false;
	ml_mtime = cfile->time_write;
	ml_fsize = cfile->size;
	mb_first = true;
	return 0;
#else
	mp_dir = opendir((const char*)_dir);
	if(NULL==mp_dir)
		return -1;
	else
		return 0;
#endif
}

int OsDir::StepNextChild_r()
{
#ifdef _WIN32
	if(mb_first)
	{
		mb_first = false;
		return 0;
	}
	struct _finddata_t * cfile = (struct _finddata_t*)mcfile;
	int ret=_findnext(hFile, cfile);
	if(ret<0)
		return ret;
	ms_name = cfile->name;
	if(cfile->attrib & _A_SUBDIR)
		mb_dir = true;
	else
		mb_dir = false;
	ml_mtime = cfile->time_write;
	ml_fsize = cfile->size;
	return 0;
#else
	struct dirent fileinfo;
	struct dirent *result=NULL;
	int ret;

	ret = readdir_r((DIR*)mp_dir, &fileinfo, &result);
	if(ret!=0)
		return -1;

	ms_name = fileinfo.d_name;
	if(fileinfo.d_type == DT_DIR)
		mb_dir = true;
	else
		mb_dir = false;

	struct    stat    file_stat;
	str tmpstr=ms_dir;
	tmpstr += fileinfo.d_name;
    if(stat((const char*)tmpstr, &file_stat)==-1)
		return -1;
	
	ml_fsize = file_stat.st_size;
	ml_mtime = file_stat.st_mtime;
	return 0;
#endif
}

void OsDir::CloseDir()
{
#ifdef _WIN32
	if(hFile!=-1)
	{
		_findclose(hFile);
		hFile = -1;
	}
#else
	if(mp_dir)
	{
		closedir((DIR*)mp_dir);
		mp_dir = NULL;
	}
#endif
}

DLL_EXPORT void os_dir_test()
{
	str tmpstr;
	os_get_selfexe_fullpath(&tmpstr);
	printf("os_get_selfexe_fullpath return[%s]", (const char*)tmpstr);

}
#endif
