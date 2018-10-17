#include "SyncBlocks.h"
#include "db_access/db_access.h"
#include "common/ut_strconvert.h"
#include "common/os_time.h"
#include "common/os_type.h"
#include "common/os_log.h"
#include "common/os_libc.h"
#include "common/os_dir.h"
#include "common/ut_stl.h"
#include "json/json.h"
#include "httpclient.h"
#include <math.h>
#include "BtcData.h"

//#include "ThreadPool.h"

#ifdef OS_LINUX
#include <unistd.h>
#endif

extern string g_post_address_value_api;
extern str g_self_dir;
extern string g_btc_start_field;//btc开始块的字段名
extern int g_btc_server_num;

//static ThreadPool sync_tx_thread_pool(8);//同步交易线程池

struct utxo
{
	string txid;
	string hash;
	string address;//可以花费这笔交易的地址
	double value;
	int index;//该txid中的第几笔输出
	int isused;//是否已经花费，0未花费，1已花费
	string script;
	string script_hex;
	uint64 time;
	uint64 block_number;
	string walletid;//地址对应的walletid，用于后面推送消息
};

struct input
{
	string pre_txid;
	string pre_hash;
	int pre_vout_index;
	string pre_vout_address;
	double pre_vout_value;
	string walletid;
};

struct transaction
{
	string txid;
	string hash;
	string fromadd;
	string toadd;
	double value;
	double minerfee;
	int blocknumber;
	string blockhash;
	string time;
};

uint64 get_btc_start_block()
{
	char sql[256];
	snprintf(sql, 256, "select height from sync_status where chain='%s'", g_btc_start_field.c_str());
	DataSet res = db_query(sql);
	if (res.size() != 0)
	{
		return atof(res[0]["HEIGHT"].c_str());
	}
	return 0;
}

bool set_btc_start_block(uint64 block_height)
{
	char sql[256];
	snprintf(sql, 256, "update sync_status set height=%s where chain='%s'", osc_print_uint64(block_height).getbuf(), g_btc_start_field.c_str());
	int ret = db_update(sql, true);
	if (ret != 1)
	{
		return false;
	}
	return true;
}

uint64 getCurrentBlock(BtcData* btc) {
	string height = btc->getBlockHeight();
	Json::Value root;
	Json::Reader reader;
	if (reader.parse(height, root))
	{
		return root["result"].asDouble();
	}
	return 0;
}


bool insert_one_utxo(utxo& utxo_)
{
	//如果存在，continue
	char sql[10240];
	snprintf(sql, 1024, "select * from bch_utxo where txid='%s' and outindex=%d", utxo_.txid.c_str(), utxo_.index);
	DataSet res = db_query(sql);
	if (res.size() != 0)
		return true;
	snprintf(
		sql,
		10240 - 1,
		"insert into bch_utxo(txid,hash,address,value,outindex,isused,script,scripthex,time,height) values('%s','%s','%s','%.8lf','%d','%d','%s','%s',%s,%s)",
		utxo_.txid.c_str(),
		utxo_.hash.c_str(),
		utxo_.address.c_str(),
		utxo_.value,
		utxo_.index,
		utxo_.isused,
		utxo_.script.c_str(),
		utxo_.script_hex.c_str(),
		osc_print_uint64(utxo_.time).getbuf(),
		osc_print_uint64(utxo_.block_number).getbuf()
	);
	int ret = db_update(sql, true);
	if (ret != 1)
	{
		return false;
	}
	return true;
}

bool set_utxo_used(input& input_)
{
	char sql[10240];
	snprintf(
		sql,
		10240 - 1,
		"update bch_utxo set isused=1 where txid='%s' and outindex=%d",
		input_.pre_txid.c_str(),
		input_.pre_vout_index
	);
	int ret = db_update(sql, true);
	if (ret != 1)
	{
		snprintf(
			sql,
			10240 - 1,
			"insert into bch_utxo(txid,hash,address,value,outindex,isused) values('%s','%s','%s','%.8lf','%d','%d')",
			input_.pre_txid.c_str(),
			input_.pre_hash.c_str(),
			input_.pre_vout_address.c_str(),
			input_.pre_vout_value,
			input_.pre_vout_index,
			1
		);
		ret = db_update(sql, true);
		if(ret !=1)
			return false;
	}
	return true;
}

