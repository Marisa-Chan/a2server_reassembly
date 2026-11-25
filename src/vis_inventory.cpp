#include <winsock2.h>
#ifdef A2CLIENT_PATCH
#include "patch/hotkeys.h"
#endif
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
#include "vis_inventory.h"
#include "gameplay.h"
#include "vis_playing_area.h"
#include "vis_char.h"
#include "vis_shop.h"
#include "vis_menu.h"

// Unit-targetable flag per 24-cell spellbook UI slot (0..23). 62f8a8 (client: 5fb998)
uint32_t g_SpellbookUnitTargetable[24] = {1,0,0,1, 1,1,1,1, 1,1,1,1, 1,0,0,1, 1,0,0,1, 1,1,0,1};

// Unit-targetable flag per spell ID (indexed by spell_id - 1 for all 29 spells). 62fa28 (client: 5fbb18)
uint32_t g_SpellUnitTargetable[29] = {1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1};

// Spellbook position castable-from-book flag. 62f968
uint32_t g_SpellbookCastable[24] = {1,1,1,0, 1,0,0,1, 0,0,1,1, 1,1,1,0, 1,0,0,0, 0,1,1,1};

// Spellbook position double-click-castable flag. 62f9c8
uint32_t g_SpellbookDblClickCastable[24] = {0,0,0,1, 1,1,1,0, 1,1,0,0, 0,0,0,1, 1,0,0,1, 1,0,0,0};

// 4a5fab (client: 4a9fda)
void VisInvBase::DrawGlowTrail(int32_t x, int32_t y, int32_t idx) {
    static const int32_t trail_alpha[7] = {0x3F, 0x7F, 0xBF, 0xFF, 0xBF, 0x7F, 0x3F};

    int32_t i = this->anim_frames.GetAt(idx) >> 1;
    for (int32_t k = 0; k < 7; k++) {
        if (i < k) {
            break;
        }
        int32_t j = (i - k) % 1024;
        PlotGlowDot(x + this->random_offsets1[j], y + this->random_offsets2[j], 0xFF, 0, 0xFF, trail_alpha[k]);
    }
}


// 4a58dc (client: 4a990b)
TokenEntry* VisInvBase::SplitItem(TokenEntry* o, int32_t num) {
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (!o->IsStackableWith(entry)) {
            continue;
        }
        if (entry->RemoveCount(num)) {
            o = new TokenEntry();
            *o = *entry;
            o->count = num;
            return o;
        }
        if (entry->IsEmpty()) {
            o = new TokenEntry();
            *o = *entry;
            entry->count = 0;
            return o;
        }
        o = entry;
        this->grid_source->RemoveAt(i, 1);
        o->count = num;
        return o;
    }
    return nullptr;
}


// 4a554f (client: 4a957e)
int32_t VisInvBase::InsertItem(TokenEntry* o, int32_t idx) {
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (!o->IsStackableWith(entry)) {
            continue;
        }
        entry->AddCount(o->count);
        delete o;
        return i;
    }
    if (idx >= 0 && idx < this->grid_source->GetSize()) {
        this->grid_source->InsertAt(idx, o, 1);
        return idx;
    }
    if (this->grid_source->GetSize() != 0 && this->grid_source->GetAt(this->grid_source->GetUpperBound())->IsEmpty()) {
        this->grid_source->InsertAt(this->grid_source->GetUpperBound(), o, 1);
        return this->grid_source->GetUpperBound() - 1;
    }
    this->grid_source->Add(o);
    return this->grid_source->GetUpperBound();
}


// 4a57a1 (client: 4a97d0)
TokenEntry* VisInvBase::TakeItemById(uint32_t id) {
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (id != entry->match_id) {
            continue;
        }
        if (entry->RemoveCount(1)) {
            TokenEntry* result = new TokenEntry();
            *result = *entry;
            result->count = 1;
            return result;
        }
        this->grid_source->RemoveAt(i, 1);
        entry->count = 1;
        return entry;
    }
    return nullptr;
}


// 4a56c3 (client: 4a96f2)
int32_t VisInvBase::AppendItem(TokenEntry* o) {
    if (this->grid_source == nullptr) {
        return -1;
    }
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (!entry->IsStackableWith(o)) {
            continue;
        }
        entry->AddCount(o->count);
        delete o;
        return i;
    }
    this->grid_source->Add(o);
    return this->grid_source->GetUpperBound();
}


// 4a5de0 (client: 4a9e0f)
int32_t VisInvBase::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_WORLD_TICK) {
        this->TickCellAnims();
    }
    return CVisualObject::MsgProc(msg, wparam, lparam);
}


// 46fb90 (client: 472fd0)
int32_t VisInvBase::GetVisibleStart() {
    return *this->visible_startref;
}


// 4a79a0 (client: 4ab810)
int32_t VisInvBase::GetSlotAt(int32_t x, int32_t y) {
    return -1;
}


// 4a7990 (client: 4ab800)
int32_t VisInvBase::GetSlotAtPoint(const CPoint* pt) {
    return -1;
}


// 4a4cf8 (client: 4a8d28)
const char* VisInvBase::GetHint() {
    return nullptr;
}


// 4a4938 (client: 4a8968)
VisInvBase::VisInvBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->visible_columns = 0;
    this->visible_rows = 0;
    this->visible_startref = nullptr;
    this->grid_source = nullptr;
    this->InitGlowOffsets();
}


// 4a4bcb (client: 4a8bfb)
void VisInvBase::InitGlowOffsets() {
    for (int32_t i = 0; i < 1024; i++) {
        this->random_offsets1[i] = rand() / 0x1FF + 8;
        this->random_offsets2[i] = rand() / 0x1FF + 8;
    }
}


// 4a4acc (client: 4a8afc)
VisInvBase::~VisInvBase() {
    for (int32_t i = 0; i < this->visible_columns * this->visible_rows; i++) {
        CSprite256* sprite = this->spr_cells.GetAt(i);
        if (sprite != nullptr) {
            delete sprite;
        }
    }
    this->spr_cells.RemoveAll();
}


// 4a5c39 (client: 4a9c68)
int32_t VisInvBase::PlaceDraggedItem(int32_t idx) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry == nullptr) {
        return -1;
    }
    if (main_wnd->drag_entry->IsEmpty()) {
        main_wnd->vis_map_context->my_main_unit->gold += main_wnd->drag_entry->count;
    }
    int32_t count = main_wnd->drag_entry->count;

    int32_t same_tab;
    int32_t tab = this->GetTransferTab();
    if (tab >= 5 && tab <= 8 && tab == main_wnd->drag_src_tab) {
        same_tab = 0;
    } else {
        same_tab = 1;
    }

    idx = this->InsertItem(main_wnd->drag_entry, idx);
    if (this->GetTransferTab() == 2) {
        this->BindUnitInventory(main_wnd->vis_map_context->primary_selected_object);
    } else {
        this->BindGridSource(this->grid_source);
    }
    if (same_tab != 0) {
        main_wnd->vis_map_context->SendItemOperation(main_wnd->drag_src_tab, main_wnd->drag_src_slot, this->GetTransferTab(), idx, count);
    }
    main_wnd->ResetItemCursor();
    main_wnd->vis_root->MsgProc(VIS_MSG_SET_OPTION, this->id, 0);
    return idx;
}


// 4a5aae (client: 4a9add)
TokenEntry* VisInvBase::PickupSlotItem(int32_t idx, int32_t num) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr || this->grid_source->GetSize() <= idx) {
        return nullptr;
    }
    CGameObject* selected = main_wnd->vis_map_context->primary_selected_object;
    if (selected->map_player != main_wnd->vis_map_context->my_main_unit) {
        return nullptr;
    }
    TokenEntry* entry = this->grid_source->GetAt(idx);
    if (entry == nullptr) {
        return nullptr;
    }
    if (entry->GetType() < 2 && (selected->last_action == ACTION_ATTACK || selected->last_action == ACTION_SHOOT || selected->last_action == ACTION_CAST)) {
        return nullptr;
    }
    main_wnd->drag_entry = this->SplitItem(entry, num);
    main_wnd->drag_src_slot = idx;
    main_wnd->drag_src_tab = this->GetTransferTab();
    if (this->GetTransferTab() == 2) {
        this->BindUnitInventory(selected);
    } else {
        this->BindGridSource(this->grid_source);
    }
    main_wnd->vis_root->MsgProc(VIS_MSG_SET_OPTION, this->id, 0);
    return main_wnd->drag_entry;
}


// 4a5e12 (client: 4a9e41)
void VisInvBase::TickCellAnims() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (this->grid_source == nullptr) {
        return;
    }
    int32_t total_cells = this->visible_columns * this->visible_rows;
    int32_t count = this->grid_source->GetSize();
    if (count >= total_cells) {
        count = total_cells;
    }
    this->has_anim_visible_cells = 0;
    for (int32_t i = 0; i < total_cells; i++) {
        if (i >= count) {
            continue;
        }
        if (this->cell_update_counter.GetAt(i) == 0) {
            continue;
        }
        TokenEntry* entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (main_wnd->drag_entry == entry && main_wnd->drag_entry != nullptr && main_wnd->drag_entry->count == 1) {
            continue;
        }
        if (entry == nullptr || !(entry->flg & 0x20)) {
            continue;
        }
        this->anim_frames.ElementAt(i)++;
        this->has_anim_visible_cells = 1;
    }
}


// 4a4c2f (client: 4a8c5f)
void VisInvBase::InitCellArrays() {
    int32_t total = this->visible_columns * this->visible_rows;
    this->spr_cells.SetSize(total, -1);
    this->cell_update_counter.SetSize(total, -1);
    this->anim_frames.SetSize(total, -1);
    for (int32_t i = 0; i < total; i++) {
        this->spr_cells.ElementAt(i) = nullptr;
        this->cell_update_counter.ElementAt(i) = 0;
    }
}


// 4a4d05 (client: 4a8d35)
void VisInvBase::BindUnitInventory(CGameObject* uni) {
    if (uni == nullptr) {
        this->grid_source = nullptr;
        this->visible_startref = nullptr;
    } else {
        this->grid_source = &uni->tokenEntries;
        this->visible_startref = &uni->shopInventoryVisibleStart;
    }
    int32_t total = this->visible_columns * this->visible_rows;
    for (int32_t i = 0; i < total; i++) {
        this->cell_update_counter.ElementAt(i) = 0;
    }
    this->UpdateVisibleStart();
}


// 4a4da9 (client: 4a8dd9)
void VisInvBase::BindGridSource(CArray<TokenEntry*>* arr) {
    if (arr == nullptr) {
        this->grid_source = nullptr;
    } else {
        this->grid_source = arr;
    }
    int32_t total = this->visible_columns * this->visible_rows;
    for (int32_t i = 0; i < total; i++) {
        this->cell_update_counter.ElementAt(i) = 0;
    }
    this->UpdateVisibleStart();
}


// 4a4e28 (client: 4a8e58)
void VisInvBase::UpdateVisibleStart() {
    if (this->grid_source == nullptr) {
        return;
    }
    if (this->grid_source->GetSize() - *this->visible_startref < this->visible_columns * this->visible_rows) {
        *this->visible_startref = this->grid_source->GetSize() - this->visible_columns * this->visible_rows;
    }
    if (*this->visible_startref < 0) {
        *this->visible_startref = 0;
    }
}


// 4a79b0 (client: 4ab820)
int32_t VisInvBase::GetTransferTab() {
    return -1;
}


// 4a4ebc (client: 4a8eec)
void VisInvBase::RefreshCellSprites() {
    if (this->grid_source == nullptr) {
        return;
    }
    int32_t total_cells = this->visible_columns * this->visible_rows;
    int32_t count = this->grid_source->GetSize();
    if (count >= total_cells) {
        count = total_cells;
    }
    for (int32_t i = 0; i < count; i++) {
        if (this->cell_update_counter.GetAt(i) != 0) {
            continue;
        }
        TokenEntry* entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry == nullptr) {
            continue;
        }
        if (entry->IsEmpty() != 0 || entry->IsDepleted() != 0) {
            this->cell_update_counter.ElementAt(i) = 1;
            continue;
        }
        CSprite256* sprite = this->spr_cells.GetAt(i);
        if (sprite != nullptr) {
            delete sprite;
        }
        CString name = entry->GetSpriteName();
        sprite = new CA16("graphics\\inventory\\" + name + ".16a");
        this->spr_cells.ElementAt(i) = sprite;
        sprite->ResetPalette(0x10, 4, 0);
        this->cell_update_counter.ElementAt(i) = 1;
    }
}


// 4a51c9 (client: 4a91f8)
void VisInvBase::ScrollCellsForward() {
    if (this->grid_source == nullptr) {
        return;
    }
    int32_t total_cells = this->visible_columns * this->visible_rows;
    if (this->grid_source->GetSize() <= *this->visible_startref + total_cells) {
        return;
    }
    CSprite256* first = this->spr_cells.GetAt(0);
    for (int32_t i = 0; i < total_cells - 1; i++) {
        this->spr_cells.ElementAt(i) = this->spr_cells.GetAt(i + 1);
        this->anim_frames.ElementAt(i) = this->anim_frames.GetAt(i + 1);
    }
    this->anim_frames.ElementAt(total_cells - 1) = 0;
    this->spr_cells.ElementAt(total_cells - 1) = first;
    this->cell_update_counter.ElementAt(total_cells - 1) = 0;
    *this->visible_startref += 1;
    this->RefreshCellSprites();
}


