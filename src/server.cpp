#include "server.h"

#include <cstring>
#include <cstdlib>  // atoi
#include <cmath>    // sqrt
#include <set>

#include "alm.h"
#include "buildings_list.h"
#include "building.h"
#include "constants.h"
#include "eye.h"
#include "game_app.h"
#include "world.h"
#include "player.h"
#include "unit.h"
#include "unit_list.h"
#include "packet.h"
#include "players_list.h"
#include "net.h"
#include "map_stuff.h"
#include "outpost.h"
#include "spell_effect.h"
#include "effect.h"
#include "quest_glue.h"
#include "quest_map.h"
#include "group.h"
#include "sack.h"
#include "item.h"
#include "inn.h"
#include "shop.h"
#include "shelf.h"
#include "inventory.h"
#include "spell.h"
#include "virtual_caster.h"
#include "player_file.h"
#include "file.h"

#include "patch/allowed_picture.h"
#include "patch/flags.h"

// ---- Global variables used by JoinPlayer ----
UnitList* dword_6CDB3C;  // pending-unit list
Server* g_Server; // 642c2c
int32_t g_IsServer; // 665d00
int32_t g_ShutdownIn; // 63623c

UnitList* dword_6B37C4; // 6b37c4

PasswordManager PasswordManager::manager; //6a8b98

void PasswordManager::OpenFile(const char* name) { //4f6bf9
    this->fname = name;
    CFileFind find;
    if (find.FindFile(name)) {
        find.FindNextFile();
        find.GetLastWriteTime(this->date);
        this->Load();
    } else {
        this->fname.Empty();
    }
}

void PasswordManager::Load() { //4f6c92
    CStdioFile file;
    if (!file.Open(this->fname, CFile::modeRead)) {
        return;
    }
    CString login;
    CString password;
    CString skip;
    this->content.SetSize(0, -1);
    while (file.GetPosition() < file.GetLength()) {
        file.ReadString(login);
        file.ReadString(password);
        file.ReadString(skip);
        file.ReadString(skip);
        char buf[1024];
        strcpy(buf, login);
        buf[login.GetLength()] = '\x01';
        strcpy(buf + login.GetLength() + 1, password);
        this->content.SetAtGrow(this->content.GetSize(), buf);
    }
    file.Close();
}


// ---- Helpers used by FUN_00500907 ----
// 5008CA
// Stat point cost: (int)(pow(1.15, level - 1) * 0.349 + 0.5)
int32_t StatLevelPoints(int32_t level) {
    return (int32_t)(pow(1.15, (double)(level - 1)) * 0.349 + 0.5);
}
uint32_t BldIdSet_AllocBit(); // Allocate a token/building ID bit
void BldIdSet_Clear();         // Clear allocated token/building ID bits

// 5049D1
// Strip an optional leading integer count from *str (modifies str in-place),
// returns the parsed count (or 1 if none present).
int32_t ParseCountPrefix(CString* str) {
    int32_t count = 1;
    int32_t pos = str->Find(' ');
    if (pos != 0) {
        count = atoi(str->Left(pos));
        if (count <= 0) {
            count = 1;
        } else {
            *str = str->Mid(pos + 1);
        }
    }
    return count;
}

// ---- Helpers used by LoadGame ----
// 43A820
// itoa -> CString
CString* IntToCString(CString* out, uint32_t value) {
    char buf[21];
    _itoa(value, buf, 10);
    *out = buf;
    return out;
}

int dword_6CDB38; // File checksum global

// ---- Helpers used by arena Server methods ----
// 4FA768
// Send game-end broadcast to all clients
static void SendFragLimit(Player* player) {
    g_NetStru1_main.SendInfoPacket(0x5C, g_ServerConfig.frag_limit, 0, player);
}

uint16_t Server::somewords[32][32];

Srv1::~Srv1() = default; // 59b6f0

SrvStru1 SrvStru1::Instance; //6cf390

// 4f2c26
Server::~Server() {
    g_NetStru1_main.SendBarePacket(0xaf, nullptr);
    g_NetStru1_main.SendPacket_64(this->tick | 1, 0);
    g_NetStru1_main.SendAllData();

    if (this->field15_0x88) {
        ResumeThread(this->field15_0x88);
    }

    while (WaitForSingleObject(this->field15_0x88, 50) == WAIT_TIMEOUT)
    {}

    if (g_World) {
        delete g_World;
        g_World = nullptr;
    }

    if (MapStuff_Instance) {
        delete MapStuff_Instance;
        MapStuff_Instance = nullptr;
    }

    this->srv_stru1->Deinit();

    if (g_PlayersList) {
        delete g_PlayersList; // destructor will delete objects
        g_PlayersList = nullptr;
    }

    if (dword_6CDB3C) {
        delete dword_6CDB3C; // WAT? do not delete objects??
        dword_6CDB3C = nullptr;
    }

    if (dword_6B37C4) {
        delete dword_6B37C4; // WAT? do not delete objects??
        dword_6B37C4 = nullptr;
    }

    for (int i = 1; i < this->spells.size(); i++) {
        if (this->spells[i]) {
            delete this->spells[i];
            this->spells[i] = nullptr;
        }
    }

    if (this->script_settings) {
        delete this->script_settings;
        this->script_settings = nullptr;
    }

    LogMessage("Server closed\n");
}

// 4F465E
void Server::AnnounceHatGameStart() {
    if (g_CLlDriver.provider != 4) {
        return;
    }
    if (g_HatLLDriver.IsListen()) {
        NetStru1::HatConnector.SendHatLobbyInfo(-1, this->current_map_title, this->MapLevel, g_ServerConfig.gameType, 0);
    }
    if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
        g_NetStru1_main.SendTeamState(0);
    }
}

// 4F1E2A
void Server::ProcessPlayers() {
    this->AnnounceHatGameStart();
    this->field16_0x8c = 1;

    // First pass: handle each human player's main unit (shop/inn departure, decay respawn).
    {
        POSITION player_it = g_PlayersList->list.GetHeadPosition();
        while (player_it != nullptr) {
            Player* player = g_PlayersList->list.GetNext(player_it);
            if (!player->is_ai && player->main_unit != nullptr) {
                Shop* shop = this->FindShopAt(player->main_unit->position);
                if (shop != nullptr && shop->shop_template != nullptr) {
                    if (shop->shop_template->FindInstanceFor(player->main_unit) != nullptr) {
                        shop->shop_template->CloseInstance(player->main_unit);
                        if (this->field4_0x74 != 0) {
                            player->main_unit->ReturnToMap();
                        }
                    }
                }

                Inn* inn = this->FindInnAt(player->main_unit->position);
                if (inn != nullptr) {
                    inn->RemoveUnit(player->main_unit, -1);
                }

                if (player->main_unit->decay != 0 && g_Server->field4_0x74 != 0) {
                    player->respawn_requested = 0;
                    Human* main_unit = player->main_unit;
                    if (main_unit->inventory == nullptr) {
                        main_unit->inventory = new Inventory();
                    }
                    CList<Unit*>& unit_list = this->srv_stru1->units_list->unit_list;
                    POSITION unit_it = unit_list.Find(main_unit);
                    if (unit_it != nullptr) {
                        unit_list.RemoveAt(unit_it);
                    }
                    main_unit->sent_to_mask = 0;
                    this->CreatePlayerCharacter(player, 0, 0, 0, 0, 0, 0);
                    this->EnsurePlayerGroup(player);
                    player->mission_result = 0;

                    Player* self = g_PlayersList->FindByName("Self");
                    if (self != nullptr && (g_ServerConfig.gameType == 0 || g_ServerConfig.gameType == 3)) {
                        for (int32_t i = 2; i < 16; i++) {
                            g_World->diplomacy.diplomacy[i][player->player_id] = g_World->diplomacy.diplomacy[i][self->player_id];
                            g_World->diplomacy.diplomacy[player->player_id][i] = g_World->diplomacy.diplomacy[self->player_id][i];
                        }
                        g_World->diplomacy.diplomacy[self->player_id][player->player_id] = 0x12;
                        g_World->diplomacy.diplomacy[player->player_id][self->player_id] = 0x12;
                        g_NetStru1_main.SendVisibilityState(player);
                    }
                }
            }
        }
    }

    this->srv_stru1->FreeLists();

    if (g_World != nullptr) {
        delete g_World;
        g_World = nullptr;
    }

    if (MapStuff_Instance != nullptr) {
        delete MapStuff_Instance;
        MapStuff_Instance = nullptr;
    }

    dword_6CDB3C->unit_list.RemoveAll();
    dword_6B37C4->unit_list.RemoveAll();

    // Second pass: strip all effects from every human player's units.
    {
        POSITION player_it = g_PlayersList->list.GetHeadPosition();
        while (player_it != nullptr) {
            Player* player = g_PlayersList->list.GetNext(player_it);
            if (!player->is_ai) {
                CList<Unit*>& unit_list = player->unit_list->unit_list;
                POSITION unit_it = unit_list.GetHeadPosition();
                while (unit_it != nullptr) {
                    Unit* unit = unit_list.GetNext(unit_it);
                    POSITION effect_it = unit->_effects.GetHeadPosition();
                    while (effect_it != nullptr) {
                        auto current = effect_it;
                        Effect* effect = unit->_effects.GetNext(effect_it);

                        effect->spell_value = 1;
                        effect->caster = nullptr;
                        effect->TickOnUnit(unit);
                        unit->_effects.RemoveAt(current);
                        delete effect;
                    }
                }
            }
        }
    }

    // Third pass: reset surviving players' units, drop AI / "Self" players.
    {
        POSITION player_it = g_PlayersList->list.GetHeadPosition();
        while (player_it != nullptr) {
            auto current_it = player_it;
            Player* player = g_PlayersList->list.GetNext(player_it);

            if (!player->is_ai && player->name != "Self") {
                player->is_in_world = 0;
                CList<Unit*>& unit_list = player->unit_list->unit_list;
                POSITION unit_it = unit_list.GetHeadPosition();
                while (unit_it != nullptr) {
                    Unit* unit = unit_list.GetNext(unit_it);
                    if (unit->typeId < 0x21 || unit->typeId > 0x3f) {
                        player->RemoveUnit(unit);
                    } else {
                        unit->hp = unit->hp_max;
                        unit->mp = unit->mp_max;
                        unit->decay = 0;
                        unit->ResetActionState();
                        unit->cast_target = nullptr;
                        unit->some_spell = nullptr;
                        unit->spell = nullptr;
                        unit->some_item = nullptr;
                        unit->last_hit_by = nullptr;
                    }
                }
            } else {
                g_PlayersList->list.RemoveAt(current_it);
                delete player;
            }
        }
    }

    if (this->field4_0x74 == 0) {
        g_PlayersList->next_player_id = 2;
    }

    this->field19_0x98.field2_0x20 = 0;
    this->field19_0x98.field4_0x28.SetSize(0, -1);
    this->field21_0xd4 = 0;
    this->field18_0x94 = 0;
    this->tick16 = 0;
    this->tick = 0;

    g_NetStru1_main.SendBarePacket(0xb8, nullptr);
    g_NetStru1_main.SendPacket_64(this->tick | 1, 0);
    this->ScanSavedCharacters();
}

// 4F9E55
void Server::ScanSavedCharacters() {
    if (g_ServerConfig.gameType != 0) {
        return;
    }

    CFileFind finder;
    CStringArray files;

    CString pattern = g_ServerConfig.chr_base + "*.sck";
    BOOL found = finder.FindFile(pattern);

    while (found) {
        found = finder.FindNextFile();
        if (!finder.IsDirectory()) {
            files.SetAtGrow(files.GetSize(), finder.GetFilePath());
        }
    }

    for (int i = 0; i < files.GetSize(); i++) {
        try {
            CFile::Remove(files[i]);
        } catch (CFileException* e) {
            e->Delete();
        }
    }

    try {
        CFile::Remove(g_ServerConfig.chr_base + "info.map");
    } catch (CFileException* e) {
        e->Delete();
    }
}


// 59B7EA
int Srv1::LoadMap(CString map_name) {
    if (this->field2_0x20 != 0) {
        map_name = "Scenario\\" + map_name;
    }

    MapAlm* alm = new MapAlm(map_name);

    if (alm->map_heights == nullptr) {
        CString error;
        switch (alm->error_loading) {
        case 1:
            error = "File not found";
            break;
        case 2:
            error = "Not a map file";
            break;
        case 3:
            error = "Wrong block number";
            break;
        case 4:
            error = "Map version too new (update loader!)";
            break;
        case 5:
            error = "Tiles block not found";
            break;
        case 6:
            error = "Altitudes block not found";
            break;
        default:
            error = "Unknown error";
            break;
        }
        LogMessage("Map error :" + error + ", in " + map_name);

        if (alm != nullptr) {
            delete alm;
        }

        return 1;
    }

    g_Server->field4_0x74 = (alm->recommended_player_count > 1) ? 1 : 0;
    g_Server->field27_0x178 = (g_Server->field4_0x74 == 0) ? 1 : 0;
    g_Server->current_map_title = alm->map_name;
    g_Server->MapLevel = alm->map_level;

    this->CreatePlayers(alm);

    MapStuff* map_stuff = new MapStuff(alm, dword_6CDB3C);
    MapStuff_Instance = map_stuff;

    World* world = new World(MapStuff_Instance, g_PlayersList);
    g_World = world;

    this->CreateBuildings(alm);

    POSITION pos = g_PlayersList->list.GetHeadPosition();
    while (pos) {
        Player* player = g_PlayersList->list.GetNext(pos);

        if (alm->map_players.GetSize() > player->player_id - 1) {
            for (int32_t i = 0; i < 16; i++) {
                MapPlayerData* mpd = alm->map_players[player->player_id - 1];
                g_World->diplomacy.diplomacy[player->player_id][i + 1] = mpd->diplomacy[i];
            }
        }

        g_World->diplomacy.diplomacy[player->player_id][player->player_id] = 0x12;
    }

    this->CreateUnits(alm);

    Player* self = g_PlayersList->FindByName("Self");
    if (self != nullptr && g_Server->field4_0x74 != 0) {
        POSITION pos2 = g_PlayersList->list.GetHeadPosition();
        while (pos2) {
            Player* player2 = g_PlayersList->list.GetNext(pos2);

            if ((g_World->diplomacy.diplomacy[player2->player_id][self->player_id] & 7) == 0 && player2->is_ai) {
                if (player2->unit_list != nullptr) {
                    POSITION upos = player2->unit_list->unit_list.GetHeadPosition();
                    while (upos) {
                        Unit* unit = player2->unit_list->unit_list.GetNext(upos);
                        unit->summoned = 1;
                    }
                }
            }
        }
    }

    this->LoadTriggers(alm, 1);
    this->LoadSacks(alm);

    if (g_Server->field4_0x74 != 0) {
        this->SpawnAiGroupOutposts(alm);
    }

    if (alm != nullptr) {
        delete alm;
    }

    g_Server->field38_0x1a4 = (g_Server->field4_0x74 == 0) ? 1 : 0;

    return 0;
}

// 59C56E
void Srv1::CreateBuildings(MapAlm* alm) {
    static std::set<int16_t> shop_type_ids = {0x22, 0x23, 0x5D, 0x5E, 0x5F, 0x69, 0x6A, 0x6B};
    static std::set<int16_t> inn_type_ids = {0x43, 0x44, 0x45, 0x63, 0x64, 0x65, 0x6F, 0x70, 0x71};

    if (MapStuff_Instance == nullptr) {
        return;
    }

    g_QuestMap.DeleteGlues();

    for (int32_t i = 0; i < alm->map_buildings.GetSize(); i++) {
        MapBuildingData* bd = alm->map_buildings[i];
        if (bd == nullptr) {
            continue;
        }

        Building* building = nullptr;
        int16_t type_id = bd->type_id;

        if (shop_type_ids.count(type_id) > 0) {
            TokenPos pos(bd->x, bd->y, MapStuff_Instance);
            Shop* shop = new Shop(bd->type_id, &pos);
            building = shop;

            for (int32_t j = 0; j < alm->shops.GetSize(); j++) {
                MapShopData* sd = alm->shops[j];
                if (sd->shop_id != bd->building_id) {
                    continue;
                }
                for (int32_t k = 0; k < 4; k++) {
                    shop->gen_params[k].min_cost = sd->min_price[k];
                    shop->gen_params[k].max_cost = sd->max_price[k];
                    shop->gen_params[k].max_same_count = sd->max_same_count[k];
                    shop->gen_params[k].max_count = sd->max_count[k];
                    shop->gen_params[k].flags = sd->flags[k];
                }
                break;
            }
        } else if (inn_type_ids.count(type_id) > 0) {
            TokenPos pos(bd->x, bd->y, MapStuff_Instance);
            Inn* inn = new Inn(bd->type_id, &pos);
            building = inn;

            for (int32_t j = 0; j < alm->taverns.GetSize(); j++) {
                MapInnData* td = alm->taverns[j];
                if (td->inn_id != bd->building_id) {
                    continue;
                }
                for (int32_t k = 0; k < 4; k++) {
                    inn->delivery_item_id = (td->flags & 2) ? td->delivery_item_id : 0;
                    inn->has_kill_all_humans = (td->flags & 0x10) != 0;
                    inn->has_kill_all_monsters = (td->flags & 0x20) != 0;
                    inn->has_kill_all_undead = (td->flags & 0x40) != 0;
                    inn->has_raise_dead = (td->flags & 4) != 0;
                }
            }
        } else if (0x46 <= type_id && type_id <= 0x4B) {
            // Pointer
            TokenPos pos(bd->x, bd->y, MapStuff_Instance);
            Pointer* pointer = new Pointer(bd->type_id, &pos, bd->bridge_width, bd->bridge_height);
            building = pointer;

            for (int32_t j = 0; j < alm->pointers.GetSize(); j++) {
                MapPointerData* pd = alm->pointers[j];
                if (pd->building_id != bd->building_id) {
                    continue;
                }
                if (pd->flags != 0) {
                    POSITION pos2 = alm->logic_instances.GetHeadPosition();
                    int32_t script_count = 0;
                    for (int32_t k = 0; k < pd->instance_id; k++) {
                        MapLogicData* logic_data = alm->logic_instances.GetNext(pos2);
                        if (logic_data->type_id < 0x10002) {
                            script_count++;
                        }
                    }
                    pointer->script_instance_id = script_count;
                }
            }
        } else {
            // Default Building
            TokenPos pos(bd->x, bd->y, MapStuff_Instance);
            building = new Building(bd->type_id, &pos, bd->bridge_width, bd->bridge_height);
            if (bd->health == 0) {
                building->hp = bd->health;
            }
        }

        // Common: set owner
        building->pOwner = g_PlayersList->FindByTokenId(bd->player_id);
        if (building->pOwner == nullptr) {
            LogMessage("Warning - Building without owner have been loaded");
        }

        // Set building ID and add to list
        building->TokenID = bd->building_id;
        this->srv_stru->building_list->AddBuilding(building);

        // For Inns, create QuestInnGlue
        if (building->IsKindOf(RUNTIME_CLASS(Inn))) {
            QuestInnGlue* glue = new QuestInnGlue(building->building_id, building->position, g_World, 4);
            g_QuestMap.AddInnGlue(glue);
        }

        // Delete building data and null the array entry
        delete bd;
        alm->map_buildings[i] = nullptr;
    }
}

// 59FC97
void Srv1::SpawnRandomSacks(int count) {
    uint8_t x_range = (uint8_t)(MapStuff_Instance->map_width - 20);
    uint8_t y_range = (uint8_t)(MapStuff_Instance->map_height - 20);
    if (count == 0) {
        count = (std::max)(10, MapStuff_Instance->map_width / 4);
    }
    for (int i = 0; i < count; i++) {
        Inventory* inventory = new Inventory();
        for (int n = Random0N(3); n > 0; n--) {
            Item* item = RollRandomItem(10, 500);
            inventory->PutItemIntoBagAtDefault(item);
        }
        uint8_t x = (uint8_t)(Random0N(x_range) + 10);
        uint8_t y = (uint8_t)(Random0N(y_range) + 10);
        int gold = 0;
        if (inventory->items.GetSize() == 0) {
            gold = Random0N(400) + 100;
        }
        TokenPos pos(x, y, MapStuff_Instance);
        g_Server->srv_stru1->sack_list->DumpInventoryAt(&pos, inventory, gold, 0);
    }
}

// 59BEDB
void Srv1::SpawnGroupOutpost(Group* group, MapAlm* alm) {
    if (group->unit_list.GetCount() == 0) {
        return;
    }

    int32_t min_x = 1000;
    int32_t max_x = 0;
    int32_t min_y = 1000;
    int32_t max_y = 0;
    int32_t sum_x = 0;
    int32_t sum_y = 0;
    int32_t count = 0;

    POSITION pos = group->unit_list.GetHeadPosition();
    while (pos != nullptr) {
        Unit* unit = group->unit_list.GetNext(pos);
        uint8_t x = unit->position->GetX();
        uint8_t y = unit->position->GetY();
        min_x = (std::min)(min_x, (int32_t)x);
        max_x = (std::max)(max_x, (int32_t)x);
        min_y = (std::min)(min_y, (int32_t)y);
        max_y = (std::max)(max_y, (int32_t)y);
        sum_x += x;
        sum_y += y;
        count++;
    }

    uint8_t avg_x = (uint8_t)(sum_x / count);
    uint8_t avg_y = (uint8_t)(sum_y / count);
    int32_t spread = (std::max)(max_x - min_x, max_y - min_y);

    TokenPos token_pos(avg_x, avg_y, MapStuff_Instance);
    token_pos.ClampToMap(avg_x, avg_y);

    Outpost* outpost = new Outpost(&token_pos);
    outpost->pOwner = group->owner;
    outpost->group_id = group->group_id;
    outpost->repop_delay = (120 * g_ServerConfig.repop_delay) / 100;
    outpost->repop_countdown = outpost->repop_delay;
    outpost->spread = spread;
    outpost->FillUnits(group);

    for (int32_t i = 0; i < alm->groups.GetSize(); i++) {
        MapGroupData* group_data = alm->groups[i];
        if (group_data->group_id != group->group_id) {
            continue;
        }

        outpost->repop_delay = (group_data->repop_time * g_ServerConfig.repop_delay) / 100;
        outpost->repop_countdown = outpost->repop_delay;

        if ((group_data->flags & MapGroupData::HAS_SCRIPT_INSTANCE) != 0) {
            int32_t script_count = 0;
            POSITION logic_pos = alm->logic_instances.GetHeadPosition();
            for (uint32_t j = 0; j < group_data->instance_id; j++) {
                MapLogicData* logic_data = alm->logic_instances.GetNext(logic_pos);
                if (logic_data->type_id < 0x10002) {
                    script_count++;
                }
            }
            outpost->script_id = script_count;
        } else {
            outpost->script_id = -1;
        }

        outpost->has_quest_kill = (group_data->flags & MapGroupData::HAS_GROUP_KILL) != 0;
        group->has_quest_kill = outpost->has_quest_kill;
        outpost->has_quest_intercept = (group_data->flags & MapGroupData::HAS_GROUP_INTERCEPT) != 0;
        group->has_quest_intercept = outpost->has_quest_intercept;

        if ((group_data->flags & MapGroupData::RANDOM_POSITIONS) == 0) {
            outpost->spread = 0;
        }
    }

    this->srv_stru->building_list->AddBuilding(outpost);
    g_World->PlaceRespawnedGroup(group, 0);

    if (outpost->script_id >= 0) {
        g_World->DoScriptInstID(outpost->script_id);
    }
}

// 59C37A
void Srv1::SpawnAiGroupOutposts(MapAlm* alm) {
    POSITION player_iter = g_PlayersList->list.GetHeadPosition();
    while (player_iter) {
        Player* player = g_PlayersList->list.GetNext(player_iter);
        if (player->is_ai != 0) {
            POSITION group_iter = player->group_list->groups.GetHeadPosition();
            while (group_iter) {
                Group* group = player->group_list->groups.GetNext(group_iter);
                this->SpawnGroupOutpost(group, alm);
            }
        }
    }
}

// 59C3FB
void Srv1::CreatePlayers(MapAlm* alm) {
    for (int32_t i = 1; i <= alm->map_players.GetSize(); i++) {
        MapPlayerData* player_data = alm->map_players[i - 1];
        if (player_data == nullptr) {
            continue;
        }

        Player* player;
        if (g_Server->field4_0x74 == 0 && i == 1 && g_PlayersList->list.GetCount() == 1) {
            player = g_PlayersList->list.GetHead();
        } else {
            player = new Player();
            player->name = player_data->name;
            player->is_ai = 1;
            player->token_id = player_data->index;
            g_PlayersList->AddPlayer(player);
        }

        player->color_slot = player_data->color + 1;
        player->is_ai = 1;
        if (g_Server->field4_0x74 == 0 && i == 1) {
            player->is_ai = 0;
        }
        player->field_0xa60 = (player_data->flags & 2) ? 1 : 0;
    }
}

// 59CD45
void Srv1::CreateUnits(MapAlm* alm) {
    if (MapStuff_Instance == nullptr) {
        return;
    }

    for (int32_t i = 1; i <= alm->units_datas.GetSize(); i++) {
        MapUnitData* unit_data = alm->units_datas[i - 1];
        if (unit_data == nullptr) {
            continue;
        }

        Player* player = g_PlayersList->FindByTokenId(unit_data->player_id);
        if (player == nullptr) {
            CString msg;
            msg.Format("Can't resolve player %d for unit %d.", (int)unit_data->player_id, (int)unit_data->unit_id);
            LogMessage(msg);
            continue;
        }

        if (g_Server->field26_0x174 != 0 && player->is_ai != 0) {
            continue;
        }

        Unit* unit;
        if ((unit_data->flags1 & 0x10) == 0) {
            int32_t idx = g_GameDataRes.FindMonsterByServerId(unit_data->server_id);
            unit = new Unit(g_GameDataRes.monsters[idx].name);

            if (g_Server->field22_0xd8 != 2 && g_Server->field4_0x74 == 0) {
                if (g_Server->field22_0xd8 == 1) { // Easy
                    unit->hp_max *= 0.5;
                    unit->hp = unit->hp_max;
                    unit->eye2->wimpy >>= 1;
                } else { // Hard
                    unit->hit_values.attack += 50;
                    unit->protections.defense += 50;
                    unit->hp_max *= 1.5;
                    unit->hp = unit->hp_max;
                }
            }
        } else {
            int32_t idx = g_GameDataRes.FindHumanByServerId(unit_data->server_id);
            int32_t is_hero = g_GameDataRes.humans[idx].name.Find("_Hero") > 0 ? 1 : 0;
            unit = new Human(g_GameDataRes.humans[idx].name, is_hero, nullptr);
        }

        unit->quest_kill_mark = (unit_data->flags2 & 1) ? 1 : 0;
        unit->quest_intercept_mark = (unit_data->flags2 & 2) ? 1 : 0;
        unit->quest_escort_mark = (unit_data->flags2 & 4) ? 1 : 0;
        unit->summoned   = (unit_data->flags2 & 8) ? 1 : 0;

        if (unit->typeId == 0) {
            CString msg;
            msg.Format("Invalid unit %d during loading.", unit_data->unit_id);
            LogMessage(msg);
            delete unit;
            continue;
        }

        unit->RecalcStats();

        if ((unit_data->flags1 & 0x10) != 0 && (int16_t)unit_data->max_hp != -1 && unit_data->max_hp == unit_data->hp) {
            unit_data->hp = -1;
            unit_data->max_hp = -1;
        }
        if (unit_data->max_hp != -1) {
            unit->hp_max = unit_data->max_hp;
        }
        if (unit_data->hp == -1) {
            unit->hp = unit->hp_max;
        } else {
            unit->hp = unit_data->hp;
        }

        int32_t placed = unit->PlaceOnMap(unit_data->x >> 8, unit_data->y >> 8, 0);
        if (!placed) {
            CString msg;
            msg.Format("Can't place unit %d during loading on the tile(%d, %d).", unit_data->unit_id, unit_data->x >> 8, unit_data->y >> 8);
            LogMessage(msg);
            delete unit;
            continue;
        }

        unit->TokenID = unit_data->unit_id;
        dword_6CDB3C->AddTailId6xxx(unit);
        unit->pOwner = player;
        player->unit_list->AddTail(unit);

        bool found_group = false;
        POSITION group_iter = player->group_list->groups.GetHeadPosition();
        while (group_iter) {
            Group* grp = player->group_list->groups.GetNext(group_iter);
            if (grp->group_id == unit_data->group_id) {
                grp->AddUnit(unit);
                found_group = true;
                break;
            }
        }
        if (!found_group) {
            Group* new_grp = new Group();
            player->group_list->groups.AddTail(new_grp);
            new_grp->AddUnit(unit);
            new_grp->group_id = unit_data->group_id;
        }
    }
}

