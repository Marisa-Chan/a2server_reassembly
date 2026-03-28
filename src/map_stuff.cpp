#include "map_stuff.h"

#include "unit.h"

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