// 4a5350 (client: 4a937f)
void VisInvBase::ScrollCellsBack() {
    if (this->grid_source == nullptr) {
        return;
    }
    if (*this->visible_startref == 0) {
        return;
    }
    int32_t total_cells = this->visible_columns * this->visible_rows;
    CSprite256* last = this->spr_cells.GetAt(total_cells - 1);
    for (int32_t i = total_cells - 1; i > 0; i--) {
        this->spr_cells.ElementAt(i) = this->spr_cells.GetAt(i - 1);
        this->anim_frames.ElementAt(i) = this->anim_frames.GetAt(i - 1);
    }
    this->spr_cells.ElementAt(0) = last;
    this->cell_update_counter.ElementAt(0) = 0;
    *this->visible_startref -= 1;
    this->RefreshCellSprites();
}

// 41f940 (client: 4207b0)
int32_t VisInvBase::HasAnimatingCells() {
    return this->has_anim_visible_cells;
} 


// 4a6449 (client: 4aa478)
void VisBagInventory::Draw() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->RefreshCellSprites();

    CRect screen_rect = this->ClientRectToScreen(this->rect);

    int32_t edge_width = ((g_ScreenSize.right - 0xf0) % 0x50) / 2;

    g_bmp_invframe->DrawSubRectMasked(screen_rect.left + edge_width, screen_rect.top, 0, 0, 0x110, g_bmp_invframe->GetHeight(0));

    for (int32_t i = 0; i < (g_ScreenSize.right - 0x280) / 0x50; i++) {
        g_bmp_invframe->DrawSubRectMasked(screen_rect.left + edge_width + 0x110 + i * 0x50, screen_rect.top, 0xc0, 0, 0x110, g_bmp_invframe->GetHeight(0));
    }

#ifdef A2CLIENT_PATCH
    if (edge_width != 0 && g_bmp_inv1024l != nullptr && g_bmp_inv1024r != nullptr) {
#else
    if (edge_width != 0 && g_ScreenSize.bottom > 600) {
#endif
        g_bmp_inv1024l->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
        g_bmp_inv1024r->DrawSubRectMasked(screen_rect.right - edge_width - 0x10, screen_rect.top, 0, 0, g_bmp_inv1024r->GetWidth(0), g_bmp_inv1024r->GetHeight(0));
    }

    int32_t frame_w = g_bmp_invframe->GetWidth(0);
    int32_t frame_h = g_bmp_invframe->GetHeight(0);
    g_bmp_invframe->DrawSubRectMasked(screen_rect.right - edge_width - g_bmp_invframe->GetWidth(0) + 0x110, screen_rect.top, 0x110, 0, frame_w, frame_h);

    CRect left_arrow_rect(screen_rect.left, screen_rect.top, screen_rect.left + 0x20 + edge_width, screen_rect.bottom);
    CRect right_arrow_rect(screen_rect.left + edge_width + 0x20 + this->visible_columns * 0x50, screen_rect.top,
        screen_rect.left + 0x40 + edge_width * 2 + this->visible_columns * 0x50, screen_rect.bottom);

    CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());

    if (this->grid_source != nullptr) {
        #ifdef A2CLIENT_PATCH
        if (this->visible_startref == nullptr) { // CRASH_fix_4AAB7E (client 4AAB7E): null guard
            return;
        }
        #endif
        if (*this->visible_startref != 0) {
            if (left_arrow_rect.PtInRect(mouse_pt)) {
                g_bmp_invarrow3->DrawSubRectMasked(screen_rect.left + edge_width, screen_rect.top + 2, 0, 0, 0x20, 0x58);
            } else {
                g_bmp_invarrow1->DrawSubRectMasked(screen_rect.left + edge_width, screen_rect.top + 2, 0, 0, 0x20, 0x58);
            }
        }
        if (this->grid_source->GetSize() > this->visible_columns + *this->visible_startref) {
            if (right_arrow_rect.PtInRect(mouse_pt)) {
                g_bmp_invarrow4->DrawSubRectMasked(screen_rect.left + edge_width + 0x20 + this->visible_columns * 0x50, screen_rect.top + 2, 0, 0, 0x20, 0x58);
            } else {
                g_bmp_invarrow2->DrawSubRectMasked(screen_rect.left + edge_width + 0x20 + this->visible_columns * 0x50, screen_rect.top + 2, 0, 0, 0x20, 0x58);
            }
        }
    }

    if (this->grid_source == nullptr) {
        g_font2->DrawTextWithShadow(edge_width + (screen_rect.left + screen_rect.right - 0x10) / 2, (screen_rect.top + screen_rect.bottom) / 2,
            TxtFile::AllLines[0x33], CGameFont::ALIGN_CENTER, clrsh_DullGold, 1);
        return;
    }

    int32_t draw_count;
    if (this->grid_source->GetSize() < this->visible_columns * this->visible_rows) {
        draw_count = this->grid_source->GetSize();
    } else {
        draw_count = this->visible_columns * this->visible_rows;
    }

    for (int32_t i = 0; i < this->visible_columns * this->visible_rows; i++) {
        if (i >= draw_count) {
            continue;
        }
        if (this->cell_update_counter.GetAt(i) == 0) {
            continue;
        }

        TokenEntry* entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (main_wnd->drag_entry == entry && main_wnd->drag_entry != nullptr && main_wnd->drag_entry->count == 1) {
            continue;
        }

        entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry == nullptr) {
            continue;
        }

        int32_t cell_x = edge_width + screen_rect.left + 0x20 + i * 0x50;
        int32_t cell_y = screen_rect.top + 6;

        if (entry->IsEmpty() != 0) {
            g_bmp_backinv->Draw(cell_x, cell_y, 0, 0, 0);
            g_ca16_money->Draw(cell_x, cell_y, 0, 0, 0);

            CString text;
            text.Format("%d", main_wnd->vis_map_context->my_main_unit->gold);
            InsertThousandsSeparators(&text);
            g_font2->DrawTextWithShadow(cell_x + 3, screen_rect.bottom - 0xe, text, CGameFont::ALIGN_LEFT_TOP, clrsh_DullGold, 1);
            continue;
        }

        g_bmp_backinv->Draw(cell_x, cell_y, 0, 0, 0);
        CSprite256* sprite = this->spr_cells.GetAt(i);
        sprite->Draw(cell_x, cell_y, 0, 0, 0);

        entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry->count > 1) {
            char buf[0x50];
            sprintf(buf, "%d", entry->count);
            g_font2->DrawTextWithShadow(cell_x + 3, screen_rect.bottom - 0xe, buf, CGameFont::ALIGN_LEFT_TOP, clrsh_DullGold, 1);
        }

        entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry->flg & 0x20) {
            this->DrawGlowTrail(cell_x, cell_y, i);
        }

#if defined(A2CLIENT) && defined(A2TANGAR_HAT)
        // Tangar mark_item_hotkey (4AAC7C): custom-hotkey labels replace the
        // retail F4-F12 labels.
        entry = this->grid_source->GetAt(i + *this->visible_startref);
        tangar_hotkeys::DrawItemHotkeyLabels(edge_width + screen_rect.left, screen_rect.top, i, entry);
#else
        for (int32_t j = 0; j < 9; j++) {
            entry = this->grid_source->GetAt(i + *this->visible_startref);
            if (main_wnd->m_GameSession.shortcuts[j].MatchesItem(entry) != 0) {
                char key_buf[0x50];
                sprintf(key_buf, "F%d", j + 4);
                g_font3->DrawTextWithShadow(cell_x + 3, screen_rect.top + 8, key_buf, CGameFont::ALIGN_LEFT_TOP, clrsh_ShockingBlack, 1);
                break;
            }
        }
#endif
    }
}


// 4a716f (client: 4ab19d)
int32_t VisBagInventory::OnMouseMove(uint32_t wparam, CPoint pos) {
    if (g_mousept.GetSelectState() != 0) {
        g_mousept.ResetStates();
    }

    if ((wparam & 1) == 0) {
        return 0;
    }

    if (this->grid_source == nullptr) {
        return 0;
    }

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        return 0;
    }

    int32_t idx = this->GetSlotAt(pos.x, -pos.y);
    if (idx == -1) {
        return 0;
    }

    int32_t count;
    if (this->grid_source->GetAt(idx)->IsEmpty() != 0) {
        if (main_wnd->vis_map_context->my_main_unit->gold < 1) {
            count = 0;
        } else if (g_kbShiftState == 0) {
            count = 1000;
        } else {
            count = this->grid_source->GetAt(idx)->count;
        }
    } else {
        int32_t stack_count = this->grid_source->GetAt(idx)->count;
#ifdef A2CLIENT_PATCH
        // p10itm.xck (4ab2a8 -> 6e21d0): Shift = all, Alt = min(count, 1000), Ctrl = min(count, 10), else 1
        if (g_kbShiftState != 0) {
            count = stack_count;
        } else if (g_kbMenuState != 0) {
            count = (stack_count > 1000) ? 1000 : stack_count;
        } else if (g_kbControlState != 0) {
            count = (stack_count > 10) ? 10 : stack_count;
        } else {
            count = 1;
        }
#else
        if (g_kbShiftState == 0) {
            count = 1;
        } else {
            count = stack_count;
        }
#endif
    }

    if (this->grid_source->GetAt(idx)->IsQuestItem() != 0) {
        return 0;
    }

    if (this->PickupSlotItem(idx, count) == 0) {
        return 0;
    }

    if (main_wnd->drag_entry->IsEmpty() != 0) {
        main_wnd->vis_map_context->my_main_unit->gold -= main_wnd->drag_entry->count;
        main_wnd->SetItemCursor(main_wnd->drag_entry, main_wnd->drag_src_slot, "graphics\\interface\\money\\money.16a", main_wnd->drag_src_tab);
    } else {
        CString name = main_wnd->drag_entry->GetSpriteName();
        CString path = "graphics\\inventory\\" + name + ".16a";
        main_wnd->SetItemCursor(main_wnd->drag_entry, main_wnd->drag_src_slot, path, main_wnd->drag_src_tab);
    }

    return 0;
}


// 4A7441
#ifdef A2CLIENT_PATCH
// Tangar pack_scrolling: PSCRL_gameInventory replaces the retail (base-class)
// VisBagInventory::OnKeyDown - PageUp/PageDown page through the bags, one bag
// column per step (page width = (screen width - 240) / 80 - 1).
int32_t VisBagInventory::OnKeyDown(uint32_t wparam) {
    if (wparam == VK_PRIOR || wparam == VK_NEXT) {
        int32_t page_steps = ((int32_t)g_ScreenSize.right - 240) / 80 - 1;
        if (page_steps <= 1) {
            page_steps = 1;
        }
        for (int32_t i = 0; i < page_steps; i++) {
            if (wparam == VK_PRIOR) {
                this->ScrollCellsBack();
            } else {
                this->ScrollCellsForward();
            }
        }
        return 1;
    }
    return 0;
}
#endif

// 4a7441 (client: 4ab46f)
int32_t VisBagInventory::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (msg == VIS_MSG_SHORTCUT_KEY) {
        if (lparam != 0) {
            CRect screen_rect = this->ClientRectToScreen(this->rect);
            CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
            if (screen_rect.PtInRect(mouse_pt)) {
                int32_t idx = this->GetSlotAt(g_mousept.GetX(), -g_mousept.GetY());
                if (idx >= 0) {
#if defined(A2CLIENT) && defined(A2TANGAR_HAT)
                    // Tangar item_hotkey_set (4AB545): the custom-hotkey map
                    // owns the binding (with its own dedup); retail dedup skipped.
                    tangar_hotkeys::ItemHotkeySet((int32_t)wparam + VK_F4, this->grid_source->GetAt(idx));
#else
                    main_wnd->m_GameSession.shortcuts[wparam].SetItem(this->grid_source->GetAt(idx));
                    for (int32_t i = 0; i < 9; i++) {
                        if (i != wparam && main_wnd->m_GameSession.shortcuts[i].MatchesItem(this->grid_source->GetAt(idx)) != 0) {
                            main_wnd->m_GameSession.shortcuts[i].kind = 0;
                        }
                    }
#endif
                    main_wnd->m_GameSession.SaveShortcuts();
                }
            }
        } else {
#if defined(A2CLIENT) && defined(A2TANGAR_HAT)
            // Tangar item_hotkey_find (4AB61A): match against the custom hotkey.
            UserShortcut* tangar_hotkey = tangar_hotkeys::LookupHotkey((int32_t)wparam + VK_F4);
            if (tangar_hotkey != nullptr && this->grid_source != nullptr) {
                for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
                    TokenEntry* candidate = this->grid_source->GetAt(i);
                    // p10mny.xck (4ab621 -> 6e20ee): skip money slot (item_id == 0xFFFF) before MatchesItem
                    if (candidate->item_id != ItemId::REWARD_GOLD && tangar_hotkey->MatchesItem(candidate) != 0) {
                        TokenEntry* entry = this->PickupSlotItem(i, 1);
                        if (entry != nullptr) {
                            main_wnd->vis_charinfo->EquipDraggedItem(entry->GetType() - 1);
                        }
                        break;
                    }
                }
            }
#else
            if (this->grid_source != nullptr) {
                for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
                    if (main_wnd->m_GameSession.shortcuts[wparam].MatchesItem(this->grid_source->GetAt(i)) != 0) {
                        TokenEntry* entry = this->PickupSlotItem(i, 1);
                        if (entry != nullptr) {
                            main_wnd->vis_charinfo->EquipDraggedItem(entry->GetType() - 1);
                        }
                        break;
                    }
                }
            }
