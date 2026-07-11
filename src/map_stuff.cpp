#include "map_stuff.h"

#include "constants.h"
#include "building.h"
#include "eye.h"
#include "game_app.h"
#include "group.h"
#include "net.h"
#include "player.h"
#include "sack.h"
#include "spell_effect.h"
#include "unit.h"
#include "world.h"

CString g_MissionText; //660f2c
CString g_MissionBriefing; //660de8
CStringArray g_MissionFailures; //660f18
CStringArray g_MissionSubjs; //660ea8

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
            int32_t reached = this->sub_594709(dest.val);
            if (reached == 0) {
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
