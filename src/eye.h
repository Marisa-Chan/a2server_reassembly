#ifndef EYE_H
#define EYE_H

#include <cstdint>

#include "assert_offset.h"
#include <afxtempl.h>
#include "token.h"


class Building;
class Spell;
class Unit;


// UnitEye: some fields might be unused. Dumping them all as is for now.
struct UnitEye {
    uint8_t facing; // Current angle.
    uint8_t target_facing; // Intent angle.
    uint8_t gap_0x2;
    uint8_t gap_0x3;
    uint8_t gap_0x4;
    uint8_t path_mask;
    uint16_t position1;
    uint8_t scan_radius_base;
    uint8_t static_age;
    uint8_t rotation_speed;
    uint8_t gap_0xb;
    uint8_t gap_0xc;
    uint8_t gap_0xd;
    uint8_t gap_0xe;
    uint8_t gap_0xf;
    uint8_t gap_0x10;
    uint8_t gap_0x11;
    uint8_t gap_0x12;
    uint8_t gap_0x13;
    uint8_t gap_0x14;
    uint8_t gap_0x15;
    uint8_t gap_0x16;
    uint8_t gap_0x17;
    uint8_t gap_0x18;
    uint8_t gap_0x19;
    uint8_t gap_0x1a;
    uint8_t gap_0x1b;
    uint8_t gap_0x1c;
    uint8_t gap_0x1d;
    uint8_t gap_0x1e;
    uint8_t gap_0x1f;
    uint8_t gap_0x20;
    uint8_t gap_0x21;
    uint8_t gap_0x22;
    uint8_t gap_0x23;
    uint8_t gap_0x24;
    uint8_t gap_0x25;
    uint8_t gap_0x26;
    uint8_t gap_0x27;
    uint8_t gap_0x28;
    uint8_t gap_0x29;
    uint8_t gap_0x2a;
    uint8_t gap_0x2b;
    uint8_t gap_0x2c;
    uint8_t gap_0x2d;
    uint8_t gap_0x2e;
    uint8_t gap_0x2f;
    uint8_t gap_0x30;
    uint8_t gap_0x31;
    uint8_t gap_0x32;
    uint8_t gap_0x33;
    uint8_t gap_0x34;
    uint8_t gap_0x35;
    uint8_t gap_0x36;
    uint8_t gap_0x37;
    uint8_t gap_0x38;
    uint8_t gap_0x39;
    uint8_t gap_0x3a;
    uint8_t gap_0x3b;
    uint8_t gap_0x3c;
    uint8_t gap_0x3d;
    uint8_t gap_0x3e;
    uint8_t gap_0x3f;
    uint8_t gap_0x40;
    uint8_t gap_0x41;
    uint8_t gap_0x42;
    uint8_t gap_0x43;
    uint8_t gap_0x44;
    uint8_t gap_0x45;
    uint8_t gap_0x46;
    uint8_t gap_0x47;
    uint8_t gap_0x48;
    uint8_t gap_0x49;
    uint8_t gap_0x4a;
    uint8_t gap_0x4b;
    uint8_t gap_0x4c;
    uint8_t gap_0x4d;
    uint8_t gap_0x4e;
    uint8_t gap_0x4f;
    uint8_t gap_0x50;
    uint8_t gap_0x51;
    uint8_t gap_0x52;
    uint8_t gap_0x53;
    uint8_t gap_0x54;
    uint8_t gap_0x55;
    uint8_t gap_0x56;
    uint8_t gap_0x57;
    uint8_t gap_0x58;
    uint8_t gap_0x59;
    uint8_t gap_0x5a;
    uint8_t gap_0x5b;
    uint8_t gap_0x5c;
    uint8_t gap_0x5d;
    uint8_t gap_0x5e;
    uint8_t gap_0x5f;
    uint8_t gap_0x60;
    uint8_t gap_0x61;
    uint8_t gap_0x62;
    uint8_t gap_0x63;
    uint8_t gap_0x64;
    uint8_t gap_0x65;
    uint8_t gap_0x66;
    uint8_t gap_0x67;
    uint8_t gap_0x68;
    uint8_t gap_0x69;
    uint8_t gap_0x6a;
    uint8_t gap_0x6b;
    uint32_t moving_to;
    uint16_t reserved_cell;
    uint16_t tile_move_time; // Effective speed?
    PosYX static_path_cell;
    PosYX move_target;
    uint8_t counter;
    uint8_t gap_0x79;
    uint8_t gap_0x7a;
    uint8_t gap_0x7b;
    Unit* target_unit;
    PosYX guard_cell;
    uint8_t place_x; // X position
    uint8_t place_y; // Y position
    uint8_t place_x_sub; // XX sub-cell position
    uint8_t place_y_sub; // YY sub-cell position
    uint8_t placed_x;
    uint8_t placed_y;
    uint8_t placed_x_sub;
    uint8_t placed_y_sub;
    uint16_t static_path_len;
    PosYX dynamic_path_cell;
    uint8_t gap_0x8e;
    uint8_t gap_0x8f;
    uint32_t target_reached;
    PosYX gap_0x94;
    PosYX gap_0x96;
    uint32_t order_arrived;
    uint8_t gap_0x9c;
    uint8_t turn_tick;
    uint8_t gap_0x9e;
    uint8_t gap_0x9f;
    uint32_t turning;
    uint8_t turn_steps;
    uint8_t gap_0xa5;
    uint8_t gap_0xa6;
    uint8_t gap_0xa7;
    uint16_t move_speed;
    uint16_t move_step_total;
    uint16_t move_step_index;
    uint16_t move_dir;
    int8_t move_dx;
    int8_t move_dy;
    uint8_t gap_0xb2;
    uint8_t gap_0xb3;

public:
    UnitEye(); // 58BE86 (client: 55b336)
    uint8_t GetFacingHalf(); // 58C00E (client: 55b4be)
    void SetPathMaskByMovement(Unit* unit); // 5925C9 (client: 561a79)
    int CheckRotation(Unit* unit, uint8_t* out_dir, uint8_t eye0, uint8_t eye1); // 59367D (client: 562b2d)
};
ASSERT_OFFSET(UnitEye, counter, 0x78);
ASSERT_OFFSET(UnitEye, place_x, 0x82);
ASSERT_SIZE(UnitEye, 0xb4);

