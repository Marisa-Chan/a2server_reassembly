#include <winsock2.h>
#include <ws2tcpip.h>
#include <timeapi.h>

#include "server.h"
#include "main_window.h"
#include "gfx.h"
#include "mouse.h"
#include "file.h"
#include "game_app.h"
#include "quest_map.h"
#include "quest.h"
#include "ingame.h"
#include "util.h"
#include "resource.h"
#include "spell.h"
#include "players_list.h"
#include "player.h"
#include "unit.h"
#include "unit_list.h"
#include "buildings_list.h"
#include "packet.h"
#include "vis_tavern.h"
#include "vis_menu.h"
#include "gameplay.h"
#include "vis_playing_area.h"
#include "vis_char.h"
#include "vis_inventory.h"

// Statics for VisTavSceneDruid::VMethod7 (665D44-665D6C, 631FB0 in the binary).
static bool tavscene_druid_statics_inited = false;  // 665D64 init bitmask
static uint32_t tavscene_druid_anim_delay = 0;      // 665D50 delay before druid animation; low bits select which one plays
static uint32_t tavscene_druid_frame_ts = 0;        // 665D58
static uint32_t tavscene_druid_anims_ts = 0;        // 665D5C
static uint32_t tavscene_druid_state_ts = 0;        // 665D44
static uint32_t tavscene_druid_ambient_ts = 0;      // 665D6C (written, never read)
static int32_t tavscene_druid_anim_state = -1;      // 631FB0 -1=idle, 1=anims_druid[1], 2=anims_druid[2], 3/4=field_0x3cc flash




// Statics for VisTavSceneKaarg::VMethod7 (665D40-665D74, 631E30, 631FB4 in the binary).
static bool tavscene_kaarg_statics_inited = false;  // 665D40 init bitmask
static uint32_t tavscene_kaarg_unused_rand = 0;     // 665D48 (written, never read)
static uint32_t tavscene_kaarg_frame_ts = 0;        // 665D4C
static uint32_t tavscene_kaarg_unused_ts1 = 0;      // 665D74 (written, never read)
static uint32_t tavscene_kaarg_state_ts = 0;        // 665D54
static uint32_t tavscene_kaarg_unused_ts2 = 0;      // 665D70 (written, never read)
static int32_t tavscene_kaarg_anim_state = -1;      // 631FB4 -1=idle, 1-5=current animation
// 631E30 Scripted frame indices for anims_kaarg[4] (state 5), -1 terminated.
static const int32_t kaarg_frame_script[96] = {
    0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 14, 15, 16, 17, 18,
    19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
    22, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 5,
    6, 7, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    25, 26, 27, 28, -1,
};

// Statics for VisTavScene::VMethod7 (665D60-665D84 in the binary).
static bool tavscene_statics_inited = false;
static uint32_t tavscene_anim_delay = 0;    // 665D78 delay before tavern animation; bit 0 selects which one plays
static uint32_t tavscene_frame_ts = 0;      // 665D3C
static uint32_t tavscene_anims_ts = 0;      // 665D68
static uint32_t tavscene_state_ts = 0;      // 665D38
static uint32_t tavscene_ambient_ts = 0;    // 665D7C
static int32_t tavscene_anim_state = 0;     // 665D80 0=none, 1=anims[3], 2=anims[2]
static int32_t tavscene_anim_substate = 0;  // 665D84 1=playing forward, -1=playing backward

// 49E34F
void VisTav::VMethod28() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    g_mousept.DisableHint();

    this->map_context = main_wnd->vis_map_context;
    this->info_panel = main_wnd->vis_charinfo;
    main_wnd->vis_right_panel->RemoveChild(this->info_panel);

    CRect& panel_rect = this->info_panel->GetRect();
    CPoint pt(0x280 - panel_rect.Width(), 0);
    panel_rect.OffsetRect(pt.x, pt.y);
    this->info_panel->SetRect(&panel_rect);
    this->AddChild(this->info_panel);

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        CString tips_text;
        MissionGetTips(2, &tips_text);
        this->tips = new VisTipsDialog(0x467, 0, 0, 0x138, 0xC8, tips_text);
        this->scene->AddChild(this->tips);
    } else {
        if (this->tips != nullptr) {
            this->scene->RemoveChild(this->tips);
            delete this->tips;
        }
        this->tips = nullptr;
    }

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        int32_t ids[32];
        int32_t count;
        ScenarioEnterInn(ids, &count);
        this->entrie_id.RemoveAll();
        for (int32_t i = 0; i < count; i++) {
            this->entrie_id.Add(ids[i]);
        }
    }

    this->avail_entries.RemoveAll();
    this->reserved_entries.RemoveAll();
    for (int32_t i = 0; i < this->entrie_id.GetSize(); i++) {
        uint32_t unit_id = this->entrie_id[i] & 0xFFFF;
        uint32_t category = (this->entrie_id[i] >> 0x1C) & 7;
        if (category == 1 || category == 2) {
            this->avail_entries.Add(this->map_context->FindOrCreateUnitByServerId(unit_id));
        } else {
            this->reserved_entries.Add(this->map_context->FindOrCreateUnitByServerId(unit_id));
        }
    }

    if (this->avail_entries.GetSize() + this->reserved_entries.GetSize() != 0) {
        this->selection_index = 0;
    } else {
        this->selection_index = -1;
    }

    g_StructEnter.RebuildFromMap();
    this->selected_entries.Copy(g_StructEnter.structure_list);
    this->select_party = g_StructEnter.FindFlaggedStructureUnit();

    this->map_context->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);
    this->selected_entries[this->select_party]->SetSelected(1);
    this->map_context->UpdateSelectionState();
    this->selected_entries[this->select_party]->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;

    this->scene->VMethod26();

    CStringArray names;
    CString name;

    for (int32_t i = 0; i < 10; i++) {
        name.Format("graphics\\interface\\inn\\candle\\t%.4d.bmp", i);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        this->scene->anims[0].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 0x15; i++) {
        name.Format("graphics\\interface\\inn\\cauldron\\t%.4d.bmp", i);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        this->scene->anims[1].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    for (int32_t i = 1; i <= 0x18; i++) {
        name.Format("graphics\\interface\\inn\\tender\\breath\\br%.4d.bmp", i);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        this->scene->anims[2].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    for (int32_t i = 1; i <= 0x28; i++) {
        name.Format("graphics\\interface\\inn\\tender\\drink\\dr%.4d.bmp", i);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        this->scene->anims[3].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    this->right_panel->LoadRightPanelBmps();
    this->right_panel->ResetButtonStates();
    this->scene->VMethod28();
    this->left_panel->LoadLeftPanelBmps();
    this->right_panel->UpdateButtonLabels();
    this->VMethod30();

    this->dialog_active = 1;

    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();

    VisScreen::VMethod28();
    CSound::Play(this->sounds[8]);
    g_Cursors[0]->Use();
    g_mousept.EnableHint();

    this->quest_id = 0;
    this->quest_selection = -1;
    this->quest_alt_close = 0;
}


// 49EB6C
void VisTav::DoClose(uint32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    this->VMethod9();
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        ScenarioLeaveInn();
    }

    if (this->quest_alt_close != 0) {
        main_wnd->vis_map_context->LeaveInn(0xAAAAAAAA);
    } else {
        main_wnd->vis_map_context->LeaveInn(this->quest_selection);
    }

    this->dialog_active = 0;

    CRect& panel_rect = this->info_panel->GetRect();
    CPoint pt(panel_rect.Width() - 0x280, 0);
    panel_rect.OffsetRect(pt.x, pt.y);
    this->info_panel->SetRect(&panel_rect);
    this->RemoveChild(this->info_panel);
    main_wnd->vis_right_panel->AddChild(this->info_panel);

    if (this->tips != nullptr) {
        this->scene->RemoveChild(this->tips);
        delete this->tips;
        this->tips = nullptr;
    }

    this->right_panel->FreeRightPanelBmps();
    this->scene->VMethod29();
    this->scene->VMethod27();
    this->left_panel->FreeLeftPanelBmps();

    this->scene->anims[0].FreeAnimFrames();
    this->scene->anims[1].FreeAnimFrames();
    this->scene->anims[3].FreeAnimFrames();
    this->scene->anims[2].FreeAnimFrames();

    this->VMethod31();

    this->avail_entries.RemoveAll();
    this->selected_entries.RemoveAll();
    this->reserved_entries.RemoveAll();

    VisScreen::DoClose(code);
    this->quest_map->RemovePlayerQuests(0);

    while (this->rewards.GetSize() != 0) {
        TokenEntry* entry = this->rewards[0];
        if (entry != nullptr) {
            delete entry;
        }
        this->rewards.RemoveAt(0, 1);
    }
}


// 49DDAF
void VisTav::VMethod26() {
    this->dialog_active = 0;
    for (int32_t i = 0; i < 13; i++) {
        this->sounds[i].sample = nullptr;
    }
    this->tips = nullptr;

    this->left_panel = new VisTavLeftPanel(0x44D, 0, 0, 0xA0, 0x1E0, this);
    this->right_panel = new VisTavRightPanel(0x44E, 0x1E0, 0, 0x280, 0xEE, this);
    this->scene = new VisTavScene(0x450, 0xA0, 0, 0x1E0, 0x1E0, this);

    this->AddChild(this->left_panel);
    this->AddChild(this->right_panel);
    this->AddChild(this->scene);

    this->selection_index = 0;
    this->avail_entries.RemoveAll();
    this->quest_map = new QuestMap();
}


// 49E082
int32_t VisTav::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    case VIS_MSG_FRAME_TICK:
        if (this->rewards.GetSize() != 0 || this->quest_map->GetQuestCount() != 0) {
            if (this->right_panel->texts[0] == " ") {
                this->right_panel->UpdateButtonLabels();
            }
        }
        if (main_wnd->dialogsMask == DIALOG_MASK_TAVERN || main_wnd->dialogsMask == (DIALOG_MASK_INGAME | DIALOG_MASK_TAVERN)) {
            this->VMethod9();
        }
        break;
    case VIS_MSG_SELECT_PREV:
        this->SelectPrevPartyUnit();
        break;
    case VIS_MSG_SELECT_NEXT:
        this->SelectNextPartyUnit();
        break;
    case VIS_MSG_CLOSE_TIPS:
        if (this->tips != nullptr) {
            this->scene->RemoveChild(this->tips);
            delete this->tips;
            this->tips = nullptr;
        }
        break;
    }

    return VisScreen::MsgProc(msg, wparam, lparam);
}


// 49F298
void VisTav::VMethod30() {
    this->VMethod31();
    ReloadSfx(&this->sounds[0].sample, "SFX\\Town\\Inn\\drink.wav");
    ReloadSfx(&this->sounds[1].sample, "SFX\\Town\\Inn\\glotok.wav");
    ReloadSfx(&this->sounds[2].sample, "SFX\\Town\\Inn\\steam.wav");
    ReloadSfx(&this->sounds[3].sample, "SFX\\Town\\Inn\\water.wav");
    ReloadSfx(&this->sounds[4].sample, "SFX\\Town\\Inn\\chair.wav");
    ReloadSfx(&this->sounds[5].sample, "SFX\\Add.wav");
    ReloadSfx(&this->sounds[6].sample, "SFX\\NoAdd.wav");
    ReloadSfx(&this->sounds[7].sample, "SFX\\Town\\Shop\\nofit.wav");
    ReloadSfx(&this->sounds[8].sample, "SFX\\Town\\Inn\\enter.wav");
    ReloadSfx(&this->sounds[9].sample, "SFX\\Town\\Inn\\Helper.wav");
    ReloadSfx(&this->sounds[10].sample, "SFX\\Town\\Shop\\Breath.wav");
    ReloadSfx(&this->sounds[11].sample, "SFX\\Out.wav");
    ReloadSfx(&this->sounds[12].sample, "SFX\\Talk.wav");
}


// 49F3D7
void VisTav::VMethod31() {
    for (int32_t i = 0; i < 13; i++) {
        FreeSfxSample(&this->sounds[i].sample);
    }
}


// 49DB79
VisTav::VisTav(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) {
}


// 49E261
int32_t VisTav::OnMouseMove(uint32_t wparam, CPoint pos) {
    CRect r = this->right_panel->GetRect() + this->rect.TopLeft();
    if (!r.PtInRect(pos)) {
        this->right_panel->ResetButtonStates();
    }

    return CVisualObject::OnMouseMove(wparam, pos);
}


// 49EECD
void VisTav::SelectPrevPartyUnit() {
    this->map_context->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);

    if (this->select_party != 0) {
        this->select_party--;
    } else {
        this->select_party = this->selected_entries.GetUpperBound();
    }

    this->selected_entries[this->select_party]->SetSelected(1);
    this->map_context->UpdateSelectionState();
}


// 49EE36
void VisTav::SelectNextPartyUnit() {
    this->map_context->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);

    this->select_party++;
    if (this->select_party >= this->selected_entries.GetSize()) {
        this->select_party = 0;
    }

    this->selected_entries[this->select_party]->SetSelected(1);
    this->map_context->UpdateSelectionState();
}


