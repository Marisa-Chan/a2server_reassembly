#include "map_stuff.h"

#include "eye.h"
#include "unit.h"
#include "world.h"

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
int MapStuff::sub_58FEDA(Unit* unit, uint16_t yx, uint8_t max_range) {
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
void MapStuff::sub_58FF51(Unit* unit, uint16_t yx, uint8_t max_range) {
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
        this->sub_58826D(unit, yx, yx >> 8, 1, nullptr);
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
