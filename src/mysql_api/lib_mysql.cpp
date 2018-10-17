
#include "lib_mysql.h"
#include "common/os_libc.h"
//#include "plat/os_log.h"

MysqlClient::MysqlClient()
{
    mysql = mysql_init((MYSQL*)0);
    mi_col_num = 0;
	mi_affected_row_num = 0;
    result  = NULL;
}

MysqlClient::~MysqlClient()
{
    disconnect();
}

int MysqlClient::connect(const char *_sIp, int _iPort, const char *_sUser, const char* _sPasswd, const char* _db)
{
	//os_assert(_iPort>0);

    if (!mysql_real_connect(mysql, _sIp, _sUser, _sPasswd, _db, _iPort, NULL, CLIENT_MULTI_RESULTS))//CLIENT_MULTI_STATEMENTS|
    {
        return -1;
    }

	mysql_set_character_set(mysql,"utf8");

    //my_bool ret=mysql_autocommit(mysql, 0);// disable auto commit
    /*if(!mysql_autocommit(mysql, 0))
	{
		return -1;
	};*/

    return 0;
}

int MysqlClient::disconnect()
{
    if(NULL!=result)
    {
        mysql_free_result(result);
    }
    mysql_close(mysql);
    return 0;
}

int MysqlClient::realquery(const char* SQLString, int length)
{
    if(NULL!=result)
    {
        mysql_free_result(result);
        result = NULL;
    }

    if (length ==0)
    {
        return -1;
    }
    else
    {
        if (mysql_real_query(mysql, SQLString, length))
        {
            //printf("%s",mysql_error(mysql));
            return -2;
        }
        else
        {
			mi_affected_row_num = (int)mysql_affected_rows(mysql);
            result =  mysql_store_result(mysql);
            if (NULL == result)
            {
                /*if (mysql_field_count(mysql) == 0)
                    return 0;
                else*/
                    return -3;
            }
			mi_col_num = mysql_num_fields(result);
            
            return 0;
        }
    }
}

