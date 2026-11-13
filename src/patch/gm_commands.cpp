#include "gm_commands.h"

#include <cstdio>

#include "config_new.h"
#include "flags.h"
#include "tangar_utils.h"

#include "../game_app.h"
#include "../inventory.h"
#include "../item.h"
#include "../map_stuff.h"
#include "../net.h"
#include "../player.h"
#include "../server.h"
#include "../spell.h"
#include "../table.h"
#include "../token.h"
#include "../unit.h"
#include "../unit_list.h"

namespace tangar_gm {

namespace {

struct SpellBookDef {
    const char* name;
    uint8_t book_sphere;
};

// Matches SpellNames[32] and SpellBooks[6] in my-srvmgr/cheat_codes_new.cpp:92-137.
const SpellBookDef kSpellDefs[32] = {
    {"", 0},
    {"Fire_Arrow", 1},
    {"Fire_Ball", 1},
    {"Wall_of_Fire", 1},
    {"Protection_from_Fire", 1},
    {"Ice_Missile", 2},
    {"Poison_Cloud", 2},
    {"Blizzard", 2},
    {"Protection_from_Water", 2},
    {"Acid_Stream", 2},
    {"Lightning", 3},
    {"Prismatic_Spray", 3},
    {"Invisibility", 3},
    {"Protection_from_Air", 3},
    {"Darkness", 3},
    {"Light", 3},
    {"Diamond_Dust", 4},
    {"Wall_of_Earth", 4},
    {"Stone_Curse", 4},
    {"Protection_from_Earth", 4},
    {"Bless", 5},
    {"Haste", 5},
    {"Control_Spirit", 5},
    {"Teleport", 5},
    {"Heal", 5},
    {"Summon", 5},
    {"Drain_Life", 5},
    {"Shield", 5},
    {"Curse", 5},
    {"Slow", 5},
    {"", 0},
    {"", 0},
};

const char* const kSpellBooks[6] = {
    "None",
    "Fire",
    "Water",
    "Air",
    "Earth",
    "Astral",
};

} // namespace

Item* ConstructItem(const std::string& definition) {
    if (definition.empty()) {
        return nullptr;
    }
    CString cdef(definition.c_str());
    return g_GameDataRes.FindItemTemplate(&cdef);
}

void GiveItemTo(Item* item, Player* player) {
    if (item == nullptr) {
        return;
    }
    if (player == nullptr || player->main_unit == nullptr || player->main_unit->inventory == nullptr) {
        delete item;
        return;
    }
    player->main_unit->inventory->PutItemIntoBagAtDefault(item);
    player->main_unit->AddCarriedWeight(0);
}

void ExecuteCreateCommand(Player* player, const std::string& raw_args) {
    if (player == nullptr || player->main_unit == nullptr) {
        return;
    }

    std::string command = Trim(raw_args);
    if (command.empty()) {
        return;
    }

    uint32_t count = 1;
    size_t fsp = command.find_first_of(' ');
    if (fsp != std::string::npos) {
        std::string intstr = command.substr(0, fsp);
        if (CheckInt(intstr)) {
            command = TrimLeft(command.substr(fsp + 1));
            count = StrToUInt(intstr);
            if (count == 0) {
                count = 1;
            }
        }
    }

    if (command.empty()) {
        return;
    }

    if (ToLower(command) == "gold") {
        player->AddMoney(static_cast<int32_t>(count), 1);
    } else if (command.find('{') != std::string::npos) {
        for (uint32_t i = 0; i < count; i++) {
            Item* t_item = ConstructItem(command);
            if (t_item == nullptr) {
                return;
            }

            uint8_t item_slot = 0;
            if (t_item->IsKindOf(RUNTIME_CLASS(Weapon))) {
                item_slot = 1;
            } else if (t_item->IsKindOf(RUNTIME_CLASS(Shield))) {
                item_slot = 2;
            } else if (t_item->IsKindOf(RUNTIME_CLASS(Armor))) {
                item_slot = static_cast<Armor*>(t_item)->slot;
            }

            // Special case for scrolls/potions (stackable 1-weight slot-0 items).
            if (item_slot == 0 && t_item->shape_id == 0 && t_item->material_id == 0 && t_item->weight == 1) {
                t_item->count = static_cast<uint16_t>(count);
                GiveItemTo(t_item, player);
                break;
            }

            t_item->count = 1;
            GiveItemTo(t_item, player);
        }
    } else {
        Item* t_item = ConstructItem(command);
        if (t_item == nullptr) {
            return;
        }
        t_item->count = static_cast<uint16_t>(count);
        GiveItemTo(t_item, player);
    }

    g_NetStru1_main.SendUnitUpdate(player->main_unit, player, 0xFFFFFFFFu, 0xFFB, 0, 0);
}

uint32_t GetSpells(Unit* unit) {
    if (unit == nullptr || unit->spell_book == nullptr) {
        return 0;
    }
    return static_cast<uint32_t>(unit->spell_book->CalcSpellMask());
}

void SetSpells(Unit* unit, uint32_t spells) {
    if (unit == nullptr) {
        return;
    }
    if (unit->spell_book != nullptr) {
        delete unit->spell_book;
        unit->spell_book = nullptr;
    }
    unit->spell_book = new SpellBook();
    for (int i = 1; i <= 29; i++) {
        if (((1u << i) & spells) != 0) {
            Spell* sp = new Spell(static_cast<uint8_t>(i));
            unit->spell_book->SetSpell(i, sp);
        }
    }
}

void CreateSack(const char* item_definition, uint8_t x, uint8_t y, int money) {
    if (g_Server == nullptr || g_Server->srv_stru1 == nullptr || g_Server->srv_stru1->sack_list == nullptr) {
        return;
    }

    Inventory* inv = new Inventory();
    if (item_definition != nullptr && item_definition[0] != '\0') {
        Item* item = ConstructItem(item_definition);
        if (item != nullptr) {
            inv->PutItemIntoBagAtDefault(item);
        }
    }

    TokenPos pos(x, y, MapStuff_Instance);
    g_Server->srv_stru1->sack_list->DumpInventoryAt(&pos, inv, money, 0);
}

void DropEverything(Unit* unit, bool full) {
    if (Config::GameMode != GAMEMODE_COOPERATIVE) {
        return;
    }
    if (unit == nullptr || unit->position == nullptr) {
        return;
    }

    uint8_t p_x = unit->position->x;
    uint8_t p_y = unit->position->y;

    uint32_t spellmask = GetSpells(unit);
    for (int i = 0; i < 32; i++) {
        if (((1u << i) & spellmask) != 0 && kSpellDefs[i].name[0] != '\0' && kSpellDefs[i].book_sphere != 0) {
            char def_buf[128];
            std::snprintf(def_buf, sizeof(def_buf), "Book %s {teachSpell=%s}",
                          kSpellBooks[kSpellDefs[i].book_sphere], kSpellDefs[i].name);
            CreateSack(def_buf, p_x, p_y, 0);
        }
    }

    SetSpells(unit, 0);
    g_NetStru1_main.SendUnitUpdate(unit, nullptr, 0xA31FFFFFu, 0xFFB, 0, 0);

    if (!full) {
        return;
    }

    Player* owner = unit->pOwner;
    if (owner == nullptr) {
        return;
    }

    uint32_t money = static_cast<uint32_t>(owner->money);
    if (money == 0) {
        return;
    }
    if (money > 0x7FFFFFFFu) {
        money = 0x7FFFFFFFu;
    }

    CreateSack(nullptr, p_x, p_y, static_cast<int>(money));
    owner->money = 0;
    owner->AddMoney(0, 0);
}

void KillPlayerUnits(Player* target, Player* caster) {
    if (target == nullptr || g_ServerUnits == nullptr) {
        return;
    }

    CList<Unit*>& list = g_ServerUnits->unit_list;
    for (POSITION pos = list.GetHeadPosition(); pos != nullptr;) {
        Unit* unit = list.GetNext(pos);
        if (unit == nullptr || unit->pOwner != target) {
            continue;
        }
        if (caster != nullptr && unit == caster->main_unit) {
            continue;
        }
        unit->last_hit_by = nullptr;
        unit->hp = -50;
        g_NetStru1_main.SendUnitUpdate(unit, nullptr, 0xFFFFFFFFu, 0xFFB, 0, 0);
    }
}

void KillAllUnits(Player* caster, bool ai_only) {
    if (g_ServerUnits == nullptr) {
        return;
    }

    CList<Unit*>& list = g_ServerUnits->unit_list;
    for (POSITION pos = list.GetHeadPosition(); pos != nullptr;) {
        Unit* unit = list.GetNext(pos);
        if (unit == nullptr) {
            continue;
        }
        if (caster != nullptr && unit == caster->main_unit) {
            continue;
        }
        if (ai_only && unit->pOwner != nullptr && unit->pOwner->is_ai == 0) {
            continue;
        }
        unit->last_hit_by = nullptr;
        unit->hp = -50;
        g_NetStru1_main.SendUnitUpdate(unit, nullptr, 0xFFFFFFFFu, 0xFFB, 0, 0);
    }
}

void OwnUnits(Player* from, Player* to) {
    if (from == nullptr || to == nullptr || g_ServerUnits == nullptr) {
        return;
    }

    CList<Unit*>& list = g_ServerUnits->unit_list;
    for (POSITION pos = list.GetHeadPosition(); pos != nullptr;) {
        Unit* unit = list.GetNext(pos);
        if (unit == nullptr) {
            break;
        }
        if (unit->pOwner != from || unit == from->main_unit) {
            continue;
        }
        g_NetStru1_main.TransferUnitOwnership(unit, to);
    }
}

bool ParseSpaceDelimited(const std::string& str, std::vector<std::string>& out) {
    out.clear();
    char quote = 0;
    std::string part;
    for (size_t i = 0; i < str.size(); ++i) {
        char ch = str[i];
        if (quote != 0) {
            if (ch == quote) {
                quote = 0;
            } else {
                if (ch == '\\') {
                    if (++i >= str.size()) {
                        return false;
                    }
                    ch = str[i];
                }
                part += ch;
            }
        } else {
            if (ch == '"' || ch == '\'' || ch == '`') {
                quote = ch;
            } else if (ch != ' ') {
                if (ch == '\\') {
                    if (++i >= str.size()) {
                        return false;
                    }
                    ch = str[i];
                }
                part += ch;
            } else if (!part.empty()) {
                out.push_back(part);
                part.clear();
            }
        }
    }
    if (!part.empty()) {
        out.push_back(part);
    }
    return true;
}

} // namespace tangar_gm
