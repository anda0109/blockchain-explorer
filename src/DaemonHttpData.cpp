#include "DaemonHttpData.h"

DaemonHttpData::DaemonHttpData()
{
	m_client = new Json::Rpc::HttpClient();
}

DaemonHttpData::~DaemonHttpData()
{
	delete m_client;
}

string DaemonHttpData::getData(string uri, string params)
{
	std::string responseStr;
	//Json::Rpc::HttpClient client;
	m_client->ChangeAddress(uri);
	if (m_client->Send(params) == 0)
	{
		m_client->WaitRecv(responseStr, 5);
	}
	m_client->Close();

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
	return  responseStr;
}

