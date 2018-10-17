#pragma once;  
#include <string>
using namespace std;
 

//DLL_API int add(int x, int y); //简单方法  
int http_client_init(); //初始化
int http_client_post( string& Urlstr,string& Pdata , string& Rdata);
int http_client_get( string& Urlstr, string& Rdata);