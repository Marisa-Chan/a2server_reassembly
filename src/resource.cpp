#include "resource.h"
#include <string>
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include "file.h"

//6a6a20
Resources Resources::ResFiles;

//6a6a60
FatEntry* ResBase::g_lastDstRoot = nullptr;


// 4e6de0 (client: 4ec310)
uint16_t Resources::GetPathType(const char* path) {

    WIN32_FIND_DATAA fnd;
    HANDLE hfind = FindFirstFileA(path, &fnd);
    FindClose(hfind);
    if (hfind == INVALID_HANDLE_VALUE) {
        char letter = tolower(*path);

        char buf[104];
        DWORD num = GetLogicalDriveStringsA(100, buf);
        for (DWORD i = 0; i < num; i++) {
            if (isalpha(buf[i])) {
                buf[i] = tolower(buf[i]);
            }
        }
        if (isalnum(letter) &&
            strcmp(path + 1, ":\\") == 0 &&
            memchr(buf, letter, num - 2) != NULL) {
            return 2;
        }

        return 0;
    } else if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        return 1;
    }
    
    return 2;
}

// 4e5e06 (client: 4eb336)
Resources::Resources() {
#if defined(A2CLIENT_PATCH) || defined(A2SERVER_PATCH)
    // Tangar a2mgr DllMain_Init + BASE_setCurrentDirectory1 (client 486534, 5A4988):
    // pin the process working directory to the executable directory before search_paths[0]
    // is captured during static initialization.
    char exe_path[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, exe_path, MAX_PATH - 1) > 0) {
        char* slash = strrchr(exe_path, '\\');
        char* fslash = strrchr(exe_path, '/');
        if (fslash != nullptr && (slash == nullptr || fslash > slash)) {
            slash = fslash;
        }
        if (slash != nullptr) {
            *slash = '\0';
            SetCurrentDirectoryA(exe_path);
        }
    }
#endif

    char path[260];
    GetCurrentDirectoryA(256, path);
    this->AddPath(path);
}

// 4e5e9f (client: 4eb3cf)
Resources::~Resources() {

    for (uint32_t i = 0; i < this->res_files.GetSize(); i++) {
        if (this->res_files[i] != nullptr) {
            delete this->res_files[i];
        }
    }

    for (uint32_t i = 0; i < this->files.GetSize(); i++) {
        if (this->files[i] != nullptr) {
            delete this->files[i];
        }
    }
}

// 4e6273 (client: 4eb7a3)
void Resources::AddPath(const char* path) {
    if (GetPathType(path) != 2) {
        #ifdef A2SERVER_PATCH
        ::MessageBoxA(NULL, path, "Ignoring resource path", MB_OK);
        return;
        #elif A2CLIENT_PATCH
        ::MessageBoxA(NULL, path, "Ignoring resource path", MB_OK);
        return;
        #else
        AfxThrowFileException(CFileException::badPath);
        #endif
    }

    char buf[1024];
    pathStrCpy(path, buf);

    this->search_paths.Add(buf);
}

// 4e605a (client: 4eb58a)
void Resources::AddResFile(const char* path) {
    char buf[1024];
    pathStrCpy(path, buf);

    if (this->search_paths.GetSize() <= 0) {
        AfxThrowFileException(CFileException::fileNotFound);
    }

    char pbuf[256];
    strcpy(pbuf, (const char*)this->search_paths[0]);
    size_t len = strlen(pbuf);
    if (pbuf[len - 1] != '\\' || pbuf[len - 2] != ':') {
        strcat(pbuf, "\\");
    }
    strcat(pbuf, buf);

    this->res_files.Add(new ResFile(pbuf));
}

#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
bool Resources::TryAddResFile(const char* path) {
    char buf[1024];
    pathStrCpy(path, buf);

    if (this->search_paths.GetSize() <= 0) {
        return false;
    }

    char pbuf[256];
    strcpy(pbuf, (const char*)this->search_paths[0]);
    size_t len = strlen(pbuf);
    if (pbuf[len - 1] != '\\' || pbuf[len - 2] != ':') {
        strcat(pbuf, "\\");
    }
    strcat(pbuf, buf);

    char check_buf[260];
    strcpy(check_buf, pbuf);
    if (strchr(pbuf, '.') == nullptr) {
        strcat(check_buf, ".res");
    }
    DWORD attr = GetFileAttributesA(check_buf);
    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return false;
    }

    this->res_files.Add(new ResFile(pbuf));
    return true;
}
#endif




// 4e6242 (client: 4eb772)
void Resources::StaticAddPath(const char* path) {
    char buf[1024];
    pathStrCpy(path, buf);
    ResFiles.AddPath(buf);
}

// 4e6211 (client: 4eb741)
void Resources::StaticAddResFile(const char* path) {
#ifdef A2CLIENT_PATCH
    // RES_noLoad hook (client 4EB741): unpacked assets under data\ replace .res archives.
    (void)path;
    return;
#else
    char buf[1024];
    pathStrCpy(path, buf);
    ResFiles.AddResFile(buf);
#endif
}

#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
bool Resources::TryStaticAddResFile(const char* path) {
#ifdef A2CLIENT_PATCH
    (void)path;
    return true;
#else
    char buf[1024];
    pathStrCpy(path, buf);
    return ResFiles.TryAddResFile(buf);
#endif
}
#endif


#ifdef A2CLIENT_PATCH
// RES_loadFile hook (client 4EBEDA): before the regular archive lookup, try
// the loose-file overrides "data\patch\<file>" then "data\<file>" for files
// outside the known archive prefixes (TestMap in the manager)
static bool TangarTestMapPath(const char* file);
#endif

