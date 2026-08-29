#include "alm.h"

#include "file.h"
#include <stdio.h>

// 43C040
MapPlayerData::MapPlayerData() {
    this->gold = 0;
    this->flags = 0;
    this->diplomacy.SetSize(0x20, -1);
}

// 43C17F
MapPlayerData::MapPlayerData(int32_t index, int32_t color) {
    this->index = index;
    this->color = color;
    this->gold = 0;
    this->flags = 0;
    strcpy(this->name, "NoName");
    this->diplomacy.SetSize(0x20, -1);
}

// 41EDF0
int32_t MapPlayerData::FUN_0041edf0(int32_t idx) {
    return this->diplomacy[idx] & 4;
}

// 41EE20
uint32_t MapPlayerData::FUN_0041ee20(int32_t idx) {
    return this->diplomacy[idx] & 1;
}

// 56E910
MapLogicData::MapLogicData() {
    this->name[0] = '\0';
    this->type_id = 0;
    for (int32_t i = 0; i < 10; i++) {
        this->argument_values[i] = 0;
        this->argument_types[i] = 0;
        this->argument_names[i][0] = '\0';
    }
    this->exec_once_flag = 0;
}

// 56ED8C
MapEffectData::MapEffectData() {
    this->effect_id = 0;
    this->sack = nullptr;
    this->trap_x = 0;
    this->trap_y = 0;
    this->values[0] = 0;
    this->values[1] = 0;
    this->values[2] = 0;
    this->type_id = 0;
    this->spell_strength = 0;
}

// 56EAC7
MapSackData::MapSackData() {
    this->unit_id = 0;
    this->x = 0;
    this->y = 0;
    this->gold = 0;
}

