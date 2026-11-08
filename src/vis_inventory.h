#ifndef A2_VIS_INVENTORY_H
#define A2_VIS_INVENTORY_H

#pragma once
#include "vis_core.h"

//60c9b0
class VisInvBase : public CVisualObject {
public:
	VisInvBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4a4938 (client: 4a8968)

	virtual ~VisInvBase(); // 4A77D0 (client: 4ab830)
	virtual const char* GetHint() override; // 4a4cf8 (client: 4a8d28)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4A5DE0 (client: 4a9e0f)
	virtual int32_t VMethod26(TokenEntry* o, int32_t idx); //4a554f (client: 4a957e)
	virtual int32_t VMethod27(TokenEntry* o); //4a56c3 (client: 4a96f2)
	virtual TokenEntry* VMethod28(uint32_t id); //4a57a1 (client: 4a97d0)
	virtual TokenEntry* VMethod29(TokenEntry* o, int32_t num); //4a58dc (client: 4a990b)
	virtual int32_t VMethod30(int32_t x, int32_t y); //4a79a0 (client: 4ab800)
	virtual int32_t VMethod31(const CPoint* pt); //4a7990 (client: 4ab810)
	virtual void VMethod32(CArray<TokenEntry*>* arr); //4a4da9 (client: 4a8dd9)
	virtual void VMethod33(CGameObject* uni); //4a4d05 (client: 4a8d35)
	virtual void VMethod34(); //4a4e28 (client: 4a8e58)
	virtual void VMethod35(); //4a4c2f (client: 4a8c5f)
	virtual TokenEntry* VMethod36(int32_t idx, int32_t num); //4a5aae (client: 4a9add)
	virtual int32_t VMethod37(int32_t idx); //4a5c39 (client: 4a9c68)
	virtual int32_t VMethod38(); //4a79b0 (client: 4ab820)

	int32_t GetVisibleStart(); //46fb90 (client: 472fd0)
	int32_t HasAnimatingCells(); //41f940 (client: 4207b0)
	void RefreshCellSprites(); //4a4ebc (client: 4a8eec)
	void InitGlowOffsets(); //4a4bcb (client: 4a8bfb)
	void ScrollCellsBack(); //4a5350 (client: 4a937f) in asm
	void ScrollCellsForward(); //4a51c9 (client: 4a91f8) in asm
	void TickCellAnims(); //4a5e12 (client: 4a9e41) in asm
	void DrawGlowTrail(int32_t x, int32_t y, int32_t idx); //4a5fab (client: 4a9fda) in asm

public:
	CArray<CSprite256*> spr_cells;
	CDWordArray cell_update_counter;
	CArray<TokenEntry*>* grid_source;
	int32_t visible_columns;
	int32_t visible_rows;
	int32_t* visible_startref;
	int32_t random_offsets1[1024];
	int32_t random_offsets2[1024];
	CDWordArray anim_frames;
	int32_t has_anim_visible_cells;
};
ASSERT_SIZE(VisInvBase, 0x20ac);

//60ca60
class VisInvType1 : public VisInvBase {
public:
	virtual ~VisInvType1(); // 4A7830 (client: 4aba40)

	virtual const char* GetHint() override; // 4A765D (client: 4ab68b)
	virtual void VMethod7() override; // 4A6449 (client: 4aa478)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4A7441 (client: 4ab46f)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4A716F (client: 4ab19d)
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 4A741D (client: 4ab44b)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4A6E57 (client: 4aae86)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4A70EC (client: 4ab11a)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4A6F44 (client: 4aaf73)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4a6f32 (client: 4aaf61)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4a715d (client: 4ab18b)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4a70da (client: 4ab108)

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override; // 4A79F0 (client: 4ab9d0)
	virtual int32_t VMethod27(TokenEntry* o) override; // 4A79C0 (client: 4ab9a0)
	virtual int32_t VMethod30(int32_t x, int32_t y) override; // 4A6D11 (client: 4aad40)
	virtual int32_t VMethod38() override; // 4a7a20 (client: 4aba00)

	VisInvType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4a630b (client: 4aa33a) in asm

public:
};
ASSERT_SIZE(VisInvType1, 0x20ac);

//60d0e8
class VisInvExtBase : public VisInvBase {
public:
	VisInvExtBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4b4a0c (client: 4b93dc) in asm
	virtual ~VisInvExtBase(); // 4B4AEC (client: 4b94bc)

	virtual const char* GetHint() override; // 4B4C64 (client: 4b9634)
	virtual void VMethod7() override; // 4B5072 (client: 4b9a42)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B5BBE (client: 4ba58d)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B5D19 (client: 4ba6e8)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B5D5C (client: 4ba72b)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4B5F3F (client: 4ba90d)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4b62a2 (client: 4bac6c)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4b62b4 (client: 4bac7e)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b62c6 (client: 4bac90)

