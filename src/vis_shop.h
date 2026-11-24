#ifndef A2_VIS_SHOP_H
#define A2_VIS_SHOP_H

#pragma once
#include "vis_core.h"

//60d480
class VisShopCompass : public CVisualObject {
public:
    virtual ~VisShopCompass(); // 4be1df (client: 4c2ba4)
    virtual const char* GetHint() override; // 4bea64 (client: 4c3429)
    virtual void Draw() override; // 4beb77 (client: 4c353c)
    virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4bf6ce (client: 4c4091)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4bf748 (client: 4c410b)
    virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4bfa01 (client: 4c43bc)

    virtual void LoadBaseGfx(); // 4be238 (client: 4c2bfd)
    virtual void FreeBaseGfx(); // 4be372 (client: 4c2d37)
    virtual void LoadYesFrames(); // 4be5de (client: 4c2fa3)
    virtual void LoadNoFrames(); // 4be6df (client: 4c30a4)
    virtual void AdvanceCenterFrame(); // 4be7e0 (client: 4c31a5)
    virtual void LoadDirectionFrames(int32_t dir); // 4be45d (client: 4c2e22)
    virtual void FreeDirectionFrames(int32_t dir); // 4be541 (client: 4c2f06)
    virtual void FreeYesFrames(); // 4be8b0 (client: 4c3275)
    virtual void FreeNoFrames(); // 4be92d (client: 4c32f2)
    virtual void FreeCenterFrame(); // 4be9aa (client: 4c336f)
    virtual void FreeAllAnimFrames(); // 4bea01 (client: 4c33c6)
    virtual int32_t SelectCategory(int32_t category); // 4bf4e8 (client: 4c3eab)
    virtual int32_t GetHoverRegion(); // 4c6bc0 (client: 4cb560)

    VisShopCompass(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); // 4bd495 (client: 4c1e5a)

    void AdvanceDirectionFrames(int32_t dir); // 4bf63f (client: 4c4002)

public:
    VisShop* shop; // 0x5c
    CRect outer_rects[4]; // 0x60
    CRect inner_rects[4]; // 0xa0
    CRect center_rect; // 0xe0
    CRect confirm_rect; // 0xf0
    CRect confirm_yes_rect; // 0x100
    CRect confirm_no_rect; // 0x110
    CSprite256* frame_sprite; // 0x120
    CBmp64* bkg_bmp; // 0x124
    CBmp64* idle_bmp; // 0x128
    CBmp64* trigger_bmp; // 0x12c
    CBmp64* direction_frames[4][11]; // 0x130
    CBmp64* fwd_frames[12]; // 0x1e0
    CBmp64* ret_frames[12]; // 0x210
    int32_t state; // 0x240
    int32_t dir_frm[4]; // 0x244
    int32_t center_frm; // 0x254
};
ASSERT_SIZE(VisShopCompass, 0x258);

//60d658
class VisShopCompassDruid : public VisShopCompass {
public:
    virtual ~VisShopCompassDruid(); // 4c6a30 (client: 4cb3d0)

    virtual const char* GetHint() override; // 4c3690 (client: 4c8047)
    virtual void Draw() override; // 4c30b4 (client: 4c7a6d)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4c359d (client: 4c7f56)
    virtual void LoadBaseGfx() override; // 4c2882 (client: 4c723b)
    virtual void FreeBaseGfx() override; // 4c2b29 (client: 4c74e2)
    virtual void LoadYesFrames() override; // 4c2d8a (client: 4c7743)
    virtual void LoadNoFrames() override; // 4c2da3 (client: 4c775c)
    virtual void AdvanceCenterFrame() override; // 4c2dbc (client: 4c7775)
    virtual void LoadDirectionFrames(int32_t dir) override; // 4c2cce (client: 4c7687)
    virtual void FreeDirectionFrames(int32_t dir) override; // 4c2ced (client: 4c76a6)
    virtual void FreeYesFrames() override; // 4c2f00 (client: 4c78b9)
    virtual void FreeNoFrames() override; // 4c2f7d (client: 4c7936)
    virtual void FreeCenterFrame() override; // 4c2ffa (client: 4c79b3)
    virtual void FreeAllAnimFrames() override; // 4c3051 (client: 4c7a0a)
    virtual int32_t SelectCategory(int32_t category) override; // 4c34e6 (client: 4c7e9f)

    VisShopCompassDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); // 4c247b (client: 4c6e34)

    int32_t IsDruidShopAvailable(); // 4c2842 (client: 4c71fb)

public:
    int32_t gap_0x258; // 0x258
    CBmp64* armor_bmp; // 0x25c
    CBmp64* magic_bmp; // 0x260
    CBmp64* potions_bmp; // 0x264
    CBmp64* sel_goods_bmp; // 0x268
    CBmp64* category_bmp; // 0x26c
};
ASSERT_SIZE(VisShopCompassDruid, 0x270);

