// Solo / Ironman / Pure / Giga character loot & drop restrictions, ported from
// my-srvmgr/solo.cpp (remember_a2server @ 504A99, solo_pickup_sack @ 5A9977,
// solo_pick_all_sacks @ 5A99DD, drop_item_under @ 505E9C, drop_item_to_another @ 505EBE,
// choose_drop_item @ 50597C, drop_gold @ 5060A4, plus ClearStapleCells on map load).

#include "solo.h"

#include "../game_app.h"
#include "../eye.h"
#include "../inventory.h"
#include "../item.h"
#include "../net.h"
#include "../player.h"
#include "../sack.h"
#include "../server.h"
#include "../token.h"
#include "../unit.h"
#include "config_new.h"
#include "tangar_log.h"

#include <string>
#include <unordered_map>

namespace {

// Solo-characters are allowed to pick up bags only from the stapled cells (from mobs they've killed).
// When a monster leaves a new bag, this cell is stapled with the ID of last-hit player.
// If several different players killed mobs in the same cell without picking up a bag, the staple is poisoned (set to -1).
// If a player adds an item to a staple bag (by dying or moving an item), the staple is also poisoned.
std::unordered_map<uint16_t, int8_t> staple_cells;

// Names of characters that have stapled cells. If the name changes, that's due to relogin, so we remove all staples.
std::unordered_map<int8_t, std::string> staple_char_names;

Server* a2server_instance = nullptr;

bool GigaAllowedToPickup(const Item* item, ServerIDType server_id)
{
    if (item == nullptr) {
        return false;
    }

    if (item->item_id == 3667) { // Treasures can be picked up always.
        return true;
    }

    if (server_id >= QUEST_T1) {
        // At quest maps we allow picking up quest items.
        return item->_exp == 2 && !item->_effects.IsEmpty();
    }

    return false;
}

// Refresh the inventory cached at the client (taken from around 00503663 -> 00519221).
void RefreshPlayerInventory(Unit* unit)
{
    if (unit == nullptr) {
        return;
    }
    g_NetStru1_main.SendUnitUpdate(unit, unit->pOwner, 0xFFFFFFFFu, 0x0FFB, 0, 0);
}

// If the player's name has changed, it means that the player has logged out and another one logged in.
// The player ID is reused, but we should clear all staples for the logged-out player.
void CheckStaplesForReloggedCharacter(Unit* unit)
{
    if (!unit || !unit->pOwner) {
        return;
    }

    const int8_t player_id = static_cast<int8_t>(unit->pOwner->player_id);
    const char* unit_name = static_cast<const char*>(unit->name);
    auto stapled_name_it = staple_char_names.find(player_id);
    if (stapled_name_it == staple_char_names.end()) {
        staple_char_names[player_id] = unit_name;
        return;
    }

    if (stapled_name_it->second != unit_name) {
        staple_char_names.erase(stapled_name_it);
        staple_char_names[player_id] = unit_name;

        for (auto it = staple_cells.begin(); it != staple_cells.end();) {
            if (it->second == player_id) {
                it = staple_cells.erase(it);
            } else {
                ++it;
            }
        }
    }
}

// Giga-players are allowed to pick up only certain items.
// This function is a pre-pickup --- it removes all forbidden items from the bag the unit is standing on.
bool GigaPickup(Unit* unit)
{
    if (!IsGigaPlayer(unit)) {
        return true;
    }

    if (EASY < Config::ServerID) {
        if (!unit->position) {
            return false;
        }

        bool allowed = false;

        Sack* sack = FindSack(unit->position->YX.val);
        if (!sack || !sack->inventory) {
            return false;
        }

        for (POSITION pos = sack->inventory->items.GetHeadPosition(); pos != nullptr;) {
            Item* item = sack->inventory->items.GetNext(pos);
            if (GigaAllowedToPickup(item, Config::ServerID)) {
                allowed = true;
                break;
            }
        }

        if (!allowed) {
            // Don't do anything if we're not allowed to pick the bag up.
            return false;
        }

        // Can pickup? Remove all forbidden items before the pickup.
        Printf("[giga] character %s picks up bag at %d. It had %d items and %d money",
               static_cast<const char*>(unit->name),
               static_cast<int>(unit->position->YX.val),
               static_cast<int>(sack->inventory->items.GetCount()),
               static_cast<int>(sack->money));

        int position = 0;
        POSITION item_pos = sack->inventory->items.GetHeadPosition();
        while (item_pos != nullptr) {
            POSITION next_pos = item_pos;
            Item* item = sack->inventory->items.GetNext(next_pos);

            if (!GigaAllowedToPickup(item, Config::ServerID)) {
                Printf("[giga] deleting item in sack at position %d: id=%d, amount=%d, price=%d, effects total=%d",
                       position,
                       static_cast<int>(item->item_id),
                       static_cast<int>(item->count),
                       static_cast<int>(item->_exp),
                       static_cast<int>(item->_effects.GetCount()));
                Item* removed = sack->inventory->TakeItemAt(position, item->count);
                delete removed;
                item_pos = next_pos;
            } else {
                Printf("[giga] leaving item in sack at position %d: id=%d, amount=%d, price=%d, effects total=%d",
                       position,
                       static_cast<int>(item->item_id),
                       static_cast<int>(item->count),
                       static_cast<int>(item->_exp),
                       static_cast<int>(item->_effects.GetCount()));
                item_pos = next_pos;
                ++position;
            }
        }

        sack->money = 0;
    }

    return true;
}

} // namespace

