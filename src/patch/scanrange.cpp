// Server-side scan-range subsystem, ported from my-srvmgr scanrange.cpp
// (see scanrange.h). The manager read map data from raw addresses; this
// port uses the real classes (MapStuff_Instance->height_map / map_width /
// map_height, Unit::token_pos / scan_range, g_World->diplomacy).

#include "scanrange.h"

#include <windows.h>

#include <string>

#include "scanrange_calc.h"
#include "player_info.h"
#include "tangar_log.h"
#include "../server.h"
#include "../world.h"
#include "../unit.h"
#include "../player.h"
#include "../token.h"
#include "../map_stuff.h"

namespace tangar_scanrange {

static CScanrangeCalc Scanrange;
static bool Scanrange_Initialized = false;

// Are coordinates on the map valid. Regular case is up to 240, but in some
// cases the calculator feeds it computed coordinates, e.g. (x - 20) + x.
static bool SR_CheckValid(int16_t x, int16_t y) {
	// max size of the map is 240 (255, but because of borders -(8+8)); note
	// the <= (the client-side check in BuildShadowField uses <)
	int16_t map_width = (int16_t)MapStuff_Instance->map_width; // value from ALM
	int16_t map_height = (int16_t)MapStuff_Instance->map_height;

	return (x >= 7 && y >= 7 && // non-valid for values less than 6 (see map editor 'corners')
	        x <= map_width - 7 &&
	        y <= map_height - 7);
}

// Get relief at the coordinates (up to 240, 240). The ALM height map is a
// fixed 256-stride block.
static uint8_t SR_GetHeight(int16_t x, int16_t y) {
	if (!SR_CheckValid(x, y)) {
		return 0;
	}
	return MapStuff_Instance->height_map[y * 256 + x];
}

static void SR_UpdateUnit(Unit* unit) {
	if (!Scanrange_Initialized) {
		Scanrange.InitializeTables();
		Scanrange_Initialized = true;
	}

	if (unit == nullptr) {
		return;
	}
	Player* unit_player = unit->pOwner;
	if (unit_player == nullptr) {
		return;
	}
	// number of milliseconds from the moment the system was started
	uint32_t ticks_count = GetTickCount();
	bool dCalc = false;
	for (int i = 0; i < 32; i++) {
		if (!g_PlayerInfo[i].Exists) {
			continue;
		}
		if (g_PlayerInfo[i].Class == nullptr) {
			continue;
		}
		if (g_PlayerInfo[i].Class->is_ai != 0) {
			continue;
		}
		if (g_PlayerInfo[i].Class->main_unit == nullptr) {
			continue;
		}

		if (g_PlayerInfo[i].Class == unit_player ||
		    (g_World->diplomacy.diplomacy[unit_player->player_id][g_PlayerInfo[i].Class->player_id] & DIPLO_VISION_SHARE) != 0) {
			uint8_t unit_x = unit->token_pos->x;
			uint8_t unit_y = unit->token_pos->y;
			uint16_t unit_vision = unit->scan_range;

			if (!dCalc) {
				// unit_x, unit_y are uint8_t values (up to 255), which we
				// send to 32-bit 'baskets'
				Scanrange.CalculateVision(unit_x, unit_y, unit_vision, &SR_GetHeight, &SR_CheckValid);
				dCalc = true;
			}

			int16_t gen_x = unit_x - 20;
			int16_t gen_y = unit_y - 20;

			for (uint8_t x = 0; x < 41; x++) {
				for (uint8_t y = 0; y < 41; y++) {
					if (SR_CheckValid(gen_x + x, gen_y + y) && (Scanrange.pTablesVision[x][y] > 0)) {
						g_PlayerInfo[i].Vision[gen_x + x][gen_y + y] = ticks_count;
					}
				}
			}
		}
	}
}

void SR_Step() {
	CList<Unit*>& units = g_Server->srv_stru1->units_list->unit_list;

	for (int i = 0; i < 32; i++) {
		if (!g_PlayerInfo[i].Exists) {
			continue;
		}
		if (g_PlayerInfo[i].Class == nullptr) {
			continue;
		}
		memset(g_PlayerInfo[i].Vision, 0, sizeof(g_PlayerInfo[i].Vision));
	}

	for (POSITION it = units.GetHeadPosition(); it != nullptr;) {
		Unit* unit = units.GetNext(it);
		if (unit != nullptr && unit->pOwner != nullptr && unit != unit->pOwner->main_unit) {
			SR_UpdateUnit(unit);
		}
	}

	for (int i = 0; i < 32; i++) {
		if (!g_PlayerInfo[i].Exists) {
			continue;
		}
		if (g_PlayerInfo[i].Class == nullptr) {
			continue;
		}

		if (g_PlayerInfo[i].Class->main_unit != nullptr) {
			SR_UpdateUnit(g_PlayerInfo[i].Class->main_unit);
		}
	}
}

bool SR_CheckVision(Player* player, uint8_t check_x, uint8_t check_y) {
	PlayerInfo* pi = PI_Get(player);
	if (pi == nullptr) {
		return false;
	}
	if (!SR_CheckValid(check_x, check_y)) {
		return false;
	}
	uint32_t ticks_count = GetTickCount();
	for (int32_t x = -1; x <= 1; x++) {
		for (int32_t y = -1; y <= 1; y++) {
			if (ticks_count - pi->Vision[check_x + x][check_y + y] <= 3000) {
				return true;
			}
		}
	}
	return false;
}

// Can not be used from the console.
void SR_DumpToFile(Player* player) {
	PlayerInfo* pi = PI_Get(player);
	if (pi == nullptr) {
		return;
	}
	uint32_t ticks_count = GetTickCount(); // ms from the moment the system was started

	Log() << "SR_DumpToFile: dump initiated (for player " << (const char*)player->name << ").";

	for (uint32_t y = 0; y < 256; y++) {
		std::string row;
		row.reserve(256);
		for (uint32_t x = 0; x < 256; x++) {
			if (ticks_count - pi->Vision[x][y] <= 3000) {
				row += '\x7F'; // ASCII 127 (delete)
			} else {
				row += ' ';
			}
		}
		LogString(row);
	}
	Log() << "SR_DumpToFile: dump finished.";
}

} // namespace tangar_scanrange