// 49EF63
void VisTav::ShowHireResponseDialog() {
    CUnit* unit = this->avail_entries[this->selection_index];
    int32_t entry = this->FindEntryIndexForUnit(unit);
    uint32_t npc_id = this->entrie_id[entry] & 0xFFFF;

    CString text;
    if ((this->entrie_id[entry] & 0x70000000) == 0x10000000) {
        text.Format("npc%daccept%d", npc_id, ScenarioGetVar(0x300));
        ShowRoleKeyDialog(text);
        this->entrie_id[entry] |= 0x80000000;
        ScenarioTalkTo(this->entrie_id[entry]);
    } else {
        text.Format("npc%dreject%d", npc_id, ScenarioGetVar(0x300));
        ShowRoleKeyDialog(text);
    }

    CSound::Play(this->sounds[5]);
}


// 49F179
void VisTav::ShowReservedNpcTalk() {
    CUnit* unit = this->reserved_entries[this->selection_index - this->avail_entries.GetSize()];
    int32_t entry = this->FindEntryIndexForUnit(unit);
    uint32_t v = this->entrie_id[entry];

    CString text;
    text.Format("npc%dtalk%d", v & 0xFFFF, (v >> 0x10) & 0xFFF);
    ShowRoleKeyDialog(text);
    ScenarioTalkTo(this->entrie_id[entry]);

    CSound::Play(this->sounds[12]);
}


// 49F0DA
void VisTav::RetalkAvailNpc() {
    CUnit* unit = this->avail_entries[this->selection_index];
    int32_t entry = this->FindEntryIndexForUnit(unit);
    this->entrie_id[entry] &= 0x7FFFFFFF;

    unit = this->avail_entries[this->selection_index];
    entry = this->FindEntryIndexForUnit(unit);
    ScenarioTalkTo(this->entrie_id[entry]);

    CSound::Play(this->sounds[6]);
}


// 49E2E3
int32_t VisTav::FindEntryIndexForUnit(CUnit* unit) {
    for (int32_t i = 0; i < this->entrie_id.GetSize(); i++) {
        if (unit->serverId == (this->entrie_id[i] & 0xFFFF)) {
            return i;
        }
    }
    return -1;
}


// 49EDEC
void VisTav::CloseDialog() {
    this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
    if (wnd->dialogsMask == DIALOG_MASK_TAVERN) {
        AfxGetMainWnd()->PostMessageA(MAIN_MSG_SHOW_CURRENT_TOWN, 0, 0);
    }
}


// 49E233
int32_t VisTav::OnKeyDown(uint32_t wparam) {
    if (wparam == VK_ESCAPE) {
        this->CloseDialog();
        return 1;
    }
    return 0;
}


// 49E044
void VisTav::VMethod7() {
    if (this->dialog_active != 0) {
        VisScreen::VMethod7();
    }

    PlayAmbientSfx(&this->sounds[3].sample);
}


// 49E075
void VisTav::VMethod8(CRect* rect) {
}


// 49DC33
VisTav::~VisTav() {
    this->VMethod31();
    this->avail_entries.RemoveAll();
    this->entrie_id.RemoveAll();

    this->RemoveChild(this->info_panel);
    this->info_panel = nullptr;
    this->map_context = nullptr;

    if (this->tips != nullptr) {
        this->scene->RemoveChild(this->tips);
        delete this->tips;
        this->tips = nullptr;
    }

    if (this->quest_map != nullptr) {
        delete this->quest_map;
    }
}


// 49934D
const char* VisTavLeftPanel::GetHint() {
    if (this->vis_tav->dialog_active == 0) {
        return nullptr;
    }
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != SESSION_LOCAL_BATTLE) {
        return nullptr;
    }
    CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    CPoint pt(mouse_pt.x - tav_topleft.x, mouse_pt.y - tav_topleft.y);
    CRect client_rect;
    this->ClientRectToScreen(&client_rect, this->rect);

    CUnit* unit = nullptr;
    if (this->vis_tav->avail_entries.GetSize() != 0 || this->vis_tav->reserved_entries.GetSize() != 0) {
        if (this->vis_tav->selection_index < this->vis_tav->avail_entries.GetSize()) {
            unit = this->vis_tav->avail_entries[this->vis_tav->selection_index];
        } else {
            unit = this->vis_tav->reserved_entries[this->vis_tav->selection_index - this->vis_tav->avail_entries.GetSize()];
        }
    }

    CRect item_rect(CPoint(0, 0), CSize(client_rect.Width(), 0xEE));
    if (item_rect.PtInRect(pt)) {
        if (unit->unitFlags & UNIT_FLAG_REMOTE_HERO) {
            return nullptr;
        }
        return unit->GetStatsHint(pt.x, pt.y);
    }

    client_rect.TopLeft().x -= 4;
    uint8_t* data = (uint8_t*)this->hitmap->GetData();
    CPoint client_topleft = client_rect.TopLeft();
    uint8_t index = data[(mouse_pt.y - client_topleft.y - 0xF0) * 0xA0 + (mouse_pt.x - client_topleft.x) - 0x10];
    if (index == 0) {
        return nullptr;
    }
    return unit->equipmentTokens[index - 1]->GetHintText();
}


// 4995D1
void VisTavLeftPanel::LoadLeftPanelBmps() {
    this->FreeLeftPanelBmps();
    this->bmp_left_stats = new CBmp64("graphics\\Interface\\Inn\\LeftStats.bmp");
    g_mousept.Update();
    this->bmp_left_picture = new CBmp64("graphics\\Interface\\Inn\\LeftPicture.bmp");
    g_mousept.Update();
}


// 4996AB
void VisTavLeftPanel::FreeLeftPanelBmps() {
    if (this->bmp_left_stats != nullptr) {
        delete this->bmp_left_stats;
    }
    this->bmp_left_stats = nullptr;
    if (this->bmp_left_picture != nullptr) {
        delete this->bmp_left_picture;
    }
    this->bmp_left_picture = nullptr;
}


// 497990
VisTavLeftPanel::VisTavLeftPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->vis_tav = tav;
    this->InitLeftPanel();
}


// 4A3E10
VisTavLeftPanel::~VisTavLeftPanel() {
    if (this->portrait_bmp != nullptr) {
        delete this->portrait_bmp;
    }
    if (this->hitmap != nullptr) {
        delete this->hitmap;
    }
}


// 497ACE
void VisTavLeftPanel::InitLeftPanel() {
    this->flags |= 2;
    CRect client_rect;
    this->ClientRectToScreen(&client_rect, this->rect);
    this->left_btn_rect = CRect(CPoint(client_rect.left + 0x11, client_rect.top + 0x1BB), CSize(0x20, 0x20));
    this->right_btn_rect = CRect(CPoint(client_rect.left + 0x87, client_rect.top + 0x1BB), CSize(0x20, 0x20));
    this->hitmap = new CBmp256(0xA0, 0xF0);
    this->portrait_bmp = new CBmp64(0xA0, 0xF0);
    this->picture_name[0] = 0;
    this->bmp_left_picture = nullptr;
    this->bmp_left_stats = nullptr;
}


// 497EC1
int32_t VisTavLeftPanel::OnLButtonDown(uint32_t wparam, CPoint pos) {
    return 0;
}


// 497ED0
void VisTavLeftPanel::VMethod7() {
    if (this->vis_tav->selection_index < 0) {
        this->DrawUnitPanel(nullptr);
    } else if (this->vis_tav->selection_index < this->vis_tav->avail_entries.GetSize()) {
        this->DrawUnitPanel(this->vis_tav->avail_entries[this->vis_tav->selection_index]);
    } else {
        this->DrawUnitPanel(this->vis_tav->reserved_entries[this->vis_tav->selection_index - this->vis_tav->avail_entries.GetSize()]);
    }
}


// 497F82
void VisTavLeftPanel::DrawUnitPanel(CUnit* unit) {
    CRect screen_rect;
    CRect rect_1C(CPoint(0xC, 0), CSize(0xA0, 0xEE));
    this->ClientRectToScreen(&screen_rect, this->rect);
    LockSurface2();
    this->bmp_left_stats->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
    this->bmp_left_picture->Draw(screen_rect.left, screen_rect.top + 0xEE, 0, 0, 0);

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        if (unit == nullptr) {
            g_font2->DrawTextWithShadow(screen_rect.left + 0x58, screen_rect.top + 0x36, TxtFile::AllLines[0x2F], 2, clrsh_DullGold, 1);
            g_font2->DrawTextWithShadow(screen_rect.left + 0x58, screen_rect.top + 0x42, TxtFile::AllLines[0x30], 2, clrsh_DullGold, 1);
        } else {
            if ((unit->unitFlags & UNIT_FLAG_REMOTE_HERO) == 0) {
                CRect unit_rect = rect_1C + screen_rect.TopLeft();
                unit->DrawStatsPanel(&unit_rect);
            }
            if ((unit->unitFlags & (UNIT_FLAG_PLAYER_CLASS | UNIT_FLAG_CREATURE)) == 0) {
                char picture[0x100];
                char suffix[0x50];
                strcpy(picture, g_VFX_info[unit->typeId]->info_picture);
                sprintf(suffix, "%d", unit->face);
                if (unit->face > 1) {
                    strcat(picture, suffix);
                }
                if (strcmp(this->picture_name, picture) != 0) {
                    strcpy(this->picture_name, picture);
                    char path[0x100];
                    sprintf(path, "graphics\\infowindow\\%s.bmp", this->picture_name);
                    this->portrait_bmp->LoadFile(path, nullptr);
                    memset(this->hitmap->GetData(), 0, this->hitmap->GetWidth() * this->hitmap->GetHeight());
                }
            } else {
                char temp_path[0x100];
                char fname[0x100];
                char full_path[0x100];
                GetTempPathA(0x100, temp_path);
                sprintf(fname, "allods-2-%d.$$$", unit->unit_id);
                sprintf(full_path, "%s%s", temp_path, fname);
                if (strcmp(this->picture_name, fname) != 0) {
                    strcpy(this->picture_name, fname);
                    if ((unit->unitFlags & UNIT_FLAG_EQUIP_CHANGED) != 0) {
                        UnlockSurface2();
                        unit->RenderHeroPortrait(full_path, this->portrait_bmp, this->hitmap);
                        LockSurface2();
                    } else {
                        this->portrait_bmp->LoadFile(fname, this->hitmap);
                    }
                } else if ((unit->unitFlags & UNIT_FLAG_EQUIP_CHANGED) != 0) {
                    UnlockSurface2();
                    unit->RenderHeroPortrait(full_path, this->portrait_bmp, this->hitmap);
                    LockSurface2();
                }
            }
            this->portrait_bmp->DrawSubRectMasked(screen_rect.left + 0xB, screen_rect.top + 0xF0, 0, 0, 0xA0, 0xF0);
        }
    }

    uint32_t quest_result = main_wnd->vis_map_context->quest_map->OnEvent(0x11, main_wnd->vis_map_context->my_main_unit->index, this->vis_tav->inn_id);
    Quest* quest;
    int32_t quest_found;
    if (quest_result != 0) {
        quest_found = main_wnd->vis_map_context->quest_map->FindQuest(quest_result, &quest);
    } else {
        quest_found = this->vis_tav->quest_map->FindQuest(this->vis_tav->quest_id, &quest);
    }

    if (quest_found != 0) {
        char picture[0x100];
        picture[0] = 0;
        CString unit_name;
        switch (quest->Kind()) {
        case 1:
        case 3:
        case 4:
        case 0xB:
        case 0xC: {
            int32_t found = 0;
            CGameObject* obj;
            if (quest->Kind() == 3 || quest->Kind() == 0xC) {
                found = main_wnd->vis_map_context->FindUnitByQuestFlag(quest->GetObj(), (CUnit**)&obj);
            } else {
                found = main_wnd->vis_map_context->objects.Lookup((uint16_t)quest->GetObj(), obj);
            }
            CUnit* target = (CUnit*)obj;
            if (found != 0) {
                if ((target->unitFlags & (UNIT_FLAG_PLAYER_CLASS | UNIT_FLAG_CREATURE)) != 0) {
                    char temp_path[0x100];
                    char fname[0x100];
                    char full_path[0x100];
                    GetTempPathA(0x100, temp_path);
                    sprintf(fname, "allods-2-%d.$$$", target->unit_id);
                    sprintf(full_path, "%s%s", temp_path, fname);
                    if (strcmp(this->picture_name, fname) != 0) {
                        strcpy(this->picture_name, fname);
                        if ((target->unitFlags & UNIT_FLAG_EQUIP_CHANGED) != 0) {
                            UnlockSurface2();
                            target->RenderHeroPortrait(full_path, this->portrait_bmp, this->hitmap);
                            LockSurface2();
                        } else {
                            this->portrait_bmp->LoadFile(fname, this->hitmap);
                        }
                    } else if ((target->unitFlags & UNIT_FLAG_EQUIP_CHANGED) != 0) {
                        UnlockSurface2();
                        target->RenderHeroPortrait(full_path, this->portrait_bmp, this->hitmap);
                        LockSurface2();
                    }
                    unit_name = txt_npcnames.GetLine(target->serverId - 1);
                } else {
                    char suffix[0x50];
                    strcpy(picture, g_VFX_info[target->typeId]->info_picture);
                    sprintf(suffix, "%d", target->face);
                    if (target->face > 1) {
                        strcat(picture, suffix);
                    }
                    if (strcmp(this->picture_name, picture) != 0) {
                        strcpy(this->picture_name, picture);
                        char path[0x100];
                        sprintf(path, "graphics\\infowindow\\%s.bmp", this->picture_name);
                        this->portrait_bmp->LoadFile(path, nullptr);
                        memset(this->hitmap->GetData(), 0, this->hitmap->GetWidth() * this->hitmap->GetHeight());
                    }
                    if (obj->typeId >= 0x52 && obj->typeId <= 0x66) {
                        unit_name.Format("%s", txt_unitname.GetLine(obj->typeId));
                    } else {
                        unit_name.Format("%s[%d]", txt_unitname.GetLine(obj->typeId), target->face);
                    }
                }
                this->portrait_bmp->DrawSubRectMasked(screen_rect.left + 0xB, screen_rect.top + 0xF0, 0, 0, 0xA0, 0xF0);
            }
            break;
        }
        case 2: {
            char suffix[0x50];
            strcpy(picture, g_VFX_info[quest->GetObj() & 0xFF]->info_picture);
            sprintf(suffix, "%d", quest->GetObj() >> 8);
            if ((quest->GetObj() >> 8) > 1) {
                strcat(picture, suffix);
            }
            if ((quest->GetObj() & 0xFF) >= 0x52 && (quest->GetObj() & 0xFF) <= 0x66) {
                unit_name.Format("%s", txt_unitname.GetLine(quest->GetObj() & 0xFF));
            } else {
                unit_name.Format("%s[%d]", txt_unitname.GetLine(quest->GetObj() & 0xFF), quest->GetObj() >> 8);
            }
            if (picture[0] != 0) {
                if (strcmp(this->picture_name, picture) != 0) {
                    strcpy(this->picture_name, picture);
                    char path[0x100];
                    sprintf(path, "graphics\\infowindow\\%s.bmp", this->picture_name);
                    this->portrait_bmp->LoadFile(path, nullptr);
                    memset(this->hitmap->GetData(), 0, this->hitmap->GetWidth() * this->hitmap->GetHeight());
                }
                this->portrait_bmp->DrawSubRectMasked(screen_rect.left + 0xB, screen_rect.top + 0xF0, 0, 0, 0xA0, 0xF0);
            }
            break;
        }
        default:
            break;
        }

        CString area_name;
        CString building_name;
        CGameObject* landmark;
        if (main_wnd->vis_map_context->objects.Lookup((uint16_t)quest->GetLandmarkId(), landmark)) {
            building_name = txt_building.GetLine(landmark->typeId - 1);
            int32_t cell_x = ((landmark->tileX - 8) * 5) / (main_wnd->vis_map_context->map_width - 0x10);
            int32_t cell_y = ((landmark->tileY - 8) * 5) / (main_wnd->vis_map_context->map_height - 0x10);
            area_name = TxtFile::AllLines[cell_x + 0x13D + cell_y * 5];
        }

        CString quest_text;
        switch (quest->Kind()) {
        case 1:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], (const char*)unit_name, (const char*)area_name, (const char*)building_name);
            break;
        case 2:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->GetLimit(), (const char*)unit_name);
            break;
        case 3:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], (const char*)area_name, (const char*)building_name);
            break;
        case 4:
        case 5:
        case 0xD:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], (const char*)area_name);
            break;
        case 6:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->GetLimit() / 0x3C0, (quest->GetLimit() >> 4) % 0x3C, (const char*)area_name);
            break;
        case 8:
        case 9:
        case 0xA:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->GetLimit());
            break;
        case 0xB:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], (const char*)unit_name);
            break;
        case 0xC:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C]);
            break;
        default:
            break;
        }

        CRect text_rect = CRect(CPoint(0x12, 0x32), CSize(0x84, 0xEE)) + screen_rect.TopLeft();
        g_font2->DrawTextJustifyInRectShadow(text_rect, quest_text, clrsh_DullGold, 0xA);
    } else {
        if (this->vis_tav->rewards.GetSize() != 0 && this->vis_tav->rewards[this->vis_tav->quest_id]->item_id == 0xFFFD) {
            char picture[0x100];
            picture[0] = 0;
            char suffix[0x50];
            strcpy(picture, g_VFX_info[this->vis_tav->rewards[this->vis_tav->quest_id]->count & 0xFF]->info_picture);
            sprintf(suffix, "%d", (int32_t)this->vis_tav->rewards[this->vis_tav->quest_id]->count >> 8);
            if ((int32_t)this->vis_tav->rewards[this->vis_tav->quest_id]->count >> 8 > 1) {
                strcat(picture, suffix);
            }
            if (picture[0] != 0) {
                if (strcmp(this->picture_name, picture) != 0) {
                    strcpy(this->picture_name, picture);
                    char path[0x100];
                    sprintf(path, "graphics\\infowindow\\%s.bmp", this->picture_name);
                    this->portrait_bmp->LoadFile(path, nullptr);
                    memset(this->hitmap->GetData(), 0, this->hitmap->GetWidth() * this->hitmap->GetHeight());
                }
                this->portrait_bmp->DrawSubRectMasked(screen_rect.left + 0xB, screen_rect.top + 0xF0, 0, 0, 0xA0, 0xF0);
            }
        }
    }

    UnlockSurface2();
}


