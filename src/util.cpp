#include "util.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

int g_PathLower = 1;


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




}