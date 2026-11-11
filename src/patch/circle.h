#pragma once

// Tangar "circle" progression system, ported from my-srvmgr/circle.cpp.
// A player whose name starts with a digit (optionally after '_' or '@')
// belongs to that circle: circle members take extra damage from monsters
// and gain extra experience off their main sphere.

#include "../unit.h"

namespace circle {

// The circle number encoded in the unit owner's name, or 0.
int Circle(const Unit* unit);

// Extra damage multiplier / flat bonus for a circle member, by ServerID.
double Multiplier(int circle_number, int server_id);
int Bonus(int circle_number, int server_id);

// Scales monster damage dealt to circle-member players.
int IncreaseDamage(Unit* attacker, Unit* target, int damage);

// Experience multiplier for a circle member's off-sphere skills.
double ExperienceMultiplier(const Unit* unit);

} // namespace circle
