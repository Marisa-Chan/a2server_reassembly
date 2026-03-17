#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "unit.h"

#include "building.h"
#include "buildings_list.h"
#include "effect.h"
#include "eye.h"
#include "game_app.h"
#include "inn.h"
#include "inventory.h"
#include "item.h"
#include "map_stuff.h"
#include "net.h"
#include "player.h"
#include "sack.h"
#include "server.h"
#include "shop.h"
#include "shop_assortment.h"
#include "spell.h"
#include "table.h"
#include "world.h"

//52e9e3
EquipmentExtra::EquipmentExtra() = default;
//52eaa3
EquipmentExtra& EquipmentExtra::operator=(const EquipmentExtra&) = default;

void EquipmentExtra::Serialize(CArchive& ar)
{
    //52ec3d
    if (ar.IsStoring())
    {
        //ar.Write(this, sizeof(EquipmentExtra));
        ar.Write(this, 0x40);
    }
    else
    {
        //ar.Read(this, sizeof(EquipmentExtra));
        ar.Read(this, 0x40);
    }
}

void EquipmentExtra::SetToUnit(Unit *unit)
{
    //52ea17
    unit->speed = speed;
    unit->carrying_body_100g = carrying_body_100g;
    unit->hp_max = hp_max;
    unit->mp_max = mp_max;
    unit->scan_range = scan_range;
    unit->hit_values = hit_values;
    unit->protections = protections;
}

void EquipmentExtra::AddToUnit(Unit *unit)
{
    //52eb64
    unit->speed += speed;

    if (unit->speed < 6)
        unit->speed = 6;

    unit->carrying_body_100g += carrying_body_100g;
    unit->hp_max += hp_max;
    unit->mp_max += mp_max;
    unit->scan_range += scan_range;

    unit->protections += protections;

    unit->hit_values.AddHits(hit_values);
}




/***************   Unit   *******************/
Unit::Unit()
{
    //528aef
    FUN_0052931b("");
    last_hit_by = nullptr;
    last_hit_spell_id = 0;
    spell = nullptr;
}

Unit::Unit(const CString &str)
{
    //528c97
    FUN_0052931b(str);
    last_hit_by = nullptr;
    last_hit_spell_id = 0;
    spell = nullptr;
}

Unit::Unit(const CString &str, const TokenPos *pos)
    : Token(pos)
{
    //528e24
    FUN_0052931b(str);
    last_hit_by = nullptr;
    last_hit_spell_id = 0;
    spell = nullptr;
}

Unit::Unit(const TokenPos *pos)
    : Token(pos)
{
    //528fb7
    FUN_0052931b("");
    last_hit_by = nullptr;
    last_hit_spell_id = 0;
    spell = nullptr;
}

Unit::Unit(const TokenPos *pos, Player *player)
    : Token(pos, player)
{
    //529167
    FUN_0052931b("");
    last_hit_by = nullptr;
    last_hit_spell_id = 0;
    spell = nullptr;
}

Unit::~Unit()
{
    Unit::VMethod10();
}

// CRuntimeClass objects for building sub-types, defined in Main.asm.
extern "C" CRuntimeClass PointerRuntimeClass; // stru_637228
extern "C" CRuntimeClass ShopRuntimeClass;   // stru_637258
extern "C" CRuntimeClass InnRuntimeClass;    // stru_637330

// Free functions called from VMethod2 (all in Main.asm)
extern "C" void __cdecl sub_536630(Unit* self, Unit* target, int* out_charge); // Start attack?
extern "C" void __cdecl sub_53678F(Unit* self, Unit* target); // Execute attack?
extern "C" int32_t __cdecl sub_542216(int32_t n); // RangedRand(n)
extern "C" int32_t sub_530726(int32_t skill_level); // Returns experience required for given skill level.


