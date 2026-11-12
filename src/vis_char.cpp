#include <winsock2.h>
#ifdef A2CLIENT_PATCH
#include "patch/hotkeys.h"
#endif
#ifdef A2CLIENT_PATCH
#include "patch/stats_level.h"
#endif
#include "patch/locale.h"
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
#include "vis_char.h"
#include "gameplay.h"
#include "vis_playing_area.h"
#include "vis_menu.h"
#include "vis_shop.h"
#include "vis_inventory.h"

// Blink-cycle state (binary: 659530, 659518, 62CCF4, 62CCF8, 659550, 65953C)
static uint32_t blink_hover_time = 0;
static uint32_t blink_cycle_time = 0;
static int32_t blink_hover_slot = -1;
static int32_t blink_cycle_len = 4;
static int32_t blink_slot = 0;
static uint8_t blink_inited = 0;

int32_t VisCharSellectButtons::OnLButtonUp(uint32_t wparam, CPoint pos) {
    //4303c8
    // client: 4322bb
    MainWindow* mainwnd = (MainWindow*)AfxGetMainWnd();

    if (this->mouse_down_box > -1 && this->mouse_down_box < 4 &&
        this->GetMouseOnBox(pos) == this->mouse_down_box) {
        int32_t box = this->mouse_down_box;
        this->mouse_down_box = -1;
        this->UpdateMouseOverBox(wparam, pos);

        MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

        switch (box) {
        case 0:
            if (this->parent_screen->roster_list->renaming == 0)
#ifdef A2CLIENT
            {
                int32_t idx = this->parent_screen->roster_list->selected_row;
                if (idx == this->parent_screen->session->GetStringArray1Size() - 1 &&
                    mwnd->net_session.internet_game != 0 && mwnd->hat_settings.ishat != 0 &&
                    this->parent_screen->session->hat_char_ids.GetSize() > 15) {
                    VisScreen* mbox = new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(141), nullptr, 0x2000);
                    mwnd->vis_msgbox_list = mbox;
                    mwnd->ModalScreen(mbox);
                } else if (idx == this->parent_screen->session->GetStringArray1Size() - 1 &&
                    mwnd->net_session.internet_game != 0 && mwnd->hat_settings.ishat != 0 && mwnd->hat_settings.deathmatch != 0) {
                    VisScreen* mbox = new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(142), nullptr, 0x2000);
                    mwnd->vis_msgbox_list = mbox;
                    mwnd->ModalScreen(mbox);
                } else {
                    this->parent_screen->CloseOk();
                }
            }
#else
                parent_screen->CloseOk();
#endif
            break;

        case 1:
#ifdef A2CLIENT
            if (mwnd->net_session.internet_game != 0 && mwnd->hat_settings.ishat != 0 && mwnd->hat_settings.deathmatch != 0) {
                break;
            }
#endif
            if (this->parent_screen->roster_list->renaming == 0) {
                int32_t idx = this->parent_screen->roster_list->selected_row;
                if (idx != this->parent_screen->session->GetStringArray1Size() - 1) {
                    CString str = CString(txt_patch.GetLine(93)) + CString(this->parent_screen->session->character_name) + CString(txt_patch.GetLine(94));
                    VisScreen* box = new VisMessageBoxWithList(1, 64, 100, 380, 594, str, nullptr, 4);
                    mwnd->vis_msgbox_list = box;
                    mwnd->ModalScreen(box);

                    if (box->GetCloseCode() == VIS_MSG_BOX_OK) {
                        this->parent_screen->session->DeleteSelectedCharacter();

                        if (idx < this->parent_screen->session->GetStringArray1Size() - 1) {
                            this->parent_screen->session->LoadCharacterRosterEntry(idx);
                            this->parent_screen->vis_stats->ReadUnitStats();
                            this->parent_screen->SelectCharacter(this->parent_screen->selected_unit);
                        } else {
                            this->parent_screen->roster_list->selected_row = this->parent_screen->session->GetStringArray1Size() - 1;
                            this->parent_screen->selected_unit->SetSelected(0);
                            this->parent_screen->map_context->UpdateSelectionState();
                        }
                    }
                }
            }
            break;

        case 2:
            if (this->parent_screen->roster_list->renaming == 0) {
                this->parent_screen->OpenRenameWindow();
                if (this->parent_screen->snd_rename) {
                    this->parent_screen->snd_rename->Play();
                }
            }
            break;

        case 3:
            if (this->parent_screen->roster_list->renaming == 0) {
                this->parent_screen->CloseCancel();
            }
            break;
            
        default:
            break;
        }
    }
    return 1;
}

int VisCharSellectButtons::GetMouseOnBox(CPoint pos) {
    pos -= this->parent_screen->rect.TopLeft();

    for (int i = 0; i < 4; i++) {
        if (this->areas[i].PtInRect(pos)) {
            return i;
        }
    }
    return -1;
}

void VisCharSellectButtons::UpdateMouseOverBox(uint32_t wparam, CPoint pos) {
    int over = this->GetMouseOnBox(pos);
    if (over >= 0 && (wparam & 1) == 0) {
        this->mouse_over_box = over;
    } else if (over >= 0 && this->mouse_down_box == over && (wparam & 1) != 0) {
        this->mouse_over_box = over;
    } else {
        this->mouse_over_box = -1;
    }
}


// 43005F
void VisCharSellectButtons::VMethod7() {
    CPoint top_left = this->parent_screen->rect.TopLeft();
    if (this->parent_screen->active_flag != 0) {
        LockSurface2();
        this->bmp_area->Draw(top_left.x + this->rect.left, top_left.y + this->rect.top, 0, 0, 0);
        for (int32_t i = 0; i < 4; i++) {
            uint16_t* pal;
            if (this->mouse_over_box == i) {
                pal = palette_paris_daisy->GetPalette(0);
            } else {
                pal = palette_husk->GetPalette(0);
            }
            if (this->mouse_over_box < 0 || this->mouse_down_box != this->mouse_over_box || this->mouse_over_box != i) {
                g_font4->DrawTxt(top_left.x + this->areas[i].left + this->areas[i].Width() / 2,
                                 top_left.y + this->areas[i].top + this->areas[i].Height() / 2,
                                 this->captions[i], 10, pal);
            } else {
                this->buttons_bmp[i]->DrawSubRectMasked(top_left.x + this->areas[i].left, top_left.y + this->areas[i].top,
                                                0, 0, this->areas[i].Width(), this->areas[i].Height());
                g_font4->DrawTxt(top_left.x + this->areas[i].left + this->areas[i].Width() / 2,
                                 top_left.y + this->areas[i].top + 1 + this->areas[i].Height() / 2,
                                 this->captions[i], 10, pal);
            }
        }
        UnlockSurface2();
    }
}


// 430a1a
void VisCharSellectButtons::FreeBitmaps() {
    for (int32_t i = 0; i < 4; i++) {
        if (this->buttons_bmp[i] != nullptr) {
            delete this->buttons_bmp[i];
        }
        this->buttons_bmp[i] = nullptr;
        // Never assigned anywhere; kept faithful — dispatch is virtual so the
        // static type only satisfies the compiler.
        if (this->gap_0x84[i] != nullptr) {
            delete static_cast<CBmp64*>(this->gap_0x84[i]);
        }
        this->gap_0x84[i] = nullptr;
    }
    if (this->bmp_area != nullptr) {
        delete this->bmp_area;
    }
    this->bmp_area = nullptr;
}


// 430850
void VisCharSellectButtons::LoadBitmaps() {
    this->FreeBitmaps();
    this->buttons_bmp[0] = new CBmp64("graphics\\interface\\shop_druid\\ShopButton1.bmp");
    g_mousept.Update();
    this->buttons_bmp[1] = new CBmp64("graphics\\interface\\shop_druid\\ShopButton2.bmp");
    g_mousept.Update();
    this->buttons_bmp[2] = new CBmp64("graphics\\interface\\shop_druid\\ShopButton3.bmp");
    g_mousept.Update();
    this->buttons_bmp[3] = new CBmp64("graphics\\interface\\shop_druid\\ShopButton4.bmp");
    g_mousept.Update();
    this->bmp_area = new CBmp64("graphics\\interface\\chrgen\\ButtonsArea.bmp");
    g_mousept.Update();
}


// 430318
int32_t VisCharSellectButtons::OnLButtonDown(uint32_t wparam, CPoint pos) {
    (void)wparam;
    this->mouse_down_box = this->GetMouseOnBox(pos);
    int32_t box = this->mouse_down_box;
    if (box == 0) {
        CSound::Play((CSound&)this->parent_screen->snd_ok);
    } else if (box == 1) {
        CSound::Play((CSound&)this->parent_screen->snd_delete);
    } else if (box == 3) {
        CSound::Play((CSound&)this->parent_screen->snd_cancel);
    }
    if (this->mouse_down_box == 0) {
        CSound::Play((CSound&)this->parent_screen->snd_ok);
    }
    return 1;
}


// 4302F5
int32_t VisCharSellectButtons::OnMouseMove(uint32_t wparam, CPoint pos) {
    this->UpdateMouseOverBox(wparam, pos);
    return 0;
}


// 43072B
void VisCharSellectButtons::ResetMouseBoxes() {
    this->mouse_down_box = -1;
    this->mouse_over_box = -1;
}


// 42FE62
void VisCharSellectButtons::Init() {
    this->mouse_down_box = -1;
    this->mouse_over_box = -1;
    this->areas[0] = CRect(0x1EE, 0xF, 0x266, 0x43);
    this->areas[1] = CRect(0x1E3, 0x43, 0x26F, 0x71);
    this->areas[2] = CRect(0x1E3, 0x72, 0x26F, 0xA0);
    this->areas[3] = CRect(0x1EE, 0xA0, 0x266, 0xD4);
    for (int32_t i = 0; i < 4; i++) {
        this->buttons_bmp[i] = nullptr;
        this->gap_0x84[i] = nullptr;
    }
    this->bmp_area = nullptr;
    this->captions.SetSize(4, -1);
    this->captions[0] = TxtFile::AllLines[0xEE];
    this->captions[1] = TxtFile::AllLines[0xF0];
    this->captions[2] = TxtFile::AllLines[0xF1];
    this->captions[3] = TxtFile::AllLines[0x4E];
    this->flags |= FLAG_NOTFOCUS;
}


// 42FD5C
VisCharSellectButtons::VisCharSellectButtons(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->parent_screen = parent_screen;
    this->Init();
}


// 42FDF8 (scalar deleting dtor ??_G at 4385B0)
VisCharSellectButtons::~VisCharSellectButtons() {
    this->FreeBitmaps();
    this->parent_screen = nullptr;
}


// 430FEC
void VisCharSellectList::VMethod7() {
    if (this->parent_screen->active_flag != 0) {
        CPoint top_left = this->parent_screen->rect.TopLeft();
        int32_t visible_end = this->top_row + this->list_area.Height() / this->row_rect.Height();
        if (visible_end > this->parent_screen->session->GetStringArray1Size()) {
            visible_end = this->parent_screen->session->GetStringArray1Size();
        }
        LockSurface2();
        this->bmp_center_area->Draw(top_left.x + this->rect.left, top_left.y + this->rect.top, 0, 0, 0);
        if (this->bmp_arrow_up != nullptr) {
            this->bmp_arrow_up->Draw(top_left.x + 0x78 + this->rect.left, top_left.y + this->rect.top, 0, 0, 0);
        }
        if (this->bmp_arrow_down != nullptr) {
            this->bmp_arrow_down->Draw(top_left.x + 0x78 + this->rect.left, top_left.y + 0x188 + this->rect.top, 0, 0, 0);
        }
        for (int32_t i = this->top_row; i < visible_end; i++) {
            uint16_t* pal;
            if (i == this->hovered_row) {
                pal = palette_paris_daisy->GetPalette(0);
            } else {
                pal = palette_husk->GetPalette(0);
            }
            if (i == this->selected_row) {
                CRect rc(top_left.x + this->list_area.left,
                         top_left.y + this->list_area.top + this->row_rect.Height() * ((i - this->top_row) + 1) - 4,
                         top_left.x + this->list_area.right,
                         top_left.y + this->list_area.top + this->row_rect.Height() * ((i - this->top_row) + 2));
                ShadowRect(rc, 8);
            }
            if (this->selected_row != i || this->renaming == 0) {
                g_font4->DrawTextWithShadow(top_left.x + this->list_area.left + this->list_area.Width() / 2,
                                            top_left.y + this->list_area.top + this->row_rect.Height() * ((i - this->top_row) + 1),
                                            this->parent_screen->session->characterRosterNames[i], 2, pal, 1);
            }
        }
        UnlockSurface2();
        this->CVisualObject::VMethod7();
    }
}


// 43148A
void VisCharSellectList::FreeBitmaps() {
    if (this->bmp_center_area != nullptr) {
        delete this->bmp_center_area;
    }
    this->bmp_center_area = nullptr;
    if (this->bmp_up_shine != nullptr) {
        delete this->bmp_up_shine;
    }
    this->bmp_up_shine = nullptr;
    if (this->bmp_up_shine_on != nullptr) {
        delete this->bmp_up_shine_on;
    }
    this->bmp_up_shine_on = nullptr;
    if (this->bmp_down_shine != nullptr) {
        delete this->bmp_down_shine;
    }
    this->bmp_down_shine = nullptr;
    if (this->bmp_down_shine_on != nullptr) {
        delete this->bmp_down_shine_on;
    }
    this->bmp_down_shine_on = nullptr;
}


// 4312B7
void VisCharSellectList::LoadBitmaps() {
    this->FreeBitmaps();
    this->bmp_center_area = new CBmp64("graphics\\interface\\chrgen\\loader\\CenterArea.bmp");
    g_mousept.Update();
    this->bmp_up_shine = new CBmp64("graphics\\interface\\chrgen\\loader\\up\\shine.bmp");
    g_mousept.Update();
    this->bmp_up_shine_on = new CBmp64("graphics\\interface\\chrgen\\loader\\up\\shine_on.bmp");
    g_mousept.Update();
    this->bmp_down_shine = new CBmp64("graphics\\interface\\chrgen\\loader\\down\\shine.bmp");
    g_mousept.Update();
    this->bmp_down_shine_on = new CBmp64("graphics\\interface\\chrgen\\loader\\down\\shine_on.bmp");
    g_mousept.Update();
}