// 4e646d (client: 4eb99d)
FatInfo* Resources::GetFatInfo(const char* fpath) {
    char buf[1024];
    pathStrCpy(fpath, buf);

#ifdef A2CLIENT_PATCH
    if (!TangarTestMapPath(buf)) {
        char dbuf[1024];
        _snprintf(dbuf, sizeof(dbuf), "data\\patch\\%s", buf);
        FatInfo* inf = this->GetFatInfo(dbuf);
        if (inf != nullptr) {
            return inf;
        }
        _snprintf(dbuf, sizeof(dbuf), "data\\%s", buf);
        inf = this->GetFatInfo(dbuf);
        if (inf != nullptr) {
            return inf;
        }
    }
#endif

    for (uint32_t i = 0; i < this->res_files.GetSize(); i++) {
        FatInfo* inf = this->res_files[i]->GetFatInfo(buf);
        if (inf != nullptr) {
            if (inf != FATINFO_T1) {
                return inf;
            }
            break;
        }
    }

    CFile* cfile = new CFile();
    for (uint32_t i = 0; i < this->search_paths.GetSize(); i++) {
        char dbuf[256];
        strcpy(dbuf, this->search_paths[i]);

        int ln = strlen(dbuf);
        if (dbuf[ln - 1] != '\\' || dbuf[ln - 2] != ':') {
            strcat(dbuf, "\\");
        }

        strcat(dbuf, buf);

        CFileException exc;
        if (cfile->Open(dbuf, 0, &exc) == FALSE) {
            if (exc.m_cause != CFileException::fileNotFound && exc.m_cause != CFileException::badPath) {
                AfxThrowFileException(exc.m_cause);
            }
        } else {
            this->files.Add(cfile);
            FatInfo* inf = new FatInfo();
            inf->file = cfile;
            inf->offset = 0;
            inf->size = cfile->GetLength();
            return inf;
        }
    }

    if (cfile) {
        delete cfile;
    }

    return nullptr;
}

// 4e6763 (client: 4ebc93)
void Resources::CloseFile(CFile* fil) {
    for (int i = 0; i < this->files.GetSize(); i++) {
        if (this->files[i] == fil) {
            delete fil;
            this->files.RemoveAt(i);
            break;
        }
    }
}

// 4e6409 (client: 4eb939)
void Resources::MarkFileSubstituted(const char* fname) {
    for (int i = 0; i < this->res_files.GetSize(); i++) {
        FatEntry *fat = this->res_files[i]->FindEntry(fname);
        if (fat) {
            fat->flags.is_substituted = 1;
            break;
        }
    }
}

// 4e632d (client: 4eb85d)
void Resources::MarkSubstitutedFiles(const char* fname) {
    FILE* f = fopen(fname, "r");
    if (!f) {
        return;
    }

    char bufline[256];
    while (fgets(bufline, 255, f) && isprint(bufline[0])) {
        char* pc = strchr(bufline, '\n');
        if (pc) {
            *pc = 0;
        }

        pc = strchr(bufline, '\r');
        if (pc) {
            *pc = 0;
        }

        ResFiles.MarkFileSubstituted(bufline);
    }
    fclose(f);
}




// 4e5950 (client: 4eae80)
ResFile::ResFile(const char* fpath) {
    this->modify_fat = nullptr;

    char buf[260];
    strcpy(buf, fpath);

    const char *puVar1 = strchr(fpath, '.');

    if (puVar1 == NULL) {
        strcat(buf, ".res");    
    }

    this->memory_size = 0;

    
    this->file = new CFile(buf, this->memory_size | CFile::shareDenyNone);

    char *pext = strchr(buf, '.');
    *pext = 0;

    char* pchar = pext;
    while (*pchar != '\\' && *pchar != '/' && pchar != buf) {
        pchar--;
    }

    if (pchar != buf) {
        pchar++;
    }

    this->root.SetName(pchar);

    *pext = '.';

    this->file->Read(&this->root.magic, 4);

    if (this->root.magic != RES_ARCHIVE_MAGIC) {
        AfxThrowFileException(CFileException::genericException);
    }
    
    this->file->Read(&this->root.offset, 4);
    this->file->Read(&this->root.size, 4);
    this->file->Read(&this->root.flags, 4);
    this->file->Read(&this->fat_offset, 4);
    this->file->Read(&this->fat_size, 4);

    if ((this->memory_size & 0xf) == 0) {
        this->memory_size = 0;
    } else {
        this->memory_size = 1;
    }

    if (this->memory_size == 0) {
        this->size = this->fat_size;
        this->fat_data = (FatEntry*)malloc(this->size * sizeof(FatEntry));

        if (this->fat_data == nullptr) {
            AfxThrowMemoryException();
        }
    } else {
        this->size = this->fat_size + 10000 + (this->fat_size / 4);
        this->fat_data = (FatEntry*)malloc(this->size * sizeof(FatEntry));

        if (this->fat_data == nullptr) {
            AfxThrowMemoryException();
        }
    }

    if (this->root.flags.fat_inline == 0) {
        this->file->Seek(this->fat_offset, 0);
    }

    this->file->Read(this->fat_data, this->fat_size << 5);

    this->modify_fat = nullptr;
    this->content_size = 0;
}

// 4e5d0b (client: 4eb23b)
ResFile::~ResFile() {
    if (this->fat_data) {
        free(this->fat_data);
    }

    if (this->file) {
        delete this->file;
    }
}


