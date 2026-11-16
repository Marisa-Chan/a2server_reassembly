#pragma once

struct MapAlm;
struct MapBuildingData;
class Shop;
struct TokenPos;

void ClearShops();
Shop* CreateMultiShops(MapBuildingData* struc, MapAlm* mapdata);
Shop* CheckReplacementBuilding(Shop* shop, TokenPos* coords);
