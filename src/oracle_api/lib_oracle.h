
#ifndef _LIB_ORACLE_H_
#define _LIB_ORACLE_H_

#include "common/os_type.h"

#ifdef OS_WIN
	#include "oci/oci.h"
	#include "oci/ocidfn.h"
	#include "oci/ociapr.h"
#else
	#include "oci.h"
	#include "ocidfn.h"
	#include "ociapr.h"
#endif // OS_WIN




#define MAX_RET_COLNUMBER       32//查询结果集最大列数
#define MAX_COL_LEN             1100//查询结果集单列的最大长度
#define MAX_COL_NAME_LEN  64 //字段名的最大长度

struct T_OciCtx{
	OCIEnv        *envhp;
	OCIServer     *srvhp;
	OCISvcCtx     *svchp;
	OCISession    *authp;
	OCIError      *errhp;
	OCIStmt       *stmthp;
};

#define  MAX_STRING_LENGTH 2048


#define STATEMENTPARA_MAXLEN 2048
#define MAXSTPARANUMBER        1024
typedef struct {
    char cParamType;
    uint len;
    union{
        char sString[STATEMENTPARA_MAXLEN];
        long lNumber;
        double  fNumber;
    }oValue;
}tOCIStatementParam;


class C_OracleConn
{
public:
	C_OracleConn();
	~C_OracleConn();
public:
	int Connect(const char *server, const char *uid, const char *pwd);
	int DisConnect();

	int Update(const char *SQLString, uint32 len, uint16 WaitTime); //返回受影响的行数
	int Commit();
	int RollBack();
	int GetError();
    

	int Query( char *SQLString, uint32 len, uint16 WaitTime); 
	int FetchRow(); // <0 fail, >=0 success
	const char *GetFieldName(int index); // [0, mi_colcount-1]
	const char *GetField(int index); // [0, mi_colcount-1]
	int  GetColCount();
	
	int DoProc(char *SQLString, uint16 WaitTime); 

	int ParseProcStatement(char *sStatement, OUT tOCIStatementParam *Paras, OUT char* sResStatement, OUT int *pLen);


private:
	T_OciCtx *mp_OciCtx;
	int  mi_state;//

	int mi_sql_type;// 1 query, 2 update, 3 proc, 4 proc fetched
	//int  mi_rowcount;
	int  mi_colcount;
	char sqlResult[MAX_RET_COLNUMBER][MAX_COL_LEN];//??????
	char sqlResultField[MAX_RET_COLNUMBER][MAX_COL_NAME_LEN];//??????
	int   cbt[MAX_RET_COLNUMBER];//what's meaning?
public:
	str  ms_server;
	str  ms_user;
	str  ms_passwd;
	
	int   mi_affected_rows;
	char    szErrorMsg[200];
	int		dwErrCode;	

	tOCIStatementParam aras[MAXSTPARANUMBER];
    char  sql_str[MAX_STRING_LENGTH];
	char  sWkStr[STATEMENTPARA_MAXLEN];
};

#endif
