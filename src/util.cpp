#include "util.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

int g_PathLower = 1;

int32_t g_isDosCP = 0; //660f64


// dos -> 1251
uint8_t __cdecl DecodeChar(uint8_t ch)
{
	//45e181
	if (g_isDosCP == 0)
		return ch;

	uint8_t t = ch;
	if (t >= 0x80 && t < 0xb0)
		return t + 0x30;
	else if (t >= 0xe0 && t < 0xf0)
		return t + 0x10;
	return t;
}

// 1251 -> dos
uint8_t __cdecl EncodeChar(uint8_t c)
{
	//4763a9
	if (g_isDosCP == 1 && c >= 0x80)
	{
		if (c >= 0xc0 && c < 0xf0)
			c -= 0x40;
		else if (c >= 0xf0)
			c -= 0x10;
	}
	return c;
}

uint8_t __cdecl ToLowerChar(uint8_t c)
{
	//47641d
	if (g_isDosCP == 0)
		return tolower((char)c);

	if (c >= 0x80 && c < 0x90)
		return c + 0x20;
	else if (c >= 0x90 && c < 0xa0)
		return c + 0x50;

	return tolower((char)c);
}


extern "C"
{

void pathStrCpy(const char* src, char* dst)
{
	//4e5fd9
	if (g_PathLower)
	{
		for (const char* a = src; *a != 0; a++, dst++)
			*dst = tolower(*a);
		*dst = 0;
	}
	else
	{
		strcpy(dst, src);
	}
}


void __cdecl ReportWarning(const char* warn)
{
	//527de3
	CString str;
	str.Format("WARNING!\n%s", warn);
	puts(str);
}

void __cdecl XorData(void* data, uint32_t size)
{
	//51fd7d
	static const uint8_t xorkey[80] =
	{ 0x2e, 0xc7, 0xc4, 0x8d, 0xff, 0xe5, 0x5d, 0x0b,
		0xd6, 0xff, 0x7f, 0xff, 0xd7, 0x34, 0xd2, 0x02,
		0xe2, 0x6d, 0x9e, 0x48, 0x7b, 0xc6, 0x6a, 0xf1,
		0x97, 0x73, 0x56, 0x77, 0xfa, 0x9d, 0x80, 0x00,
		0x11, 0x04, 0x29, 0x08, 0xa6, 0x8b, 0x2a, 0x47,
		0x64, 0x00, 0x01, 0x38, 0x84, 0xa0, 0x40, 0x69,
		0x01, 0xf9, 0xfa, 0xbe, 0xea, 0xff, 0x97, 0x7b,
		0xa7, 0x26, 0xed, 0xf7, 0x6b, 0x7b, 0x3b, 0x4f,
		0x44, 0x74, 0xa3, 0x09, 0x79, 0x40, 0x38, 0x3a,
		0x20, 0x5d, 0xa3, 0x40, 0xc3, 0xe8, 0x7f, 0x3b };

	uint8_t* p = (uint8_t*)data;

	while (size != 0)
	{
		uint32_t num = size;
		if (num > 80)
			num = 80;

		const uint8_t* pkey = xorkey;
		for (int i = num; i != 0; i--)
		{
			*p ^= *pkey;
			p++;
			pkey++;
		}

		size -= num;
	}	
}




}