// 4316C4
void VisCharSellectList::SelectRow(int32_t row) {
    if (row != -1) {
        if (row == this->parent_screen->session->GetStringArray1Size() - 1) {
            this->parent_screen->selected_unit->SetSelected(0);
            this->parent_screen->map_context->UpdateSelectionState();
        } else {
            this->parent_screen->session->LoadCharacterRosterEntry(row);
            this->parent_screen->vis_stats->ReadUnitStats();
            this->parent_screen->SelectCharacter(this->parent_screen->selected_unit);
        }
        this->selected_row = row;
    }
}


// 431872
int32_t VisCharSellectList::HitTest(CPoint pos) {
    int32_t visible_end = this->top_row + this->list_area.Height() / this->row_rect.Height();
    CPoint top_left = this->parent_screen->rect.TopLeft();
    if (visible_end > this->parent_screen->session->GetStringArray1Size()) {
        visible_end = this->parent_screen->session->GetStringArray1Size();
    }
    for (int32_t i = this->top_row; i < visible_end; i++) {
        CPoint origin(top_left.x + this->list_area.left,
                      top_left.y + this->list_area.top + this->row_rect.Height() * (i - this->top_row + 1));
        CRect row_rect(origin, this->row_rect.Size());
        if (row_rect.PtInRect(pos)) {
            return i;
        }
    }
    return -1;
}


// 431A05
int32_t VisCharSellectList::SelectPrevRow() {
    if (this->selected_row - 1 < 0) {
        return 0;
    }
    if (this->selected_row - 1 < this->top_row) {
        this->top_row = this->top_row - 1;
        this->selected_row = this->selected_row - 1;
    } else {
        this->selected_row = this->selected_row - 1;
    }
    this->SelectRow(this->selected_row);
    return 1;
}


// 431A93
int32_t VisCharSellectList::SelectNextRow() {
    int32_t visible_end = this->top_row + this->list_area.Height() / this->row_rect.Height();
    if (visible_end > this->parent_screen->session->GetStringArray1Size()) {
        visible_end = this->parent_screen->session->GetStringArray1Size();
    }
    if (this->selected_row + 1 >= this->parent_screen->session->GetStringArray1Size()) {
        return 0;
    }
    if (this->selected_row + 1 > visible_end - 1) {
        this->top_row = this->top_row + 1;
        this->selected_row = this->selected_row + 1;
    } else {
        this->selected_row = this->selected_row + 1;
    }
    this->SelectRow(this->selected_row);
    return 1;
}


// 431C97
// client: 433dee
void VisCharSellectList::UpdateArrows(CPoint pos, bool is_down) {
#ifdef A2CLIENT
    pos -= this->parent_screen->rect.TopLeft();
#endif
    if (is_down) {
        if (this->selected_row - 1 < 0 || !this->up_arrow_rect.PtInRect(pos)) {
            this->bmp_arrow_up = nullptr;
        } else {
            this->bmp_arrow_up = this->bmp_up_shine_on;
        }
        if (this->selected_row + 1 < this->parent_screen->session->GetStringArray1Size()
            && this->down_arrow_rect.PtInRect(pos)) {
            this->bmp_arrow_down = this->bmp_down_shine_on;
        } else {
            this->bmp_arrow_down = nullptr;
        }
    } else {
        if (this->selected_row - 1 < 0 || !this->up_arrow_rect.PtInRect(pos)) {
            this->bmp_arrow_up = nullptr;
        } else {
            this->bmp_arrow_up = this->bmp_up_shine;
        }
        if (this->selected_row + 1 < this->parent_screen->session->GetStringArray1Size()
            && this->down_arrow_rect.PtInRect(pos)) {
            this->bmp_arrow_down = this->bmp_down_shine;
        } else {
            this->bmp_arrow_down = nullptr;
        }
    }
}


// 431609
int32_t VisCharSellectList::OnMouseMove(uint32_t wparam, CPoint pos) {
    if (this->renaming != 0) {
        return this->CVisualObject::OnMouseMove(wparam, pos);
    }
    this->hovered_row = this->HitTest(pos);
    if (this->hovered_row == -1) {
        g_Cursors[0]->Use();
    } else {
        g_Cursors[5]->Use();
    }
    this->UpdateArrows(pos, (wparam & 1) != 0);
    return this->CVisualObject::OnMouseMove(wparam, pos);
}


// 431764
int32_t VisCharSellectList::OnLButtonDown(uint32_t wparam, CPoint pos) {
    if (this->renaming != 0) {
        return this->CVisualObject::OnLButtonDown(wparam, pos);
    }
    int32_t row = this->HitTest(pos);
    this->SelectRow(row);
    if (this->down_arrow_rect.PtInRect(pos)) {
        this->SelectNextRow();
    } else if (this->up_arrow_rect.PtInRect(pos)) {
        this->SelectPrevRow();
    }
    this->UpdateArrows(pos, true);
    return 1;
}


// 43182C
int32_t VisCharSellectList::OnLButtonUp(uint32_t wparam, CPoint pos) {
    if (this->renaming != 0) {
        return this->CVisualObject::OnLButtonUp(wparam, pos);
    }
    this->UpdateArrows(pos, false);
    return 1;
}


// 4316A0
int32_t VisCharSellectList::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    return this->OnLButtonDown(wparam, pos);
}


// 431B95
int32_t VisCharSellectList::OnKeyDown(uint32_t wparam) {
    switch (wparam) {
    case 0x26:
        this->SelectPrevRow();
        return 1;
    case 0x28:
        this->SelectNextRow();
        return 1;
    case 0x0D:
        if (this->renaming != 0) {
            char buf[100];
            this->parent_screen->rename_txt->WriteData(buf);
            this->parent_screen->session->SetNameAndColor(buf);
            this->parent_screen->CloseRenameWindow();
        } else {
            this->parent_screen->CloseOk();
        }
        return 1;
    case 0x1B:
        if (this->renaming != 0) {
            this->parent_screen->CloseRenameWindow();
        }
        return 0;
    default:
        return 0;
    }
}


// 430FAD
void VisCharSellectList::ResetListState() {
    this->top_row = 0;
    this->selected_row = 0;
    this->hovered_row = -1;
    this->renaming = 0;
}


// 430CB3
// client: 432d7f
void VisCharSellectList::Init() {
    this->row_rect = CRect(CPoint(0x38, 0x38), CSize(0xCE, g_font4->GetHeight()));
    this->list_area = CRect(CPoint(this->rect.left + 0x38, this->rect.top + 0x38), CSize(0xCE, 0xEC));
    this->bmp_center_area = nullptr;
    this->bmp_up_shine = nullptr;
    this->bmp_up_shine_on = nullptr;
    this->bmp_down_shine = nullptr;
    this->bmp_down_shine_on = nullptr;
    this->bmp_arrow_down = nullptr;
    this->bmp_arrow_up = nullptr;
#ifdef A2CLIENT
    CPoint top_left = this->rect.TopLeft();
    CPoint vp = vis_scr_rect.TopLeft();
    this->up_arrow_rect = CRect(CPoint(0x78, 0), CSize(0x50, 0x3C)) + top_left + vp;
    this->down_arrow_rect = CRect(CPoint(0x78, 0x188), CSize(0x50, 0x34)) + top_left + vp;
#else
    CPoint top_left = this->rect.TopLeft();
    this->up_arrow_rect = CRect(CPoint(0x78, 0), CSize(0x50, 0x3C)) + top_left;
    this->down_arrow_rect = CRect(CPoint(0x78, 0x188), CSize(0x50, 0x34)) + top_left;
#endif
    this->ResetListState();
}


// 430BBB
VisCharSellectList::VisCharSellectList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->parent_screen = parent_screen;
    this->Init();
}


// 430C62 (scalar deleting dtor ??_G at 4385E0)
VisCharSellectList::~VisCharSellectList() {
    this->FreeBitmaps();
}

// 42F61A
void VisCharSellectStats::FreeBitmaps() {
    if (this->bmp_full_stats != nullptr) {
        delete this->bmp_full_stats;
    }
    this->bmp_full_stats = nullptr;
    if (this->bmp_left_up != nullptr) {
        delete this->bmp_left_up;
    }
    this->bmp_left_up = nullptr;
    if (this->cube_sprites != nullptr) {
        delete this->cube_sprites;
    }
    this->cube_sprites = nullptr;
}


// 42F4DF
void VisCharSellectStats::LoadBitmaps() {
    this->FreeBitmaps();
    this->bmp_full_stats = new CBmp64("graphics\\Interface\\chrgen\\FullStatsL.bmp");
    g_mousept.Update();
    this->bmp_left_up = new CBmp64("graphics\\interface\\chrgen\\loader\\LeftUp.bmp");
    g_mousept.Update();
    this->cube_sprites = new CA16("graphics\\interface\\chrgen\\cube\\sprites.16a");
    this->cube_sprites->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
}


// 42F6F3
void VisCharSellectStats::ReadUnitStats() {
    if (this->parent_screen->selected_unit == nullptr) {
        this->shadow_body = 0;
        this->shadow_reaction = 0;
        this->shadow_mind = 0;
        this->shadow_spirit = 0;
    } else {
        this->shadow_body = this->parent_screen->selected_unit->body;
        this->shadow_reaction = this->parent_screen->selected_unit->reaction;
        this->shadow_mind = this->parent_screen->selected_unit->mind;
        this->shadow_spirit = this->parent_screen->selected_unit->spirit;
    }
}


// 42F7B9
void VisCharSellectStats::VMethod7() {
    CPoint top_left = this->parent_screen->rect.TopLeft();
    if (this->parent_screen->active_flag != 0) {
        LockSurface2();
        this->bmp_left_up->Draw(top_left.x, top_left.y, 0, 0, 0);
        this->bmp_full_stats->Draw(top_left.x, top_left.y + 0xEE, 0, 0, 0);
        if (this->parent_screen->roster_list->selected_row == this->parent_screen->session->GetStringArray1Size() - 1) {
            UnlockSurface2();
            return;
        }
        CString str;
        uint16_t* pal = palette_husk->GetPalette(0);
        int32_t center_x = top_left.x + (this->rect.Width() + 0xC) / 2;

        str.Format("%d", this->parent_screen->session->money);
        g_font4->DrawTxt(center_x, top_left.y + 0x35 + g_font2->GetHeight() + g_font4->GetHeight() / 2,
                         str, 10, pal);
        g_font2->DrawTextWithShadow(center_x, top_left.y + 0x35, TxtFile::AllLines[0x59], 10, clrsh_DullGold, 1);

        str.Format("%d", this->parent_screen->session->monster_killed);
        g_font4->DrawTxt(center_x, top_left.y + 0x57 + g_font2->GetHeight() + g_font4->GetHeight() / 2,
                         str, 10, pal);
        g_font2->DrawTextWithShadow(center_x, top_left.y + 0x57, TxtFile::AllLines[0xF4], 10, clrsh_DullGold, 1);

        str.Format("%d", this->parent_screen->session->player_killed);
        g_font4->DrawTxt(center_x, top_left.y + 0x79 + g_font2->GetHeight() + g_font4->GetHeight() / 2,
                         str, 10, pal);
        g_font2->DrawTextWithShadow(center_x, top_left.y + 0x79, TxtFile::AllLines[0xF5], 10, clrsh_DullGold, 1);

        str.Format("%d", this->parent_screen->session->death_count);
        g_font4->DrawTxt(center_x, top_left.y + 0x9B + g_font2->GetHeight() + g_font4->GetHeight() / 2,
                         str, 10, pal);
        g_font2->DrawTextWithShadow(center_x, top_left.y + 0x9B, TxtFile::AllLines[0xF6], 10, clrsh_DullGold, 1);

        CRect rc = this->rect + top_left;
        OffsetRect(&rc, 0xC, 0);
        rc.top = top_left.y + 0xEE;
        rc.bottom = top_left.y + 0x1E0;
        this->parent_screen->selected_unit->DrawStatsPanel(&rc);
        UnlockSurface2();
    }
}


// 42FC61
const char* VisCharSellectStats::GetHint() {
    if (this->parent_screen->active_flag == 0) {
        return nullptr;
    }
    CRect rc = this->ClientRectToScreen(this->rect);
    CPoint pos(g_mousept.GetX() - rc.left - 0xC, g_mousept.GetY() - rc.top - 0xEE);
    return this->parent_screen->selected_unit->GetStatsHint(pos.x, pos.y);
}


// 42F1E8
void VisCharSellectStats::Init() {
    this->bmp_full_stats = nullptr;
    this->bmp_left_up = nullptr;
    this->cube_sprites = nullptr;
    this->stat_labels.SetSize(4, -1);
    this->stat_labels[0] = TxtFile::AllLines[0xF];
    this->stat_labels[1] = TxtFile::AllLines[0x10];
    this->stat_labels[2] = TxtFile::AllLines[0x11];
    this->stat_labels[3] = TxtFile::AllLines[0x12];
    this->stat_icon_rects[0] = CRect(CPoint(0x70, 0x31), CSize(0x20, 0x20));
    this->stat_icon_rects[1] = CRect(CPoint(0x70, 0x52), CSize(0x20, 0x20));
    this->stat_icon_rects[2] = CRect(CPoint(0x70, 0x73), CSize(0x20, 0x20));
    this->stat_icon_rects[3] = CRect(CPoint(0x70, 0x94), CSize(0x20, 0x20));
    this->shadow_body = 0x1E;
    this->shadow_reaction = 0x1F;
    this->shadow_mind = 0x20;
    this->shadow_spirit = 0x21;
}


// 42F0EC
VisCharSellectStats::VisCharSellectStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->parent_screen = parent_screen;
    this->Init();
}


// 42F188 (scalar deleting dtor ??_G at 438580)
VisCharSellectStats::~VisCharSellectStats() {
    this->FreeBitmaps();
}


// 431FDF
void VisCharSelect::VMethod26() {
    this->info_panel = nullptr;
    this->map_context = nullptr;
    this->snd_ok = nullptr;
    this->snd_rename = nullptr;
    this->snd_delete = nullptr;
    this->snd_cancel = nullptr;
    this->vis_stats = new VisCharSellectStats(0x462, 0, 0, 0xA0, 0x1E0, this);
    this->buttons = new VisCharSellectButtons(0x461, 0x1E0, 0, 0x280, 0xEE, this);
    this->roster_list = new VisCharSellectList(0x463, 0xA0, 0, 0x1E0, 0x1E0, this);

    CPoint top_left = this->roster_list->GetRect().TopLeft();
    CRect rc = this->roster_list->row_rect;
    OffsetRect(&rc, top_left.x, top_left.y);
    rc.left -= top_left.x;
    rc.right -= top_left.x;

    this->rename_txt = new VisTextBox(0x464, rc, g_font4, palette_husk->GetPalette(0), nullptr);
    this->AddChild(this->vis_stats);
    this->AddChild(this->buttons);
    this->AddChild(this->roster_list);
    this->selected_unit = nullptr;
    this->active_flag = 0;
}


