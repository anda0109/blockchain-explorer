#pragma once
#include<string>
#include <map>
#include "jsonrpc/jsonrpc_httpclient.h"

using namespace std;

class DaemonHttpData
{
public:
	DaemonHttpData();
	~DaemonHttpData();
public:
	string getData(string uri, string params);

	Json::Rpc::HttpClient* m_client;
};

