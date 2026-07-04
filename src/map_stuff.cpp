#include "map_stuff.h"

#include "eye.h"
#include "unit.h"
#include "world.h"
#include "building.h"
#include "sack.h"
#include "net.h"
#include "spell_effect.h"

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
    gap_0x38[0] = 0;
    gap_0x38[1] = 0;
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
    uint8_t curX = unit->position->GetX();
    uint8_t curY = unit->position->GetY();
    this->sub_5882AE(unit, curX, curY, x, y, flag, target);
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
