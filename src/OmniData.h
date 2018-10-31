#pragma once
#include "common/os_type.h"
#include "DaemonData.h"

class OmniData : public DaemonData
{
public:
	OmniData(string address, uint16_t port, string username, string password);
	~OmniData();

public:
	//block
	string getBlockHeight();
	string getBlock(const string& blockHash);
	string getBlockHash(uint64 blockIndex);
	string getBlockByNumber(uint64 blockIndex);

	//transaction
	string getRawTransaction(const string& txid);

	//Omni
	string omni_listblocktransactions(uint64 blockIndex);
	string omni_gettransaction(const string& txid);
	string omni_getbalance(const string& address, const string& propertyid);
};

