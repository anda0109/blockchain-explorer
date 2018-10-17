#pragma once
#include "common/os_type.h"
#include <string>
#include <vector>
using namespace std;

struct Node
{
	string address;
	uint16 port;
	string username;
	string password;
};


bool is_user_address(string address, string& walletid);
bool post_address_value(string walletid, string contract, double value, string content, string time, double totalamount);


void sync_eth_blocks(vector<Node> eth_node);
void sync_one_eth_block(vector<Node> eth_node, uint64 height);

void sync_eos_blocks(vector<Node> eos_node);

void sync_btc_blocks(vector<Node> btc_node);
bool sync_one_btc_block(vector<Node> btc_node, uint64 block_number);


