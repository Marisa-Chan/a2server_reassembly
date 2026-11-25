#ifndef FILE_H
#define FILE_H

#include <cstdint>
#include <afx.h>

#include "assert_offset.h"
#include <afxtempl.h>


int __cdecl CalcFileChecksum(CString filename); // 4f1d0d (client: 4f49fe) - File checksum (simple sum of int32-s).


class ResArchiveFile : public CFile {
public:
    virtual ~ResArchiveFile(); // 4e6882 (client: 4ebdb2)

    virtual ULONGLONG GetPosition() const override; // 4e6d04 (client: 4ec234)
    virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL) override; // 4e68d3 (client: 4ebe03)
    virtual CFile* Duplicate() const override; // 4e6cec (client: 4ec21c)
    virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom) override; // 4e6acb (client: 4ebffb)
    virtual void SetLength(ULONGLONG dwNewLen) override; // 4e6b83 (client: 4ec0b3)
    virtual ULONGLONG GetLength() const override; // 4e6b9b (client: 4ec0cb)

    virtual UINT Read(void* lpBuf, UINT nCount) override; // 4e6bac (client: 4ec0dc)
    virtual void Write(const void* lpBuf, UINT nCount) override; // 4e6c68 (client: 4ec198)

    virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount) override; // 4e6c80 (client: 4ec1b0)
    virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount) override; // 4e6c98 (client: 4ec1c8)

    virtual void Abort() override; // 4e6cb0 (client: 4ec1e0)
    virtual void Flush() override; // 4e6cbb (client: 4ec1eb)
    virtual void Close() override; // 4e6cc6 (client: 4ec1f6)


    ResArchiveFile(); // 4e67ea (client: 4ebd1a)
    ResArchiveFile(LPCTSTR lpszFileName, UINT nOpenFlags); // 4e6813 (client: 4ebd43)

    void Rewind(); // 4e6a97 (client: 4ebfc7)


public:
    CFile* file;
    int32_t offset;
    int32_t size;
    int32_t position;
};

// own data members are file/offset/size/position (0x10); the CFile base is
// larger in MFC140 (binary total was 0x20)
ASSERT_SIZE(ResArchiveFile, sizeof(CFile) + 0x10);


#endif
