#ifndef SHOP_H
#define SHOP_H

#include <cstdint>

#include "building.h"
#include "group.h"
#include "unit_list.h"
#include "assert_offset.h"


// Outpost is a great name. It actually remembers the original group properties for repop.
// Why is it derived from Building, Nival? Why?
class Outpost : public Building {
public:
    DECLARE_SERIAL(Outpost); // Runtime class definition at 637240.

public: // VTable at 60f3b8.
    Outpost(); // 54326A
    Outpost(TokenPos* pos); // 5432DF
    virtual ~Outpost() override;

public:
    virtual int32_t IsOutpost() override;
    virtual void VMethod1() override;

public:
    void sub_543EC3(Group* group);
    void sub_54335a(); // Initializes fields.

public:
    UnitList units_original;
    UnitList units_copy;
    uint32_t group_id;
    uint32_t repop_delay;
    int32_t repop_countdown;
    int32_t script_id;
    uint32_t spread;
    uint32_t is_awaiting_repop;
    uint32_t has_quest_kill;
    uint32_t has_quest_intercept;
};
ASSERT_OFFSET(Outpost, script_id, 0xb8);
ASSERT_SIZE(Outpost, 0xcc);


#endif