#endif
        }
    }

    return VisInvBase::MsgProc(msg, wparam, lparam);
}


// 4a765d (client: 4ab68b)
const char* VisBagInventory::GetHint() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        return nullptr;
    }
    if (main_wnd->dialogsMask != DIALOG_MASK_INGAME) {
        return nullptr;
    }

    int32_t edge_width = ((g_ScreenSize.right - 0xf0) % 0x50) / 2;
    int32_t mouse_x = g_mousept.GetX();

    if (mouse_x < edge_width + 0x20) {
        return TxtFile::AllLines[0x36];
    }
    if (mouse_x >= edge_width + 0x20 + this->visible_columns * 0x50) {
        return TxtFile::AllLines[0x37];
    }
    if (this->grid_source == nullptr) {
        return TxtFile::AllLines[0x3a];
    }

    int32_t idx = *this->visible_startref + ((mouse_x - 0x20) - edge_width) / 0x50;
    if (idx >= this->grid_source->GetSize()) {
        return TxtFile::AllLines[0x3a];
    }
    TokenEntry* entry = this->grid_source->GetAt(idx);
    if (entry == nullptr) {
        return nullptr;
    }
    if (entry->IsEmpty() != 0) {
        return TxtFile::AllLines[0x4a];
    }
    return entry->GetHintText();
}


// 4a6d11 (client: 4aad40)
int32_t VisBagInventory::GetSlotAt(int32_t x, int32_t y) {
    if (this->grid_source == nullptr) {
        return -1;
    }

    int32_t edge_width = ((g_ScreenSize.right - 0xf0) % 0x50) / 2;

    if (y < 0) {
        if (x < edge_width + 0x20) {
            return -1;
        }
        if (x >= edge_width + 0x20 + this->visible_columns * 0x50) {
            return -1;
        }
    } else {
        if (x < edge_width + 0x20) {
            return *this->visible_startref;
        }
        if (x >= edge_width + 0x20 + this->visible_columns * 0x50) {
            if (*this->visible_startref + this->visible_columns * this->visible_rows - 1 < this->grid_source->GetSize()) {
                return *this->visible_startref + this->visible_columns * this->visible_rows - 1;
            }
        }
    }

    int32_t idx = *this->visible_startref + ((x - 0x20) - edge_width) / 0x50;
    if (idx >= this->grid_source->GetSize()) {
        return -1;
    }
    return idx;
}


// 4a6e57 (client: 4aae86)
int32_t VisBagInventory::OnLButtonDown(uint32_t wparam, CPoint pos) {
    CRect screen_rect = this->ClientRectToScreen(this->rect);

    int32_t edge_width = ((g_ScreenSize.right - 0xf0) % 0x50) / 2;

    CRect left_rect(screen_rect.left, screen_rect.top, screen_rect.left + 0x20 + edge_width, screen_rect.bottom);
    CRect right_rect(screen_rect.right - 0x30 - edge_width, screen_rect.top, screen_rect.right - 0x10, screen_rect.bottom);

    if (left_rect.PtInRect(pos)) {
        this->ScrollCellsBack();
    }
    if (right_rect.PtInRect(pos)) {
        this->ScrollCellsForward();
    }

    return 1;
}


// 4a70ec (client: 4ab11a)
int32_t VisBagInventory::OnLButtonUp(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (this->grid_source != nullptr) {
        int32_t idx = this->GetSlotAt(pos.x, pos.y);
        if (main_wnd->drag_entry != nullptr) {
            ApplyCursor(g_Cursors[0]);
            this->PlaceDraggedItem(idx);
        }
    }
    return 1;
}


// 4a741d (client: 4ab44b)
int32_t VisBagInventory::OnWmUser(uint32_t wparam, CPoint pos) {
    return this->OnLButtonDown(wparam, pos);
}


// 4a6f32 (client: 4aaf61)
int32_t VisBagInventory::OnRButtonDown(uint32_t wparam, CPoint pos) {
    return 1;
}


// 4a715d (client: 4ab18b)
int32_t VisBagInventory::OnRButtonUp(uint32_t wparam, CPoint pos) {
    return 1;
}


// 4a70da (client: 4ab108)
int32_t VisBagInventory::OnRButtonDblClk(uint32_t wparam, CPoint pos) {
    return 1;
}


// 4a79c0 (client: 4ab9a0)
int32_t VisBagInventory::AppendItem(TokenEntry* o) {
    o->slot_code = 2;
    return VisInvBase::AppendItem(o);
}


// 4a79f0 (client: 4ab9d0)
int32_t VisBagInventory::InsertItem(TokenEntry* o, int32_t idx) {
    o->slot_code = 2;
    return VisInvBase::InsertItem(o, idx);
}


// 4a7830 (client: 4aba40)
VisBagInventory::~VisBagInventory() {
}


// 4a7a20 (client: 4aba00)
int32_t VisBagInventory::GetTransferTab() {
    return TRANSFER_DST_BAG;
}


// 4a630b (client: 4aa33a)
VisBagInventory::VisBagInventory(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : VisInvBase(_id, l, t, r, b) {
    this->visible_columns = this->rect.Width() / 0x50 - 1;
    this->visible_rows = this->rect.Height() / 0x50;
    this->InitCellArrays();
}


// 4a6f44 (client: 4aaf73)
int32_t VisBagInventory::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    if (this->grid_source == nullptr) {
        return 1;
    }

    int32_t idx = this->GetSlotAt(pos.x, -pos.y);
    if (idx == -1) {
        return 1;
    }

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (this->grid_source->GetAt(idx)->IsEmpty() != 0) {
        if (main_wnd->vis_map_context->my_main_unit->gold > 0) {
            main_wnd->dialogsMask |= DIALOG_MASK_POPUP;
            main_wnd->vis_root->AddChild(main_wnd->vis_dropgold);
            main_wnd->vis_dropgold->SetSelection(idx);
            main_wnd->vis_dropgold->DoOpen();
            main_wnd->vis_root->Redraw();
            ApplyCursor(g_Cursors[0]);
            if (g_mousept.GetSelectState() != 0) {
                g_mousept.ResetStates();
            }
            main_wnd->app_initialized = 0;
        }
    } else {
        if (this->grid_source->GetAt(idx)->IsQuestTreasure() == 0) {
            TokenEntry* entry = this->PickupSlotItem(idx, 1);
            if (entry != nullptr) {
                main_wnd->vis_charinfo->EquipDraggedItem(entry->GetType() - 1);
            }
        }
    }

    return 1;
}


// 4b5072 (client: 4b9a42)
void VisInvExtBase::Draw() {
    VisShop* shop = this->shop;
    int32_t off_x = shop->rect.left;
    int32_t off_y = shop->rect.top;

    if (shop->dialog_active == 0) {
        return;
    }

    this->RefreshCellSprites();

    if (this->grid_source == nullptr) {
        return;
    }

    int32_t total_cells = this->visible_columns * this->visible_rows;
    int32_t draw_count = this->grid_source->GetSize();
    if (draw_count >= total_cells) {
        draw_count = total_cells;
    }

    LockBackSurface();
    this->TickCellAnims();

    for (int32_t i = 0; i < total_cells; i++) {
        CPoint pos(this->cell_rects[i].left + off_x, this->cell_rects[i].top + off_y);
        if (i >= draw_count) {
            continue;
        }
        if (this->cell_update_counter.GetAt(i) == 0) {
            continue;
        }
        TokenEntry* entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry->IsEmpty() != 0) {
            if (shop->gameplay != nullptr) {
                entry->count = shop->gameplay->my_main_unit->gold;
            }
            g_bmp_backinv->Draw(pos.x, pos.y + 1, 0, 0, 0);
            g_ca16_money->Draw(pos.x, pos.y + 1, 0, 0, 0);
        } else if (entry->IsDepleted() != 0) {
            shop->bmp_backinvg->Draw(pos.x, pos.y + 1, 0, 0, 0);
        } else {
            CUnit* unit = shop->selected_units[shop->select_index];
            int32_t compatible = unit->IsItemRestricted(entry);
            if (entry->slot_code >= 5 && entry->slot_code <= 8) {
                int32_t price = entry->GetAttribute(1);
                if (shop->current_gold < price || compatible == 0) {
                    shop->bmp_backinvg->Draw(pos.x, pos.y + 1, 0, 0, 0);
                } else {
                    shop->bmp_backinvs->Draw(pos.x, pos.y + 1, 0, 0, 0);
                }
            } else {
                if (compatible != 0) {
                    g_bmp_backinv->Draw(pos.x, pos.y + 1, 0, 0, 0);
                } else {
                    shop->bmp_backinvg->Draw(pos.x, pos.y + 1, 0, 0, 0);
                }
            }
            CSprite256* sprite = this->spr_cells.GetAt(i);
            sprite->Draw(pos.x, pos.y + 1, 0, 0, 0);
            if (entry->flg & 0x20) {
                this->DrawGlowTrail(pos.x, pos.y + 1, i);
            }
        }

        entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry->count > 1) {
            CPoint bottom_right(this->cell_rects[i].right + off_x, this->cell_rects[i].bottom + off_y);
            CString text;
            text.Format("%d", entry->count);
            InsertThousandsSeparators(&text);
            g_font2->DrawTextWithShadow(pos.x + 10, bottom_right.y - 15, text, CGameFont::ALIGN_LEFT_TOP, clrsh_DullGold, 1);
        }

        entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry->IsEmpty() == 0 && entry->IsDepleted() == 0) {
            CPoint cost_pos(this->cell_rects[i].right + off_x, this->cell_rects[i].top + off_y + 1);
            CArray<CBmp64*>* cost_arr;
            if (entry->slot_code == 2) {
                cost_arr = &shop->bmp_cost_medium;
            } else {
                cost_arr = &shop->bmp_cost_small;
            }
            int32_t digits = (int32_t)log10((double)entry->GetAttribute(1));
            if (digits > cost_arr->GetUpperBound()) {
                digits = cost_arr->GetUpperBound();
            }
            CBmp64* cost_bmp = cost_arr->GetAt(digits);
            // the tag is right-anchored: shift left by its width before drawing
            cost_pos.x -= cost_bmp->GetWidth(0);
            cost_bmp->DrawBrightened(cost_pos.x - 3, cost_pos.y + 2, 0, 8, 0);
            int32_t bmp_w = cost_bmp->GetWidth(0);
            int32_t bmp_h = cost_bmp->GetHeight(0);
            cost_bmp->DrawSubRectMasked(cost_pos.x, cost_pos.y, 0, 0, bmp_w, bmp_h);
            int32_t amount;
            if (entry->slot_code == 2) {
                amount = (entry->GetAttribute(1) + 1) / 2;
            } else {
                amount = entry->GetAttribute(1);
            }
            CString cost_text;
            cost_text.Format("%d", amount);
            InsertThousandsSeparators(&cost_text);
            g_font2->DrawTextWithShadow(cost_pos.x - 6 + cost_bmp->GetWidth(0), cost_pos.y, cost_text, CGameFont::ALIGN_RIGHT, clrsh_DullGold, 1);
        }
    }

    shop->DrawInfoPanelEdge();
    UnlockBackSurface();
    shop->dirty |= 0x40;
}


// 4b5f3f (client: 4ba90d)
int32_t VisInvExtBase::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    VisShop* shop = this->shop;
    int32_t idx = this->GetSlotAtPoint(&pos);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (idx >= 0 && this->grid_source->GetSize() > idx) {
        TokenEntry* entry = this->grid_source->GetAt(idx);
        if (entry->IsQuestTreasure() == 0) {
            this->PickupSlotItem(idx, 1);
            if (main_wnd->drag_entry != nullptr) {
                if (main_wnd->drag_src_tab == 4) {
                    switch (main_wnd->drag_entry->slot_code) {
                    case 1:
                        ((VisCharInfo*)shop->select_info_panel)->EquipDraggedItem(main_wnd->drag_src_slot);
                        shop->dirty |= 8;
                        break;
                    case 2:
                        shop->to_sell->PlaceDraggedItem(*shop->to_sell->visible_startref);
                        shop->to_sell->RefreshGrid();
                        break;
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                        shop->assortiment->grid_source = &shop->assortiment->category_grids[main_wnd->drag_entry->slot_code - 5];
                        shop->assortiment->PlaceDraggedItem(-1);
                        shop->assortiment->grid_source = &shop->assortiment->category_grids[shop->select_category];
                        shop->assortiment->RefreshGrid();
                        break;
                    }
                    this->RefreshGrid();
                    shop->RecalcShopGold();
                    shop->dirty |= 0x20;
                } else if (main_wnd->drag_src_tab == 2) {
                    ((VisCharInfo*)shop->select_info_panel)->EquipDraggedItem(main_wnd->drag_entry->GetType() - 1);
                } else {
                    if (shop->to_buy->IsBuyGridFull(main_wnd->drag_entry) == 0) {
                        shop->to_buy->PlaceDraggedItem(*shop->to_buy->visible_startref);
                        this->RefreshGrid();
                        this->Redraw();
                        shop->to_buy->RefreshGrid();
                        shop->RecalcShopGold();
                        shop->dirty |= 0x20;
                    }
                }
            }
        }
    }
    return 1;
}


