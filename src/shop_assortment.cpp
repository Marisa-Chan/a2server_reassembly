#include "shop_assortment.h"

#include <cmath>

#include "constants.h"
#include "effect.h"
#include "game_app.h"
#include "inventory.h"
#include "item.h"
#include "server.h"
#include "table.h"

extern "C" void __cdecl sub_54C6DD(CArray<Item*>* arr, Item* item); // add item to sorted CArray, merging stackables
extern "C" void __cdecl sub_54C556(CArray<Item*>* arr); // sort CArray<Item*>
extern "C" Effect* __cdecl sub_54F04F(int32_t is_warrior, int32_t magic_volume, int32_t budget, int32_t max_attempts);

static const uint16_t warrior_spells[] = { spell::stone_curse, spell::drain_life };
static const uint16_t mage_spells[] = { spell::fire_arrow, spell::lightning, spell::prismatic_spray, spell::stone_curse, spell::drain_life, spell::ice_missile, spell::stone_missile };

// 54A13B
ShopAssortment::ShopAssortment() {
    this->min_cost = 0;
    this->max_cost = 0;
    this->sub_54C08A();
}

// 54A2DD
ShopAssortment::~ShopAssortment() {
    this->sub_54BC09();
    this->arr.SetSize(0, -1);
    this->min_cost = 0;
    this->max_cost = 0;
}

// 54EDE9
Effect* __cdecl sub_54EDE9(int32_t is_warrior, int32_t item_type, int32_t sub_type, int32_t magic_volume, int32_t budget, int32_t exp, int32_t max_attempts) {
    if (budget <= 0 || magic_volume <= 0) {
        return nullptr;
    }

    int32_t column_index = sub_type - 1 + (1 - is_warrior) * 12;

    Effect* effect = new Effect();

    int32_t attempts = 0;
    int32_t found = -1;
    int32_t result = 0;

    while (attempts < max_attempts) {
        attempts++;

        int32_t last_index = g_GameDataRes.magics.GetSize() - 1;
        int32_t* last_slots = &g_GameDataRes.magics[last_index].Values()[0].in_warrior_weapon;
        int32_t total_weight = last_slots[column_index];

        int32_t roll = Random1N(total_weight);

        for (int32_t i = 1; i <= last_index; i++) {
            if (g_GameDataRes.magics[i].name.GetLength() == 0) {
                continue;
            }

            int32_t* prev_slots = &g_GameDataRes.magics[i - 1].Values()[0].in_warrior_weapon;
            int32_t prev_weight = prev_slots[column_index];

            int32_t* curr_slots = &g_GameDataRes.magics[i].Values()[0].in_warrior_weapon;
            int32_t curr_weight = curr_slots[column_index];

            if (roll > prev_weight && roll <= curr_weight) {
                found = i;
                break;
            }
        }

        if (found != -1) {
            break;
        }
    }

    if (found != -1) {
        if (!is_warrior && item_type == 2) {
            found = modifier::castspell;
        }

        effect->effect_id = found;

        if (found == modifier::castspell) {
            if (is_warrior) {
                effect->spell_or_damage = warrior_spells[Random0N(1)];
            } else {
                effect->spell_or_damage = mage_spells[Random0N(6)];
            }
        }

        if (effect->effect_id == modifier::castspell) {
            if (is_warrior) {
                budget = exp * 10;
            } else {
                budget = exp * 100;
            }
        }

        result = effect->sub_541FD7(budget, magic_volume);
    }

    if (attempts == max_attempts || result == -1) {
        delete effect;
        return nullptr;
    }

    effect->VMethod17(result);
    return effect;
}

// 54BF6B
void ShopAssortment::GenerateAssortment(AssortGenParams* params) {
    this->sub_54BC09();
    this->min_cost = params->min_cost;
    this->max_cost = params->max_cost;
    this->flags = params->flags;
    if ((this->flags & 0x400000) != 0) {
        this->sub_54ACCB(&g_GameDataRes.weapons, (this->flags & 0xF23FFFFF) | 0x400000);
    }
    if ((this->flags & 0x8000000) != 0) {
        this->sub_54ACCB(&g_GameDataRes.weapons, (this->flags & 0xF23FFFFF) | 0x8000000);
    }
    if ((this->flags & 0x1000000) != 0) {
        this->sub_54ACCB(&g_GameDataRes.armors, (this->flags & 0xF23FFFFF) | 0x1000000);
    }
    if ((this->flags & 0x800000) != 0) {
        this->sub_54ACCB(&g_GameDataRes.shields, (this->flags & 0xF23FFFFF) | 0x800000);
    }
    if ((this->flags & 0x4000000) != 0) {
        this->sub_54AF21(&g_GameDataRes.magic_items);
    }
}

