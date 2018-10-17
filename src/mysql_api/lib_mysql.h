
#ifndef _LIB_MYSQL_H_
#define _LIB_MYSQL_H_

//#include "plat/os_type.h"

#ifdef _WIN32
#include <windows.h>
#endif

//#include "plat/os_socket.h"



#ifdef OS_LINUX
#include "mysql.h"
#else
#include "mysql/mysql.h"
#endif

#ifndef  LIB_MYSQL_EXPORTS
#define  LIB_MYSQL_EXPORTS
#endif

#define MAX_RET_COLNUMBER       100
#define MAX_COL_LEN             600

class LIB_MYSQL_EXPORTS MysqlClient
{
public:
    MysqlClient();
    ~MysqlClient();
    
public:
    int connect(const char *_csIp, int _iPort, const char *_csUser, const char* _csPasswd, const char* _csDb);
    int disconnect();
    int realquery(const char *_csSql, int _iSqlLen);
	int doproc(const char *_csSql, int _iSqlLen);
	int querypage(int row_begin, int row_num, const char* _sql_str);
    int fetch_row();
    const char* get_field(int index);
	
	int reconnect();
	int commit();
	int rollback();
	
    int col_num();
	int affected_row_num();

    int err_no();
    const char* err_str();

private:
    MYSQL* mysql;
    MYSQL_RES* result;
    MYSQL_ROW  row;
	int mi_col_num;
    int mi_affected_row_num;
};


#endif

