#ifndef A2_VIS_WORLDMAP_H
#define A2_VIS_WORLDMAP_H

#pragma once
#include "vis_core.h"

class GlobalMapRouteArray : public CWordArray {
public:
public:
	int32_t routeCost = 0;
};
ASSERT_SIZE(GlobalMapRouteArray, 0x18);

// Pathmap/graph builder state allocated by VisGlobalMap::VMethod28.
class GMapThing {
public:
	GMapThing(); // 473195 (client: 476ee5)
	~GMapThing(); // 4733cf (client: 47711f)
	void* TakeAdjacency(void* nodePoints); // 473b4a (client: 47789a)

	void LoadPathmap(); // 473508 (client: 477258)
	void FreePathmap(); // 4735e2 (client: 42cab2)
	void BuildAdjacency(); // 47362d (client: 47737d)
	int32_t FloodFill(int32_t x, int32_t y, uint16_t mark, CArray<CPoint>* foundPoints); // 47391f (client: 47766f)

public:
	CBmp256* pathmap; //0x0
	int32_t nodeCount; //0x4
	CArray<CPoint>*** adjacencyMatrix; //0x8
	CArray<CPoint> nodePoints; //0xc
	uint16_t visitedGrid[640][480]; //0x20
	uint8_t* bmp_data; // 0x96020
	CPoint floodPos; // 0x96024
};
ASSERT_SIZE(GMapThing, 0x9602c);

//60b760
class VisGlobalMap : public VisScreen {
public:
	virtual ~VisGlobalMap(); // 473D70 (client: 477c20)

	virtual const char* GetHint() override; // 472729 (client: 476487)
	virtual void VMethod7() override; // 4714E7 (client: 475245)
	virtual void VMethod8(CRect* rect) override; // 471f08 (client: 475c66)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 472CC5 (client: 476a15)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 472D99 (client: 476ae9)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 473038 (client: 476d88)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 474f10 (client: 477c10)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 473077 (client: 476dc7)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 473175 (client: 476ec5)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4726E6 (client: 47642a)
	virtual int32_t OnChar(uint32_t wparam) override; // 4726CC (client: 476444)

	virtual void VMethod26() override; // 47031D (client: 47407b)
	virtual void VMethod28() override; // 47115D (client: 474ebb)
	virtual void DoClose(uint32_t code) override; // 471367 (client: 4750c5)

	void SetTravelOrigin(const CPoint& pt) { this->currentLocationPoint = pt; } //4972a0 (client: 4210f0)

	void RebuildScenarioLocations(); //47024a (client: 473faa)
	void PopulateScenarioLocationFlags(); //47025d (client: 473fbd)
	void ComputeTravelRoute(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY); // 472820 (client: 47657e)
	void OnMapClick(); // 472dca (client: 476b1a)
	void UpdateHoveredLocation(CPoint pos); // 472d4b (client: 476a9b)
	void FreeBitmaps(); // 470cd9 (client: 474a37)
	void FreeSamples(); // 470571 (client: 4742cf)
	void LoadBitmaps(); // 4705c2 (client: 474320)
	void LoadSamples(); // 470504 (client: 474262)
	void FinishTravel(); // 472684 (client: 4763e2)
	void SearchRoute(uint16_t from, uint16_t to, GlobalMapRouteArray* route); // 472ada (client: 476832)

	VisGlobalMap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); // 46fd7b (client: 473adb) in asm
public:
	CBmp64* gmap; //0x68
	CBmp64* hero_bmp; //0x6c
	CBmp64* ballmap; //0x70
	CA16* flag1_spr; //0x74
	CA16* flag_spr; //0x78
	CA16* cross_spr; //0x7c
	CA16* mission_flg; //0x80
	CA16* flg_on_map; //0x84
	CA16* yflag_spr; //0x88
	CBmp64* scroll1_bmp; //0x8c
	CBmp64* scroll2_bmp; //0x90
	CBmp64* scroll3_bmp; //0x94
	CBmp64* scrollp1_bmp; //0x98
	CBmp64* scrollp3_bmp; //0x9c
	CBmp64* scrollp2_bmp; //0xa0
	CWordArray locationAvailabilityFlags; //0xa4
	CArray<CRect> locationHitRects; //0xb8
	CArray<CPoint> locationPoints; //0xcc
	CArray<CPoint> travelRoutePoints; //0xe0
	CSize heroBitmapSize; //0xf4
	CPoint heroDrawPoint; //0xfc
	CPoint currentLocationPoint; //0x104
	CPoint targetLocationPoint; //0x10c
	CRect partDetailsRect; //0x114
	int32_t hoveredLocationIndex; //0x124
	int32_t mapFlagAnimationFrame; //0x128
	int32_t travelProgress; //0x12c
	int32_t partyFlagAnimationFrame; //0x130
	int32_t targetCrossAnimationFrame; //0x134
	uint32_t bestRouteCost; //0x138
	GlobalMapRouteArray routeNodeIndices; //0x13c
	void* routeAdjacencyMatrix; //0x154
	CArray<CPoint> graphNodePoints; //0x158
	CArray<DWORD> locationMetadata; //0x16c
	CSound snd_scrollup; //0x180
	CSound snd_scrolldn; //0x184
	CSound snd_point1; //0x188
	CSound snd_point2; //0x18c
	int32_t routePointSoundIndex; //0x190
	int32_t renderActiveFlag; //0x194
	CString hoveredLocationTitle; //0x198
	CStringArray hoveredLocationLines; //0x19c
	int32_t umoirMapMode; //0x1b0
};
ASSERT_SIZE(VisGlobalMap, 0x1b4);

#endif
