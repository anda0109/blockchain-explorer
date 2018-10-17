#include "MvsData.h"
#include "common/os_libc.h"

MvsData::MvsData(string address, uint16_t port, string username, string password):DaemonData( address,  port,  username,  password)
{

}

MvsData::~MvsData()
{

}

string MvsData::getBlockHeight()
{
	string method = "getheight";
	Json::Value params = Json::arrayValue;
	return getData(method, params);
}

string MvsData::getBlock(string blockHash)
{
	string method = "getblock";
	Json::Value params = Json::arrayValue;
	Json::UInt index = 0;
	params[index] = blockHash;
	return getData(method, params);
}

string MvsData::getBlock(uint64 blockHeigth)
{
	string method = "getblock";
	Json::Value params = Json::arrayValue;
	Json::UInt index = 0;
	params[index] = osc_print_uint64(blockHeigth).getbuf();
	return getData(method, params);
}

string MvsData::getTx(const string& txHash)
{
	string method = "gettx";
	Json::Value params = Json::arrayValue;
	Json::UInt index = 0;
	params[index] = txHash;
	return getData(method, params);
}

string MvsData::listTxs(const string& accountName, const string& accountAuth)
{
	string method = "listtxs";
	Json::Value params = Json::arrayValue;
	Json::UInt index = 0;
	params[index++] = accountName;
	params[index++] = accountAuth;
	return getData(method, params);
}

string MvsData::deposit(const string& accountName, const string& accountAuth, double amount)
{
	string method = "deposit";
	Json::Value params = Json::arrayValue;
	Json::UInt index = 0;
	params[index++] = accountName;
	params[index++] = accountAuth;
	params[index++] = amount;
	return getData(method, params);
}

string MvsData::send(const string& accountName, const string& accountAuth, const string& toAddress, double amount)
{
	string method = "send";
	Json::Value params = Json::arrayValue;
	Json::UInt index = 0;
	params[index++] = accountName;
	params[index++] = accountAuth;
	params[index++] = toAddress;
	params[index++] = amount;
	return getData(method, params);
}

string MvsData::sendfrom(const string& accountName, const string& accountAuth, const string& fromAddress, const string& toAddress, double amount)
{
	string method = "sendfrom";
	Json::Value params = Json::arrayValue;
	Json::UInt index = 0;
	params[index++] = accountName;
	params[index++] = accountAuth;
	params[index++] = fromAddress; 
	params[index++] = toAddress;
	params[index++] = amount;
	return getData(method, params);
}

string MvsData::getBalance(string accountName, string accountAuth)
{
	string method = "getbalance";
	Json::Value params = Json::arrayValue;
	Json::UInt index = 0;
	params[index++] = accountName;
	params[index++] = accountAuth;
	return getData(method, params);
}

string MvsData::listBalances(string accountName, string accountAuth)
{
	string method = "listbalances";
	Json::Value params = Json::arrayValue;
	Json::UInt index = 0;
	params[index++] = accountName;
	params[index++] = accountAuth;
	return getData(method, params);
}

int MvsData::getAssetDecimals(const string& symbol)
{
	string method = "getasset";
	Json::Value params = Json::arrayValue;
	Json::UInt index = 0;
	params[index++] = symbol;
	string res = getData(method, params);

	Json::Value root;
	Json::Reader reader;
	if (reader.parse(res, root))
	{
		return root["result"][(Json::UInt)0]["decimal_number"].asInt();
	}
	return -1;
}

string MvsData::listAssets()
{
	string method = "listassets";
	Json::Value params = Json::arrayValue;
	return getData(method, params);
}
 
string MvsData::sendrawtx(const string& rawtx)
{
	string method = "sendrawtx";
	Json::Value params = Json::arrayValue;
	Json::UInt index = 0;
	params[index++] = rawtx;
	return getData(method, params);
}