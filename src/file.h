#ifndef FILE_H
#define FILE_H

#include <cstdint>

#include "assert_offset.h"
#include "mfc_templ.h"


class File2 : public CFile {
public:
    CFile* file;
    uint32_t field_0x14;
    uint32_t size;
    uint32_t position;
};
ASSERT_SIZE(File2, 0x20);


#endif