// 4e84c0 (client: 4ed9f0)
RegFile::RegFile() {
    this->modify_fat = nullptr;
    this->fat_offset = 0x20;

    this->pdata = (uint8_t *)malloc(this->fat_offset);

    if (this->pdata == nullptr) {
        AfxThrowMemoryException();
    }

    this->memory_size = 0;
    this->size = 4;

    this->fat_data = (FatEntry *)malloc(this->size * sizeof(FatEntry));

    if (this->fat_data == nullptr) {
        AfxThrowMemoryException();
    }

    this->fat_size = 0;
    this->root.SetName("");
    this->root.magic = RES_ARCHIVE_MAGIC;
    this->root.offset = 0;
    this->root.size = 0;
    this->root.flags = FatEntryFlags::FLAG_DIRECTORY;
    this->content_size = 0;
}


// 4e8585 (client: 4edab5)
RegFile::RegFile(const char* fpath) {

    this->modify_fat = nullptr;

    File2 *cfile = new File2();

    CFileException exc;
    if (cfile->Open(fpath, 0, &exc) == FALSE) {
        if (cfile) {
            delete cfile;
        }
        
        char buf[256];
        sprintf(buf, " Could`t create registry file :%d", exc.m_cause);
        throw std::runtime_error(buf);
    }

    const char *pdot = strchr(fpath, '.');
    const char* pcur = pdot - 1;

    while (*pcur != '\\' && *pcur != '/' && pcur != fpath) {
        pcur--;
    }

    if (pcur != fpath) {
        pcur++;
    }

    int cpylen = pdot - pcur;

    if (cpylen > 15) {
        cpylen = 15;
    }

    strncpy(this->root.name, pcur, cpylen);

    this->file = nullptr;
    this->fat_data = nullptr;

    this->ReadFromFile(cfile);

    if (cfile != nullptr) {
        delete cfile;
    }
}

// 4e9989 (client: 4eeeb9)
RegFile::~RegFile() {
    if (this->fat_data) {
        free(this->fat_data);
    }

    if (this->pdata) {
        free(this->pdata);
    }
}




// 4eca9e (client: 4f1f4e)
FatEntry* ResBase::Find(FatEntry* node, const char* fname) {
    if (!node) {
        return nullptr;
    }

    FatEntry* searchBase = this->fat_data + node->offset;

    if (node->flags.is_sorted != 0 && node->size != 0) {
        FatEntry tmp;
        tmp.SetName(fname);
        return (FatEntry*)bsearch(&tmp, searchBase, node->size, sizeof(FatEntry), FatEntry::NameCompare);
    }

    for (int i = 0; i < node->size; i++) {
        if (_strnicmp(fname, searchBase[i].name, 15) == 0) {
            return &searchBase[i];
        }
    }

    return nullptr;
}


// 4ecb78 (client: 4f2028)
FatEntry* ResBase::FindNext(FatEntry* node, const char** fpath) {
    
    // 10.02.2026
    // fix for const char use
    char buf[256];
    int buflen = 0;

    const char* pchar = *fpath;
    while (*pchar != '\\' && *pchar != '/' && *pchar != 0) {
        buf[buflen] = *pchar;
        buflen++;

        pchar++;
    }

    buf[buflen] = 0;

    FatEntry *node_find = this->Find(node, buf);

    if (*pchar == 0) {
        *fpath = pchar;
    } else {
        *fpath = pchar + 1;
    }

    return node_find;
}

// 4ec956 (client: 4f1e06)
FatEntry* ResBase::TracePath(const char* fpath) {
    for (char* pchar = this->root.name; *fpath != '\\' && *fpath != '/' && *fpath != 0 && *pchar != 0; fpath++, pchar++) {
        if (*fpath != *pchar) {
            return nullptr;
        }
    }

    if (*fpath != 0) {
        fpath++;
    }

    FatEntry* node = &this->root;

    while (true) {
        if (*fpath == 0) {
            return node;
        }

        if (node == nullptr) { 
            break;
        }

        if ((node->flags & FatEntryFlags::DIR_OR_DELETED_MASK) != FatEntryFlags::FLAG_DIRECTORY) {
            return nullptr;
        }

        node = this->FindNext(node, &fpath);
    }
    return nullptr;
}

// 4ec8f4 (client: 4f1da4)
void ResBase::GrowFat() {

    if (this->fat_size + 1 >= this->size) {
        this->size += 1 + (this->size / 4);
        this->fat_data = (FatEntry*)realloc(this->fat_data, this->size * sizeof(FatEntry));
        if (this->fat_data == nullptr) {
            AfxThrowMemoryException();
        }
    }
}

// 4ecc05 (client: 4f20b5)
void ResBase::AccumulateSize(FatEntry* node) {
    static int s_FatWalkAbort = 0;

    if ((node->flags & FatEntryFlags::DIR_OR_DELETED_MASK) == 0) {
        node->flags.is_deleted = 1;

        int8_t tmp = node->flags.value_type;
        if (tmp != FatEntryFlags::TYPE_INT && tmp != FatEntryFlags::TYPE_DOUBLE) {
            this->content_size += node->size;
        }
    } else if ((node->flags & FatEntryFlags::DIR_OR_DELETED_MASK) == FatEntryFlags::FLAG_DIRECTORY) {
        node->flags.is_deleted = 1;
        
        uint32_t cnt = 0;
        while (cnt < node->size) {
            this->AccumulateSize(this->fat_data + node->offset + cnt);

            if (s_FatWalkAbort) {
                break;
            }

            cnt++;
        }
    }
}

