#include "OmniData.h"


OmniData::OmniData(string address, uint16_t port, string username, string password):DaemonData(address,port,username,password)
{

}

OmniData::~OmniData()
{

}

string OmniData::getBlockHeight()
{
	Json::Value params;
	return getData("getblockcount", params);
}

//该函数弃用，以太坊全部使用16进制字符串表示
string OmniData::getBlockByNumber(uint64 blockIndex)
{
	string blockHash = getBlockHash(blockIndex);
	Json::Value root;
	Json::Reader reader;
	if (reader.parse(blockHash, root))
		blockHash = root["result"].asString();
	return getBlock(blockHash);
}

string OmniData::getBlock(const string& blockHash)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = blockHash;

	return getData("getblock", params);
}

string OmniData::getBlockHash(uint64 blockIndex)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = (int)blockIndex;
	return getData("getblockhash", params);
}

string OmniData::getRawTransaction(const string& txid)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = txid;
	params[index++] = 1;
	return getData("getrawtransaction", params);
}

string OmniData::omni_listblocktransactions(uint64 blockIndex)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = (int)blockIndex;
	return getData("omni_listblocktransactions", params);
}

string OmniData::omni_gettransaction(const string& txid)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = txid;
	//params[index++] = 1;
	return getData("omni_gettransaction", params);
}

string OmniData::omni_getbalance(const string& address, const string& propertyid)
{
	//uint64 propertyid_int = atol(propertyid.c_str());
	uint64 propertyid_int;
	sscanf(propertyid.c_str(), "%d", &propertyid_int);
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = address;
	params[index++] = (uint)propertyid_int;
	return getData("omni_getbalance", params);
}
