#include "file.h"
#include "util.h"
#include "resource.h"

File2::File2()
{
	//4e67ea
	file = nullptr;
}


File2::File2(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	//4e67ea
	if (!Open(lpszFileName, nOpenFlags))
		AfxThrowFileException(CFileException::accessDenied, -1, nullptr);
}

File2::~File2()
{
	//4e6882
	Close();
}


DWORD File2::GetPosition() const 
{
	//4e6d04
	return position;
};

BOOL File2::Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError) 
{
	//4e68d3
	char fname[1024];
	pathStrCpy(lpszFileName, fname);

	if (nOpenFlags & modeWrite)
		return FALSE;

	m_strFileName = fname;

	char fbuf[1024];
	strcpy(fbuf, fname);

	FatInfo* inf = Resources::ResFiles.FUN_004e646d(fbuf);
	if (inf == nullptr)
	{
		if (pError)
			pError->m_cause = 2;
		return FALSE;
	}
	
	file = inf->file;
	offset = inf->offset;
	size = inf->size;

	delete inf;

	position = 0;
	m_hFile = file->m_hFile;

	if (pError)
		pError->m_cause = 0;

	return TRUE;
};

CFile* File2::Duplicate() const 
{
	//4e6cec
	AfxThrowFileException(CFileException::accessDenied);
	return nullptr;
};

LONG File2::Seek(LONG lOff, UINT nFrom) 
{
	//4e6acb
	if (m_hFile == (UINT)-1)
		AfxThrowFileException(CFileException::invalidFile);

	if (nFrom == 0)
	{
		if (size < lOff)
			AfxThrowFileException(CFileException::badSeek);
		position = lOff;
	}
	else if (nFrom == 1)
	{
		if (size < lOff + position)
			AfxThrowFileException(CFileException::badSeek);
		position = position + lOff;
	}
	else if (nFrom == 2)
	{
		if (-size != lOff && size <= -lOff)
			AfxThrowFileException(CFileException::badSeek);
		position = size + lOff;
	}
	return position;
};

void File2::SetLength(DWORD dwNewLen) 
{
	//4e6b83
	AfxThrowFileException(CFileException::accessDenied);
};

DWORD File2::GetLength() const 
{
	//4e6b9b
	return size;
};

UINT File2::Read(void* lpBuf, UINT nCount) 
{
	//4e6bac
	if (m_hFile == (UINT)-1)
		AfxThrowFileException(CFileException::invalidFile);

	UINT toRead = 0;
	if (nCount < size - position)
		toRead = nCount;
	else
		toRead = size - position;

	file->Seek(offset + position, 0);

	if (toRead == 0)
		return 0;

	position += toRead;
	
	if (size < position)
		AfxThrowFileException(CFileException::endOfFile);

	return file->Read(lpBuf, toRead);
};

void File2::Write(const void* lpBuf, UINT nCount) 
{
	//4e6c68
	AfxThrowFileException(CFileException::accessDenied);
};

void File2::LockRange(DWORD dwPos, DWORD dwCount) 
{
	//4e6c80
	AfxThrowFileException(CFileException::accessDenied);
};

void File2::UnlockRange(DWORD dwPos, DWORD dwCount) 
{
	//4e6c98
	AfxThrowFileException(CFileException::accessDenied);
};

void File2::Abort() 
{
	//4e6cb0
};

void File2::Flush() 
{
	//4e6cbb
};

void File2::Close() 
{
	//4e6763
	Resources::ResFiles.FUN_004e6763(file);
	file = nullptr;
};


void File2::Rewind()
{
	//4e6a97
	Seek(0, 0);
}