int MysqlClient::doproc(const char* SQLString, int length)
{
    int i ;
    int out_param_num=0; // 输出参数个数
	char sql_text[255]={0};
	
    //APC_DEBUG "[lib_mysql.cpp]*********1 ******SQLString[%s] len[%d]", SQLString, length);
    if(NULL!=result)
    {
        mysql_free_result(result);
        result = NULL;
    }
    
    if (length ==0)
    {
        return -1;
    }

    //mysql->server_status&= ~SERVER_MORE_RESULTS_EXISTS;
    
    //strncpy(sql_text, SQLString, sizeof(sql_text));
	//strncpy(sql_text, SQLString, sizeof(sql_text));
	strncpy_s(sql_text, sizeof(sql_text), SQLString, strlen(SQLString));

    sql_text[0] = 'c';
    sql_text[1] = 'a';
    sql_text[2] = 'l';
    sql_text[3] = 'l';
    sql_text[4] = ' ';

    //APC_DEBUG "[lib_mysql.cpp]*********2 sql_text[%s]******", sql_text);
    i=0;
    while(sql_text[i])
    {
		if(sql_text[i] == ':')
		{
			if(i >= 2 && (sql_text[i-1]!=',' || sql_text[i-2]==','))
			{
				i++;
				continue;
			}
			/*if(i >= 2 && (sql_text[i-1]==',' || sql_text[i-2]==','))
			{
					sql_text[i] = '@';
					i++;
			}*/
		/*	if(sql_text[i+1]>='0' && sql_text[i+1]<='9' )
			{
				sql_text[i] = '@';
				++out_param_num;
				++i;
			}
        */
            if(sql_text[i+1]>='0' && sql_text[i+1]<='9' )
			{
				sql_text[i] = '@';
				++out_param_num;
				++i;
			}
		}
        i++;
    }

    //APC_DEBUG "[lib_mysql.cpp]*********3 sql_text[%s]******", sql_text);
    char *pend=strstr(sql_text,"end;");
    if(NULL!=pend)
    {
       pend[0]=pend[1]=pend[2]=pend[3]=' ';
    }
    //APC_DEBUG "[lib_mysql.cpp]*********4 sql_text[%s]******", sql_text);

    char *pcommit=strstr(sql_text,"commit;");
    if(NULL!=pcommit)
    {
       pcommit[0]=pcommit[1]=pcommit[2]=pcommit[3]=' ';
       pcommit[4]=pcommit[5]=pcommit[6]=' ';
    }
    //APC_DEBUG "[lib_mysql.cpp]*********41 sql_text[%s]******", sql_text);
	//os_trace(FL,"recv PROC[%s]",(const char*)sql_text);
    if (mysql_real_query(mysql, sql_text, length))
    {
    	//os_error(FL,"[lib_mysql.cpp]*********5 ******mysql_error[%s]", mysql_error(mysql));
        return -2;
    }
    else
    {
        result =  mysql_store_result(mysql);
        if (NULL == result)
		{
            if (mysql_field_count(mysql) != 0)
            {
				//os_error(FL,"[lib_mysql.cpp]*********6 ******mysql_error[%s]", mysql_error(mysql));
				return -3;
            }
            mi_col_num = 0;
        }
        else
		{
        
			mi_col_num = mysql_num_fields(result);
        }
        //APC_DEBUG "[lib_mysql.cpp]*********[[[[success!!!]]] ******\n\n");
    }
    if(out_param_num==0)
       return 0;
     
    // 查询结果
    char select_str[1024]="select ";
    int  len=strlen("select ");
    for(i=0; i<out_param_num; ++i)
    {
       int tmp_len=osc_snprintf_r(select_str+len,sizeof(select_str)-1-len, sizeof(select_str) - 1 - len, "@%d,",i);
       len += tmp_len;
    }
    select_str[len-1]=';';
    //APC_DEBUG "[%s],len[%d]", select_str, len);
    
//    if(NULL!=result)
 //       mysql_free_result(result);


//add by tang
	do
    {
		result=mysql_store_result(mysql);
		mysql_free_result(result);
    }while(!mysql_next_result(mysql)); // to solve the "2014:Commands out of sync; " problem
//add by tang end


       
    if (mysql_real_query(mysql, select_str, len))
    {
        //os_error(FL,"[lib_mysql.cpp]*********55 ******mysql_error[%s]", mysql_error(mysql));
        return -1;
    }

    result =  mysql_store_result(mysql);
	
    if (NULL == result)
    {
        //os_error(FL,"[lib_mysql.cpp]*********66 ******mysql_error[%s]", mysql_error(mysql));
        return -3;
    }
    mi_col_num = mysql_num_fields(result);
    return 0;
}
/*
// 查询结果
char select_str[1024]="select ";
int  len=strlen("select ");
for(i=0; i<out_param_num; ++i)
{
	int tmp_len=snprintf(select_str+len,sizeof(select_str)-1-len, "@%d,",i+1);
	len += tmp_len;
}
select_str[len-1]=';';
APC_DEBUG "[%s],len[%d]", select_str, len);

if(NULL!=result)
mysql_free_result(result);

if (mysql_real_query(mysql, select_str, len))
{
	return -1;
}

result =  mysql_store_result(mysql);
if (NULL == result)
{
	return -3;
}
mui32_col_num = mysql_num_fields(result);
    return 0;
	*/
// 返回查询结果行数
int MysqlClient::querypage(int row_begin, int row_num, const char* _sql_str)
{
	int sql_len=0;
	//a<char>	sqlstr(2048, 2048);
	char sqlstr[2048 * 2048];
	//os_assert(row_num>0);
	
    if(NULL!=result)
    {
        mysql_free_result(result);
        result = NULL;
    }
	
	if(row_num>0)
	{
		//sql_len += osc_snprintf_r(sqlstr.get(), sqlstr.len() -1 -sql_len, "%s limit %d,%d", _sql_str, row_begin, row_num);
		sql_len += osc_snprintf_r(sqlstr, strlen(sqlstr) - 1 - sql_len, strlen(sqlstr) - 1 - sql_len, "%s limit %d,%d", _sql_str, row_begin, row_num);
	}

    //if (mysql_real_query(mysql, sqlstr.get(), sql_len))
	if (mysql_real_query(mysql, sqlstr, sql_len))
    {
        return -2;
    }
    else
	{
		mi_affected_row_num = (int)mysql_affected_rows(mysql);
        result =  mysql_store_result(mysql);
        if (NULL == result)
        {
                /*if (mysql_field_count(mysql) == 0)
                    return 0;
                else*/
				return -3;
		}
		mi_col_num = mysql_num_fields(result);
            
        return 0;
	}
}



int MysqlClient::col_num()
{
    return mi_col_num; 
    
}

int MysqlClient::affected_row_num()
{
	return mi_affected_row_num;
}

int MysqlClient::fetch_row()
{
    if(result==NULL)
    {
		return -1;
    }
    row = mysql_fetch_row(result);
	if(row==NULL)
		return -1;
    return 0;
}

const char* MysqlClient::get_field(int index)
{
	//os_assert(index>=0);
	//os_assert(index<mi_col_num);
	// os_assert(row != NULL);

    const char *str=row[index];
    
    //printf("index[%d], [%s]\n", index, str);
 
    return str;//row[index];    
}


