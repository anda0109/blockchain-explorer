#include "SyncBlocks.h"
#include "db_access/db_access.h"
#include "common/ut_strconvert.h"
#include "common/os_time.h"
#include "common/os_type.h"
#include "common/os_log.h"
#include "common/os_libc.h"
#include "common/os_dir.h"
#include "json/json.h"
#include "EthData.h"
#include "httpclient.h"
#include <math.h>
#include <vector>

#ifdef OS_LINUX
#include <unistd.h>
#endif

extern string g_post_address_value_api;
extern str g_self_dir;
extern int g_log_level;

DataSet g_token_contracts;

bool read_token_contracts()
{
	char sql[256];
	snprintf(sql, 256, "select ADDRESS from TOKEN_ADDRESS");
	DataSet res = db_query(sql);
	if (res.size() != 0)
	{
		g_token_contracts = res;
		return true;
	}
	return false;
}

bool conf_update_startblock(uint64 blocknumber)
{
	Json::Value root;
	Json::Reader reader;

	//os_get_selfexe_fullpath("syncblock", &g_self_dir);
	char confFile[255];
	snprintf(confFile, 255, "%s/syncstatus.conf", g_self_dir.getbuf());
	char buf[1024] = { 0 };
	FILE* fp = NULL;
#ifdef WIN32
	fopen_s(&fp, confFile, "w");
#else
	fp = fopen(confFile, "w");
#endif // WIN32

	if (fp != NULL)
	{
		root["eth"]["latestblock"] = (int)blocknumber;

		Json::FastWriter writer;
		string conf = writer.write(root);

		fwrite(conf.c_str(), conf.length(), 1, fp);
		fclose(fp);
		return false;
	}
	return true;
}

uint64 get_eth_start_block()
{
	char sql[256];
	snprintf(sql, 256, "select height from sync_status where chain='ETH'");
	DataSet res = db_query(sql);
	if (res.size() != 0)
	{
		return atof(res[0]["HEIGHT"].c_str());
	}
	return 0;
}

bool set_eth_start_block(uint64 block_height)
{
	char sql[256];
	snprintf(sql, 256, "update sync_status set height=%s where chain='ETH'", osc_print_uint64(block_height).getbuf());
	int ret = db_update(sql, true);
	if (ret != 1)
	{
		return false;
	}
	return true;
}

//获取节点的当前块高
uint64 currentHeight(vector<Node> eth_node)
{
	Json::Value root;
	Json::Reader reader;
	EthData* eth = new EthData(eth_node[0].address, eth_node[0].port, eth_node[0].username, eth_node[0].password);
	string strHeight = eth->getBlockHeight();
	if (reader.parse(strHeight, root, true))
	{
		return hexToDec(root["result"].asString().c_str() + 2);
	}
	delete eth;
	return 0;
}

//是否是我们需要同步的ERC20智能合约
bool is_erc20_contract(string address, int* decimals)
{
	if (g_token_contracts.size() > 0)
	{
		printf("Check token in mem\n");
		for (int i = 0; i < g_token_contracts.size(); ++i)
		{
			if (g_token_contracts[i]["ADDRESS"] == address)
			{
				*decimals = atoi(g_token_contracts[i]["DECIMALS"].c_str());
				return true;
			}
		}
		return false;
	}

	char sql[255];
	snprintf(sql, 255, "select ID,TOKEN,DECIMALS from token_address where address='%s'", address.c_str());
	DataSet res = db_query(sql);
	if (res.size() != 0)
	{
		*decimals = atoi(res[0]["DECIMALS"].c_str());
		return true;
	}		
	return false;
}

//是否是钱包用户的地址
bool is_user_address(string address, string& walletid)
{
	char sql[255];
	snprintf(sql, 255, "select ID,WALLETID from user_address where address='%s'", address.c_str());//加上ID改数据查询丢失的问题
	DataSet res = db_query(sql);
	if (res.size() != 0)
	{
		walletid = res[0]["WALLETID"];
		return true;
	}		
	return false;
}