// 54BC09
void ShopAssortment::sub_54BC09() {
    for (int32_t i = 0; i < this->items.GetSize(); i++) {
        delete this->items[i];
    }
    this->items.SetSize(0, -1);
}

// 54C08A
void ShopAssortment::sub_54C08A() {
    this->arr.SetSize(0x400, -1);
    for (int32_t i = 0; i < this->arr.GetSize(); i++) {
        double x = (3.2 / this->arr.GetSize()) * i + -1.6;
        this->arr[i] = 1.0 / std::exp(x * x);
    }
    this->field_0x18 = 1.0 / std::exp(2.56);
}

// 54ACCB
void ShopAssortment::sub_54ACCB(CArray<WorldEquip>* items, uint32_t flags) {
    uint32_t param_5 = (flags & 0x20000000) != 0;
    uint32_t low_flags = flags & 0x7FFF;
    uint32_t shape_flags = (flags & 0x3F8000) >> 0xF;
    uint32_t type_flags = flags & 0xFC00000;

    CArray<uint32_t> shape_ids;
    CArray<uint32_t> material_ids;

    int32_t index = 0;
    while (shape_flags != 0) {
        if ((shape_flags & 1) != 0) {
            shape_ids.SetAtGrow(shape_ids.GetSize(), index);
        }
        index++;
        shape_flags >>= 1;
    }

    index = 0;
    while (low_flags != 0) {
        if ((low_flags & 1) != 0) {
            material_ids.SetAtGrow(material_ids.GetSize(), index);
        }
        index++;
        if (index == 0xD) {
            index++;
        }
        low_flags >>= 1;
    }

    for (int32_t i = 0; i < shape_ids.GetSize(); i++) {
        for (int32_t j = 0; j < material_ids.GetSize(); j++) {
            if ((type_flags & 0x400000) != 0) {
                this->sub_54A420(items, shape_ids[i], material_ids[j], 2, 0);
            }
            if ((type_flags & 0x1000000) != 0) {
                this->sub_54A420(items, shape_ids[i], material_ids[j], 1, param_5);
            }
            if ((type_flags & 0x800000) != 0) {
                this->sub_54A420(items, shape_ids[i], material_ids[j], 7, 0);
            }
            if ((type_flags & 0x8000000) != 0 && param_5 != 0) {
                this->sub_54A420(items, shape_ids[i], material_ids[j], 8, param_5);
            }
        }
    }
}

