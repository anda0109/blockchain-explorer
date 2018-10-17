
#include "common/os_ipc.h"
#include "common/os_errno.h"
#include "common/os_log.h"
#include "common/os_libc.h"

#ifdef OS_WINDOWS
	#include <windows.h>
#else
	#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
       #include <sys/msg.h>
#include <unistd.h>

#endif

// 线程局部存储
class CTsd
{
#ifdef  OS_WINDOWS
	uint32 m_tsd;
#else
	pthread_key_t m_tsd;
#endif

public:
	CTsd()
	{
#ifdef OS_WINDOWS
		m_tsd = TlsAlloc();
		os_assert(m_tsd!=TLS_OUT_OF_INDEXES);
#else
		int ret=pthread_key_create(&m_tsd, NULL);
		os_assert(ret==0);
#endif
	}
	~CTsd()
	{
#ifdef OS_WINDOWS
		BOOL r=TlsFree(m_tsd);
		os_assert(r!=0);
#else
		int r=pthread_key_delete(m_tsd);
		os_assert(r==0);
#endif
	}
	void* getData()
	{
#ifdef OS_WINDOWS
		return TlsGetValue(m_tsd);
#else
		return pthread_getspecific(m_tsd);
#endif
	}
	void setData(void*_pData)
	{
#ifdef OS_WINDOWS
		BOOL r=TlsSetValue(m_tsd,(void *)_pData);
		os_assert(r!=0);
#else
		int r=pthread_setspecific(m_tsd, _pData);
		os_assert(r==0);
#endif
	}
};

OsTsd::OsTsd()
{
	m_tsd = (void*) new CTsd();
}

OsTsd::~OsTsd()
{
	CTsd *ptsd=(CTsd*)m_tsd;

	delete ptsd;
}

void* OsTsd::getData()
{
	CTsd *ptsd=(CTsd*)m_tsd;
	return ptsd->getData();
}

void OsTsd::setData(void*_pData)
{
	CTsd *ptsd=(CTsd*)m_tsd;
	ptsd->setData(_pData);
}

class C_Lock
{
public:
    C_Lock()
	{
#ifdef _WIN32
		InitializeCriticalSection(&m_CriticalSection);    
#else
		pthread_mutex_init(&m_Mutex, NULL);
#endif
	}
    ~C_Lock()
	{
#ifdef _WIN32
		DeleteCriticalSection(&m_CriticalSection);
#else
		pthread_mutex_destroy(&m_Mutex);
#endif
	}
    void Lock()
	{
#ifdef _WIN32
		EnterCriticalSection(&m_CriticalSection);
#else
		pthread_mutex_lock(&m_Mutex);
#endif
	}
    void Unlock()
	{
#ifdef _WIN32
		LeaveCriticalSection(&m_CriticalSection);
#else
		pthread_mutex_unlock(&m_Mutex);
#endif
	}

private:
#ifdef _WIN32
    CRITICAL_SECTION m_CriticalSection;
#else
    pthread_mutex_t  m_Mutex;
#endif
};

OsLock::OsLock()
{
	m_lock = new C_Lock();
}
OsLock::~OsLock()
{
	C_Lock *pLock=(C_Lock*)m_lock;
	delete pLock;
}

void OsLock::lock()
{
	C_Lock *pLock=(C_Lock*)m_lock;
	pLock->Lock();
}

void OsLock::unlock()
{
	C_Lock *pLock=(C_Lock*)m_lock;
	pLock->Unlock();
}

os_msq_t os_msq_create(os_thread_t msq_key)
{
#ifdef OS_WINDOWS
	return msq_key;
#else
    os_msq_t MsqId;

    MsqId = msgget(msq_key, 0666|IPC_CREAT);	//|IPC_EXCL);//_ALLOC);
	if(-1==MsqId)
	{
		os_log(FL, "msgget fail, err:%d/%s", os_errno(), os_errstr());
	}
	os_assert(MsqId!=-1);
	return MsqId;
#endif
}

#ifdef _WIN32
static HANDLE GetThreadHandle(os_thread_t _hThread)
{
	typedef HANDLE (__stdcall*OPENTHREAD) (DWORD dwFlag, BOOL bUnknow, DWORD dwThreadId);
	HMODULE hMod=LoadLibrary("kernel32.dll");
	OPENTHREAD lpfnOpenThread = (OPENTHREAD)GetProcAddress(hMod,"OpenThread");  
    HANDLE hand=lpfnOpenThread(THREAD_ALL_ACCESS, FALSE, _hThread);
	CloseHandle(hand); 
	FreeLibrary(hMod);
	return hand;
}
#endif