Unit* Srv1::GetScenarioScriptPCUnit(const char* name) { //59d4b8
    if (g_Server->field4_0x74 == 0) {
        CString pcname("PC_");
        pcname += name;

        Player* first_player = g_PlayersList->GetHead();
        for (POSITION pos = first_player->unit_list->unit_list.GetHeadPosition(); pos != nullptr;) {
            Unit* unit = first_player->unit_list->unit_list.GetNext(pos);
            if (unit->monster_info->name.Find(pcname) == 0) {
                return unit;
            }
        }

        for (POSITION pos = dword_6CDB3C->unit_list.GetHeadPosition(); pos != nullptr;) {
            Unit* unit = dword_6CDB3C->unit_list.GetNext(pos);
            if (unit->monster_info->name.Find(pcname) == 0) {
                return unit;
            }
        }
    }
    return nullptr;
}

Unit* Srv1::GetScenarioScriptHero(uint32_t idx) { //59d664
    if (g_Server->field4_0x74 == 0) {
        Player* first_player = g_PlayersList->GetHead();
        for (POSITION pos = first_player->unit_list->unit_list.GetHeadPosition(); pos != nullptr;) {
            Unit* unit = first_player->unit_list->unit_list.GetNext(pos);
            if (unit->server_id - 21 == idx) {
                return unit;
            }
        }

        for (POSITION pos = dword_6CDB3C->unit_list.GetHeadPosition(); pos != nullptr;) {
            Unit* unit = dword_6CDB3C->unit_list.GetNext(pos);
            if (unit->server_id - 21 == idx) {
                return unit;
            }
        }
    }
    return nullptr;
}

Unit* Srv1::GetScenarioScriptUnit(CMap<int32_t, int32_t, Unit*, Unit*>& umap, uint32_t idx) { //59d3c0
    Unit* result = nullptr;
    if (idx < 10001) {
        result = umap[idx];
    } else if (idx < 11001) {
        result = this->GetScenarioScriptHero(idx - 10001);
    } else {
        static const char* PCUnitNames[] = { "Danath", "Reniesta", "Fergard", "Naira", "Treyrak", "Brian", "Glaen", "Woman" }; //63d5d8
        result = this->GetScenarioScriptPCUnit(PCUnitNames[idx - 11001]);
    }
    if (!result) {
        CString msg;
        if (idx < 10001) {
            msg.Format("Can\'t resolve unit %d.", idx);
        } else {
            msg.Format("Can\'t resolve hero %d.", idx);
        }

        LogWarning(msg);
    }
    return result;
}

Group* Srv1::GetScenarioScriptGroup(CMap<int32_t, int32_t, Group*, Group*>& gmap, uint32_t idx) { //59d808
    Group* g = gmap[idx];
    if (!g) {
        CString msg;
        msg.Format("Can\'t resolve group %d.", idx);
        LogWarning(msg);
    }
    return g;
}

Player* Srv1::GetScenarioScriptPlayer(CMap<int32_t, int32_t, Player*, Player*>& pmap, uint32_t idx) { //59d77f
    Player* p = pmap[idx];
    if (!p) {
        CString msg;
        msg.Format("Can\'t resolve player %d.", idx);
        LogWarning(msg);
    }
    return p;
}

// 59D891
void Srv1::LoadTriggers(MapAlm* alm, int flag) {
    CMap<int32_t, int32_t, Unit*, Unit*> units_map;
    CMap<int32_t, int32_t, Group*, Group*> groups_map;
    CMap<int32_t, int32_t, Player*, Player*> players_map;
    CMap<int32_t, int32_t, Building*, Building*> buildings_map;

    // Build buildings map
    POSITION building_iter = this->srv_stru->building_list->GetHeadPosition();
    while (building_iter != nullptr) {
        Building* building = this->srv_stru->building_list->GetNext(building_iter);
        buildings_map[building->TokenID] = building;
    }
    
    // Build units maps from two lists
    if (dword_6CDB3C) {
        const auto& unit_list = dword_6CDB3C->unit_list;
        POSITION unit_iter = unit_list.GetHeadPosition();
        while (unit_iter != nullptr) {
            Unit* unit = unit_list.GetNext(unit_iter);
            units_map[unit->TokenID] = unit;
        }
    }
    if (this->srv_stru->units_list) {
        const auto& unit_list = this->srv_stru->units_list->unit_list;
        POSITION unit_iter = unit_list.GetHeadPosition();
        while (unit_iter != nullptr) {
            Unit* unit = unit_list.GetNext(unit_iter);
            units_map[unit->TokenID] = unit;
        }
    }

    // Build players and groups maps
    POSITION player_iter = g_PlayersList->list.GetHeadPosition();
    while (player_iter != nullptr) {
        Player* player = g_PlayersList->list.GetNext(player_iter);
        players_map[player->token_id] = player;
        
        // Iterate player's groups
        if (player->group_list != nullptr) {
            const auto& group_list = player->group_list->groups;
            POSITION group_iter = group_list.GetHeadPosition();
            while (group_iter != nullptr) {
                Group* group = group_list.GetNext(group_iter);
                groups_map[group->group_id] = group;
                
                if (flag != 0 && g_Server->field4_0x74 == 0) {
                    if (player->player_id == 1) {
                        g_World->ProcessAnimateDeadGroup(group);
                    } else {
                        g_World->PlaceRespawnedGroup(group, 0);
                    }
                }
            }
        }
        
        // Add player's units to units_map
        if (player->unit_list != nullptr) {
            const auto& unit_list = player->unit_list->unit_list;
            POSITION unit_iter = unit_list.GetHeadPosition();
            while (unit_iter != nullptr) {
                Unit* unit = unit_list.GetNext(unit_iter);
                units_map[unit->TokenID] = unit;
            }
        }
    }

    // Count actions with TypeID < 0x10002
    int32_t instance_count = 0;
    POSITION instance_iter = alm->logic_instances.GetHeadPosition();
    while (instance_iter != nullptr) {
        MapLogicData* inst = alm->logic_instances.GetNext(instance_iter);
        if (inst->type_id < 0x10002) {
            instance_count++;
        }
    }

    CMap<int32_t, int32_t, int32_t, int32_t> instance_idx_map;
    g_World->trigger_actions->SetSize(instance_count, -1);

    instance_iter = alm->logic_instances.GetHeadPosition();
    int action_idx = 0;
    while (instance_iter != nullptr) {
        MapLogicData* action_data = alm->logic_instances.GetNext(instance_iter);
        TriggerAction& action = g_World->trigger_actions->ElementAt(action_idx);

        if (action_data->type_id < 0x10002) {
            int has_unit = 0;
            int has_group = 0;
            int has_player = 0;
            int valid = 1;
            int data_idx = 0;

            for (int arg_idx = 0; arg_idx < 10; arg_idx = arg_idx + 1) {
                const uint32_t arg_type = action_data->argument_types[arg_idx];
                uint32_t& arg_value = action_data->argument_values[arg_idx];

                action.ref_kind[arg_idx] = (byte)arg_type;
                if (
                    (arg_value < 8000 || 9999 < arg_value) ||
                    (arg_type != 2 && arg_type != 3 && arg_type != 4)
                ) {
                    switch(arg_type) {
                    case 2: // Group
                        if (!has_group) {
                            action.group = this->GetScenarioScriptGroup(groups_map, arg_value);
                            if (action.group == nullptr) {
                                valid = 0;
                            }
                        } else {
                            action.multi = this->GetScenarioScriptGroup(groups_map, arg_value);
                            if (action.multi == nullptr) {
                                valid = 0;
                            }
                        }

                        if (action.group != nullptr) {
                            action.group->group_sub->active = 1;
                        } else if (action.multi != nullptr) {
                            static_cast<Group*>(action.multi)->group_sub->active = 1;
                        }
                        has_group = 1;
                        break;

                    case 3: // Player
                        if (!has_player) {
                            action.player = this->GetScenarioScriptPlayer(players_map, arg_value);
                            if (action.player == nullptr) {
                                valid = 0;
                            }
                        } else {
                            action.multi = this->GetScenarioScriptPlayer(players_map, arg_value);
                            if (action.multi == nullptr) {
                                valid = 0;
                            }
                        }
                        has_player = 1;
                        break;
                        
                    case 4: // Unit
                        if (!has_unit) {
                            action.unit = this->GetScenarioScriptUnit(units_map, arg_value);
                            if (action.unit == nullptr) {
                                valid = 0;
                            }
                        } else {
                            action.multi = this->GetScenarioScriptUnit(units_map, arg_value);
                            if (action.multi == nullptr) {
                                valid = 0;
                            }
                        }

                        if (action.multi != nullptr && static_cast<Unit*>(action.multi)->group != nullptr) {
                            static_cast<Unit*>(action.multi)->group->group_sub->active = 1;
                        } else if (action.unit != nullptr && action.unit->group != nullptr) {
                            action.unit->group->group_sub->active = 1;
                        }

                        has_unit = 1;
                        break;
                        
                    case 8: // Item
                        action.item_id = static_cast<uint16_t>(arg_value + 3608);
                        break;
                        
                    case 9: // Building
                        action.building = buildings_map[arg_value];
                        if (action.building == nullptr) {
                            CString msg;
                            msg.Format("Can't resolve building %d.", arg_value);
                            LogWarning(msg);
                            valid = 0;
                        }
                        break;
                        
                    default:
                        action.data[data_idx++] = arg_value;
                        break;
                    }
                } else {
                    action.data[arg_idx] = arg_value;
                    if (arg_value < 9000) {
                        action.ref_scope[arg_idx] = 1;
                        arg_value = arg_value - 8000;
                    } else {
                        action.ref_scope[arg_idx] = 2;
                        arg_value = arg_value - 9000;
                    }
                    action.ref_page[arg_idx] = (arg_value / 100);
                    action.ref_index[arg_idx] = (arg_value % 100);
                    action.ref_count++;
                }
            }
            if (valid == 1) {
                instance_idx_map[action_data->index] = action_idx;
                action.type = action_data->type_id;
                action.id = action_idx;
                action_idx++;
            } else {
                CString message;
                message.Format("Failed to create instant %d %s",action_data->index, action_data->name);
                LogMessage(message);
            }
        } else if (action_data->type_id == 0x10002) {
            const uint16_t val = (action_data->argument_values[0] & 0xFF) | ((action_data->argument_values[1] & 0xFF) << 8);
            this->field4_0x28.Add(val);
        }
    }

    CMap<int32_t, int32_t, int32_t, int32_t> check_idx_map;
    POSITION check_iter = alm->logic_checks.GetHeadPosition();
    int32_t check_idx = 0;

    while (check_iter != nullptr) {
        auto* check_data = alm->logic_checks.GetNext(check_iter);

        check_idx_map[check_data->index] = check_idx;

        TriggerCheck check = TriggerCheck();
        check.type = check_data->type_id;
        check.id = check_idx;
        check.exec_once = check_data->exec_once_flag;
        check.counter = 0;

        if (check.type < 0x10002) {
            int has_unit = 0;
            int has_group = 0;
            int has_player = 0;
            bool valid = true;
            int data_idx = 0;

            for (int arg_idx = 0; arg_idx < 10; arg_idx = arg_idx + 1) {
                const uint32_t arg_type = check_data->argument_types[arg_idx];
                uint32_t& arg_value = check_data->argument_values[arg_idx];

                check.ref_kind[arg_idx] = static_cast<uint8_t>(arg_type);

                if (
                    arg_value < 8000 || 9999 < arg_value ||
                    (arg_type != 2 && arg_type != 3 && arg_type != 4)
                ) {
                    switch (arg_type) {
                    case 2: // Group
                        if (!has_group) {
                            check.group = this->GetScenarioScriptGroup(groups_map, arg_value);
                            if (check.group == nullptr) {
                                valid = false;
                            }
                        } else {
                            check.multi = this->GetScenarioScriptGroup(groups_map, arg_value);
                            if (check.multi == nullptr) {
                                valid = false;
                            }
                        }

                        if (check.group != nullptr && check.type != 1) {
                            check.group->group_sub->active = 1;
                        } else if (check.multi != nullptr && check.type != 1) {
                            static_cast<Group*>(check.multi)->group_sub->active = 1;
                        }

                        has_group = 1;
                        break;
                        
                    case 3: // Player
                        if (!has_player) {
                            check.player = this->GetScenarioScriptPlayer(players_map, arg_value);
                            if (check.player == nullptr) {
                                valid = false;
                            }
                        } else {
                            check.multi = this->GetScenarioScriptPlayer(players_map, arg_value);
                            if (check.multi == nullptr) {
                                valid = false;
                            }
                        }
                        has_player = 1;
                        break;
                        
                    case 4: // Unit
                        if (!has_unit) {
                            check.unit = this->GetScenarioScriptUnit(units_map, arg_value);
                            if (check.unit == nullptr) {
                                valid = false;
                            }
                        } else {
                            check.multi = this->GetScenarioScriptUnit(units_map, arg_value);
                            if (check.multi == nullptr) {
                                valid = false;
                            }
                        }
                        
                        if (check.unit != nullptr && check.unit->group != nullptr && check.type != 1) {
                            check.unit->group->group_sub->active = 1;
                        } else if (check.multi != nullptr && static_cast<Unit*>(check.multi)->group != nullptr && check.type != 1) {
                            static_cast<Unit*>(check.multi)->group->group_sub->active = 1;
                        }

                        has_unit = 1;
                        break;

                    case 8: // Item
                        check.item_id = static_cast<uint16_t>(check_data->argument_values[arg_idx] + 3608);
                        break;

                    case 9: // Building
                        check.building = buildings_map[arg_value];
                        if (check.building == nullptr) {
                            CString msg;
                            msg.Format("Can't resolve building %d.", arg_value);
                            LogWarning(msg);
                            valid = false;
                        }
                        break;
                        
                    default:
                        check.data[data_idx++] = arg_value;
                        break;
                    }
                } else {
                    check.data[arg_idx] = arg_value;
                    if (arg_value < 9000) {
                        check.ref_scope[arg_idx] = 1;
                        arg_value = arg_value + -8000;
                    } else {
                        check.ref_scope[arg_idx] = 2;
                        arg_value = arg_value + -9000;
                    }
                    check.ref_page[arg_idx] = (byte)(arg_value / 100);
                    check.ref_index[arg_idx] = (byte)(arg_value % 100);
                    check.ref_count++;
                }
            }

            if (valid) {
                g_World->trigger_checks->AddTail(check);
                check_idx++;
            } else {
                check_idx_map[check_data->index] = 0;
                CString msg;
                msg.Format("Failed to create check %d %s", check_data->index, check_data->name);
                LogMessage(msg);
            }
        } else {
            if (check.type == 0x10002) {
                g_World->trigger_variables[check_idx] = check_data->argument_values[0];
                check_idx++;
            }
        }
    }

    for (int trig_idx = 0; trig_idx < alm->map_logic_triggers.GetSize(); ++trig_idx) {
        auto trigger_data = &alm->map_logic_triggers[trig_idx];
        if (trigger_data->check_ids[0] == 0) {
            continue;
        }

        Trigger trigger = Trigger();

        for (int check_idx = 0; check_idx < 3; check_idx = check_idx + 1) {
            if ((trigger_data->check_ids[check_idx * 2] != 0) && (trigger_data->check_ids[check_idx * 2 + 1] != 0)) {
                int32_t check1_id = check_idx_map[trigger_data->check_ids[check_idx * 2]];
                int32_t check2_id = check_idx_map[trigger_data->check_ids[check_idx * 2 + 1]];

                trigger.once = trigger_data->execute_once;

                Check check = Check();
                check.arg1 = check1_id;
                check.arg2 = check2_id;
                check.compare = trigger_data->check_operators[check_idx];

                POSITION check_iter = g_World->trigger_checks->GetHeadPosition();
                while (check_iter != nullptr) {
                    TriggerCheck& trigger_check = g_World->trigger_checks->GetNext(check_iter);
                    if (trigger_check.id == check1_id) {
                        trigger_check.counter++;
                    }
                    if (trigger_check.id == check2_id) {
                        trigger_check.counter++;
                    }
                }

                trigger.checks->AddTail(check);
            }
        }
        for (int action_idx = 0; action_idx < 4; action_idx = action_idx + 1) {
            if (trigger_data->instance_ids[action_idx] != 0) {
                if (5000 < trigger_data->instance_ids[action_idx]) {
                    CString msg;
                    msg.Format("Reference to suspicsious instant (%d) from pattern %d.", trigger_data->instance_ids[action_idx], trig_idx);
                    LogMessage(msg);
                }
                auto instance_id = instance_idx_map[trigger_data->instance_ids[action_idx]];
                trigger.actions->AddTail(instance_id);
            }
        }
        trigger.trigger_id = trig_idx;

        g_World->triggers->AddTail(trigger);
    }
}

// 59F1BE
void Srv1::LoadSacks(MapAlm* alm) {
    // Build lookup maps
    CMap<int32_t, int32_t, Building*, Building*> buildings_map;
    CMap<int32_t, int32_t, Unit*, Unit*> units_map;
    
    // Build units map
    POSITION unit_iter = dword_6CDB3C->unit_list.GetHeadPosition();
    while (unit_iter != nullptr) {
        Unit* unit = dword_6CDB3C->unit_list.GetNext(unit_iter);
        units_map[unit->TokenID] = unit;
    }
    if (this->srv_stru->units_list) {
        const auto& unit_list = this->srv_stru->units_list->unit_list;
        POSITION unit_iter = unit_list.GetHeadPosition();
        while (unit_iter != nullptr) {
            Unit* unit = unit_list.GetNext(unit_iter);
            units_map[unit->TokenID] = unit;
        }
    }
    
    // Build buildings map
    POSITION building_iter = this->srv_stru->building_list->GetHeadPosition();
    while (building_iter != nullptr) {
        Building* building = this->srv_stru->building_list->GetNext(building_iter);
        buildings_map[building->TokenID] = building;
    }

    // Process effects (MapEffectData array)
    for (int32_t effect_idx = 0; effect_idx < alm->effects.GetSize(); effect_idx++) {
        MapEffectData* effect_data = alm->effects[effect_idx];

        // Trap.
        if ((effect_data->trap_x != 0 || effect_data->trap_y != 0) && effect_data->values[0] < 4) {
            uint8_t data[6];
            data[0] = effect_data->type_id;
            data[1] = effect_data->spell_strength;
            data[2] = effect_data->modifiers_data[0].x;
            data[3] = effect_data->modifiers_data[0].y;
            data[4] = effect_data->modifiers_data[1].x;
            data[5] = effect_data->modifiers_data[1].y;

            uint16_t yx = (effect_data->trap_y << 8) | effect_data->trap_x;
            MapStuff_Instance->ApplyTerrainMod(yx, data);
        }

        // Process spellbook entries for units
        if (effect_data->sack == nullptr && (effect_data->values[0] & 8)) {
            Unit* unit = units_map[effect_data->values[1]];
            if (unit == nullptr) {
                CString msg;
                msg.Format("Can't resolve unit %d for spellbook.", effect_data->values[1]);
                LogMessage(msg);
                continue;
            }

            // Ensure unit has a spellbook
            if (unit->spell_book == nullptr) {
                unit->spell_book = new SpellBook();
                unit->unit_attrs |= 2;
                if (unit->mp_max != 0) {
                    unit->unit_attrs |= 4;
                }
            }

            // Add spell to spellbook
            Spell* spell = new Spell(effect_data->type_id);
            unit->spell_book->SetSpell(effect_data->type_id, spell);
        } else if ((effect_data->trap_x != 0 || effect_data->trap_y != 0) && effect_data->sack == nullptr && (effect_data->values[0] & 4)) {
            Building* building = buildings_map[effect_data->values[1]];
            if (building == nullptr) {
                CString msg;
                msg.Format("Can't resolve building %d for building - caster", effect_data->values[1]);
                LogMessage(msg);
                continue;
            }

            VirtualCaster* vcaster = new VirtualCaster();
            *vcaster->position = *building->position;
            vcaster->byte_0x3c = static_cast<uint8_t>(effect_data->effect_id);
            vcaster->properties[0] = static_cast<uint8_t>(effect_data->type_id);
            vcaster->properties[1] = static_cast<uint8_t>(effect_data->spell_strength);
            vcaster->properties[2] = static_cast<uint8_t>(effect_data->trap_x);
            vcaster->properties[3] = static_cast<uint8_t>(effect_data->trap_y);
            vcaster->pOwner = building->pOwner;

            this->srv_stru->virtual_casters_list.AddTail(vcaster);
        }
    }
    
    // Process sacks
    for (int32_t sack_idx = 0; sack_idx < alm->sacks.GetSize(); sack_idx++) {
        MapSackData* sack_data = alm->sacks[sack_idx];
        
        Inventory* inventory = nullptr;
        Unit* unit = nullptr;
        
        // Create inventory or resolve unit
        if (sack_data->unit_id == 0) {
            inventory = new Inventory();
        } else {
            unit = units_map[sack_data->unit_id];
        }
        
        // Process items
        for (int32_t item_idx = 0; item_idx < sack_data->item_ids.GetSize(); item_idx++) {
            uint16_t item_id = static_cast<uint16_t>(sack_data->item_ids[item_idx]);
            Item* item = g_GameDataRes.CreateItemById(item_id);
            
            if (item != nullptr) {
                // Get effect data ID for this item
                int32_t effect_id = sack_data->item_effect_ids[item_idx];
                if (effect_id != 0) {
                    MapEffectData* effect_data = alm->effects[effect_id - 1];

                    if (effect_data->trap_x != 0 || effect_data->trap_y != 0 || effect_data->sack == nullptr) {
                        continue;
                    }
                    
                    // Add primary effect
                    if (effect_data->values[0] != 0) {
                        Effect* effect = new Effect();
                        effect->effect_id = effect_data->values[0] + (modifier::damagefire - 1);
                        effect->damage_min = effect_data->values[1];
                        effect->damage_spread = effect_data->values[2];
                        item->_effects.AddTail(effect);
                        item->CalcValue();
                    }
                    
                    // Add secondary effect (spell)
                    if (effect_data->type_id != 0) {
                        Effect* effect = new Effect();
                        if (item->item_type == Item::ItemType::BOOK) {
                            effect->effect_id = modifier::teachspell;
                        } else {
                            effect->effect_id = modifier::castspell;
                        }
                        effect->spell_or_damage = effect_data->type_id;
                        effect->spell_value = effect_data->spell_strength;
                        item->_effects.AddTail(effect);
                        item->CalcValue();
                    }
                    
                    // Add modifier effects
                    for (int32_t i = 0; i < effect_data->modifiers_data.GetSize(); i++) {
                        MapEffectModifier* mod = &effect_data->modifiers_data[i];
                        Effect* effect = new Effect();
                        
                        if (mod->x == modifier::castspell) {
                            effect->effect_id = modifier::damagebonus;
                        } else {
                            effect->effect_id = mod->x;
                        }
                        
                        effect->spell_or_damage = mod->y;
                        effect->spell_value = mod->flags;
                        item->_effects.AddTail(effect);
                        item->CalcValue();
                    }
                }
                
                // Add item to inventory or unit
                if (sack_data->unit_id == 0) {
                    inventory->PutItemIntoBagAtDefault(item);
                } else {
                    uint16_t wielded = sack_data->items_wielded[item_idx];
                    if (wielded == 0) {
                        unit->inventory->PutItemIntoBagAtDefault(item);
                    } else {
                        unit->UseItemOrStash(item);
                    }
                }
            }
        }
        
        // Create sack on map if this is a ground sack
        if (sack_data->unit_id == 0 && inventory != nullptr) {
            TokenPos pos(sack_data->x >> 8, sack_data->y >> 8, MapStuff_Instance);
            this->srv_stru->sack_list->DumpInventoryAt(&pos, inventory, sack_data->gold, 0);
        }
    }
}