// 4b5d5c (client: 4ba72b)
int32_t VisInvExtBase::OnLButtonUp(uint32_t wparam, CPoint pos) {
    VisShop* shop = this->shop;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry == nullptr) {
        return 1;
    }
    ApplyCursor(g_Cursors[0]);
    if (main_wnd->drag_entry->GetAttribute(1) == 0 && this->GetTransferTab() != 2) {
        shop->placement_lock = 0;
    }
    if (shop->placement_lock == 0) {
        shop->CancelItemDrag();
        CSound::Play((CSound&)shop->snd_notif);
        return 1;
    }
    int32_t region = this->GetTransferTab();
    if (region == 4) {
        this->PlaceDraggedItem(this->GetSlotAtPoint(&pos));
        shop->RecalcShopGold();
        shop->dirty |= 0x20;
    } else if (region < 4 || region > 8) {
        this->PlaceDraggedItem(this->GetSlotAtPoint(&pos));
    } else {
        if (this->GetTransferTab() == main_wnd->drag_entry->slot_code) {
            this->PlaceDraggedItem(this->GetSlotAtPoint(&pos));
        } else {
            shop->CancelItemDrag();
        }
    }
    this->EndItemDrag();
    this->RefreshGrid();
    this->Redraw();
    return 1;
}


// 4b5bbe (client: 4ba58d)
int32_t VisInvExtBase::OnMouseMove(uint32_t wparam, CPoint pos) {
    VisShop* shop = this->shop;
    if (shop->hovered_region != this->GetTransferTab()) {
        shop->hovered_region = this->GetTransferTab();
        shop->buttons->ResetSelected();
    }
    if (wparam & 1) {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        if (main_wnd->drag_src_slot >= 0 && main_wnd->drag_src_slot < this->grid_source->GetSize() && main_wnd->drag_entry == nullptr) {
            int32_t stack_count = this->grid_source->GetAt(main_wnd->drag_src_slot)->count;
            int32_t count;
#ifdef A2CLIENT_PATCH
            // p10itm.xck (4ba637 -> 6e21d0): Shift = all, Alt = min(count, 1000), Ctrl = min(count, 10), else 1
            if (g_kbShiftState != 0) {
                count = stack_count;
            } else if (g_kbMenuState != 0) {
                count = (stack_count > 1000) ? 1000 : stack_count;
            } else if (g_kbControlState != 0) {
                count = (stack_count > 10) ? 10 : stack_count;
            } else {
                count = 1;
            }
#else
            if (g_kbShiftState != 0) {
                count = stack_count;
            } else {
                count = 1;
            }
#endif
            if (this->grid_source->GetAt(main_wnd->drag_src_slot)->IsQuestItem() == 0) {
                if (this->PickupSlotItem(main_wnd->drag_src_slot, count) != 0) {
                    this->StartItemDrag(main_wnd->drag_src_slot);
                    this->ClampVisibleStart();
                    this->RefreshGrid();
                    this->Redraw();
                }
            }
        }
    }
    return 0;
}


// 4b4e88 (client: 4b9858)
int32_t VisInvExtBase::GetSlotAtPoint(const CPoint* pt) {
    VisShop* shop = this->shop;
    CPoint rel = *pt - CPoint(shop->rect.left, shop->rect.top);
    for (int32_t i = 0; i < this->visible_columns * this->visible_rows; i++) {
        if (this->cell_rects[i].PtInRect(rel)) {
            int32_t idx = i + *this->visible_startref;
            if (idx <= this->grid_source->GetUpperBound() && this->grid_source->GetAt(idx)->item_id != ItemId::EMPTY_EQUIP_SLOT) {
                return idx;
            }
        }
    }
    return -1;
}


// 4b4c64 (client: 4b9634)
const char* VisInvExtBase::GetHint() {
    VisShop* shop = this->shop;
    if (shop->dialog_active == 0) {
        return nullptr;
    }
    int32_t idx = this->GetSlotAt(g_mousept.GetX(), g_mousept.GetY());
    if (this->grid_source->GetAt(idx) == nullptr) {
        return nullptr;
    }
    if (this->grid_source->GetAt(idx)->IsEmpty() != 0) {
        return TxtFile::AllLines.GetAt(0x4A);
    }
    if (this->grid_source->GetAt(idx)->IsDepleted() != 0) {
        return nullptr;
    }
    return this->grid_source->GetAt(idx)->GetHintText();
}


// 4b4fd1 (client: 4b99a1)
void VisInvExtBase::ClampVisibleStart() {
    int32_t size = this->grid_source->GetSize();
    if (size - *this->visible_startref < this->visible_columns * this->visible_rows) {
        int32_t start = size - this->visible_columns * this->visible_rows;
        if (start <= 0) {
            start = 0;
        }
        *this->visible_startref = start;
    }
}


// 4b4d33 (client: 4b9703)
void VisInvExtBase::RefreshGrid() {
    this->ClampVisibleStart();
    for (int32_t i = 0; i < this->visible_columns * this->visible_rows; i++) {
        this->cell_update_counter.ElementAt(i) = 0;
    }
    this->RefreshCellSprites();
}


// 4b4bc5 (client: 4b9595)
void VisInvExtBase::LoadGridSfx() {
    this->FreeGridSfx();
    ReloadSfx(&this->snd_put_on.sample, "SFX\\Put_On.wav");
    ReloadSfx(&this->snd_put_off.sample, "SFX\\Put_Off.wav");
    ReloadSfx(&this->snd_scroll.sample, "SFX\\Scroll.wav");
}


// 4b5d19 (client: 4ba6e8)
int32_t VisInvExtBase::OnLButtonDown(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry == nullptr) {
        main_wnd->drag_src_slot = this->GetSlotAtPoint(&pos);
    }
    return 1;
}


// 4b4ba9 (client: 4b9579)
void VisInvExtBase::FreeSpeechSfx() {
    FreeSfxSample(&this->snd_speech.sample);
}


// 4b4c1c (client: 4b95ec)
void VisInvExtBase::FreeGridSfx() {
    this->FreeSpeechSfx();
    FreeSfxSample(&this->snd_put_on.sample);
    FreeSfxSample(&this->snd_put_off.sample);
    FreeSfxSample(&this->snd_scroll.sample);
}


// 4b4fa3 (client: 4b9973)
int32_t VisInvExtBase::GetSlotAt(int32_t x, int32_t y) {
    CPoint pt(x, y);
    return this->GetSlotAtPoint(&pt);
}


// 4b4d91 (client: 4b9761)
void VisInvExtBase::BindGridSource(CArray<TokenEntry*>* arr) {
    this->grid_source = arr;
}


// 4b9e40 (client: 4be810)
int32_t VisInvExtBase::GetTransferTab() {
    return -1;
}


// 4b9e30 (client: 4be800)
void VisInvExtBase::InitCellRects() {
}


// 4b4daa (client: 4b977a)
void VisInvExtBase::ReplaceGridEntries(CArray<TokenEntry*>* arr) {
    if (this->GetTransferTab() != 4) {
        for (int32_t i = 0; i < arr->GetSize(); i++) {
            arr->GetAt(i)->slot_code = this->GetTransferTab();
        }
    }
    while (this->grid_source->GetSize() != 0) {
        TokenEntry* entry = this->grid_source->GetAt(0);
        if (entry != nullptr) {
            delete entry;
        }
        this->grid_source->RemoveAt(0, 1);
    }
    this->grid_source->InsertAt(0, arr);
    arr->RemoveAll();
}


// 4b9e50 (client: 4be820)
void VisInvExtBase::LoadBitmaps() {
}


// 4b9e60 (client: 4be830)
void VisInvExtBase::FreeBitmaps() {
}


// 4b62a2 (client: 4bac6c)
int32_t VisInvExtBase::OnRButtonDown(uint32_t wparam, CPoint pos) {
    return 1;
}


// 4b62b4 (client: 4bac7e)
int32_t VisInvExtBase::OnRButtonUp(uint32_t wparam, CPoint pos) {
    return 1;
}


// 4b62c6 (client: 4bac90)
int32_t VisInvExtBase::OnRButtonDblClk(uint32_t wparam, CPoint pos) {
    return 1;
}


// 4b4a0c (client: 4b93dc)
VisInvExtBase::VisInvExtBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
: VisInvBase(_id, l, t, r, b) {
    this->grid_source = nullptr;
    this->shop = shop;
    this->flags |= 2;
    this->visible_startref = new int32_t(0);
    this->snd_speech.sample = nullptr;
    this->snd_put_on.sample = nullptr;
    this->snd_put_off.sample = nullptr;
    this->snd_scroll.sample = nullptr;
    // field_0x20c4 is deliberately left uninitialized here, as in the original
}


// 4b4aec (client: 4b94bc)
VisInvExtBase::~VisInvExtBase() {
    if (this->cell_rects != nullptr) {
        delete this->cell_rects;
    }
    this->shop = nullptr;
    this->grid_source = nullptr;
    if (this->visible_startref != nullptr) {
        delete this->visible_startref;
    }
    this->FreeSpeechSfx();
    this->FreeGridSfx();
}


// 4b5a6c (client: 4ba43b)
int32_t VisInvExtBase::StartItemDrag(int32_t idx) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    TokenEntry* entry = main_wnd->drag_entry;
    int32_t region = this->GetTransferTab();
    CString name = entry->GetSpriteName();
    CString cs1 = "graphics\\inventory\\" + name;
    CString cs2 = cs1 + ".16a";
    main_wnd->SetItemCursor(entry, idx, cs2, region);
    ApplyCursor(main_wnd->item_cursor);
    this->shop->placement_lock = 1;
    return 1;
}


// 4b5b81 (client: 4ba550)
int32_t VisInvExtBase::EndItemDrag() {
    ApplyCursor(g_Cursors[0]);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->ResetItemCursor();
    this->shop->placement_lock = 0;
    return 1;
}


// 4b7a9f (client: 4bc469)
TokenEntry* VisShopShelfGrid::SplitItem(TokenEntry* o, int32_t num) {
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (!o->IsStackableWith(entry)) {
            continue;
        }
        if (entry->IsDepleted()) {
            return nullptr;
        }
        if (entry->RemoveCount(num)) {
            o = new TokenEntry();
            *o = *entry;
            o->count = num;
        } else if (entry->IsEmpty()) {
            o = new TokenEntry();
            *o = *entry;
            entry->count = 0;
        } else {
            o = new TokenEntry();
            *o = *entry;
            o->count = num;
            entry->count = 0;
            entry->item_id = 0;
        }
        o->gap_0x1c = i;
        o->slot_code = this->GetTransferTab();
        return o;
    }
    return nullptr;
}


// 4b70c0 (client: 4bba8a)
void VisShopShelfGrid::Draw() {
    VisShop* shop = this->shop;
    CPoint topleft = shop->rect.TopLeft();

    if (shop->dialog_active == 0 || this->shop_inv == nullptr) {
        return;
    }

    LockBackSurface();
    this->shop_inv->Draw(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, 0);
    UnlockBackSurface();
    VisInvExtBase::Draw();

    if (*this->visible_startref > 0) {
        CPoint pos(g_mousept.GetX(), g_mousept.GetY());
        CRect rc = this->up_arrow_rect;
        rc.OffsetRect(topleft.x, topleft.y);
        if (rc.PtInRect(pos)) {
            this->DrawUpArrowHover();
        } else {
            this->DrawUpArrowIdle();
        }
    }
    if (this->grid_source->GetSize() - *this->visible_startref > this->visible_columns * this->visible_rows) {
        CPoint pos(g_mousept.GetX(), g_mousept.GetY());
        CRect rc = this->down_arrow_rect;
        rc.OffsetRect(topleft.x, topleft.y);
        if (rc.PtInRect(pos)) {
            this->DrawDownArrowHover();
        } else {
            this->DrawDownArrowIdle();
        }
    }
}


// 4b790d (client: 4bc2d7)
int32_t VisShopShelfGrid::InsertItem(TokenEntry* o, int32_t idx) {
    o->slot_code = o->gap_0x14 + 5;
    if (o->slot_code >= 5 && o->slot_code <= 8 && o->gap_0x1c >= 0 && o->gap_0x1c < this->grid_source->GetSize()) {
        TokenEntry* entry = this->grid_source->GetAt(o->gap_0x1c);
        if (entry->IsDepleted()) {
            *entry = *o;
        } else {
            entry->AddCount(o->count);
        }
        return o->gap_0x1c;
    }
    if (idx >= 0 && idx < this->grid_source->GetSize()) {
        this->grid_source->InsertAt(idx, o, 1);
        return idx;
    }
    if (this->grid_source->GetSize() != 0 && this->grid_source->GetAt(this->grid_source->GetUpperBound())->IsEmpty()) {
        this->grid_source->InsertAt(this->grid_source->GetUpperBound(), o, 1);
        return this->grid_source->GetUpperBound() - 1;
    }
    this->grid_source->Add(o);
    return this->grid_source->GetUpperBound();
}


