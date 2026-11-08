#include "visual.h"
#include <timeapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>

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
#include "vis_shop.h"

// 4BA5A0
VisShop::~VisShop() {
    this->FreeShopGfx();
    this->VMethod32();
    if (this->assortiment != nullptr) {
        this->RemoveChild(this->assortiment);
        delete this->assortiment;
        this->assortiment = nullptr;
    }
    if (this->to_sell != nullptr) {
        this->RemoveChild(this->to_sell);
        delete this->to_sell;
        this->to_sell = nullptr;
    }
    if (this->to_buy != nullptr) {
        this->RemoveChild(this->to_buy);
        delete this->to_buy;
        this->to_buy = nullptr;
    }
    if (this->tips != nullptr) {
        this->shop_compass->RemoveChild(this->tips);
        delete this->tips;
        this->tips = nullptr;
    }
    if (this->shop_compass != nullptr) {
        this->RemoveChild(this->shop_compass);
        delete this->shop_compass;
        this->shop_compass = nullptr;
    }
    if (this->buttons != nullptr) {
        this->RemoveChild(this->buttons);
        delete this->buttons;
        this->buttons = nullptr;
    }
}


// 4BCD02
void VisShop::CloseShopDialog() {
    if (this->spell_panel != nullptr) {
        this->ShowBuyPanel();
        this->gameplay->CloseSpellBook();
    }
    this->ReturnPendingBuyItems();
    this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
}


// 4BA832
int32_t VisShop::OnKeyDown(uint32_t wparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (wparam == 0xd) {
        return 1;
    }
    if (wparam == 0x1b) {
        this->CloseShopDialog();
        if (main_wnd->dialogsMask == DIALOG_MASK_SHOP) {
            main_wnd->PostMessage(MAIN_MSG_SHOW_CURRENT_TOWN, 0, 0);
        }
        return 1;
    }
    return 0;
}


// 4BB045
int32_t VisShop::OnLButtonUp(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr && this->spell_panel != nullptr) {
        if (this->spell_panel->GetRect().PtInRect(pos)) {
            this->CancelItemDrag();
            return 1;
        }
    }
    if (main_wnd->drag_entry != nullptr && !this->rect.PtInRect(pos)) {
        this->CancelItemDrag();
        return 1;
    }
    return CVisualObject::OnLButtonUp(wparam, pos);
}


// 4BB79A
void VisShop::VMethod8(CRect* rect) {
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    if (this->dialog_active != 0) {
        LockSurface2();
        FillRectColorSimple(screen_rect.left, screen_rect.top + 0x184, screen_rect.right + 0x1d0, screen_rect.bottom + 0x188, GetColorRGB(0, 0, 0));
        UnlockSurface2();
    }
}


// 4BA342
VisShop::VisShop(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm)
    : VisScreen(_id, l, t, r, b, btm) {
    this->placement_lock = 1;
    this->select_index = 0;
    this->select_info_panel = nullptr;
    this->gameplay = nullptr;
    this->dirty = 0;
    this->hovered_region = -1;
    this->result_gold = 0;
    this->sell_gold = 0;
    this->buy_gold = 0;
    this->current_gold = 0;
    this->scenario_talk_target = 0;
}


// 4BD0FB
void VisShop::VMethod31() {
    this->VMethod32();
    ReloadSfx(&this->snd_notif, "SFX\\Town\\Shop\\nofit.wav");
    ReloadSfx(&this->snd_step1, "SFX\\Town\\Shop\\step1.wav");
    ReloadSfx(&this->snd_step2, "SFX\\Town\\Shop\\step2.wav");
    ReloadSfx(&this->snd_breath, "SFX\\Town\\Shop\\breath.wav");
    ReloadSfx(&this->snd_depart, "SFX\\Town\\Shop\\depart.wav");
    ReloadSfx(&this->snd_buy, "SFX\\Town\\buy.wav");
    ReloadSfx(&this->snd_sell, "SFX\\Town\\sell.wav");
    ReloadSfx(&this->snd_enter, "SFX\\Town\\Shop\\enter.wav");
    ReloadSfx(&this->snd_start, "SFX\\Town\\Shop\\start.wav");
    ReloadSfx(&this->snd_pov1, "SFX\\Town\\Shop\\Povorot1.wav");
    ReloadSfx(&this->snd_pov2, "SFX\\Town\\Shop\\Povorot2.wav");
    ReloadSfx(&this->snd_inshop, "SFX\\Town\\Shop\\InShop.wav");
    ReloadSfx(&this->snd_out, "SFX\\Out.wav");
    ReloadSfx(&this->snd_undo, "SFX\\Undo.wav");
}


// 4BCF2F
void VisShop::VMethod30() {
    static uint8_t snd_timing_init = 0; // 665DA8 in asm
    static uint32_t last_snd_time; // 665DA0 in asm

    if ((snd_timing_init & 1) == 0) {
        snd_timing_init |= 1;
        last_snd_time = timeGetTime();
    }
    uint32_t now = timeGetTime();
    uint32_t delay = GetRandS16(30000) + 30000;
    uint32_t compass_flags = this->shop_compass->state;
    if ((compass_flags & 0x10) == 0 && (compass_flags & 0x20) == 0 && (compass_flags & 0x40) == 0) {
        if (now - last_snd_time > delay && !IsSfxPlaying(&this->to_buy->snd_speech)) {
            if (this->snd_start != nullptr && this->snd_start->FindPlayingChannel() == nullptr) {
                this->snd_start->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
            }
            last_snd_time = now;
            delay = GetRandS16(10000) + 10000;
        }
    } else if ((compass_flags & 0x10) != 0) {
        int32_t compass_state = this->shop_compass->center_frm;
        if (compass_state == 1) {
            this->snd_pov1->Play();
        } else if (compass_state == 0xa || compass_state == 0xe || compass_state == 0x12 || compass_state == 0x16) {
            this->snd_step1->Play();
        } else if (compass_state == 0x18) {
            this->snd_pov2->Play();
        }
    }
}


// 4BC0E7
void VisShop::DoClose(uint32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    this->VMethod9();
    this->dialog_active = 0;
    this->VMethod32();

    if (main_wnd->drag_entry != nullptr) {
        this->CancelItemDrag();
        ApplyCursor(g_Cursors[0]);
        main_wnd->ResetItemCursor();
        this->placement_lock = 0;
    }

    if (this->tips != nullptr) {
        this->shop_compass->RemoveChild(this->tips);
        delete this->tips;
        this->tips = nullptr;
    }

    this->shop_compass->state = 0;

    CRect& panel_rect = this->select_info_panel->GetRect();
    CPoint pt(panel_rect.Width() - 0x280, 0);
    panel_rect.OffsetRect(pt.x, pt.y);
    this->select_info_panel->SetRect(&panel_rect);
    this->RemoveChild(this->select_info_panel);
    main_wnd->vis_right_panel->AddChild(this->select_info_panel);
    this->select_info_panel = nullptr;
    this->gameplay = nullptr;

    if (this->spell_panel != nullptr) {
        this->RemoveChild(this->spell_panel);
    }
    this->spell_panel = nullptr;

    this->to_buy->VMethod42();
    this->assortiment->VMethod42();
    this->buttons->ReleaseBmp();
    this->shop_compass->VMethod27();
    this->shop_compass->VMethod36();
    this->FreeShopGfx();
    this->assortiment->ClearCategoryGrids();
    this->assortiment->FreeGridSfx();
    this->to_sell->FreeGridSfx();
    this->to_buy->FreeGridSfx();

    VisScreen::DoClose(code);

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        ScenarioLeaveShop();
    }
}


// 4BADCB
int32_t VisShop::OnMouseMove(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    pos -= CPoint(this->rect.left, this->rect.top);

    if (main_wnd->drag_entry != nullptr) {
        TokenEntry* item = main_wnd->drag_entry;
        bool reset = false;

        if ((this->to_sell->GetRect().PtInRect(pos) || this->select_info_panel->GetRect().PtInRect(pos))
            && (int32_t)item->slot_code >= 5 && (int32_t)item->slot_code <= 8) {
            reset = true;
        }
        if (!reset && this->assortiment->GetRect().PtInRect(pos)
            && (item->slot_code == 2 || (int32_t)item->slot_code <= 1)) {
            reset = true;
        }
        if (!reset
            && !this->to_sell->GetRect().PtInRect(pos)
            && !this->to_buy->GetRect().PtInRect(pos)
            && !this->assortiment->GetRect().PtInRect(pos)
            && !this->select_info_panel->GetRect().PtInRect(pos)) {
            reset = true;
        }
        if (!reset && !this->to_sell->GetRect().PtInRect(pos) && item->GetAttribute(1) == 0) {
            reset = true;
        }

        if (reset) {
            this->placement_lock = 0;
            ApplyCursor(g_Cursors[0x17]);
        }
    }

    if (this->select_info_panel->GetRect().PtInRect(pos) && this->hovered_region != 0x66) {
        this->hovered_region = 0x66;
        this->ResetInfoPanelButtons();
    }

    return 0;
}


// 4BBD75
void VisShop::VMethod26() {
    this->tips = nullptr;
    this->to_buy = new VisInvExtType3(0x3EB, 0, 0x12F, 0x1E0, 0x186, this);
    this->to_sell = new VisInvExtType2(0x3E9, 0, 0x186, 0x1E0, 0x1E0, this);
    this->assortiment = new VisInvExtType1(0x3EA, 0, 0, 0xA4, 0x12F, this);
    this->shop_compass = new VisShopCompass(0x3ED, 0xA4, 0, 0x1E0, 0x12F, this);
    this->buttons = new VisShopButtons(0x3EE, 0x1D0, 0, 0x280, 0xEE, this);

    this->AddChild(this->assortiment);
    this->AddChild(this->to_sell);
    this->AddChild(this->to_buy);
    this->AddChild(this->shop_compass);
    this->AddChild(this->buttons);

    this->gameplay = nullptr;
    this->select_info_panel = nullptr;
    this->spell_panel = nullptr;
    this->spr_myitem = nullptr;
    this->spr_shopitem = nullptr;
    this->bmp_backinvg = nullptr;
    this->bmp_backinvb = nullptr;
    this->bmp_backinvs = nullptr;
    this->snd_notif = nullptr;
    this->snd_step1 = nullptr;
    this->snd_step2 = nullptr;
    this->snd_breath = nullptr;
    this->snd_depart = nullptr;
    this->snd_buy = nullptr;
    this->snd_sell = nullptr;
    this->snd_enter = nullptr;
    this->snd_start = nullptr;
    this->snd_pov1 = nullptr;
    this->snd_pov2 = nullptr;
    this->snd_inshop = nullptr;
    this->snd_out = nullptr;
    this->snd_undo = nullptr;
}


// 4BA892
int32_t VisShop::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    case VIS_MSG_FRAME_TICK:
        if ((main_wnd->dialogsMask & DIALOG_MASK_POPUP) == 0) {
            this->UpdateShopTips();
            this->shop_compass->VMethod9();
            this->to_sell->VMethod9();
            this->assortiment->VMethod9();
            if (this->spell_panel != nullptr) {
                this->spell_panel->VMethod9();
            } else {
                this->to_buy->VMethod9();
            }
            this->select_info_panel->VMethod9();
            this->RecalcShopGold();
            this->buttons->VMethod9();
        }
        break;
    case VIS_MSG_TOGGLE_SPELL_PANEL:
        if (this->spell_panel != nullptr) {
            this->ShowBuyPanel();
        } else {
            this->ShowSpellPanel();
        }
        break;
    case VIS_MSG_REFRESH_ITEM_GRIDS:
        if (lparam == 0) {
            this->to_sell->RefreshGrid();
            this->assortiment->RefreshGrid();
            this->to_buy->RefreshGrid();
            ((CUnit*)this->gameplay->primary_selected_object)->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
            this->dirty |= 0x28;
        } else {
            switch (wparam) {
            case 1:
                ((CUnit*)this->gameplay->primary_selected_object)->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
                this->dirty |= 8;
                break;
            case 2:
                this->to_sell->VMethod40((CArray<TokenEntry*>*)lparam);
                this->to_sell->RefreshGrid();
                break;
            case 4:
                this->to_buy->VMethod40((CArray<TokenEntry*>*)lparam);
                this->to_buy->RefreshGrid();
                this->RecalcShopGold();
                this->dirty |= 0x20;
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                this->SetAssortmentCategory(wparam - 5, (CArray<TokenEntry*>*)lparam);
                this->assortiment->RefreshGrid();
                break;
            }
            ((CArray<TokenEntry*>*)lparam)->RemoveAll();
        }
        break;
    case VIS_MSG_SELECT_PREV:
        this->SelectPrevPartyUnit();
        this->dirty |= 1;
        break;
    case VIS_MSG_SELECT_NEXT:
        this->SelectNextPartyUnit();
        this->dirty |= 1;
        break;
    case VIS_MSG_CLOSE_TIPS:
        if (this->tips != nullptr) {
            if (this->shop_compass != nullptr) {
                this->shop_compass->RemoveChild(this->tips);
            }
            delete this->tips;
            this->tips = nullptr;
        }
        break;
    case VIS_MSG_SET_OPTION:
        if (wparam == 7) {
            this->to_sell->RefreshGrid();
            this->to_buy->RefreshGrid();
        } else {
            switch (wparam) {
            case 1:
            case 2:
                this->to_sell->RefreshGrid();
                break;
            case 4:
                this->to_buy->RefreshGrid();
                this->RecalcShopGold();
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                this->assortiment->RefreshGrid();
                break;
            }
        }
        break;
    }

    return VisScreen::MsgProc(msg, wparam, lparam);
}


