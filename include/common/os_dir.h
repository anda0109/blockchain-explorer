
#ifndef _PLAT_OS_DIR_H_
#define _PLAT_OS_DIR_H_

#include "common/os_type.h"

// ---------- 公开接口
PLATAPI const char * os_home();	// 获取JENC的home目录

// 目录操作类
class PLATAPI OsDir
{
public:
	// 打开目录，可以填相对目录或者绝对目录。
	int OpenDir_r(const str &_dir);	

	// 读取下一个子节点，读取后，通过访问Name，IsDir，mb_dir，ModifyTime，FileSize字段来获取读取结果。
	int StepNextChild_r();
	
	// 关闭目录
	void CloseDir();	

	// 当前子节点的名称，可能是目录名或文件名。
	const str Name(){return ms_name;}	

	// 是否目录
	bool IsDir(){return mb_dir;}
	
	// 修改时间
	longint ModifyTime(){return ml_mtime;}
	
	// 文件大小
	longint FileSize(){return ml_fsize;}
public:
	OsDir();
	~OsDir();

private:
	str		ms_dir;
	str		ms_name;
	bool	mb_dir;
	longint	ml_mtime;	// 修改时间
	longint	ml_fsize;	// 文件大小

private:
#ifdef _WIN32
	long hFile;
	void *mcfile; // _finddata_t*
	bool mb_first;
#else
	void *mp_dir;	//	DIR * dp
#endif
};
 
// ---------- 非公开接口
PLATAPI void os_home_init();
PLATAPI void _os_set_home(const str &_home);
PLATAPI void os_get_cwd(OUT str *_curDir);
PLATAPI void os_get_selfexe_fullpath(const char*selfexename, OUT str *_curDir);
PLATAPI int os_chdir(const char* _dir);

#endif
