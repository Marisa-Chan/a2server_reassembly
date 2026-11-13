// Configurable partial item drop on player death, ported from my-srvmgr/partial_drop.cpp
// (partial_drop_skip_undress @ 52DA11, imp_drop_partially @ 52DEEF & 52E264).

#define _USE_MATH_DEFINES
#include "partial_drop.h"

#include "../game_app.h"
#include "../inventory.h"
#include "../item.h"
#include "../player.h"
#include "../unit.h"
#include "config_new.h"
#include "solo.h"

#include <cmath>
#include <cstdlib>
#include <vector>

namespace {

struct IndNum {
    int16_t ind;
    int16_t num;
};

float Rnd()
{
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX + 1);
}

float RndGaussian(float mu, float sigma)
{
    float two_pi = static_cast<float>(2.0 * M_PI);
    float u1 = Rnd();
    float u2 = Rnd();
    float z0 = static_cast<float>(std::sqrt(-2.0 * std::log(u1)) * std::cos(two_pi * u2));
    return z0 * sigma + mu;
}

float Limit(float n, float lo, float hi)
{
    if (!(n >= lo)) {
        return lo;
    }
    if (n > hi) {
        return hi;
    }
    return n;
}

int RoundNum(float a)
{
    return static_cast<int>(a + 0.5f);
}

int GetDropNum(int num, float probability)
{
    int drop_n = 0;
    if (probability == 0.0f) {
        drop_n = 0;
    } else if (probability == 1.0f) {
        drop_n = num;
    } else {
        if (num == 1) {
            if (Rnd() < probability) {
                drop_n = 1;
            }
        } else {
            float odds = Limit(RndGaussian(probability, 0.1f), 0.0f, 1.0f);
            drop_n = RoundNum(odds * static_cast<float>(num));
        }
    }
    return drop_n;
}

void DropRndItems(Inventory* item_list_src, Inventory* item_list_dst, float probability)
{
    if (item_list_src == nullptr || item_list_dst == nullptr) {
        return;
    }

    POSITION src_current = item_list_src->items.GetTailPosition();
    int ind = item_list_src->items.GetCount() - 1;
    std::vector<IndNum> to_remove;

    while (src_current != nullptr) {
        Item* item = item_list_src->items.GetPrev(src_current);
        if (item == nullptr || item->item_id == 3667) {
            ind--;
            continue;
        }

        int drop_n = GetDropNum(static_cast<int>(item->count), probability);
        if (drop_n > 0) {
            IndNum entry;
            entry.ind = static_cast<int16_t>(ind);
            entry.num = static_cast<int16_t>(drop_n);
            to_remove.push_back(entry);
        }

        ind--;
    }

    for (const IndNum& entry : to_remove) {
        Item* removed = item_list_src->TakeItemAt(entry.ind, entry.num);
        item_list_dst->PutItemIntoBag(0, removed);
    }
}

void DropRndWearedItems(Unit* unit, Inventory* item_list_dst, float probability)
{
    if (unit == nullptr || item_list_dst == nullptr || !unit->IsKindOf(RUNTIME_CLASS(Human))) {
        return;
    }

    Human* human = static_cast<Human*>(unit);

    if (!IsPurePlayer(unit)) {
        if (unit->weapon != nullptr && GetDropNum(1, probability) > 0) {
            item_list_dst->PutItemIntoBagAtDefault(unit->Unequip(unit->weapon));
        }
    }

    for (int i = -1; i < 13; ++i) {
        Item* item = (i == -1) ? static_cast<Item*>(unit->shield) : human->equipment[i];
        if (item != nullptr && GetDropNum(1, probability) > 0) {
            item_list_dst->PutItemIntoBagAtDefault(unit->Unequip(item));
        }
    }
}

void CopyInventoryToMap(Unit* unit, Inventory* inventory, int32_t money, int32_t main_unit)
{
    if (inventory->items.GetCount() > 0 || money != 0) {
        unit->CreateSackFromInventory(inventory, money, main_unit);
    } else {
        delete inventory;
    }
}

} // namespace

// 52DEEF, 52E264
void DropPartially(Unit* unit, Inventory* /*unused*/, int32_t money, int32_t main_unit)
{
    if (unit == nullptr || unit->pOwner == nullptr) {
        return;
    }

    if (unit->pOwner->is_ai == 0) {
        Inventory* bag = new Inventory();

        DropRndItems(unit->inventory, bag, Config::InventoryDropProbability);
        DropRndWearedItems(unit, bag, Config::WearDropProbability);

        if (bag->items.GetCount() > 0) {
            PoisonStapleCell(unit->position);
        }

        if (IsGigaPlayer(unit) || IsPurePlayer(unit)) {
            delete bag;
        } else {
            CopyInventoryToMap(unit, bag, money, main_unit);
        }
    } else if (unit->inventory != nullptr) {
        StapleCellOnMobKill(unit);

        CopyInventoryToMap(unit, unit->inventory, money, main_unit);
        unit->inventory = new Inventory();
    }
}
