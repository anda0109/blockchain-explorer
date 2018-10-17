#pragma once
#include "common/os_type.h"
#include "DaemonHttpData.h"

class EosData : public DaemonHttpData
{
public:
	EosData(string address, uint16_t port, string username, string password);
	~EosData();

public:
	//block
	uint64 getBlockHeight();
	string getBlock(string blockId);
	string getBlock(uint64 blockNumber);
	string getCurrencyBalance(string code, string account, string symbol);
	string getAccount(string account_name, string& balance, uint64& ram_usage, uint64_t& ram_bytes);

private:
	string address;
	uint16_t port;
	string username;
	string password;
};