// 4BC32B
void VisShop::VMethod28() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    g_mousept.DisableHint();
    this->buttons->ResetSelected();
    this->to_buy->VMethod41();
    this->assortiment->VMethod41();
    this->shop_compass->VMethod26();
    this->buttons->LoadBmp();
    this->LoadShopGfx();
    this->VMethod31();

    this->gameplay = main_wnd->vis_map_context;
    this->select_info_panel = main_wnd->vis_charinfo;

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        CString tips_text;
        MissionGetTips(3, &tips_text);
        this->tips = new VisTipsDialog(0x3F3, 0, 0xA2, 0x138, 0x12A, tips_text);
        this->shop_compass->AddChild(this->tips);
    } else {
        if (this->tips != nullptr) {
            this->shop_compass->RemoveChild(this->tips);
            delete this->tips;
        }
        this->tips = nullptr;
    }
    this->tips_update_flag = 0;

    main_wnd->vis_right_panel->RemoveChild(this->select_info_panel);
    CRect& panel_rect = this->select_info_panel->GetRect();
    CPoint pt(0x280 - panel_rect.Width(), 0);
    panel_rect.OffsetRect(pt.x, pt.y);
    this->select_info_panel->SetRect(&panel_rect);
    this->AddChild(this->select_info_panel);

    g_StructEnter.RebuildFromMap();
    this->selected_units.Copy(g_StructEnter.structure_list);
    this->select_index = g_StructEnter.FindFlaggedStructureUnit();

    this->to_sell->visible_startref = &this->selected_units[0]->shopInventoryVisibleStart;
    *this->assortiment->visible_startref = 0;
    this->select_category = 100;
    this->shop_compass->VMethod37(0);
    this->to_sell->VMethod32(&this->selected_units[this->select_index]->tokenEntries);
    this->assortiment->SelectCategoryGrid(0);
    this->assortiment->ClearCategoryGrids();
    this->to_buy->ClearBuyEntries();
    this->gameplay->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);
    this->selected_units[this->select_index]->SetSelected(1);
    this->gameplay->UpdateSelectionState();
    this->selected_units[this->select_index]->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
    this->to_sell->RefreshGrid();
    this->assortiment->RefreshGrid();
    this->to_buy->RefreshGrid();
    this->dirty |= 0x2F;
    this->assortiment->LoadGridSfx();
    this->to_sell->LoadGridSfx();
    this->to_buy->LoadGridSfx();

    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();

    VisScreen::VMethod28();
    this->dialog_active = 1;
    ApplyCursor(g_Cursors[0]);
    this->snd_enter->Play();
    PlayAmbientSfx(&this->snd_inshop);

    if (this->snd_start != nullptr && this->snd_start->FindPlayingChannel() == nullptr) {
        this->snd_start->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
    }

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        this->scenario_talk_target = ScenarioEnterShop();
    } else {
        this->scenario_talk_target = 0;
    }

    if (this->scenario_talk_target != 0) {
        int32_t npc_id = (this->scenario_talk_target >> 16) & 0xFFF;
        CString npc_name;
        npc_name.Format("shop\\npc31m%d", npc_id);
        ShowRoleKeyDialog(npc_name);
        ScenarioTalkTo(this->scenario_talk_target);
    }

    g_mousept.EnableHint();
}


// 4BCD79
void VisShop::ShowBuyPanel() {
    this->spell_panel = nullptr;
    this->AddChild(this->to_buy);
}


// 4BCD4B
void VisShop::ShowSpellPanel() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->spell_panel = main_wnd->vis_spellbook;
    this->RemoveChild(this->to_buy);
}


// 4BCEA4
void VisShop::ResetInfoPanelButtons() {
    this->buttons->ResetSelected();
    this->dirty &= ~0x200;
    this->dirty &= ~0x400;
    this->dirty &= ~0x80;
    this->dirty &= ~0x100;
    this->dirty |= 0x20;
}


// 4BC8D7
void VisShop::UpdateShopTips() {
    if (this->tips != nullptr) {
        if (this->to_buy->grid_source->GetSize() != 0 && this->tips_update_flag == 0) {
            this->tips_update_flag = 1;
            CString tips_text;
            MissionGetTips(4, &tips_text);
            ((VisMultiText*)this->tips->FindChild(MAP_CHILD_TIPS_TEXT))->SetText(tips_text);
        }
    }
}


// 4BCC38
// client: 4c1604
void VisShop::ReturnPendingBuyItems() {
    TokenEntry* removed = nullptr;
    this->RecalcShopGold();
#ifndef A2CLIENT
    return;
#endif
    while (this->to_buy->grid_source->GetSize() != 0) {
        TokenEntry* first = (*this->to_buy->grid_source)[0];
        removed = this->to_buy->VMethod36(0, first->count);
        if (removed != nullptr) {
            if (removed->slot_code == 2) {
                this->to_sell->VMethod37(-1);
            } else {
                this->assortiment->VMethod37(-1);
            }
        }
    }
}


// 4BD251
void VisShop::VMethod32() {
    FreeSfxSample(&this->snd_notif);
    FreeSfxSample(&this->snd_step1);
    FreeSfxSample(&this->snd_step2);
    FreeSfxSample(&this->snd_breath);
    FreeSfxSample(&this->snd_depart);
    FreeSfxSample(&this->snd_buy);
    FreeSfxSample(&this->snd_sell);
    FreeSfxSample(&this->snd_enter);
    FreeSfxSample(&this->snd_start);
    FreeSfxSample(&this->snd_pov1);
    FreeSfxSample(&this->snd_pov2);
    FreeSfxSample(&this->snd_inshop);
    FreeSfxSample(&this->snd_out);
    FreeSfxSample(&this->snd_undo);
}


// 4C6B90
CString VisShop::VMethod33() {
    return CString();
}


// 4BCDA0
void VisShop::RecalcShopGold() {
    this->current_gold = this->gameplay->my_main_unit->gold;
    this->sell_gold = 0;
    this->buy_gold = 0;
    for (int32_t i = 0; i < this->to_buy->grid_source->GetSize(); i++) {
        TokenEntry* entry = (*this->to_buy->grid_source)[i];
        if (entry->slot_code == 2) {
            this->sell_gold += (int32_t)(entry->GetAttribute(1) + 1) / 2 * entry->count;
        } else {
            this->buy_gold -= entry->GetAttribute(1) * entry->count;
        }
    }
    this->result_gold = this->current_gold + this->buy_gold + this->sell_gold;
}


// 4BB895
void VisShop::SelectNextPartyUnit() {
    this->gameplay->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);
    this->select_index++;
    if (this->select_index >= this->selected_units.GetSize()) {
        this->select_index = 0;
    }
    CUnit* unit = this->selected_units[this->select_index];
    unit->SetSelected(1);
    this->gameplay->UpdateSelectionState();
    this->to_sell->VMethod32(&this->selected_units[this->select_index]->tokenEntries);
    this->to_sell->visible_startref = &this->selected_units[this->select_index]->shopInventoryVisibleStart;
    this->to_sell->RefreshGrid();
    this->to_sell->ClampVisibleStart();
    this->selected_units[this->select_index]->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
    this->dirty |= 9;
}


// 4BBA06
void VisShop::SelectPrevPartyUnit() {
    this->gameplay->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);
    if (this->select_index != 0) {
        this->select_index--;
    } else {
        this->select_index = this->selected_units.GetUpperBound();
    }
    CUnit* unit = this->selected_units[this->select_index];
    unit->SetSelected(1);
    this->gameplay->UpdateSelectionState();
    this->to_sell->VMethod32(&this->selected_units[this->select_index]->tokenEntries);
    this->to_sell->visible_startref = &this->selected_units[this->select_index]->shopInventoryVisibleStart;
    this->to_sell->RefreshGrid();
    this->to_sell->ClampVisibleStart();
    this->selected_units[this->select_index]->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
    this->dirty |= 9;
}


// 4BC97B
void VisShop::CancelItemDrag() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    ApplyCursor(g_Cursors[0]);
    switch (main_wnd->drag_src_tab) {
    case 1:
        ((VisCharInfo*)this->select_info_panel)->VMethod27(main_wnd->drag_src_slot);
        this->dirty |= 8;
        break;
    case 2:
        this->to_sell->VMethod37(main_wnd->drag_src_slot);
        this->to_sell->RefreshGrid();
        this->dirty |= 1;
        break;
    case 4:
        this->to_buy->VMethod37(main_wnd->drag_src_slot);
        this->to_buy->RefreshGrid();
        if (this->spell_panel == nullptr) {
            this->dirty |= 4;
        }
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        this->assortiment->VMethod37(main_wnd->drag_src_slot);
        this->assortiment->RefreshGrid();
        this->dirty |= 2;
        break;
    }
    main_wnd->ResetItemCursor();
}

// 4BBBD6
void VisShop::SetAssortmentCategory(int32_t category, CArray<TokenEntry*>* arr) {
    AfxGetMainWnd();
    for (int32_t i = 0; i < arr->GetSize(); i++) {
        (*arr)[i]->slot_code = category + 5;
        (*arr)[i]->gap_0x14 = category;
    }
    CArray<TokenEntry*>& cat_arr = this->assortiment->category_grids[category];
    for (int32_t i = 0; i < cat_arr.GetSize(); i++) {
        if (cat_arr[i] != nullptr) {
            delete cat_arr[i];
        }
        cat_arr[i] = nullptr;
    }
    cat_arr.RemoveAll();
    for (int32_t i = 0; i < arr->GetSize(); i++) {
        (*arr)[i]->list_index = i;
    }
    SortTokenEntries(arr);
    cat_arr.Copy(*arr);
    arr->RemoveAll();
}


// 4BB4FB
void VisShop::FreeShopGfx() {
    if (this->spr_myitem != nullptr) {
        delete this->spr_myitem;
    }
    this->spr_myitem = nullptr;
    if (this->spr_shopitem != nullptr) {
        delete this->spr_shopitem;
    }
    this->spr_shopitem = nullptr;
    if (this->bmp_backinvg != nullptr) {
        delete this->bmp_backinvg;
    }
    this->bmp_backinvg = nullptr;
    if (this->bmp_backinvb != nullptr) {
        delete this->bmp_backinvb;
    }
    this->bmp_backinvb = nullptr;
    if (this->bmp_backinvs != nullptr) {
        delete this->bmp_backinvs;
    }
    this->bmp_backinvs = nullptr;
    for (int32_t i = 0; i < this->bmp_cost_small.GetSize(); i++) {
        if (this->bmp_cost_small[i] != nullptr) {
            delete this->bmp_cost_small[i];
        }
        this->bmp_cost_small[i] = nullptr;
        if (this->bmp_cost_medium[i] != nullptr) {
            delete this->bmp_cost_medium[i];
        }
        this->bmp_cost_medium[i] = nullptr;
    }
    this->bmp_cost_small.RemoveAll();
    this->bmp_cost_medium.RemoveAll();
}


