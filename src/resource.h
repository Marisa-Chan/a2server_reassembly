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

enum : uint32_t {
    RES_ARCHIVE_MAGIC = 0x31415926u,
};

struct FatEntryFlags {
    enum ValueType : uint32_t {
        TYPE_STRING       = 0,
        TYPE_INT          = 1,
        TYPE_DOUBLE       = 2,
        TYPE_INT_ARRAY    = 3,
        TYPE_STRING_ARRAY = 4,
        TYPE_DOUBLE_ARRAY = 5,
    };

    enum Mask : uint32_t {
        FLAG_DIRECTORY      = 0x00000001u,
        VALUE_TYPE_MASK     = 0x0000000Eu,
        FLAG_SORTED         = 0x00000010u,
        FLAG_NAME_TRUNCATED = 0x10000000u,
        FLAG_SUBSTITUTED    = 0x20000000u,
        FLAG_DELETED        = 0x40000000u,
        FLAG_FAT_INLINE     = 0x80000000u,
        DIR_OR_DELETED_MASK = FLAG_DIRECTORY | FLAG_DELETED, // 0x40000001u
    };

    union {
        uint32_t raw;
        struct {
            uint32_t is_directory   : 1;  // [0]     0x00000001
            uint32_t value_type     : 3;  // [3:1]   0x0000000E (ValueType in RegFile)
            uint32_t is_sorted      : 1;  // [4]     0x00000010 (children sorted for bsearch)
            uint32_t reserved       : 23; // [27:5]
            uint32_t name_truncated : 1;  // [28]    0x10000000 (name > 15 chars)
            uint32_t is_substituted : 1;  // [29]    0x20000000 (substituted by loose file)
            uint32_t is_deleted     : 1;  // [30]    0x40000000 (marked removed in RegFile)
            uint32_t fat_inline     : 1;  // [31]    0x80000000 (root: FAT immediately follows header)
        };
    };

    FatEntryFlags() = default;
    constexpr FatEntryFlags(uint32_t v) : raw(v) {}
    constexpr operator uint32_t() const { return this->raw; }
    FatEntryFlags& operator|=(uint32_t v) { this->raw |= v; return *this; }
    FatEntryFlags& operator&=(uint32_t v) { this->raw &= v; return *this; }

    static constexpr FatEntryFlags MakeValue(ValueType vt) {
        return FatEntryFlags(static_cast<uint32_t>(vt) << 1);
    }
};
ASSERT_SIZE(FatEntryFlags, 4);

struct FatEntry {
    uint32_t magic;
    uint32_t offset;
    uint32_t size;
    FatEntryFlags flags;
    char name[16];

