#ifndef A2_VIS_TAVERN_H
#define A2_VIS_TAVERN_H

#pragma once
#include "vis_core.h"

class VisTavSceneAnim {
public:
	void LoadAnimFrames(CStringArray* names); //4010ee (client: 4010ee) in asm
	void FreeAnimFrames(); //4014f2 (client: 4014f1) in asm
	bool StepForward(); //4015c6 (client: 4015c5) in asm
	bool StepBackward(); //401613 (client: 401612) in asm
	void NextFrame(); //401659 (client: 401658) in asm
	void Draw(int32_t x, int32_t y); //4016ef (client: 4016ee) in asm

public:
	CArray<CBmp64*> frames;
	CBmp64* current_frame; // 0x14
	int32_t frame_idx; // 0x18
	CStringArray names;
};
ASSERT_SIZE(VisTavSceneAnim, 0x30);

//60c5d8
class VisTavRightPanel : public CVisualObject {
public:
	virtual ~VisTavRightPanel(); // 4A3E40 (client: 4a7ec0)

	virtual const char* GetHint() override; // 4a4710 (client: 4a7eb0) in asm
	virtual void VMethod7() override; // 499dfa (client: 49de1a) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49a26e (client: 49e28e) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 49a291 (client: 49e2b1) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 49a30a (client: 49e32a) in asm
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 49a2e6 (client: 49e306) in asm

	VisTavRightPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //4997c6 (client: 42cc93) in asm

	void LoadRightPanelBmps(); //499a67 (client: 49da87) in asm
	void ResetButtonStates(); //49a84e (client: 49e87d) in asm
	void UpdateButtonLabels(); //49a973 (client: 49e9a2) in asm
	void FreeRightPanelBmps(); //499cdf in asm - client FreeBitmaps restructured away (both client 283b FreeBitmaps fns belong to VisCharGenAction 42d704 / VisCharSellectButtons 432ae6)
	void InitRightPanel(); //4998c2 (client: 42cd99) in asm
	int32_t HitTestButton(CPoint pos); //49a873 (client: 43281c) in asm
	void UpdateHoveredButton(uint32_t wparam, CPoint pos); //49a8fa in asm - client FreeBitmaps restructured away (42d41b is VisCharGenAction::UpdateMouseOver)

public:
	VisTav* vis_tav; // 0x5c
	CStringArray texts; // 0x60
	CBmp64* bmp_btn_on[3]; // 0x74
	CBmp64* bmp_btn_off[3]; // 0x80
	CBmp64* bmp_area; // 0x8c
	CRect btn_rects[3]; // 0x90
	int32_t pressed_btn; // 0xc0
	int32_t hovered_btn; // 0xc4
};
ASSERT_SIZE(VisTavRightPanel, 0xc8);

// 60c560
class VisTavLeftPanel : public CVisualObject {
public:
	virtual ~VisTavLeftPanel(); // 4A3E10 (client: 4a7e40)

	virtual const char* GetHint() override; // 49934d (client: 49d36d) in asm
	virtual void VMethod7() override; // 497ed0 (client: 49bef0) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 497ec1 (client: 49bee1) in asm

	VisTavLeftPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //497990 (client: 49b9b0) in asm

	void LoadLeftPanelBmps(); //4995d1 (client: 49d5f1) in asm
	void FreeLeftPanelBmps(); //4996ab (client: 49d6cb) in asm
	void InitLeftPanel(); //497ace (client: 49baee) in asm
	void DrawUnitPanel(CUnit* unit); //497f82 (client: 49bfa2) in asm
	void SelectRosterEntry(int32_t idx); //497e06 (client: 49be26) in asm

public:
	VisTav* vis_tav; // 0x5c
	char picture_name[0x100]; // 0x60
	CBmp64* portrait_bmp; // 0x160
	CBmp256* hitmap; // 0x164
	CBmp64* bmp_left_stats; // 0x168
	CBmp64* bmp_left_picture; // 0x16c
	CRect left_btn_rect; // 0x170
	CRect right_btn_rect; // 0x180
};
ASSERT_SIZE(VisTavLeftPanel, 0x190);

