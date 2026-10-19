#include "player.h"

#include <cstring>
#include "game_app.h"
#include "group.h"
#include "unit.h"
#include "unit_list.h"
#include "world.h"
#include "server.h"

// 5A5047
Settings::Settings() {
    memset(this, 0, sizeof(Settings));
    this->formation    = 2;
    this->autobuff_mask = 0xFF;
}

// 534118
Player::Player() {
    this->player_id = 0;
    this->hat_player_id = 0;
    this->flags = 0;
    this->name = "-unnamed-";
    this->main_unit = nullptr;
    this->money = 0;
    this->token_id = 1;
    this->color_slot = 0;
    this->monster_kills = 0;
    this->player_kills = 0;
    this->mission_result = 0;
    this->field_0xa98 = 0;
    this->field_0x20 = -10000;
    this->vision_sharing_id = 0;
    this->is_in_world = 0;
    this->is_logged_in = 0;
    this->is_active = 0;
    this->deaths = 0;
    this->frags = 0;
    this->drop_deadline_tick16 = 0;
    this->field_0xa5c = 0x5F; // '_'
    this->has_cd = 0;
    this->unit_list = new UnitList();
    this->group_list = new GroupList();
    memset(this->kill_stats, 0, sizeof(this->kill_stats));
    this->is_ai = 1;
    this->settings = new Settings();
    this->building_entered_from_yx = 0;
    this->login.Empty();
    this->field_0xa60 = 0;
    this->respawn_requested = 0;
    this->shout_cooldown = 0;
    this->field_0xa7c = 0;
    this->field_0xa84 = 0;
    this->field_0xa80 = 0;
    this->max_server_level = 0;
    this->min_server_level = 0;
    this->field_0xa6c = 1;
    this->field_0xa70 = 0;
    this->field_0xa90 = 0;
    this->field_0xa94 = 0;
}

// 5B051F
void Settings::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar.Write(this, sizeof(Settings));
    } else {
        ar.Read(this, sizeof(Settings));
    }
}

// 55B343
void Player::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar << this->name;
        ar << this->player_id;
        ar << this->token_id;

        ar.Write(&this->hat_player_id, 8);      // hat_player_id + flags (4+4 bytes)
        ar << this->color_slot;
        ar << this->is_ai;
        ar << this->vision_sharing_id;

        ar << ((uint32_t)this->money ^ 0x5C073F4D);
        ar << this->mission_result;
        ar << this->is_in_world;

        ar << ((uint32_t)this->monster_kills ^ 0x5C073F4D);
        ar << this->drop_deadline_tick16;

        ar << (this->deaths > 0x7FFF ? (uint16_t)0x7FFF : (uint16_t)this->deaths);
        ar << (this->player_kills > 0x7FFF ? (uint16_t)0x7FFF : (uint16_t)this->player_kills);
        ar << this->field_0xa5c;

        ar << (uint32_t)this->main_unit;
        ar << (uint32_t)this;

        ar.Write(this->kill_stats, 0xA00);
    } else {
        ar >> this->name;
        ar >> this->player_id;
        ar >> this->token_id;

        ar.Read(&this->hat_player_id, 8);
        ar >> this->color_slot;
        ar >> this->is_ai;
        ar >> this->vision_sharing_id;
        this->vision_sharing_mask = this->vision_sharing_id;

        ar >> this->money;
        this->money ^= 0x5C073F4D;
        ar >> this->mission_result;
        ar >> this->is_in_world;

        ar >> this->monster_kills;
        this->monster_kills ^= 0x5C073F4D;        
        ar >> this->drop_deadline_tick16;

        int16_t t16;
        ar >> t16;
        this->deaths = t16;
        ar >> t16;
        this->player_kills = t16;
        ar >> this->field_0xa5c;

        uint32_t raw;
        ar >> raw;
        this->main_unit = (Human*)(uintptr_t)raw;

        ar >> raw;
        g_Server->field23_0xdc[(void*)(uintptr_t)raw] = (void*)this;

        ar.Read(this->kill_stats, 0xA00);
    }

    this->group_list->Serialize(ar);
    this->settings->Serialize(ar);

    if (ar.IsLoading()) {
        // Resolve raw main_unit pointer via the server's pointer-remap table.
        void* remapped = nullptr;
        if (g_Server->field23_0xdc.Lookup((void*)this->main_unit, remapped)) {
            this->main_unit = (Human*)remapped;
        } else {
            this->main_unit = nullptr;
        }

        // Rebuild unit_list: collect all units from all groups and set pOwner.
        POSITION outer = this->group_list->groups.GetHeadPosition();
        while (outer != NULL) {
            Group* grp = this->group_list->groups.GetNext(outer);
            POSITION inner = grp->unit_list.GetHeadPosition();
            while (inner != NULL) {
                Unit* unit = grp->unit_list.GetNext(inner);
                this->unit_list->AddTail(unit);
                unit->pOwner = this;
            }
        }
    }
}