//60d7a0
class VisShopCompassKaarg : public VisShopCompass {
public:
    virtual ~VisShopCompassKaarg(); // 4c6ad0 (client: 4cb470)

    virtual const char* GetHint() override; // 4c6800 (client: 4cb19b)
    virtual void Draw() override; // 4c608e (client: 4caa2b)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4c671f (client: 4cb0bc)
    virtual void LoadBaseGfx() override; // 4c4a78 (client: 4c942f)
    virtual void FreeBaseGfx() override; // 4c5a80 (client: 4ca41d)
    virtual void LoadYesFrames() override; // 4c5d64 (client: 4ca701)
    virtual void LoadNoFrames() override; // 4c5d7d (client: 4ca71a)
    virtual void AdvanceCenterFrame() override; // 4c5d96 (client: 4ca733)
    virtual void LoadDirectionFrames(int32_t dir) override; // 4c5ca8 (client: 4ca645)
    virtual void FreeDirectionFrames(int32_t dir) override; // 4c5cc7 (client: 4ca664)
    virtual void FreeYesFrames() override; // 4c5eda (client: 4ca877)
    virtual void FreeNoFrames() override; // 4c5f57 (client: 4ca8f4)
    virtual void FreeCenterFrame() override; // 4c5fd4 (client: 4ca971)
    virtual void FreeAllAnimFrames() override; // 4c602b (client: 4ca9c8)
    virtual int32_t SelectCategory(int32_t category) override; // 4c65ba (client: 4caf57)

    VisShopCompassKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); // 4c4693 (client: 4c904a)

public:
    int32_t gap_0x258; // 0x258
    CBmp64* armor_bmp; // 0x25c
    CBmp64* magic_bmp; // 0x260
    CBmp64* scrolls_bmp; // 0x264
    CBmp64* weapons_bmp; // 0x268
    CBmp64* tail_frames[18]; // 0x26c
    CBmp64* wpn_tail_frames[18]; // 0x2b4
    int tail_frame; // 0x2fc
    int wpn_tail_frame; // 0x300
};
ASSERT_SIZE(VisShopCompassKaarg, 0x304);

//60d540
class VisShopButtons : public CVisualObject {
public:
    virtual ~VisShopButtons(); // 4c0020 (client: 4c49db)
    virtual const char* GetHint() override { return nullptr; } // 4c04a4 (client: 4c4e5d)
    virtual void Draw() override; // 4c04b1 (client: 4c4e6a)
    virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4c10c4 (client: 4c5a7d)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4c1134 (client: 4c5aed)
    virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4c11c2 (client: 4c5b7b)
    virtual int32_t GetHoverRegion(); // 4c6bd0 (client: 4cb570)

    void ReleaseBmp(); // 4c0352 (client: 4c4d0b)
    void UpdateHoveredState(uint32_t wparam, CPoint pos); // 4c14c4 (client: 4c5e7d)
    int32_t GetButtonAt(CPoint pos); // 4c137d (client: 4c5d36)
    void LoadBmp(); // 4c0088 (client: 4c4a43)
    void ResetSelected(); // 4c1358 (client: 4c5d11)

    VisShopButtons(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); // 4bfa5a (client: 4c4415)

public:
    VisShop* shop; // 0x5c
    int32_t gap_0x60;
    CBmp64* button_bmps[4]; // 0x64
    CBmp64* menu_bmp; // 0x74
    CRect outer_rects[4]; // 0x78
    CRect inner_rects[4]; // 0xb8
    int32_t pressed_btn; // 0xf8
    int32_t hovered_btn; // 0xfc
};
ASSERT_SIZE(VisShopButtons, 0x100);

//60d3e8
class VisShop : public VisScreen {
public:
    virtual ~VisShop(); // 4ba5a0 (client: 4bef70)

    virtual void DrawBackground(CRect* rect) override; // 4bb79a (client: 4c0167)
    virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4ba892 (client: 4bf262)
    virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4badcb (client: 4bf798)
    virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4bb045 (client: 4bfa12)
    virtual int32_t OnKeyDown(uint32_t wparam) override; // 4ba832 (client: 4bf202)

    virtual void OnCreate() override; // 4bbd75 (client: 4c0742)
    virtual void DoOpen() override; // 4bc32b (client: 4c0cf8)
    virtual void DoClose(uint32_t code) override; // 4bc0e7 (client: 4c0ab4)

    virtual void UpdateShopSfx(); // 4bcf2f (client: 4c18f4)
    virtual void LoadShopSfx(); // 4bd0fb (client: 4c1ac0)
    virtual void FreeShopSfx(); // 4bd251 (client: 4c1c16)
    virtual CString GetAssetSubdir(); // 4c6b90 (client: 4cb530)


