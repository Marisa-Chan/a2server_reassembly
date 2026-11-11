// Ported from my-srvmgr/summon.cpp. The stat tables (unit_changes and the
// *_type_changes globals) are copied verbatim; the fixup function is adapted
// to our field names (experience/typeId/seeInvisible).

#include "summon.h"

#include <unordered_map>

#include "config_new.h"
#include "tangar_log.h"

#include "../eye.h"
#include "../unit.h"

struct UnitChanges {
    int16_t hp_max[4];
    uint8_t damage_min[4];
    uint8_t damage_spread[4];
    uint16_t defence[4];
    uint16_t attack[4];
    uint16_t absorption[4];
    uint16_t protection_magic[4];
    uint16_t speed[4];
    uint8_t rotation[4];
    uint8_t scan_range;
};

struct UnitChangesBlock {
    UnitChanges squirrel, snake, turtle;
};

std::unordered_map<ServerIDType, UnitChangesBlock> unit_changes{
    {ServerIDType::EASY, UnitChangesBlock{
        { // squirrel
            {30, 40, 50, 60}, // hp_max
            {1, 2, 3, 4}, // damage_min
            {4, 4, 4, 4}, // damage_spread
            {0, 5, 10, 20}, // defence
            {20, 40, 60, 80}, // attack
            {0, 0, 0, 2}, // absorption
            {0, 0, 0, 0}, // protection_magic
            {12, 13, 14, 15}, // speed
            {18, 20, 22, 24}, // rotation
            9, // scan_range
        },
        { // snake
            {45, 60, 80, 90}, // hp_max
            {1, 1, 1, 1}, // damage_min
            {2, 4, 6, 8}, // damage_spread
            {5, 10, 20, 40}, // defence
            {40, 60, 80, 100}, // attack
            {0, 1, 2, 2}, // absorption
            {40, 50, 55, 60}, // protection_magic
            {15, 16, 17, 18}, // speed
            {16, 16, 16, 16}, // rotation
            5, // scan_range
        },
        { // turtle
            {55, 80, 100, 110}, // hp_max
            {1, 1, 1, 1}, // damage_min
            {3, 5, 7, 9}, // damage_spread
            {30, 50, 55, 60}, // defence
            {20, 40, 60, 80}, // attack
            {1, 2, 3, 4}, // absorption
            {0, 0, 0, 0}, // protection_magic
            {8, 9, 10, 11}, // speed
            {12, 12, 12, 12}, // rotation
            6, // scan_range
        },
    }},
    {ServerIDType::KIDS, UnitChangesBlock{
        { // squirrel
            {30, 40, 50, 60}, // hp_max
            {1, 2, 3, 4}, // damage_min
            {4, 4, 4, 4}, // damage_spread
            {5, 10, 20, 40}, // defence
            {20, 40, 60, 80}, // attack
            {0, 0, 0, 2}, // absorption
            {0, 0, 0, 0}, // protection_magic
            {12, 13, 14, 15}, // speed
            {18, 20, 22, 24}, // rotation
            9, // scan_range
        },
        { // snake
            {45, 60, 80, 90}, // hp_max
            {1, 1, 1, 1}, // damage_min
            {2, 4, 6, 8}, // damage_spread
            {10, 20, 40, 60}, // defence
            {40, 60, 80, 100}, // attack
            {0, 1, 2, 2}, // absorption
            {40, 50, 55, 60}, // protection_magic
            {15, 16, 17, 18}, // speed
            {16, 16, 16, 16}, // rotation
            5, // scan_range
        },
        { // turtle
            {55, 80, 100, 110}, // hp_max
            {1, 1, 1, 1}, // damage_min
            {3, 5, 7, 9}, // damage_spread
            {50, 60, 80, 100}, // defence
            {20, 40, 60, 80}, // attack
            {1, 2, 3, 4}, // absorption
            {0, 0, 0, 0}, // protection_magic
            {9, 10, 11, 12}, // speed
            {12, 12, 12, 12}, // rotation
            6, // scan_range
        },
    }},
    {ServerIDType::NIVAL, UnitChangesBlock{
        { // squirrel
            {40, 50, 60, 80}, // hp_max
            {1, 2, 3, 4}, // damage_min
            {4, 4, 4, 4}, // damage_spread
            {10, 20, 40, 60}, // defence
            {20, 40, 60, 80}, // attack
            {0, 0, 0, 2}, // absorption
            {0, 0, 0, 0}, // protection_magic
            {12, 13, 14, 15}, // speed
            {18, 20, 22, 24}, // rotation
            9, // scan_range
        },
        { // snake
            {60, 80, 90, 100}, // hp_max
            {1, 1, 1, 1}, // damage_min
            {2, 4, 6, 8}, // damage_spread
            {20, 40, 60, 80}, // defence
            {40, 60, 80, 100}, // attack
            {0, 1, 2, 2}, // absorption
            {40, 50, 55, 60}, // protection_magic
            {15, 16, 17, 18}, // speed
            {16, 16, 16, 16}, // rotation
            5, // scan_range
        },
        { // turtle
            {80, 100, 110, 120}, // hp_max
            {1, 1, 1, 1}, // damage_min
            {3, 5, 7, 9}, // damage_spread
            {60, 80, 100, 120}, // defence
            {20, 40, 60, 80}, // attack
            {1, 2, 3, 4}, // absorption
            {0, 0, 0, 0}, // protection_magic
            {10, 11, 12, 13}, // speed
            {12, 12, 12, 12}, // rotation
            6, // scan_range
        },
    }},
    {ServerIDType::MEDIUM, UnitChangesBlock{
        { // squirrel
            {50, 60, 80, 100}, // hp_max
            {1, 2, 3, 4}, // damage_min
            {4, 4, 4, 4}, // damage_spread
            {20, 40, 60, 80}, // defence
            {20, 40, 60, 80}, // attack
            {0, 0, 0, 2}, // absorption
            {0, 0, 0, 0}, // protection_magic
            {12, 13, 14, 15}, // speed
            {18, 20, 22, 24}, // rotation
            9, // scan_range
        },
        { // snake
            {80, 90, 100, 110}, // hp_max
            {1, 1, 1, 1}, // damage_min
            {2, 4, 6, 8}, // damage_spread
            {40, 60, 80, 100}, // defence
            {40, 60, 80, 100}, // attack
            {0, 1, 2, 2}, // absorption
            {40, 50, 55, 60}, // protection_magic
            {15, 16, 17, 18}, // speed
            {16, 16, 16, 16}, // rotation
            5, // scan_range
        },
        { // turtle
            {100, 110, 120, 130}, // hp_max
            {1, 1, 1, 1}, // damage_min
            {3, 5, 7, 9}, // damage_spread
            {80, 100, 120, 150}, // defence
            {20, 40, 60, 80}, // attack
            {1, 2, 3, 4}, // absorption
            {0, 0, 0, 0}, // protection_magic
            {11, 12, 13, 14}, // speed
            {12, 12, 12, 12}, // rotation
            6, // scan_range
        },
    }},
};

