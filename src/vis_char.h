#ifndef A2_VIS_CHAR_H
#define A2_VIS_CHAR_H

#pragma once
#include "vis_core.h"

//609898
class VisCharSelect : public VisScreen {
public:
	virtual ~VisCharSelect(); // 438650 (client: 43ac00)

	virtual void VMethod8(CRect* rect) override; // 438d50 (client: 43abf0)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4326A4 (client: 4347d5)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4326EA (client: 43481b)

	virtual void VMethod26() override; // 431FDF (client: 43410d)
	virtual void VMethod28() override; // 43233E (client: 43446f)
	virtual void DoClose(uint32_t code) override; // 43251C (client: 43464d)


	void OpenRenameWindow(); //4327f3 (client: 434924)

	void SelectCharacter(CUnit* unit); // client: 434786 (client: 434786)

	void LoadSfx(); // 432280 (client: 430eb0)
	void FreeSfx(); // 4322ed (client: 430f1d)
	void CloseRenameWindow(); // 432933 (client: 434a64)
	CUnit* GetSelectedMapUnit(); // 438d30 (client: 430ac8)

	VisCharSelect(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //431eac (client: 433fda) in asm

public:
	CGameSession* session;
	CVisualObject* info_panel;
	BigStruct2* map_context;
	VisCharSellectStats* vis_stats;
	VisCharSellectButtons* buttons;
	VisCharSellectList* roster_list;
	CVisualObject* rename_txt;
	CUnit* selected_unit;
	SfxSample* snd_ok;
	SfxSample* snd_rename;
	SfxSample* snd_delete;
	SfxSample* snd_cancel;
	uint32_t active_flag;
};
ASSERT_SIZE(VisCharSelect, 0x9c);

//6097a8
class VisCharSellectButtons : public CVisualObject {
public:
	virtual ~VisCharSellectButtons(); // 4385B0 (client: 43ab30)

	virtual void VMethod7() override; // 43005F (client: 431f52)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4302F5 (client: 4321e8)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 430318 (client: 43220b)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;  // client: 4322bb

	VisCharSellectButtons(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen); // 42fd5c (client: 431c50)

	void LoadBitmaps(); // 430850 (client: 43291c)
	void FreeBitmaps(); // 430a1a (client: 432ae6)
	void Init(); // 42fe62 (client: 431d56)
	void ResetMouseBoxes(); // 43072b (client: 4327f7)

public:
	VisCharSelect* parent_screen;
	CStringArray captions;
	CBmp64* buttons_bmp[4];
	void* gap_0x84[4];
	CBmp64* bmp_area;
	CRect areas[4];
	int32_t mouse_down_box;
	int32_t mouse_over_box;

public:
	int GetMouseOnBox(CPoint pos);
	void UpdateMouseOverBox(uint32_t wparam, CPoint pos);
};
ASSERT_SIZE(VisCharSellectButtons, 0xe0);

//609820
class VisCharSellectList : public CVisualObject {
public:
	virtual ~VisCharSellectList(); // 4385E0 (client: 43ab80)

	virtual void VMethod7() override; // 430FEC (client: 433143)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 431609 (client: 433760)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 431764 (client: 4338bb)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 43182C (client: 433983)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4316A0 (client: 4337f7)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 431B95 (client: 433cec)

	VisCharSellectList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen); // 430bbb (client: 432c87)

	void ResetListState(); // 430fad (client: 433104)
	void Init(); // 430cb3 (client: 432d7f)
	void SelectRow(int32_t row); // 4316c4 (client: 43381b)
	int32_t SelectPrevRow(); // 431a05 (client: 433b5c)
	int32_t SelectNextRow(); // 431a93 (client: 433bea)
	void UpdateArrows(CPoint pos, bool is_down); // 431c97 (client: 433dee)
	int32_t HitTest(CPoint pos); // 431872 (client: 4339c9)
	void LoadBitmaps(); // 4312b7 (client: 43291c)
	void FreeBitmaps(); // 43148a (client: 4335e1)

public:
	VisCharSelect* parent_screen;
	CRect row_rect;
	CRect list_area;
	CRect up_arrow_rect;
	CRect down_arrow_rect;
	int32_t top_row;
	CBmp64* bmp_center_area;
	CBmp64* bmp_up_shine;
	CBmp64* bmp_up_shine_on;
	CBmp64* bmp_down_shine;
	CBmp64* bmp_down_shine_on;
	CBmp64* bmp_arrow_up;
	CBmp64* bmp_arrow_down;
	uint32_t gap_0xc0;
	uint32_t gap_0xc4;
	uint32_t gap_0xc8;
	uint32_t gap_0xcc;
	int32_t selected_row;
	int32_t hovered_row;
	int32_t renaming;
};
ASSERT_SIZE(VisCharSellectList, 0xdc);