//60c650
class VisTavScene : public CVisualObject {
public:
	virtual ~VisTavScene(); // 4A3E70 (client: 4a7ef0)

	virtual const char* GetHint() override; // 49cd2b (client: 4a0d59) in asm
	virtual void VMethod7() override; // 49b22f (client: 49f25e) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49d8d9 (client: 4a1907) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 49da98 (client: 4a1ac6) in asm
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 49d8e8 (client: 4a1916) in asm

	virtual void VMethod26(); //49ced4 (client: 4a0f02) in asm
	virtual void VMethod27(); //49d1b0 (client: 4a11de) in asm
	virtual void VMethod28(); //49d2bb (client: 4a12e9) in asm
	virtual void VMethod29(); //49d5f9 (client: 4a1627) in asm

	VisTavScene(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //49ad56 (client: 49ed85) in asm

	void DrawQuestRoster(); //49bc23 (client: 49fc52) in asm
	int32_t HitTestRoster(CPoint* pos); //49cab8 (client: 4a0ae6) in asm
	void InitSceneRects(); //49af8c (client: 49efbb) in asm

public:
	VisTav* vis_tav; // 0x5c
	CRect roster_rects[3][6]; // 0x60
	CRect party_rects[3][3]; // 0x180
	CArray<CSprite256*> roster_sprites; // 0x210
	CArray<CSprite256*> avail_sprites; // 0x224
	CArray<CSprite256*> reserved_sprites; // 0x238
	CArray<uint32_t> avail_frames; // 0x24c
	CArray<uint32_t> reserved_frames; // 0x260
	CBmp64* bmp_man_back; // 0x274
	CBmp64* bmp_man_back_talk; // 0x278
	CBmp64* bmp_background; // 0x27c
	VisTavSceneAnim anims[4]; // 0x280
	CBmp64* bmp_lu_over; // 0x340
	CBmp64* bmp_ld_over; // 0x344
	CBmp64* bmp_ru_over; // 0x348
	CBmp64* quest_bmps[13]; // 0x34c
	uint8_t __gap_0x380[76];
};
ASSERT_SIZE(VisTavScene, 0x3cc);

//60c7f8
class VisTavSceneDruid : public VisTavScene {
public:
	virtual ~VisTavSceneDruid(); // 4A3F90 (client: 4a80b0)

	virtual void VMethod7() override; // 4a0673 (client: 4a46a1) in asm
	virtual void VMethod28() override; // 4a11e3 (client: 4a5211) in asm
	virtual void VMethod29() override; // 4a158f (client: 4a55bd) in asm

	VisTavSceneDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //4a05a0 (client: 4a45ce) in asm

public:
	CBmp64* bmp_flash;
	VisTavSceneAnim anims_druid[3]; // 0x3d0
	uint32_t ambient_last_tick;
	uint32_t ambient_interval;
};
ASSERT_SIZE(VisTavSceneDruid, 0x468);

//60c910
class VisTavSceneKaarg : public VisTavScene {
public:
	virtual ~VisTavSceneKaarg(); // 4A4090 (client: 4a81b0)

	virtual void VMethod7() override; // 4a2bb8 (client: 4a6be6) in asm
	virtual void VMethod28() override; // 4a37e7 (client: 4a7815) in asm
	virtual void VMethod29() override; // 4a3b25 (client: 4a7b53) in asm

	VisTavSceneKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //4a2aa4 (client: 4a6ad2) in asm

public:
	VisTavSceneAnim anims_kaarg[5]; // 0x3cc
	uint32_t script_frame_index;
	uint32_t ambient1_last_tick;
	uint32_t ambient1_interval;
	uint32_t ambient2_last_tick;
	uint32_t ambient2_interval;
};
ASSERT_SIZE(VisTavSceneKaarg, 0x4d0);

//60c6d8
class VisTav : public VisScreen {
public:
	virtual ~VisTav(); // 4A3F10 (client: 4a7fd0)