// 4b6f8d (client: 4bb957)
const char* VisShopShelfGrid::GetHint() {
    VisShop* shop = this->shop;
    if (shop->dialog_active == 0) {
        return nullptr;
    }
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        return nullptr;
    }
    CPoint mouse(g_mousept.GetX(), g_mousept.GetY());
    CPoint topleft = shop->rect.TopLeft();
    CPoint rel = mouse - topleft;
    // WAT: vanilla (4B6F8D, client 4bb957) passes the already shop-relative point here, but `GetSlotAtPoint` subtracts the shop origin again, so
    // at resolutions above 640x480 every shelf item hints as "shop shelf". We pass the absolute mouse position instead.
    int32_t region = this->GetSlotAtPoint(&mouse);
    if (region >= 0) {
        return VisInvExtBase::GetHint();
    }
    if (this->up_arrow_rect.PtInRect(rel)) {
        return TxtFile::AllLines[0x38];
    }
    if (this->down_arrow_rect.PtInRect(rel)) {
        return TxtFile::AllLines[0x39];
    }
    return TxtFile::AllLines[0x3C];
}


// 4b7562 (client: 4bbf2c)
int32_t VisShopShelfGrid::OnLButtonDown(uint32_t wparam, CPoint pos) {
    VisShop* shop = this->shop;
    CPoint topleft = shop->rect.TopLeft();
    CPoint pt = pos - topleft;
    if (this->up_arrow_rect.PtInRect(pt)) {
        this->ScrollCellsBack();
        this->ScrollCellsBack();
        StopSfx(&this->snd_scroll.sample);
        CSound::Play(this->snd_scroll);
        return 1;
    }
    if (this->down_arrow_rect.PtInRect(pt)) {
        this->ScrollCellsForward();
        this->ScrollCellsForward();
        StopSfx(&this->snd_scroll.sample);
        CSound::Play(this->snd_scroll);
        return 1;
    }
    return VisInvExtBase::OnLButtonDown(wparam, pos);
}


// 4b7433 (client: 4bbdfd)
int32_t VisShopShelfGrid::OnWmUser(uint32_t wparam, CPoint pos) {
    VisShop* shop = this->shop;
    CPoint topleft = shop->rect.TopLeft();
    CPoint pt = pos - topleft;
    if (wparam & 1) {
        if (this->up_arrow_rect.PtInRect(pt)) {
            this->ScrollCellsBack();
            this->ScrollCellsBack();
            StopSfx(&this->snd_scroll.sample);
            CSound::Play(this->snd_scroll);
            return 1;
        }
        if (this->down_arrow_rect.PtInRect(pt)) {
            this->ScrollCellsForward();
            this->ScrollCellsForward();
            StopSfx(&this->snd_scroll.sample);
            CSound::Play(this->snd_scroll);
            return 1;
        }
    }
    return 1;
}


// 4b7859 (client: 4bc223)
void VisShopShelfGrid::ClearCategoryGrids() {
    for (int32_t i = 0; i < 4; i++) {
        CArray<TokenEntry*>& arr = this->category_grids[i];
        for (int32_t j = 0; j < arr.GetSize(); j++) {
            TokenEntry* entry = arr[j];
            if (entry != nullptr) {
                delete entry;
            }
        }
        arr.RemoveAll();
    }
}


// 4b777c (client: 4bc146)
int32_t VisShopShelfGrid::OnKeyDown(uint32_t wparam) {
#ifdef A2CLIENT_PATCH
    // Tangar pack_scrolling (PSCRL_shopShelfOnKeyDown, client vtable slot
    // 0x5D92F4): the shelf page keys only reach the retail handler with Ctrl held.
    if (GetAsyncKeyState(VK_CONTROL) == 0) {
        return 0;
    }
#endif
    if (wparam == VK_PRIOR) {
        this->ScrollCellsBack();
        this->ScrollCellsBack();
        this->ScrollCellsBack();
        this->ScrollCellsBack();
        this->ScrollCellsBack();
        this->ScrollCellsBack();
        StopSfx(&this->snd_scroll.sample);
        CSound::Play(this->snd_scroll);
        return 1;
    }
    if (wparam == VK_NEXT) {
        this->ScrollCellsForward();
        this->ScrollCellsForward();
        this->ScrollCellsForward();
        this->ScrollCellsForward();
        this->ScrollCellsForward();
        this->ScrollCellsForward();
        StopSfx(&this->snd_scroll.sample);
        CSound::Play(this->snd_scroll);
        return 1;
    }
    return 0;
}


// 4b7690 (client: 4bc05a)
int32_t VisShopShelfGrid::OnMouseMove(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        switch (main_wnd->drag_entry->slot_code) {
        case 5:
        case 6:
        case 7:
        case 8:
            this->shop->placement_lock = 1;
            ApplyCursor(main_wnd->item_cursor);
            break;
        default:
            ApplyCursor(g_Cursors[23]);
            this->shop->placement_lock = 0;
            break;
        }
    }
    return VisInvExtBase::OnMouseMove(wparam, pos);
}


// 4b6522 (client: 4baeec)
VisShopShelfGrid::~VisShopShelfGrid() {
    for (int32_t i = 0; i < 4; i++) {
        CArray<TokenEntry*>& arr = this->category_grids[i];
        for (int32_t j = 0; j < arr.GetSize(); j++) {
            TokenEntry* entry = arr[j];
            if (entry != nullptr) {
                delete entry;
            }
        }
        arr.RemoveAll();
    }
    this->grid_source = nullptr;
}


// 4b7412 (client: 4bbddc)
int32_t VisShopShelfGrid::OnLButtonUp(uint32_t wparam, CPoint pos) {
    return VisInvExtBase::OnLButtonUp(wparam, pos);
}


// 4b73e4 (client: 4bbdae)
void VisShopShelfGrid::SelectCategoryGrid(int32_t category) {
    this->grid_source = &this->category_grids[category];
    this->RefreshGrid();
}


// 4b63f7 (client: 4badc1)
VisShopShelfGrid::VisShopShelfGrid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisInvExtBase(_id, l, t, r, b, shop) {
    this->visible_columns = this->rect.Width() / 0x50;
    this->visible_rows = this->rect.Height() / 0x50;
    this->InitCellRects();
    this->VisInvBase::InitCellArrays();
    this->arrow1 = nullptr;
    this->arrow3 = nullptr;
    this->arrow2 = nullptr;
    this->arrow4 = nullptr;
    this->shop_inv = nullptr;
}


// 4c6c40 (client: 4cb5e0)
VisShopShelfGridDruid::VisShopShelfGridDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisShopShelfGrid(_id, l, t, r, b, shop) {
}


// 4c6cd0 (client: 4cb670)
VisShopShelfGridKaarg::VisShopShelfGridKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisShopShelfGrid(_id, l, t, r, b, shop) {
}


// 4b6d46 (client: 4bb710)
void VisShopShelfGrid::InitCellRects() {
    int32_t total_cells = this->visible_columns * this->visible_rows;
    this->cell_rects = new CRect[total_cells];

    CPoint topleft = this->rect.TopLeft();
    this->up_arrow_rect = CRect(topleft.x + 0x2E, topleft.y, topleft.x + 0x76, topleft.y + 0x20);
    this->down_arrow_rect = CRect(topleft.x + 0x2E, topleft.y + 0x10F, topleft.x + 0x76, topleft.y + 0x12F);
    CPoint origin(this->rect.left, this->up_arrow_rect.bottom);

    for (int32_t i = 0; i < this->visible_columns; i++) {
        for (int32_t j = 0; j < this->visible_rows; j++) {
            this->cell_rects[j * this->visible_columns + i] = CRect(
                origin.x + i * 0x50 + 1,
                origin.y + j * 0x50 - 1,
                origin.x + i * 0x50 + 0x51,
                origin.y + j * 0x50 + 0x4F);
        }
    }
}


// 4b663c (client: 4bb006)
void VisShopShelfGrid::LoadBitmaps() {
    this->FreeBitmaps();
    this->arrow1 = new CBmp64("graphics\\interface\\ShopArrow1.bmp");
    g_mousept.Update();
    this->arrow3 = new CBmp64("graphics\\interface\\ShopArrow3.bmp");
    g_mousept.Update();
    this->arrow2 = new CBmp64("graphics\\interface\\ShopArrow2.bmp");
    g_mousept.Update();
    this->arrow4 = new CBmp64("graphics\\interface\\ShopArrow4.bmp");
    g_mousept.Update();
    this->shop_inv = new CBmp64("graphics\\interface\\ShopInv.bmp");
    g_mousept.Update();
}


// 4b69ee (client: 4bb3b8)
void VisShopShelfGridKaarg::LoadBitmaps() {
    this->FreeBitmaps();
    this->arrow1 = new CBmp64("graphics\\interface\\shop_kaarg\\ShopArrow1.bmp");
    g_mousept.Update();
    this->arrow3 = new CBmp64("graphics\\interface\\shop_kaarg\\ShopArrow3.bmp");
    g_mousept.Update();
    this->arrow2 = new CBmp64("graphics\\interface\\shop_kaarg\\ShopArrow2.bmp");
    g_mousept.Update();
    this->arrow4 = new CBmp64("graphics\\interface\\shop_kaarg\\ShopArrow4.bmp");
    g_mousept.Update();
    this->shop_inv = new CBmp64("graphics\\interface\\shop_kaarg\\ShopInv.bmp");
    g_mousept.Update();
}


// 4b6815 (client: 4bb1df)
void VisShopShelfGridDruid::LoadBitmaps() {
    this->FreeBitmaps();
    this->arrow1 = new CBmp64("graphics\\interface\\shop_druid\\ShopArrow1.bmp");
    g_mousept.Update();
    this->arrow3 = new CBmp64("graphics\\interface\\shop_druid\\ShopArrow3.bmp");
    g_mousept.Update();
    this->arrow2 = new CBmp64("graphics\\interface\\shop_druid\\ShopArrow2.bmp");
    g_mousept.Update();
    this->arrow4 = new CBmp64("graphics\\interface\\shop_druid\\ShopArrow4.bmp");
    g_mousept.Update();
    this->shop_inv = new CBmp64("graphics\\interface\\shop_druid\\ShopInv.bmp");
    g_mousept.Update();
}


// 4b6bc7 (client: 4bb591)
void VisShopShelfGrid::FreeBitmaps() {
    if (this->arrow1 != nullptr) {
        delete this->arrow1;
    }
    this->arrow1 = nullptr;
    if (this->arrow3 != nullptr) {
        delete this->arrow3;
    }
    this->arrow3 = nullptr;
    if (this->arrow2 != nullptr) {
        delete this->arrow2;
    }
    this->arrow2 = nullptr;
    if (this->arrow4 != nullptr) {
        delete this->arrow4;
    }
    this->arrow4 = nullptr;
    if (this->shop_inv != nullptr) {
        delete this->shop_inv;
    }
    this->shop_inv = nullptr;
}


// 4b7d1c (client: 4bc6e6)
int32_t VisShopShelfGrid::PlaceDraggedItem(int32_t idx) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry == nullptr) {
        return -1;
    }
    int32_t count = main_wnd->drag_entry->count;
    int32_t tab_before = this->GetTransferTab();
    int32_t new_idx = this->InsertItem(main_wnd->drag_entry, idx);
    TokenEntry* entry = this->grid_source->GetAt(new_idx);
    if (this->GetTransferTab() == 2) {
        this->VisInvBase::BindUnitInventory(main_wnd->vis_map_context->primary_selected_object);
    } else {
        this->BindGridSource(this->grid_source);
    }
    if (tab_before != main_wnd->drag_src_tab) {
        main_wnd->vis_map_context->SendItemOperation(main_wnd->drag_src_tab, main_wnd->drag_src_slot, this->GetTransferTab(), entry->list_index, count);
    }
    main_wnd->ResetItemCursor();
    main_wnd->vis_root->MsgProc(VIS_MSG_SET_OPTION, this->id, 0);
    return new_idx;
}


// 4b7264 (client: 4bbc2e)
void VisShopShelfGrid::DrawUpArrowHover() {
    CPoint topleft = this->shop->rect.TopLeft();
    if (this->arrow1 != nullptr) {
        this->arrow1->Draw(topleft.x + 0x2E, topleft.y, 0, 0, 0);
    }
}


// 4b72c1 (client: 4bbc8b)
void VisShopShelfGrid::DrawUpArrowIdle() {
    CPoint topleft = this->shop->rect.TopLeft();
    if (this->arrow3 != nullptr) {
        this->arrow3->Draw(topleft.x + 0x2E, topleft.y, 0, 0, 0);
    }
}


// 4b731e (client: 4bbce8)
void VisShopShelfGrid::DrawDownArrowHover() {
    CPoint topleft = this->shop->rect.TopLeft();
    if (this->arrow2 != nullptr) {
        this->arrow2->Draw(topleft.x + 0x2E, topleft.y + 0x10F, 0, 0, 0);
    }
}


// 4b7381 (client: 4bbd4b)
void VisShopShelfGrid::DrawDownArrowIdle() {
    CPoint topleft = this->shop->rect.TopLeft();
    if (this->arrow4 != nullptr) {
        this->arrow4->Draw(topleft.x + 0x2E, topleft.y + 0x10F, 0, 0, 0);
    }
}