//609730
class VisCharSellectStats : public CVisualObject {
public:
	virtual ~VisCharSellectStats(); // 438580 (client: 43aac0)

	virtual const char* GetHint() override; // 42FC61 (client: 431b55)
	virtual void VMethod7() override; // 42F7B9 (client: 4316ad)


	void ReadUnitStats(); // 42f6f3 (client: 4315ef)

	VisCharSellectStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen); // 42f0ec (client: 430fe8)

	void Init(); // 42f1e8 (client: 4310e4)
	void LoadBitmaps(); // 42f4df (client: 4313db)
	void FreeBitmaps(); // 42f61a (client: 431516)

public:
	VisCharSelect* parent_screen;
	CBmp64* bmp_full_stats;
	CBmp64* bmp_left_up;
	CStringArray stat_labels;
	uint32_t shadow_body;
	uint32_t shadow_reaction;
	uint32_t shadow_mind;
	uint32_t shadow_spirit;
	CA16* cube_sprites;
	CRect stat_icon_rects[4];
};
ASSERT_SIZE(VisCharSellectStats, 0xd0);

class VisCharGen;
class VisTipsDialog;

//6094c8
class VisCharGenStats : public CVisualObject {
public:
	virtual ~VisCharGenStats(); // 428f12 (client: 42ae32) in asm

	virtual const char* GetHint() override; // 42a43b (client: 42c351) in asm
	virtual void VMethod7() override; // 429d03 (client: 42bc1b) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42a312 (client: 42d1f6) in asm
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 42a3f2 (client: 42c308) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 42a335 (client: 42c24b) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42a3a8 (client: 42c2be) in asm
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 42a3ce (client: 42c2e4) in asm

	VisCharGenStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent); //428e51 (client: 42ad71) in asm

	void ReadUnitStats(); // 429c39 (client: 42bb59) in asm
	void LoadBitmaps(); // 429475 (client: 42b395) in asm
	void FreeBitmaps(); // 4298b6 (client: 42b7d6) in asm
	uint32_t HandleClick(int32_t mode, CPoint pt); // 42a161 (client: 42c077) in asm
private:
	int32_t StatUpCost(int32_t stat); // 42a96d (client: 42c883) in asm
	int32_t StatDownRefund(int32_t stat); // 42a99d (client: 42c8b3) in asm
	int32_t HitTest(CPoint pt); // 42a02a (client: 42bf42) in asm
	int32_t OnStatUp(int32_t idx); // 42a778 (client: 42c68e) in asm
	int32_t OnStatDown(int32_t idx); // 42a87b (client: 42c791) in asm
	void Init(); // 428f7f (client: 42ae9f) in asm
public:
	VisCharGen* parent_screen; // 0x5c
	CBmp64* bmp; // 0x60
	CRect label_rects[4];
	CRect points_rect;
	CRect areas[4][3]; // 0xb4: per-stat rects [value, plus_btn, minus_btn]
	CBmp64* stat_btn_pairs[4][2]; // 0x174: per-stat [plus, minus] button bitmap pair
	CBmp64* stat_button_bmps[10]; // 0x194: plon/ploff/pnlon/pnloff/pdisable/mlon/mloff/mnlon/mnloff/mdisable
	CStringArray texts; // 0x1bc
	int32_t stat_body; // 0x1d0
	int32_t stat_reaction; // 0x1d4
	int32_t stat_mind; // 0x1d8
	int32_t stat_spirit; // 0x1dc
	int32_t gap_0x1e0[4];
	int32_t stat_points;
};
ASSERT_SIZE(VisCharGenStats, 0x1f4);