// 52A857
void Unit::VMethod2()
{
    if (this->some_state == 0x10) {
        return;
    }

    // --- Effect processing ---
    {
        POSITION pos = this->_effects.GetHeadPosition();
        while (pos != nullptr) {
            POSITION cur_pos = pos;
            Effect* effect = this->_effects.GetNext(pos); // returns cur, advances pos to next

            effect->VMethod10(this);

            if (effect->usage_type & 0x80) {
                this->_effects.RemoveAt(cur_pos);
                delete effect;
            }
        }
    }

    // Decay.
    if (hp < 1) {
        this->state = 0;
        this->some_state = 0;
        if (this->decay == 0) {
            this->decay = 1;
            this->sub_52C813();
            this->protections.defense /= 2;
            this->charge_countdown = (int8_t)(this->VMethod25() - 1);
            g_NetStru1_main.sub_519221(this, nullptr, 0x4001, 0xffb, 0, 0);
        } else if (this->charge_countdown < 1) {
            if (this->movement_type > 1)
                this->hp = -1000;
            if (this->hp < -9) {
                this->some_state = 0x10;
                this->sub_52D94E(); // Kill the unit.
            }
        } else {
            this->charge_countdown--;
        }
        return;
    }

    if (g_Server && g_Server->field4_0x74) {
        if (this->pOwner && this == this->pOwner->main_unit && this->sub_52BABD() == 0x50) {
            g_NetStru1_main.sub_519221(this, nullptr, 0x10, 0xffb, 0, 0);
        }
    }

    uint16_t x_wide = this->position->GetXx();
    uint16_t y_wide = this->position->GetYy();
    uint8_t eye_f0 = this->eye->field0_0x0;
    uint8_t eye_f1 = this->eye->field1_0x1;

    if (this->monster_info) {
        g_World->sub_5A9B6B(this);
    }

    switch (this->some_state) {
    case 1: // Move?
        sub_52BDD7(x_wide, y_wide, eye_f0, eye_f1);
        this->some_state2 = 0;
        break;

    case 2: // Pick up sack.
        {
            Sack* sack = MapStuff_Instance->sub_58E5F3(this->position);
            if (sack) {
                g_Server->sub_4F9AD3(sack);
                MapStuff_Instance->sub_58E525(sack);
                POSITION pos = g_Server->srv_stru1->sack_list->list.Find(sack);
                if (pos) {
                    g_Server->srv_stru1->sack_list->list.RemoveAt(pos);
                }
                sub_52C98B(sack);
            }
            break;
        }

    case 3: // Attack.
    case 0xd: // Spell cast.
    case 0xe: // Area spell cast.
        {
            if (this->some_state2 == 0) {
                int32_t charge_extra = 0;
                if (this->some_state == 3) {
                    if (!weapon || !weapon->imbued_spell || !(unit_attrs & 4)) {
                        sub_536630(this, cast_target, &charge_extra);
                    } else {
                        this->some_spell = weapon->imbued_spell;
                        this->some_item  = static_cast<Item*>(weapon);
                        this->some_state = 0xd;
                    }
                }
                if (this->some_state == 0xd || this->some_state == 0xe) {
                    int32_t ok;

                    if (this->some_state == 0xd) {
                        ok = this->some_spell->sub_539958(this, this->cast_target, 0, 0);
                    } else {
                        ok = this->some_spell->sub_539958(this, nullptr, this->area_cast_x, this->area_cast_y);
                    }

                    if (ok) {
                        this->field_0x136 = 0;
                        this->some_state2 = 5;
                        this->charge_countdown = this->charge;
                    }
                } else {
                    this->field_0x136 = 0;
                    this->some_state2 = 5;
                    this->charge_countdown = (int8_t)(this->charge + charge_extra);
                }
            } else if (this->some_state2 == 5) { // Countdown to attack or spell execution.
                this->charge_countdown--;
                if (this->charge_countdown == 0) {
                    if (this->some_state == 3) {
                        if (!weapon || !weapon->imbued_spell || !(unit_attrs & 4)) {
                            sub_53678F(this, cast_target);
                        } else {
                            this->some_spell = weapon->imbued_spell;
                            this->some_item  = static_cast<Item*>(weapon);
                            this->some_state = 0xd;
                        }
                    }
                    if (this->some_state == 0xd || this->some_state == 0xe) {
                        SpellInfo* expected = &g_GameDataRes.spells[this->some_spell->spell_id];
                        if (this->some_spell->spell_info != expected) {
                            uint8_t sid = this->some_spell->spell_id;
                            if (sid == 0 || sid >= 30) {
                                LogMessage(CString("Bad spell data, casting rejected"));
                                break;
                            }
                            LogMessage(CString("Bad spell data, restoring spell"));
                            this->some_spell->spell_info = expected;
                        }
                        if (this->some_state == 0xd) {
                            this->some_spell->sub_539F21(this, this->cast_target);
                        } else {
                            this->some_spell->sub_539F5A(this, nullptr, this->area_cast_x, this->area_cast_y);
                        }

                        if (this->some_item != nullptr) {
                            if (((this->some_item->item_id >> 8) & 0xF) == 0xE) {
                                delete this->some_item;
                                delete this->some_spell;
                            }
                            this->some_item = nullptr;
                            this->some_spell = nullptr;
                        }
                    }

                    this->some_state2 = 7;
                    int32_t relax_extra = 0;
                    if (this->weapon && this->VMethod8()) {
                        relax_extra = (this->weapon->weight + (30 - (int)this->reaction) * 5) / 12;
                        if (relax_extra < 0) {
                            relax_extra = 0;
                        } else if (relax_extra > 12) {
                            relax_extra = 12;
                        }
                    }
                    this->charge_countdown = (int8_t)(this->relax + relax_extra + sub_542216(3));
                    if ((this->some_state == 0xd || this->some_state == 0xe) && this->some_spell != nullptr) {
                        this->charge_countdown += (int8_t)this->some_spell->spell_info->values[0].level;
                    }
                }
            } else if (this->some_state2 == 7) { // Countdown after attack or spell execution.
                this->charge_countdown--;
                if (this->charge_countdown == 0) {
                    this->some_state2 = 0;
                    this->field_0x136 = 1;
                }
            }
            break;
        }

    case 0xf: // Interact with building
        {
            BuildingsList* building_list = g_Server->srv_stru1->building_list;

            Building* building = building_list->sub_558128(this->position);
            if (building) {
                // TODO: also check C++ runtime class.
                if (building->IsKindOf(&ShopRuntimeClass)) { // Shop
                    g_NetStru1_main.FUN_0051cefb(0x83, building->object_info_id, 0, this->pOwner);
                    break;
                }
                if (building->IsKindOf(&InnRuntimeClass)) { // Inn
                    g_NetStru1_main.FUN_0051cefb(0x84, building->object_info_id, building->building_id, this->pOwner);
                    break;
                }
                if (building->IsKindOf(&PointerRuntimeClass)) { // Pointer
                    Pointer* ptr = static_cast<Pointer*>(building);
                    if (ptr->script_instance_id > 0) {
                        g_World->sub_5B0E08(ptr->script_instance_id);
                    }
                    break;
                }
                if (building->typeId == 0x42 && g_CLlDriver.provider == 4) {
                    NetStru2* ns2 = g_NetStru1_main.FUN_00518544(this->pOwner->player_id);
                    if (ns2) {
                        g_NetStru1_main.FUN_005170b6(ns2);
                    }
                    g_NetStru1_main.FUN_0051800f();
                    this->pOwner->field_0xa50 = (uint32_t)g_Server->tick16;
                }
                // Toggles.
                if (building->typeId == 0x1c || building->typeId == 0x1d || building->typeId == 0x4c || building->typeId == 0x4d) {
                    building->hp = (building->hp == 0) ? 1 : 0;
                    g_NetStru1_main.sub_51AC77(building, nullptr, 0);
                }
            }

            Building* near_building = building_list->sub_557EA5(this->position);
            if (near_building) {
                if (near_building->object_info_id == 0x0f && near_building->hp > 0) {
                    near_building->hp--;
                    g_NetStru1_main.sub_51AC77(near_building, nullptr, 0);
                    this->VMethod13(new Item(CString("Potion Big Healing")));
                } else if (near_building->object_info_id == 0x10 && near_building->hp > 0) {
                    near_building->hp--;
                    g_NetStru1_main.sub_51AC77(near_building, nullptr, 0);
                    this->VMethod13(new Item(CString("Potion Big Mana")));
                }
            }
            break;
        }
    }
}

