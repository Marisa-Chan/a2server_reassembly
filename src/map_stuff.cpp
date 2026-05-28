#include "map_stuff.h"

#include "eye.h"
#include "unit.h"
#include "world.h"
#include "building.h"

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