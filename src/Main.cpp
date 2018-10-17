#include <stdio.h>
#include <vector>
#include <thread>
#include "json/json.h"
#include "db_access/db_access.h"
#include "common/os_dir.h"
#include "common/os_log.h"
#include "SyncBlocks.h"
#include "common/ut_strconvert.h"
//#include "jsonrpc/jsonrpc_httpclient.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include<event2/event.h>
#include<event2/http.h>
#include "http_server_api.h"


#ifdef _WIN32
#pragma comment(lib, "libevent.lib")
//#pragma comment(lib, "libcurl.lib")
//#pragma comment(lib, "ws2_32.lib")
#endif

#ifdef _WIN32
#include <windows.h>
//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif

#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif
#include <signal.h>

#include <crtdbg.h>

//配置文件信息
vector<Node> g_eth_node;//以太坊节点
int g_eth_start_block = 0;//以太坊开始遍历的块高

vector<Node> g_eos_node;//EOS节点
int g_eos_start_block = 0;//EOS开始遍历的块高

vector<Node> g_btc_node;//BTC节点
//int g_btc_start_block = 0;//BTC开始遍历的块高
string g_btc_start_field;//btc开始块的字段名
int g_btc_server_num;//同进遍历的服务数量，各个服务间隔g_server_num个块遍历

//db
string g_db_ip;
int		 g_db_port;
string g_db_dbname;
string g_db_user;
string g_db_passwd;

//loglevel
int g_log_level = JENC_LOG;

//http api
string g_post_address_value_api;

//全局变量
str g_self_dir;//当前程序所在目录

int create_http_server();

bool readConf()
{
	Json::Value root;
	Json::Reader reader;

	char confFile[255];
	snprintf(confFile, 255, "%s/syncblock-bch.conf", g_self_dir.getbuf());
	char* buf = NULL;
	FILE* fp = NULL;
#ifdef OS_WIN
	fopen_s(&fp, confFile, "r");
#else
	fp = fopen(confFile, "r");
#endif // OS_WIN	
	if (fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		rewind(fp);
		buf = (char*)malloc(size);
		fread(buf, size, 1, fp);
		fclose(fp);
	}

	if (!reader.parse(buf, root, true))
	{
		free( buf);
		printf("parse conf fail!");
		return false;
	}
	free(buf);
	//以太坊节点配置
	Node ethnode;
	for (int i = 0; i<root["eth"]["node"].size(); i++)
	{
		ethnode.address = root["eth"]["node"][i]["address"].asString().c_str();
		ethnode.port = root["eth"]["node"][i]["port"].asInt();
		ethnode.username = root["eth"]["node"][i]["username"].asString().c_str();
		ethnode.password = root["eth"]["node"][i]["password"].asString().c_str();
		g_eth_node.push_back(ethnode);
	}
	g_eth_start_block = root["eth"]["startblock"].asInt();

	Node eosnode;
	for (int i = 0; i<root["eos"]["node"].size(); i++)
	{
		eosnode.address = root["eos"]["node"][i]["address"].asString().c_str();
		eosnode.port = root["eos"]["node"][i]["port"].asInt();
		eosnode.username = root["eos"]["node"][i]["username"].asString().c_str();
		eosnode.password = root["eos"]["node"][i]["password"].asString().c_str();
		g_eos_node.push_back(eosnode);
	}
	//g_eos_start_block = root["eos"]["startblock"].asInt();

	Node btcnode;
	for (int i = 0; i<root["btc"]["node"].size(); i++)
	{
		btcnode.address = root["btc"]["node"][i]["address"].asString().c_str();
		btcnode.port = root["btc"]["node"][i]["port"].asInt();
		btcnode.username = root["btc"]["node"][i]["username"].asString().c_str();
		btcnode.password = root["btc"]["node"][i]["password"].asString().c_str();
		g_btc_node.push_back(btcnode);
	}
	g_btc_start_field = root["btc"]["startfield"].asString();
	g_btc_server_num = root["btc"]["servernum"].asInt();
	
	//数据库配置
	g_db_ip = root["oracle_db"]["address"].asString().c_str();
	g_db_port = root["oracle_db"]["port"].asInt();
	g_db_dbname = root["oracle_db"]["dbname"].asString().c_str();
	g_db_user = root["oracle_db"]["username"].asString().c_str();
	g_db_passwd = root["oracle_db"]["password"].asString().c_str();

	//日志级别配置
	string loglevel = root["loglevel"].asString();
	if (loglevel == "sys")
		g_log_level = JENC_SYS;
	if (loglevel == "debug")
		g_log_level = JENC_DEBUG;
	if (loglevel == "trace")
		g_log_level = JENC_TRACE;
	if (loglevel == "error")
		g_log_level = JENC_ERROR;
	if (loglevel == "log")
		g_log_level = JENC_LOG;
	if (loglevel == "alarm")
		g_log_level = JENC_ALARM;

	//http接口地址
	g_post_address_value_api = root["post_address_value_api"].asString();

	return true;
}

