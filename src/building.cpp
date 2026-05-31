#include "building.h"
#include "game_app.h"
#include "map_stuff.h"
#include "net.h"
#include "unit_to_hit.h"
#include "virtual_caster.h"

//  542341   CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb)  
IMPLEMENT_SERIAL(Building, Token, 1);

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

// 57c3a0
int32_t Building::VMethod9() {
    return 1;
}

// 57c3b0
int32_t Building::IsOutpost() {
    return 0;
}

// 542A31
int32_t Building::sub_542A31(UnitToHit* to_hit, Unit* /*caster*/) {
    if (to_hit == nullptr || this->hp_max == 0) {
        return 0;
    }

    int32_t damage = 0;
    if (to_hit->some_damage2_spread != 0) {
        damage = to_hit->some_damage2_min + Random0N(to_hit->some_damage2_spread);
    }
    damage -= 5;
    if (damage <= 0) {
        return 0;
    }
    if (damage < this->hp) {
        return damage;
    }

    CList<VirtualCaster*>& vc_list = g_Server->srv_stru1->virtual_casters_list;
    POSITION pos = vc_list.GetHeadPosition();
    while (pos != nullptr) {
        POSITION cur = pos;
        VirtualCaster* vc = vc_list.GetNext(pos);
        if (this->position->IsSameYX(vc->position)) {
            vc_list.RemoveAt(cur);
            delete vc;
        }
    }

    return damage;
}

IMPLEMENT_SERIAL(Pointer, Building, 1);

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
