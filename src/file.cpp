#include "file.h"

#include <cstdlib>
#include <cstring>

#include "util.h"
#include "resource.h"


int __cdecl sub_4F1D0D(CString filename) {
	CFile file;
	int checksum = 0;

	if (!file.Open((const char*)filename, 0, nullptr)) {
		return checksum;
	}

	const uint32_t file_length = file.GetLength();
	const uint32_t padded_length = (file_length & 0xfffffffc) + 4;
	int32_t* buffer = static_cast<int32_t*>(malloc(padded_length));
	if (buffer == nullptr) {
		file.Close();
		return checksum;
	}

	memset(buffer, 0, padded_length);
	file.Read(buffer, file_length);
	file.Close();

	const uint32_t word_count = padded_length / sizeof(int32_t);
	for (uint32_t index = 0; index < word_count; index++) {
		checksum += buffer[index];
	}

	free(buffer);
	return checksum;
}

File2::File2() {
	//4e67ea
	this->file = nullptr;
}


File2::File2(LPCTSTR lpszFileName, UINT nOpenFlags) {
	//4e6813
	if (!this->Open(lpszFileName, nOpenFlags)) {
		AfxThrowFileException(CFileException::accessDenied, -1, nullptr);
	}
}

File2::~File2() {
	//4e6882
	this->Close();
}


ULONGLONG File2::GetPosition() const {
	//4e6d04
	return this->position;
};

BOOL File2::Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError) {
	//4e68d3
	char fname[1024];
	pathStrCpy(lpszFileName, fname);

	if (nOpenFlags & modeWrite) {
		return FALSE;
	}

	m_strFileName = fname;

	char fbuf[1024];
	strcpy(fbuf, fname);

	FatInfo* inf = Resources::ResFiles.GetFatInfo(fbuf);
	if (inf == nullptr) {
		if (pError) {
			pError->m_cause = 2;
		}
		return FALSE;
	}
	
	this->file = inf->file;
	this->offset = inf->offset;
	this->size = inf->size;

	delete inf;

	this->position = 0;
	m_hFile = this->file->m_hFile;

	if (pError) {
		pError->m_cause = 0;
	}

	return TRUE;
};

CFile* File2::Duplicate() const {
	//4e6cec
	AfxThrowFileException(CFileException::accessDenied);
	return nullptr;
};

ULONGLONG File2::Seek(LONGLONG lOff, UINT nFrom) {
	//4e6acb
	if (m_hFile == (HANDLE)-1) {
		AfxThrowFileException(CFileException::invalidFile);
	}

	if (nFrom == 0) {
		if (this->size < lOff) {
			AfxThrowFileException(CFileException::badSeek);
		}
		this->position = lOff;
	} else if (nFrom == 1) {
		if (this->size < lOff + this->position) {
			AfxThrowFileException(CFileException::badSeek);
		}
		this->position = this->position + lOff;
	} else if (nFrom == 2) {
		if (-this->size != lOff && size <= -lOff) {
			AfxThrowFileException(CFileException::badSeek);
		}
		this->position = size + lOff;
	}
	return this->position;
};

void File2::SetLength(ULONGLONG dwNewLen) {
	//4e6b83
	AfxThrowFileException(CFileException::accessDenied);
};

ULONGLONG File2::GetLength() const {
	//4e6b9b
	return this->size;
};

UINT File2::Read(void* lpBuf, UINT nCount) {
	//4e6bac
	if (m_hFile == (HANDLE)-1) {
		AfxThrowFileException(CFileException::invalidFile);
	}

	UINT toRead = 0;
	if (nCount < this->size - this->position) {
		toRead = nCount;
	} else {
		toRead = this->size - this->position;
	}

	this->file->Seek(this->offset + this->position, 0);

	if (toRead == 0) {
		return 0;
	}

	this->position += toRead;
	
	if (this->size < this->position) {
		AfxThrowFileException(CFileException::endOfFile);
	}

	return this->file->Read(lpBuf, toRead);
};

void File2::Write(const void* lpBuf, UINT nCount) {
	//4e6c68
	AfxThrowFileException(CFileException::accessDenied);
};

void File2::LockRange(ULONGLONG dwPos, ULONGLONG dwCount) {
	//4e6c80
	AfxThrowFileException(CFileException::accessDenied);
};

void File2::UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount) {
	//4e6c98
	AfxThrowFileException(CFileException::accessDenied);
};

void File2::Abort() {
	//4e6cb0
};

void File2::Flush() {
	//4e6cbb
};

void File2::Close() {
	//4e6cc6
	Resources::ResFiles.FUN_004e6763(this->file);
	this->file = nullptr;
};


void File2::Rewind() {
	//4e6a97
	this->Seek(0, 0);
}
