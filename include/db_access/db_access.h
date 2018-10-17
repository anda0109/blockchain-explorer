#ifndef __H_DB_ACCESS_H__
#define __H_DB_ACCESS_H__
#include "common/os_type.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

typedef map<string, string> DataRow;//一行数据，key为字段名，value为字段值
typedef vector<DataRow> DataSet;//数据集，对应多条查询结果

bool db_connect(string ip, int port, string dbname, string user, string passwd);
//必须首先调用db_init才能进行后面的数据库操作
bool db_reconnect();
DataSet db_query(string sql);//查询
int db_update(string sql, bool commit);//直接执行sql语句，可为insert和update，返回受影响的行数
//int db_insert(string table, DataSet data, bool commit);//未实现
DataSet  db_proc(string _csProcName, int _procNameLen, string _csInParam, int _paramLen, int _outParaCount, bool commit);
int db_commit();

#endif
