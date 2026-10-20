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

    void SetName(const char* resname);
    static int NameCompare(const void* key, const void* datum);
    static int SortCompare(const void* a, const void* b);
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

    FatEntry* Find(FatEntry* node, const char* fname);
    FatEntry* FindNext(FatEntry* node, const char** fpath);
    FatEntry* TracePath(const char* fpath);
    void GrowFat();
    void AccumulateSize(FatEntry* node);
    FatEntry* AppendEntry(FatEntry** pOldNode, const char* fname);

    void CopyFatExcluding(FatEntry* newFat, int* pCount, const FatEntry* srcFat, const FatEntry* srcRoot, FatEntry* node, FatEntry* dstRoot);
    FatEntry* RemoveEntry(FatEntry* node);
    void SortTree(FatEntry* root);
    void WriteToFile(CFile* cfile, int32_t offset = -1);
    void ReadFromFile(CFile* cfile, int32_t offset = -1);

    FatEntry* CreateEntry(const char* fpath, const char* fname);
    FatEntry* RecreateEntry(const char* fpath, const char* fname);
    FatEntry* RequireFileEntry(const char* fpath);
    FatEntry* FindEntry(const char* fpath);

    FatInfo* GetFatInfo(const char* fpath);

    int GetSizedString(const char* fpath, const char* fname, const char* defval, char* out, uint32_t sz);

    void SetString(const char* fpath, const char* fname, const char* str);
    void SetInt(const char* fpath, const char* fname, int32_t val);
    void SetDouble(const char* fpath, const char* fname, double val);
    void SetInt8Array(const char* fpath, const char* fname, CArray<uint8_t> &array);
    void SetInt16Array(const char* fpath, const char* fname, CArray<uint16_t>& array);
    void SetInt32Array(const char* fpath, const char* fname, CArray<uint32_t>& array);

    CString GetValueString(const char* fpath, const char* fname, const char *defval);
    int32_t GetInt(const char* fpath, const char* fname, int32_t defval = 0);
    double GetDouble(const char* fpath, const char* fname, double defval = 0.0);

    int GetAllNodeNames(char* pbuf, int bufsz);

    int GetInt8Array(const char* fpath, const char* fname, CArray<uint8_t>* array);
    int GetInt16Array(const char* fpath, const char* fname, CArray<uint16_t>* array);
    int GetInt32Array(const char* fpath, const char* fname, CArray<uint32_t>* array);
    int GetInt32Array(const char* fpath, const char* fname, CArray<int32_t>* array);

    int GetStringArray(const char* fpath, const char* fname, CStringArray* array);

    void RebuildFat();
    uint8_t* AllocData(uint32_t size);

    static FatEntry *g_lastDstRoot;
};

struct ResFile : ResBase {
    ResFile(const char* fpath);
    ~ResFile();
};

struct RegFile : ResBase {
    RegFile();
    RegFile(const char* fpath);
    ~RegFile();
};

ASSERT_SIZE(ResBase, 0x40);

struct Resources {
    CArray<ResBase*> res_files;
    CArray<CString> search_paths;
    CArray<CFile*> files;

    Resources();
    ~Resources();


    void AddPath(const char* path);
    void AddResFile(const char* path);

    FatInfo* GetFatInfo(const char* fpath);
    void CloseFile(CFile* fil);
    void FUN_004e6409(const char* fpath);

    static void FUN_004e632d(const char* fpath);
    static uint16_t GetPathType(const char* path);
    static void StaticAddPath(const char* path);
    static void StaticAddResFile(const char* path);

    static Resources ResFiles;
};
ASSERT_SIZE(Resources, 0x3c);


#endif
