#ifndef RESOURCE_H
#define RESOURCE_H

#include <cstdint>

#include "assert_offset.h"
#include "mfc_templ.h"


class CFile;


struct ResFile {
    uint32_t magic; // 0x31415926 :D
    uint32_t root_offset;
    uint32_t root_size;
    uint32_t res_flags;
    char name[16];
    uint32_t fat_size;
    uint32_t size;
    uint32_t fat_data;
    uint32_t field_0x2c;
    uint32_t field_0x30;
    CFile* file;
    uint32_t fat_offset;
    uint16_t field_0x3c;
    uint8_t gap_0x3e[2];
};
ASSERT_SIZE(ResFile, 0x40);

struct Resources {
    CArray<ResFile*> res_files;
    CArray<void*> field_0x14;
    CArray<CFile*> files;
};
ASSERT_SIZE(Resources, 0x3c);


#endif
