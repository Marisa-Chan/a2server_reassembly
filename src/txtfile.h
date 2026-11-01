#ifndef TXTFILE_H
#define TXTFILE_H

#include "assert_offset.h"
#include <afx.h>
#include <afxtempl.h>



class TxtFile : public CObject {
public:
	char* buffer = nullptr;
	uint32_t line_count = 0;
	uint32_t start_index = 0;

public:
	TxtFile();  // client: 4799b8
	~TxtFile();  // client: 43f070

	void LoadChunkFile(const char* fname);  // client: 4799f5
	void Free();  // client: 479b35

	char* GetLine(int index);  // client: 479ba3
	int GetCount();  // client: 43f130

public:
	static CArray<char*> AllLines;
};

ASSERT_SIZE(TxtFile, 0x10);





#endif TXTFILE_H