// 5693A3
MapAlm::MapAlm(const char* filename) {
    this->map_width = 0x10;
    this->map_height = 0x10;
    this->inv_sun_angle = 0.0f;
    this->error_loading = 1;
    this->field16_0x90 = 0;
    this->contrast = 0x20;
    this->a_brightness = 0x20;
    this->landscape = nullptr;
    this->map_heights = nullptr;
    this->map_objects = nullptr;
    this->default_music.x = 0;
    this->default_music.y = 0;
    this->default_music.radius = 0;
    this->default_music.melody_type_id[0] = 0xFFFFFFFF;
    this->default_music.melody_type_id[1] = 0xFFFFFFFF;
    this->default_music.melody_type_id[2] = 0xFFFFFFFF;
    this->default_music.melody_type_id[3] = 0xFFFFFFFF;
    this->loading_player = nullptr;

    File2* file = new File2();
    if (!file->Open(filename, CFile::shareDenyWrite, nullptr)) {
        delete file;
        return;
    }

    MapHeader header;
    file->Read(&header, sizeof(MapHeader));

    if (strcmp(header.file_signature, "M7R") != 0) {
        delete file;
        this->error_loading = 2;
        return;
    }
    if (header.section_count < 3) {
        delete file;
        this->error_loading = 3;
        return;
    }
    if (header.version > 1600) {
        delete file;
        this->error_loading = 4;
        return;
    }

    int32_t player_count = 0, building_count = 0, unit_count = 0, trigger_count = 0, sack_count = 0;
    int32_t groups_count = 0, tavern_count = 0, shop_count = 0, pointer_count = 0, music_count = 0;
    int32_t has_info_section = 0;
    uint32_t map_area = 0;

    for (uint32_t section_num = 0; section_num < header.section_count; section_num++) {
        MapSectionHeader sec;
        file->Read(&sec, sizeof(MapSectionHeader));

        switch (sec.section_id) {
        case 0:
            has_info_section = 1;
            file->Read(&this->map_width, 4);
            file->Read(&this->map_height, 4);
            file->Read(&this->inv_sun_angle, 4);
            file->Read(&this->day_time_minutes, 4);
            file->Read(&this->a_brightness, 4);
            file->Read(&this->contrast, 4);
            file->Read(&this->use_tiles_mask, 4);
            file->Read(&player_count, 4);
            file->Read(&building_count, 4);
            file->Read(&unit_count, 4);
            file->Read(&trigger_count, 4);
            file->Read(&sack_count, 4);
            if (header.version >= 1150) {
                file->Read(&groups_count, 4);
            }
            if (header.version >= 1230) {
                file->Read(&tavern_count, 4);
                file->Read(&shop_count, 4);
                file->Read(&pointer_count, 4);
            }
            if (header.version >= 1300) {
                file->Read(&music_count, 4);
            }
            file->Read(this->map_name, sizeof(this->map_name));
            file->Read(&this->recommended_player_count, 4);
            file->Read(&this->map_level, 4);
            if (header.version >= 1160) {
                file->Read(&this->id0_unk1, 4);
                file->Read(&this->id0_unk2, 4);
            }
            file->Read(this->map_author, sizeof(this->map_author));
            break;
        case 1:
            this->landscape_val1 = sec.val1;
            map_area = this->map_width * this->map_height * 2;
            this->landscape = new uint16_t[map_area / 2];
            file->Read(this->landscape, map_area);
            break;
        case 2:
            map_area = sec.section_size;
            this->map_heights = new uint8_t[map_area];
            file->Read(this->map_heights, map_area);
            break;
        case 3:
            // WAT: vanilla version doesn't set `map_area`. They rely on section 3 being right
            // after section 2 and having the same number of objects as heights.
            map_area = sec.section_size;

            this->map_objects = new uint8_t[map_area];
            file->Read(this->map_objects, map_area);
            break;
        case 4:
            for (int32_t i = 0; i < building_count; i++) {
                uint32_t x_raw, y_raw, type_flags;
                uint16_t health;
                uint32_t player_id;
                uint16_t building_id;
                file->Read(&x_raw, 4);
                file->Read(&y_raw, 4);
                file->Read(&type_flags, 4);
                file->Read(&health, 2);
                file->Read(&player_id, 4);
                file->Read(&building_id, 2);

                MapBuildingData* building = new MapBuildingData();
                building->x = x_raw >> 8;
                building->y = y_raw >> 8;
                building->type_id = (uint16_t)type_flags;
                building->health = health;
                building->player_id = (uint16_t)player_id;
                building->building_id = building_id;

                if (type_flags == 0x21 || (type_flags & 0x1000000) != 0) {
                    uint16_t bridge[4];
                    file->Read(bridge, 8);
                    building->bridge_width = bridge[0];
                    building->bridge_height = bridge[2];
                } else {
                    building->bridge_width = 0;
                    building->bridge_height = 0;
                }
                this->map_buildings.SetAtGrow(this->map_buildings.GetSize(), building);
            }
            break;
        case 5:
            for (int32_t i = 0; i < player_count; i++) {
                MapPlayerData* player = new MapPlayerData();
                file->Read(&player->color, 4);
                file->Read(&player->flags, 4);
                file->Read(&player->gold, 4);
                file->Read(player->name, sizeof(player->name));
                player->index = i + 1;
                for (int32_t j = 0; j < 0x10; j++) {
                    file->Read(&player->diplomacy[j], 2);
                }
                this->map_players.SetAtGrow(this->map_players.GetSize(), player);
            }
            this->loading_player = this->map_players.GetData()[0];
            break;
        case 6:
            for (int32_t i = 0; i < unit_count; i++) {
                MapUnitData* unit = new MapUnitData{};
                file->Read(&unit->x, 4);
                file->Read(&unit->y, 4);
                int16_t type_id;
                file->Read(&type_id, 2);
                unit->type_id = (uint32_t)type_id;
                file->Read(&unit->face, 2);
                file->Read(&unit->flags1, 4);
                if (header.version >= 1150) {
                    file->Read(&unit->flags2, 4);
                } else {
                    unit->flags2 = 0;
                }
                if (header.version >= 987) {
                    file->Read(&unit->server_id, 4);
                } else {
                    unit->server_id = 0;
                }
                uint32_t player_id_raw;
                file->Read(&player_id_raw, 4);
                unit->player_id = (uint16_t)player_id_raw;
                file->Read(&unit->sack_id, 4);
                unit->sack_id--;
                file->Read(&unit->rotation, 4);
                file->Read(&unit->hp, 2);
                file->Read(&unit->max_hp, 2);
                if (header.version < 1100) {
                    file->Read(&unit->field17_0x24, 2);
                    file->Read(&unit->field18_0x26, 2);
                    if (header.version >= 951) {
                        file->Read(&unit->field36_0x4c, 2);
                        file->Read(&unit->field37_0x4e, 2);
                    }
                    file->Read(&unit->field19_0x28, 1);
                    file->Read(&unit->field20_0x29, 1);
                    file->Read(&unit->field21_0x2a, 1);
                    file->Read(&unit->field22_0x2b, 1);
                    file->Read(&unit->field23_0x2c, 1);
                    file->Read(&unit->field24_0x2d, 1);
                    if (header.version >= 985) {
                        file->Read(&unit->field25_0x2e, 1);
                    }
                    file->Read(&unit->field26_0x2f, 1);
                    file->Read(&unit->field27_0x30, 1);
                    file->Read(unit->field28_0x31, 6);
                    file->Read(unit->field29_0x37, 5);
                }
                if (header.version >= 1110) {
                    file->Read(&unit->unit_id, 4);
                } else {
                    file->Read(&unit->unit_id, 2);
                }
                file->Read(&unit->group_id, 4);
                if (unit->group_id > this->field16_0x90) {
                    this->field16_0x90 = unit->group_id;
                }
                if (unit->sack_id == -1 || unit->sack_id >= sack_count) {
                    unit->sack_id = -1;
                } else {
                    this->sacks.GetData()[unit->sack_id]->unit_id = unit->unit_id;
                }
                this->units_datas.SetAtGrow(this->units_datas.GetSize(), unit);
            }
            break;
        case 7:
            {
                int32_t instance_count;
                file->Read(&instance_count, 4);
                for (int32_t i = 0; i < instance_count; i++) {
                    MapLogicData* data = new MapLogicData();
                    file->Read(data->name, sizeof(data->name));
                    file->Read(&data->type_id, 4);
                    file->Read(&data->index, 4);
                    file->Read(&data->exec_once_flag, 4);
                    file->Read(data->argument_values, sizeof(data->argument_values));
                    file->Read(data->argument_types, sizeof(data->argument_types));
                    file->Read(data->argument_names, sizeof(data->argument_names));
                    this->logic_instances.AddTail(data);
                }
                int32_t check_count;
                file->Read(&check_count, 4);
                for (int32_t i = 0; i < check_count; i++) {
                    MapLogicData* data = new MapLogicData();
                    file->Read(data->name, sizeof(data->name));
                    file->Read(&data->type_id, 4);
                    file->Read(&data->index, 4);
                    file->Read(&data->exec_once_flag, 4);
                    file->Read(data->argument_values, sizeof(data->argument_values));
                    file->Read(data->argument_types, sizeof(data->argument_types));
                    file->Read(data->argument_names, sizeof(data->argument_names));
                    this->logic_checks.AddTail(data);
                }
                int32_t num_triggers;
                file->Read(&num_triggers, 4);
                this->map_logic_triggers.SetSize(num_triggers, -1);
                for (int32_t i = 0; i < num_triggers; i++) {
                    file->Read(&this->map_logic_triggers[i], sizeof(MapLogicTrigger));
                }
                break;
            }
        case 8:
            for (int32_t i = 0; i < sack_count; i++) {
                int32_t item_count;
                file->Read(&item_count, 4);

                MapSackData* sack = new MapSackData();

                file->Read(&sack->unit_id, 4);
                file->Read(&sack->x, 4);
                file->Read(&sack->y, 4);
                if (header.version >= 989) {
                    file->Read(&sack->gold, 4);
                } else {
                    sack->gold = 0;
                }
                for (int32_t j = 0; j < item_count; j++) {
                    uint32_t item_id, effect_id;
                    uint16_t wielded;
                    file->Read(&item_id, 4);
                    file->Read(&wielded, 2);
                    file->Read(&effect_id, 4);
                    sack->item_ids.SetAtGrow(sack->item_ids.GetSize(), item_id);
                    sack->items_wielded.SetAtGrow(sack->items_wielded.GetSize(), wielded);
                    sack->item_effect_ids.SetAtGrow(sack->item_effect_ids.GetSize(), effect_id);
                    if (effect_id != 0) {
                        MapEffectData* eff = this->effects[effect_id - 1];
                        eff->effect_id = j;
                        eff->sack = sack;
                    }
                }
                this->sacks.SetAtGrow(this->sacks.GetSize(), sack);
            }
            break;
        case 9:
            {
                int32_t num_effects;
                file->Read(&num_effects, 4);
                for (int32_t i = 0; i < num_effects; i++) {
                    MapEffectData* eff = new MapEffectData();
                    if (header.version > 989) {
                        file->Read(&eff->effect_id, 4);
                    }
                    file->Read(&eff->trap_x, 4);
                    file->Read(&eff->trap_y, 4);
                    file->Read(eff->values, 6);
                    file->Read(&eff->type_id, 4);
                    int32_t num_mods;
                    file->Read(&num_mods, 4);
                    for (int32_t j = 0; j < num_mods; j++) {
                        MapEffectModifier mod;
                        file->Read(&mod, sizeof(MapEffectModifier));
                        eff->modifiers_data.SetAtGrow(eff->modifiers_data.GetSize(), mod);
                    }
                    this->effects.SetAtGrow(this->effects.GetSize(), eff);
                }
                break;
            }
        case 10:
            for (int32_t i = 0; i < groups_count; i++) {
                MapGroupData* grp = new MapGroupData{};
                file->Read(grp, sizeof(MapGroupData));
                this->groups.SetAtGrow(this->groups.GetSize(), grp);
            }
            break;
        case 11:
            for (int32_t i = 0; i < tavern_count; i++) {
                MapInnData* inn = new MapInnData{};
                file->Read(inn, sizeof(MapInnData));
                this->taverns.SetAtGrow(this->taverns.GetSize(), inn);
            }
            for (int32_t i = 0; i < shop_count; i++) {
                MapShopData* shop = new MapShopData{};
                file->Read(shop, sizeof(MapShopData));
                this->shops.SetAtGrow(this->shops.GetSize(), shop);
            }
            for (int32_t i = 0; i < pointer_count; i++) {
                MapPointerData* pointer = new MapPointerData{};
                file->Read(pointer, sizeof(MapPointerData));
                this->pointers.SetAtGrow(this->pointers.GetSize(), pointer);
            }
            break;
        case 12:
            file->Read(&this->default_music, sizeof(MapMusicInfo));
            for (int32_t i = 0; i < music_count; i++) {
                MapMusicInfo* music = new MapMusicInfo{};
                file->Read(music, sizeof(MapMusicInfo));
                this->music_info.SetAtGrow(this->music_info.GetSize(), music);
            }
            break;
        default:
            file->Seek(sec.section_size, CFile::current);
            break;
        }
    }

    delete file;

    if (this->landscape == nullptr) {
        this->error_loading = 5;
    } else if (this->map_heights == nullptr) {
        this->error_loading = 6;
    } else {
        if (this->map_objects == nullptr) {
            this->map_objects = new uint8_t[map_area];
            memset(this->map_objects, 0, map_area);
        }
        if (this->loading_player == nullptr) {
            this->loading_player = new MapPlayerData(1, 1);
            this->loading_player->gold = 5000;
            this->loading_player->flags = 0;
            this->map_players.SetAtGrow(this->map_players.GetSize(), this->loading_player);
        }
        if (has_info_section == 0) {
            this->map_name[0] = '\0';
            this->map_author[0] = '\0';
            this->map_level = 1;
            this->recommended_player_count = 1;
        }
    }
}