//解析erc20的input字段
void parse_erc20_input(const string &input, string& to, int decimals, double *amount)
{
	string methodid = input.substr(0, 10);
	to = "0x" + input.substr(10 + 24, 40);
	string str_amount = "0x" + input.substr(74);
	*amount = hexToDouble(str_amount.c_str(), decimals);
}

bool post_address_value(string walletid, string contract, double value, string content, string time,double totalamount)
{
	string url = g_post_address_value_api;
	string response;
	str postParams;
	postParams.nfmt(1024, "walletid=%s&contractid=%s&value=%.8lf&content=%s&time=%s&totalamount=%.8lf", walletid.c_str(), contract.c_str(), value,content.c_str(), time.c_str(),totalamount);
	printf("%s\n", postParams.getbuf());
	os_log(FL, postParams.getbuf());
	curl_post_req(url, postParams.getbuf(), response);
	//cout << response << endl;
	if (response.length()>0 && response[response.length() - 1] == '1')
		return true;
	return false;
}

double get_token_balance(EthData* eth, string tokenAddress, string address, int decimal)
{
	string  s = eth->getTokenBalance(tokenAddress, address);
	Json::Value root;
	Json::Reader reader;
	if (reader.parse(s, root, true))
	{
		return hexToDouble(root["result"].asString().c_str(), decimal);
	}
	return 0;
}

double get_eth_balance(EthData* eth, string address)
{
	string  s = eth->getBalance(address);
	Json::Value root;
	Json::Reader reader;
	if (reader.parse(s, root, true))
	{
		return hexToDouble(root["result"].asString().c_str(), 18);
	}
	return 0;
}

