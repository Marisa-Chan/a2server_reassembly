#include "map_stuff.h"

#include <cstdlib>

#include "alm.h"
#include "constants.h"
#include "building.h"
#include "eye.h"
#include "file.h"
#include "game_app.h"
#include "group.h"
#include "net.h"
#include "player.h"
#include "players_list.h"
#include "resource.h"
#include "sack.h"
#include "spell_effect.h"
#include "unit.h"
#include "world.h"

CString g_MissionText; //660f2c
CString g_MissionBriefing; //660de8
CStringArray g_MissionFailures; //660f18
CStringArray g_MissionSubjs; //660ea8

// 587E19
MapStuff::MapStuff(MapAlm* alm, UnitList* unit_list) {
    this->scratch_cell_state.Null();
    this->field_0x58d88.Null();
    this->sub_58E891(unit_list);
    this->sub_58F166(alm);
}

// 587F70
MapStuff::~MapStuff() {
}

// 58E891
void MapStuff::sub_58E891(UnitList* unit_list) {
    this->field69_0xa456c = unit_list;
    this->scan_presence_grid.unit_list = unit_list;
    this->scan_presence_grid.num_detected = unit_list->unit_list.GetCount();
    this->scan_presence_grid.scan_delta = unit_list->unit_list.GetCount();

    this->map_width = 0x80;
    this->map_height = 0x80;
    this->visibility.map_stuff = this;

    memset(this->walk_cost_map, 1, sizeof(this->walk_cost_map));
    memset(this->obstacle_map, 0, sizeof(this->obstacle_map));
    memset(this->obstacle_map2, 0, sizeof(this->obstacle_map2));
    memset(this->height_map, 0, sizeof(this->height_map));

    this->cell_states.InitHashTable(100, 1);

    this->field48_0x58e88[0] = 0;   this->field49_0x58e90[0] = -1;
    this->field48_0x58e88[1] = 1;   this->field49_0x58e90[1] = -1;
    this->field48_0x58e88[2] = 1;   this->field49_0x58e90[2] = 0;
    this->field48_0x58e88[3] = 1;   this->field49_0x58e90[3] = 1;
    this->field48_0x58e88[4] = 0;   this->field49_0x58e90[4] = 1;
    this->field48_0x58e88[5] = -1;  this->field49_0x58e90[5] = 1;
    this->field48_0x58e88[6] = -1;  this->field49_0x58e90[6] = 0;
    this->field48_0x58e88[7] = -1;  this->field49_0x58e90[7] = -1;
    for (int32_t i = 0; i < 8; i++) {
        this->field50_0x58e98[i] = PosYX(this->field48_0x58e88[i], this->field49_0x58e90[i]);
    }

    // Direction/terrain cost lookup table; purpose not fully reversed.
    static const uint8_t direction_cost_table[4][14] = {
        { 2, 3, 2, 4, 3, 4, 2, 2, 2, 2, 4, 4, 4, 4 },
        { 3, 5, 3, 3, 1, 3, 2, 4, 2, 2, 4, 2, 4, 4 },
        { 2, 3, 2, 4, 3, 4, 2, 4, 2, 2, 4, 2, 4, 4 },
        { 5, 5, 5, 5, 5, 5, 2, 2, 2, 2, 4, 4, 4, 4 },
    };
    for (int32_t row = 0; row < 4; row++) {
        for (int32_t col = 0; col < 14; col++) {
            this->field26_0x540a6[row][col] = direction_cost_table[row][col];
        }
    }

    this->field28_0x54126[0] = PosYX(2, 1);
    this->field28_0x54126[1] = PosYX(5, 1);
    this->field28_0x54126[2] = PosYX(4, 1);
    this->field28_0x54126[3] = PosYX(7, 1);
    this->field28_0x54126[4] = PosYX(6, 1);
    this->field28_0x54126[5] = PosYX(5, 6);
    this->field28_0x54126[6] = PosYX(3, 7);
    this->field28_0x54126[7] = PosYX(8, 6);
    this->field28_0x54126[12] = PosYX(10, 1);

    File2 f;
    f.Open("World\\Data\\map.reg", 0, nullptr);

    RegFile reg;
    reg.ReadFromFile(&f, -1);

    this->walk_cost[0] = 0xFF;
    this->walk_cost[1] = reg.GetInt("Terrain", "CostLand", 8);
    this->walk_cost[2] = reg.GetInt("Terrain", "CostGrass", 8);
    this->walk_cost[3] = reg.GetInt("Terrain", "CostFlowers", 9);
    this->walk_cost[4] = reg.GetInt("Terrain", "CostSand", 14);
    this->walk_cost[5] = reg.GetInt("Terrain", "CostCracked", 6);
    this->walk_cost[6] = reg.GetInt("Terrain", "CostStones", 12);
    this->walk_cost[7] = reg.GetInt("Terrain", "CostSavanna", 11);
    this->walk_cost[8] = reg.GetInt("Terrain", "CostMountain", 16);
    this->walk_cost[9] = reg.GetInt("Terrain", "CostWater", 8);
    this->walk_cost[10] = reg.GetInt("Terrain", "CostRoad", 6);

    this->speed_multiplier = reg.GetInt("Path Finding", "SpeedMultiplier", 8);

    this->line_walk_delta[0][0] = 1;  this->line_walk_delta[0][1] = 0;
    this->line_walk_delta[1][0] = 0;  this->line_walk_delta[1][1] = 1;
    this->line_walk_delta[2][0] = -1; this->line_walk_delta[2][1] = 0;
    this->line_walk_delta[3][0] = 0;  this->line_walk_delta[3][1] = -1;
    this->line_walk_delta[4][0] = 1;  this->line_walk_delta[4][1] = 0;
    this->line_walk_delta[5][0] = 0;  this->line_walk_delta[5][1] = 1;
    this->line_walk_delta[6][0] = -1; this->line_walk_delta[6][1] = 0;
    this->line_walk_delta[7][0] = 0;  this->line_walk_delta[7][1] = -1;
    this->line_walk_delta[8][0] = 1;  this->line_walk_delta[8][1] = 0;
    this->line_walk_delta[9][0] = 0;  this->line_walk_delta[9][1] = 1;
    this->line_walk_delta[10][0] = -1; this->line_walk_delta[10][1] = 0;
    this->line_walk_delta[11][0] = 0;  this->line_walk_delta[11][1] = -1;

    memset(this->field31_0x54170, 0, sizeof(this->field31_0x54170));

    this->static_scan_ahead = reg.GetInt("Path Finding", "StaticScanAhead", 5);
    this->dynamic_scan_ahead = reg.GetInt("Path Finding", "DynamicScanAhead", 3);
    this->static_refresh_rate = reg.GetInt("Path Finding", "StaticRefreshRate", 16);
    this->dynamic_refresh_rate = reg.GetInt("Path Finding", "DynamicRefreshRate", 32);
    this->dynamic_by_static_lookup = reg.GetInt("Path Finding", "DynamicByStaticLookup", 3);
    this->static_isnt_needed = reg.GetInt("Path Finding", "StaticIsntNeeded", 5);
}

// Marks an 8-tile-thick obstacle border around the map edges — 58E670
void MapStuff::sub_58E670() {
    if (this->map_width == this->map_height) {
        for (int32_t i = 0; i < this->map_width; i++) {
            for (int32_t j = 0; j < 8; j++) {
                this->ObstacleAt(PosYX(j, i)) = 0x1F;
                this->ObstacleAt(PosYX(this->map_width - 1 - j, i)) = 0x1F;
                this->ObstacleAt(PosYX(i, j)) = 0x1F;
                this->ObstacleAt(PosYX(i, this->map_width - 1 - j)) = 0x1F;
            }
        }
    } else {
        for (int32_t i = 0; i < 256; i++) {
            for (int32_t j = 0; j < 8; j++) {
                this->ObstacleAt(PosYX(j, i)) = 0x1F;
                this->ObstacleAt(PosYX(255 - j, i)) = 0x1F;
                this->ObstacleAt(PosYX(i, j)) = 0x1F;
                this->ObstacleAt(PosYX(i, 255 - j)) = 0x1F;
            }
        }
        for (int32_t i = 0; i < this->map_height; i++) {
            for (int32_t j = 0; j < 8; j++) {
                this->ObstacleAt(PosYX(this->map_width - 1 - j, i)) = 0x1F;
            }
        }
        for (int32_t i = 0; i < this->map_width; i++) {
            for (int32_t j = 0; j < 8; j++) {
                this->ObstacleAt(PosYX(i, this->map_height - 1 - j)) = 0x1F;
            }
        }
    }
}

// 58F166
void MapStuff::sub_58F166(MapAlm* alm) {
    this->map_width = alm->map_width;
    this->map_height = alm->map_height;

    this->map_min_x = 8;
    this->map_min_y = 8;
    this->map_max_x = this->map_width - 9;
    this->map_max_y = this->map_height - 9;
    this->map_min_xy = PosYX(this->map_min_x, this->map_min_y).val;
    this->map_max_xy = PosYX(this->map_max_x, this->map_max_y).val;

    for (uint16_t x = 0; x < this->map_width; x++) {
        for (uint16_t y = 0; y < this->map_height; y++) {
            uint16_t idx = y * this->map_width + x;
            PosYX yx(x, y);

            uint16_t terrain = alm->landscape[idx];
            if (terrain & 0x2000) {
                this->ObstacleAt(yx) = 1;
            }

            uint8_t map_height_val = alm->map_heights[idx];
            uint8_t map_object_val = alm->map_objects[idx];

            uint8_t walk_cost_val;
            uint8_t classification = this->sub_58F48D(terrain & 0x3FF, &walk_cost_val);
            if (classification == 8) {
                this->ObstacleAt(yx) = 1;
            }

            if (classification < 0xD) {
                this->WalkCostAt(yx) = walk_cost_val;
            } else {
                this->WalkCostAt(yx) = this->walk_cost[0];
            }

            if ((terrain & 0x300) == 0x200) {
                this->ObstacleAt(yx) = 1;
            }

            if (map_object_val != 0) {
                this->ObstacleAt(yx) = 5;
            }

            this->height_map[yx.val] = map_height_val;
        }
    }

    this->sub_58E670();
    memcpy(this->obstacle_map2, this->obstacle_map, sizeof(this->obstacle_map));
}

// Computes edge-blended walk cost + classification code for a terrain cell (bits 0-9 of a
// landscape tile: bits 0-3 = weight index, bits 4-5 = sub index, bits 6-9 = direction pair
// index, bits 8-9 == 2 selects a special-case path). 58F48D
uint8_t MapStuff::sub_58F48D(uint16_t terrain_code, uint8_t* out_walk_cost) {
    uint8_t result = 0xFF;
    *out_walk_cost = 8;

    if ((terrain_code & 0x300) == 0x200) {
        if ((terrain_code & 0xF) < 8) {
            if ((terrain_code & 0xF) == 4) {
                result = (((terrain_code & 0x30) >> 4) == 1) ? 1 : 9;
            } else {
                result = 9;
            }
        }
    } else {
        uint32_t dir_idx = (terrain_code & 0x3C0) >> 6;
        uint32_t sub_idx = (terrain_code & 0x30) >> 4;
        uint32_t weight_idx = terrain_code & 0xF;

        if (weight_idx < 0xE) {
            PosYX dir_pair(this->field28_0x54126[dir_idx]);
            uint8_t idx_a = dir_pair.x;
            uint8_t idx_b = dir_pair.y;
            uint8_t weight = this->field26_0x540a6[sub_idx][weight_idx];

            result = (weight > 2) ? idx_a : idx_b;

            switch (weight) {
            case 1:
                *out_walk_cost = this->walk_cost[idx_b];
                break;
            case 2:
                *out_walk_cost = (this->walk_cost[idx_a] + this->walk_cost[idx_b] * 3) >> 2;
                break;
            case 3:
                *out_walk_cost = (this->walk_cost[idx_a] + this->walk_cost[idx_b]) >> 1;
                break;
            case 4:
                *out_walk_cost = (this->walk_cost[idx_a] * 3 + this->walk_cost[idx_b]) >> 2;
                break;
            case 5:
                *out_walk_cost = this->walk_cost[idx_a];
                break;
            }
        }
    }

    return result;
}

