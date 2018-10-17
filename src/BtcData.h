#pragma once
#include "common/os_type.h"
#include "DaemonData.h"

class BtcData : public DaemonData
{
public:
	BtcData(string address, uint16_t port, string username, string password);
	~BtcData();

public:
	//block
	string getBlockHeight();
	string getBlock(const string& blockHash);
	string getBlockHash(uint64 blockIndex);
	string getBlockByNumber(uint64 blockIndex);

	//transaction
	string getRawTransaction(const string& txid);

};

