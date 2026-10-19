#include "txtfile.h"
#include "file.h"

//660d68
CArray<char*> TxtFile::AllLines;

//475c6c
TxtFile::TxtFile() = default;

TxtFile::~TxtFile()
{
	//43ccd0

	this->Free();
}

void TxtFile::Free()
{
	//475de9
	
	if (this->buffer)
	{
		for (int i = 0; i < AllLines.GetSize(); i++)
		{
			if (AllLines[i] == this->buffer)
			{
				AllLines.RemoveAt(i, this->line_count);
				free(this->buffer);
				this->buffer = nullptr;
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

	this->buffer = (char*)malloc(fsize + 1);
	this->buffer[fsize] = 0;

	fil.Read(this->buffer, fsize);

	this->line_count = 0;
	this->start_index = AllLines.GetSize();

	for (char *pcur = this->buffer; pcur < this->buffer + fsize; )
	{
		char* linestart = pcur;
		AllLines.Add(linestart);

		for(; *pcur != '\r' && pcur < this->buffer + fsize; pcur++ )
		{}

		if (pcur >= this->buffer + fsize) {
			break;
		}

		*pcur = 0;
		pcur += 2;

		this->line_count++;
		CharToOemA(linestart, linestart);
	}
}

char* TxtFile::GetLine(int index)
{
	//475e57
	return AllLines[this->start_index + index];
}


int TxtFile::GetCount()
{
	//43cd30
	return this->line_count;
}