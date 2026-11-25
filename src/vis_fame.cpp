#include <winsock2.h>
#ifdef A2CLIENT_PATCH
#include "patch/locale.h"
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
#include "vis_fame.h"
#include "gameplay.h"
#include "vis_playing_area.h"

// 4ae7a3 (client: 4b30e3)
void VisFameDocument::Draw() {
    CPoint top_left = this->rect.TopLeft();
    if (this->visible_flag != 0) {
        LockBackSurface();
        FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
        if (this->bmp_sheet != nullptr) {
            this->bmp_sheet->Draw(top_left.x, top_left.y, 0, 0, 0);
        }
        if (this->bmp_cur_left != nullptr) {
            this->bmp_cur_left->Draw(top_left.x + this->rect_prev.left, top_left.y + this->rect_prev.top, 0, 0, 0);
        }
        if (this->bmp_cur_right != nullptr) {
            this->bmp_cur_right->Draw(top_left.x + this->rect_next.left, top_left.y + this->rect_next.top, 0, 0, 0);
        }
        if (this->bmp_cur_ok != nullptr) {
            this->bmp_cur_ok->Draw(top_left.x + this->rect_ok.left, top_left.y + this->rect_ok.top, 0, 0, 0);
        }
        Fame2& doc = this->fame->m_Documents[this->selected_doc];
        doc.Draw(top_left.x, top_left.y);
        UnlockBackSurface();
        this->VisScreen::Draw();
    }
}


// 4add24 (client: 4b2664)
void VisFameDocument::LoadBitmaps() {
    this->FreeBitmaps();
    this->bmp_sheet = new CBmp64("graphics\\interface\\Docs\\sheet.bmp");
    this->bmp_leftarrow.SetSize(3, -1);
    this->bmp_leftarrow[0] = new CBmp64("graphics\\interface\\Docs\\Arrows\\00_l.bmp");
    this->bmp_leftarrow[1] = new CBmp64("graphics\\interface\\Docs\\Arrows\\01_l.bmp");
    this->bmp_leftarrow[2] = new CBmp64("graphics\\interface\\Docs\\Arrows\\11_l.bmp");
    this->bmp_cur_left = this->bmp_leftarrow[0];
    this->bmp_rightarrow.SetSize(3, -1);
    this->bmp_rightarrow[0] = new CBmp64("graphics\\interface\\Docs\\Arrows\\00_r.bmp");
    this->bmp_rightarrow[1] = new CBmp64("graphics\\interface\\Docs\\Arrows\\01_r.bmp");
    this->bmp_rightarrow[2] = new CBmp64("graphics\\interface\\Docs\\Arrows\\11_r.bmp");
    this->bmp_cur_right = this->bmp_rightarrow[0];
    this->bmp_okbutton.SetSize(4, -1);
    this->bmp_okbutton[0] = new CBmp64("graphics\\interface\\Docs\\OK\\Ok_off.bmp");
    this->bmp_okbutton[1] = new CBmp64("graphics\\interface\\Docs\\OK\\Ok_on.bmp");
    this->bmp_okbutton[2] = new CBmp64("graphics\\interface\\Docs\\OK\\Ok_l_off.bmp");
    this->bmp_okbutton[3] = new CBmp64("graphics\\interface\\Docs\\OK\\Ok_l_on.bmp");
    this->bmp_cur_ok = this->bmp_okbutton[0];
}


