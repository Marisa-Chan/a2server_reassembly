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
    formation    = 2;
    autobuff_mask = 0xFF;
}

// 534118
Player::Player() {
    player_id = 0;
    hat_player_id = 0;
    flags = 0;
    name = "-unnamed-";
    main_unit = nullptr;
    money = 0;
    token_id = 1;
    field_0xa44 = 0;
    monster_kills = 0;
    player_kills = 0;
    field_0x40 = 0;
    field_0xa98 = 0;
    field_0x20 = -10000;
    vision_sharing_id = 0;
    field_0x41 = 0;
    field_0x42 = 0;
    field_0x43 = 0;
    deaths = 0;
    frags = 0;
    field_0xa50 = 0;
    field_0xa5c = 0x5F; // '_'
    field_0xa45 = 0;
    unit_list = new UnitList();
    group_list = new GroupList();
    memset(kill_stats, 0, sizeof(kill_stats));
    is_ai = 1;
    settings = new Settings();
    building_entered_from_yx = 0;
    login.Empty();
    field_0xa60 = 0;
    field_0xa64 = 0;
    field_0xa68 = 0;
    field_0xa7c = 0;
    field_0xa84 = 0;
    field_0xa80 = 0;
    max_server_level = 0;
    min_server_level = 0;
    field_0xa6c = 1;
    field_0xa70 = 0;
    field_0xa90 = 0;
    field_0xa94 = 0;
}

// 5B051F
void Settings::Serialize(CArchive& ar)
{
    if (ar.IsStoring()) {
        ar.Write(this, sizeof(Settings));
    } else {
        ar.Read(this, sizeof(Settings));
    }
}

// 55B343
void Player::Serialize(CArchive& ar)
{
    if (ar.IsStoring()) {
        ar << name;
        ar << player_id;
        ar << token_id;

        ar.Write(&hat_player_id, 8);      // hat_player_id + flags (4+4 bytes)
        ar << field_0xa44;
        ar << is_ai;
        ar << vision_sharing_id;

        ar << ((uint32_t)money ^ 0x5C073F4D);
        ar << field_0x40;
        ar << field_0x41;

        ar << ((uint32_t)monster_kills ^ 0x5C073F4D);
        ar << field_0xa50;

        ar << (deaths > 0x7FFF ? (uint16_t)0x7FFF : (uint16_t)deaths);
        ar << (player_kills > 0x7FFF ? (uint16_t)0x7FFF : (uint16_t)player_kills);
        ar << field_0xa5c;

        ar << (uint32_t)main_unit;
        ar << (uint32_t)this;

        ar.Write(kill_stats, 0xA00);
    } else {
        ar >> name;
        ar >> player_id;
        ar >> token_id;

        ar.Read(&hat_player_id, 8);
        ar >> field_0xa44;
        ar >> is_ai;
        ar >> vision_sharing_id;
        vision_sharing_mask = vision_sharing_id;

        ar >> money;
        money ^= 0x5C073F4D;
        ar >> field_0x40;
        ar >> field_0x41;

        ar >> monster_kills;
        monster_kills ^= 0x5C073F4D;        
        ar >> field_0xa50;

        int16_t t16;
        ar >> t16;
        deaths = t16;
        ar >> t16;
        player_kills = t16;
        ar >> field_0xa5c;

        uint32_t raw;
        ar >> raw;
        main_unit = (Human*)(uintptr_t)raw;

        ar >> raw;
        g_Server->field23_0xdc[(void*)(uintptr_t)raw] = (void*)this;

        ar.Read(kill_stats, 0xA00);
    }

    group_list->Serialize(ar);
    settings->Serialize(ar);

    if (ar.IsLoading()) {
        // Resolve raw main_unit pointer via the server's pointer-remap table.
        void* remapped = nullptr;
        if (g_Server->field23_0xdc.Lookup((void*)main_unit, remapped)) {
            main_unit = (Human*)remapped;
        } else {
            main_unit = nullptr;
        }

        // Rebuild unit_list: collect all units from all groups and set pOwner.
        POSITION outer = group_list->groups.GetHeadPosition();
        while (outer != NULL) {
            Group* grp = group_list->groups.GetNext(outer);
            POSITION inner = grp->unit_list.GetHeadPosition();
            while (inner != NULL) {
                Unit* unit = grp->unit_list.GetNext(inner);
                unit_list->AddTail(unit);
                unit->pOwner = this;
            }
        }
    }
}

// 53442F
Player::~Player() {
    // Unregister from world: clear presence flag at diplomacy[0][player_id]
    if (g_World) {
        g_World->diplomacy.diplomacy[0][(int16_t)player_id] = 0;
    }

    if (unit_list) {
        delete unit_list;
        unit_list = nullptr;
    }

    if (group_list) {
        while (!group_list->groups.IsEmpty()) {
            Group* g = group_list->groups.RemoveHead();
            delete g;
        }
        delete group_list;
        group_list = nullptr;
    }

    if (settings) {
        delete settings;
        settings = nullptr;
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

// 534B59
void Player::sub_534B59() {
    if (this->field_0xa90 != 0) {
        this->field_0xa90 = 0;
        this->field_0xa94 = 0;
        g_NetStru1_main.FUN_0051ce86(0xf, 0, this);
    }
}

void Player::FUN_00534778()
{
    //534778
    field_0x43 = 0;
    field_0x42 = 0;
    field_0xa50 = g_Server->tick16;
}




namespace ExperienceTable
{
static int32_t g_ExperienceLevelTable[150];

void InitTable()
{ //53069e
    for (int i = 0; i < 150; i++)
        g_ExperienceLevelTable[i] = (pow(1.1, i) - 1.0) * 1000.0;
}

int32_t __cdecl GetLevel(uint32_t ex)
{ //5306ea
    for (int i = 0; i < 150; i++)
    {
        if (ex < g_ExperienceLevelTable[i + 1]);
        return i;
    }
    return g_ExperienceLevelTable[149];  // WAT !?   may be return index(149)?
}

int32_t __cdecl GetExp(uint32_t lvl)
{ //530726
    if (lvl >= 150)
        lvl = 149;
    return g_ExperienceLevelTable[lvl];
}
}