// 4BB102
void VisShop::LoadShopGfx() {
    char fname[0x400];

    this->FreeShopGfx();
    this->spr_myitem = new CSprite256("graphics\\interface\\myitem.256");
    this->spr_myitem->ResetPalette(1, 1, 0);
    g_mousept.Update();
    this->spr_shopitem = new CSprite256("graphics\\interface\\shopitem.256");
    this->spr_shopitem->ResetPalette(1, 1, 0);
    g_mousept.Update();
    for (int32_t i = 0; i < 7; i++) {
        sprintf(fname, "graphics\\interface\\costs%d.bmp", i + 1);
        this->bmp_cost_small.Add(new CBmp64(fname));
        g_mousept.Update();
        sprintf(fname, "graphics\\interface\\costm%d.bmp", i + 1);
        this->bmp_cost_medium.Add(new CBmp64(fname));
        g_mousept.Update();
    }
    this->bmp_backinvg = new CBmp64("graphics\\interface\\backinvg.bmp");
    g_mousept.Update();
    this->bmp_backinvb = new CBmp64("graphics\\interface\\backinvb.bmp");
    g_mousept.Update();
    this->bmp_backinvs = new CBmp64("graphics\\interface\\backinvs.bmp");
    g_mousept.Update();
}


// 4BAD1D
void VisShop::DrawInfoPanelEdge() {
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    if (this->dialog_active && this->select_info_panel) {
        if (((VisCharInfo*)this->select_info_panel)->info_mode == 0) {
            g_bmp_textbackl->DrawSubRectMasked(screen_rect.left + 0x1D0, screen_rect.top + 0xEE, 0, 0, 0x10, 0xF2);
        } else {
            g_bmp_humanbackl->DrawSubRectMasked(screen_rect.left + 0x1D0, screen_rect.top + 0xEE, 0, 0, 0x10, 0xF2);
        }
    }
}


// 4BD495
VisShopCompass::VisShopCompass(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : CVisualObject(_id, l, t, r, b, nullptr) {
    CPoint topleft = this->rect.TopLeft();
    this->inner_rects[3] = CRect(topleft.x + 0x25, topleft.y + 0x14, topleft.x + 0x95, topleft.y + 0x6C);
    this->inner_rects[2] = CRect(topleft.x + 0x95, topleft.y + 0x14, topleft.x + 0x119, topleft.y + 0x6C);
    this->inner_rects[1] = CRect(topleft.x + 0x21, topleft.y + 0x6C, topleft.x + 0x71, topleft.y + 0xDC);
    this->inner_rects[0] = CRect(topleft.x + 0xBD, topleft.y + 0x6C, topleft.x + 0x10D, topleft.y + 0xDC);
    this->outer_rects[0] = CRect(topleft.x + 0xBE, topleft.y + 0x6E, topleft.x + 0x127, topleft.y + 0x127);
    this->outer_rects[1] = CRect(topleft.x + 5, topleft.y + 0x6E, topleft.x + 0x6E, topleft.y + 0x127);
    this->outer_rects[2] = CRect(topleft.x + 0x96, topleft.y + 5, topleft.x + 0x122, topleft.y + 0x69);
    this->outer_rects[3] = CRect(topleft.x + 8, topleft.y + 5, topleft.x + 0x96, topleft.y + 0x69);
    this->center_rect = CRect(topleft.x + 0x6E, topleft.y + 0x6E, topleft.x + 0xBE, topleft.y + 0x127);
    this->confirm_rect = CRect(0xDC, 0x14, 0x1C7, 0x73);
    this->confirm_yes_rect = CRect((int32_t)(this->confirm_rect.Width() * 0.25) - 9, 0x22, (int32_t)(this->confirm_rect.Width() * 0.25) + 9, 0x2E);
    this->confirm_no_rect = CRect((int32_t)(this->confirm_rect.Width() * 0.75) - 9, 0x22, (int32_t)(this->confirm_rect.Width() * 0.75) + 9, 0x2E);
    this->shop = shop;
    this->frame_sprite = 0;
    this->bkg_bmp = nullptr;
    this->idle_bmp = nullptr;
    this->state = 0;
    for (int i = 0; i < 4; ++i) {
        this->dir_frm[i] = 0;
    }
    this->center_frm = 0;
    for (int32_t i = 0; i < 4; i++) {
        for (int32_t j = 0; j < 11; j++) {
            this->direction_frames[i][j] = nullptr;
        }
    }
    for (int32_t i = 0; i < 12; i++) {
        this->fwd_frames[i] = nullptr;
    }
    for (int32_t i = 0; i < 12; i++) {
        this->ret_frames[i] = nullptr;
    }
    this->trigger_bmp = nullptr;
}


// 4BEB77
void VisShopCompass::VMethod7() {
    static uint32_t last_frame_tick = timeGetTime() - 100;
    static uint32_t idle_tick = timeGetTime();
    CPoint topleft = this->shop->rect.TopLeft();
    if (this->shop->dialog_active == 0) {
        return;
    }
    if (timeGetTime() - last_frame_tick >= 100) {
        this->shop->VMethod30();
        last_frame_tick = timeGetTime();
        for (int32_t i = 0; i < 4; i++) {
            if ((this->state & (1 << i)) != 0 && this->direction_frames[i][this->dir_frm[i]] != nullptr) {
                this->AdvanceDirectionFrames(i);
            }
        }
        if (timeGetTime() - idle_tick >= (uint32_t)(rand() % 5 * 1000 + 5000) &&
            (this->state & 0x10) == 0 && (this->state & 0x20) == 0 && (this->state & 0x40) == 0) {
            this->state |= 0x10;
        }
        if (this->state & 0x10) {
            this->VMethod30();
            if (this->center_frm == 0x1C) {
                this->state &= ~0x10;
                idle_tick = timeGetTime();
                this->center_frm = 0;
            }
        } else if (this->state & 0x20) {
            this->center_frm++;
            if (this->center_frm == 0xC) {
                this->state &= ~0x20;
                idle_tick = timeGetTime();
                this->center_frm = 0;
                this->VMethod33();
            }
        } else if (this->state & 0x40) {
            this->center_frm++;
            if (this->center_frm == 0xC) {
                this->state &= ~0x40;
                idle_tick = timeGetTime();
                this->center_frm = 0;
                this->VMethod34();
            }
        }
    }
    LockSurface2();
    if (this->frame_sprite != nullptr) {
        this->frame_sprite->Draw(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, 0);
    }
    if (this->bkg_bmp != nullptr) {
        this->bkg_bmp->Draw(topleft.x + this->rect.left + 5, topleft.y + this->rect.top + 8, 0, 0, 0);
    }
    for (int32_t i = 0; i < 4; i++) {
        if ((this->state & (1 << i)) != 0 && this->direction_frames[i][this->dir_frm[i]] != nullptr) {
            this->direction_frames[i][this->dir_frm[i]]->Draw(topleft.x + this->inner_rects[i].left, topleft.y + this->inner_rects[i].top, 0, 0, 0);
        }
    }
    if (this->state & 0x10) {
        this->trigger_bmp->Draw(topleft.x + this->rect.left + 0x71, topleft.y + this->rect.top + 0x70, 0, 0, 0);
    } else if (this->state & 0x20) {
        this->fwd_frames[this->center_frm]->Draw(topleft.x + this->rect.left + 0x71, topleft.y + this->rect.top + 0x70, 0, 0, 0);
    } else if (this->state & 0x40) {
        this->ret_frames[this->center_frm]->Draw(topleft.x + this->rect.left + 0x71, topleft.y + this->rect.top + 0x70, 0, 0, 0);
    } else if (this->idle_bmp != nullptr) {
        this->idle_bmp->Draw(topleft.x + this->rect.left + 0x71, topleft.y + this->rect.top + 0x70, 0, 0, 0);
    }
    if (this->state & 0x80) {
        CPoint pt(g_mousept.GetX(), g_mousept.GetY());
        CRect dlg_rect(pt.x + this->confirm_rect.left, pt.y + this->confirm_rect.top, pt.x + this->confirm_rect.right, pt.y + this->confirm_rect.bottom);
        ShadowRect(dlg_rect, 8);
        g_font2->DrawTxt(pt.x + this->confirm_rect.left + this->confirm_rect.Width() / 2, pt.y + this->confirm_rect.top + 5, TxtFile::AllLines[0x4F], 2, clrsh_ShockingBlack);
        g_font2->DrawTxt(pt.x + this->confirm_rect.left + this->confirm_rect.Width() / 2, pt.y + this->confirm_rect.top + 0xF, TxtFile::AllLines[0x50], 2, clrsh_ShockingBlack);
        CRect yes_rect = this->confirm_yes_rect + this->confirm_rect.TopLeft() + pt;
        uint16_t* color = yes_rect.PtInRect(pt) ? clrsh_DullGold : clrsh_ShockingBlack;
        g_font2->DrawTxt(yes_rect.left + yes_rect.Width() / 2, yes_rect.top + 1, "Yes", 2, color);
        CRect no_rect = this->confirm_no_rect + this->confirm_rect.TopLeft() + pt;
        color = no_rect.PtInRect(pt) ? clrsh_DullGold : clrsh_ShockingBlack;
        g_font2->DrawTxt(no_rect.left + no_rect.Width() / 2, no_rect.top + 1, "No", 2, color);
    }
    UnlockSurface2();
    CVisualObject::VMethod7();
}


// 4BF748
// client: 4c410b
int32_t VisShopCompass::OnLButtonDown(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CPoint topleft = this->shop->rect.TopLeft();
    if (this->state & 0x80) {
        CRect yes_rect = this->confirm_yes_rect + this->confirm_rect.TopLeft() + topleft;
        CRect no_rect = this->confirm_no_rect + this->confirm_rect.TopLeft() + topleft;
        if (yes_rect.PtInRect(pos) || no_rect.PtInRect(pos)) {
            this->state &= ~0x80;
            this->shop->CancelItemDrag();
            main_wnd->ResetItemCursor();
        }
    } else {
        for (int32_t i = 0; i < 4; i++) {
            CRect hit_rect = this->outer_rects[i] + topleft;
            if (hit_rect.PtInRect(pos) && this->VMethod37(i) != 0) {
                this->state |= 0x20;
                this->VMethod28();
                this->shop->assortiment->SelectCategoryGrid((uint16_t)this->shop->select_category);
                this->shop->assortiment->VMethod9();
                break;
            }
        }
    }
    return 0;
}


// 4BEA64
const char* VisShopCompass::GetHint() {
    if (this->shop->dialog_active == 0) {
        return nullptr;
    }
    CPoint topleft = this->shop->rect.TopLeft();
    CPoint pt(g_mousept.GetX() - topleft.x, g_mousept.GetY() - topleft.y);
    if (this->state & 0x80) {
        return nullptr;
    }
    for (int32_t i = 0; i < 4; i++) {
        if (this->outer_rects[i].PtInRect(pt)) {
            return TxtFile::AllLines[i + 0x3E];
        }
    }
    if (this->center_rect.PtInRect(pt)) {
        return TxtFile::AllLines[0x3D];
    }
    return nullptr;
}


// 4BE238
void VisShopCompass::VMethod26() {
    this->VMethod27();
    this->frame_sprite = new CSprite256("graphics\\interface\\ShopFrame.256");
    this->frame_sprite->ResetPalette(1, 1, 0);
    g_mousept.Update();
    this->bkg_bmp = new CBmp64("graphics\\interface\\shopanim\\ShopMain.bmp");
    this->idle_bmp = new CBmp64("movies\\shopanim\\Pose2-3\\1.bmp");
    g_mousept.Update();
}


// 4BE5DE
void VisShopCompass::VMethod28() {
    char fname[52];

    this->VMethod33();
    this->fwd_frames[0] = new CBmp64("movies\\shopanim\\Pose2-3\\1.bmp");
    for (int32_t i = 1; i < 12; i++) {
        sprintf(fname, "movies\\shopanim\\Yes\\%d.bmp", i + 1);
        this->fwd_frames[i] = new CBmp64(fname);
    }
}


// 4BE6DF
void VisShopCompass::VMethod29() {
    char fname[52];

    this->VMethod34();
    this->ret_frames[0] = new CBmp64("movies\\shopanim\\Pose2-3\\1.bmp");
    for (int32_t i = 1; i < 12; i++) {
        sprintf(fname, "movies\\shopanim\\No\\%d.bmp", i + 1);
        this->ret_frames[i] = new CBmp64(fname);
    }
}


// 4BF4E8
int32_t VisShopCompass::VMethod37(int32_t arg) {
    if ((uint16_t)this->shop->select_category == (uint16_t)arg) {
        return 0;
    }
    StopSfx(&this->shop->snd_depart);
    CSound::Play((CSound&)this->shop->snd_depart);
    if (this->shop->select_category == 100) {
        this->state = 0;
        this->dir_frm[3] = 0;
        this->dir_frm[2] = 0;
        this->dir_frm[1] = 0;
        this->dir_frm[0] = 0;
        this->VMethod36();
    } else {
        this->dir_frm[(uint16_t)this->shop->select_category] = 9;
    }
    this->state |= 1 << (arg & 0x1F);
    this->VMethod31((uint16_t)arg);
    this->dir_frm[(uint16_t)arg] = 0;
    this->shop->select_category = arg;
    *this->shop->assortiment->visible_startref = 0;
    return 1;
}


// 4BE45D
void VisShopCompass::VMethod31(int32_t arg) {
    char fname[52];

    this->VMethod32(arg);
    for (int32_t i = 0; i < 11; i++) {
        sprintf(fname, "graphics\\interface\\shopanim\\%.2d\\%d.bmp", 4 - arg, i + 1);
        this->direction_frames[arg][i] = new CBmp64(fname);
    }
    this->dir_frm[arg] = 0;
}


// 4BE372
void VisShopCompass::VMethod27() {
    if (this->frame_sprite != nullptr) {
        delete this->frame_sprite;
    }
    if (this->bkg_bmp != nullptr) {
        delete this->bkg_bmp;
    }
    if (this->idle_bmp != nullptr) {
        delete this->idle_bmp;
    }
    this->frame_sprite = nullptr;
    this->bkg_bmp = nullptr;
    this->idle_bmp = nullptr;
}


// 4BE7E0
void VisShopCompass::VMethod30() {
    char fname[52];

    this->VMethod35();
    this->center_frm = (this->center_frm + 1) % 30;
    sprintf(fname, "movies\\shopanim\\Pose2-3\\%d.bmp", this->center_frm + 1);
    this->trigger_bmp = new CBmp64(fname);
}


// 4BE541
void VisShopCompass::VMethod32(int32_t arg) {
    for (int32_t i = 0; i < 11; i++) {
        if (this->direction_frames[arg][i] != nullptr) {
            delete this->direction_frames[arg][i];
        }
        this->direction_frames[arg][i] = nullptr;
    }
}


// 4BE8B0
void VisShopCompass::VMethod33() {
    for (int32_t i = 0; i < 12; i++) {
        if (this->fwd_frames[i] != nullptr) {
            delete this->fwd_frames[i];
        }
        this->fwd_frames[i] = nullptr;
    }
}


// 4BE92D
void VisShopCompass::VMethod34() {
    for (int32_t i = 0; i < 12; i++) {
        if (this->ret_frames[i] != nullptr) {
            delete this->ret_frames[i];
        }
        this->ret_frames[i] = nullptr;
    }
}


// 4BEA01
void VisShopCompass::VMethod36() {
    for (int32_t i = 0; i < 4; i++) {
        this->VMethod32(i);
    }
    this->VMethod33();
    this->VMethod34();
    this->VMethod35();
}


// 4BF6CE
int32_t VisShopCompass::OnMouseMove(uint32_t wparam, CPoint pos) {
    if (this->shop->hovered_region != this->VMethod38()) {
        this->shop->hovered_region = this->VMethod38();
        this->shop->buttons->ResetSelected();
        this->shop->dirty |= 0x2F;
    }
    return 0;
}


// 4BE9AA
void VisShopCompass::VMethod35() {
    if (this->trigger_bmp != nullptr) {
        delete this->trigger_bmp;
    }
    this->trigger_bmp = nullptr;
}


// 4BE1DF
VisShopCompass::~VisShopCompass() {
    this->VMethod27();
    this->VMethod36();
}


// 4BFA01
int32_t VisShopCompass::OnLButtonUp(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        this->shop->placement_lock = 0;
        ApplyCursor(g_Cursors[0]);
        this->shop->CancelItemDrag();
        main_wnd->ResetItemCursor();
    }
    return 1;
}


