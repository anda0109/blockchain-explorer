#pragma once
#include<string>
#include "jsonrpc/jsonrpc_httpclient.h"

using namespace std;

class DaemonData
{
public:
	DaemonData(string address, uint16_t port, string username, string password);
	~DaemonData();
public:
	string getData(const string& method, const Json::Value& params);

	//virtual string getBlockHeight() = 0;
	//virtual string getBlock(uint32_t blockNumber) = 0;
	//virtual string getBlock(string blockHash) = 0;
	//virtual string getTransaction(string transactionHash) = 0;

private:
	string address;
	uint16_t port;
	string username;
	string password;

private:
	string fetchData(const string& request);

	//Json::Rpc::HttpClient* m_client;
};

