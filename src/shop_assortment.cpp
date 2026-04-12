#include "shop_assortment.h"

#include "constants.h"
#include "effect.h"
#include "game_app.h"
#include "inventory.h"
#include "item.h"
#include "server.h"

extern "C" void __cdecl sub_54C6DD(CArray<Item*>* arr, Item* item); // add item to sorted CArray, merging stackables
extern "C" void __cdecl sub_54C556(CArray<Item*>* arr); // sort CArray<Item*>
extern "C" Effect* __cdecl sub_54F04F(int32_t is_magical, int32_t magic_volume, int32_t budget, int32_t max_attempts);
extern "C" int32_t __cdecl sub_54F176(Item* item);
extern "C" Effect* __cdecl sub_54EDE9(int32_t is_magical, int32_t item_type, int32_t sub_type, int32_t magic_volume, int32_t budget, int32_t exp, int32_t max_attempts);

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
    int32_t is_something = (item->world_equip->values[0].other_param & 1) != 0;
    int32_t item_type = item->item_type;
    int32_t local_18 = (item->item_id >> 8) & 0xF;

    int32_t budget = this->max_cost * 2 - item->_exp;

    if (!is_something && item->item_type == 2) {        
        int32_t cap = item->_exp * 100;
        if (budget > cap) {
            budget = cap;
        }

        Effect* effect = sub_54F04F(is_something, item->magic_volume, budget, 100);
        if (!effect) {
            return 0;
        }

        item->sub_548FAA(effect);
        item->VMethod15();
        return 1;
    }

    int32_t magic_volume = item->magic_volume;

    Effect* effect = sub_54EDE9(is_something, item_type, local_18, magic_volume, budget, item->_exp, 100);
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
        effect = sub_54EDE9(is_something, item_type, local_18, magic_volume, budget, item->_exp, 100);
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
        effect = sub_54EDE9(is_something, item_type, local_18, magic_volume, budget, item->_exp, 100);
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
