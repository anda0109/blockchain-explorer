#ifndef _DB_API_H_
#define _DB_API_H_

// �������ݿ�
void* _db_connect(const char *ip, int port, const char* dbname,  const char*user, const char* passwd,  char *errstr, int _buflen);
	
// �Ͽ����ݿ�����
void  _db_disconnect(void * _hDb);
	
// ����
int  _db_reconnect(void * _hDb);
	
// <0 fail, >=0 success
// ��ѯ
int  _db_query(void *_hDb, const char*_csSql, int _sqlLen);

// ��ҳ��ѯ
int  db_query_page(void *_hDb, const char*_csSql, int _sqlLen, int row_begin, int row_num);

// ����
int   _db_update(void *_hDb, const char*_sSql, int _sqlLen, bool commit);
	
// �洢����
int   _db_proc(void *_hDb, const char*_csProcName, int _procNameLen, const char*_csInParam, int _paramLen, int _outParaCount, bool commit);
	
// ��ȡ����
int _db_get_colcount(void *_hDb);
	
// ��ȡ��Ӱ�������
int _db_get_affected_rowcount(void *_hDb);

// �α굽�������һ��
int _db_fetch_row(void *_hDb);

// ��ȡ��ǰ���ֶ�����������0��ʼ
const char *_db_get_field_name(void *_hDb, int _col); // 0��ʼ

// ��ȡ��ǰ���ֶ�ֵ��������0��ʼ
const char *_db_get_field(void *_hDb, int _col); // 0��ʼ

// commit
int _db_commit(void *_hDb);
	
// rollback
int _db_rollback(void *_hDb);
	
// ��ȡ�����
int   _db_get_errno(void *_hDb);
	
// ��ȡ��������
const char* _db_get_errstr(void *_hDb);

#endif