// Called when a player enters a map; streams the current game state to them.
// 4FF937
void Server::SendWorldState(Player* player, int32_t bool_arg4) {
    // Send existing players' PacketJoin data to the new player.
    g_NetStru1_main.SendExistingPlayers(player);

    // Reconnect check?
    #ifdef A2SERVER_PATCH
    const bool check = false; // WAT: Patch seems to try to check player flags against GMF_CMD_CHAT, but it uses `JMP` and skips the block unconditionally.
    #else
    const bool check = (player->hat_player_id == 0xF6D04773 && player->flags == 4);
    #endif
    if (check) {
        CString player_nick(player->name);

        int pipe_pos = player_nick.Find('|');
        if (pipe_pos != -1) {
            player_nick = player_nick.Mid(pipe_pos + 1);
        }

        Player* found_player = g_PlayersList->FindByName(player_nick);
        if (found_player != nullptr && dword_6CDB3C != nullptr) {
            // Transfer any unit from the original player to the re-connecting player.
            POSITION node = dword_6CDB3C->unit_list.GetHeadPosition();
            while (node != nullptr) {
                Unit* unit = dword_6CDB3C->unit_list.GetNext(node);
                if (unit != nullptr && unit->pOwner == found_player) {
                    g_NetStru1_main.TransferUnitOwnership(unit, player);
                }
            }
        }
    }

    // Arena mode: run the arena entry handler for this player.
    if (g_ServerConfig.gameType == 3) {
        this->HandleArenaPlayer(player);
    }

    // Broadcast a PacketJoin for the new player to all existing players.
    {
        PacketJoin* pkt = new PacketJoin(this->current_map_name);
        pkt->id = 6;
        // [pkt+0xA] = Server::field21_0xd4.  Offset 0xA within PacketJoin is
        // the first uint32_t after the base Packet (Packet = 0xA bytes, pkt+0xA =
        // player_id of PacketJoin, but the ASM stores field21_0xd4 there as a dword).
        pkt->__field_0xa = this->field21_0xd4;
        pkt->to_player_id = player->player_id;
        g_NetStru1_main.QueuePacketSend(pkt);
        delete pkt;
    }

    // Require the player to have a main unit.
    if (player->main_unit == nullptr) {
        LogMessage("Client " + player->name + " tries to enter mission without Hero. Return");
        g_NetStru1_main.AnnouncePlayerName("You can't enter mission without Hero", player);
        return;
    }

    // Reset the rejoining flag.
    player->mission_result = 0;

    // Early-out for some game types.
    if (this->field4_0x74 != 0 && bool_arg4 != 0) {
        if (g_ServerConfig.gameType == 0 || g_ServerConfig.gameType == 3) {
            // Diplomacy and vision-mask setup with the "Self" map player.

            Player* self_player = g_PlayersList->FindByName("Self");
            if (self_player != nullptr) {
                uint16_t self_id = self_player->player_id;
                uint16_t player_id = player->player_id;

                // Copy Self's diplomacy column into the new player's row (slots 2..15).
                for (int slot = 2; slot <= 15; ++slot) {
                    g_World->diplomacy.diplomacy[slot][player_id] = g_World->diplomacy.diplomacy[slot][self_id];
                    g_World->diplomacy.diplomacy[self_id][slot] = g_World->diplomacy.diplomacy[player_id][slot];
                }
                // Mark Self as allied with the new player.
                g_World->diplomacy.diplomacy[self_id][player_id] = 0x12;

                // Walk all non-AI players and update alliance / vision masks.
                for (POSITION node = g_PlayersList->list.GetHeadPosition(); node != nullptr;) {
                    Player* p = g_PlayersList->list.GetNext(node);
                    auto p_id = p->player_id;
                    if (p->is_ai != 0 || p_id == player_id) {
                        continue;
                    }

                    // Co-op: clear mutual alliance bytes.
                    if (g_ServerConfig.gameType == 0) {
                        g_World->diplomacy.diplomacy[p_id][player_id] = 0;
                        g_World->diplomacy.diplomacy[player_id][p_id] = 0;
                    }

                    // Clear each player's vision bits for the other.
                    player->vision_sharing_mask &= ~p->vision_sharing_id;
                    p->vision_sharing_mask &= ~player->vision_sharing_id;
                }
            }
        }
    }

    // Softcore (gameType == 2) team-alliance setup.
    if (g_ServerConfig.gameType == 2) {
        Player* red  = g_PlayersList->FindByName("Red");
        Player* blue = g_PlayersList->FindByName("Blue");

        uint16_t red_id = red->player_id;
        uint16_t blue_id = blue->player_id;
        uint16_t my_id = player->player_id;

        if (red != nullptr && blue != nullptr) {
            if (player->field_0xa70 == 0) {
                // Allied with Red (0x12), hostile to Blue (1).
                g_World->diplomacy.diplomacy[red_id][my_id] = 0x12;
                g_World->diplomacy.diplomacy[my_id][red_id] = 0x12;
                g_World->diplomacy.diplomacy[blue_id][my_id] = 1;
                g_World->diplomacy.diplomacy[my_id][blue_id] = 1;

                // Share vision with Red, clear from Blue.
                player->vision_sharing_mask |= red->vision_sharing_id;
                red->vision_sharing_mask |= player->vision_sharing_id;
                player->vision_sharing_mask &= ~blue->vision_sharing_id;
                blue->vision_sharing_mask &= ~player->vision_sharing_id;
            } else {
                // Allied with Blue (0x12), hostile to Red (1).
                g_World->diplomacy.diplomacy[blue_id][my_id] = 0x12;
                g_World->diplomacy.diplomacy[my_id][blue_id] = 0x12;
                g_World->diplomacy.diplomacy[red_id][my_id] = 1;
                g_World->diplomacy.diplomacy[my_id][red_id] = 1;

                // Share vision with Blue, clear from Red.
                player->vision_sharing_mask |= blue->vision_sharing_id;
                blue->vision_sharing_mask |= player->vision_sharing_id;

                player->vision_sharing_mask &= ~red->vision_sharing_id;
                red->vision_sharing_mask &= ~player->vision_sharing_id;
            }
        }
    }

    // Send updated terrain/diplomacy state to every non-AI player.
    for (POSITION node = g_PlayersList->list.GetHeadPosition(); node != nullptr;) {
        Player* p = g_PlayersList->list.GetNext(node);
        if (p != nullptr && p->is_ai == 0) {
            g_NetStru1_main.SendVisibilityState(p);
        }
    }

    // If `bool_arg4` is set and field_0x41 is not yet set, put the unit on the map (EnsurePlayerGroup).
    if (bool_arg4 != 0 && player->is_in_world == 0) {
        this->EnsurePlayerGroup(player);
    }

    // Mark the player as fully active and refresh world / map state.
    player->is_in_world = 1;
    player->is_active = 1;
    g_World->sub_5AFBFD(); // increment some counter (number of joined players?).
    MapStuff_Instance->scan_presence_grid.RebuildPresenceGrid();  // refresh scan grid

    // Clear vision mask bits for this player across every unit list.
    // Three separate lists are processed:
    //   a) The global pending unit list (dword_6CDB3C).
    //   b) srv_stru1->sack_list --- clears Token::field_x18 word bits (ClearSentToMask).
    //   c) srv_stru1->units_list.
    // Then for each player in g_PlayersList, also clear their per-player unit list.
    dword_6CDB3C->ClearSentToMask(player);

    this->srv_stru1->sack_list->ClearSentToMask(player);
    this->srv_stru1->units_list->ClearSentToMask(player);

    // Per-player unit lists.
    for (POSITION node = g_PlayersList->list.GetHeadPosition(); node != nullptr;) {
        Player* p = g_PlayersList->list.GetNext(node);
        if (p != nullptr) {
            p->unit_list->ClearSentToMask(player);
        }
    }

    // Send player's main unit's position.
    if (player->main_unit) {
        PacketInfo& inf = PacketInfo::Inst;
        inf.id = 0xABu;
        inf.to_player_id = player->player_id;
        inf.param1 = player->main_unit->position->GetX() & 0xFF;
        inf.param2 = player->main_unit->position->GetY() & 0xFF;

        g_NetStru1_main.QueuePacketSend(&inf);

        // Send all data for the main unit.
        g_NetStru1_main.SendUnitUpdate(player->main_unit, nullptr, -1, 0xFFB, 0, 0);
    }

    // Stream remaining server state to the joining player.
    g_NetStru1_main.SendPendingUnits(player);          // send units from dword_6CDB3C
    g_NetStru1_main.SendServerState(player);          // send server state
    g_NetStru1_main.SendKillStats(0, player);       // send kill stats
    g_NetStru1_main.SendUnitVisibility(player->main_unit, 0, nullptr); // hero visibility packet

    // Send any active area effects to the player.
    if (this->srv_stru1->effects_list != nullptr) {
        CList<SpellEffect*>& effect_list = this->srv_stru1->effects_list->list;
        for (POSITION node = effect_list.GetHeadPosition(); node != nullptr; ) {
            SpellEffect* effect = effect_list.GetNext(node);
            if (effect == nullptr) {
                continue;
            }

            // Check runtime class: is this an AreaEffect?
            if (!effect->IsKindOf(RUNTIME_CLASS(AreaEffect))) {
                continue;
            }
            AreaEffect* ae = reinterpret_cast<AreaEffect*>(effect);
            if (ae->zone_active != 0) {
                g_NetStru1_main.SendAreaEffect(ae, 1);
            }
        }
    }

    if (g_Server->field4_0x74 != 0) {
        g_NetStru1_main.SendSrvStru1State(&g_QuestMap, player, 0);
    }

    // Encode and send the full map-terrain packet to the player.
    // encode_list is a stack-local CArray<uint16_t> (= CWordArray, var_88 in ASM).
    // MapStuff_Instance->EncodeTerrain fills it with run-length encoded terrain data.
    // The result is copied into unk_6E9DB0 (the large static packet buffer)
    // beginning at offset 0xE, then the packet is sent.
    {
        // Set up terrain packet header.
        PacketTerrain& pkt = PacketTerrain::Inst;
        pkt.id = 0x9Bu;
        pkt.to_player_id = player->player_id;

        CWordArray encode_list;
        MapStuff_Instance->EncodeTerrain(&encode_list);

        int32_t count = encode_list.GetSize();
        uint16_t* src_ptr = encode_list.GetData();
        std::memcpy(&pkt.buf, src_ptr, count * 2);
        pkt.count = count;

        g_NetStru1_main.QueuePacketSend(&pkt);
    }

    // Finalise the player's session state.
    player->AddMoney(0, 1);                // notify mission entry
    g_NetStru1_main.SendBarePacket(3, player); // send "mission entered" event
    this->UpdateHatAnnounce();                      // server-state refresh
}


void Server::AssignPlayerSlot(Player* player, int32_t arg4) {
    //4FF439
    if (player->main_unit == nullptr) {
        return;
    }

    int slot = (arg4 >> 8) & 0xFF;
    player->drop_deadline_tick16 = 0;
    player->is_logged_in = 1;

    auto main_unit = player->main_unit;
    int phased_in = ((main_unit->unit_attrs & 8) != 0) ? 1 : 0;
    if (slot == 0 && phased_in != 0) {
        // Vanilla only scans for a free slot when arg4 gave none and the unit is phased in.
        for (slot = 1; slot < 0x10; slot++) {
            bool found = false;
            POSITION node = g_PlayersList->list.GetHeadPosition();
            while (node != nullptr) {
                Player* candidate = g_PlayersList->list.GetNext(node);
                if (candidate != nullptr) {
                    if (candidate->color_slot == static_cast<uint8_t>(slot) && candidate->is_ai == 0) {
                        found = true;
                        break;
                    }
                }
            }

            if (!found) {
                break;
            }
        }
    }

    if (slot != 0) {
        player->color_slot = static_cast<uint8_t>(slot);
    }

    PacketJoin& packet = PacketJoin::Inst;
    POSITION node = g_PlayersList->list.GetHeadPosition();
    while (node != nullptr) {
        Player* target = g_PlayersList->list.GetNext(node);
        if (target != nullptr) {
            if (target->is_ai == 0 && target->is_logged_in != 0) {
                bool should_send = true;
                if (target != player) {
                    #ifdef A2SERVER_PATCH
                    const bool condition = (player->flags & GMF_INVISIBLE) == GMF_INVISIBLE;
                    #else
                    const bool condition = player->hat_player_id == 0xF6D04773 && player->flags == 4;
                    #endif

                    if (condition) {
                        should_send = false;
                    }
                }

                if (should_send) {
                    std::strcpy(packet.name, (LPCTSTR)player->name);
                    packet.id = 0x96;
                    packet.player_id = player->player_id;
                    packet.token_id = player->token_id;
                    packet.color = player->color_slot - 1;
                    packet.flags = 0;

                    if (phased_in != 0) {
                        packet.flags |= 2;
                    }
                    
                    if (player->is_ai != 0) {
                        packet.flags |= 1;
                    }
                    
                    packet.to_player_id = target->player_id;

                    g_NetStru1_main.QueuePacketSend(&packet);

                    int msg_type = (phased_in != 0) ? 3 : 4;
                    g_NetStru1_main.FUN_0051ce86(msg_type, player->player_id, target);
                }
            }
        }
    }

    LogMessage("Player " + player->name + " has joined the game");

    player->AddMoney(0, 1);

    if (this->field18_0x94 != 0) {
        g_NetStru1_main.SendBarePacket(0xB7, player);
    }

    g_NetStru1_main.SendTickPacket(nullptr);
    if (this->field3_0x70 != 0) {
        g_NetStru1_main.SendAllData();
    }
}

// JoinPlayer --- validate and process a player join request.
// Returns: 0=success, 1=server full, 2=duplicate name, 3=banned name,
//          4=name too short, 5=invalid char data, 6=too strong, 7=too weak,
//          8=team play started, 9=shutdown initiated.
// Calling convention: __thiscall (ecx=this=Server*), args are stdcall-pushed.
// `name` / `login` are CStrings passed by value (caller owns them; function may assign into them).
int Server::JoinPlayer(uint32_t pkt_word0, uint32_t pkt_word1,
                       CString name,          // player display name (in/out)
                       CString login,         // player login credential
                       void* Block,           // raw network packet (heap-allocated, ownership transferred here)
                       int32_t block_size,
                       int32_t team_id) {
    Player* player = nullptr;

    #ifdef A2SERVER_PATCH
    const int max_players = g_ServerConfig.max_players;
    #else
    const int max_players = 16;
    #endif

    // 1. Server full?
    if (g_PlayersList->CountHumanPlayers() >= max_players) {
        LogMessage("Player " + name + " login " + login + " has been rejected (Server full)");

        // ASM also checks `if (player != 0) {delete player}`, but player is always null at this point, so we skip that.

        if (Block != nullptr) {
            delete[] Block;
        }
        return 1;
    }

    // 2. Team play already started?
    if (g_ServerConfig.gameType == 2 && g_PlayersList->AreTeamsStarted()) {
        LogMessage("Player " + name + " login " + login + " has been rejected (Team play already started)");
        if (Block != nullptr) {
            delete[] Block;
        }
        return 8;
    }

    // 3. Shutdown initiated?
    if (g_ShutdownIn < 0x7FFFFFFF || this->field59_0x208 != 0) {
        LogMessage("Player " + name + " login " + login + " has been rejected (Shutdown initiated)");
        if (Block != nullptr) {
            delete[] Block;
        }
        return 9;
    }

    // 4. Duplicate name check
    for (POSITION node = g_PlayersList->list.GetHeadPosition(); node != nullptr;) {
        Player* existing = g_PlayersList->list.GetNext(node);

        if (existing && std::strcmp(name, existing->name) == 0) {
            LogMessage("Player " + name + " login " + login + " has been rejected (Duplicated name)");
            if (Block != nullptr) {
                delete[] Block;
            }
            return 2;
        }
    }

    // 5. Split nickname and clan.
    CString nickname(name);
    CString clan;

    int pipe_pos = nickname.Find('|');
    if (pipe_pos != -1) {
        clan = nickname.Mid(pipe_pos + 1);
        nickname = nickname.Left(pipe_pos);
    }

    // 6. Banned name check
    for (int i = 0; i < g_ServerConfig.banned_names.GetSize(); ++i) {
        if (std::strcmp(nickname, g_ServerConfig.banned_names[i]) == 0) {
            LogMessage("Player " + name + " login " + login + " has been rejected (Banned name)");
            if (Block != nullptr) {
                delete[] Block;
            }
            return 3;
        }
    }

    // 7. Name starts with space -> banned
    if (nickname[0] == ' ') {
        LogMessage("Player " + name + " login " + login + " has been rejected (Banned name)");
        if (Block != nullptr) {
            delete[] Block;
        }
        return 3;
    }

    // --- 8. Name too short ---
    if (nickname.GetLength() <= 2) {
        LogMessage("Player " + name + " login " + login + " has been rejected (varName too short)");
        if (Block != nullptr) {
            delete[] Block;
        }
        return 4;
    }

    player = new Player();

    Human* unit = nullptr;

    // Process network packet.
    // TODO: create the Block struct as a real struct.
    if (*reinterpret_cast<uint32_t*>(Block) != 0xFFDDAA11u) {
        // Normal packet path
        unit = this->LoadCharacter(Block, block_size, player);
        Block = nullptr;
    } else {
        // Magic packet 0xFFDDAA11
        uint8_t* pkt = reinterpret_cast<uint8_t*>(Block);
        unit = this->CreatePlayerCharacter(player, pkt[5], pkt[6], pkt[7], pkt[8], pkt[9], pkt[10]);

        // Copy 8 bytes from Block+0xC into player+0x10 (token_id / hat_player_id area)
        std::memcpy(&player->hat_player_id, pkt + 0x0C, 8);

        player->name = name;

        #ifdef A2SERVER_TANGAR_HAT
        player->money = 0;
        #else
        player->money = 1000;
        #endif

        delete[] Block;
        Block = nullptr;
    }

    // Invalid character data?
    if (unit == nullptr) {
        LogMessage("Player " + name + " login " + login + " has been rejected (Invalid character data)");
        if (player != nullptr) {
            delete player;
            player = nullptr;
        }
        if (Block != nullptr) {
            delete[] Block;
        }
        return 5;
    }

    // Map level range checks.
    if (g_ServerConfig.gameType == 0 && g_ServerConfig.map_range_check != 0) {
        if (this->MapLevel < player->min_server_level) {
            LogMessage("Player " + name + " login " + login + " has been rejected (Too strong for this map)");
            if (player != nullptr) {
                delete player;
                player = nullptr;
            }
            if (Block != nullptr) {
                delete[] Block;
            }
            return 6;
        }

        if (this->MapLevel > player->max_server_level) {
            LogMessage("Player " + name + " login " + login + " has been rejected (Too weak for this map)");
            if (player != nullptr) {
                delete player;
                player = nullptr;
            }
            if (Block != nullptr) {
                delete[] Block;
            }
            return 7;
        }
    }

    // Normalize the player's stored name (var_34 / var_14 split).
    // Re-read name from the character data (player->name at +0x18, which was set by packet processing)
    CString var_34(player->name);
    CString var_14;

    int pipe_pos2 = var_34.Find('|');
    if (pipe_pos2 != -1) {
        var_14 = var_34.Mid(pipe_pos2 + 1);
        var_34 = var_34.Left(pipe_pos2);
    }

    // Replace leading space with '-'.
    if (var_34[0] == ' ') {
        var_34.SetAt(0, '-');
    }

    // Pad to at least length 3 with '-'.
    while (var_34.GetLength() < 3) {
        var_34 += '-';
    }

    // Rebuild name: if var_38 is non-empty, include suffix; otherwise just var_34.
    if (clan.GetLength() != 0) {
        name = var_34 + '|' + clan;
    } else {
        name = var_34;
    }

    // Set player fields.
    player->main_unit = unit;
    player->login = login;
    player->hat_char_id1 = pkt_word0;
    player->hat_char_id2 = pkt_word1;
    player->name = name;
    unit->name = name;
    player->is_ai = 0;
    player->drop_deadline_tick16 = this->tick16 + 300;
    player->is_logged_in = 0;

    // Team game flags.
    if (g_ServerConfig.gameType == 2) {
        player->team_ready = 0;
        player->field_0xa70 = (team_id == 1) ? 1 : 0;
    }

    // Add player to global list.
    g_PlayersList->AddPlayer(player);

    // Notify existing players (send join packets).
    this->SaveCharacter(unit);

    // Clear state fields.
    player->building_entered_from_yx = 0;
    player->mission_result = 0;

    // Initialize diplomacy.
    Player* self = g_PlayersList->FindByName("Self");
    if (self != nullptr) {
        int16_t partner_id = (int16_t)self->player_id;
        int16_t my_id      = (int16_t)player->player_id;

        for (int i = 2; i <= 15; ++i) {
            uint8_t val1 = g_World->diplomacy.diplomacy[i][partner_id];
            g_World->diplomacy.diplomacy[i][my_id] = val1;

            uint8_t val2 = g_World->diplomacy.diplomacy[partner_id][i];
            g_World->diplomacy.diplomacy[my_id][i] = val2;
        }

        g_World->diplomacy.diplomacy[partner_id][my_id] = 0x12;
    }

    // Set team pairing bytes and vision masks.
    for (POSITION node = g_PlayersList->list.GetHeadPosition(); node != nullptr;) {
        Player* other = g_PlayersList->list.GetNext(node);

        if (other != nullptr && other->is_ai == 0 && player->player_id != other->player_id) {
            int16_t other_id = other->player_id;
            int16_t my_id = player->player_id;

            switch (g_ServerConfig.gameType) {
            case 0:
                // Friendly: clear team slot bytes, remove from each other's vision mask
                g_World->diplomacy.diplomacy[other_id][my_id] = 0;
                g_World->diplomacy.diplomacy[my_id][other_id] = 0;
                other->vision_sharing_mask &= (uint16_t)(~other->vision_sharing_id);
                player->vision_sharing_mask &= (uint16_t)(~player->vision_sharing_id);
                break;
            case 1:
            case 3:
                // Deathmatch/arena: mark as enemy (1)
                g_World->diplomacy.diplomacy[other_id][my_id] = 1;
                g_World->diplomacy.diplomacy[my_id][other_id] = 1;
                other->vision_sharing_mask &= (uint16_t)(~other->vision_sharing_id);
                player->vision_sharing_mask &= (uint16_t)(~player->vision_sharing_id);
                break;
            case 2:
                // Team play
                if (player->field_0xa70 == other->field_0xa70) {
                    // Same team: mark 0x12 (teammate), share vision
                    g_World->diplomacy.diplomacy[other_id][my_id] = 0x12;
                    g_World->diplomacy.diplomacy[my_id][other_id] = 0x12;
                    player->vision_sharing_mask |= other->vision_sharing_id;
                    other->vision_sharing_mask |= player->vision_sharing_id;
                } else {
                    // Different team: mark 1 (enemy), clear vision
                    g_World->diplomacy.diplomacy[other_id][my_id] = 1;
                    g_World->diplomacy.diplomacy[my_id][other_id] = 1;
                    other->vision_sharing_mask &= (uint16_t)(~other->vision_sharing_id);
                    player->vision_sharing_mask &= (uint16_t)(~player->vision_sharing_id);
                }
                break;
            default:
                break;
            }
        }
    }

    // Player group setup.
    this->EnsurePlayerGroup(player);

    // Mark player as active.
    player->is_in_world = 1;
    player->is_active = 1;

    // Increment World::field65_0xc780
    g_World->sub_5AFBFD();

    // Notify MapStuff section
    MapStuff_Instance->scan_presence_grid.RebuildPresenceGrid();

    // Add character unit to map.
    MapStuff_Instance->AddUnitToMap(player->main_unit);

    // Set unit_attrs |= 8 (phased in).
    player->main_unit->unit_attrs |= 8;

    // Remove player->main_unit from dword_6CDB3C->unit_list.
    {
        CList<Unit*>& lst = dword_6CDB3C->unit_list;
        Unit* target = player->main_unit;

        // Find the node whose data == target
        POSITION pos = dword_6CDB3C->unit_list.Find(target);
        if (pos != nullptr) {
            dword_6CDB3C->unit_list.RemoveAt(pos);
        }
    }

    // Co-op mode: set starting enchantments.
    if (g_ServerConfig.gameType == 0) {
        uint32_t& enchantments = player->main_unit->enchantments;
        enchantments |= (1u << spell::shield);
        enchantments |= (1u << spell::invisibility);
        enchantments |= (1u << spell::protection_from_fire);
        enchantments |= (1u << spell::protection_from_water);
        enchantments |= (1u << spell::protection_from_air);
        enchantments |= (1u << spell::protection_from_earth);
    }

    // Arena mode: name/stat lookup.
    if (g_ServerConfig.gameType == 3) {
        this->GetArenaFrags(&player->name, &player->frags);
        this->AddArenaFrags(&player->name, 0);
    }

    // Server state update.
    this->UpdateHatAnnounce();

    return 0;
}

void Server::SignalGameStart(int32_t arg) {
    //4F94C0
    LogMessage("Server::SignalGameStart\n");
    if ((g_ServerConfig.gameType == 1) || (g_ServerConfig.gameType == 2)) {
        g_NetStru1_main.SendTeamState(0);
    }
    g_NetStru1_main.SendInfoPacket(0xc3, arg, 0, nullptr);
    this->field59_0x208 = 1;
}


ServerConfig::ServerConfig() {
    //4f6fc9
    this->game_speed = 4;
    this->repop_delay = 100;
    this->protocol = 2;
    this->current_map_index = 0;
    this->chat_range = 0x100;
    this->shout_delay = 0x78;
    this->save_is_server = 1;
    this->max_players = 0x10;
    this->server_name = "unnamed server";
    this->login_timeout = 0x3c;
    this->reconnect_delay = 1;
    this->gameType = 0;
    this->frag_limit = 0x7fffffff;
    this->map_range_check = 1;
    this->shutdown_delay = 5;
    this->always_load_sacks = 0;
    this->arena_time_limit = 0x7fffffff;
    this->treasure_probability = 100;
}

// Create (or revive) the hero for a player who is joining via the magic-packet path.
// 500907
Human* Server::CreatePlayerCharacter(Player* player, uint8_t body, uint8_t reaction, uint8_t mind, uint8_t spirit, uint8_t main_skill, uint8_t character_class) {
    // Does player already have the main unit (reconnect / rejoin)?
    if (player->main_unit != nullptr) {
        Human* unit = player->main_unit;
        if (unit->decay > 0) {
            unit->ReviveWithPenalty();           // reinitialize stats from class template
            unit->hp = unit->hp_max;
            unit->mp = unit->mp_max;
            player->unit_list->AddTail(unit);
            if (unit->group == nullptr) {
                Group* grp = new Group();
                player->group_list->groups.AddTail(grp);
                grp->AddUnit(unit);
            }
        }
        return player->main_unit;
    }

    // Create a new character.
    // Build file extension: ".f5" (default), or ".f"/".m" + (class & 0x3F)
    CString filename;
    if (character_class == 0) {
        filename = ".f5";
    } else {
        #ifdef A2SERVER_PATCH
        character_class = FilterAllowedCharacterPicture(character_class);
        #endif
        const char* ext = (character_class & 0x80) ? ".f" : ".m";
        filename.Format("%s%u", ext, character_class & 0x3F);
    }

    // Pick "Start_XX" prefix: bit6 = is_mage, bit7 = is_female
    bool is_mage   = (character_class & 0x40) != 0;
    bool is_female = (character_class & 0x80) != 0;
    const char* prefix = (!is_mage && !is_female) ? "Start_MF"
                       : (!is_mage &&  is_female) ? "Start_FF"
                       : ( is_mage && !is_female) ? "Start_MM"
                       :                            "Start_FM";

    Human* unit = new Human(CString(prefix) + filename, 1, nullptr);

    #ifdef A2SERVER_PATCH
    if (body < 15 || body > 43 || reaction < 15 || reaction > 43 || mind < 15 || mind > 43 || spirit < 15 || spirit > 43) {
        unit->body = unit->reaction = unit->mind = unit->spirit = 25;
    }
    #endif

    // Validate stat budget: sum of point costs must not exceed 140
    int32_t budget = 140 - StatLevelPoints(body) - StatLevelPoints(reaction) - StatLevelPoints(mind) - StatLevelPoints(spirit);
    if (budget >= 0) {
        unit->body     = body;
        unit->reaction = reaction;
        unit->mind     = mind;
        unit->spirit   = spirit;
    } else {
        unit->body = unit->reaction = unit->mind = unit->spirit = 25;
    }

    // Configure skills, then trigger stat-recalc `VMethod18()`.
    #ifdef A2SERVER_PATCH
    if (main_skill < 1 || main_skill > 4) {
        main_skill = 1;
    }
    #endif

    int32_t main_skill_level = 20;
    #ifdef A2SERVER_TANGAR_HAT
    main_skill_level = 0;
    #endif
    unit->InitCreationSkills(main_skill, main_skill_level);
    unit->RecalcStats();

    unit->hp = unit->hp_max;
    unit->mp = unit->mp_max;
    unit->name = player->name;
    unit->building_id = BldIdSet_AllocBit() & 0xFFFF;
    unit->pOwner = player;
    player->unit_list->AddTail(unit);

    // Create a group and place the unit in it
    Group* grp = new Group();
    player->group_list->groups.AddTail(grp);
    grp->AddUnit(unit);

    // Place the unit on the map at tile (8, 12)
    if (MapStuff_Instance != nullptr) {
        new (unit->position) TokenPos(8, 12, MapStuff_Instance);
    }

    player->main_unit = unit;
    if (this->field4_0x74 == 0) {
        unit->server_id = 21;
    }

    // Copy the player's vision-sharing ID into unit.
    unit->seen_by_mask = player->vision_sharing_id;

#ifdef A2CLIENT
    g_NetStru1_main.SendUnitUpdate(unit, player, -1, 0xffb, 0, 0);
#else
    player->min_server_level = 1;
    player->max_server_level = 1;
#endif
    return unit;
}

// 4ef4e7
// Loads a `Human` unit from a saved character packet.
Human* Server::LoadCharacter(void* data, int32_t size, Player* player) {
    CMemFile file(static_cast<uint8_t*>(data), size, 0);

    FileSectionBasicInfo* basic_info = nullptr;
    FileSectionStats* stats = nullptr;
    uint8_t* kill_stats = nullptr;
    PacketUnitStateVec* equipment = nullptr;
    PacketUnitStateVec* inventory = nullptr;
    uint8_t* section_40a = nullptr;
    uint32_t size_40a = 0;

    int32_t parse_ok = ParsePlayerFile_4F62E6(&file, &basic_info, &stats, &kill_stats, &equipment, &inventory, &section_40a, &size_40a);
    free(file.Detach());

    if (!parse_ok || basic_info == nullptr || stats == nullptr || equipment == nullptr) {
        return nullptr;
    }

    Human* unit = nullptr;
    switch (basic_info->character_class & 0xC0) {
    case 0x00:
        unit = new Human("Start_MF", 1, nullptr);
        break;
    case 0x40:
        unit = new Human("Start_MM", 1, nullptr);
        break;
    case 0x80:
        unit = new Human("Start_FF", 1, nullptr);
        break;
    case 0xC0:
        unit = new Human("Start_FM", 1, nullptr);
        break;
    }

    unit->body = stats->body;
    unit->reaction = stats->reaction;
    unit->mind = stats->mind;
    unit->spirit = stats->spirit;
    unit->face = basic_info->picture;
    for (int i = 0; i < 5; i++) {
        unit->experience_per_sphere[i] = stats->experience[i];
    }
    unit->main_sphere = basic_info->main_sphere;

    int32_t max_stat = 0;
    for (int32_t sphere = 1; sphere < 6; sphere++) {
        unit->hit_values.skill_levels[sphere] = ExperienceTable::GetLevel(unit->experience_per_sphere[sphere - 1]);
        max_stat = (std::max)(max_stat, (int32_t)unit->hit_values.skill_levels[sphere]);
    }

    for (int32_t sphere = 1; sphere < 6; sphere++) {
        unit->hit_values2.skill_levels[sphere] = unit->hit_values.skill_levels[sphere];
    }

    if (max_stat > 95) {
        player->min_server_level = 4;
    } else if (max_stat > 75) {
        player->min_server_level = 3;
    } else if (max_stat > 50) {
        player->min_server_level = 2;
    } else {
        player->min_server_level = 1;
    }

    if (max_stat < 26) {
        player->max_server_level = 1;
    } else if (max_stat < 51) {
        player->max_server_level = 2;
    } else if (max_stat < 90) {
        player->max_server_level = 3;
    } else {
        player->max_server_level = 4;
    }

    unit->experience = 0;
    for (int32_t sphere = 1; sphere < 6; sphere++) {
        unit->experience += unit->experience_per_sphere[sphere - 1];
    }

    unit->RecalcStats();

    if (unit->mp_max > 0) {
        unit->unit_attrs |= 6;
        if (unit->spell_book != nullptr) {
            delete unit->spell_book;
        }
        unit->spell_book = new SpellBook();
        stats->spells |= 0x1000000;
        for (int32_t spell_id = 1; spell_id <= spell::max_spell_id; spell_id++) {
            if ((stats->spells & (1 << spell_id)) != 0) {
                Spell* spell = new Spell(spell_id);
                unit->spell_book->SetSpell(spell_id, spell);
            }
        }
        if (unit->eye2 != nullptr) {
            unit->eye2->spell_id = (uint8_t)stats->active_spell;
        }
    }

    unit->name = player->name = basic_info->nick;
    player->money = stats->money;
    player->monster_kills = stats->monster_kills;
    player->deaths = stats->deaths;
    player->player_kills = stats->player_kills;
    player->frags = stats->frags;

    // Looks like this does `player->hat_player_id = basic_info->id1; player->flags = basic_info->id2;`?
    std::memcpy(&player->hat_player_id, basic_info, 8);

    unit->building_id = BldIdSet_AllocBit() & 0xFFFF;
    unit->pOwner = player;
    player->unit_list->AddTail(unit);

    Group* group = new Group();
    player->group_list->groups.AddTail(group);
    group->AddUnit(unit);

    if (kill_stats != nullptr) {
        std::memcpy(player->kill_stats, kill_stats, sizeof(player->kill_stats));
    }

    // Equip the 12 fixed equipment slots.
    unit->UnequipAll();
    if (unit->weapon != nullptr) {
        unit->inventory->PutItemIntoBagAtDefault(unit->Unequip(unit->weapon));
    }
    if (unit->shield != nullptr) {
        unit->inventory->PutItemIntoBagAtDefault(unit->Unequip(unit->shield));
    }
    if (unit->inventory != nullptr) {
        delete unit->inventory;
    }
    unit->inventory = new Inventory();
    uint8_t* eq_data = equipment->data;
    for (int32_t i = 0; i < 12; i++) {
        Item* item = ReadItem(&eq_data);
        if (item->item_id != 0) {
            unit->UseItemOrStash(item);
        }
        if (item->field14_0x50 != 0) {
            unit->field_0x1bc = 1;
        }
    }

    // Load the bag/inventory items.
    if (inventory != nullptr) {
        uint8_t* inv_data = inventory->data;
        while (inv_data < inventory->data + inventory->data_size) {
            Item* item = ReadItem(&inv_data);
            if (g_ServerConfig.gameType == 3 && item->item_id != 0 && item->world_equip != nullptr) {
                const auto& name = item->world_equip->name;
                if (name == "Potion Big Healing" || name == "Potion Medium Healing" || name == "Potion Health Regeneration") {
                    item->item_id = 0;
                }
            }
            if (item->item_id == 0) {
                delete item;
            } else {
                unit->inventory->PutItemIntoBagAtDefault(item);
                if (item->field14_0x50 != 0) {
                    unit->field_0x1bc = 1;
                }
            }
        }
    }

    delete unit->eye;
    unit->eye = new UnitEye();
    if (unit->eye2 != nullptr) {
        delete unit->eye2;
    }
    unit->eye2 = new UnitEye2();
    delete unit->position;
    unit->position = new TokenPos();
    unit->cast_target = nullptr;
    unit->some_spell = nullptr;
    unit->spell = nullptr;
    unit->some_item = nullptr;
    unit->last_hit_by = nullptr;
    unit->ResetActionState();
    unit->hp = unit->hp_max;
    unit->mp = unit->mp_max;
    unit->decay = 0;
    unit->RecalcStats();
    unit->unit_attrs &= ~0x08;
    unit->sent_to_mask = 0;
    unit->seen_by_mask = player->vision_sharing_id;

    return unit;
}