    void SetName(const char* resname); // 4e6f30 (client: 4ec460)
    static int NameCompare(const void* key, const void* datum); // 4ecd2a (client: 4f21da)
    static int SortCompare(const void* a, const void* b); // 4eccd1 (client: 4f2181)
};
ASSERT_SIZE(FatEntry, 0x20);

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

    FatEntry* Find(FatEntry* node, const char* fname); // 4eca9e (client: 4f1f4e)
    FatEntry* FindNext(FatEntry* node, const char** fpath); // 4ecb78 (client: 4f2028)
    FatEntry* TracePath(const char* fpath); // 4ec956 (client: 4f1e06)
    void GrowFat(); // 4ec8f4 (client: 4f1da4)
    void AccumulateSize(FatEntry* node); // 4ecc05 (client: 4f20b5)
    FatEntry* AppendEntry(FatEntry** pOldNode, const char* fname); // 4ec7e0 (client: 4f1c90)

    void CopyFatExcluding(FatEntry* newFat, int* pCount, const FatEntry* srcFat, const FatEntry* srcRoot, FatEntry* node, FatEntry* dstRoot); // 4ec553 (client: 4f1a03)
    FatEntry* RemoveEntry(FatEntry* node); // 4ec438 (client: 4f18e8)
    void SortTree(FatEntry* root); // 4ec736 (client: 4f1be6)
    void WriteToFile(CFile* cfile, int32_t offset = -1); // 4e8396 (client: 4ed8c6)
    void ReadFromFile(CFile* cfile, int32_t offset = -1); // 4e819c (client: 4ed6cc)

    FatEntry* CreateEntry(const char* fpath, const char* fname); // 4ec8c6 (client: 4f1d76)
    FatEntry* RecreateEntry(const char* fpath, const char* fname); // 4ea183 (client: 4ef6b3)
    FatEntry* RequireFileEntry(const char* fpath); // 4eca16 (client: 4f1ec6)
    FatEntry* FindEntry(const char* fpath); // 4eca63 (client: 4f1f13)

    FatInfo* GetFatInfo(const char* fpath); // 4e5d55 (client: 4eb285)

    int GetSizedString(const char* fpath, const char* fname, const char* defval, char* out, uint32_t sz); // 4ea2d2 (client: 4ef802)

    void SetString(const char* fpath, const char* fname, const char* str); // 4ea73d (client: 4efc6d)
    void SetInt(const char* fpath, const char* fname, int32_t val); // 4ea928 (client: 4efe58)
    void SetDouble(const char* fpath, const char* fname, double val); // 4eaa8b (client: 4effbb)
    void SetInt8Array(const char* fpath, const char* fname, CArray<uint8_t> &array); // 4eb2a3 (client: 4f096a)
    void SetInt16Array(const char* fpath, const char* fname, CArray<uint16_t>& array); // 4eb43a (client: 4f07d3)
    void SetInt32Array(const char* fpath, const char* fname, CArray<uint32_t>& array); // 4eb5d2 (client: 4f0b02)

    CString GetValueString(const char* fpath, const char* fname, const char *defval); // 4ea69d (client: 4efbcd)
    int32_t GetInt(const char* fpath, const char* fname, int32_t defval = 0); // 4ea8af (client: 4efddf)
    double GetDouble(const char* fpath, const char* fname, double defval = 0.0); // 4eaa06 (client: 4eff36)

    int GetAllNodeNames(char* pbuf, int bufsz); // 4eab6b (client: 4f009b)

    int GetInt8Array(const char* fpath, const char* fname, CArray<uint8_t>* array); // 4eaca7 (client: 4f01d7)
    int GetInt16Array(const char* fpath, const char* fname, CArray<uint16_t>* array); // 4ead9d (client: 4f02cd)
    int GetInt32Array(const char* fpath, const char* fname, CArray<uint32_t>* array); // 4eae99 (client: 4f03c9)
    int GetInt32Array(const char* fpath, const char* fname, CArray<int32_t>* array); // 4eafb6 (client: 4f04e6)

    int GetStringArray(const char* fpath, const char* fname, CStringArray* array); // 4eb0d3 (client: 4f0603)

    void RebuildFat(); // 4e99c4 (client: 4eeef4)
    uint8_t* AllocData(uint32_t size); // 4e9bb6 (client: 4ef0e6)

    static FatEntry *g_lastDstRoot;
};

struct ResFile : ResBase {
    ResFile(const char* fpath); // 4e5950 (client: 4eae80)
    ~ResFile(); // 4e5d0b (client: 4eb23b)
};

struct RegFile : ResBase {
    RegFile(); // 4e84c0 (client: 4ed9f0)
    RegFile(const char* fpath); // 4e8585 (client: 4edab5)
    ~RegFile(); // 4e9989 (client: 4eeeb9)
};

ASSERT_SIZE(ResBase, 0x40);

struct Resources {
    CArray<ResBase*> res_files;
    CArray<CString> search_paths;
    CArray<CFile*> files;

    Resources(); // 4e5e06 (client: 4eb336)
    ~Resources(); // 4e5e9f (client: 4eb3cf)


    void AddPath(const char* path); // 4e6273 (client: 4eb7a3)
    void AddResFile(const char* path); // 4e605a (client: 4eb58a)

    FatInfo* GetFatInfo(const char* fpath); // 4e646d (client: 4eb99d)
    void CloseFile(CFile* fil); // 4e6763 (client: 4ebc93)
    void MarkFileSubstituted(const char* fpath); // 4e6409 (client: 4eb939)

    static void MarkSubstitutedFiles(const char* fpath); // 4e632d (client: 4eb85d)
    static uint16_t GetPathType(const char* path); // 4e6de0 (client: 4ec310)
    static void StaticAddPath(const char* path); // 4e6242 (client: 4eb772)
    static void StaticAddResFile(const char* path); // 4e6211 (client: 4eb741)

    static Resources ResFiles;
};
ASSERT_SIZE(Resources, 0x3c);


#endif
