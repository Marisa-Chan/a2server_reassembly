#ifndef UTIL_H
#define UTIL_H

#include "asm_mfc.h"
#include "mfc_templ.h"

extern "C"
{
	void pathStrCpy(const char* src, char* dst);
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


#endif