// Processes a cheat/admin command string sent by a player.
//
// Admin commands (require ns2->field_0x29c != 0):
//   #kick <name>:         disconnect a non-admin player
//   #locate <name>:       report a player's map position back to the caller
//
// Non-admin network commands (available to any connected player):
//   #set latency <ms>:    clamp the connection latency (50--10000 ms)
//   #show latency:        reply with current latency and packet-loss stats
//   #ready:               signal readiness for a team-game start (gameType==2)
//
// Cheat (elevated-privilege) commands --- require field_0xa98 > 50, or the player
// must first authenticate via the coward-activation code:
//   #create <item|Gold>:  spawn an item or gold in the player's inventory
//   #modify self|army +god|+spell <id>|+spells|+knowledge: adds stuff
//   #summon <name>:       summon a unit next to the player
//   #killall / #kill all: kill every visible enemy unit
//   #kill cheaters:       reset & kill all cheating players
//   #kill <name>:         kill a named player's units
//   #pickup all:          collect every sack on the current map
//   #show map / #hide map:  reveal or conceal the full map for the player
//   #victory:             trigger a victory event for the player's team
//   #event <id>:          fire an in-game script event
// 502D0B
void Server::CheatCommand(Player* player, CString cheat_string) {
    LogMessage("Server::CheatCommand: player " + player->name + " sent cheat command: " + cheat_string);

    if (this->field4_0x74 != 0) {
        // Non-admin commands.
        NetStru2* ns2 = g_NetStru1_main.GetClientByPlayerID(player->player_id);
        if (ns2 == nullptr || ns2->is_local_player == 0) {
            if (cheat_string.Find("#set latency ") == 0) {
                cheat_string = cheat_string.Mid(13);
                cheat_string.TrimLeft();
                int32_t lat = atoi(cheat_string);
                if (lat != 0) {
                    if (lat < 50 || lat > 10000) {
                        g_NetStru1_main.FUN_0051ce86(6, player->player_id, player);
                        return;
                    }
                    NetStru2* ns2 = g_NetStru1_main.GetClientByPlayerID(player->player_id);
                    if (ns2 != nullptr) {
                        g_CLlDriver.SetLatency(ns2->uid, lat);
                        g_NetStru1_main.SendLatency(lat, player);
                    }
                }
            } else if (cheat_string.Find("#show latency") == 0) {
                NetStru2* ns2 = g_NetStru1_main.GetClientByPlayerID(player->player_id);
                if (ns2 != nullptr) {
                    int32_t lat  = g_CLlDriver.GetLatency(ns2->uid);
                    int32_t loss = g_CLlDriver.GetPacketLoss(ns2->uid);
                    cheat_string.Format("%s: latency %dms, packet loss %d.%03d%%",
                        player->name, lat, loss / 1000, loss % 1000);
                    g_NetStru1_main.AnnouncePlayerName(cheat_string, player);
                }
            } else if (cheat_string.Find("#ready") == 0) {
                if (g_ServerConfig.gameType == 2) {
                    if (g_PlayersList->AreTeamsStarted() == 0 && player->team_ready == 0) {
                        player->team_ready = 1;
                        g_NetStru1_main.SendTeamState(0);
                        if (g_PlayersList->AreTeamsStarted() != 0) {
                            // All players now ready --- start the match.
                            g_NetStru1_main.FUN_0051ce86(10, 0, nullptr);
                            this->OnTeamsReady();
                            this->UpdateQuestRune(0, 0);
                            this->UpdateQuestRune(1, 0);
                        }
                    }
                }
            }
        } else {
            // Admin commands.
            if (cheat_string.Find("#kick ") == 0) {
                cheat_string = cheat_string.Mid(6);
                cheat_string.TrimLeft();
                Player* target = g_PlayersList->FindByName(cheat_string);
                if (target != nullptr) {
                    NetStru2* tns2 = g_NetStru1_main.GetClientByPlayerID(target->player_id);
                    if (tns2 != nullptr && tns2->is_local_player == 0) {
                        g_NetStru1_main.AnnouncePlayer(target);
                    }
                }
            } else if (cheat_string.Find("#locate ") == 0) {
                cheat_string = cheat_string.Mid(8);
                cheat_string.TrimLeft();
                Player* target = g_PlayersList->FindByName(cheat_string);
                if (target != nullptr && target->is_ai == 0) {
                    NetStru2* tns2 = g_NetStru1_main.GetClientByPlayerID(target->player_id);
                    if (tns2 != nullptr) {
                        CString msg;
                        msg.Format("%s (%d,%d)",
                            (LPCTSTR)target->name,
                            target->main_unit->position->GetX(),
                            target->main_unit->position->GetY());
                        g_NetStru1_main.AnnouncePlayerName(msg, player);
                    }
                }
            }
        }
    }
    
    bool is_reconnect = player->hat_player_id == 0xF6D04773u && player->flags == 4;

    if (this->field4_0x74 != 0 && !is_reconnect) {
        return;
    }

    // Players whose cheat level is not yet elevated must supply the activation secret.
    if (player->gm_level <= 50) {
        if (this->coward_activation.CheckActivationKey(cheat_string) == 1) {
            CString msg = "Player " + player->name + " enable cheating.";
            LogMessage(msg);
            player->gm_level = 0xFF;
            if (!is_reconnect) {
                g_NetStru1_main.FUN_0051ce86(5, player->player_id, nullptr);
            }
        }
        return;
    }

    // player->field_0xa98 > 50: elevated cheat commands available.

    if (cheat_string.Find("#create ") == 0) {
        cheat_string = cheat_string.Mid(8);
        cheat_string.TrimLeft();

        if (player->main_unit->decay != 0) {
            // Unit is dead/decayed --- cannot receive items.
            if (!is_reconnect) {
                g_NetStru1_main.FUN_0051ce86(6, player->player_id, nullptr);
            }
            return;
        }

        // ParseCountPrefix trims an optional leading integer from cheat_string and returns it.
        int32_t count = ParseCountPrefix(&cheat_string);

        if (_stricmp((const char*)cheat_string, "Gold") == 0) {
            player->AddMoney(count, 0);
            if (!is_reconnect) {
                g_NetStru1_main.FUN_0051ce86(7, player->player_id, nullptr);
            }
            return;
        }

        Item* item = g_GameDataRes.FindItemTemplate(&cheat_string);
        if (item == nullptr) {
            if (!is_reconnect) {
                g_NetStru1_main.FUN_0051ce86(6, player->player_id, nullptr);
            }
            return;
        }

        if (!item->IsCheatCreatable()) {
            // Item cannot be freely created; destroy the template copy we got.
            item->~Item(); // virtual dtor with delete-flag=1 equivalent
            if (!is_reconnect) {
                g_NetStru1_main.FUN_0051ce86(6, player->player_id, nullptr);
            }
            return;
        }

        item->count = (uint16_t)count;
        player->main_unit->inventory->PutItemIntoBagAtDefault(item);
        player->main_unit->AddCarriedWeight(0);
        g_NetStru1_main.SendUnitUpdate(player->main_unit, player, -1, 0xFFB, 0, 0);
        if (!is_reconnect) {
            g_NetStru1_main.FUN_0051ce86(7, player->player_id, 0);
        }
    } else if (cheat_string.Find("#modify ") == 0) {
        cheat_string = cheat_string.Mid(8);
        cheat_string.TrimLeft();

        int target_mode = 0; // 1 = self, 2 = army
        if (cheat_string.Find("self") == 0) {
            cheat_string = cheat_string.Mid(4);
            cheat_string.TrimLeft();
            target_mode = 1;
        } else if (cheat_string.Find("army") == 0) {
            cheat_string = cheat_string.Mid(4);
            cheat_string.TrimLeft();
            target_mode = 2;
        } else {
            return; // unknown target keyword
        }

        if (cheat_string.Find("+god") == 0) {
            if (target_mode == 1) {
                player->main_unit->ApplyInvincibility();
                g_NetStru1_main.SendUnitUpdate(player->main_unit, 0, (int32_t)0xA31FFFFF, 0xFFB, 0, 0);
            } else if (target_mode == 2) {
                for (POSITION node = player->unit_list->unit_list.GetHeadPosition(); node != nullptr;) {
                    Unit* unit = player->unit_list->unit_list.GetNext(node);
                    if (unit != nullptr) {
                        unit->ApplyInvincibility();
                        g_NetStru1_main.SendUnitUpdate(unit, 0, (int32_t)0xA31FFFFF, 0xFFB, 0, 0);
                    }
                }
            }
            g_NetStru1_main.FUN_0051ce86(7, player->player_id, nullptr);
        } else if (cheat_string.Find("+spell ") == 0) {
            cheat_string = cheat_string.Mid(7);
            cheat_string.TrimLeft();
            if (target_mode == 1) {
                SpellBook* spell_book = player->main_unit->spell_book;
                if (spell_book == nullptr) {
                    return;
                }
                int32_t spell_id = atoi(cheat_string);
                if (spell_id > 0 && spell_id <  g_GameDataRes.spells.GetSize()) {
                    Spell* sp = new Spell((uint8_t)spell_id);
                    spell_book->SetSpell(spell_id, sp);
                }
                // FUN_00519221(&g_NetStru1_main,&pHStack_4c->_,(pHStack_4c->_)._.pOwner,0xa31fffff, 0xffb,0,0);
                g_NetStru1_main.SendUnitUpdate(player->main_unit, player, 0xA31FFFFF, 0xFFB, 0, 0);
                g_NetStru1_main.FUN_0051ce86(7, player->player_id, nullptr);
            }
        } else if (cheat_string.Find("+spells") == 0) {
            if (target_mode == 1) {
                SpellBook* spell_book = player->main_unit->spell_book;
                if (spell_book == nullptr) {
                    return;
                }
                for (int i = 1; i <= 29; i++) {
                    Spell* sp = new Spell(i);
                    spell_book->SetSpell(i, sp);
                }
                g_NetStru1_main.SendUnitUpdate(player->main_unit, player, (int32_t)0xA31FFFFF, 0xFFB, 0, 0);
                if (!is_reconnect) {
                    g_NetStru1_main.FUN_0051ce86(7, player->player_id, nullptr);
                }
            }
        }

        // +knowledge --- all above paths fall through here
        if (cheat_string.Find("+knowledge") == 0) {
            g_NetStru1_main.SendKillStats(0, player);
        }
    } else if (cheat_string.Find("#summon ") == 0) {
        cheat_string = cheat_string.Mid(8);
        cheat_string.TrimLeft();

        if (player->main_unit == nullptr) {
            return;
        }

        int32_t count = ParseCountPrefix(&cheat_string);
        int32_t is_hero = 0;

        if (cheat_string.Find("hero") == 0) {
            is_hero = 1;
            count = 1;
            cheat_string = cheat_string.Mid(5);
            cheat_string.TrimLeft();
        }

        for (int i = 1; i <= count; i++) {
            this->SummonUnit(&cheat_string, player->main_unit, is_hero);
        }
    } else if (cheat_string.Find("#killall") == 0 || cheat_string.Find("#kill all") == 0) {
        // Kill every enemy player.
        for (POSITION node = g_PlayersList->list.GetHeadPosition(); node != nullptr;) {
            Player* p = g_PlayersList->list.GetNext(node);
            if (p == nullptr) {
                continue;
            }
            int diplomacy = g_World->diplomacy.diplomacy[player->player_id][p->player_id];
            if (diplomacy & 1) {
                p->KillAllUnits();
            }
        }
        if (!is_reconnect) {
            g_NetStru1_main.FUN_0051ce86(7, player->player_id, player);
        }
    } else if (cheat_string.Find("#kill cheaters") == 0) {
        for (POSITION node = g_PlayersList->list.GetHeadPosition(); node != nullptr;) {
            Player* p = g_PlayersList->list.GetNext(node);
            if (p != nullptr && p->gm_level > 50 && p != player) {
                p->gm_level = 0;
                p->KillAllUnits();
            }
        }
    } else if (cheat_string.Find("#kill ") == 0) {
        cheat_string = cheat_string.Mid(6);
        cheat_string.TrimLeft();
        Player* target = g_PlayersList->FindByName(cheat_string);
        if (target != nullptr) {
            target->KillAllUnits();
            if (target->hat_player_id != 0xF6D04773 || target->flags != 4) {
                g_NetStru1_main.FUN_0051ce86(7, target->player_id, nullptr);
            }
        }
    } else if (cheat_string.Find("#pickup all") == 0) {
        if (player->main_unit == nullptr) {
            return;
        }

        SackList* sack_list = this->srv_stru1->sack_list;
        POSITION pos = sack_list->list.GetHeadPosition();
        while (pos != nullptr) {
            POSITION cur_pos = pos;
            Sack* sack = sack_list->list.GetNext(pos); // advances pos
            this->PrepareSackPickup(sack);
            MapStuff_Instance->RemoveSack(sack);
            g_Server->srv_stru1->sack_list->list.RemoveAt(cur_pos);
            player->main_unit->PickUpSack(sack);
        }

        if (!is_reconnect) {
            g_NetStru1_main.FUN_0051ce86(7, player->player_id, nullptr);
        }

        LogMessage("All sacks picked up");
    } else if (cheat_string.Find("#show map") == 0) {
        g_NetStru1_main.SendInfoPacket(0xAA, 1, 0, player);
        if (!is_reconnect) {
            g_NetStru1_main.FUN_0051ce86(7, player->player_id, nullptr);
        }
    } else if (cheat_string.Find("#hide map") == 0) {
        g_NetStru1_main.SendInfoPacket(0xAA, 0, 0, player);
        if (!is_reconnect) {
            g_NetStru1_main.FUN_0051ce86(7, player->player_id, nullptr);
        }
    } else if (cheat_string.Find("#victory") == 0) {
        g_NetStru1_main.SendInfoPacket(0xAA, 2, 0, player);
    } else if (cheat_string.Find("#event ") == 0) {
        cheat_string = cheat_string.Mid(7);
        cheat_string.TrimLeft();
        int32_t event_id = atoi(cheat_string);
        g_NetStru1_main.SendGameEvent(player, event_id, 0);
    }
}

Shop g_DefaultShop;     // unk_6D10B8

// 502C50
Shop* Server::FindShopAt(TokenPos* pos) {
    if (this->field38_0x1a4 != 0) {
        return &g_DefaultShop;
    }
    if (this->srv_stru1->building_list == nullptr) {
        return nullptr;
    }
    Building* building = this->srv_stru1->building_list->FindNearby(pos);
    if (building == nullptr) {
        return nullptr;
    }
    if (!building->IsKindOf(RUNTIME_CLASS(Shop))) {
        return nullptr;
    }
    return static_cast<Shop*>(building);
}

const char* PacketType(Packet *pkt) {
    if (dynamic_cast<PacketInfo*>(pkt) != nullptr) { return "PacketInfo"; }
    else if (dynamic_cast<PacketJoin*>(pkt) != nullptr) { return "PacketJoin"; }
    else if (dynamic_cast<PacketTerrain*>(pkt) != nullptr) { return "PacketTerrain"; }
    else if (dynamic_cast<PacketUnitUpdate*>(pkt) != nullptr) { return "PacketUnitUpdate"; }
    else if (dynamic_cast<PacketWord*>(pkt) != nullptr) { return "PacketWord"; }
    else if (dynamic_cast<PacketItemOperation*>(pkt) != nullptr) { return "PacketItemOperation"; }
    else if (dynamic_cast<PacketCmd*>(pkt) != nullptr) { return "PacketCmd"; }
    else if (dynamic_cast<Packet3Dwords*>(pkt) != nullptr) { return "Packet3Dwords"; }
    else if (dynamic_cast<PacketDword*>(pkt) != nullptr) { return "PacketDword"; }
    else if (dynamic_cast<PacketAbility*>(pkt) != nullptr) { return "PacketAbility"; }
    else if (dynamic_cast<PacketEight*>(pkt) != nullptr) { return "PacketEight"; }
    else if (dynamic_cast<PacketMoveCmd*>(pkt) != nullptr) { return "PacketMoveCmd"; }
    else if (dynamic_cast<PacketEffect*>(pkt) != nullptr) { return "PacketEffect"; }
    else if (dynamic_cast<PacketAoeZone*>(pkt) != nullptr) { return "PacketAoeZone"; }
    else if (dynamic_cast<PacketMount*>(pkt) != nullptr) { return "PacketMount"; }
    else if (dynamic_cast<PacketPlayerInfo*>(pkt) != nullptr) { return "PacketPlayerInfo"; }
    else if (dynamic_cast<PacketData*>(pkt) != nullptr) { return "PacketData"; }
    else if (dynamic_cast<PacketUnitStateVec*>(pkt) != nullptr) { return "PacketUnitStateVec"; }
    else if (dynamic_cast<PacketSync*>(pkt) != nullptr) { return "PacketSync"; }
    else if (dynamic_cast<PacketUnitProperties*>(pkt) != nullptr) { return "PacketUnitProperties"; }
    else if (dynamic_cast<PacketPing*>(pkt) != nullptr) { return "PacketPing"; }
    else if (dynamic_cast<Packet*>(pkt) != nullptr) { return "Packet"; }

    return "unknown";
}

void CheckPacketType(Packet* pkt, const char* want_type) {
    const char* got_type = PacketType(pkt);
    if (strcmp(got_type, want_type) != 0) {
        CString msg;
        if (pkt->order_mode != 0) {
            msg.Format("sub_504a96 [group 0x%x]: Packet type mismatch: incoming packet is '%s', but handler is '%s'", pkt->id, got_type, want_type);
        } else {
            msg.Format("sub_504a96 [0x%x]: Packet type mismatch: incoming packet is '%s', but handler is '%s'", pkt->id, got_type, want_type);
        }
        LogMessage(msg);
    }
}