// 4ec553 (client: 4f1a03)
void ResBase::CopyFatExcluding(FatEntry* newFat, int* pCount, const FatEntry* srcFat, const FatEntry* srcRoot, FatEntry* node, FatEntry* dstRoot) {

    int32_t newSize = 0;
    int32_t fatOffset = *pCount;

    if (srcRoot == node) {
        fatOffset = this->fat_size - node->size;
        g_lastDstRoot = dstRoot;
    }

    const uint32_t srcOffset = srcRoot->offset;

    dstRoot->offset = fatOffset;
    dstRoot->SetName(srcRoot->name);
    dstRoot->flags = srcRoot->flags & (~FatEntryFlags::FLAG_SORTED);
    dstRoot->magic = srcRoot->magic;

    for (uint32_t i = 0; i < srcRoot->size; i++) {
        if ((srcFat[srcOffset + i].flags & FatEntryFlags::DIR_OR_DELETED_MASK) == 0) {
            newFat[fatOffset] = srcFat[srcOffset + i];

            fatOffset++;
            newSize++;
        }
    }

    for (uint32_t i = 0; i < srcRoot->size; i++) {
        if ((srcFat[srcOffset + i].flags & FatEntryFlags::DIR_OR_DELETED_MASK) == FatEntryFlags::FLAG_DIRECTORY) {
            newSize++;
        }
    }

    if (srcRoot != node) {
        *pCount += newSize;
    }

    for (uint32_t i = 0; i < srcRoot->size; i++) {
        if ((srcFat[srcOffset + i].flags & FatEntryFlags::DIR_OR_DELETED_MASK) == FatEntryFlags::FLAG_DIRECTORY) {
            this->CopyFatExcluding(newFat, pCount, srcFat, srcFat + srcOffset + i, node, newFat + fatOffset);
            fatOffset++;
        }
    }

    dstRoot->size = newSize;
}

// 4ec438 (client: 4f18e8)
FatEntry* ResBase::RemoveEntry(FatEntry* node) {
    if (node == this->modify_fat) {
        return node;
    }

    FatEntry *newFat = (FatEntry*)malloc(this->size * sizeof(FatEntry));

    if (newFat == nullptr) {
        AfxThrowMemoryException();
        return nullptr;
    }

    int exactSize = 0;
    int aproxSize = this->fat_size - node->size;

    this->CopyFatExcluding(newFat, &exactSize, this->fat_data, &this->root, node, &this->root);

    if (aproxSize != exactSize) {
        for (uint32_t i = 0; i < g_lastDstRoot->size; i++) {
            newFat[exactSize + i] = newFat[aproxSize + i];
        }
    }

    g_lastDstRoot->offset = exactSize;

    exactSize += g_lastDstRoot->size;

    this->fat_size = exactSize;
    free(this->fat_data);

    this->fat_data = newFat;

    this->modify_fat = node;

    return g_lastDstRoot;
}



// 4ec7e0 (client: 4f1c90)
FatEntry* ResBase::AppendEntry(FatEntry** pOldNode, const char* fname) {
    FatEntry* oldFat = this->fat_data; // save old pointer

    this->GrowFat();

    if (*pOldNode != &this->root) {
        *pOldNode = this->fat_data + (*pOldNode - oldFat);
    }

    FatEntry *fndNode = this->Find(*pOldNode, fname);
    if (fndNode != nullptr) {
        this->AccumulateSize(fndNode);
    }
    
    this->fat_size++;

    FatEntry *node = this->fat_data + (*pOldNode)->offset + (*pOldNode)->size;

    (*pOldNode)->size++;
    (*pOldNode)->flags.is_sorted = 0;

    node->SetName(fname);
    node->magic = 0;
    return node;
}


// 4ec736 (client: 4f1be6)
void ResBase::SortTree(FatEntry* root) {
    for (uint32_t i = 0; i < root->size; i++) {
        if ((this->fat_data[root->offset + root->size].flags & FatEntryFlags::DIR_OR_DELETED_MASK) == FatEntryFlags::FLAG_DIRECTORY) {
            this->SortTree(this->fat_data + root->offset + root->size);
        }
    }
    if (root->size != 0) {
        qsort(this->fat_data + root->offset, root->size, sizeof(FatEntry), FatEntry::NameCompare);
        root->flags.is_sorted = 1;
    }
}

// 4e8396 (client: 4ed8c6)
void ResBase::WriteToFile(CFile* cfile, int32_t offset) {
    this->SortTree(&this->root);

    if (offset != -1) {
        cfile->Seek(offset, 0);
    }

    cfile->Write(&this->root.magic, 4);
    cfile->Write(&this->root.offset, 4);
    cfile->Write(&this->root.size, 4);
    cfile->Write(&this->root.flags, 4);
    cfile->Write(&this->fat_size, 4);
    cfile->Write(&this->content_size, 4);
    cfile->Write(this->fat_data, this->fat_size * sizeof(FatEntry));
    cfile->Write(&this->memory_size, 4);
    cfile->Write(this->pdata, this->memory_size);
}

