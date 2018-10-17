#include "http_server_api.h"
#include <string>
#include <map>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>
#include "jsonrpc/jsonrpc_httpclient.h"
#include "db_access/db_access.h"
#include "common/os_log.h"

using namespace std;

const char* GetRequestType(struct evhttp_request *req)
{
	const char *cmdtype;
	switch (evhttp_request_get_command(req)) {
	case EVHTTP_REQ_GET: cmdtype = "GET"; break;
	case EVHTTP_REQ_POST: cmdtype = "POST"; break;
	case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
	case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
	case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
	case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
	case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
	case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
	case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
	default: cmdtype = "unknown"; break;
	}
	return cmdtype;
}

string getMethodFromUri(const string& uri)
{
	const char* p = uri.c_str();// /api/v1/blockheight?chaintype=eth
	while (strstr(p, "/") != NULL)
	{
		p++;
	}
	char* method = (char*)p;
	char* methodend = strstr(method, "?");
	if (methodend)
		*methodend = 0;
	return string(method);
}

std::map<string, string> getParams(std::string& params, std::map<string, string>& mapParams)
{
	cout << params << endl;
	Json::Value root;
	Json::Reader reader;
	Json::FastWriter writer;
	if (reader.parse(params, root)) {
		for (Json::Value::iterator it = root.begin(); it != root.end(); it++)
		{
			cout << "jsondecode:" << it.key().asString() << endl;
			cout << "jsondecode:" << root[it.key().asString()].asString() << endl;
			mapParams[it.key().asString()] = root[it.memberName()].asString();
		}
	}
	return mapParams;
}

//请求消息响应回调函数
void request_cb(struct evhttp_request *req, void *arg)
{
	const char *cmdtype = GetRequestType(req);
	struct evkeyvalq *headers = NULL;
	struct evkeyval *header;
	struct evbuffer *buf;
	std::map<std::string, std::string> paramsMap;

	const char *uri = evhttp_request_get_uri(req);
	printf("Received a %s request for %s\n", cmdtype, uri);

	string method = getMethodFromUri(uri);
	cout << " method:" << method << endl;
	// Returns the input headers
	headers = evhttp_request_get_input_headers(req);
	printf("Headers:\n");
	for (header = headers->tqh_first; header;
		header = header->next.tqe_next) {
		printf(" %s: %s\n", header->key, header->value);
	}

	//获取请求参数
	struct evkeyvalq v;
	string chaintype = "btc";
	//method = "blockheight";
	const char* uriparams = strstr(uri, "?");
	if (uriparams != NULL && strlen(uriparams) >3)
	{
		int nRet = evhttp_parse_query_str(uriparams + 1, &v);
		if (nRet == 0)
		{
			for (header = v.tqh_first; header;
				header = header->next.tqe_next) {
				printf(" %s: %s\n", header->key, header->value);
				//所有通过get请求的参数要加入到paramsMap中传递过去
				if (strcmp(header->key, "chaintype") == 0)
				{
					chaintype = header->value;
				}
				paramsMap[header->key] = header->value;
				//paramsMap.insert(make_pair(header->key, header->value));
			}
		}
	}

	buf = evhttp_request_get_input_buffer(req);
	string form_body;
	while (evbuffer_get_length(buf)) {
		int n;
		char cbuf[128] = { 0 };
		n = evbuffer_remove(buf, cbuf, sizeof(cbuf) - 1);
		if (n > 0)
		{
			form_body += cbuf;
			memset(cbuf, 0, sizeof(cbuf));
			//(void)fwrite(cbuf, 1, n, stdout);
		}
	}
	//cout << form_body << endl;
	if (form_body.length() > 0)
		getParams(form_body, paramsMap);

	Json::Value ret;
	ret["code"] = 1;
	ret["message"] = "success";
	//操作数据库处理
	char sql[1024];
	if (method == "add")
	{
		snprintf(sql, 1024, "INSERT INTO TOKEN_ADDRESS(TOKEN,ADDRESS,DECIMALS) VALUES('%s', '%s', %s)", paramsMap["token"].c_str(), paramsMap["address"].c_str(), paramsMap["decimals"].c_str());
		os_log(FL, sql);
		int row = db_update(sql, true);
		if (row != 1)
		{
			ret["code"] = 0;
			ret["message"] = "fail";
		}
	}
	
	if (method == "query")
	{
		if(paramsMap["address"] != "")
			snprintf(sql, 1024, "SELECT TOKEN,ADDRESS,DECIMALS FROM TOKEN_ADDRESS WHERE ADDRESS='%s'", paramsMap["address"].c_str());
		else
			snprintf(sql, 1024, "SELECT TOKEN,ADDRESS,DECIMALS FROM TOKEN_ADDRESS");
		os_log(FL, sql);
		DataSet res = db_query(sql);
		for (int i = 0; i < res.size(); ++i)
		{
			ret["data"][i]["token"] = res[i]["TOKEN"];
			ret["data"][i]["address"] = res[i]["ADDRESS"];
			ret["data"][i]["decimals"] = res[i]["DECIMALS"];
		}
		if (res.size()  == 0)
		{
			Json::Value data;
			ret["code"] = 0;
			ret["message"] = "address not exist";
			ret["data"] = data;
		}
	}
	
	if (method == "delete")
	{
		snprintf(sql, 1024, "DELETE FROM TOKEN_ADDRESS WHERE ADDRESS='%s'", paramsMap["address"].c_str());
		os_log(FL, sql);
		int row = db_update(sql, true);
		if (row != 1)
		{
			ret["code"] = 0;
			ret["message"] = "delete fail";
		}
	}
	
	Json::Reader reader;
	Json::FastWriter writer;

	string responseStr = writer.write(ret);

	// 创建用于响应请求的对象evbuffer
	struct evbuffer* evbuf = evbuffer_new();
	if (!evbuf)
	{
		printf("Create evbuffer failed!\n");
		return;
	}
	evbuffer_add_printf(evbuf, "%s", responseStr.c_str());

	//发送响应内容 
	evhttp_send_reply(req, 200, "OK", evbuf);
}