bool insert_one_transaction(transaction& tx)
{
	//如果存在，continue
	char sql[10240];
	snprintf(sql, 1024, "select * from bch_transaction where txid='%s' and toadd='%s'", tx.txid.c_str(), tx.toadd.c_str());
	DataSet res = db_query(sql);
	if (res.size() != 0)
		return true;
	snprintf(
		sql,
		10240 - 1,
		"insert into bch_transaction(txid,hash,fromadd,toadd,value,minerfee,blocknumber,blockhash,time) values('%s','%s','%s','%s','%.8lf','%.8lf','%s','%s','%s')",
		tx.txid.c_str(),
		tx.hash.c_str(),
		tx.fromadd.c_str(),
		tx.toadd.c_str(),
		tx.value,
		tx.minerfee,
		osc_print_uint64(tx.blocknumber).getbuf(),
		tx.blockhash.c_str(),
		tx.time.c_str()
	);
	int ret = db_update(sql, true);
	if (ret != 1)
	{
		return false;
	}
	return true;
}

bool get_address_balance(string& address,double& balance)
{
	char sql[1024];
	snprintf(sql, 1024, "SELECT SUM(VALUE) AS BALANCE FROM BCH_UTXO WHERE ADDRESS='%s' AND (ISUSED=0 OR ISUSED=3)", address.c_str());
	DataSet res = db_query(sql);
	if (res.size() != 0)
	{
		balance = atof(res[0]["BALANCE"].c_str());
		return true;
	}
	return false;
}

bool push_wallet_transaction(string walletid, string contract, double value, transaction& tx, double totalamount)
{	
	Json::Value root;
	Json::FastWriter writer;
	root["txid"] = tx.txid;
	root["hash"] = tx.hash;
	root["from"] = tx.fromadd;
	root["to"] = tx.toadd;
	root["value"] = tx.value;
	root["minerfee"] = tx.minerfee;
	root["blocknumber"] = tx.blocknumber;
	root["blockhash"] = tx.blockhash;
	root["time"] = tx.time;
	string content = writer.write(root);
	return post_address_value(walletid, contract, value, content, tx.time, totalamount);
}

