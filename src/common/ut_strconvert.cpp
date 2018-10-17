#include "common/ut_strconvert.h"
#include <string.h>
#include <math.h>


int getIndexOfSigns(char ch);

/* 十六进制数转换为十进制数 
decimal为精度，n位小数则decimal=n
*/
double hexToDouble(const char *hex, int decimal)
{
	double sum = 0;
	double t = 1/pow(10, decimal);
	int i, len;

	//如果十六进制字符串前面有0x或0X先去掉
	if (strncmp(hex, "0x", 2) == 0 || strncmp(hex, "0X", 2) == 0)
		hex += 2;

	len = strlen(hex);
	for (i = len - 1; i >= 0; i--)
	{
		sum += t * getIndexOfSigns(*(hex + i));
		t *= 16;
	}

	return sum;
}

uint64 hexToDec(const char *hex)
{
	uint64 sum = 0;
	uint64 t = 1;
	int i, len;

	//如果十六进制字符串前面有0x或0X先去掉
	if (strncmp(hex, "0x", 2) == 0 || strncmp(hex, "0X", 2) == 0)
		hex += 2;

	len = strlen(hex);
	for (i = len - 1; i >= 0; i--)
	{
		sum += t * getIndexOfSigns(*(hex + i));
		t *= 16;
	}

	return sum;
}

/*
uint128_t hexToUint128(const char *hex)
{
	
	uint64 sum1 = 0,sum2 = 0;
	uint64 t = 1;
	int i, len;

	//如果十六进制字符串前面有0x或0X先去掉
	if (strncmp(hex, "0x", 2) == 0 || strncmp(hex, "0X", 2) == 0)
		hex += 2;

	len = strlen(hex);
	for (i = len - 1; i >= 0; i--)
	{
		sum1 += t * getIndexOfSigns(*(hex + i));
		t *= 16;
	}
	uint128_t sum(sum1, sum2);
	return sum;
}
*/
/* 返回ch字符在sign数组中的序号 */
int getIndexOfSigns(char ch)
{
	if (ch >= '0' && ch <= '9')
	{
		return ch - '0';
	}
	if (ch >= 'A' && ch <= 'F')
	{
		return ch - 'A' + 10;
	}
	if (ch >= 'a' && ch <= 'f')
	{
		return ch - 'a' + 10;
	}
	return -1;
}

void hexToBin(const char* hex, void* bin, int bin_size)
{
	int in_len = strlen(hex);
	if (in_len % 2 != 0)
		return;
	char buf[3] = {0};

}