void Server::sub_504a96(Packet* pkt) {
    PacketItemOperation* packet_item = reinterpret_cast<PacketItemOperation*>(pkt);
    CString message;
    void* vtable = *reinterpret_cast<void**>(pkt);
    const char* packet_type = PacketType(pkt);

    message.Format("sub_504a96: received item operation packet id=0x%02X, order_mode=%d, player_id=%d;  packet is '%s' --- 0x%x", pkt->id, pkt->order_mode, packet_item->from_player_id, packet_type, vtable);
    LogMessage(message);

    // PATH 1: Group order (pkt->order_mode >= 1 means multiple units)
    if (pkt->order_mode != 0) {
        if (!g_World) {
            return;
        }
        if (packet_item->count == 0) {
            return;
        }

        Unit* unit = this->FindPlayerUnit(packet_item->from_player_id, packet_item->entries[0]);
        if (!unit) {
            return;
        }

        Player* player = unit->pOwner;
        GroupList* group_list = player->group_list;
        Group* group = group_list->AllocGroup();
        group->AddUnit(unit);
        unit->CancelQueuedSpell();

        for (int i = 1; i < packet_item->count; ++i) {
            Unit* u = this->FindPlayerUnit(packet_item->from_player_id, packet_item->entries[i]);
            if (u) {
                group->AddUnit(u);
                u->CancelQueuedSpell();
            }
        }
        group_list->groups.AddTail(group);

        Token* target = nullptr; // Unit or building.
        if (pkt->order_mode == 3) {
            // Unit target.
            target = dword_6CDB3C->FindById(packet_item->param);
            if (!target) {
                // Building target.
                target = this->srv_stru1->building_list->FindById(packet_item->param);
            }
            if (!target) {
                return;
            }
        }

        Spell* cast_spell = nullptr;
        if (pkt->id == 0x1E || pkt->id == 0x1F) {
            uint8_t spell_id = BOOK_POS_TO_SPELL_ID[packet_item->slot];
            packet_item->slot = spell_id;
            // Find hero unit in group that matches the spell type
            for (POSITION node = group->unit_list.GetHeadPosition(); node; ) {
                Unit* u = group->unit_list.GetNext(node);
                if (u && u->spell_book) {
                    cast_spell = u->spell_book->GetSpell(spell_id);
                    u->spell = cast_spell;
                }
            }
        }

        // Group order dispatch
        uint8_t x = packet_item->x;
        uint8_t y = packet_item->y;

        switch (pkt->id) {
        case 0x12: // move to position?
            g_World->GroupStopCommand(group);
            player->ResetAfkTimer();
            break;
        case 0x13:
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                g_World->sub_5A99C7(unit);
                player->ResetAfkTimer();
            }
            break;
        case 0x14:
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                g_World->GroupGoHomeCommand(group);
                player->ResetAfkTimer();
            }
            break;                  
        case 0x16:
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                g_World->GroupMoveCommand(group, x, y);
                player->ResetAfkTimer();
            }
            break;
        case 0x17:
            g_World->GroupGuardCommand(group, 0);
            player->ResetAfkTimer();
            break;
        case 0x18:
            g_World->GroupAggressiveStance(group);
            player->ResetAfkTimer();
            break;
        case 0x19: // move to building?
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                g_World->GroupAttackCommand(group, static_cast<Unit*>(target));
                player->ResetAfkTimer();
            }
            break;
        case 0x1A: // attack-move to position?
            CheckPacketType(pkt, "PacketItemOperation");
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                g_World->sub_5AC881(group, x, y);
                player->ResetAfkTimer();
            }
            break;
        case 0x1B: // attack building?
            CheckPacketType(pkt, "PacketItemOperation");
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                g_World->GroupAttackTargetCommand(group, static_cast<Unit*>(target), 0);
                player->ResetAfkTimer();
            }
            break;
        case 0x1C: // Defend location.
            CheckPacketType(pkt, "PacketItemOperation");
            LogMessage("defend location comes");
            g_World->GroupMoveCommand(group, x, y);
            break;
        case 0x1D: // patrol?
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                g_World->GroupPatrolCommand(group, x, y);
                player->ResetAfkTimer();
            }
            break;
        case 0x1E: // cast at unit/building?
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                g_World->GroupCastAtUnitCommand(group, static_cast<Unit*>(target), cast_spell);
                player->ResetAfkTimer();
            }
            break;
        case 0x1F: // cast at position?
            CheckPacketType(pkt, "PacketItemOperation");
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                player->ResetAfkTimer();
                if (cast_spell && cast_spell->IsAreaSpell(packet_item->x, packet_item->y)) {
                    g_World->GroupCastAtPositionCommand(group, x, y, cast_spell);
                }
            }
            break;
        case 0x21:
            CheckPacketType(pkt, "PacketItemOperation");
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                player->ResetAfkTimer();
                Sack* sack = MapStuff_Instance->GetSackAt(x, y);
                if (!sack) {
                    CString msg;
                    msg.Format("Sack not found at %d,%d", x, y);
                    LogMessage(msg);
                    break;
                }
                unit->inventory->default_position = packet_item->param;
                g_World->OrderSwarmTo(unit, x, y);
            }
            break;
        case 0x24:
            CheckPacketType(pkt, "PacketItemOperation");
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                player->ResetAfkTimer();
                auto* building = this->srv_stru1->building_list->FindById(packet_item->param);
                if (!building) {
                    CString msg;
                    msg.Format("No building #%d", packet_item->param);
                    LogMessage(msg);
                    break;
                }
                g_World->OrderEnterBuilding(unit, building);
            }
            break;
        case 0x25: // Cast point spell with a scroll.
        case 0x26: // Cast area spell with a scroll.
            CheckPacketType(pkt, "PacketItemOperation");
            if (g_ServerConfig.gameType != 2 || g_PlayersList->AreTeamsStarted() != 0) {
                player->ResetAfkTimer();
                auto* item = player->main_unit->inventory->TakeItemAt(packet_item->slot, 1);
                if (!item || item->_effects.IsEmpty()) {
                    break;
                }

                Effect* eff = item->_effects.GetHead();
                if (eff && eff->effect_id == 0x29) {
                    player->main_unit->some_item = item;

                    Spell* spell = new Spell(eff->spell_or_damage);
                    player->main_unit->spell = spell;

                    spell->SetPower(eff->spell_value);
                    if (packet_item->id == 0x25) {
                        g_World->GroupCastAtUnitCommand(group, static_cast<Unit*>(target), spell);
                    } else {
                        if (!spell->IsAreaSpell(packet_item->x, packet_item->y)) {
                            player->main_unit->inventory->PutItemIntoBag(packet_item->slot, item);
                            player->main_unit->some_item = nullptr;
                            delete spell;
                            player->main_unit->spell = nullptr;
                        } else {
                            g_World->GroupCastAtPositionCommand(group, x, y, spell);
                        }
                    }
                }
            }
            break;
        case 0x4D:
            g_World->GroupStopCommand(group);
            if (g_ServerConfig.gameType == 0) {
                player->StartAfkTimer();
            }
            break;
        default:
            break;
        }
        return;
    }

    // PATH 2: Single command dispatch
    Player* player = nullptr;
    Packet3Dwords* packet_3d = reinterpret_cast<Packet3Dwords*>(pkt);
    PacketJoin* packet_join = reinterpret_cast<PacketJoin*>(pkt);
    PacketCmd* packet_cmd = reinterpret_cast<PacketCmd*>(pkt);
    PacketInfo* packet_info = reinterpret_cast<PacketInfo*>(pkt);
    PacketDword* packet_dword = reinterpret_cast<PacketDword*>(pkt);
    PacketData* packet_data = reinterpret_cast<PacketData*>(pkt);
    PacketWord* packet_word = reinterpret_cast<PacketWord*>(pkt);

    // TODO: re-check 0x39, 0x48

    switch (pkt->id) {
    case 0x02: // Send player stat update?
#ifdef A2CLIENT
        //allods2 50001c
    {
        NetStru2* cli = g_NetStru1_main.GetClientByLowUid(packet_3d->field_0x5);
        this->Allods2_JoinPlayer(packet_3d->field_0xa, packet_3d->field_0xe, packet_3d->field_0x16, cli, packet_3d->field_0x12);
    }
#else
        CheckPacketType(pkt, "Packet3Dwords");

        player = this->FindPlayerById(packet_3d->from_player_id);
        if (player) {
            NetStru2* ns2 = g_NetStru1_main.GetClientByPlayerID(packet_3d->from_player_id);
            if (ns2 && ns2->is_claimed != 0) {
                this->AssignPlayerSlot(player, packet_3d->value3);
            }
        }
#endif
        break;

    case 0x04: // Join mission.
        CheckPacketType(pkt, "PacketInfo");
        player = g_PlayersList->FindById(packet_info->from_player_id);
        if (player) {
            this->SendWorldState(player, (packet_info->param1 == 0) ? 1 : 0);
        }
        break;

    case 0x05:
        CheckPacketType(pkt, "Packet");
        player = this->FindPlayerById(pkt->from_player_id);
        if (player) {
            player->is_active = 0;
        }
        break;

    case 0x07: // Execute string command
        CheckPacketType(pkt, "PacketJoin");
        {
            CString cmd(packet_join->name);
            this->SaveGame(&cmd);
            break;
        }

    case 0x08:
        CheckPacketType(pkt, "Packet");
        LogMessage("Loading not suppported now.");
        break;

    case 0x09:
        CheckPacketType(pkt, "Packet");
        LogMessage("Client request shutdown.");
        break;

    case 0x22: // Item operation.
        CheckPacketType(pkt, "PacketCmd");
        {
            Unit* target_unit = this->FindPlayerUnit(packet_cmd->from_player_id, packet_cmd->unit_id);
            if (!target_unit) {
                break;
            }

            Humanoid* human = static_cast<Humanoid*>(target_unit);

            uint8_t from_type = packet_cmd->type;  // 1=unequip, 2=from_bag, 3=pickup_ground, 4=from_shop
            uint8_t to_type = packet_cmd->subtype; // 1=equip,   2=to_bag,   3=drop_ground,   4=sell_shop

            // Both types in [4..8]: shop-slot interaction --- delegate entirely.
            if (from_type >= 4 && from_type <= 8 && to_type >= 4 && to_type <= 8) {
                Shop* shop = this->FindShopAt(human->position);
                if (shop) {
                    shop->HandleItemOperation(
                        human,
                        from_type, packet_cmd->param1,
                        to_type, packet_cmd->param2,
                        packet_cmd->param3
                    );
                }
                break;
            }

            // from_type == 3: pickup sack from ground.
            if (from_type == 3) {
                Sack* sack = MapStuff_Instance->GetSackAtPos(human->position);
                if (!sack) {
                    LogMessage("Invalid pickup order - no sack there.");
                } else {
                    human->inventory->default_position = packet_cmd->param1;
                    human->state = 2;
                }
                break;
            }

            uint32_t update_mask = 0;
            int32_t equip_mask = 0;
            int32_t bag_pos = 0;
            int32_t bag_pos_delta = 0;
            int32_t count_before = 0;
            human->item_op_dirty_flags = 0;

            Item* item = nullptr;
            if (from_type == 2) {
                // Pick item from bag.
                count_before = human->inventory->items.GetSize();
                item = human->inventory->TakeItemAt(packet_cmd->param1, packet_cmd->param3);
                if (item) {
                    update_mask |= 0x282000;
                    item->field15_0x54 = 0;
                }
            } else if (from_type == 4) {
                // Buy item from shop.
                Shop* shop = this->FindShopAt(human->position);
                if (!shop) {
                    break;
                }
                item = shop->BuyItem(human, packet_cmd->param1, packet_cmd->param3);
            } else if (from_type == 1) {
                // Unequip item from body slot.
                if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
                    break;
                }
                int32_t slot = (int16_t)packet_cmd->param1 + 1;
                if (slot == 1) {
                    item = human->Unequip(human->weapon);
                } else if (slot == 2) {
                    item = human->Unequip(human->shield);
                } else if (slot >= 3 && slot <= 12) {
                    if (human->IsHumanoid() == 0) {
                        LogMessage("Error - Trying to takeoff armor from non humanoid ");
                    } else {
                        item = human->Unequip(human->equipment[slot]);
                    }
                }
                if (item) {
                    item->field15_0x54 = 0;
                    update_mask |= 0x482000;
                    equip_mask  |= (slot < 3) ? 3 : (1 << (slot - 1));
                }
            }

            if (!item) {
                break;
            }

            if (to_type == 1) {
                // Equip item.
                human->inventory->default_position = packet_cmd->param1;
                uint16_t item_slot_type = (item->item_id >> 8) & 0xF;
                equip_mask |= (item_slot_type < 3) ? 3 : (1 << (item_slot_type - 1));
                Item* prev = item;
                item = human->UseItem(item); // equip; returns displaced item if any
                if (item) {
                    human->inventory->PutItemIntoBag(packet_cmd->param1, item);
                }
                if (from_type == 2 && human->inventory->items.GetSize() == count_before && item != prev) {
                    bag_pos = packet_cmd->param1;
                    bag_pos_delta = 1;
                }
                human->AddCarriedWeight(0);
                update_mask |= 0x482000;
            } else if (to_type == 2) {
                // Put item into bag slot.
                human->inventory->PutItemIntoBag(packet_cmd->param2, item);
                human->AddCarriedWeight(0);
                update_mask |= 0x282000;
            } else if (to_type == 4) {
                // Sell item to shop.
                item->pOwner = human->pOwner;
                Shop* shop = this->FindShopAt(human->position);
                if (!shop) {
                    break;
                }
                shop->SellItemFor(human, packet_cmd->param2, item);
                update_mask |= 0x82000;
            } else if (to_type == 3) {
                // Drop item on ground (create sack).
                Inventory* sack_inv = new Inventory();
                sack_inv->PutItemIntoBagAtDefault(item);
                uint8_t drop_x = (uint8_t)(packet_cmd->param2 & 0xFF);
                uint8_t drop_y = (uint8_t)(packet_cmd->param2 >> 8);
                int dx = std::abs((int)human->position->GetX() - (int)drop_x);
                int dy = std::abs((int)human->position->GetY() - (int)drop_y);
                if (dx <= 2 && dy <= 2) {
                    TokenPos new_pos(drop_x, drop_y, MapStuff_Instance);
                    this->srv_stru1->sack_list->DropInventoryAt(&new_pos, sack_inv, 0, 1);
                } else {
                    this->srv_stru1->sack_list->DropInventoryAt(human->position, sack_inv, 0, 1);
                }
                human->AddCarriedWeight(0);
                update_mask |= 0x82000;
            }

            g_NetStru1_main.SendUnitUpdate(human, nullptr,
                                    update_mask | human->item_op_dirty_flags,
                                    equip_mask, bag_pos, bag_pos + bag_pos_delta);
            DWORD tick = GetTickCount();
            if (tick - human->pOwner->last_save_tick > 15000 || to_type == 3) {
                this->SaveCharacter(human);
            }
            break;
        }

    case 0x23: // Gold drop - create money sack on the ground
        CheckPacketType(pkt, "PacketInfo");
        {
            player = g_PlayersList->FindById(packet_info->from_player_id);
            if (!player) {
                CString msg;
                msg.Format("Order error: no such Player %d", packet_info->from_player_id);
                LogMessage(msg);
                break;
            }

            UnitList* unit_list = player->unit_list;
            Unit* unit = (unit_list && unit_list->unit_list.GetSize() > 0)
                        ? unit_list->unit_list.GetHead()
                        : nullptr;

            int32_t amount = packet_info->param1;
            if (!unit || amount < 1 || player->money < amount) {
                break;
            }

            player->money -= amount;

            uint8_t drop_x = static_cast<uint8_t>(packet_info->param2 & 0xFF);
            uint8_t drop_y = static_cast<uint8_t>((packet_info->param2 >> 8) & 0xFF);

            int dx = std::abs(static_cast<int>(unit->position->GetX()) - static_cast<int>(drop_x));
            int dy = std::abs(static_cast<int>(unit->position->GetY()) - static_cast<int>(drop_y));

            if (dx <= 2 && dy <= 2) {
                TokenPos drop_pos(drop_x, drop_y, MapStuff_Instance);
                this->srv_stru1->sack_list->DumpInventoryAt(&drop_pos, nullptr, amount, 1);
            } else {
                this->srv_stru1->sack_list->DumpInventoryAt(unit->position, nullptr, amount, 1);
            }

            if (GetTickCount() - unit->pOwner->last_save_tick > 15000) {
                this->SaveCharacter((Humanoid*)unit);
            }
            break;
        }

    case 0x32: // Enter shop.
        CheckPacketType(pkt, "PacketWord");
        {
            Unit* unit = this->FindPlayerUnit(packet_word->from_player_id, packet_word->value);
            if (!unit) {
                break;
            }
            Shop* shop = this->FindShopAt(unit->position);
            if (!shop) {
                break;
            }
            unit->CancelQueuedSpell();
            unit->pOwner->building_entered_from_yx = unit->position->GetY() * 0x100 + unit->position->GetX();
            shop->AttachUnit(unit);
            break;
        }

    case 0x33:
        CheckPacketType(pkt, "PacketWord");
        {
            Unit* unit = this->FindPlayerUnit(packet_word->from_player_id, packet_word->value);
            if (!unit) {
                break;
            }
            Shop* shop = this->FindShopAt(unit->position);
            if (!shop) {
                break;
            }
            shop->ConfirmBuy(unit);
            if (GetTickCount() - unit->pOwner->last_save_tick > 15000) {
                this->SaveCharacter((Humanoid*)unit);
            }
            break;
        }

    case 0x34:
        CheckPacketType(pkt, "PacketWord");
        {
            Unit* unit = this->FindPlayerUnit(packet_word->from_player_id, packet_word->value);
            if (!unit || !unit->position) {
                break;
            }
            Shop* shop = this->FindShopAt(unit->position);
            if (!shop) {
                break;
            }
            shop->ConfirmSell(unit);
            if (GetTickCount() - unit->pOwner->last_save_tick > 15000) {
                this->SaveCharacter((Humanoid*)unit);
            }
            break;
        }

    case 0x35:
        CheckPacketType(pkt, "PacketWord");
        {
            Unit* unit = this->FindPlayerUnit(packet_word->from_player_id, packet_word->value);
            if (!unit) {
                break;
            }
            Shop* shop = this->FindShopAt(unit->position);
            if (!shop) {
                break;
            }
            shop->ReturnItems(unit);
            break;
        }

    case 0x36:
        CheckPacketType(pkt, "PacketWord");
        {
            Unit* unit = this->FindPlayerUnit(packet_word->from_player_id, packet_word->value);
            if (!unit) {
                break;
            }
            Shop* shop = this->FindShopAt(unit->position);
            if (!shop) {
                break;
            }
            shop->ReturnItems(unit);
            shop->CloseShop(unit);
            break;
        }

    case 0x38: // Enter inn.
        CheckPacketType(pkt, "PacketWord");
        {
            Unit* unit = this->FindPlayerUnit(packet_word->from_player_id, packet_word->value);
            if (!unit) {
                break;
            }
            Inn* inn = this->FindInnAt(unit->position);
            if (!inn) {
                break;
            }
            unit->CancelQueuedSpell();
            unit->pOwner->building_entered_from_yx = unit->position->GetY() * 0x100 + unit->position->GetX();
            inn->UpdateQuests(unit);
            CString m;
            m.Format("Enter inn: player %d, unit %d, inn %d at (%d, %d)", packet_word->from_player_id, packet_word->value, inn->building_id, unit->position->GetX(), unit->position->GetY());
            LogMessage(m);
            break;
        }

    case 0x39:
        CheckPacketType(pkt, "PacketInfo");
        {
            player = g_PlayersList->FindById(packet_info->from_player_id);
            if (player) {
                this->SendMapInfo(player);
            }
            break;
        }

    case 0x3A:
        CheckPacketType(pkt, "PacketInfo");
        {
            Unit* unit = this->FindPlayerUnit(packet_info->from_player_id, packet_info->param1);
            if (!unit) {
                break;
            }
            Inn* inn = this->FindInnAt(unit->position);
            if (!inn) {
                break;
            }
            inn->RemoveUnit(static_cast<Humanoid*>(unit), packet_info->param2);
            CString m;
            m.Format("Exit inn: player %d, unit %d, field: %x", packet_info->from_player_id, packet_info->param1, packet_info->param2);
            LogMessage(m);
            break;
        }

    case 0x3B: // Map file chunk download --- send a chunk of current_map_name to the client
        CheckPacketType(pkt, "PacketInfo");
        {
            player = this->FindPlayerById(packet_info->from_player_id);
            if (!player) {
                break;
            }

            PacketPlayerInfo& resp = PacketPlayerInfo::Inst;
            resp.to_player_id = player->player_id;
            resp.id = 0xC0;

            // Copy the map file name into preamble.
            int backslash_pos = this->current_map_name.ReverseFind('\\');
            CString tail;
            const char* name_src;
            if (backslash_pos == -1) {
                name_src = this->current_map_name;
            } else {
                tail = this->current_map_name.Mid(backslash_pos + 1);
                name_src = tail;
            }
            std::strcpy(reinterpret_cast<char*>(resp.preamble), name_src);

            int32_t file_start = packet_info->param1;

            CFile f;
            if (f.Open(this->current_map_name, CFile::modeRead | CFile::shareDenyNone, nullptr)) {
                int32_t file_length = f.GetLength();

                int32_t chunk_size = 0x1000;
                if (file_length < file_start + 0x1000) {
                    chunk_size = file_length - file_start;
                }

                resp.offset = file_start;
                resp.total_length = file_length;

                resp.count = chunk_size;
                f.Seek(static_cast<LONG>(file_start), CFile::begin);
                f.Read(resp.var_data, static_cast<UINT>(chunk_size));
                f.Close();
            }
            g_NetStru1_main.QueuePacketSend(&resp);
            g_NetStru1_main.SendTickPacket(player);
            break;
        }

    case 0x3E:
        CheckPacketType(pkt, "PacketDword");
        {
            if (this->field4_0x74 != 0) {
                break;
            }
            player = this->FindPlayerById(packet_dword->from_player_id);
            if (player) {
                player->AddMoney(packet_dword->value, 1);
            }
            break;
        }

    case 0x3F:
        CheckPacketType(pkt, "Packet");
        if (this->field4_0x74 == 0) {
            g_DefaultShop.GenerateAssortment();
        }
        break;

    case 0x45: // Diplomacy bulk update
        CheckPacketType(pkt, "PacketData");
        {
            if (!g_World || (g_ServerConfig.gameType != 0 && g_ServerConfig.gameType != 3)) {
                break;
            }

            player = g_PlayersList->FindById(packet_data->from_player_id);
            if (!player) {
                break;
            }

            int count = packet_data->count;
            const int16_t* modification = reinterpret_cast<const int16_t*>(packet_data->data);

            for (POSITION node = g_PlayersList->list.GetHeadPosition(); node;) {
                Player* other = g_PlayersList->list.GetNext(node);
                if (!other) {
                    break;
                }

                int other_id = other->player_id;

                uint8_t* diplo_slot = &g_World->diplomacy.diplomacy[player->player_id][other_id];
                uint8_t old_val = *diplo_slot;
                uint8_t new_val = static_cast<uint8_t>(modification[other_id] & 0x17);

                if (old_val == new_val) {
                    continue;
                }

                *diplo_slot = new_val;

                if ((new_val & 0x10) == 0) {
                    // Vision sharing disabled with `other`
                    player->vision_sharing_mask &= ~other->vision_sharing_id;
                    if (old_val & 0x10) {
                        // Was sharing before --- revoke visibility
                        g_NetStru1_main.SendUnitUpdate(player->main_unit, other, 0x400000, 0xffb, 0, 0);
                    }
                } else {
                    // Vision sharing enabled with `other`
                    player->vision_sharing_mask |= other->vision_sharing_id;
                    if (!(old_val & 0x10)) {
                        // Wasn't sharing before --- send full unit state for each of player's units
                        if (player->unit_list) {
                            for (POSITION unode = player->unit_list->unit_list.GetHeadPosition(); unode;) {
                                g_NetStru1_main.SendUnitUpdate(player->unit_list->unit_list.GetNext(unode), other, 0xa35fffff, 0xffb, 0, 0);
                            }
                        }
                    }
                }
                g_NetStru1_main.SendVisibilityState(other);
            }
            break;
        }

    case 0x46: // Player parameter change
        CheckPacketType(pkt, "PacketInfo");
        {
            if (!g_World) {
                break;
            }
            player = this->FindPlayerById(packet_info->from_player_id);
            if (!player) {
                break;
            }

            int32_t param_type  = packet_info->param1;
            int32_t param_value = packet_info->param2;

            if (param_type >= 0x80) {
                CString msg;
                msg.Format("Request to set unknown parameter %d unprocessed", param_type);
                LogMessage(msg);
                break;
            }

            int32_t val;

            switch (param_type) {
            case 1:
                val = (param_value == 1) ? 10 : (param_value == 2) ? 30 : 0;
                g_World->sub_5AF805(0, val * 2, val * 3, player);
                g_World->sub_5AFA01(val, val, val, player);
                break;
            case 2:
                val = (param_value == 0) ? 0 : (param_value == 1) ? 2 : 1;
                g_World->SetFormation(player, val);
                break;
            case 3: // Autobuff mask
                if (player->settings) {
                    player->settings->autobuff_mask = static_cast<uint8_t>(param_value);
                }
                break;
            case 4: {
                    Unit* unit = this->FindPlayerUnit(packet_info->from_player_id, static_cast<uint16_t>(param_value));
                    if (unit && unit->eye2) {
                        uint8_t new_spell_id = static_cast<uint8_t>(param_value >> 16);
                        if (unit->eye2->spell_id == new_spell_id) {
                            unit->eye2->spell_id = 0;
                        } else {
                            unit->eye2->spell_id = new_spell_id;
                        }
                        
                        Humanoid* human = static_cast<Humanoid*>(unit);
                        g_NetStru1_main.SendUnitUpdate(human, human->pOwner, 0x100000, 0xffb, 0, 0);
                    }
                    break;
                }
            default: {
                CString msg;
                msg.Format("Request to set unknown parameter %d unprocessed", param_type);
                LogMessage(msg);
                break;
            }
            }
            break;
        }

    case 0x48: { // Join map.
#ifdef A2CLIENT
        Player* player = this->FindPlayerById(packet_info->field_0x5);
        if (player) {
            Human* human = this->CreatePlayerCharacter(player, packet_info->field_0xa & 0xff, 
                                            (packet_info->field_0xa >> 8) & 0xff, 
                                            (packet_info->field_0xa >> 16) & 0xff, 
                                            (packet_info->field_0xa >> 24) & 0xff,
                                            (packet_info->field_0xe) & 0xff,
                                            (packet_info->field_0xe >> 8) & 0xff );
            g_NetStru1_main.SendTickPacket(human->pOwner);

            if (GetTickCount() - human->pOwner->field_0xa7c > 15000) {
#pragma message("REVERSE IT");
                this->SaveCharacter(human); // similar to FUN_004f2c57, so need reverse it!
                //FUN_004f2c57(local_104);
            }
        }
        break;
#else
            CheckPacketType(pkt, "PacketInfo");
            player = this->FindPlayerById(packet_info->from_player_id);
            if (!player) {
                break;
            }

            if (g_ServerConfig.gameType == 0 && (player->main_unit->unit_attrs & 8) != 0) {
                {
                    Effect* eff = new Effect("castSpell=Invisibility:20");
                    eff->itemDataID  = 0xC;
                    eff->typeId      = 0x20;
                    eff->usage_type |= 1;
                    eff->spell_value = 0x140;
                    eff->RemoveFromUnit(player->main_unit);
                }
                {
                    Effect* eff = new Effect("castSpell=Protection_from_Fire:200");
                    eff->itemDataID     = 4;
                    eff->typeId         = 0x10;
                    eff->usage_type    |= 1;
                    eff->effect_id      = 0x15;
                    eff->spell_or_damage = 0x96;
                    eff->spell_value    = 0x140;
                    eff->RemoveFromUnit(player->main_unit);
                }
                {
                    Effect* eff = new Effect("castSpell=Protection_from_Water:200");
                    eff->itemDataID     = 8;
                    eff->typeId         = 0x18;
                    eff->usage_type    |= 1;
                    eff->effect_id      = 0x16;
                    eff->spell_or_damage = 0x96;
                    eff->spell_value    = 0x140;
                    eff->RemoveFromUnit(player->main_unit);
                }
                {
                    Effect* eff = new Effect("castSpell=Protection_from_Earth:200");
                    eff->itemDataID     = 0x13;
                    eff->typeId         = 0x2E;
                    eff->usage_type    |= 1;
                    eff->effect_id      = 0x18;
                    eff->spell_or_damage = 0x96;
                    eff->spell_value    = 0x140;
                    eff->RemoveFromUnit(player->main_unit);
                }
                {
                    Effect* eff = new Effect("castSpell=Protection_from_Air:200");
                    eff->itemDataID     = 0xD;
                    eff->typeId         = 0x22;
                    eff->usage_type    |= 1;
                    eff->effect_id      = 0x17;
                    eff->spell_or_damage = 0x96;
                    eff->spell_value    = 0x140;
                    eff->RemoveFromUnit(player->main_unit);
                }
                {
                    Effect* eff = new Effect("castSpell=Shield:200");
                    eff->itemDataID     = 0x1B;
                    eff->typeId         = 0x3E;
                    eff->usage_type    |= 1;
                    eff->effect_id      = 0x10;
                    eff->spell_or_damage = 0x96;
                    eff->spell_value    = 0x140;
                    eff->RemoveFromUnit(player->main_unit);
                }
            }

            g_NetStru1_main.SendUnitUpdate(player->main_unit, player, 0xFFFFFFFF, 0xFFB, 0, 0);
            if ((player->main_unit->unit_attrs & 8) != 0) {
                player->main_unit->ReturnToMap();
            }
            g_NetStru1_main.SendGameInfo(g_ServerConfig.game_speed, player);
            break;
#endif
        }

    case 0x49:
        CheckPacketType(pkt, "PacketData");
        {
            if (this->field4_0x74 != 0) {
                break;
            }
            player = g_PlayersList->FindById(packet_data->from_player_id);
            if (!player || !player->main_unit) {
                break;
            }

            int idx = packet_data->count;
            if (idx <= 0 || idx >= g_GameDataRes.humans.GetSize()) {
                break;
            }

            Human* new_unit = this->SummonUnit(&g_GameDataRes.humans[idx].name, player->main_unit, 1);
            if (new_unit) {
                new_unit->name = CString(packet_data->data);
                g_NetStru1_main.SendUnitUpdate(new_unit, new_unit->pOwner, 0xFFFFFFFF, 0xFFB, 0, 0);
            }
            break;
        }

    case 0x4A:
        CheckPacketType(pkt, "Packet");
        {
            player = g_PlayersList->FindById(pkt->from_player_id);
            if (player) {
                g_NetStru1_main.ResendPlayerUnits(player);
            }
            break;
        }

    case 0x4B: // Set respawn flag when unit is deeply dead (hp < -39)
        CheckPacketType(pkt, "Packet");
        {
            player = this->FindPlayerById(pkt->from_player_id);
            if (player && player->main_unit && player->main_unit->hp < -39) {
                player->respawn_requested = 1;
            }
            break;
        }

    case 0x4C: // "Hurt myself" command.
        CheckPacketType(pkt, "Packet");
        {
            if (!g_World) {
                break;
            }
            player = this->FindPlayerById(pkt->from_player_id);
            if (player) {
                player->ResetAfkTimer();

                if (player->main_unit && player->main_unit->hp > -1) {
                    player->main_unit->hp -= 1;
                    g_NetStru1_main.SendLevelUp(player->main_unit, 1);
                }
            }
            break;
        }

    case 0x91: // Chat message.
        CheckPacketType(pkt, "PacketJoin");
        {
            CString msg(packet_join->name);
            if (msg.IsEmpty()) {
                break;
            }

            player = g_PlayersList->FindById(packet_join->from_player_id);
            if (!player) {
                break;
            }

            LogMessage(player->name + ": " + msg);

            if (msg[0] == '#') {
                this->CheatCommand(player, msg);
                break;
            }

            uint8_t chat_type = packet_join->token_id;   // chat type (byte 0xb)
            uint8_t recipient_id = packet_join->player_id;  // private message target (byte 0xa)

            PacketJoin out;
            out.id = packet_join->id;
            out.__field_0xa = (uint32_t)(packet_join->from_player_id & 0xFF) | ((uint32_t)chat_type << 8);

            switch (chat_type) {
            case 0: // Regular message: send to all non-AI players within range.
                for (POSITION node = g_PlayersList->list.GetHeadPosition(); node;) {
                    Player* other = g_PlayersList->list.GetNext(node);
                    if (!other->is_ai && player->main_unit->position->Distance(other->main_unit->position) <= g_ServerConfig.chat_range) {
                        out.to_player_id = other->player_id;
                        strcpy(out.name, packet_join->name);
                        g_NetStru1_main.QueuePacketSend(&out);
                    }
                }
                break;
            case 1: // Allies: send to players with diplomacy bit 2 set toward sender.
                for (POSITION node = g_PlayersList->list.GetHeadPosition(); node;) {
                    Player* other = g_PlayersList->list.GetNext(node);
                    if (!other->is_ai && (g_World->diplomacy.diplomacy[player->player_id][other->player_id] & 2) != 0) {
                        out.to_player_id = other->player_id;
                        strcpy(out.name, packet_join->name);
                        g_NetStru1_main.QueuePacketSend(&out);
                    }
                }
                break;
            case 2: // Private message.
                out.from_player_id = player->player_id;
                out.player_id = player->player_id;
                out.token_id = chat_type;
                // CC self.
                out.to_player_id = packet_join->to_player_id;
                strcpy(out.name, packet_join->name);
                g_NetStru1_main.QueuePacketSend(&out);
                // Deliver to target.
                out.to_player_id = (uint16_t)recipient_id;
                g_NetStru1_main.QueuePacketSend(&out);
                break;
            case 3: // Yell: global with cooldown.
                if (player->shout_cooldown == 0) {
                    out.from_player_id = player->player_id;
                    out.player_id = player->player_id;
                    out.token_id = chat_type;
                    out.to_player_id = 0;
                    strcpy(out.name, packet_join->name);
                    g_NetStru1_main.QueuePacketSend(&out);
                    player->shout_cooldown = g_ServerConfig.shout_delay;
                } else {
                    g_NetStru1_main.FUN_0051ce86(8, player->shout_cooldown, player);
                }
                break;
            case 4: // Global broadcast (no cooldown)
                out.from_player_id = player->player_id;
                out.player_id = player->player_id;
                out.token_id = chat_type;
                out.to_player_id = 0;
                strcpy(out.name, packet_join->name);
                g_NetStru1_main.QueuePacketSend(&out);
                break;
            default:
                break;
            }
            break;
        }

    case 0xAE: // Server-wide announcement
        CheckPacketType(pkt, "PacketJoin");
        {
            PacketJoin announcement;
            std::strncpy(announcement.name, packet_join->name, 1023);
            g_NetStru1_main.QueuePacketSend(&announcement);
            break;
        }

    case 0xBE:
        CheckPacketType(pkt, "Packet");
        player = this->FindPlayerById(pkt->from_player_id);
        if (player->main_unit != nullptr) {
            g_NetStru1_main.SendUnitUpdate(player->main_unit, player, 0xffffffff, 0xffb, 0, 0);
        }
        break;

    case 0xC1: // Set network latency for this connection?
        CheckPacketType(pkt, "PacketDword");
        {
            int32_t latency = packet_dword->value;
            if (latency == 0 || (50 <= latency && latency <= 10000)) {
                NetStru2* ns2 = g_NetStru1_main.GetClientByLowUid(packet_dword->from_player_id);
                if (ns2) {
                    g_CLlDriver.SetLatency(ns2->uid, latency);
                }
            }
            break;
        }

    case 0xD0: // Reconnect?
        CheckPacketType(pkt, "PacketInfo");
        {
            // Only handle if this connection has not yet claimed a player (field_0x2a8 == 0)
            NetStru2* ns2 = g_NetStru1_main.GetClientByLowUid(packet_info->from_player_id);
            if (!ns2 || ns2->is_claimed != 0) {
                break;
            }

            // Find the target player by two identity fields carried in the packet
            Player* target = g_PlayersList->FindByHatCharId(packet_info->param1, packet_info->param2);
            // Only proceed if the target player has no active NetStru2 yet
            if (target && g_NetStru1_main.GetClientByPlayerID(target->player_id) == nullptr) {
                ns2->player_id = target->player_id;
                ns2->is_claimed = 1;
                ns2->str = target->login;
                g_NetStru1_main.SendSpectatorState(ns2);
            }
                break;
        }
    }
}