// Shared by sub_591424/sub_59166C/sub_592831
uint8_t MapStuff::ClassifyDirection16(int32_t dx, int32_t dy) {
    uint16_t abs_dx = abs(dx);
    uint16_t abs_dy = abs(dy);

    if (dx > 0) {
        if (dy > 0) {
            if (abs_dx > abs_dy) {
                return (abs_dx <= abs_dy * 2) ? 5 : 4;
            }
            return (abs_dy <= abs_dx * 2) ? 6 : 7;
        }
        if (abs_dx > abs_dy) {
            return (abs_dx <= abs_dy * 2) ? 2 : 3;
        }
        return (abs_dy <= abs_dx * 2) ? 1 : 0;
    }
    if (dy > 0) {
        if (abs_dx > abs_dy) {
            return (abs_dx <= abs_dy * 2) ? 10 : 11;
        }
        return (abs_dy <= abs_dx * 2) ? 9 : 8;
    }
    if (abs_dx > abs_dy) {
        return (abs_dx <= abs_dy * 2) ? 13 : 12;
    }
    return (abs_dy <= abs_dx * 2) ? 14 : 15;
}

// Shared by sub_591424/sub_59166C
uint8_t MapStuff::ClassifyFacing16(int32_t dx, int32_t dy) {
    uint8_t raw = this->ClassifyDirection16(dx, dy);
    if (raw != 0) {
        raw += 1;
    }
    return (raw >> 1) << 5;
}

// 591424
uint8_t MapStuff::sub_591424(Unit* unit, Unit* target) {
    int32_t dx = target->sub_528725() - unit->sub_528725();
    int32_t dy = target->sub_528763() - unit->sub_528763();
    return this->ClassifyFacing16(dx, dy);
}

// 59166C
uint8_t MapStuff::sub_59166C(Unit* unit, PosYX yx) {
    int32_t dx = yx.x - unit->position->GetX();
    int32_t dy = yx.y - unit->position->GetY();
    return this->ClassifyFacing16(dx, dy);
}

// 58FE6D
int MapStuff::sub_58FE6D(Unit* unit, Unit* target, uint8_t max_range) {
    unit->position->sub_58bec3();
    uint8_t current_facing = unit->eye->field0_0x0;
    uint8_t computed_facing = this->sub_591424(unit, target);
    if (current_facing != computed_facing) {
        return 0;
    }
    if (this->sub_59190D(unit, target) <= max_range) {
        return 1;
    }
    return 0;
}

// 58FEDA
int MapStuff::sub_58FEDA(Unit* unit, PosYX yx, uint8_t max_range) {
    unit->position->sub_58bec3();
    uint8_t current_facing = unit->eye->field0_0x0;
    uint8_t computed_facing = this->sub_59166C(unit, yx);
    if (current_facing != computed_facing) {
        return 0;
    }
    uint8_t distance = this->sub_593B29(unit->position->GetYX(), yx);
    if (distance <= max_range) {
        return 1;
    }
    return 0;
}

// 58FF51
void MapStuff::sub_58FF51(Unit* unit, PosYX yx, uint8_t max_range) {
    uint8_t distance = this->sub_593B29(unit->position->GetYX(), yx);
    int32_t in_cell_middle = unit->position->sub_58bec3();

    if (!in_cell_middle) {
        this->sub_590678(unit);
        return;
    }

    if (distance <= max_range) {
        if (max_range != 0) {
            this->sub_5918E2(unit, yx);
        }
        return;
    }

    unit->eye->counter++;

    if (unit->eye->field114_0x74 != yx || unit->eye->field8_0x9 > this->static_refresh_rate) {
        this->sub_58826D(unit, yx.x, yx.y, 1, nullptr);
        unit->eye->field136_0x90 = 0;
        unit->eye->field114_0x74 = yx;

        if (!unit->list1.IsEmpty()) {
            unit->eye->field115_0x76 = unit->list1.GetTail();
        } else {
            unit->eye->field115_0x76 = unit->position->GetYX();
            unit->eye->field139_0x98 = 1;
        }
        unit->eye->field8_0x9 = 0;
        unit->list2.RemoveAll();
    }

    if (unit->position->GetYX() == unit->eye->field115_0x76) {
        unit->eye->field136_0x90 = 1;
        return;
    }

    uint8_t ok = 0;
    if (unit->list2.IsEmpty() || unit->eye->counter > this->dynamic_refresh_rate) {
        uint16_t next_yx = this->sub_593AA4(unit);
        ok = 0;
        if (next_yx != 0) {
            if (this->sub_597140(unit, next_yx, 0) == 0) {
                ok = this->field41_0x58d80->sub_5ABB32(unit, next_yx);
            }
        }
        if (ok) {
            uint8_t facing = this->sub_59166C(unit, next_yx);
            if (unit->eye->field0_0x0 != facing) {
                this->sub_5918E2(unit, next_yx);
                unit->eye2->cast_action = 10;
            } else {
                unit->eye2->cast_action = 0;
            }
        } else {
            this->sub_590902(unit, nullptr);
            unit->eye->field8_0x9++;
            unit->eye->counter = 0;
        }
    }
    if (!ok) {
        this->sub_5907BE(unit);
    }
}

// 59028D
void MapStuff::sub_59028D(Unit* unit, Unit* target, uint8_t max_range) {
    PosYX target_yx = target->position->GetYX();
    uint8_t range = this->sub_59190D(unit, target);
    int32_t in_cell_middle = unit->position->sub_58bec3();

    if (!in_cell_middle) {
        this->sub_590678(unit);
        return;
    }

    if (range <= max_range) {
        this->sub_5918B8(unit, target);
        return;
    }

    // Out of range — pursue target
    unit->eye->counter++;

    if (unit->eye->field120_0x7c != target) {
        unit->eye->field8_0x9 = 0xFF;
        unit->list1.RemoveAll();
        unit->eye->field130_0x8a = 0xFF;
        unit->eye->field115_0x76 = target_yx;
        unit->eye->field132_0x8c = target_yx;
    }

    if (unit->eye->field8_0x9 > unit->list1.GetCount() / 3 + 1) {
        if (unit->eye->field130_0x8a > this->static_isnt_needed) {
            this->sub_58826D(unit, target_yx.x, target_yx.y, 1, target);
        } else {
            unit->list1.RemoveAll();
            unit->list1.AddTail(unit->eye->field115_0x76);
        }

        unit->eye->field130_0x8a = unit->list1.GetCount();
        unit->eye->field114_0x74 = target_yx;

        if (!unit->list1.IsEmpty()) {
            unit->eye->field115_0x76 = unit->list1.GetTail();
            unit->eye->field132_0x8c = target_yx;
        } else {
            unit->eye->field115_0x76 = unit->position->GetYX();
            unit->eye->field132_0x8c = target_yx;
            unit->eye->field139_0x98 = 1;
        }

        unit->eye->field120_0x7c = target;
        unit->eye->field8_0x9 = 0;

        if (unit->eye->field130_0x8a > this->static_isnt_needed) {
            unit->list2.RemoveAll();
        }
    }

    if (unit->position->GetYX() == unit->eye->field115_0x76 && unit->eye->field132_0x8c == target_yx) {
        unit->eye->field8_0x9++;
        unit->eye->field120_0x7c = nullptr;
        return;
    }

    if (unit->list2.IsEmpty() || unit->eye->counter > this->dynamic_by_static_lookup) {
        if (unit->eye->field0_0x0 == unit->eye->field1_0x1) {
            this->sub_590902(unit, target);
            unit->eye->field137_0x94 = target_yx;
            unit->eye->field138_0x96 = unit->position->GetYX();
            unit->eye->field8_0x9++;
            unit->eye->counter = 0;
        }
    }

    this->sub_5907BE(unit);
}

// 5913BD
int16_t MapStuff::sub_5913BD(Unit* unit, uint8_t x, uint8_t y) {
    uint8_t type = (uint8_t)unit->VMethod4();
    if (type == 1) {
        return unit->speed * 8 / walk_cost_map[y * 256 + x];
    }
    if (type == 2 || type == 3) {
        return unit->speed;
    }
    return 0;
}

// 58FB12 --- Reconciles eye position1/rotation-target state against the unit's current tile.
void MapStuff::sub_58FB12(Unit* unit) {
    uint16_t old_x = unit->position->GetXx();
    uint16_t old_y = unit->position->GetYy();

    UnitEye* eye = unit->eye;
    eye->field156_0xac += 1;

    uint16_t new_x = old_x + eye->field160_0xb0;
    uint16_t new_y = old_y + eye->field161_0xb1;

    int16_t mode = eye->field158_0xae;
    if ((mode == 1 || mode == 5) && (new_x & 0xFF) == 0 && (new_y & 0xFF) == 0) {
        new_x -= 1;
    }

    if ((old_x >> 8) == (new_x >> 8) && (old_y >> 8) == (new_y >> 8)) {
        unit->position->SetCoords2(new_x, new_y);
    } else {
        PosYX old_yx = unit->position->CompatGetYX();
        this->FUN_005969c6(unit, 0, 2);
        this->sub_58B1D7(unit);
        unit->position->SetCoords2(new_x, new_y);
        this->sub_58AD4A(unit);
        this->FUN_005969c6(unit, old_yx, 0);
    }

    if (eye->field154_0xaa <= eye->field156_0xac) {
        // Inline sub_58FD16: reset subcell position to cell center.
        unit->position->x_subcell = 0x80;
        unit->position->y_subcell = 0x80;
    }
}

// 590678
void MapStuff::sub_590678(Unit* unit) {
    uint16_t prev_yx = unit->position->CompatGetYX();
    this->sub_58FB12(unit);

    if (prev_yx != unit->position->CompatGetYX()) {
        unit->eye->field113_0x72 = this->sub_5913BD(unit, unit->position->GetX(), unit->position->GetY());
    }

    if (!unit->position->sub_58bec3()) {
        return;
    }

    unit->eye->field152_0xa8 = 0;
    unit->eye->field154_0xaa = 0;
    unit->eye->field156_0xac = 0;

    this->FUN_005969c6(unit, 0, 2);
    this->FUN_005969c6(unit, 0, 1);
    unit->list2.RemoveAll();

    if (!this->cell_states.Lookup(unit->position->CompatGetYX(), this->scratch_cell_state)) {
        return;
    }

    if (this->scratch_cell_state.spell_id == spell::teleport) {
        this->sub_5954AC(unit, this->scratch_cell_state.teleport_x, this->scratch_cell_state.teleport_y);
    }
}

// 5907BE
void MapStuff::sub_5907BE(Unit* unit) {
    if (unit->list2.IsEmpty()) {
        this->sub_58FD2F(unit);
        return;
    }

    unit->eye->position1 = unit->list2.GetHead();

    uint16_t fresh_yx = unit->position->CompatGetYX();
    unit->eye->field111_0x70 = fresh_yx;
    unit->eye->field140_0x9c = 0;

    this->FUN_005969c6(unit, 0, 2);
    this->FUN_005969c6(unit, unit->eye->position1, 0);

    unit->eye->field1_0x1 = this->sub_58A158(unit, unit->eye->position1);

    if (unit->eye->field0_0x0 == unit->eye->field1_0x1) {
        this->sub_593CD5(unit, unit->position->CompatGetYX(), unit->eye->field0_0x0);
        unit->eye->field156_0xac = 0;
        this->sub_58FB12(unit);
    } else {
        this->sub_590F94(unit, unit->eye->field1_0x1);
    }
}

// 58FD2F
void MapStuff::sub_58FD2F(Unit* unit) {
    uint8_t orig_facing = unit->eye->field0_0x0;

    uint8_t diff = abs(unit->eye->field0_0x0 - unit->eye->field1_0x1);
    if (diff > 0x80) {
        diff = 0x100 - diff;
    }

    if (diff < unit->eye->rotation_speed) {
        unit->eye->field0_0x0 = unit->eye->field1_0x1;
        return;
    }

    if (((orig_facing + diff) & 0xFF) == unit->eye->field1_0x1) {
        unit->eye->field0_0x0 = unit->eye->field0_0x0 + unit->eye->rotation_speed;
    } else if (((orig_facing - diff) & 0xFF) == unit->eye->field1_0x1) {
        unit->eye->field0_0x0 = unit->eye->field0_0x0 - unit->eye->rotation_speed;
    }
}

// 590F94
void MapStuff::sub_590F94(Unit* unit, uint8_t facing) {
    unit->eye->field1_0x1 = facing;

    uint8_t diff = abs(unit->eye->field0_0x0 - unit->eye->field1_0x1);

    if (unit->eye->field144_0xa0 == 0) {
        unit->eye->field141_0x9d = 0;
    }

    if (unit->eye->field144_0xa0 == 0 && (diff < 0x21 || (0x100 - diff) < 0x21)) {
        unit->eye->field0_0x0 = unit->eye->field1_0x1;
        unit->eye->field148_0xa4 = 1;
    } else {
        this->sub_58FD2F(unit);

        if (diff > 0x80) {
            diff = 0x100 - diff;
        }

        unit->eye->field148_0xa4 = diff / unit->eye->rotation_speed;

        if (unit->eye->field148_0xa4 * unit->eye->rotation_speed != diff) {
            unit->eye->field148_0xa4 += 1;
        }
    }

    unit->eye->field144_0xa0 = 1;
    unit->eye->field141_0x9d += 1;

    if (unit->eye->field0_0x0 == unit->eye->field1_0x1) {
        unit->eye->field144_0xa0 = 0;
    }
}