const char* MysqlClient::err_str()
{
    return mysql_error(mysql);
}

int MysqlClient::err_no()
{
    return mysql_errno(mysql);
}

#if 0

int MysqlClient::proc(const char* SQLString, int length)
{
    int i ;
    int out_param_num=0; // 输出参数个数
	
    APC_DEBUG "[lib_mysql.cpp]*********1 ******SQLString[%s] len[%d]", SQLString, length);
    if(NULL!=result)
    {
        mysql_free_result(result);
        result = NULL;
    }
    
    if (length ==0)
    {
        return -1;
    }

    //mysql->server_status&= ~SERVER_MORE_RESULTS_EXISTS;
    
    strncpy(sql_text, SQLString, sizeof(sql_text));

    sql_text[0] = 'c';
    sql_text[1] = 'a';
    sql_text[2] = 'l';
    sql_text[3] = 'l';
    sql_text[4] = ' ';

    APC_DEBUG "[lib_mysql.cpp]*********2 sql_text[%s]******", sql_text);
    i=0;
    while(sql_text[i])
    {
		if(sql_text[i] == ':')
		{
			if(i >= 2 && (sql_text[i-1]!=',' || sql_text[i-2]==','))
			{
				i++;
				continue;
			}
			/*if(i >= 2 && (sql_text[i-1]==',' || sql_text[i-2]==','))
			{
					sql_text[i] = '@';
					i++;
			}*/
		/*	if(sql_text[i+1]>='0' && sql_text[i+1]<='9' )
			{
				sql_text[i] = '@';
				++out_param_num;
				++i;
			}
        */
            if(sql_text[i+1]>='0' && sql_text[i+1]<='9' )
			{
				sql_text[i] = '@';
				++out_param_num;
				++i;
			}
		}
        i++;
    }

    APC_DEBUG "[lib_mysql.cpp]*********3 sql_text[%s]******", sql_text);
    char *pend=strstr(sql_text,"end;");
    if(NULL!=pend)
    {
       pend[0]=pend[1]=pend[2]=pend[3]=' ';
    }
    APC_DEBUG "[lib_mysql.cpp]*********4 sql_text[%s]******", sql_text);

    char *pcommit=strstr(sql_text,"commit;");
    if(NULL!=pcommit)
    {
       pcommit[0]=pcommit[1]=pcommit[2]=pcommit[3]=' ';
       pcommit[4]=pcommit[5]=pcommit[6]=' ';
    }
    APC_DEBUG "[lib_mysql.cpp]*********41 sql_text[%s]******", sql_text);

    if (mysql_real_query(mysql, sql_text, length))
    {
    	APC_ERROR "[lib_mysql.cpp]*********5 ******mysql_error[%s]", mysql_error(mysql));
        return -2;
    }
    else
    {
        result =  mysql_store_result(mysql);
        if (NULL == result)
        {
            if (mysql_field_count(mysql) != 0)
            {
            APC_ERROR "[lib_mysql.cpp]*********6 ******mysql_error[%s]", mysql_error(mysql));
            return -3;
            }
            mui32_col_num = 0;
        }
        else{
        
        mui32_col_num = mysql_num_fields(result);
        }
    }

    if(out_param_num==0)
       return 0;
     
    // 查询结果
    char select_str[1024]="select ";
    int  len=strlen("select ");
    for(i=0; i<out_param_num; ++i)
    {
       int tmp_len=snprintf(select_str+len,sizeof(select_str)-1-len, "@%d,",i+1);
       len += tmp_len;
    }
    select_str[len-1]=';';
    APC_DEBUG "[%s],len[%d]", select_str, len);
    
    if(NULL!=result)
        mysql_free_result(result);
       
    if (mysql_real_query(mysql, select_str, len))
    {
        return -1;
    }

    result =  mysql_store_result(mysql);
    if (NULL == result)
    {
        return -3;
    }
    mui32_col_num = mysql_num_fields(result);
    return 0;
}
#endif

/*void MysqlClient::data_seek(int row_number)
{
    mysql_data_seek(result, row_number);
}
*/

int MysqlClient::reconnect()
{
	my_bool ret=mysql_ping(mysql);
	
	if(ret==0)
		return 0;
	else
		return -1;
}

int MysqlClient::commit()
{
	my_bool ret=mysql_commit(mysql);
	
	if(ret==0)
		return 0;
	else
       {
			return -1;
      }
}

int MysqlClient::rollback()
{
	my_bool ret=mysql_rollback(mysql);

	if(ret==0)
		return 0;
	else
		return -1;
}