// 438D30
CUnit* VisCharSelect::GetSelectedMapUnit() {
    return this->map_context->GetUnit_3f6c();
}


// 432280
void VisCharSelect::LoadSfx() {
    this->FreeSfx();
    ReloadSfx(&this->snd_ok, "SFX\\Click_Ok.wav");
    ReloadSfx(&this->snd_rename, "SFX\\Rename.wav");
    ReloadSfx(&this->snd_delete, "SFX\\Delete.wav");
    ReloadSfx(&this->snd_cancel, "SFX\\Undo.wav");
}


// 4322ED
void VisCharSelect::FreeSfx() {
    FreeSfxSample(&this->snd_ok);
    FreeSfxSample(&this->snd_rename);
    FreeSfxSample(&this->snd_delete);
    FreeSfxSample(&this->snd_cancel);
}


// 432933
void VisCharSelect::CloseRenameWindow() {
    if (this->roster_list->renaming != 0) {
        this->roster_list->renaming = 0;
        this->roster_list->RemoveChild(this->rename_txt);
        this->roster_list->focus_obj = nullptr;
        this->roster_list->cursor_over_obj = nullptr;
    }
}


// 43233E
void VisCharSelect::VMethod28() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->session = &main_wnd->m_GameSession;
    g_mousept.DisableHint();
    this->session->RefreshCharacterRosterFiles(1);
    this->info_panel = main_wnd->vis_charinfo;
    this->map_context = main_wnd->vis_map_context;
    this->map_context->quest_some_id_2 = 0;
    this->map_context->quest_landmark_some_id = 0;
    this->map_context->quest_building_some_id = 0;
    this->map_context->quest_some_id = 0;
    this->selected_unit = this->GetSelectedMapUnit();
    this->vis_stats->ReadUnitStats();
    if (this->session->GetStringArray1Size() > 1) {
        this->SelectCharacter(this->selected_unit);
    }
    main_wnd->vis_right_panel->RemoveChild(this->info_panel);
    CRect& rc = this->info_panel->GetRect();
    OffsetRect(&rc, 0x280 - rc.Width(), 0);
    this->info_panel->SetRect(&rc);
    this->AddChild(this->info_panel);
    this->LoadSfx();
    this->vis_stats->LoadBitmaps();
    this->buttons->LoadBitmaps();
    this->roster_list->LoadBitmaps();
    this->roster_list->ResetListState();
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();
    this->VisScreen::VMethod28();
    g_Cursors[0]->Use();
    this->active_flag = 1;
    g_mousept.EnableHint();
}


// 43251C
void VisCharSelect::DoClose(uint32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->VMethod9();
    if (this->roster_list->selected_row == this->session->GetStringArray1Size() - 1 && code == VIS_MSG_HIDE_SCREEN) {
        this->session->InitializeNewCharacterSession(0, nullptr);
    }
    this->active_flag = 0;
    CRect& rc = this->info_panel->GetRect();
    OffsetRect(&rc, rc.Width() - 0x280, 0);
    this->info_panel->SetRect(&rc);
    this->RemoveChild(this->info_panel);
    main_wnd->vis_right_panel->AddChild(this->info_panel);
    this->info_panel = nullptr;
    this->map_context = nullptr;
    this->selected_unit = nullptr;
    this->FreeSfx();
    this->vis_stats->FreeBitmaps();
    this->buttons->FreeBitmaps();
    this->roster_list->FreeBitmaps();
    this->CloseRenameWindow();
    this->VisScreen::DoClose(code);
}


// 4327F3
void VisCharSelect::OpenRenameWindow() {
    if (this->session->characterRosterNames.GetSize() == this->roster_list->selected_row) {
        return;
    }
    int32_t row_height = this->roster_list->row_rect.Height();
    int32_t sel = this->roster_list->selected_row;
    int32_t first_row = this->roster_list->top_row;
    int32_t list_top = this->roster_list->list_area.top;
    CRect& rc = this->rename_txt->GetRect();
    rc.top = list_top + row_height * (sel + 1 - first_row) - 4;
    rc.bottom = list_top + row_height * (sel + 2 - first_row);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->rename_txt->ReadData(main_wnd->vis_map_context->my_unit->str2);
    this->roster_list->AddChild(this->rename_txt);
    this->FocusTo(this->rename_txt, 1);
    this->roster_list->renaming = 1;
}


// 4326EA
int32_t VisCharSelect::OnMouseMove(uint32_t wparam, CPoint pos) {
    CPoint top_left = this->rect.TopLeft();
    CRect btn_area = this->buttons->GetRect() + top_left;
    if (!btn_area.PtInRect(pos)) {
        this->buttons->ResetMouseBoxes();
    }
    CRect stats_area = this->vis_stats->GetRect() + top_left;
    if (stats_area.PtInRect(pos)) {
        g_Cursors[0]->Use();
    }
    return this->CVisualObject::OnMouseMove(wparam, pos);
}


// 4326A4
int32_t VisCharSelect::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_FRAME_TICK) {
        this->VMethod9();
        return 1;
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 432655
void VisCharSelect::SelectCharacter(CUnit* unit) {
    if (unit != nullptr) {
        unit->SetSelected(1);
        unit->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
    }
    this->map_context->primary_selected_object = unit;
    this->map_context->UpdateSelectionState();
}


// 438D50
void VisCharSelect::VMethod8(CRect* rect) {
    (void)rect;
}


// 431EAC
VisCharSelect::VisCharSelect(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) {
    this->VMethod26();
}


// 431F18
VisCharSelect::~VisCharSelect() {
    if (this->info_panel != nullptr) {
        this->RemoveChild(this->info_panel);
        this->info_panel = nullptr;
    }
    if (this->rename_txt != nullptr) {
        this->CloseRenameWindow();
        if (this->rename_txt != nullptr) {
            delete this->rename_txt;
        }
        this->rename_txt = nullptr;
    }
    this->FreeSfx();
}

// 42F021
void VisCharGen::FreeSamples() {
    FreeSfxSample(&this->snd_stat);
    FreeSfxSample(&this->snd_ok);
    FreeSfxSample(&this->snd_reset);
    FreeSfxSample(&this->snd_back);
}


// 42EFB4
void VisCharGen::LoadSamples() {
    this->FreeSamples();
    ReloadSfx(&this->snd_stat, "SFX\\ChrGen\\+_-.wav");
    ReloadSfx(&this->snd_ok, "SFX\\Click_Ok.wav");
    ReloadSfx(&this->snd_reset, "SFX\\Sbros.wav");
    ReloadSfx(&this->snd_back, "SFX\\Back.wav");
}


// 42EBAB
void VisCharGen::OnClickBack() {
    this->MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
}


// 42EB8C
void VisCharGen::OnClickFwd() {
    this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
}


// 42EA87
void VisCharGen::ShowTipHint() {
    if (g_settings.TipsMode != 0 && this->tips_step == 0 && this->tips != nullptr) {
        CString str;
        MissionGetTips(7, &str);
        this->tips->SetText(str);
        this->tips_step = this->tips_step + 1;
    }
}


// 42E92A
void VisCharGen::RollStats() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->stats_panel->stat_points = 100;
    this->stats_panel->stat_body = 0x19;
    this->stats_panel->stat_reaction = 0x19;
    this->stats_panel->stat_mind = 0x19;
    this->stats_panel->stat_spirit = 0x19;
    main_wnd->m_GameSession.SetCharacterStats(
        this->stats_panel->stat_body,
        this->stats_panel->stat_reaction,
        this->stats_panel->stat_mind,
        this->stats_panel->stat_spirit,
        this->skills_panel->selected_slot + 1);
    this->stats_panel->HandleClick(0, CPoint(0, 0));
}


// 42EBE0
void VisCharGen::OnNextFace() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        return;
    }
    if (this->mage_flag == 0) {
        if (this->female_face == 0) {
            this->selected_face_set = (this->selected_face_set + 1) % this->face_sets[1].GetSize();
            this->selected_face = this->face_sets[1].GetAt(this->selected_face_set);
        } else {
            this->selected_face_set = (this->selected_face_set + 1) % this->face_sets[3].GetSize();
            this->selected_face = this->face_sets[3].GetAt(this->selected_face_set);
        }
    } else {
        if (this->female_face == 0) {
            this->selected_face_set = (this->selected_face_set + 1) % this->face_sets[0].GetSize();
            this->selected_face = this->face_sets[0].GetAt(this->selected_face_set);
        } else {
            this->selected_face_set = (this->selected_face_set + 1) % this->face_sets[2].GetSize();
            this->selected_face = this->face_sets[2].GetAt(this->selected_face_set);
        }
    }
    this->current_char->face = this->selected_face;
    this->current_char->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
    main_wnd->m_GameSession.face = this->current_char->face;
}


// 42EDC2
void VisCharGen::OnPrevFace() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        return;
    }
    if (this->mage_flag == 0) {
        if (this->female_face == 0) {
            this->selected_face_set = (this->face_sets[1].GetSize() + this->selected_face_set - 1) % this->face_sets[1].GetSize();
            this->selected_face = this->face_sets[1].GetAt(this->selected_face_set);
        } else {
            this->selected_face_set = (this->face_sets[3].GetSize() + this->selected_face_set - 1) % this->face_sets[3].GetSize();
            this->selected_face = this->face_sets[3].GetAt(this->selected_face_set);
        }
    } else {
        if (this->female_face == 0) {
            this->selected_face_set = (this->face_sets[0].GetSize() + this->selected_face_set - 1) % this->face_sets[0].GetSize();
            this->selected_face = this->face_sets[0].GetAt(this->selected_face_set);
        } else {
            this->selected_face_set = (this->face_sets[2].GetSize() + this->selected_face_set - 1) % this->face_sets[2].GetSize();
            this->selected_face = this->face_sets[2].GetAt(this->selected_face_set);
        }
    }
    this->current_char->face = this->selected_face;
    this->current_char->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
    main_wnd->m_GameSession.face = this->current_char->face;
}


// 42DCD9
VisCharGen::VisCharGen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) {
    this->VMethod26();
}


// 42DD66
VisCharGen::~VisCharGen() {
    this->RemoveChild(this->info_panel);
    this->info_panel = nullptr;
    this->FreeSamples();
}


// 42DDEC
void VisCharGen::VMethod26() {
    this->snd_stat = nullptr;
    this->snd_ok = nullptr;
    this->snd_reset = nullptr;
    this->snd_back = nullptr;
    this->tips = nullptr;
    this->stats_panel = new VisCharGenStats(0x457, 0, 0, 0xA0, 0xEE, this);
    this->fullstats_panel = new VisCharGenFullStats(0x458, 0, 0xEE, 0xA0, 0xF2, this);
    this->action_panel = new VisCharGenAction(0x459, 0x1E0, 0, 0x280, 0xEE, this);
    this->skills_panel = new VisCharGenSkills(0x45A, 0xA0, 0, 0x1E0, 0x1E0, this);
    this->AddChild(this->stats_panel);
    this->AddChild(this->fullstats_panel);
    this->AddChild(this->action_panel);
    this->AddChild(this->skills_panel);
    this->fwd_btn = 0;
    this->gap_0x9c = -1;
    this->mage_flag = 0;
    this->female_face = 0;
    this->selected_face = 0;
    this->active_flag = 0;
}


// 42EB29
void VisCharGen::VMethod7() {
    CSprite256* cursor_sprite = g_mousept.GetCursorSprite();
    CSprite256* default_sprite = g_Cursors[CURSOR_DEFAULT]->GetSprite();
    if (cursor_sprite != default_sprite) {
        cursor_sprite = g_mousept.GetCursorSprite();
        default_sprite = g_Cursors[CURSOR_DICE]->GetSprite();
        if (cursor_sprite != default_sprite) {
            g_Cursors[CURSOR_DEFAULT]->Use();
        }
    }
    this->VisScreen::VMethod7();
}


// 42EB7F
void VisCharGen::VMethod8(CRect* rect) {
    (void)rect;
}


// 42EA01
int32_t VisCharGen::OnLButtonUp(uint32_t wparam, CPoint pos) {
    if (this->fwd_btn == 0) {
        return this->CVisualObject::OnLButtonUp(wparam, pos);
    }
    this->stats_panel->OnLButtonUp(wparam, pos);
    return 0;
}


// 42EA4F
int32_t VisCharGen::OnKeyDown(uint32_t wparam) {
    if (wparam == 0xD) {
        this->OnClickFwd();
        return 1;
    }
    return this->VisScreen::OnKeyDown(wparam);
}


// 42E196
int32_t VisCharGen::OnMouseMove(uint32_t wparam, CPoint pos) {
    CRect area = this->action_panel->GetRect() + this->rect.TopLeft();
    if (!area.PtInRect(pos)) {
        this->action_panel->ResetMouseBoxes();
    }
    return this->CVisualObject::OnMouseMove(wparam, pos);
}