// 4ae17a (client: 4b2aba)
void VisFameDocument::FreeBitmaps() {
    if (this->bmp_sheet != nullptr) {
        delete this->bmp_sheet;
    }
    this->bmp_sheet = nullptr;
    for (int32_t i = 0; i < this->bmp_leftarrow.GetSize(); i++) {
        if (this->bmp_leftarrow[i] != nullptr) {
            delete this->bmp_leftarrow[i];
        }
        this->bmp_leftarrow[i] = nullptr;
    }
    this->bmp_leftarrow.RemoveAll();
    this->bmp_cur_left = nullptr;
    for (int32_t i = 0; i < this->bmp_rightarrow.GetSize(); i++) {
        if (this->bmp_rightarrow[i] != nullptr) {
            delete this->bmp_rightarrow[i];
        }
        this->bmp_rightarrow[i] = nullptr;
    }
    this->bmp_rightarrow.RemoveAll();
    this->bmp_cur_right = nullptr;
    for (int32_t i = 0; i < this->bmp_okbutton.GetSize(); i++) {
        if (this->bmp_okbutton[i] != nullptr) {
            delete this->bmp_okbutton[i];
        }
        this->bmp_okbutton[i] = nullptr;
    }
    this->bmp_okbutton.RemoveAll();
    this->bmp_cur_ok = nullptr;
}


// 4ae41b (client: 4b2d5b)
void VisFameDocument::ClearDocs() {
    if (this->fame != nullptr) {
        for (int32_t i = 0; i < this->fame->m_Documents.GetSize(); i++) {
            this->fame->m_Documents[i].Clear();
        }
    }
}


// 4ae3c6 (client: 4b2d06)
void VisFameDocument::LoadDocs() {
    this->ClearDocs();
    for (int32_t i = 0; i < this->fame->m_Documents.GetSize(); i++) {
        this->fame->m_Documents[i].LoadDocument();
    }
}


// 4ae952 (client: 4b3292)
int32_t VisFameDocument::UpdateButtons(CPoint pos, bool is_down) {
    pos -= this->rect.TopLeft();
    if (this->rect_prev.PtInRect(pos)) {
        this->bmp_cur_left = this->bmp_leftarrow[is_down ? 2 : 1];
        return 1;
    }
    if (this->rect_next.PtInRect(pos)) {
        this->bmp_cur_right = this->bmp_rightarrow[is_down ? 2 : 1];
        return 2;
    }
    if (this->rect_ok.PtInRect(pos)) {
        this->bmp_cur_ok = this->bmp_okbutton[is_down ? 3 : 2];
        return 3;
    }
    CBmp64* old_left = this->bmp_cur_left;
    CBmp64* old_right = this->bmp_cur_right;
    CBmp64* old_ok = this->bmp_cur_ok;
    this->bmp_cur_left = this->bmp_leftarrow[0];
    this->bmp_cur_right = this->bmp_rightarrow[0];
    this->bmp_cur_ok = this->bmp_okbutton[0];
    if (old_left != this->bmp_cur_left || old_right != this->bmp_cur_right || old_ok != this->bmp_cur_ok) {
        this->Redraw();
        FlushScreen();
    }
    return -1;
}


// 4aec7a (client: 4b35ba)
void VisFameDocument::UpdateArrowStates() {
    if (this->selected_doc == 0) {
        if (this->fame->m_Documents[0].top_line == 0) {
            this->bmp_cur_left = this->bmp_leftarrow[0];
        }
    }
    int32_t last = this->fame->m_Documents.GetUpperBound();
    if (this->selected_doc == last) {
        Fame2& doc = this->fame->m_Documents[this->selected_doc];
        if (doc.str_arr.GetSize() <= (int32_t)doc.top_line + 0x15) {
            this->bmp_cur_right = this->bmp_rightarrow[0];
        }
    }
}


// 4ae638 (client: 4b2f78)
void VisFameDocument::NextDoc() {
    Fame2& doc = this->fame->m_Documents[this->selected_doc];
    if (!doc.ScrollDown()) {
        this->selected_doc = this->selected_doc + 1;
        int32_t last = this->fame->m_Documents.GetUpperBound();
        if (this->selected_doc >= last) {
            this->selected_doc = last;
        }
    }
    this->UpdateArrowStates();
}


// 4ae6b3 (client: 4b2ff3)
void VisFameDocument::PrevDoc() {
    Fame2& doc = this->fame->m_Documents[this->selected_doc];
    if (doc.ScrollUp() == 0) {
        this->selected_doc = this->selected_doc - 1;
        if (this->selected_doc < 1) {
            this->selected_doc = 0;
        }
    }
    this->UpdateArrowStates();
}