// 4b9e70 (client: 4be840)
int32_t VisShopShelfGrid::GetTransferTab() {
    return (uint16_t)this->shop->select_category + TRANSFER_DST_SHELF_1;
}


// 4b846c (client: 4bce36)
void VisShopBagGrid::Draw() {
    VisShop* shop = this->shop;
    CPoint topleft = shop->rect.TopLeft();

    if (shop->dialog_active == 0) {
        return;
    }

    LockBackSurface();
    g_bmp_invframe->DrawSubRectMasked(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, this->rect.Width(), this->rect.Height());
    UnlockBackSurface();
    VisInvExtBase::Draw();

    if (*this->visible_startref > 0) {
        CPoint pos(g_mousept.GetX(), g_mousept.GetY());
        CRect rc = this->up_arrow_rect;
        rc.OffsetRect(topleft.x, topleft.y);
        if (rc.PtInRect(pos)) {
            this->DrawUpArrowHover();
        } else {
            this->DrawUpArrowIdle();
        }
    }
    if (this->grid_source->GetSize() - *this->visible_startref > this->visible_columns * this->visible_rows) {
        CPoint pos(g_mousept.GetX(), g_mousept.GetY());
        CRect rc = this->down_arrow_rect;
        rc.OffsetRect(topleft.x, topleft.y);
        if (rc.PtInRect(pos)) {
            this->DrawDownArrowHover();
        } else {
            this->DrawDownArrowIdle();
        }
    }
}


// 4b8339 (client: 4bcd03)
const char* VisShopBagGrid::GetHint() {
    VisShop* shop = this->shop;
    if (shop->dialog_active == 0) {
        return nullptr;
    }
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        return nullptr;
    }
    CPoint mouse(g_mousept.GetX(), g_mousept.GetY());
    CPoint topleft = shop->rect.TopLeft();
    CPoint rel = mouse - topleft;
    int32_t region = this->GetSlotAtPoint(&mouse);
    if (region < 0) {
        if (this->up_arrow_rect.PtInRect(rel)) {
            return TxtFile::AllLines.GetAt(0x36);
        }
        if (this->down_arrow_rect.PtInRect(rel)) {
            return TxtFile::AllLines.GetAt(0x37);
        }
        return TxtFile::AllLines.GetAt(0x3A);
    }
    return VisInvExtBase::GetHint();
}


// 4b87ce (client: 4bd198)
int32_t VisShopBagGrid::OnLButtonDown(uint32_t wparam, CPoint pos) {
    VisShop* shop = this->shop;
    CPoint topleft = shop->rect.TopLeft();
    CPoint rel = pos - topleft;

    if (this->up_arrow_rect.PtInRect(rel)) {
        this->ScrollCellsBack();
        StopSfx(&this->snd_scroll.sample);
        CSound::Play(this->snd_scroll);
        return 1;
    }
    if (this->down_arrow_rect.PtInRect(rel)) {
        this->ScrollCellsForward();
        StopSfx(&this->snd_scroll.sample);
        CSound::Play(this->snd_scroll);
        return 1;
    }
    return VisInvExtBase::OnLButtonDown(wparam, pos);
}


// 4b88ec (client: 4bd2b6)
int32_t VisShopBagGrid::OnMouseMove(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        switch (main_wnd->drag_entry->slot_code) {
        case 1:
        case 2:
            this->shop->placement_lock = 1;
            ApplyCursor(main_wnd->item_cursor);
            break;
        default:
            ApplyCursor(g_Cursors[23]);
            this->shop->placement_lock = 0;
            break;
        }
    }
    return VisInvExtBase::OnMouseMove(wparam, pos);
}


// 4b877a (client: 4bd144)
int32_t VisShopBagGrid::OnWmUser(uint32_t wparam, CPoint pos) {
    if (wparam & 1) {
        this->OnLButtonDown(wparam, pos);
    }
    return 1;
}


// 4b87ad (client: 4bd177)
int32_t VisShopBagGrid::OnLButtonUp(uint32_t wparam, CPoint pos) {
    return VisInvExtBase::OnLButtonUp(wparam, pos);
}


// 4b9ea0 (client: 4be870)
int32_t VisShopBagGrid::AppendItem(TokenEntry* o) {
    o->slot_code = 2;
    return VisInvBase::AppendItem(o);
}


// 4b9ed0 (client: 4be8a0)
int32_t VisShopBagGrid::InsertItem(TokenEntry* o, int32_t idx) {
    o->slot_code = 2;
    return VisInvBase::InsertItem(o, idx);
}


// 4b9457 (client: 4bde21)
int32_t VisShopBuyGrid::InsertItem(TokenEntry* o, int32_t idx) {
    if (o->slot_code == 1) {
        o->slot_code = 2;
    }
    return VisInvBase::InsertItem(o, idx);
}


// 4b95c5 (client: 4bdf8f)
TokenEntry* VisShopBuyGrid::TakeItemByIdAndSlot(int32_t id1, int32_t id2) {
    if (this->grid_source == nullptr) {
        return nullptr;
    }
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (entry->match_id != id1) {
            continue;
        }
        if (entry->slot_code != id2) {
            continue;
        }
        if (entry->RemoveCount(1) == 0) {
            this->grid_source->RemoveAt(i, 1);
            entry->count = 1;
            this->ClampVisibleStart();
            return entry;
        }
        TokenEntry* copy = new TokenEntry();
        *copy = *entry;
        copy->count = 1;
        this->ClampVisibleStart();
        return copy;
    }
    this->ClampVisibleStart();
    return nullptr;
}


// 4b970e (client: 4be0d8)
void VisShopBuyGrid::ClearBuyEntries() {
    while (this->entries.GetSize() != 0) {
        if (this->entries.GetAt(0) != nullptr) {
            delete this->entries.GetAt(0);
        }
        this->entries.RemoveAt(0, 1);
    }
}


// 4b90cf (client: 4bda99)
const char* VisShopBuyGrid::GetHint() {
    VisShop* shop = this->shop;
    if (shop->dialog_active == 0) {
        return nullptr;
    }
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        return nullptr;
    }
    CPoint mouse(g_mousept.GetX(), g_mousept.GetY());
    int32_t region = this->GetSlotAtPoint(&mouse);
    if (region < 0) {
        return TxtFile::AllLines.GetAt(0x3B);
    }
    return VisInvExtBase::GetHint();
}


// 4b8e43 (client: 4bd80d)
void VisShopBuyGrid::FreeBitmaps() {
    if (this->shoptable != nullptr) {
        delete this->shoptable;
    }
    this->shoptable = nullptr;
}


// 4b9b2f (client: 4be4f9)
int32_t VisShopBuyGrid::DropDraggedItem(int32_t idx) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry == nullptr) {
        return -1;
    }
    TokenEntry* entry = main_wnd->drag_entry;
    int32_t amount = entry->count;
    int32_t grade = -1;
    if (main_wnd->drag_src_tab >= 5 && main_wnd->drag_src_tab <= 8) {
        grade = entry->list_index;
    } else {
        grade = main_wnd->drag_src_slot;
    }
    int32_t result = this->InsertItem(entry, idx);
    this->BindGridSource(this->grid_source);
    uint8_t kind = this->GetTransferTab();
    main_wnd->vis_map_context->SendItemOperation(main_wnd->drag_src_tab, grade, kind, result, amount);
    main_wnd->ResetItemCursor();
    main_wnd->vis_root->MsgProc(VIS_MSG_SET_OPTION, this->id, 0);
    return result;
}


// 4b8cc5 (client: 4bd68f)
VisShopBuyGrid::~VisShopBuyGrid() {
    this->grid_source->RemoveAll();
    this->grid_source = nullptr;
    this->FreeGridSfx();
}


// 4b8bba (client: 4bd584)
VisShopBuyGrid::VisShopBuyGrid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisInvExtBase(_id, l, t, r, b, shop) {
    this->visible_columns = this->rect.Width() / 0x50 - 1;
    this->visible_rows = this->rect.Height() / 0x50;
    this->grid_source = &this->entries;
    this->entries.RemoveAll();
    this->InitCellRects();
    this->VisInvBase::InitCellArrays();
    this->shoptable = nullptr;
}


// 4b8d43 (client: 4bd70d)
void VisShopBuyGrid::LoadBitmaps() {
    this->FreeBitmaps();
    CString path = "graphics\\interface\\";
    if (this->shop != nullptr) {
        path += this->shop->GetAssetSubdir();
    }
    path += "ShopTable.bmp";
    this->shoptable = new CBmp64(path);
}


// 4b8e9a (client: 4bd864)
void VisShopBuyGrid::InitCellRects() {
    this->cell_rects = new CRect[this->visible_columns * this->visible_rows];

    CPoint topleft = this->rect.TopLeft();
    CPoint bottomright = this->rect.BottomRight();

    this->up_arrow_rect = CRect(topleft.x, topleft.y, topleft.x + 0x20, bottomright.y);
    this->down_arrow_rect = CRect(topleft.x + 0x1B0, topleft.y, bottomright.x, bottomright.y);

    CPoint start(this->up_arrow_rect.right, this->up_arrow_rect.top);
    for (int32_t col = 0; col < this->visible_columns; col++) {
        for (int32_t row = 0; row < this->visible_rows; row++) {
            this->cell_rects[row * this->visible_columns + col] = CRect(
                start.x + col * 0x50,
                start.y + row * 0x50,
                start.x + col * 0x50 + 0x50,
                start.y + row * 0x50 + 0x50);
        }
    }
}


// 4b9f10 (client: 4be8e0)
int32_t VisShopBuyGrid::GetTransferTab() {
    return TRANSFER_DST_SHOP;
}


// 4b9790 (client: 4be15a)
int32_t VisShopBuyGrid::PlaceDraggedItem(int32_t idx) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (GetRandS16(100) < 0x1E && main_wnd->sessionMode == SESSION_LOCAL_BATTLE && main_wnd->drag_entry != nullptr) {
        TokenEntry* entry = main_wnd->drag_entry;
        if (entry->slot_code > 4 && entry->slot_code < 9) {
            char shopname[80];
            if (main_wnd->vis_shopkaarg == this->shop) {
                strcpy(shopname, "shop_kaarg");
            } else if (main_wnd->vis_shopdruid == this->shop) {
                strcpy(shopname, "shop_druid");
            } else {
                strcpy(shopname, "shop");
            }
            char path[1024];
            int32_t effect = entry->GetFirstModAttrId();
            if (effect != 0) {
                sprintf(path, "speech\\%s\\effects\\%.2d.wav", shopname, effect);
            } else if (entry->GetAttribute(0x2A) != 0) {
                sprintf(path, "speech\\%s\\books\\%.2d.wav", shopname, entry->GetAttribute(0x2A));
            } else {
                sprintf(path, "speech\\%s\\s%.2di%.2dp%d.wav", shopname, entry->GetType(), entry->GetId(), GetRandS16(3) + 1);
            }
            if (this->snd_speech.sample != nullptr) {
                if (this->snd_speech.sample->FindPlayingChannel() == nullptr) {
                    delete this->snd_speech.sample;
                    this->snd_speech.sample = nullptr;
                    this->snd_speech.sample = new SfxSample(path);
                    this->snd_speech.sample->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
                }
            } else {
                this->snd_speech.sample = new SfxSample(path);
                this->snd_speech.sample->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
            }
        }
    }
    StopSfx(&this->snd_put_on.sample);
    CSound::Play(this->snd_put_on);
    return this->DropDraggedItem(idx);
}


// 4b9c3c (client: 4be606)
TokenEntry* VisShopBuyGrid::PickupSlotItem(int32_t idx, int32_t num) {
    StopSfx(&this->snd_put_off.sample);
    CSound::Play(this->snd_put_off);
    return VisInvBase::PickupSlotItem(idx, num);
}


// 4b9436 (client: 4bde00)
int32_t VisShopBuyGrid::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    return VisInvExtBase::OnLButtonDblClk(wparam, pos);
}


// 4b93ae (client: 4bdd78)
int32_t VisShopBuyGrid::OnLButtonDown(uint32_t wparam, CPoint pos) {
    return VisInvExtBase::OnLButtonDown(wparam, pos);
}


// 4b93cf (client: 4bdd99)
int32_t VisShopBuyGrid::OnLButtonUp(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry == nullptr) {
        return 1;
    }
    if (this->IsBuyGridFull(main_wnd->drag_entry) != 0) {
        this->shop->placement_lock = 0;
    }
    return VisInvExtBase::OnLButtonUp(wparam, pos);
}


// 4b92f9 (client: 4bdcc3)
int32_t VisShopBuyGrid::OnMouseMove(uint32_t wparam, CPoint pos) {
    if (wparam & 1) {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        if (main_wnd->drag_entry != nullptr) {
            if (this->IsBuyGridFull(main_wnd->drag_entry) == 0) {
                if (main_wnd->drag_entry->GetAttribute(1) > 0) {
                    this->shop->placement_lock = 1;
                    ApplyCursor(main_wnd->item_cursor);
                }
            } else {
                this->shop->placement_lock = 0;
                ApplyCursor(g_Cursors[23]);
            }
        }
    }
    return VisInvExtBase::OnMouseMove(wparam, pos);
}