// 590902
void MapStuff::sub_590902(Unit* unit, Unit* target) {
    int32_t distance = 0;
    int32_t kind = 0;
    PosYX cur_yx = unit->position->CompatGetYX();
    PosYX next_yx;

    int32_t list1_count = unit->list1.GetCount();
    if (list1_count != 0) {
        next_yx = unit->list1.GetHead();
        distance = this->sub_58BFA3(cur_yx.x, cur_yx.y, next_yx.x, next_yx.y);
    }

    PosYX target_yx;
    if (list1_count <= this->static_isnt_needed) {
        target_yx = unit->eye->field115_0x76;
        kind = 1;
    } else if (distance > this->dynamic_by_static_lookup) {
        target_yx = next_yx;
        kind = 2;
    } else {
        POSITION pos = unit->list1.GetHeadPosition();
        for (int32_t i = 0; i < this->dynamic_by_static_lookup + 1; i++) {
            target_yx = unit->list1.GetNext(pos);
        }
        kind = 3;
    }

    this->sub_58826D(unit, target_yx.x, target_yx.y, 0, target);

    if (unit->list2.GetCount() == 0 && kind == 1) {
        unit->eye->field139_0x98 = 1;
    }

    if (list1_count != 0 && distance <= this->dynamic_by_static_lookup) {
        unit->list1.RemoveHead();
    }
}

// 58E407
int MapStuff::sub_58E407(Sack* sack) {
    if (this->Obstacle2At(sack->position->CompatGetYX()).TestBits(1)) {
        return 0;
    }

    if (this->cell_states.Lookup(sack->position->CompatGetYX(), this->scratch_cell_state)) {
        if (this->scratch_cell_state.sack != nullptr) {
            return 0;
        }
        this->scratch_cell_state.sack = sack;
        this->cell_states.SetAt(sack->position->CompatGetYX(), this->scratch_cell_state);
        return 1;
    }

    this->FUN_0058b3e0(sack->position->CompatGetYX());

    if (!this->cell_states.Lookup(sack->position->CompatGetYX(), this->scratch_cell_state)) {
        return 0;
    }

    this->scratch_cell_state.sack = sack;
    this->cell_states.SetAt(sack->position->CompatGetYX(), this->scratch_cell_state);
    this->sub_58B593(sack->position->CompatGetYX());
    return 1;
}

// 596576
int MapStuff::sub_596576(uint16_t yx, void* src) {
    if (this->Obstacle2At(yx).TestBits(1)) {
        return 0;
    }

    if (this->cell_states.Lookup(yx, this->scratch_cell_state)) {
        bool had_spell = this->scratch_cell_state.spell_id != 0;
        memcpy(&this->scratch_cell_state.spell_id, src, 6);
        this->cell_states.SetAt(yx, this->scratch_cell_state);
        return had_spell ? 0 : 1;
    }

    this->FUN_0058b3e0(yx);

    if (!this->cell_states.Lookup(yx, this->scratch_cell_state)) {
        return 0;
    }

    memcpy(&this->scratch_cell_state.spell_id, src, 6);
    this->cell_states.SetAt(yx, this->scratch_cell_state);
    this->sub_58B593(yx);
    return 1;
}

// 5964D7
void MapStuff::sub_5964D7(uint16_t yx, uint8_t range, Player* owner, UnitList* out_list) {
    out_list->unit_list.RemoveAll();
    if (this->field69_0xa456c == nullptr) {
        return;
    }

    CList<Unit*>& list = this->field69_0xa456c->unit_list;

    POSITION it = list.GetHeadPosition();
    while (it != nullptr) {
        Unit* unit = list.GetNext(it);
        uint8_t distance = this->sub_593B29(yx, unit->position->GetYX());
        if (distance <= range) {
            out_list->AddTail(unit);
        }
    }
}

// 5948B0
void MapStuff::sub_5948B0(CWordArray* encode_buf) {
    encode_buf->RemoveAll();

    uint8_t max_x = this->map_max_x;
    uint8_t max_y = this->map_max_y;

    PosYX yx(8, 8);

    uint8_t cur = this->ObstacleAt(yx).TestBits(0x10);
    encode_buf->Add(cur);
    uint16_t run_len = 1;

    for (;;) {
        yx.val += 1;
        uint8_t bits = this->ObstacleAt(yx).TestBits(0x10);

        if (cur == bits) {
            run_len += 1;
        } else {
            encode_buf->Add(run_len);
            cur = bits;
            run_len = 1;
        }

        if (yx.x == max_x && yx.y == max_y) {
            encode_buf->Add(run_len);
            break;
        }

        if (yx.x == max_x) {
            yx.val = (yx.val & 0xFF00) + 0x107;
        }
    }
}

// 594076
bool MapStuff::IsInMapBounds(uint8_t x, uint8_t y) const {
    if (x < this->map_min_x || x > this->map_max_x) {
        return false;
    }
    if (y < this->map_min_y || y > this->map_max_y) {
        return false;
    }
    return true;
}

// 59190D
int MapStuff::sub_59190D(Unit* target, Unit* observer) {
    int32_t target_size = target->VMethod3();
    int32_t observer_size = observer->VMethod3();

    int32_t x1 = target->position->GetXx() + (target_size - 1) * 0x80;
    int32_t y1 = target->position->GetYy() + (target_size - 1) * 0x80;
    int32_t x2 = observer->position->GetXx() + (observer_size - 1) * 0x80;
    int32_t y2 = observer->position->GetYy() + (observer_size - 1) * 0x80;

    int32_t dx = abs(x1 - x2) - (target_size + observer_size) * 0x80;
    if (dx < 0) {
        dx = 0;
    }

    int32_t dy = abs(y1 - y2) - (target_size + observer_size) * 0x80;
    if (dy < 0) {
        dy = 0;
    }

    int32_t range = (std::max)(dx, dy);
    return (range >> 8) + 1;
}

// 597140
int MapStuff::sub_597140(Unit* unit, PosYX yx, int32_t flag) {
    int32_t size = unit->VMethod3();

    for (int32_t dx = 0; dx < size; dx++) {
        for (int32_t dy = 0; dy < size; dy++) {
            PosYX cell(yx.x + dx, yx.y + dy);
            bool blocked = (flag != 0)
                ? this->ObstacleAt(cell).TestBits(unit->eye->field5_0x5)
                : this->Obstacle2At(cell).TestBits(unit->eye->field5_0x5);
            if (blocked) {
                return 0;
            }
        }
    }

    return 1;
}

// 58AEEF
int32_t MapStuff::sub_58AEEF(Unit* unit, uint8_t x, uint8_t y) {
    PosYX yx(x, y);
    int32_t movement_type = unit->sub_59A030();

    CellState& cell = this->scratch_cell_state;

    if (movement_type != 0 && movement_type <= 2) {
        if (this->cell_states.Lookup(yx, cell)) {
            if (cell.spell_id != 0) {
                if (cell.spell_id == spell::teleport) {
                    unit->eye2->field81_0xb0 = PosYX{cell.teleport_x, cell.teleport_y}.val;
                } else {
                    // WAT: original code guards this call with a stub that always returns true here,
                    // so the else branch (SrvStru1::sub_4FBB24) is unreachable.
                    g_Server->srv_stru1->sub_4FBAE3(cell.spell_x, cell.spell_y, unit, cell.spell_id, cell.spell_damage);
                }
            }
            if (this->scratch_cell_state.small_unit != nullptr) {
                return 0;
            }
            cell.small_unit = unit;
            this->cell_states.SetAt(yx, cell);
            this->sub_58B593(yx);
            return 1;
        }

        this->FUN_0058b3e0(yx);
        if (!this->cell_states.Lookup(yx, cell)) {
            return 0;
        }
        cell.small_unit = unit;
        this->cell_states.SetAt(yx, cell);
        this->sub_58B593(yx);
        return 1;
    }

    if (movement_type == 3) {
        if (this->cell_states.Lookup(yx, cell)) {
            if (cell.large_unit != nullptr) {
                return 0;
            }
            cell.large_unit = unit;
            this->cell_states.SetAt(yx, cell);
            this->sub_58B593(yx);
            return 1;
        }

        this->FUN_0058b3e0(yx);
        if (!this->cell_states.Lookup(yx, cell)) {
            return 0;
        }
        cell.large_unit = unit;
        this->cell_states.SetAt(yx, cell);
        this->sub_58B593(yx);
        return 1;
    }

    return 0;
}

// 58AD4A
int MapStuff::sub_58AD4A(Unit* unit) {
    int32_t size = unit->VMethod3();

    uint8_t x = unit->position->GetX();
    uint8_t y = unit->position->GetY();

    unit->eye->field113_0x72 = this->sub_5913BD(unit, x, y);
    unit->eye->field122_0x82 = x;
    unit->eye->field123_0x83 = y;
    unit->eye->field124_0x84 = unit->position->GetXx();
    unit->eye->field125_0x85 = unit->position->GetYy();

    if (!this->sub_590F0A(unit)) {
        if (unit->eye->field121_0x80.val != unit->position->CompatGetYX()) {
            // WAT: Original code calls an empty no-op stub function here.
        }
    }

    for (int32_t dy = 0; dy < size; dy++) {
        for (int32_t dx = 0; dx < size; dx++) {
            if (!this->sub_58AEEF(unit, x + dx, y + dy)) {
                return 0;
            }
        }
    }

    this->FUN_005969c6(unit, unit->position->CompatGetYX(), 1);
    return 1;
}

void MapStuff::FUN_0058b5d6(CellState& cell)
{ //58b5d6

    PosYX pos = cell.cell_yx;
    Obstacle old = ObstacleAt(pos);

    Obstacle* ob1 = &ObstacleAt(pos);
    Obstacle* ob2 = &Obstacle2At(pos);

    *ob1 = cell.obstacle;
    WalkCostAt(pos) = cell.walk_cost;

    ob1->SetBits(0x20);
    *ob2 = *ob1;

    if (cell.static_blocker)
        ob2->SetBits(0x40);

    if (cell.dynamic_blocker)
        ob2->SetBits(0x80);

    if (cell.building)
    {
        TokenPos* bldpos = cell.building->position;
        if ((cell.building->unpassable_mask & (1 << (((pos.y - bldpos->GetY()) * cell.building->width) + pos.x - bldpos->GetX()))) == 0)
        {
            ob1->UnsetBits(1 | 4);
            ob2->UnsetBits(1 | 4);
            WalkCostAt(pos) = walk_cost[5];
        }
        else
        {
            ob1->SetBits(1 | 4);
            ob2->SetBits(1 | 4);
        }
    }

    for (int i = 0; i < 6; i++)
    {
        if (cell.area_effects[i])
        {
            if (true) //FUN_0059a050
                WalkCostAt(pos) *= 4;
        }
    }
    if (cell.area_effects[3])
    {
        ob1->SetBits(1 | 4);
        ob2->SetBits(1 | 4);
    }

    if (old.TestBits(0x10))
    {
        ob1->SetBits(0x10);
        ob2->SetBits(0x10);
    }
}

int MapStuff::FUN_0058b4a6(PosYX yx)
{ //58b4a6
    if (!cell_states.Lookup(yx.val, scratch_cell_state))
        return 0;

    Obstacle old = ObstacleAt(yx);
    WalkCostAt(yx) = scratch_cell_state.walk_cost;
    ObstacleAt(yx) = scratch_cell_state.obstacle;
    cell_states.RemoveKey(yx.val);

    if (old.TestBits(0x10))
    {
        ObstacleAt(yx).SetBits(0x10);
        Obstacle2At(yx).SetBits(0x10);
    }
    return 1;
}

void MapStuff::FUN_00596d3e(Unit* unit, PosYX yx)
{ //596d3e
    if (ObstacleAt(yx).TestBits(0x20) && cell_states.Lookup(yx.val, scratch_cell_state))
    {
        if ((unit->VMethod4() & 0xff) == 3)
        {
            if (scratch_cell_state.dynamic_blocker == unit)
                scratch_cell_state.dynamic_blocker = nullptr;
        }
        else
        {
            if (scratch_cell_state.static_blocker == unit)
                scratch_cell_state.static_blocker = nullptr;
        }

        FUN_0058b5d6(scratch_cell_state);
        cell_states[yx.val] = scratch_cell_state;
        if (scratch_cell_state.IsEmpty())
            FUN_0058b4a6(yx);
    }
}