bool sync_one_block(EthData* eth, uint64 block_number)
{
	char blockNumberHex[32];
	//将uint64转化为16进制形式，#为指定前缀带0x，llx为64位，x为32位
	snprintf(blockNumberHex, 32 - 1, "%#llx", block_number);
	string block = eth->getBlock(blockNumberHex, false);

	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(block, root, true))
	{
		os_error(FL, "parse block error.");
		return false;
	}		
	else
	{
		string blockhash = root["result"]["hash"].asString();
		if (blockhash == "")
			return false;
		printf("\nSync block height:%s, hash:%s", osc_print_uint64(block_number).getbuf(), root["result"]["hash"].asString().c_str());
		os_log(FL, "Sync block height:%s, hash:%s\n", osc_print_uint64(block_number).getbuf(), root["result"]["hash"].asString().c_str());
		int txnumber = root["result"]["transactions"].size();
		for (int i = 0; i < txnumber; i++)
		{
			string hash = root["result"]["transactions"][i]["hash"].asString();
			string from = root["result"]["transactions"][i]["from"].asString();
			string to = root["result"]["transactions"][i]["to"].asString();
			double value = hexToDouble(root["result"]["transactions"][i]["value"].asString().c_str(), 18);
			string input = root["result"]["transactions"][i]["input"].asString();
			string contract_addr = "dd66bc5a-f07d-48a0-9da9-d81e5123024d";//默认以太坊
			string token_to = "";
			double token_amount;

			int decimals = 18;
			if (input.length()>74)//ERC20 INPUT
			{
				if (is_erc20_contract(to, &decimals))
				{
					parse_erc20_input(input, token_to, decimals, &token_amount);
					contract_addr = to;
					to = token_to;
					value = token_amount ;
				}
				else
				{
					continue;
				}
			}
			
			//判断是否是需要同步的地址
			string walletid_from, walletid_to;
			bool bfrom = is_user_address(from, walletid_from);
			bool bto = is_user_address(to, walletid_to);
			if (!bfrom && !bto)
				continue;

			uint64 blockNumber = hexToDec(root["result"]["transactions"][i]["blockNumber"].asString().c_str());
			string blockHash = root["result"]["transactions"][i]["blockHash"].asString();
			uint64 time = hexToDec(root["result"]["timestamp"].asString().c_str());
			str strTime = os_time_format(time);

			double gas = hexToDec(root["result"]["transactions"][i]["gas"].asString().c_str());
			double gasprice = hexToDec(root["result"]["transactions"][i]["gasPrice"].asString().c_str())/pow(10,18);

			//gasused,status从getTransactionReceipt结果中获取
			string txReceipt = eth->getTransactionReceipt(hash);
			Json::Value txReceiptRoot;
			if (!reader.parse(txReceipt, txReceiptRoot, true))
			{
				printf("parse txReceipt error.");
				i--;continue;
			}
			double gasused = hexToDec(txReceiptRoot["result"]["gasUsed"].asString().c_str());
			int status = hexToDec(txReceiptRoot["result"]["status"].asString().c_str());
			double minerfee = gasused * gasprice;

			Json::Value tx;
			tx["hash"] = hash;
			tx["from"] = from;
			tx["to"] = to;
			tx["contract"] = contract_addr;
			tx["value"] = value;
			tx["gas"] = gas;
			tx["gasprice"] = gasprice;
			tx["gasused"] = gasused;
			tx["minerfee"] = minerfee;
			tx["blocknumber"] = (int)blockNumber;
			tx["blockhash"] = blockHash;
			tx["time"] = strTime.getbuf();
			tx["status"] = status;
			Json::FastWriter writer;
			string txcontent = writer.write(tx);

			string eth_contract = "dd66bc5a-f07d-48a0-9da9-d81e5123024d";
			string eth_balance_from = eth->getBalance(from);
			if (from == to || status == 0)
			{	
				if (bfrom)
				{
					//同一个地址交易，或者交易失败的情况，只需要扣除发送账户的矿工费
					double balance = get_eth_balance(eth, from);
					//扣除矿工费
					bool ret = post_address_value(walletid_from, eth_contract, -minerfee, "", strTime.getbuf(), balance);
					if (!ret)
					{
						//db_update(sql, true);
						os_error(FL, "Post address value fail!");
						return false;
					}
				}
			}
			else
			{
				if (contract_addr != eth_contract)
				{
					if (bfrom)
					{
						double from_amount = get_token_balance(eth, contract_addr, from, decimals);
						bool ret = post_address_value(walletid_from, contract_addr, -value, txcontent, strTime.getbuf(), from_amount);
						if (!ret)
						{
							//db_update(sql, true);
							os_error(FL, "Post address value fail!");
							return false;
						}
						//扣除矿工费
						from_amount = get_eth_balance(eth, from);
						ret = post_address_value(walletid_from, eth_contract, -minerfee, "", strTime.getbuf(), from_amount);
						if (!ret)
						{
							//db_update(sql, true);
							os_error(FL, "Post address value fail!");
							return false;
						}
					}
					if (bto)
					{
						double to_amount = get_token_balance(eth, contract_addr, to, decimals);
						bool ret = post_address_value(walletid_to, contract_addr, value, txcontent, strTime.getbuf(), to_amount);
						if (!ret)
						{
							//db_update(sql, true);
							os_error(FL, "Post address value fail!");
							return false;
						}
					}
				}
				else
				{
					//ETH
					if (bfrom)
					{
						double balance = get_eth_balance(eth, from);
						bool ret = post_address_value(walletid_from, contract_addr, -value-minerfee, txcontent, strTime.getbuf(), balance);
						if (!ret)
						{
							//db_update(sql, true);
							os_error(FL, "Post address value fail!");
							return false;
						}
					}
					if (bto)
					{
						double balance = get_eth_balance(eth, to);
						bool ret = post_address_value(walletid_to, contract_addr, value, txcontent, strTime.getbuf(), balance);
						if (!ret)
						{
							//db_update(sql, true);
							os_error(FL, "Post address value fail!");
							return false;
						}
					}					
				}
				
			}
			//如果存在，continue
			char sql[10240];
			snprintf(sql, 1024, "select ID from eth_transaction where hash='%s'", hash.c_str());
			DataSet res = db_query(sql);
			if (res.size() != 0)
				continue;
			snprintf(
				sql,
				10240 - 1,
				"insert into eth_transaction(hash,fromadd,toadd,contractadd,value,gas,gasprice,gasused,minerfee,blocknumber,blockhash,time,status) values('%s','%s','%s','%s','%.18lf','%.18lf','%.18lf','%.18lf','%.18lf','%s','%s','%s',%d)",
				hash.c_str(),
				from.c_str(),
				to.c_str(),
				contract_addr.c_str(),
				value,
				gas,
				gasprice,
				gasused,
				minerfee,
				osc_print_uint64(blockNumber).getbuf(),//64位输出时兼容处理
				blockHash.c_str(),
				strTime.getbuf(),
				status
			);
			int ret = db_update(sql, true);
			if (ret != 1)
			{
				printf("Sync transaction hash:%s in block %s db_update failed.\n", hash.c_str(), osc_print_uint64(block_number).getbuf());
				os_error(FL, "Sync transaction hash:%s in block %s db_update failed.\n", hash.c_str(), osc_print_uint64(block_number).getbuf());
				i--;//如果同步失败，循环不自增，再执行一次
				continue;
			}
			/*
			string proc_name = "update_eth_transaction";
			char params[2048];
			snprintf(
			params,
			2048 - 1,
			"'%s','%s','%s','%s',%.18lf,%.18lf,%.18lf,%.18lf,%.18lf,'%s','%s','%s',%d",
			hash.c_str(),
			from.c_str(),
			to.c_str(),
			contract_addr.c_str(),
			value,
			gas,
			gasprice,
			gasused,
			minerfee,
			osc_print_uint64(blockNumber).getbuf(),//64位输出时兼容处理
			blockHash.c_str(),
			strTime.getbuf(),
			status
			);
			int _outParaCount = 1;
			bool commit = false;
			DataSet data = db_proc(proc_name, proc_name.length(), params, strlen(params), _outParaCount, commit);
			if (data[0]["0"] != "0")
			{
			printf("Sync transaction hash:%s in block %s db_update failed.\n", hash.c_str(), osc_print_uint64(block_number).getbuf());
			os_error(FL, "Sync transaction hash:%s in block %s db_update failed.\n", hash.c_str(), osc_print_uint64(block_number).getbuf());
			i--;//如果同步失败，循环不自增，再执行一次
			continue;
			}
			*/
			printf("Sync transaction hash:%s\n", hash.c_str());
			os_log(FL, "Sync transaction hash:%s\n", hash.c_str());
		}
	}
	return true;
}