// 54D423
Inventory* ShopAssortment::ArrangeShelfs(int32_t max_count, int32_t max_same, int32_t min_cost, int32_t max_cost, CArray<Item*>* output) {
    CArray<Item*> item_array;

    if (this->items.GetSize() == 0) {
        return nullptr;
    }

    if ((this->flags & 0x4000000) != 0) {
        for (int32_t i = 0; i < this->items.GetSize(); i++) {
            if (this->items[i]->item_type == 5) {
                this->items[i]->count = 2;

                Item* item = this->items[i]->TakeOne();
                item_array.SetAtGrow(item_array.GetSize(), item);
            }
        }

        if (g_ServerConfig.gameType != 3) {
            Item* item;

            item = new Item("Potion Health Regeneration");
            item->count = Random1N(50) + 50;
            item_array.SetAtGrow(item_array.GetSize(), item);

            item = new Item("Potion Medium Healing");
            item->count = Random1N(50) + 50;
            item_array.SetAtGrow(item_array.GetSize(), item);

            item = new Item("Potion Big Healing");
            item->count = Random1N(50) + 50;
            item_array.SetAtGrow(item_array.GetSize(), item);
        }

        Item* item;

        item = new Item("Potion Mana Regeneration");
        item->count = Random1N(50) + 50;
        item_array.SetAtGrow(item_array.GetSize(), item);

        item = new Item("Potion Medium Mana");
        item->count = Random1N(50) + 50;
        item_array.SetAtGrow(item_array.GetSize(), item);

        item = new Item("Potion Big Mana");
        item->count = Random1N(50) + 50;
        item_array.SetAtGrow(item_array.GetSize(), item);
    }

    if (max_count > 1000) {
        HWND hWnd = nullptr;
        CWinThread* thread = AfxGetThread();
        if (thread != nullptr) {
            CWnd* main_wnd = thread->GetMainWnd();
            if (main_wnd != nullptr) {
                hWnd = main_wnd->m_hWnd;
            }
        }
        MessageBoxA(hWnd, "Amount > 1000", "Shop Error", 0);
    }

    int32_t retry_count = 0;
    for (int32_t i = 0; i < max_count && retry_count <= max_count * 10; i++) {
        int32_t attempts = 0;
        int32_t pick;
        do {
            attempts++;
            pick = Random0N(this->items.GetSize() - 1);
            if (this->items[pick]->item_type != 5) {
                break;
            }
        } while (attempts < 1000);

        if (attempts >= 1000) {
            break;
        }

        this->items[pick]->count = 2;
        Item* item = this->items[pick]->TakeOne();

        if ((this->flags & 0x20000000) != 0) {
            int32_t ok;
            if ((this->flags & 0x10000000) != 0) {
                if (Random0N(100) > 50) {
                    ok = this->sub_54EA76(item);
                } else {
                    ok = 1;
                }
            } else {
                ok = this->sub_54EA76(item);
            }

            if (ok == 0 || item->_exp < min_cost || item->_exp > max_cost) {
                i--;
                retry_count++;
                delete item;
                continue;
            }
        }

        if (item->VMethod16() != 0) {
            item->count = Random0N(max_same) + 1;
        } else {
            item->count = 1;
        }
        sub_54C6DD(&item_array, item);
    }

    sub_54C556(&item_array);

    if (output == nullptr) {
        Inventory* inv = new Inventory();
        for (int32_t i = 0; i < item_array.GetSize(); i++) {
            inv->PutItemIntoBagAtDefault(item_array[i]);
        }
        return inv;
    }
    
    output->SetSize(item_array.GetSize());
    for (int32_t i = 0; i < item_array.GetSize(); i++) {
        output->GetData()[i] = item_array[i];
    }
    return nullptr;
}

// 54EA76
int32_t ShopAssortment::sub_54EA76(Item* item) {
    int32_t is_warrior = (item->world_equip->Values()[0].other_param & 1) != 0;
    int32_t item_type = item->item_type;
    int32_t local_18 = (item->item_id >> 8) & 0xF;

    int32_t budget = this->max_cost * 2 - item->_exp;

    if (!is_warrior && item->item_type == 2) {        
        int32_t cap = item->_exp * 100;
        if (budget > cap) {
            budget = cap;
        }

        Effect* effect = sub_54F04F(is_warrior, item->magic_volume, budget, 100);
        if (!effect) {
            return 0;
        }

        item->sub_548FAA(effect);
        item->VMethod15();
        return 1;
    }

    int32_t magic_volume = item->magic_volume;

    Effect* effect = sub_54EDE9(is_warrior, item_type, local_18, magic_volume, budget, item->_exp, 100);
    if (!effect) {
        return 0;
    }

    item->sub_548FAA(effect);
    item->VMethod15();

    if (effect->effect_id == modifier::castspell) {
        return 1;
    }

    // Second effect attempt (50% chance)
    budget = this->max_cost * 2 - item->_exp;
    magic_volume = item->magic_volume - sub_54F176(item);

    if (Random0N(100) < 50) {
        effect = sub_54EDE9(is_warrior, item_type, local_18, magic_volume, budget, item->_exp, 100);
        if (!effect) {
            return 1;
        }
        if (effect->effect_id == modifier::castspell) {
            delete effect;
            return 1;
        }
        item->sub_548FAA(effect);
        item->VMethod15();
    }

    // Third effect attempt (25% chance)
    budget = this->max_cost * 2 - item->_exp;
    magic_volume = item->magic_volume - sub_54F176(item);

    if (Random0N(100) < 25) {
        effect = sub_54EDE9(is_warrior, item_type, local_18, magic_volume, budget, item->_exp, 100);
        if (!effect) {
            return 1;
        }
        if (effect->effect_id == modifier::castspell) {
            delete effect;
            return 1;
        }
        item->sub_548FAA(effect);
        item->VMethod15();
    }

    item->magic_volume -= sub_54F176(item);
    return 1;
}