// 4C6BC0
int32_t VisShopCompass::VMethod38() {
    return 0x64;
}

// 4bf63f
void VisShopCompass::AdvanceDirectionFrames(int32_t dir) {
    ++this->dir_frm[dir];

    if (this->dir_frm[dir] == 9) {
        this->dir_frm[dir] = 3;
    } else if (this->dir_frm[dir] == 10) {
        this->state &= ~(1 << dir);
        this->VMethod32(dir);
    }
}


// 4C247B
VisShopCompassDruid::VisShopCompassDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisShopCompass(_id, l, t, r, b, shop) {
    CPoint topleft = this->rect.TopLeft();
    this->outer_rects[0] = CRect(CPoint(topleft.x, topleft.y + 0x68), CSize(0x58, 0x70));
    this->outer_rects[1] = CRect(CPoint(topleft.x, topleft.y + 0x2C), CSize(0x60, 0x3C));
    this->outer_rects[2] = CRect(CPoint(topleft.x + 0x74, topleft.y + 0x1C), CSize(0x48, 0x70));
    this->outer_rects[3] = CRect(CPoint(topleft.x + 0x58, topleft.y + 0xA4), CSize(0x60, 0x68));
    for (int32_t i = 0; i < 4; i++) {
        this->outer_rects[i].OffsetRect(5, 5);
    }
    this->center_rect = CRect(CPoint(topleft.x + 0xC0, topleft.y + 0x54), CSize(0x48, 0xA4));
    this->center_rect.OffsetRect(5, 5);
    this->armor_bmp = nullptr;
    this->magic_bmp = nullptr;
    this->potions_bmp = nullptr;
    this->sel_goods_bmp = nullptr;
    this->category_bmp = nullptr;
}


// 4C30B4
void VisShopCompassDruid::VMethod7() {
    static uint32_t last_frame_tick = timeGetTime() - 100;
    static uint32_t idle_tick = timeGetTime();
    CPoint topleft = this->shop->rect.TopLeft();
    if (this->shop->dialog_active == 0) {
        return;
    }
    if (timeGetTime() - last_frame_tick >= 100) {
        this->shop->VMethod30();
        last_frame_tick = timeGetTime();
        if (timeGetTime() - idle_tick >= (uint32_t)(rand() % 5 * 1000 + 5000) &&
            (this->state & 0x10) == 0 && (this->state & 0x20) == 0) {
            this->state |= 0x10 << (GetRandS16(2) & 0x1F);
        }
        if (this->state & 0x30) {
            this->VMethod30();
            if (this->center_frm == 0) {
                idle_tick = timeGetTime();
            }
        }
    }
    LockSurface2();
    if (this->frame_sprite != nullptr) {
        this->frame_sprite->Draw(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, 0);
    }
    if (this->bkg_bmp != nullptr) {
        this->bkg_bmp->Draw(topleft.x + this->rect.left + 5, topleft.y + this->rect.top + 8, 0, 0, 0);
    }
    if (this->category_bmp != nullptr && this->IsDruidShopAvailable() != 0) {
        if (this->shop->select_category == 3) {
            this->sel_goods_bmp->Draw(topleft.x + this->rect.left + 0x5D, topleft.y + this->rect.top + 0xAC, 0, 0, 0);
        } else {
            this->category_bmp->Draw(topleft.x + this->rect.left + 0x5D, topleft.y + this->rect.top + 0xAC, 0, 0, 0);
        }
    }
    if (this->shop->select_category == 0) {
        this->armor_bmp->Draw(topleft.x + this->rect.left + 5, topleft.y + this->rect.top + 0x70, 0, 0, 0);
    }
    if (this->shop->select_category == 1) {
        this->magic_bmp->Draw(topleft.x + this->rect.left + 5, topleft.y + this->rect.top + 0x34, 0, 0, 0);
    }
    if (this->shop->select_category == 2) {
        this->potions_bmp->Draw(topleft.x + this->rect.left + 0x79, topleft.y + this->rect.top + 0x24, 0, 0, 0);
    }
    if ((this->state & 0x30) == 0) {
        if (this->idle_bmp != nullptr) {
            this->idle_bmp->Draw(topleft.x + this->rect.left + 0xC5, topleft.y + this->rect.top + 0x5C, 0, 0, 0);
        }
    } else if (this->trigger_bmp != nullptr) {
        this->trigger_bmp->Draw(topleft.x + this->rect.left + 0xC5, topleft.y + this->rect.top + 0x5C, 0, 0, 0);
    }
    UnlockSurface2();
    CVisualObject::VMethod7();
}


// 4C2882
void VisShopCompassDruid::VMethod26() {
    this->VMethod27();
    this->frame_sprite = new CSprite256("graphics\\interface\\shop_druid\\ShopFrame.256");
    this->frame_sprite->ResetPalette(1, 1, 0);
    g_mousept.Update();
    this->bkg_bmp = new CBmp64("graphics\\interface\\shop_druid\\ShopMain.bmp");
    this->idle_bmp = new CBmp64("movies\\shop_druid\\a10001.bmp");
    this->armor_bmp = new CBmp64("graphics\\interface\\shop_druid\\hili_armor.bmp");
    this->magic_bmp = new CBmp64("graphics\\interface\\shop_druid\\hili_magic.bmp");
    this->potions_bmp = new CBmp64("graphics\\interface\\shop_druid\\hili_potion.bmp");
    this->sel_goods_bmp = new CBmp64("graphics\\interface\\shop_druid\\hili_elven.bmp");
    this->category_bmp = new CBmp64("graphics\\interface\\shop_druid\\elven.bmp");
    g_mousept.Update();
}


// 4C2B29
void VisShopCompassDruid::VMethod27() {
    if (this->frame_sprite != nullptr) {
        delete this->frame_sprite;
    }
    if (this->bkg_bmp != nullptr) {
        delete this->bkg_bmp;
    }
    if (this->idle_bmp != nullptr) {
        delete this->idle_bmp;
    }
    if (this->category_bmp != nullptr) {
        delete this->category_bmp;
    }
    this->frame_sprite = nullptr;
    this->bkg_bmp = nullptr;
    this->idle_bmp = nullptr;
    this->category_bmp = nullptr;
    if (this->armor_bmp != nullptr) {
        delete this->armor_bmp;
    }
    this->armor_bmp = nullptr;
    if (this->magic_bmp != nullptr) {
        delete this->magic_bmp;
    }
    this->magic_bmp = nullptr;
    if (this->potions_bmp != nullptr) {
        delete this->potions_bmp;
    }
    this->potions_bmp = nullptr;
    if (this->sel_goods_bmp != nullptr) {
        delete this->sel_goods_bmp;
    }
    this->sel_goods_bmp = nullptr;
}


// 4C2DBC
void VisShopCompassDruid::VMethod30() {
    char fname[52];

    this->VMethod35();
    this->center_frm = (this->center_frm + 1) % 30;
    if (this->center_frm == 0) {
        this->state &= 0xf;
    }
    if ((this->state & 0x10) == 0) {
        if ((this->state & 0x20) == 0) {
            sprintf(fname, "movies\\shop_druid\\a10001.bmp");
        } else {
            sprintf(fname, "movies\\shop_druid\\a2%04d.bmp", this->center_frm + 1);
        }
    } else {
        sprintf(fname, "movies\\shop_druid\\a1%04d.bmp", this->center_frm + 1);
    }
    this->trigger_bmp = new CBmp64(fname);
}


// 4C3690
const char* VisShopCompassDruid::GetHint() {
    if (this->shop->dialog_active == 0) {
        return nullptr;
    }
    CPoint topleft = this->shop->rect.TopLeft();
    CPoint pt(g_mousept.GetX() - topleft.x, g_mousept.GetY() - topleft.y);
    if (this->state & 0x80) {
        return nullptr;
    }
    for (int32_t i = 0; i < 4; i++) {
        if (this->outer_rects[i].PtInRect(pt)) {
            return TxtFile::AllLines[i + 0x112];
        }
    }
    if (this->center_rect.PtInRect(pt)) {
        return TxtFile::AllLines[0x3D];
    }
    return nullptr;
}