bool sync_one_transaction(vector<Node> btc_node, const string& txid, const uint64 block_number)
{
	BtcData btc(btc_node[0].address, btc_node[0].port, btc_node[0].username, btc_node[0].password);
	string rawtransaction = btc.getRawTransaction(txid);
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(rawtransaction, root))

	{
		//sync_tx_thread_pool.enqueue(sync_one_transaction, btc_node, txid, block_number);
		return false;
	}

	string hash = root["result"]["hash"].asString();
	double input_value = 0, output_value = 0;

	bool isWalletAddress = false;

	//遍历输出
	vector<utxo> utxos;
	for (int j = 0; j < root["result"]["vout"].size(); j++)
	{
		if (root["result"]["vout"][j]["scriptPubKey"] != Json::nullValue && root["result"]["vout"][j]["scriptPubKey"]["type"].asString() != "nulldata")
		{
			utxo utxo_;
			utxo_.txid = txid;
			utxo_.hash = hash;
			utxo_.index = j;
			utxo_.isused = 0;
			utxo_.time = (uint64)root["result"]["time"].asDouble();
			utxo_.block_number = block_number;
			//查找输出地址
			string vout_type = root["result"]["vout"][j]["scriptPubKey"]["type"].asString();
			if (vout_type == "pubkeyhash" || vout_type == "pubkey" || vout_type == "scripthash" || vout_type == "witness_v0_scripthash" || vout_type == "witness_v0_keyhash")
			{
				utxo_.address = root["result"]["vout"][j]["scriptPubKey"]["addresses"][(Json::UInt)0].asString();
				utxo_.script = root["result"]["vout"][j]["scriptPubKey"]["asm"].asString();
				utxo_.script_hex = root["result"]["vout"][j]["scriptPubKey"]["hex"].asString();
				utxo_.value = root["result"]["vout"][j]["value"].asDouble();
				output_value += root["result"]["vout"][j]["value"].asDouble();
			}
			utxos.push_back(utxo_);
		}
	}

	bool isOurWalletAddress = false;
	//utxo存入数据库
	for (int i = 0; i < utxos.size(); ++i)
	{
		if (is_user_address(utxos[i].address, utxos[i].walletid))
		{
			isOurWalletAddress = true;
			insert_one_utxo(utxos[i]);
			printf("sync utxo:txid=%s,outindex=%d\n", utxos[i].txid.c_str(), utxos[i].index);
			os_log(FL, "sync utxo:txid=%s,outindex=%d\n", utxos[i].txid.c_str(), utxos[i].index);
		}
	}
	if (!isOurWalletAddress)
		return true;

	//遍历输入
	vector<input> inputs;
	Json::Value pre_raw;
	for (int j = 0; j < root["result"]["vin"].size(); j++)
	{
		input input_;
		if (root["result"]["vin"][j]["txid"] != Json::nullValue)
		{
			input_.pre_txid = root["result"]["vin"][j]["txid"].asString();//用于支付的交易id
			input_.pre_vout_index = root["result"]["vin"][j]["vout"].asInt();//用于支付的交易的输出的索引，即使用上一笔交易的第几个输出进行支付
			string pre_rawtransaction = btc.getRawTransaction(input_.pre_txid);

			if (!reader.parse(pre_rawtransaction, pre_raw))
			{
				j--; continue;
			}

			input_.pre_hash = pre_raw["result"]["hash"].asString();
			input_.pre_vout_address = pre_raw["result"]["vout"][input_.pre_vout_index]["scriptPubKey"]["addresses"][(Json::UInt)0].asString();
			input_.pre_vout_value = pre_raw["result"]["vout"][input_.pre_vout_index]["value"].asDouble();
			input_value += input_.pre_vout_value;
		}
		else
		{
			//coinbase
			string coinbase = root["result"]["vin"][j]["coinbase"].asString();
			input_.pre_vout_address = coinbase;
		}
		inputs.push_back(input_);
	}

	//更新utxo状态，如果某个utxo在某个交易里作为了输入，说明这笔utxo已经花费
	for (int i = 0; i < inputs.size(); ++i)
	{
		if (is_user_address(inputs[i].pre_vout_address, inputs[i].walletid))
		{
			set_utxo_used(inputs[i]);
		}
	}

	//非钱包地址	
	//if (!((inputs.size() > 0 && inputs[0].walletid.length() != 0) || (utxos.size() > 0 && utxos[0].walletid.length() != 0)))
	//	return true;

	//多笔输出
	transaction tx;
	tx.txid = txid;
	tx.hash = hash;
	tx.blockhash = root["result"]["blockhash"].asString();
	tx.blocknumber = block_number;
	tx.time = os_time_format((longint)root["result"]["time"].asDouble()).getbuf();
	tx.minerfee = input_value - output_value;
	tx.fromadd = inputs[0].pre_vout_address;
	for (int i = 0; i < utxos.size(); ++i)
	{
		if ((inputs[0].walletid.length() != 0 || utxos[i].walletid.length() != 0) && inputs[0].pre_vout_address!=utxos[i].address)
		{
			tx.toadd = utxos[i].address;
			tx.value = utxos[i].value;

			//推送交易记录消息
			string btcid = "21398df7-9541-48c2-8f85-dece3fd18182";
			double balance1, balance2;
			if (inputs[0].walletid.length() > 0)
			{
				get_address_balance(tx.fromadd, balance1);
				//while (!push_wallet_transaction(inputs[0].walletid, btcid, -tx.value - tx.minerfee, tx, balance1));//发送方
				push_wallet_transaction(inputs[0].walletid, btcid, -tx.value - tx.minerfee, tx, balance1);
			}
			
			if (utxos[i].walletid.length() > 0)
			{
				get_address_balance(tx.toadd, balance2);
				//while (!push_wallet_transaction(utxos[i].walletid, btcid, tx.value, tx, balance2));//接收方
				push_wallet_transaction(utxos[i].walletid, btcid, tx.value, tx, balance2);
			}			

			//交易记录写入数据库
			bool ret = insert_one_transaction(tx);
			if (!ret)
			{
				i--; continue;
			}
		}
	}

	printf("sync transaction:txid=%s,value=%.18f,from=%s,to=%s\n", tx.txid.c_str(), tx.value,tx.fromadd.c_str(), tx.toadd.c_str());
	os_log(FL, "sync transaction:txid=%s,value=%.18f,from=%s,to=%s\n", tx.txid.c_str(), tx.value, tx.fromadd.c_str(), tx.toadd.c_str());

	return true;
}


