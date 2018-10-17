#ifndef __H_DB_ACCESS_H__
#define __H_DB_ACCESS_H__
#include "common/os_type.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

typedef map<string, string> DataRow;//һ�����ݣ�keyΪ�ֶ�����valueΪ�ֶ�ֵ
typedef vector<DataRow> DataSet;//���ݼ�����Ӧ������ѯ���

bool db_connect(string ip, int port, string dbname, string user, string passwd);
//�������ȵ���db_init���ܽ��к�������ݿ����
bool db_reconnect();
DataSet db_query(string sql);//��ѯ
int db_update(string sql, bool commit);//ֱ��ִ��sql��䣬��Ϊinsert��update��������Ӱ�������
//int db_insert(string table, DataSet data, bool commit);//δʵ��
DataSet  db_proc(string _csProcName, int _procNameLen, string _csInParam, int _paramLen, int _outParaCount, bool commit);
int db_commit();

#endif