// 4e819c (client: 4ed6cc)
void ResBase::ReadFromFile(CFile* cfile, int32_t offset) {
    if (offset != -1) { 
        this->file->Seek(offset, 0);
    }
    
    cfile->Read(&this->root.magic, 4);

    if (this->root.magic != RES_ARCHIVE_MAGIC) {
        throw std::runtime_error("bad signature");
    }

    cfile->Read(&this->root.offset, 4);
    cfile->Read(&this->root.size, 4);
    cfile->Read(&this->root.flags, 4);
    cfile->Read(&this->fat_size, 4);
    cfile->Read(&this->content_size, 4);

    if (this->fat_data == nullptr) {
        this->fat_data = (FatEntry*)malloc(this->fat_size * sizeof(FatEntry));
    } else {
        if (this->fat_size == 0) {
            this->fat_data = (FatEntry*)realloc(this->fat_data, 1 * sizeof(FatEntry));
        } else {
            this->fat_data = (FatEntry*)realloc(this->fat_data, this->fat_size * sizeof(FatEntry));
        }
    }

    if (this->fat_data == nullptr) {
        AfxThrowMemoryException();
        return;
    }

    cfile->Read(this->fat_data, this->fat_size * sizeof(FatEntry));

    this->size = this->fat_size;

    cfile->Read(&this->memory_size, 4);

    this->fat_offset = this->memory_size;

    if (this->pdata == nullptr) {
        this->pdata = (uint8_t *)malloc(this->memory_size);
    } else {
        if (this->memory_size == 0) {
            this->pdata = (uint8_t*)realloc(this->pdata, 1);
        } else {
            this->pdata = (uint8_t*)realloc(this->pdata, this->memory_size);
        }
    }

    if (this->pdata == nullptr) {
        AfxThrowMemoryException();
        return;
    }

    if (this->memory_size != 0) {
        cfile->Read(this->pdata, this->memory_size);
    }
}

// 4ec8c6 (client: 4f1d76)
FatEntry* ResBase::CreateEntry(const char* fpath, const char* fname) {
    FatEntry* pathNode = this->TracePath(fpath);
    return this->AppendEntry(&pathNode, fname);
}

// 4ea183 (client: 4ef6b3)
FatEntry* ResBase::RecreateEntry(const char* fpath, const char* fname) {
    FatEntry *pathNode = this->TracePath(fpath);
    if (pathNode == nullptr) {
        return nullptr;
    }

    this->RemoveEntry(pathNode);

    FatEntry* node = this->CreateEntry(fpath, fname);
    node->offset = this->fat_size;
    node->size = 0;
    node->flags = FatEntryFlags::FLAG_DIRECTORY;
    return node;
}

// 4eca16 (client: 4f1ec6)
FatEntry* ResBase::RequireFileEntry(const char* fpath) {
    FatEntry* node = this->TracePath(fpath);
    if (node == nullptr) {
        AfxThrowFileException(CFileException::fileNotFound);
    }

    if ((node->flags & FatEntryFlags::DIR_OR_DELETED_MASK) == 0) {
        AfxThrowFileException(CFileException::badPath);
    }

    return node;
}

// 4eca63 (client: 4f1f13)
FatEntry* ResBase::FindEntry(const char* fpath) {
    FatEntry* node = this->TracePath(fpath);

    if (node != nullptr && (node->flags & (FatEntryFlags::FLAG_DELETED | FatEntryFlags::FLAG_SORTED)) != 0) {
        return nullptr;
    }

    return node;
}

// 4e5d55 (client: 4eb285)
FatInfo* ResBase::GetFatInfo(const char* fpath) {
    FatEntry* node = this->FindEntry(fpath);
    if (node == nullptr) {
        return nullptr;
    }
    
    if (node->flags.is_substituted != 0) {
        return FATINFO_T1;
    }

    FatInfo *info = new FatInfo();
    info->file = this->file;
    info->offset = node->offset;
    info->size = node->size;

    return info;
}


// 4ea2d2 (client: 4ef802)
int ResBase::GetSizedString(const char* fpath, const char* fname, const char* defval, char* out, uint32_t sz) {
    FatEntry *nodepath = this->Find(&this->root, fpath);
    FatEntry *node = this->Find(nodepath, fname);

    if (node == nullptr) {
        uint32_t copylen = strlen(defval);

        if (sz < copylen + 1) {
            copylen = sz;
        } else {
            copylen++;
        }

        strncpy(out, defval, copylen);
        return strlen(out);
    }

    char buf[32];

    int ntype = node->flags.value_type;
    switch (ntype) {
    case FatEntryFlags::TYPE_STRING:
        if (sz < node->size) {
            strncpy(out, (char *)this->pdata + node->offset, sz);
        } else {
            strncpy(out, (char *)this->pdata + node->offset, node->size);
        }
        break;

    case FatEntryFlags::TYPE_INT:
        _itoa(node->offset, buf, 10);
        strncpy(out, buf, sz);
        break;

    case FatEntryFlags::TYPE_DOUBLE:
        sprintf(buf, "%f", *(double *)&node->offset);
        // Vanilla overwrites buf with the decimal form of node->offset; the sprintf result is dead.
        _itoa(node->offset, buf, 10);
        strncpy(out, buf, sz);
        break;

    case FatEntryFlags::TYPE_INT_ARRAY: {
        char *pout = out;
        for (uint32_t i = 0; i < node->size / 4; i++) {
            _itoa(*((int *)(this->pdata + node->offset) + i), buf, 10);

            if (i < (node->size / 4) - 1) {
                strcat(buf, ",");
            }

            uint32_t ln = strlen(buf);

            if (sz < (pout - out) + ln + 2) {
                break;
            }

            strcat(pout, buf);

            pout += ln;
        }
    }
        break;

    case FatEntryFlags::TYPE_STRING_ARRAY:
        if (sz - 2 < node->size - 4) {
            memcpy(out, (uint8_t *)this->pdata + 4 + node->offset, sz - 2);
        } else {
            memcpy(out, (uint8_t*)this->pdata + 4 + node->offset, node->size - 4);
        }
        out[sz - 2] = 0;
        out[sz - 1] = 0;
        break;

    case FatEntryFlags::TYPE_DOUBLE_ARRAY: {
        char *pout = out;
        for (uint32_t i = 0; i < node->size / 8; i++) {
            sprintf(buf, "%f", *((double*)(this->pdata + node->offset) + i));

            if (i < (node->size / 8) - 1) {
                strcat(buf, ",");
            }

            int ln = strlen(buf);
            if (sz < (pout - out) + (ln + 2)) {
                break;
            }
            strcat(pout, buf);
            pout += ln;
        }
    }
        break;

    default:
        throw std::runtime_error("UNKNOWN TYPE. CANT CONVERT");
        break;
    }
    
    return strlen(out);
}