struct TypeChanges {
    uint8_t body, reaction, mind, spirit;
    uint16_t hp_regen;
    uint32_t wimpy;
    uint8_t see_invisible;
};

TypeChanges squirrel_type_changes{
    10, 1, 1, 6, // body, reaction, mind, spirit
    50, // hp_regen,
    4, // wimpy
    1, // see_invisible
};
TypeChanges snake_type_changes{
    15, 1, 1, 3, // body, reaction, mind, spirit
    60, // hp_regen,
    0, // wimpy
    1, // see_invisible
};
TypeChanges turtle_type_changes{
    27, 1, 1, 10, // body, reaction, mind, spirit
    150, // hp_regen,
    0, // wimpy
    0, // see_invisible
};


void TangarFixSummonedUnit(Unit* unit, int level) {
    unit->summoned = 1; // Original instruction.

    // Summoned units should not give exp.
    unit->experience = 0;

    // This removes the units from counting towards "kill N" quests progress,
    // because the server checks unit type by `face << 8 | type_id`.
    unit->face = 0;

    if (unit->typeId != 74 && unit->typeId != 75 && unit->typeId != 76) {
        Printf("[summon] unhandled unit type_id: %d", unit->typeId);
        return;
    }

    if (unit_changes.count(Config::ServerID) == 0) {
        return;
    }

    const auto& changes_block = unit_changes[Config::ServerID];
    const auto& changes = unit->typeId == 74 ? changes_block.squirrel : unit->typeId == 75 ? changes_block.snake : changes_block.turtle;

    unit->hp_max = changes.hp_max[level-1];
    unit->hp = changes.hp_max[level-1];
    unit->hit_values.hand_damage_min = changes.damage_min[level-1];
    unit->hit_values.hand_damage_spread = changes.damage_spread[level-1];
    unit->protections.defense = changes.defence[level-1];
    unit->hit_values.attack = changes.attack[level-1];
    unit->protections.absorption = changes.absorption[level-1];
    for (int i = 1; i < 6; ++i) {
        unit->protections.magic_protections[i] = changes.protection_magic[level-1];
        unit->protections.weapon_protections[i] = 0;
    }

    unit->speed = changes.speed[level-1];
    if (unit->eye != nullptr) {
        unit->eye->rotation_speed = changes.rotation[level-1];
    }

    unit->scan_range = changes.scan_range << 8;

    const auto& type_changes = unit->typeId == 74 ? squirrel_type_changes : unit->typeId == 75 ? snake_type_changes : turtle_type_changes;

    unit->body = type_changes.body;
    unit->reaction = type_changes.reaction;
    unit->mind = type_changes.mind;
    unit->spirit = type_changes.spirit;
    unit->hp_regen = type_changes.hp_regen;

    if (unit->eye2 != nullptr) {
        unit->eye2->wimpy = type_changes.wimpy;
        unit->eye2->seeInvisible = type_changes.see_invisible;
    }
}