int32_t Unit::VMethod3()
{
    return token_size;
}

int32_t Unit::VMethod4()
{
    return movement_type;
}

int32_t Unit::VMethod7()
{
    return 1;
}

int32_t Unit::VMethod8()
{
    return 0;
}

void Unit::VMethod10()
{
    // 52a4fa
    if (some_item && ((some_item->item_id >> 8) & 0xF) == 0xE) {
        delete some_item;
        some_item = nullptr;
        if (some_spell) {
            delete some_spell;
        }
        some_spell = nullptr;
    }

    if (eye) {
        ::operator delete(eye);
        eye = nullptr;
    }

    if (eye2) {
        eye2->sub_5A4F30();
        ::operator delete(eye2);
        eye2 = nullptr;
    }

    if (inventory) {
        delete inventory;
        inventory = nullptr;
    }

    if (shield) {
        delete shield;
        shield = nullptr;
    }

    if (weapon) {
        delete weapon;
        weapon = nullptr;
    }

    spell = nullptr;

    if (spell_book) {
        delete spell_book;
        spell_book = nullptr;
    }
}

Item *Unit::VMethod12(Item *item)
{
    Item *itm = item->VMethod10(this);
    VMethod19();
    return itm;
}

void Unit::VMethod13(Item *item)
{
    Item* itm = item->VMethod10(this);
    if (itm)
    {
        inventory->PutItemIntoBagAtDefault(itm);
        VMethod19();
    }
}

