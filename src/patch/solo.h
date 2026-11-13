#pragma once

// Solo / Ironman / Pure / Giga character loot & drop restrictions, ported from
// my-srvmgr/solo.{h,cpp} (remember_a2server @ 504A99, solo_pickup_sack @ 5A9977,
// solo_pick_all_sacks @ 5A99DD, drop_item_under @ 505E9C, drop_item_to_another @ 505EBE,
// choose_drop_item @ 50597C, drop_gold @ 5060A4, plus ClearStapleCells on map load).

#include <cstdint>

class Player;
class Sack;
class Server;
struct TokenPos;
class Unit;

void RememberA2Server(Server* server);
void ClearStapleCells();

void PoisonStapleCell(const TokenPos* pos);
void StapleCellOnMobKill(Unit* killed_unit);
Sack* FindSack(uint16_t pos_yx);

void SoloPickup(Unit* unit, int y, int x);
void SoloPickupAll(Unit* unit);
bool ChooseDropItem(uint8_t from_type, uint8_t to_type, Unit* unit);
bool HandleDropGoldSolo(Player* player);

bool IsSoloPlayer(const Unit* unit);
bool IsGigaPlayer(const Unit* unit);
bool IsIronPlayer(const Unit* unit);
bool IsPurePlayer(const Unit* unit);