void Server::Allods2_JoinPlayer(int32_t id, int32_t arg, CString name, NetStru2* client, uint32_t params) {
    //Allods2.exe 4f9a78    Join Player
    if (name.GetLength() < 1) {
        CString str;
        str.Format("Illegal player %s ID: %d", name, id);
        LogMessage(str);

        this->JoinErrorSend(client, 1, "Your name too short. At least 1 letter required.");
        return;
    }

    if (name == "Self" && this->field4_0x74 == 0) {
        name = g_PlayersList->list.GetHead()->name;
    }

    CString nickname = name;

    int pipe_pos = nickname.Find('|');

    if (pipe_pos != -1) {
        nickname = nickname.Left(pipe_pos);
    }

    if (this->field4_0x74) {
        for (int i = 0; i < g_ServerConfig.banned_names.GetSize(); i++) {
            if (nickname == g_ServerConfig.banned_names[i]) {
                CString str;
                str.Format("Player %s rejected (banned name)", name);
                LogMessage(str);

                this->JoinErrorSend(client, 2, "Restricted name. Try different.");
                return;
            }
        }

        if (g_CLlDriver.provider == 4) {
            for (int i = 0; i < g_ServerConfig.banned_ips.GetSize(); i++) {
                CString banIP = g_ServerConfig.banned_ips[i];
                if (false) { //banIP == )
                    CString str;
                    str.Format("Player %s rejected (banned ip)", name);
                    LogMessage(str);

                    this->JoinErrorSend(client, 2, "Restricted ip.");
                    return;
                }
            }
        }
    }

    uint8_t cd_flag = params & 0xff;
    uint8_t some_id = (params >> 8) & 0xff;
    uint8_t strong = (params >> 16) & 0xff;
    uint8_t weak = (params >> 24) & 0xff;

    bool player_not_exist = false;

    if (cd_flag != 0 && cd_flag != g_IsCdPresent) {
        this->JoinErrorSend(client, 8, "Demo & Release are incompatible");
        return;
    }

    Player* player = nullptr;

    for (POSITION pos = g_PlayersList->list.GetHeadPosition(); pos != nullptr;) {
        Player* pl = g_PlayersList->list.GetNext(pos);
        if (pl->name == name) {
            if ((pl->hat_player_id != id || pl->flags != arg || g_NetStru1_main.GetClientByPlayerID(pl->player_id) != nullptr) 
                && this->field4_0x74 != 0) {
                LogMessage("Warning - other player with same name joined");

                this->JoinErrorSend(client, 3, "This name already used. Try different.");
                return;
            }
            
            player = pl;
            break;
        }
    }

    if (player) {
        player_not_exist = false;

        if (this->MapLevel != -1 && g_CLlDriver.provider != 4) {
            if (strong > this->MapLevel) {
                this->JoinErrorSend(client, 14, "Character is too strong for this map.");

                CString str;
                str.Format("Player %s rejected(too strong for this map)", name);
                LogMessage(str);                
                return;
            }
            if (weak < this->MapLevel) {
                this->JoinErrorSend(client, 15, "Character is too weak for this map.");

                CString str;
                str.Format("Player %s rejected (too weak for this map)", name);
                LogMessage(str);
                return;
            }
        }

        CString str;
        str.Format("Player %s returns to game.", player->name);
        LogMessage(str);

        player->drop_deadline_tick16 = 0;
    } else {
        player_not_exist = true;

        if (this->MapLevel != -1) {
            if (strong > this->MapLevel) {
                this->JoinErrorSend(client, 14, "Character is too strong for this map.");

                CString str;
                str.Format("Player %s rejected(too strong for this map)", name);
                LogMessage(str);
                return;
            }
            if (weak < this->MapLevel) {
                this->JoinErrorSend(client, 15, "Character is too weak for this map.");

                CString str;
                str.Format("Player %s rejected (too weak for this map)", name);
                LogMessage(str);
                return;
            }
        }

        if (cd_flag == 0) {
            int cds = g_PlayersList->CountCD();
            if (this->field3_0x70 == 0) {
                cds++;
            }

            int humans = g_PlayersList->CountHumanPlayers();
            if (humans > 15) {
                humans = 15;
            }

            static const int neededCD[16] = 
            {1, 1, 1, 2,
             2, 2, 3, 3,
             3, 3, 4, 4,
             4, 4, 4, 4};

            if (cds < neededCD[humans]) {
                this->JoinErrorSend(client, 6, "too few CDs");
                return;
            }
        }

        player = this->Allods2_CreatePlayer(name);
        player->hat_player_id = id;
        player->flags = arg;
        player->has_cd = cd_flag;
    }

    client->player_id = player->player_id;

    player->is_logged_in = 1;

    if (some_id == 0 && player_not_exist) {
        for (int i = 0; i < 16; i++) {
            bool used = false;

            for (POSITION pos = g_PlayersList->list.GetHeadPosition(); pos != nullptr;) {
                Player* pl = g_PlayersList->list.GetNext(pos);
                if (pl->color_slot == i && pl->is_ai == 0) {
                    used = true;
                    break;
                }
            }

            if (!used) {
                some_id = i;
                break;
            }
        }
    }

    if (some_id != 0) {
        player->color_slot = some_id;
    }

    PacketJoin& pkt = PacketJoin::Inst;
    strcpy(pkt.name, player->name);
    pkt.id = 0x96;
    pkt.to_player_id = 0;
    pkt.player_id = player->player_id;
    pkt.token_id = player->token_id;
    pkt.color = player->color_slot - 1;
    pkt.flags = (player_not_exist ? 2 : 0) | (player->is_ai != 0 ? 1 : 0);

    g_NetStru1_main.QueuePacketSend(&pkt);

    LogMessage("Player " + name + " joined.");

    if (player_not_exist) {
        g_NetStru1_main.FUN_0051ce86(3, player->player_id, nullptr);
    } else {
        g_NetStru1_main.FUN_0051ce86(4, player->player_id, nullptr);
    }

    player->AddMoney(0, 1);

    g_NetStru1_main.SendExistingPlayers(player);

    if (this->field18_0x94 != 0) {
        g_NetStru1_main.SendBarePacket(0xb7, player);
    }
    
    g_NetStru1_main.SendTickPacket(nullptr);

    if (this->field3_0x70 != 0) {
        g_NetStru1_main.SendAllData();
    }
}

void Server::JoinErrorSend(NetStru2* client, int32_t arg, const CString& str) {
    //4fc4f3
    g_NetStru1_main.SendAllData();
    Player pl;
    pl.player_id = 9999;
    client->player_id = 9999;
    if (!str.IsEmpty()) {
        g_NetStru1_main.AnnouncePlayerName(str, &pl);
    }
    g_NetStru1_main.SendInfoPacket(0xb, arg, 0, &pl);
    g_NetStru1_main.SendTickPacket(&pl);
    g_NetStru1_main.SendAllData();
    pl.player_id = 0;
}


Player* Server::Allods2_CreatePlayer(CString name) {
    Player* player = nullptr;
    if (this->field4_0x74 == 0) {
        player = g_PlayersList->FindByName("Self");
    }

    bool created = false;
    if (!player) {
        player = new Player();
        created = true;
    }

    player->name = name;
    player->is_ai = 0;
    if (player->money == 0) {
        player->money = 1000;
    }

    if (created) {
        g_PlayersList->AddPlayer(player);
    }

    return player;
}

// 4f0bef
int32_t Server::ConnectToHat() {
    if (g_CLlDriver.provider != 4) {
        return 0;
    }

    if (g_HatLLDriver.listen_socket.is_in_use == 1) {
        return 1;
    }

    CLlAddress addr;
    if (g_ServerConfig.ip_address2.IsEmpty()) {
        strcpy(addr.address, g_ServerConfig.ip_address);
    } else {
        strcpy(addr.address, g_ServerConfig.ip_address2);
    }

    LogMessage("Connecting to hat...");

    int result = g_HatLLDriver.PrepareForConnect(g_ServerConfig.hat_address, &addr);
    if (result != 0) {
        result = g_HatLLDriver.Connect(g_ServerConfig.server_name, nullptr);
    }

    if (result == 0) {
        LogMessage("Connection to hat failed.");
        return 0;
    }

    LogMessage("Connected. Logging in...");
    NetStru1::HatConnector.ProcessConnections();
    NetStru1::HatConnector.SendHatLogin(g_ServerConfig.ip_address);
    return 1;
}

// 4f0ecf
void Server::HatKeepaliveTick() {
    if (this->field51_0x1d8 != 0 || this->field50_0x1d4 != 0) {
        return;
    }

    DWORD tick_count = GetTickCount();

    if (tick_count - this->field64_0x21c > 60000) {
        this->field64_0x21c = tick_count;

        if (g_PlayersList->CountHumanPlayers() == 0 || g_ServerConfig.gameType != 0) {
            NetStru1::HatConnector.sub_51E1C7(0);
        } else {
            Player* player = nullptr;
            do {
                this->field65_0x220 += 1;
                if (this->field65_0x220 > 31) {
                    this->field65_0x220 = 16;
                }
                player = g_PlayersList->FindById(this->field65_0x220);
            } while (player == nullptr);

            CString login(player->login);
            CFile file;
            CString filename = g_ServerConfig.chr_base + login;

            if (file.Open(filename, CFile::modeRead, nullptr)) {
                FileSectionBasicInfo* basic_info = nullptr;
                FileSectionStats* stats = nullptr;
                uint8_t* kill_stats = nullptr;
                PacketUnitStateVec* equip_pkt = nullptr;
                PacketUnitStateVec* inv_pkt = nullptr;
                uint8_t* param7 = nullptr;
                uint32_t param8 = 0;

                ParsePlayerFile_4F62E6(&file, &basic_info, &stats, &kill_stats, &equip_pkt, &inv_pkt, &param7, &param8);
                if (basic_info == nullptr) {
                    LogMessage("Error in character data file. Login: " + login);
                    this->FileList.RemoveAt(0, 1);
                    return;
                }

                file.Seek(0, CFile::begin);

                size_t file_size = file.GetLength();
                void* file_data = malloc(file_size);
                file.Read(file_data, file_size);

                NetStru1::HatConnector.SendHatMessage(basic_info->id1, basic_info->id2, login, file_data, file_size, 0);
                LogMessage("Updating character data file for " + login);
                file.Close();
            }
        }
    }

    if (this->FileList.GetSize() == 0) {
        return;
    }

    if (this->field53_0x1f0 != 0 && tick_count - this->field53_0x1f0 <= 15000) {
        return;
    }

    CString login(this->FileList.GetAt(0));
    CFile file;
    CString filename = g_ServerConfig.chr_base + login;

    if (file.Open(filename, CFile::modeRead, nullptr)) {
        FileSectionBasicInfo* basic_info = nullptr;
        FileSectionStats* stats = nullptr;
        uint8_t* kill_stats = nullptr;
        PacketUnitStateVec* equip_pkt = nullptr;
        PacketUnitStateVec* inv_pkt = nullptr;
        uint8_t* param7 = nullptr;
        uint32_t param8 = 0;

        ParsePlayerFile_4F62E6(&file, &basic_info, &stats, &kill_stats, &equip_pkt, &inv_pkt, &param7, &param8);
        if (basic_info == nullptr) {
            LogMessage("Error in character data file. Login: " + login);
            this->FileList.RemoveAt(0, 1);
            return;
        }

        file.Seek(0, CFile::begin);

        size_t file_size = file.GetLength();
        void* file_data = malloc(file_size);
        file.Read(file_data, file_size);

        NetStru1::HatConnector.SendHatCharData(basic_info->id1, basic_info->id2, login, file_data, file_size, 0);
        LogMessage("Returning character data file for " + login);
        this->field53_0x1f0 = tick_count;
        file.Close();
    }
}

int Server::Start(int mode) {
    //4f06f5
    g_buildingIdSet.Clear();

#ifdef A2CLIENT
    char tmpbuf[256];
    GetProfileStringA("Allods Server", "BaseDir", "", tmpbuf, 255);
    this->field5_0x78 = tmpbuf;
#endif

    this->field4_0x74 = mode < 2;
    this->field3_0x70 = mode > 0;

    if (this->field4_0x74 == 0) {
        this->field39_0x1a8 = 0;
    } else {
        this->field39_0x1a8 = 1;
    }

    this->field16_0x8c = 0;

    g_Server = this;

    g_NetStru1_main.SetLLDriver(&g_CLlDriver);

#ifndef A2CLIENT
    if (mode == 0) {
        NetStru1::HatConnector.SetLLDriver(&g_HatLLDriver);
        g_HatLLDriver.hl_driver = &NetStru1::HatConnector;
        g_HatLLDriver.ResetProvider(4);
    }
#endif

    if (g_GameDataRes.ParseWorldIn("World\\Data\\") != 0) {
        return 2;
    }

#ifndef A2CLIENT
    if (g_GameDataRes.magics[3].Values()[0].mana_cost < 200) {
        return 2;
    }
#endif

    this->spells[0] = nullptr;

    for (int i = 1; i < 30; i++) {
        this->spells[i] = new Spell(i);
    }

    dword_6CDB3C = new UnitList();
    dword_6B37C4 = new UnitList();

    this->srv_stru1->units_list = new UnitList();

    g_PlayersList = new PlayersList();

    for (int j = 0; j < 32; j++) {
        for (int i = 0; i < 32; i++) {
            somewords[j][i] = 0;
        }
    }

#ifndef A2CLIENT
    this->FileList.RemoveAll();

    CFileFind fnd;
    bool res = fnd.FindFile(g_ServerConfig.chr_base + "*.");
    while (res) {
        res = fnd.FindNextFileA();
        if (!fnd.IsDirectory()) {
            this->FileList.Add(fnd.GetFileName());
        }
    }

    if (this->FileList.GetSize()) {
        CString str;
        str.Format("%d unsent characters found. Ordered to return to hat.", this->FileList.GetSize());
        LogMessage(str);
    }
#endif

    return 0;
}

// Save a player's character data to file.
// 4EE028
void Server::SaveCharacter(Humanoid* humanoid) {
    Player* player = humanoid->pOwner;
    player->last_save_tick = GetTickCount();

    PacketUnitStateVec equip_pkt;
    PacketUnitStateVec inv_pkt;

    // Gather type/attr flags
    uint8_t flags = 0;
    if (humanoid->typeId == 0x22 || humanoid->typeId == 0x24) {
        flags |= 0x80;
    }
    if (humanoid->unit_attrs & 4) {
        flags |= 0x40;
    }

    // Build basic info block
    FileSectionBasicInfo basic_info = {};
    basic_info.id1 = player->hat_player_id;
    basic_info.id2 = player->flags;
    basic_info.hat_id = this->field54_0x1f4;
    strcpy(basic_info.nick, (const char*)player->name);
    basic_info.character_class = flags;
    basic_info.picture = humanoid->face;
    basic_info.main_sphere = humanoid->main_sphere;
    basic_info.flags = 4;
    basic_info.color = player->color_slot;

    // Build stats block
    FileSectionStats stats = {};
    stats.monster_kills = player->monster_kills;
    stats.player_kills = player->player_kills;
    stats.frags = player->frags;
    stats.deaths = player->deaths;
    stats.money = player->money;
    stats.body = humanoid->body - humanoid->equipment_extra.body;
    stats.reaction = humanoid->reaction - humanoid->equipment_extra.reaction;
    stats.mind = humanoid->mind - humanoid->equipment_extra.mind;
    stats.spirit = humanoid->spirit - humanoid->equipment_extra.spirit;

    if (humanoid->spell_book) {
        stats.spells = humanoid->spell_book->CalcSpellMask();
    } else {
        stats.spells = 0;
    }

    if (humanoid->eye2) {
        stats.active_spell = humanoid->eye2->spell_id;
    } else {
        stats.active_spell = 0;
    }

    for (int i = 0; i < 5; i++) {
        stats.experience[i] = humanoid->experience_per_sphere[i];
    }

    // Serialize weapon
    if (humanoid->weapon) {
        humanoid->weapon->StoreToPacket(&equip_pkt, 0);
    } else {
        Item blank;
        blank.StoreToPacket(&equip_pkt, 0);
    }

    // Serialize shield
    if (humanoid->shield) {
        humanoid->shield->StoreToPacket(&equip_pkt, 0);
    } else {
        Item blank;
        blank.StoreToPacket(&equip_pkt, 0);
    }

    // Serialize equipment slots 3..12
    for (int i = 3; i <= 12; i++) {
        if (humanoid->equipment[i]) {
            humanoid->equipment[i]->StoreToPacket(&equip_pkt, 0);
        } else {
            Item blank;
            blank.StoreToPacket(&equip_pkt, 0);
        }
    }

    // Serialize inventory items
    POSITION pos = humanoid->inventory->items.GetHeadPosition();
    while (pos) {
        Item* item = humanoid->inventory->items.GetNext(pos);
        item->StoreToPacket(&inv_pkt, 0);
    }

    // Format filename and save
    CString filename;
    filename.Format("%s%s", (const char*)g_ServerConfig.chr_base, (const char*)player->login);

    WritePlayerFile_4F53EA((const char*)filename, &basic_info, &stats, player->kill_stats, &equip_pkt, &inv_pkt, nullptr, 0);
}

// Load map (or a saved game?)
// 4F1471
int Server::LoadGame(CString param_1) {
    this->field16_0x8c = 0;
    this->field59_0x208 = 0;
    this->current_map_name = param_1;

    SrandInit();

    CString map_code;

    if (this->field21_0xd4 == 0) {
        CString lower = this->current_map_name;
        lower.MakeLower();
        int ext = lower.Find(".alm");
        if (ext == -1) {
            return 3;
        }
        map_code = lower.Left(ext);
        int code = atoi(map_code);
        if (code > 0) {
            this->field21_0xd4 = code;
        }
    } else {
        // I don't get this branch. Looks useless?
        CString tmp;
        IntToCString(&tmp, this->field21_0xd4);
        map_code = tmp;
    }

    this->field19_0x98.field2_0x20 = this->field21_0xd4;

    this->srv_stru1->building_list = new BuildingsList();
    this->srv_stru1->effects_list = new SpellEffectList();
    this->srv_stru1->sack_list = new SackList();

    if (this->field40_0x1ac == 0) {
        if (this->field19_0x98.LoadMap(this->current_map_name) != 0) {
            return 5;
        }
        this->tick16 = 0;
        this->tick = 0;
    } else {
        if (this->LoadSavedGame("game0000.sav") != 0) {
            return 4;
        }
    }

    if (this->field4_0x74 != 0) {
        dword_6CDB38 = CalcFileChecksum(this->current_map_name);
    }

    this->field18_0x94 = 1;
    g_NetStru1_main.SendBarePacket(0xB7, nullptr);

    this->field42_0x1b4 = 0;
    this->field43_0x1b8 = 0;
    this->field44_0x1bc = 0;
    this->tic16 = 0;
    this->field46_0x1c4 = 0;
    this->map_elapsed_time = 0;
    this->map_elapsed_time2 = 0;

    this->UpdateHatAnnounce();

    LogMessage("Loaded map \"" + param_1 + "\"");

    CStdioFile file;
    CString info_map = g_ServerConfig.chr_base + "info.map";
    if (file.Open(info_map, CFile::modeCreate | CFile::modeWrite)) {
        CString content;
        content.Format("%s\n%d\n", (LPCTSTR)param_1, dword_6CDB38);
        file.WriteString(content);
        file.Close();
    }

    for (int j = 0; j < 32; j++) {
        for (int i = 0; i < 32; i++) {
            Server::somewords[j][i] = 0;
        }
    }

    g_QuestMap.RemovePlayerQuests(0);

    if (this->field19_0x98.field4_0x28.GetSize() < 3 && g_ServerConfig.gameType == 3) {
        BuildingsList* building_list = this->srv_stru1->building_list;
        POSITION pos = building_list->GetHeadPosition();
        while (pos != nullptr) {
            Building* building = building_list->GetNext(pos);
            if (building->IsKindOf(RUNTIME_CLASS(Inn)) || building->IsKindOf(RUNTIME_CLASS(Shop))) {
                uint8_t x = building->position->GetX();
                uint8_t y = building->position->GetY();
                uint16_t cell = (x - 1) | ((y - 1) << 8);
                this->field19_0x98.field4_0x28.SetAtGrow(this->field19_0x98.field4_0x28.GetSize(), cell);
            }
        }
    }

    if (g_ServerConfig.gameType == 3) {
        this->SetupArenaFileList();
    }

    return 0;
}

// 4FBB79
void Server::ServerTic() {
    if (this->field18_0x94 == 0) {
        return;
    }

    uint32_t tick_count = GetTickCount();
    static uint32_t tick_after_5min = 0;

    if (tick_count - tick_after_5min > 300000) {
        tick_after_5min = tick_count;
        this->UpdateHatAnnounce();
    }

    if ((this->tick & 0xf) == 6) {
        g_World->ProcessTick(g_PlayersList);
    }

    if ((this->tick & 0xf) == 10) {
        this->ProcessHatConnections();
        this->HatKeepaliveTick();
    }

    if (g_ServerConfig.gameType == 2) {
        int teams_ready = g_PlayersList->AreTeamsStarted();
        if ((this->tick & 0xff) == 0 && teams_ready == 0) {
            POSITION ppos = g_PlayersList->list.GetHeadPosition();
            while (ppos != nullptr) {
                Player* player = g_PlayersList->list.GetNext(ppos);
                if (player->is_ai == 0 && player->is_in_world != 0 && player->team_ready == 0) {
                    g_NetStru1_main.FUN_0051ce86(0xb, 0, player);
                }
            }
        }

        if (teams_ready != 0) {
            for (int i = 0; i < 2; i++) {
                if (this->ctf_dropped[i] > 0) {
                    this->ctf_dropped[i]++;
                }
                if (this->ctf_dropped[i] > 320) {
                    this->UpdateQuestRune(i, 1);
                    g_NetStru1_main.FUN_0051ce86(i + 0x100, 0, nullptr);
                }
            }

            for (int i = 0; i < 2; i++) {
                if (this->ctf_carrying[i] != 0) {
                    Player* player = g_PlayersList->FindById(this->ctf_carrying[i]);
                    if (player == nullptr || player->main_unit == nullptr) {
                        continue;
                    }

                    uint16_t yx = this->field19_0x98.field4_0x28.GetAt(1 - i);
                    uint8_t unit_x = player->main_unit->position->GetX();
                    if (abs((yx & 0xff) - unit_x) < 3) {
                        uint8_t unit_y = player->main_unit->position->GetY();
                        if (abs((yx >> 8) - unit_y) < 3) {
                            g_NetStru1_main.FUN_0051ce86(i + 0x106, this->ctf_carrying[i], nullptr);
                            this->UpdateQuestRune(i, 0);

                            POSITION ppos = g_PlayersList->list.GetHeadPosition();
                            while (ppos != nullptr) {
                                Player* p = g_PlayersList->list.GetNext(ppos);
                                if (p->is_ai == 0 && p->field_0xa70 == 1 - i) {
                                    p->frags += g_ServerConfig.flag_score;
                                    this->ctf_score[1-i] += g_ServerConfig.flag_score;
                                }
                            }
                            g_NetStru1_main.SendTeamState(0);
                        }
                    }
                }
            }
        }
    }

    if ((this->tick & 0xf) == 12) {
        dword_6CDB3C->RegenTick();
        this->srv_stru1->UpdateMapObjects();
        g_PlayersList->ProcessPlayerTimeouts();
        g_NetStru1_main.UpdateConnStats();
        g_QuestMap.ProcessDeferredEvents();

        POSITION pos = g_QuestMap.flags.GetStartPosition();
        while (pos != nullptr) {
            uint32_t k, v;
            g_QuestMap.flags.GetNextAssoc(pos, k, v);
            Player* p = g_PlayersList->FindById(k);
            if (v != 0 && p != nullptr) {
                g_NetStru1_main.SendSrvStru1State(&g_QuestMap, p, 0);
                if ((v & 0x10) != 0) {
                    g_NetStru1_main.FUN_0051ce86(0x10, 0, p);
                }
                if ((v & 0x20) != 0) {
                    g_NetStru1_main.FUN_0051ce86(0x20, 0, p);
                }
                if ((v & 0x40) != 0) {
                    g_NetStru1_main.FUN_0051ce86(0x40, 0, p);
                }
                if ((v & 0x80) != 0) {
                    g_NetStru1_main.FUN_0051ce86(0x80, 0, p);
                }
            }
        }
        g_QuestMap.flags.RemoveAll();
    }

    this->Tick();
    if ((this->tick & 0xf) == 15) {
        this->sub_50979A();
        this->tick16++;
    }

    uint32_t dt = GetTickCount();
    this->field43_0x1b8 += (dt - tick_count);
    if ((this->tick & 0xf) == 15) {
        this->field42_0x1b4 = this->field43_0x1b8;
        this->field44_0x1bc += this->field43_0x1b8;
        this->tic16++;
        if (this->field46_0x1c4 < this->field43_0x1b8) {
            this->field46_0x1c4 = this->field43_0x1b8;
        }
        this->field43_0x1b8 = 0;
    }
}



void Server::ProcessAvailPackets() {
    //509010
    while (true) {
        Packet* pkt = g_NetStru1_main.ReceiveAnyPacket();
        if (!pkt) {
            break;
        }

        this->sub_504a96(pkt);
    }
}

void Server::PumpNetwork() {
    //50907e
    this->ProcessAvailPackets();
    g_NetStru1_main.SendTickPacket(nullptr);
    g_NetStru1_main.SendAllData();
}

Server::Server()
: field5_0x78(""), srv_stru1(&SrvStru1::Instance) { //4ece8b
    g_World = nullptr;
    dword_6CDB3C = nullptr;
    dword_6B37C4 = nullptr;
    g_PlayersList = nullptr;
    MapStuff_Instance = nullptr;

    this->field19_0x98.srv_stru = this->srv_stru1;

    this->field18_0x94 = 0;
    this->field3_0x70 = 0;
    this->field4_0x74 = 0;
    this->field15_0x88 = nullptr;

    this->current_map_name = "1.alm";

    this->tick = 0;
    this->tick16 = 0;

    this->script_settings = new ScriptSettings();
    this->field26_0x174 = 0;
    this->field27_0x178 = 0;
    this->field28_0x17c = 0;
    this->field29_0x180 = 0;
    this->field30_0x184 = 0;
    this->field31_0x188 = 0;
    this->field32_0x18c = 0;
    this->field33_0x190 = 0;
    this->field34_0x194 = 0;
    this->field37_0x1a0 = 0;
    this->field36_0x19c = 0;
    this->field38_0x1a4 = 1;
    this->field39_0x1a8 = 1;
    this->field40_0x1ac = 0;
    this->field35_0x198 = 0;
    this->field21_0xd4 = 0;
    this->field22_0xd8 = 2;
    this->field41_0x1b0 = 0;

    SrandInit();

    this->spells.fill(nullptr);

    this->MapLevel = -1;
    this->field49_0x1d0 = 0;
    this->field50_0x1d4 = 0;
    this->field51_0x1d8 = 1;
    this->field53_0x1f0 = 0;
    this->field64_0x21c = 0;
    this->field65_0x220 = 15;
    this->field54_0x1f4 = 0;
    this->field55_0x1f8 = 0;
    this->field56_0x1fc = 0;
    this->field59_0x208 = 0;
    this->ctf_score[0] = 0;
    this->ctf_score[1] = 0;
    this->ctf_carrying[0] = 0;
    this->ctf_carrying[1] = 0;
    this->ctf_dropped[0] = 0;
    this->ctf_dropped[1] = 0;
    this->field66_0x224 = 0;
    g_ShutdownIn = 0x7fffffff;
    this->map_elapsed_time = 0;
    this->map_elapsed_time2 = 0;
}