// 53442F
Player::~Player() {
    // Unregister from world: clear presence flag at diplomacy[0][player_id]
    if (g_World) {
        g_World->diplomacy.diplomacy[0][(int16_t)this->player_id] = 0;
    }

    if (this->unit_list) {
        delete this->unit_list;
        this->unit_list = nullptr;
    }

    if (this->group_list) {
        while (!this->group_list->groups.IsEmpty()) {
            Group* g = this->group_list->groups.RemoveHead();
            delete g;
        }
        delete this->group_list;
        this->group_list = nullptr;
    }

    if (this->settings) {
        delete this->settings;
        this->settings = nullptr;
    }
    // name, str, login: ~CString() called automatically
}

Player player_instantiation_check;

// 609088
IMPLEMENT_SERIAL(Player, CObject, 1);



// 534AC1
void Player::sub_534AC1(int32_t amount, uint32_t notify_flag) {
    this->money += amount;
    g_NetStru1_main.FUN_0051cefb(0x67, this->money, notify_flag, this);
}

// 534B17
void Player::sub_534B17() {
    if (this->field_0xa90 == 0) {
        this->field_0xa90 = GetTickCount() + 30500;
        this->field_0xa94 = this->field_0xa90 - 35500;
    }
}

// 534B59
void Player::sub_534B59() {
    if (this->field_0xa90 != 0) {
        this->field_0xa90 = 0;
        this->field_0xa94 = 0;
        g_NetStru1_main.FUN_0051ce86(0xf, 0, this);
    }
}

void Player::FUN_00534778() {
    //534778
    this->is_active = 0;
    this->is_logged_in = 0;
    this->drop_deadline_tick16 = g_Server->tick16;
}

// 5346AC
void Player::sub_5346AC() {
    POSITION it = this->unit_list->unit_list.GetHeadPosition();
    while (it != nullptr) {
        Unit* unit = this->unit_list->unit_list.GetNext(it);
        unit->hp = -50;
    }
}

// 5347A1
void Player::sub_5347A1(Unit* unit) {
    Group* group = unit->group;
    if (group == nullptr) {
        LogMessage("Warning! Player::RemoveUnit from NULL group!");
    } else {
        group->RemoveUnit(unit);
        if (group->unit_list.GetCount() == 0 && !group->owner->is_ai) {
            POSITION it = this->group_list->groups.Find(group);
            if (it != nullptr) {
                this->group_list->groups.RemoveAt(it);
            }
            delete group;
        }
    }

    POSITION it = this->unit_list->unit_list.Find(unit);
    if (it != nullptr) {
        this->unit_list->unit_list.RemoveAt(it);
    }
}

// 534A74
void Player::sub_534A74(Unit* unit) {
    this->sub_5347A1(unit);
    delete unit;
}




namespace ExperienceTable {
static int32_t g_ExperienceLevelTable[150];

void InitTable() { //53069e
    for (int i = 0; i < 150; i++) {
        g_ExperienceLevelTable[i] = (pow(1.1, i) - 1.0) * 1000.0;
    }
}

int32_t __cdecl GetLevel(uint32_t ex) { //5306ea
    for (int i = 0; i < 150; i++) {
        if (ex < g_ExperienceLevelTable[i + 1]) {
            return i;
        }
    }
    return g_ExperienceLevelTable[149];  // WAT !?   may be return index(149)?
}

int32_t __cdecl GetExp(uint32_t lvl) { //530726
    if (lvl >= 150) {
        lvl = 149;
    }
    return g_ExperienceLevelTable[lvl];
}
}