// 497E06
void VisTavLeftPanel::SelectRosterEntry(int32_t idx) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        this->vis_tav->selection_index = idx;
        this->vis_tav->right_panel->UpdateButtonLabels();
    } else {
        this->vis_tav->quest_id = idx;
    }

    CSound::Play(this->vis_tav->sounds[9]);
}


// 49A30A
// client: 49e32a
int32_t VisTavRightPanel::OnLButtonUp(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (this->pressed_btn >= 0 && this->pressed_btn < 3 && this->HitTestButton(pos) == this->pressed_btn) {
        int32_t button = this->pressed_btn;
        this->pressed_btn = -1;
        this->UpdateHoveredButton(wparam, pos);

        if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
            if (button == 0) {
                if (this->vis_tav->selection_index < this->vis_tav->avail_entries.GetSize()) {
                    CUnit* unit = this->vis_tav->avail_entries[this->vis_tav->selection_index];
                    if (this->vis_tav->entrie_id[this->vis_tav->FindEntryIndexForUnit(unit)] & 0x80000000) {
                        this->vis_tav->RetalkAvailNpc();
                    } else {
                        this->vis_tav->ShowHireResponseDialog();
                    }
                    this->UpdateButtonLabels();
                }
            } else if (button == 1) {
                if (this->vis_tav->selection_index < this->vis_tav->avail_entries.GetSize()) {
                    if (this->vis_tav->selection_index >= 0) {
                        CUnit* unit = this->vis_tav->avail_entries[this->vis_tav->selection_index];
                        CString str;
                        str.Format("npc%dabout", unit->serverId);
                        ShowRoleKeyDialog(str);
                        CSound::Play(this->vis_tav->sounds[12]);
                    }
                } else {
                    this->vis_tav->ShowReservedNpcTalk();
                }
            } else if (button == 2) {
                this->vis_tav->CloseDialog();
            }
        } else {
            if (button == 0) {
                if (main_wnd->vis_map_context->quest_map->OnEvent(0x11, main_wnd->vis_map_context->my_main_unit->index, this->vis_tav->inn_id) != 0) {
                    this->vis_tav->quest_alt_close = (this->vis_tav->quest_alt_close == 0);
                } else {
                    if (this->vis_tav->quest_map->GetQuestCount() != 0 || this->vis_tav->rewards.GetSize() != 0) {
                        if (this->vis_tav->quest_selection == this->vis_tav->quest_id) {
                            this->vis_tav->quest_selection = -1;
                        } else {
                            this->vis_tav->quest_selection = this->vis_tav->quest_id;
                        }
                    }
                }
            } else if (button == 1) {
                if (this->vis_tav->quest_map->GetQuestCount() != 0) {
                    Quest* quest;
                    if (this->vis_tav->quest_map->FindQuest(this->vis_tav->quest_id, &quest) != 0) {
                        CString quest_str;
                        quest_str.Format("quest%d", quest->Kind());
                        ShowRoleKeyDialog(quest_str);
                    }
                    CSound::Play(this->vis_tav->sounds[12]);
                } else {
                    if (this->vis_tav->rewards.GetSize() != 0) {
                        TokenEntry* reward = this->vis_tav->rewards[this->vis_tav->quest_id];
                        if (reward->item_id == 0xFFFD) {
                            ShowRoleKeyDialog("treasureally");
                        } else if (reward->item_id == 0xFFFE) {
                            ShowRoleKeyDialog("treasurexp");
                        } else if (reward->item_id == 0xFFFF) {
                            ShowRoleKeyDialog("treasuremoney");
                        } else if ((uint32_t)this->vis_tav->quest_id < (uint32_t)(this->vis_tav->rewards.GetSize() - 1)) {
                            ShowRoleKeyDialog("treasureitem");
                        } else {
                            ShowRoleKeyDialog("treasureenchant");
                        }
                        CSound::Play(this->vis_tav->sounds[12]);
                    }
                }
            } else if (button == 2) {
                this->vis_tav->CloseDialog();
            }
        }
    }

    this->pressed_btn = -1;
    this->UpdateHoveredButton(wparam, pos);
    return 1;
}


// 499A67
void VisTavRightPanel::LoadRightPanelBmps() {
    this->FreeRightPanelBmps();
    this->bmp_btn_on[0] = new CBmp64("graphics\\interface\\Inn\\button1on.bmp");
    g_mousept.Update();
    this->bmp_btn_on[1] = new CBmp64("graphics\\interface\\Inn\\button2on.bmp");
    g_mousept.Update();
    this->bmp_btn_on[2] = new CBmp64("graphics\\interface\\Inn\\button3on.bmp");
    g_mousept.Update();
    this->bmp_btn_off[0] = new CBmp64("graphics\\interface\\Inn\\button1off.bmp");
    g_mousept.Update();
    this->bmp_btn_off[1] = new CBmp64("graphics\\interface\\Inn\\button2off.bmp");
    g_mousept.Update();
    this->bmp_btn_off[2] = new CBmp64("graphics\\interface\\Inn\\button3off.bmp");
    g_mousept.Update();
    this->bmp_area = new CBmp64("graphics\\interface\\Inn\\ButtonsArea.bmp");
    g_mousept.Update();
}


// 49A8FA
void VisTavRightPanel::UpdateHoveredButton(uint32_t wparam, CPoint pos) {
    int32_t index = this->HitTestButton(pos);
    if (index < 0 || (wparam & 1) != 0) {
        if (index < 0 || index != this->pressed_btn || (wparam & 1) == 0) {
            this->hovered_btn = -1;
        } else {
            this->hovered_btn = index;
        }
    } else {
        this->hovered_btn = index;
    }
}


// 49A873
int32_t VisTavRightPanel::HitTestButton(CPoint pos) {
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    CPoint pt(pos.x - tav_topleft.x, pos.y - tav_topleft.y);

    for (int32_t i = 0; i < 3; i++) {
        if (this->btn_rects[i].PtInRect(pt)) {
            return i;
        }
    }
    return -1;
}


// 4998C2
void VisTavRightPanel::InitRightPanel() {
    this->pressed_btn = -1;
    this->hovered_btn = -1;
    this->btn_rects[0] = CRect(0x1E4, 0x2C, 0x270, 0x5A);
    this->btn_rects[1] = CRect(0x1E4, 0x5B, 0x270, 0x89);
    this->btn_rects[2] = CRect(0x1E4, 0x8A, 0x270, 0xB8);
    for (int32_t i = 0; i < 3; i++) {
        this->bmp_btn_on[i] = nullptr;
        this->bmp_btn_off[i] = nullptr;
    }
    this->bmp_area = nullptr;

    this->texts.SetSize(3, -1);
    this->texts[0] = TxtFile::AllLines[0xF3];
    this->texts[1] = TxtFile::AllLines[0xF2];
    this->texts[2] = TxtFile::AllLines[0xE8];
    this->flags |= 2;
}


// 4997C6
VisTavRightPanel::VisTavRightPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->vis_tav = tav;
    this->InitRightPanel();
}


// 4A3E40
VisTavRightPanel::~VisTavRightPanel() {
    this->FreeRightPanelBmps();
}


// 4A4710
const char* VisTavRightPanel::GetHint() {
    return nullptr;
}


// 49A84E
void VisTavRightPanel::ResetButtonStates() {
    this->pressed_btn = -1;
    this->hovered_btn = -1;
}


// 49A26E
int32_t VisTavRightPanel::OnMouseMove(uint32_t wparam, CPoint pos) {
    this->UpdateHoveredButton(wparam, pos);
    return 0;
}


// 49A2E6
int32_t VisTavRightPanel::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    return this->OnLButtonDown(wparam, pos);
}


// 49A291
int32_t VisTavRightPanel::OnLButtonDown(uint32_t wparam, CPoint pos) {
    this->pressed_btn = this->HitTestButton(pos);
    if (this->pressed_btn == 2) {
        CSound::Play(this->vis_tav->sounds[11]);
    }
    return 1;
}


// 499CDF
void VisTavRightPanel::FreeRightPanelBmps() {
    for (int32_t i = 0; i < 3; i++) {
        if (this->bmp_btn_on[i] != nullptr) {
            delete this->bmp_btn_on[i];
        }
        this->bmp_btn_on[i] = nullptr;
        if (this->bmp_btn_off[i] != nullptr) {
            delete this->bmp_btn_off[i];
        }
        this->bmp_btn_off[i] = nullptr;
    }

    if (this->bmp_area != nullptr) {
        delete this->bmp_area;
    }
    this->bmp_area = nullptr;
}


