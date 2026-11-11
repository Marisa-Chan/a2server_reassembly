// Ported from my-srvmgr/cheat_codes_new.cpp (OnDamage, GetDamageBonus) and
// pvm2.cpp (VerifyDamage2). The strong/weak check of VerifyDamage2
// (vd2_CheckStrong) is not ported yet — advanced PvM blocks nothing until it
// lands; the rest of the filter is complete.

#include "damage_filter.h"

#include <cmath>

#include "config_new.h"
#include "flags.h"
#include "player_info.h"

#include "../effect.h"
#include "../item.h"
#include "../player.h"
#include "../unit.h"

namespace {

// Sum of effect-26 ("damage bonus") values across the unit's equipped items.
uint32_t TangarGetDamageBonus(Unit* unit) {
    if (unit == nullptr) {
        return 0;
    }

    // only humans wear equipment with stats
    if (unit->typeId <= 0x20 || unit->typeId >= 0x40) {
        return 0;
    }

    if (unit->inventory == nullptr) {
        return 0;
    }

    Humanoid* human = static_cast<Humanoid*>(unit);
    uint32_t retval = 0;

    for (uint32_t i = 1; i <= 12; i++) {
        Item* item = nullptr;

        if (i == 1) {
            item = human->weapon;
        } else if (i == 2) {
            item = human->shield;
        } else {
            item = human->equipment[i];
        }

        if (item == nullptr) {
            continue;
        }

        for (POSITION pos = item->_effects.GetHeadPosition(); pos != nullptr;) {
            Effect* effect = item->_effects.GetNext(pos);
            if (effect != nullptr && effect->effect_id == 26) {
                retval += effect->spell_value;
            }
        }
    }

    return retval;
}

} // namespace

int32_t TangarOnDamage(Unit* attacker, Unit* victim, int16_t damage) {
    if (damage < 0) {
        return 0;
    }

    if (victim != nullptr && (victim->unit_attrs & 8) != 0) {
        return 0;
    }

    // calculate damage bonus
    uint32_t damage_bonus = TangarGetDamageBonus(attacker);
    if (damage_bonus != 0 && damage > 0) {
        damage = (int16_t)((double)damage * ((double)damage_bonus / 100.0 + 1.0));
    }

    int32_t retval = damage;

    Player* attacker_player = attacker != nullptr ? attacker->pOwner : nullptr;
    Player* victim_player = victim != nullptr ? victim->pOwner : nullptr;

    // Damage modificators in PvP
    if (attacker_player != nullptr && victim_player != nullptr &&
        attacker_player->is_ai == 0 && victim_player->is_ai == 0 &&
        attacker_player != victim_player) { // Ensure it's not self-inflicted damage
        if ((attacker->unit_attrs & 4) != 0) { // if mage or witch
            // This method is being called twice, so it needs to reduce the
            // factor like shown below. If the method would be called once, it
            // would be unnecessary: factor*x = k*(k*x) => k = sqrt(factor)
            float factor = (float)std::sqrt(Config::mage_pvp_dmg_factor);
            damage = (uint16_t)(damage * factor);
            retval = damage;
        }

        // limit maximum damage dealt to player
        int16_t pvp_dmg_lim = (int16_t)Config::max_pvp_dmg;
        if (damage > pvp_dmg_lim) {
            retval = pvp_dmg_lim;
        }
    }

    uint32_t rights1 = 0;
    uint32_t rights2 = 0;

    // PvM: players cannot damage players
    if ((Config::ServerFlags & SVF_PVM) != 0) {
        if (attacker_player != nullptr && attacker_player->is_ai == 0 &&
            victim_player != nullptr && victim_player->is_ai == 0) {
            retval = 0;
        }
    }

    // Advanced PvM: strong players cannot attack weak players and vice versa
    // (VerifyDamage2's strong/weak check is not ported yet; block nothing)
    if ((Config::ServerFlags & SVF_ADVPVM) != 0) {
        // no-op until vd2_CheckStrong lands
    }

    if (attacker_player != nullptr && (attacker_player->flags & GMF_ANY) == GMF_ANY) {
        retval = damage;
        rights1 = attacker_player->flags & 0xFFFFFF;

        if (attacker_player->is_ai != 0) {
            rights1 = 0;
        }
    }

    if (victim_player != nullptr && (victim_player->flags & GMF_ANY) == GMF_ANY) {
        rights2 = victim_player->flags & 0xFFFFFF;

        if (victim_player->is_ai != 0) {
            rights1 = 0;
        }
    }

    if ((rights1 & GMF_MAXDAMAGE) != 0) {
        retval = 32767;
    }

    // God mode
    if ((rights2 & GMF_GODMODE) != 0 &&
        ((rights1 & GMF_GODMODE_ADMIN) == 0 || (rights2 & GMF_GODMODE_ADMIN) != 0) &&
        victim_player != attacker_player) {
        retval = 0;
    }

    if (((rights2 & GMF_GODMODE) != 0 || (rights2 & GMF_GODMODE_ADMIN) != 0) &&
        attacker_player == victim_player &&
        victim == victim_player->main_unit) {
        retval = 0;
    }

    // temporary god mode (set via #godmode)
    if (victim_player != nullptr) {
        PlayerInfo* pi = PI_Get(victim_player);
        if (pi != nullptr && pi->GodMode &&
            (attacker_player == nullptr ||                  // cast from nowhere (building/trigger)
             (attacker_player->flags & GMF_ANY) != GMF_ANY)) { // or damage from regular player
            retval = 0;
        }
    }

    return retval;
}
