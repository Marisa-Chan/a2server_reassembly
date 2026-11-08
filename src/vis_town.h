#ifndef A2_VIS_TOWN_H
#define A2_VIS_TOWN_H

#pragma once
#include "vis_core.h"

//60dbb0
class VisTown : public VisScreen {
public:
	virtual ~VisTown(); // 4D6B80 (client: 4db490)

	virtual const char* GetHint() override; // 4D1602 (client: 4d5f33)
	virtual void VMethod7() override; // 4CE886 (client: 4d31cd)
	virtual void VMethod8(CRect* rect) override; // 4ce719 (client: 4d3063)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4CE3B8 (client: 4d2d05)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4CE4E5 (client: 4d2e32)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4CE50A (client: 4d2e57)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4CE4AF (client: 4d2dfc)

	virtual void VMethod26() override; // 4CDDAD (client: 4d26fd)
	virtual void VMethod28() override; // 4CE002 (client: 4d2952)
	virtual void DoClose(uint32_t code) override; // 4CE310 (client: 4d2c5d)

	virtual void VMethod30(); // 4D1723 (client: 4d6054)
	virtual void VMethod31(); // 4D184A (client: 4d617b)

	virtual void VMethod32(); // 4D1709 (client: 4d603a)
	virtual int32_t VMethod33(CPoint pos); // 4D10F0 (client: 4d5a27)
	virtual void VMethod34(CPoint pos); // 4D1429 (client: 4d5d5a)
	virtual void VMethod35(); // 4D12CB (client: 4d5c02)
	virtual void VMethod36(); // 4CEEEF (client: 4d3836)
	virtual void VMethod37(); // 4CFD0E (client: 4d4655)
	virtual void VMethod38(); // 4D0E31 (client: 4d5776)
	virtual void VMethod39(); // 4D15E3 (client: 4d5f14)

	int32_t StartBirdFlight(); //4ce64b (client: 4d2f98) in asm
	void DrawBirdFlock(); //4ce726 (client: 4d3070) in asm

	void AdvanceGuardAnim(); //4d06aa (client: 4d4ff1) in asm
	void AdvanceTavernAnim(); //4d07e8 (client: 4d512f) in asm
	void AdvanceSignAnim(); //4d0884 (client: 4d56e7) in asm
	void AdvanceGateAnim(); //4d0913 (client: 4d525a) in asm
	void AdvanceStarsAnim(); //4d0ad7 (client: 4d541c) in asm
	void AdvanceFighterAnim(); //4d0b95 (client: 4d54da) in asm
	void AdvanceMageAnim(); //4d0c6e (client: 4d55b3) in asm
	void AdvanceShopAnim(); //4d0d47 (client: 4d568c) in asm
	void AdvanceFlugelAnim(); //4d0da2 (client: 4d51cb) in asm
	void AdvanceBlackBirdAnim(); //4d0f66 (client: 4d58ab) in asm
	void AdvanceHorseAnim(); //4d1002 (client: 4d5940) in asm
	void AdvanceDervishAnim(); //4d109e (client: 4d59d5) in asm

	VisTown(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4cda5c (client: 4d23ac) in asm
public:
	CBmp64* bmp_bkg;
	CBmp256* bmp_hover_mask;
	CBmp64* bmp_bird_overlay;
	SfxSample* sounds[14];
	int32_t hover_snd_shop;
	int32_t hover_snd_second;
	int32_t hovered_action_mask;
	int32_t last_bird;
	int32_t bird_group_index;
	int32_t active_bird;
	CArray<CA16*> spr_birds;
	int32_t bird_frame0;
	int32_t bird_frame1;
	int32_t bird_frame2;
	CA16* spr_guard;
	int32_t guard_frame;
	int32_t guard_frame_step; // signed: compared against -1
	int32_t guard_sound;
	CA16* spr_cur_bbird;
	CArray<CA16*> spr_bbird;
	CPoint bbird_position;
	int32_t bbird_frame;
	uint32_t bbird_last_tick;
	int32_t bbird_delay;
	CA16* spr_cur_horse;
	CArray<CA16*> spr_horse;
	CPoint horse_position;
	int32_t horse_frame;
	int32_t horse_anim_index;
	uint32_t horse_last_tick;
	int32_t horse_delay;
	CA16* spr_dervish;
	CPoint dervish_position;
	int32_t dervish_frame;
	CA16* spr_tavern;
	CBmp64* bmp_tavern_hover;
	int32_t tavern_frame;
	CArray<CBmp64*> bmp_sign;
	CBmp64* bmp_cur_sign;
	int32_t sign_frame;
	CArray<CBmp64*> bmp_door;
	CBmp64* bmp_cur_door;
	int32_t door_frame;
	uint32_t door_open_flag;
	CArray<CBmp64*> bmp_stars;
	CBmp64* bmp_cur_stars;
	int32_t stars_frame;
	CBmp64* bmp_trainer_hover;
	CA16* spr_fighter;
	int32_t fighter_frame;
	CA16* spr_mage;
	int32_t mage_frame;
	CA16* spr_shop;
	CBmp64* bmp_shop_hover;
	int32_t shop_frame;
	CArray<CBmp64*> bmp_flugel;
	CBmp64* bmp_cur_flugel;
	int32_t flugel_frame;
	CVisualObject* tips;
	uint32_t dialog_active;
	uint32_t town_anim;
};
ASSERT_SIZE(VisTown, 0x20c);

//60dc60
class VisTownDruid : public VisTown {
public:
	virtual ~VisTownDruid(); // 4D6BB0 (client: 4db4c0)

