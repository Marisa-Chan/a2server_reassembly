// Anti-invisibility target exploit checks, ported from my-srvmgr/antiinvis.cpp
// (stop_unit_attack @ 505563, stop_mage_cast @ 5055FD, stop_unit_cast @ 50564D).

#include "antiinvis.h"

#include "../game_app.h"
#include "../player.h"
#include "../token.h"
#include "../unit.h"
#include "../world.h"
#include "tangar_log.h"

bool block_invis = false;

static bool CheckInvisibleTarget(Player* attacker, Token* target, const char* fmt)
{
    if (attacker == nullptr || target == nullptr || !target->IsUnit()) {
        return false;
    }

    Unit* target_unit = static_cast<Unit*>(target);
    if ((target_unit->enchantments & 0x1000u) == 0) {
        return false;
    }

    if (g_World == nullptr || target_unit->pOwner == nullptr) {
        return false;
    }

    uint16_t target_pid = target_unit->pOwner->player_id;
    uint16_t attacker_pid = attacker->player_id;
    if (target_pid >= 70 || attacker_pid >= 70) {
        return false;
    }

    if ((g_World->diplomacy.diplomacy[target_pid][attacker_pid] & DIPLO_VISION_SHARE) != 0) {
        return false;
    }

    log_format(fmt,
               static_cast<const char*>(attacker->login),
               static_cast<const char*>(attacker->name),
               static_cast<const char*>(target_unit->pOwner->name));

    return block_invis;
}

// 505563
bool StopUnitAttack(Player* attacker, Token* target)
{
    return CheckInvisibleTarget(attacker, target, "[invis_fix] %s/%s attacks %s!\n");
}

// 5055FD
bool StopMageCast(Player* attacker, Token* target)
{
    return CheckInvisibleTarget(attacker, target, "[invis_fix] %s/%s attacks %s!\n");
}

// 50564D
bool StopUnitCast(Player* attacker, Token* target)
{
    return CheckInvisibleTarget(attacker, target, "[invis_fix] %s/%s casts spell onto %s!\n");
}