struct UnitEye2 {
    PosYX position1;
    PosYX position2;
    uint32_t patrol_reanchor;
    uint8_t cast_action;
    uint8_t ai_state;
    uint16_t command_to;
    Unit* unit;
    Unit* unit2;
    uint8_t max_range;
    uint8_t counter;
    uint16_t tick16;
    Unit* unit3;
    uint8_t gap_0x1c;
    uint8_t gap_0x1d;
    uint8_t gap_0x1e;
    uint8_t gap_0x1f;
    Unit* unit4;
    uint16_t x;
    uint16_t y;
    Unit* unit5;
    uint8_t gap_0x2c;
    uint8_t gap_0x2d;
    uint8_t gap_0x2e;
    uint8_t gap_0x2f;
    Spell* spell;
    uint8_t gap_0x34;
    uint8_t gap_0x35;
    uint8_t gap_0x36;
    uint8_t gap_0x37;
    uint32_t gap_0x38;
    uint16_t cast_pos;
    uint8_t gap_0x3e;
    uint8_t gap_0x3f;
    uint32_t withdraw;
    uint32_t wimpy;
    uint8_t gap_0x48;
    uint8_t gap_0x49;
    uint8_t gap_0x4a;
    uint8_t gap_0x4b;
    uint8_t wimpy_level;
    uint8_t gap_0x4d;
    uint8_t gap_0x4e;
    uint8_t gap_0x4f;
    uint32_t has_command;
    uint32_t engaged;
    uint16_t position3;
    uint8_t counter2;
    uint8_t gap_0x5b;
    uint32_t cast_is_area;
    uint32_t action_done;
    uint32_t gap_0x64;
    Building* enter_building;
    uint32_t gap_0x6c;
    uint8_t range;
    uint8_t seeInvisible;
    uint8_t gap_0x72;
    uint8_t gap_0x73;
    uint8_t gap_0x74;
    uint8_t gap_0x75;
    uint8_t gap_0x76;
    uint8_t gap_0x77;
    uint8_t spell_id;
    uint8_t gap_0x79;
    uint8_t gap_0x7a;
    uint8_t gap_0x7b;
    uint32_t known_spells[3];
    uint32_t known_spell_probabilities[3];
    Unit* unit6;
    uint32_t tick;
    uint16_t attack_move_pos;
    uint8_t counter3;
    uint8_t gap_0x9f;
    uint16_t gap_0xa0;
    uint8_t gap_0xa2;
    uint8_t gap_0xa3;
    uint32_t autobuff_spell_id; // TODO: unclear if this is really spell ID, as autobuff checks for `!= 20` only.
    uint32_t autobuff_tick;
    int32_t hits_taken;
    PosYX teleport_pos;
    uint8_t gap_0xb2;
    uint8_t gap_0xb3;
    CList<uint16_t>* positions_list;

public:
    UnitEye2(); // 5A4E83 (client: 572ff3)
    ~UnitEye2(); // 5A4F30 (client: 47b660)
    void Serialize(CArchive& ar); // 5B031B (client: 57E48B)
};
ASSERT_OFFSET(UnitEye2, known_spells, 0x7c);
ASSERT_SIZE(UnitEye2, 0xb8);


#endif