	virtual void VMethod7() override; // 4D2E99 (client: 4d77ca)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4D3F18 (client: 4d883c)
	virtual void VMethod28() override; // 4D1E80 (client: 4d67b1)
	virtual void VMethod30() override; // 4D2AFF (client: 4d7430)
	virtual void VMethod31() override; // 4D2D1B (client: 4d764c)
	virtual void VMethod32() override; // 4D2AE3 (client: 4d7414)
	virtual void VMethod34(CPoint pos) override; // 4D359C (client: 4d7ec8)
	virtual void VMethod35() override; // 4D3922 (client: 4d824e)
	virtual void VMethod36() override; // 4D2153 (client: 4d6a84)
	virtual void VMethod37() override; // 4D27FF (client: 4d7130)
	virtual void VMethod38() override; // 4D3EA7 (client: 4d87cb)

	void AdvanceTavernAnim(); //4d3384 (client: 4d7cb5) in asm
	void AdvanceShopAnim(); //4d3435 (client: 4d7d66) in asm
	void AdvanceBugAnim(); //4d34b6 (client: 4d7de7) in asm
	void AdvanceLizardAnim(); //4d3520 (client: 4d7e4e) in asm

	VisTownDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4d1af8 (client: 4d6429) in asm
public:
	SfxSample* snd_lizard[4];
	SfxSample* snd_bug[3];
	SfxSample* snd_bird[3];
	SfxSample* snd_tree[4];
	SfxSample* snd_forest;
	SfxSample* snd_shop;
	SfxSample* snd_tavern;
	SfxSample* snd_shop_enter;
	SfxSample* snd_tavern_enter;
	SfxSample* snd_town_exit;
	SfxSample* snd_wolf;
	uint32_t tavern_hover_snd_flag;
	uint32_t mission_exit_hover_snd_flag;
	CArray<CBmp64*> tavern_frame_group[3];
	int32_t tavern_group;
	uint32_t tavern_tick;
	int32_t tavern_delay;
	CArray<CBmp64*> shop_frame_group[3];
	int32_t shop_group;
	uint32_t shop_tick;
	int32_t shop_delay;
	uint32_t tree_tick;
	uint32_t wolf_tick;
	CA16* spr_bug;
	uint32_t bug_tick;
	int32_t bug_delay;
	int32_t bug_variant;
	int32_t bug_frame;
	CA16* spr_lizard;
	int32_t lizard_variant;
	int32_t lizard_frame;
};
ASSERT_SIZE(VisTownDruid, 0x320);

//60dd10
class VisTownKaarg : public VisTown {
public:
	virtual ~VisTownKaarg(); // 4D6C60 (client: 4db570)

	virtual const char* GetHint() override; // 4D6A1C (client: 4db32d)
	virtual void VMethod7() override; // 4D57B9 (client: 4da0dd)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4D6802 (client: 4db113)
	virtual void VMethod28() override; // 4D4592 (client: 4d8eb6)
	virtual void VMethod30() override; // 4D54CF (client: 4d9df3)
	virtual void VMethod31() override; // 4D5682 (client: 4d9fa6)
	virtual void VMethod32() override; // 4D54B5 (client: 4d9dd9)
	virtual void VMethod34(CPoint pos) override; // 4D5C9A (client: 4da5be)
	virtual void VMethod35() override; // 4D5EF0 (client: 4da814)
	virtual void VMethod36() override; // 4D4866 (client: 4d918a)
	virtual void VMethod37() override; // 4D5112 (client: 4d9a36)
	virtual void VMethod38() override; // 4D6786 (client: 4db097)

	void AdvanceGateAnim(); //4d6480 (client: 4dad9c) in asm
	void AdvanceGirl1Anim(); //4d65dd (client: 4daef7) in asm
	void AdvanceGirl2Anim(); //4d6655 (client: 4daf6c) in asm
	void AdvanceGuardAnim(); //4d66ca (client: 4dafe1) in asm
	void AdvanceDervishAnim(); //4d6728 (client: 4db03c) in asm

	VisTownKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4d423a (client: 4d8b5e) in asm
public:
	SfxSample* snd_voice[3];
	SfxSample* snd_bird[4];
	SfxSample* snd_voice1;
	SfxSample* snd_shop_enter;
	SfxSample* snd_tavern_enter;
	SfxSample* snd_dervish;
	SfxSample* snd_guard[6];
	uint32_t tavern_snd_flag;
	uint32_t exit_snd_flag;
	uint32_t guard_snd_flag;
	uint32_t bird_snd_tick;
	uint32_t dervish_snd_tick;
	CArray<CBmp64*> bmp_guard;
	uint32_t guard_tick;
	int32_t guard_delay;
	CArray<CBmp64*> bmp_dervish;
	uint32_t dervish_tick;
	int32_t dervish_delay;
	CArray<CBmp64*> bmp_girl1[2];
	int32_t girl1_group;
	CArray<CBmp64*> bmp_girl2[2];
	int32_t girl2_group;
	int32_t girl2_frame;
	uint32_t girl2_tick;
	int32_t girl2_delay;
	CArray<CBmp64*> bmp_gate;
};
ASSERT_SIZE(VisTownKaarg, 0x314);

//60a060
class TownMenuListDialogVisualObject : public VisMenuWnd {
public:
	TownMenuListDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CRect& _r); //44160e (client: 443bdd)
};
ASSERT_SIZE(TownMenuListDialogVisualObject, 0x7c);

#endif