	virtual int32_t VMethod30(int32_t x, int32_t y) override; // 4B4FA3 (client: 4b9973)
	virtual int32_t VMethod31(const CPoint* pt) override; // 4B4E88 (client: 4b9858)
	virtual void VMethod32(CArray<TokenEntry*>* arr) override;  // client: 4b9761
	virtual int32_t VMethod38() override;  // client: 4be810

	virtual void VMethod39();  // client: 4be800
	virtual void VMethod40(CArray<TokenEntry*>* arr);  // client: 4b977a
	virtual void VMethod41();  // client: 4be820
	virtual void VMethod42();  // client: 4be830

	int32_t EndItemDrag(); //4b5b81 (client: 4ba550) in asm
	int32_t StartItemDrag(int32_t idx); //4b5a6c (client: 4ba43b) in asm
	void FreeSpeechSfx(); //4b4ba9 (client: 461193) in asm
	void FreeGridSfx(); //4b4c1c (client: 4b95ec)
	void LoadGridSfx(); //4b4bc5 (client: 4b9595)
	void RefreshGrid(); //4b4d33 (client: 4b9703)
	void ClampVisibleStart(); //4b4fd1 (client: 4b99a1)

public:
	VisShop* shop;
	CSound snd_speech;
	CSound snd_put_on;
	CSound snd_put_off;
	CSound snd_scroll;
	int32_t gap_0x20c0; //unk type
	CRect* cell_rects;
};
ASSERT_SIZE(VisInvExtBase, 0x20c8);

//60d1a8
class VisInvExtType1 : public VisInvExtBase {
public:
	virtual ~VisInvExtType1(); // 4B9CB0 (client: 4be680)

	virtual const char* GetHint() override; // 4B6F8D (client: 4bb957)
	virtual void VMethod7() override; // 4B70C0 (client: 4bba8a)

	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B7690 (client: 4bc05a)
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 4B7433 (client: 4bbdfd)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B7562 (client: 4bbf2c)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4b7412 (client: 4bbddc)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4B777C (client: 4bc146)

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override; // 4B790D (client: 4bc2d7)
	virtual TokenEntry* VMethod29(TokenEntry* o, int32_t num) override; // 4B7A9F (client: 4bc469)

	virtual int32_t VMethod37(int32_t idx) override;  // client: 4bc6e6
	virtual int32_t VMethod38() override;  // client: 4be840
	virtual void VMethod39() override;  // client: 4bb710

	virtual void VMethod41() override;  // client: 4bb006
	virtual void VMethod42() override;  // client: 4bb591

	void ClearCategoryGrids(); //4b7859 (client: 4bc223)
	void SelectCategoryGrid(int32_t category); //4b73e4 (client: 4bbdae)
	void DrawUpArrowHover(); //4b7264 (client: 4bbc2e) in asm
	void DrawUpArrowIdle(); //4b72c1 (client: 4bbc8b) in asm
	void DrawDownArrowHover(); //4b731e (client: 4bbce8) in asm
	void DrawDownArrowIdle(); //4b7381 (client: 4bbd4b) in asm

	VisInvExtType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4b63f7 (client: 4badc1) in asm

public:
	int32_t gap_0x20c8; //unk type
	CRect up_arrow_rect;
	CRect down_arrow_rect;
	CBmp64* arrow1;
	CBmp64* arrow3;
	CBmp64* arrow2;
	CBmp64* arrow4;
	CBmp64* shop_inv;
	CArray<TokenEntry*> category_grids[4];
};
ASSERT_SIZE(VisInvExtType1, 0x2150);

//60d910
class VisInvExtType1Kaarg : public VisInvExtType1 {
public:
	virtual void VMethod41() override; //4b69ee (client: 4bb3b8) in asm

	VisInvExtType1Kaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4c6cd0 (client: 4cb670) in asm
};
ASSERT_SIZE(VisInvExtType1Kaarg, 0x2150);

//60d850
class VisInvExtType1Druid : public VisInvExtType1 {
public:
	virtual void VMethod41() override; //4b6815 (client: 4bb1df) in asm

	VisInvExtType1Druid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4c6c40 (client: 4cb5e0) in asm
};
ASSERT_SIZE(VisInvExtType1Druid, 0x2150);

//60d268
class VisInvExtType2 : public VisInvExtBase {
public:
	virtual ~VisInvExtType2(); // 4B9CE0 (client: 4be6b0)