/*
struct msgbuf
{
    long int mtype;             // type of received/sent message 
    char mtext[1];              // text of the message 
};
*/
int os_msq_send_r(os_msq_t _tRecvMsqId, int _ui32_check, os_thread_t _ui32FromThread, void* _pvBody)
{
	//os_trace(FL, "send to %d, flag:%d check:%d msg:%p", _tRecvMsqId,_ui32_check,  _ui32FromThread, _pvBody);
#ifdef OS_WINDOWS
    if(PostThreadMessage(_tRecvMsqId, (UINT)_ui32_check, (WPARAM)_ui32FromThread, (LPARAM)_pvBody)==false)
    {
		int err=os_errno();
		os_log(FL, "send_msg PostThreadMessage(%d) from(%d) fail[%d][%s]"
			, _tRecvMsqId, GetCurrentThreadId(), os_errno(), os_errstr());
		while(err==1444 &&  _tRecvMsqId!=0)
		{
			HANDLE hThread=GetThreadHandle(_tRecvMsqId);
			if(hThread ==INVALID_HANDLE_VALUE)
			{
				break;
			}
			else 
			{
				Sleep(10);
				if(PostThreadMessage(_tRecvMsqId, (UINT)_ui32_check, (WPARAM)_ui32FromThread, (LPARAM)_pvBody)==false)
				{
					err=os_errno();
					continue;
				}
				else
				{
					return 0;
				}
			}
		}

		return -1;
    }
	return 0;
#else
	msgbuf *mb;
	char buf[sizeof(msgbuf)+sizeof(int)*2+sizeof(void*)] = {0};
	int flag = 0;
	int ret;
	
	mb = (struct msgbuf *) buf;

    *(int*)mb->mtext = _ui32_check;
    *(os_thread_t*)(mb->mtext+sizeof(int)) = _ui32FromThread;
    *(void**)(mb->mtext+sizeof(int)+sizeof(os_thread_t)) = _pvBody;

	//memcpy(mb->mtext,buf,len);
	
	mb->mtype = 1;
	//if (timeout==0) 
        flag |= IPC_NOWAIT;
	ret=msgsnd(_tRecvMsqId,mb,sizeof(int)+sizeof(os_thread_t)+sizeof(void*),flag);
	if (ret != 0)
    {
		os_log(FL, "!!!send_msg msgsnd(%d) fail[%d][%s]"
			, _tRecvMsqId, os_errno(), os_errstr());
		return -1;
    }
	return 0;
#endif
}

// 同步等待
int os_msq_recv_r(os_msq_t _tRecvMsqId, int *_ui32_check, os_thread_t *_ui32FromThread, pvoid *oppv_Param3)
{
#ifdef OS_WINDOWS

	// 如果缓冲区有数据，peek其他消息。并返回缓冲区的第一个消息。


	if(_tRecvMsqId)
	{	;}

    MSG msg;
    BOOL bRet;
    bRet = GetMessage(&msg, NULL, 0, 0);
    if(bRet > 0)
    {
		*_ui32_check = (int)msg.message;
		*_ui32FromThread = (os_thread_t)msg.wParam;
		*oppv_Param3 = (void*)msg.lParam;
    }
	else //if(bRet<=0) // 出错了
    {
		os_log(FL, "!!!recv_msg GetMessage fail, cur msq(%d) fail[%d][%s]"
			, GetCurrentThreadId(), os_errno(), os_errstr());
		return -1;
    }

	// 在继续peek其他消息，并放入队列。可以用来计算消息队列积压。

//	os_trace(FL, "recv %d, flag:%d check:%d msg:%p", _tRecvMsqId, *opui32_Param1, *opui32_Param2, *oppv_Param3);
#else
	msgbuf *mb;
	char buf[sizeof(msgbuf)+sizeof(int)+sizeof(os_thread_t)+sizeof(void*)] = {0};
	int flag = 0;
	int rtn=-1;
	
	osc_memset(buf,0,sizeof(buf));
	mb = (struct msgbuf *)buf;

    /*
    flag |= IPC_NOWAIT;
	flag |= MSG_NOERROR;
	rtn=msgrcv(_tRecvMsqId,mb,sizeof(UINT)*2, 1, flag);
    */
	if (rtn == -1) 
	{
        flag=0;
    	flag |= MSG_NOERROR;
	    rtn=msgrcv(_tRecvMsqId,mb,sizeof(int)+sizeof(os_thread_t)+sizeof(void*), 0, flag);
        if (rtn == -1) {
            os_log(FL, "!!!recv_msg fail, msqid[%d] err[%d] msg[%s]"
                , _tRecvMsqId, os_errno(), os_errstr());
            return -1;
        }
	}
    *_ui32_check = *(int*)mb->mtext;
    *_ui32FromThread = *(os_thread_t*)(mb->mtext+sizeof(int));
    *oppv_Param3 = *(void**)(mb->mtext+sizeof(int)+ sizeof(os_thread_t));
#endif
	return 0;
}

void os_msq_remove(os_msq_t _tMsqId)
{
#ifdef OS_WINDOWS
	if(_tMsqId)
		return;
#else
	struct msqid_ds buf;
    if(msgctl(_tMsqId, IPC_RMID, &buf) == -1)
    {
		os_log(FL, "msgctl ICP_RMID(%d) fail, [%d][%s]", _tMsqId, os_errno(), os_errstr());
		return;
    }
#endif
}

