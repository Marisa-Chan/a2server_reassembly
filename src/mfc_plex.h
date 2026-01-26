#ifndef MFC_PLEX_H
#define MFC_PLEX_H

#include <windows.h>

struct CPlex     // warning variable length structure
{
	CPlex* pNext;
	// BYTE data[maxNum*elementSize];

	void* data() { return this + 1; }

	static CPlex* PASCAL Create(CPlex*& head, UINT nMax, UINT cbElement);
	// like 'calloc' but no zero fill
	// may throw memory exceptions

	void FreeDataChain();       // free this one and links
};


#endif