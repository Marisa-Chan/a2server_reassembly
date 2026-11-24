#ifndef A2_VIS_TOWN_H
#define A2_VIS_TOWN_H

#pragma once
#include "vis_core.h"

//60dbb0
class VisTown : public VisScreen {
public:
    virtual ~VisTown(); // 4cdc78 (client: 4d25c8)

    virtual const char* GetHint() override; // 4d1602 (client: 4d5f33)
    virtual void Draw() override; // 4ce886 (client: 4d31cd)
    virtual void DrawBackground(CRect* rect) override; // 4ce719 (client: 4d3063)
    virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4ce3b8 (client: 4d2d05)
    virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4ce4e5 (client: 4d2e32)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4ce50a (client: 4d2e57)
    virtual int32_t OnKeyDown(uint32_t wparam) override; // 4ce4af (client: 4d2dfc)

    virtual void OnCreate() override; // 4cddad (client: 4d26fd)
    virtual void DoOpen() override; // 4ce002 (client: 4d2952)
    virtual void DoClose(uint32_t code) override; // 4ce310 (client: 4d2c5d)

    virtual void LoadTownSfx(); // 4d1723 (client: 4d6054)
    virtual void FreeTownSfx(); // 4d184a (client: 4d617b)

    virtual void StartAmbientSfx(); // 4d1709 (client: 4d603a)
    virtual int32_t HitTestHotspot(CPoint pos); // 4d10f0 (client: 4d5a27)
    virtual void UpdateHoverState(CPoint pos); // 4d1429 (client: 4d5d5a)
    virtual void UpdateAmbientEvents(); // 4d12cb (client: 4d5c02)
    virtual void LoadTownGfx(); // 4ceeef (client: 4d3836)
    virtual void FreeTownGfx(); // 4cfd0e (client: 4d4655)
    virtual void AdvanceAnims(); // 4d0e31 (client: 4d5776)
    virtual void HideTownScreen(); // 4d15e3 (client: 4d5f14)

    int32_t StartBirdFlight(); // 4ce64b (client: 4d2f98)
    void DrawBirdFlock(); // 4ce726 (client: 4d3070)

    void AdvanceGuardAnim(); // 4d06aa (client: 4d4ff1)
    void AdvanceTavernAnim(); // 4d07e8 (client: 4d512f)
    void AdvanceSignAnim(); // 4d0884 (client: 4d56e7)
    void AdvanceGateAnim(); // 4d0913 (client: 4d525a)
    void AdvanceStarsAnim(); // 4d0ad7 (client: 4d541c)
    void AdvanceFighterAnim(); // 4d0b95 (client: 4d54da)
    void AdvanceMageAnim(); // 4d0c6e (client: 4d55b3)
    void AdvanceShopAnim(); // 4d0d47 (client: 4d568c)
    void AdvanceFlugelAnim(); // 4d0da2 (client: 4d51cb)
    void AdvanceBlackBirdAnim(); // 4d0f66 (client: 4d58ab)
    void AdvanceHorseAnim(); // 4d1002 (client: 4d5940)
    void AdvanceDervishAnim(); // 4d109e (client: 4d59d5)

    VisTown(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 4cda5c (client: 4d23ac)
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
    virtual ~VisTownDruid(); // 4d6be0 (client: 4db4f0)

    virtual void Draw() override; // 4d2e99 (client: 4d77ca)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4d3f18 (client: 4d883c)
    virtual void DoOpen() override; // 4d1e80 (client: 4d67b1)
    virtual void LoadTownSfx() override; // 4d2aff (client: 4d7430)
    virtual void FreeTownSfx() override; // 4d2d1b (client: 4d764c)
    virtual void StartAmbientSfx() override; // 4d2ae3 (client: 4d7414)
    virtual void UpdateHoverState(CPoint pos) override; // 4d359c (client: 4d7ec8)
    virtual void UpdateAmbientEvents() override; // 4d3922 (client: 4d824e)
    virtual void LoadTownGfx() override; // 4d2153 (client: 4d6a84)
    virtual void FreeTownGfx() override; // 4d27ff (client: 4d7130)
    virtual void AdvanceAnims() override; // 4d3ea7 (client: 4d87cb)

    void AdvanceTavernAnim(); // 4d3384 (client: 4d7cb5)
    void AdvanceShopAnim(); // 4d3435 (client: 4d7d66)
    void AdvanceBugAnim(); // 4d34b6 (client: 4d7de7)
    void AdvanceLizardAnim(); // 4d3520 (client: 4d7e4e)

    VisTownDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 4d1af8 (client: 4d6429)
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
    virtual ~VisTownKaarg(); // 4d6c90 (client: 4db5a0)

    virtual const char* GetHint() override; // 4d6a1c (client: 4db32d)
    virtual void Draw() override; // 4d57b9 (client: 4da0dd)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4d6802 (client: 4db113)
    virtual void DoOpen() override; // 4d4592 (client: 4d8eb6)
    virtual void LoadTownSfx() override; // 4d54cf (client: 4d9df3)
    virtual void FreeTownSfx() override; // 4d5682 (client: 4d9fa6)
    virtual void StartAmbientSfx() override; // 4d54b5 (client: 4d9dd9)
    virtual void UpdateHoverState(CPoint pos) override; // 4d5c9a (client: 4da5be)
    virtual void UpdateAmbientEvents() override; // 4d5ef0 (client: 4da814)
    virtual void LoadTownGfx() override; // 4d4866 (client: 4d918a)
    virtual void FreeTownGfx() override; // 4d5112 (client: 4d9a36)
    virtual void AdvanceAnims() override; // 4d6786 (client: 4db097)

    void AdvanceGateAnim(); // 4d6480 (client: 4dad9c)
    void AdvanceGirl1Anim(); // 4d65dd (client: 4daef7)
    void AdvanceGirl2Anim(); // 4d6655 (client: 4daf6c)
    void AdvanceGuardAnim(); // 4d66ca (client: 4dafe1)
    void AdvanceDervishAnim(); // 4d6728 (client: 4db03c)

    VisTownKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 4d423a (client: 4d8b5e)
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
    TownMenuListDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CRect& _r); // 44160e (client: 443bdd)
};
ASSERT_SIZE(TownMenuListDialogVisualObject, 0x7c);

#endif
