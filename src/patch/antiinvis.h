#pragma once

class Player;
class Token;

extern bool block_invis;

bool StopUnitAttack(Player* attacker, Token* target);
bool StopMageCast(Player* attacker, Token* target);
bool StopUnitCast(Player* attacker, Token* target);