// 4b9155 (client: 4bdb1f)
void VisShopBuyGrid::Draw() {
    VisShop* shop = this->shop;
    CPoint topleft = shop->rect.TopLeft();

    if (shop->dialog_active == 0) {
        return;
    }
    if (this->shoptable == nullptr) {
        return;
    }

    LockBackSurface();
    this->shoptable->DrawSubRectMasked(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, this->rect.Width(), this->rect.Height());
    UnlockBackSurface();
    VisInvExtBase::Draw();
}


// 4b91f9 (client: 4bdbc3)
int32_t VisShopBuyGrid::IsBuyGridFull(TokenEntry* o) {
    TokenEntry copy(o);
    if (o->slot_code == 1) {
        copy.slot_code = 2;
    }
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (copy.IsStackableWith(entry)) {
            return 0;
        }
    }
    return this->grid_source->GetSize() >= this->visible_columns * this->visible_rows;
}


// 4b949a (client: 4bde64)
int32_t VisShopBuyGrid::AppendItem(TokenEntry* o) {
    if (this->grid_source == nullptr) {
        return -1;
    }
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (entry->match_id != o->match_id) {
            continue;
        }
        if (entry->slot_code != o->slot_code) {
            continue;
        }
        entry->AddCount(o->count);
        delete o;
        return i;
    }
    if (this->grid_source->GetSize() < this->visible_columns * this->visible_rows) {
        this->grid_source->Add(o);
    }
    return this->grid_source->GetSize() - 1;
}


// 4b80d9 (client: 4bcaa3)
VisShopBagGrid::~VisShopBagGrid() {
    this->visible_startref = nullptr;
}


// 4b7fe5 (client: 4bc9af)
VisShopBagGrid::VisShopBagGrid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisInvExtBase(_id, l, t, r, b, shop) {
    this->visible_columns = this->rect.Width() / 0x50 - 1;
    this->visible_rows = this->rect.Height() / 0x50;
    delete this->visible_startref;
    this->visible_startref = nullptr;
    this->InitCellRects();
    this->VisInvBase::InitCellArrays();
}


// 4b8102 (client: 4bcacc)
void VisShopBagGrid::InitCellRects() {
    CPoint topleft = this->rect.TopLeft();
    CPoint bottomright = this->rect.BottomRight();

    this->cell_rects = new CRect[this->visible_columns * this->visible_rows];
    this->up_arrow_rect = CRect(topleft.x, topleft.y, topleft.x + 0x20, bottomright.y);
    this->down_arrow_rect = CRect(topleft.x + 0x1B0, topleft.y, bottomright.x, bottomright.y);

    CPoint start(this->up_arrow_rect.right, this->up_arrow_rect.top);
    for (int32_t col = 0; col < this->visible_columns; col++) {
        for (int32_t row = 0; row < this->visible_rows; row++) {
            this->cell_rects[row * this->visible_columns + col] = CRect(
                start.x + col * 0x50,
                start.y + row * 0x50 + 5,
                start.x + col * 0x50 + 0x50,
                start.y + row * 0x50 + 0x55);
        }
    }
}


// 4b860e (client: 4bcfd8)
void VisShopBagGrid::DrawUpArrowHover() {
    CPoint topleft = this->shop->rect.TopLeft();
    if (g_bmp_invarrow1 != nullptr) {
        g_bmp_invarrow3->DrawSubRectMasked(topleft.x, topleft.y + 0x188, 0, 0, 0x20, 0x58);
    }
}


// 4b8666 (client: 4bd030)
void VisShopBagGrid::DrawUpArrowIdle() {
    CPoint topleft = this->shop->rect.TopLeft();
    if (g_bmp_invarrow3 != nullptr) {
        g_bmp_invarrow1->DrawSubRectMasked(topleft.x, topleft.y + 0x188, 0, 0, 0x20, 0x58);
    }
}


// 4b86be (client: 4bd088)
void VisShopBagGrid::DrawDownArrowHover() {
    CPoint topleft = this->shop->rect.TopLeft();
    if (g_bmp_invarrow2 != nullptr) {
        g_bmp_invarrow4->DrawSubRectMasked(topleft.x + 0x1B0, topleft.y + 0x188, 0, 0, 0x20, 0x58);
    }
}


// 4b871c (client: 4bd0e6)
void VisShopBagGrid::DrawDownArrowIdle() {
    CPoint topleft = this->shop->rect.TopLeft();
    if (g_bmp_invarrow4 != nullptr) {
        g_bmp_invarrow2->DrawSubRectMasked(topleft.x + 0x1B0, topleft.y + 0x188, 0, 0, 0x20, 0x58);
    }
}


// 4b9f00 (client: 4be8d0)
int32_t VisShopBagGrid::GetTransferTab() {
    return TRANSFER_DST_BAG;
}

// 41f9b0 (client: 4208e0)
int32_t VisSpellBook::GetActiveSlot() {
    // Vanilla: selected < 0 ? pressed : selected  (fields 0x64 / 0x60).
    if (this->selected < 0) {
        return this->pressed;
    }
    return this->selected;
}


// 41f9e0 (client: 420910)
int32_t VisSpellBook::HasSelectedSpell() {
    return this->selected >= 0;
}


// 4ca89b (client: 4cf229)
int32_t VisSpellBook::IsSpellAvailable(int32_t id) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    return (main_wnd->vis_map_context->selected_available_spell_mask & (1 << (id & 0x1F))) != 0;
}


// 4ca8e0 (client: 4cf26e)
int32_t VisSpellBook::IsSpellEffectActive(int32_t id) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    return (main_wnd->vis_map_context->active_spell_effect_mask & (1 << (id & 0x1F))) != 0;
}


// 4ca925 (client: 4cf2b3)
void VisSpellBook::SelectScrollEntry(int32_t idx) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisPlayingArea* mc = main_wnd->vis_map_context;
    this->spell = idx;
    TokenEntry* entry = mc->primary_selected_object->tokenEntries[this->spell];
    this->selected = (int32_t)entry->GetCastSpellId() - 1;
}


// 4caaa7 (client: 4cf435)
int32_t VisSpellBook::NeedsUnitTarget() {
    if (this->selected < 0) {
        return g_SpellbookUnitTargetable[this->pressed];
    }
    return g_SpellUnitTargetable[this->selected];
}


// 4caa69 (client: 4cf3f7)
void VisSpellBook::ClearSpellSelection() {
    (void)AfxGetMainWnd();
    if (this->selected >= 0) {
        this->selected = -1;
        this->spell = -1;
    }
}


// 4ca7c2 (client: 4cf150)
int32_t VisSpellBook::HitTestSpellCell(CPoint* pos) {
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    *pos += CPoint(-5 - rc.left, -5 - rc.top);
    int32_t left = vis_scr_rect.left;
    if ((main_wnd->dialogsMask & DIALOG_MASK_SHOP) != 0) {
        left = 0;
    }
    *pos += CPoint(-left, 0);
    if (pos->x < 0) {
        return -1;
    }
    if (pos->y < 0) {
        return -1;
    }
    if (pos->y >= 0x4B) {
        return -1;
    }
    if (pos->x >= 0x1C8) {
        return -1;
    }
    return (pos->y / 0x26) * 0xC + pos->x / 0x26;
}


// 4cadd2 (client: 4cf758)
int32_t VisSpellBook::OnRButtonDown(uint32_t wparam, CPoint pos) {
    (void)wparam;
    (void)pos;
    this->pressed = -1;
    return 1;
}


// 4caf70 (client: 4cf8f6)
int32_t VisSpellBook::OnRButtonUp(uint32_t wparam, CPoint pos) {
    (void)wparam;
    (void)pos;
    this->pressed = -1;
    return 1;
}


// 4cae94 (client: 4cf81a)
int32_t VisSpellBook::OnRButtonDblClk(uint32_t wparam, CPoint pos) {
    (void)wparam;
    (void)pos;
    return 1;
}


// 4cad6a (client: 4cf6f0)
int32_t VisSpellBook::OnLButtonDown(uint32_t wparam, CPoint pos) {
    (void)wparam;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisPlayingArea* mc = main_wnd->vis_map_context;
    int32_t idx = this->HitTestSpellCell(&pos);
    if (idx >= 0
        && ((mc->selected_available_spell_mask | mc->active_spell_effect_mask) & (1 << (idx & 0x1F))) != 0) {
        this->pressed = idx;
    }
    return 1;
}


// 4caf8c (client: 4cf912)
int32_t VisSpellBook::OnMouseMove(uint32_t wparam, CPoint pos) {
    if (g_mousept.GetSelectState() != 0) {
        g_mousept.ResetStates();
    }
    if ((wparam & 1) != 0) {
        this->OnLButtonDown(wparam, pos);
    }
    if ((wparam & 2) != 0) {
        this->OnRButtonDown(wparam, pos);
    }
    return 0;
}


// 4caff5 (client: 4cf97b)
int32_t VisSpellBook::OnWmUser(uint32_t wparam, CPoint pos) {
    return this->OnLButtonDown(wparam, pos);
}


// 4caea6 (client: 4cf82c)
int32_t VisSpellBook::OnLButtonUp(uint32_t wparam, CPoint pos) {
    (void)wparam;
    (void)pos;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        g_Cursors[0]->Use();
        if ((main_wnd->dialogsMask & DIALOG_MASK_SHOP) == 0) {
            if (main_wnd->drag_src_tab == 2) {
                main_wnd->vis_invtype1->PlaceDraggedItem(main_wnd->drag_src_slot);
            } else if (main_wnd->drag_src_tab == 1) {
                main_wnd->vis_invtype1->PlaceDraggedItem(main_wnd->vis_invtype1->GetVisibleStart());
            }
        } else {
            CVisualObject* obj = main_wnd->vis_root->FindChild(1000);
#ifdef A2CLIENT_PATCH
            if (obj != nullptr) {
                ((VisShop*)obj)->CancelItemDrag();
            }
#else
            ((VisShop*)obj)->CancelItemDrag();
#endif
        }
    }
    return 1;
}


// 4cadee (client: 4cf774)
int32_t VisSpellBook::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisPlayingArea* mc = main_wnd->vis_map_context;
    int32_t idx = this->HitTestSpellCell(&pos);
    if (idx < 0
        || ((mc->selected_available_spell_mask | mc->active_spell_effect_mask) & (1 << (idx & 0x1F))) == 0
        || g_SpellbookDblClickCastable[idx] == 0) {
        this->OnLButtonDown(wparam, pos);
    } else {
        mc->SendGroupSelfCast(idx + 1);
        this->OnLButtonDown(wparam, pos);
    }
    return 1;
}


// 4cac9b (client: 4cf621)
int32_t VisSpellBook::OnKeyDown(uint32_t wparam) {
    if (g_kbControlState != 0 && (wparam == 'a' || wparam == 'A')) {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        VisPlayingArea* mc = main_wnd->vis_map_context;
        uint32_t spell_id = 0;
        if (this->pressed == -1) {
            CPoint pt;
#ifdef A2CLIENT_PATCH
            GetLogicalCursorPos(&pt);
#else
            GetCursorPos(&pt);
#endif
            int32_t idx = this->HitTestSpellCell(&pt);
            if (idx >= 0 && g_SpellbookCastable[idx] != 0) {
                spell_id = BOOK_POS_TO_SPELL_ID[idx + 1];
            }
        } else if (g_SpellbookCastable[this->pressed] != 0) {
            spell_id = BOOK_POS_TO_SPELL_ID[this->pressed + 1];
        }
        if (spell_id != 0) {
            mc->SendAutoSpellSelect((int32_t)spell_id);
            mc->UpdateSelectionState();
        }
    }
    return 0;
}


// 4caad7 (client: 4cf465)
int32_t VisSpellBook::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (msg == VIS_MSG_SHORTCUT_RELEASE) {
        this->pressed = -1;
    } else if (msg == VIS_MSG_SHORTCUT_KEY) {
        if (lparam == 0) {
#if defined(A2CLIENT) && defined(A2TANGAR_HAT)
            // Tangar spell_hotkey_use (4CF5DF): select from the custom hotkey.
            UserShortcut* tangar_hotkey = tangar_hotkeys::LookupHotkey((int32_t)wparam + VK_F4);
            if (tangar_hotkey != nullptr && tangar_hotkey->kind == 1) {
                this->pressed = tangar_hotkey->item_id;
            }
#else
            if (main_wnd->m_GameSession.shortcuts[wparam].kind == 1) {
                this->pressed = main_wnd->m_GameSession.shortcuts[wparam].item_id;
            }
#endif
        } else {
#if defined(A2CLIENT) && defined(A2TANGAR_HAT)
            // Tangar spell_hotkey_set_mouse/set_selected (4CF4EE/4CF562): the
            // custom-hotkey map owns the binding (with its own dedup).
            if (this->pressed == -1) {
                CPoint pt;
                GetLogicalCursorPos(&pt);
                int32_t idx = this->HitTestSpellCell(&pt);
                if (idx >= 0) {
                    tangar_hotkeys::SpellHotkeySet((int32_t)wparam + VK_F4, (short)idx);
                }
            } else {
                tangar_hotkeys::SpellHotkeySet((int32_t)wparam + VK_F4, (short)this->pressed);
            }
#else
            if (this->pressed == -1) {
                CPoint pt;
#ifdef A2CLIENT_PATCH
                GetLogicalCursorPos(&pt);
#else
                GetCursorPos(&pt);
#endif
                int32_t idx = this->HitTestSpellCell(&pt);
                if (idx >= 0) {
                    main_wnd->m_GameSession.shortcuts[wparam].SetSpell((short)idx);
                    for (int32_t i = 0; i < 9; i++) {
                        if (i != (int32_t)wparam
                            && main_wnd->m_GameSession.shortcuts[i].IsSpellShortcut((short)idx)) {
                            main_wnd->m_GameSession.shortcuts[i].SetNull();
                        }
                    }
                }
            } else {
                main_wnd->m_GameSession.shortcuts[wparam].SetSpell((short)this->pressed);
                for (int32_t i = 0; i < 9; i++) {
                    if (i != (int32_t)wparam
                        && main_wnd->m_GameSession.shortcuts[i].IsSpellShortcut((short)this->pressed)) {
                        main_wnd->m_GameSession.shortcuts[i].SetNull();
                    }
                }
            }
#endif
            main_wnd->m_GameSession.SaveShortcuts();
        }
    }
    return this->CVisualObject::MsgProc(msg, wparam, lparam);
}


