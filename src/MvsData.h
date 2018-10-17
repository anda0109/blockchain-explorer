#pragma once
#include "common/os_type.h"
#include "DaemonData.h"

class MvsData: public DaemonData
{
public:
	MvsData(string address, uint16_t port, string username, string password);
	~MvsData();

public:
	/**
	* ��ȡ��ǰ����߶�
	* @param void
	* @return string
	*/
	string getBlockHeight();

	/**
	* ��ȡ��������
	* @param blockHash
	* @return string
	*/
	string getBlock(string blockHash);

	/**
	* ��ȡ��ǰ����߶�
	* @param blockHeigth
	* @return string
	*/
	string getBlock(uint64 blockHeigth);

	/**
	* ���ݽ���hash��ȡ������Ϣ
	* @param $txHash
	* @return array
	*/
	string getTx(const string& txHash);


	/**
	* ��ȡĳ���˻��µĽ����б�
	* @param $accountName
	* @param $accountAuth
	* @return array
	*/
	string listTxs(const string& accountName, const string& accountAuth);

	/**
	* Deposit some etp, then get reward for frozen some etp.
	* @param $accountName
	* @param $accountAuth
	* @param $amount
	* @return array1
	*/
	string deposit(const string& accountName, const string& accountAuth, double amount);

	/**
	* send etp to a targert address, mychange goes to another existed address of this account.
	* @param $accountName
	* @param $accountAuth
	* @param $toAddress
	* @param $amount
	* @return array
	*/
	string send(const string& accountName, const string& accountAuth, const string& toAddress, double amount);

	/**
	* send etp from a specified address of this account to target address, mychange goes to from_address
	* @param $accountName
	* @param $accountAuth
	* @param $fromAddress
	* @param $toAddress
	* @param $amount
	* @return array
	*/
	string sendfrom(const string& accountName, const string& accountAuth, const string& fromAddress, const string& toAddress, double amount);

	/**
	* Show total balance details of this account.
	* @param $accountName
	* @param $accountAuth
	* @return array
	*/
	string getBalance(string accountName, string accountAuth);

	/**
	* List all balance details of each address of this account.
	* @param $accountName
	* @param $accountAuth
	* @return array
	*/
	string listBalances(string accountName, string accountAuth);


	int getAssetDecimals(const string& symbol);


	string listAssets();


	string sendrawtx(const string& rawtx);

};