// 4ae57f (client: 4b2ebf)
int32_t VisFameDocument::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_FRAME_TICK) {
        this->Redraw();
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 4ae5be (client: 4b2efe)
int32_t VisFameDocument::OnMouseMove(uint32_t wparam, CPoint pos) {
    int32_t hit = this->UpdateButtons(pos, (wparam & 1) != 0);
    this->UpdateArrowStates();
    if (hit != -1) {
        this->Redraw();
        FlushScreen();
    }
    return 0;
}


// 4ae607 (client: 4b2f47)
int32_t VisFameDocument::OnLButtonDown(uint32_t wparam, CPoint pos) {
    this->UpdateButtons(pos, (wparam & 1) != 0);
    this->UpdateArrowStates();
    return 1;
}


// 4ae717 (client: 4b3057)
int32_t VisFameDocument::OnLButtonUp(uint32_t wparam, CPoint pos) {
    int32_t hit = this->UpdateButtons(pos, (wparam & 1) != 0);
    if (hit == 1) {
        this->PrevDoc();
        this->Redraw();
    } else if (hit == 2) {
        this->NextDoc();
        this->Redraw();
    } else if (hit == 3) {
        this->CloseOk();
    }
    return 1;
}


// 4ae473 (client: 4b2db3)
void VisFameDocument::DoOpen() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->fame = &main_wnd->m_FameHall;
    g_mousept.DisableHint();
    this->LoadBitmaps();
    this->LoadDocs();
    this->selected_doc = 0;
    this->UpdateButtons(CPoint(0, 0), false);
    this->UpdateArrowStates();
    LockBackSurface();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockBackSurface();
    FlushScreen();
    this->visible_flag = 1;
    this->VisScreen::DoOpen();
    g_mousept.EnableHint();
}


// 4ae534 (client: 4b2e74)
void VisFameDocument::DoClose(uint32_t code) {
    this->Redraw();
    this->visible_flag = 0;
    this->FreeBitmaps();
    this->ClearDocs();
    this->fame = nullptr;
    this->VisScreen::DoClose(code);
}


// 4ae791 (client: 4b30d1)
int32_t VisFameDocument::OnKeyDown(uint32_t wparam) {
    (void)wparam;
    return 1;
}


// 4ae926 (client: 4b3266)
void VisFameDocument::DrawBackground(CRect* rect) {
    (void)rect;
}


// 4aeda0 (client: 4b36e0)
const char* VisFameDocument::GetHint() {
    return nullptr;
}


// 4ad961 (client: 4b22a1)
VisFameDocument::VisFameDocument(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) {
    this->OnCreate();
}


// 4ada2a (client: 4b236a) - (scalar deleting dtor ??_G at 4AED50)
VisFameDocument::~VisFameDocument() {
    this->FreeBitmaps();
    this->ClearDocs();
}


// 45d7f9 (client: 461429)
void VisFameHall::Draw() {
    CPoint top_left = this->rect.TopLeft();
    CString str;
    if (this->visible_flag != 0) {
        LockBackSurface();
        this->bmp_bkg->Draw(top_left.x, top_left.y, 0, 0, 0);
        if (this->bmp_cur_close != nullptr) {
            this->bmp_cur_close->Draw(top_left.x + this->close_rect.left, top_left.y + this->close_rect.top, 0, 0, 0);
        }
        for (int32_t i = 0; i < this->name_rects.GetSize(); i++) {
            str.Format("%d.", i + 1);
            g_font4->DrawTextWithShadow(top_left.x + this->rank_rects[i].left, top_left.y + this->rank_rects[i].top, str, CGameFont::ALIGN_LEFT_TOP, palette_brown_derby->GetPalette(0), 1);
            Fame1& entry = this->fame->m_Entries[i];
            g_font4->DrawTextWithShadow(top_left.x + this->name_rects[i].left, top_left.y + this->name_rects[i].top, entry.str, CGameFont::ALIGN_LEFT_TOP, palette_brown_derby->GetPalette(0), 1);
            str.Format("%d", entry.score);
            InsertThousandsSeparators(&str);
            g_font4->DrawTextWithShadow(top_left.x + this->score_rects[i].right, top_left.y + this->score_rects[i].top, str, CGameFont::ALIGN_RIGHT, palette_tawny_port->GetPalette(0), 1);
        }
        UnlockBackSurface();
    }
#ifndef A2CLIENT
    this->VisScreen::Draw();
#endif
}


