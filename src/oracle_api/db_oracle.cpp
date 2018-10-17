//#include "db_access/db_access.h"
#include "lib_oracle.h"
//#include "plat/os_log.h"
#include "common/os_libc.h"
#include "db_api/db_api.h"
 
#include <stdio.h>
#include <string.h>

void* _db_connect(const char *ip, int port, const char* dbname,  const char*user, const char* passwd, OUT char *_errstr, int _buflen)
{
	str  server;
	server.nfmt(256, "(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=%s)(PORT=%d)))(CONNECT_DATA=(SERVICE_NAME=%s)))", ip, port, dbname);
	//server.nfmt(256, "%s/%s", ip, dbname);
    C_OracleConn* oracle_conn = new C_OracleConn;
    int ret=oracle_conn->Connect((const char*)server, user, passwd);
	if(ret<0)
    {
		if(_errstr)
			osc_strncpy(_errstr, _buflen, oracle_conn->szErrorMsg, _buflen-1);
        delete oracle_conn;
        return NULL;  
    }
    
    return oracle_conn;
}

void _db_disconnect(void * _hDb)
{
    C_OracleConn* _horacle = (C_OracleConn*) _hDb;
    delete _horacle;
}

// ÖØÁ¬
int  _db_reconnect(void * _hDb)
{
	os_assert(false);
	_db_disconnect(_hDb);
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;
    int ret =  _horacle->Connect(_horacle->ms_server, _horacle->ms_user, _horacle->ms_passwd);
	return ret;
}

int _db_query(void *_hDb, const char*_sSql, int _sqlLen)
{
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;
    int ret = _horacle->Query((char*)_sSql, _sqlLen, 0);
    return ret;
}

int   _db_update(void *_hDb, const char*_sSql, int _sqlLen, bool commit)
{
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;
    int ret = _horacle->Update(_sSql, _sqlLen, 0);
	if(commit)
		_horacle->Commit();
    return ret;
}

int   _db_proc(void *_hDb, const char*_csProcName, int _procNameLen, const char*_csInParam, int _paramLen, int _outParaCount, bool commit)
{
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;

	int buflen=_outParaCount*3 + _paramLen + _procNameLen + 64;
	char* buff = new char[buflen];
	

	if(_outParaCount>0)
	{
		int pos=osc_snprintf_r(buff, buflen-1, buflen - 1, "begin %s(%s", _csProcName, _csInParam);
		for(int i=0; i<_outParaCount; ++i)
		{
			pos += osc_snprintf_r(buff + pos, buflen-1-pos, buflen - 1 - pos, ",:%d", i);
		}
		pos += osc_snprintf_r(buff + pos, buflen-1-pos, buflen - 1 - pos, "); end;");
	}
	else
	{
		//int pos=
			osc_snprintf(buff, buflen, buflen, "begin %s(%s); end;", _csProcName, _csInParam);
	}
    int ret = _horacle->DoProc(buff,0);
	

	if(commit)
		_horacle->Commit();

	delete[] buff;
			
	return ret;
}

int _db_get_colcount(void *_hDb)
{
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;
    return _horacle->GetColCount();
}

int _db_get_affected_rowcount(void *_hDb)
{
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;

	return _horacle->mi_affected_rows;
}

int _db_fetch_row(void *_hDb)
{
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;
    int ret =  _horacle->FetchRow();
	return ret;
}

const char *_db_get_field_name(void *_hDb, int _col)
{
	C_OracleConn* _horacle = (C_OracleConn*)_hDb;
	return _horacle->GetFieldName(_col);
}

const char *_db_get_field(void *_hDb, int _col)
{
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;
    return _horacle->GetField(_col);  
}

// commit
int _db_commit(void *_hDb)
{
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;
    int ret =  _horacle->Commit();
	return ret;
}

// rollback
int _db_rollback(void *_hDb)
{
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;
    int ret =  _horacle->RollBack();
	return ret;
}


int   _db_get_errno(void *_hDb)
{
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;
    return _horacle->GetError();
}

const char* _db_get_errstr(void *_hDb)
{
    C_OracleConn* _horacle = (C_OracleConn*)_hDb;
    _horacle->GetError();
	return _horacle->szErrorMsg;
}

