#pragma once;  
#include <string>
using namespace std;
 

//DLL_API int add(int x, int y); //�򵥷���  
int http_client_init(); //��ʼ��
int http_client_post( string& Urlstr,string& Pdata , string& Rdata);
int http_client_get( string& Urlstr, string& Rdata);