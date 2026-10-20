#ifndef FILE_H
#define FILE_H

#include <cstdint>
#include <afx.h>

#include "assert_offset.h"
#include <afxtempl.h>


int __cdecl CalcFileChecksum(CString filename); // File checksum (simple sum of int32-s). 4f1d0d.


class File2 : public CFile {
public:
	virtual ~File2();

	virtual ULONGLONG GetPosition() const override;
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL) override;
	virtual CFile* Duplicate() const override;
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom) override;
	virtual void SetLength(ULONGLONG dwNewLen) override;
	virtual ULONGLONG GetLength() const override;

	virtual UINT Read(void* lpBuf, UINT nCount) override;
	virtual void Write(const void* lpBuf, UINT nCount) override;

	virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount) override;
	virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount) override;

	virtual void Abort() override;
	virtual void Flush() override;
	virtual void Close() override;


	File2();
	File2(LPCTSTR lpszFileName, UINT nOpenFlags);

	void Rewind();


public:
    CFile* file;
    int32_t offset;
    int32_t size;
    int32_t position;
};

ASSERT_SIZE(File2, 0x20);


#endif