// 49A973
void VisTavRightPanel::UpdateButtonLabels() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        if (this->vis_tav->avail_entries.GetUpperBound() < this->vis_tav->selection_index || this->vis_tav->selection_index == -1) {
            this->texts[0] = "";
        } else {
            CUnit* unit = this->vis_tav->avail_entries[this->vis_tav->selection_index];
            if (this->vis_tav->entrie_id[this->vis_tav->FindEntryIndexForUnit(unit)] & 0x80000000) {
                this->texts[0] = TxtFile::AllLines[0x103];
            } else {
                this->texts[0] = TxtFile::AllLines[0x102];
            }
        }
        this->texts[1] = TxtFile::AllLines[0xF2];
    } else {
        if (this->vis_tav->quest_map->GetQuestCount() != 0) {
            this->texts[0] = TxtFile::AllLines[0x15F];
            this->texts[1] = TxtFile::AllLines[0x160];
        } else {
            if (this->vis_tav->rewards.GetSize() != 0) {
                this->texts[0] = TxtFile::AllLines[0x164];
                this->texts[1] = TxtFile::AllLines[0x165];
            } else {
                if (main_wnd->vis_map_context->quest_map->OnEvent(0x11, main_wnd->vis_map_context->my_main_unit->index, this->vis_tav->inn_id) != 0) {
                    this->texts[0] = txt_patch.GetLine(0x60);
                    this->texts[1] = " ";
                } else {
                    this->texts[0] = " ";
                    this->texts[1] = " ";
                }
            }
        }
    }
}


// 499DFA
void VisTavRightPanel::VMethod7() {
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    if (this->vis_tav->dialog_active == 0) {
        return;
    }

    LockSurface2();
    this->bmp_area->Draw(tav_topleft.x + this->rect.left, tav_topleft.y + this->rect.top, 0, 0, 0);

    uint16_t* pal;
    if (this->hovered_btn == 1) {
        pal = palette_paris_daisy->GetPalette(0);
    } else {
        pal = palette_husk->GetPalette(0);
    }

    if (this->hovered_btn < 0 || this->pressed_btn != this->hovered_btn || this->hovered_btn != 1) {
        this->bmp_btn_off[1]->Draw(tav_topleft.x + this->btn_rects[1].left, tav_topleft.y + this->btn_rects[1].top, 0, 0, 0);
        g_font4->DrawTxt(tav_topleft.x + this->btn_rects[1].left + this->btn_rects[1].Width() / 2, tav_topleft.y + this->btn_rects[1].top + this->btn_rects[1].Height() / 2, this->texts[1], 10, pal);
    } else {
        this->bmp_btn_on[1]->Draw(tav_topleft.x + this->btn_rects[1].left, tav_topleft.y + this->btn_rects[1].top, 0, 0, 0);
        g_font4->DrawTxt(tav_topleft.x + this->btn_rects[1].left + this->btn_rects[1].Width() / 2, tav_topleft.y + this->btn_rects[1].top + 1 + this->btn_rects[1].Height() / 2, this->texts[1], 10, pal);
    }

    for (int32_t i = 0; i < 3; i++) {
        if (i == 1) {
            continue;
        }
        if (this->hovered_btn == i) {
            pal = palette_paris_daisy->GetPalette(0);
        } else {
            pal = palette_husk->GetPalette(0);
        }
        if (this->hovered_btn < 0 || this->pressed_btn != this->hovered_btn || this->hovered_btn != i) {
            this->bmp_btn_off[i]->Draw(tav_topleft.x + this->btn_rects[i].left, tav_topleft.y + this->btn_rects[i].top, 0, 0, 0);
            g_font4->DrawTxt(tav_topleft.x + this->btn_rects[i].left + this->btn_rects[i].Width() / 2, tav_topleft.y + this->btn_rects[i].top + this->btn_rects[i].Height() / 2, this->texts[i], 10, pal);
        } else {
            this->bmp_btn_on[i]->Draw(tav_topleft.x + this->btn_rects[i].left, tav_topleft.y + this->btn_rects[i].top, 0, 0, 0);
            g_font4->DrawTxt(tav_topleft.x + this->btn_rects[i].left + this->btn_rects[i].Width() / 2, tav_topleft.y + this->btn_rects[i].top + 1 + this->btn_rects[i].Height() / 2, this->texts[i], 10, pal);
        }
    }

    UnlockSurface2();
}


// 4010EE
void VisTavSceneAnim::LoadAnimFrames(CStringArray* names) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    this->frames.SetSize(names->GetSize(), -1);
    for (int32_t i = 0; i < this->frames.GetSize(); i++) {
        this->frames[i] = new CBmp64(names->GetAt(i));
        if (main_wnd->music_update_proc != nullptr) {
            main_wnd->music_update_proc();
        }
        g_mousept.Update();
    }

    this->frame_idx = 0;
    this->current_frame = this->frames[this->frame_idx];
}


// 4014F2
void VisTavSceneAnim::FreeAnimFrames() {
    if (this->frames.GetSize() == 0 && this->current_frame != nullptr) {
        delete this->current_frame;
    }
    this->current_frame = nullptr;

    for (int32_t i = 0; i < this->frames.GetSize(); i++) {
        if (this->frames[i] != nullptr) {
            delete this->frames[i];
        }
    }
    this->frames.RemoveAll();
    this->frame_idx = -1;
}


// 4015C6
bool VisTavSceneAnim::StepForward() {
    if (this->frame_idx == this->frames.GetUpperBound()) {
        return false;
    }
    this->frame_idx++;
    this->current_frame = this->frames.GetAt(this->frame_idx);
    return this->current_frame != nullptr;
}


// 401613
bool VisTavSceneAnim::StepBackward() {
    if (this->frame_idx == 0) {
        return false;
    }
    this->frame_idx--;
    this->current_frame = this->frames.GetAt(this->frame_idx);
    return this->current_frame != nullptr;
}


// 401659
void VisTavSceneAnim::NextFrame() {
    this->frame_idx = (this->frame_idx + 1) % this->frames.GetUpperBound();
    this->current_frame = this->frames.GetAt(this->frame_idx);
}


// 4016EF
void VisTavSceneAnim::Draw(int32_t x, int32_t y) {
    this->current_frame->Draw(x, y, 0, 0, 0);
}

// 49B22F
void VisTavScene::VMethod7() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (!tavscene_statics_inited) {
        tavscene_statics_inited = true;
        tavscene_anim_delay = 3000 + rand() / 16;
        uint32_t t = timeGetTime();
        tavscene_frame_ts = t;
        tavscene_anims_ts = t;
        tavscene_state_ts = t;
        tavscene_ambient_ts = t;
    }

    uint32_t now = timeGetTime();
    if (now - tavscene_ambient_ts > 10000) {
        CSound::Play(this->vis_tav->sounds[2]);
        tavscene_ambient_ts = now;
    }

    LockSurface2();
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    int32_t x = tav_topleft.x + this->rect.left;
    int32_t y = tav_topleft.y + this->rect.top;

    this->bmp_background->Draw(x, y, 0, 0, 0);

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        this->anims[0].Draw(x, y + 0x30);
        this->anims[1].Draw(x + 0x104, y + 0xA0);
        if (now - tavscene_anims_ts > 100) {
            this->anims[0].NextFrame();
            this->anims[1].NextFrame();
            tavscene_anims_ts = now;
        }

        if (now - tavscene_state_ts > tavscene_anim_delay) {
            if ((tavscene_anim_delay & 1) == 0) {
                tavscene_anim_state = 2;
                CSound::Play(this->vis_tav->sounds[4]);
                CSound::Play(this->vis_tav->sounds[10]);
            } else {
                tavscene_anim_substate = 1;
                tavscene_anim_state = 1;
            }
        }

        if (tavscene_anim_state == 1) {
            this->anims[3].Draw(x + 0x50, y + 0x98);
            if (this->anims[3].frame_idx == 0x1E) {
                CSound::Play(this->vis_tav->sounds[0]);
            }
        } else if (tavscene_anim_state == 2) {
            this->anims[2].Draw(x + 0x50, y + 0x98);
        }

        if (tavscene_anim_state != 0 && now - tavscene_state_ts > 0x53) {
            if (tavscene_anim_state == 1) {
                if (tavscene_anim_substate == -1) {
                    if (!this->anims[3].StepBackward()) {
                        tavscene_anim_substate = 0;
                        tavscene_anim_state = 0;
                        tavscene_anim_delay = 3000 + rand() / 16;
                        CSound::Play(this->vis_tav->sounds[1]);
                    }
                } else if (tavscene_anim_substate == 1) {
                    if (!this->anims[3].StepForward()) {
                        tavscene_anim_substate = -1;
                        this->anims[3].StepBackward();
                    }
                }
                tavscene_state_ts = now;
            } else if (tavscene_anim_state == 2) {
                if (!this->anims[2].StepForward()) {
                    tavscene_anim_state = 0;
                    tavscene_anim_delay = 3000 + rand() / 16;
                    this->anims[2].frame_idx = 0;
                }
                tavscene_state_ts = now;
            }
        }

        // cell rects are tavern-space (InitSceneRects positions them from this->rect), and the
        // epilogue mask offsets are tavern-space constants: the binary draws both from raw
        // tav_topleft, without this->rect (only bg/anims use it).
        int32_t avail_count = this->vis_tav->avail_entries.GetSize();
        for (int32_t i = 0; i < avail_count; i++) {
            int32_t row = i / 6;
            int32_t col = i % 6;
            CPoint pt = this->roster_rects[row][col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            this->bmp_man_back->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            CSprite256* sprite = this->avail_sprites[i];
            sprite->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, this->avail_frames[i], 0, 0);
            if (this->vis_tav->selection_index == i && now - tavscene_frame_ts > 0x7D) {
                this->avail_frames[i] = (this->avail_frames[i] + 1) % sprite->GetFrameCount();
                tavscene_frame_ts = now;
            }

            CUnit* unit = this->vis_tav->avail_entries[i];
            int32_t entry = this->vis_tav->FindEntryIndexForUnit(unit);
            if (this->vis_tav->entrie_id[entry] & 0x80000000) {
                CRect& r = this->roster_rects[row][col];
                g_font2->DrawTextWithShadow(tav_topleft.x + r.left + r.Width() / 2, tav_topleft.y + r.top + r.Height() / 2, TxtFile::AllLines[0x101], 10, clrsh_CoralRed, 1);
            }
        }

        for (int32_t i = 0; i < this->vis_tav->reserved_entries.GetSize(); i++) {
            int32_t row = (avail_count + i) / 6;
            int32_t col = (avail_count + i) % 6;
            CPoint pt = this->roster_rects[row][col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            if (this->vis_tav->selection_index == avail_count + i && now - tavscene_frame_ts > 0x7D) {
                this->reserved_frames[i] = (this->reserved_frames[i] + 1) % this->reserved_sprites[i]->GetFrameCount();
                tavscene_frame_ts = now;
            }
            this->bmp_man_back_talk->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            this->reserved_sprites[i]->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, this->reserved_frames[i], 0, 0);
        }
    } else {
        this->DrawQuestRoster();
    }

    this->bmp_lu_over->DrawSubRectMasked(tav_topleft.x + 0xA0, tav_topleft.y, 0, 0, 0x10, 0xEE);
    this->bmp_ld_over->DrawSubRectMasked(tav_topleft.x + 0xA0, tav_topleft.y + 0xEE, 0, 0, 0x10, 0xF2);
    this->bmp_ru_over->DrawSubRectMasked(tav_topleft.x + 0x1D0, tav_topleft.y, 0, 0, 0x10, 0xEE);
    if (this->vis_tav->info_panel->info_mode) {
        g_bmp_humanbackl->DrawSubRectMasked(tav_topleft.x + 0x1D0, tav_topleft.y + 0xEE, 0, 0, 0x10, 0xF2);
    } else {
        g_bmp_textbackl->DrawSubRectMasked(tav_topleft.x + 0x1D0, tav_topleft.y + 0xEE, 0, 0, 0x10, 0xF2);
    }
    UnlockSurface2();
    CVisualObject::VMethod7();
}


// 49D5F9
void VisTavScene::VMethod29() {
    delete this->bmp_man_back;
    this->bmp_man_back = nullptr;
    delete this->bmp_man_back_talk;
    this->bmp_man_back_talk = nullptr;
    delete this->bmp_background;
    this->bmp_background = nullptr;
    delete this->bmp_lu_over;
    this->bmp_lu_over = nullptr;
    delete this->bmp_ld_over;
    this->bmp_ld_over = nullptr;
    delete this->bmp_ru_over;
    this->bmp_ru_over = nullptr;

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != SESSION_LOCAL_BATTLE) {
        for (int32_t i = 0; i < 13; i++) {
            delete this->quest_bmps[i];
            this->quest_bmps[i] = nullptr;
        }
    }

    for (int32_t i = 0; i < this->roster_sprites.GetSize(); i++) {
        if (this->roster_sprites[i] != nullptr) {
            delete this->roster_sprites[i];
            this->roster_sprites[i] = nullptr;
        }
    }
    this->roster_sprites.RemoveAll();
}