// 4e99c4 (client: 4eeef4)
void ResBase::RebuildFat() {
    this->modify_fat = nullptr;
    this->content_size = 0;

    this->RemoveEntry(&this->root);

    FatEntry** pnodes = new FatEntry*[this->fat_size]();

    if (pnodes == nullptr) {
        AfxThrowMemoryException();
        return;
    }

    FatEntry tmp;
    tmp.size = 0;
    tmp.offset = 0;

    pnodes[0] = &tmp;

    uint32_t idx = 0;
    uint32_t n = 1;
    for ( idx = 0; idx < this->fat_size; idx++) {
        if ((this->fat_data[idx].flags & FatEntryFlags::DIR_OR_DELETED_MASK) == 0) {
            int tp = this->fat_data[idx].flags.value_type;
            if (tp != FatEntryFlags::TYPE_INT && tp != FatEntryFlags::TYPE_DOUBLE) {
                pnodes[n] = this->fat_data + idx;
                n++;
            }
        }
    }

    if (n > 1) {
        qsort(pnodes, n, sizeof(FatEntry *), FatEntry::SortCompare);

        for (idx = 0; idx < n - 1; idx++) {
            uint32_t of = pnodes[idx + 1]->offset - (pnodes[idx]->offset + pnodes[idx]->size);
            if (of != 0) {
                FatEntry* nd = pnodes[idx + 1];
                memcpy(this->pdata + nd->offset, this->pdata + of + nd->offset, nd->size);
                nd->offset -= of;
            }
        }
    }

    this->memory_size = pnodes[idx]->offset + pnodes[idx]->size;
    this->content_size = 0;

    delete[] pnodes;
}

// 4e9bb6 (client: 4ef0e6)
uint8_t *ResBase::AllocData(uint32_t size) {

    if (this->fat_offset <= this->memory_size + size) {
        if (this->memory_size / 8 < this->content_size) {
            this->RebuildFat();

            if (this->fat_offset <= this->memory_size + size) {
                this->fat_offset += size + (this->fat_offset / 8);
                this->pdata = (uint8_t *)realloc(this->pdata, this->fat_offset);

                if (this->pdata == nullptr) {
                    AfxThrowMemoryException();
                    return nullptr;
                }
            }
        } else {
            this->fat_offset += size + (this->fat_offset / 8);

            this->pdata = (uint8_t*)realloc(this->pdata, this->fat_offset);

            if (this->pdata == nullptr) {
                AfxThrowMemoryException();
                return nullptr;
            }
        }
    }
    uint32_t off = this->memory_size;
    this->memory_size += size;
    return this->pdata + off;
}

// 4ea73d (client: 4efc6d)
void ResBase::SetString(const char* fpath, const char* fname, const char* str) {
    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        if (nodePath == nullptr) {
            nodePath = this->RecreateEntry(this->root.name, fpath);
        }

        node = this->RemoveEntry(nodePath);
        node = this->AppendEntry(&node, fname);
        node->flags = FatEntryFlags::MakeValue(FatEntryFlags::TYPE_STRING);
        node->size = 0;
        node->offset = this->pdata - this->AllocData(node->size); //??? BUG
    }

    if (node->flags.value_type != FatEntryFlags::TYPE_STRING) {
        throw std::runtime_error("not a string associated with specified key");
    }
    
    int ln = strlen(str) + 1;
    if (node->size < ln) {
        this->content_size += node->size;
        node->size = ln;
        node->offset = this->AllocData(ln) - this->pdata;
    } else {
        this->content_size += (node->size - ln);
    }

    strcpy((char *)this->pdata + node->offset, str);
}


// 4ea928 (client: 4efe58)
void ResBase::SetInt(const char* fpath, const char* fname, int32_t val) {
    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        if (nodePath == nullptr) {
            nodePath = this->RecreateEntry(this->root.name, fpath);
        }

        node = this->RemoveEntry(nodePath);
        node = this->AppendEntry(&node, fname);
        node->flags = FatEntryFlags::MakeValue(FatEntryFlags::TYPE_INT);
        node->size = 0;
        node->offset = val;
    } else {
        if (node->flags.value_type != FatEntryFlags::TYPE_INT) {
            throw std::runtime_error("not an int associated with specified key");
        }
        node->offset = val;
    }
}

// 4eaa8b (client: 4effbb)
void ResBase::SetDouble(const char* fpath, const char* fname, double val) {
    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        if (nodePath == nullptr) {
            nodePath = this->RecreateEntry(this->root.name, fpath);
        }

        node = this->RemoveEntry(nodePath);
        node = this->AppendEntry(&node, fname);
        node->flags = FatEntryFlags::MakeValue(FatEntryFlags::TYPE_DOUBLE);
        //node->size = 0;
        *(double *)&node->offset = val;
    } else {
        if (node->flags.value_type != FatEntryFlags::TYPE_DOUBLE) {
            throw std::runtime_error("not an double associated with specified key");

        //node->size = 0;
        }
        *(double*)&node->offset = val;
    }
}