void MapStuff::FUN_0058b3e0(PosYX yx)
{ //58b3e0
    if (!cell_states.Lookup(yx.val, scratch_cell_state))
    {
        scratch_cell_state.Null();
        scratch_cell_state.walk_cost = WalkCostAt(yx);
        scratch_cell_state.obstacle = ObstacleAt(yx);
        scratch_cell_state.cell_yx = yx;

        cell_states[yx.val] = scratch_cell_state;

        ObstacleAt(yx).SetBits(0x20);
    }
}

void MapStuff::FUN_00596e0e(Unit* unit, PosYX yx)
{ //596e0e
    if (ObstacleAt(yx).TestBits(0x20) == 0 || !cell_states.Lookup(yx.val, scratch_cell_state))
    {
        FUN_0058b3e0(yx);

        if (!cell_states.Lookup(yx.val, scratch_cell_state))
            return;
    }
    if ((unit->VMethod4() & 0xff) == 3)
    {
        if (scratch_cell_state.dynamic_blocker && scratch_cell_state.dynamic_blocker != unit)
            return;

        scratch_cell_state.dynamic_blocker = unit;
    }
    else
    {
        if (scratch_cell_state.static_blocker && scratch_cell_state.static_blocker != unit)
            return;

        scratch_cell_state.static_blocker = unit;
    }
        
    FUN_0058b5d6(scratch_cell_state);
    cell_states[yx.val] = scratch_cell_state;
}

void MapStuff::FUN_005969c6(Unit* unit, PosYX yx, uint8_t t)
{ //5969c6
    int sz = unit->VMethod3();
    field_0xa452c.fill(0);
    for (int j = 0; j < sz + 2; j++)
    {
        for (int i = 0; i < sz + 2; i++)
        {
            field_0xa452c(i, j) = 1;
        }
    }

    if (t == 0)
    {
        int dx = unit->position->GetX();
        int dy = unit->position->GetY();
        for (int j = 1; j < sz + 1; j++)
        {
            for (int i = 1; i < sz + 1; i++)
            {
                int x = j + yx.x - dx;
                int y = i + yx.y - dy;
                field_0xa452c(y, x) = 2; // y x for save order how it's in original
            }
        }
    }

    if (t != 2)
    {
        for (int j = 1; j < sz + 1; j++)
        {
            for (int i = 1; i < sz + 1; i++)
            {
                field_0xa452c(i, j) = 3;
            }
        }
    }


    int dx = unit->position->GetX() - 1;
    int dy = unit->position->GetY() - 1;
    for (int j = 0; j < sz + 2; j++)
    {
        for (int i = 0; i < sz + 2; i++)
        {
            switch (field_0xa452c(i, j))
            {
            case 0:
                break;

            case 1:
                FUN_00596d3e(unit, PosYX(dx + j, dy + i));
                break;

            case 2:
                FUN_00596e0e(unit, PosYX(dx + j, dy + i));
                break;

            case 3:
                FUN_00596e0e(unit, PosYX(dx + j, dy + i));
                break;
            }
        }
    }

    if (t == 0)
        unit->eye->field121_0x80 = yx;
    else if (t == 1)
        unit->eye->field121_0x80 = unit->position->GetYX();
    else if (t == 2)
        unit->eye->field121_0x80 = PosYX(0, 0);
}




// 5955B5
ScanPresenceGrid::ScanPresenceGrid() {
    memset(this->control, 0, sizeof(this->control));
}

// 596047
void ScanPresenceGrid::sub_596047(AreaEffect* ae) {
    int32_t x_sec = (ae->position->GetX() >> 3) + 1;
    int32_t y_sec = (ae->position->GetY() >> 3) + 1;
    ae->field4_0x42 = (uint16_t)this->sector_grid[x_sec + 1][y_sec + 1];
    ae->field4_0x42 |= (uint16_t)this->sector_grid[x_sec + 1][y_sec - 1];
    ae->field4_0x42 |= (uint16_t)this->sector_grid[x_sec - 1][y_sec + 1];
    ae->field4_0x42 |= (uint16_t)this->sector_grid[x_sec - 1][y_sec - 1];
}

// 5957E2
void ScanPresenceGrid::sub_5957E2(Unit* unit) {
    if (!unit->pOwner->is_ai) {
        int32_t x_sec = (unit->position->GetX() >> 3) + 1;
        int32_t y_sec = (unit->position->GetY() >> 3) + 1;
        for (int32_t dx = -2; dx <= 2; dx++) {
            for (int32_t dy = -2; dy <= 2; dy++) {
                this->sector_grid[x_sec + dx][y_sec + dy] |= unit->pOwner->vision_sharing_mask;
            }
        }
    } else if (unit->hp < unit->hp_max) {
        unit->group->group_sub->field_0x45 = 1;
    }
}

// 595F46
void ScanPresenceGrid::sub_595F46(Unit* unit) {
    uint32_t sector = this->sector_grid[(unit->position->GetX() >> 3) + 1][(unit->position->GetY() >> 3) + 1];
    if (sector != 0) {
        unit->group->group_sub->field_0x45 = 1;
        this->num_detected++;
        unit->field_0x1a4 = (uint16_t)sector;
    } else if (unit->group->group_sub->active != 0) {
        unit->group->group_sub->field_0x45 = 1;
        this->num_detected++;
        this->field_0x1624++;
    }
}

// 5963B9
void ScanPresenceGrid::sub_5963B9() {
    memset(this->sector_grid, 0, sizeof(this->sector_grid));
    POSITION player_it = g_PlayersList->list.GetHeadPosition();
    while (player_it != NULL) {
        Player* player = g_PlayersList->list.GetNext(player_it);
        if (player->is_ai) {
            POSITION group_it = player->group_list->groups.GetHeadPosition();
            while (group_it != NULL) {
                Group* group = player->group_list->groups.GetNext(group_it);
                group->group_sub->field_0x45 = 0;
            }
        }
    }
    if (dword_6CDB3C != nullptr) {
        POSITION it = dword_6CDB3C->unit_list.GetHeadPosition();
        while (it != NULL) {
            Unit* unit = dword_6CDB3C->unit_list.GetNext(it);
            unit->field_0x1a4 = 0;
        }
    }
}

// 596131
void ScanPresenceGrid::sub_596131() {
    this->sub_5963B9();
    this->num_detected = 0;
    this->field_0x1624 = 0;

    POSITION unit_it = this->unit_list->unit_list.GetHeadPosition();
    while (unit_it != NULL) {
        Unit* unit = this->unit_list->unit_list.GetNext(unit_it);
        this->sub_5957E2(unit);
    }

    POSITION player_it = g_PlayersList->list.GetHeadPosition();
    while (player_it != NULL) {
        Player* player = g_PlayersList->list.GetNext(player_it);
        if (!player->is_ai && player->main_unit != nullptr) {
            this->sub_5957E2(player->main_unit);
        }
    }

    unit_it = this->unit_list->unit_list.GetHeadPosition();
    while (unit_it != NULL) {
        Unit* unit = this->unit_list->unit_list.GetNext(unit_it);
        this->sub_595F46(unit);
    }

    unit_it = g_Server->srv_stru1->units_list->unit_list.GetHeadPosition();
    while (unit_it != NULL) {
        Unit* unit = g_Server->srv_stru1->units_list->unit_list.GetNext(unit_it);
        uint32_t sector = this->sector_grid[(unit->position->GetX() >> 3) + 1][(unit->position->GetY() >> 3) + 1];
        if (sector != 0) {
            unit->field_0x1a4 = (uint16_t)sector;
        }
    }

    POSITION effect_it = g_Server->srv_stru1->effects_list->list.GetHeadPosition();
    while (effect_it != NULL) {
        SpellEffect* effect = g_Server->srv_stru1->effects_list->list.GetNext(effect_it);
        this->sub_596047(static_cast<AreaEffect*>(effect));
    }

    this->scan_delta = this->unit_list->unit_list.GetCount() - this->num_detected;
}




// 599FF0
FieldBlock54190::FieldBlock54190() {
    this->field_0x0 = 0;
    this->field_0x4 = 0;
    this->data[1000] = 2;
}

extern "C" {
    extern uint8_t unk_70B46C[4];
    extern uint8_t unk_70B470[4];
    extern uint8_t unk_70B474[8];
}

// 5A3180
FieldBlock5859c::FieldBlock5859c() {
    this->field_0x0 = unk_70B46C;
    this->field_0x4 = unk_70B470;
    this->field_0x8 = unk_70B474;
    this->field_0xc = 0;
}

int CellState::IsEmpty() const
{ //58bef6
    if (small_unit != nullptr ||
        large_unit != nullptr ||
        building != nullptr ||
        sack != nullptr ||
        effect_count != 0 ||
        spell_id != 0 ||
        static_blocker != nullptr ||
        dynamic_blocker != nullptr)
        return 0;
    return 1;
}

void CellState::Null()
{ //58a3da
    walk_cost = 0;
    obstacle = 0;
    effect_count = 0;
    gap_0x03 = 0;
    small_unit = nullptr;
    large_unit = nullptr;
    building = nullptr;
    sack = nullptr;
    area_effects.fill(nullptr);
    static_blocker = nullptr;
    dynamic_blocker = nullptr;
    spell_id = 0;
    spell_damage = 0;
    spell_x = 0;
    spell_y = 0;
    teleport_x = 0;
    teleport_y = 0;
    cell_yx = 0;
}

// 59423F
void CellState::sub_59423F() {
    void* remapped;

    if (this->small_unit != nullptr && g_Server->field23_0xdc.Lookup(this->small_unit, remapped)) {
        this->small_unit = (Unit*)remapped;
    }
    if (this->large_unit != nullptr && g_Server->field23_0xdc.Lookup(this->large_unit, remapped)) {
        this->large_unit = (Unit*)remapped;
    }
    if (this->building != nullptr && g_Server->field23_0xdc.Lookup(this->building, remapped)) {
        this->building = (Building*)remapped;
    }
    if (this->sack != nullptr && g_Server->field23_0xdc.Lookup(this->sack, remapped)) {
        this->sack = (Sack*)remapped;
    }
    if (this->static_blocker != nullptr && g_Server->field23_0xdc.Lookup(this->static_blocker, remapped)) {
        this->static_blocker = (Unit*)remapped;
    }
    if (this->dynamic_blocker != nullptr && g_Server->field23_0xdc.Lookup(this->dynamic_blocker, remapped)) {
        this->dynamic_blocker = (Unit*)remapped;
    }
    for (int32_t i = 0; i < 6; i++) {
        if (this->area_effects[i] != nullptr && g_Server->field23_0xdc.Lookup(this->area_effects[i], remapped)) {
            this->area_effects[i] = (AreaEffect*)remapped;
        }
    }
}



void __cdecl MissionGetBriefing(CString* out)
{ //4e13db
    out->Empty();
    int pos = g_MissionText.Find("#briefing");
    if (pos != -1)
    {
        CString txt = g_MissionText.Mid(pos + 11);
        pos = txt.Find('#');
        if (pos == -1)
            pos = txt.GetLength();

        *out = txt.Left(pos);
    }
}

void __cdecl MissionGetFailure(int32_t idx, CString* out)
{ //4e14a4
    out->Empty();

    CString str;
    str.Format("#failure%d", idx);

    int pos = g_MissionText.Find(str);
    if (pos != -1)
    {
        CString txt = g_MissionText.Mid(pos + str.GetLength() + 2);
        pos = txt.Find('#');
        if (pos == -1)
            pos = txt.GetLength();

        *out = txt.Left(pos);
    }
}

void __cdecl MissionGetTips(int32_t idx, CString* out)
{ //4e159b
    out->Empty();

    CString str;
    str.Format("#tips%d", idx);

    int pos = g_MissionText.Find(str);
    if (pos != -1)
    {
        CString txt = g_MissionText.Mid(pos + str.GetLength() + 2);
        pos = txt.Find('#');
        if (pos == -1)
            pos = txt.GetLength();

        *out = txt.Left(pos);
    }
}

void __cdecl MissionGetSubj(int32_t idx, CString* out)
{ //4e16ce
    out->Empty();

    CString str;
    str.Format("#subobjective%d", idx);

    int pos = g_MissionText.Find(str);
    if (pos != -1)
    {
        CString txt = g_MissionText.Mid(pos + str.GetLength() + 2);
        pos = txt.Find('#');
        if (pos == -1)
            pos = txt.GetLength();

        *out = txt.Left(pos);
    }
}

void __cdecl MissionGetLocName(int32_t t, int32_t idx, CString* out)
{ //4e17e3
    out->Empty();

    CString str;
    if (t == 1)
        str.Format("#title%d", idx);
    else
        str.Format("#town%d", idx);

    int pos = g_MissionText.Find(str);
    if (pos != -1)
    {
        CString txt = g_MissionText.Mid(pos + str.GetLength() + 2);
        pos = txt.Find('#');
        if (pos == -1)
            pos = txt.GetLength();

        *out = txt.Left(pos);
    }
}


