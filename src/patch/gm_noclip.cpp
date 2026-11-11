// Ported from my-srvmgr zxmgr::MakeUnitNoClip + CheckPlayerNoClip
// (common_new.cpp). The DLL zeroed dwords in the two obstacle grids through
// the raw MapStuff pointer; here the same cells are cleared through the
// named obstacle_map / obstacle_map2 fields. eye->byte5 is path_mask.

#include "gm_noclip.h"

#include "flags.h"

#include "../eye.h"
#include "../game_app.h"
#include "../map_stuff.h"
#include "../net.h"
#include "../player.h"
#include "../unit.h"

static void MakeUnitNoClip(Unit* unit) {
    unit->eye->path_mask = 0;
    int x = unit->position->x;
    int y = unit->position->y;
    // clear unit radius
    MapStuff* map = MapStuff_Instance;
    unit->position->pInstance = map;
    int unit_size = unit->GetTokenSize();
    if (map != nullptr) {
        for (int i = x; i < x + unit_size; i++) {
            if (i >= 0 && i <= 255) {
                for (int j = y; j < y + unit_size; j++) {
                    if (j >= 0 && j <= 255) {
                        map->obstacle_map[(uint32_t)(j << 8) + i] = 0;
                        map->obstacle_map2[(uint32_t)(j << 8) + i] = 0;
                    }
                }
            }
        }
    }
}

void TangarCheckPlayerNoClip(Player* player) {
    Unit* unit = player->main_unit;
    if ((player->flags & GMF_NOCLIP) == GMF_NOCLIP) {
        MakeUnitNoClip(unit);
    }

    g_NetStru1_main.SendUnitUpdate(unit, player, 0xFFFFFFFF, EQUIP_SLOTS_ALL, 0, 0);
}
