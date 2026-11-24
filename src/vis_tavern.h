#ifndef A2_VIS_TAVERN_H
#define A2_VIS_TAVERN_H

#pragma once
#include "vis_core.h"

struct ScenarioTavernEntryId {
    enum Kind : uint32_t {
        KIND_DIALOG_NPC     = 0,
        KIND_HIREABLE_ALLY  = 1,
        KIND_FIXED_ALLY     = 2,
        KIND_UNLOCK_MISSION = 3,
    };

    enum : uint32_t {
        HIRED_FLAG = 0x80000000u,
    };

    union {
        uint32_t raw;
        struct {
            uint32_t entry_id           : 16;
            uint32_t talk_or_mission_id : 12;
            uint32_t kind               : 3;
            uint32_t is_hired           : 1;
        };
    };

    constexpr ScenarioTavernEntryId(uint32_t v = 0) : raw(v) {}
    constexpr operator uint32_t() const { return this->raw; }
};
ASSERT_SIZE(ScenarioTavernEntryId, 4);

class VisTavSceneAnim {
public:
    void LoadAnimFrames(CStringArray* names); // 4010ee (client: 4010ee)
    void FreeAnimFrames(); // 4014f2 (client: 4014f1)
    bool StepForward(); // 4015c6 (client: 4015c5)
    bool StepBackward(); // 401613 (client: 401612)
    void NextFrame(); // 401659 (client: 401658)
    void Draw(int32_t x, int32_t y); // 4016ef (client: 4016ee)

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
    virtual ~VisTavRightPanel(); // 499862 (client: 49d882)

    virtual const char* GetHint() override; // 4a4710 (client: 4a7eb0)
    virtual void Draw() override; // 499dfa (client: 49de1a)
    virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49a26e (client: 49e28e)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 49a291 (client: 49e2b1)
    virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 49a30a (client: 49e32a)
    virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 49a2e6 (client: 49e306)

    VisTavRightPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); // 4997c6 (client: 49d7e6)

    void LoadRightPanelBmps(); // 499a67 (client: 49da87)
    void ResetButtonStates(); // 49a84e (client: 49e87d)
    void UpdateButtonLabels(); // 49a973 (client: 49e9a2)
    void FreeRightPanelBmps(); // 499cdf (client: 49dcff) - client FreeBitmaps restructured away (both client 283b FreeBitmaps fns belong to VisCharGenAction 42d704 / VisCharSellectButtons 432ae6)
    void InitRightPanel(); // 4998c2 (client: 49d8e2)
    int32_t HitTestButton(CPoint pos); // 49a873 (client: 49e8a2)
    void UpdateHoveredButton(uint32_t wparam, CPoint pos); // 49a8fa (client: 49e929) - client FreeBitmaps restructured away (42d41b is VisCharGenAction::UpdateMouseOver)

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
    virtual ~VisTavLeftPanel(); // 497a21 (client: 49ba41)

    virtual const char* GetHint() override; // 49934d (client: 49d36d)
    virtual void Draw() override; // 497ed0 (client: 49bef0)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 497ec1 (client: 49bee1)

    VisTavLeftPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); // 497990 (client: 49b9b0)

    void LoadLeftPanelBmps(); // 4995d1 (client: 49d5f1)
    void FreeLeftPanelBmps(); // 4996ab (client: 49d6cb)
    void InitLeftPanel(); // 497ace (client: 49baee)
    void DrawUnitPanel(CUnit* unit); // 497f82 (client: 49bfa2)
    void SelectRosterEntry(int32_t idx); // 497e06 (client: 49be26)

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
    virtual ~VisTavScene(); // 49ae99 (client: 49eec8)

    virtual const char* GetHint() override; // 49cd2b (client: 4a0d59)
    virtual void Draw() override; // 49b22f (client: 49f25e)
    virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49d8d9 (client: 4a1907)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 49da98 (client: 4a1ac6)
    virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 49d8e8 (client: 4a1916)

    virtual void LoadUnitSprites(); // 49ced4 (client: 4a0f02)
    virtual void FreeUnitSprites(); // 49d1b0 (client: 4a11de)
    virtual void LoadSceneGfx(); // 49d2bb (client: 4a12e9)
    virtual void FreeSceneGfx(); // 49d5f9 (client: 4a1627)

    VisTavScene(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); // 49ad56 (client: 49ed85)

    void DrawQuestRoster(); // 49bc23 (client: 49fc52)
    int32_t HitTestRoster(CPoint* pos); // 49cab8 (client: 4a0ae6)
    void InitSceneRects(); // 49af8c (client: 49efbb)

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
    virtual ~VisTavSceneDruid(); // 4a3fc0 (client: 4a80e0)

    virtual void Draw() override; // 4a0673 (client: 4a46a1)
    virtual void LoadSceneGfx() override; // 4a11e3 (client: 4a5211)
    virtual void FreeSceneGfx() override; // 4a158f (client: 4a55bd)

    VisTavSceneDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); // 4a05a0 (client: 4a45ce)

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
    virtual ~VisTavSceneKaarg(); // 4a40c0 (client: 4a81e0)

    virtual void Draw() override; // 4a2bb8 (client: 4a6be6)
    virtual void LoadSceneGfx() override; // 4a37e7 (client: 4a7815)
    virtual void FreeSceneGfx() override; // 4a3b25 (client: 4a7b53)

    VisTavSceneKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); // 4a2aa4 (client: 4a6ad2)

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
    virtual ~VisTav(); // 49dc33 (client: 4a1c61)

    virtual void Draw() override; // 49e044 (client: 4a2072)
    virtual void DrawBackground(CRect* rect) override; // 49e075 (client: 4a20a3)
    virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 49e082 (client: 4a20b0)
    virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49e261 (client: 4a228f)
    virtual int32_t OnKeyDown(uint32_t wparam) override; // 49e233 (client: 4a2261)

    virtual void OnCreate() override; // 49ddaf (client: 4a1ddd)
    virtual void DoOpen() override; // 49e34f (client: 4a237d)
    virtual void DoClose(uint32_t code) override; // 49eb6c (client: 4a2b9a)

    virtual void LoadTavernSfx(); // 49f298 (client: 4a32c6)
    virtual void FreeTavernSfx(); // 49f3d7 (client: 4a3405)

    void CloseDialog(); // 49edec (client: 4a2e1a)
    void SelectNextPartyUnit(); // 49ee36 (client: 4a2e64)
    void SelectPrevPartyUnit(); // 49eecd (client: 4a2efb)
    int32_t FindEntryIndexForUnit(CUnit* unit); // 49e2e3 (client: 4a2311)
    void ShowHireResponseDialog(); // 49ef63 (client: 4a2f91)
    void RetalkAvailNpc(); // 49f0da (client: 4a3108)
    void ShowReservedNpcTalk(); // 49f179 (client: 4a31a7)

    VisTav(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 49db79 (client: 4a1ba7)

public:
    VisPlayingArea* map_context;
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
    virtual ~VisTavDruid(); // 4a3f70 (client: 4a8070)

    virtual void OnCreate() override; // 49f51b (client: 4a3549)
    virtual void DoOpen() override; // 49faaa (client: 4a3ad8)
    virtual void DoClose(uint32_t code) override; // 4a0278 (client: 4a42a6)
    virtual void LoadTavernSfx() override; // 49f7b0 (client: 4a37de)
    virtual void FreeTavernSfx() override; // 49f961 (client: 4a398f)

    VisTavDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 49f4e6 (client: 4a3514)
public:
    SfxSample* snd_druid[11];
};
ASSERT_SIZE(VisTavDruid, 0x16c);

//60c880
class VisTavKaarg : public VisTav {
public:
    virtual ~VisTavKaarg(); // 4a4070 (client: 4a8190)

    virtual void OnCreate() override; // 4a190d (client: 4a593b)
    virtual void DoOpen() override; // 4a1ba2 (client: 4a5bd0)
    virtual void DoClose(uint32_t code) override; // 4a2448 (client: 4a6476)
    virtual void LoadTavernSfx() override; // 4a26d9 (client: 4a6707)
    virtual void FreeTavernSfx() override; // 4a2873 (client: 4a68a1)

    VisTavKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 4a18d8 (client: 4a5906)
public:
    SfxSample* snd_kaarg[10];
};
ASSERT_SIZE(VisTavKaarg, 0x168);

#endif