Item* Unit::Unequip(Item* item)
{
    if (item)
    {
        item->VMethod11(this);
        VMethod19();
    }
    return item;
}

void Unit::VMethod15()
{}

void Unit::VMethod16(Item*)
{}

void Unit::VMethod18()
{
    if (!pOwner)
        mp2 = mp_max;
    else
        mp2 = (pOwner->field_0xa5c * mp_max) / 100;
}

uint32_t Unit::VMethod19()
{
    return _exp;
}

void Unit::VMethod20()
{}

void Unit::VMethod21(uint32_t newExp, Unit *target, int32_t sphere)
{
    experience += newExp;
    if (target && target->hp < 0)
        pOwner->monster_kills += 1;
}

void Unit::VMethod22(Unit*, int32_t)
{}

void Unit::VMethod23(Unit*, uint32_t, int32_t, int32_t)
{}

void Unit::VMethod24(Unit*, uint32_t, int32_t, int32_t)
{}

int32_t Unit::VMethod25()
{
    return monster_info->values[0].dying_time;
}

// 52C98B
void Unit::sub_52C98B(Sack* sack)
{
    if (sack == nullptr) {
        return;
    }

    uint32_t update_mask = 0;

    // Give gold to owner.
    if (sack->money > 0) {
        this->pOwner->FUN_00534AC1(sack->money, 0);
    }

    if (sack->inventory != nullptr) {
        // Safe iteration: cache next pointer before processing, in case node is removed.
        auto* cur_node  = sack->inventory->items.m_pNodeHead;
        Item* item      = cur_node ? cur_node->data  : nullptr;
        auto* next_node = cur_node ? cur_node->pNext : nullptr;

        while (item != nullptr) {
            if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
                bool apply_buff = false;

                // Check for consumable buff scrolls.
                if (item->world_equip->name.Find("Scroll Protection") != -1 ||
                        item->world_equip->name.Find("Scroll Bless") != -1 ||
                        item->world_equip->name.Find("Scroll Shield") != -1 ||
                        item->world_equip->name.Find("Scroll Haste") != -1 ||
                        item->world_equip->name.Find("Scroll Invisibility") != -1) {
                    sack->inventory->sub_574C20(cur_node);
                    apply_buff = true;
                } else if (item->world_equip->name.Find("Quest Meta") == 0) {
                    // "Quest Meta X": digit at name[10] gives per-sphere skill boost.
                    int16_t boost = (item->world_equip->name[10] - '0') * 5;
                    sack->inventory->sub_574C20(cur_node);
                    delete item;
                    item = nullptr;

                    Humanoid* human = static_cast<Humanoid*>(this);
                    human->experience = 0;
                    for (int sphere = 1; sphere < 6; ++sphere) {
                        int16_t new_lvl = (std::min)(human->hit_values.skill_levels[sphere] + boost, 100);
                        human->hit_values.skill_levels[sphere] = new_lvl;
                        human->hit_values2.skill_levels[sphere] = new_lvl;
                        int32_t exp_val = sub_530726(new_lvl);
                        human->experience_per_sphere[sphere - 1] = exp_val;
                        human->experience += exp_val;
                    }
                    human->VMethod18();
                    update_mask = 0x31f0f;
                    g_NetStru1_main.FUN_0051ce86(9, boost, human->pOwner);
                } else {
                    item->TokenID = 1;
                }

                // In team mode, discard buff if this player is the current rune-holder.
                if (g_ServerConfig.gameType == 2 && apply_buff && item != nullptr) {
                    uint32_t player_id = (this->pOwner->field_0xa70 == 0) ? g_Server->field61_0x210 : g_Server->field60_0x20c;
                    if (player_id == this->pOwner->player_id) {
                        apply_buff = false;
                        delete item;
                        item = nullptr;
                    }
                }

                // Apply buff scroll effect to self.
                if (apply_buff && item != nullptr) {
                    Effect* effect = item->_effects.m_nCount > 0 ? item->_effects.m_pNodeHead->data : nullptr;
                    if (effect != nullptr) {
                        this->some_item = item;
                        Spell* spl = new Spell(static_cast<uint8_t>(effect->spell_or_damage));
                        spl->sub_539541(static_cast<uint8_t>(effect->spell_value));
                        spl->sub_539F5A(this, this, 0, 0);
                        if (this->some_item != nullptr) {
                            delete this->some_item;
                            this->some_item = nullptr;
                        }
                    }
                    item = nullptr;
                }

                // Quest rune logic (team deathmatch only).
                if (g_ServerConfig.gameType == 2 && item != nullptr) {
                    bool rune_scored = false;

                    if (item->world_equip->name.Find("Quest RuneF") != -1) {
                        // Original logic: remove item from inventory and call `sub_57b990`. Looks equivalent to calling `sub_574C20`.
                        sack->inventory->sub_574C20(cur_node);

                        delete item;
                        item = nullptr;

                        if (this->pOwner->field_0xa70 == 0) {
                            g_Server->sub_4F8FBF(0, 0);
                            g_NetStru1_main.FUN_0051ce86(0x100, 0, nullptr);
                        } else {
                            rune_scored = true;
                            g_Server->field60_0x20c = this->pOwner->player_id;
                            g_Server->field62_0x214 = 0;
                            g_NetStru1_main.FUN_0051d6b4(0);
                            g_NetStru1_main.FUN_0051ce86(0x102, this->pOwner->player_id, nullptr);
                        }
                    } else if (item->world_equip->name.Find("Quest RuneA") != -1) {
                        sack->inventory->sub_574C20(cur_node);
                        delete item;
                        item = nullptr;
                        if (this->pOwner->field_0xa70 == 1) {
                            g_Server->sub_4F8FBF(1, 0);
                            g_NetStru1_main.FUN_0051ce86(0x101, 0, nullptr);
                        } else {
                            rune_scored = true;
                            g_Server->field61_0x210 = this->pOwner->player_id;
                            g_Server->field63_0x218 = 0;
                            g_NetStru1_main.FUN_0051d6b4(0);
                            g_NetStru1_main.FUN_0051ce86(0x103, this->pOwner->player_id, nullptr);
                        }
                    }

                    // Rune scored: flush all active effects on this unit.
                    if (rune_scored) {
                        POSITION effect_pos = this->_effects.GetHeadPosition();
                        while (effect_pos != nullptr) {
                            POSITION cur_pos = effect_pos;
                            Effect* effect = this->_effects.GetNext(effect_pos);

                            effect->spell_value = 1;
                            effect->field_0x44 = 0;
                            effect->VMethod10(this);

                            if (effect->usage_type & 0x80) {
                                this->_effects.RemoveAt(cur_pos);
                                delete effect;
                            }
                        }
                    }
                }
            }

            // Advance (safe against node removal during iteration).
            if (next_node == nullptr) {
                item = nullptr;
            } else {
                item = next_node->data;
                next_node = next_node->pNext;
            }
        } // end item loop

        // Transfer all remaining items from sack to this unit's inventory.
        this->inventory->sub_552A42(sack->inventory);
        sack->inventory = nullptr;
        this->sub_52A790(0); // Add weight.
    } // end if sack->inventory

    g_NetStru1_main.sub_51C61E(sack);
    delete sack;

    g_NetStru1_main.sub_519221(this, nullptr, update_mask | 0x282000, 0xffb, 0, 0);
}

