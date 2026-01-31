#ifndef ALM_H
#define ALM_H

#include <cstdint>

#include "asm_mfc.h"
#include "mfc_templ.h"

struct MapHeader {
    uint32_t file_signature; // "M7R\x00"
    uint32_t field1_0x4; // Maybe ALM size?
    uint32_t field2_0x8;
    uint32_t section_count;
    uint32_t version;
};

struct MapSectionHeader {
    uint32_t val1; // It's 5 or 7 always, and doesn't seem to do anything.
    uint32_t alm_size;
    uint32_t section_size;
    uint32_t section_id;
    uint32_t signature;
};

class MapPlayerData : public CObject {
public:
    uint32_t index;
    uint32_t field2_0x8;
    uint32_t color;
    uint32_t gold;
    char name[32];
    uint32_t flags;
    CArray<int16_t> diplomacy;
};

struct MapLogicData {
    char name[64];
    uint32_t type_id;
    uint32_t index;
    uint32_t argument_values[10];
    uint32_t argument_types[10];
    char argument_names[10][64];
    uint32_t exec_once_flag;
};

struct MapLogicTrigger {
    char name[128];
    uint32_t check_ids[6];
    uint32_t instance_ids[4];
    uint32_t check_operators[3];
    uint32_t execute_once;
};

struct MapSackData {
    CArray<uint32_t> item_ids;
    CArray<uint32_t> item_effect_ids;
    CArray<uint16_t> items_wielded;
    uint32_t unit_id;
    uint32_t x;
    uint32_t y;
    uint32_t gold;
};

struct MapEffectModifier {
    uint32_t map_position;
    uint16_t flags;
};

struct MapEffectData {
    uint32_t effect_id;
    uint32_t something;
    uint32_t trap_x;
    uint32_t trap_y;
    uint16_t values[3];
    uint16_t type_id;
    uint16_t spell_strength;
    CArray<MapEffectModifier> modifiers_data;
};

struct MapGroupData {
    uint32_t group_id;
    uint32_t repop_time;
    uint32_t flags;
    uint32_t instance_id;
};

struct MapShopData {
    uint32_t shop_id;
    uint32_t flags[4];
    uint32_t min_price[4];
    uint32_t max_price[4];
    uint32_t max_count[4];
    uint32_t max_same_count[4];
};

struct MapInnData {
    uint32_t inn_id;
    uint32_t flags;
    uint32_t delivery_item_id;
};

struct MapPointerData {
    uint32_t building_id;
    uint32_t flags;
    uint32_t instance_id;
};

struct MapMusicInfo {
    uint32_t x;
    uint32_t y;
    uint32_t radius;
    uint32_t melody_type_id[4];
};

struct MapBuildingData {
    uint16_t x;
    uint16_t y;
    uint16_t type_id;
    uint16_t health;
    uint16_t player_id;
    uint16_t bridge_width;
    uint16_t bridge_height;
    uint8_t gap_0xe[2];
    uint32_t building_id;
};

struct MapUnitData {
    uint32_t x;
    uint32_t y;
    uint32_t type_id;
    uint16_t face;
    uint8_t gap_0xe[2];
    uint32_t server_id;
    uint16_t player_id;
    uint8_t gap_0x16[6];
    uint32_t rotation;
    uint16_t hp;
    uint16_t max_hp;
    uint16_t field17_0x24;
    uint16_t field18_0x26;
    uint8_t field19_0x28;
    uint8_t field20_0x29;
    uint8_t field21_0x2a;
    uint8_t field22_0x2b;
    uint8_t field23_0x2c;
    uint8_t field24_0x2d;
    uint8_t field25_0x2e;
    uint8_t field26_0x2f;
    uint8_t field27_0x30;
    uint8_t field28_0x31[6];
    uint8_t field29_0x37[5];
    uint16_t unit_id;
    uint8_t gap_0x3e[2];
    uint32_t group_id;
    uint32_t sack_id;
    uint32_t flags1;
    uint16_t field36_0x4c;
    uint16_t field37_0x4e;
    uint32_t flags2;
};

struct MapAlm {
    uint32_t map_width;
    uint32_t map_height;
    uint32_t landscape_val1;
    uint16_t *landscape;
    uint8_t *map_objects;
    uint8_t *map_heights;
    float inv_sun_angle;
    uint32_t day_time_minutes;
    uint32_t a_brightness;
    uint32_t contrast;
    CArray<MapPlayerData> map_players;
    MapPlayerData *loading_player;
    CList<MapLogicData> logic_instances;
    CList<MapLogicData> logic_checks;
    CArray<MapLogicTrigger> map_logic_triggers;
    uint32_t error_loading;
    uint32_t field16_0x90;
    uint32_t use_tiles_mask;
    char map_name[64];
    uint32_t recommended_player_count;
    uint32_t map_level;
    uint32_t id0_unk1;
    uint32_t id0_unk2;
    char map_author[512];
    CArray<MapSackData> sacks;
    CArray<MapEffectData> effects;
    CArray<MapGroupData> groups;
    CArray<MapShopData> shops;
    CArray<MapInnData> taverns;
    CArray<MapPointerData> pointers;
    CArray<MapMusicInfo> music_info;
    MapMusicInfo default_music;
    CArray<MapBuildingData> map_buildings;
    CArray<MapUnitData> units_datas;
};

#endif
