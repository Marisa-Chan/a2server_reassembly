#ifndef SHOP_H
#define SHOP_H

#include <cstdint>

#include "building.h"
#include "unit_list.h"


// Outpost is a great name. It actually remembers the original group properties for repop.
// Why is it derived from Building, Nival? Why?
class Outpost : public Building {
public:
    virtual int32_t IsOutpost() override;

public:
    UnitList units_original;
    UnitList units_copy;
    uint32_t group_id;
    uint32_t repop_delay;
    uint32_t repop_countdown;
    int32_t script_id;
    uint32_t spread;
    uint32_t is_awaiting_repop;
    uint32_t has_quest_kill;
    uint32_t has_quest_intercept;
};
static_assert(offsetof(Outpost, script_id) == 0xb8, "Outpost::script_id offset mismatch");
static_assert(sizeof(Outpost) == 0xcc, "Outpost size mismatch");


#endif
