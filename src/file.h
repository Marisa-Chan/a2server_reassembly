#ifndef FILE_H
#define FILE_H

#include <cstdint>

#include "assert_offset.h"
#include "mfc_templ.h"


class File2 : public CFile {
public:
	virtual ~File2();

	virtual DWORD GetPosition() const override;
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL) override;
	virtual CFile* Duplicate() const override;
	virtual LONG Seek(LONG lOff, UINT nFrom) override;
	virtual void SetLength(DWORD dwNewLen) override;
	virtual DWORD GetLength() const override;

	virtual UINT Read(void* lpBuf, UINT nCount) override;
	virtual void Write(const void* lpBuf, UINT nCount) override;

	virtual void LockRange(DWORD dwPos, DWORD dwCount) override;
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount) override;

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