// 56D92A
MapAlm::~MapAlm() {
    if (this->map_heights != nullptr) {
        delete[] this->map_heights;
    }
    if (this->landscape != nullptr) {
        delete[] this->landscape;
    }
    if (this->map_objects != nullptr) {
        delete[] this->map_objects;
    }

    for (int32_t i = 0; i < this->map_buildings.GetSize(); i++) {
        MapBuildingData* building = this->map_buildings[i];
        if (building != nullptr) {
            delete building;
        }
    }

    for (int32_t i = 0; i < this->units_datas.GetSize(); i++) {
        MapUnitData* unit = this->units_datas[i];
        if (unit != nullptr) {
            delete unit;
        }
    }

    for (int32_t i = 0; i < this->map_players.GetSize(); i++) {
        MapPlayerData* player = this->map_players[i];
        if (player != nullptr) {
            delete player;
        }
    }

    while (!this->logic_instances.IsEmpty()) {
        MapLogicData* data = this->logic_instances.RemoveHead();
        delete data;
    }

    while (!this->logic_checks.IsEmpty()) {
        MapLogicData* data = this->logic_checks.RemoveHead();
        delete data;
    }

    for (int32_t i = 0; i < this->sacks.GetSize(); i++) {
        MapSackData* sack = this->sacks[i];
        if (sack != nullptr) {
            delete sack;
        }
    }

    for (int32_t i = 0; i < this->effects.GetSize(); i++) {
        MapEffectData* effect = this->effects[i];
        if (effect != nullptr) {
            delete effect;
        }
    }

    for (int32_t i = 0; i < this->groups.GetSize(); i++) {
        delete this->groups[i];
    }

    for (int32_t i = 0; i < this->pointers.GetSize(); i++) {
        delete this->pointers[i];
    }

    for (int32_t i = 0; i < this->taverns.GetSize(); i++) {
        delete this->taverns[i];
    }

    for (int32_t i = 0; i < this->shops.GetSize(); i++) {
        delete this->shops[i];
    }

    for (int32_t i = 0; i < this->music_info.GetSize(); i++) {
        delete this->music_info[i];
    }
}

int32_t MapPlayerData::FUN_0041ee50(int32_t idx)
{ //41ee50
    return diplomacy[idx] & 8;
}