//609540
class VisCharGenFullStats : public CVisualObject {
public:
	virtual ~VisCharGenFullStats(); // 42aaa0 (client: 42c9b6) in asm

	virtual const char* GetHint() override; // 42ac85 (client: 42cb9b) in asm
	virtual void VMethod7() override; // 42abea (client: 42cb00) in asm

	VisCharGenFullStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent); //42aa2b (client: 42c941) in asm

	void LoadBitmaps(); // 42ab18 (client: 42ca2e) in asm
	void FreeBitmaps(); // 42ab9c (client: 42cab2) in asm
public:
	VisCharGen* parent_screen; // 0x5c
	CBmp64* bmp; // 0x60
	uint8_t gap_0x64[0x44];
};
ASSERT_SIZE(VisCharGenFullStats, 0xa8);

//6095b8
class VisCharGenAction : public CVisualObject {
public:
	virtual ~VisCharGenAction(); // 42ae19 (client: 42ae32) in asm

	virtual void VMethod7() override; // 42b028 (client: 42cf3e) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42b2e0 (client: 42d1f6) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 42b303 (client: 42d219) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42b392 (client: 42d2a8) in asm

	VisCharGenAction(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent); //42ad7d (client: 42cc93) in asm

	void LoadBitmaps(); // 42b57e (client: 42d494) in asm
	void FreeBitmaps(); // 42b7ee (client: 42d704) in asm
	void ResetMouseBoxes(); // 42b455 (client: 42d36b) in asm
private:
	int32_t HitTest(CPoint pt); // 42b47a (client: 42d390) in asm
	void UpdateMouseOver(uint32_t wparam, CPoint pos); // 42b505 (client: 42d41b) in asm
	void Init(); // 42ae83 (client: 42cd99) in asm
public:
	VisCharGen* parent_screen; // 0x5c
	CStringArray texts; // 0x60
	CBmp64* btn_on[3]; // 0x74
	CBmp64* btn_off[3]; // 0x80
	CBmp64* area_bmp; // 0x8c
	CRect areas[3]; // 0x90
	int32_t mouse_down_box; // 0xc0
	int32_t mouse_over_box; // 0xc4
};
ASSERT_SIZE(VisCharGenAction, 0xc8);

//609630
class VisCharGenSkills : public CVisualObject {
public:
	virtual ~VisCharGenSkills(); // 42ba30 (client: 422210) in asm

	virtual const char* GetHint() override; // 42dbce (client: 42fae2) in asm
	virtual void VMethod7() override; // 42bb5a (client: 42da70) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42bfcc (client: 42dee1) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 42c072 (client: 42df86) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42c1ab (client: 43e1da) in asm (Ghidra-labeled accHitTest)

	VisCharGenSkills(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent); //42b98c (client: 42d8a2) in asm

	void LoadBitmaps(uint32_t mage_flag); // 42c1cc (client: 42e0e0) in asm
	void LoadSamples(uint32_t mage_flag); // 42d963 (client: 42f877) in asm
	void FreeBitmaps(); // 42d6ed (client: 42f601) in asm
	void FreeSamples(); // 42da5a (client: 42f96e) in asm
private:
	int32_t HitTest(CPoint pt); // 42dadf (client: 42f9f3) in asm
	void DrawBlinkSlot(); // 42bd41 (client: 42dc56) in asm
	void Init(); // 42ba81 (client: 42d997) in asm
public:
	VisCharGen* parent_screen; // 0x5c
	CBmp64* bmp_column; // 0x60: "chrgen\<class>\column.bmp", background drawn in VMethod7 (owned)
	CBmp256* bmp_mask; // 0x64: "chrgen\<class>\mask.bmp", HitTest color-key source (owned)
	CBmp64* bmp_roll_stats; // 0x68: "chrgen\\RollStatsR.bmp" (owned, never drawn)
	CBmp64* bmp_full_stats; // 0x6c: "chrgen\\FullStatsR.bmp" (owned, never drawn)
	CBmp64* bmp_ru_over; // 0x70: "inn\\RUOver.bmp" (owned, never drawn)
	CBmp64* bmp_human_back; // 0x74: alias to g_bmp_humanbackl (not owned; vanilla never frees it)
	CBmp64* bmp_on[5]; // 0x78
	CBmp64* bmp_shine_off[5]; // 0x8c
	CBmp64* bmp_shine_on[5]; // 0xa0
	CPoint slot_positions[5]; // 0xb4: slot positions (parent-screen coords)
	CPoint slot_sizes[5]; // 0xdc: slot sizes
	uint8_t color_keys[8]; // 0x104: per-slot mask color keys
	int32_t slot_states[5]; // 0x10c: per-slot state (bit0 selected, bit1 hover)
	SfxSample* skill_sounds[5]; // 0x120: skill sounds
	int32_t selected_slot; // 0x134
};
ASSERT_SIZE(VisCharGenSkills, 0x138);

