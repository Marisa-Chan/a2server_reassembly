#include "file.h"

#include <cstdlib>
#include <cstring>
#include <string>

#include "util.h"
#include "resource.h"


// 4f1d0d (client: 4f49fe) - File checksum (simple sum of int32-s).
int __cdecl CalcFileChecksum(CString filename) {
    CFile file;
    int checksum = 0;

#if defined(A2SERVER_PATCH) && !defined(A2CLIENT)
    // loading_map_bug hook (4F1D4A): resolve relative map paths against the server executable directory.
    CString open_path = filename;
    const char* raw_name = (const char*)filename;
    const bool is_abs = (raw_name[0] != '\0' && raw_name[1] == ':') ||
                        raw_name[0] == '\\' || raw_name[0] == '/';
    if (!is_abs) {
        char module_fn[MAX_PATH] = {};
        if (GetModuleFileNameA(nullptr, module_fn, MAX_PATH - 1) > 0) {
            std::string dir(module_fn);
            size_t slash_fwd = dir.rfind('/');
            size_t slash_bwd = dir.rfind('\\');
            size_t k = slash_fwd;
            if (k == std::string::npos || (slash_bwd != std::string::npos && k < slash_bwd)) {
                k = slash_bwd;
            }
            if (k != std::string::npos) {
                dir.erase(k + 1);
                dir += raw_name;
                open_path = dir.c_str();
            }
        }
    }
    if (!file.Open((const char*)open_path, 0, nullptr) &&
        (open_path == filename || !file.Open((const char*)filename, 0, nullptr))) {
        return checksum;
    }
#else
    if (!file.Open((const char*)filename, 0, nullptr)) {
        return checksum;
    }
#endif

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

// 4e67ea (client: 4ebd1a)
ResArchiveFile::ResArchiveFile() {
    this->file = nullptr;
}


// 4e6813 (client: 4ebd43)
ResArchiveFile::ResArchiveFile(LPCTSTR lpszFileName, UINT nOpenFlags) {
    if (!this->Open(lpszFileName, nOpenFlags)) {
        AfxThrowFileException(CFileException::accessDenied, -1, nullptr);
    }
}

// 4e6882 (client: 4ebdb2)
ResArchiveFile::~ResArchiveFile() {
    this->Close();
}


// 4e6d04 (client: 4ec234)
ULONGLONG ResArchiveFile::GetPosition() const {
    return this->position;
};

// 4e68d3 (client: 4ebe03)
BOOL ResArchiveFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError) {
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

// 4e6cec (client: 4ec21c)
CFile* ResArchiveFile::Duplicate() const {
    AfxThrowFileException(CFileException::accessDenied);
    return nullptr;
};

// 4e6acb (client: 4ebffb)
ULONGLONG ResArchiveFile::Seek(LONGLONG lOff, UINT nFrom) {
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

// 4e6b83 (client: 4ec0b3)
void ResArchiveFile::SetLength(ULONGLONG dwNewLen) {
    AfxThrowFileException(CFileException::accessDenied);
};

// 4e6b9b (client: 4ec0cb)
ULONGLONG ResArchiveFile::GetLength() const {
    return this->size;
};

// 4e6bac (client: 4ec0dc)
UINT ResArchiveFile::Read(void* lpBuf, UINT nCount) {
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

// 4e6c68 (client: 4ec198)
void ResArchiveFile::Write(const void* lpBuf, UINT nCount) {
    AfxThrowFileException(CFileException::accessDenied);
};

// 4e6c80 (client: 4ec1b0)
void ResArchiveFile::LockRange(ULONGLONG dwPos, ULONGLONG dwCount) {
    AfxThrowFileException(CFileException::accessDenied);
};

// 4e6c98 (client: 4ec1c8)
void ResArchiveFile::UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount) {
    AfxThrowFileException(CFileException::accessDenied);
};

// 4e6cb0 (client: 4ec1e0)
void ResArchiveFile::Abort() {
};

// 4e6cbb (client: 4ec1eb)
void ResArchiveFile::Flush() {
};

// 4e6cc6 (client: 4ec1f6)
void ResArchiveFile::Close() {
    Resources::ResFiles.CloseFile(this->file);
    this->file = nullptr;
};


// 4e6a97 (client: 4ebfc7)
void ResArchiveFile::Rewind() {
    this->Seek(0, 0);
}