// 4ca2a1 (client: 4cec31)
void VisSpellBook::Draw() {
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    LockBackSurface();
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisPlayingArea* mc = main_wnd->vis_map_context;
    int32_t left = vis_scr_rect.left;
    if ((main_wnd->dialogsMask & DIALOG_MASK_SHOP) != 0) {
        left = 0;
    }
    if (left != 0) {
#ifdef A2CLIENT_PATCH
        CRect prev_clip;
        GetClipRect(&prev_clip);
        SetClipRect(rc.left, rc.top, rc.right - 0x10, rc.bottom);
        CBmp64* spb_l = (g_ScreenSize.right > 800) ? g_bmp_spb1024l : g_bmp_spb800l;
        CBmp64* spb_r = (g_ScreenSize.right > 800) ? g_bmp_spb1024r : g_bmp_spb800r;
        if (spb_l != nullptr && spb_r != nullptr) {
            int32_t spb_l_x = rc.left + left - spb_l->GetWidth(0);
            int32_t spb_r_x = rc.left + left + g_bmp_spellbook->GetWidth(0) - 0x10;
            int32_t spb_r_end = spb_r_x + spb_r->GetWidth(0);
            FillRectColorSimple(rc.left, rc.top + 2, spb_l_x + 0x10, rc.bottom, 0);
            FillRectColorSimple(spb_r_end - 0x20, rc.top + 2, rc.right - 0x10, rc.bottom, 0);
            spb_l->DrawSubRectMasked(spb_l_x, rc.top, 0, 0,
                spb_l->GetWidth(0), spb_l->GetHeight(0));
            spb_r->DrawSubRectMasked(spb_r_x, rc.top, 0, 0,
                spb_r->GetWidth(0), spb_r->GetHeight(0));
        } else {
            FillRectColorSimple(rc.left, rc.top + 2, rc.left + left, rc.bottom, 0);
            FillRectColorSimple(rc.left + left + g_bmp_spellbook->GetWidth(0) - 0x10, rc.top + 2, rc.right - 0x10, rc.bottom, 0);
        }
        SetClipRect(prev_clip);
#else
        if (g_ScreenSize.bottom < 0x259) {
            if (g_ScreenSize.bottom > 0x1E0) {
                g_bmp_spb800l->DrawSubRectMasked(rc.left, rc.top, 0, 0,
                    g_bmp_spb800l->GetWidth(0), g_bmp_spb800l->GetHeight(0));
                g_bmp_spb800r->DrawSubRectMasked(rc.left + g_bmp_spellbook->GetWidth(0) - 0x10 + left, rc.top, 0, 0,
                    g_bmp_spb800r->GetWidth(0), g_bmp_spb800r->GetHeight(0));
            }
        } else {
            g_bmp_spb1024l->DrawSubRectMasked(rc.left, rc.top, 0, 0,
                g_bmp_spb1024l->GetWidth(0), g_bmp_spb1024l->GetHeight(0));
            g_bmp_spb1024r->DrawSubRectMasked(rc.left + g_bmp_spellbook->GetWidth(0) - 0x10 + left, rc.top, 0, 0,
                g_bmp_spb1024r->GetWidth(0), g_bmp_spb1024r->GetHeight(0));
        }
#endif
    }
    g_bmp_spellbook->DrawSubRectMasked(rc.left + left, rc.top, 0, 0,
        g_bmp_spellbook->GetWidth(0), g_bmp_spellbook->GetHeight(0));
    for (int32_t i = 0; i < 24; i++) {
        if (((mc->selected_available_spell_mask | mc->active_spell_effect_mask) & (1 << (i & 0x1F))) == 0) {
            g_bmp_spellback->Draw(left + rc.left + 6 + (i % 0xC) * 0x26,
                rc.top + 6 + (i / 0xC) * 0x26, 0, 0, 0);
        }
    }
    if (this->pressed >= 0
        && ((mc->selected_available_spell_mask | mc->active_spell_effect_mask) & (1 << (this->pressed & 0x1F))) != 0) {
        int32_t bx = (this->pressed % 0xC) * 0x26 + 6;
        int32_t by = (this->pressed / 0xC) * 0x26 + 6;
        FillRectBrightness(left + rc.left + bx, rc.top + by,
            left + rc.left + 0x24 + bx, rc.top + 0x24 + by, 4);
    }
    for (int32_t i = 0; i < 0x18; i++) {
        if (((mc->selected_available_spell_mask | mc->active_spell_effect_mask) & (1 << (i & 0x1F))) != 0) {
#if defined(A2CLIENT) && defined(A2TANGAR_HAT)
            // Tangar mark_spell_hotkey (4CEFBA): custom spell-hotkey labels
            // replace the retail F4-F12 labels.
            tangar_hotkeys::DrawSpellHotkeyLabel(left + rc.left, rc.top, i, this->pressed);
#else
            int32_t fkey = 0;
            for (int32_t s = 0; s < 9; s++) {
                if (main_wnd->m_GameSession.shortcuts[s].IsSpellShortcut((short)i)) {
                    fkey = s + 4;
                }
            }
            if (fkey != 0) {
                char buf[80];
                sprintf(buf, "F%d", fkey);
                int32_t off = 0;
                if (this->pressed == i) {
                    off = 2;
                }
                g_font3->DrawTextWithShadow(left + rc.left + 8 + off + (i % 0xC) * 0x26,
                    rc.top + 8 + off + (i / 0xC) * 0x26, buf, CGameFont::ALIGN_LEFT_TOP, clrsh_TechBlack, 1);
            }
#endif
        }
        if ((mc->selected_active_spell_mask & (1 << (i & 0x1F))) != 0) {
            char buf[80];
            sprintf(buf, "A");
            int32_t off = 0;
            if (this->pressed == i) {
                off = 2;
            }
            g_font3->DrawTextWithShadow(left + rc.left + 8 + off + (i % 0xC) * 0x26,
                rc.top + 0x22 + off + (i / 0xC) * 0x26, buf, CGameFont::ALIGN_LEFT_TOP, clrsh_TechBlack, 1);
        }
    }
    UnlockBackSurface();
}


// 4c9a6f (client: 4ce3ff)
const char* VisSpellBook::GetHint() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisPlayingArea* mc = main_wnd->vis_map_context;
    CPoint mpt(g_mousept.GetX(), g_mousept.GetY());
    int32_t idx = this->HitTestSpellCell(&mpt);
    if ((mc->selected_available_spell_mask & (1 << (idx & 0x1F))) == 0) {
        return nullptr;
    }
    if ((main_wnd->dialogsMask & DIALOG_MASK_SHOP) == 0 && main_wnd->dialogsMask != DIALOG_MASK_INGAME) {
        return nullptr;
    }
    g_HintBuf[0] = 0;
    if (idx < 0 || mc->spell_mana_cost1[idx] > 0xFFFE) {
        return nullptr;
    }
    CString str;
    CString dmg_str;
    CString range_str;
    CString power_str;
    CString vals_str;
    str.Format("%s#%s: %d", TxtFile::AllLines[0x75], txt_spells.GetLine(idx),
        mc->spell_mana_cost1[idx]);
    if (mc->spell_damage_max[idx] != 0) {
        if (mc->spell_damage_min[idx] == mc->spell_damage_max[idx]) {
            dmg_str.Format("#%s: %d", TxtFile::AllLines[0x76], mc->spell_damage_min[idx]);
        } else {
            dmg_str.Format("#%s: %d-%d", TxtFile::AllLines[0x76],
                mc->spell_damage_min[idx], mc->spell_damage_max[idx]);
        }
    }
    if (mc->spell_range_max[idx] != 0) {
        if (mc->spell_range_min[idx] == mc->spell_range_max[idx]) {
            range_str.Format("#%s: %d", TxtFile::AllLines[0x7B], mc->spell_range_min[idx]);
        } else {
            range_str.Format("#%s: %d-%d", TxtFile::AllLines[0x7B],
                mc->spell_range_min[idx], mc->spell_range_max[idx]);
        }
    }
    if (mc->spell_power_max[idx] != 0) {
        if (mc->spell_power_min[idx] == mc->spell_power_max[idx]) {
            power_str.Format("#%s: %5.1f", TxtFile::AllLines[0x7C],
                (double)mc->spell_power_min[idx] / 16.0);
        } else {
            power_str.Format("#%s: %5.1f -%5.1f", TxtFile::AllLines[0x7C],
                (double)mc->spell_power_min[idx] / 16.0,
                (double)mc->spell_power_max[idx] / 16.0);
        }
    }
    if (mc->spell_val1_max[idx] > -0xFFFF) {
        if (mc->spell_val1_min[idx] == mc->spell_val1_max[idx]) {
            vals_str.Format("#%s: %d", TxtFile::AllLines[0xB6], mc->spell_val1_min[idx]);
        } else {
            vals_str.Format("#%s: %d...%d", TxtFile::AllLines[0xB6],
                mc->spell_val1_min[idx], mc->spell_val1_max[idx]);
        }
    }
    if (mc->spell_val2_max[idx] != 0) {
        if (mc->spell_val2_min[idx] == mc->spell_val2_max[idx]) {
            vals_str.Format("#%s: +%d", TxtFile::AllLines[0xB7], mc->spell_val2_min[idx]);
        } else {
            vals_str.Format("#%s: +%d...+%d", TxtFile::AllLines[0xB7],
                mc->spell_val2_min[idx], mc->spell_val2_max[idx]);
        }
    }
    if (mc->spell_val3_max[idx] > -0xFFFF) {
        if (mc->spell_val3_min[idx] == mc->spell_val3_max[idx]) {
            vals_str.Format("#%s: %d", TxtFile::AllLines[0xB8], mc->spell_val3_min[idx]);
        } else {
            vals_str.Format("#%s: %d...%d", TxtFile::AllLines[0xB8],
                mc->spell_val3_min[idx], mc->spell_val3_max[idx]);
        }
    }
    if (mc->spell_val4_max[idx] != 0) {
        if (mc->spell_val4_min[idx] == mc->spell_val4_max[idx]) {
            vals_str.Format("#%s: +%d%%", TxtFile::AllLines[0xB9], mc->spell_val4_min[idx]);
        } else {
            vals_str.Format("#%s: +%d...+%d%%", TxtFile::AllLines[0xB9],
                mc->spell_val4_min[idx], mc->spell_val4_max[idx]);
        }
    }
    if (mc->spell_val5_max[idx] != 0) {
        if (mc->spell_val5_min[idx] == mc->spell_val5_max[idx]) {
            vals_str.Format("#%s: +%d%%", TxtFile::AllLines[0xBB], mc->spell_val5_min[idx]);
        } else {
            vals_str.Format("#%s: +%d...+%d%%", TxtFile::AllLines[0xBB],
                mc->spell_val5_min[idx], mc->spell_val5_max[idx]);
        }
    }
    if (mc->spell_val6_max[idx] != 0) {
        if (mc->spell_val6_min[idx] == mc->spell_val6_max[idx]) {
            vals_str.Format("#%s: %d", TxtFile::AllLines[0xBA], mc->spell_val6_min[idx]);
        } else {
            vals_str.Format("#%s: %d-%d", TxtFile::AllLines[0xBA],
                mc->spell_val6_min[idx], mc->spell_val6_max[idx]);
        }
    }
    if (mc->spell_val7_max[idx] != 0) {
        if (mc->spell_val7_min[idx] == mc->spell_val7_max[idx]) {
            vals_str.Format("#%s: %d", TxtFile::AllLines[0xD9], mc->spell_val7_min[idx]);
        } else {
            vals_str.Format("#%s: %d-%d", TxtFile::AllLines[0xD9],
                mc->spell_val7_min[idx], mc->spell_val7_max[idx]);
        }
    }
    sprintf(g_HintBuf, "%s%s%s%s%s", (LPCTSTR)str, (LPCTSTR)dmg_str,
        (LPCTSTR)range_str, (LPCTSTR)power_str, (LPCTSTR)vals_str);
    return g_HintBuf;
}


// 4c99c7 (client: 4ce357)
VisSpellBook::VisSpellBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->dirty = 0;
    this->pressed = -1;
    this->selected = -1;
    this->spell = -1;
}


// 4cb050 (client: 4cf9d0) - (scalar deleting dtor ??_G at 4CB020; complete dtor 4cb050 has no custom cleanup)
VisSpellBook::~VisSpellBook() {
}