void __cdecl MissionGetDescription(int32_t t, int32_t idx, CString* out)
{ //4e1915
    out->Empty();

    CString str;
    if (t == 1)
        str.Format("#briefing%d", idx);
    else
        str.Format("#description%d", idx);

    int pos = g_MissionText.Find(str);
    if (pos != -1)
    {
        CString txt = g_MissionText.Mid(pos + str.GetLength() + 2);
        pos = txt.Find('#');
        if (pos == -1)
            pos = txt.GetLength();

        *out = txt.Left(pos);
    }
}

// 58CA1B
Unit* MapStuff::sub_58CA1B(PosYX yx) {
    if (!this->ObstacleAt(yx).TestBits(0x20)) {
        return nullptr;
    }
    if (!this->cell_states.Lookup(yx.val, this->scratch_cell_state)) {
        return nullptr;
    }
    return this->scratch_cell_state.small_unit;
}

// 5897AA
UnitList* MapStuff::sub_5897AA(PosYX yx, uint8_t range) {
    this->field9_0x5400c.unit_list.RemoveAll();

    if (this->ObstacleAt(yx).TestBits(0x20)) {
        Unit* unit = this->sub_58CA1B(yx);
        if (unit != nullptr) {
            this->field9_0x5400c.AddTail(unit);
        }
    }

    for (uint8_t ring = 1; ring < range + 1; ring++) {
        for (int32_t offset = -(int32_t)ring; offset < ring + 1; offset++) {
            uint16_t cell = (ring << 8) + offset + yx.val;
            if (this->obstacle_map[cell].TestBits(0x20)) {
                Unit* unit = this->sub_58CA1B(cell);
                if (unit != nullptr) {
                    this->field9_0x5400c.AddTail(unit);
                }
            }

            cell = yx.val - (ring << 8) + offset;
            if (this->obstacle_map[cell].TestBits(0x20)) {
                Unit* unit = this->sub_58CA1B(cell);
                if (unit != nullptr) {
                    this->field9_0x5400c.AddTail(unit);
                }
            }

            if (std::abs(offset) != ring) {
                cell = yx.val + (offset << 8) + ring;
                if (this->obstacle_map[cell].TestBits(0x20)) {
                    Unit* unit = this->sub_58CA1B(cell);
                    if (unit != nullptr) {
                        this->field9_0x5400c.AddTail(unit);
                    }
                }

                cell = yx.val + (offset << 8) - ring;
                if (this->obstacle_map[cell].TestBits(0x20)) {
                    Unit* unit = this->sub_58CA1B(cell);
                    if (unit != nullptr) {
                        this->field9_0x5400c.AddTail(unit);
                    }
                }
            }
        }
    }

    return &this->field9_0x5400c;
}

// 58CB5A
Unit* MapStuff::sub_58CB5A(uint16_t yx) {
    if (!this->ObstacleAt(yx).TestBits(0x20)) {
        return nullptr;
    }
    if (!this->cell_states.Lookup(yx, this->scratch_cell_state)) {
        return nullptr;
    }
    return this->scratch_cell_state.small_unit;
}

// 58CBB9
Unit* MapStuff::sub_58CBB9(uint16_t yx) {
    if (!this->ObstacleAt(yx).TestBits(0x20)) {
        return nullptr;
    }
    if (!this->cell_states.Lookup(yx, this->scratch_cell_state)) {
        return nullptr;
    }
    return this->scratch_cell_state.large_unit;
}

// 58826D
void MapStuff::sub_58826D(Unit* unit, uint8_t x, uint8_t y, int32_t flag, Unit* target) {
    uint8_t cur_x = unit->position->GetX();
    uint8_t cur_y = unit->position->GetY();
    this->sub_5882AE(unit, cur_x, cur_y, x, y, flag, target);
}

// 58B27F
int MapStuff::sub_58B27F(Unit* unit, uint8_t x, uint8_t y) {
    PosYX yx(x, y);
    if (!this->cell_states.Lookup(yx.val, this->scratch_cell_state)) {
        return 0;
    }

    uint32_t type = unit->VMethod4();
    if (type == 1 || type == 2) {
        if (!this->scratch_cell_state.small_unit) {
            return 0;
        }
        this->scratch_cell_state.small_unit = nullptr;
    } else if (type == 3) {
        if (!this->scratch_cell_state.large_unit) {
            return 0;
        }
        this->scratch_cell_state.large_unit = nullptr;
    }

    this->cell_states[yx.val] = this->scratch_cell_state;
    this->sub_58B593(yx);
    if (this->scratch_cell_state.IsEmpty()) {
        this->FUN_0058b4a6(yx);
    }

    unit->eye->field126_0x86 = unit->position->GetX();
    unit->eye->field127_0x87 = unit->position->GetY();
    unit->eye->field128_0x88 = unit->position->GetXx();
    unit->eye->field129_0x89 = unit->position->GetYy();
    return 1;
}

// 58B1D7
int MapStuff::sub_58B1D7(Unit* unit) {
    int32_t size = unit->VMethod3() & 0xFF;
    uint8_t x = unit->position->GetX();
    uint8_t y = unit->position->GetY();

    for (int32_t i = 0; i < size; i++) {
        for (int32_t j = 0; j < size; j++) {
            if (!this->sub_58B27F(unit, x + j, y + i)) {
                return 0;
            }
        }
    }
    return 1;
}

// 58B593
void MapStuff::sub_58B593(PosYX yx) {
    if (this->cell_states.Lookup(yx.val, this->scratch_cell_state)) {
        this->FUN_0058b5d6(this->scratch_cell_state);
    }
}

// 58E611
Sack* MapStuff::sub_58E611(uint16_t yx) {
    if (!this->ObstacleAt(yx).TestBits(0x20)) {
        return nullptr;
    }
    if (!this->cell_states.Lookup(yx, this->scratch_cell_state)) {
        return nullptr;
    }
    return this->scratch_cell_state.sack;
}

// 59449A
int MapStuff::sub_59449A(Building* building, PosYX yx) {
    if (!this->cell_states.Lookup(yx.val, this->scratch_cell_state)) {
        return 0;
    }

    this->scratch_cell_state.building = nullptr;
    this->cell_states[yx.val] = this->scratch_cell_state;
    this->sub_58B593(yx);
    if (this->scratch_cell_state.IsEmpty()) {
        this->FUN_0058b4a6(yx);
    }
    return 1;
}

// 594A2D --- collect passable sack cells in expanding rings around the unit, up to its scan range
void MapStuff::sub_594A2D(Unit* unit, int32_t flag) {
    uint16_t yx = unit->position->CompatGetYX();
    uint8_t range = unit->scan_range >> 8;
    this->field67_0xa4510.RemoveAll();
    if (this->sub_58E611(yx) != nullptr) {
        this->field67_0xa4510.AddHead(yx);
        if (flag != 0) {
            return;
        }
    }

    for (uint8_t ring = 1; ring < range + 1; ring++) {
        for (int8_t offset = -ring; offset <= ring; offset++) {
            uint16_t pos = yx + (ring << 8) + offset;
            if (this->sub_594C4B(unit, pos) != 0) {
                this->field67_0xa4510.AddHead(pos);
            }
            pos = yx - (ring << 8) + offset;
            if (this->sub_594C4B(unit, pos) != 0) {
                this->field67_0xa4510.AddHead(pos);
            }
            if (abs(offset) != ring) {
                pos = yx + (offset << 8) + ring;
                if (this->sub_594C4B(unit, pos) != 0) {
                    this->field67_0xa4510.AddHead(pos);
                }
                pos = yx + (offset << 8) - ring;
                if (this->sub_594C4B(unit, pos) != 0) {
                    this->field67_0xa4510.AddHead(pos);
                }
            }
            if (flag != 0 && this->field67_0xa4510.GetCount() != 0) {
                return;
            }
        }
    }
}

// 594C4B --- check whether cell yx holds a sack and is passable for unit
int32_t MapStuff::sub_594C4B(Unit* unit, uint16_t yx) {
    if (this->sub_58E611(yx) != nullptr && this->sub_5978F0(unit, yx) != 0) {
        return 1;
    }
    return 0;
}

// 595468
int MapStuff::sub_595468(Unit* unit, PosYX yx) {
    if (!this->sub_597140(unit, yx, 0)) {
        return 0;
    }
    if (!this->sub_597140(unit, yx, 1)) {
        return 0;
    }
    return 1;
}

// 58E3D1 --- add unit to map
int MapStuff::sub_58E3D1(Unit* unit) {
    this->FUN_005969c6(unit, PosYX(0, 0), 2);
    return this->sub_58B1D7(unit) != 0;
}

// 58E525 --- remove a sack token from the map
void MapStuff::sub_58E525(Sack* sack) {
    PosYX yx = sack->position->CompatGetYX();
    if (this->cell_states.Lookup(yx.val, this->scratch_cell_state)) {
        this->scratch_cell_state.sack = nullptr;
        this->cell_states[yx.val] = this->scratch_cell_state;
        this->sub_58B593(yx);
        if (!this->scratch_cell_state.IsEmpty()) {
            this->FUN_0058b4a6(yx);
        }
    }
}

// 58E5C7 --- look up sack at map position
Sack* MapStuff::sub_58E5C7(uint16_t param_2, uint16_t param_3) {
    PosYX yx((uint8_t)param_2, (uint8_t)param_3);
    return this->sub_58E611(yx.val);
}

// 58E5F3
Sack* MapStuff::sub_58E5F3(TokenPos* pos) {
    return this->sub_58E611(pos->CompatGetYX());
}

// 590F0A --- check if unit can be placed at current position
int MapStuff::sub_590F0A(Unit* unit) {
    PosYX yx = unit->position->CompatGetYX();
    if (!this->sub_597140(unit, yx, 0)) {
        return 0;
    }
    if (!this->sub_597140(unit, yx, 1)) {
        return 0;
    }
    return 1;
}

// 5918B8
void MapStuff::sub_5918B8(Unit* unit, Unit* target) {
    uint8_t facing = this->sub_591424(unit, target);
    this->sub_590F94(unit, facing);
}

// 5918E2
void MapStuff::sub_5918E2(Unit* unit, PosYX yx) {
    uint8_t facing = this->sub_59166C(unit, yx);
    this->sub_590F94(unit, facing);
}

// 593AA4
uint16_t MapStuff::sub_593AA4(Unit* unit) {
    if (unit->list1.IsEmpty()) {
        return 0;
    }
    PosYX tail = unit->list1.GetHead();
    uint8_t x = unit->position->GetX();
    uint8_t y = unit->position->GetY();
    if (this->sub_593B29(PosYX(x, y), tail) == 1) {
        return tail.val;
    }
    return 0;
}

// 593B29 --- Chebyshev distance between two YX positions
uint8_t MapStuff::sub_593B29(PosYX yx1, PosYX yx2) {
    uint8_t dx = abs(yx1.x - yx2.x);
    uint8_t dy = abs(yx1.y - yx2.y);
    return (std::max)(dx, dy);
}

// 58AB48 --- serialize obstacle/cell state
void MapStuff::sub_58AB48(CArchive& ar) {
    CList<uint32_t> obstacle_diffs;

    if (ar.IsStoring()) {
        for (uint32_t yx = 0x807; yx < 0xEDEE; yx++) {
            uint8_t obstacle2 = this->Obstacle2At(yx);
            if (obstacle2 > 0xF) {
                uint8_t obstacle = this->ObstacleAt(yx);
                uint32_t packed = (((yx << 8) + obstacle2) << 8) + obstacle;
                obstacle_diffs.AddTail(packed);
            }
        }
        obstacle_diffs.Serialize(ar);
        this->cell_states.Serialize(ar);
        ar << (DWORD)this;
    }

    if (ar.IsLoading()) {
        obstacle_diffs.Serialize(ar);
        this->cell_states.Serialize(ar);

        DWORD key;
        ar >> key;
        g_Server->field23_0xdc.SetAt((void*)key, this);

        POSITION pos = obstacle_diffs.GetHeadPosition();
        while (pos != nullptr) {
            uint32_t packed = obstacle_diffs.GetNext(pos);
            uint16_t yx = (uint16_t)(packed >> 16);
            this->Obstacle2At(yx) = (uint8_t)(packed >> 8);
            this->ObstacleAt(yx) = (uint8_t)packed;
        }
    }
}

// 594125 --- post-load map relink helper: remaps stale on-disk pointers in
// each cell_states entry via g_Server's pointer remap table (sub_59423F).
void MapStuff::sub_594125() {
    POSITION pos = this->cell_states.GetStartPosition();
    while (pos != nullptr) {
        uint16_t key;
        CellState val;
        this->cell_states.GetNextAssoc(pos, key, val);
        val.sub_59423F();
        this->cell_states.SetAt(key, val);
    }
}

