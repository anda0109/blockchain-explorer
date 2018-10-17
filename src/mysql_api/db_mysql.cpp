//#include "db_api/db_api.h"
#include "lib_mysql.h"
#include "common/os_libc.h"
//#include "plat/os_log.h"

#include <stdio.h>

#ifdef  DB_USE_MYSQL

void* _db_connect(const char *ip, int port, const char* dbname,  const char*user, const char* passwd, OUT char *_errstr, int _buflen)
{
	if(strcmp(passwd, "null")==0)
		passwd ="";

    MysqlClient* mysql_conn = new MysqlClient;
    
    int ret=mysql_conn->connect(ip,  port, user,  passwd,  dbname);
    if(ret<0)
    {
        //int error=mysql_conn->err_no();
		//strncpy(_errstr, mysql_conn->err_str(), _buflen-1);
		strncpy_s(_errstr, _buflen - 1,  mysql_conn->err_str(), strlen(mysql_conn->err_str()));
        delete mysql_conn;
        return NULL;
    }
    return mysql_conn;
}

void _db_disconnect(void * _hDb)
{
    MysqlClient* _hmysql = (MysqlClient*) _hDb;
    delete _hmysql;
}

// reconnect
int  _db_reconnect(void * _hDb)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
	int ret;
	ret=_hmysql->reconnect();
	
	return ret;
}

int _db_query(void *_hDb, const char*_sSql, int _sqlLen)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
    int ret = _hmysql->realquery(_sSql, _sqlLen);
    return ret;
}

int   _db_query_page(void *_hDb, const char*_csSql, int _sqlLen, int row_begin, int row_num)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
    int ret = _hmysql->querypage(row_begin, row_num, _csSql);
    return ret;
}


int   _db_update(void *_hDb, const char*_sSql, int _sqlLen, bool commit)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
    int ret = _hmysql->realquery(_sSql, _sqlLen);
	if(commit)
		_hmysql->commit();
	if(ret==-3)
		return _hmysql->affected_row_num();
    return ret;
}

int   _db_proc(void *_hDb, const char*_csProcName, int _procNameLen, const char*_csInParam, int _paramLen, int _outParaCount, bool commit)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
	
	// ×é×°sqlÓï¾ä
	int buf_len =_outParaCount*3 + _procNameLen + _paramLen + 10;
	char *sql_buf= new char[buf_len]; // call proc(pramlist);

	if(_outParaCount>0)
	{
		int pos=osc_snprintf_r(sql_buf, buf_len-1, buf_len - 1, "call %s(%s", _csProcName, _csInParam);
		for(int i=0; i<_outParaCount; ++i)
		{
			pos += osc_snprintf_r(sql_buf + pos, buf_len-1-pos, buf_len - 1 - pos, ",:%d", i);
		}
		pos += osc_snprintf_r(sql_buf + pos, buf_len-1-pos, buf_len - 1 - pos, ");");
	}
	else
	{
		int sqllen = osc_snprintf_r(sql_buf, buf_len-1, buf_len - 1, "call %s(%s);", _csInParam, _csInParam);
	}
	
    int ret = _hmysql->doproc(sql_buf, strlen(sql_buf)+1);
//	os_trace(FL, "recv proc[%s]", (const char*)sql_buf);
	if(commit)
		_hmysql->commit();
    
	delete [] sql_buf;
    return ret;
}

int _db_get_colcount(void *_hDb)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
    return _hmysql->col_num();
}


int _db_get_affected_rowcount(void *_hDb)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
    return _hmysql->affected_row_num();
}

int _db_fetch_row(void *_hDb)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
	
    return _hmysql->fetch_row();
}


const char *_db_get_field(void *_hDb, int _col)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
    return _hmysql->get_field(_col);
}

// commit
int _db_commit(void *_hDb)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
    int ret;
    ret=_hmysql->commit();
	
    return ret;
}

// rollback
int _db_rollback(void *_hDb)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
    int ret;
    ret=_hmysql->rollback();
	
    return ret;
}

int   _db_get_errno(void *_hDb)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
    return _hmysql->err_no();
}

const char* _db_get_errstr(void *_hDb)
{
    MysqlClient* _hmysql = (MysqlClient*)_hDb;
    return _hmysql->err_str();
}
#endif
