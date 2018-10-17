#include "BtcData.h"


BtcData::BtcData(string address, uint16_t port, string username, string password):DaemonData(address,port,username,password)
{

}

BtcData::~BtcData()
{

}

string BtcData::getBlockHeight()
{
	Json::Value params;
	return getData("getblockcount", params);
}

//该函数弃用，以太坊全部使用16进制字符串表示
string BtcData::getBlockByNumber(uint64 blockIndex)
{
	string blockHash = getBlockHash(blockIndex);
	Json::Value root;
	Json::Reader reader;
	if (reader.parse(blockHash, root))
		blockHash = root["result"].asString();
	return getBlock(blockHash);
}

string BtcData::getBlock(const string& blockHash)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = blockHash;

	return getData("getblock", params);
}

string BtcData::getBlockHash(uint64 blockIndex)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = (int)blockIndex;
	return getData("getblockhash", params);
}

string BtcData::getRawTransaction(const string& txid)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = txid;
	params[index++] = 1;
	return getData("getrawtransaction", params);
}

