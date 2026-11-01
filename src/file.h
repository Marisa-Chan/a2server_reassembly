#ifndef FILE_H
#define FILE_H

#include <cstdint>
#include <afx.h>

#include "assert_offset.h"
#include <afxtempl.h>


int __cdecl CalcFileChecksum(CString filename); // File checksum (simple sum of int32-s). 4f1d0d (client: 4f49fe).


class File2 : public CFile {
public:
	virtual ~File2();  // client: 4ebdb2

	virtual ULONGLONG GetPosition() const override;  // client: 4ec234
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL) override;  // client: 4ebe03
	virtual CFile* Duplicate() const override;  // client: 4ec21c
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom) override;  // client: 4ebffb
	virtual void SetLength(ULONGLONG dwNewLen) override;  // client: 4ec0b3
	virtual ULONGLONG GetLength() const override;  // client: 4ec0cb

	virtual UINT Read(void* lpBuf, UINT nCount) override;  // client: 4ec0dc
	virtual void Write(const void* lpBuf, UINT nCount) override;  // client: 4ec198

	virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount) override;  // client: 4ec1b0
	virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount) override;  // client: 4ec1c8

	virtual void Abort() override;  // client: 4ec1e0
	virtual void Flush() override;  // client: 4ec1eb
	virtual void Close() override;  // client: 4ec1f6


	File2();  // client: 4ebd1a
	File2(LPCTSTR lpszFileName, UINT nOpenFlags);  // client: 4ebd43

	void Rewind();  // client: 4ebfc7


public:
    CFile* file;
    int32_t offset;
    int32_t size;
    int32_t position;
};

// own data members are file/offset/size/position (0x10); the CFile base is
// larger in MFC140 (binary total was 0x20)
ASSERT_SIZE(File2, sizeof(CFile) + 0x10);


#endif
