#include "db_access/db_access.h"
#include "db_api/db_api.h"
#include "common/os_log.h"

//数据库连接句柄
static void* gp_db = NULL;
string g_ip;
int g_port;
string g_dbname;
string g_user;
string g_passwd;

bool db_connect(string ip, int port, string dbname, string user, string passwd)
{
	g_ip = ip; g_port = port; g_dbname = dbname; g_user = user; g_passwd = passwd;
	char buf[255] = {0};
	int len = 255;
	gp_db = _db_connect(ip.c_str(), port, dbname.c_str(), user.c_str(), passwd.c_str(), buf, 255);
	os_log(FL, "_db_connect:%s", buf);
	if (gp_db == NULL)
		return false;
	return true;
}

bool db_reconnect()
{
	_db_disconnect(gp_db);
	char buf[255];
	int len = 255;
	gp_db = _db_connect(g_ip.c_str(), g_port, g_dbname.c_str(), g_user.c_str(), g_passwd.c_str(), buf, 255);
	if (gp_db == NULL)
		return false;
	return true;
}

DataSet db_query(string sql)
{
	DataSet queryData;
	int ret = _db_query(gp_db, sql.c_str(), sql.length());
	while (ret < 0)
	{
		if (db_reconnect())
		{
			os_log(FL, "db reconnect success!");
			ret = _db_query(gp_db, sql.c_str(), sql.length());
		}			
		else
		{
			os_log(FL, "db reconnect fail!");
		}	
	}
	if (ret == 0)
	{
		int colcount = _db_get_colcount(gp_db);
		while (_db_fetch_row(gp_db) == 0)
		{
			DataRow rowData;
			for (int i = 0; i < colcount; i++)
			{
				const char* fieldname = _db_get_field_name(gp_db, i);
				rowData[fieldname] = _db_get_field(gp_db, i);
			}
			queryData.push_back(rowData);
		}
	}
	return queryData;
}

int db_update(string sql, bool commit)
{
	int ret = _db_update(gp_db, sql.c_str(), sql.length(), commit);
	while (ret < 0)
	{
		if (db_reconnect())
		{
			os_log(FL, "db reconnect success!");
			ret = _db_update(gp_db, sql.c_str(), sql.length(), commit);
		}
		else
		{
			os_log(FL, "db reconnect fail!");
		}
	}
	return ret;
}

int db_insert(string table, DataSet data, bool commit)
{
	for (int i = 0; i < data.size(); i++)
	{
		DataRow::iterator it = data[i].begin();
		//拼装sql
		//Todo
		char sql[1024 * 10];
		//sprintf_s(sql, 1024 * 10 - 1, "insert into %s(%s) values()", table.c_str(), );		
		db_update(sql, false);
	}
	if(commit)
		db_commit();
	return _db_get_affected_rowcount(gp_db);
}

int db_commit()
{
	return _db_commit(gp_db);
}

DataSet  db_proc(string _csProcName, int _procNameLen, string _csInParam, int _paramLen, int _outParaCount, bool commit)
{
	DataSet queryData;
	int ret = _db_proc(gp_db, _csProcName.c_str(), _procNameLen, _csInParam.c_str(), _paramLen, _outParaCount, commit);
	if (ret == 0)
	{
		int colcount = _db_get_colcount(gp_db);
		while (_db_fetch_row(gp_db) == 0)
		{
			DataRow rowData;
			for (int i = 0; i < colcount; i++)
			{
				//const char* fieldname = _db_get_field_name(gp_db, i);
				char fieldname[64];
				snprintf(fieldname, 64, "%d", i);
				rowData[fieldname] = _db_get_field(gp_db, i);
			}
			queryData.push_back(rowData);
		}
	}
	return queryData;
}