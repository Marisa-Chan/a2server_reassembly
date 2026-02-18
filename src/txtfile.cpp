#include "txtfile.h"
#include "file.h"

//660d68
CArray<char*> TxtFile::AllLines;

//475c6c
TxtFile::TxtFile() = default;

TxtFile::~TxtFile()
{
	//43ccd0

	Free();
}

void TxtFile::Free()
{
	//475de9
	
	if (buffer)
	{
		for (int i = 0; i < AllLines.GetSize(); i++)
		{
			if (AllLines[i] == buffer)
			{
				AllLines.RemoveAt(i, line_count);
				free(buffer);
				buffer = nullptr;
				return;
			}
		}
	}
}

void TxtFile::LoadChunkFile(const char* fname)
{
	//475ca9

	File2 fil;
	fil.Open(fname, 0);

	uint32_t fsize = fil.GetLength();

	buffer = (char*)malloc(fsize + 1);
	buffer[fsize] = 0;

	fil.Read(buffer, fsize);

	line_count = 0;
	start_index = AllLines.GetSize();

	for (char *pcur = buffer; pcur < buffer + fsize; )
	{
		char* linestart = pcur;
		AllLines.Add(linestart);

		for(; *pcur != '\r' && pcur < buffer + fsize; pcur++ )
		{}

		if (pcur >= buffer + fsize)
			break;

		*pcur = 0;
		pcur += 2;

		line_count++;
		CharToOemA(linestart, linestart);
	}
}

char* TxtFile::GetLine(int index)
{
	//475e57
	return AllLines[start_index + index];
}


int TxtFile::GetCount()
{
	//43cd30
	return line_count;
}