// 5943D0 --- set building pointer in cell state at yx
int MapStuff::sub_5943D0(Building* building, uint16_t yx) {
    if (this->cell_states.Lookup(yx, this->scratch_cell_state)) {
        if (this->scratch_cell_state.building != nullptr) {
            return 0;
        }
        this->scratch_cell_state.building = building;
        this->cell_states[yx] = this->scratch_cell_state;
        return 1;
    }

    this->FUN_0058b3e0(yx);
    if (!this->cell_states.Lookup(yx, this->scratch_cell_state)) {
        return 0;
    }
    this->scratch_cell_state.building = building;
    this->cell_states[yx] = this->scratch_cell_state;
    this->sub_58B593(yx);
    return 1;
}

// 5945EF --- remove building from map
int32_t MapStuff::sub_5945EF(Building* building) {
    int bit_index = 0;
    for (int row = 0; row < building->height; row++) {
        for (int col = 0; col < building->width; col++) {
            if (building->tiles_mask & (1u << bit_index)) {
                uint8_t x = building->position->GetX() + col;
                uint8_t y = building->position->GetY() + row;
                if (!this->sub_59449A(building, PosYX(x, y))) {
                    return 0;
                }
            }
            bit_index++;
        }
    }
    return 1;
}

// 59451F --- place building on the map
int32_t MapStuff::sub_59451F(Building* building) {
    int bit_index = 0;
    for (int row = 0; row < building->height; row++) {
        for (int col = 0; col < building->width; col++) {
            if (building->tiles_mask & (1u << bit_index)) {
                uint8_t x = building->position->GetX() + col;
                uint8_t y = building->position->GetY() + row;
                if (!this->sub_5943D0(building, PosYX(x, y).val)) {
                    return 0;
                }
            }
            bit_index++;
        }
    }
    return 1;
}

// 594EBA --- register area effect into cell state at yx
int MapStuff::sub_594EBA(AreaEffect* ae, uint16_t yx) {
    if (!this->cell_states.Lookup(yx, this->scratch_cell_state)) {
        this->FUN_0058b3e0(yx);
        if (!this->cell_states.Lookup(yx, this->scratch_cell_state)) {
            return 0;
        }
    }
    uint8_t idx = ae->sub_538897();
    this->scratch_cell_state.area_effects[idx] = ae;
    this->scratch_cell_state.effect_count = 0;
    for (int32_t i = 0; i < 6; i++) {
        if (this->scratch_cell_state.area_effects[i] != nullptr) {
            this->scratch_cell_state.effect_count++;
        }
    }
    this->cell_states[yx] = this->scratch_cell_state;
    this->sub_58B593(yx);
    return 1;
}

// 595073 --- remove area effect from cell state at yx
int MapStuff::sub_595073(AreaEffect* ae, uint16_t yx) {
    if (!this->cell_states.Lookup(yx, this->scratch_cell_state)) {
        return 0;
    }
    uint8_t idx = ae->sub_538897();
    if (this->scratch_cell_state.area_effects[idx] != ae) {
        return 0;
    }
    this->scratch_cell_state.area_effects[idx] = nullptr;
    this->scratch_cell_state.effect_count = 0;
    for (int32_t i = 0; i < 6; i++) {
        if (this->scratch_cell_state.area_effects[i] != nullptr) {
            this->scratch_cell_state.effect_count++;
        }
    }
    this->cell_states[yx] = this->scratch_cell_state;
    this->sub_58B593(yx);
    if (this->scratch_cell_state.IsEmpty()) {
        this->FUN_0058b4a6(yx);
    }
    return 1;
}

// 59536C --- get pointer to area_effects[6] array at cell yx
AreaEffect** MapStuff::sub_59536C(uint32_t yx) {
    uint16_t key = (uint16_t)yx;
    if (!this->ObstacleAt(key).TestBits(0x20)) {
        return nullptr;
    }
    if (!this->cell_states.Lookup(key, this->scratch_cell_state)) {
        return nullptr;
    }
    return this->scratch_cell_state.area_effects.data();
}

// 5953CB --- check if area effect covers cell coord
AreaEffect* MapStuff::sub_5953CB(AreaEffect* ae, uint16_t coord) {
    if (!this->ObstacleAt(coord).TestBits(0x20)) {
        return nullptr;
    }
    if (!this->cell_states.Lookup(coord, this->scratch_cell_state)) {
        return nullptr;
    }
    uint8_t idx = ae->sub_538897();
    return this->scratch_cell_state.area_effects[idx];
}

// 595438 --- check if area effect can be applied at (x, y)
int MapStuff::sub_595438(AreaEffect* ae, uint8_t x, uint8_t y) {
    return this->sub_5953CB(ae, PosYX(x, y).val) != nullptr;
}

// 595561 --- smallest angular difference between two facing bytes.
uint8_t MapStuff::FacingDiff(uint8_t a, uint8_t b) {
    uint8_t diff = std::abs(a - b);
    if (diff > 0x80) {
        diff = 0x100 - diff;
    }
    return diff;
}

// 5954AC --- teleport unit to (x, y)
void MapStuff::sub_5954AC(Unit* unit, uint8_t x, uint8_t y) {
    PosYX yx(x, y);
    if (!this->sub_595468(unit, yx)) {
        return;
    }
    this->sub_58E3D1(unit);
    unit->position->SetCoords(x, y);
    this->sub_58AD4A(unit);
    this->field41_0x58d80->sub_5A9A6A(unit);
    g_NetStru1_main.sub_519221(unit, nullptr, 0xFFFFFFFF, 0xFFB, 0, 0);
}

// Shared by sub_597BB0/sub_5983D0/sub_598BF0/sub_599410.
void MapStuff::PropagateNeighborCost(uint16_t yx, uint8_t flag_byte, int32_t cond_flag, Obstacle* obstacle_map, uint16_t& queue_tail, uint16_t* queue) {
    static const int32_t offsets[8] = { -0x101, -0xFF, 0xFF, 0x101, -0x100, -1, 1, 0x100 };
    uint16_t source_cost = this->field3_0x30000[yx];

    for (int32_t i = 0; i < 8; i++) {
        bool diagonal = i < 4;
        uint16_t neighbor = yx + offsets[i];

        if (obstacle_map[neighbor].TestBits(flag_byte)) {
            continue;
        }

        uint16_t new_cost;
        if (cond_flag != 0) {
            new_cost = source_cost + this->walk_cost_map[neighbor];
            if (diagonal) {
                new_cost += this->walk_cost_map[neighbor] / 2;
            }
        } else {
            new_cost = source_cost + (diagonal ? 3 : 2);
        }

        if (new_cost < this->field3_0x30000[neighbor]) {
            this->field3_0x30000[neighbor] = new_cost;
            queue[queue_tail++] = neighbor;
        }
    }
}

// 597BB0
void MapStuff::sub_597BB0(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag) {
    this->PropagateNeighborCost(yx, flag_byte, cond_flag, this->obstacle_map, this->field8_0x5400a, this->field_0x56584);
}

// 5983D0
void MapStuff::sub_5983D0(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag) {
    this->PropagateNeighborCost(yx, flag_byte, cond_flag, this->obstacle_map, this->field7_0x54008, this->field_0x54584);
}

// 598BF0
void MapStuff::sub_598BF0(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag) {
    this->PropagateNeighborCost(yx, flag_byte, cond_flag, this->obstacle_map2, this->field8_0x5400a, this->field_0x56584);
}

// 599410
void MapStuff::sub_599410(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag) {
    this->PropagateNeighborCost(yx, flag_byte, cond_flag, this->obstacle_map2, this->field7_0x54008, this->field_0x54584);
}

// 5882AE --- pathfinding BFS from (cur_x, cur_y) to (x, y)
void MapStuff::sub_5882AE(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y, int32_t flag, Unit* target) {
    PosYX dest(x, y);
    int32_t unit_size = unit->VMethod3();

    if (flag != 0) {
        unit->list1.RemoveAll();
    }

    if (cur_x == x && cur_y == y) {
        return;
    }

    const uint16_t distance = this->sub_58BFA3(cur_x, cur_y, x, y);

    // Seed the BFS work queue with the unit's current tile.
    this->field7_0x54008 = 0;
    this->field8_0x5400a = 0;
    this->field_0x50008[0] = cur_x;
    this->field_0x50008[0x1000] = cur_y;
    this->field7_0x54008 = 1;

    uint16_t ring = 0;

    if (flag == 0) {
        this->FUN_005969c6(unit, unit->position->CompatGetYX(), 2);

        if (unit_size == 1) {
            this->field_0x54584[0] = PosYX(cur_x, cur_y);
            uint8_t flag_byte = unit->eye->field5_0x5;
            int32_t cond_flag = (unit->sub_59A030() == 1) ? 1 : 0;

            int32_t extra = distance >> 2;
            if (extra < this->dynamic_scan_ahead) {
                extra = this->dynamic_scan_ahead;
            }
            uint16_t ring_limit = distance + extra;

            int32_t cur_packed = PosYX(cur_x, cur_y).val;
            int32_t half_width = (ring_limit << 8) + ring_limit + 0x200;
            int32_t lo = cur_packed - half_width;
            if (lo < 0) {
                lo = 0;
            }
            int32_t hi = cur_packed + half_width;
            if ((uint32_t)hi >= 0x10000) {
                hi = 0x10000;
            }
            memset(this->field3_0x30000 + lo, 0xFF, (hi - lo) * 2);
            *this->sub_59A670(cur_x, cur_y) = 0;

            while (this->field3_0x30000[dest.val] == 0xFFFF && this->field7_0x54008 != 0 && ring < ring_limit) {
                this->field8_0x5400a = 0;
                for (int i = 0; i < this->field7_0x54008; i++) {
                    this->sub_598BF0(unit, this->field_0x54584[i], flag_byte, cond_flag);
                }
                this->field7_0x54008 = 0;
                for (int i = 0; i < this->field8_0x5400a; i++) {
                    this->sub_599410(unit, this->field_0x56584[i], flag_byte, cond_flag);
                }
                ring += 2;
            }
        } else {
            memset(this->field3_0x30000, 0xFF, 0x20000);
            *this->sub_59A670(cur_x, cur_y) = 0;

            while (*this->sub_59A670(x, y) == 0xFFFF && this->field7_0x54008 != 0 && ring < distance + this->dynamic_scan_ahead) {
                this->field8_0x5400a = 0;
                for (int i = 0; i < this->field7_0x54008; i++) {
                    this->sub_5976D0(unit, this->field_0x50008[i], this->field_0x50008[0x1000 + i]);
                }
                this->field7_0x54008 = 0;
                for (int i = 0; i < this->field8_0x5400a; i++) {
                    this->sub_597990(unit, this->field_0x50008[0x2000 + i], this->field_0x50008[0x3000 + i]);
                }
                ring += 2;
            }
        }

        this->FUN_005969c6(unit, unit->position->CompatGetYX(), 1);
    } else if (unit_size == 1) {
        this->field_0x54584[0] = PosYX(cur_x, cur_y);
        uint8_t flag_byte = unit->eye->field5_0x5;
        int32_t cond_flag = (unit->sub_59A030() == 1) ? 1 : 0;

        int32_t extra = distance >> 2;
        if (extra < this->static_scan_ahead) {
            extra = this->static_scan_ahead;
        }
        int32_t ring_limit = distance + extra;

        if (unit->pOwner->is_ai == 0 && this->sub_596F80(unit, dest.val) != 0) {
            ring_limit = 1000;
            memset(this->field3_0x30000, 0xFF, 0x20000);
            *this->sub_59A670(cur_x, cur_y) = 0;
        } else {
            int32_t cur_packed = PosYX(cur_x, cur_y).val;
            int32_t half_width = (ring_limit << 8) + ring_limit + 0x200;
            int32_t lo = cur_packed - half_width;
            if (lo < 0) {
                lo = 0;
            }
            int32_t hi = cur_packed + half_width;
            if ((uint32_t)hi >= 0x10000) {
                hi = 0x10000;
            }
            memset(this->field3_0x30000 + lo, 0xFF, (hi - lo) * 2);
            *this->sub_59A670(cur_x, cur_y) = 0;
        }

        while (this->field3_0x30000[dest.val] == 0xFFFF && this->field7_0x54008 != 0 && (int32_t)ring < ring_limit) {
            this->field8_0x5400a = 0;
            for (int i = 0; i < this->field7_0x54008; i++) {
                this->sub_597BB0(unit, this->field_0x54584[i], flag_byte, cond_flag);
            }
            this->field7_0x54008 = 0;
            for (int i = 0; i < this->field8_0x5400a; i++) {
                this->sub_5983D0(unit, this->field_0x56584[i], flag_byte, cond_flag);
            }
            ring += 2;
        }
    } else {
        memset(this->field3_0x30000, 0xFF, 0x20000);
        *this->sub_59A670(cur_x, cur_y) = 0;

        while (*this->sub_59A670(x, y) == 0xFFFF && this->field7_0x54008 != 0 && ring < distance + this->static_scan_ahead) {
            this->field8_0x5400a = 0;
            for (int i = 0; i < this->field7_0x54008; i++) {
                this->sub_597290(unit, this->field_0x50008[i], this->field_0x50008[0x1000 + i]);
            }
            this->field7_0x54008 = 0;
            for (int i = 0; i < this->field8_0x5400a; i++) {
                this->sub_5974B0(unit, this->field_0x50008[0x2000 + i], this->field_0x50008[0x3000 + i]);
            }
            ring += 2;
        }
    }

    if (*this->sub_59A670(x, y) == 0xFFFF) {
        if (flag == 0) {
            uint16_t result_yx;
            if (target == nullptr) {
                result_yx = this->sub_593134(unit, 0, dest.val, 8);
            } else {
                result_yx = this->sub_592A48(unit, target);
            }
            if (result_yx != 0) {
                PosYX result(result_yx);
                this->sub_5893C6(unit, cur_x, cur_y, result.x, result.y);
            }
        } else {
            uint16_t step = this->sub_593134(unit, 0, dest.val, (distance >> 2) + 4);
            Building* reached = this->sub_594709(dest.val);
            if (reached == nullptr) {
                if (this->sub_593B29(PosYX(step), dest) > 1) {
                    g_NetStru1_main.FUN_0051ce86(1, 0, unit->pOwner);
                }
            } else {
                if (this->sub_593B29(PosYX(step), dest) > 2) {
                    g_NetStru1_main.FUN_0051ce86(1, 0, unit->pOwner);
                }
            }

            if (unit->group->group_sub->field_0x20 == 0x12 && unit->eye2->cast_action == 5) {
                int32_t size = unit->VMethod3();
                uint8_t range = this->sub_591A96(step, dest.val, size);
                if (this->field41_0x58d80->UnitMaxRange(unit) < range) {
                    unit->eye2->tick16 = g_Server->tick16;
                }
            }

            if (step == 0) {
                unit->list1.RemoveAll();
            } else {
                this->sub_5890CC(unit, cur_x, cur_y, (uint8_t)step, (uint8_t)(step >> 8));
            }
        }
    } else if (flag == 0) {
        this->sub_5893C6(unit, cur_x, cur_y, x, y);
    } else {
        this->sub_5890CC(unit, cur_x, cur_y, x, y);
    }
}