void Server::FUN_00501b9e(int32_t var300, CDWordArray& arr) { //501b9e
    UnitList ulst;
    for (int i = 0; i < arr.GetSize(); i++) {
        uint32_t serverID = arr[i] & 0xffff;
        bool bVar7 = true;

        if (serverID < 21 || serverID > 30) {
            bVar7 = false;
        }

        bool bVar2 = false;
        int32_t idx = g_GameDataRes.FindHumanByServerId(serverID * 10 + 10000);
        if (idx == 0) {
            idx = g_GameDataRes.FindMonsterByServerId(serverID * 10 + 10000);
            if (idx) {
                bVar2 = true;
            }
        }

        if (idx == 0) {
            idx = g_GameDataRes.FindHumanByServerId(serverID);
            if (idx == 0) {
                idx = g_GameDataRes.FindMonsterByServerId(serverID);
                bVar2 = true;
            }
        } else {
            for (int i = 1; i < 10; i++) {
                int32_t idd = 0;
                if (bVar2) {
                    idd = g_GameDataRes.FindMonsterByServerId(serverID * 10 + 10000 + i);
                } else {
                    idd = g_GameDataRes.FindHumanByServerId(serverID * 10 + 10000 + i);
                }

                if (idd != 0) {
                    if (bVar2) {
                        idd = g_GameDataRes.FindMonsterByServerId(serverID * 10 + 9998 + var300 / 10);
                    } else {
                        idd = g_GameDataRes.FindHumanByServerId(serverID * 10 + 9998 + var300 / 10);
                    }

                    if (idd != 0) {
                        idx = idd;
                    }
                    break;
                }
            }
        }

        if (bVar7) {
            bool has = false;
            Player* pl = g_PlayersList->list.GetHead();
            for (POSITION it = pl->unit_list->unit_list.GetHeadPosition(); it != nullptr;) {
                Unit* u = pl->unit_list->unit_list.GetNext(it);
                if (u->server_id == serverID) {
                    has = true;
                    break;
                }
            }

            if (!has) {
                Human* hm = new Human(g_GameDataRes.humans[idx].name, 1, nullptr);
                hm->server_id = serverID;
                hm->building_id = g_buildingIdSet.AllocBit();
                hm->pOwner = pl;
                pl->unit_list->AddTail(hm);

                Group* grp = new Group();
                pl->group_list->groups.AddTail(grp);
                grp->AddUnit(hm);

                g_World->ResetUnitToHold(hm);
                g_World->ProcessAnimateDeadGroup(grp);
                g_NetStru1_main.SendUnitUpdate(hm, pl, (uint32_t)-1, 0xffb, 0, 0);
            }
        } else {
            Player* pl = g_PlayersList->list.GetHead();
            Unit* u;
            if (bVar2) {
                u = new Unit(g_GameDataRes.monsters[idx].name);
            } else {
                u = new Human(g_GameDataRes.humans[idx].name, 0, nullptr);
            }
            u->server_id = serverID;
            u->pOwner = pl;
            ulst.AddTailAllocId(u);
            g_NetStru1_main.SendUnitUpdate(u, pl, (uint32_t)-1, 0xffb, 0, 0);
        }
    }

    for (POSITION it = ulst.unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* u = ulst.unit_list.GetNext(it);
        g_buildingIdSet.Unset(u->building_id);
    }
    for (POSITION it = ulst.unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* u = ulst.unit_list.GetNext(it);
        delete u;
    }
    ulst.unit_list.RemoveAll();

    g_NetStru1_main.SendTickPacket(nullptr);
    g_NetStru1_main.SendAllData();
}

// 4F0D58
void Server::ProcessHatConnections() {
    NetStru1::HatConnector.ProcessConnections();
    if (g_HatLLDriver.IsListen()) {
        if (this->field50_0x1d4 != 0) {
            NetStru2* first_client;
            if (NetStru1::HatConnector.active_connects.IsEmpty()) {
                first_client = nullptr;
            } else {
                first_client = NetStru1::HatConnector.active_connects.GetHead();
            }
            if ((first_client == nullptr || first_client->is_claimed == 0) && GetTickCount() - this->field50_0x1d4 > 30000) {
                g_HatLLDriver.Close();
                NetStru1::HatConnector.ProcessConnections();
                this->field51_0x1d8 = GetTickCount();
                this->field50_0x1d4 = 0;
                LogMessage(CString("Login to hat failed. Hat does not respond."));
            }
        }
        if (GetTickCount() - this->field55_0x1f8 > 15000) {
            NetStru1::HatConnector.SendPacket_64(1, 0);
            this->field55_0x1f8 = GetTickCount();
        }
    } else {
        if (this->field51_0x1d8 != 0 && GetTickCount() - this->field51_0x1d8 > g_ServerConfig.reconnect_delay * 60000) {
            if (!this->ConnectToHat()) {
                this->field51_0x1d8 = GetTickCount();
            } else {
                this->field51_0x1d8 = 0;
                this->field50_0x1d4 = GetTickCount();
            }
        }
    }
}

// 5013D4
void Server::EnsurePlayerGroup(Player* player) {
    Group* group;
    if (player->group_list->groups.GetCount() == 0) {
        group = nullptr;
    } else {
        group = player->group_list->groups.GetHead();
    }
    if (group == nullptr) {
        // Original builds a discarded "Oops - player has no groups" CString (no logging).
        group = new Group();
        player->group_list->groups.AddTail(group);
    }

    while (group->unit_list.GetCount() == 0 && player->group_list->groups.GetCount() > 1) {
        POSITION node = player->group_list->groups.Find(group);
        if (node != nullptr) {
            player->group_list->groups.RemoveAt(node);
        }
        delete group;
        if (player->group_list->groups.GetCount() == 0) {
            group = nullptr;
        } else {
            group = player->group_list->groups.GetHead();
        }
    }

    uint8_t x = 0;
    uint8_t y = 0;
    if (this->field4_0x74 != 0 && player->building_entered_from_yx != 0 && g_ServerConfig.gameType == 0) {
        x = player->building_entered_from_yx;
        y = player->building_entered_from_yx >> 8;
    } else {
        CArray<uint16_t>& drop_locations = this->field19_0x98.field4_0x28;
        if (drop_locations.GetSize() > 0) {
            int32_t index = Random0N(drop_locations.GetSize() - 1);
            if (g_ServerConfig.gameType == 2) {
                index = player->field_0xa70;
            }
            x = drop_locations[index];
            y = drop_locations[index] >> 8;
        }
    }

    if (x * y == 0) {
        x = Random0N(70) + 30;
        y = Random0N(70) + 30;
        LogMessage(CString("Warning - no drop location in .alm - random used"));
    }

    double sqrt_count = std::sqrt((double)player->unit_list->unit_list.GetCount());
    double radius_f;
    if (sqrt_count + 4.0 < 5.0) {
        radius_f = 5.0;
    } else {
        radius_f = sqrt_count + 4.0;
    }
    int32_t radius = (int32_t)radius_f;

    POSITION unit_it = player->unit_list->unit_list.GetHeadPosition();
    if (unit_it == nullptr) {
        return;
    }

    Unit* unit = player->unit_list->unit_list.GetNext(unit_it);
    while (unit != nullptr) {
        uint16_t yx = unit->position->CompatGetYX();
        if (MapStuff_Instance->GetSmallUnitAt(yx) != unit && MapStuff_Instance->GetLargeUnitAt(yx) != unit) {
            *unit->position = TokenPos(x, y, MapStuff_Instance);

            int32_t placed = 0;
            if (unit == player->main_unit) {
                placed = unit->PlaceOnMap(x, y, 0);
            }
            if (placed == 0) {
                placed = unit->PlaceOnMap(x, y, radius);
            }
            if (placed == 0) {
                LogMessage(CString("Error - can't place hero from previous mission on map."));
            }

            dword_6CDB3C->AddTail(unit);

            delete unit->eye;
            unit->eye = new UnitEye();

            uint8_t spell_id = unit->eye2->spell_id;
            delete unit->eye2;
            unit->eye2 = new UnitEye2();
            unit->eye2->spell_id = spell_id;

            g_World->ProcessAnimateDeadGroup(unit->group);
        }
        unit = unit_it ? player->unit_list->unit_list.GetNext(unit_it) : nullptr;
    }
}

// 4F4570
void Server::UpdateHatAnnounce() {
    if (g_CLlDriver.provider != 4) {
        return;
    }
    int32_t map_range = 0;
    if (MapStuff_Instance != nullptr) {
        map_range = MapStuff_Instance->GetWidth() - 16;
    }
    if (g_HatLLDriver.IsListen()) {
        NetStru1::HatConnector.SendHatLobbyInfo(g_PlayersList->CountHumanPlayers(), this->current_map_title, this->MapLevel, g_ServerConfig.gameType, map_range);
    }
    if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
        g_NetStru1_main.SendTeamState(0);
    }
}

// 4F8831
void Server::KickAllPlayers() {
    for (POSITION it = g_PlayersList->list.GetHeadPosition(); it != nullptr;) {
        Player* player = g_PlayersList->list.GetNext(it);
        if (!player->is_ai) {
            NetStru2* client = g_NetStru1_main.GetClientByPlayerID(player->player_id);
            if (client != nullptr) {
                g_NetStru1_main.DisconnectClient(client);
            }
            player->drop_deadline_tick16 = this->tick16 - 1;
        }
    }
    g_NetStru1_main.ProcessConnections();
    g_PlayersList->ProcessPlayerTimeouts();
}

// 4F8F86
void Server::OnTeamsReady() {
    this->ctf_score[0] = 0;
    this->ctf_score[1] = 0;
    if (g_PlayersList != nullptr) {
        g_PlayersList->ResetFrags();
    }
}

// 4F9AD3
void Server::PrepareSackPickup(Sack* sack) {
    if (g_ServerConfig.gameType != 0) {
        return;
    }
    if (sack == nullptr || sack->position == nullptr) {
        return;
    }
    CString filename;
    filename.Format("%s%03d %03d .sck", (LPCTSTR)g_ServerConfig.chr_base, sack->position->GetX(), sack->position->GetY());

#ifdef A2SERVER_PATCH
    try {
#endif
        CFile::Remove(filename);
#ifdef A2SERVER_PATCH
    } catch (CFileException* e) {
        e->Delete();
    }
#endif
}

// 4F9B9E
void Server::SaveSack(Sack* sack) {
    if (g_ServerConfig.gameType != 0) {
        return;
    }
    if (sack == nullptr || sack->position == nullptr) {
        return;
    }

    CFile file;
    PacketUnitStateVec packet;
    for (POSITION pos = sack->inventory->items.GetHeadPosition(); pos != nullptr;) {
        Item* item = sack->inventory->items.GetNext(pos);
        item->StoreToPacket(&packet, 0);
        packet.entry_count++;
    }
    CString filename;
    filename.Format("%s%03d %03d .sck", (LPCTSTR)g_ServerConfig.chr_base, sack->position->GetX(), sack->position->GetY());

    if (!file.Open(filename, CFile::modeWrite | CFile::modeCreate)) {
        LogMessage("Error saving sack file " + filename);
    } else {
        file.Write(&sack->money, 4);
        file.Write(&packet.building_id, packet.data_size + 9);
        file.Close();
    }
}

// 4F950F
void Server::LoadSavedSacks() {
    if (g_ServerConfig.gameType != 0 || this->field66_0x224 != 0) {
        return;
    }

    CStdioFile info_file;
    CString info_map_path = g_ServerConfig.chr_base + "info.map";
    if (!info_file.Open(info_map_path, CFile::modeRead)) {
        LogMessage("Map info file not found, sacks ignored");
        return;
    }

    CString saved_map_name;
    CString saved_checksum_str;
    info_file.ReadString(saved_map_name);
    info_file.ReadString(saved_checksum_str);
    info_file.Close();

    int32_t saved_checksum = atoi(saved_checksum_str);
    if (saved_checksum != dword_6CDB38 || saved_map_name != this->current_map_name) {
        LogMessage("Server restarted with new map, sacks ignored");
        return;
    }

    CFileFind fnd;
    BOOL found = fnd.FindFile(g_ServerConfig.chr_base + "*.sck");
    while (found) {
        found = fnd.FindNextFileA();
        if (fnd.IsDirectory()) {
            continue;
        }

        CString file_path = fnd.GetFilePath();
        CFile file;
        if (!file.Open(file_path, CFile::modeRead)) {
            LogMessage("Error opening sack file " + file_path);
            continue;
        }

        int32_t money = 0;
        file.Read(&money, 4);

        PacketUnitStateVec packet;
        file.Read(&packet.building_id, file.GetLength() - 4);
        file.Close();

        Inventory* inventory = new Inventory();

        uint8_t* p = packet.data;
        while (p < packet.data + packet.data_size) {
            Item* item = ReadItem(&p);
            if (item->item_id == 0) {
                delete item;
            } else {
                inventory->PutItemIntoBagAtDefault(item);
            }
        }

        CString file_name = fnd.GetFileName();
        int32_t x = atoi(file_name);
        int32_t y = atoi(file_name.Mid(4));

        TokenPos pos(x, y, MapStuff_Instance);
        this->srv_stru1->sack_list->DumpInventoryAt(&pos, inventory, money, 1);
    }

    this->field66_0x224 = 1;
}

// 4FA348
void Server::AddArenaFrags(CString* name, int32_t flag) {
    for (int32_t i = 0; i < this->field67_0x228.GetSize(); i++) {
        if (this->field67_0x228[i] == *name) {
            this->field68_0x23c[i] += flag;
            g_NetStru1_main.SendArenaScore(this->field67_0x228[i], this->field68_0x23c[i], '[', nullptr);
            return;
        }
    }

    int32_t idx = this->field67_0x228.GetSize();
    this->field67_0x228.SetAtGrow(idx, *name);

    int32_t idx_23c = this->field68_0x23c.GetSize();
    this->field68_0x23c.SetSize(idx_23c + 1, -1);
    this->field68_0x23c[idx_23c] = flag;

    g_NetStru1_main.SendArenaScore(this->field67_0x228[idx], this->field68_0x23c[idx_23c], '[', nullptr);
    SendFragLimit(nullptr);
}

// 4FA4BB
void Server::GetArenaFrags(CString* name, uint32_t* frags) {
    *frags = 0;
    for (int32_t i = 0; i < this->field67_0x228.GetSize(); i++) {
        if (this->field67_0x228[i] == *name) {
            *frags = this->field68_0x23c[i];
            return;
        }
    }
}

// 5088DF
void Server::ProcessHatPacket(Packet* pkt) {
    if (NetStru1::HatConnector.active_connects.IsEmpty()) {
        return;
    }

    NetStru2* hat_connection = NetStru1::HatConnector.active_connects.GetHead();
    if (hat_connection == nullptr) {
        return;
    }
    
    PacketInfo* pkt_info = reinterpret_cast<PacketInfo*>(pkt);
    PacketJoin* pkt_join = reinterpret_cast<PacketJoin*>(pkt);

    switch (pkt->id) {
    case 0x64: // no-op
        break;

    case 0xd3: { // 0xd3 - Character return confirmation from hat
            CString login(pkt_join->name);
            if (this->FileList.GetSize() == 0) {
                LogMessage("Confirmation from hat for non-sended character. Login: " + login);
                return;
            }

            if (this->FileList[0] != login) {
                LogMessage("Confirmation from hat for non-sended character. Login: " + login);
                return;
            }
        
            this->FileList.RemoveAt(0, 1);
            this->field53_0x1f0 = 0;
#ifdef A2SERVER_PATCH
            try {
#endif
                CFile::Remove(g_ServerConfig.chr_base + login);
#ifdef A2SERVER_PATCH
            } catch (CFileException* e) {
                e->Delete();
            }
#endif
            LogMessage(login + "'s character returned to hat");
            if (this->FileList.GetSize() == 0 && this->field56_0x1fc != 0) {
                NetStru1::HatConnector.SendBarePacket(3, nullptr);
                this->field56_0x1fc = 0;
            }
            break;
        }
        
    case 0xd5: // Login to hat successful
        hat_connection->is_claimed = 1;
        LogMessage("Login to hat successful.");
        this->field50_0x1d4 = 0;
        this->UpdateHatAnnounce();
        if (pkt_info->param2 == 0 || g_ServerConfig.always_load_sacks != 0) {
            this->LoadSavedSacks();
        }
        this->field54_0x1f4 = pkt_info->param1;
        if (this->FileList.GetSize() == 0) {
            this->field56_0x1fc = 0;
            NetStru1::HatConnector.SendBarePacket(3, nullptr);
        } else {
            this->field56_0x1fc = 1;
        }
        break;

    case 0xdd: { // New player join request from hat
            // The packet is likely `PacketData`, but it still uses raw indices in data offsets, so whatever.
            uint8_t* p = reinterpret_cast<uint8_t*>(pkt);

            uint32_t id1        = *reinterpret_cast<uint32_t*>(p + 0x0E);
            uint32_t id2        = *reinterpret_cast<uint32_t*>(p + 0x12);
            uint32_t login_len  = *reinterpret_cast<uint32_t*>(p + 0x16);
            uint32_t name_len   = *reinterpret_cast<uint32_t*>(p + 0x1A);
            uint32_t block_size = *reinterpret_cast<uint32_t*>(p + 0x1E);
            int32_t  team_id    = *reinterpret_cast<int32_t*>(p + 0x22);
            const char* data    = reinterpret_cast<const char*>(p + 0x26);

            void*    block      = malloc(block_size);
            memcpy(block, data, block_size);

            CString login(data + block_size, login_len);
            CString name(data + block_size + login_len, name_len);

            int32_t result = this->JoinPlayer(id1, id2, name, login, block, block_size, team_id);
            if (result == 0) {
                LogMessage("New player " + name + " has logged in as " + login);
                NetStru1::HatConnector.SendHatJoinOk(id1, id2);
            } else {
                NetStru1::HatConnector.SendHatJoinFail(id1, id2, result);
            }
            break;
        }

    case 0xe1: { // 0xe1 - Player existence check from hat
            int32_t id1 = pkt_info->param1;
            int32_t id2 = pkt_info->param2;
            Player* player = g_PlayersList->FindByHatCharId(id1, id2);
            if (player == nullptr) {
                NetStru1::HatConnector.SendHatPlayerCheck(id1, id2, 1, 0);
                return;
            }

            NetStru2* client = g_NetStru1_main.GetClientByPlayerID(player->player_id);
            if (client != nullptr) {
                NetStru1::HatConnector.SendHatPlayerCheck(id1, id2, 2, 0);
            } else {
                NetStru1::HatConnector.SendHatPlayerCheck(id1, id2, 0, 0);
                player->drop_deadline_tick16 = this->tick16 + 300;
            }
            break;
        }

    default: {
            CString msg;
            msg.Format("Invalid command <%d> from hat", pkt->id);
            LogMessage(msg);
            break;
        }
    }
}

// 4FA551
void Server::HandleArenaPlayer(Player* player) {
    for (int32_t i = 0; i < this->field67_0x228.GetSize(); i++) {
        g_NetStru1_main.SendArenaScore(this->field67_0x228[i], this->field68_0x23c[i], '[', player);
    }
    SendFragLimit(player);
}

// 4FA78E
void Server::ResetArenaRound(int32_t arg) {
    g_NetStru1_main.SendInfoPacket(0x5c, g_ServerConfig.frag_limit, arg, nullptr);
    this->SendArenaResults();
    this->tick = 0;
    this->SetupArenaFileList();
    for (POSITION pos = g_PlayersList->list.GetHeadPosition(); pos != nullptr;) {
        Player* player = g_PlayersList->list.GetNext(pos);
        if (!player->is_ai) {
            player->KillAllUnits();
            player->frags = 0;
            this->AddArenaFrags(&player->name, 0);
        }
    }
}

CowardActivation::CowardActivation() { // 5a335c
    memset(this->key, 0, sizeof(this->key));
    this->enabled = 0;

    this->key[0] = 0x39;
    this->key[1] = 0x20;
    this->key[2] = 0x59;
    this->key[3] = 0x7c;
    this->key[4] = 0x2;
    this->key[5] = 0x11;
    this->key[6] = 0x5d;
    this->key[7] = 0x40;
    this->key[8] = 0x46;
    this->key[9] = 0x0;
    this->key[10] = 0x23 - this->key[0];
    this->key[0xb] = 0x23 - this->key[1];
    this->key[0xc] = 0x43 - this->key[2];
    this->key[0xd] = 0x6f - this->key[3];
    this->key[0xe] = 0x77 - this->key[4];
    this->key[0xf] = 0x61 - this->key[5];
    this->key[0x10] = 0x72 - this->key[6];
    this->key[0x11] = 0x64 - this->key[7];
    this->key[0x12] = 0x20 - this->key[8];
    this->key[0x13] = 0x20 - this->key[9];
    this->key[0x11] = 0;
}

// 5A3498
int32_t CowardActivation::CheckActivationKey(const char* str) {
    // WAT: 100 is excessive here, should be at most 10, because `key` is 100 bytes.
    for (int32_t i = 1; i < 100; i++) {
        this->enabled = 1;

        int32_t j = 0;
        while (this->key[i * 10 + j] != '\0') {
            // Decode by adding the cipher key (first 10 bytes) to the encrypted row
            if (this->key[i * 10 + j] + this->key[j] != str[j]) {
                this->enabled = 0;
            }
            j++;
        }

        if (this->enabled && j > 2) {
            return i;
        }
    }
    
    return 0;
}

// 4FAEB4
void SrvStru1::FreeLists() {
    if (this->sack_list) {
        delete this->sack_list;
        this->sack_list = nullptr;
    }

    if (this->building_list) {
        while (!this->building_list->IsEmpty()) {
            Building* building = this->building_list->RemoveTail();
            delete building;
        }
        delete this->building_list;
        this->building_list = nullptr;
    }

    if (this->effects_list) {
        while (!this->effects_list->list.IsEmpty()) {
            SpellEffect* effect = this->effects_list->list.RemoveTail();
            delete effect;
        }
        delete this->effects_list;
        this->effects_list = nullptr;
    }

    if (this->units_list) {
        for (POSITION it = this->units_list->unit_list.GetHeadPosition(); it != nullptr;) {
            Unit* unit = this->units_list->unit_list.GetNext(it);
            delete unit;
        }
        this->units_list->unit_list.RemoveAll();
    }

    while (!this->virtual_casters_list.IsEmpty()) {
        VirtualCaster* caster = this->virtual_casters_list.RemoveTail();
        delete caster;
    }

    // The original calls UnitList::Remove here, which removes the unit from
    // the list AND deletes it.
    while (!this->some_unit_list.IsEmpty()) {
        Unit* unit = this->some_unit_list.GetHead();
        delete unit;

        this->units_list->unit_list.RemoveHead();
    }
}

void SrvStru1::Deinit() { //4fab1d
    if (this->sack_list) {
        delete this->sack_list;
        this->sack_list = nullptr;
    }

    if (this->units_list) {
        for (POSITION pos = this->units_list->unit_list.GetHeadPosition(); pos != nullptr;) {
            Unit* unit = this->units_list->unit_list.GetNext(pos);
            if (unit) {
                delete unit;
            }
        }
        this->units_list->unit_list.RemoveAll();

        delete this->units_list;
        this->units_list = nullptr;
    }

    if (this->effects_list) {
        delete this->effects_list;
        this->effects_list = nullptr;
    }

    if (this->building_list) {
        for (POSITION pos = this->building_list->GetHeadPosition(); pos != nullptr;) {
            Building* bld = this->building_list->GetNext(pos);
            if (bld) {
                delete bld;
            }
        }
        this->building_list->RemoveAll();

        delete this->building_list;
        this->building_list = nullptr;
    }

    if (!this->virtual_casters_list.IsEmpty()) {
        for (POSITION pos = this->virtual_casters_list.GetHeadPosition(); pos != nullptr;) {
            VirtualCaster* caster = this->virtual_casters_list.GetNext(pos);
            if (caster) {
                delete caster;
            }
        }
        this->virtual_casters_list.RemoveAll();
    }

    if (!this->some_unit_list.IsEmpty()) {
        for (POSITION pos = this->some_unit_list.GetHeadPosition(); pos != nullptr;) {
            Unit* unit = this->some_unit_list.GetNext(pos);
            if (unit) {
                delete unit;
            }
        }
        this->some_unit_list.RemoveAll();
    }
}

Unit* SrvStru1::AddDummyUnitWithSpell(int32_t xpos, int32_t ypos, int32_t spellID, int16_t SpellDmg) { //4fb8cd
    Unit* unit = new Unit();
    unit->building_id = 0;

    this->some_unit_list.AddTail(unit);

    unit->position->Set(xpos, ypos, MapStuff_Instance);

    unit->spell_book = new SpellBook();

    Spell* spl = new Spell(spellID);

    unit->mind = 30;

    unit->hit_values.skill_levels[ spl->GetSphere() ] = SpellDmg;

    unit->spell_book->spells.SetAtGrow(spellID, spl);
    unit->some_spell = spl;
    unit->charge = 1;
    unit->relax = 1;
    return unit;
}

void SrvStru1::MakeAreaSpellEffect(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y, uint32_t spell_id, uint32_t power) { // 4FBB24
    Unit* unit = this->AddDummyUnitWithSpell(from_x, from_y, spell_id, power);
    unit->some_state = 14;
    unit->area_cast_x = to_x;
    unit->area_cast_y = to_y;
}

void SrvStru1::MakeTargetSpellEffect(uint8_t a0, uint8_t a1, Unit* unit, uint32_t a3, uint32_t a4) { // 4FBAE3
    Unit* u = this->AddDummyUnitWithSpell(a0, a1, a3, a4);
    u->some_state = 13;
    u->cast_target = unit;
}

void SrvStru1::UpdateMapObjects() { // 4FB4AA
    this->building_list->RegenTick();
    this->sack_list->UpdateSackVisibility();
}

// 4FB273
void SrvStru1::ProcessTick() {
    POSITION unit_it = this->some_unit_list.GetHeadPosition();
    while (unit_it != nullptr) {
        POSITION cur = unit_it;
        Unit* unit = this->some_unit_list.GetNext(unit_it);
        unit->ProcessEffects();
        if (unit->attack_ready != 0) {
            this->some_unit_list.RemoveAt(cur);
            delete unit;
        }
    }
    this->effects_list->ProcessTick();
    this->units_list->SendVisibilityUpdates();
}

SrvStru1::SrvStru1() { //4fa89c
    this->sack_list = nullptr;
    this->units_list = nullptr;
    this->effects_list = nullptr;
    this->building_list = nullptr;
}

SrvStru1::~SrvStru1() { //4fa981
    this->Deinit();
}

// 554B03
void SackList::ClearSentToMask(Player* player) {
    for (POSITION pos = this->list.GetHeadPosition(); pos != nullptr;) {
        Sack* sack = this->list.GetNext(pos);
        
        if (sack) {
            sack->sent_to_mask = sack->sent_to_mask & ~player->vision_sharing_id;
        }
    }
}

// 554927
void SackList::DropInventoryAt(TokenPos* pos, Inventory* inventory, int money, int is_main_player_unit) {
    if (this->DumpInventoryAt(pos, inventory, money, is_main_player_unit)) {
        Sack* sack = MapStuff_Instance->GetSackAtPos(pos);
        g_NetStru1_main.BroadcastTokenState(sack, nullptr, 0);
    }
}

// 55496E
int32_t SackList::AddSack(Sack* sack) {
    // Validate sack can be placed on map
    if (!MapStuff_Instance->CanPlaceSack(sack)) {
        delete sack;
        return 0;
    }
    
    this->list.AddTail(sack);
    return 1;
}

// 554460
int32_t SackList::DumpInventoryAt(TokenPos* pos, Inventory* inventory, int money, int param_5) {
    Sack* sack = MapStuff_Instance->GetSackAtPos(pos);
    
    if (!sack) {
        // No sack exists at this position - create a new one
        if (!inventory) {
            sack = new Sack(pos);
        } else {
            sack = new Sack(pos, inventory);
        }
        
        // Add to list; returns 0 on failure (e.g., can't place on map)
        if (!this->AddSack(sack)) {
            return 0;
        }
    } else if (inventory) {
        // Sack exists and we have inventory to merge - move sack to end of list
        POSITION found_pos = this->list.Find(sack);
        if (found_pos) {
            this->list.RemoveAt(found_pos);
            this->list.AddTail(sack);
            sack->inventory->TransferFrom(inventory);
        }
    }
    
    // Update money
    sack->money += money;
    
    if (sack->needs_save == 0 && param_5 == 0) {
        sack->needs_save = 0;
    } else {
        sack->needs_save = 1;
    }
    
    // Calculate experience level for sack icon/name update
    double old_level;
    if (sack->_exp <= 0) {
        old_level = -1.0;
    } else {
        old_level = log10((double)sack->_exp);
    }
    int32_t old_level_trunc = (int32_t)old_level;

    // Update sack name/icon based on contents
    sack->CalcContentsValue();

    // Check if level changed; the original compares the truncated old value
    // against the full-precision new value, so this broadcasts more often
    // than an int-to-int comparison would.
    double new_level = log10((double)sack->_exp);
    if ((double)old_level_trunc != new_level) {
        // Broadcast sack update
        if (g_Server->field4_0x74 == 0) {
            g_NetStru1_main.BroadcastTokenState(sack, nullptr, 0);
        } else {
            sack->resend_mask = 0xFFFF;
        }
    }

    if (sack->needs_save) {
        g_Server->SaveSack(sack);
    }
    
    return 1;
}

