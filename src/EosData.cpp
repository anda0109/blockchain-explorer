#include "EosData.h"
#include "json/json.h"
#include "common/os_libc.h"

EosData::EosData(string address, uint16_t port, string username, string password):DaemonHttpData()
{
	this->address = address;
	this->port = port;
	this->username = username;
	this->password = password;
}

EosData::~EosData()
{
}

uint64 EosData::getBlockHeight()
{
	char uri[1024];
	snprintf(uri, 1024, "http://%s:%d/v1/chain/get_info", address.c_str(), port);
	Json::Value params;
	string res = getData(uri, "");
	Json::Value root;
	Json::Reader reader;
	if (reader.parse(res, root))
	{
		return root["head_block_num"].asDouble();
	}
	else
		return -1;
}

string EosData::getBlock(string blockId)
{
	char uri[1024];
	snprintf(uri, 1024, "http://%s:%d/v1/chain/get_block", address.c_str(), port);
	Json::Value params,ret;
	Json::FastWriter writer;
	Json::Reader reader;
	params["block_num_or_id"] = blockId;
	string reqdata = writer.write(params);
	string res = getData(uri, reqdata);
	return res;
}

string EosData::getBlock(uint64 blockNumber)
{
	char uri[1024];
	snprintf(uri, 1024, "http://%s:%d/v1/chain/get_block", address.c_str(), port);
	Json::Value params, ret;
	Json::FastWriter writer;
	Json::Reader reader;
	params["block_num_or_id"] = osc_print_uint64(blockNumber).getbuf();//可以用uint64，但json不支持
	string reqdata = writer.write(params);
	string res = getData(uri, reqdata);
	return res;
}

string EosData::getCurrencyBalance(string code, string account, string symbol)
{
	char uri[1024];
	snprintf(uri, 1024, "http://%s:%d/v1/chain/get_currency_balance", address.c_str(), port);
	Json::Value params, ret;
	Json::FastWriter writer;
	Json::Reader reader;
	params["code"] = code;
	params["account"] = account;
	params["symbol"] = symbol;
	string reqdata = writer.write(params);
	string res = getData(uri, reqdata);
	return res;
}

string EosData::getAccount(string account_name, string& balance, uint64& ram_usage, uint64_t& ram_bytes)
{
	char uri[1024];
	snprintf(uri, 1024, "http://%s:%d/v1/chain/get_account", address.c_str(), port);
	Json::Value params,ret;
	Json::FastWriter writer;
	Json::Reader reader;
	params["account_name"] = account_name;
	string reqdata = writer.write(params);
	string res = getData(uri, reqdata);
	if (reader.parse(res, ret))
	{
		balance = ret["core_liquid_balance"].asString();
		ram_usage = ret["ram_usage"].asUInt();
		ram_bytes = ret["total_resources"]["ram_bytes"].asUInt();
	}
	return res;
}
