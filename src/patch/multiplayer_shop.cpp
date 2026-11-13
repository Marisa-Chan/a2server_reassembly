// Per-player custom .alm shops, ported from my-srvmgr/multiplayer_shop.cpp
// (ClearShops, imp_LoadShopData @ 59C66E, imp_EnterCustomShop @ 502C50).

#include "multiplayer_shop.h"

#include "../alm.h"
#include "../buildings_list.h"
#include "../game_app.h"
#include "../player.h"
#include "../players_list.h"
#include "../server.h"
#include "../shop.h"
#include "../token.h"
#include "../unit.h"

#include <cstdint>
#include <map>

static std::map<uint32_t, Shop*> s_ShopReplacements;

static Shop* CreateShopFromALMStructure(MapBuildingData* struc, MapAlm* mapdata)
{
    TokenPos pos(static_cast<uint8_t>(struc->x), static_cast<uint8_t>(struc->y), MapStuff_Instance);
    Shop* shop = new Shop(static_cast<uint8_t>(struc->type_id), &pos);
    shop->TokenID = static_cast<int32_t>(struc->building_id);
    shop->pOwner = (g_PlayersList != nullptr) ? g_PlayersList->FindById(struc->player_id) : nullptr;

    for (int32_t i = 0; i < mapdata->shops.GetSize(); i++) {
        MapShopData* sd = mapdata->shops[i];
        if (sd != nullptr && sd->shop_id == struc->building_id) {
            for (int32_t j = 0; j < 4; j++) {
                shop->gen_params[j].min_cost = static_cast<int32_t>(sd->min_price[j]);
                shop->gen_params[j].max_cost = static_cast<int32_t>(sd->max_price[j]);
                shop->gen_params[j].max_same_count = static_cast<int32_t>(sd->max_same_count[j]);
                shop->gen_params[j].max_count = static_cast<int32_t>(sd->max_count[j]);
                shop->gen_params[j].flags = static_cast<int32_t>(sd->flags[j]);
            }
            break;
        }
    }

    return shop;
}

Shop* CreateMultiShops(MapBuildingData* struc, MapAlm* mapdata)
{
    Shop* main_shop = CreateShopFromALMStructure(struc, mapdata);
    uint32_t shop_id = static_cast<uint32_t>(main_shop->TokenID);

    for (uint32_t i = 0; i < 16; i++) {
        uint32_t additional_id = (shop_id & 0x00FFFFFFu) | ((16u + i) << 24);
        Shop* add_shop = CreateShopFromALMStructure(struc, mapdata);
        add_shop->TokenID = static_cast<int32_t>(additional_id);

        if (g_Server != nullptr && g_Server->srv_stru1 != nullptr && g_Server->srv_stru1->building_list != nullptr) {
            g_Server->srv_stru1->building_list->AddBuilding(add_shop);
        }

        s_ShopReplacements[additional_id] = add_shop;
    }

    return main_shop;
}

void ClearShops()
{
    s_ShopReplacements.clear();
}

Shop* CheckReplacementBuilding(Shop* shop, TokenPos* coords)
{
    if (shop == nullptr || coords == nullptr || g_PlayersList == nullptr) {
        return nullptr;
    }

    for (POSITION pos = g_PlayersList->list.GetHeadPosition(); pos != nullptr;) {
        Player* plr = g_PlayersList->list.GetNext(pos);
        if (plr == nullptr || plr->main_unit == nullptr) {
            continue;
        }
        if (plr->main_unit->position == coords) {
            uint32_t replacement_id = (static_cast<uint32_t>(plr->player_id) << 24) |
                                      (static_cast<uint32_t>(shop->TokenID) & 0x00FFFFFFu);
            auto it = s_ShopReplacements.find(replacement_id);
            return (it != s_ShopReplacements.end()) ? it->second : nullptr;
        }
    }

    return nullptr;
}
