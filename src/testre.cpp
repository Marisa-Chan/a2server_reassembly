#include <stdio.h>
#include "token.h"


namespace A2 
{

#define CHECKSIZE(__name,__size) \
		if (sizeof(__name) != __size)\
		{\
			printf(#__name" size %x but must be %x\n", sizeof(__name), __size);\
			res = false;\
		}\
		else\
			printf(#__name" size == %x OK\n", sizeof(__name));

extern "C"
{
	const void** __cdecl GetVtblToken();
}


bool TestVtbl(const char *className, const void** vt, const void** mt, int ignoreid = -1)
{
	bool res = true;

	for (int i = 0; i < 5; i++)
	{
		if (i != ignoreid && vt[i] != mt[i])
		{
			printf("Error in %s vtable %x  vt %p !=  m %p\n", className, i, vt[i], mt[i]);
			res = false;
		}
	}

	return res;
}

bool TestTokenVtbl()
{
	// Fill array with vtbl methods addresses
	const void* m[14];
	__asm
	{
		mov eax, (Token::GetRuntimeClass)
		mov m[0 * 4], eax
		mov eax, (Token::~Token)
		mov m[1 * 4], eax
		mov eax, (Token::Serialize)
		mov m[2 * 4], eax
		mov eax, (Token::AssertValid)
		mov m[3 * 4], eax
		mov eax, (Token::Dump)
		mov m[4 * 4], eax
		mov eax, (Token::VMethod1)
		mov m[5 * 4], eax
		mov eax, (Token::VMethod2)
		mov m[6 * 4], eax
		mov eax, (Token::VMethod3)
		mov m[7 * 4], eax
		mov eax, (Token::VMethod4)
		mov m[8 * 4], eax
		mov eax, (Token::VMethod5)
		mov m[9 * 4], eax
		mov eax, (Token::VMethod6)
		mov m[10 * 4], eax
		mov eax, (Token::VMethod7)
		mov m[11 * 4], eax
		mov eax, (Token::VMethod8)
		mov m[12 * 4], eax
		mov eax, (Token::VMethod9)
		mov m[13 * 4], eax
	}

	// ignore vtable 1 "destructor", because in vtbl it's "eh destructor"
	return TestVtbl("Token", GetVtblToken(), m, 1);
}


bool TestReversing()
{
	bool res = true;
	const char* err = nullptr;

	CHECKSIZE(TokenPos, 0xC);
	CHECKSIZE(Token, 0x3C);

	TestTokenVtbl();

	return res;	
}


}