//遍历同步所有区块
void sync_eth_blocks(vector<Node> eth_node)
{
	uint64 start_block = get_eth_start_block();
	printf("begin sync block from %s\n", osc_print_uint64(start_block).getbuf());
	EthData* eth = new EthData(eth_node[0].address, eth_node[0].port, eth_node[0].username, eth_node[0].password);

	uint64 current_height = currentHeight(eth_node);
	for (uint64 i = start_block; i <= current_height; i++)
	{
		bool ret = sync_one_block(eth, i);
		if (!ret)
		{
			printf("\nsync block %s fail.\n", osc_print_uint64(i).getbuf());
			i--;
			continue;
		}

		set_eth_start_block(i);
		//等待新块
		while (i >= current_height || current_height == 0)
		{
#ifdef OS_WIN
			Sleep(1000);
#else
			sleep(1);
#endif	
			current_height = currentHeight(eth_node);
		}			
	}
	os_error(FL, "Sync finished.");
	printf("Sync finished.");
}

void sync_one_eth_block(vector<Node> eth_node, uint64 height)
{
	printf("sync block %s\n", osc_print_uint64(height).getbuf());
	EthData* eth = new EthData(eth_node[0].address, eth_node[0].port, eth_node[0].username, eth_node[0].password);
	bool ret = sync_one_block(eth, height);
	if(ret)
		printf("Sync success finished.");
}