// 4C4693
VisShopCompassKaarg::VisShopCompassKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisShopCompass(_id, l, t, r, b, shop) {
    CPoint topleft = this->rect.TopLeft();
    this->outer_rects[0] = CRect(CPoint(topleft.x, topleft.y), CSize(0x3C, 0xCA));
    this->outer_rects[1] = CRect(CPoint(topleft.x + 0x3C, topleft.y + 0x3A), CSize(0x30, 0x96));
    this->outer_rects[2] = CRect(CPoint(topleft.x + 0x3C, topleft.y), CSize(0x7C, 0x3A));
    this->outer_rects[3] = CRect(CPoint(topleft.x + 0xBE, topleft.y), CSize(0x62, 0x104));
    for (int32_t i = 0; i < 4; i++) {
        this->outer_rects[i].OffsetRect(5, 5);
    }
    this->center_rect = CRect(CPoint(topleft.x + 0x78, topleft.y + 0x6C), CSize(0x48, 0x60));
    this->center_rect.OffsetRect(5, 5);
    this->armor_bmp = nullptr;
    this->magic_bmp = nullptr;
    this->scrolls_bmp = nullptr;
    this->weapons_bmp = nullptr;
    for (int32_t i = 0; i < 18; i++) {
        this->tail_frames[i] = nullptr;
    }
    for (int32_t i = 0; i < 18; i++) {
        this->wpn_tail_frames[i] = nullptr;
    }
}


// 4C4A78
void VisShopCompassKaarg::VMethod26() {
    this->VMethod27();
    this->frame_sprite = new CSprite256("graphics\\interface\\shop_kaarg\\ShopFrame.256");
    this->frame_sprite->ResetPalette(1, 1, 0);
    g_mousept.Update();
    this->bkg_bmp = new CBmp64("graphics\\interface\\shop_kaarg\\ShopMain.bmp");
    this->idle_bmp = new CBmp64("movies\\shop_kaarg\\a10000.bmp");
    this->armor_bmp = new CBmp64("graphics\\interface\\shop_kaarg\\hili_armor.bmp");
    this->magic_bmp = new CBmp64("graphics\\interface\\shop_kaarg\\hili_magic.bmp");
    this->scrolls_bmp = new CBmp64("graphics\\interface\\shop_kaarg\\hili_potion.bmp");
    this->weapons_bmp = new CBmp64("graphics\\interface\\shop_kaarg\\hili_weapon.bmp");
    for (int32_t i = 0; i < 6; i++) {
        this->tail_frames[i] = new CBmp64(CString("graphics\\interface\\shop_kaarg\\fire\\dark\\lite") + (char)('1' + i) + ".bmp");
        this->wpn_tail_frames[i] = new CBmp64(CString("graphics\\interface\\shop_kaarg\\fire\\select\\lite") + (char)('1' + i) + ".bmp");
    }
    for (int32_t i = 0; i < 6; i++) {
        this->tail_frames[i + 6] = new CBmp64(CString("graphics\\interface\\shop_kaarg\\fire\\dark\\burn") + (char)('1' + i) + ".bmp");
        this->wpn_tail_frames[i + 6] = new CBmp64(CString("graphics\\interface\\shop_kaarg\\fire\\select\\burn") + (char)('1' + i) + ".bmp");
    }
    for (int32_t i = 0; i < 6; i++) {
        this->tail_frames[i + 12] = new CBmp64(CString("graphics\\interface\\shop_kaarg\\fire\\dark\\cicle") + (char)('1' + i) + ".bmp");
        this->wpn_tail_frames[i + 12] = new CBmp64(CString("graphics\\interface\\shop_kaarg\\fire\\select\\cicle") + (char)('1' + i) + ".bmp");
    }
    g_mousept.Update();
    this->tail_frame = 0;
    this->wpn_tail_frame = 0;
}


// 4C608E
void VisShopCompassKaarg::VMethod7() {
    static uint32_t last_frame_tick = timeGetTime() - 100;
    static uint32_t idle_tick = timeGetTime();
    CPoint topleft = this->shop->rect.TopLeft();
    if (this->shop->dialog_active == 0) {
        return;
    }
    if (timeGetTime() - last_frame_tick >= 100) {
        this->shop->VMethod30();
        last_frame_tick = timeGetTime();
        if (timeGetTime() - idle_tick >= (uint32_t)(rand() % 5 * 1000 + 5000) &&
            (this->state & 0x10) == 0 && (this->state & 0x20) == 0) {
            this->state |= 0x10 << (GetRandS16(2) & 0x1F);
        }
        if (this->state & 0x30) {
            this->VMethod30();
            if (this->center_frm == 0) {
                idle_tick = timeGetTime();
            }
        }
        if (this->wpn_tail_frame == 0) {
            this->tail_frame++;
            if (this->tail_frame == 6) {
                this->tail_frame = 0;
            }
            if (0x11 < this->tail_frame) {
                this->tail_frame = 0xc;
            }
        } else {
            this->tail_frame += this->wpn_tail_frame;
            if (this->tail_frame < 1) {
                this->wpn_tail_frame = 0;
            }
            if (0x10 < this->tail_frame) {
                this->wpn_tail_frame = 0;
            }
        }
    }
    LockSurface2();
    if (this->frame_sprite != nullptr) {
        this->frame_sprite->Draw(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, 0);
    }
    if (this->bkg_bmp != nullptr) {
        this->bkg_bmp->Draw(topleft.x + this->rect.left + 5, topleft.y + this->rect.top + 8, 0, 0, 0);
    }
    if (this->shop->select_category == 0) {
        this->armor_bmp->Draw(topleft.x + this->rect.left + 5, topleft.y + this->rect.top + 8, 0, 0, 0);
    }
    if (this->shop->select_category == 1) {
        this->magic_bmp->Draw(topleft.x + this->rect.left + 0x31, topleft.y + this->rect.top + 0x48, 0, 0, 0);
    }
    if (this->shop->select_category == 2) {
        this->scrolls_bmp->Draw(topleft.x + this->rect.left + 0x31, topleft.y + this->rect.top + 8, 0, 0, 0);
    }
    if (this->shop->select_category == 3) {
        this->weapons_bmp->Draw(topleft.x + this->rect.left + 0x99, topleft.y + this->rect.top + 8, 0, 0, 0);
    }
    if ((this->state & 0x30) == 0) {
        if (this->idle_bmp != nullptr) {
            this->idle_bmp->Draw(topleft.x + this->rect.left + 0x7D, topleft.y + this->rect.top + 0x74, 0, 0, 0);
        }
    } else if (this->trigger_bmp != nullptr) {
        this->trigger_bmp->Draw(topleft.x + this->rect.left + 0x7D, topleft.y + this->rect.top + 0x74, 0, 0, 0);
    }
    if (this->shop->select_category == 3) {
        this->wpn_tail_frames[this->tail_frame]->Draw(topleft.x + this->rect.left + 0xC1, topleft.y + this->rect.top + 0xCC, 0, 0, 0);
    } else {
        this->tail_frames[this->tail_frame]->Draw(topleft.x + this->rect.left + 0xC1, topleft.y + this->rect.top + 0xCC, 0, 0, 0);
    }
    UnlockSurface2();
    CVisualObject::VMethod7();
}


// 4C5A80
void VisShopCompassKaarg::VMethod27() {
    if (this->frame_sprite != nullptr) {
        delete this->frame_sprite;
    }
    if (this->bkg_bmp != nullptr) {
        delete this->bkg_bmp;
    }
    if (this->idle_bmp != nullptr) {
        delete this->idle_bmp;
    }
    this->frame_sprite = nullptr;
    this->bkg_bmp = nullptr;
    this->idle_bmp = nullptr;
    if (this->armor_bmp != nullptr) {
        delete this->armor_bmp;
    }
    this->armor_bmp = nullptr;
    if (this->magic_bmp != nullptr) {
        delete this->magic_bmp;
    }
    this->magic_bmp = nullptr;
    if (this->scrolls_bmp != nullptr) {
        delete this->scrolls_bmp;
    }
    this->scrolls_bmp = nullptr;
    if (this->weapons_bmp != nullptr) {
        delete this->weapons_bmp;
    }
    this->weapons_bmp = nullptr;
    for (int32_t i = 0; i < 18; i++) {
        if (this->tail_frames[i] != nullptr) {
            delete this->tail_frames[i];
        }
        this->tail_frames[i] = nullptr;
        if (this->wpn_tail_frames[i] != nullptr) {
            delete this->wpn_tail_frames[i];
        }
        this->wpn_tail_frames[i] = nullptr;
    }
}


// 4C5D96
void VisShopCompassKaarg::VMethod30() {
    char fname[52];

    this->VMethod35();
    this->center_frm = (this->center_frm + 1) % 20;
    if (this->center_frm == 0) {
        this->state &= 0xf;
    }
    if ((this->state & 0x10) == 0) {
        if ((this->state & 0x20) == 0) {
            sprintf(fname, "movies\\shop_kaarg\\a10000.bmp");
        } else {
            sprintf(fname, "movies\\shop_kaarg\\a2%04d.bmp", this->center_frm + 1);
        }
    } else {
        sprintf(fname, "movies\\shop_kaarg\\a1%04d.bmp", this->center_frm + 1);
    }
    this->trigger_bmp = new CBmp64(fname);
}


// 4C6800
const char* VisShopCompassKaarg::GetHint() {
    if (this->shop->dialog_active == 0) {
        return nullptr;
    }
    CPoint topleft = this->shop->rect.TopLeft();
    CPoint pt(g_mousept.GetX() - topleft.x, g_mousept.GetY() - topleft.y);
    if (this->state & 0x80) {
        return nullptr;
    }
    for (int32_t i = 0; i < 4; i++) {
        if (this->outer_rects[i].PtInRect(pt)) {
            return TxtFile::AllLines[i + 0x116];
        }
    }
    if (this->center_rect.PtInRect(pt)) {
        return TxtFile::AllLines[0x3D];
    }
    return nullptr;
}


// 4C65BA
int32_t VisShopCompassKaarg::VMethod37(int32_t arg) {
    if ((uint16_t)this->shop->select_category == (uint16_t)arg) {
        return 0;
    }
    StopSfx(&this->shop->snd_depart);
    CSound::Play((CSound&)this->shop->snd_depart);
    if (this->shop->select_category == 100) {
        this->state = 0;
        this->dir_frm[3] = 0;
        this->dir_frm[2] = 0;
        this->dir_frm[1] = 0;
        this->dir_frm[0] = 0;
    } else {
        this->dir_frm[(uint16_t)this->shop->select_category] = 9;
    }
    this->dir_frm[(uint16_t)arg] = 0;
    if (this->shop->select_category != 100) {
        if ((uint16_t)arg == 3) {
            if (this->tail_frame < 0xc) {
                this->wpn_tail_frame = 1;
            }
        } else if (0 < this->tail_frame) {
            this->wpn_tail_frame = -1;
        }
    }
    this->shop->select_category = arg;
    *this->shop->assortiment->visible_startref = 0;
    return 1;
}


// 4C359D
int32_t VisShopCompassDruid::OnLButtonDown(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CPoint topleft = this->shop->rect.TopLeft();
    for (int32_t i = 0; i < 4; i++) {
        CRect hit_rect = this->outer_rects[i] + topleft;
        if (hit_rect.PtInRect(pos) && (i != 3 || this->IsDruidShopAvailable() != 0) && this->VMethod37(i) != 0) {
            this->shop->assortiment->SelectCategoryGrid((uint16_t)this->shop->select_category);
            this->shop->assortiment->VMethod9();
            break;
        }
    }
    return 0;
}


// 4C34E6
int32_t VisShopCompassDruid::VMethod37(int32_t arg) {
    if ((uint16_t)arg == 3 && this->IsDruidShopAvailable() == 0) {
        return 0;
    }
    int32_t result = 0;
    if ((uint16_t)this->shop->select_category != (uint16_t)arg) {
        StopSfx(&this->shop->snd_depart);
        CSound::Play((CSound&)this->shop->snd_depart);
        this->dir_frm[(uint16_t)arg] = 0;
        this->shop->select_category = arg;
        *this->shop->assortiment->visible_startref = 0;
        result = 1;
    }
    return result;
}


