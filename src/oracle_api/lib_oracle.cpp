
#include "lib_oracle.h"
#include "common/os_log.h"
#include "common/os_libc.h"

#include <stdio.h>
#include <stdlib.h>

int SQLFetch(T_OciCtx * ocihdbc);

C_OracleConn::C_OracleConn()
{
	mp_OciCtx=NULL;
	mi_state = 0;
	mi_sql_type = 0;
	mi_affected_rows=0;

	osc_memset(sqlResult, 0 , sizeof(sqlResult));
	osc_memset(sqlResultField, 0, sizeof(sqlResultField));

	// OCI_THREADED
    int ret = OCIInitialize(
		(ub4) OCI_THREADED, 
		(dvoid *)0, 
        (dvoid * (*)(dvoid *, size_t)) 0,
        (dvoid * (*)(dvoid *, dvoid *, size_t))0,
        (void (*)(dvoid *, dvoid *)) 0 );
	if(ret!=0)
    {
        //os_log(FL, "C_OracleConn::logon: OCIInitialize fail");
		printf("C_OracleConn::logon: OCIInitialize fail");
	}
}

C_OracleConn::~C_OracleConn()
{
	if(mp_OciCtx!=NULL) 
		DisConnect(); 

    //OCITerminate(OCI_SUCCESS);
	mp_OciCtx=NULL;
}

