#pragma once
#include "common/os_type.h"
#include "DaemonData.h"

class EthData : public DaemonData
{
public:
	EthData(string address, uint16_t port, string username, string password);
	~EthData();

public:
	//block
	string getBlockHeight();
	string getBlock(uint32_t blockNumber);
	string getBlock(string blockHashOrNumber, bool isHash);
	string getBlock(std::map<string, string>& params);
	//transaction
	string getTransaction(string transactionHash);
	string getTransaction(std::map<string,string>& params);

	string getTransactionReceipt(string transactionHash);

	string sendTransaction(string from, string to, string gas, string gasPrice, string value, string data, string nonce, string passphrase);
	string sendTransaction(std::map<string,string>& params);

	string getBalance(string& address);
	string getBalance(std::map<string, string>& params);

	string getTokenBalance(string tokenAddress, string address);
	
	string getAddress();//Returns a list of addresses owned by client.

	string getCode(const string& address);
};