//全局初始化
bool init()
{
	os_get_selfexe_fullpath("syncblock", &g_self_dir);

	//0、log文件初始化，由于log是基于线程独立的，如果是多线程，在每个线程都需要初始化
	str logfile;
	logfile.nfmt(255, "%s/logs/syncblocks_bch.log", g_self_dir.getbuf());
	os_log_init(logfile.getbuf(), g_log_level);
	os_log(FL, "Log init.\n");

	//1、读取配置文件
	readConf();

	//2、数据库初始化
	if (!db_connect(g_db_ip, g_db_port, g_db_dbname, g_db_user, g_db_passwd))
	{
		os_error(FL,"db connect fail!");
		printf("db connect fail!");
		return false;
	}
	printf("db connect success!\n");
	os_log(FL,"db connect success!\n");
	os_log(FL, "Syncblock start.\n");

	//4、其他...

	return true;
}


void EnableMemLeakCheck()
{
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpFlag);
}


void test_memory()
{
	double value = 0.0;
	char buffer[] = "0.486700010000000000";
	sscanf(buffer, "%.8lf", &value);
	value = atof(buffer);
	value = strtold(buffer, NULL);

	Json::Value root;
	Json::Reader reader;
	string sss = "{\"test\":0.10545445}";
	reader.parse(sss, root, false);
	double test = root["test"].asDouble();
}

int main(int argc, char** argv)
{	
	//test_memory();
	EnableMemLeakCheck();
	//_CrtDumpMemoryLeaks();
	//_CrtSetBreakAlloc(474440);
	
	if (!init())
	{
		printf("init fail!");
		return -1;
	}

	//如果带参数
	if (argc > 1)
	{
		uint64 height = atol(argv[1]);
		sync_one_btc_block(g_btc_node, height);
		return 0;
	}
		
	//创建一个http服务接口线程，用于接收消息更新token_address表
	//create_http_server();

	//同步BTC
	sync_btc_blocks(g_btc_node);
	
	printf("exit!");
	
	getchar();
    return 0;
}

int create_http_server( )
{
	//0、log文件初始化，由于log是基于线程独立的，如果是多线程，在每个线程都需要初始化
	//str logfile;
	//logfile.nfmt(255, "%s/logs/httpserver.log", g_self_dir.getbuf());
	//os_log_init(logfile.getbuf(), g_log_level);
	//os_log(FL, "Log init.\n");

	struct event_base *base;
	struct evhttp *http;
	struct evhttp_bound_socket *handle;

	const char* address = "0.0.0.0";
	ev_uint16_t port = 82;
	int http_timeout = 60;

	//0、网络初始化
#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
	WSADATA WSAData;
	WSAStartup(0x101, &WSAData);
#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return 1;
#endif

	//1、创建一个event_base对象
	base = event_base_new();
	if (!base)
	{
		cout << "create event_base fail!" << endl;
		return 1;
	}

	//2、创建一个evhttp对象
	http = evhttp_new(base);
	if (!http)
	{
		cout << "evhttp_new fail!" << endl;
		return 1;
	}

	//3、设置http请求的回调函数，类似路由功能
	evhttp_set_cb(http, "/token/add", request_cb, NULL);
	evhttp_set_cb(http, "/token/query", request_cb, NULL);
	evhttp_set_cb(http, "/token/delete", request_cb, NULL);

	// 设置响应超时时间，单位：秒
	evhttp_set_timeout(http, http_timeout);

	//4、绑定监听地址和端口
	handle = evhttp_bind_socket_with_handle(http, address, port);
	if (!handle)
	{
		cout << "evhttp_bind_socket_with_handle fail!" << endl;
		return 1;
	}
	//5、启动事件分发循环
	event_base_dispatch(base);

	//6、释放对象
	evhttp_free(http);
	event_base_free(base);
	return 0;
}