    void CloseShopDialog(); // 4bcd02 (client: 4c16d3)
    void DrawInfoPanelEdge(); // 4bad1d (client: 4bf6ea)
    void FreeShopGfx(); // 4bb4fb (client: 4bfec8)
    void SelectNextPartyUnit(); // 4bb895 (client: 4c0262)
    void SelectPrevPartyUnit(); // 4bba06 (client: 4c03d3)
    void LoadShopGfx(); // 4bb102 (client: 4bfacf)
    void SetAssortmentCategory(int32_t category, CArray<TokenEntry*>* arr); // 4bbbd6 (client: 4c05a3)
    void UpdateShopTips(); // 4bc8d7 (client: 4c12a4)
    void ReturnPendingBuyItems(); // 4bcc38 (client: 4c1604)
    void CancelItemDrag(); // 4bc97b (client: 4c1348)
    void ShowSpellPanel(); // 4bcd4b (client: 4c171c)
    void ShowBuyPanel(); // 4bcd79 (client: 4c174a)
    void RecalcShopGold(); // 4bcda0 (client: 4c1771)
    void ResetInfoPanelButtons(); // 4bcea4 (client: 4c1875)
    void CommitSell(); // 4bcaf1 (client: 4c14bd)
    int CommitBuy(); // 4bcb63 (client: 4c152f)
    void CancelAllPendingBuys(); // 4bcce1 (client: 4c16b2)

    VisShop(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); // 4ba342 (client: 4bed12)

public:
    VisInvExtType1* assortiment;
    VisInvExtType2* to_sell;
    VisInvExtType3* to_buy;
    VisShopCompass* shop_compass;
    VisShopButtons* buttons;
    CVisualObject* select_info_panel;
    VisPlayingArea* gameplay;
    VisSpellBook* spell_panel;
    CVisualObject* tips;
    int32_t tips_update_flag;
    SfxSample* snd_notif;
    SfxSample* snd_step1;
    SfxSample* snd_step2;
    SfxSample* snd_breath;
    SfxSample* snd_depart;
    SfxSample* snd_buy;
    SfxSample* snd_sell;
    SfxSample* snd_enter;
    SfxSample* snd_start;
    SfxSample* snd_pov1;
    SfxSample* snd_pov2;
    SfxSample* snd_inshop;
    SfxSample* snd_out;
    SfxSample* snd_undo;
    CBmp64* bmp_backinvg;
    CBmp64* bmp_backinvb;
    CBmp64* bmp_backinvs;
    CSprite256* spr_myitem;
    CSprite256* spr_shopitem;
    CArray<CBmp64*> bmp_cost_medium;
    CArray<CBmp64*> bmp_cost_small;
    int32_t hovered_region;
    CArray<CUnit*> selected_units;
    CWordArray units_order;
    int16_t select_index;
    int16_t select_category;
    int32_t gap_0x134;
    int32_t gap_0x138;
    int32_t gap_0x13c;
    int32_t gap_0x140;
    int32_t placement_lock;
    int32_t dialog_active;
    int32_t dirty;
    int32_t current_gold;
    int32_t buy_gold;
    int32_t sell_gold;
    int32_t result_gold;
    int32_t scenario_talk_target;
};
ASSERT_SIZE(VisShop, 0x164);

//60d5c0
class VisShopDruid : public VisShop {
public:
    virtual ~VisShopDruid(); // 4c69e0 (client: 4cb380)

    virtual void OnCreate() override; // 4c15e6 (client: 4c5f9f)
    virtual void DoOpen() override; // 4c1e5c (client: 4c6815)
    virtual void UpdateShopSfx() override; // 4c1958 (client: 4c6311)
    virtual void LoadShopSfx() override; // 4c1b8b (client: 4c6544)
    virtual void FreeShopSfx() override; // 4c1d25 (client: 4c66de)
    virtual CString GetAssetSubdir() override; // 4c6be0 (client: 4cb580)

    VisShopDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); // 4c15ad (client: 4c5f66)
public:
    SfxSample* snd_bird[3];
    SfxSample* snd_tool[4];
    int32_t bird_tick;
    int32_t tool_tick;
    int32_t next_bird;
    int32_t next_tool;
};
ASSERT_SIZE(VisShopDruid, 0x190);

//60d708
class VisShopKaarg : public VisShop {
public:
    virtual ~VisShopKaarg(); // 4c6a80 (client: 4cb420)

    virtual void OnCreate() override; // 4c37fe (client: 4c81b5)
    virtual void DoOpen() override; // 4c3b70 (client: 4c8527)
    virtual void UpdateShopSfx() override; // 4c4170 (client: 4c8b27)
    virtual void LoadShopSfx() override; // 4c43a3 (client: 4c8d5a)
    virtual void FreeShopSfx() override; // 4c453d (client: 4c8ef4)
    virtual CString GetAssetSubdir() override; // 4c6c10 (client: 4cb5b0)

    VisShopKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); // 4c37c5 (client: 4c817c)
public:
    SfxSample* snd_voice[3];
    SfxSample* snd_tool[4];
    int32_t voice_tick;
    int32_t tool_tick;
    int32_t next_voice;
    int32_t next_tool;
};
ASSERT_SIZE(VisShopKaarg, 0x190);

#endif