// 52D94E
void Unit::sub_52D94E()
{
    // Summoned units: remove completely, don't leave a corpse.
    if (this->summon_id != 0) {
        this->hp = -10000;
        this->summon_id = 0;
    }

    this->some_state = 0x10;
    this->state = 0x10;

    MapStuff_Instance->sub_58E3D1(this);

    // Inline sub_58FD16: reset subcell position to cell center.
    this->position->x_subcell = 0x80;
    this->position->y_subcell = 0x80;

    if (g_ServerConfig.gameType == 0 || (g_ServerConfig.gameType == 3 && this->typeId >= 0x40)) {
        if (this->shield != nullptr) {
            this->inventory->PutItemIntoBagAtDefault(this->Unequip(this->shield));
        }

        // Unequip weapon if it has a non-zero 'other_param' (why?).
        if (this->weapon != nullptr && this->weapon->world_equip->values.GetData()[0].other_param != 0) {
            this->inventory->PutItemIntoBagAtDefault(this->Unequip(this->weapon));
        }

        // VMethod15: unequip all remaining equipped items.
        this->VMethod15();

        bool is_npc = (this->monster_info->name.Find("NPC") != -1);
        bool should_drop = false;

        if (this->typeId >= 0x40) {
            int roll = sub_542216(99); // [0, 99]
            should_drop = (roll < 100 - g_ServerConfig.field_0xc4);
        }

        // Reset inventory if NPC, summoned, or flagged for drop.
        if (is_npc || this->summoned != 0 || should_drop) {
            delete this->inventory;
            this->inventory = new Inventory();
        }

        int32_t gold = 0;

        if (this->typeId >= 0x40 && this->summoned == 0) {
            MonsterInfoData& monster_data = this->monster_info->values.GetData()[0];

            // Gold drop.
            if (sub_542216(100) < monster_data.treasure_gold) {
                gold = monster_data.treasure_gold_min + sub_542216(monster_data.treasure_gold_max);
            }

            // Item drop.
            if (sub_542216(100) < monster_data.treasure_item) {
                ShopAssortment assort;

                AssortGenParams params;
                params.min_cost = monster_data.treasure_item_min;
                params.max_cost = monster_data.treasure_item_max;
                params.max_count = 100;
                params.max_same_count = 1;
                params.flags = monster_data.treasure_item_mask;

                assort.GenerateAssortment(&params);

                CArray<Item*> result;
                assort.ArrangeShelfs(100, 1, params.min_cost, params.max_cost, &result);

                if (result.GetSize() > 0) {
                    int idx = sub_542216(result.GetSize() - 1); // [0, m_nSize-1]
                    Item* item = result[idx];
                    item->count = 1;
                    this->inventory->PutItemIntoBagAtDefault(item);
                }
                // assort destructor deletes all items in assort.items.
                // result destructor frees the pointer array (items not deleted here).
            }
        }

        // Create sack if inventory has items or gold was dropped.
        if ((this->inventory->items.m_nCount > 0 || gold > 0) && this->pOwner != nullptr) {
            int is_main = (this->pOwner->main_unit == this) ? 1 : 0;
            // Inline sub_52D8D3: drop inventory as a sack on the map.
            g_Server->srv_stru1->sack_list->sub_554927(this->position, this->inventory, gold, is_main);
            delete this->inventory;
            this->inventory = new Inventory();
        }
    } else if (this->pOwner != nullptr && g_ServerConfig.gameType != 3) {
        if (g_ServerConfig.gameType == 2) {
            // Check if this unit's player is holding the opposing team's rune.
            int32_t opposite_team = 1 - this->pOwner->field_0xa70;
            int32_t rune_holder = (opposite_team == 0)
                ? g_Server->field60_0x20c
                : g_Server->field61_0x210;

            if (rune_holder == this->pOwner->player_id) {
                // Drop the appropriate rune item.
                Item* rune;
                if (this->pOwner->field_0xa70 == 0) {
                    // Blue team was holding red's rune.
                    rune = new Item(CString("Quest RuneA"));
                } else {
                    // Red team was holding blue's rune.
                    rune = new Item(CString("Quest RuneF"));
                }
                this->inventory->PutItemIntoBagAtDefault(rune);

                // Clear rune-holder state and flag rune as on the ground.
                if (opposite_team == 0) {
                    g_Server->field60_0x20c = 0;
                    g_Server->field62_0x214 = 1;
                } else {
                    g_Server->field61_0x210 = 0;
                    g_Server->field63_0x218 = 1;
                }

                g_NetStru1_main.FUN_0051ce86(this->pOwner->field_0xa70 ? 0x104 : 0x105, this->pOwner->player_id, nullptr);
                g_NetStru1_main.FUN_0051d6b4(0);
            }
        }

        // Create sack if inventory has items.
        if (this->inventory->items.m_nCount > 0) {
            int is_main = (this->pOwner->main_unit == this) ? 1 : 0;
            g_Server->srv_stru1->sack_list->sub_554927(this->position, this->inventory, 0, is_main);
            delete this->inventory;
            this->inventory = new Inventory();
        }
    }

    {
        POSITION pos = this->_effects.GetHeadPosition();
        while (pos != nullptr) {
            POSITION cur_pos = pos;
            Effect* effect = this->_effects.GetNext(pos);

            // Inline sub_53EE08: check `usage_type & 3`.
            if (effect->usage_type & 3) {
                effect->spell_value = 1;
                effect->VMethod10(this);

                if (effect->usage_type & 0x80) {
                    this->_effects.RemoveAt(cur_pos);
                    delete effect;
                }
            }
        }
    }

    // Hero-death: increment death counter and notify clients.
    if (this->pOwner != nullptr && this->pOwner->main_unit == this) {
        this->pOwner->deaths += 1;
        g_NetStru1_main.sub_519221(this, this->pOwner, 0x2400000, 0xFFB, 0, 0);
        g_Server->sub_4EE028(this);
        this->hp = -50;
    }

    // Respawn notification for player character types (typeId 0x21-0x3F).
    if (!g_Server->field4_0x74 && this->pOwner != nullptr && !this->pOwner->is_ai && 0x21 <= this->typeId && this->typeId <= 0x3F) {
        this->pOwner->field_0x40 = (uint8_t)(this->server_id - 0x13);
        g_NetStru1_main.FUN_0051cefb(0xb4, this->pOwner->field_0x40, 0, this->pOwner);
    }

    this->sub_52E7FA();
}

