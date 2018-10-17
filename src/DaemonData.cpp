#include "DaemonData.h"

DaemonData::DaemonData(string address, uint16_t port, string username, string password)
{
	this->address = address;
	this->port = port;
	this->username = username;
	this->password = password;
	//m_client = new Json::Rpc::HttpClient (address, port, username, password);
}

DaemonData::~DaemonData()
{
	//delete m_client;
}

string DaemonData::getData(const string& method, const Json::Value& params)
{
	Json::Value query;
	Json::FastWriter writer;
	std::string queryStr;

	/* build JSON-RPC query */
	query["jsonrpc"] = "2.0";
	query["id"] = 1010;
	query["method"] = method;
	query["params"] = params;
	queryStr = writer.write(query);

	return fetchData(queryStr);
}

string DaemonData::fetchData(const string& request)
{
	std::string responseStr;
	Json::Rpc::HttpClient client(address, port, username, password);
	if (client.Send(request) == 0)
	{
		//cout << "send queryStr:" << request << endl;
		client.WaitRecv(responseStr, 5);//5秒超时
	}

	//client.Close();
	/*
	//Json解析其返回值
	Json::Value root;
	Json::Reader reader;
	Json::FastWriter writer;
	if (reader.parse(responseStr, root)) {
		//responseStr = writer.write(root["result"]);
	}
	else
	{
		//responseStr = "Request from the node error.";
	}
	*/
	return  responseStr;
}