	virtual const char* GetHint() override; // 4B8339 (client: 4bcd03)
	virtual void VMethod7() override; // 4B846C (client: 4bce36)

	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B88EC (client: 4bd2b6)
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 4B877A (client: 4bd144)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B87CE (client: 4bd198)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4b87ad (client: 4bd177)

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override; // 4B9ED0 (client: 4be8a0)
	virtual int32_t VMethod27(TokenEntry* o) override; // 4B9EA0 (client: 4be870)

	virtual int32_t VMethod38() override;  // client: 4be8d0
	virtual void VMethod39() override;  // client: 4bcacc

	void DrawUpArrowHover(); //4b860e (client: 4bcfd8) in asm
	void DrawUpArrowIdle(); //4b8666 (client: 4bd030) in asm
	void DrawDownArrowHover(); //4b86be (client: 4bd088) in asm
	void DrawDownArrowIdle(); //4b871c (client: 4bd0e6) in asm

	VisInvExtType2(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4b7fe5 (client: 4bc9af) in asm
public:
	int32_t gap_0x20c8; //unk type
	CRect up_arrow_rect;
	CRect down_arrow_rect;
};
ASSERT_SIZE(VisInvExtType2, 0x20ec);

//60d328
class VisInvExtType3 : public VisInvExtBase {
public:
	virtual ~VisInvExtType3(); // 4B9D10 (client: 4be6e0)

	virtual const char* GetHint() override; // 4B90CF (client: 4bda99)
	virtual void VMethod7() override; // 4B9155 (client: 4bdb1f)

	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B92F9 (client: 4bdcc3)

	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4b93ae (client: 4bdd78)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B93CF (client: 4bdd99)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b9436 (client: 4bde00)

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override; // 4B9457 (client: 4bde21)
	virtual int32_t VMethod27(TokenEntry* o) override; // 4B949A (client: 4bde64)

	virtual TokenEntry* VMethod36(int32_t idx, int32_t num) override;  // client: 4be606
	virtual int32_t VMethod37(int32_t idx) override;  // client: 4be15a
	virtual int32_t VMethod38() override;  // client: 4be8e0
	virtual void VMethod39() override;  // client: 4bd864

	virtual void VMethod41() override;  // client: 4bd70d
	virtual void VMethod42() override;  // client: 4bd80d

	virtual TokenEntry* VMethod43(int32_t id1, int32_t id2);  // client: 4bdf8f

	int32_t IsBuyGridFull(TokenEntry* entry); //4b91f9 (client: 4bdbc3) in asm
	VisInvExtType3(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4b8bba (client: 4bd584) in asm

	void ClearBuyEntries(); //4b970e (client: 4be0d8)
	int32_t DropDraggedItem(int32_t idx); //4b9b2f (client: 4be4f9) in asm
public:
	int32_t gap_0x20c8; //unk type
	CRect up_arrow_rect;
	CRect down_arrow_rect;
	CBmp64* shoptable;
	int32_t gap_0x20f0; //unk type
	CArray<TokenEntry*> entries;
};
ASSERT_SIZE(VisInvExtType3, 0x2108);

// VTable at 60da00
class VisSpellBook : public CVisualObject {
public:
	virtual ~VisSpellBook(); // 4CB020 (client: 4cf9a0)

	virtual const char* GetHint() override; // 4C9A6F (client: 4ce3ff)
	virtual void VMethod7() override; // 4CA2A1 (client: 4cec31)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4CAAD7 (client: 4cf465)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4CAF8C (client: 4cf912)
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 4CAFF5 (client: 4cf97b)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4CAD6A (client: 4cf6f0)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4CAEA6 (client: 4cf82c)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4CADEE (client: 4cf774)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4CADD2 (client: 4cf758)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4CAF70 (client: 4cf8f6)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4cae94 (client: 4cf81a)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4CAC9B (client: 4cf621)

	void ClearSpellSelection(); // client: 4cf3f7 (client: 4cf3f7)

	int32_t HitTestSpellCell(CPoint* pos); //4ca7c2 (client: 4cf150) in asm
	void SelectScrollEntry(int32_t idx); //4ca925 (client: 4cf2b3) in asm
	int32_t IsSpellAvailable(int32_t id); //4ca89b (client: 4cf229)
	int32_t IsSpellEffectActive(int32_t id); //4ca8e0 (client: 4cf26e)
	int32_t GetActiveSlot(); //41f9b0 (client: 4208e0)
	int32_t HasSelectedSpell(); //41f9e0 (client: 420910)
	int32_t NeedsUnitTarget(); //4caaa7 (client: 4cf435)

	VisSpellBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4c99c7 (client: 4ce357)

public:
	int32_t dirty;
	int32_t pressed;
	int32_t selected;
	int32_t spell;
};
ASSERT_SIZE(VisSpellBook, 0x6c);

#endif