// 42E058
int32_t VisCharGen::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    switch (msg) {
    case VIS_MSG_FRAME_TICK:
        this->VMethod9();
        break;
    case VIS_MSG_SELECT_PREV:
        this->OnPrevFace();
        break;
    case VIS_MSG_SELECT_NEXT:
        this->OnNextFace();
        break;
    case VIS_MSG_CLOSE_TIPS:
        if (this->tips != nullptr) {
            this->skills_panel->RemoveChild(this->tips);
            delete this->tips;
            this->tips = nullptr;
        }
        break;
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 42E7BC
void VisCharGen::DoClose(uint32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->VMethod9();
    this->active_flag = 0;
    CRect& rc = this->info_panel->GetRect();
    OffsetRect(&rc, rc.Width() - 0x280, 0);
    this->info_panel->SetRect(&rc);
    this->RemoveChild(this->info_panel);
    main_wnd->vis_right_panel->AddChild(this->info_panel);
    this->info_panel = nullptr;
    this->map_visual = nullptr;
    if (this->tips != nullptr) {
        this->skills_panel->RemoveChild(this->tips);
        delete this->tips;
        this->tips = nullptr;
    }
    this->stats_panel->FreeBitmaps();
    this->fullstats_panel->FreeBitmaps();
    this->action_panel->FreeBitmaps();
    this->skills_panel->FreeBitmaps();
    this->skills_panel->FreeSamples();
    this->FreeSamples();
    this->current_char = nullptr;
    this->VisScreen::DoClose(code);
}


// 42E218
void VisCharGen::VMethod28() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    g_mousept.DisableHint();
    this->info_panel = main_wnd->vis_charinfo;
    this->map_visual = main_wnd->vis_map_context;
    this->current_char = this->map_visual->GetUnit_3f6c();
    this->stats_panel->ReadUnitStats();
    this->selected_face = this->current_char->face - 1;
    this->mage_flag = (this->current_char->unitFlags & UNIT_FLAG_MAGE) << 5;
    this->female_face = (this->current_char->unitFlags & UNIT_FLAG_FEMALE) << 6;
    main_wnd->vis_right_panel->RemoveChild(this->info_panel);
    CRect& rc = this->info_panel->GetRect();
    OffsetRect(&rc, 0x280 - rc.Width(), 0);
    this->info_panel->SetRect(&rc);
    this->AddChild(this->info_panel);
    if (main_wnd->sessionMode == SESSION_SINGLEPLAYER) {
        RegFile reg("scenario\\npc.reg");
        this->face_sets[0].RemoveAll();
        this->face_sets[1].RemoveAll();
        this->face_sets[2].RemoveAll();
        this->face_sets[3].RemoveAll();
        reg.GetInt16Array("Multiplayer", "FacesMM", &this->face_sets[0]);
        reg.GetInt16Array("Multiplayer", "FacesMF", &this->face_sets[1]);
        reg.GetInt16Array("Multiplayer", "FacesFM", &this->face_sets[2]);
        reg.GetInt16Array("Multiplayer", "FacesFF", &this->face_sets[3]);
    }
    if (g_settings.TipsMode != 0) {
        CString str;
        if (this->current_char->unitFlags & UNIT_FLAG_MAGE) {
            MissionGetTips(6, &str);
        } else {
            MissionGetTips(5, &str);
        }
        this->tips = new VisTipsDialog(0x467, 0, 0x118, 0x138, 0x1E0, str);
        this->skills_panel->AddChild(this->tips);
    } else {
        if (this->tips != nullptr) {
            this->skills_panel->RemoveChild(this->tips);
            delete this->tips;
            this->tips = nullptr;
        } else {
            this->tips = nullptr;
        }
    }
    this->tips_step = 0;
    this->fwd_btn = 0;
    this->gap_0x9c = -1;
    this->stats_panel->LoadBitmaps();
    this->fullstats_panel->LoadBitmaps();
    this->action_panel->LoadBitmaps();
    this->skills_panel->LoadBitmaps(this->current_char->unitFlags & UNIT_FLAG_MAGE);
    this->skills_panel->LoadSamples(this->current_char->unitFlags & UNIT_FLAG_MAGE);
    this->LoadSamples();
    this->stats_panel->stat_points = 0;
    this->stats_panel->HandleClick(0, CPoint(0, 0));
    for (int32_t i = 0; i < 5; i++) {
        this->skills_panel->slot_states[i] = 0;
    }
    this->skills_panel->selected_slot = main_wnd->m_GameSession.main_sphere - 1;
    this->skills_panel->slot_states[this->skills_panel->selected_slot] = 1;
    this->selected_face_set = 0;
    this->active_flag = 1;
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();
    this->VisScreen::VMethod28();
    g_Cursors[CURSOR_DEFAULT]->Use();
    g_mousept.EnableHint();
}


void VisCharGenStats::LoadBitmaps() { //429475
#ifdef A2CLIENT_PATCH
    // Tangar: the char-gen stats panel is up, so a new character is being
    // created (it does not re-read the a2c file) - gui_bugfix.cpp
    // new_character_screen hook, 42B599.
    tangar_stats::creating_new_character = true;
#endif
    this->FreeBitmaps();
    this->bmp = new CBmp64("main\\graphics\\chrgen\\leftup.bmp");
    g_mousept.Update();
    this->stat_button_bmps[0] = new CBmp64("graphics\\interface\\chrgen\\buttons\\plon.bmp");
    g_mousept.Update();
    this->stat_button_bmps[1] = new CBmp64("graphics\\interface\\chrgen\\buttons\\ploff.bmp");
    g_mousept.Update();
    this->stat_button_bmps[2] = new CBmp64("graphics\\interface\\chrgen\\buttons\\pnlon.bmp");
    g_mousept.Update();
    this->stat_button_bmps[3] = new CBmp64("graphics\\interface\\chrgen\\buttons\\pnloff.bmp");
    g_mousept.Update();
    this->stat_button_bmps[4] = new CBmp64("graphics\\interface\\chrgen\\buttons\\pdisable.bmp");
    g_mousept.Update();
    this->stat_button_bmps[5] = new CBmp64("graphics\\interface\\chrgen\\buttons\\mlon.bmp");
    g_mousept.Update();
    this->stat_button_bmps[6] = new CBmp64("graphics\\interface\\chrgen\\buttons\\mloff.bmp");
    g_mousept.Update();
    this->stat_button_bmps[7] = new CBmp64("graphics\\interface\\chrgen\\buttons\\mnlon.bmp");
    g_mousept.Update();
    this->stat_button_bmps[8] = new CBmp64("graphics\\interface\\chrgen\\buttons\\mnloff.bmp");
    g_mousept.Update();
    this->stat_button_bmps[9] = new CBmp64("graphics\\interface\\chrgen\\buttons\\mdisable.bmp");
    g_mousept.Update();
    for (int32_t i = 0; i < 4; i++) {
        this->stat_btn_pairs[i][0] = this->stat_button_bmps[4];
        this->stat_btn_pairs[i][1] = this->stat_button_bmps[8];
    }
}


void VisCharGenStats::FreeBitmaps() { //4298b6
    if (this->bmp != nullptr) {
        delete this->bmp;
    }
    this->bmp = nullptr;
    for (int32_t i = 0; i < 10; i++) {
        if (this->stat_button_bmps[i] != nullptr) {
            delete this->stat_button_bmps[i];
        }
        this->stat_button_bmps[i] = nullptr;
    }
    for (int32_t i = 0; i < 4; i++) {
        this->stat_btn_pairs[i][0] = nullptr;
        this->stat_btn_pairs[i][1] = nullptr;
    }
}


// 42a96d
int32_t VisCharGenStats::StatUpCost(int32_t stat) {
    return StatLevelPoints(stat + 1) - StatLevelPoints(stat);
}

// 42a99d
int32_t VisCharGenStats::StatDownRefund(int32_t stat) {
    return StatLevelPoints(stat) - StatLevelPoints(stat - 1);
}

// 42a43b
const char* VisCharGenStats::GetHint() {
    if (this->parent_screen->active_flag == 0) {
        return nullptr;
    }
    static CString str;
    CPoint pt(g_mousept.GetX(), g_mousept.GetY());
    CPoint topleft;
    this->ClientPtToScreen(&topleft, this->rect.TopLeft());
    for (int32_t i = 0; i < 4; i++) {
        if ((this->label_rects[i] + topleft).PtInRect(pt)) {
            return TxtFile::AllLines[i + 0x9B];
        }
        if ((this->points_rect + topleft).PtInRect(pt)) {
            return TxtFile::AllLines[0x111];
        }
        if ((this->areas[i][0] + topleft).PtInRect(pt)) {
            str.Format("%s = %d", (LPCTSTR)this->texts.ElementAt(i), (&this->stat_body)[i]);
            return str;
        }
        if ((this->areas[i][1] + topleft).PtInRect(pt)) {
            str.Format("%+d", -this->StatUpCost((&this->stat_body)[i]));
            InsertThousandsSeparators(&str);
            return str;
        }
        if ((this->areas[i][2] + topleft).PtInRect(pt)) {
            str.Format("%+d", this->StatDownRefund((&this->stat_body)[i]));
            InsertThousandsSeparators(&str);
            return str;
        }
    }
    return nullptr;
}


// 429d03
void VisCharGenStats::VMethod7() {
    static CString str;
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    if (this->parent_screen->active_flag == 0) {
        return;
    }
    LockSurface2();
    this->bmp->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
    for (int32_t i = 0; i < 4; i++) {
        str.Format("%d", (&this->stat_body)[i]);
        uint16_t* pal = palette_husk->GetPalette(0);
        g_font4->DrawTextWithShadow(screen_rect.left + this->areas[i][0].left + 2,
                                    screen_rect.top + this->areas[i][0].top + 4,
                                    str, 0, pal, 1);
        CBmp64* plus_bmp = this->stat_btn_pairs[i][0];
        plus_bmp->DrawSubRectMasked(screen_rect.left + this->areas[i][1].left,
                            screen_rect.top + this->areas[i][1].top,
                            0, 0, plus_bmp->GetWidth(0), plus_bmp->GetHeight(0));
        CBmp64* minus_bmp = this->stat_btn_pairs[i][1];
        minus_bmp->DrawSubRectMasked(screen_rect.left + this->areas[i][2].left,
                             screen_rect.top + this->areas[i][2].top,
                             0, 0, minus_bmp->GetWidth(0), minus_bmp->GetHeight(0));
        str.Format("%d", this->stat_points);
        InsertThousandsSeparators(&str);
        pal = palette_husk->GetPalette(0);
        g_font4->DrawTextWithShadow(
            screen_rect.left + this->points_rect.left + this->points_rect.Width() / 2,
            screen_rect.top + this->points_rect.top + this->points_rect.Height() / 2 + 2,
            str, 10, pal, 1);
    }
    UnlockSurface2();
}


// 42a02a
int32_t VisCharGenStats::HitTest(CPoint pt) {
    CPoint topleft;
    this->ClientPtToScreen(&topleft, this->rect.TopLeft());
    for (int32_t i = 0; i < 4; i++) {
        if ((this->areas[i][0] + topleft).PtInRect(pt)) {
            return i << 8;
        }
        if ((this->areas[i][1] + topleft).PtInRect(pt)) {
            return (i << 8) | 1;
        }
        if ((this->areas[i][2] + topleft).PtInRect(pt)) {
            return (i << 8) | 2;
        }
    }
    return -1;
}

// 42a161
uint32_t VisCharGenStats::HandleClick(int32_t mode, CPoint pt) {
    int32_t hit = this->HitTest(pt);
    int32_t button;
    int32_t idx;
    if (hit == -1) {
        button = -1;
        idx = -1;
    } else {
        button = hit & 0xFF;
        idx = hit >> 8;
    }
    for (int32_t i = 0; i < 4; i++) {
        int32_t up_cost = this->StatUpCost((&this->stat_body)[i]);
        this->stat_btn_pairs[i][0] = this->stat_button_bmps[3];
        if (this->stat_points >= up_cost && (&this->stat_body)[i] < 0x2D) {
            if (i == idx && button == 1) {
                if (mode & 1) {
                    this->stat_btn_pairs[i][0] = this->stat_button_bmps[0];
                } else {
                    this->stat_btn_pairs[i][0] = this->stat_button_bmps[1];
                }
            }
        } else {
            this->stat_btn_pairs[i][0] = this->stat_button_bmps[4];
        }
        this->StatDownRefund((&this->stat_body)[i]);
        this->stat_btn_pairs[i][1] = this->stat_button_bmps[8];
        if ((&this->stat_body)[i] > 0xF) {
            if (i == idx && button == 2) {
                if (mode & 1) {
                    this->stat_btn_pairs[i][1] = this->stat_button_bmps[5];
                } else {
                    this->stat_btn_pairs[i][1] = this->stat_button_bmps[6];
                }
            }
        } else {
            this->stat_btn_pairs[i][1] = this->stat_button_bmps[9];
        }
    }
    return hit;
}


// 42a778
int32_t VisCharGenStats::OnStatUp(int32_t idx) {
    int32_t cost = this->StatUpCost((&this->stat_body)[idx]);
    if (this->stat_points < cost) {
        return 0;
    }
    if ((&this->stat_body)[idx] >= 0x2D) {
        return 0;
    }
    (&this->stat_body)[idx] += 1;
    this->stat_points -= cost;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->m_GameSession.SetCharacterStats(this->stat_body, this->stat_reaction, this->stat_mind, this->stat_spirit,
                                              this->parent_screen->skills_panel->selected_slot + 1);
    StopSfx(&this->parent_screen->snd_stat);
    CSound::Play((CSound&)this->parent_screen->snd_stat);
    return 1;
}

// 42a87b
int32_t VisCharGenStats::OnStatDown(int32_t idx) {
    int32_t refund = this->StatDownRefund((&this->stat_body)[idx]);
    if ((&this->stat_body)[idx] <= 0xF) {
        return 0;
    }
    (&this->stat_body)[idx] -= 1;
    this->stat_points += refund;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->m_GameSession.SetCharacterStats(this->stat_body, this->stat_reaction, this->stat_mind, this->stat_spirit,
                                              this->parent_screen->skills_panel->selected_slot + 1);
    StopSfx(&this->parent_screen->snd_stat);
    CSound::Play((CSound&)this->parent_screen->snd_stat);
    return 1;
}

// 42a335
int32_t VisCharGenStats::OnLButtonDown(uint32_t wparam, CPoint pos) {
    int32_t hit = this->HandleClick(wparam, pos);
    if (hit != -1) {
        int32_t button = hit & 0xFF;
        int32_t idx = hit >> 8;
        if (button == 1) {
            this->OnStatUp(idx);
        } else if (button == 2) {
            this->OnStatDown(idx);
        }
    }
    return 1;
}


// 429c39
void VisCharGenStats::ReadUnitStats() {
    this->stat_body = this->parent_screen->current_char->body;
    this->stat_reaction = this->parent_screen->current_char->reaction;
    this->stat_mind = this->parent_screen->current_char->mind;
    this->stat_spirit = this->parent_screen->current_char->spirit;
}

// 42a312
int32_t VisCharGenStats::OnMouseMove(uint32_t wparam, CPoint pos) {
    this->HandleClick(wparam, pos);
    return 0;
}

// 42a3a8
int32_t VisCharGenStats::OnLButtonUp(uint32_t wparam, CPoint pos) {
    this->HandleClick(wparam, pos);
    return 1;
}

// 42a3ce
int32_t VisCharGenStats::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    return this->OnLButtonDown(wparam, pos);
}

// 42a3f2
int32_t VisCharGenStats::OnWmUser(uint32_t wparam, CPoint pos) {
    if (wparam & 1) {
        this->OnLButtonDown(wparam, pos);
        return 1;
    }
    return CVisualObject::OnWmUser(wparam, pos);
}