// 49CED4
void VisTavScene::VMethod26() {
    this->avail_sprites.SetSize(this->vis_tav->avail_entries.GetSize(), -1);
    this->avail_frames.SetSize(this->vis_tav->avail_entries.GetSize(), -1);
    this->reserved_sprites.SetSize(this->vis_tav->reserved_entries.GetSize(), -1);
    this->reserved_frames.SetSize(this->vis_tav->reserved_entries.GetSize(), -1);

    CString name;
    for (int32_t i = 0; i < this->vis_tav->avail_entries.GetSize(); i++) {
        CUnit* unit = this->vis_tav->avail_entries[i];
        name.Format("graphics\\interface\\inn\\Unit%d\\sprites.16a", unit->typeId);
        this->avail_sprites[i] = new CA16(name);
        this->avail_sprites[i]->ResetPalette(0x10, 4, 0);
    }

    for (int32_t i = 0; i < this->vis_tav->reserved_entries.GetSize(); i++) {
        CUnit* unit = this->vis_tav->reserved_entries[i];
        if (unit->unitFlags & UNIT_FLAG_PLAYER_CLASS) {
            if (unit->unitFlags & UNIT_FLAG_MAGE) {
                name.Format("graphics\\interface\\inn\\HeroMage\\sprites.16a");
            } else {
                name.Format("graphics\\interface\\inn\\HeroFighter\\sprites.16a");
            }
        } else {
            name.Format("graphics\\interface\\inn\\Unit%d\\sprites.16a", unit->typeId);
        }
        this->reserved_sprites[i] = new CA16(name);
        this->reserved_sprites[i]->ResetPalette(0x10, 4, 0);
    }
}


// 49D2BB
void VisTavScene::VMethod28() {
    this->VMethod29();
    this->bmp_man_back = new CBmp64("graphics\\Interface\\Inn\\manback.bmp");
    g_mousept.Update();
    this->bmp_man_back_talk = new CBmp64("graphics\\Interface\\Inn\\ManBackTalk.bmp");
    g_mousept.Update();
    this->bmp_background = new CBmp64("graphics\\Interface\\Inn\\CenterArea.bmp");
    g_mousept.Update();
    this->bmp_lu_over = new CBmp64("graphics\\interface\\inn\\LUOver.bmp");
    g_mousept.Update();
    this->bmp_ld_over = new CBmp64("graphics\\interface\\inn\\LDOver.bmp");
    g_mousept.Update();
    this->bmp_ru_over = new CBmp64("graphics\\interface\\inn\\RUOver.bmp");
    g_mousept.Update();

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != SESSION_LOCAL_BATTLE) {
        CString name;
        for (int32_t i = 0; i < 13; i++) {
            name.Format("graphics\\interface\\inn\\quests\\%02d.bmp", i + 1);
            this->quest_bmps[i] = new CBmp64(name);
        }
    }
}


// 49CD2B
const char* VisTavScene::GetHint() {
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    CPoint pt = CPoint(g_mousept.GetX(), g_mousept.GetY()) - tav_topleft;

    for (int32_t i = 0; i < this->vis_tav->rewards.GetSize(); i++) {
        int32_t row = i / 3;
        int32_t col = i % 3;
        if (this->party_rects[row][col].PtInRect(pt)) {
            TokenEntry* entry = this->vis_tav->rewards[i];
            if (entry->item_id == 0xFFFD) {
                return TxtFile::AllLines[0x166];
            }
            if (entry->item_id == 0xFFFE) {
                return TxtFile::AllLines[0x2E];
            }
            if (entry->item_id == 0xFFFF) {
                return TxtFile::AllLines[0x4A];
            }
            return entry->GetHintText();
        }
    }
    return nullptr;
}


// 49D8E8
int32_t VisTavScene::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (main_wnd->sessionMode != SESSION_LOCAL_BATTLE) {
        if (main_wnd->vis_map_context->quest_map->OnEvent(0x11, main_wnd->vis_map_context->my_main_unit->index, this->vis_tav->inn_id) != 0) {
            this->vis_tav->quest_alt_close = (this->vis_tav->quest_alt_close == 0) ? 1 : 0;
            return 1;
        }
    }

    if (this->OnLButtonDown(wparam, pos) == 0) {
        return 0;
    }

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        if (this->vis_tav->selection_index < this->vis_tav->avail_entries.GetSize()) {
            CUnit* unit = this->vis_tav->avail_entries[this->vis_tav->selection_index];
            int32_t entry = this->vis_tav->FindEntryIndexForUnit(unit);
            if (this->vis_tav->entrie_id[entry] & 0x80000000) {
                this->vis_tav->RetalkAvailNpc();
            } else {
                this->vis_tav->ShowHireResponseDialog();
            }
            this->vis_tav->right_panel->UpdateButtonLabels();
        } else {
            this->vis_tav->ShowReservedNpcTalk();
        }
    } else {
        if (this->vis_tav->quest_selection != this->vis_tav->quest_id) {
            this->vis_tav->quest_selection = this->vis_tav->quest_id;
        } else {
            this->vis_tav->quest_selection = -1;
        }
    }
    return 1;
}


// 49D1B0
void VisTavScene::VMethod27() {
    for (int32_t i = 0; i < this->avail_sprites.GetSize(); i++) {
        if (this->avail_sprites[i] != nullptr) {
            delete this->avail_sprites[i];
        }
    }
    this->avail_sprites.RemoveAll();
    this->avail_frames.RemoveAll();

    for (int32_t i = 0; i < this->reserved_sprites.GetSize(); i++) {
        if (this->reserved_sprites[i] != nullptr) {
            delete this->reserved_sprites[i];
        }
    }
    this->reserved_sprites.RemoveAll();
    this->reserved_frames.RemoveAll();
}


// 49DA98
int32_t VisTavScene::OnLButtonDown(uint32_t wparam, CPoint pos) {
    int32_t idx = this->HitTestRoster(&pos);
    if (idx == -1) {
        return 0;
    }
    this->vis_tav->left_panel->SelectRosterEntry(idx);
    return 1;
}


// 49D8D9
int32_t VisTavScene::OnMouseMove(uint32_t wparam, CPoint pos) {
    return 0;
}


// 4A3E70
VisTavScene::~VisTavScene() {
    this->VMethod29();
}


// 49AD56
VisTavScene::VisTavScene(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->vis_tav = tav;
    this->InitSceneRects();
}


// 49BC23
void VisTavScene::DrawQuestRoster() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();

    int32_t quest_found = main_wnd->vis_map_context->quest_map->OnEvent(0x11, main_wnd->vis_map_context->my_main_unit->index, this->vis_tav->inn_id);
    if (quest_found != 0) {
        Quest* quest = nullptr;
        main_wnd->vis_map_context->quest_map->FindQuest(quest_found, &quest);
        CPoint pt = this->roster_rects[0][0].TopLeft();
        int32_t kind = quest->Kind();
        this->quest_bmps[kind - 1]->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
        if (this->vis_tav->quest_alt_close == 0) {
            CRect& r = this->roster_rects[0][0];
            g_font2->DrawTextWithShadow(tav_topleft.x + r.left + r.Width() / 2, tav_topleft.y + r.top + r.Height() / 2, TxtFile::AllLines[0x158], 10, clrsh_CoralRed, 1);
        }
        return;
    }

    if (this->vis_tav->quest_map->GetQuestCount() != 0) {
        POSITION it = this->vis_tav->quest_map->quests_map.GetStartPosition();
        int32_t i = 0;
        while (it != nullptr) {
            uint32_t key;
            Quest* quest;
            this->vis_tav->quest_map->quests_map.GetNextAssoc(it, key, quest);
            int32_t row = i / 6;
            int32_t col = i % 6;
            CPoint pt = this->roster_rects[row][col].TopLeft();
            int32_t kind = quest->Kind();
            this->quest_bmps[kind - 1]->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            if (this->vis_tav->quest_selection == key) {
                CRect& r = this->roster_rects[row][col];
                g_font2->DrawTextWithShadow(tav_topleft.x + r.left + r.Width() / 2, tav_topleft.y + r.top + r.Height() / 2, TxtFile::AllLines[0x158], 10, clrsh_CoralRed, 1);
            }
            i++;
        }
        return;
    }

    for (int32_t i = 0; i < this->vis_tav->rewards.GetSize(); i++) {
        int32_t row = i / 3;
        int32_t col = i % 3;
        CPoint pt = this->party_rects[row][col].TopLeft();
        g_bmp_backinv->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
        if (this->vis_tav->quest_id == i) {
            FillRectBrightness(tav_topleft.x + pt.x, tav_topleft.y + pt.y, tav_topleft.x + pt.x + 0x50, tav_topleft.y + pt.y + 0x50, 2);
        }
        if (this->roster_sprites.GetSize() <= i) {
            this->roster_sprites.SetSize(i + 1, -1);
        }

        TokenEntry* entry = this->vis_tav->rewards[i];
        if (entry->item_id == 0xFFFD) {
            g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 0x28, tav_topleft.y + pt.y + 0x23, TxtFile::AllLines[0x166], 2, clrsh_DullGold, 1);
        } else if (entry->item_id == 0xFFFE) {
            g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 0x28, tav_topleft.y + pt.y + 0x23, TxtFile::AllLines[0x2E], 2, clrsh_DullGold, 1);
            CString text;
            text.Format("%d", entry->count * 250);
            InsertThousandsSeparators(&text);
            g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 3, tav_topleft.y + pt.y + 0x42, text, 0, clrsh_DullGold, 1);
        } else if (entry->item_id == 0xFFFF) {
            g_ca16_money->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            CString text;
            text.Format("%d", entry->count * 250);
            InsertThousandsSeparators(&text);
            g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 3, tav_topleft.y + pt.y + 0x42, text, 0, clrsh_DullGold, 1);
        } else {
            if (this->roster_sprites[i] == nullptr) {
                if (this->roster_sprites.GetSize() <= i) {
                    this->roster_sprites.SetSize(i + 1, -1);
                }
                CString path = "graphics\\inventory\\" + entry->GetSpriteName() + ".16a";
                this->roster_sprites[i] = new CA16(path);
                this->roster_sprites[i]->ResetPalette(0x10, 4, 0);
            }
            this->roster_sprites[i]->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            if (entry->count > 1) {
                char buf[80];
                sprintf(buf, "%d", entry->count);
                g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 3, tav_topleft.y + pt.y + 0x42, buf, 0, clrsh_DullGold, 1);
            }

            if (entry->flg & 0x20) {
                uint32_t t = GetTickCount() / 120;
                int32_t cell_x = tav_topleft.x + pt.x;
                int32_t cell_y = tav_topleft.y + pt.y;
                static const uint32_t sparkle_alpha[7] = {0x3F, 0x7F, 0xBF, 0xFF, 0xBF, 0x7F, 0x3F};
                for (uint32_t k = 0; k < 7; k++) {
                    if (t >= k) {
                        PlotGlowDot(cell_x + main_wnd->vis_invtype1->random_offsets1[(t - k) & 0x3FF],
                                   cell_y + main_wnd->vis_invtype1->random_offsets2[(t - k) & 0x3FF],
                                   0xFF, 0, 0xFF, sparkle_alpha[k]);
                    }
                }
                main_wnd->vis_invtype1->DrawGlowTrail(cell_x, cell_y, i);
            }

            if (i > 1 && this->vis_tav->rewards[i - 1]->item_id >= 0xFFFD) {
                g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 0x28, tav_topleft.y + pt.y + 0x23, TxtFile::AllLines[0x161], 2, clrsh_DullGold, 1);
                g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 0x28, tav_topleft.y + pt.y + 0x2D, TxtFile::AllLines[0x162], 2, clrsh_DullGold, 1);
            }
        }

        if (this->vis_tav->quest_selection == i) {
            g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 0x28, tav_topleft.y + pt.y + 0xF, TxtFile::AllLines[0x163], 2, clrsh_TechBlack, 1);
        }
    }
}


// 49AF8C
void VisTavScene::InitSceneRects() {
    CPoint base(this->rect.left + 0x10, this->rect.bottom);

    for (int32_t i = 0; i < 3; i++) {
        for (int32_t j = 0; j < 6; j++) {
            CPoint pt = base + CPoint(j * 0x30, -(i + 1) * 0x40);
            this->roster_rects[i][j] = CRect(pt, CSize(0x30, 0x40));
        }
    }

    for (int32_t i = 0; i < 3; i++) {
        for (int32_t j = 0; j < 3; j++) {
            CPoint pt = base + CPoint(j * 0x50 + 0x18, -(i + 1) * 0x50);
            this->party_rects[i][j] = CRect(pt, CSize(0x50, 0x50));
        }
    }

    this->bmp_man_back = nullptr;
    this->bmp_man_back_talk = nullptr;
    this->bmp_background = nullptr;
    this->bmp_lu_over = nullptr;
    this->bmp_ld_over = nullptr;
    this->bmp_ru_over = nullptr;
    for (int32_t i = 0; i < 13; i++) {
        this->quest_bmps[i] = nullptr;
    }
}


// 49CAB8
int32_t VisTavScene::HitTestRoster(CPoint* pos) {
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    CPoint pt = *pos - tav_topleft;

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        int32_t count = this->vis_tav->avail_entries.GetSize() + this->vis_tav->reserved_entries.GetSize();
        for (int32_t i = 0; i < count; i++) {
            int32_t row = i / 6;
            int32_t col = i % 6;
            if (this->roster_rects[row][col].PtInRect(pt)) {
                return i;
            }
        }
        return -1;
    }

    if (this->vis_tav->quest_map->GetQuestCount() != 0) {
        POSITION it = this->vis_tav->quest_map->quests_map.GetStartPosition();
        int32_t i = 0;
        while (it != nullptr) {
            uint32_t key;
            Quest* quest;
            this->vis_tav->quest_map->quests_map.GetNextAssoc(it, key, quest);
            int32_t row = i / 6;
            int32_t col = i % 6;
            if (this->roster_rects[row][col].PtInRect(pt)) {
                return quest->GetSomeId();
            }
            i++;
        }
        return -1;
    }

    for (int32_t i = 0; i < this->vis_tav->rewards.GetSize(); i++) {
        int32_t row = i / 3;
        int32_t col = i % 3;
        if (this->party_rects[row][col].PtInRect(pt)) {
            return i;
        }
    }
    return -1;
}

