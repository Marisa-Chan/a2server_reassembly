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

    enum TypeId : uint8_t {
        TYPE_MAGIC_WELL_HP  = 0x0F,
        TYPE_MAGIC_WELL_MP  = 0x10,
        TYPE_GATE_A         = 0x1C,
        TYPE_GATE_B         = 0x1D,
        TYPE_BRIDGE_VERT    = 0x21,
        TYPE_SHOP_PLAGAT_1  = 0x22,
        TYPE_SHOP_PLAGAT_2  = 0x23,
        TYPE_BRIDGE_HORIZ   = 0x25,
        TYPE_HAT_PORTAL     = 0x42,
        TYPE_INN_PLAGAT_1   = 0x43,
        TYPE_INN_PLAGAT_2   = 0x44,
        TYPE_INN_PLAGAT_3   = 0x45,
        TYPE_SIGNPOST_FIRST = 0x46,
        TYPE_SIGNPOST_LAST  = 0x4B,
        TYPE_GATE_C         = 0x4C,
        TYPE_GATE_D         = 0x4D,
        TYPE_SHOP_KAARG_1   = 0x5D,
        TYPE_SHOP_KAARG_2   = 0x5E,
        TYPE_SHOP_KAARG_3   = 0x5F,
        TYPE_INN_KAARG_1    = 0x63,
        TYPE_INN_KAARG_2    = 0x64,
        TYPE_INN_KAARG_3    = 0x65,
        TYPE_SHOP_DRUID_1   = 0x69,
        TYPE_SHOP_DRUID_2   = 0x6A,
        TYPE_SHOP_DRUID_3   = 0x6B,
        TYPE_INN_DRUID_1    = 0x6F,
        TYPE_INN_DRUID_2    = 0x70,
        TYPE_INN_DRUID_3    = 0x71,
    };

public: // VTable at 60f378
    virtual ~Building();  // client: 529d8e
    virtual void RegenTick() override;  // client: 529e08
    virtual void Serialize(CArchive& ar) override;  // client: 529f6d
    virtual int32_t IsBuilding() override;  // client: 54d380
    virtual int32_t IsOutpost();  // client: 54d390

public:
    Building(uint8_t type_id, TokenPos* pos, uint8_t width, uint8_t height); // 542599 (client: 5299f8)
    Building(); // 54242A (client: 5298ee)
    Building(const CString& name); // 542534 (client: 5299f8)
    Building(TokenPos* pos); // 5424AC (client: 529a5d)
    void InitFromTable(uint8_t width, uint8_t height); // Initialize building from info table and place on map. (client: 529c04)
    void InitByName(const CString& name); // Initialize building fields and look up type by name. (client: 529ae6)
    int32_t RollDamage(UnitToHit* to_hit, Unit* caster); // Building attack. (client: 529e83)

public:
    BuildingInfo* building_info;
    uint8_t object_info_id;
    uint8_t gap_0x41;
    int16_t hp;
    int16_t hp_max;
    uint16_t value1;
    uint8_t scan_range;
    uint8_t gap_0x49;
    Protections protections;
    uint8_t width;
    uint8_t height;
    uint16_t gap_0x62;
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
    Pointer(uint8_t type_id, TokenPos* pos, uint8_t width, uint8_t height); // 5A3140 (client: 5712b0)
    Pointer(); // default, used by CreateObject (client: 54d350)
    virtual ~Pointer() override;  // client: 54d3d0
    virtual void Serialize(CArchive& ar) override;  // client: 52a0ec

public:
    int32_t script_instance_id;
};
ASSERT_OFFSET(Pointer, script_instance_id, 0x6c);
ASSERT_SIZE(Pointer, 0x70);


#endif