void os_msq_stat(os_msq_t _tMsqId, int *opui32MsgNum, int *opui32MsgSize)
{
#ifdef OS_WINDOWS
    if(opui32MsgNum)
        *opui32MsgNum =0;
    if(opui32MsgSize)
        *opui32MsgSize =0;
	if(_tMsqId)
		return;
#else
    struct msqid_ds buf;

    if(msgctl(_tMsqId, IPC_STAT, &buf) != -1)
    {
        if(opui32MsgNum)
            *opui32MsgNum =buf.msg_qnum;
        if(opui32MsgSize)
            *opui32MsgSize =buf.msg_cbytes;
    }
#endif
}

void os_thread_create( 				// 返回值为线程标识
	  _OS_THRAED_FUN _thread_fun   // 线程函数体
      , void *_pvInitPara					// 参数
	  , uint32 _ui32_StackSize
	  , OUT os_thread_t *oph_thread
	  , OUT os_msq_t *oph_msq          // 是否创建消息队列
) 
{
#ifdef OS_WINDOWS
	DWORD ThreadId;
	HANDLE hRet= CreateThread(NULL, _ui32_StackSize, _thread_fun, _pvInitPara, 0, &ThreadId);
    if(NULL==hRet)
	{
        os_assert(false);
		return;
	}

	if(oph_thread)
		*oph_thread = ThreadId;
	if(oph_msq)
		*oph_msq = ThreadId;
	return;
#else
//	int ret;
    pthread_t   dwThreadId;
	pthread_attr_t      thread_attr;

    // create thread
	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setscope(&thread_attr, PTHREAD_SCOPE_SYSTEM);
	if (_ui32_StackSize > 0)
		pthread_attr_setstacksize(&thread_attr, _ui32_StackSize);
	if (0 != pthread_create(&dwThreadId, &thread_attr, 
			(void *(*)(void *))_thread_fun, _pvInitPara)) 
    {
        os_assert(false);
		return;
	} 
    else 
	{
		if(oph_thread)
			*oph_thread = dwThreadId;
	}

	// create msq 
    if(oph_msq)
    {
        *oph_msq = os_msq_create(dwThreadId);
		if(*oph_msq<0)
		{
			os_assert(false);
			return;
		}
    }
	return;
#endif
}

void os_thread_wait(os_thread_t _hThread, uint32 _timeout)
{
#ifdef _WIN32
    DWORD dwWaitResult;
    HANDLE hand=GetThreadHandle(_hThread);
	dwWaitResult = WaitForSingleObject(hand, _timeout);
	CloseHandle(hand);

    switch (dwWaitResult) 
    { 
    case WAIT_OBJECT_0: 
        {
            return;
        }
        break; 
    case WAIT_TIMEOUT:
        {
            os_thread_terminate(_hThread);
        }
        break;
    default:
        os_log(FL,  "!!!os_wait_thread_quit WaitForSingleObject fail[%d][%s]", os_errno(), os_errstr());
        break; 
    }
#else
    pthread_join(_hThread, NULL);
#endif

}

void os_thread_terminate(os_thread_t _hThread)
{
#ifdef _WIN32
	//DWORD dwWaitResult;
    HANDLE hand=GetThreadHandle(_hThread);
    BOOL ret=TerminateThread(hand, 0);
	if(ret==FALSE)
	{
		//int a=os_errno();
	}
	CloseHandle(hand);
#else
    pthread_cancel(_hThread);
    if(_hThread>=0)
    {
        struct msqid_ds ds;
        if(msgctl(_hThread,IPC_STAT,&ds)==-1)	
        {
            os_log(FL, "!!!os_terminate_thread::msgctl(%d) fail, [%d][%]\n", _hThread, os_errno(), os_errstr());
            return;//(-1);
        }
        msgctl(_hThread,IPC_RMID,&ds);
    }
#endif
}

os_thread_t os_thread_getcurrent()
{
#ifdef _WIN32
    return (os_thread_t)GetCurrentThreadId();
#else
	os_thread_t thid=pthread_self();
	if(thid==0)
		thid=getpid();
    return thid;
#endif
}


#ifdef _WIN32
void os_thread_sleep(int timerinterval)
{
	Sleep(timerinterval);
}

#else
void os_thread_sleep(int timerinterval)
{
    struct timeval timeout;
    
    if (timerinterval <= 0 ) 
    {
        return ;
    }
    if (timerinterval < 1000)
    {
        timeout.tv_sec  = 0;
        timeout.tv_usec = timerinterval*1000;
    }
    else
    {
        timeout.tv_sec  = timerinterval/1000;	
        timeout.tv_usec = (timerinterval%1000)*1000;
    }
    select(0,NULL,NULL,NULL,&timeout);
    //nanosleep(&timeout,&timeo);
}
#endif