// 428f7f
void VisCharGenStats::Init() {
    this->parent_screen = nullptr;
    this->bmp = nullptr;
    for (int32_t i = 0; i < 10; i++) {
        this->stat_button_bmps[i] = nullptr;
    }
    this->stat_body = 0x1F;
    this->stat_reaction = 0x20;
    this->stat_mind = 0x21;
    this->stat_spirit = 0x22;
    this->texts.SetSize(4, -1);
    for (int32_t i = 0; i < 4; i++) {
        this->texts.ElementAt(i) = TxtFile::AllLines[0xF + i];
    }
    for (int32_t i = 0; i < 4; i++) {
        this->areas[i][0] = CRect(CPoint(0x52, (i << 5) + 0x36), CSize(0x14, 0x14));
        this->areas[i][1] = CRect(CPoint(0x6B, (i << 5) + 0x36), CSize(0x14, 0x14));
        this->areas[i][2] = CRect(CPoint(0x84, (i << 5) + 0x36), CSize(0x14, 0x14));
    }
    for (int32_t i = 0; i < 4; i++) {
        this->label_rects[i] = CRect(CPoint(0x10, i * 0x21 + 0x39),
                                    CSize(g_font4->GetStrWidth(this->texts.ElementAt(i)), g_font4->GetHeight()));
    }
    this->points_rect = CRect(CPoint(0x2E, 0xB5), CSize(0x4D, 0x16));
    this->stat_points = 0;
}

// 428f12
VisCharGenStats::~VisCharGenStats() {
    this->FreeBitmaps();
    this->parent_screen = nullptr;
}

// 428e51
VisCharGenStats::VisCharGenStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->Init();
    this->parent_screen = parent;
}


// 42abea
void VisCharGenFullStats::VMethod7() {
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    LockSurface2();
    this->bmp->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
    CRect unit_rect = screen_rect;
    unit_rect.left += 0xC;
    unit_rect.right += 0xC;
    this->parent_screen->current_char->DrawStatsPanel(&unit_rect);
    UnlockSurface2();
}


// 42ab18
void VisCharGenFullStats::LoadBitmaps() {
    this->FreeBitmaps();
    this->bmp = new CBmp64("graphics\\Interface\\chrgen\\FullStatsL.bmp");
    g_mousept.Update();
}


// 42ac85
const char* VisCharGenFullStats::GetHint() {
    if (this->parent_screen->active_flag == 0) {
        return nullptr;
    }
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    CPoint pt(g_mousept.GetX() - screen_rect.left - 0xC, g_mousept.GetY() - screen_rect.top - 0xC);
    return this->parent_screen->current_char->GetStatsHint(pt.x, pt.y);
}


// 42ab9c
void VisCharGenFullStats::FreeBitmaps() {
    if (this->bmp != nullptr) {
        delete this->bmp;
    }
    this->bmp = nullptr;
}


// 42aaa0
VisCharGenFullStats::~VisCharGenFullStats() {
    this->FreeBitmaps();
    this->parent_screen = nullptr;
}

// 42aa2b
VisCharGenFullStats::VisCharGenFullStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->parent_screen = parent;
    this->bmp = nullptr;
}


// 42b028
void VisCharGenAction::VMethod7() {
    CPoint topleft = this->parent_screen->rect.TopLeft();
    if (this->parent_screen->active_flag == 0) {
        return;
    }
    LockSurface2();
    this->area_bmp->Draw(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, 0);
    for (int32_t i = 0; i < 3; i++) {
        uint16_t* pal;
        if (this->mouse_over_box == i) {
            pal = palette_paris_daisy->GetPalette(0);
        } else {
            pal = palette_husk->GetPalette(0);
        }
        if (this->mouse_over_box < 0 || this->mouse_down_box != this->mouse_over_box || this->mouse_over_box != i) {
            this->btn_off[i]->Draw(topleft.x + this->areas[i].left, topleft.y + this->areas[i].top, 0, 0, 0);
            g_font4->DrawTxt(topleft.x + this->areas[i].left + this->areas[i].Width() / 2,
                             topleft.y + this->areas[i].top + this->areas[i].Height() / 2,
                             this->texts.ElementAt(i), 10, pal);
        } else {
            this->btn_on[i]->Draw(topleft.x + this->areas[i].left, topleft.y + this->areas[i].top, 0, 0, 0);
            g_font4->DrawTxt(topleft.x + this->areas[i].left + this->areas[i].Width() / 2,
                             topleft.y + this->areas[i].top + 1 + this->areas[i].Height() / 2,
                             this->texts.ElementAt(i), 10, pal);
        }
    }
    UnlockSurface2();
}


// 42b57e
void VisCharGenAction::LoadBitmaps() {
    this->FreeBitmaps();
    this->btn_on[0] = new CBmp64("graphics\\interface\\Inn\\button1on.bmp");
    g_mousept.Update();
    this->btn_on[1] = new CBmp64("graphics\\interface\\Inn\\button2on.bmp");
    g_mousept.Update();
    this->btn_on[2] = new CBmp64("graphics\\interface\\Inn\\button3on.bmp");
    g_mousept.Update();
    this->btn_off[0] = new CBmp64("graphics\\interface\\Inn\\button1off.bmp");
    g_mousept.Update();
    this->btn_off[1] = new CBmp64("graphics\\interface\\Inn\\button2off.bmp");
    g_mousept.Update();
    this->btn_off[2] = new CBmp64("graphics\\interface\\Inn\\button3off.bmp");
    g_mousept.Update();
    this->area_bmp = new CBmp64("graphics\\interface\\Inn\\ButtonsArea.bmp");
    g_mousept.Update();
}


// 42b7ee
void VisCharGenAction::FreeBitmaps() {
    for (int32_t i = 0; i < 3; i++) {
        if (this->btn_on[i] != nullptr) {
            delete this->btn_on[i];
        }
        this->btn_on[i] = nullptr;
        if (this->btn_off[i] != nullptr) {
            delete this->btn_off[i];
        }
        this->btn_off[i] = nullptr;
    }
    if (this->area_bmp != nullptr) {
        delete this->area_bmp;
    }
    this->area_bmp = nullptr;
}


// 42b392
int32_t VisCharGenAction::OnLButtonUp(uint32_t wparam, CPoint pos) {
    AfxGetMainWnd();
    if (this->mouse_down_box >= 0 && this->mouse_down_box < 3) {
        if (this->HitTest(pos) == this->mouse_down_box) {
            int32_t box = this->mouse_down_box;
            this->mouse_down_box = -1;
            this->UpdateMouseOver(wparam, pos);
            if (box == 0) {
                this->parent_screen->OnClickFwd();
            } else if (box == 1) {
                this->parent_screen->RollStats();
            } else if (box == 2) {
                this->parent_screen->OnClickBack();
            }
        }
    }
    return 1;
}


// 42b303
int32_t VisCharGenAction::OnLButtonDown(uint32_t wparam, CPoint pos) {
    this->mouse_down_box = this->HitTest(pos);
    if (this->mouse_down_box == 0) {
        CSound::Play((CSound&)this->parent_screen->snd_ok);
    } else if (this->mouse_down_box == 1) {
        CSound::Play((CSound&)this->parent_screen->snd_reset);
    } else if (this->mouse_down_box == 2) {
        CSound::Play((CSound&)this->parent_screen->snd_back);
    }
    return 1;
}


// 42b2e0
int32_t VisCharGenAction::OnMouseMove(uint32_t wparam, CPoint pos) {
    this->UpdateMouseOver(wparam, pos);
    return 0;
}


// 42b455
void VisCharGenAction::ResetMouseBoxes() {
    this->mouse_down_box = -1;
    this->mouse_over_box = -1;
}


// 42b47a
int32_t VisCharGenAction::HitTest(CPoint pt) {
    CPoint topleft = this->parent_screen->rect.TopLeft();
    for (int32_t i = 0; i < 3; i++) {
        if ((this->areas[i] + topleft).PtInRect(pt)) {
            return i;
        }
    }
    return -1;
}


// 42b505
void VisCharGenAction::UpdateMouseOver(uint32_t wparam, CPoint pos) {
    int32_t hit = this->HitTest(pos);
    if (hit >= 0 && !(wparam & 1)) {
        this->mouse_over_box = hit;
    } else if (hit >= 0 && hit == this->mouse_down_box && (wparam & 1)) {
        this->mouse_over_box = hit;
    } else {
        this->mouse_over_box = -1;
    }
}


// 42ae83
void VisCharGenAction::Init() {
    this->mouse_down_box = -1;
    this->mouse_over_box = -1;
    this->areas[0] = CRect(0x1E4, 0x2C, 0x270, 0x5A);
    this->areas[1] = CRect(0x1E4, 0x5B, 0x270, 0x89);
    this->areas[2] = CRect(0x1E4, 0x8A, 0x270, 0xB8);
    for (int32_t i = 0; i < 3; i++) {
        this->btn_on[i] = nullptr;
        this->btn_off[i] = nullptr;
    }
    this->area_bmp = nullptr;
    this->texts.SetSize(3, -1);
    this->texts.ElementAt(0) = TxtFile::AllLines[0xEE];
    this->texts.ElementAt(1) = TxtFile::AllLines[0xEF];
    this->texts.ElementAt(2) = TxtFile::AllLines[0x104];
    this->flags |= FLAG_NOTFOCUS;
}


// 42ae19
VisCharGenAction::~VisCharGenAction() {
    this->FreeBitmaps();
    this->parent_screen = nullptr;
}

// 42ad7d
VisCharGenAction::VisCharGenAction(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->parent_screen = parent;
    this->Init();
}