//6096a8
class VisCharGen : public VisScreen {
public:
	virtual ~VisCharGen(); // 4383F0 (client: 43a9f0)

	virtual void VMethod7() override; // 42EB29 (client: 430a27)
	virtual void VMethod8(CRect* rect) override; // 42eb7f (client: 430a7d)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 42E058 (client: 42ff6c)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42E196 (client: 4300aa)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42EA01 (client: 4308ff)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 42EA4F (client: 43094d)

	virtual void VMethod26() override; // 42DDEC (client: 42fd00)
	virtual void VMethod28() override; // 42E218 (client: 43012c)
	virtual void DoClose(uint32_t code) override; // 42E7BC (client: 4306d0)

	VisCharGen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //42dcd9 (client: 42fbed) in asm

	void OnPrevFace(); // 42edc2 (client: 430cbf) in asm
	void OnNextFace(); // 42ebe0 (client: 430ade) in asm
	void RollStats(); // 42e92a (client: 43083e) in asm
	void OnClickFwd(); // 42eb8c (client: 430a8a) in asm
	void OnClickBack(); // 42ebab (client: 430a8a) in asm
	void ShowTipHint(); // 42ea87 (client: 430985) in asm
	void LoadSamples(); // 42efb4 (client: 430eb0) in asm
	void FreeSamples(); // 42f021 (client: 430f1d) in asm
public:
	VisCharInfo* info_panel;
	BigStruct2* map_visual;
	VisCharGenStats* stats_panel;
	VisCharGenFullStats* fullstats_panel;
	VisCharGenAction* action_panel;
	VisCharGenSkills* skills_panel;
	VisTipsDialog* tips;
	SfxSample* snd_stat;
	SfxSample* snd_ok;
	SfxSample* snd_reset;
	SfxSample* snd_back;
	CUnit* current_char;
	int32_t fwd_btn;
	int32_t gap_0x9c;
	CArray<uint16_t> face_sets[4];
	uint32_t mage_flag;
	int32_t female_face;
	int32_t selected_face;
	int32_t selected_face_set;
	int32_t tips_step;
	uint32_t active_flag;
};
ASSERT_SIZE(VisCharGen, 0x108);

//60cfe8
class VisCharInfo : public CVisualObject {
public:
	virtual ~VisCharInfo(); // 4B47B0 (client: 4b9180)

	virtual const char* GetHint() override; // 4B19B0 (client: 4b62e2)
	virtual void VMethod7() override; // 4B2B3D (client: 4b746f)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4B201C (client: 4b694e)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B289E (client: 4b71d0)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4b2215 (client: 4b6b47)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B2346 (client: 4b6c78)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4B2239 (client: 4b6b6b)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4b2227 (client: 4b6b59)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4B2857 (client: 4b7189)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b2334 (client: 4b6b6b)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4B2AD5 (client: 4b7407)

	virtual int32_t VMethod26(int32_t a); //4b38af (client: 4b81df)
	virtual int32_t VMethod27(int32_t a); //4b3a0d (client: 4b8374)
	virtual int32_t VMethod28(); // 4b4830 (client: 4b9200)

	void UpdateCursor(); //4b36b4 (client: 4b7fe4)

	VisCharInfo(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b16c3 (client: 4b5ff5) in asm
public:
	BigStruct2* map_context;
	int32_t dirty;
	int32_t gap_64;
	int32_t selection_panel_state;
	int32_t spell_panel_state;
	int32_t info_mode;
	CBmp64* bitmap;
	CBmp256* hitmap;
	char picturename[256];
};
ASSERT_SIZE(VisCharInfo, 0x17c);

#endif