// Characters with `@` as the first letter of the name are in the solo mode.
bool IsIronPlayer(const Unit* unit)
{
    return unit != nullptr && !unit->name.IsEmpty() && unit->name[0] == '@';
}

bool IsPurePlayer(const Unit* unit)
{
    return unit != nullptr && !unit->name.IsEmpty() && unit->name[0] == '!';
}

// Giga-players --- solo players which get reverted to checkpoint if they die.
bool IsGigaPlayer(const Unit* unit)
{
    return unit != nullptr && !unit->name.IsEmpty() && unit->name[0] == '_';
}

// All solo characters.
bool IsSoloPlayer(const Unit* unit)
{
    return IsIronPlayer(unit) || IsGigaPlayer(unit) || IsPurePlayer(unit);
}

// 504A99
void RememberA2Server(Server* server)
{
    if (a2server_instance != server) {
        Printf("remembering a2server instance: now 0x%x", reinterpret_cast<unsigned int>(server));
        a2server_instance = server;
    }
}

void ClearStapleCells()
{
    staple_cells.clear();
    staple_char_names.clear();
}

// 5A9977
void SoloPickup(Unit* unit, int y, int x)
{
    if (!IsSoloPlayer(unit)) {
        // Original logic for regular chars.
        if (unit != nullptr) {
            unit->state = 2;
        }
        return;
    }

    if (unit == nullptr || unit->position == nullptr) {
        Printf("[solo_pickup]: nullptr unit");
        return;
    }

    uint16_t yx = static_cast<uint16_t>((((y & 0xFF) << 8) | (x & 0xFF)) & 0xFFFF);

    CheckStaplesForReloggedCharacter(unit);

    auto staple_it = staple_cells.find(yx);
    if (staple_it != staple_cells.end() && unit->pOwner != nullptr &&
        static_cast<int>(staple_it->second) == static_cast<int>(static_cast<int16_t>(unit->pOwner->player_id))) {
        // If the player is currently staying on the sack, let them pick it up. Otherwise let them only go to the sack.
        // This is to prevent races with other players poisoning the cell.
        if (unit->position->YX.val == yx) {
            if (!GigaPickup(unit)) {
                return;
            }

            unit->state = 2;
        } else {
            unit->state = 1;
        }
    }
}