int C_OracleConn::Connect(const char *server, const char *uid, const char *pwd)
{
	ms_server = server;
	ms_user   = uid;
	ms_passwd = pwd;

	if(mp_OciCtx)
		DisConnect();

	mp_OciCtx = new T_OciCtx;
    osc_memset(mp_OciCtx, 0, sizeof(T_OciCtx));
	T_OciCtx *pOciCtx=(T_OciCtx*)mp_OciCtx;

	if (OCIEnvInit(&pOciCtx->envhp, (ub4) OCI_THREADED, (size_t) 0, (dvoid **) 0 ))
	{
		os_log(FL, "C_OracleConn::logon: OCIEnvInit fail");
		return OCI_ERROR;
	}

	if (OCIHandleAlloc((dvoid*)pOciCtx->envhp, (dvoid **) &pOciCtx->svchp,
		(ub4) OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0))
	{
		os_log(FL, "FAILED: OCIHandleAlloc");
		return OCI_ERROR;
	}
	
	if (OCIHandleAlloc((dvoid *) pOciCtx->envhp, (dvoid **) &pOciCtx->errhp,
		(ub4) OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0))
	{
		os_log(FL, "FAILED: OCIHandleAlloc");
		return OCI_ERROR;
	}
	
	if (OCIHandleAlloc((dvoid *) pOciCtx->envhp, (dvoid **) &pOciCtx->srvhp,
		(ub4) OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0))
	{
		os_log(FL, "FAILED: OCIHandleAlloc");
		return OCI_ERROR;
	}
	
	if (OCIHandleAlloc((dvoid *) pOciCtx->envhp, (dvoid **) &pOciCtx->authp,
		(ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0))
	{
		os_log(FL, "FAILED: OCIHandleAlloc");
		return OCI_ERROR;
	}

	// connect to tns
	if (OCIServerAttach(pOciCtx->srvhp, pOciCtx->errhp, (text *) server,
		(sb4) osc_strlen((char *)server), (ub4) OCI_DEFAULT))
	{
		GetError();
        DisConnect();
            
        os_log(FL, "FAILED: OCIServerAttach()\n");
        return OCI_ERROR;
	}
	mi_state = 1;
	// set user name
	if (OCIAttrSet((dvoid *) pOciCtx->authp, (ub4) OCI_HTYPE_SESSION,
		(dvoid *) uid, (ub4) osc_strlen((char *)uid),
		(ub4) OCI_ATTR_USERNAME, pOciCtx->errhp))
	{
		os_log(FL, "FAILED: OCIAttrSet");
		return OCI_ERROR;
	}

	// set password
	if (OCIAttrSet((dvoid *) pOciCtx->authp, (ub4) OCI_HTYPE_SESSION,
		(dvoid *) pwd, (ub4) osc_strlen((char *)pwd),
		(ub4) OCI_ATTR_PASSWORD, pOciCtx->errhp))
	{
		os_log(FL, "FAILED: OCIAttrSet");
		return OCI_ERROR;
	}
	
	// set the server attribute in the service context 
	if (OCIAttrSet((dvoid *) pOciCtx->svchp, (ub4) OCI_HTYPE_SVCCTX,
		(dvoid *) pOciCtx->srvhp, (ub4) 0, (ub4) OCI_ATTR_SERVER, pOciCtx->errhp))
	{
		os_log(FL, "FAILED: OCIAttrSet");
		return OCI_ERROR;
	}
	
	// log on 
	if (OCISessionBegin(pOciCtx->svchp, pOciCtx->errhp, pOciCtx->authp, (ub4) OCI_CRED_RDBMS,
		(ub4) OCI_DEFAULT))
	{
		GetError();

        DisConnect();

		os_log(FL, "FAILED: OCISessionBegin");
		return OCI_ERROR;
	}
	mi_state = 2;
	
	/* set the session attribute in the service context */
	if (OCIAttrSet((dvoid *) pOciCtx->svchp, (ub4) OCI_HTYPE_SVCCTX, (dvoid *) pOciCtx->authp,
		(ub4) 0, (ub4) OCI_ATTR_SESSION, pOciCtx->errhp))
	{
		os_log(FL, "FAILED: OCIAttrSet");
		return OCI_ERROR;
	}
	
	// get stmt
	if (OCIHandleAlloc((dvoid *) pOciCtx->envhp, (dvoid **) &pOciCtx->stmthp,
		(ub4) OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0))
	{
		os_log(FL, "FAILED: OCIHandleAlloc");
		return OCI_ERROR;
	}
	
	mi_colcount = 0;
	//mi_rowcount = 0;

	return OCI_SUCCESS;
}

int C_OracleConn::DisConnect()
{
	T_OciCtx *pOciCtx=(T_OciCtx*)mp_OciCtx;
	
	if(mp_OciCtx==NULL) 
		return -1;
	
	if(pOciCtx->stmthp)
		OCIHandleFree((dvoid *) pOciCtx->stmthp, (ub4) OCI_HTYPE_STMT); 
	pOciCtx->stmthp = NULL;
	
	if(mi_state==2)
	{
		OCISessionEnd(pOciCtx->svchp, pOciCtx->errhp, pOciCtx->authp, (ub4) 0);
		mi_state = 1;
	}
	
	if(mi_state==1)
	{
		OCIServerDetach(pOciCtx->srvhp, pOciCtx->errhp, (ub4) OCI_DEFAULT);
		mi_state = 0;
	}

	if(pOciCtx->srvhp)
		OCIHandleFree((dvoid *) pOciCtx->srvhp, (ub4) OCI_HTYPE_SERVER);
	pOciCtx->srvhp = NULL;
	
	if(pOciCtx->svchp)
		OCIHandleFree((dvoid *) pOciCtx->svchp, (ub4) OCI_HTYPE_SVCCTX);
	pOciCtx->svchp = NULL;

	if(pOciCtx->errhp)
		OCIHandleFree((dvoid *) pOciCtx->errhp, (ub4) OCI_HTYPE_ERROR);
	pOciCtx->errhp = NULL;

	if(pOciCtx->envhp)
		OCIHandleFree((dvoid *) pOciCtx->envhp, (ub4) OCI_HTYPE_ENV);  
	pOciCtx->envhp = NULL;

    delete pOciCtx;

    mp_OciCtx = NULL;
	return 0;
}

int  C_OracleConn::GetError()
{
	T_OciCtx *pOciCtx=(T_OciCtx*)mp_OciCtx;

	OCIErrorGet((dvoid *) pOciCtx->errhp, (ub4) 1, (text *) NULL, (sb4 *)&dwErrCode, 
		(text *)szErrorMsg, (ub4) sizeof(szErrorMsg), (ub4) OCI_HTYPE_ERROR);
	//os_trace(FL, " OCI ERROR CODE = [%d], [%s]", dwErrCode, szErrorMsg);

	if ( dwErrCode == 3113 || dwErrCode == 3114 || dwErrCode == 1012 || dwErrCode == 28 )
	{	
		// 与数据库通讯中断 
		return -1;  
	}
	return 0; // 其他错误
}	

int C_OracleConn::Commit()
{
	T_OciCtx *pOciCtx=(T_OciCtx*)mp_OciCtx;
	int ret;

	ret = OCITransCommit(pOciCtx->svchp, pOciCtx->errhp, OCI_DEFAULT);
	if (ret != OCI_SUCCESS){
		GetError(); 
		return -1;
	}
	return 0;
}

int C_OracleConn::RollBack()
{
	T_OciCtx *pOciCtx=(T_OciCtx*)mp_OciCtx;
	int ret;

	ret = OCITransRollback(pOciCtx->svchp, pOciCtx->errhp, OCI_DEFAULT);
	if (ret != OCI_SUCCESS){
		GetError();
		return -1;
	}
	return 0;
}

int  C_OracleConn::Update(const char *SQLString, uint len, uint16 WaitTime)
{
	int    ret;	
	//struct   timeval     StartTime = {0, 0};
	//struct   timeval     EndTime = {0, 0};
	int RowCount;
	T_OciCtx *pOciCtx=(T_OciCtx*)mp_OciCtx;

	//gettimeofday(&StartTime, NULL);	

	if(len==0)
		len = osc_strlen(SQLString);
  	
	// prepare sql 
	ret = OCIStmtPrepare(pOciCtx->stmthp, pOciCtx->errhp, (text *) SQLString,(ub4) len
		, (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	if (ret != OCI_SUCCESS) 
	{
		GetError();
		return -1;
	}

	// exec sql
	ret = OCIStmtExecute(pOciCtx->svchp, pOciCtx->stmthp, pOciCtx->errhp
				, (ub4) 1, (ub4) 0,(CONST OCISnapshot *) 0, (OCISnapshot *) 0,(ub4) OCI_DEFAULT);

	// 获取受delete insert update所影响的行数
	ret = OCIAttrGet((dvoid*)pOciCtx->stmthp, (ub4) OCI_HTYPE_STMT,
		(dvoid *) &RowCount, (ub4 *) 0,
		(ub4) OCI_ATTR_ROW_COUNT, (OCIError *) pOciCtx->errhp);
	//os_log(FL, "OCI_ATTR_ROW_COUNT is [%d]\n",OCI_ATTR_ROW_COUNT);
	//os_log(FL, "RowCount is [%d]",RowCount);
	if (ret != OCI_SUCCESS){
		GetError();
		return -1;
	}

	//gettimeofday(&EndTime, NULL);	
	//uTimes = (EndTime.tv_sec - StartTime.tv_sec) * 1000 + (EndTime.tv_usec - StartTime.tv_usec)/1000;	
	//record_trace_sql(uTimes, SQLString);
	
	mi_sql_type = 2;
	mi_affected_rows = RowCount;
	return RowCount;
}

// <0 fail, >0 rowcount
int C_OracleConn::Query(char *SQLString, uint len, uint16 WaitTime)
{
	ub4         ColumnCount;//列数目
	int RowCount;
	int ret;
	int i;
	T_OciCtx *pOciCtx=(T_OciCtx*)mp_OciCtx;

	if(len==0)
		len = osc_strlen(SQLString);
	
	// prepare sql 
	ret = OCIStmtPrepare(pOciCtx->stmthp, pOciCtx->errhp, (text *) SQLString,(ub4) len
		, (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	if (ret != OCI_SUCCESS) 
	{
		GetError();
		os_log(FL, "OCIStmtPrepare failed");
		return -1;
	}
	
	// exec sql
	ret = OCIStmtExecute(pOciCtx->svchp, pOciCtx->stmthp, pOciCtx->errhp
				, (ub4) 0, (ub4) 0,(CONST OCISnapshot *) 0, (OCISnapshot *) 0,(ub4) OCI_DEFAULT);
	if (ret != OCI_SUCCESS) 
	{
		GetError();
		os_log(FL, "OCIStmtExecute failed");
		return -2;
	}

	// 获取列数
	ret = OCIAttrGet(
		(dvoid*)pOciCtx->stmthp, 
		(ub4)OCI_HTYPE_STMT, 
		(dvoid*)&ColumnCount, 
		(ub4 *) 0, 
		(ub4)OCI_ATTR_PARAM_COUNT, 
		pOciCtx->errhp);
	if (ret != OCI_SUCCESS){
		GetError();
		return -4;
	}

	// bind col
	for(i=0; i<(int)ColumnCount; i++)
	{
        sb2 bindindex=-1;
        OCIDefine *bndhp = (OCIDefine *) 0;        
        ret=OCIDefineByPos  (
			pOciCtx->stmthp, 
			&bndhp, 
			pOciCtx->errhp,
            i+1, 
			(dvoid *) sqlResult[i], 
			(sb4) MAX_COL_LEN+1, 
			SQLT_STR,
            (dvoid *) &bindindex, 
			(ub2 *)0, 
			(ub2 *)0, 
			(ub4) OCI_DEFAULT
		);
        if (ret != OCI_SUCCESS){
			GetError();
			return -5;
		}

		//获取列名称，通过SQL语句处理句柄hStmt，获得域值处理参数param
		void* param = NULL;
		OCIParamGet(
			pOciCtx->stmthp, 
			OCI_HTYPE_STMT, 
			pOciCtx->errhp,
			(dvoid**)&param,
			(ub4)i + 1
		);
		char* ColumnName = NULL;
		ub4  ColumnNameLength;
		ret = OCIAttrGet(
			(dvoid*)param,
			OCI_DTYPE_PARAM,
			(dvoid**)&ColumnName,
			(ub4 *)&ColumnNameLength,
			OCI_ATTR_NAME, 
			pOciCtx->errhp
		);
		if (ret != OCI_SUCCESS) {
			GetError();
			return -4;
		}
		memset(sqlResultField[i], 0, sizeof(sqlResultField[i]));
		strncpy(sqlResultField[i], ColumnName, ColumnNameLength);
		//printf((char*)ColumnName);
	}
	
	RowCount=0;
	mi_colcount = ColumnCount;

	mi_sql_type = 1;
	return RowCount;
}

int C_OracleConn::FetchRow()
{
	T_OciCtx *pOciCtx=(T_OciCtx*)mp_OciCtx;
	
	if(mi_sql_type==1)
	{
		if (pOciCtx->stmthp == NULL) {
			//fprintf(stderr, "Invalid statement handle.\n");
			os_log(FL,  "[lib_oracle.cpp] ocihdbc->stmthp == NULL");
			return OCI_ERROR;
		}   
		
		int rc = OCIStmtFetch(pOciCtx->stmthp, pOciCtx->errhp, (ub4) 1, 
			(ub4) OCI_FETCH_NEXT, (ub4) OCI_DEFAULT);
		
		//os_log(FL,  "[lib_oracle.cpp] rc=[%d] errno_mesg[%s]", rc, get_errstr(ocihdbc->errhp));
		if (rc != OCI_SUCCESS) {
			/* check the real error, if it is something like null-string.
			just return as normal.
			*/
			sb4 errcode;
			int ret;
			ret = OCIErrorGet((dvoid *)pOciCtx->errhp, (ub4) 1,
				(text *) NULL, &errcode, NULL,
				0, OCI_HTYPE_ERROR);
			//os_log(FL,  "[lib_oracle.cpp] error ret=[%d] errcode[%d]", ret, (int)errcode);
			switch ((int)errcode) {
				
			case 1405:
				//fprintf(stderr, "Some Cols are null.\n");
				rc = 0;
				break;
			case 1406:
				//fprintf(stderr, "Some Cols are truncated.\n");
				rc = 0;
				break;
			default:
				break;
			}
		}
		//os_log(FL,  "[lib_oracle.cpp] SQLFetch return");
		return (int) rc;

	}
	else if(mi_sql_type==2)
		return -1;
	else if(mi_sql_type==3)
	{
		mi_sql_type = 4;
		return 0;
	}
	else
	{
		return -1;
	}
}

// index [0, mi_rowcount-1]
const char* C_OracleConn::GetFieldName(int index)
{
	if (index<MAX_RET_COLNUMBER)
		return sqlResultField[index];
	else
		return NULL;
}

// index [0, mi_rowcount-1]
const char* C_OracleConn::GetField(int index)
{
	if(index<MAX_RET_COLNUMBER)
		return sqlResult[index];
	else
		return NULL;
}

int C_OracleConn::DoProc( char *SQLString, uint16 WaitTime)
{
	int RowCount;
	int ret;
	int i;
	T_OciCtx *pOciCtx=(T_OciCtx*)mp_OciCtx;
    int BindParamNumber;
    int BindOuter=0;
    int len;
    OCIBind       *bndhp = (OCIBind *) 0;

    BindParamNumber = ParseProcStatement(SQLString, aras, sql_str, &len);
	// prepare sql 
	ret = OCIStmtPrepare(pOciCtx->stmthp, pOciCtx->errhp, (text *) sql_str,(ub4) len
		, (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	if (ret != OCI_SUCCESS) 
	{
		GetError();
		return -1;
	}

    for(i=0; i<BindParamNumber; i++)
    {
        switch(aras[i].cParamType)
        {
        case 'N':
            {
                ret = OCIBindByPos(
					(pOciCtx)->stmthp, 
					&bndhp,
					(pOciCtx)->errhp, 
					i+1, 
					&(aras[i].oValue)
                    , sizeof(long), SQLT_NUM,//SQLT_INT
                    (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                    (ub4) 0, (ub4 *) 0, (ub4)OCI_DEFAULT );
            }
            break;
        case 'F':
            {
                ret = OCIBindByPos((pOciCtx)->stmthp, &bndhp,(pOciCtx)->errhp, i+1, &(aras[i].oValue)
                    , sizeof(double), SQLT_FLT,
                    (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                    (ub4) 0, (ub4 *) 0, (ub4)OCI_DEFAULT );
            }
            break;
        case 'S':
            {
                ret = OCIBindByPos((pOciCtx)->stmthp, &bndhp,(pOciCtx)->errhp, i+1, &(aras[i].oValue)
                    , aras[i].len+1, SQLT_STR,
                    (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                    (ub4) 0, (ub4 *) 0, (ub4)OCI_DEFAULT );
            }
            break;
        case ':':
            {
                //ret = SQLBindParameter(pOciCtx, (UWORD)(i+1), SQL_PARAM_INPUT, SQL_C_CHAR, SQL_C_CHAR, MAX_COL_LEN+1, 0, sqlResult[BindOuter], MAX_COL_LEN+1, (SDWORD)0, -1);
               
                sb2 bindindex=-1;
                
                sqlResult[BindOuter][0]='\0';
                ret = OCIBindByPos(pOciCtx->stmthp, &bndhp, pOciCtx->errhp, i+1, 
                    (dvoid *) sqlResult[BindOuter], (sb4) MAX_COL_LEN+1
                    , SQLT_STR, (dvoid *) &bindindex, (ub2 *)0, (ub2 *)0,
                     (ub4) 0, (ub4 *) 0, (ub4) OCI_DEFAULT);

                BindOuter++;
            }
            break;
        default:
            //fprintf(stderr, "what is this statement parameter??\n");
			os_assert(false);
            break;
        }
        if (ret != OCI_SUCCESS){
			GetError();
			return -4;
        }
    }

	  ub2 stmttype = (ub2) 0;
  ub4 iters = (ub4) 1;

	  ret = OCIAttrGet((dvoid*)pOciCtx->stmthp, (ub4) OCI_HTYPE_STMT,
             (dvoid*)& stmttype, (ub4 *) 0,
             (ub4) OCI_ATTR_STMT_TYPE, (OCIError *) pOciCtx->errhp);

  if (ret == OCI_SUCCESS && stmttype == OCI_STMT_SELECT) {
    iters = (ub4) 0;
  } else {
    iters = (ub4) 1;
  }
  
					
	// exec sql
	ret = OCIStmtExecute(pOciCtx->svchp, pOciCtx->stmthp, pOciCtx->errhp
				, (ub4) iters, (ub4) 0,(CONST OCISnapshot *) 0, (OCISnapshot *) 0,(ub4) OCI_DEFAULT);
	if (ret != OCI_SUCCESS) 
	{
		GetError();
		return -2;
	}
	
	// 获取受delete insert update所影响的列数
	ret = OCIAttrGet((dvoid*)pOciCtx->stmthp, (ub4) OCI_HTYPE_STMT,
		(dvoid *) &RowCount, (ub4 *) 0,
		(ub4) OCI_ATTR_ROW_COUNT, (OCIError *) pOciCtx->errhp);
	if (ret != OCI_SUCCESS){
		GetError();
		return -3;
	}
/*
	//获取列名称
	//通过SQL语句处理句柄hStmt，获得域值处理参数param
	for (i = 0; i < (int)BindOuter; i++)
	{
		void* param;
		OCIParamGet(
			pOciCtx->stmthp,
			OCI_HTYPE_STMT,
			pOciCtx->errhp,
			(dvoid**)&param,
			(ub4)i + 1
		);
		char* ColumnName = NULL;
		ub4  ColumnNameLength;
		ret = OCIAttrGet(
			(dvoid*)param,
			OCI_DTYPE_PARAM,
			(dvoid**)&ColumnName,
			(ub4 *)&ColumnNameLength,
			OCI_ATTR_NAME,
			pOciCtx->errhp
		);
		if (ret != OCI_SUCCESS) {
			GetError();
			return -4;
		}
		strcpy_s(sqlResultField[i], ColumnName);
	}
	*/
	
	mi_affected_rows = RowCount;
	mi_colcount = BindOuter;
	mi_sql_type = 3;
	return 0;
}

int C_OracleConn::GetColCount()
{
	return mi_colcount;
}

int C_OracleConn::ParseProcStatement(char *sStatement, OUT tOCIStatementParam *Paras, OUT char* sResStatement, OUT int *pLen)
{
    char *pCMouse = sStatement;
    
    int  i, j = 0, nParamPos = 0;
    
    while((sResStatement[j++]=*(pCMouse))!='('&&*(pCMouse++)!='\0');
    sResStatement[j]=*(++pCMouse);
    while(*pCMouse!='\0')
    {
        i = 0;
        while(*(pCMouse)==' ')pCMouse++;
        switch( *pCMouse)
        {
        case ':':
            (Paras+nParamPos)->cParamType = ':';
            while(*(pCMouse)!=')'&&(sResStatement[j++]=*(pCMouse))!=','&&*(pCMouse++)!='\0');
            break;
        case ')':
            break;
        case '\'':
            (Paras+nParamPos)->cParamType = 'S';
            pCMouse++;
            while(*(pCMouse)!='\''&&*(pCMouse)!='\0')
                (Paras+nParamPos)->oValue.sString[i++]=*(pCMouse++);
            (Paras+nParamPos)->oValue.sString[i]='\0';
            (Paras+nParamPos)->len = i;

            while(*(pCMouse)!=','&&*(pCMouse)!=')')pCMouse++;
            sResStatement[j++]=':';
            sResStatement[j++]=(nParamPos%52<26?'A':'a')+nParamPos%26;
            sResStatement[j++]= '0'+nParamPos/26%10;
            if((*pCMouse)==',')
                sResStatement[j++]=',';
            break;
        default:
            (Paras+nParamPos)->cParamType = 'N';
            while(*(pCMouse)!=','&&*(pCMouse)!=')'&&*(pCMouse)!='\0')
                sWkStr[i++]=*(pCMouse++);
            sWkStr[i]='\0';
            (Paras+nParamPos)->oValue.lNumber = atol(sWkStr);
			//(Paras + nParamPos)->oValue.fNumber = atof(sWkStr);
            sResStatement[j++]=':';
            sResStatement[j++]=(nParamPos%52<26?'A':'a')+nParamPos%26;
            sResStatement[j++]= '0'+nParamPos/26%10;
            if((*pCMouse)==',')
                sResStatement[j++]=',';
            break;
        }
        nParamPos++;
        if( *pCMouse == ')' )
        {
            int len;
            //len=sprintf(&sResStatement[j], "%s", pCMouse);
			len = snprintf(&sResStatement[j], strlen(pCMouse)+1, "%s", pCMouse);
            *pLen = j+len;
            break;
        }
        pCMouse++;
    }

    /*
    for(i = 0; i < nParamPos; i++)
    printf("result : %c, %d[%s] at <%d>\n", (Paras+nParamPos)->cParamType, (Paras+nParamPos)->oValue.lNumber, (Paras+nParamPos)->oValue.sString, i);
    printf("and res: [%s]\n", sResStatement);
    */
    return nParamPos;
}

int SQLFetch(T_OciCtx * ocihdbc)
{
	sword rc = (sword)0;
	
	if (ocihdbc->stmthp == NULL) {
		//fprintf(stderr, "Invalid statement handle.\n");
		os_log(FL,  "[lib_oracle.cpp] ocihdbc->stmthp == NULL");
		return OCI_ERROR;
	}   
	
	rc = OCIStmtFetch(ocihdbc->stmthp, ocihdbc->errhp, (ub4) 1, 
		(ub4) OCI_FETCH_NEXT, (ub4) OCI_DEFAULT);

	//os_log(FL,  "[lib_oracle.cpp] rc=[%d] errno_mesg[%s]", rc, get_errstr(ocihdbc->errhp));
	if (rc != OCI_SUCCESS) {
	/* check the real error, if it is something like null-string.
	just return as normal.
		*/
		sb4 errcode;
		int ret;
		ret = OCIErrorGet((dvoid *)ocihdbc->errhp, (ub4) 1,
			(text *) NULL, &errcode, NULL,
			0, OCI_HTYPE_ERROR);
        //os_log(FL,  "[lib_oracle.cpp] error ret=[%d] errcode[%d]", ret, (int)errcode);
		switch ((int)errcode) {
			
		case 1405:
			//fprintf(stderr, "Some Cols are null.\n");
			rc = 0;
			break;
		case 1406:
			//fprintf(stderr, "Some Cols are truncated.\n");
			rc = 0;
			break;
		default:
			break;
		}
		return -1;
	}
    //os_log(FL,  "[lib_oracle.cpp] SQLFetch return");
	return (int) rc;
	//return 0;
}