IMPLEMENT_SERIAL(Unit, Token, 1);

/*  Created by IMPLEMENT_SERIAL
// 6363e8
AFX_DATA CRuntimeClass Unit::classUnit =
{ "Unit", sizeof(Unit), 1, &Unit::CreateObject, &Token::classCObject, NULL };

CObject* PASCAL Unit::CreateObject() // 528a35
CRuntimeClass* Unit::GetRuntimeClass() const //528aa5

CArchive& AFXAPI operator>>(CArchive& ar, Unit* &pOb) //528ad3

//528ab5
static const AFX_CLASSINIT _init_Unit((CRuntimeClass*)&Unit::classUnit);
*/







/***************   Humanoid   *******************/

IMPLEMENT_SERIAL(Humanoid, Unit, 1);
/*
* Humanoid::classHumanoid   636400
* CreateObject     52fee4
* GetRuntimeClass  52ff54
* _init_Humanoid   52ff64
* operator>>       52ff82
*/


Humanoid::Humanoid()
{
    //52ff9e
    InitStats();
}

Humanoid::Humanoid(const TokenPos *tpos)
 :Unit(tpos)
{
    //52fff2
    InitStats();
}

Humanoid::~Humanoid()
{
    //53015b
    Humanoid::VMethod10();
}

void Humanoid::VMethod5()
{
    //55ab26
    Unit::VMethod5();
}