// 5A99DD
void SoloPickupAll(Unit* unit)
{
    if (!unit || !unit->position || !unit->pOwner) {
        Printf("[solo_pickup_all] null unit");
    }

    if (!IsSoloPlayer(unit)) {
        // Original logic for regular chars.
        if (unit != nullptr) {
            unit->state = 26;
        }
        return;
    }

    if (!unit->position || !unit->pOwner) {
        return;
    }

    CheckStaplesForReloggedCharacter(unit);

    // If the player stands on top of a stapled cell, let them pick up only that bag.
    auto staple_it = staple_cells.find(unit->position->YX.val);
    if (staple_it != staple_cells.end() &&
        static_cast<int>(staple_it->second) == static_cast<int>(static_cast<int16_t>(unit->pOwner->player_id))) {
        if (!GigaPickup(unit)) {
            return;
        }

        unit->state = 2;
        unit->eye2->command_to = unit->position->YX.val;
    }
}

void PoisonStapleCell(const TokenPos* pos)
{
    if (!pos) {
        return;
    }
    staple_cells[pos->YX.val] = -1;
}

Sack* FindSack(uint16_t pos_yx)
{
    Server* server = a2server_instance;

    if (server == nullptr) {
        Printf("[staple]: nullptr a2server");
        return nullptr; // Likely there are no sacks yet.
    }

    SrvStru1* server_struct = server->srv_stru1;
    if (server_struct == nullptr) {
        Printf("[staple]: nullptr server_struct");
        return nullptr; // Likely there are no sacks yet.
    }

    SackList* sacks = server_struct->sack_list;
    if (sacks == nullptr || sacks->list.IsEmpty()) {
        return nullptr;
    }

    for (POSITION pos = sacks->list.GetHeadPosition(); pos != nullptr;) {
        Sack* sack = sacks->list.GetNext(pos);
        if (sack != nullptr && sack->position != nullptr && sack->position->YX.val == pos_yx) {
            return sack;
        }
    }

    return nullptr;
}

void StapleCellOnMobKill(Unit* killed_unit)
{
    if (!killed_unit || !killed_unit->position) {
        return;
    }

    int8_t staple_with = -1;
    if (killed_unit->last_hit_by && killed_unit->last_hit_by->pOwner &&
        static_cast<int16_t>(killed_unit->last_hit_by->pOwner->player_id) >= 16) {
        staple_with = static_cast<int8_t>(killed_unit->last_hit_by->pOwner->player_id);
    }

    uint16_t position = killed_unit->position->YX.val;

    bool sack_exists_here = FindSack(position) != nullptr;

    // If there's no sack, staple the cell with the killer player. Even if the cell was previously stapled with other players.
    if (sack_exists_here) {
        // There is a sack. Is it stapled?
        auto staple_it = staple_cells.find(position);
        if (staple_it != staple_cells.end()) {
            // Is it stapled by another player? Yes --- poison it.
            if (staple_it->second != staple_with) {
                staple_with = -1;
            }
        }
    }

    CheckStaplesForReloggedCharacter(killed_unit->last_hit_by);

    staple_cells[position] = staple_with;
}

// 50597C
bool ChooseDropItem(uint8_t from_type, uint8_t to_type, Unit* unit)
{
    if (to_type == 3 && (from_type == 1 || from_type == 2)) {
        if (IsGigaPlayer(unit) || IsPurePlayer(unit)) {
            Printf("[giga-drop] %d->%d for player %s is not allowed",
                   static_cast<int>(from_type),
                   static_cast<int>(to_type),
                   static_cast<const char*>(unit->name));

            // Refresh player's inventory so that client knows that nothing was dropped.
            RefreshPlayerInventory(unit);

            return true;
        }
    }

    return false;
}

// 5060A4
bool HandleDropGoldSolo(Player* player)
{
    if (player && player->main_unit) {
        Unit* main_unit = player->main_unit;
        if (IsGigaPlayer(main_unit) || IsPurePlayer(main_unit)) {
            Printf("[giga-drop] dropping gold for player %s is not allowed",
                   static_cast<const char*>(main_unit->name));

            // Refresh player's inventory so that client knows that nothing was dropped.
            RefreshPlayerInventory(main_unit);

            return true;
        } else {
            PoisonStapleCell(main_unit->position);
        }
    }

    return false;
}