// Shared by sub_597290/sub_5974B0/sub_597990/sub_5976D0: recomputes movement cost to each
// of the 9 neighbor cells (including self) around (x,y) in the field3_0x30000 cost grid,
// gated by the given passability check, and enqueues improved cells into the given queue.
void MapStuff::ExpandCostGridNeighbors(Unit* unit, uint8_t x, uint8_t y, PassableCheckFn passable, uint8_t* out_x, uint8_t* out_y, uint16_t& out_count) {
    for (int8_t dx = -1; dx <= 1; dx++) {
        for (int8_t dy = -1; dy <= 1; dy++) {
            uint8_t nx = x + dx;
            uint8_t ny = y + dy;
            if (!(this->*passable)(unit, PosYX(nx, ny).val)) {
                continue;
            }

            uint16_t cost;
            if (dx == 0 || dy == 0) {
                if (unit->sub_59A030() == 1) {
                    cost = *this->sub_59A670(x, y) + *this->sub_59A6F0(nx, ny);
                } else {
                    cost = *this->sub_59A670(x, y) + 2;
                }
            } else {
                if (unit->sub_59A030() == 1) {
                    uint8_t walk_cost = *this->sub_59A6F0(nx, ny);
                    cost = *this->sub_59A670(x, y) + walk_cost + (walk_cost / 2);
                } else {
                    cost = *this->sub_59A670(x, y) + 3;
                }
            }

            if (cost < *this->sub_59A670(nx, ny)) {
                *this->sub_59A670(nx, ny) = cost;
                out_x[out_count] = nx;
                out_y[out_count] = ny;
                out_count++;
            }
        }
    }
}

// 597290 --- expands the static-path cost grid from (x,y) using sub_596F80 as the passability
// check, queueing improved neighbors into queue2 (field_0x50008[0x2000/0x3000], field8_0x5400a).
void MapStuff::sub_597290(Unit* unit, uint8_t x, uint8_t y) {
    this->ExpandCostGridNeighbors(unit, x, y, &MapStuff::sub_596F80, this->field_0x50008 + 0x2000, this->field_0x50008 + 0x3000, this->field8_0x5400a);
}

// 5974B0 --- expands the static-path cost grid from (x,y) using sub_596F80 as the passability
// check, queueing improved neighbors into queue1 (field_0x50008[0/0x1000], field7_0x54008).
void MapStuff::sub_5974B0(Unit* unit, uint8_t x, uint8_t y) {
    this->ExpandCostGridNeighbors(unit, x, y, &MapStuff::sub_596F80, this->field_0x50008, this->field_0x50008 + 0x1000, this->field7_0x54008);
}

// 5976D0 --- expands the dynamic-path cost grid from (x,y) using sub_5978F0 as the passability
// check, queueing improved neighbors into queue2 (field_0x50008[0x2000/0x3000], field8_0x5400a).
void MapStuff::sub_5976D0(Unit* unit, uint8_t x, uint8_t y) {
    this->ExpandCostGridNeighbors(unit, x, y, &MapStuff::sub_5978F0, this->field_0x50008 + 0x2000, this->field_0x50008 + 0x3000, this->field8_0x5400a);
}

// 597990 --- expands the dynamic-path cost grid from (x,y) using sub_5978F0 as the passability
// check, queueing improved neighbors into queue1 (field_0x50008[0/0x1000], field7_0x54008).
void MapStuff::sub_597990(Unit* unit, uint8_t x, uint8_t y) {
    this->ExpandCostGridNeighbors(unit, x, y, &MapStuff::sub_5978F0, this->field_0x50008, this->field_0x50008 + 0x1000, this->field7_0x54008);
}

// Shared by sub_5890CC/sub_5893C6: walks backward from (x,y) toward (cur_x,cur_y) through the
// field3_0x30000 cost grid, picking the cheapest neighbor at each step (its cost weighted by
// walk_cost_map at the current work position), appending each visited cell to `path`. On
// success (reached (cur_x,cur_y) within 1000 steps) drops the initial (x,y) entry from the
// front of `path`; on failure (gave up) clears `path` entirely.
void MapStuff::WalkCostPath(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y, CList<PosYX>& path) {
    path.AddHead(PosYX(x, y));

    uint8_t wx = x;
    uint8_t wy = y;
    int8_t best_dx = 0;
    int8_t best_dy = 0;
    uint16_t attempts = 0;

    while (wx != cur_x || wy != cur_y) {
        uint16_t best_cost = 0xFFFF;
        attempts++;
        if (attempts > 1000) {
            path.RemoveAll();
            return;
        }

        for (int8_t dx = -1; dx <= 1; dx++) {
            for (int8_t dy = -1; dy <= 1; dy++) {
                int32_t nx = (int32_t)wx + dx;
                int32_t ny = (int32_t)wy + dy;
                if (nx < 8 || nx > this->map_width + 8 || ny < 8 || ny > this->map_height + 8) {
                    continue;
                }

                uint16_t cost = *this->sub_59A670((uint8_t)nx, (uint8_t)ny);
                if (cost == 0xFFFF) {
                    continue;
                }

                if (dx == 0 || dy == 0) {
                    if (unit->sub_59A030() == 1) {
                        cost += *this->sub_59A6F0(wx, wy);
                    } else {
                        cost += 2;
                    }
                    if (cost <= best_cost) {
                        best_cost = cost;
                        best_dx = dx;
                        best_dy = dy;
                    }
                } else {
                    if (unit->sub_59A030() == 1) {
                        uint8_t wc = *this->sub_59A6F0(wx, wy);
                        cost += wc + (wc / 2);
                    } else {
                        cost += 3;
                    }
                    if (cost < best_cost) {
                        best_cost = cost;
                        best_dx = dx;
                        best_dy = dy;
                    }
                }
            }
        }

        wx += best_dx;
        wy += best_dy;
        path.AddHead(PosYX(wx, wy));
    }

    path.RemoveHead();
}

// 5890CC --- builds unit->list1 with a cost-grid-guided path from (x,y) back to (cur_x,cur_y).
void MapStuff::sub_5890CC(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y) {
    this->WalkCostPath(unit, cur_x, cur_y, x, y, unit->list1);
}

// 5893C6 --- builds unit->list2 with a cost-grid-guided path from (x,y) back to (cur_x,cur_y).
// Similar to sub_5890CC, but uses list2 instead of list1.
void MapStuff::sub_5893C6(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y) {
    this->WalkCostPath(unit, cur_x, cur_y, x, y, unit->list2);
}

// 592831
uint8_t MapStuff::sub_592831(Unit* target, Unit* unit) {
    int32_t dx = unit->sub_528725() - target->sub_528725();
    int32_t dy = unit->sub_528763() - target->sub_528763();
    return this->ClassifyDirection16(dx, dy);
}

// 592A48 --- find the lowest-cost cell in an expanding box around target, walking along the box
// perimeter starting from where the unit->target line of sight crosses it.
uint16_t MapStuff::sub_592A48(Unit* unit, Unit* target) {
    int32_t box_max_x = target->position->GetX() + target->VMethod3();
    int32_t box_max_y = target->position->GetY() + target->VMethod3();
    int32_t box_min_x = target->position->GetX() - unit->VMethod3();
    int32_t box_min_y = target->position->GetY() - unit->VMethod3();

    uint8_t direction = this->sub_592831(target, unit);
    int32_t quadrant = (((direction + 2) & 0xC) >> 2) + 4; // 4..7
    
    float dx = (int32_t)target->sub_528725() - unit->sub_528725();
    float dy = (int32_t)target->sub_528763() - unit->sub_528763();

    float slope;
    int32_t intercept;
    if (quadrant & 1) {
        // Quadrants 5,7: line expressed as Y = slope*X + intercept.
        if (dx == 0.0f) {
            dx = 1.0f;
        }
        slope = dy / dx;
        intercept = target->sub_528763() - target->sub_528725() * slope;
    } else {
        // Quadrants 4,6: line expressed as X = slope*Y + intercept.
        slope = dx / dy;
        intercept = target->sub_528725() - target->sub_528763() * slope;
    }

    uint16_t best_cost = 0xFFFF;
    uint16_t best_x = 0;
    uint16_t best_y = 0;
    int32_t attempts = 0;

    while (best_cost == 0xFFFF && attempts < 8) {
        attempts++;

        // Find where the line crosses the current box edge facing `quadrant`; that's the shared
        // starting point for the two perimeter-walking cursors below.
        uint16_t seed_x, seed_y;
        int32_t edge;
        switch (quadrant) {
        case 4:
            edge = (box_min_y << 8) + 0x80;
            seed_x = ((int)(edge * slope + intercept)) >> 8;
            seed_y = box_min_y;
            break;
        case 5:
            edge = (box_max_x << 8) + 0x80;
            seed_x = box_max_x;
            seed_y = ((int)(edge * slope + intercept)) >> 8;
            break;
        case 6:
            edge = (box_max_y << 8) + 0x80;
            seed_x = ((int)(edge * slope + intercept)) >> 8;
            seed_y = box_max_y;
            break;
        case 7:
            edge = (box_min_x << 8) + 0x80;
            seed_x = box_min_x;
            seed_y = ((int)(edge * slope + intercept)) >> 8;
            break;
        }

        // Two cursors walk the box perimeter from the seed point in opposite directions,
        // meeting roughly halfway around, checking the map cost at every cell visited.
        int32_t dir_a = quadrant;
        int32_t dir_b = quadrant;
        uint16_t x_a = seed_x, y_a = seed_y;
        uint16_t x_b = seed_x, y_b = seed_y;

        int32_t perimeter_len = (box_max_x - box_min_x) + (box_max_y - box_min_y) + 1;

        for (int32_t step = 0; step < perimeter_len; step++) {
            uint16_t cost_a = *this->sub_59A670((uint8_t)x_a, (uint8_t)y_a);
            if (cost_a < best_cost) {
                best_cost = cost_a;
                best_x = x_a;
                best_y = y_a;
            }

            uint16_t cost_b = *this->sub_59A670((uint8_t)x_b, (uint8_t)y_b);
            if (cost_b < best_cost) {
                best_cost = cost_b;
                best_x = x_b;
                best_y = y_b;
            }

            switch (dir_a % 4) {
            case 0: if (x_a == box_max_x) { dir_a++; } break;
            case 1: if (y_a == box_max_y) { dir_a++; } break;
            case 2: if (x_a == box_min_x) { dir_a++; } break;
            case 3: if (y_a == box_min_y) { dir_a++; } break;
            }

            switch (dir_b % 4) {
            case 0: if (x_b == box_min_x) { dir_b--; } break;
            case 1: if (y_b == box_min_y) { dir_b--; } break;
            case 2: if (x_b == box_max_x) { dir_b--; } break;
            case 3: if (y_b == box_max_y) { dir_b--; } break;
            }

            x_a += this->line_walk_delta[dir_a][0];
            y_a += this->line_walk_delta[dir_a][1];
            x_b -= this->line_walk_delta[dir_b][0];
            y_b -= this->line_walk_delta[dir_b][1];

            if (step > 100) {
                break;
            }
        }

        // Grow the search box and try again if nothing was found yet.
        box_min_x--;
        box_min_y--;
        box_max_x++;
        box_max_y++;
    }

    if (best_cost == 0xFFFF) {
        return 0;
    }
    return PosYX((uint8_t)best_x, (uint8_t)best_y).val;
}

