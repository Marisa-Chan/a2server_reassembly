#ifndef UTIL_H
#define UTIL_H

#include "asm_mfc.h"
#include "mfc_templ.h"

extern int32_t g_isDosCP; //660f64


uint8_t __cdecl DecodeChar(uint8_t ch); //45e181
uint8_t __cdecl EncodeChar(uint8_t c); //4763a9
uint8_t __cdecl ToLowerChar(uint8_t c); //47641d

extern "C"
{
	void pathStrCpy(const char* src, char* dst);
	void __cdecl ReportWarning(const char* warn);
	void __cdecl XorData(void* data, uint32_t size); //51fd7d
}


template<typename TYPE, class HLDR = CList<TYPE*> >
struct Listerator
{
	HLDR *holder = nullptr;
	POSITION position = NULL;

	TYPE* Begin(HLDR &hld)
	{
		holder = &hld;
		position = hld.GetHeadPosition();

		if (position = NULL)
			return nullptr;

		return GetNext();
	}

	TYPE* GetNext()
	{
		if (position == NULL)
			return nullptr;

		return holder->GetNext(position);
	}
};


inline void SetU8(void* buf, uint8_t value) {
	*(uint8_t*)buf = value;
}

inline void SetU16(void* buf, uint16_t value) {
	*(uint16_t*)buf = value;
}

inline void SetU32(void* buf, uint32_t value) {
	*(uint32_t*)buf = value;
}


#endif