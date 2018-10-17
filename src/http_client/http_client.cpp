#include <curl/curl.h>
#define DLL_IMPLEMENT
#include "http_client.h"
#include "mxcurl.h"

int  http_client_init()
{
	curl_global_init(CURL_GLOBAL_ALL);  
	return 0;
}

//POST
int  http_client_post( string& Urlstr,string& Pdata , string& Rdata)
{
	int ret = 0;
	if(Urlstr.length()==0)
	{
		return -1;
	}
	Session *oneSession=new Session();//
	oneSession->SetUrl(Urlstr);
	oneSession->SetPayload(Pdata);

	Response Re =oneSession->Post();

	if(Re.status_code!=200 && Re.status_code!=300)
	{
		ret=-1;
	}
	else
	{
		int Rlen=Re.text.length();
		if(Rlen==0)
		{
		
			ret=-1;
		}
		else
		{
			Rdata=Re.text;
			
		}
	}
	delete oneSession;
	return ret;
}
//GET

int  http_client_get( string& Urlstr, string& Rdata)
{
	int ret = 0;
	if(Urlstr.length()==0)
	{
		return -1;
	}
	Session *oneSession=new Session();//
	oneSession->SetUrl(Urlstr);

	Response Re =oneSession->Get();

	//printf("ret=%d\r\nRdata=%s\r\n",Re.status_code,"sss");
	if(Re.status_code!=200&&Re.status_code!=300)
	{
		ret=-1;
	}
	else
	{
		int Rlen=Re.text.length();
		if(Rlen==0)
		{
		
			ret=-1;
		}
		else
		{
			Rdata=Re.text;
			
		}
	}
	delete oneSession;
	return ret;
}