bool sync_one_btc_block(vector<Node> btc_node, uint64 block_number)
{
	char buf[12] = { 0 };
	//string block_index = ltoa(block_number, buf, 10);
	BtcData btc(btc_node[0].address, btc_node[0].port, btc_node[0].username, btc_node[0].password);
	string block = btc.getBlockByNumber(block_number);
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(block, root))
	{
		return false;
	}

	printf("\n[%s]Sync block height:%s, hash:%s", os_time_format().getbuf(), osc_print_uint64(block_number).getbuf(), root["result"]["hash"].asString().c_str());
	os_log(FL, "Sync block height:%s, hash:%s\n", osc_print_uint64(block_number).getbuf(), root["result"]["hash"].asString().c_str());

	Json::Value transactions = root["result"]["tx"];

	//遍历区块内的所有交易
	for (int i = 0; i<transactions.size(); i++)
	{
		string txid = transactions[i].asString();
		printf("txid:%s\n", txid.c_str());
		//sync_tx_thread_pool.enqueue(sync_one_transaction, btc_node, txid, block_number);
		
		if (!sync_one_transaction(btc_node, txid, block_number))
		{
			i--;
			continue;
		}
		
	}
	printf("\n[%s]Sync block height:%s finish, hash:%s", os_time_format().getbuf(), osc_print_uint64(block_number).getbuf(), root["result"]["hash"].asString().c_str());
	os_log(FL, "Sync block height finish:%s, hash:%s\n", osc_print_uint64(block_number).getbuf(), root["result"]["hash"].asString().c_str());
	return true;
}

//遍历同步所有区块
void sync_btc_blocks(vector<Node> btc_node)
{
	uint64 start_block = get_btc_start_block();
	printf("begin sync block from %s\n", osc_print_uint64(start_block).getbuf());
	BtcData* btc = new BtcData(btc_node[0].address, btc_node[0].port, btc_node[0].username, btc_node[0].password);
	uint64 current_height = getCurrentBlock(btc);
	if (current_height == 0)
	{
		printf("节点未启动!\n");
	}
	for (uint64 i = start_block;true; i+=g_btc_server_num)
	{
		//等待新块
		while (i > current_height || current_height == 0)
		{
#ifdef OS_WIN
			Sleep(1000);
#else
			sleep(1);
#endif	
			current_height = getCurrentBlock(btc);
		}

		bool ret = sync_one_btc_block(btc_node, i);
		if (!ret)
		{
			printf("sync block %s fail.\n", osc_print_uint64(i).getbuf());
			i -= g_btc_server_num;
			continue;
		}

		set_btc_start_block(i);		
	}
	delete btc;
	os_error(FL, "Sync finished.");
	printf("Sync finished.");
}