// 42c1cc
void VisCharGenSkills::LoadBitmaps(uint32_t mage_flag) {
    CPoint topleft = this->parent_screen->rect.TopLeft();
    this->FreeBitmaps();
    this->bmp_roll_stats = new CBmp64("graphics\\interface\\chrgen\\RollStatsR.bmp");
    g_mousept.Update();
    this->bmp_full_stats = new CBmp64("graphics\\interface\\chrgen\\FullStatsR.bmp");
    g_mousept.Update();
    this->bmp_ru_over = new CBmp64("graphics\\interface\\inn\\RUOver.bmp");
    g_mousept.Update();
    this->bmp_human_back = g_bmp_humanbackl;
    if (mage_flag == 0) {
        this->bmp_mask = new CBmp256("graphics\\interface\\chrgen\\fighter\\mask.bmp");
        g_mousept.Update();
        this->bmp_column = new CBmp64("graphics\\interface\\chrgen\\fighter\\column.bmp");
        g_mousept.Update();
        this->bmp_on[0] = new CBmp64("graphics\\interface\\chrgen\\fighter\\sword\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[0] = new CBmp64("graphics\\interface\\chrgen\\fighter\\sword\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[0] = new CBmp64("graphics\\interface\\chrgen\\fighter\\sword\\shine_on.bmp");
        g_mousept.Update();
        this->slot_positions[0] = CPoint(0xF8 + topleft.x, 0x5D + topleft.y);
        this->slot_sizes[0] = CPoint(0x8C, 0x2F);
        this->bmp_on[1] = new CBmp64("graphics\\interface\\chrgen\\fighter\\axe\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[1] = new CBmp64("graphics\\interface\\chrgen\\fighter\\axe\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[1] = new CBmp64("graphics\\interface\\chrgen\\fighter\\axe\\shine_on.bmp");
        g_mousept.Update();
        this->slot_positions[1] = CPoint(0xFC + topleft.x, 0x7E + topleft.y);
        this->slot_sizes[1] = CPoint(0x84, 0x39);
        this->bmp_on[2] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Mace\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[2] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Mace\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[2] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Mace\\shine_on.bmp");
        g_mousept.Update();
        this->slot_positions[2] = CPoint(0xF8 + topleft.x, 0xB6 + topleft.y);
        this->slot_sizes[2] = CPoint(0x8C, 0x2E);
        this->bmp_on[3] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Pike\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[3] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Pike\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[3] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Pike\\shine_on.bmp");
        g_mousept.Update();
        this->slot_positions[3] = CPoint(0xF4 + topleft.x, 0xE1 + topleft.y);
        this->slot_sizes[3] = CPoint(0x94, 0x1C);
        this->bmp_on[4] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Bow\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[4] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Bow\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[4] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Bow\\shine_on.bmp");
        g_mousept.Update();
        this->slot_positions[4] = CPoint(0xF8 + topleft.x, 0xFA + topleft.y);
        this->slot_sizes[4] = CPoint(0x8C, 0x28);
        this->color_keys[0] = 0xFF;
        this->color_keys[1] = 0xBF;
        this->color_keys[2] = 0x98;
        this->color_keys[3] = 0x7F;
        this->color_keys[4] = 0x66;
    } else {
        this->bmp_mask = new CBmp256("graphics\\interface\\chrgen\\mag\\mask.bmp");
        g_mousept.Update();
        this->bmp_column = new CBmp64("graphics\\interface\\chrgen\\mag\\column.bmp");
        g_mousept.Update();
        this->bmp_on[0] = new CBmp64("graphics\\interface\\chrgen\\mag\\fire\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[0] = new CBmp64("graphics\\interface\\chrgen\\mag\\fire\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[0] = new CBmp64("graphics\\interface\\chrgen\\mag\\fire\\shine_on.bmp");
        g_mousept.Update();
        this->slot_positions[0] = CPoint(0x168 + topleft.x, 0x96 + topleft.y);
        this->slot_sizes[0] = CPoint(0x2C, 0x34);
        this->bmp_on[1] = new CBmp64("graphics\\interface\\chrgen\\mag\\water\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[1] = new CBmp64("graphics\\interface\\chrgen\\mag\\water\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[1] = new CBmp64("graphics\\interface\\chrgen\\mag\\water\\shine_on.bmp");
        g_mousept.Update();
        this->slot_positions[1] = CPoint(0xE8 + topleft.x, 0xA5 + topleft.y);
        this->slot_sizes[1] = CPoint(0x30, 0x24);
        this->bmp_on[2] = new CBmp64("graphics\\interface\\chrgen\\mag\\air\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[2] = new CBmp64("graphics\\interface\\chrgen\\mag\\air\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[2] = new CBmp64("graphics\\interface\\chrgen\\mag\\air\\shine_on.bmp");
        g_mousept.Update();
        this->slot_positions[2] = CPoint(0x124 + topleft.x, 0x62 + topleft.y);
        this->slot_sizes[2] = CPoint(0x30, 0x26);
        this->bmp_on[3] = new CBmp64("graphics\\interface\\chrgen\\mag\\earth\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[3] = new CBmp64("graphics\\interface\\chrgen\\mag\\earth\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[3] = new CBmp64("graphics\\interface\\chrgen\\mag\\earth\\shine_on.bmp");
        g_mousept.Update();
        this->slot_positions[3] = CPoint(0x12C + topleft.x, 0xE4 + topleft.y);
        this->slot_sizes[3] = CPoint(0x30, 0x26);
        this->bmp_on[4] = new CBmp64("graphics\\interface\\chrgen\\mag\\astral\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[4] = new CBmp64("graphics\\interface\\chrgen\\mag\\astral\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[4] = new CBmp64("graphics\\interface\\chrgen\\mag\\astral\\shine_on.bmp");
        g_mousept.Update();
        this->slot_positions[4] = CPoint(0x128 + topleft.x, 0x9E + topleft.y);
        this->slot_sizes[4] = CPoint(0x30, 0x2D);
        this->color_keys[0] = 0x7F;
        this->color_keys[1] = 0x66;
        this->color_keys[2] = 0xFF;
        this->color_keys[3] = 0x98;
        this->color_keys[4] = 0xBF;
    }
}


// 42d6ed
void VisCharGenSkills::FreeBitmaps() {
    if (this->bmp_column != nullptr) {
        delete this->bmp_column;
    }
    this->bmp_column = nullptr;
    if (this->bmp_roll_stats != nullptr) {
        delete this->bmp_roll_stats;
    }
    this->bmp_roll_stats = nullptr;
    if (this->bmp_full_stats != nullptr) {
        delete this->bmp_full_stats;
    }
    this->bmp_full_stats = nullptr;
    if (this->bmp_ru_over != nullptr) {
        delete this->bmp_ru_over;
    }
    this->bmp_ru_over = nullptr;
    this->bmp_human_back = nullptr; // Don't free the memory here, as it's not owned.
    if (this->bmp_mask != nullptr) {
        delete this->bmp_mask;
    }
    this->bmp_mask = nullptr;
    for (int32_t i = 0; i < 5; i++) {
        if (this->bmp_on[i] != nullptr) {
            delete this->bmp_on[i];
        }
        this->bmp_on[i] = nullptr;
        if (this->bmp_shine_off[i] != nullptr) {
            delete this->bmp_shine_off[i];
        }
        this->bmp_shine_off[i] = nullptr;
        if (this->bmp_shine_on[i] != nullptr) {
            delete this->bmp_shine_on[i];
        }
        this->bmp_shine_on[i] = nullptr;
    }
}


// 42bb5a
void VisCharGenSkills::VMethod7() {
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    this->slot_states[4] |= 1;
    if (this->parent_screen->active_flag != 0) {
        LockSurface2();
        this->bmp_column->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
        for (int32_t i = 0; i < 4; i++) {
            int32_t state = this->slot_states[i];
            if (state == 1) {
                this->bmp_on[i]->DrawSubRectMasked(this->slot_positions[i].x, this->slot_positions[i].y, 0, 0,
                                           this->slot_sizes[i].x, this->slot_sizes[i].y);
            } else if (state == 2) {
                this->bmp_shine_off[i]->DrawSubRectMasked(this->slot_positions[i].x, this->slot_positions[i].y, 0, 0,
                                                  this->slot_sizes[i].x, this->slot_sizes[i].y);
            } else if (state == 3) {
                this->bmp_shine_on[i]->DrawSubRectMasked(this->slot_positions[i].x, this->slot_positions[i].y, 0, 0,
                                                 this->slot_sizes[i].x, this->slot_sizes[i].y);
            }
        }
        this->DrawBlinkSlot();
        UnlockSurface2();
    }
    CVisualObject::VMethod7();
}


// 42c072
int32_t VisCharGenSkills::OnLButtonDown(uint32_t wparam, CPoint pos) {
    int32_t hit = this->HitTest(pos);
    if (hit == -1 || hit >= 4) {
        return this->CVisualObject::OnLButtonDown(wparam, pos);
    }
    for (int32_t i = 0; i < 4; i++) {
        this->slot_states[i] &= ~1;
    }
    this->slot_states[hit] |= 1;
    this->selected_slot = hit;
    CSound::Play((CSound&)this->skill_sounds[hit]);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->m_GameSession.SetCharacterStats(this->parent_screen->stats_panel->stat_body,
                                              this->parent_screen->stats_panel->stat_reaction,
                                              this->parent_screen->stats_panel->stat_mind,
                                              this->parent_screen->stats_panel->stat_spirit,
                                              hit + 1);
    this->parent_screen->ShowTipHint();
    return this->CVisualObject::OnLButtonDown(wparam, pos);
}


// 42d963
void VisCharGenSkills::LoadSamples(uint32_t mage_flag) {
    if (mage_flag == 0) {
        ReloadSfx(&this->skill_sounds[0], "SFX\\ChrGen\\Skill\\FSword.wav");
        ReloadSfx(&this->skill_sounds[1], "SFX\\ChrGen\\Skill\\FAxe.wav");
        ReloadSfx(&this->skill_sounds[2], "SFX\\ChrGen\\Skill\\FClub.wav");
        ReloadSfx(&this->skill_sounds[3], "SFX\\ChrGen\\Skill\\FPike.wav");
        ReloadSfx(&this->skill_sounds[4], "SFX\\ChrGen\\Skill\\FBow.wav");
    } else {
        ReloadSfx(&this->skill_sounds[0], "SFX\\ChrGen\\Skill\\MFire.wav");
        ReloadSfx(&this->skill_sounds[1], "SFX\\ChrGen\\Skill\\MWater.wav");
        ReloadSfx(&this->skill_sounds[2], "SFX\\ChrGen\\Skill\\MAir.wav");
        ReloadSfx(&this->skill_sounds[3], "SFX\\ChrGen\\Skill\\MEarth.wav");
        ReloadSfx(&this->skill_sounds[4], "SFX\\ChrGen\\Skill\\MAstral.wav");
    }
}


// 42bfcc
int32_t VisCharGenSkills::OnMouseMove(uint32_t wparam, CPoint pos) {
    for (int32_t i = 0; i < 5; i++) {
        this->slot_states[i] &= ~2;
    }
    int32_t hit = this->HitTest(pos);
    if (hit == -1) {
        return this->CVisualObject::OnMouseMove(wparam, pos);
    }
    this->slot_states[hit] |= 2;
    return this->CVisualObject::OnMouseMove(wparam, pos);
}


// 42dbce
const char* VisCharGenSkills::GetHint() {
    if (this->parent_screen->active_flag == 0) {
        return nullptr;
    }
    CPoint pt(g_mousept.GetX(), g_mousept.GetY());
    int32_t hit = this->HitTest(pt);
    if (hit == -1) {
        return nullptr;
    }
    if (this->parent_screen->mage_flag == 0) {
        return TxtFile::AllLines[hit + 0xAB];
    }
    return TxtFile::AllLines[hit + 0xB0];
}


// 42da5a
void VisCharGenSkills::FreeSamples() {
    FreeSfxSample(&this->skill_sounds[0]);
    FreeSfxSample(&this->skill_sounds[1]);
    FreeSfxSample(&this->skill_sounds[2]);
    FreeSfxSample(&this->skill_sounds[3]);
    FreeSfxSample(&this->skill_sounds[4]);
}


// 42c1ab
int32_t VisCharGenSkills::OnLButtonUp(uint32_t wparam, CPoint pos) {
    return this->CVisualObject::OnLButtonUp(wparam, pos);
}

// 42bd41
void VisCharGenSkills::DrawBlinkSlot() {
    CPoint pt(g_mousept.GetX(), g_mousept.GetY());
    this->parent_screen->GetRect().TopLeft();
    if ((blink_inited & 1) == 0) {
        blink_inited |= 1;
        blink_hover_time = timeGetTime();
    }
    if ((blink_inited & 2) == 0) {
        blink_inited |= 2;
        blink_cycle_time = timeGetTime();
    }
    uint32_t now = timeGetTime();
    int32_t hover = this->HitTest(pt);
    if (this->parent_screen->tips_step != 0 || this->parent_screen->tips == nullptr) {
        return;
    }
    if (now - blink_hover_time < 0x1F4) {
        blink_cycle_time = now;
        return;
    }
    if (blink_hover_slot != -1) {
        blink_cycle_len = 4;
        blink_slot = (blink_hover_slot + 1) % 4;
    }
    if (hover != -1) {
        blink_hover_time = now;
        blink_cycle_time = now;
        blink_hover_slot = hover;
        return;
    }
    blink_cycle_len = 4;
    blink_slot = blink_slot % 4;
    if (this->slot_states[blink_slot] == 1) {
        this->bmp_shine_on[blink_slot]->DrawSubRectMasked(this->slot_positions[blink_slot].x, this->slot_positions[blink_slot].y, 0, 0,
                                                  this->slot_sizes[blink_slot].x, this->slot_sizes[blink_slot].y);
    } else {
        this->bmp_shine_off[blink_slot]->DrawSubRectMasked(this->slot_positions[blink_slot].x, this->slot_positions[blink_slot].y, 0, 0,
                                                   this->slot_sizes[blink_slot].x, this->slot_sizes[blink_slot].y);
    }
    blink_hover_slot = -1;
    if (now - blink_cycle_time <= 0x12C) {
        return;
    }
    if (blink_cycle_len == -1) {
        return;
    }
    blink_slot = (blink_slot + 1) % blink_cycle_len;
    blink_cycle_time = now;
}

// 42dadf
int32_t VisCharGenSkills::HitTest(CPoint pt) {
    if (this->parent_screen->active_flag == 0) {
        return -1;
    }
    if (!this->parent_screen->rect.PtInRect(pt)) {
        return -1;
    }
    CPoint topleft = this->parent_screen->rect.TopLeft();
    pt -= CSize(topleft.x, topleft.y);
    uint8_t* data = (uint8_t*)this->bmp_mask->GetData();
    int32_t stride = this->bmp_mask->GetWidth(0);
    int32_t offset = pt.y * stride + pt.x - 0xA0;
    for (int32_t i = 0; i < 4; i++) {
        if (data[offset] == this->color_keys[i]) {
            return i;
        }
    }
    return -1;
}


// 42ba81
void VisCharGenSkills::Init() {
    this->bmp_column = nullptr;
    this->bmp_mask = nullptr;
    this->bmp_roll_stats = nullptr;
    this->bmp_full_stats = nullptr;
    this->bmp_ru_over = nullptr;
    this->bmp_human_back = nullptr;
    for (int32_t i = 0; i < 5; i++) {
        this->bmp_on[i] = nullptr;
        this->bmp_shine_off[i] = nullptr;
        this->bmp_shine_on[i] = nullptr;
        this->slot_states[i] = 0;
        this->skill_sounds[i] = nullptr;
    }
    this->selected_slot = 0;
    this->slot_states[this->selected_slot] = 1;
}


// 42ba30
VisCharGenSkills::~VisCharGenSkills() {
    this->FreeBitmaps();
}

// 42b98c
VisCharGenSkills::VisCharGenSkills(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->parent_screen = parent;
    this->Init();
}



// 4B2B3D
void VisCharInfo::VMethod7() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if ((main_wnd->dialogsMask & (DIALOG_MASK_CHAR_SELECT | DIALOG_MASK_CHARGEN | 0x20 | DIALOG_MASK_TAVERN | DIALOG_MASK_SHOP | DIALOG_MASK_INGAME)) == 0) {
        return;
    }

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    int32_t moved_up;
    if (g_ScreenSize.bottom - screen_rect.bottom > screen_rect.Height() && main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
        moved_up = 1;
    } else {
        moved_up = 0;
    }

    VisPlayingArea* map = this->map_context;
    LockSurface2();
    if (moved_up != 0) {
        this->info_mode = 1;
    }

    if (this->info_mode != 0) {
        g_bmp_humanbackr->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
    } else {
        g_bmp_textbackr->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
    }

    if ((main_wnd->dialogsMask & (DIALOG_MASK_INGAME | DIALOG_MASK_SHOP)) != 0 && (main_wnd->dialogsMask & DIALOG_MASK_TAVERN) == 0) {
#ifdef A2CLIENT_PATCH
        // Tangar is_spell_book_open (4B758F): the open/closed book sprite is the
        // only reliable spell-book indicator, so record it while drawing.
        bool tangar_book_open = map->IsBookOpen() != 0;
        tangar_hotkeys::SetSpellBookOpen(tangar_book_open);
        if (tangar_book_open) {
#else
        if (map->IsBookOpen()) {
#endif
            g_bmp_bookopened->DrawSubRectMasked(screen_rect.left, screen_rect.top, 0, 0, 0x1C, 0x26);
        } else {
            g_bmp_bookclosed->DrawSubRectMasked(screen_rect.left, screen_rect.top + 4, 0, 0, 0x1C, 0x25);
        }
    }

    if ((main_wnd->dialogsMask & (DIALOG_MASK_CHARGEN | 0x20 | DIALOG_MASK_TAVERN | DIALOG_MASK_SHOP)) != 0) {
        if ((main_wnd->dialogsMask & DIALOG_MASK_CHARGEN) == 0 || main_wnd->sessionMode != SESSION_LOCAL_BATTLE) {
            g_bmp_ar1->DrawSubRectMasked(screen_rect.left + 1, screen_rect.top + 0xCD, 0, 0, 0x20, 0x20);
            g_bmp_ar2->DrawSubRectMasked(screen_rect.left + 0x77, screen_rect.top + 0xCD, 0, 0, 0x20, 0x20);
        }
    } else if ((main_wnd->dialogsMask & DIALOG_MASK_CHAR_SELECT) == 0) {
        if (map->IsBagOpen()) {
            g_bmp_backpackop->DrawSubRectMasked(screen_rect.left, screen_rect.top + 0xD0, 0, 0, 0x20, 0x1F);
        } else {
            g_bmp_backpackcl->DrawSubRectMasked(screen_rect.left + 1, screen_rect.top + 0xC9, 0, 0, 0x1C, 0x1E);
        }
        g_bmp_diskette->DrawSubRectMasked(screen_rect.left + 0x7E, screen_rect.top + 0xCE, 0, 0, 0x20, 0x20);
    }

    if ((main_wnd->dialogsMask & (DIALOG_MASK_CHAR_SELECT | DIALOG_MASK_CHARGEN)) == 0 && moved_up == 0) {
        if (this->info_mode != 0) {
            g_bmp_humanmode->DrawSubRectMasked(screen_rect.left + 0x80, screen_rect.top + 4, 0, 0, 0x1C, 0x20);
        } else {
            g_bmp_textmode->DrawSubRectMasked(screen_rect.left + 0x80, screen_rect.top + 4, 0, 0, 0x1C, 0x20);
        }
    }

    CStructure* structure = nullptr;
    CUnit* unit = nullptr;
    uint16_t selected_id = (uint16_t)map->hovered_unit_id;
    CGameObject* found_obj = nullptr;
    int32_t found = 0;
    if (selected_id != 0 && main_wnd->drag_entry == nullptr && main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
        found = map->objects.Lookup(selected_id, found_obj);
    }
    if (selected_id != 0 && found != 0) {
        if (found_obj->IsKindOf(RUNTIME_CLASS(CStructure))) {
            structure = (CStructure*)found_obj;
        } else {
            unit = (CUnit*)found_obj;
        }
    } else if (map->selected_object_count == 1) {
        if (map->primary_selected_object->IsKindOf(RUNTIME_CLASS(CStructure))) {
            structure = (CStructure*)map->primary_selected_object;
        } else {
            unit = (CUnit*)map->primary_selected_object;
        }
    }

    if (structure != nullptr) {
        if (this->info_mode == 0) {
            g_font2->DrawTextWithShadow(screen_rect.right - 0x58, screen_rect.top + 0x1C, txt_building.GetLine(structure->typeId - 1), 2, clrsh_DullGold, 1);
            g_font2->DrawTextWithShadow(screen_rect.right - 0x58, screen_rect.top + 0x2C, TxtFile::AllLines[0x13], 2, clrsh_DullGold, 1);
            char hp_text[0x10];
            sprintf(hp_text, "%d/%d", structure->hp, structure->hp_max);
            g_font2->DrawTextWithShadow(screen_rect.right - 0x58, screen_rect.top + 0x36, hp_text, 2, clrsh_Oxley, 1);
        } else {
            if (strcmp(this->picturename, g_StructuresInfo[structure->typeId]->picture) != 0) {
                strcpy(this->picturename, g_StructuresInfo[structure->typeId]->picture);
                char path[0x100];
                sprintf(path, "graphics\\infowindow\\%s.bmp", this->picturename);
                this->bitmap->LoadFile(path, nullptr);
                memset(this->hitmap->GetData(), 0, this->hitmap->GetWidth() * this->hitmap->GetHeight());
            }
            this->bitmap->DrawSubRectMasked(screen_rect.left, screen_rect.top + 2, 0, 0, 0xA0, 0xF0);
            int32_t text_y = 0x36;
            if (map->quest_landmark_some_id != 0) {
                g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + text_y, TxtFile::AllLines[0x15B], 2, clrsh_DullGold, 1);
                g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + text_y + 0xC, TxtFile::AllLines[0x15C], 2, clrsh_DullGold, 1);
                text_y += 0x20;
            }
            if (map->quest_building_some_id != 0) {
                g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + text_y, TxtFile::AllLines[0x15D], 2, clrsh_DullGold, 1);
                g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + text_y + 0xC, TxtFile::AllLines[0x15E], 2, clrsh_DullGold, 1);
            }
        }
    } else if (unit != nullptr) {
        if (this->info_mode == 0) {
            unit->DrawStatsPanel(&screen_rect);
        } else {
            if ((unit->unitFlags & (UNIT_FLAG_PLAYER_CLASS | UNIT_FLAG_CREATURE)) != 0) {
                char temp_path[0x100];
                char fname[0x100];
                char full_path[0x100];
                GetTempPathA(0x100, temp_path);
                sprintf(fname, A2CLIENT_PATCH_ENABLED ? "allods-%u-%u.$$$" : "allods-2-%d.$$$",
                A2CLIENT_PATCH_ENABLED ? GetCurrentProcessId() : unit->unit_id,
                A2CLIENT_PATCH_ENABLED ? unit->unit_id : 0);
                sprintf(full_path, "%s%s", temp_path, fname);
                if (strcmp(this->picturename, fname) != 0) {
                    strcpy(this->picturename, fname);
                    if ((unit->unitFlags & UNIT_FLAG_EQUIP_CHANGED) != 0) {
                        UnlockSurface2();
                        unit->RenderHeroPortrait(full_path, this->bitmap, this->hitmap);
                        LockSurface2();
                    } else {
                        this->bitmap->LoadFile(fname, this->hitmap);
                    }
                } else if ((unit->unitFlags & UNIT_FLAG_EQUIP_CHANGED) != 0) {
                    UnlockSurface2();
                    unit->RenderHeroPortrait(full_path, this->bitmap, this->hitmap);
                    LockSurface2();
                }
            } else {
                char picture[0x100];
                strcpy(picture, g_VFX_info[unit->typeId]->info_picture);
                char suffix[0x50];
                sprintf(suffix, "%d", unit->face);
                if (unit->face > 1) {
                    strcat(picture, suffix);
                }
                if (strcmp(this->picturename, picture) != 0) {
                    strcpy(this->picturename, picture);
                    char path[0x100];
                    sprintf(path, "graphics\\infowindow\\%s.bmp", this->picturename);
                    this->bitmap->LoadFile(path, nullptr);
                    memset(this->hitmap->GetData(), 0, this->hitmap->GetWidth() * this->hitmap->GetHeight());
                }
            }
            this->bitmap->DrawSubRectMasked(screen_rect.left, screen_rect.top + 2, 0, 0, 0xA0, 0xF0);
            if (map->quest_some_id_2 != 0 || map->quest_some_id != 0) {
                g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x36, TxtFile::AllLines[0x159], 2, clrsh_DullGold, 1);
                g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x42, TxtFile::AllLines[0x15A], 2, clrsh_DullGold, 1);
            }
        }
    } else {
        if (map->selected_object_count > 1) {
            g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x36, TxtFile::AllLines[0x31], 2, clrsh_DullGold, 1);
            g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x42, TxtFile::AllLines[0x32], 2, clrsh_DullGold, 1);
            char count_text[0x10];
            sprintf(count_text, "%d", map->selected_object_count);
            g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x4E, count_text, 2, clrsh_DullGold, 1);
        } else if (map->selected_object_count == 0) {
            g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x36, TxtFile::AllLines[0x2F], 2, clrsh_DullGold, 1);
            g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x42, TxtFile::AllLines[0x30], 2, clrsh_DullGold, 1);
        }
    }

    UnlockSurface2();
    this->dirty = 0;
}