// 593134 --- search outward ring by ring (up to `radius` rings) from `yx` for the cell with the
// lowest field3_0x30000 cost-grid value. Returns 0 if none found within range.
int16_t MapStuff::sub_593134(Unit* unit, int32_t zero, uint16_t yx, uint32_t radius) {
    uint16_t best_cost = 0xFFFF;
    uint16_t best_yx = 0;

    for (int32_t ring = 1; ring < (int32_t)(radius & 0xFFFF); ring++) {
        for (int32_t offset = -ring; offset < ring + 1; offset++) {
            uint16_t pos = yx + (ring << 8) + offset;
            if (this->field3_0x30000[pos] < best_cost) {
                best_cost = this->field3_0x30000[pos];
                best_yx = pos;
            }

            pos = yx - (ring << 8) + offset;
            if (this->field3_0x30000[pos] < best_cost) {
                best_cost = this->field3_0x30000[pos];
                best_yx = pos;
            }

            pos = yx + (offset << 8) + ring;
            if (this->field3_0x30000[pos] < best_cost) {
                best_cost = this->field3_0x30000[pos];
                best_yx = pos;
            }

            pos = yx + (offset << 8) - ring;
            if (this->field3_0x30000[pos] < best_cost) {
                best_cost = this->field3_0x30000[pos];
                best_yx = pos;
            }
        }
        if (best_cost < 0xFFFF) {
            break;
        }
    }

    return best_cost == 0xFFFF ? 0 : (int16_t)best_yx;
}

// 5936D2 --- Project a point `range` tiles away from unit along the line from packed_yyxx through the unit, clamped to the map with an 8-tile border.
int16_t MapStuff::sub_5936D2(Unit* unit, uint32_t packed_yyxx, uint8_t range) {
    uint16_t target_xx = packed_yyxx & 0xFFFF;
    uint16_t target_yy = packed_yyxx >> 16;
    uint16_t unit_xx = unit->position->GetXx();
    uint16_t unit_yy = unit->position->GetYy();

    int32_t dxx = unit_xx - target_xx;
    int32_t dyy = unit_yy - target_yy;
    if (dxx == 0) {
        dxx = 1;
    }
    if (dyy == 0) {
        dyy = 1;
    }

    int32_t out_x;
    int32_t out_y;
    if (abs(dxx) > abs(dyy)) {
        float slope = (float)dyy / (float)dxx;
        float intercept = (float)unit_yy - (float)unit_xx * slope;
        if (dxx > 0) {
            out_x = unit_xx + (range << 8);
        } else {
            out_x = unit_xx - (range << 8);
        }
        out_y = (int32_t)((float)out_x * slope + intercept);
    } else {
        float slope = (float)dxx / (float)dyy;
        float intercept = (float)unit_xx - (float)unit_yy * slope;
        if (dyy > 0) {
            out_y = unit_yy + (range << 8);
        } else {
            out_y = unit_yy - (range << 8);
        }
        out_x = (int32_t)((float)out_y * slope + intercept);
    }

    out_x >>= 8;
    out_y >>= 8;

    if (out_x < 8) {
        out_x = 8;
    }
    if (out_x > this->GetWidth() - 9) {
        out_x = this->GetWidth() - 9;
    }
    if (out_y < 8) {
        out_y = 8;
    }
    if (out_y > this->GetHeight() - 9) {
        out_y = this->GetHeight() - 9;
    }

    return PosYX((uint8_t)out_x, (uint8_t)out_y).val;
}

// 59A6F0 --- get pointer into walk_cost_map for (x, y)
uint8_t* MapStuff::sub_59A6F0(uint8_t x, uint8_t y) {
    return &this->WalkCostAt(PosYX(x, y));
}

// 59A670 --- get pointer into field3_0x30000 cost grid for (x, y)
uint16_t* MapStuff::sub_59A670(uint8_t x, uint8_t y) {
    return &this->field3_0x30000[PosYX(x, y).val];
}

// 5946BF --- get building at (x, y); callers use the result as Unit*
Unit* MapStuff::sub_5946BF(uint8_t x, uint8_t y) {
    return (Unit*)this->sub_594709(PosYX(x, y).val);
}

// 594768 --- set bit 0x10 in both obstacle maps at (x, y)
void MapStuff::sub_594768(uint8_t x, uint8_t y) {
    PosYX yx(x, y);
    this->ObstacleAt(yx).SetBits(0x10);
    this->Obstacle2At(yx).SetBits(0x10);
}

// 594E65 --- register area effect at (x, y)
void MapStuff::sub_594E65(AreaEffect* ae, uint8_t x, uint8_t y) {
    this->sub_594EBA(ae, PosYX(x, y).val);
}

// 59501E --- remove area effect from (x, y)
void MapStuff::sub_59501E(AreaEffect* ae, uint8_t x, uint8_t y) {
    this->sub_595073(ae, PosYX(x, y).val);
}

// 594709
Building* MapStuff::sub_594709(uint16_t yx) {
    if (!this->ObstacleAt(yx).TestBits(0x20)) {
        return nullptr;
    }
    if (!this->cell_states.Lookup(yx, this->scratch_cell_state)) {
        return nullptr;
    }
    return this->scratch_cell_state.building;
}

// 58BFA3 --- Chebyshev distance between two (x, y) points
int32_t MapStuff::sub_58BFA3(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    return (std::max)(abs(x1 - x2), abs(y1 - y2));
}

// 593C9A --- Returns unit's speed, overridden by group_sub->field_0x44 if nonzero. `this` is unused.
int32_t MapStuff::sub_593C9A(Unit* unit) {
    uint8_t override_speed = unit->group->group_sub->field_0x44;
    if (override_speed != 0) {
        return override_speed;
    }
    return (int16_t)unit->speed;
}

// 594C87 --- Walk cost at yx, adjusted for area effects when the cell is occupied.
uint8_t MapStuff::sub_594C87(uint16_t yx) {
    if (!this->ObstacleAt(yx).TestBits(0x20)) {
        return this->WalkCostAt(yx);
    }

    CellState cell;
    if (!this->cell_states.Lookup(yx, cell)) {
        return this->WalkCostAt(yx);
    }

    if (cell.effect_count == 0) {
        return this->WalkCostAt(yx);
    }

    for (int i = 0; i < 6; ++i) {
        // In vanilla, they do `if (FUN_0059a050())`, but this function always returned true.
        this->WalkCostAt(yx) >>= 2;
        break;
    }

    return this->WalkCostAt(yx);
}

// 593CD5 --- Computes movement step magnitude/heading toward yx.
int32_t MapStuff::sub_593CD5(Unit* unit, uint16_t yx, uint8_t val) {
    uint8_t dir = (val + 0x10) >> 5;
    uint16_t yx2 = yx + this->field50_0x58e98[dir];

    int32_t speed;
    if (unit->sub_59A030() == 1) {
        int8_t height_delta = this->height_map[yx] - this->height_map[yx2];
        if (abs(height_delta) > 32) {
            height_delta = (height_delta < 0) ? -32 : 32;
        }

        speed = this->sub_593C9A(unit) * this->speed_multiplier;
        if (height_delta < 0) {
            speed -= speed * abs(height_delta) >> 6;
        } else {
            speed += speed * height_delta >> 6;
        }

        uint8_t walk_cost1 = this->sub_594C87(yx);
        uint8_t walk_cost2 = this->sub_594C87(yx2);
        uint8_t avg_walk_cost = (walk_cost1 + walk_cost2) / 2;
        if (avg_walk_cost == 0) {
            avg_walk_cost = 8;
        }
        speed /= avg_walk_cost;
    } else {
        speed = this->sub_593C9A(unit);
    }

    if (speed < 1) {
        speed = 1;
    }
    if (speed > 63) {
        speed = 63;
    }

    unit->eye->field152_0xa8 = (uint16_t)speed;
    unit->eye->field158_0xae = (uint16_t)dir;

    int8_t dir_dx = this->field48_0x58e88[dir];
    int8_t dir_dy = this->field49_0x58e90[dir];
    if (dir_dx * dir_dy != 0) {
        unit->eye->field160_0xb0 = (int8_t)((double)(speed * dir_dx) * 0.707);
        unit->eye->field161_0xb1 = (int8_t)((double)(speed * dir_dy) * 0.707);
    } else {
        unit->eye->field160_0xb0 = speed * dir_dx;
        unit->eye->field161_0xb1 = speed * dir_dy;
    }

    int8_t abs_component;
    if (unit->eye->field160_0xb0 == 0) {
        abs_component = abs(unit->eye->field161_0xb1);
    } else {
        abs_component = abs(unit->eye->field160_0xb0);
    }
    if (abs_component == 0) {
        abs_component = 1;
    }

    unit->eye->field154_0xaa = (uint16_t)(256 / abs_component);
    if (256 % abs_component != 0) {
        unit->eye->field154_0xaa += 1;
    }

    return speed;
}

// 596F80 --- checks whether unit can occupy the size x size box of cells at yx, per obstacle_map
int32_t MapStuff::sub_596F80(Unit* unit, uint16_t yx) {
    int8_t size = unit->VMethod3();
    for (int32_t i = 0; i < size; i++) {
        for (int32_t j = 0; j < size; j++) {
            if (this->ObstacleAt(PosYX(yx + PosYX(i, j).val)).TestBits(unit->eye->field5_0x5)) {
                return 0;
            }
        }
    }
    return 1;
}

// 5978F0 --- Passability check used by sub_5976D0/sub_597990; same as sub_596F80 but against obstacle_map2
int32_t MapStuff::sub_5978F0(Unit* unit, uint16_t yx) {
    int8_t size = unit->VMethod3();
    for (int32_t i = 0; i < size; i++) {
        for (int32_t j = 0; j < size; j++) {
            if (this->Obstacle2At(PosYX(yx + PosYX(i, j).val)).TestBits(unit->eye->field5_0x5)) {
                return 0;
            }
        }
    }
    return 1;
}

// 58A158 --- classifies direction/octant (0-7, shifted << 5) from unit's position toward yx.
// If yx is exactly at the unit's current position, keeps the unit's current eye angle instead.
int32_t MapStuff::sub_58A158(Unit* unit, uint16_t yx) {
    PosYX target(yx);
    int32_t dxx = (target.x << 8) + 0x80 - unit->position->GetXx();
    int32_t dyy = (target.y << 8) + 0x80 - unit->position->GetYy();

    uint8_t dir = unit->eye->field0_0x0;
    if (dxx > 0) {
        if (dyy > 0) {
            dir = 3;
        } else if (dyy == 0) {
            dir = 2;
        } else {
            dir = 1;
        }
    } else if (dxx < 0) {
        if (dyy > 0) {
            dir = 5;
        } else if (dyy == 0) {
            dir = 6;
        } else {
            dir = 7;
        }
    } else {
        if (dyy > 0) {
            dir = 4;
        } else if (dyy < 0) {
            dir = 0;
        }
    }

    return dir << 5;
}

// 591A96 --- Chebyshev-like distance between yx1 and yx2 for a unit of a given size.
uint8_t MapStuff::sub_591A96(uint16_t yx1, uint16_t yx2, int32_t size) {
    PosYX p1(yx1);
    PosYX p2(yx2);

    int32_t dx;
    if (p1.x < p2.x) {
        dx = (std::max)(0, (p2.x - p1.x) - size + 1);
    } else {
        dx = p1.x - p2.x;
    }

    int32_t dy;
    if (p1.y < p2.y) {
        dy = (std::max)(0, (p2.y - p1.y) - size + 1);
    } else {
        dy = p1.y - p2.y;
    }

    return (std::max)(dx, dy);
}