// 4A0673
void VisTavSceneDruid::VMethod7() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisTavDruid* tav_druid = (VisTavDruid*)this->vis_tav;

    if (!tavscene_druid_statics_inited) {
        tavscene_druid_statics_inited = true;
        tavscene_druid_anim_delay = 3200 + rand() / 16;
        uint32_t t = timeGetTime();
        tavscene_druid_frame_ts = t;
        tavscene_druid_anims_ts = t;
        tavscene_druid_state_ts = t;
        tavscene_druid_ambient_ts = t;
    }

    uint32_t now = timeGetTime();
    LockSurface2();
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    int32_t x = tav_topleft.x + this->rect.left;
    int32_t y = tav_topleft.y + this->rect.top;

    this->bmp_background->Draw(x, y, 0, 0, 0);

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        this->anims_druid[0].Draw(x + 0xA8, y + 0x90);
        if (now - tavscene_druid_anims_ts > 100) {
            if (this->anims_druid[0].frame_idx == 0) {
                switch (GetRandS16(4) + 1) {
                case 1:
                    CSound::Play((CSound&)tav_druid->snd_druid[0]);
                    break;
                case 2:
                    CSound::Play((CSound&)tav_druid->snd_druid[1]);
                    break;
                case 3:
                    CSound::Play((CSound&)tav_druid->snd_druid[2]);
                    break;
                case 4:
                    CSound::Play((CSound&)tav_druid->snd_druid[3]);
                    break;
                }
            }
            this->anims_druid[0].NextFrame();
            tavscene_druid_anims_ts = now;
        }

        if (now - tavscene_druid_state_ts > tavscene_druid_anim_delay) {
            tavscene_druid_anim_state = (tavscene_druid_anim_delay & 3) + 1;
            if (tavscene_druid_anim_state == 4) {
                tavscene_druid_anim_state = 3;
            }
            if (tavscene_druid_anim_state == 1) {
                CSound::Play((CSound&)tav_druid->snd_druid[5]);
            }
            if (tavscene_druid_anim_state == 2) {
                CSound::Play((CSound&)tav_druid->snd_druid[4]);
            }
        }

        switch (tavscene_druid_anim_state) {
        case 1:
            this->anims_druid[1].Draw(x + 0x28, y + 0x80);
            if (this->anims_druid[1].frame_idx == 0x1E) {
                CSound::Play((CSound&)tav_druid->snd_druid[6]);
            }
            break;
        case 2:
            this->anims_druid[2].Draw(x + 0x28, y + 0x80);
            break;
        case 3:
        case 4:
            this->bmp_flash->Draw(x + 0x68, y + 0x98, 0, 0, 0);
            break;
        }

        if (tavscene_druid_anim_state != -1 && now - tavscene_druid_state_ts > 100) {
            switch (tavscene_druid_anim_state) {
            case 1:
                if (!this->anims_druid[1].StepForward()) {
                    tavscene_druid_anim_state = -1;
                    tavscene_druid_anim_delay = 3200 + rand() / 16;
                    this->anims_druid[1].frame_idx = 0;
                }
                tavscene_druid_state_ts = now;
                break;
            case 2:
                if (!this->anims_druid[2].StepForward()) {
                    tavscene_druid_anim_state = -1;
                    tavscene_druid_anim_delay = 3200 + rand() / 16;
                    this->anims_druid[2].frame_idx = 0;
                }
                tavscene_druid_state_ts = now;
                break;
            case 3:
                tavscene_druid_anim_state++;
                tavscene_druid_anim_delay = 1000 + rand() / 16;
                tavscene_druid_state_ts = now;
                break;
            case 4:
                tavscene_druid_anim_state = -1;
                break;
            }
        }

        // cell rects are tavern-space (InitSceneRects positions them from this->rect), and the
        // epilogue mask offsets are tavern-space constants: the binary draws both from raw
        // tav_topleft, without this->rect (only bg/anims use it).
        int32_t avail_count = this->vis_tav->avail_entries.GetSize();
        for (int32_t i = 0; i < avail_count; i++) {
            int32_t row = i / 6;
            int32_t col = i % 6;
            CPoint pt = this->roster_rects[row][col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            this->bmp_man_back->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            CSprite256* sprite = this->avail_sprites[i];
            sprite->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, this->avail_frames[i], 0, 0);
            if (this->vis_tav->selection_index == i && now - tavscene_druid_frame_ts > 0x7D) {
                this->avail_frames[i] = (this->avail_frames[i] + 1) % sprite->GetFrameCount();
                tavscene_druid_frame_ts = now;
            }

            CUnit* unit = this->vis_tav->avail_entries[i];
            int32_t entry = this->vis_tav->FindEntryIndexForUnit(unit);
            if (this->vis_tav->entrie_id[entry] & 0x80000000) {
                CRect& r = this->roster_rects[row][col];
                g_font2->DrawTextWithShadow(tav_topleft.x + r.left + r.Width() / 2, tav_topleft.y + r.top + r.Height() / 2, TxtFile::AllLines[0x101], 10, clrsh_CoralRed, 1);
            }
        }

        for (int32_t i = 0; i < this->vis_tav->reserved_entries.GetSize(); i++) {
            int32_t row = (avail_count + i) / 6;
            int32_t col = (avail_count + i) % 6;
            CPoint pt = this->roster_rects[row][col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            if (this->vis_tav->selection_index == avail_count + i && now - tavscene_druid_frame_ts > 0x7D) {
                this->reserved_frames[i] = (this->reserved_frames[i] + 1) % this->reserved_sprites[i]->GetFrameCount();
                tavscene_druid_frame_ts = now;
            }
            this->bmp_man_back_talk->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            this->reserved_sprites[i]->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, this->reserved_frames[i], 0, 0);
        }
    } else {
        this->DrawQuestRoster();
    }

    this->bmp_lu_over->DrawSubRectMasked(tav_topleft.x + 0xA0, tav_topleft.y, 0, 0, 0x10, 0xEE);
    this->bmp_ld_over->DrawSubRectMasked(tav_topleft.x + 0xA0, tav_topleft.y + 0xEE, 0, 0, 0x10, 0xF2);
    this->bmp_ru_over->DrawSubRectMasked(tav_topleft.x + 0x1D0, tav_topleft.y, 0, 0, 0x10, 0xEE);
    if (this->vis_tav->info_panel->info_mode) {
        g_bmp_humanbackl->DrawSubRectMasked(tav_topleft.x + 0x1D0, tav_topleft.y + 0xEE, 0, 0, 0x10, 0xF2);
    } else {
        g_bmp_textbackl->DrawSubRectMasked(tav_topleft.x + 0x1D0, tav_topleft.y + 0xEE, 0, 0, 0x10, 0xF2);
    }
    UnlockSurface2();
    CVisualObject::VMethod7();

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE && now - this->ambient_last_tick > this->ambient_interval) {
        switch (GetRandS16(3) + 1) {
        case 1:
            CSound::Play((CSound&)tav_druid->snd_druid[8]);
            break;
        case 2:
            CSound::Play((CSound&)tav_druid->snd_druid[9]);
            break;
        case 3:
            CSound::Play((CSound&)tav_druid->snd_druid[10]);
            break;
        }
        this->ambient_interval = GetRandS16(2000) + 2000;
        this->ambient_last_tick = timeGetTime();
    }
}


// 4A3FC0
VisTavSceneDruid::~VisTavSceneDruid() {}


// 4A05A0
VisTavSceneDruid::VisTavSceneDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav)
: VisTavScene(_id, l, t, r, b, tav) {
    this->ambient_last_tick = timeGetTime();
    this->ambient_interval = GetRandS16(2000) + 2000;
    this->bmp_flash = nullptr;
}


// 4A158F
void VisTavSceneDruid::VMethod29() {
    delete this->bmp_man_back;
    this->bmp_man_back = nullptr;
    delete this->bmp_man_back_talk;
    this->bmp_man_back_talk = nullptr;
    delete this->bmp_background;
    this->bmp_background = nullptr;
    delete this->bmp_lu_over;
    this->bmp_lu_over = nullptr;
    delete this->bmp_ld_over;
    this->bmp_ld_over = nullptr;
    delete this->bmp_ru_over;
    this->bmp_ru_over = nullptr;
    delete this->bmp_flash;
    this->bmp_flash = nullptr;

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != SESSION_LOCAL_BATTLE) {
        for (int32_t i = 0; i < 13; i++) {
            delete this->quest_bmps[i];
            this->quest_bmps[i] = nullptr;
        }
    }

    for (int32_t i = 0; i < this->roster_sprites.GetSize(); i++) {
        if (this->roster_sprites[i] != nullptr) {
            delete this->roster_sprites[i];
            this->roster_sprites[i] = nullptr;
        }
    }
    this->roster_sprites.RemoveAll();
}


// 4A11E3
void VisTavSceneDruid::VMethod28() {
    this->VMethod29();
    this->bmp_man_back = new CBmp64("graphics\\Interface\\Inn\\manback.bmp");
    g_mousept.Update();
    this->bmp_man_back_talk = new CBmp64("graphics\\Interface\\Inn\\ManBackTalk.bmp");
    g_mousept.Update();
    this->bmp_background = new CBmp64("graphics\\Interface\\Inn_druid\\TavernMain.bmp");
    g_mousept.Update();
    this->bmp_lu_over = new CBmp64("graphics\\interface\\inn\\LUOver.bmp");
    g_mousept.Update();
    this->bmp_ld_over = new CBmp64("graphics\\interface\\inn\\LDOver.bmp");
    g_mousept.Update();
    this->bmp_ru_over = new CBmp64("graphics\\interface\\inn\\RUOver.bmp");
    g_mousept.Update();
    this->bmp_flash = new CBmp64("graphics\\interface\\inn_druid\\taverner\\a30001.bmp");
    g_mousept.Update();

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != SESSION_LOCAL_BATTLE) {
        CString name;
        for (int32_t i = 0; i < 13; i++) {
            name.Format("graphics\\interface\\inn\\quests\\%02d.bmp", i + 1);
            this->quest_bmps[i] = new CBmp64(name);
        }
    }
}

