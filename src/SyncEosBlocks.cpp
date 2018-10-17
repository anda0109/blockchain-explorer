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
#include "EosData.h"
#include "httpclient.h"
#include <math.h>

#ifdef OS_LINUX
#include <unistd.h>
#endif

extern string g_post_address_value_api;
extern str g_self_dir;

//transfer类型的ACTION解析，symbol为币种类型，如EOS
bool parse_transfer_action(Json::Value& action, string& from, string& to, double& value, string& symbol)
{
	string type = action["name"].asString();
	if (type != "transfer")
	{
		return false;
	}
	
	Json::Value data = action["data"];
	from = data["from"].asString();
	to = data["to"].asString();
	string quantity = data["quantity"].asString();//10.00 EOS
	string memo = data["memo"].asString();
	vector<string> svalue = split(quantity, " ");
	value = atof(svalue[0].c_str());
	symbol = svalue[1];
	cout << "from:" << from << " To:" << to << " quantity:" << quantity << endl;	
	return true;
}

bool parse_one_action(Json::Value& action)
{
	string type = action["name"].asString();//newaccount/buyrambytes/delegatebw/transfer
	Json::Value data = action["data"];
	string hex_data = action["hex_data"].asString();//data的十六进制形式
	if (type == "transfer")
	{
		string from = data["from"].asString();
		string to = data["to"].asString();
		string quantity = data["quantity"].asString();
		string memo = data["memo"].asString();
		cout << "from:" << from << " To:" << to << " quantity:" << quantity << endl;
	}
	if (type == "newaccount")
	{
		string accountname = data["name"].asString();
	}
	if (type == "buyrambytes")
	{
		string payer = data["payer"].asString();
		string receiver = data["receiver"].asString();
		double bytes = data["bytes"].asDouble();
	}
	if (type == "buyram")
	{
		string payer = data["payer"].asString();
		string receiver = data["receiver"].asString();
		string quant = data["quant"].asString();
	}
	if (type == "delegatebw")//抵押带宽（NET、CPU）
	{
		string from = data["from"].asString();
		string receiver = data["receiver"].asString();
		string stake_net_quantity = data["stake_net_quantity"].asString();
		string stake_cpu_quantity = data["stake_cpu_quantity"].asString();
		double transfer = data["transfer"].asDouble();
	}
	if (type == "voteproducer")
	{
		string voter = data["voter"].asString();
		string proxy = data["proxy"].asString();
		string producers = data["producers"].asString();
	}
	return true;
}

bool sync_one_block(EosData* eos,uint64 blockNumber)
{
	string block = eos->getBlock(blockNumber);
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(block, root))
	{
		return false;
	}

	string timestamp = root["timestamp"].asString();
	string id = root["id"].asString();
	uint64 block_num = root["block_num"].asDouble();
	printf("Sync EOS block height:%s, hash:%s\n", osc_print_uint64(blockNumber).getbuf(), id.c_str());
	os_log(FL, "Sync EOS block height:%s, hash:%s\n", osc_print_uint64(blockNumber).getbuf(), id.c_str());
	//transactions
	for (int i = 0; i < root["transactions"].size(); i++)
	{
		string txid = root["transactions"][i]["trx"]["id"].asString();
		//actions		
		Json::Value actions = root["transactions"][i]["trx"]["transaction"]["actions"];
		for (int j = 0; j < actions.size(); ++j)
		{
			//parse_one_action(actions[j]);
			string from,to,symbol;
			double value;
			if (parse_transfer_action(actions[j], from, to, value, symbol))
			{
				//如果存在，continue
				char sql[10240];
				snprintf(sql, 1024, "select * from eos_transaction where hash='%s' and toadd='%s'", txid.c_str(), to.c_str());
				DataSet res = db_query(sql);
				if (res.size() != 0)
					continue;
				snprintf(
					sql,
					10240 - 1,
					"insert into eos_transaction(hash,fromadd,toadd,contractadd,value,gas,gasprice,gasused,minerfee,blocknumber,blockhash,time,status) values('%s','%s','%s','%s','%.18lf','%.18lf','%.18lf','%.18lf','%.18lf','%s','%s','%s',%d)",
					txid.c_str(),
					from.c_str(),
					to.c_str(),
					symbol.c_str(),
					value,
					0.0,
					0.0,
					0.0,
					0.0,
					osc_print_uint64(blockNumber).getbuf(),//64位输出时兼容处理
					id.c_str(),
					timestamp.c_str(),
					1
				);
				int ret = db_update(sql, true);
				if (ret != 1)
				{
					printf("Sync transaction hash:%s in block %s db_update failed.\n", txid.c_str(), osc_print_uint64(blockNumber).getbuf());
					os_error(FL, "Sync transaction hash:%s in block %s db_update failed.\n", txid.c_str(), osc_print_uint64(blockNumber).getbuf());
					i--;//如果同步失败，循环不自增，再执行一次
					continue;
				}
				printf("Sync transaction hash:%s\n", txid.c_str());
				os_log(FL, "Sync transaction hash:%s\n", txid.c_str());
			}
		}
	}
	return true;
}

uint64 get_start_block()
{
	char sql[256];
	snprintf(sql, 256, "select height from sync_status where chain='EOS'");
	DataSet res = db_query(sql);
	if (res.size() != 0)
	{
		return atof(res[0]["HEIGHT"].c_str());
	}
	return 0;
}

bool set_start_block(uint64 block_height)
{
	char sql[256];
	snprintf(sql, 256, "update sync_status set height=%s where chain='EOS'", osc_print_uint64(block_height).getbuf());
	int ret = db_update(sql, true);
	if (ret != 1)
	{
		return false;
	}
	return true;
}

//遍历同步所有区块
void sync_eos_blocks(vector<Node> node)
{
	uint64 start_block = get_start_block();
	printf("begin sync block from %d\n", start_block);
	EosData* eos = new EosData(node[0].address, node[0].port, node[0].username, node[0].password);
	uint64 current_height = eos->getBlockHeight();
	for (uint64 i = start_block; i <= current_height; i++)
	{
		bool ret = sync_one_block(eos, i);
		if (!ret)
		{
			printf("sync block %s fail.\n", osc_print_uint64(i).getbuf());
			i--;
			continue;
		}
		set_start_block(i);
		//current_height = eos->getBlockHeight();
		//等待新块
		while (i >= current_height || current_height == 0)
		{
#ifdef OS_WIN
			Sleep(1000);
#else
			sleep(1);
#endif	
			current_height = eos->getBlockHeight();
		}			
	}
	delete eos;
	os_error(FL, "Sync finished.");
	printf("Sync finished.");
}

