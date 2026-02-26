#include "player.h"

#include <cstring>
#include "group.h"
#include "unit_list.h"
#include "world.h"

extern "C" World*   dword_6A8B8C;   // TODO: g_World

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
    group_list = new CList<Group*>();
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

// 53442F
Player::~Player() {
    // Unregister from world: clear presence flag at diplomacy[0][player_id]
    if (dword_6A8B8C) {
        dword_6A8B8C->diplomacy[0][(int16_t)player_id] = 0;
    }

    if (unit_list) {
        delete unit_list;
        unit_list = nullptr;
    }

    if (group_list) {
        while (!group_list->IsEmpty()) {
            Group* g = group_list->RemoveHead();
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
AFX_DATADEF CRuntimeClass Player::classPlayer =
{ "Player", sizeof(Player), 1, (CObject* (__stdcall *)())(0x0053405e), &CObject::classCObject, NULL };