// 4A2BB8
void VisTavSceneKaarg::VMethod7() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisTavKaarg* tav_kaarg = (VisTavKaarg*)this->vis_tav;

    if (!tavscene_kaarg_statics_inited) {
        tavscene_kaarg_statics_inited = true;
        tavscene_kaarg_unused_rand = rand() / 0x41;
        uint32_t t = timeGetTime();
        tavscene_kaarg_frame_ts = t;
        tavscene_kaarg_unused_ts1 = t;
        tavscene_kaarg_state_ts = t;
        tavscene_kaarg_unused_ts2 = t;
    }

    uint32_t now = timeGetTime();
    LockSurface2();
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    int32_t x = tav_topleft.x + this->rect.left;
    int32_t y = tav_topleft.y + this->rect.top;

    this->bmp_background->Draw(x, y, 0, 0, 0);

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        if (tavscene_kaarg_anim_state == -1) {
            this->script_frame_index = 0;
            int32_t which = Random0N(0x14) + 1;
            switch (which) {
            case 1:
            case 2:
            case 5:
                tavscene_kaarg_anim_state = which;
                break;
            case 3:
            case 4:
            case 6:
                tavscene_kaarg_anim_state = 3;
                break;
            default:
                tavscene_kaarg_anim_state = 4;
                break;
            }
            if (tavscene_kaarg_anim_state == 1) {
                CSound::Play((CSound&)tav_kaarg->snd_kaarg[5]);
            }
            if (tavscene_kaarg_anim_state == 2) {
                CSound::Play((CSound&)tav_kaarg->snd_kaarg[4]);
            }
        }

        switch (tavscene_kaarg_anim_state) {
        case 1:
            this->anims_kaarg[0].Draw(x + 0x48, y + 0x58);
            break;
        case 2:
            this->anims_kaarg[1].Draw(x + 0x48, y + 0x58);
            break;
        case 3:
            this->anims_kaarg[2].Draw(x + 0x70, y + 0x70);
            break;
        case 4:
            this->anims_kaarg[3].Draw(x + 0xC8, y + 0x88);
            break;
        case 5:
            this->anims_kaarg[4].frames[kaarg_frame_script[this->script_frame_index]]->Draw(x + 0x58, y + 0x54, 0, 0, 0);
            break;
        }

        if (tavscene_kaarg_anim_state != -1 && now - tavscene_kaarg_state_ts > 100) {
            switch (tavscene_kaarg_anim_state) {
            case 1:
                if (!this->anims_kaarg[0].StepForward()) {
                    tavscene_kaarg_anim_state = -1;
                    this->anims_kaarg[0].frame_idx = 0;
                }
                tavscene_kaarg_state_ts = now;
                break;
            case 2:
                if (!this->anims_kaarg[1].StepForward()) {
                    tavscene_kaarg_anim_state = -1;
                    this->anims_kaarg[1].frame_idx = 0;
                }
                tavscene_kaarg_state_ts = now;
                break;
            case 3:
                if (!this->anims_kaarg[2].StepForward()) {
                    tavscene_kaarg_anim_state = -1;
                    this->anims_kaarg[2].frame_idx = 0;
                }
                tavscene_kaarg_state_ts = now;
                break;
            case 4:
                if (!this->anims_kaarg[3].StepForward()) {
                    tavscene_kaarg_anim_state = -1;
                    this->anims_kaarg[3].frame_idx = 0;
                }
                tavscene_kaarg_state_ts = now;
                break;
            case 5:
                this->script_frame_index++;
                if (kaarg_frame_script[this->script_frame_index] == -1) {
                    tavscene_kaarg_anim_state = -1;
                    this->anims_kaarg[4].frame_idx = 0;
                }
                tavscene_kaarg_state_ts = now;
                break;
            }
        }

        // cell rects are tavern-space (InitSceneRects positions them from this->rect), and the
        // epilogue mask offsets are tavern-space constants: the binary draws both from raw
        // tav_topleft, without this->rect (only bg/anims use it).
        int32_t avail_count = this->vis_tav->avail_entries.GetSize();
        for (int32_t i = 0; i < avail_count; i++) {
            int32_t row = i / 6;
            int32_t col = i % 6;
            CPoint pt = this->roster_rects[row][col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            this->bmp_man_back->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            CSprite256* sprite = this->avail_sprites[i];
            sprite->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, this->avail_frames[i], 0, 0);
            if (this->vis_tav->selection_index == i && now - tavscene_kaarg_frame_ts > 0x7D) {
                this->avail_frames[i] = (this->avail_frames[i] + 1) % sprite->GetFrameCount();
                tavscene_kaarg_frame_ts = now;
            }

            CUnit* unit = this->vis_tav->avail_entries[i];
            int32_t entry = this->vis_tav->FindEntryIndexForUnit(unit);
            if (this->vis_tav->entrie_id[entry] & 0x80000000) {
                CRect& r = this->roster_rects[row][col];
                g_font2->DrawTextWithShadow(tav_topleft.x + r.left + r.Width() / 2, tav_topleft.y + r.top + r.Height() / 2, TxtFile::AllLines[0x101], 10, clrsh_CoralRed, 1);
            }
        }

        for (int32_t i = 0; i < this->vis_tav->reserved_entries.GetSize(); i++) {
            int32_t row = (avail_count + i) / 6;
            int32_t col = (avail_count + i) % 6;
            CPoint pt = this->roster_rects[row][col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            if (this->vis_tav->selection_index == avail_count + i && now - tavscene_kaarg_frame_ts > 0x7D) {
                this->reserved_frames[i] = (this->reserved_frames[i] + 1) % this->reserved_sprites[i]->GetFrameCount();
                tavscene_kaarg_frame_ts = now;
            }
            this->bmp_man_back_talk->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            this->reserved_sprites[i]->Draw(tav_topleft.x + pt.x, tav_topleft.y + pt.y, this->reserved_frames[i], 0, 0);
        }
    } else {
        this->DrawQuestRoster();
    }

    this->bmp_lu_over->DrawSubRectMasked(tav_topleft.x + 0xA0, tav_topleft.y, 0, 0, 0x10, 0xEE);
    this->bmp_ld_over->DrawSubRectMasked(tav_topleft.x + 0xA0, tav_topleft.y + 0xEE, 0, 0, 0x10, 0xF2);
    this->bmp_ru_over->DrawSubRectMasked(tav_topleft.x + 0x1D0, tav_topleft.y, 0, 0, 0x10, 0xEE);
    if (this->vis_tav->info_panel->info_mode) {
        g_bmp_humanbackl->DrawSubRectMasked(tav_topleft.x + 0x1D0, tav_topleft.y + 0xEE, 0, 0, 0x10, 0xF2);
    } else {
        g_bmp_textbackl->DrawSubRectMasked(tav_topleft.x + 0x1D0, tav_topleft.y + 0xEE, 0, 0, 0x10, 0xF2);
    }
    UnlockSurface2();

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE && now - this->ambient2_last_tick > this->ambient2_interval) {
        switch (GetRandS16(3) + 1) {
        case 1:
            CSound::Play((CSound&)tav_kaarg->snd_kaarg[7]);
            break;
        case 2:
            CSound::Play((CSound&)tav_kaarg->snd_kaarg[8]);
            break;
        case 3:
            CSound::Play((CSound&)tav_kaarg->snd_kaarg[9]);
            break;
        }
        this->ambient2_interval = GetRandS16(2000) + 2000;
        this->ambient2_last_tick = timeGetTime();
    }

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE && now - this->ambient1_last_tick > this->ambient1_interval) {
        switch (GetRandS16(4) + 1) {
        case 1:
            CSound::Play((CSound&)tav_kaarg->snd_kaarg[0]);
            break;
        case 2:
            CSound::Play((CSound&)tav_kaarg->snd_kaarg[1]);
            break;
        case 3:
            CSound::Play((CSound&)tav_kaarg->snd_kaarg[2]);
            break;
        case 4:
            CSound::Play((CSound&)tav_kaarg->snd_kaarg[3]);
            break;
        }
        this->ambient1_interval = GetRandS16(2000) + 2000;
        this->ambient1_last_tick = timeGetTime();
    }

    CVisualObject::VMethod7();
}


// 4A2AA4
VisTavSceneKaarg::VisTavSceneKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav)
: VisTavScene(_id, l, t, r, b, tav) {
    this->ambient1_last_tick = timeGetTime();
    this->ambient1_interval = GetRandS16(2000) + 2000;
    this->ambient2_last_tick = timeGetTime();
    this->ambient2_interval = GetRandS16(2000) + 2000;
}


// 4A40C0
VisTavSceneKaarg::~VisTavSceneKaarg() {}


// 4A3B25
void VisTavSceneKaarg::VMethod29() {
    delete this->bmp_man_back;
    this->bmp_man_back = nullptr;
    delete this->bmp_man_back_talk;
    this->bmp_man_back_talk = nullptr;
    delete this->bmp_background;
    this->bmp_background = nullptr;
    delete this->bmp_lu_over;
    this->bmp_lu_over = nullptr;
    delete this->bmp_ld_over;
    this->bmp_ld_over = nullptr;
    delete this->bmp_ru_over;
    this->bmp_ru_over = nullptr;

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != SESSION_LOCAL_BATTLE) {
        for (int32_t i = 0; i < 13; i++) {
            delete this->quest_bmps[i];
            this->quest_bmps[i] = nullptr;
        }
    }

    for (int32_t i = 0; i < this->roster_sprites.GetSize(); i++) {
        if (this->roster_sprites[i] != nullptr) {
            delete this->roster_sprites[i];
            this->roster_sprites[i] = nullptr;
        }
    }
    this->roster_sprites.RemoveAll();
}


// 4A37E7
void VisTavSceneKaarg::VMethod28() {
    this->VMethod29();
    this->bmp_man_back = new CBmp64("graphics\\Interface\\Inn\\manback.bmp");
    g_mousept.Update();
    this->bmp_man_back_talk = new CBmp64("graphics\\Interface\\Inn\\ManBackTalk.bmp");
    g_mousept.Update();
    this->bmp_background = new CBmp64("graphics\\Interface\\inn_kaarg\\TavernMain.bmp");
    g_mousept.Update();
    this->bmp_lu_over = new CBmp64("graphics\\interface\\inn\\LUOver.bmp");
    g_mousept.Update();
    this->bmp_ld_over = new CBmp64("graphics\\interface\\inn\\LDOver.bmp");
    g_mousept.Update();
    this->bmp_ru_over = new CBmp64("graphics\\interface\\inn\\RUOver.bmp");
    g_mousept.Update();

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != SESSION_LOCAL_BATTLE) {
        CString name;
        for (int32_t i = 0; i < 13; i++) {
            name.Format("graphics\\interface\\inn\\quests\\%02d.bmp", i + 1);
            this->quest_bmps[i] = new CBmp64(name);
        }
    }
}