// 4eb2a3 (client: 4f096a)
void ResBase::SetInt8Array(const char* fpath, const char* fname, CArray<uint8_t>& array) {
    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        if (nodePath == nullptr) {
            nodePath = this->RecreateEntry(this->root.name, fpath);
        }

        node = this->RemoveEntry(nodePath);
        node = this->AppendEntry(&node, fname);
        node->flags = FatEntryFlags::MakeValue(FatEntryFlags::TYPE_INT_ARRAY);
        node->size = 0;
        node->offset = this->AllocData(node->size) - this->pdata;
    }

    if (node->flags.value_type != FatEntryFlags::TYPE_INT_ARRAY) {
        throw std::runtime_error("not an int array associated with specified key");
    }

    uint32_t ln = array.GetSize() * 4;
    if (node->size < ln) {
        this->content_size += node->size;
        node->size = ln;
        node->offset = this->AllocData(node->size) - this->pdata;
    } else {
        this->content_size += node->size - ln;
    }

    for (uint32_t i = 0; i < array.GetSize(); i++) {
        *(uint32_t*)(this->pdata + node->offset + i * 4) = array[i];
    }
}


// 4eb43a (client: 4f07d3)
void ResBase::SetInt16Array(const char* fpath, const char* fname, CArray<uint16_t>& array) {
    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        if (nodePath == nullptr) {
            nodePath = this->RecreateEntry(this->root.name, fpath);
        }

        node = this->RemoveEntry(nodePath);
        node = this->AppendEntry(&node, fname);
        node->flags = FatEntryFlags::MakeValue(FatEntryFlags::TYPE_INT_ARRAY);
        node->size = 0;
        node->offset = this->AllocData(node->size) - this->pdata;
    }

    if (node->flags.value_type != FatEntryFlags::TYPE_INT_ARRAY) {
        throw std::runtime_error("not an int array associated with specified key");
    }

    uint32_t ln = array.GetSize() * 4;
    if (node->size < ln) {
        this->content_size += node->size;
        node->size = ln;
        node->offset = this->AllocData(node->size) - this->pdata;
    } else {
        this->content_size += node->size - ln;
    }

    for (uint32_t i = 0; i < array.GetSize(); i++) {
        *(uint32_t*)(this->pdata + node->offset + i * 4) = array[i];
    }
}


// 4eb5d2 (client: 4f0b02)
void ResBase::SetInt32Array(const char* fpath, const char* fname, CArray<uint32_t>& array) {
    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        if (nodePath == nullptr) {
            nodePath = this->RecreateEntry(this->root.name, fpath);
        }

        node = this->RemoveEntry(nodePath);
        node = this->AppendEntry(&node, fname);
        node->flags = FatEntryFlags::MakeValue(FatEntryFlags::TYPE_INT_ARRAY);
        node->size = 0;
        node->offset = this->AllocData(node->size) - this->pdata;
    }

    if (node->flags.value_type != FatEntryFlags::TYPE_INT_ARRAY) {
        throw std::runtime_error("not an int array associated with specified key");
    }

    uint32_t ln = array.GetSize() * 4;
    if (node->size < ln) {
        this->content_size += node->size;
        node->size = ln;
        node->offset = this->AllocData(node->size) - this->pdata;
    } else {
        this->content_size += node->size - ln;
    }

    for (uint32_t i = 0; i < array.GetSize(); i++) {
        *(uint32_t*)(this->pdata + node->offset + i * 4) = array[i];
    }
}


// 4ea69d (client: 4efbcd)
CString ResBase::GetValueString(const char* fpath, const char* fname, const char* defval) {

    char buf[0x800];
    this->GetSizedString(fpath, fname, defval, buf, 0x800);
    return buf;
}

// 4ea8af (client: 4efddf)
int32_t ResBase::GetInt(const char* fpath, const char* fname, int32_t defval) {

    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        return defval;
    }

    if (node->flags.value_type != FatEntryFlags::TYPE_INT) {
        throw std::runtime_error("not an int associated with specified key");
    }

    return node->offset;
}

// 4eaa06 (client: 4eff36)
double ResBase::GetDouble(const char* fpath, const char* fname, double defval) {

    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        return defval;
    }

    if (node->flags.value_type != FatEntryFlags::TYPE_DOUBLE) {
        throw std::runtime_error("not an double associated with specified key");
    }

    return *(double *)&node->offset;
}

// 4eab6b (client: 4f009b)
int ResBase::GetAllNodeNames(char* pbuf, int bufsz) {
    int out_len = 0;
    for (int i = 0; i < this->root.size; i++) {
        const char* name = this->fat_data[this->root.offset + i].name;
        int ln = strlen(name);
        if (ln + 1 < bufsz - out_len) {
            ln++;
        } else {
            ln = bufsz - out_len;
        }

        if (ln <= 1) {
            *pbuf = 0;
            return bufsz - 2;
        }

        *pbuf = 0;
        strncat(pbuf, name, ln);
        pbuf += ln;
        out_len += ln;
    }
    *pbuf = 0;
    return out_len;
}


// 4eaca7 (client: 4f01d7)
int ResBase::GetInt8Array(const char* fpath, const char* fname, CArray<uint8_t>* array) {
    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        return 0;
    }

    int tp = node->flags.value_type;
    if (tp == FatEntryFlags::TYPE_INT_ARRAY) {
        array->SetSize(node->size / 4);
        for (int i = 0; i < node->size / 4; i++) {
            array->SetAt(i, *(int32_t*)(this->pdata + node->offset + i * 4));
        }

        return 1;
    }

    if (tp == FatEntryFlags::TYPE_INT) {
        array->SetSize(1);
        array->SetAt(0, node->offset);
        return 1;
    }

    throw std::runtime_error("not an int array associated with specified key");
    return 0;
}

