#pragma once

struct MapAlm;
struct MapBuildingData;
class Shop;
class Unit;

void ClearShops();
Shop* CreateMultiShops(MapBuildingData* struc, MapAlm* mapdata);
Shop* CheckReplacementBuilding(Shop* shop, Unit* unit);
