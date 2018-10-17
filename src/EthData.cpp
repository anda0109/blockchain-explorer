#include "EthData.h"


EthData::EthData(string address, uint16_t port, string username, string password):DaemonData(address,port,username,password)
{

}

EthData::~EthData()
{
}

string EthData::getBlockHeight()
{
	Json::Value params;
	return getData("eth_blockNumber", params);
}

//该函数弃用，以太坊全部使用16进制字符串表示
string EthData::getBlock(uint32_t blockNumber)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = blockNumber;
	params[index++] = true;
	return getData("eth_getBlockByNumber", params);
}

string EthData::getBlock(string blockHashOrNumber, bool isHash)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = blockHashOrNumber;
	params[index++] = true;
	if(isHash)
		return getData("eth_getBlockByHash", params);
	else
		return getData("eth_getBlockByNumber", params);
}

string EthData::getBlock(std::map<string, string>& params)
{
	if (params.find("blocknumber") != params.end())
		return getBlock(params["blocknumber"], false);
	if (params.find("blockhash") != params.end())
		return getBlock(params["blockhash"], true);
	return "Invalid param.";
}

string EthData::getTransaction(string transactionHash)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = transactionHash;
	return getData("eth_getTransactionByHash", params);
}

string EthData::getTransaction(std::map<string, string>& params)
{
	if (params.find("transactionhash") != params.end())
		return getTransaction(params["transactionhash"]);
	return "Invalid param";
}

string EthData::getTransactionReceipt(string transactionHash)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = transactionHash;
	return getData("eth_getTransactionReceipt", params);
}

string EthData::sendTransaction(string from, string to, string gas, string gasPrice, string value, string data, string nonce, string passphrase)
{
	cout << from << " " << to << " " << gas << " " << gasPrice << " " << value << " " << data << " " << nonce << endl;
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index]["from"] = from;
	if(to.length()>0) params[index]["to"] = to;
	if (gas.length()>0) params[index]["gas"] = gas;
	if (gasPrice.length()>0) params[index]["gasPrice"] = gasPrice;
	if (value.length()>0) params[index]["value"] = value;
	if (data.length()>0) params[index]["data"] = data;
	if (nonce.length()>0) params[index]["nonce"] = nonce;
	params[index + 1] = passphrase;
	return getData("eth_sendTransaction", params);
}

string EthData::sendTransaction(std::map<string, string>& params)
{
	string from, to, gas, gasPrice, value, data, nonce, passphrase;
	if (params.find("from") != params.end())
		from = params["from"];
	else
		return "Invalid param.";
	if (params.find("to") != params.end())
		to = params["to"];
	if (params.find("gas") != params.end())
		gas = params["gas"];
	if (params.find("gasprice") != params.end())
		gasPrice = params["gasprice"];
	if (params.find("value") != params.end())
		value = params["value"];
	if (params.find("data") != params.end())
		data = params["data"];
	if (params.find("nonce") != params.end())
		nonce = params["nonce"];
	if (params.find("passphrase") != params.end())
		passphrase = params["passphrase"];
	return sendTransaction( from,  to,  gas,  gasPrice,  value,  data,  nonce, passphrase);
}


string EthData::getBalance(string& address)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = address;
	params[index++] = "latest";
	return getData("eth_getBalance", params);
}

string EthData::getBalance(std::map<string,string>& params)
{
	string address;
	if (params.find("address") != params.end())
		address = params["address"];
	else
		return "Invalid param.";
	return getBalance(address);
}

//Returns a list of addresses owned by client.
string EthData::getAddress()
{
	Json::Value params;
	return getData("eth_accounts", params);
}


string EthData::getTokenBalance(string tokenAddress, string address)
{
	string method = "0x70a08231000000000000000000000000";//getTokenBalance方法
	string data(method + address.substr(2));

	Json::Value params;
	Json::Value::UInt index = 0;
	params[index]["to"] = tokenAddress;
	//params[index]["to"] = address;
	params[index]["data"] = data;//Hash of the method signature and encoded parameters.
	
	params[index+1] = "latest";
	return getData("eth_call", params);
}

string EthData::getCode(const string& address)
{
	Json::Value params;
	Json::Value::UInt index = 0;
	params[index++] = address;
	params[index++] = "latest";
	return getData("eth_getCode", params);
}