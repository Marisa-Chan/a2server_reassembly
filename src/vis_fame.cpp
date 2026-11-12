#include <winsock2.h>
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
#include "vis_fame.h"
#include "gameplay.h"
#include "vis_playing_area.h"

// 4AE7A3
void VisFameDocument::VMethod7() {
    CPoint top_left = this->rect.TopLeft();
    if (this->visible_flag != 0) {
        LockSurface2();
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
        UnlockSurface2();
        this->VisScreen::VMethod7();
    }
}


// 4ADD24
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


// 4AE17A
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


// 4AE41B
void VisFameDocument::ClearDocs() {
    if (this->fame != nullptr) {
        for (int32_t i = 0; i < this->fame->m_Documents.GetSize(); i++) {
            this->fame->m_Documents[i].Clear();
        }
    }
}


// 4AE3C6
void VisFameDocument::LoadDocs() {
    this->ClearDocs();
    for (int32_t i = 0; i < this->fame->m_Documents.GetSize(); i++) {
        this->fame->m_Documents[i].LoadDocument();
    }
}


// 4AE952
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
        this->VMethod9();
        FlushScreen();
    }
    return -1;
}


// 4AEC7A
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


// 4AE638
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


// 4AE6B3
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


// 4AE57F
int32_t VisFameDocument::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_FRAME_TICK) {
        this->VMethod9();
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 4AE5BE
int32_t VisFameDocument::OnMouseMove(uint32_t wparam, CPoint pos) {
    int32_t hit = this->UpdateButtons(pos, (wparam & 1) != 0);
    this->UpdateArrowStates();
    if (hit != -1) {
        this->VMethod9();
        FlushScreen();
    }
    return 0;
}


// 4AE607
int32_t VisFameDocument::OnLButtonDown(uint32_t wparam, CPoint pos) {
    this->UpdateButtons(pos, (wparam & 1) != 0);
    this->UpdateArrowStates();
    return 1;
}


// 4AE717
int32_t VisFameDocument::OnLButtonUp(uint32_t wparam, CPoint pos) {
    int32_t hit = this->UpdateButtons(pos, (wparam & 1) != 0);
    if (hit == 1) {
        this->PrevDoc();
        this->VMethod9();
    } else if (hit == 2) {
        this->NextDoc();
        this->VMethod9();
    } else if (hit == 3) {
        this->CloseOk();
    }
    return 1;
}


// 4AE473
void VisFameDocument::VMethod28() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->fame = &main_wnd->m_FameHall;
    g_mousept.DisableHint();
    this->LoadBitmaps();
    this->LoadDocs();
    this->selected_doc = 0;
    this->UpdateButtons(CPoint(0, 0), false);
    this->UpdateArrowStates();
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();
    this->visible_flag = 1;
    this->VisScreen::VMethod28();
    g_mousept.EnableHint();
}


// 4AE534
void VisFameDocument::DoClose(uint32_t code) {
    this->VMethod9();
    this->visible_flag = 0;
    this->FreeBitmaps();
    this->ClearDocs();
    this->fame = nullptr;
    this->VisScreen::DoClose(code);
}


// 4AE791
int32_t VisFameDocument::OnKeyDown(uint32_t wparam) {
    (void)wparam;
    return 1;
}


// 4AE926
void VisFameDocument::VMethod8(CRect* rect) {
    (void)rect;
}


// 4AEDA0
const char* VisFameDocument::GetHint() {
    return nullptr;
}


// 4AD961
VisFameDocument::VisFameDocument(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) {
    this->VMethod26();
}


// 4ADA2A (scalar deleting dtor ??_G at 4AED50)
VisFameDocument::~VisFameDocument() {
    this->FreeBitmaps();
    this->ClearDocs();
}


