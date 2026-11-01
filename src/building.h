#ifndef BUILDING_H
#define BUILDING_H

#include <cstdint>

#include "protections.h"
#include "token.h"
#include "unit_list.h"
#include "assert_offset.h"


class BuildingInfo;
class Unit;
struct UnitToHit;


class Building : public Token {
public:
    DECLARE_SERIAL(Building);

public: // VTable at 60f378
    virtual ~Building();  // client: 529d8e
    virtual void RegenTick() override;  // client: 529e08
    virtual void Serialize(CArchive& ar) override;  // client: 529f6d
    virtual int32_t IsBuilding() override;  // client: 54d380
    virtual int32_t IsOutpost();  // client: 54d390

public:
    Building(uint8_t type_id, TokenPos* pos, uint8_t width, uint8_t height); // sub_542599 (client: 5299f8)
    Building(); // sub_54242A (client: 5298ee)
    Building(const CString& name); // sub_542534 (client: 5299f8)
    Building(TokenPos* pos); // sub_5424AC (client: 529a5d)
    void InitFromTable(uint8_t width, uint8_t height); // Initialize building from info table and place on map. (client: 529c04)
    void InitByName(const CString& name); // Initialize building fields and look up type by name. (client: 529ae6)
    int32_t RollDamage(UnitToHit* to_hit, Unit* caster); // Building attack. (client: 529e83)

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
ASSERT_OFFSET(Building, object_info_id, 0x40);
ASSERT_OFFSET(Building, width, 0x60);
ASSERT_SIZE(Building, 0x6c);

class Pointer : public Building {
public:
    DECLARE_SERIAL(Pointer);

public:
    Pointer(uint8_t type_id, TokenPos* pos, uint8_t width, uint8_t height); // sub_5A3140 (client: 5712b0)
    Pointer(); // default, used by CreateObject (client: 54d350)
    virtual ~Pointer() override;  // client: 54d3d0
    virtual void Serialize(CArchive& ar) override;  // client: 52a0ec

public:
    int32_t script_instance_id;
};
ASSERT_OFFSET(Pointer, script_instance_id, 0x6c);
ASSERT_SIZE(Pointer, 0x70);


#endif
