#pragma once

#include <cstdint>

class Unit;
struct Inventory;

void DropPartially(Unit* unit, Inventory* unused, int32_t money, int32_t main_unit);