// 4C2CED
void VisShopCompassDruid::VMethod32(int32_t arg) {
    for (int32_t i = 0; i < 11; i++) {
        if (this->direction_frames[arg][i] != nullptr) {
            delete this->direction_frames[arg][i];
        }
        this->direction_frames[arg][i] = nullptr;
    }
}


// 4C2F00
void VisShopCompassDruid::VMethod33() {
    for (int32_t i = 0; i < 12; i++) {
        if (this->fwd_frames[i] != nullptr) {
            delete this->fwd_frames[i];
        }
        this->fwd_frames[i] = nullptr;
    }
}


// 4C2F7D
void VisShopCompassDruid::VMethod34() {
    for (int32_t i = 0; i < 12; i++) {
        if (this->ret_frames[i] != nullptr) {
            delete this->ret_frames[i];
        }
        this->ret_frames[i] = nullptr;
    }
}


// 4C2FFA
void VisShopCompassDruid::VMethod35() {
    if (this->trigger_bmp != nullptr) {
        delete this->trigger_bmp;
    }
    this->trigger_bmp = nullptr;
}


// 4C3051
void VisShopCompassDruid::VMethod36() {
    for (int32_t i = 0; i < 4; i++) {
        this->VMethod32(i);
    }
    this->VMethod33();
    this->VMethod34();
    this->VMethod35();
}


// 4C2CCE
void VisShopCompassDruid::VMethod31(int32_t arg) {
    this->VMethod32(arg);
}


// 4C2D8A
void VisShopCompassDruid::VMethod28() {
    this->VMethod33();
}


// 4C2DA3
void VisShopCompassDruid::VMethod29() {
    this->VMethod34();
}


// 4C6A30
VisShopCompassDruid::~VisShopCompassDruid() {
}


// 4C2842
int32_t VisShopCompassDruid::IsDruidShopAvailable() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        if (ScenarioGetVar(0x302) != 0) {
            return 1;
        }
        return 0;
    }
    return 1;
}


// 4C671F
int32_t VisShopCompassKaarg::OnLButtonDown(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CPoint topleft = this->shop->rect.TopLeft();
    for (int32_t i = 0; i < 4; i++) {
        CRect hit_rect = this->outer_rects[i] + topleft;
        if (hit_rect.PtInRect(pos) && this->VMethod37(i) != 0) {
            this->shop->assortiment->SelectCategoryGrid((uint16_t)this->shop->select_category);
            this->shop->assortiment->VMethod9();
            break;
        }
    }
    return 0;
}


// 4C5CC7
void VisShopCompassKaarg::VMethod32(int32_t arg) {
    for (int32_t i = 0; i < 11; i++) {
        if (this->direction_frames[arg][i] != nullptr) {
            delete this->direction_frames[arg][i];
        }
        this->direction_frames[arg][i] = nullptr;
    }
}


// 4C5EDA
void VisShopCompassKaarg::VMethod33() {
    for (int32_t i = 0; i < 12; i++) {
        if (this->fwd_frames[i] != nullptr) {
            delete this->fwd_frames[i];
        }
        this->fwd_frames[i] = nullptr;
    }
}


// 4C5F57
void VisShopCompassKaarg::VMethod34() {
    for (int32_t i = 0; i < 12; i++) {
        if (this->ret_frames[i] != nullptr) {
            delete this->ret_frames[i];
        }
        this->ret_frames[i] = nullptr;
    }
}


// 4C5FD4
void VisShopCompassKaarg::VMethod35() {
    if (this->trigger_bmp != nullptr) {
        delete this->trigger_bmp;
    }
    this->trigger_bmp = nullptr;
}


// 4C602B
void VisShopCompassKaarg::VMethod36() {
    for (int32_t i = 0; i < 4; i++) {
        this->VMethod32(i);
    }
    this->VMethod33();
    this->VMethod34();
    this->VMethod35();
}


// 4C5CA8
void VisShopCompassKaarg::VMethod31(int32_t arg) {
    this->VMethod32(arg);
}


// 4C5D64
void VisShopCompassKaarg::VMethod28() {
    this->VMethod33();
}


// 4C5D7D
void VisShopCompassKaarg::VMethod29() {
    this->VMethod34();
}


// 4C6AD0
VisShopCompassKaarg::~VisShopCompassKaarg() {
}


// 4BFA5A
VisShopButtons::VisShopButtons(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : CVisualObject(_id, l, t, r, b, nullptr) {
    this->shop = shop;
    this->outer_rects[0] = CRect(0x1EE, 0x0F, 0x266, 0x43);
    this->outer_rects[1] = CRect(0x1E3, 0x43, 0x26F, 0x71);
    this->outer_rects[2] = CRect(0x1E3, 0x72, 0x26F, 0xA0);
    this->outer_rects[3] = CRect(0x1EE, 0xA0, 0x266, 0xD4);
    this->inner_rects[0] = CRect(0x203, 0x0F, 0x24E, 0x23);
    this->inner_rects[1] = CRect(0x1EE, 0x23, 0x27B, 0x43);
    this->inner_rects[2] = CRect(0x1EE, 0xA0, 0x266, 0xC0);
    this->inner_rects[3] = CRect(0x203, 0xC0, 0x24E, 0xD4);
    this->menu_bmp = nullptr;
    this->button_bmps[3] = nullptr;
    this->button_bmps[2] = nullptr;
    this->button_bmps[1] = nullptr;
    this->button_bmps[0] = nullptr;
    this->pressed_btn = -1;
    this->hovered_btn = -1;
}


// 4C0020
VisShopButtons::~VisShopButtons() {
    this->shop = nullptr;
    this->ReleaseBmp();
    this->pressed_btn = -1;
}


// 4C6BD0
int32_t VisShopButtons::VMethod30() {
    return 0x65;
}


// 4C10C4
int32_t VisShopButtons::OnMouseMove(uint32_t wparam, CPoint pos) {
    if (this->shop->hovered_region != this->VMethod30()) {
        this->shop->hovered_region = this->VMethod30();
        this->shop->dirty |= 0x2F;
    }
    this->UpdateHoveredState(wparam, pos);
    return 0;
}


// 4C1134
int32_t VisShopButtons::OnLButtonDown(uint32_t wparam, CPoint pos) {
    this->pressed_btn = this->GetButtonAt(pos);
    this->shop->dirty |= 0x20;
    switch (this->pressed_btn) {
    case 0:
        this->shop->snd_undo->Play();
        break;
    case 3:
        this->shop->snd_out->Play();
        break;
    }
    return 1;
}


// 4C11C2
int32_t VisShopButtons::OnLButtonUp(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        this->shop->CancelItemDrag();
        this->pressed_btn = -1;
        this->UpdateHoveredState(wparam, pos);
        this->shop->dirty |= 0x20;
        return 1;
    }
    if (this->pressed_btn >= 0 && this->pressed_btn < 4 && this->GetButtonAt(pos) == this->pressed_btn) {
        int32_t button = this->pressed_btn;
        this->pressed_btn = -1;
        this->UpdateHoveredState(wparam, pos);
        switch (button) {
        case 0:
            this->shop->CancelAllPendingBuys();
            break;
        case 1:
            this->shop->CommitBuy();
            break;
        case 2:
            this->shop->CommitSell();
            break;
        case 3:
            this->shop->CloseShopDialog();
            if (main_wnd->dialogsMask == DIALOG_MASK_SHOP) {
                main_wnd->PostMessageA(MAIN_MSG_SHOW_CURRENT_TOWN, 0, 0);
            }
            break;
        }
    }
    this->shop->dirty |= 0x20;
    this->pressed_btn = -1;
    this->UpdateHoveredState(wparam, pos);
    return 1;
}


// 4C04B1
void VisShopButtons::VMethod7() {
    CPoint topleft = this->shop->rect.TopLeft();
    CString str;
    if (this->shop->dialog_active == 0) {
        return;
    }
    if (this->button_bmps[0] == nullptr || this->button_bmps[1] == nullptr || this->button_bmps[2] == nullptr || this->button_bmps[3] == nullptr || this->menu_bmp == nullptr) {
        return;
    }
    LockSurface2();
    this->menu_bmp->DrawSubRectMasked(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, this->rect.Width(), this->rect.Height());
    uint16_t* pal;
    if (this->hovered_btn == 0) {
        pal = palette_paris_daisy->GetPalette(0);
    } else {
        pal = palette_husk->GetPalette(0);
    }
    str = TxtFile::AllLines[0x48];
    g_font4->DrawTxt(topleft.x + this->outer_rects[0].left + this->outer_rects[0].Width() / 2, topleft.y + this->outer_rects[0].top + 6 + this->outer_rects[0].Height() / 4, str, 10, pal);
    str.Format("%d", this->shop->current_gold);
    InsertThousandsSeparators(&str);
    g_font4->DrawTxt(topleft.x + this->outer_rects[0].left + this->outer_rects[0].Width() / 2, topleft.y + this->outer_rects[0].top - 2 + this->outer_rects[0].Height() * 3 / 4, str, 10, pal);
    if (this->hovered_btn == 1) {
        pal = palette_paris_daisy->GetPalette(0);
    } else {
        pal = palette_husk->GetPalette(0);
    }
    str = TxtFile::AllLines[0x46];
    g_font4->DrawTxt(topleft.x + this->outer_rects[1].left + this->outer_rects[1].Width() / 2, topleft.y + this->outer_rects[1].top + 6 + this->outer_rects[1].Height() / 4, str, 10, pal);
    str.Format("%d", this->shop->buy_gold);
    InsertThousandsSeparators(&str);
    g_font4->DrawTxt(topleft.x + this->outer_rects[1].left + this->outer_rects[1].Width() / 2, topleft.y + this->outer_rects[1].top - 2 + this->outer_rects[1].Height() * 3 / 4, str, 10, pal);
    if (this->hovered_btn == 2) {
        pal = palette_paris_daisy->GetPalette(0);
    } else {
        pal = palette_husk->GetPalette(0);
    }
    str = TxtFile::AllLines[0x47];
    g_font4->DrawTxt(topleft.x + this->outer_rects[2].left + this->outer_rects[2].Width() / 2, topleft.y + this->outer_rects[2].top + 8 + this->outer_rects[2].Height() / 4, str, 10, pal);
    str.Format("%d", this->shop->sell_gold);
    InsertThousandsSeparators(&str);
    g_font4->DrawTxt(topleft.x + this->outer_rects[2].left + this->outer_rects[2].Width() / 2, topleft.y + this->outer_rects[2].top + this->outer_rects[2].Height() * 3 / 4, str, 10, pal);
    if (this->hovered_btn == 3) {
        pal = palette_paris_daisy->GetPalette(0);
    } else {
        pal = palette_husk->GetPalette(0);
    }
    str = TxtFile::AllLines[0x49];
    g_font4->DrawTxt(topleft.x + this->outer_rects[3].left + this->outer_rects[3].Width() / 2, topleft.y + this->outer_rects[3].top + 6 + this->outer_rects[3].Height() / 4, str, 10, pal);
    str.Format("%d", this->shop->result_gold);
    InsertThousandsSeparators(&str);
    g_font4->DrawTxt(topleft.x + this->outer_rects[3].left + this->outer_rects[3].Width() / 2, topleft.y + this->outer_rects[3].top - 2 + this->outer_rects[3].Height() * 3 / 4, str, 10, pal);
    if (this->hovered_btn >= 0 && this->pressed_btn >= 0 && this->pressed_btn == this->hovered_btn) {
        CRect& rect = this->outer_rects[this->hovered_btn];
        this->button_bmps[this->hovered_btn]->DrawSubRectMasked(topleft.x + rect.left, topleft.y + rect.top, 0, 0, rect.Width(), rect.Height());
        pal = palette_paris_daisy->GetPalette(0);
        switch (this->hovered_btn) {
        case 0:
            str = TxtFile::AllLines[0x48];
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top + 7 + rect.Height() / 4, str, 10, pal);
            str.Format("%d", this->shop->current_gold);
            InsertThousandsSeparators(&str);
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top - 1 + rect.Height() * 3 / 4, str, 10, pal);
            break;
        case 1:
            str = TxtFile::AllLines[0x46];
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top + 7 + rect.Height() / 4, str, 10, pal);
            str.Format("%d", this->shop->buy_gold);
            InsertThousandsSeparators(&str);
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top - 1 + rect.Height() * 3 / 4, str, 10, pal);
            break;
        case 2:
            str = TxtFile::AllLines[0x47];
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top + 9 + rect.Height() / 4, str, 10, pal);
            str.Format("%d", this->shop->sell_gold);
            InsertThousandsSeparators(&str);
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top + 1 + rect.Height() * 3 / 4, str, 10, pal);
            break;
        case 3:
            str = TxtFile::AllLines[0x49];
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top + 7 + rect.Height() / 4, str, 10, pal);
            str.Format("%d", this->shop->result_gold);
            InsertThousandsSeparators(&str);
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top - 1 + rect.Height() * 3 / 4, str, 10, pal);
            break;
        }
    }
    UnlockSurface2();
}