	virtual void VMethod7() override; // 49E044 (client: 4a2072)
	virtual void VMethod8(CRect* rect) override; // 49e075 (client: 4a20a3)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 49E082 (client: 4a20b0)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49E261 (client: 4a228f)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 49E233 (client: 4a2261)

	virtual void VMethod26() override; // 49DDAF (client: 4a3549)
	virtual void VMethod28() override; // 49E34F (client: 4a237d)
	virtual void DoClose(uint32_t code) override; // 49EB6C (client: 4a2b9a)

	virtual void VMethod30(); // 49F298 (client: 4a32c6)
	virtual void VMethod31(); // 49F3D7 (client: 4a3405)

	void CloseDialog(); // client: 4a2e1a (client: 4a2e1a)
	void SelectNextPartyUnit(); //49ee36 (client: 4a2e64) in asm
	void SelectPrevPartyUnit(); //49eecd (client: 4a2efb) in asm
	int32_t FindEntryIndexForUnit(CUnit* unit); //49e2e3 (client: 4a2311) in asm
	void ShowHireResponseDialog(); //49ef63 (client: 4a2f91) in asm
	void RetalkAvailNpc(); //49f0da (client: 4a3108) in asm
	void ShowReservedNpcTalk(); //49f179 (client: 4a31a7) in asm

	VisTav(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //49db79 (client: 4a1ba7) in asm

public:
	BigStruct2* map_context;
	VisCharInfo* info_panel;
	VisTavLeftPanel* left_panel;
	VisTavRightPanel* right_panel;
	int32_t gap_0x78; //unk type
	VisTavScene* scene;
	CVisualObject* tips;
	CSound sounds[13];
	int32_t selection_index;
	int32_t select_party;
	CArray <CUnit*> avail_entries;
	CArray <CUnit*> selected_entries;
	CArray <CUnit*> reserved_entries;
	CDWordArray entrie_id;
	int32_t dialog_active;
	QuestMap* quest_map;
	int32_t quest_id;
	int32_t quest_selection;
	int32_t quest_mirror_selection;
	CArray<TokenEntry*> rewards;
	int32_t inn_id;
	int32_t quest_alt_close;
};
ASSERT_SIZE(VisTav, 0x140);

//60c768
class VisTavDruid : public VisTav {
public:
	virtual ~VisTavDruid(); // 4A3F40 (client: 4a8040)

	virtual void VMethod26() override; // 49F51B (client: 4a8040)
	virtual void VMethod28() override; // 49FAAA (client: 4a3ad8)
	virtual void DoClose(uint32_t code) override; // 4A0278 (client: 4a42a6)
	virtual void VMethod30() override; // 49F7B0 (client: 4a37de)
	virtual void VMethod31() override; // 49F961 (client: 4a398f)

	VisTavDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 49f4e6 (client: 4544a0) in asm
public:
	SfxSample* snd_druid[11];
};
ASSERT_SIZE(VisTavDruid, 0x16c);

//60c880
class VisTavKaarg : public VisTav {
public:
	virtual ~VisTavKaarg(); // 4A4040 (client: 4a8160)

	virtual void VMethod26() override; // 4A190D (client: 4a1ddd)
	virtual void VMethod28() override; // 4A1BA2 (client: 4a5bd0)
	virtual void DoClose(uint32_t code) override; // 4A2448 (client: 4a6476)
	virtual void VMethod30() override; // 4A26D9 (client: 4a6707)
	virtual void VMethod31() override; // 4A2873 (client: 4a68a1)

	VisTavKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 4a18d8 (client: 4544a0) in asm
public:
	SfxSample* snd_kaarg[10];
};
ASSERT_SIZE(VisTavKaarg, 0x168);

#endif