// 4B19B0
const char* VisCharInfo::GetHint() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        return nullptr;
    }
    if ((main_wnd->dialogsMask & DIALOG_MASK_POPUP) != 0) {
        return nullptr;
    }

    VisPlayingArea* map = this->map_context;
    CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    int32_t moved_up;
    if (g_ScreenSize.bottom - screen_rect.bottom > screen_rect.Height() && main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
        moved_up = 1;
    } else {
        moved_up = 0;
    }

    CRect book_rect(screen_rect.left, screen_rect.top, screen_rect.left + 0x1C, screen_rect.top + 0x24);
    CRect backpack_rect(screen_rect.left, screen_rect.bottom - 0x28, screen_rect.left + 0x1C, screen_rect.bottom);
    CRect mode_btn_rect(screen_rect.left + 0x80, screen_rect.top, screen_rect.right, screen_rect.top + 0x24);
    CRect ar1_rect(screen_rect.left + 1, screen_rect.top + 0xCD, screen_rect.left + 0x21, screen_rect.top + 0xED);
    CRect ar2_rect(screen_rect.left + 0x77, screen_rect.top + 0xCD, screen_rect.left + 0x97, screen_rect.top + 0xED);
    CRect diskette_rect(screen_rect.left + 0x7E, screen_rect.top + 0xCE, screen_rect.left + 0x9E, screen_rect.top + 0xEE);

    if ((main_wnd->dialogsMask & (DIALOG_MASK_INGAME | DIALOG_MASK_SHOP)) != 0 && (main_wnd->dialogsMask & DIALOG_MASK_TAVERN) == 0) {
        if (book_rect.PtInRect(mouse_pt)) {
            if (map->IsBookOpen()) {
                return TxtFile::AllLines[9];
            }
            return TxtFile::AllLines[8];
        }
    }

    if ((main_wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
        if (backpack_rect.PtInRect(mouse_pt)) {
            if ((main_wnd->dialogsMask & DIALOG_MASK_SHOP) == 0 && (main_wnd->dialogsMask & DIALOG_MASK_TAVERN) == 0) {
                if (map->IsBagOpen()) {
                    return TxtFile::AllLines[0xB];
                }
                return TxtFile::AllLines[0xA];
            }
        }
    }

    if ((main_wnd->dialogsMask & (DIALOG_MASK_CHAR_SELECT | DIALOG_MASK_CHARGEN)) == 0) {
        if (mode_btn_rect.PtInRect(mouse_pt) && moved_up == 0) {
            if (this->info_mode != 0) {
                return TxtFile::AllLines[0xD];
            }
            return TxtFile::AllLines[0xC];
        }
    }

    if ((main_wnd->dialogsMask & (DIALOG_MASK_CHARGEN | 0x20 | DIALOG_MASK_TAVERN | DIALOG_MASK_SHOP)) != 0) {
        if ((main_wnd->dialogsMask & DIALOG_MASK_CHARGEN) == 0 || main_wnd->sessionMode != SESSION_LOCAL_BATTLE) {
            if (ar1_rect.PtInRect(mouse_pt)) {
                return TxtFile::AllLines[0x79];
            }
            if (ar2_rect.PtInRect(mouse_pt)) {
                return TxtFile::AllLines[0x7A];
            }
        }
        if (ar1_rect.PtInRect(mouse_pt)) {
            return TxtFile::AllLines[0x34];
        }
        if (ar2_rect.PtInRect(mouse_pt)) {
            return TxtFile::AllLines[0x35];
        }
    } else if ((main_wnd->dialogsMask & DIALOG_MASK_CHAR_SELECT) == 0) {
        if (diskette_rect.PtInRect(mouse_pt)) {
            return TxtFile::AllLines[0xE];
        }
    }

    CGameObject* sel = nullptr;
    if (map->selected_object_count == 1) {
        sel = map->primary_selected_object;
    }
    if (sel == nullptr) {
        return nullptr;
    }
    if ((main_wnd->dialogsMask & DIALOG_MASK_POPUP) != 0) {
        return nullptr;
    }

    CUnit* unit = (CUnit*)sel;
    if (unit->map_player != nullptr && unit->map_player->index != 0) {
        if (map->my_main_unit->IsVisibleTo(unit->map_player->index) == 0) {
            if (this->info_mode != 0) {
                return nullptr;
            }
            return unit->GetStatsHint(mouse_pt.x - screen_rect.left, mouse_pt.y - screen_rect.top - 2);
        }
    }

    if (this->info_mode == 0) {
        return unit->GetStatsHint(mouse_pt.x - screen_rect.left, mouse_pt.y - screen_rect.top - 2);
    }

    uint8_t* data = (uint8_t*)this->hitmap->GetData();
    uint8_t color = data[(mouse_pt.y - screen_rect.top - 2) * 0xA0 + (mouse_pt.x - screen_rect.left)];
    if (color == 0) {
        return nullptr;
    }
    TokenEntry* entry = unit->equipmentTokens[color - 1];
    if (entry == nullptr) {
        return nullptr;
    }
    return entry->GetHintText();
}

// 4B3A0D
// client: 4b8374
int32_t VisCharInfo::VMethod27(int32_t a) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    int32_t disallow = 0;
    if (main_wnd->drag_entry == nullptr) {
        disallow = 1;
    }

    VisPlayingArea* map = this->map_context;
    if (map->selected_object_count != 1) {
        disallow = 1;
    }

    CUnit* unit = (CUnit*)map->primary_selected_object;
    if (unit->map_player != map->my_main_unit) {
        return 0;
    }

    if (a < 2 && (unit->last_action == ACTION_ATTACK || unit->last_action == ACTION_SHOOT || unit->last_action == ACTION_CAST)) {
        disallow = 1;
    }

    TokenEntry* token = main_wnd->drag_entry;
    if ((token->flg & 6) == 0 && token->item_id == 0xE4D) {
        main_wnd->PostMessageA(MAIN_MSG_SHOW_FAME_HALL_DOC, 0, 0);
    }

    if (unit->IsItemRestricted(token) == 0) {
        disallow = 1;
    }
    if (token->IsQuestTreasure() != 0) {
        disallow = 1;
    }

    int32_t spell_id = token->GetAttribute(0x2A);
    if (spell_id != 0) {
        if ((unit->unitFlags & UNIT_FLAG_MAGE) != 0) {
            if (((1 << (spell_id & 0x1F)) & unit->spells) != 0) {
                disallow = 1;
            }
        } else {
            disallow = 1;
        }
    }
#ifdef A2CLIENT
    if (main_wnd->net_session.internet_game != 0 && main_wnd->hat_settings.ishat != 0 &&
        (main_wnd->hat_settings.deathmatch == 1 || main_wnd->hat_settings.deathmatch == 2)) {
        disallow = (token->IsPotion() == 0 && token->IsScroll() == 0) ? 1 : 0;
    }