// 4C1358
void VisShopButtons::ResetSelected() {
    this->pressed_btn = -1;
    this->hovered_btn = -1;
}


// 4C0352
void VisShopButtons::ReleaseBmp() {
    for (int32_t i = 0; i < 4; i++) {
        if (this->button_bmps[i] != nullptr) {
            delete this->button_bmps[i];
        }
    }
    if (this->menu_bmp != nullptr) {
        delete this->menu_bmp;
    }
    this->menu_bmp = nullptr;
    this->button_bmps[3] = nullptr;
    this->button_bmps[2] = nullptr;
    this->button_bmps[1] = nullptr;
    this->button_bmps[0] = nullptr;
}


// 4C0088
void VisShopButtons::LoadBmp() {
    this->ReleaseBmp();
    CString base = "graphics\\interface\\";
    if (this->shop != nullptr) {
        base += this->shop->VMethod33();
    }

    for (int32_t i = 0; i < 4; i++) {
        CString fname = base + "ShopButton" + CString((char)('1' + i), 1) + ".bmp";
        this->button_bmps[i] = new CBmp64(fname);
        g_mousept.Update();
    }
    this->menu_bmp = new CBmp64(base + "ShopMenu.bmp");
    g_mousept.Update();
}

void VisShopButtons::UpdateHoveredState(uint32_t wparam, CPoint pos) { //4c14c4
    int32_t idx = this->GetButtonAt(pos);
    if (idx < 0 || (wparam & 1) != 0) {
        if (idx < 0 || idx != this->pressed_btn || (wparam & 1) == 0) {
            this->hovered_btn = -1;
        } else {
            this->hovered_btn = idx;
        }
    } else {
        this->hovered_btn = idx;
    }

    this->shop->dirty |= 0x20;
}

int32_t VisShopButtons::GetButtonAt(CPoint pos) { //4c137d
    CPoint pt = pos - this->shop->rect.TopLeft();
    int32_t idx = -1;
    for (int i = 0; i < 4; i++) {
        if (this->outer_rects[i].PtInRect(pt)) {
            idx = i;
            break;
        }
    }

    if (idx < 0) {
        return -1;
    }

    if (idx == 0) {
        if (this->inner_rects[0].PtInRect(pt) || this->inner_rects[1].PtInRect(pt)) {
            return 0;
        }
        return -1;
    }

    if (idx != 3) {
        return idx;
    }

    if (this->inner_rects[2].PtInRect(pt) || this->inner_rects[3].PtInRect(pt)) {
        return 3;
    }

    return -1;
}


// 4C69E0
VisShopDruid::~VisShopDruid() {
}


// 4C15AD
VisShopDruid::VisShopDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm)
    : VisShop(_id, l, t, r, b, btm) {
}


// 4C15E6
void VisShopDruid::VMethod26() {
    this->tips = nullptr;
    this->to_buy = new VisInvExtType3(0x3EB, 0, 0x12F, 0x1E0, 0x186, this);
    this->to_sell = new VisInvExtType2(0x3E9, 0, 0x186, 0x1E0, 0x1E0, this);
    this->assortiment = new VisInvExtType1Druid(0x3EA, 0, 0, 0xA4, 0x12F, this);
    this->shop_compass = new VisShopCompassDruid(0x3ED, 0xA4, 0, 0x1E0, 0x12F, this);
    this->buttons = new VisShopButtons(0x3EE, 0x1D0, 0, 0x280, 0xEE, this);

    this->AddChild(this->assortiment);
    this->AddChild(this->to_sell);
    this->AddChild(this->to_buy);
    this->AddChild(this->shop_compass);
    this->AddChild(this->buttons);

    this->gameplay = nullptr;
    this->select_info_panel = nullptr;
    this->spell_panel = nullptr;
    this->spr_myitem = nullptr;
    this->spr_shopitem = nullptr;
    this->bmp_backinvg = nullptr;
    this->bmp_backinvb = nullptr;
    this->bmp_backinvs = nullptr;
    this->snd_notif = nullptr;
    this->snd_step1 = nullptr;
    this->snd_step2 = nullptr;
    this->snd_breath = nullptr;
    this->snd_depart = nullptr;
    this->snd_buy = nullptr;
    this->snd_sell = nullptr;
    this->snd_enter = nullptr;
    this->snd_start = nullptr;
    this->snd_pov1 = nullptr;
    this->snd_pov2 = nullptr;
    this->snd_inshop = nullptr;
    this->snd_out = nullptr;
    this->snd_undo = nullptr;
    for (int32_t i = 0; i < 3; i++) {
        this->snd_bird[i] = nullptr;
    }
    for (int32_t i = 0; i < 4; i++) {
        this->snd_tool[i] = nullptr;
    }
}


// 4C1B8B
void VisShopDruid::VMethod31() {
    this->VMethod32();
    ReloadSfx(&this->snd_notif, "SFX\\Town\\Shop\\nofit.wav");
    ReloadSfx(&this->snd_depart, "SFX\\Town_Druid\\Shop\\Dotdel.wav");
    ReloadSfx(&this->snd_buy, "SFX\\Town\\buy.wav");
    ReloadSfx(&this->snd_sell, "SFX\\Town\\sell.wav");
    ReloadSfx(&this->snd_enter, "SFX\\Town_Druid\\Shop\\Din2.wav");
    ReloadSfx(&this->snd_pov1, "SFX\\Town_Druid\\Shop\\Ddruid5.wav");
    ReloadSfx(&this->snd_pov2, "SFX\\Town_Druid\\Shop\\Ddruid6.wav");
    ReloadSfx(&this->snd_inshop, "SFX\\Town_Druid\\Inn\\Dforest2.wav");
    ReloadSfx(&this->snd_out, "SFX\\Out.wav");
    ReloadSfx(&this->snd_undo, "SFX\\Undo.wav");
    ReloadSfx(&this->snd_bird[0], "SFX\\Town_druid\\Inn\\Dbird4.wav");
    ReloadSfx(&this->snd_bird[1], "SFX\\Town_druid\\Inn\\Dbird41.wav");
    ReloadSfx(&this->snd_bird[2], "SFX\\Town_druid\\Inn\\Dbird42.wav");
    ReloadSfx(&this->snd_tool[0], "SFX\\Town_druid\\shop\\Dtools1.wav");
    ReloadSfx(&this->snd_tool[1], "SFX\\Town_druid\\shop\\Dtools2.wav");
    ReloadSfx(&this->snd_tool[2], "SFX\\Town_druid\\shop\\Dtools3.wav");
    ReloadSfx(&this->snd_tool[3], "SFX\\Town_druid\\shop\\Dtools4.wav");
}


// 4C1958
void VisShopDruid::VMethod30() {
    static uint8_t snd_timing_init = 0; // 665D9C in asm
    static uint32_t last_snd_time; // 665DD4 in asm

    if ((snd_timing_init & 1) == 0) {
        snd_timing_init |= 1;
        last_snd_time = timeGetTime();
    }
    uint32_t now = timeGetTime();
    GetRandS16(30000);
    if ((this->shop_compass->state & 0x10) != 0 && this->shop_compass->center_frm == 1) {
        this->snd_pov1->Play();
    }
    if ((this->shop_compass->state & 0x20) != 0 && this->shop_compass->center_frm == 1) {
        this->snd_pov2->Play();
    }
    if (now - this->bird_tick > (uint32_t)this->next_bird) {
        int32_t bird = GetRandS16(3) + 1;
        if (bird == 1) {
            this->snd_bird[0]->Play();
        } else if (bird == 2) {
            this->snd_bird[1]->Play();
        } else if (bird == 3) {
            this->snd_bird[2]->Play();
        }
        this->next_bird = GetRandS16(2000) + 2000;
        this->bird_tick = timeGetTime();
    }
    if (now - this->tool_tick > (uint32_t)this->next_tool) {
        int32_t tool = GetRandS16(4) + 1;
        switch (tool) {
        case 1:
            this->snd_tool[0]->Play();
            break;
        case 2:
            this->snd_tool[1]->Play();
            break;
        case 3:
            this->snd_tool[2]->Play();
            break;
        case 4:
            this->snd_tool[3]->Play();
            break;
        }
        this->next_tool = GetRandS16(2000) + 2000;
        this->tool_tick = timeGetTime();
    }
}


// 4C1E5C
void VisShopDruid::VMethod28() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    g_mousept.DisableHint();
    this->buttons->ResetSelected();
    this->to_buy->VMethod41();
    this->assortiment->VMethod41();
    this->shop_compass->VMethod26();
    this->buttons->LoadBmp();
    this->LoadShopGfx();
    this->VMethod31();

    this->gameplay = main_wnd->vis_map_context;
    this->select_info_panel = main_wnd->vis_charinfo;

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        CString tips_text;
        MissionGetTips(3, &tips_text);
        this->tips = new VisTipsDialog(0x3F3, 0, 0xA2, 0x138, 0x12A, tips_text);
        this->shop_compass->AddChild(this->tips);
    } else {
        if (this->tips != nullptr) {
            this->shop_compass->RemoveChild(this->tips);
            delete this->tips;
        }
        this->tips = nullptr;
    }
    this->tips_update_flag = 0;

    main_wnd->vis_right_panel->RemoveChild(this->select_info_panel);
    CRect& panel_rect = this->select_info_panel->GetRect();
    CPoint pt(0x280 - panel_rect.Width(), 0);
    panel_rect.OffsetRect(pt.x, pt.y);
    this->select_info_panel->SetRect(&panel_rect);
    this->AddChild(this->select_info_panel);

    g_StructEnter.RebuildFromMap();
    this->selected_units.Copy(g_StructEnter.structure_list);
    this->select_index = g_StructEnter.FindFlaggedStructureUnit();

    this->to_sell->visible_startref = &this->selected_units[0]->shopInventoryVisibleStart;
    *this->assortiment->visible_startref = 0;
    this->select_category = 100;
    this->shop_compass->VMethod37(0);
    this->to_sell->VMethod32(&this->selected_units[this->select_index]->tokenEntries);
    this->assortiment->SelectCategoryGrid(0);
    this->assortiment->ClearCategoryGrids();
    this->to_buy->ClearBuyEntries();
    this->gameplay->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);
    this->selected_units[this->select_index]->SetSelected(1);
    this->gameplay->UpdateSelectionState();
    this->selected_units[this->select_index]->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
    this->to_sell->RefreshGrid();
    this->assortiment->RefreshGrid();
    this->to_buy->RefreshGrid();
    this->dirty |= 0x2F;
    this->assortiment->LoadGridSfx();
    this->to_sell->LoadGridSfx();
    this->to_buy->LoadGridSfx();

    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();

    VisScreen::VMethod28();
    this->dialog_active = 1;
    ApplyCursor(g_Cursors[0]);
    this->snd_enter->Play();
    PlayAmbientSfx(&this->snd_inshop);

    this->next_bird = GetRandS16(2000) + 2000;
    this->bird_tick = timeGetTime();
    this->next_tool = GetRandS16(2000) + 2000;
    this->tool_tick = timeGetTime();

    if (this->snd_start != nullptr && this->snd_start->FindPlayingChannel() == nullptr) {
        this->snd_start->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
    }

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        this->scenario_talk_target = ScenarioEnterShop();
    } else {
        this->scenario_talk_target = 0;
    }

    if (this->scenario_talk_target != 0) {
        int32_t npc_id = (this->scenario_talk_target >> 16) & 0xFFF;
        CString npc_name;
        npc_name.Format("shop\\npc31m%d", npc_id);
        ShowRoleKeyDialog(npc_name);
        ScenarioTalkTo(this->scenario_talk_target);
    }

    g_mousept.EnableHint();
}