// 49FAAA
void VisTavDruid::VMethod28() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    g_mousept.DisableHint();

    this->map_context = main_wnd->vis_map_context;
    this->info_panel = main_wnd->vis_charinfo;
    main_wnd->vis_right_panel->RemoveChild(this->info_panel);

    CRect& panel_rect = this->info_panel->GetRect();
    CPoint pt(0x280 - panel_rect.Width(), 0);
    panel_rect.OffsetRect(pt.x, pt.y);
    this->info_panel->SetRect(&panel_rect);
    this->AddChild(this->info_panel);

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        CString tips_text;
        MissionGetTips(2, &tips_text);
        this->tips = new VisTipsDialog(0x467, 0, 0, 0x138, 0xC8, tips_text);
        this->scene->AddChild(this->tips);
    } else {
        if (this->tips != nullptr) {
            this->scene->RemoveChild(this->tips);
            delete this->tips;
        }
        this->tips = nullptr;
    }

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        int32_t ids[32];
        int32_t count;
        ScenarioEnterInn(ids, &count);
        this->entrie_id.RemoveAll();
        for (int32_t i = 0; i < count; i++) {
            this->entrie_id.Add(ids[i]);
        }
    }

    this->avail_entries.RemoveAll();
    this->reserved_entries.RemoveAll();
    for (int32_t i = 0; i < this->entrie_id.GetSize(); i++) {
        uint32_t unit_id = this->entrie_id[i] & 0xFFFF;
        uint32_t category = (this->entrie_id[i] >> 0x1C) & 7;
        if (category == 1 || category == 2) {
            this->avail_entries.Add(this->map_context->FindOrCreateUnitByServerId(unit_id));
        } else {
            this->reserved_entries.Add(this->map_context->FindOrCreateUnitByServerId(unit_id));
        }
    }

    if (this->avail_entries.GetSize() + this->reserved_entries.GetSize() != 0) {
        this->selection_index = 0;
    } else {
        this->selection_index = -1;
    }

    g_StructEnter.RebuildFromMap();
    this->selected_entries.Copy(g_StructEnter.structure_list);
    this->select_party = g_StructEnter.FindFlaggedStructureUnit();

    this->map_context->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);
    this->selected_entries[this->select_party]->SetSelected(1);
    this->map_context->UpdateSelectionState();
    this->selected_entries[this->select_party]->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;

    this->scene->VMethod26();

    VisTavSceneDruid* scene_druid = (VisTavSceneDruid*)this->scene;

    CStringArray names;
    CString name;

    for (int32_t i = 0; i < 10; i++) {
        name.Format("graphics\\interface\\inn_druid\\waterdrop\\d%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        scene_druid->anims_druid[0].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 0x28; i++) {
        name.Format("graphics\\interface\\inn_druid\\taverner\\a1%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        scene_druid->anims_druid[1].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 0x1E; i++) {
        name.Format("graphics\\interface\\inn_druid\\taverner\\a2%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        scene_druid->anims_druid[2].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    this->right_panel->LoadRightPanelBmps();
    this->right_panel->ResetButtonStates();
    this->scene->VMethod28();
    this->left_panel->LoadLeftPanelBmps();
    this->right_panel->UpdateButtonLabels();
    this->VMethod30();

    this->dialog_active = 1;

    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();

    VisScreen::VMethod28();
    CSound::Play(this->sounds[8]);
    PlayAmbientSfx(&this->snd_druid[7]);
    g_Cursors[0]->Use();
    g_mousept.EnableHint();

    this->quest_id = 0;
    this->quest_selection = -1;
    this->quest_alt_close = 0;
}


// 4A0278
void VisTavDruid::DoClose(uint32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    this->VMethod9();
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        ScenarioLeaveInn();
    }

    if (this->quest_alt_close != 0) {
        main_wnd->vis_map_context->LeaveInn(0xAAAAAAAA);
    } else {
        main_wnd->vis_map_context->LeaveInn(this->quest_selection);
    }

    this->dialog_active = 0;

    CRect& panel_rect = this->info_panel->GetRect();
    CPoint pt(panel_rect.Width() - 0x280, 0);
    panel_rect.OffsetRect(pt.x, pt.y);
    this->info_panel->SetRect(&panel_rect);
    this->RemoveChild(this->info_panel);
    main_wnd->vis_right_panel->AddChild(this->info_panel);

    if (this->tips != nullptr) {
        this->scene->RemoveChild(this->tips);
        delete this->tips;
        this->tips = nullptr;
    }

    this->right_panel->FreeRightPanelBmps();
    this->scene->VMethod29();
    this->scene->VMethod27();
    this->left_panel->FreeLeftPanelBmps();

    VisTavSceneDruid* scene_druid = (VisTavSceneDruid*)this->scene;
    scene_druid->anims_druid[0].FreeAnimFrames();
    scene_druid->anims_druid[1].FreeAnimFrames();
    scene_druid->anims_druid[2].FreeAnimFrames();

    this->VMethod31();

    this->avail_entries.RemoveAll();
    this->selected_entries.RemoveAll();
    this->reserved_entries.RemoveAll();

    VisScreen::DoClose(code);
    this->quest_map->RemovePlayerQuests(0);

    while (this->rewards.GetSize() != 0) {
        TokenEntry* entry = this->rewards[0];
        if (entry != nullptr) {
            delete entry;
        }
        this->rewards.RemoveAt(0, 1);
    }
}


// 49F51B
void VisTavDruid::VMethod26() {
    this->dialog_active = 0;
    for (int32_t i = 0; i < 13; i++) {
        this->sounds[i].sample = nullptr;
    }
    for (int32_t i = 0; i < 11; i++) {
        this->snd_druid[i] = nullptr;
    }
    this->tips = nullptr;

    this->left_panel = new VisTavLeftPanel(0x44D, 0, 0, 0xA0, 0x1E0, this);
    this->right_panel = new VisTavRightPanel(0x44E, 0x1E0, 0, 0x280, 0xEE, this);
    this->scene = new VisTavSceneDruid(0x450, 0xA0, 0, 0x1E0, 0x1E0, this);

    this->AddChild(this->left_panel);
    this->AddChild(this->right_panel);
    this->AddChild(this->scene);

    this->selection_index = 0;
    this->avail_entries.RemoveAll();
    this->quest_map = new QuestMap();
}


// 49F7B0
void VisTavDruid::VMethod30() {
    this->VMethod31();
    ReloadSfx(&this->sounds[5].sample, "SFX\\Add.wav");
    ReloadSfx(&this->sounds[6].sample, "SFX\\NoAdd.wav");
    ReloadSfx(&this->sounds[7].sample, "SFX\\Town\\Shop\\nofit.wav");
    ReloadSfx(&this->sounds[8].sample, "SFX\\Town_druid\\Inn\\din1.wav");
    ReloadSfx(&this->sounds[9].sample, "SFX\\Town\\Inn\\Helper.wav");
    ReloadSfx(&this->sounds[11].sample, "SFX\\Out.wav");
    ReloadSfx(&this->sounds[12].sample, "SFX\\Talk.wav");
    ReloadSfx(&this->snd_druid[0], "SFX\\Town_druid\\Inn\\DWater1.wav");
    ReloadSfx(&this->snd_druid[1], "SFX\\Town_druid\\Inn\\DWater2.wav");
    ReloadSfx(&this->snd_druid[2], "SFX\\Town_druid\\Inn\\DWater3.wav");
    ReloadSfx(&this->snd_druid[3], "SFX\\Town_druid\\Inn\\DWater4.wav");
    ReloadSfx(&this->snd_druid[4], "SFX\\Town_druid\\Inn\\DDruid3.wav");
    ReloadSfx(&this->snd_druid[5], "SFX\\Town_druid\\Inn\\Ddruid4.wav");
    ReloadSfx(&this->snd_druid[6], "SFX\\Town_druid\\Inn\\Ddruid41.wav");
    ReloadSfx(&this->snd_druid[7], "SFX\\Town_druid\\Inn\\Dforest2.wav");
    ReloadSfx(&this->snd_druid[8], "SFX\\Town_druid\\Inn\\Dbird4.wav");
    ReloadSfx(&this->snd_druid[9], "SFX\\Town_druid\\Inn\\Dbird41.wav");
    ReloadSfx(&this->snd_druid[10], "SFX\\Town_druid\\Inn\\Dbird42.wav");
}


// 49F961
void VisTavDruid::VMethod31() {
    FreeSfxSample(&this->sounds[5].sample);
    FreeSfxSample(&this->sounds[6].sample);
    FreeSfxSample(&this->sounds[7].sample);
    FreeSfxSample(&this->sounds[8].sample);
    FreeSfxSample(&this->sounds[9].sample);
    FreeSfxSample(&this->sounds[11].sample);
    FreeSfxSample(&this->sounds[12].sample);
    for (int32_t i = 0; i < 11; i++) {
        FreeSfxSample(&this->snd_druid[i]);
    }
}


// 4A3F70
VisTavDruid::~VisTavDruid() {
}


// 49F4E6
VisTavDruid::VisTavDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisTav(_id, l, t, r, b) {
}


// 4A1BA2
void VisTavKaarg::VMethod28() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    g_mousept.DisableHint();

    this->map_context = main_wnd->vis_map_context;
    this->info_panel = main_wnd->vis_charinfo;
    main_wnd->vis_right_panel->RemoveChild(this->info_panel);

    CRect& panel_rect = this->info_panel->GetRect();
    CPoint pt(0x280 - panel_rect.Width(), 0);
    panel_rect.OffsetRect(pt.x, pt.y);
    this->info_panel->SetRect(&panel_rect);
    this->AddChild(this->info_panel);

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        CString tips_text;
        MissionGetTips(2, &tips_text);
        this->tips = new VisTipsDialog(0x467, 0, 0, 0x138, 0xC8, tips_text);
        this->scene->AddChild(this->tips);
    } else {
        if (this->tips != nullptr) {
            this->scene->RemoveChild(this->tips);
            delete this->tips;
        }
        this->tips = nullptr;
    }

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        int32_t ids[32];
        int32_t count;
        ScenarioEnterInn(ids, &count);
        this->entrie_id.RemoveAll();
        for (int32_t i = 0; i < count; i++) {
            this->entrie_id.Add(ids[i]);
        }
    }

    this->avail_entries.RemoveAll();
    this->reserved_entries.RemoveAll();
    for (int32_t i = 0; i < this->entrie_id.GetSize(); i++) {
        uint32_t unit_id = this->entrie_id[i] & 0xFFFF;
        uint32_t category = (this->entrie_id[i] >> 0x1C) & 7;
        if (category == 1 || category == 2) {
            this->avail_entries.Add(this->map_context->FindOrCreateUnitByServerId(unit_id));
        } else {
            this->reserved_entries.Add(this->map_context->FindOrCreateUnitByServerId(unit_id));
        }
    }

    if (this->avail_entries.GetSize() + this->reserved_entries.GetSize() != 0) {
        this->selection_index = 0;
    } else {
        this->selection_index = -1;
    }

    g_StructEnter.RebuildFromMap();
    this->selected_entries.Copy(g_StructEnter.structure_list);
    this->select_party = g_StructEnter.FindFlaggedStructureUnit();

    this->map_context->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);
    this->selected_entries[this->select_party]->SetSelected(1);
    this->map_context->UpdateSelectionState();
    this->selected_entries[this->select_party]->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;

    this->scene->VMethod26();

    VisTavSceneKaarg* scene_kaarg = (VisTavSceneKaarg*)this->scene;

    CStringArray names;
    CString name;

    for (int32_t i = 0; i < 0xF; i++) {
        name.Format("graphics\\interface\\inn_kaarg\\taverner\\a1%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        scene_kaarg->anims_kaarg[0].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 0x19; i++) {
        name.Format("graphics\\interface\\inn_kaarg\\taverner\\a2%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        scene_kaarg->anims_kaarg[1].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 3; i++) {
        name.Format("graphics\\interface\\inn_kaarg\\taverner\\a3%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        scene_kaarg->anims_kaarg[2].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 7; i++) {
        name.Format("graphics\\interface\\inn_kaarg\\taverner\\a4%.4d.bmp", i);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        scene_kaarg->anims_kaarg[3].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 0x1D; i++) {
        name.Format("graphics\\interface\\inn_kaarg\\taverner\\a5%.4d.bmp", i);
        names.Add(name);
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        scene_kaarg->anims_kaarg[4].LoadAnimFrames(&names);
    }
    names.RemoveAll();

    this->right_panel->LoadRightPanelBmps();
    this->right_panel->ResetButtonStates();
    this->scene->VMethod28();
    this->left_panel->LoadLeftPanelBmps();
    this->right_panel->UpdateButtonLabels();
    this->VMethod30();

    this->dialog_active = 1;

    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();

    VisScreen::VMethod28();
    CSound::Play(this->sounds[8]);
    PlayAmbientSfx(&this->snd_kaarg[6]);
    g_Cursors[0]->Use();
    g_mousept.EnableHint();

    this->quest_id = 0;
    this->quest_selection = -1;
    this->quest_alt_close = 0;
}


// 4A2448
void VisTavKaarg::DoClose(uint32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    this->VMethod9();
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        ScenarioLeaveInn();
    }

    if (this->quest_alt_close != 0) {
        main_wnd->vis_map_context->LeaveInn(0xAAAAAAAA);
    } else {
        main_wnd->vis_map_context->LeaveInn(this->quest_selection);
    }

    this->dialog_active = 0;

    CRect& panel_rect = this->info_panel->GetRect();
    CPoint pt(panel_rect.Width() - 0x280, 0);
    panel_rect.OffsetRect(pt.x, pt.y);
    this->info_panel->SetRect(&panel_rect);
    this->RemoveChild(this->info_panel);
    main_wnd->vis_right_panel->AddChild(this->info_panel);

    if (this->tips != nullptr) {
        this->scene->RemoveChild(this->tips);
        delete this->tips;
        this->tips = nullptr;
    }

    this->right_panel->FreeRightPanelBmps();
    this->scene->VMethod29();
    this->scene->VMethod27();
    this->left_panel->FreeLeftPanelBmps();

    VisTavSceneKaarg* scene_kaarg = (VisTavSceneKaarg*)this->scene;
    scene_kaarg->anims_kaarg[0].FreeAnimFrames();
    scene_kaarg->anims_kaarg[1].FreeAnimFrames();
    scene_kaarg->anims_kaarg[2].FreeAnimFrames();
    scene_kaarg->anims_kaarg[3].FreeAnimFrames();
    scene_kaarg->anims_kaarg[4].FreeAnimFrames();

    this->VMethod31();

    this->avail_entries.RemoveAll();
    this->selected_entries.RemoveAll();
    this->reserved_entries.RemoveAll();

    VisScreen::DoClose(code);
    this->quest_map->RemovePlayerQuests(0);

    while (this->rewards.GetSize() != 0) {
        TokenEntry* entry = this->rewards[0];
        if (entry != nullptr) {
            delete entry;
        }
        this->rewards.RemoveAt(0, 1);
    }
}


// 4A190D
void VisTavKaarg::VMethod26() {
    this->dialog_active = 0;
    for (int32_t i = 0; i < 13; i++) {
        this->sounds[i].sample = nullptr;
    }
    for (int32_t i = 0; i < 10; i++) {
        this->snd_kaarg[i] = nullptr;
    }
    this->tips = nullptr;

    this->left_panel = new VisTavLeftPanel(0x44D, 0, 0, 0xA0, 0x1E0, this);
    this->right_panel = new VisTavRightPanel(0x44E, 0x1E0, 0, 0x280, 0xEE, this);
    this->scene = new VisTavSceneKaarg(0x450, 0xA0, 0, 0x1E0, 0x1E0, this);

    this->AddChild(this->left_panel);
    this->AddChild(this->right_panel);
    this->AddChild(this->scene);

    this->selection_index = 0;
    this->avail_entries.RemoveAll();
    this->quest_map = new QuestMap();
}


// 4A26D9
void VisTavKaarg::VMethod30() {
    this->VMethod31();
    ReloadSfx(&this->sounds[5].sample, "SFX\\Add.wav");
    ReloadSfx(&this->sounds[6].sample, "SFX\\NoAdd.wav");
    ReloadSfx(&this->sounds[7].sample, "SFX\\Town\\Shop\\nofit.wav");
    ReloadSfx(&this->sounds[8].sample, "SFX\\Town_kaarg\\Inn\\Kin1.wav");
    ReloadSfx(&this->sounds[9].sample, "SFX\\Town\\Inn\\Helper.wav");
    ReloadSfx(&this->sounds[11].sample, "SFX\\Out.wav");
    ReloadSfx(&this->sounds[12].sample, "SFX\\Talk.wav");
    ReloadSfx(&this->snd_kaarg[0], "SFX\\Town_kaarg\\Inn\\Kdish1.wav");
    ReloadSfx(&this->snd_kaarg[1], "SFX\\Town_kaarg\\Inn\\Kdish2.wav");
    ReloadSfx(&this->snd_kaarg[2], "SFX\\Town_kaarg\\Inn\\Kdish3.wav");
    ReloadSfx(&this->snd_kaarg[3], "SFX\\Town_kaarg\\Inn\\Kdish4.wav");
    ReloadSfx(&this->snd_kaarg[4], "SFX\\Town_kaarg\\Inn\\Kman2.wav");
    ReloadSfx(&this->snd_kaarg[5], "SFX\\Town_kaarg\\Inn\\Kman3.wav");
    ReloadSfx(&this->snd_kaarg[6], "SFX\\Town_kaarg\\Inn\\Kvox5.wav");
    ReloadSfx(&this->snd_kaarg[7], "SFX\\Town_kaarg\\Inn\\Kvox6.wav");
    ReloadSfx(&this->snd_kaarg[8], "SFX\\Town_kaarg\\Inn\\Kvox7.wav");
    ReloadSfx(&this->snd_kaarg[9], "SFX\\Town_kaarg\\Inn\\Kvox8.wav");
}


// 4A2873
void VisTavKaarg::VMethod31() {
    FreeSfxSample(&this->sounds[5].sample);
    FreeSfxSample(&this->sounds[6].sample);
    FreeSfxSample(&this->sounds[7].sample);
    FreeSfxSample(&this->sounds[8].sample);
    FreeSfxSample(&this->sounds[9].sample);
    FreeSfxSample(&this->sounds[11].sample);
    FreeSfxSample(&this->sounds[12].sample);
    for (int32_t i = 0; i < 10; i++) {
        FreeSfxSample(&this->snd_kaarg[i]);
    }
}


// 4A18D8
VisTavKaarg::VisTavKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisTav(_id, l, t, r, b) {
}


// 4A4070
VisTavKaarg::~VisTavKaarg() {
}