// 4ead9d (client: 4f02cd)
int ResBase::GetInt16Array(const char* fpath, const char* fname, CArray<uint16_t>* array) {
    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        return 0;
    }

    int tp = node->flags.value_type;
    if (tp == FatEntryFlags::TYPE_INT_ARRAY) {
        array->SetSize(node->size / 4);
        for (int i = 0; i < node->size / 4; i++) {
            array->SetAt(i, *(int32_t*)(this->pdata + node->offset + i * 4));
        }

        return 1;
    }

    if (tp == FatEntryFlags::TYPE_INT) {
        array->SetSize(1);
        array->SetAt(0, node->offset);
        return 1;
    }

    throw std::runtime_error("not an int array associated with specified key");
    return 0;
}

// 4eae99 (client: 4f03c9)
int ResBase::GetInt32Array(const char* fpath, const char* fname, CArray<uint32_t>* array) {
    //same as 4eafb6?
    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        return 0;
    }

    int tp = node->flags.value_type;
    if (tp == FatEntryFlags::TYPE_INT_ARRAY) {
        array->SetSize(node->size / 4);
        for (int i = 0; i < node->size / 4; i++) {
            array->SetAt(i, *(uint32_t*)(this->pdata + node->offset + i * 4));
        }

        return 1;
    }

    if (tp == FatEntryFlags::TYPE_INT) {
        array->SetSize(1);
        array->SetAt(0, node->offset);
        return 1;
    }

    if (tp == FatEntryFlags::TYPE_STRING && node->size <= 1) {
        array->RemoveAll();
        return 1;
    }

    throw std::runtime_error("not an int array associated with specified key");
    return 0;
}

// 4eafb6 (client: 4f04e6)
int ResBase::GetInt32Array(const char* fpath, const char* fname, CArray<int32_t>* array) {
    //same as 4eae99?
    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        return 0;
    }

    int tp = node->flags.value_type;
    if (tp == FatEntryFlags::TYPE_INT_ARRAY) {
        array->SetSize(node->size / 4);
        for (int i = 0; i < node->size / 4; i++) {
            array->SetAt(i, *(int32_t*)(this->pdata + node->offset + i * 4));
        }

        return 1;
    }

    if (tp == FatEntryFlags::TYPE_INT) {
        array->SetSize(1);
        array->SetAt(0, node->offset);
        return 1;
    }

    if (tp == FatEntryFlags::TYPE_STRING && node->size <= 1) {
        array->RemoveAll();
        return 1;
    }

    throw std::runtime_error("not an int array associated with specified key");
    return 0;
}


// 4eb0d3 (client: 4f0603)
int ResBase::GetStringArray(const char* fpath, const char* fname, CStringArray* array) {
    FatEntry* nodePath = this->Find(&this->root, fpath);
    FatEntry* node = this->Find(nodePath, fname);

    if (node == nullptr) {
        return 0;
    }

    int tp = node->flags.value_type;

    if (tp != FatEntryFlags::TYPE_STRING_ARRAY && tp != FatEntryFlags::TYPE_STRING) {
        throw std::runtime_error("not an string array associated with specified key");
        return 0;
    }

    int32_t count = 0;
    char* dat = (char *)this->pdata + node->offset;
    char* datstart = dat;
    if (tp == FatEntryFlags::TYPE_STRING) {
        array->SetSize(1);
        count = 1;
    } else {
        count = *(int32_t*)(dat);
        dat += 4;
        array->SetSize(count);
    }

    for (int i = 0; i < count; i++) {
        if ((dat - datstart) >= node->size) {
            break;
        }

        array->SetAt(i, dat);
        int ln = strlen(dat);
        dat += ln + 1;

    }

    return 1;
}





// 4e6f30 (client: 4ec460)
void FatEntry::SetName(const char* resname) {
    size_t sVar1;
    int local_8;

    if (resname != this->name) {
        size_t len = strlen(resname);
        if (len > 15) {
            this->flags.name_truncated = 1;
        }

        if (len + 1 > 15) {
            len = 15;
        }

        this->name[len] = 0;
        strncpy(this->name, resname, len);
    }
}

// 4ecd2a (client: 4f21da)
int FatEntry::NameCompare(const void* key, const void* datum) {
    return strcmp(((FatEntry*)key)->name, ((FatEntry*)datum)->name);
}

// 4eccd1 (client: 4f2181)
int FatEntry::SortCompare(const void* a, const void* b) {
    const FatEntry *_a = *(const FatEntry**)a;
    const FatEntry *_b = *(const FatEntry**)b;
    if (_b->offset < _a->offset) {
        return 1;
    } else if (_a->offset == _b->offset) {
        if (_b->size < _a->size) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return -1;
    }
}
#ifdef A2CLIENT_PATCH
// TestMap from a2mgr resource_control.cpp: paths inside the archive
// namespaces are loaded unchanged; everything else may be overridden on disk
static bool TangarTestMapPath(const char* file)
{
    std::string filename = file;
    for (char& ch : filename) {
        if (ch == 0x5C) { // backslash
            ch = '/';
        }
    }
    for (char& ch : filename) {
        ch = (char)tolower((unsigned char)ch);
    }
    static const char* prefixes[] = {
        "graphics/", "locale/", "main/", "movies/", "music/",
        "patch/", "scenario/", "sfx/", "speech/", "video/", "world/",
    };
    for (const char* prefix : prefixes) {
        if (filename.find(prefix) == 0) {
            return false;
        }
    }
    return true;
}
#endif