int32_t Humanoid::VMethod7()
{
    //57bb50
    return 1;
}

int32_t Humanoid::VMethod8()
{
    //57bb60
    return 1;
}

void Humanoid::VMethod10()
{
    //5301ac
    Unit::VMethod10();
    for (int i = 1; i < equipment.size(); i++)
    {
        Item*& rEq = equipment[i];
        if (rEq)
        {
            delete rEq;
            rEq = nullptr;
        }
    }
}



void Humanoid::VMethod13(Item *item)
{
    //5304b2
    Item *itm = VMethod12(item);
    if (itm)
        inventory->PutItemIntoBagAtDefault(itm);
}

Item *Humanoid::Unequip(Item* item)
{
    //5304e8
    if (item)
        item->VMethod11(this);
    return item;
}


void Humanoid::VMethod15()
{
    //530511
    for (int i = 1; i < equipment.size(); i++)
    {
        Item *itm = equipment[i];
        Unequip(itm);
        inventory->PutItemIntoBagAtDefault(itm);
    }
}

/*void Humanoid::VMethod16(Item* item)
{
    //53111b
    
}*/

void Humanoid::VMethod18()
{
    // 005315ec
    const bool fighter = (unit_attrs & 4) == 0;
    const bool female = (typeId == 0x22 || typeId == 0x24);

    uint16_t max_body, max_reaction, max_mind, max_spirit;

    if (fighter) {
        if (female) {
            max_body = 50;
            max_reaction = 52;
            max_mind = 46;
            max_spirit = 48;
        } else {
            max_body = 52;
            max_reaction = 50;
            max_mind = 48;
            max_spirit = 46;
        }
    } else {
        if (female) {
            max_body = 46;
            max_reaction = 48;
            max_mind = 50;
            max_spirit = 52;
        } else {
            max_body = 48;
            max_reaction = 46;
            max_mind = 52;
            max_spirit = 50;
        }
    }

    body = min(body, max_body + equipment_extra.body);
    reaction = min(reaction, max_reaction + equipment_extra.reaction);
    mind = min(mind, max_mind + equipment_extra.mind);
    spirit = min(spirit, max_spirit + equipment_extra.spirit);

    hp_max = static_cast<int16_t>(body * (fighter ? 2 : 1));
    if (hp_max > 0) {
        const double exp_factor = std::log(static_cast<double>(experience) / 5000.0 + 1.0) / std::log(1.1);
        hp_max = static_cast<int16_t>(hp_max + (fighter ? 2.0 : 1.0) * exp_factor);
        const double body_factor = std::pow(1.1, static_cast<double>(body)) / 100.0 + 1.0;
        hp_max = static_cast<int16_t>(hp_max * body_factor);
    }

    if (mp_max == 0) {
        mp = 0;
    } else {
        mp_max = static_cast<int16_t>(spirit * 2);
        const double exp_factor = std::log(static_cast<double>(experience) / 5000.0 + 1.0) / std::log(1.1);
        mp_max = static_cast<int16_t>(mp_max + (fighter ? 1.0 : 2.0) * exp_factor);
        const double spirit_factor = std::pow(1.1, static_cast<double>(spirit)) / 100.0 + 1.0;
        mp_max = static_cast<int16_t>(mp_max * spirit_factor);
    }

    scan_range = static_cast<uint16_t>(((mind + reaction) / 25.0 + 4.0) * 256.0);
    carrying_body_100g = static_cast<uint16_t>(body * 10 + 1);

    speed = (reaction < 12) ? reaction : reaction / 5 + 12;
    if (typeId == 0x13 || typeId == 0x15) { // Horse-riders, with sword and with lance.
        speed += 10;
    }

    carrying_weight_100g = extra_carrying_weight;
    if (inventory) {
        if (inventory->total_weight < 64000) {
            carrying_weight_100g = static_cast<uint16_t>(carrying_weight_100g + (inventory->total_weight / 2));
        }  else {
            carrying_weight_100g = 32000;
        }
    }

    if (carrying_body_100g != 0 && carrying_body_100g <= carrying_weight_100g) {
        const uint16_t penalty = carrying_weight_100g / carrying_body_100g;
        const int16_t adjusted = static_cast<int16_t>(speed) - penalty;
        speed = static_cast<uint16_t>(adjusted < 6 ? 6 : adjusted);
    }

    const double body_pow = std::pow(1.1, static_cast<double>(body));
    hit_values.hand_damage_spread = static_cast<uint8_t>(body_pow / 20.0);
    hit_values.hand_damage_min = hit_values.hand_damage_spread;

    const double reaction_pow = std::pow(1.1, static_cast<double>(reaction));
    hit_values.attack = static_cast<uint16_t>((body_pow + reaction_pow) / 5.0);

    for (int i = 1; i < static_cast<int>(hit_values.skill_levels.size()); i++) {
        hit_values.skill_levels[i] = hit_values2.skill_levels[i];
    }

    for (int i = 1; i < static_cast<int>(hit_values.skill_levels.size()); i++) {
        if (hit_values.skill_levels[i] > 100) {
            hit_values.skill_levels[i] = 100;
        }
    }

    if (hit_values.physical_damage_type != 0 && hit_values.physical_damage_type < hit_values.skill_levels.size()) {
        const uint16_t lvl = hit_values.skill_levels[hit_values.physical_damage_type];
        hit_values.attack = static_cast<uint16_t>(hit_values.attack + lvl * 3);
        hit_values.hand_damage_min = static_cast<uint8_t>(hit_values.hand_damage_min + (lvl / 5));
    }

    hit_values.some_damage_min = 0;
    hit_values.some_damage_spread = 0;
    hit_values.some_damage2_min = 0;
    hit_values.some_damage2_spread = 0;

    protections.Clear();
    protections.defense = reaction / 3;
    for (size_t i = 1; i < protections.magic_protections.size(); i++) {
        protections.magic_protections[i] = spirit / 2;
    }

    equipment_extra.AddToUnit(this);

    if (hp > hp_max) {
        hp = hp_max;
    }

    if (mp > mp_max) {
        mp = mp_max;
    }
    if (mp < 0) {
        mp = 0;
    }

    if (!pOwner) {
        mp2 = mp_max;
    } else {
        mp2 = static_cast<int16_t>((pOwner->field_0xa5c * mp_max) / 100);
    }

    if (eye) {
        eye->rotation_speed = static_cast<uint8_t>(speed);
    }

    protections.defense = max(0, protections.defense);
    protections.absorption = max(0, protections.absorption);
    carrying_weight_100g = max(0, carrying_weight_100g);

    const uint16_t max_magic = spirit / 2 + 70;
    const uint16_t ceiling = min(100, max_magic);
    for (size_t i = 1; i < protections.magic_protections.size(); i++) {
        uint16_t value = protections.magic_protections[i];
        protections.magic_protections[i] = min(value, ceiling);
    }

    if (spell_book != nullptr) {
        spell_book->RefreshForHumanoid(this);
    }
}

uint32_t Humanoid::VMethod19()
{
    //537199
    uint32_t exps = 0;
    for (uint32_t ex : experience_per_sphere)
        exps += ex;

    _exp = (exps * 0.01);
    return _exp;
}

/*
int32_t Humanoid::VMethod25()
{
    //530394
    if ()
}
*/




/***************   Human   *******************/

IMPLEMENT_SERIAL(Human, Humanoid, 1);
/*
* Human::classHuman   636418
* CreateObject     532273
* GetRuntimeClass  5322e3
* _init_Human      5322f3
* operator>>       532311
*/

Human::Human()
{
    //53232d
    FUN_00532587(CString("Man_Unarmed"), 0, nullptr);
}

Human::Human(const CString& hname, int32_t t, const char* unk)
{
    //53239a
    FUN_00532587(hname, t, unk);
}

Human::~Human()
{}
