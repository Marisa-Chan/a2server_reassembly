// Ported from my-srvmgr/circle.cpp.

#include "circle.h"

#include "config_new.h"

#include "../player.h"

namespace circle {

int Circle(const Unit* unit) {
    if (unit->pOwner == nullptr) {
        return 0;
    }
    const char* name = (const char*)unit->pOwner->name;

    char maybe_circle = name[0];
    if (name[0] == '_' || name[0] == '@') {
        maybe_circle = name[1];
    }

    if ('1' <= maybe_circle && maybe_circle <= '9') {
        return static_cast<int>(maybe_circle - '0');
    }

    return 0;
}

double Multiplier(int circle_number, int server_id) {
    switch (server_id) {
        case EASY:      return 0.05 * circle_number;
        case KIDS:      return 0.10 * circle_number;
        case NIVAL:     return 0.15 * circle_number;
        case MEDIUM:    return 0.20 * circle_number;
        case HARD:      return 0.25 * circle_number;
        case NIGHTMARE: return 0.30 * circle_number;
        case QUEST_T1:  return 0.25 * circle_number;
        case QUEST_T2:  return 0.20 * circle_number;
        case QUEST_T3:  return 0.15 * circle_number;
        case QUEST_T4:  return 0.10 * circle_number;
        default:        return 0.0;
    }
}

int Bonus(int circle_number, int server_id) {
    switch (server_id) {
        case EASY:      return (circle_number + 1) / 2;
        case KIDS:      return circle_number;
        case NIVAL:     return circle_number;
        case MEDIUM:    return circle_number;
        case HARD:      return 2 * circle_number;
        case NIGHTMARE: return 3 * circle_number;
        case QUEST_T1:  return 4 * circle_number;
        case QUEST_T2:  return 5 * circle_number;
        case QUEST_T3:  return 6 * circle_number;
        case QUEST_T4:  return 7 * circle_number;
        default:        return 0;
    }
}

int IncreaseDamage(Unit* attacker, Unit* target, int damage) {
    // Not a player's unit --- no changes.
    if (target == nullptr || target->pOwner == nullptr || target->pOwner->is_ai != 0) {
        return damage;
    }

    // On-map effects are not modified.
    if (attacker == nullptr || attacker->pOwner == nullptr) {
        return damage;
    }

    // PvP is untouched.
    if (attacker->pOwner->is_ai == 0) {
        return damage;
    }

    int circle_number = Circle(target);
    if (circle_number == 0) {
        return damage;
    }

    double multiplier = 1 + Multiplier(circle_number, Config::ServerID);
    int bonus = Bonus(circle_number, Config::ServerID);
    return static_cast<int>(damage * multiplier + bonus);
}

double ExperienceMultiplier(const Unit* unit) {
    return 1.0 + 0.5 * Circle(unit);
}

} // namespace circle
