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
    DECLARE_SERIAL(Building);

public: // VTable at 60f378
    virtual ~Building();
    virtual void VMethod1() override;
    virtual void Serialize(CArchive& ar) override;
    virtual int32_t IsOutpost();

public:
    Building(uint8_t type_id, TokenPos* pos, uint8_t width, uint8_t height); // sub_542599
    Building(); // sub_54242A
    Building(const CString& name); // sub_542534
    Building(TokenPos* pos); // sub_5424AC
    void sub_5427AC(uint8_t width, uint8_t height); // Initialize building from info table and place on map.
    void sub_542624(const CString& name); // Initialize building fields and look up type by name.

public:
    BuildingInfo* building_info;
    uint8_t object_info_id;
    uint8_t gap_0x41;
    int16_t hp;
    int16_t hp_max;
    uint16_t field_0x46;
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
    Pointer(uint8_t type_id, TokenPos* pos, uint8_t width, uint8_t height); // sub_5A3140

public:
    int32_t script_instance_id;
};
ASSERT_OFFSET(Pointer, object_info_id, 0x40);
ASSERT_OFFSET(Pointer, script_instance_id, 0x6c);
ASSERT_SIZE(Pointer, 0x70);


#endif