// 45D7F9
// client: 461429
void VisFameHall::VMethod7() {
    CPoint top_left = this->rect.TopLeft();
    CString str;
    if (this->visible_flag != 0) {
        LockSurface2();
        this->bmp_bkg->Draw(top_left.x, top_left.y, 0, 0, 0);
        if (this->bmp_cur_close != nullptr) {
            this->bmp_cur_close->Draw(top_left.x + this->close_rect.left, top_left.y + this->close_rect.top, 0, 0, 0);
        }
        for (int32_t i = 0; i < this->name_rects.GetSize(); i++) {
            str.Format("%d.", i + 1);
            g_font4->DrawTextWithShadow(top_left.x + this->rank_rects[i].left, top_left.y + this->rank_rects[i].top, str, 0, palette_brown_derby->GetPalette(0), 1);
            Fame1& entry = this->fame->m_Entries[i];
            g_font4->DrawTextWithShadow(top_left.x + this->name_rects[i].left, top_left.y + this->name_rects[i].top, entry.str, 0, palette_brown_derby->GetPalette(0), 1);
            str.Format("%d", entry.score);
            InsertThousandsSeparators(&str);
            g_font4->DrawTextWithShadow(top_left.x + this->score_rects[i].right, top_left.y + this->score_rects[i].top, str, 1, palette_tawny_port->GetPalette(0), 1);
        }
        UnlockSurface2();
    }
#ifndef A2CLIENT
    this->VisScreen::VMethod7();
#endif
}


// 45CE6E
void VisFameHall::VMethod26() {
    this->bmp_bkg = nullptr;
    this->bmp_close_off = nullptr;
    this->bmp_close_on = nullptr;
    this->snd_close = nullptr;
    this->close_rect = CRect(CPoint(0x230, 0x1A0), CSize(0x2C, 0x20));
    this->AddChild(new VisButton(4, 0, 0, 0, 0, "", g_font1, clrsh_TechBlack, 0x445, 0, nullptr));
    this->visible_flag = 0;
}


// 45D5F0
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


// 45D6C3
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


// 45D773
int32_t VisFameHall::OnLButtonUp(uint32_t wparam, CPoint pos) {
    CPoint top_left = this->rect.TopLeft();
    if ((this->close_rect + top_left).PtInRect(pos)) {
        this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
    }
    return this->CVisualObject::OnLButtonUp(wparam, pos);
}


// 45D222
void VisFameHall::VMethod28() {
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
    this->VisScreen::VMethod28();
    g_Cursors[CURSOR_SELECT]->Use();
}