#endif

    if (disallow != 0) {
        main_wnd->vis_invtype1->VMethod37(main_wnd->drag_src_slot);
        return 0;
    }

    if ((token->flg & 0x10) != 0 && (token->flg & 1) != 0) {
        int32_t removed = main_wnd->vis_invtype1->VMethod37(main_wnd->drag_src_slot);
        main_wnd->vis_spellbook->SelectScrollEntry(removed);
        map->SetOrderMode(0xA);
        return 0;
    }

    if ((token->flg & 1) != 0) {
        TokenEntry* split = nullptr;
        if (token->count > 1) {
            split = new TokenEntry(token);
            split->count -= 1;
            token->count = 1;
        }

        int32_t amount = token->count;
        map->SendItemOperation(main_wnd->drag_src_tab, main_wnd->drag_src_slot, this->VMethod28(), a + 1, amount);

        if (split != nullptr) {
            main_wnd->vis_invtype1->VMethod37(main_wnd->drag_src_slot);
            return 0;
        }
        if (main_wnd->drag_entry != nullptr) {
            delete main_wnd->drag_entry;
        }
        main_wnd->ResetItemCursor();
        return 0;
    }

    TokenEntry* old_token = unit->equipmentTokens[a];
    if (old_token != nullptr) {
        main_wnd->vis_invtype1->VMethod26(old_token, main_wnd->drag_src_slot);
        main_wnd->vis_invtype1->VMethod33(unit);
    }

    if (a == 0) {
        if (token->IsTwoHandedWeapon() != 0 && unit->equipmentTokens[1] != nullptr) {
            main_wnd->vis_invtype1->VMethod26(unit->equipmentTokens[1], main_wnd->drag_src_slot);
            main_wnd->vis_invtype1->VMethod33(unit);
            unit->equipmentTokens[1] = nullptr;
        }
    }

    if (a == 1) {
        if (unit->equipmentTokens[0] != nullptr && unit->equipmentTokens[0]->IsTwoHandedWeapon() != 0) {
            main_wnd->vis_invtype1->VMethod26(unit->equipmentTokens[0], main_wnd->drag_src_slot);
            main_wnd->vis_invtype1->VMethod33(unit);
            unit->equipmentTokens[0] = nullptr;
        }
    }

    unit->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
    this->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
    unit->equipmentTokens[a] = main_wnd->drag_entry;

    int32_t amount = main_wnd->drag_entry->count;
    map->SendItemOperation(main_wnd->drag_src_tab, main_wnd->drag_src_slot, this->VMethod28(), a + 1, amount);

    main_wnd->ResetItemCursor();
    unit->ReloadSprite();
    main_wnd->vis_root->MsgProc(VIS_MSG_SET_OPTION, this->id, 0);
    return (int32_t)old_token;
}

// 4B2346
int32_t VisCharInfo::OnLButtonUp(uint32_t wparam, CPoint pos) {
    CRect screen_rect;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->ClientRectToScreen(&screen_rect, this->rect);

    int32_t moved_up;
    if (g_ScreenSize.bottom - screen_rect.bottom > screen_rect.Height() && main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
        moved_up = 1;
    } else {
        moved_up = 0;
    }

    VisPlayingArea* map = this->map_context;

    CRect backpack_rect(screen_rect.left, screen_rect.bottom - 0x28, screen_rect.left + 0x1C, screen_rect.bottom);
    CRect ar1_rect(screen_rect.left + 1, screen_rect.top + 0xCD, screen_rect.left + 0x21, screen_rect.top + 0xED);
    CRect ar2_rect(screen_rect.left + 0x77, screen_rect.top + 0xCD, screen_rect.left + 0x97, screen_rect.top + 0xED);
    CRect diskette_rect(screen_rect.left + 0x7E, screen_rect.top + 0xCE, screen_rect.left + 0x9E, screen_rect.top + 0xEE);

    if (main_wnd->drag_entry != nullptr) {
        TokenEntry* token = main_wnd->drag_entry;
        if (token->slot_code == 2 || token->slot_code == 1) {
            g_Cursors[0]->Use();
            this->VMethod27(token->GetType() - 1);
            return 1;
        }
        if (token->slot_code >= 5 && token->slot_code <= 8) {
            ((VisShop*)main_wnd->vis_root->FindChild(0x3E8))->CancelItemDrag();
            return 1;
        }
    }

    if (main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
        if (backpack_rect.PtInRect(pos)) {
            this->MsgProc(VIS_MSG_TOGGLE_SELECT_PANEL, 0, 0);
            map->MsgProc(VIS_MSG_TOGGLE_SELECT_PANEL, 0, 0);
        }
    }

    CRect book_rect(screen_rect.left, screen_rect.top, screen_rect.left + 0x1C, screen_rect.top + 0x24);
    if ((main_wnd->dialogsMask & (DIALOG_MASK_INGAME | DIALOG_MASK_SHOP)) != 0 && (main_wnd->dialogsMask & DIALOG_MASK_TAVERN) == 0) {
        if (book_rect.PtInRect(pos)) {
            this->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
            map->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
            if ((main_wnd->dialogsMask & DIALOG_MASK_SHOP) != 0) {
                main_wnd->vis_root->FindChild(0x3E8)->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
            }
        }
    }

    CRect mode_btn_rect(screen_rect.left + 0x80, screen_rect.top, screen_rect.right, screen_rect.top + 0x24);
    if ((main_wnd->dialogsMask & (DIALOG_MASK_CHAR_SELECT | DIALOG_MASK_CHARGEN)) == 0) {
        if (mode_btn_rect.PtInRect(pos) && moved_up == 0) {
            this->MsgProc(VIS_MSG_TOGGLE_INFO_MODE, 0, 0);
            map->full_redraw_requested = 1;
        }
    }

    if ((main_wnd->dialogsMask & (DIALOG_MASK_CHARGEN | 0x20 | DIALOG_MASK_TAVERN | DIALOG_MASK_SHOP)) != 0) {
        if (ar1_rect.PtInRect(pos)) {
            g_SfxArray[1]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xDC, 0);
            main_wnd->vis_root->MsgProc(VIS_MSG_SELECT_PREV, 0, 0);
        }
        if (ar2_rect.PtInRect(pos)) {
            g_SfxArray[1]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xDC, 0);
            main_wnd->vis_root->MsgProc(VIS_MSG_SELECT_NEXT, 0, 0);
        }
    }

    if ((main_wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
        if (diskette_rect.PtInRect(pos)) {
            g_SfxArray[1]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xDC, 0);
            main_wnd->PostMessageA(MAIN_MSG_SHOW_INGAME_MENU, 0, 0);
        }
    }
    return 1;
}

// 4B289E
int32_t VisCharInfo::OnMouseMove(uint32_t wparam, CPoint pos) {
    if ((wparam & 1) == 0) {
        return 0;
    }

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisPlayingArea* map = this->map_context;

    if (main_wnd->drag_entry != nullptr) {
        return 0;
    }
    if (this->info_mode == 0) {
        return 0;
    }

    CUnit* unit = (CUnit*)map->primary_selected_object;
    if (map->selected_object_count != 1) {
        return 0;
    }
    if ((unit->unitFlags & UNIT_FLAG_PLAYER_CLASS) == 0) {
        return 0;
    }

    uint8_t* data = (uint8_t*)this->hitmap->GetData();
    uint8_t color = data[(pos.y - screen_rect.top - 2) * 0xA0 + (pos.x - screen_rect.left)];
    if (color == 0) {
        return 0;
    }

    if (this->VMethod26(color - 1) == 0) {
        return 0;
    }

    CString name = main_wnd->drag_entry->GetSpriteName();
    CString path = "graphics\\inventory\\" + name + ".16a";
    main_wnd->SetItemCursor(main_wnd->drag_entry, main_wnd->drag_src_slot, path, main_wnd->drag_src_tab);

    unit->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
    this->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, 0);
    unit->m_bSelectionDirty = 1;
    return 0;
}

// 4B38AF
// client: 4b81df
int32_t VisCharInfo::VMethod26(int32_t a) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        return 0;
    }
#ifdef A2CLIENT
    if (main_wnd->net_session.internet_game != 0 && main_wnd->hat_settings.ishat != 0 &&
        (main_wnd->hat_settings.deathmatch == 1 || main_wnd->hat_settings.deathmatch == 2)) {
        return 0;
    }
#endif
    if ((main_wnd->dialogsMask & DIALOG_MASK_SHOP) == 0 && main_wnd->dialogsMask != DIALOG_MASK_INGAME) {
        return 0;
    }

    VisPlayingArea* map = this->map_context;
    if (map->selected_object_count != 1) {
        return 0;
    }

    CUnit* unit = (CUnit*)map->primary_selected_object;
    if (unit->map_player != map->my_main_unit) {
        return 0;
    }

    if (a < 2 && (unit->last_action == ACTION_ATTACK || unit->last_action == ACTION_SHOOT || unit->last_action == ACTION_CAST)) {
        return 0;
    }

    unit->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
    this->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
    main_wnd->drag_entry = unit->equipmentTokens[a];
    unit->equipmentTokens[a] = nullptr;
    main_wnd->drag_src_slot = a;
    main_wnd->drag_src_tab = this->VMethod28();
    unit->ReloadSprite();
    main_wnd->vis_root->MsgProc(VIS_MSG_SET_OPTION, this->id, 0);
    return (int32_t)main_wnd->drag_entry;
}

// 4B2239
int32_t VisCharInfo::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    uint8_t* data = (uint8_t*)this->hitmap->GetData();
    VisPlayingArea* map = this->map_context;

    uint8_t color = data[(pos.y - screen_rect.top - 2) * 0xA0 + (pos.x - screen_rect.left)];
    if (color == 0) {
        return 1;
    }

    CUnit* unit = (CUnit*)map->primary_selected_object;
    if (map->selected_object_count != 1) {
        return 0;
    }
    if ((unit->unitFlags & UNIT_FLAG_PLAYER_CLASS) == 0) {
        return 0;
    }

    if (this->VMethod26(color - 1) != 0) {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        int32_t slot = main_wnd->vis_invtype1->GetVisibleStart();
        main_wnd->vis_invtype1->VMethod37(slot);
    }
    return 1;
}

// 4B2AD5
int32_t VisCharInfo::OnKeyDown(uint32_t wparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (wparam != 9) {
        return 0;
    }

    if ((main_wnd->dialogsMask & DIALOG_MASK_CHAR_SELECT) == 0 && (main_wnd->dialogsMask & DIALOG_MASK_CHARGEN) == 0) {
        this->MsgProc(VIS_MSG_TOGGLE_INFO_MODE, 0, 0);
    }
    return 1;
}

// 4B2857
int32_t VisCharInfo::OnRButtonUp(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisPlayingArea* map = this->map_context;
    if (main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
        return map->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);
    }
    return 1;
}

// 4B2215
int32_t VisCharInfo::OnLButtonDown(uint32_t wparam, CPoint pos) {
    return 1;
}

// 4B2227
int32_t VisCharInfo::OnRButtonDown(uint32_t wparam, CPoint pos) {
    return 1;
}

// 4B2334
int32_t VisCharInfo::OnRButtonDblClk(uint32_t wparam, CPoint pos) {
    return 1;
}

// 4B4830
int32_t VisCharInfo::VMethod28() {
    return TRANSFER_DST_EQUIP;
}

// 4B16C3
VisCharInfo::VisCharInfo(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : CVisualObject(_id, l, t, r, b, nullptr) {
    this->gap_64 = 0;
    this->dirty = 0;
    this->selection_panel_state = 0;
    this->spell_panel_state = 0;
    this->info_mode = 1;
    this->bitmap = new CBmp64(0xa0, 0xf0);
    this->picturename[0] = 0;
    this->hitmap = new CBmp256(0xa0, 0xf0);
}

// 4B1909
VisCharInfo::~VisCharInfo() {
    delete this->bitmap;
    delete this->hitmap;
}

// 4B201C
int32_t VisCharInfo::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    int32_t result = CVisualObject::MsgProc(msg, wparam, lparam);

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
#ifdef A2CLIENT_PATCH
    // Tangar panel_toggle (4B6977): remember the char-info panel object.
    tangar_hotkeys::RememberPanel(this);
#endif
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    int32_t moved_up;
    if (g_ScreenSize.bottom - screen_rect.bottom > screen_rect.Height() && main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
        moved_up = 1;
    } else {
        moved_up = 0;
    }

    if (result == 0) {
        switch (msg) {
        case VIS_MSG_SET_MAP_CONTEXT:
            this->map_context = (VisPlayingArea*)wparam;
            this->dirty = 1;
            break;
        case VIS_MSG_DATA_REFRESH:
            this->dirty = 1;
            break;
        case VIS_MSG_FRAME_TICK:
            if (main_wnd->dialogsMask == DIALOG_MASK_INGAME || main_wnd->dialogsMask == (DIALOG_MASK_INGAME | DIALOG_MASK_SHOP) || main_wnd->dialogsMask == (DIALOG_MASK_INGAME | DIALOG_MASK_TAVERN)) {
                if (this->dirty != 0) {
                    this->VMethod9();
                }
            }
            if ((main_wnd->dialogsMask & (DIALOG_MASK_INGAME | DIALOG_MASK_SHOP)) != 0) {
                this->UpdateCursor();
            }
            break;
        case VIS_MSG_TOGGLE_SELECT_PANEL:
            this->dirty = 1;
            this->selection_panel_state = (this->selection_panel_state == 0);
            break;
        case VIS_MSG_TOGGLE_SPELL_PANEL:
            this->dirty = 1;
            this->spell_panel_state = (this->spell_panel_state == 0);
            break;
        case VIS_MSG_STATUS_REFRESH:
            this->dirty = 1;
            break;
        case VIS_MSG_TOGGLE_INFO_MODE:
            if (moved_up == 0) {
                g_SfxArray[1]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xDC, 0);
                this->info_mode = (this->info_mode == 0);
                this->dirty = 1;
            }
            break;
        }
    }
    return result;
}

// 4B36B4
void VisCharInfo::UpdateCursor() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CSprite256* cur_sprite = g_mousept.GetCursorSprite();

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    CCursor* new_cursor = nullptr;
    CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
    if (!screen_rect.PtInRect(mouse_pt)) {
        return;
    }

    if ((main_wnd->dialogsMask & DIALOG_MASK_SHOP) != 0) {
        if (main_wnd->drag_entry != nullptr) {
            VisShop* shop = (VisShop*)main_wnd->vis_root->FindChild(0x3E8);
            TokenEntry* token = main_wnd->drag_entry;
            if (shop->placement_lock != 0 || token->slot_code == 1 || token->slot_code == 2) {
                new_cursor = main_wnd->item_cursor;
            } else {
                new_cursor = g_Cursors[CURSOR_CANTPUT];
            }
        }
    } else {
        if (mouse_pt.x >= g_ScreenSize.right - 2 && main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
            if (mouse_pt.y == 0) {
                new_cursor = g_Cursors[CURSOR_ARROW1];
            } else if (mouse_pt.y >= g_ScreenSize.bottom - 2) {
                new_cursor = g_Cursors[CURSOR_ARROW3];
            } else {
                new_cursor = g_Cursors[CURSOR_ARROW2];
            }
        } else if (mouse_pt.y >= g_ScreenSize.bottom - 2 && main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
            new_cursor = g_Cursors[CURSOR_ARROW4];
        } else {
            if (screen_rect.PtInRect(mouse_pt)) {
                new_cursor = g_Cursors[CURSOR_DEFAULT];
            }
        }
        if (main_wnd->drag_entry != nullptr) {
            new_cursor = main_wnd->item_cursor;
        }
    }

    if (new_cursor == nullptr) {
        return;
    }
    if (new_cursor->GetSprite() == cur_sprite) {
        return;
    }
    new_cursor->Use();
}





