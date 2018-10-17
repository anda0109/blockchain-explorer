#pragma once
#include <string>
#include "curl/curl.h"

using namespace std;

CURLcode curl_get_req(const std::string &url, std::string &response);

CURLcode curl_post_req(const string &url, const string &postParams, string &response);