// 45D57F
int32_t VisFameHall::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_FRAME_TICK) {
        this->CVisualObject::VMethod9();
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 45D2D9
void VisFameHall::DoClose(uint32_t code) {
    this->visible_flag = 0;
    this->FreeBitmaps();
    this->FreeSounds();
    this->fame = nullptr;
    this->name_rects.RemoveAll();
    this->VisScreen::DoClose(code);
    g_mousept.EnableHint();
}


// 45D5BE
int32_t VisFameHall::OnKeyDown(uint32_t wparam) {
    return this->VisScreen::OnKeyDown(wparam);
}


// 45D5D7
int32_t VisFameHall::OnChar(uint32_t wparam) {
    return this->CVisualObject::OnChar(wparam);
}


// 45DB50
const char* VisFameHall::GetHint() {
    return nullptr;
}


// 45DA6A
void VisFameHall::VMethod8(CRect* rect) {
    (void)rect;
}


// 45CFAE
void VisFameHall::UpdateRects() {
    for (int32_t i = 0; i < this->name_rects.GetSize(); i++) {
        int32_t font_h = g_font4->GetHeight();
        int32_t y = (int32_t)(font_h * (i * 1.5)) + 0x91;
        this->rank_rects[i] = CRect(CPoint(0x91, y), CSize(0x19, font_h));
        this->name_rects[i] = CRect(CPoint(0xAF, y), CSize(200, font_h));
        this->score_rects[i] = CRect(CPoint(0x181, y), CSize(100, font_h));
    }
}


// 45D32E
void VisFameHall::LoadBitmaps() {
    this->FreeBitmaps();
    this->bmp_bkg = new CBmp64(A2CLIENT_PATCH_ENABLED ? tangar_locale::gfx_famehall : "main\\graphics\\famehall\\hall.bmp");
    this->bmp_close_off = new CBmp64("graphics\\interface\\Docs\\OK\\Ok_l_off.bmp");
    this->bmp_close_on = new CBmp64("graphics\\interface\\Docs\\OK\\Ok_l_on.bmp");
    this->bmp_cur_close = nullptr;
}


// 45D44A
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


// 45D542
void VisFameHall::LoadSounds() {
    ReloadSfx(&this->snd_close, "SFX\\ChrGen\\Ok.wav");
}


// 45D563
void VisFameHall::FreeSounds() {
    FreeSfxSample(&this->snd_close);
}


// 45CD35
VisFameHall::VisFameHall(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) {
    this->VMethod26();
}


// 45CDE2 (complete dtor ??1; deleting dtor ??_G at 45DA90)
VisFameHall::~VisFameHall() {
    this->FreeBitmaps();
    this->FreeSounds();
}


Fame2::Fame2() {
    //4abdab
    this->doc_id = -1;
    this->is_text = 1;
    this->picture = nullptr;
    this->str.Empty();
    this->gap_x34 = 1;
    this->top_line = 0;
}

void Fame2::Clear() {
    //4ac27f
    if (this->picture) {
        delete this->picture;
    }

    this->picture = nullptr;
    this->str.Empty();
    this->str_arr.RemoveAll();
}

void Fame2::SetDocument(int32_t doc_id, uint32_t is_text) {
    //4abf0f
    this->Clear();
    this->doc_id = doc_id;
    this->is_text = is_text;
}


int32_t Fame2::GetDocId() {
    //4ad880
    return this->doc_id;
}

uint32_t Fame2::GetIsText() {
    //4ad890
    return this->is_text;
}

void Fame2::operator=(const Fame2& b) {
    //4abd30
    this->doc_id = b.doc_id;
    this->is_text = b.is_text;
    this->picture = b.picture;
    this->str = b.str;
    this->rect = b.rect;
    this->gap_x34 = b.gap_x34;
    this->top_line = b.top_line;
}

bool Fame2::ScrollDown() { //4abf35
    int32_t next = this->top_line + 0x15;
    if (next < this->str_arr.GetSize()) {
        this->top_line = next;
        return true;
    }
    return false;
}

int32_t Fame2::ScrollUp() { //4abf72
    if (this->top_line == 0) {
        return 0;
    }
    this->top_line -= 0x15;
    if ((int32_t)this->top_line < 0) {
        this->top_line = 0;
    }
    return 1;
}

void Fame2::Draw(int32_t x, int32_t y) { //4abfb1
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

void Fame2::LoadDocument() { //4ac0af
    this->Clear();
    this->rect = CRect(CPoint(0x5c, 0x48), CSize(0x1c8, 0x158));

    char buffer[1024];
    if (this->is_text == 0) {
        sprintf(buffer, "graphics\\interface\\Docs\\%d.bmp", this->doc_id);
        this->picture = new CBmp64(buffer);
    } else {
        sprintf(buffer, "main\\text\\Docs\\%d.txt", this->doc_id);
        ReadFileToString(buffer, &this->str);
        this->str_arr.Copy(g_font4->StringArrayForRect(this->rect, this->str));
    }
    this->top_line = 0;
}

CFameHall::CFameHall() {
    //4ac2e3
    this->m_nTotalTime = 0;
    this->m_nDifficultyWeight = 0;
    this->m_nMaxEntries = 10;
}

CFameHall::~CFameHall() {
    //4ac352
}

void CFameHall::ZeroFactors() {
    //4ac3af
    this->m_nTotalTime = 0;
    this->m_nDifficultyWeight = 0;
}

void CFameHall::SetDifficulty(uint32_t difficulty) {
    //497270
    this->difficulty = difficulty;
}

uint32_t CFameHall::AddTextDocument(int32_t doc_id) {
    //4ac566
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


void CFameHall::SeedDefaultEntries() {
    //4acafa
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



// 4ac839
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

void CFameHall::LoadEntries(CFile* file) {
    //4ac945
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

void CFameHall::SaveDocuments(CFile* file) { //  4ac706
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

void CFameHall::LoadDocuments(CFile* file) { // 4ac7a2
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


void CFameHall::AddEntry(const Fame1& fame) {
    //4ac3ce
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

void CFameHall::AddDifficultyPoint() {
    //420050
    this->m_nDifficultyWeight++;
}

void CFameHall::SubmitScore() { //4ac498
    Fame1 fm;
    MainWindow *mwnd = (MainWindow*)AfxGetMainWnd();
    CUnit* unit = mwnd->vis_map_context->GetUnit_3f6c();
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



