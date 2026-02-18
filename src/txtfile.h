#ifndef TXTFILE_H
#define TXTFILE_H

#include "assert_offset.h"
#include "asm_mfc.h"
#include "mfc_templ.h"



class TxtFile : public CObject
{
public:
	char* buffer = nullptr;
	uint32_t line_count = 0;
	uint32_t start_index = 0;

public:
	TxtFile();
	~TxtFile();

	void LoadChunkFile(const char* fname);
	void Free();

	char* GetLine(int index);
	int GetCount();

public:
	static CArray<char*> AllLines;
};

ASSERT_SIZE(TxtFile, 0x10);





#endif TXTFILE_H