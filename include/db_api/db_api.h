#ifndef _DB_API_H_
#define _DB_API_H_

// 连接数据库
void* _db_connect(const char *ip, int port, const char* dbname,  const char*user, const char* passwd,  char *errstr, int _buflen);
	
// 断开数据库连接
void  _db_disconnect(void * _hDb);
	
// 重连
int  _db_reconnect(void * _hDb);
	
// <0 fail, >=0 success
// 查询
int  _db_query(void *_hDb, const char*_csSql, int _sqlLen);

// 分页查询
int  db_query_page(void *_hDb, const char*_csSql, int _sqlLen, int row_begin, int row_num);

// 更新
int   _db_update(void *_hDb, const char*_sSql, int _sqlLen, bool commit);
	
// 存储过程
int   _db_proc(void *_hDb, const char*_csProcName, int _procNameLen, const char*_csInParam, int _paramLen, int _outParaCount, bool commit);
	
// 获取列数
int _db_get_colcount(void *_hDb);
	
// 获取受影响的行数
int _db_get_affected_rowcount(void *_hDb);

// 游标到结果集下一行
int _db_fetch_row(void *_hDb);

// 获取当前行字段名，列数从0开始
const char *_db_get_field_name(void *_hDb, int _col); // 0开始

// 获取当前行字段值，列数从0开始
const char *_db_get_field(void *_hDb, int _col); // 0开始

// commit
int _db_commit(void *_hDb);
	
// rollback
int _db_rollback(void *_hDb);
	
// 获取错误号
int   _db_get_errno(void *_hDb);
	
// 获取错误描述
const char* _db_get_errstr(void *_hDb);

#endif

