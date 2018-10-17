#include "common/ut_strconvert.h"
#include <string.h>
#include <math.h>


int getIndexOfSigns(char ch);

/* ʮ��������ת��Ϊʮ������ 
decimalΪ���ȣ�nλС����decimal=n
*/
double hexToDouble(const char *hex, int decimal)
{
	double sum = 0;
	double t = 1/pow(10, decimal);
	int i, len;

	//���ʮ�������ַ���ǰ����0x��0X��ȥ��
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

	//���ʮ�������ַ���ǰ����0x��0X��ȥ��
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

	//���ʮ�������ַ���ǰ����0x��0X��ȥ��
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
/* ����ch�ַ���sign�����е���� */
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