// 45ce6e (client: 460a9e)
void VisFameHall::OnCreate() {
    this->bmp_bkg = nullptr;
    this->bmp_close_off = nullptr;
    this->bmp_close_on = nullptr;
    this->snd_close = nullptr;
    this->close_rect = CRect(CPoint(0x230, 0x1A0), CSize(0x2C, 0x20));
    this->AddChild(new VisButton(4, 0, 0, 0, 0, "", g_font1, clrsh_TechBlack, 0x445, 0, nullptr));
    this->visible_flag = 0;
}


// 45d5f0 (client: 461220)
int32_t VisFameHall::OnMouseMove(uint32_t wparam, CPoint pos) {
    CPoint top_left = this->rect.TopLeft();
    if (g_mousept.GetCursorSprite() != g_Cursors[CURSOR_SELECT]->GetSprite()) {
        g_Cursors[CURSOR_SELECT]->Use();
    }
    if ((this->close_rect + top_left).PtInRect(pos)) {
        if ((wparam & 1) == 0) {
            this->bmp_cur_close = this->bmp_close_off;
        } else {
            this->bmp_cur_close = this->bmp_close_on;
        }
    } else {
        this->bmp_cur_close = nullptr;
    }
    return 0;
}


// 45d6c3 (client: 4612f3)
int32_t VisFameHall::OnLButtonDown(uint32_t wparam, CPoint pos) {
    CPoint top_left = this->rect.TopLeft();
    if ((this->close_rect + top_left).PtInRect(pos)) {
        CSound::Play((CSound&)this->snd_close);
        this->bmp_cur_close = this->bmp_close_on;
    } else {
        this->bmp_cur_close = nullptr;
    }
    return this->VisScreen::OnLButtonDown(wparam, pos);
}


// 45d773 (client: 4613a3)
int32_t VisFameHall::OnLButtonUp(uint32_t wparam, CPoint pos) {
    CPoint top_left = this->rect.TopLeft();
    if ((this->close_rect + top_left).PtInRect(pos)) {
        this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
    }
    return this->CVisualObject::OnLButtonUp(wparam, pos);
}


// 45d222 (client: 460e52)
void VisFameHall::DoOpen() {
    g_mousept.DisableHint();
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->fame = &main_wnd->m_FameHall;
    this->LoadBitmaps();
    this->LoadSounds();
    this->name_rects.SetSize(this->fame->m_Entries.GetSize(), -1);
    this->rank_rects.SetSize(this->fame->m_Entries.GetSize(), -1);
    this->score_rects.SetSize(this->fame->m_Entries.GetSize(), -1);
    this->UpdateRects();
    this->visible_flag = 1;
    this->VisScreen::DoOpen();
    g_Cursors[CURSOR_SELECT]->Use();
}


