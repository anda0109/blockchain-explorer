
#ifndef _PLAT_OS_DIR_H_
#define _PLAT_OS_DIR_H_

#include "common/os_type.h"

// ---------- �����ӿ�
PLATAPI const char * os_home();	// ��ȡJENC��homeĿ¼

// Ŀ¼������
class PLATAPI OsDir
{
public:
	// ��Ŀ¼�����������Ŀ¼���߾���Ŀ¼��
	int OpenDir_r(const str &_dir);	

	// ��ȡ��һ���ӽڵ㣬��ȡ��ͨ������Name��IsDir��mb_dir��ModifyTime��FileSize�ֶ�����ȡ��ȡ�����
	int StepNextChild_r();
	
	// �ر�Ŀ¼
	void CloseDir();	

	// ��ǰ�ӽڵ�����ƣ�������Ŀ¼�����ļ�����
	const str Name(){return ms_name;}	

	// �Ƿ�Ŀ¼
	bool IsDir(){return mb_dir;}
	
	// �޸�ʱ��
	longint ModifyTime(){return ml_mtime;}
	
	// �ļ���С
	longint FileSize(){return ml_fsize;}
public:
	OsDir();
	~OsDir();

private:
	str		ms_dir;
	str		ms_name;
	bool	mb_dir;
	longint	ml_mtime;	// �޸�ʱ��
	longint	ml_fsize;	// �ļ���С

private:
#ifdef _WIN32
	long hFile;
	void *mcfile; // _finddata_t*
	bool mb_first;
#else
	void *mp_dir;	//	DIR * dp
#endif
};
 
// ---------- �ǹ����ӿ�
PLATAPI void os_home_init();
PLATAPI void _os_set_home(const str &_home);
PLATAPI void os_get_cwd(OUT str *_curDir);
PLATAPI void os_get_selfexe_fullpath(const char*selfexename, OUT str *_curDir);
PLATAPI int os_chdir(const char* _dir);

#endif
