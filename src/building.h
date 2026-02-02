#ifndef BUILDING_H
#define BUILDING_H

#include <cstdint>

#include "protections.h"
#include "token.h"
#include "unit_list.h"
#include "assert_offset.h"


class BuildingInfo;


class Building : public Token {
public:
    virtual int32_t IsOutpost();

public:
    BuildingInfo* building_info;
    uint8_t object_info_id;
    uint8_t gap_0x41;
    int16_t hp;
    int16_t hp_max;
    uint8_t gap_0x46[2];
    uint8_t scan_range;
    uint8_t gap_0x49;
    Protections protections;
    uint8_t width;
    uint8_t height;
    uint16_t field_0x62;
    uint32_t unpassable_mask;
    uint32_t tiles_mask;
};
ASSERT_OFFSET(Building, width, 0x60);
ASSERT_SIZE(Building, 0x6c);

class Pointer : public Building {
public:
    int32_t script_instance_id;
};
ASSERT_OFFSET(Pointer, script_instance_id, 0x6c);
ASSERT_SIZE(Pointer, 0x70);


#endif