// 55BC58
void SackList::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar << this->list.GetCount();
        for (POSITION pos = this->list.GetHeadPosition(); pos != nullptr;) {
            Sack* sack = this->list.GetNext(pos);
            ar.WriteObject(sack);
        }
    } else {
        this->list.RemoveAll();
        int32_t count;
        ar >> count;
        for (int32_t i = 0; i < count; i++) {
            Sack* sack = (Sack*)ar.ReadObject(RUNTIME_CLASS(Sack));
            this->list.AddTail(sack);
        }
    }
}

SackList::~SackList() { //554bc3
    for (POSITION pos = this->list.GetHeadPosition(); pos != nullptr;) {
        Sack* sac = this->list.GetNext(pos);
        
        if (sac) {
            delete sac;
        }
    }

    this->list.RemoveAll();
}

// 5540ed
void SackList::UpdateSackVisibility() {
    const int32_t max_sacks = g_Server->field19_0x98.GetMapCenterX() / (g_ServerConfig.gameType == 0 ? 1 : 8);

    for (POSITION it = this->list.GetHeadPosition(); it != nullptr;) {
        POSITION it_before = it;
        Sack* sack = this->list.GetNext(it);
        
        if (!sack) {
            break;
        }
        
        // Update sack's field_0x48 based on map region
        uint8_t x = sack->position->GetX();
        uint8_t y = sack->position->GetY();
        sack->seen_by_mask = MapStuff_Instance->scan_presence_grid.sector_grid[1 + (x >> 3)][1 + (y >> 3)];
        
        // If field_0x48 changed and fog of war is enabled, broadcast to newly visible players
        if (sack->seen_by_mask != sack->seen_by_mask_prev && g_Server->field4_0x74 != 0) {
            for (int32_t i = 0; i < 16; i++) {
                // Check if bit i is set in new mask but not in old mask
                if ((sack->seen_by_mask & (1 << i)) && !(sack->seen_by_mask_prev & (1 << i))) {
                    Player* player = g_PlayersList->FindById(i + 0x10);
                    if (player) {
                        // Check if player can see this sack (not masked) or it's marked for visibility
                        bool player_cannot_see = !(sack->sent_to_mask & player->vision_sharing_id);
                        bool marked_visible = (sack->resend_mask & player->vision_sharing_id) != 0;
                        if (player_cannot_see || marked_visible) {
                            g_NetStru1_main.BroadcastTokenState(sack, player, 0);
                        }
                    }
                }
            }
            sack->seen_by_mask_prev = sack->seen_by_mask;
        }
        
        // Check if sack count exceeds limit and remove sacks with `field_0x44 == 0`.
        if (g_Server->field4_0x74 != 0) {
            if (this->list.GetCount() > max_sacks && sack->needs_save == 0) {
                MapStuff_Instance->RemoveSack(sack);
                g_NetStru1_main.UntrackToken(sack);
                this->list.RemoveAt(it_before);
                
                delete sack;
            }
        }
    }
    
    // Random chance to spawn new sacks if count is too low
    if (g_Server->field4_0x74 != 0 && Random0N(20) == 0) {
        if (g_ServerConfig.gameType == 0) {
            if (this->list.GetCount() < max_sacks) {
                g_Server->field19_0x98.SpawnRandomSacks(1);
            }
        } else {
            if (this->list.GetCount() < max_sacks) {
                g_Server->SpawnRandomSack();
            }
        }
    }
}

// 55837D
SpellEffectList::SpellEffectList() {}

// 55c213
void SpellEffectList::FixupPointers() {
    for (POSITION pos = this->list.GetHeadPosition(); pos != nullptr;) {
        SpellEffect* eff = this->list.GetNext(pos);
        if (eff != nullptr) {
            eff->FixupPointers();
        }
    }
}

SpellEffectList::~SpellEffectList() { //5583ed
    for (POSITION pos = this->list.GetHeadPosition(); pos != nullptr;) {
        SpellEffect* eff = this->list.GetNext(pos);

        if (eff) {
            delete eff;
        }
    }

    this->list.RemoveAll();
}

// 55BF31
void SpellEffectList::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar << this->list.GetCount();
        for (POSITION it = this->list.GetHeadPosition(); it != nullptr;) {
            SpellEffect* effect = this->list.GetNext(it);
            ar.WriteObject(effect);
        }
    } else {
        this->list.RemoveAll();
        int32_t count;
        ar >> count;
        for (int32_t i = 0; i < count; i++) {
            SpellEffect* effect = (SpellEffect*)ar.ReadObject(RUNTIME_CLASS(SpellEffect));
            this->list.AddTail(effect);
        }
    }
}

// 5585F2
void SpellEffectList::ProcessTick() {
    for (POSITION it = this->list.GetHeadPosition(); it != nullptr;) {
        POSITION current_it = it;
        SpellEffect* effect = this->list.GetNext(it);

        if (effect->caster != nullptr && effect->caster->pOwner == nullptr) {
            effect->caster = nullptr;
        }

        if (effect->field4_0x42 != effect->field6_0x44 && effect->IsKindOf(RUNTIME_CLASS(AreaEffect)) && g_Server->field4_0x74 != 0) {
            for (int32_t i = 0; i < 16; i++) {
                if ((effect->field4_0x42 & (1 << i)) != 0 && (effect->field6_0x44 & (1 << i)) == 0) {
                    Player* player = g_PlayersList->FindById(i + 16);
                    if (player != nullptr) {
                        g_NetStru1_main.SendAreaEffect((AreaEffect*)effect, 1);
                    }
                }
            }
            effect->field6_0x44 = effect->field4_0x42;
        }

        effect->ProcessEffects();

        if (effect->field2_0x40 != 0) {
            this->list.RemoveAt(current_it);
            delete effect;
        }
    }
}

// 502B4A
Player* Server::FindPlayerById(uint16_t player_id) {
    Player* player = g_PlayersList->FindById(player_id);
    if (!player) {
        CString id_str;
        IntToCString(&id_str, (int16_t)player_id);
        LogMessage("Order error: no such Player " + id_str);
        return nullptr;
    }
    return player;
}

// 502AD1
Unit* Server::FindPlayerUnit(uint16_t player_ref, uint16_t unit_id) {
    Player* player = this->FindPlayerById(player_ref);
    if (!player) {
        return nullptr;
    }
    Unit* unit = player->unit_list->FindById(unit_id);
    if (!unit) {
        return nullptr;
    }
    if (unit->some_state == 0x10) {
        return nullptr;
    }
    return unit;
}

// 502CB7
Inn* Server::FindInnAt(TokenPos* pos) {
    if (!this->srv_stru1->building_list) {
        return nullptr;
    }
    Building* building = this->srv_stru1->building_list->FindNearby(pos);
    if (!building) {
        return nullptr;
    }
    if (!building->IsKindOf(RUNTIME_CLASS(Inn))) {
        return nullptr;
    }
    return static_cast<Inn*>(building);
}

// 4FF878
void Server::SendMapInfo(Player* player) {
    PacketJoin* pkt = new PacketJoin(this->current_map_name);
    pkt->id = 0xBF;
    pkt->__field_0xa = (uint32_t)dword_6CDB38;
    pkt->to_player_id = player->player_id;
    g_NetStru1_main.QueuePacketSend(pkt);
}

// 5090A7
void Server::Tick() {
    this->tick++;
    this->ProcessAvailPackets();
    this->ProcessHatPackets();
    this->srv_stru1->ProcessTick();
    dword_6CDB3C->ProcessTick();
    g_NetStru1_main.SendPacket_64(this->tick, 0);
}

// 50979A
void Server::sub_50979A() {
    this->UpdateMissionResults();
}

// 509042
void Server::ProcessHatPackets() {
    NetStru1::HatConnector.ProcessConnections();
    Packet* pkt;
    while ((pkt = NetStru1::HatConnector.ReceiveAnyPacket()) != nullptr) {
        this->ProcessHatPacket(pkt);
    }
}

// 5090FA
void Server::UpdateMissionResult(Player* player) {
    if (player->is_in_world == 0 || player->is_ai) {
        return;
    }

    if (player->mission_result < 2) {
        if (player->main_unit == nullptr || player->main_unit->decay != 0) {
            player->mission_result = 2;
            g_NetStru1_main.SendInfoPacket(0xB4, player->mission_result, 0, player);
            if (player->is_logged_in != 0 && this->field4_0x74 == 0) {
                g_NetStru1_main.SendInfoPacket(0xB4, 0, 0, player);
            }
        } else if (g_World->mission_fail > 0) {
            if (player->mission_result < 2) {
                LogMessage("Logic - Mission Failed");
                g_NetStru1_main.SendInfoPacket(0xB4, g_World->mission_fail, 0, player);
                player->mission_result = (uint8_t)g_World->mission_fail;
            }
        } else if (g_World->mission_complete != 0 && player->mission_result != 1) {
            LogMessage("Logic - Mission Complete");
            g_NetStru1_main.SendInfoPacket(0xB5, 0, 0, player);
            player->mission_result = 1;
        }
    } else {
        if (this->field4_0x74 != 0 && player->is_active != 0 && player->main_unit->hp <= -40 && player->respawn_requested != 0) {
            player->respawn_requested = 0;
            Humanoid* unit = player->main_unit;
            if (unit->inventory == nullptr) {
                unit->inventory = new Inventory();
            }
            POSITION pos = this->srv_stru1->units_list->unit_list.Find(unit);
            if (pos != nullptr) {
                this->srv_stru1->units_list->unit_list.RemoveAt(pos);
            }
            unit->sent_to_mask = 0;
            uint8_t saved_spell_id = unit->eye2->spell_id;
            this->CreatePlayerCharacter(player, 0, 0, 0, 0, 0, 0);
            this->EnsurePlayerGroup(player);
            player->mission_result = 0;
            unit->eye2->spell_id = saved_spell_id;
            if (g_Server->field4_0x74 != 0 && (g_ServerConfig.gameType == 0 || g_ServerConfig.gameType == 3)) {
                Player* self_player = g_PlayersList->FindByName("Self");
                if (self_player != nullptr) {
                    for (int32_t i = 2; i < 16; i++) {
                        g_World->diplomacy.diplomacy[i][player->player_id] = g_World->diplomacy.diplomacy[i][self_player->player_id];
                        g_World->diplomacy.diplomacy[player->player_id][i] = g_World->diplomacy.diplomacy[self_player->player_id][i];
                    }
                    g_World->diplomacy.diplomacy[self_player->player_id][player->player_id] = 0x12;
                    g_World->diplomacy.diplomacy[player->player_id][self_player->player_id] = 0x12;
                    g_NetStru1_main.SendVisibilityState(player);
                }
            }
            g_NetStru1_main.SendUnitUpdate(unit, nullptr, 0xFFFFFFFF, 0xFFB, 0, 0);
            PacketInfo& packet_info = PacketInfo::Inst;
            packet_info.id = 0xAB;
            packet_info.to_player_id = player->player_id;
            packet_info.param1 = player->main_unit->position->GetX() & 0xFF;
            packet_info.param2 = player->main_unit->position->GetY() & 0xFF;
            g_NetStru1_main.QueuePacketSend(&packet_info);
        }
    }
}

// 5096E4
void Server::UpdateMissionResults() {
    POSITION pos = g_PlayersList->list.GetHeadPosition();
    while (pos != nullptr) {
        Player* player = g_PlayersList->list.GetNext(pos);
        this->UpdateMissionResult(player);
    }
}

// 4FA5C4
void Server::SendArenaResults() {
    LogMessage("Sending arena results to hat");
    CTime now = CTime::GetTickCount();
    struct tm* t = now.GetLocalTm(nullptr);
    int32_t sec = t->tm_sec;
    int32_t min = t->tm_min;
    int32_t hour = t->tm_hour;
    int32_t year = t->tm_year + 1900;
    int32_t month = t->tm_mon + 1;
    int32_t day = t->tm_mday;
    CString line;
    CString result;
    result.Format("Arena scores %02d.%02d.%02d %02d:%02d:%02d\n", day, month, year, hour, min, sec);
    int32_t count = this->field67_0x228.GetSize();
    for (int32_t i = 0; i < count; i++) {
        line.Format("\"%s\" %d\n", (const char*)this->field67_0x228[i], this->field68_0x23c[i]);
        result += line;
    }
    result += "\n";
    NetStru1::HatConnector.SendHatRawString(result, result.GetLength());
}

// 4FA01F
void Server::SetupArenaFileList() {
    this->field67_0x228.SetSize(0, -1);
    this->field68_0x23c.SetSize(0, -1);
    g_NetStru1_main.SendBarePacket(0x5a, nullptr);
    this->map_elapsed_time2 = 0;
}

// 4ED2DC
void Server::SaveGame(CString* filename) {
    LogMessage("Saving to:" + *filename);

    // Drop empty groups from every human player's group_list before saving.
    POSITION player_it = g_PlayersList->list.GetHeadPosition();
    while (player_it != nullptr) {
        Player* player = g_PlayersList->list.GetNext(player_it);
        if (player->is_ai) {
            continue;
        }
        CList<Group*>& groups = player->group_list->groups;
        POSITION group_it = groups.GetHeadPosition();
        while (group_it != nullptr) {
            POSITION cur = group_it;
            Group* g = groups.GetNext(group_it);
            if (g->unit_list.GetCount() == 0) {
                groups.RemoveAt(cur);
                delete g;
            }
        }
    }

    CFile file;
    if (!file.Open(*filename, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive, nullptr)) {
        return;
    }

    CMemFile mem_file;
    CArchive ar(&mem_file, CArchive::store);
    this->SerializeGame(ar);
    ar.Close();

    uint32_t mem_size = mem_file.GetLength();
    if (mem_size & 1) {
        ++mem_size;
        uint8_t pad = 0;
        mem_file.Write(&pad, 1);
    }

    uint16_t* detached = (uint16_t*)mem_file.Detach();
    int32_t num_words = mem_size / 2;

    uint8_t* out_buf = new uint8_t[num_words * 2];
    *(uint32_t*)out_buf = num_words;
    uint8_t* out_ptr = out_buf + 4;

    uint16_t* src = detached;
    int32_t i = 0;
    while (i < num_words) {
        if (src[0] == src[1]) {
            uint16_t* p = src;
            uint8_t count = 1;
            while (*p == *(p + 1) && count < 0x7F && (i + count) < num_words) {
                ++p;
                ++count;
            }
            *out_ptr++ = count | 0x80;
            *(uint16_t*)out_ptr = *p;
            out_ptr += 2;
            src += count;
            i += count;
        } else {
            uint16_t* p = src;
            uint8_t count = 1;
            while (*p != *(p + 1) && count < 0x7F && (i + count) < num_words) {
                ++p;
                ++count;
            }
            if (i + count == num_words) {
                ++count;
            }
            *out_ptr++ = count - 1;
            memcpy(out_ptr, src, (count - 1) * 2);
            out_ptr += (count - 1) * 2;
            src += count - 1;
            i += count - 1;
        }
    }

    uint32_t compressed_size = out_ptr - out_buf;
    delete[] (uint8_t*)detached;

    uint32_t magic = 0x26677342;
    file.Write(&magic, 4);
    uint32_t end_pos_placeholder = 0;
    file.Write(&end_pos_placeholder, 4);
    uint32_t version = 0x0bad0002;
    file.Write(&version, 4);
    file.Write(&compressed_size, 4);
    file.Write(out_buf, compressed_size);
    delete[] out_buf;

    uint32_t end_pos = file.GetPosition();
    if (this->field4_0x74 != 0) {
        char footer[0x100];
        memset(footer, 0, sizeof(footer));
        memcpy(footer, "Server Multiplayer save file.", 30);
        file.Write(footer, 0x100);
    }
    file.Seek(4, CFile::begin);
    file.Write(&end_pos, 4);
    file.Close();
}

// 4F3188
void Server::SerializeGame(CArchive& ar) {
    if (ar.IsStoring()) {
        ar << this->tick;
        ar << this->tick16;
        ar << this->current_map_name;

        ar << this->field26_0x174;
        ar << this->field28_0x17c;
        ar << this->field29_0x180;
        ar << this->field30_0x184;
        ar << this->field31_0x188;
        ar << this->field32_0x18c;
        ar << this->field33_0x190;
        ar << this->field34_0x194;
        ar << this->field37_0x1a0;
        ar << this->field36_0x19c;
        ar << this->field35_0x198;
        ar << this->field21_0xd4;
        ar << this->field22_0xd8;

        g_PlayersList->Serialize(ar);
        this->srv_stru1->units_list->Serialize(ar);

        uint8_t has_world_state = (this->field18_0x94 != 0) ? 1 : 0;
        ar.Write(&has_world_state, 1);

        if (this->field18_0x94 != 0) {
            this->srv_stru1->building_list->Serialize(ar);
            this->srv_stru1->effects_list->Serialize(ar);
            MapStuff_Instance->Serialize(ar);
            g_World->sub_5B0556(ar);
            this->srv_stru1->sack_list->Serialize(ar);

            int32_t caster_count = this->srv_stru1->virtual_casters_list.GetCount();
            ar << caster_count;
            POSITION pos = this->srv_stru1->virtual_casters_list.GetHeadPosition();
            while (pos != nullptr) {
                VirtualCaster* caster = this->srv_stru1->virtual_casters_list.GetNext(pos);
                ar.WriteObject(caster);
            }
        }

        const uint32_t sentinel = 0xbadface1;
        ar << sentinel;
        ar << sentinel;
    } else {
        BldIdSet_Clear();

        ar >> this->tick;
        ar >> this->tick16;
        ar >> this->current_map_name;

        ar >> this->field26_0x174;
        ar >> this->field28_0x17c;
        ar >> this->field29_0x180;
        ar >> this->field30_0x184;
        ar >> this->field31_0x188;
        ar >> this->field32_0x18c;
        ar >> this->field33_0x190;
        ar >> this->field34_0x194;
        ar >> this->field37_0x1a0;
        ar >> this->field36_0x19c;
        ar >> this->field35_0x198;
        ar >> this->field21_0xd4;

        int32_t loaded_mode = 0;
        ar >> loaded_mode;
        if (loaded_mode > 0 && loaded_mode < 4) {
            this->field22_0xd8 = loaded_mode;
        }

        g_PlayersList->Serialize(ar);
        this->srv_stru1->units_list->Serialize(ar);

        uint8_t has_world_state = 0;
        ar.Read(&has_world_state, 1);
        if (has_world_state == 0) {
            this->field18_0x94 = 0;
        } else {
            POSITION player_it = g_PlayersList->list.GetHeadPosition();
            while (player_it != nullptr) {
                Player* player = g_PlayersList->list.GetNext(player_it);
                if (player->unit_list == nullptr) {
                    continue;
                }

                POSITION unit_it = player->unit_list->unit_list.GetHeadPosition();
                while (unit_it != nullptr) {
                    Unit* unit = player->unit_list->unit_list.GetNext(unit_it);
                    if ((unit->unit_attrs & 8) == 0) {
                        dword_6CDB3C->AddTail(unit);
                    }
                }
            }

            if (this->srv_stru1->building_list == nullptr) {
                this->srv_stru1->building_list = new BuildingsList();
            }
            this->srv_stru1->building_list->Serialize(ar);

            if (this->srv_stru1->effects_list == nullptr) {
                this->srv_stru1->effects_list = new SpellEffectList();
            }
            this->srv_stru1->effects_list->Serialize(ar);

            CString map_name = this->current_map_name;
            if (this->field21_0xd4 != 0) {
                map_name = "Scenario\\" + map_name;
            }

            MapAlm* alm = new MapAlm((const char*)map_name);
            MapStuff_Instance = new MapStuff(alm, dword_6CDB3C);
            MapStuff_Instance->Serialize(ar);

            if (g_World == nullptr) {
                g_World = new World(MapStuff_Instance, g_PlayersList);
                g_World->sub_5B0556(ar);
            }

            if (this->srv_stru1->sack_list == nullptr) {
                this->srv_stru1->sack_list = new SackList();
            }
            this->srv_stru1->sack_list->Serialize(ar);

            this->srv_stru1->virtual_casters_list.RemoveAll();
            int32_t caster_count = 0;
            ar >> caster_count;
            for (int32_t i = 0; i < caster_count; i++) {
                CObject* obj = ar.ReadObject(RUNTIME_CLASS(VirtualCaster));
                this->srv_stru1->virtual_casters_list.AddTail(static_cast<VirtualCaster*>(obj));
            }

            this->field19_0x98.LoadTriggers(alm, 0);
            delete alm;

            this->field18_0x94 = 1;
        }

        if (has_world_state != 0) {
            dword_6CDB3C->CallMethod5();
            this->srv_stru1->units_list->CallMethod5();
            MapStuff_Instance->RelinkAfterLoad();
            g_World->RelinkPointers();
            this->srv_stru1->effects_list->FixupPointers();
        } else {
            POSITION player_it = g_PlayersList->list.GetHeadPosition();
            while (player_it != nullptr) {
                Player* player = g_PlayersList->list.GetNext(player_it);
                if (player->unit_list == nullptr) {
                    continue;
                }

                POSITION unit_it = player->unit_list->unit_list.GetHeadPosition();
                while (unit_it != nullptr) {
                    Unit* unit = player->unit_list->unit_list.GetNext(unit_it);
                    unit->last_hit_by = nullptr;
                    unit->spell = nullptr;
                    unit->cast_target = nullptr;
                }
            }
        }

        uint32_t ignored = 0;
        ar >> ignored;
        ar >> ignored;
    }

    this->script_settings->sub_5B681A(ar);
}

// 4EDB83
int32_t Server::LoadSavedGame(const CString& filename) {
    LogMessage("Loading from:" + filename);
    CFile file;
    if (!file.Open(filename, CFile::modeRead, nullptr)) {
        return 1;
    }
    uint32_t val;
    file.Read(&val, 4);
    if (val != 0x26677342) {
        LogMessage("Invalid save file.");
        return 1;
    }
    file.Read(&val, 4); // skip
    file.Read(&val, 4); // version
    if ((int32_t)val < 0xbad0002) {
        LogMessage("Outdated save file.");
        return 1;
    }
    uint32_t compressed_size;
    file.Read(&compressed_size, 4);
    uint8_t* buffer = new uint8_t[compressed_size];
    file.Read(buffer, compressed_size);
    file.Close();

    // Decompress: RLE over uint16_t words.
    // First 4 bytes of buffer = number of output uint16_t words.
    uint32_t num_words = *(uint32_t*)buffer;
    uint8_t* out_base = new uint8_t[num_words * 2];
    uint8_t* out_ptr = out_base;
    const uint8_t* in_ptr = buffer + 4;
    uint32_t in_offset = 4;
    while (in_offset < compressed_size) {
        uint8_t ctrl = *in_ptr;
        if (ctrl & 0x80) {
            // RLE: repeat next uint16_t (ctrl & 0x7F) times
            int32_t count = ctrl & 0x7F;
            uint16_t val16 = *(uint16_t*)(in_ptr + 1);
            for (int32_t i = 0; i < count; i++) {
                *(uint16_t*)out_ptr = val16;
                out_ptr += 2;
            }
            in_ptr += 3;
            in_offset += 3;
        } else {
            // Literal: copy next (ctrl) uint16_t words verbatim
            int32_t count = ctrl;
            memcpy(out_ptr, in_ptr + 1, count * 2);
            out_ptr += count * 2;
            in_ptr += count * 2 + 1;
            in_offset += count * 2 + 1;
        }
    }
    delete[] buffer;

    CMemFile mem_file(out_base, num_words * 2, 0);
    CArchive ar(&mem_file, CArchive::load);
#ifdef A2SERVER_PATCH
    try {
#endif
        this->SerializeGame(ar);
        ar.Close();
        mem_file.Close();
#ifdef A2SERVER_PATCH
    } catch (CArchiveException* e) {
        e->Delete();
        delete[] out_base;
        return 1;
    }
#endif

    delete[] out_base;
    return 0;
}

// 509879
Human* Server::SummonUnit(CString* unit_name, Unit* origin, int32_t is_hero) {
    Unit* unit = new Unit(*unit_name);

    if (unit->typeId == 0) {
        delete unit;
        unit = new Human(*unit_name, is_hero, nullptr);
    }

    if (unit->typeId == 0) {
        delete unit;
        return nullptr;
    }

    if (this->field18_0x94 != 0) {
        uint8_t x = origin->position->GetX();
        uint8_t y = origin->position->GetY();
        if (unit->PlaceOnMap(x, y, 3) == 0) {
            delete unit;
            return nullptr;
        }
        dword_6CDB3C->AddTailAllocId(unit);
    } else {
        unit->building_id = BldIdSet_AllocBit() & 0xffff;
    }

    Player* player = origin->pOwner;
    unit->pOwner = player;
    player->unit_list->AddTail(unit);

    Group* group = new Group();
    player->group_list->groups.AddTail(group);

    group->AddUnit(unit);
    g_World->ResetUnitToHold(unit);
    g_World->ProcessAnimateDeadGroup(group);

    if (this->field18_0x94 == 0) {
        g_NetStru1_main.SendUnitUpdate(unit, player, 0xFFFFFFFF, 0xFFB, 0, 0);
    } else {
        g_NetStru1_main.SendUnitUpdate(unit, nullptr, 0xFFFFFFFF, 0xFFB, 0, 0);
    }

    return static_cast<Human*>(unit);
}

// 4F8B74
void Server::SpawnRandomSack() {
    uint8_t x = (uint8_t)(Random0N(MapStuff_Instance->GetWidth() - 20) + 10);
    uint8_t y = (uint8_t)(Random0N(MapStuff_Instance->GetHeight() - 20) + 10);
    int32_t scroll_type = Random0N(4);
    static const char* scroll_names[] = {
        "SuperScroll Teleport",
        "SuperScroll Fire Wall",
        "SuperScroll Poison Cloud",
        "SuperScroll Stone Curse",
        "SuperScroll Prismatic Spray",
    };
    Item* item = new Item(CString(scroll_names[scroll_type]));
    if (!item) {
        return;
    }
    Inventory* inv = new Inventory();
    inv->PutItemIntoBagAtDefault(item);
    TokenPos pos(x, y, MapStuff_Instance);
    this->srv_stru1->sack_list->DumpInventoryAt(&pos, inv, 0, 0);
}

// 4F8FBF
void Server::UpdateQuestRune(int32_t arg1, int32_t arg2) {
    const char* rune_name = arg1 ? "Quest RuneA" : "Quest RuneF";

    if (arg2) {
        // Rune was picked up: find it in a ground sack and remove it.
        SackList* sack_list = this->srv_stru1->sack_list;
        for (POSITION sack_it = sack_list->list.GetHeadPosition(); sack_it != nullptr;) {
            Sack* sack = sack_list->list.GetNext(sack_it);
            Item* rune_item = sack->inventory->FindItemByName(rune_name);
            if (!rune_item) {
                continue;
            }
            // Find and remove the inventory node that holds the rune.
            POSITION item_node = nullptr;
            for (POSITION item_it = sack->inventory->items.GetHeadPosition(); item_it != nullptr;) {
                POSITION cur = item_it;
                Item* item = sack->inventory->items.GetNext(item_it);
                if (item == rune_item) {
                    item_node = cur;
                    break;
                }
            }
            if (item_node != nullptr) {
                sack->inventory->items.RemoveAt(item_node);
            }
            delete rune_item;
            if (sack->inventory->items.GetCount() == 0) {
                // Sack is now empty: remove it from the map and the list.
                MapStuff_Instance->RemoveSack(sack);
                g_NetStru1_main.UntrackToken(sack);
                POSITION sack_pos = sack_list->list.Find(sack);
                if (sack_pos) {
                    sack_list->list.RemoveAt(sack_pos);
                }
                delete sack;
            } else {
                g_NetStru1_main.BroadcastTokenState(sack, nullptr, 0);
            }
        }
    }

    // Rune was put back: reset flags and place it on the map at its spawn point.
    this->ctf_dropped[arg1] = 0;
    this->ctf_carrying[arg1] = 0;
    Item* item = new Item(rune_name);
    Inventory* inv = new Inventory();
    inv->PutItemIntoBagAtDefault(item);
    uint16_t yx = this->field19_0x98.field4_0x28.GetAt(arg1);
    uint8_t x = yx & 0xFF;
    uint8_t y = (yx >> 8) & 0xFF;
    TokenPos pos(x, y, MapStuff_Instance);
    this->srv_stru1->sack_list->DropInventoryAt(&pos, inv, 0, 1);
}
