#ifndef _PLAT_OS_IPC_H_
#define _PLAT_OS_IPC_H_

#include "os_type.h"

// �ֲ߳̾��洢
class PLATAPI OsTsd
{
	void* m_tsd;
public:
	OsTsd();
	~OsTsd();
	void* getData();
	void setData(void*_pData);
};

// ��������
class PLATAPI OsLock
{
	void* m_lock;
public:
	OsLock();
	~OsLock();
	void lock();
	void unlock();
};

// ��Ϣ����
#ifdef _WIN32
	typedef uint os_msq_t;
#else
	typedef int os_msq_t;
#endif

#ifdef _WIN32
	typedef unsigned int  os_thread_t;
	typedef unsigned long THREAD_RETURN_TYPE;
#else
	typedef int os_thread_t;
	typedef void *  THREAD_RETURN_TYPE;
#endif
PLATAPI os_msq_t os_msq_create(os_thread_t hThread);
PLATAPI int os_msq_send_r(os_msq_t _tRecvMsqId, int _ui32_check, os_thread_t _ui32FromThread, void* _pvParam3);
PLATAPI int os_msq_recv_r(os_msq_t _tRecvMsqId, int *_ui32_check, os_thread_t *_ui32FromThread, pvoid *oppv_Param3);
PLATAPI void os_msq_remove(os_msq_t hMsq);
PLATAPI void os_msq_stat(os_msq_t _tMsqId, int *opui32MsgNum, int *opui32MsgSize);

// �����ڴ�


// �߳�
#ifndef _WIN32
	#define __stdcall
#endif
typedef THREAD_RETURN_TYPE (__stdcall * _OS_THRAED_FUN)(void*);
PLATAPI void os_thread_create(
	_OS_THRAED_FUN _thread_fun   // �̺߳�����
	, void *_pvInitPara					// ����
	, uint32 _ui32_StackSize
	, OUT os_thread_t *oph_thread	// �����߳�ID
	, OUT os_msq_t *oph_msq          // �Ƿ񴴽���Ϣ����
	);
PLATAPI void os_thread_wait(os_thread_t _hThread, uint32 _timeout);
PLATAPI void os_thread_terminate(os_thread_t _hThread);
PLATAPI os_thread_t os_thread_getcurrent();

PLATAPI void os_thread_sleep(int timerinterval);

#endif
