#include "building.h"
#include "game_app.h"
#include "map_stuff.h"
#include "net.h"

IMPLEMENT_DYNCREATE(Building, Token);

// 542599
Building::Building(uint8_t type_id, TokenPos* pos, uint8_t width, uint8_t height)
    : Token(pos)
{
    this->itemDataID = 0;
    this->object_info_id = type_id;
    this->building_info = nullptr;
    this->sub_5427AC(width, height);
}

// 5424AC
Building::Building(TokenPos* pos)
    : Token(pos)
{
    this->sub_542624("");
}

// 542534
Building::Building(const CString& name) {
    this->sub_542624(name);
}

// 54242A
Building::Building() {
    this->sub_542624("");
}

// 54293A
Building::~Building() {
    if (this->building_id != 0) {
        BldIdSet_Unset((uint16_t)this->building_id);
    }
    if (MapStuff_Instance != nullptr) {
        MapStuff_Instance->sub_5945EF(this);
    }
}

// 542CE7
void Building::Serialize(CArchive& ar) {
    Token::Serialize(ar);
    this->protections.Serialize(ar);
    if (ar.IsStoring()) {
        ar << this->object_info_id;
        ar << this->hp;
        ar << this->hp_max;
        ar << this->field_0x46;
        ar << this->scan_range;
        ar << this->width;
        ar << this->height;
        ar << this->unpassable_mask;
        ar << this->tiles_mask;
    } else {
        ar >> this->object_info_id;
        ar >> this->hp;
        ar >> this->hp_max;
        ar >> this->field_0x46;
        ar >> this->scan_range;
        ar >> this->width;
        ar >> this->height;
        ar >> this->unpassable_mask;
        ar >> this->tiles_mask;
        if (this->object_info_id == 0) {
            this->building_info = nullptr;
        } else {
            this->building_info = &g_GameDataRes.buildings[this->object_info_id];
        }
    }
}

// Replenishes magic wells every 60 ticks.
// 5429b6
void Building::VMethod1() {
    if (g_Server->tick % 60 != 0) {
        return;
    }
    // 15 is "Magic Well 2" (restores health), 16 is "Magic Well 3" (restores mana).
    if (this->object_info_id != 15 && this->object_info_id != 16) {
        return;
    }
    if (this->hp >= this->hp_max || this->hp < 0) {
        return;
    }
    this->hp++;
    g_NetStru1_main.sub_51AC77(this, nullptr, 0);
}

int32_t Building::IsOutpost() {
    return 0;
}

IMPLEMENT_DYNCREATE(Pointer, Building);

// 5A3140
Pointer::Pointer(uint8_t type_id, TokenPos* pos, uint8_t width, uint8_t height)
    : Building(type_id, pos, width, height)
{
    this->script_instance_id = -1;
}

// 54235D
Pointer::Pointer()
    : Building()
{}

// 57C3F0
Pointer::~Pointer() {}

// 5430FB
void Pointer::Serialize(CArchive& ar) {
    Building::Serialize(ar);
    if (ar.IsStoring()) {
        ar << this->script_instance_id;
    } else {
        ar >> this->script_instance_id;
    }
}