// 45d57f (client: 4611af)
int32_t VisFameHall::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_FRAME_TICK) {
        this->CVisualObject::Redraw();
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 45d2d9 (client: 460f09)
void VisFameHall::DoClose(uint32_t code) {
    this->visible_flag = 0;
    this->FreeBitmaps();
    this->FreeSounds();
    this->fame = nullptr;
    this->name_rects.RemoveAll();
    this->VisScreen::DoClose(code);
    g_mousept.EnableHint();
}


// 45d5be (client: 4611ee)
int32_t VisFameHall::OnKeyDown(uint32_t wparam) {
    return this->VisScreen::OnKeyDown(wparam);
}


// 45d5d7 (client: 461207)
int32_t VisFameHall::OnChar(uint32_t wparam) {
    return this->CVisualObject::OnChar(wparam);
}


// 45db50 (client: 4616c0)
const char* VisFameHall::GetHint() {
    return nullptr;
}


// 45da6a (client: 461692)
void VisFameHall::DrawBackground(CRect* rect) {
    (void)rect;
}


// 45cfae (client: 460bde)
void VisFameHall::UpdateRects() {
    for (int32_t i = 0; i < this->name_rects.GetSize(); i++) {
        int32_t font_h = g_font4->GetHeight();
        int32_t y = (int32_t)(font_h * (i * 1.5)) + 0x91;
        this->rank_rects[i] = CRect(CPoint(0x91, y), CSize(0x19, font_h));
        this->name_rects[i] = CRect(CPoint(0xAF, y), CSize(200, font_h));
        this->score_rects[i] = CRect(CPoint(0x181, y), CSize(100, font_h));
    }
}


// 45d32e (client: 460f5e)
void VisFameHall::LoadBitmaps() {
    this->FreeBitmaps();
#ifdef A2CLIENT_PATCH
    this->bmp_bkg = new CBmp64(tangar_locale::gfx_famehall);
#else
    this->bmp_bkg = new CBmp64("main\\graphics\\famehall\\hall.bmp");
#endif
    this->bmp_close_off = new CBmp64("graphics\\interface\\Docs\\OK\\Ok_l_off.bmp");
    this->bmp_close_on = new CBmp64("graphics\\interface\\Docs\\OK\\Ok_l_on.bmp");
    this->bmp_cur_close = nullptr;
}


// 45d44a (client: 46107a)
void VisFameHall::FreeBitmaps() {
    if (this->bmp_bkg != nullptr) {
        delete this->bmp_bkg;
    }
    this->bmp_bkg = nullptr;
    if (this->bmp_close_off != nullptr) {
        delete this->bmp_close_off;
    }
    this->bmp_close_off = nullptr;
    if (this->bmp_close_on != nullptr) {
        delete this->bmp_close_on;
    }
    this->bmp_close_on = nullptr;
    this->bmp_cur_close = nullptr;
}


// 45d542 (client: 461172)
void VisFameHall::LoadSounds() {
    ReloadSfx(&this->snd_close, "SFX\\ChrGen\\Ok.wav");
}


// 45d563 (client: 461193)
void VisFameHall::FreeSounds() {
    FreeSfxSample(&this->snd_close);
}


// 45cd35 (client: 460965)
VisFameHall::VisFameHall(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) {
    this->OnCreate();
}


// 45cde2 (client: 460a12) - body (??_G deleting dtor at 45DA90) - (complete dtor ??1; deleting dtor ??_G at 45DA90)
VisFameHall::~VisFameHall() {
    this->FreeBitmaps();
    this->FreeSounds();
}


// 4abdab (client: 4afdbb)
Fame2::Fame2() {
    this->doc_id = -1;
    this->is_text = 1;
    this->picture = nullptr;
    this->str.Empty();
    this->gap_x34 = 1;
    this->top_line = 0;
}

// 4ac27f (client: 4b028f)
void Fame2::Clear() {
    if (this->picture) {
        delete this->picture;
    }

    this->picture = nullptr;
    this->str.Empty();
    this->str_arr.RemoveAll();
}

// 4abf0f (client: 4aff1f)
void Fame2::SetDocument(int32_t doc_id, uint32_t is_text) {
    this->Clear();
    this->doc_id = doc_id;
    this->is_text = is_text;
}


// 4ad880 (client: 4b0ca0)
int32_t Fame2::GetDocId() {
    return this->doc_id;
}

// 4ad890 (client: 4b0cb0)
uint32_t Fame2::GetIsText() {
    return this->is_text;
}

// 4abd30 (client: 4afd40)
void Fame2::operator=(const Fame2& b) {
    this->doc_id = b.doc_id;
    this->is_text = b.is_text;
    this->picture = b.picture;
    this->str = b.str;
    this->rect = b.rect;
    this->gap_x34 = b.gap_x34;
    this->top_line = b.top_line;
}

// 4abf35 (client: 4aff45)
bool Fame2::ScrollDown() {
    int32_t next = this->top_line + 0x15;
    if (next < this->str_arr.GetSize()) {
        this->top_line = next;
        return true;
    }
    return false;
}

// 4abf72 (client: 4aff82)
int32_t Fame2::ScrollUp() {
    if (this->top_line == 0) {
        return 0;
    }
    this->top_line -= 0x15;
    if ((int32_t)this->top_line < 0) {
        this->top_line = 0;
    }
    return 1;
}

// 4abfb1 (client: 4affc1)
void Fame2::Draw(int32_t x, int32_t y) {
    if (this->is_text == 0) {
        if (this->picture) {
            this->picture->Draw(x + this->rect.left, y + this->rect.top, 0, 0, 0);
        }
        return;
    }

    uint16_t* clr = palette_brown_derby->GetPalette(0);
    CRect r = this->rect + CPoint(x, y);
    g_font4->DrawTextLines(r, this->top_line, this->top_line + 0x15, this->str_arr, clr, 0);
}

// 4ac0af (client: 4b00bf)
void Fame2::LoadDocument() {
    this->Clear();
    this->rect = CRect(CPoint(0x5c, 0x48), CSize(0x1c8, 0x158));

    char buffer[1024];
    if (this->is_text == 0) {
        sprintf(buffer, "graphics\\interface\\Docs\\%d.bmp", this->doc_id);
        this->picture = new CBmp64(buffer);
    } else {
#ifdef A2CLIENT_PATCH
        sprintf(buffer, tangar_locale::docs_fmt, this->doc_id);
#else
        sprintf(buffer, "main\\text\\Docs\\%d.txt", this->doc_id);
#endif
        ReadFileToString(buffer, &this->str);
        this->str_arr.Copy(g_font4->StringArrayForRect(this->rect, this->str));
    }
    this->top_line = 0;
}

// 4ac2e3 (client: 4b02f3)
CFameHall::CFameHall() {
    this->m_nTotalTime = 0;
    this->m_nDifficultyWeight = 0;
    this->m_nMaxEntries = 10;
}

// 4ac352 (client: 4b0362)
CFameHall::~CFameHall() {
}

// 4ac3af (client: 4b03bf)
void CFameHall::ZeroFactors() {
    this->m_nTotalTime = 0;
    this->m_nDifficultyWeight = 0;
}

// 497270 (client: 499fd0)
void CFameHall::SetDifficulty(uint32_t difficulty) {
    this->difficulty = difficulty;
}

// 4ac566 (client: 4b0576)
uint32_t CFameHall::AddTextDocument(int32_t doc_id) {
    for (uint32_t i = 0; i < this->m_Documents.GetSize(); i++) {
        Fame2& fame = this->m_Documents[i];
        if (fame.GetDocId() == doc_id && fame.GetIsText() != 0) {
            return 0;
        }
    }

    Fame2 f;
    f.SetDocument(doc_id, 1);
    this->m_Documents.Add(f);

    return 1;
}


// 4acafa (client: 4b0b0a)
void CFameHall::SeedDefaultEntries() {
    Fame1 fame;
    int32_t tval = 70000;
    for (int i = 0; i < 9; i++) {
        fame.score = tval + GetRandS16(5000);
        fame.str = TxtFile::AllLines[i + 0x107];
        this->AddEntry(fame);
        tval -= 7000;
    }

    fame.score = 0;
    fame.str = TxtFile::AllLines[0x110];
    this->AddEntry(fame);
}



// 4ac839 (client: 4b0849) - save m_Entries to file
void CFameHall::SaveEntries(CFile* file) {
    int32_t count = this->m_Entries.GetSize();
    file->Write(&count, 4);

    for (int32_t i = 0; i < this->m_Entries.GetSize(); i++) {
        Fame1& fame = this->m_Entries[i];

        int32_t len = fame.str.GetLength() + 1;
        file->Write(&len, 4);
        file->Write(fame.str, len);
        file->Write(&fame.score, 4);
        file->Write(&fame.value2, 4);
        file->Write(&fame.value3, 4);
    }
}

// 4ac945 (client: 4b0955)
void CFameHall::LoadEntries(CFile* file) {
    uint32_t num;
    file->Read(&num, 4);
    
    if (num == 0) {
        this->m_Entries.RemoveAll();
    } else {
        this->m_Entries.SetSize(num);
        for (uint32_t i = 0; i < num; i++) {
            uint32_t t;
            file->Read(&t, 4);

            char buf[1024];
            file->Read(buf, t);

            Fame1& fame = this->m_Entries[i];

            fame.str = buf;

            file->Read(&fame.score, 4);
            file->Read(&fame.value2, 4);
            file->Read(&fame.value3, 4);
        }
    }
}

// 4ac706 (client: 4b0716)
void CFameHall::SaveDocuments(CFile* file) {
    int32_t num = this->m_Documents.GetSize();
    file->Write(&num, 4);

    for (uint32_t i = 0; i < num; i++) {
        Fame2& fm = this->m_Documents[i];
        
        //inline 4ac04b
        file->Write(&fm.doc_id, 4);
        file->Write(&fm.is_text, 4);
    }
    file->Write(&this->m_nTotalTime, 4);
    file->Write(&this->m_nDifficultyWeight, 4);
}

// 4ac7a2 (client: 4b07b2)
void CFameHall::LoadDocuments(CFile* file) {
    int32_t cnt;
    file->Read(&cnt, 4);
    this->m_Documents.SetSize(cnt);

    for (int i = 0; i < cnt; i++) {
        Fame2& fame = this->m_Documents[i];
        file->Read(&fame.doc_id, 4);
        file->Read(&fame.is_text, 4);
    }

    file->Read(&this->m_nTotalTime, 4);
    file->Read(&this->m_nDifficultyWeight, 4);
}


// 4ac3ce (client: 4b03de)
void CFameHall::AddEntry(const Fame1& fame) {
    if (this->m_Entries.GetSize() == 0) {
        this->m_Entries.Add(fame);
    } else {
        int32_t i = 0;
        for (i = 0; i < this->m_Entries.GetSize(); i++) {
            Fame1& f = this->m_Entries[i];
            if (f.score <= fame.score) {
                this->m_Entries.InsertAt(i, fame);
                break;
            }
        }
        if (i == this->m_Entries.GetSize()) {
            this->m_Entries.Add(fame);
        }
    }

    if (this->m_Entries.GetSize() > this->m_nMaxEntries) {
        this->m_Entries.SetSize(this->m_nMaxEntries);
    }
}

// 420050 (client: 420c80)
void CFameHall::AddDifficultyPoint() {
    this->m_nDifficultyWeight++;
}

// 4ac498 (client: 4b04a8)
void CFameHall::SubmitScore() {
    Fame1 fm;
    MainWindow *mwnd = (MainWindow*)AfxGetMainWnd();
    CUnit* unit = mwnd->vis_map_context->GetMyUnit();
    if (unit) {
        fm.str = unit->str1;
        if (this->m_nTotalTime == 0) {
            fm.score = unit->exp_summary / 500000.0 * this->m_nDifficultyWeight;
        } else {
            fm.score = unit->exp_summary / (this->m_nTotalTime * 10.0) * this->m_nDifficultyWeight;
        }

        this->AddEntry(fm);
    }
}