// 4C1D25
void VisShopDruid::VMethod32() {
    FreeSfxSample(&this->snd_notif);
    FreeSfxSample(&this->snd_depart);
    FreeSfxSample(&this->snd_buy);
    FreeSfxSample(&this->snd_sell);
    FreeSfxSample(&this->snd_enter);
    FreeSfxSample(&this->snd_pov1);
    FreeSfxSample(&this->snd_pov2);
    FreeSfxSample(&this->snd_inshop);
    FreeSfxSample(&this->snd_out);
    FreeSfxSample(&this->snd_undo);
    FreeSfxSample(&this->snd_bird[0]);
    FreeSfxSample(&this->snd_bird[1]);
    FreeSfxSample(&this->snd_bird[2]);
    FreeSfxSample(&this->snd_tool[0]);
    FreeSfxSample(&this->snd_tool[1]);
    FreeSfxSample(&this->snd_tool[2]);
    FreeSfxSample(&this->snd_tool[3]);
}


// 4C6BE0
CString VisShopDruid::VMethod33() {
    return "shop_druid\\";
}


// 4C453D
void VisShopKaarg::VMethod32() {
    FreeSfxSample(&this->snd_notif);
    FreeSfxSample(&this->snd_depart);
    FreeSfxSample(&this->snd_buy);
    FreeSfxSample(&this->snd_sell);
    FreeSfxSample(&this->snd_enter);
    FreeSfxSample(&this->snd_pov1);
    FreeSfxSample(&this->snd_pov2);
    FreeSfxSample(&this->snd_inshop);
    FreeSfxSample(&this->snd_out);
    FreeSfxSample(&this->snd_undo);
    FreeSfxSample(&this->snd_voice[0]);
    FreeSfxSample(&this->snd_voice[1]);
    FreeSfxSample(&this->snd_voice[2]);
    FreeSfxSample(&this->snd_tool[0]);
    FreeSfxSample(&this->snd_tool[1]);
    FreeSfxSample(&this->snd_tool[2]);
    FreeSfxSample(&this->snd_tool[3]);
}


// 4C6C10
CString VisShopKaarg::VMethod33() {
    return "shop_kaarg\\";
}


// 4C6A50
VisShopKaarg::~VisShopKaarg() {
}


// 4C37C5
VisShopKaarg::VisShopKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm)
    : VisShop(_id, l, t, r, b, btm) {
}


// 4C43A3
void VisShopKaarg::VMethod31() {
    this->VMethod32();
    ReloadSfx(&this->snd_notif, "SFX\\Town\\Shop\\nofit.wav");
    ReloadSfx(&this->snd_depart, "SFX\\Town_kaarg\\Shop\\Kotdel.wav");
    ReloadSfx(&this->snd_buy, "SFX\\Town\\buy.wav");
    ReloadSfx(&this->snd_sell, "SFX\\Town\\sell.wav");
    ReloadSfx(&this->snd_enter, "SFX\\Town_kaarg\\Shop\\Kin2.wav");
    ReloadSfx(&this->snd_pov1, "SFX\\Town_kaarg\\Shop\\Kman4.wav");
    ReloadSfx(&this->snd_pov2, "SFX\\Town_kaarg\\Shop\\Kman4.wav");
    ReloadSfx(&this->snd_inshop, "SFX\\Town_kaarg\\Inn\\Kvox5.wav");
    ReloadSfx(&this->snd_out, "SFX\\Out.wav");
    ReloadSfx(&this->snd_undo, "SFX\\Undo.wav");
    ReloadSfx(&this->snd_voice[0], "SFX\\Town_kaarg\\Inn\\Kvox6.wav");
    ReloadSfx(&this->snd_voice[1], "SFX\\Town_kaarg\\Inn\\Kvox7.wav");
    ReloadSfx(&this->snd_voice[2], "SFX\\Town_kaarg\\Inn\\Kvox8.wav");
    ReloadSfx(&this->snd_tool[0], "SFX\\Town_kaarg\\shop\\Ktools1.wav");
    ReloadSfx(&this->snd_tool[1], "SFX\\Town_kaarg\\shop\\Ktools2.wav");
    ReloadSfx(&this->snd_tool[2], "SFX\\Town_kaarg\\shop\\Ktools3.wav");
    ReloadSfx(&this->snd_tool[3], "SFX\\Town_kaarg\\shop\\Ktools4.wav");
}


// 4C4170
void VisShopKaarg::VMethod30() {
    static uint8_t snd_timing_init = 0; // 665DA4 in asm
    static uint32_t last_snd_time; // 665DB8 in asm

    if ((snd_timing_init & 1) == 0) {
        snd_timing_init |= 1;
        last_snd_time = timeGetTime();
    }
    uint32_t now = timeGetTime();
    GetRandS16(30000);
    if ((this->shop_compass->state & 0x10) != 0 && this->shop_compass->center_frm == 1) {
        this->snd_pov1->Play();
    }
    if ((this->shop_compass->state & 0x20) != 0 && this->shop_compass->center_frm == 1) {
        this->snd_pov2->Play();
    }
    if (now - this->voice_tick > (uint32_t)this->next_voice) {
        int32_t voice = GetRandS16(3) + 1;
        if (voice == 1) {
            this->snd_voice[0]->Play();
        } else if (voice == 2) {
            this->snd_voice[1]->Play();
        } else if (voice == 3) {
            this->snd_voice[2]->Play();
        }
        this->next_voice = GetRandS16(2000) + 2000;
        this->voice_tick = timeGetTime();
    }
    if (now - this->tool_tick > (uint32_t)this->next_tool) {
        int32_t tool = GetRandS16(4) + 1;
        switch (tool) {
        case 1:
            this->snd_tool[0]->Play();
            break;
        case 2:
            this->snd_tool[1]->Play();
            break;
        case 3:
            this->snd_tool[2]->Play();
            break;
        case 4:
            this->snd_tool[3]->Play();
            break;
        }
        this->next_tool = GetRandS16(2000) + 2000;
        this->tool_tick = timeGetTime();
    }
}


// 4C37FE
void VisShopKaarg::VMethod26() {
    this->tips = nullptr;
    this->to_buy = new VisInvExtType3(0x3EB, 0, 0x12F, 0x1E0, 0x186, this);
    this->to_sell = new VisInvExtType2(0x3E9, 0, 0x186, 0x1E0, 0x1E0, this);
    this->assortiment = new VisInvExtType1Kaarg(0x3EA, 0, 0, 0xA4, 0x12F, this);
    this->shop_compass = new VisShopCompassKaarg(0x3ED, 0xA4, 0, 0x1E0, 0x12F, this);
    this->buttons = new VisShopButtons(0x3EE, 0x1D0, 0, 0x280, 0xEE, this);

    this->AddChild(this->assortiment);
    this->AddChild(this->to_sell);
    this->AddChild(this->to_buy);
    this->AddChild(this->shop_compass);
    this->AddChild(this->buttons);

    this->gameplay = nullptr;
    this->select_info_panel = nullptr;
    this->spell_panel = nullptr;
    this->spr_myitem = nullptr;
    this->spr_shopitem = nullptr;
    this->bmp_backinvg = nullptr;
    this->bmp_backinvb = nullptr;
    this->bmp_backinvs = nullptr;
    this->snd_notif = nullptr;
    this->snd_step1 = nullptr;
    this->snd_step2 = nullptr;
    this->snd_breath = nullptr;
    this->snd_depart = nullptr;
    this->snd_buy = nullptr;
    this->snd_sell = nullptr;
    this->snd_enter = nullptr;
    this->snd_start = nullptr;
    this->snd_pov1 = nullptr;
    this->snd_pov2 = nullptr;
    this->snd_inshop = nullptr;
    this->snd_out = nullptr;
    this->snd_undo = nullptr;
    for (int32_t i = 0; i < 3; i++) {
        this->snd_voice[i] = nullptr;
    }
    for (int32_t i = 0; i < 4; i++) {
        this->snd_tool[i] = nullptr;
    }
}


// 4C3B70
void VisShopKaarg::VMethod28() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    g_mousept.DisableHint();
    this->buttons->ResetSelected();
    this->to_buy->VMethod41();
    this->assortiment->VMethod41();
    this->shop_compass->VMethod26();
    this->buttons->LoadBmp();
    this->LoadShopGfx();
    this->VMethod31();

    this->gameplay = main_wnd->vis_map_context;
    this->select_info_panel = main_wnd->vis_charinfo;

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        CString tips_text;
        MissionGetTips(3, &tips_text);
        this->tips = new VisTipsDialog(0x3F3, 0, 0xA2, 0x138, 0x12A, tips_text);
        this->shop_compass->AddChild(this->tips);
    } else {
        if (this->tips != nullptr) {
            this->shop_compass->RemoveChild(this->tips);
            delete this->tips;
        }
        this->tips = nullptr;
    }
    this->tips_update_flag = 0;

    main_wnd->vis_right_panel->RemoveChild(this->select_info_panel);
    CRect& panel_rect = this->select_info_panel->GetRect();
    CPoint pt(0x280 - panel_rect.Width(), 0);
    panel_rect.OffsetRect(pt.x, pt.y);
    this->select_info_panel->SetRect(&panel_rect);
    this->AddChild(this->select_info_panel);

    g_StructEnter.RebuildFromMap();
    this->selected_units.Copy(g_StructEnter.structure_list);
    this->select_index = g_StructEnter.FindFlaggedStructureUnit();

    this->to_sell->visible_startref = &this->selected_units[0]->shopInventoryVisibleStart;
    *this->assortiment->visible_startref = 0;
    this->select_category = 100;
    this->shop_compass->VMethod37(0);
    this->to_sell->VMethod32(&this->selected_units[this->select_index]->tokenEntries);
    this->assortiment->SelectCategoryGrid(0);
    this->assortiment->ClearCategoryGrids();
    this->to_buy->ClearBuyEntries();
    this->gameplay->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);
    this->selected_units[this->select_index]->SetSelected(1);
    this->gameplay->UpdateSelectionState();
    this->selected_units[this->select_index]->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
    this->to_sell->RefreshGrid();
    this->assortiment->RefreshGrid();
    this->to_buy->RefreshGrid();
    this->dirty |= 0x2F;
    this->assortiment->LoadGridSfx();
    this->to_sell->LoadGridSfx();
    this->to_buy->LoadGridSfx();

    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();

    VisScreen::VMethod28();
    this->dialog_active = 1;
    ApplyCursor(g_Cursors[0]);
    this->snd_enter->Play();
    PlayAmbientSfx(&this->snd_inshop);

    this->next_voice = GetRandS16(2000) + 2000;
    this->voice_tick = timeGetTime();
    this->next_tool = GetRandS16(2000) + 2000;
    this->tool_tick = timeGetTime();

    if (this->snd_start != nullptr && this->snd_start->FindPlayingChannel() == nullptr) {
        this->snd_start->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
    }

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        this->scenario_talk_target = ScenarioEnterShop();
    } else {
        this->scenario_talk_target = 0;
    }

    if (this->scenario_talk_target != 0) {
        int32_t npc_id = (this->scenario_talk_target >> 16) & 0xFFF;
        CString npc_name;
        npc_name.Format("shop\\npc31m%d", npc_id);
        ShowRoleKeyDialog(npc_name);
        ScenarioTalkTo(this->scenario_talk_target);
    }

    g_mousept.EnableHint();
}


// 4BCCE1
void VisShop::CancelAllPendingBuys() {
    this->ReturnPendingBuyItems();
    this->gameplay->SendShopTakeBack();
}


// 4BCAF1
void VisShop::CommitSell() {
    this->RecalcShopGold();
    if (this->sell_gold != 0) {
        this->shop_compass->VMethod28();
        this->shop_compass->state |= 0x20;
        this->gameplay->SendShopSell();
        this->snd_sell->Play();
    }
}

// 4BCB63
int VisShop::CommitBuy() {
    this->RecalcShopGold();

    if (this->buy_gold == 0) {
        return 0;
    }
  
    if (this->current_gold + this->buy_gold < 0) {
        this->shop_compass->VMethod29();
        this->shop_compass->state |= 0x40;
        this->snd_notif->Play();
        return 0;
    }
    
    this->shop_compass->VMethod28();
    this->shop_compass->state |= 0x20;
    this->gameplay->SendShopBuy();
    this->snd_buy->Play();
    return 1;
}

