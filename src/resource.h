#ifndef RESOURCE_H
#define RESOURCE_H

#include <cstdint>

#include "assert_offset.h"
#include <afxtempl.h>


class CFile;

struct FatInfo {
    CFile* file;
    uint32_t offset;
    uint32_t size;
};

#define FATINFO_T1   ((FatInfo*)1)

struct FatEntry {
    uint32_t magic;
    uint32_t offset;
    uint32_t size;
    uint32_t flags;
    char name[16];

    void SetName(const char* resname);  // client: 4ec460
    static int NameCompare(const void* key, const void* datum);  // client: 4f21da
    static int SortCompare(const void* a, const void* b);  // client: 4f2181
};

struct ResBase {
    FatEntry root;
    uint32_t fat_size;
    uint32_t size;
    FatEntry* fat_data;
    FatEntry* modify_fat;
    uint32_t content_size;
    union {
        CFile* file;
        uint8_t* pdata;
    };
    uint32_t fat_offset;
    uint32_t memory_size;

    FatEntry* Find(FatEntry* node, const char* fname);  // client: 4f1f4e
    FatEntry* FindNext(FatEntry* node, const char** fpath);  // client: 4f2028
    FatEntry* TracePath(const char* fpath);  // client: 4f1e06
    void GrowFat();  // client: 4f1da4
    void AccumulateSize(FatEntry* node);  // client: 4f20b5
    FatEntry* AppendEntry(FatEntry** pOldNode, const char* fname);  // client: 4f1c90

    void CopyFatExcluding(FatEntry* newFat, int* pCount, const FatEntry* srcFat, const FatEntry* srcRoot, FatEntry* node, FatEntry* dstRoot);  // client: 4f1a03
    FatEntry* RemoveEntry(FatEntry* node);  // client: 4f18e8
    void SortTree(FatEntry* root);  // client: 4f1be6
    void WriteToFile(CFile* cfile, int32_t offset = -1);  // client: 4ed8c6
    void ReadFromFile(CFile* cfile, int32_t offset = -1);  // client: 4ed6cc

    FatEntry* CreateEntry(const char* fpath, const char* fname);  // client: 4f1d76
    FatEntry* RecreateEntry(const char* fpath, const char* fname);  // client: 4ef6b3
    FatEntry* RequireFileEntry(const char* fpath);  // client: 4f1ec6
    FatEntry* FindEntry(const char* fpath);  // client: 4f1f13

    FatInfo* GetFatInfo(const char* fpath);  // client: 4eb285

    int GetSizedString(const char* fpath, const char* fname, const char* defval, char* out, uint32_t sz);  // client: 4ef802

    void SetString(const char* fpath, const char* fname, const char* str);  // client: 4efc6d
    void SetInt(const char* fpath, const char* fname, int32_t val);  // client: 4efe58
    void SetDouble(const char* fpath, const char* fname, double val);  // client: 4effbb
    void SetInt8Array(const char* fpath, const char* fname, CArray<uint8_t> &array);  // client: 4f096a
    void SetInt16Array(const char* fpath, const char* fname, CArray<uint16_t>& array);  // client: 4f07d3
    void SetInt32Array(const char* fpath, const char* fname, CArray<uint32_t>& array);  // client: 4f0b02

    CString GetValueString(const char* fpath, const char* fname, const char *defval);  // client: 4efbcd
    int32_t GetInt(const char* fpath, const char* fname, int32_t defval = 0);  // client: 4efddf
    double GetDouble(const char* fpath, const char* fname, double defval = 0.0);  // client: 4eff36

    int GetAllNodeNames(char* pbuf, int bufsz);  // client: 4f009b

    int GetInt8Array(const char* fpath, const char* fname, CArray<uint8_t>* array);  // client: 4f01d7
    int GetInt16Array(const char* fpath, const char* fname, CArray<uint16_t>* array);  // client: 4f02cd
    int GetInt32Array(const char* fpath, const char* fname, CArray<uint32_t>* array);  // client: 4f03c9
    int GetInt32Array(const char* fpath, const char* fname, CArray<int32_t>* array);  // client: 4f03c9

    int GetStringArray(const char* fpath, const char* fname, CStringArray* array);  // client: 4f0603

    void RebuildFat();  // client: 4eeef4
    uint8_t* AllocData(uint32_t size);  // client: 4ef0e6

    static FatEntry *g_lastDstRoot;
};

struct ResFile : ResBase {
    ResFile(const char* fpath);  // client: 4eae80
    ~ResFile();  // client: 4eb23b
};

struct RegFile : ResBase {
    RegFile();  // client: 4ed9f0
    RegFile(const char* fpath);  // client: 4edab5
    ~RegFile();  // client: 4eeeb9
};

ASSERT_SIZE(ResBase, 0x40);

struct Resources {
    CArray<ResBase*> res_files;
    CArray<CString> search_paths;
    CArray<CFile*> files;

    Resources();  // client: 4eb336
    ~Resources();  // client: 4eb3cf


    void AddPath(const char* path);  // client: 4eb7a3
    void AddResFile(const char* path);  // client: 4eb58a

    FatInfo* GetFatInfo(const char* fpath);  // client: 4eb99d
    void CloseFile(CFile* fil);  // client: 4ebc93
    void MarkFileSubstituted(const char* fpath);  // client: 4eb939

    static void MarkSubstitutedFiles(const char* fpath);  // client: 4eb85d
    static uint16_t GetPathType(const char* path);  // client: 4ec310
    static void StaticAddPath(const char* path);  // client: 4eb741
    static void StaticAddResFile(const char* path);  // client: 4eb772

    static Resources ResFiles;
};
ASSERT_SIZE(Resources, 0x3c);


#endif
