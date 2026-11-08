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



char g_HintBuf[223]; //4c9a6f GetHint static buffer

char g_EmptyStrSrvScreenText[4]; //659a48 zeroed buffer used to clear the server screen text box
char g_EmptyStrMapTooltip[4]; //659a30 "" placeholder tooltip for net map entries with an unprintable tooltip

// Still referenced from ASM: ostream::operator<< passes it as the "" pad string and a
// RegQueryValueExA caller passes it as the "" value name, so it keeps C linkage.
extern "C" {
    char g_EmptyStrNetChatText[4]; //659a5c zeroed buffer used to clear the net chat text box
}





CVisualObject::CVisualObject() {
	//4d6d40
    this->parent = nullptr;
    this->cursor_over_obj = nullptr;
    this->focus_obj = nullptr;
    this->cursor_over_obj_last = nullptr;
    this->last_focus_obj = nullptr;
    this->flags = 1;
    this->id = -1;
    this->hint = "";
    this->rect = CRect(0, 0, 0, 0);
    this->down_obj = nullptr;
    this->up_obj = nullptr;
    this->left_obj = nullptr;
    this->right_obj = nullptr;
    this->caption_obj = nullptr;
}

CVisualObject::CVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str) {
    //4d6f5f
    this->parent = nullptr;
    this->cursor_over_obj = nullptr;
    this->focus_obj = nullptr;
    this->cursor_over_obj_last = nullptr;
    this->last_focus_obj = nullptr;
    this->flags = 1;
    this->id = _id;

    if (str) {
        this->hint = str;
    }

    this->rect = CRect(l, t, r, b);
    this->down_obj = nullptr;
    this->up_obj = nullptr;
    this->left_obj = nullptr;
    this->right_obj = nullptr;
    this->caption_obj = nullptr;
}

CVisualObject::CVisualObject(int32_t _id, const RECT& r, const char* str) {
    //4d6e50
    this->parent = nullptr;
    this->cursor_over_obj = nullptr;
    this->focus_obj = nullptr;
    this->cursor_over_obj_last = nullptr;
    this->last_focus_obj = nullptr;
    this->flags = 1;
    this->id = _id;

    if (str) {
        this->hint = str;
    }

    this->rect = r;

    this->down_obj = nullptr;
    this->up_obj = nullptr;
    this->left_obj = nullptr;
    this->right_obj = nullptr;
    this->caption_obj = nullptr;
}


CVisualObject::~CVisualObject() {
    //4d707a
    for (int32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        if (obj) {
            delete obj;
        }
    }
}


void CVisualObject::Dump(CDumpContext& dc) const {
    //4d82b3
    
    //dc << "CVisualObject";
}


const char* CVisualObject::GetHint() {
    //4d7c15

    if (this->TestFlags(FLAG_20)) {
        return nullptr;
    }

    return this->hint;
}

void CVisualObject::SetHint(const char* _name) {
    //4d7bf9

    this->hint = _name;
}


void CVisualObject::ChangeFlags(uint32_t _flags, bool setunset) {
    //4d7a03

    if (setunset) {
        this->flags |= _flags;
    } else {
        this->flags &= ~_flags;
    }
}

// 4386e0
uint32_t IsDefaultNpcName(const char* name) {
    if (strcmp(txt_npcnames.GetLine(0x17), name) == 0) {
        return 1;
    }
    if (strcmp(txt_npcnames.GetLine(0x18), name) == 0) {
        return 1;
    }
    if (strcmp(txt_npcnames.GetLine(0x19), name) == 0) {
        return 1;
    }
    if (strcmp(txt_npcnames.GetLine(0x1A), name) == 0) {
        return 1;
    }
    if (strcmp("Unnamed", name) == 0) {
        return 1;
    }
    return 0;
}


uint32_t CVisualObject::TestFlags(uint32_t _flags) {
    //4d79ed

    return this->flags & _flags;
}


void CVisualObject::SetCursorOver(bool isOver) {
    //4d7a3a

    this->ChangeFlags(FLAG_OVERCURSOR, isOver);

    if (this->parent) {
        if (isOver) {
            this->parent->cursor_over_obj_last = this->parent->cursor_over_obj;
            this->parent->cursor_over_obj = this;
        } else if (this->parent->cursor_over_obj == this) {
            this->parent->cursor_over_obj = this->parent->cursor_over_obj_last;
        }
    }
}


void CVisualObject::SetFocus(bool isFocus) {
    //4d7aa9

    this->ChangeFlags(FLAG_FOCUS, isFocus);

    if (this->parent) {
        if (isFocus) {
            this->parent->last_focus_obj = this->parent->focus_obj;
            this->parent->focus_obj = this;
            if (this->caption_obj) {
                this->caption_obj->SetActiveColor(true);
            }
        } else {
            if (this->parent->focus_obj == this) {
                this->parent->focus_obj = this->parent->last_focus_obj;
            }

            if (this->caption_obj) {
                this->caption_obj->SetActiveColor(false);
            }
        }
    }
}


void CVisualObject::VMethod7() {
    //4d7c40

    if (this->TestFlags(FLAG_20)) {
        return;
    }

    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        if (!obj->TestFlags(FLAG_20)) {
            obj->VMethod7();
        }
    }
}


void CVisualObject::VMethod8(CRect* rect) {
    //41ed60
}


void CVisualObject::VMethod9() {
    //4d7cc8
    if (this->TestFlags(FLAG_20)) {
        return;
    }

    if (g_IsServer != 0) {
        return;
    }

    this->VMethod7();
    this->VMethod10();
}


void CVisualObject::VMethod10() {
    //4d7d03
    CRect r = this->ClientRectToScreen(this->rect);
    gfxFlushRect(r);
}


void CVisualObject::WriteData(void* buf) {
    //4d7d99

    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        obj->WriteData(buf);
        buf = (uint8_t*)buf + obj->DataSize();
    }
}


uint32_t CVisualObject::DataSize() {
    //4d7d37

    uint32_t sz = 0;
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        sz += this->childs[i]->DataSize();
    }
    return sz;
}


void CVisualObject::ReadData(const void* buf) {
    //4d7e02

    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        obj->ReadData(buf);
        buf = (const uint8_t*)buf + obj->DataSize();
    }
}


int32_t CVisualObject::OnMouseMove(uint32_t wparam, CPoint pos) {
    //41ed70
    return 0;
}

int32_t CVisualObject::OnWmUser(uint32_t wparam, CPoint pos) {
    //41ed80
    return 0;
}

int32_t CVisualObject::OnLButtonDown(uint32_t wparam, CPoint pos) {
    //438850
    return 0;
}

int32_t CVisualObject::OnLButtonUp(uint32_t wparam, CPoint pos) {
    //438860
    return 0;
}

int32_t CVisualObject::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    //438870
    return 0;
}

int32_t CVisualObject::OnRButtonDown(uint32_t wparam, CPoint pos) {
    //438880
    return 0;
}

int32_t CVisualObject::OnRButtonUp(uint32_t wparam, CPoint pos) {
    //438890
    return 0;
}

int32_t CVisualObject::OnRButtonDblClk(uint32_t wparam, CPoint pos) {
    //41ed90
    return 0;
}

int32_t CVisualObject::OnKeyDown(uint32_t wparam) {
    //4388a0
    return 0;
}

int32_t CVisualObject::OnKeyUp(uint32_t wparam) {
    //41eda0
    return 0;
}

int32_t CVisualObject::OnChar(uint32_t wparam) {
    //4388b0
    return 0;
}


int32_t CVisualObject::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    //4d7fc9

    int32_t res = 0;
    if (msg >= WM_KEYDOWN && msg <= WM_CHAR) {
        if (this->focus_obj) {
            res = this->focus_obj->MsgProc(msg, wparam, lparam);
        }
        
        if (!res) {
            res = this->MsgProcOnChilds(msg, wparam, lparam);
        }
    } else if ((msg >= WM_MOUSEMOVE && msg <= WM_RBUTTONDBLCLK) || msg == WM_USER) {
        if (this->cursor_over_obj) {
            res = this->cursor_over_obj->MsgProc(msg, wparam, lparam);
        }
            
        if (!res) {
            res = this->MsgProcOnChilds(msg, wparam, lparam);
        }
    } else {
        res = this->MsgProcOnChilds(msg, wparam, lparam);
    }

    if (!res) {
        CPoint mouse(lparam & 0xffff, lparam >> 16);

        if (msg == WM_KEYDOWN) {
            res = this->OnKeyDown(wparam);
        } else if (msg == WM_KEYUP) {
            res = this->OnKeyUp(wparam);
        } else if (msg == WM_CHAR) {
            res = this->OnChar(wparam);
        } else if (msg == WM_MOUSEMOVE) {
            res = this->OnMouseMove(wparam, mouse);
        } else if (msg == WM_LBUTTONDOWN) {
            res = this->OnLButtonDown(wparam, mouse);
        } else if (msg == WM_LBUTTONUP) {
            res = this->OnLButtonUp(wparam, mouse);
        } else if (msg == WM_LBUTTONDBLCLK) {
            res = this->OnLButtonDblClk(wparam, mouse);
        } else if (msg == WM_RBUTTONDOWN) {
            res = this->OnRButtonDown(wparam, mouse);
        } else if (msg == WM_RBUTTONUP) {
            res = this->OnRButtonUp(wparam, mouse);
        } else if (msg == WM_RBUTTONDBLCLK) {
            res = this->OnRButtonDblClk(wparam, mouse);
        } else if (msg == WM_USER) {
            res = this->OnWmUser(wparam, mouse);
        }
    }

    return res;
}


int32_t CVisualObject::MsgProcOnChilds(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    //4d7e6b
    CPoint mouse(lparam & 0xffff, lparam >> 16);
    
    bool isMouseEvent = false;
    if ((msg >= WM_MOUSEMOVE && msg <= WM_RBUTTONDBLCLK) || msg == WM_USER) {
        isMouseEvent = true;
    }

    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];

        CRect r = obj->ClientRectToScreen(obj->rect);

        if (!isMouseEvent || r.PtInRect(mouse)) {
            int32_t res = obj->MsgProc(msg, wparam, lparam);
            if (res) {
                return res;
            }

            if (isMouseEvent) { // is mouse event, mouse in rect and res == 0 then break and return 0
                return 0;
            }
        }
    }
    return 0;
}

void CVisualObject::ClientPtToScreen(CPoint* out, const CPoint& _point) {
    //4d71f3
    *out = _point;
    for (CVisualObject* pobj = this->parent; pobj; pobj = pobj->parent) {
        *out += pobj->rect.TopLeft();
    }
}

void CVisualObject::ClientRectToScreen(CRect* out, const CRect& _rect) {
    //4d7283
    this->ClientPtToScreen(&(out->TopLeft()), _rect.TopLeft());
    this->ClientPtToScreen(&(out->BottomRight()), _rect.BottomRight());
}

CPoint CVisualObject::ClientPtToScreen(const CPoint& _point) {
    //4d71f3
    CPoint out = _point;
    for (CVisualObject* pobj = this->parent; pobj; pobj = pobj->parent) {
        out += pobj->rect.TopLeft();
    }
    return out;
}

CRect CVisualObject::ClientRectToScreen(const CRect& _rect) {
    //4d7283
    CRect out;
    out.TopLeft() = this->ClientPtToScreen(_rect.TopLeft());
    out.BottomRight() = this->ClientPtToScreen(_rect.BottomRight());
    return out;
}


void CVisualObject::AddChild(CVisualObject* obj) {
    //4d72c4
    this->childs.Add(obj);
    obj->parent = this;
}

void CVisualObject::RemoveChild(CVisualObject* obj) {
    //4d72e9
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        if (this->childs[i] == obj) {
            this->childs.RemoveAt(i);

            obj->parent = nullptr;

            if (this->cursor_over_obj == obj) {
                this->cursor_over_obj = nullptr;
            }

            if (this->focus_obj == obj) {
                this->focus_obj = this->last_focus_obj;
            }
            
            if (this->last_focus_obj == obj) {
                this->last_focus_obj = nullptr;
                this->focus_obj = nullptr;
            }

            break;
        }
    }
}

void CVisualObject::RemoveChildById(int32_t _id) {
    //4d7393
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        if (obj->id == _id) {
            this->childs.RemoveAt(i);

            obj->parent = nullptr;

            if (this->cursor_over_obj == obj) {
                this->cursor_over_obj = nullptr;

            /*if (focus_obj == obj)
                focus_obj = last_focus_obj;

            if (last_focus_obj == obj)
                last_focus_obj = nullptr;
            */
            }
            break;
        }
    }
}

void CVisualObject::RemoveAllChilds() {
    //4d78d4
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        this->childs[i]->parent = nullptr;
    }
    this->childs.RemoveAll();

    this->cursor_over_obj = nullptr;
    this->focus_obj = nullptr;
    this->cursor_over_obj_last = nullptr;
    this->last_focus_obj = nullptr;
}

void CVisualObject::DestroyChild(CVisualObject* obj) {
    //4d7430
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        if (this->childs[i] == obj) {
            this->childs.RemoveAt(i);

            if (this->cursor_over_obj == obj) {
                this->cursor_over_obj = nullptr;
            
            /*if (focus_obj == obj)
                focus_obj = last_focus_obj;

            if (last_focus_obj == obj)
                last_focus_obj = nullptr;*/

            }
            delete obj;
            break;
        }
    }
}

void CVisualObject::DestroyChildById(int32_t _id) {
    //4d74cf
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        if (obj->id == _id) {
            this->childs.RemoveAt(i);

            if (this->cursor_over_obj == obj) {
                this->cursor_over_obj = nullptr;

            /*if (focus_obj == obj)
                focus_obj = last_focus_obj;

            if (last_focus_obj == obj)
                last_focus_obj = nullptr;
            */

            }
            delete obj;
            break;
        }
    }
}

void CVisualObject::DestroyAllChilds() {
    //4d7950
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        delete this->childs[i];
    }
    this->childs.RemoveAll();

    this->cursor_over_obj = nullptr;
    this->focus_obj = nullptr;
    this->cursor_over_obj_last = nullptr;
    this->last_focus_obj = nullptr;
}

CVisualObject* CVisualObject::FindChild(int32_t _id) {
    //4d7873
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        if (obj->id == _id) {
            return obj;
        }
    }
    return nullptr;
}


CVisualObject* CVisualObject::GetChildAt(POINT pt) {
    //4d7b48
    for (int32_t i = this->childs.GetSize() - 1; i >= 0; i--) {
        CVisualObject* ret = this->childs[i]->GetChildAt(pt);
        if (ret) {
            return ret;
        }
    }

    CRect r = this->ClientRectToScreen(this->rect);
    if (r.PtInRect(pt)) {
        return this;
    }
    return nullptr;
}

void CVisualObject::SetLeftObj(CVisualObject* obj) {
    //4d82cd
    this->left_obj = obj;
    obj->right_obj = this;
}

void CVisualObject::SetRightObj(CVisualObject* obj) {
    //4d82ec
    this->right_obj = obj;
    obj->left_obj = this;
}

void CVisualObject::SetUpObj(CVisualObject* obj) {
    //4d830b
    this->up_obj = obj;
    obj->down_obj = this;
}

void CVisualObject::SetDownObj(CVisualObject* obj) {
    //4d832a
    this->down_obj = obj;
    obj->up_obj = this;
}

void CVisualObject::SetCaptionLabel(VisLabel* obj) {
    //4d8349
    this->caption_obj = obj;
}

void CVisualObject::FocusTo(CVisualObject* obj, bool update) {
    //4d7816
    if (this->focus_obj) {
        CVisualObject* oldo = this->focus_obj;
        oldo->SetFocus(false);
        if (update) {
            oldo->VMethod9();
        }
    }

    obj->SetFocus(true);
    if (update) {
        obj->VMethod9();
    }
}

void CVisualObject::TabFocus(bool forward, bool update) {
    //4d7674
    int inc = 1;
    if (forward) {
        inc = 1;
    } else {
        inc = -1;
    }

    int32_t indx = -1;
    if (this->focus_obj) {
        for (int32_t i = 0; i < this->childs.GetSize(); i++) {
            if (this->childs[i] == this->focus_obj) {
                indx = i;
                break;
            }
        }
    }

    if (!this->childs.GetSize()) {
        return;
    }

    int32_t next_indx = indx;
    int32_t first_check_id = -1;
    bool idIsSet = false;
    if (next_indx < 0) {
        inc = 1;
    }

    while (true) {
        next_indx += inc;

        if (next_indx >= this->childs.GetSize()) {
            next_indx = 0;
        } else if (next_indx < 0) {
            next_indx = this->childs.GetSize() - 1;
        }

        if (idIsSet) {
            if (next_indx == first_check_id) {
                return; //full loop
            }
        } else {
            first_check_id = next_indx;
            idIsSet = true;
        }

        CVisualObject* obj = this->childs[next_indx];
        if (obj->TestFlags(FLAG_ENABLED | FLAG_NOTFOCUS) == (FLAG_ENABLED | FLAG_NOTFOCUS)) {
            if (this->focus_obj) {
                CVisualObject* old_focus = this->focus_obj;
                this->focus_obj->SetFocus(false);
                if (update) {
                    old_focus->VMethod9();
                }
            }
            obj->SetFocus(true);
            if (update) {
                obj->VMethod9();
            }
            break;
        }
    }
}

void CVisualObject::SetRect(RECT r) {
    //4d7178
    this->rect = r;
}

void CVisualObject::SetRect(const RECT* r) {
    //4d7140
    this->rect = *r;
}

void CVisualObject::SetRect(int32_t l, int32_t t, int32_t r, int32_t b) {
    this->rect.left = l;
    this->rect.top = t;
    this->rect.right = r;
    this->rect.bottom = b;
}






VisLabel::VisLabel() {
    //4d835f
}

VisLabel::VisLabel(int32_t _id, const RECT& r, const char* _text, CGameFont* _font, uint16_t* colorsh, uint32_t align)
    : CVisualObject(_id, r, nullptr) {
    //4d8453

    this->text = _text;
    this->font = _font;
    this->color_sh = colorsh;
    this->align_flags = align;
}

VisLabel::VisLabel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _text, CGameFont* _font, uint16_t* colorsh, uint32_t align)
    : CVisualObject(_id, l, t, r, b, nullptr) {
    //4d83b6

    this->text = _text;
    this->font = _font;
    this->color_sh = colorsh;
    this->align_flags = align;
}

VisLabel::~VisLabel() {
    //4e3ba0
}


void VisLabel::VMethod7() {
    //4d84e4
    if (this->parent) {
        CRect res = this->ClientRectToScreen(this->rect);

        LockSurface2();

        this->parent->VMethod8(&res);

        int32_t outx = 0;
        int32_t outy = 0;

        if (this->align_flags & 1) {
            outx = res.right;
        } else if (this->align_flags & 2) {
            outx = (res.left + res.right) / 2;
        } else {
            outx = res.left;
        }

        if (this->align_flags & 4) {
            outy = res.bottom;
        } else if (this->align_flags & 8) {
            outy = (res.top + res.bottom) / 2;
        } else {
            outy = res.top;
        }

        this->font->DrawTextWithShadow(outx, outy, this->text, this->align_flags, this->color_sh, 1);

        UnlockSurface2();
    }
}

void VisLabel::SetActiveColor(bool isActive) {
    //4d85e6
    if (isActive) {
        this->color_sh = p_clrsh_Gold;
    } else {
        this->color_sh = p_clrsh_Black;
    }
    this->VMethod9();
}


VisButton::~VisButton() {
    //450a40
}

void VisButton::VMethod7() {
    //4d8f7e
    if (!this->parent) {
        return;
    }

    CRect r2 = this->ClientRectToScreen(this->rect);
    
    LockSurface2();
    this->parent->VMethod8(&r2);

    r2.right -= 1;
    r2.bottom -= 1;

    POINT pp;
    GetCursorPos(&pp);

    uint32_t clr1;
    uint32_t clr2;
    uint32_t dd;
    if (r2.PtInRect(pp) == 0 || this->downed == 0) {
        clr1 = GetColorRGB(0x29, 0x45, 0x3f);
        clr2 = GetColorRGB(7, 12, 9);
        dd = 2;
    } else {
        clr1 = GetColorRGB(7, 12, 9);
        clr2 = GetColorRGB(0x29, 0x45, 0x3f);
        dd = 4;
    }

    if (!this->clr) {
        if ((this->mouse_on == 0 && this->TestFlags(FLAG_FOCUS) == 0) || this->TestFlags(FLAG_ENABLED) == 0) {
            this->font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, this->caption, 8 | 2, p_clrsh_Black, dd);
        } else {
            this->font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, this->caption, 8 | 2, p_clrsh_Gold, dd);
        }
    } else if (this->mouse_on == 0) {
        this->font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, this->caption, 8 | 2, clrsh_DullGold, dd);
    } else {
        this->font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, this->caption, 8 | 2, clrsh_CharlieBrown, dd);
    }

    FillRectColor(r2.right, r2.top + 2, r2.right, r2.bottom - 2, clr2);
    FillRectColor(r2.right - 1, r2.top + 1, r2.right - 1, r2.bottom - 1, clr2);
    FillRectColor(r2.left + 2, r2.bottom, r2.right - 2, r2.bottom, clr2);
    FillRectColor(r2.left + 1, r2.bottom - 1, r2.right - 1, r2.bottom - 1, clr2);
    FillRectColor(r2.left + 2, r2.top, r2.right - 2, r2.top, clr1);
    FillRectColor(r2.left, r2.top + 2, r2.left, r2.bottom - 2, clr1);

    SetPixelColor(r2.left + 1, r2.top + 1, clr1);
    SetPixelColor(r2.right - 2, r2.bottom - 2, clr2);

    if (this->TestFlags(FLAG_ENABLED) == 0) {
        CRect t = this->ClientRectToScreen(this->rect);
        ShadowRect(t, 3);
    }
    UnlockSurface2();
}

int32_t VisButton::OnMouseMove(uint32_t wparam, CPoint pos) {
    //4d958f
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (this->TestFlags(FLAG_FOCUS) == 0) {
        this->parent->FocusTo(this, true);
    }

    CRect t = this->ClientRectToScreen(this->rect);
    
    if (t.PtInRect(pos)) {
        if (this->TestFlags(FLAG_OVERCURSOR) == 0) {
            this->SetCursorOver(true);
        }
        if (this->mouse_on == 0) {
            this->mouse_on = 1;
            this->VMethod9();
        }
    } else {
        if (this->TestFlags(FLAG_OVERCURSOR) != 0 && this->downed == 0) {
            this->SetCursorOver(false);
        }

        if (this->mouse_on == 1) {
            this->mouse_on = 0;
            this->VMethod9();
        }
    }
    return 0;
}

int32_t VisButton::OnLButtonDown(uint32_t wparam, CPoint pos) {
    //4d96a1
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (this->downed) {
        return 0;
    }
    
    this->SetDowned(true);
    return 1;
}

int32_t VisButton::OnLButtonUp(uint32_t wparam, CPoint pos) {
    //4d96e8
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (this->downed == 0) {
        return 0;
    }

    CRect t = this->ClientRectToScreen(this->rect);

    this->SetDowned(false);

    if (t.PtInRect(pos)) {
        AfxGetMainWnd()->PostMessage(this->msgid, 0, 0);
    }
    return 1;
}

int32_t VisButton::OnKeyDown(uint32_t wparam) {
    //4d9783
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (wparam != VK_RETURN) {
        return 0;
    }

    AfxGetMainWnd()->PostMessage(this->msgid, 0, 0);
    return 1;
}

int32_t VisButton::OnChar(uint32_t wparam) {
    //4d97d4
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (EncodeChar(wparam) != this->charid) {
        return 0;
    }

    AfxGetMainWnd()->PostMessage(this->msgid, 0, 0);
    return 1;
}

VisButton::VisButton(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: CVisualObject(_id, r, hint) {
    //4d8e2d
    this->caption = _caption;
    this->font = _font;
    this->msgid = _msgid;
    this->clr = _clr;
    this->charid = _charid;

    for (int i = 1; i < this->caption.GetLength(); i++) {
        char c0 = this->caption[i - 1];
        char c1 = this->caption[i];
        if (c1 == '~' && c0 == '~') {
            i += 2;
        } else if (c1 != '~' && c0 == '~') {
            this->charid = ToLowerChar(c1);
            break;
        }
    }

    this->mouse_on = 0;
    this->downed = 0;
    this->flags |= FLAG_NOTFOCUS;
}

VisButton::VisButton(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: CVisualObject(_id, l, t, r, b, hint) {
    //4d8cd0
    this->caption = _caption;
    this->font = _font;
    this->msgid = _msgid;
    this->clr = _clr;
    this->charid = _charid;

    for (int i = 1; i < this->caption.GetLength(); i++) {
        char c0 = this->caption[i - 1];
        char c1 = this->caption[i];
        if (c1 == '~' && c0 == '~') {
            i += 2;
        } else if (c1 != '~' && c0 == '~') {
            this->charid = ToLowerChar(c1);
            break;
        }
    }

    this->mouse_on = 0;
    this->downed = 0;
    this->flags |= FLAG_NOTFOCUS;
}

void VisButton::SetDowned(bool down) {
    //4d94b5
    if (!down) {
        this->downed = 0;
        if (this->TestFlags(FLAG_OVERCURSOR)) {
            this->SetCursorOver(false);
        }

        this->VMethod9();
    } else {
        g_SfxArray[2]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xdc, 0);

        this->downed = 1;
        if (this->TestFlags(FLAG_OVERCURSOR) == 0) {
            this->SetCursorOver(true);
        }

        this->VMethod9();
    }
}



VisScrollBar::~VisScrollBar() {
    //4e3e80
}

void VisScrollBar::SetCursorOver(bool isOver) {
    //4de6df
    CVisualObject::SetCursorOver(isOver);
    this->is_mouse_over = isOver;
}

void VisScrollBar::VMethod7() {
    if (!this->parent) {
        return;
    }

    CRect tr = this->ClientRectToScreen(this->rect);
    tr.BottomRight() += CPoint(4, 4);

    LockSurface2();
    this->parent->VMethod8(&tr);

    tr = this->ClientRectToScreen(this->rect);

    if (tr.Width() < tr.Height()) {
        int32_t val_pos = 0;

        if (this->val_max < 2) {
            val_pos = 0;
        } else {
            val_pos = (this->val * (tr.Height() - 3 * tr.Width() + 8)) / (this->val_max - 1);
        }

        if (this->mouse_on_minus == 0) {
            gfx_scrollbars->DrawBrightened(tr.left + 4, tr.top + 4, 18, 4, 0);
            gfx_scrollbars->Draw(tr.left, tr.top, 18, 0, 0);
        } else {
            gfx_scrollbars->DrawBrightened(tr.left + 4, tr.top + 4, 21, 4, 0);
            gfx_scrollbars->Draw(tr.left, tr.top, 21, 0, 0);
        }

        for (int32_t i = 1; i < tr.Height() / tr.Width(); i++) {
            gfx_scrollbars->DrawBrightened(tr.left + 4, tr.top + 4 + i * tr.Width(), 19, 4, 0);
            gfx_scrollbars->Draw(tr.left, tr.top + i * tr.Width(), 19, 0, 0);
        }

        gfx_scrollbars->DrawBrightened(tr.left + 4, tr.top + tr.Width() + val_pos, 22, 0, 0);

        if (this->mouse_on_plus == 0) {
            gfx_scrollbars->DrawBrightened(tr.left + 4, tr.bottom - tr.Width() + 4, 20, 4, 0);
            gfx_scrollbars->Draw(tr.left, tr.bottom - tr.Width(), 20, 0, 0);
        } else {
            gfx_scrollbars->DrawBrightened(tr.left + 4, tr.bottom - tr.Width() + 4, 23, 4, 0);
            gfx_scrollbars->Draw(tr.left, tr.bottom - tr.Width(), 23, 0, 0);
        }

        gfx_scrollbars->Draw(tr.left, tr.top + tr.Width() - 4 + val_pos, 22, 0, 0);
    } else {
        this->UpdateHBoxes();

        if (this->mouse_on_minus == 0) {
            gfx_scrollbars->DrawBrightened(tr.left + 4, tr.top + 4, 0, 4, 0);
            gfx_scrollbars->Draw(tr.left, tr.top, 0, 0, 0);
        } else {
            gfx_scrollbars->DrawBrightened(tr.left + 4, tr.top + 4, 3, 4, 0);
            gfx_scrollbars->Draw(tr.left, tr.top, 3, 0, 0);
        }

        for (int32_t i = 1; i < tr.Width() / tr.Height(); i++) {
            gfx_scrollbars->DrawBrightened(tr.left + 4 + i * tr.Height(), tr.top + 4, 7, 4, 0);
            gfx_scrollbars->Draw(tr.left + i * tr.Height(), tr.top, 7, 0, 0);
        }

        gfx_scrollbars->DrawBrightened(this->horiz_box_pos.left + 5, tr.top + 4, 10, 4, 0);

        if (this->mouse_on_plus == 0) {
            gfx_scrollbars->DrawBrightened(tr.right - tr.Height() + 4, tr.top + 4, 8, 4, 0);
            gfx_scrollbars->Draw(tr.right - tr.Height(), tr.top, 8, 0, 0);
        } else {
            gfx_scrollbars->DrawBrightened(tr.right - tr.Height() + 4, tr.top + 4, 11, 4, 0);
            gfx_scrollbars->Draw(tr.right - tr.Height(), tr.top, 11, 0, 0);
        }

        gfx_scrollbars->Draw(this->horiz_box_pos.left + 1, tr.top, 10, 0, 0);
    }
    
    if (this->TestFlags(FLAG_ENABLED) == 0) {
        tr = this->ClientRectToScreen(this->rect);
        tr.InflateRect(CRect(1, 1, 1, 1));

        ShadowRect(tr, 3);
    }
    UnlockSurface2();
}


void VisScrollBar::VMethod9() {
    //4de0d0
    if (g_IsServer == 0) {
        this->VMethod7();
        CRect tr = this->ClientRectToScreen(this->rect);
        tr.BottomRight() += CPoint(4, 4);
        gfxFlushRect(tr);
    }
}



void VisScrollBar::WriteData(void* buf) {
    //4e3d90
    VisScrollBar::Data* dat = (VisScrollBar::Data*)buf;
    dat->v = this->val;
    dat->vmax = this->val_max;
}


void VisScrollBar::ReadData(const void* buf) {
    //4e3dc0
    const VisScrollBar::Data* dat = (const VisScrollBar::Data*)buf;
    this->val = dat->v;
    this->val_max = dat->vmax;
}

int32_t VisScrollBar::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    //4df055
    if (msg == VIS_MSG_SCROLLBAR_RESIZE) {
        this->UpdateRects();
    }

    return CVisualObject::MsgProc(msg, wparam, lparam);
}

int32_t VisScrollBar::OnMouseMove(uint32_t wparam, CPoint pos) {
    //4deb32
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    CRect tr = this->ClientRectToScreen(this->rect);

    if (this->btn_minus.PtInRect(pos)) {
        if (this->mouse_on_minus == 0) {
            this->mouse_on_minus = 1;
            this->VMethod9();
        }

        if (this->TestFlags(FLAG_OVERCURSOR) == 0) {
            CVisualObject::SetCursorOver(true); //do not set   is_mouse_over
        }
    } else {
        if (this->mouse_on_minus != 0) {
            CVisualObject::SetCursorOver(false); //do not set   is_mouse_over
            this->mouse_on_minus = 0;
            this->VMethod9();
        }
    }

    if (this->btn_plus.PtInRect(pos)) {
        if (this->mouse_on_plus == 0) {
            this->mouse_on_plus = 1;
            this->VMethod9();
        }

        if (this->TestFlags(FLAG_OVERCURSOR) == 0) {
            CVisualObject::SetCursorOver(true); //do not set   is_mouse_over
        }
    } else {
        if (this->mouse_on_plus != 0) {
            CVisualObject::SetCursorOver(false); //do not set   is_mouse_over
            this->mouse_on_plus = 0;
            this->VMethod9();
        }
    }

    if (this->TestFlags(FLAG_FOCUS) == 0) {
        if (tr.Height() < tr.Width()) {
            this->parent->FocusTo(this, true);
        }
    }

    if (this->TestFlags(FLAG_OVERCURSOR) == 0 || this->is_mouse_over == 0) {
        if ((wparam & 1) != 0) {
            this->OnLButtonDown(wparam, pos);
        }
        return 0;
    }

    if (tr.Width() < tr.Height()) {
        int32_t val_pos = this->val_max - 1;
        if (this->val_max >= 2) {
            val_pos = ((this->val_max - 1) * (pos.y - tr.top - 24)) / (tr.Height() + (tr.Width() - 4) * -3);
        }

        if (val_pos < 0) {
            val_pos = 0;
        }

        if (val_pos > this->val_max - 1) {
            val_pos = this->val_max - 1;
        }

        this->parent->MsgProc(VIS_MSG_SCROLL_SELECT, this->id, val_pos);

        if (pos.x < tr.left - tr.Width() ||
            pos.x > tr.right + tr.Width() ||
            pos.y < tr.top - tr.Width() ||
            pos.y > tr.bottom + tr.Width() ) {
            this->SetCursorOver(false);
        }
    } else if (this->mousedown_on_hbox != 0) {
        this->val = this->GetValHPos(pos.x);
        this->VMethod9();
        this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->val);
    }
    return 0;
}


int32_t VisScrollBar::OnWmUser(uint32_t wparam, CPoint pos) {
    //4deebd
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (this->rect.Height() < this->rect.Width()) {
        if (this->TestFlags(FLAG_OVERCURSOR) &&
            (this->btn_minus.PtInRect(pos) || this->btn_plus.PtInRect(pos))) {
            return this->OnLButtonDown(wparam | 1, pos);
        }
    }

    if (this->rect.Width() < this->rect.Height()) {
        if (wparam == 1) {
            return this->OnMouseMove(1, pos);
        }
    }
    return 1;
}


int32_t VisScrollBar::OnLButtonDown(uint32_t wparam, CPoint pos) {
    //4de701
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    CRect rt = this->ClientRectToScreen(this->rect);
    if (rt.Width() < rt.Height()) {
        int32_t val_pos = this->val_max - 1;
        if (this->val_max >= 2) {
            val_pos = (this->val * (rt.Height() + rt.Width() * -3 + 8)) / (this->val_max - 1);
        }

        if (pos.y - rt.top < rt.Width()) {
            this->parent->MsgProc(VIS_MSG_SCROLL_UP, this->id, 0);
        } else if (rt.bottom - pos.y < rt.Width() - 4) {
            this->parent->MsgProc(VIS_MSG_SCROLL_DOWN, this->id, 0);
        } else if (pos.y - rt.top - rt.Width() + 4 < val_pos) {
            if (this->down_on_part != 2) {
                this->parent->MsgProc(VIS_MSG_SCROLL_PAGE_UP, this->id, 0);
                this->down_on_part = 1;
            }
        } else if ((pos.y - rt.top) - (rt.Width() * 2 - 4) < val_pos) {
            this->SetCursorOver(true);
        } else if (this->down_on_part != 1) {
            this->parent->MsgProc(VIS_MSG_SCROLL_PAGE_DOWN, this->id, 0);
            this->down_on_part = 2;
        }
    } else {
        if (this->btn_minus.PtInRect(pos)) {
            int32_t d = this->val_max / 16;
            if (d < 1) {
                d = 1;
            }

            if (this->val - d < 0) {
                this->val = 0;
            } else {
                this->val -= d;
            }
        } else if (this->btn_plus.PtInRect(pos)) {
            int32_t d = this->val_max / 16;
            if (d < 1) {
                d = 1;
            }

            if (this->val + d > this->val_max) {
                this->val = this->val_max;
            } else {
                this->val += d;
            }
        } else {
            this->val = this->GetValHPos(pos.x);
            this->UpdateHBoxes();

            if (this->mousedown_on_hbox == 0) {
                this->mousedown_on_hbox = this->horiz_box_pos.PtInRect(pos);

                if (this->mousedown_on_hbox != 0 && this->TestFlags(FLAG_OVERCURSOR) == 0) {
                    this->SetCursorOver(true);
                }
            }            
        }

        this->VMethod9();
        this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->val);
    }
    return 1;
}


int32_t VisScrollBar::OnLButtonUp(uint32_t wparam, CPoint pos) {
    //4defc7
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    this->down_on_part = 0;

    if (this->is_mouse_over) {
        this->parent->MsgProc(VIS_MSG_SFX_TEST, this->id, 0);
        this->SetCursorOver(false);
        this->mousedown_on_hbox = 0;
    }
    return 0;
}


int32_t VisScrollBar::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    //4e3df0
    if (this->OnLButtonDown(wparam, pos) != 0 && this->OnLButtonUp(wparam, pos) != 0) {
        return 1;
    }
    return 0;
}


int32_t VisScrollBar::OnKeyDown(uint32_t wparam) {
    //4df087
    if (this->TestFlags(FLAG_FOCUS) == 0 || this->rect.Height() >= this->rect.Width()) {
        return CVisualObject::OnKeyDown(wparam);
    }

    int32_t d = this->val_max / 16;
    if (d == 0) {
        d = 1;
    }

    if (wparam == VK_LEFT) {
        if (this->val - d < 0) {
            this->val = 0;
        } else {
            this->val -= d;
        }

        this->VMethod9();
        this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->val);
        return 1;
    }
    
    if (wparam == VK_RIGHT) {
        if (this->val + d >= this->val_max) {
            this->val = this->val_max;
        } else {
            this->val += d;
        }

        this->VMethod9();
        this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->val);
        return 1;
    }
    return 0;
}

VisScrollBar::VisScrollBar(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* hint)
: CVisualObject(_id, l, t, r, b, hint) {
    //4dde6a
    this->dirty = 0;
    this->val = 0;
    this->val_max = 0;
    this->mouse_on_minus = 0;
    this->mouse_on_plus = 0;
    this->is_mouse_over = 0;
    this->mousedown_on_hbox = 0;
    this->caption_obj = nullptr;

    AfxGetMainWnd()->PostMessage(0x46f, 0, 0);
}


void VisScrollBar::SetPos(int32_t pos, int32_t pos_max) {
    //4de086
    if (pos_max > -1) {
        this->val_max = pos_max;
    }

    if (pos > -1 && pos < this->val_max) {
        this->val = pos;
    }

    this->VMethod9();
}


void VisScrollBar::UpdateRects() {
    //4ddacf
    this->dirty = 1;

    CRect rt = this->ClientRectToScreen(this->rect);
    if (rt.Width() < rt.Height()) {
        this->btn_minus = CRect(rt.left, rt.top, rt.right, rt.top + rt.Width() - 4);
        this->btn_plus = CRect(rt.left, rt.bottom - rt.Width() + 4, rt.right, rt.bottom);
    } else {
        this->btn_minus = CRect(rt.left, rt.top, rt.left + rt.Height() - 4, rt.bottom);
        this->btn_plus = CRect(rt.right - rt.Height() + 4, rt.top, rt.right, rt.bottom);

        this->flags |= FLAG_NOTFOCUS;
    }
    this->UpdateHBoxes();
}


void VisScrollBar::UpdateHBoxes() {
    //4ddc41

    CRect rt = this->ClientRectToScreen(this->rect);
    if (rt.Height() < rt.Width()) {
        int32_t p = rt.left + rt.Height() - 4;
        if (this->val_max != 0) {
            p += (rt.Width() - (rt.Height() * 2 - 8) - 12) * this->val / this->val_max;
        }

        this->horiz_box_pos = CRect(p, rt.top, p + 16, rt.bottom);
        this->rect4 = CRect(this->btn_minus.right, this->btn_minus.top, this->horiz_box_pos.left, this->horiz_box_pos.bottom);
        this->rect5 = CRect(this->horiz_box_pos.right, this->horiz_box_pos.top, this->btn_plus.left, this->btn_plus.bottom);
    }
}


int32_t VisScrollBar::GetValHPos(int32_t x) {
    //4dddbe
    CRect rt = this->ClientRectToScreen(this->rect);
    int32_t p = this->val_max * (x - (rt.left + 2 + rt.Height())) / (rt.Width() - (rt.Height() * 2 - 8) - 12);
    if (p < 0) {
        p = 0;
    }
    if (p > this->val_max) {
        p = this->val_max;
    }
    return p;
}






VisListBox::~VisListBox() = default; //44f430

void VisListBox::VMethod7() {
    //4dc11d
    if (!this->parent) {
        return;
    }

    CRect rt = this->ClientRectToScreen(this->rect);

    LockSurface2();

    this->parent->VMethod8(&rt);

    int32_t y = rt.top + 2;
    for (int32_t i = this->vis_start_index; i < this->vis_start_index + this->num_vis_entry; i++) {
        if (i >= this->vis_start_index) {
            this->entry_height_full -= 2;

            FillRectColor(rt.left + 1, y - 2, rt.right - 5, y - 2, GetColorRGB(8, 8, 8));
            FillRectColor(rt.left, y - 1, rt.left, y - 3 + this->entry_height_full, GetColorRGB(8, 8, 8));

            FillRectColor(rt.left + 1, y - 2 + this->entry_height_full, rt.right - 5, y - 2 + this->entry_height_full, GetColorRGB(0x5e, 0x73, 0x65));
            FillRectColor(rt.right - 4, y - 1, rt.right - 4, y - 3 + this->entry_height_full, GetColorRGB(0x5e, 0x73, 0x65));

            this->entry_height_full += 2;
        }

        if (this->selected_index == i) {
            this->VMethod30(CPoint(rt.left, y), rt);

            if (this->TestFlags(FLAG_FOCUS) == 0) {
                this->DrawItem(i, CPoint(rt.left + 4, y - 2), p_clrsh_ShockingBlack);
            } else {
                this->DrawItem(i, CPoint(rt.left + 4, y - 2), p_clrsh_Gold);
            }
        } else {
            this->DrawItem(i, CPoint(rt.left + 4, y - 2), p_clrsh_Black);
        }

        y += this->entry_height_full;
    }
    UnlockSurface2();
}

void VisListBox::WriteData(void* buf) {
    //4dc0bc
    ((CStringArray*)buf)->Copy(this->entries);
}

uint32_t VisListBox::DataSize() {
    //4507e0
    return 4; //???
}

void VisListBox::ReadData(const void* buf) {
    //4dc0ea
    this->entries.Copy(**(const CStringArray**)buf);
}

int32_t VisListBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    //4dc62f
    int32_t res = CVisualObject::MsgProc(msg, wparam, lparam);
    if (res != 0) {
        return res;
    }

    switch (msg) {
    case VIS_MSG_SCROLL_SELECT:
        if (wparam == this->scrollbox_id) {
            this->SelectItem(lparam);
            return 1;
        }
        break;

    case VIS_MSG_SCROLL_UP:
        if (wparam == this->scrollbox_id) {
            this->Up();
            return 1;
        }
        break;

    case VIS_MSG_SCROLL_DOWN:
        if (wparam == this->scrollbox_id) {
            this->Down();
            return 1;
        }
        break;

    case VIS_MSG_SCROLL_PAGE_UP:
        if (wparam == this->scrollbox_id) {
            this->PageUp();
            return 1;
        }
        break;

    case VIS_MSG_SCROLL_PAGE_DOWN:
        if (wparam == this->scrollbox_id) {
            this->PageDown();
            return 1;
        }
    }

    return 0;
}

int32_t VisListBox::OnMouseMove(uint32_t wparam, CPoint pos) {
    //4dc8a4
    if (this->TestFlags(FLAG_FOCUS) == 0) {
        this->parent->FocusTo(this, true);
    }

    if ((wparam & 1) != 0) {
        this->OnLButtonDown(wparam, pos);
    }

    return 0;
}


int32_t VisListBox::OnLButtonDown(uint32_t wparam, CPoint pos) {
    //4dc73f

    int32_t idx = this->YToIndex(pos.y);
    if (idx < this->num_vis_entry) {
        if (this->vis_start_index + idx < this->entries.GetSize() - 1) {
            idx += this->vis_start_index;
        } else {
            idx = this->entries.GetSize() - 1;
        }
    }

    this->selected_index = idx;

    this->VMethod9();

    VisScrollBar* scrollbar = (VisScrollBar*)this->parent->FindChild(this->scrollbox_id);
    if (scrollbar) {
        scrollbar->SetPos(this->selected_index, this->entries.GetSize());
    }

    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->selected_index);

    return 1;
}

int32_t VisListBox::OnLButtonUp(uint32_t wparam, CPoint pos) {
    //4dc832
    this->parent->MsgProc(VIS_MSG_LIST_SELECT, this->id, this->selected_index);
    return 1;
}


int32_t VisListBox::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    //4dc86b
    this->parent->MsgProc(VIS_MSG_LIST_ACTIVATE, this->id, this->selected_index);
    return 1;
}


int32_t VisListBox::OnKeyDown(uint32_t wparam) {
    //4dc8f6

    if (this->TestFlags(FLAG_FOCUS) == 0) {
        return CVisualObject::OnKeyDown(wparam);
    }

    switch (wparam) {
    case VK_PRIOR:
        this->PageUp();
        break;

    case VK_NEXT:
        this->PageDown();
        break;

    case VK_UP:
        this->Up();
        break;

    case VK_DOWN:
        this->Down();
        break;

    default:
        break;
    }

    switch (wparam) {
    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
        this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->selected_index);
        return 1;

    default:
        break;
    }

    return 0;
}


int32_t VisListBox::IsValidIndex(int32_t idx) {
    //4dc528
    if (idx > -1 && idx < this->entries.GetSize()) {
        return 1;
    }
    return 0;
}

void VisListBox::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    //4dc560
    if (!this->IsValidIndex(idx)) {
        return;
    }

    CRect tmp;
    GetClipRect(&tmp);

    CRect rt = this->ClientRectToScreen(this->rect);

    CRect clip(pos.x, pos.y, rt.right - 6, pos.y + 2 + this->font->GetHeight());
    SetClipRect(clip);

    this->font->DrawTextWithShadow(pos.x, pos.y, this->entries[idx], 0, clr, 1);

    SetClipRect(tmp);
}

void VisListBox::SelectItem(int32_t idx) {
    //4dbe7f
    // Note: vanilla clamps only the upper bound, a negative idx keeps its value through
    // the vis_start_index logic (selecting nothing) and lands in selected_index.
    if (idx < 0) {
        idx = 0;
    }

    if (idx >= this->entries.GetSize()) {
        idx = this->entries.GetSize() - 1;
    }

    if (idx < 0) {
        this->vis_start_index = 0;
    } else if (idx < this->vis_start_index) {
        this->vis_start_index = idx;
    } else if (idx >= this->vis_start_index + this->num_vis_entry) {
        this->vis_start_index = (idx - this->num_vis_entry) + 1;
    }

    this->selected_index = idx;

    this->VMethod9();

    VisScrollBar* scrollbar = (VisScrollBar*)this->parent->FindChild(this->scrollbox_id);
    if (scrollbar) {
        scrollbar->SetPos(this->selected_index, this->entries.GetSize());
    }

    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->selected_index);
}

int32_t VisListBox::GetItemCount() {
    //4507c0
    return this->entries.GetSize();
}

void VisListBox::VMethod30(CPoint pos, const CRect& r) {
    //4dc4ee
    CRect rt;
    rt.top = pos.y - 1;
    rt.left = pos.x;
    rt.right = r.right - 4;
    rt.bottom = pos.y - 3 + this->entry_height_full;

    ShadowRect(rt, 10);
}


VisListBox::VisListBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: CVisualObject(_id, r, hint) {
    //4dbd31

    this->entry_height = _font->GetHeight();
    this->entry_height_full = this->entry_height + 4;

    this->flags |= FLAG_NOTFOCUS;

    this->selected_index = -1;
    this->vis_start_index = 0;
    this->font = _font;
    this->clr1 = _clr1;
    this->clr2 = _clr2;
    this->scrollbox_id = _scrollid;

    this->num_vis_entry = this->rect.Height() / this->entry_height_full;

    this->rect.bottom = this->rect.top + 2 + this->num_vis_entry * entry_height_full;
}


VisListBox::VisListBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: CVisualObject(_id, l, t, r, b, hint) {
    //4dbc11

    this->entry_height = _font->GetHeight();
    this->entry_height_full = this->entry_height + 4;

    this->flags |= FLAG_NOTFOCUS;

    this->selected_index = -1;
    this->vis_start_index = 0;
    this->font = _font;
    this->clr1 = _clr1;
    this->clr2 = _clr2;
    this->scrollbox_id = _scrollid;

    this->num_vis_entry = this->rect.Height() / this->entry_height_full;

    this->rect.bottom = this->rect.top + 2 + this->num_vis_entry * entry_height_full;
}


void VisListBox::Down() {
    //4dbfbb
    this->SelectItem(this->selected_index + 1);
}

void VisListBox::Up() {
    //4dbf89
    if (this->selected_index > 0) {
        this->SelectItem(this->selected_index - 1);
    }
}

void VisListBox::PageDown() {
    //4dc03b
    if (this->selected_index == this->vis_start_index + this->num_vis_entry - 1) {
        this->SelectItem(this->vis_start_index + this->num_vis_entry + this->num_vis_entry - 1);
    } else {
        this->SelectItem(this->vis_start_index + this->num_vis_entry - 1);
    }
}

void VisListBox::PageUp() {
    //4dbfe1
    if (this->selected_index == this->vis_start_index) {
        this->SelectItem(this->vis_start_index - this->num_vis_entry);
    } else {
        this->SelectItem(this->vis_start_index);
    }
}


int32_t VisListBox::YToIndex(int32_t y) {
    //4dbe45
    return (y - this->ClientPtToScreen(this->rect.TopLeft()).y - 1) / this->entry_height_full;
}

void VisListBox::AddItem(const char* str) {
    //4507f0
    this->entries.Add(str);
    if (this->selected_index < 0) {
        this->selected_index++;
    }
}

void VisListBox::SetSelectedIndex(int32_t idx) {
    //4507a0
    // Note: vanilla stores the raw index with no clamping.
    if (idx < 0) {
        idx = 0;
    } else if (idx >= this->entries.GetSize()) {
        idx = this->entries.GetSize() - 1;
    }

    this->selected_index = idx;
}

CString& VisListBox::GetItem(int32_t idx) {
    //4508b0
    // Vanilla: i = (idx < GetSize()) ? idx : GetSize(); no negative guard.
    if (idx < 0) {
        return this->entries[0];
    }

    if (idx < this->entries.GetSize()) {
        return this->entries[idx];
    }

    return this->entries[this->entries.GetSize() - 1];
}

void VisListBox::RemoveItem(int32_t idx) { //450830
    this->entries.RemoveAt(idx);
    this->selected_index -= (this->entries.GetSize() == 0); // WAT?
}




VisTextBox::~VisTextBox() = default; //4503a0

void VisTextBox::VMethod7() {
    //4d9a21
    // client: 4de337
    if (!this->parent) {
        return;
    }

    CRect rt = this->ClientRectToScreen(this->rect);

    LockSurface2();

    this->parent->VMethod8(&rt);

    FillRectColor(rt.left + 1, rt.top, rt.right - 1, rt.top, GetColorRGB(8, 8, 8));
    FillRectColor(rt.left, rt.top + 1, rt.left, rt.bottom - 1, GetColorRGB(8, 8, 8));
    FillRectColor(rt.right, rt.top + 1, rt.right, rt.bottom - 1, GetColorRGB(0x5e, 0x73, 0x65));
    FillRectColor(rt.left + 1, rt.bottom, rt.right - 1, rt.bottom, GetColorRGB(0x5e, 0x73, 0x65));

    CString s = this->text;
#ifdef A2CLIENT
    if (this->is_password != 0) {
        for (int32_t i = 0; i < s.GetLength(); i++) {
            s.SetAt(i, '*');
        }
    }
#endif

    if (this->select_start != this->select_end) {
        CRect rshd;
        rshd.bottom = rt.bottom - 2;

        rshd.right = rt.left + 4 + this->font->GetStrWidth(s.Left(this->select_end));
        rshd.top = rt.top + 2;

        rshd.left = rt.left + 4 + this->font->GetStrWidth(s.Left(this->select_start));

        ShadowRect(rshd, 12);
    }

    this->font->DrawTextWithShadow(rt.left + 4, rt.top + rt.Height() / 2, s, 8, this->clr, 1);

    if (this->TestFlags(FLAG_FOCUS) != 0 && this->cursor_blink != 0) {
        CRect blink;
        blink.left = rt.left + 4 + this->font->GetStrWidth(s.Left(this->cursor_pos));
        blink.right = blink.left + 2;
        blink.top = rt.top + 2;
        blink.bottom = rt.bottom - 2;
        FillRectColorSimple(blink.left, blink.top, blink.right, blink.bottom, GetColorRGB(255, 255, 255));
    }
#ifdef A2CLIENT
    if (this->TestFlags(FLAG_ENABLED) == 0) {
        ShadowRect(rt, 3);
    }
#endif
    UnlockSurface2();
}

void VisTextBox::WriteData(void* buf) {
    //450440
    strcpy((char*)buf, this->text);
}


uint32_t VisTextBox::DataSize() {
    //450430
    return 4;
}


void VisTextBox::ReadData(const void* buf) {
    //450410
    this->text = (const char*)buf;
}


int32_t VisTextBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    //4daa2b
    if (msg == VIS_MSG_CURSOR_BLINK && (timeGetTime() - this->cursor_blink_ts) > 500) {
        this->cursor_blink_ts = timeGetTime();
        this->cursor_blink = 1 - this->cursor_blink;

        if (this->TestFlags(FLAG_ENABLED)) {
            this->VMethod9();
        }
    }

    return CVisualObject::MsgProc(msg, wparam, lparam);
}


int32_t VisTextBox::OnMouseMove(uint32_t wparam, CPoint pos) {
    //4da22e
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (this->TestFlags(FLAG_FOCUS) == 0) {
        this->parent->FocusTo(this, true);
    }

    this->cursor_blink_ts = timeGetTime();

    if ((wparam & 1) == 0) {
        return 0;
    }

    this->parent->FocusTo(this, true);

    int32_t cpos = this->GetCursorPosByX(pos.x);

    if (cpos < this->cursor_pos) {
        if (this->select_end == this->select_start) {
            this->select_end = this->cursor_pos;
            this->select_start = cpos;
        } else if (this->select_start < cpos) {
            this->select_end = cpos;
        } else {
            this->select_start = cpos;
        }
    } else if (cpos > this->cursor_pos) {
        if (this->select_end == this->select_start) {
            this->select_start = this->cursor_pos;
            this->select_end = cpos;
        } else if (cpos < this->select_end) {
            this->select_start = cpos;
        } else {
            this->select_end = cpos;
        }
    }

    this->cursor_pos = cpos;

    this->ResetBlink();

    this->VMethod9();

    return 0;
}


int32_t VisTextBox::OnLButtonDown(uint32_t wparam, CPoint pos) {
    //4da37c
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    this->cursor_blink_ts = timeGetTime();

    this->cursor_pos = this->GetCursorPosByX(pos.x);;

    this->select_end = this->cursor_pos;
    this->select_start = this->select_end;

    this->VMethod9();

    return 1;
}

int32_t VisTextBox::OnKeyDown(uint32_t wparam) {
    //4da3ed
    if (this->TestFlags(FLAG_FOCUS) == 0) {
        return 0;
    }

    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    this->cursor_blink_ts = timeGetTime();

    int32_t res = 0;

    switch (wparam) {
    case VK_BACK:
        if (this->cursor_pos != 0) {
            this->cursor_pos--;

            this->text = this->text.Left(this->cursor_pos) + this->text.Right(this->text.GetLength() - this->cursor_pos - 1);
            
            this->ResetBlink();

            this->VMethod9();
            res = 1;
        } else {
            res = 0;
        }
        break;

    case VK_END:
        if (this->cursor_pos < this->text.GetLength()) {
            if (g_kbShiftState == 0) {
                this->select_end = this->select_start;
            } else {
                if (this->select_start == this->select_end) {
                    this->select_start = this->cursor_pos;
                } else if (this->cursor_pos < this->select_end) {
                    this->select_start = this->select_end;
                }

                this->select_end = this->text.GetLength();
            }

            this->cursor_pos = this->text.GetLength();

            this->ResetBlink();

            this->VMethod9();
            res = 1;
        }
        break;

    case VK_HOME:
        if (this->cursor_pos != 0) {
            if (g_kbShiftState == 0) {
                this->select_end = this->select_start;
            } else {
                if (this->select_start == this->select_end) {
                    this->select_end = this->cursor_pos;
                } else if (this->select_start < this->cursor_pos) {
                    this->select_end = this->select_start;
                }

                this->select_start = 0;
            }

            this->cursor_pos = 0;

            this->ResetBlink();

            this->VMethod9();
            res = 1;
        }
        break;

    case VK_LEFT:
        if (this->cursor_pos != 0) {
            this->cursor_pos--;

            if (g_kbShiftState == 0) {
                this->select_end = this->select_start;
            } else if (this->select_end == this->select_start) {
                this->select_end = this->cursor_pos + 1;
                this->select_start = this->cursor_pos;
            } else if (this->cursor_pos + 1 == this->select_start) {
                this->select_start--;
            } else {
                this->select_end--;
            }

            this->ResetBlink();

            this->VMethod9();
            res = 1;
        }
        break;

    case VK_RIGHT:
        if (this->cursor_pos < this->text.GetLength()) {
            this->cursor_pos++;
            if (g_kbShiftState == 0) {
                this->select_end = this->select_start;
            } else if (this->select_end == this->select_start) {
                this->select_start = this->cursor_pos - 1;
                this->select_end = this->cursor_pos;
            } else if (this->cursor_pos - 1 == this->select_end) {
                this->select_end++;
            } else {
                this->select_start++;
            }

            this->ResetBlink();

            this->VMethod9();
            res = 1;
        }
        break;

    case VK_DELETE:
        if (this->select_end == this->select_start || (this->select_end - this->select_start) < 0) {
            if (this->cursor_pos < this->text.GetLength()) {
                this->text = this->text.Left(this->cursor_pos) + this->text.Right((this->text.GetLength() - this->cursor_pos) - 1);
            }
        } else {
            this->DelSelection();
        }

        this->ResetBlink();

        this->VMethod9();
        res = 1;
        break;

    default:
        break;
    }

    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, 0);

    return res;
}


int32_t VisTextBox::OnChar(uint32_t wparam) {
    //4da96a
    // client: 4df2eb

    if (isalnum(wparam) != 0 && this->TestFlags(FLAG_FOCUS) == 0) {
        this->parent->FocusTo(this, true);
    }

    if (this->TestFlags(FLAG_FOCUS) == 0 || !this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    this->cursor_blink_ts = timeGetTime();

#ifdef A2CLIENT
    if (wparam >= ' ' && wparam != '~') {
#else
    if (wparam >= ' ') {
#endif
        this->InsertChar(wparam);
    }

    this->VMethod9();

    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, 0);

    return 1;
}


VisTextBox::VisTextBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: CVisualObject(_id, l, t, r, b, hint) {
    //4d9891
    // client: 4de195
    this->font = _font;
    this->clr = _clr;
    this->text = "";
    this->select_start = 0;
    this->select_end = 0;
    this->cursor_pos = 0;

    this->flags |= FLAG_NOTFOCUS;

    this->cursor_blink = 1;
#ifdef A2CLIENT
    this->is_password = 0;
#endif
}


VisTextBox::VisTextBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr, const char* hint)
: CVisualObject(_id, r, hint) {
    //4d995f
    this->font = _font;
    this->clr = _clr;
    this->text = "";
    this->select_start = 0;
    this->select_end = 0;
    this->cursor_pos = 0;

    this->flags |= FLAG_NOTFOCUS;

    this->cursor_blink = 1;
#ifdef A2CLIENT
    this->is_password = 0;
#endif
}

void VisTextBox::DelSelection() {
    //4da067
    this->text = this->text.Left(this->select_start) + this->text.Right(this->text.GetLength() - this->select_end);
    this->select_end = this->select_start;
    this->cursor_pos = this->select_start;
}

void VisTextBox::InsertChar(int32_t chr) {
    //4d9ef2
    if (this->select_end != this->select_start && (this->select_end - this->select_start) > -1) {
        this->DelSelection();
    }

    CString tmp = this->text.Left(this->cursor_pos) + (char)EncodeChar(chr) + this->text.Right(this->text.GetLength() - this->cursor_pos);
    if (this->font->GetStrWidth(tmp) + 8 < this->rect.Width()) {
        this->text = tmp;
        this->cursor_pos++;
    }
    this->ResetBlink();
}

void VisTextBox::ResetBlink() {
    //4daab3
    this->cursor_blink_ts = timeGetTime();
    this->cursor_blink = 1;
}

int32_t VisTextBox::GetCursorPosByX(int32_t x) {
    //4da153
    CRect rt = this->ClientRectToScreen(this->rect);
    for (int32_t i = 0; i < this->text.GetLength(); i++) {
        CString str = this->text.Left(i + 1);
        if (rt.left + 4 + this->font->GetStrWidth(str) >= x) {
            return i;
        }
    }
    return this->text.GetLength();
}


VisRadioBase::~VisRadioBase() = default; //4505b0

void VisRadioBase::SetCursorOver(bool isOver) {
    //4daccd
    CVisualObject::SetCursorOver(isOver);
    this->mouse_over = isOver;
}

void VisRadioBase::WriteData(void* buf) {
    //4504f0
    *(int32_t*)buf = this->selection;
}

uint32_t VisRadioBase::DataSize() {
    //4504e0
    return 4;
}

void VisRadioBase::ReadData(const void* buf) {
    //4506c0
    this->selection = *(const int32_t*)buf;
    this->selected = this->selection;
}

int32_t VisRadioBase::OnMouseMove(uint32_t wparam, CPoint pos) {
    //4dacef
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (this->TestFlags(FLAG_FOCUS) == 0) {
        this->parent->FocusTo(this, true);
    }

    if (wparam & 1) {
        this->OnLButtonDown(wparam, pos);
    } else {
        CRect rt = this->ClientRectToScreen(this->rect);

        if (rt.PtInRect(pos)) {
            if (this->TestFlags(FLAG_OVERCURSOR) == 0) {
                this->SetCursorOver(true);
            }
        } else {
            this->SetCursorOver(false);
        }

        this->VMethod9();
    }
    return 0;
}

int32_t VisRadioBase::GetIndex(int32_t y) {
    //4dac81
    CPoint t = this->ClientPtToScreen(this->rect.TopLeft());
    return (y - t.y) / gfx_radiob->GetHeight(0);
}

VisRadioBase::VisRadioBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: CVisualObject(_id, l, t, r, b, hint) {
    //4dab2b
    this->font = _font;
    this->clr = _clr;
    this->selection = 0;
    this->flags |= FLAG_NOTFOCUS;
    this->selected = 0;
}

void VisRadioBase::AddEntry(const char* etext) {
    //450470
    this->entries.Add(etext);
}



VisRadioType1::~VisRadioType1() = default; //450590

void VisRadioType1::VMethod7() {
    //4dadf8
    CRect rt = this->ClientRectToScreen(this->rect);
    rt.bottom += 4;
    rt.right += 4;

    int32_t x = rt.left + 6 + gfx_radiob->GetWidth(0);
    int32_t y = rt.top;

    LockSurface2();

    this->parent->VMethod8(&rt);

    rt.bottom -= 4;
    rt.right -= 4;

    for (int i = 0; i < this->entries.GetSize(); i++) {
        if ((this->selection & (1 << i)) == 0) {
            gfx_radiob->DrawBrightened(rt.left + 5, y + 4, 2, 4, 0);
            gfx_radiob->Draw(rt.left + 1, y, 2, 0, 0);
        } else {
            gfx_radiob->DrawBrightened(rt.left + 5, y + 4, 3, 4, 0);
            gfx_radiob->Draw(rt.left + 1, y, 3, 0, 0);
        }

        if (this->TestFlags(FLAG_FOCUS) == 0) {
            this->clr = p_clrsh_Black;
        } else {
            this->clr = p_clrsh_Gold;
        }

        this->font->DrawTextWithShadow(x, y + 5, this->entries[i], 0, this->clr, 1);

        y += gfx_radiob->GetHeight(0);
    }

    if (this->TestFlags(FLAG_ENABLED) == 0) {
        rt = this->ClientRectToScreen(this->rect);
        rt.InflateRect(CRect(1, 1, 1, 1));
        ShadowRect(rt, 3);
    }

    UnlockSurface2();
}

void VisRadioType1::ReadData(const void* buf) {
    //450510
    this->selection = *(const int32_t*)buf;
}

int32_t VisRadioType1::OnMouseMove(uint32_t wparam, CPoint pos) {
    //4db134
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (this->TestFlags(FLAG_FOCUS) == 0) {
        this->parent->FocusTo(this, true);
    }

    CRect rt = this->ClientRectToScreen(this->rect);
    rt.bottom += 4;
    rt.right += 4;

    if (rt.PtInRect(pos)) {
        if (this->TestFlags(FLAG_OVERCURSOR) == 0) {
            this->SetCursorOver(true);
        }
    } else {
        this->SetCursorOver(false);
    }

    this->VMethod9();
    return 0;
}

int32_t VisRadioType1::OnLButtonDown(uint32_t wparam, CPoint pos) {
    //4db062
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    this->selected = this->GetIndex(pos.y);

    uint32_t bit = 1 << this->selected;

    if ((this->selection & bit) == 0) {
        this->selection |= bit;
    } else {
        this->selection &= ~bit;
    }

    this->VMethod9();
    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->selection);
    return 1;
}


int32_t VisRadioType1::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    //450530
    return this->OnLButtonDown(wparam, pos);
}

int32_t VisRadioType1::OnKeyDown(uint32_t wparam) {
    //4db2d3
    if (wparam == VK_DOWN && TestFlags(FLAG_FOCUS) != 0) {
        int32_t num = this->entries.GetSize();
        if (num <= 1) {
            return 0;
        }

        if (this->selected < num - 1) {
            this->selected++;
        }

        this->VMethod9();
        return 1;
    }

    if (wparam == VK_UP && TestFlags(FLAG_FOCUS) != 0) {
        int32_t num = this->entries.GetSize();
        if (num <= 1) {
            return 0;
        }

        if (this->selected > 0) {
            this->selected--;
        }

        this->VMethod9();
        return 1;
    }

    return CVisualObject::OnKeyDown(wparam);
}


int32_t VisRadioType1::OnChar(uint32_t wparam) {
    //4db20d
    if (wparam != ' ' || this->TestFlags(FLAG_FOCUS) == 0) {
        return CVisualObject::OnChar(wparam);
    }

    uint32_t bit = 1 << this->selected;
    if ((this->selection & bit) == 0) {
        this->selection |= bit;
    } else { 
        this->selection &= ~bit;
    }
    
    this->VMethod9();
    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->selection);
    return 1;
}

VisRadioType1::VisRadioType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
 : VisRadioBase(_id, l, t, r, b, _font, _clr, hint) {
    //450490
}


VisRadioType2::~VisRadioType2() = default; //450720


void VisRadioType2::VMethod7() {
    //4db680
    CRect rt = this->ClientRectToScreen(this->rect);
    rt.bottom += 4;
    rt.right += 4;

    int32_t x = rt.left + 6 + gfx_radiob->GetWidth(0);
    int32_t y = rt.top;

    LockSurface2();

    this->parent->VMethod8(&rt);

    rt.bottom -= 4;
    rt.right -= 4;

    for (int i = 0; i < this->entries.GetSize(); i++) {
        if (this->selection == i) {
            gfx_radiob->DrawBrightened(rt.left + 5, y + 4, 1, 4, 0);
            gfx_radiob->Draw(rt.left + 1, y, 1, 0, 0);

            if (this->TestFlags(FLAG_FOCUS) == 0) {
                this->clr = p_clrsh_Black;
            } else {
                this->clr = p_clrsh_Gold;
            }
        } else {
            gfx_radiob->DrawBrightened(rt.left + 5, y + 4, 0, 4, 0);
            gfx_radiob->Draw(rt.left + 1, y, 0, 0, 0);

            this->clr = p_clrsh_Black;
        }

        this->font->DrawTextWithShadow(x, y + 5, this->entries[i], 0, this->clr, 1);

        y += gfx_radiob->GetHeight(0);
    }

    if (this->TestFlags(FLAG_ENABLED) == 0) {
        rt = this->ClientRectToScreen(this->rect);
        rt.InflateRect(CRect(1, 1, 1, 1));
        ShadowRect(rt, 3);
    }

    UnlockSurface2();
}

int32_t VisRadioType2::OnMouseMove(uint32_t wparam, CPoint pos) {
    //4dbb7b
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }
    return VisRadioBase::OnMouseMove(wparam, pos);
}

int32_t VisRadioType2::OnLButtonDown(uint32_t wparam, CPoint pos) {
    //4db8e7
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    int32_t idx = this->GetIndex(pos.y);
    if (idx < 0) {
        return 0;
    }
    if (idx >= this->entries.GetSize()) {
        return 0;
    }

    this->selection = idx;
    this->selected = idx;
    this->VMethod9();
    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->selection);
    return 1;
}

int32_t VisRadioType2::OnKeyDown(uint32_t wparam) {
    //4dba0e
    if (wparam == VK_DOWN && TestFlags(FLAG_FOCUS) != 0) {
        int32_t num = this->entries.GetSize();
        if (num <= 0) {
            return 0;
        }

        if (this->selection < num - 1) {
            this->selection++;
        }

        this->selected = this->selection;
        this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->selection);
        this->VMethod9();
        return 1;
    }

    if (wparam == VK_UP && TestFlags(FLAG_FOCUS) != 0) {
        int32_t num = this->entries.GetSize();
        if (num <= 0) {
            return 0;
        }

        if (this->selection > 0) {
            this->selection--;
        }

        this->selected = this->selection;
        this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->selection);
        this->VMethod9();
        return 1;
    }

    return CVisualObject::OnKeyDown(wparam);
}

int32_t VisRadioType2::OnChar(uint32_t wparam) {
    //4db993
    if (wparam != ' ' || this->TestFlags(FLAG_FOCUS) == 0) {
        return CVisualObject::OnChar(wparam);
    }

    this->selection = this->selected;

    this->VMethod9();
    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->selection);
    return 1;
}

VisRadioType2::VisRadioType2(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: VisRadioBase(_id, l, t, r, b, _font, _clr, hint) {
    //450670
}



VisBitmap::~VisBitmap() {
    //4df2c3
    if (this->bitmap) {
        delete this->bitmap;
    }
}

void VisBitmap::VMethod7() {
    //4df345
    if (!this->bitmap) {
        return;
    }

    CRect rt = this->ClientRectToScreen(this->rect);
    LockSurface2();

    if (this->draw_type == 1) {
        this->parent->VMethod8(&rt);
        CBmp64* bmp64 = (CBmp64*)this->bitmap;
        bmp64->DrawSubRectMasked(rt.left, rt.top, 0, 0, bmp64->GetWidth(0), bmp64->GetHeight(0));
    } else if (this->draw_type == 2) {
        this->parent->VMethod8(&rt);

        FillRectColorSimple(rt.left + 8, rt.top + 7, rt.left + 80, rt.top + 101, 0);

        CBmp64* bmp64 = (CBmp64*)this->bitmap;
        bmp64->DrawSubRectMasked(rt.left, rt.top, 0, 0, bmp64->GetWidth(0), bmp64->GetHeight(0));
    } else if (this->draw_type < 11) {
        this->bitmap->Draw(rt.left, rt.top, 0, 0, 0);
    } else {
        this->bitmap->Draw(rt.left, rt.top, this->draw_type - 10, 0, 0);
    }

    UnlockSurface2();
}

VisBitmap::VisBitmap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, int32_t dtype)
: CVisualObject(_id, l, t, r, b, nullptr) {
    //4df23d
    this->bitmap = _bitmap;
    this->draw_type = dtype;
}

VisBitmap::VisBitmap(int32_t _id, const RECT& r, CGameBitmap* _bitmap, int32_t dtype)
: CVisualObject(_id, r, nullptr) {
    //4df286
    this->bitmap = _bitmap;
    this->draw_type = dtype;
}

CGameBitmap* VisBitmap::GetBitmap() {
    //4e3fb0
    return this->bitmap;
}

void VisBitmap::SetBitmap(CGameBitmap* bmp) {
    //4e3fd0
    this->bitmap = bmp;
}





void VisComboBoxText::VMethod7() {
    //4e4170
    CRect rt = this->ClientRectToScreen(this->rect);

    LockSurface2();

    this->parent->VMethod8(&rt);

    FillRectColor(rt.left + 1, rt.top, rt.right - 1, rt.top, GetColorRGB(8, 8, 8));
    FillRectColor(rt.left, rt.top + 1, rt.left, rt.bottom - 1, GetColorRGB(8, 8, 8));
    FillRectColor(rt.right, rt.top + 1, rt.right, rt.bottom - 1, GetColorRGB(0x5e, 0x73, 0x65));
    FillRectColor(rt.left + 1, rt.bottom, rt.right - 1, rt.bottom, GetColorRGB(0x5e, 0x73, 0x65));

    uint16_t *clr = p_clrsh_Gold;
    if (this->parent->TestFlags(FLAG_FOCUS) == 0) {
        clr = p_clrsh_Black;
    }

    this->font->DrawTextWithShadow(rt.left + 4, rt.top + rt.Height() / 2, this->text, 8, clr, 1);

    UnlockSurface2();
}

VisComboBoxText::VisComboBoxText(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: VisTextBox(_id, l, t, r, b, _font, _clr, hint) {
    //4e4120
}



int32_t VisComboBoxList::OnKeyDown(uint32_t wparam) {
    //4e4090
    if (wparam == VK_RETURN) {
        ((VisComboBox*)this->parent)->ProcSelectList();
        return 1;
    }
    return VisListBox::OnKeyDown(wparam);
}

VisComboBoxList::VisComboBoxList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: VisListBox(_id, l, t, r, b, _font, _clr1, _clr2, _scrollid, hint) {
    //4e4040
}


void VisComboBoxButton::SetCursorOver(bool isOver) {
    //4e1cb6
    CVisualObject::SetCursorOver(isOver);

    if (isOver) {
        if (this->parent->TestFlags(FLAG_OVERCURSOR) == 0) {
            this->parent->SetCursorOver(true);
        }
    } else {
        if (this->parent->TestFlags(FLAG_OVERCURSOR) && ((VisComboBox*)this->parent)->list_showed == 0) {
            this->parent->SetCursorOver(false);
        }
    }
}


void VisComboBoxButton::VMethod7() {
    //4e1d7d
    CRect rt = this->ClientRectToScreen(this->rect);
    this->parent->VMethod8(&rt);

    if (this->mouse_on) {
        this->bitmap->Draw(rt.left, rt.top, this->frm + 1, 0, 0);
    } else {
        this->bitmap->Draw(rt.left, rt.top, this->frm, 0, 0);
    }
}

int32_t VisComboBoxButton::OnMouseMove(uint32_t wparam, CPoint pos) {
    //4e1d5c
    return VisButton::OnMouseMove(wparam, pos);
}

int32_t VisComboBoxButton::OnLButtonDown(uint32_t wparam, CPoint pos) {
    //4e1d3b
    return VisButton::OnLButtonDown(wparam, pos);
}

int32_t VisComboBoxButton::OnLButtonUp(uint32_t wparam, CPoint pos) {
    //4e1c16
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (this->downed == 0) {
        return 0;
    }

    CRect rt = this->ClientRectToScreen(this->rect);
    VisButton::SetDowned(false);

    if (rt.PtInRect(pos)) {
        this->parent->MsgProc(this->msgid, 0, 0);
    }

    return 1;
}

VisComboBoxButton::VisComboBoxButton(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, int32_t _frm, int32_t _msgid, int32_t _charid, const char* hint)
: VisButton(_id, l, t, r, b, "", g_font1, clrsh_TechBlack, _msgid, _charid, hint) {
    //4e1b5c
    this->bitmap = _bitmap;
    this->frm = _frm;
}



void VisComboBox::VMethod8(CRect* rect) {
    //4e2481
    this->parent->VMethod8(rect);
}

void VisComboBox::VMethod9() {
    //4e201e
    CVisualObject::VMethod9();
}

void VisComboBox::WriteData(void* buf) {
    //4e2031
    this->textbox->WriteData(buf);
}

int32_t VisComboBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    //4e22d8
    if (msg == VIS_MSG_COMBO_TOGGLE) {
        this->ToggleList();
        return 1;
    } else if (msg == WM_LBUTTONDOWN) {
        CRect rt = this->ClientRectToScreen(this->rect);
        CPoint cpt(g_mousept.x, g_mousept.y);
        if (rt.PtInRect(cpt)) {
            this->parent->FocusTo(this, true);
            return CVisualObject::MsgProc(WM_LBUTTONDOWN, wparam, lparam);
        } else {
            this->HideList();
            this->parent->OnLButtonDown(1, cpt);
            return 1;
        }
    } else if (msg == VIS_MSG_LIST_SELECT) {
        if (wparam == this->listbox->GetId()) {
            this->ProcSelectList();
            return 1;
        }
    } else if (msg == WM_MOUSEMOVE) {
        return CVisualObject::MsgProc(WM_MOUSEMOVE, wparam, lparam);
    }

    return CVisualObject::MsgProc(msg, wparam, lparam);
}

int32_t VisComboBox::OnLButtonDown(uint32_t wparam, CPoint pos) {
    //4e2410
    return 1;
}

int32_t VisComboBox::OnKeyDown(uint32_t wparam) {
    //4e2422
    if (wparam != VK_DOWN || this->TestFlags(FLAG_FOCUS) == 0) {
        return CVisualObject::OnKeyDown(wparam);

    //VK_DOWN
    }
    this->SetFocus(true);
    this->listbox->SetFocus(true);
    this->ToggleList();
    return 1;
}

VisComboBox::VisComboBox(int32_t _id, CRect r, const char* hint)
: CVisualObject(_id, r.left, r.top, r.right, r.top + 24, hint) {
    //4e1e10
    this->textbox = new VisComboBoxText(1, 0, 0, (r.right - r.left) - 24, 24, g_font1, clrsh_TechBlack, hint);
    this->textbox->ChangeFlags(FLAG_ENABLED, false);

    this->AddChild(this->textbox);

    this->listbox = new VisComboBoxList(2, 0, 24, (r.right - r.left), (r.bottom - r.top), g_font1, clrsh_TechBlack, clrsh_ShockingBlack, 10, hint);
    this->listbox->ChangeFlags(FLAG_20, true);

    this->AddChild(this->listbox);

    VisComboBoxButton* btn = new VisComboBoxButton(3, (r.right - r.left) - 22, 2, (r.right - r.left) - 2, 22, gfx_scrollbars, 24, 0x456, 0, "");
    this->AddChild(btn);

    this->isEmpty = 1;
    this->list_showed = 0;
    this->flags |= FLAG_NOTFOCUS;
}

void VisComboBox::AddItem(const char* str) {
    //4e2053
    if (this->isEmpty) {
        this->textbox->ReadData(str);
        this->isEmpty = 0;
    }

    this->listbox->AddItem(str);
}

void VisComboBox::ToggleList() {
    //4e2150
    if (this->list_showed) {
        this->HideList();
        return;
    }

    this->list_showed = 1;

    if (this->TestFlags(FLAG_OVERCURSOR) == 0) {
        this->SetCursorOver(true);
    }

    this->listbox->ChangeFlags(FLAG_20, false);
    this->listbox->SetFocus(true);

    this->rect.bottom = this->rect.top + 24 + this->listbox->GetRect().Height();

    CRect rt = this->ClientRectToScreen(this->rect);
    rt.top += 24;

    this->parent->VMethod8(&rt);
    this->VMethod9();
}

void VisComboBox::HideList() {
    //4e20db
    this->SetCursorOver(false);

    this->list_showed = 0;

    this->listbox->ChangeFlags(FLAG_20, true);
    this->listbox->SetFocus(false);

    this->rect.bottom = this->rect.top + 24;

    this->parent->VMethod9();
    this->VMethod9();
}

void VisComboBox::ProcSelectList() {
    //4e222a
    if (this->list_showed != 0) {
        this->SetCursorOver(false);
        this->list_showed = 0;
    }

    this->listbox->SetFocus(false);
    this->listbox->ChangeFlags(FLAG_20, true);

    this->textbox->ReadData( this->listbox->GetItem(this->listbox->GetSelectedIndex()) );

    this->rect.bottom = this->rect.top + 24;

    this->parent->VMethod9();
    this->VMethod9();
}

void VisComboBox::SelectItem(int32_t index) {
    //4e2097
    this->textbox->ReadData(this->listbox->GetItem(index));
    this->listbox->SetSelectedIndex(index);
}




void VisMultiText::VMethod7() {
    //4d8994
    if (!this->parent) {
        return;
    }

    CRect rt = this->ClientRectToScreen(this->rect);
    this->parent->VMethod8(&rt);

    LockSurface2();
    this->font->DrawTextLinesShadow(rt, this->vis_start_index, this->vis_start_index + this->num_vis_entry, this->entries, this->clr1, this->entry_height_full);
    UnlockSurface2();
}

int32_t VisMultiText::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    //4d8b99
    return VisListBox::MsgProc(msg, wparam, lparam);
}

int32_t VisMultiText::OnMouseMove(uint32_t wparam, CPoint pos) {
    //4d8bba
    return CVisualObject::OnMouseMove(wparam, pos);
}

int32_t VisMultiText::OnWmUser(uint32_t wparam, CPoint pos) {
    //4d8bdb
    if ((wparam & 1) == 0 || this->scrollbox_id == 0) {
        return 1;
    }

    CRect rt = this->ClientRectToScreen(this->rect);
    if ((rt.top + rt.bottom) / 2 < pos.y) {
        VisListBox::Down();
    } else {
        VisListBox::Up();
    }

    return 1;
}

int32_t VisMultiText::OnLButtonDown(uint32_t wparam, CPoint pos) {
    //4d8c3f
    return CVisualObject::OnLButtonDown(wparam, pos);
}

int32_t VisMultiText::OnKeyDown(uint32_t wparam) {
    //4d8c60
    return VisListBox::OnKeyDown(wparam);
}

void VisMultiText::SelectItem(int32_t idx) {
    //4d889d
    if (idx < 0) {
        idx = 0;
    }

    if (idx < this->entries.GetSize() - this->num_vis_entry) {
        this->selected_index = idx;
        this->vis_start_index = idx;
    } else {
        this->selected_index = this->entries.GetSize() - this->num_vis_entry;
        this->vis_start_index = this->selected_index;
    }

    this->VMethod9();

    VisScrollBar* scroll = (VisScrollBar*)this->parent->FindChild(this->scrollbox_id);
    if (scroll) {
        scroll->SetPos(this->vis_start_index, (this->entries.GetSize() - this->num_vis_entry) + 1);
    }

    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->vis_start_index);
}


VisMultiText::VisMultiText(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str, CGameFont* _font, uint16_t* _clr, int32_t dy)
: VisListBox(_id, l, t, r, b, _font, _clr, nullptr, 0, nullptr) {
    this->text = str;

    if (dy == 0) {
        dy = this->entry_height + 2;
    }

    this->entry_height_full = dy;
    this->entries.Copy( this->font->StringArrayForRect(this->rect, str) );

    if (this->rect.Height() / this->entry_height_full < this->entries.GetSize()) {
        this->num_vis_entry = this->rect.Height() / this->entry_height_full;
    } else {
        this->num_vis_entry = this->entries.GetSize();
    }
}

VisMultiText::VisMultiText(int32_t _id, const RECT& r, const char* str, CGameFont* _font, uint16_t* _clr, int32_t dy)
: VisListBox(_id, r, _font, _clr, nullptr, 0, nullptr) {
    this->text = str;

    if (dy == 0) {
        dy = this->entry_height + 2;
    }

    this->entry_height_full = dy;
    this->entries.Copy(this->font->StringArrayForRect(this->rect, str));

    if (this->rect.Height() / this->entry_height_full < this->entries.GetSize()) {
        this->num_vis_entry = this->rect.Height() / this->entry_height_full;
    } else {
        this->num_vis_entry = this->entries.GetSize();
    }
}

void VisMultiText::SetText(const char* _text) {
    //4d8816
    // Note: vanilla does not store _text into the text member here (only the ctor does);
    // entries are rebuilt from _text but text keeps its ctor value.
    this->entries.Copy(this->font->StringArrayForRect(this->rect, _text));

    if (this->rect.Height() / this->entry_height_full < this->entries.GetSize()) {
        this->num_vis_entry = this->rect.Height() / this->entry_height_full;
    } else {
        this->num_vis_entry = this->entries.GetSize();
    }
}

void VisMultiText::SizesCheck() {
    //4d8a3b
    this->num_vis_entry = this->entries.GetSize();

    if (this->entry_height * num_vis_entry <= this->rect.Height()) {
        this->rect.bottom = this->rect.top + num_vis_entry * entry_height_full;
        return;
    }

    this->rect.right = this->rect.right - 26;

    this->entries.Copy(this->font->StringArrayForRect(this->rect, this->text));
    
    this->scrollbox_id = 0xdf23;

    VisScrollBar* scroll = new VisScrollBar(this->scrollbox_id, this->rect.right, this->rect.top, this->rect.right + 24, this->rect.bottom, nullptr);
    this->parent->AddChild(scroll);

    num_vis_entry = this->rect.Height() / entry_height_full;
}


//4df4d1
VisScreen::VisScreen() = default;

//4df4fa
VisScreen::VisScreen(int32_t _id, const RECT& r, CGameBitmap* _bitmap)
: CVisualObject(_id, r, nullptr) {
    this->bitmap = _bitmap;
    VisScreen::VMethod26();
}

//4df56d
VisScreen::VisScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->bitmap = _bitmap;
    VisScreen::VMethod26();
}

//4df5ec
VisScreen::~VisScreen() {
    VisScreen::VMethod27();
}

//4df63d
void VisScreen::VMethod7() {
    CRect r = this->ClientRectToScreen(this->rect);

    this->VMethod8(&r);

    CVisualObject::VMethod7();
}

//4df67c
void VisScreen::VMethod8(CRect* r) {
    r->IntersectRect(r, &g_ScreenSize);

    if (this->bitmap) {
        //CRect r2;
        //ClientRectToScreen(&r2, rect);

        CRect tmp;
        GetClipRect(&tmp);
        SetClipRect(*r);
        LockSurface2();

        this->bitmap->Draw(this->rect.left, this->rect.top, 0, 0, 0);

        UnlockSurface2();
        SetClipRect(tmp);
    } else {
        CRect r2 = this->ClientRectToScreen(this->rect);

        r2.right -= 8;
        r2.bottom -= 8;

        CRect tmp;
        GetClipRect(&tmp);
        SetClipRect(*r);
        LockSurface2();

        gfx_interface_lm->DrawBrightened(r2.right - 40, r2.top + 8, 3, 6, 0);
        gfx_interface_lm->DrawBrightened(r2.left + 8, r2.bottom - 40, 6, 6, 0);
        gfx_interface_lm->DrawBrightened(r2.right - 40, r2.bottom - 40, 8, 6, 0);

        for (int32_t x = 0; x < (r2.Width() - 96) / 96; x++) {
            gfx_interface_lm->DrawBrightened(r2.left + 56 + x * 96, r2.bottom - 40, 7, 6, 0);
        }

        for (int32_t y = 0; y < (r2.Height() - 96) / 64; y++) {
            gfx_interface_lm->DrawBrightened(r2.right - 40, r2.top + 56 + y * 64, 5, 6, 0);
        }


        gfx_interface_lm->Draw(r2.left, r2.top, 1, 0, 0);
        gfx_interface_lm->Draw(r2.right - 48, r2.top, 3, 0, 0);
        gfx_interface_lm->Draw(r2.left, r2.bottom - 48, 6, 0, 0);
        gfx_interface_lm->Draw(r2.right - 48, r2.bottom - 48, 8, 0, 0);

        for (int32_t x = 0; x < (r2.Width() - 96) / 96; x++) {
            gfx_interface_lm->Draw(r2.left + 48 + x * 96, r2.top, 2, 0, 0);
            gfx_interface_lm->Draw(r2.left + 48 + x * 96, r2.bottom - 48, 7, 0, 0);
        }

        for (int32_t y = 0; y < (r2.Height() - 96) / 64; y++) {
            gfx_interface_lm->Draw(r2.left, r2.top + 48 + y * 64, 4, 0, 0);
            gfx_interface_lm->Draw(r2.right - 48, r2.top + 48 + y * 64, 5, 0, 0);
        }

        for (int32_t x = 0; x < (r2.Width() - 96) / 96; x++) {
            for (int32_t y = 0; y < (r2.Height() - 96) / 64; y++) {
                gfx_interface_lm->Draw(r2.left + 48 + x * 96, r2.top + 48 + y * 64, 0, 0, 0);
            }
        }

        UnlockSurface2();
        SetClipRect(tmp);
    }
}


int32_t VisScreen::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    //4dfbcd
    if (msg == VIS_MSG_HIDE_SCREEN || msg == VIS_MSG_CLOSE_SCREEN) {
        if (this->is_active) {
            this->DoClose(msg);
            AfxGetMainWnd()->PostMessageA(MAIN_MSG_CHILD_CLOSED, (WPARAM)this, 0);
        }
        return 1;
    } else {
        return CVisualObject::MsgProc(msg, wparam, lparam);
    }
}

int32_t VisScreen::OnLButtonDown(uint32_t wparam, CPoint pos) {
    //4dfee7
    if (this->cursor_over_obj_last == this->cursor_over_obj) {
        this->cursor_over_obj_last = nullptr;
        CVisualObject* obj = this->GetChildAt(pos);
        if (obj && obj != this) {
            return obj->OnLButtonDown(wparam, pos);
        }
    }

    return CVisualObject::OnLButtonDown(wparam, pos);
}

int32_t VisScreen::OnKeyDown(uint32_t wparam) {
    //4dfc43
    switch (wparam) {
    case VK_TAB:
        this->TabFocus(g_kbShiftState == 0, true);
        return 1;

    case VK_LEFT:
        if (this->focus_obj && focus_obj->left_obj) {
            this->FocusTo(focus_obj->left_obj, true);
            return 1;
        }
        break;

    case VK_UP:
        if (focus_obj && focus_obj->up_obj) {
            this->FocusTo(focus_obj->up_obj, true);
            return 1;
        }
        break;

    case VK_RIGHT:
        if (focus_obj && focus_obj->right_obj) {
            this->FocusTo(focus_obj->right_obj, true);
            return 1;
        }
        break;

    case VK_DOWN:
        if (focus_obj && focus_obj->down_obj) {
            this->FocusTo(focus_obj->down_obj, true);
            return 1;
        }
        break;

    default:
        break;
    }

    return 0;
}


void VisScreen::VMethod26() {
    //450900
}

void VisScreen::VMethod27() {
    //4388c0
}

void VisScreen::VMethod28() {
    //4dfb4f
    this->is_active = 1;
    this->SetCursorOver(true);
    this->SetFocus(true);
    this->TabFocus(true, false);
}

void VisScreen::DoClose(uint32_t code) {
    //4dfb8a 29 method
    if (this->is_active) {
        this->is_active = 0;
        this->SetCursorOver(false);
        this->SetFocus(false);
        this->exit_code = code;
    }
}

int32_t VisScreen::GetCloseCode() {
    //4388d0
    return this->exit_code;
}

void VisScreen::CloseOk() {
    //4327b5
    this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
}

void VisScreen::CloseCancel() {
    //4327d4
    this->MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
}


VisWindow::~VisWindow()  //451a70
{}

int32_t VisWindow::OnKeyDown(uint32_t wparam) {
    //4dfeb2
    if (wparam == VK_ESCAPE) {
        return this->MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
    } else {
        return VisScreen::OnKeyDown(wparam);
    }
}

VisWindow::VisWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap)
:VisScreen(_id, l, t, r, b, _bitmap) {
    //4dfdab
    VisWindow::UpdateWinRect();
}

void VisWindow::UpdateWinRect() {
    //4dfe19
    int32_t w = ((this->rect.Width() - 8) / 96) * 96 + 8;
    int32_t h = ((this->rect.Height() - 104) / 64) * 64 + 104;

    this->rect.left = (g_ScreenSize.right - w) / 2;
    this->rect.top = (g_ScreenSize.bottom - h) / 2;
    this->rect.right = this->rect.left + w;
    this->rect.bottom = this->rect.top + h;
}


// 4e0071
int32_t VisRoleKeyScreen::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg != VIS_MSG_SPEECH_ADVANCE) {
        return this->VisScreen::MsgProc(msg, wparam, lparam);
    }

    if (this->AdvanceSpeechLine()) {
        CVisualObject* textbox = this->FindChild(10);
        textbox->VMethod9();
        CVisualObject* portrait = this->FindChild(12);
        if (portrait != nullptr) {
            portrait->VMethod9();
        }
        return 1;
    }

    if (this->tips_id != 0) {
        AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_TIPS, this->tips_id, 0);
    }
    return this->VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
}


// 4e0027
int32_t VisRoleKeyScreen::OnKeyDown(uint32_t wparam) {
    if (wparam == VK_RETURN || wparam == VK_ESCAPE) {
        this->MsgProc(VIS_MSG_SPEECH_ADVANCE, 0, 0);
        return 1;
    }
    return this->VisScreen::OnKeyDown(wparam);
}


// 4e012d
void VisRoleKeyScreen::VMethod28() {
    this->AdvanceSpeechLine();
    this->VisScreen::VMethod28();
}


// 4e0148 (helper 4e0b20 inlined)
void VisRoleKeyScreen::DoClose(uint32_t code) {
    if (this->text_buf != nullptr) {
        free(this->text_buf);
    }
    this->VisScreen::DoClose(code);
}


// 4e0169
bool VisRoleKeyScreen::AdvanceSpeechLine() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    this->line_no = this->line_no + 1;

    if (this->speech_sample != nullptr) {
        SoundChannel* channel = this->speech_sample->FindPlayingChannel();
        if (channel != nullptr) {
            channel->Stop();
            channel->psample = nullptr;
        }
        delete this->speech_sample;
        this->speech_sample = nullptr;
    }

    CString text;
    int32_t id;
    char* name;
    char* name_end;
    int32_t music_id;
    if (!this->ParseSpeechLine(this->line_no, &id, &text, &name, &name_end, &music_id)) {
        return false;
    }

    VisMultiText* textbox = (VisMultiText*)this->FindChild(10);
    char first_char = *name_end;
    *name_end = '\0';
    textbox->SetText(name);
    *name_end = first_char;

    if (text.GetLength() == 0) {
        char gender = 0;
        if (this->spoken_by_player != 0 || id == 0x15) {
            CUnit* unit = main_wnd->vis_map_context->FindOrCreateUnitByServerId(0x15);
            if ((unit->unitFlags & UNIT_FLAG_FEMALE) == 0) {
                gender = (unit->unitFlags & UNIT_FLAG_MAGE) != 0 ? 'm' : 'f';
            } else {
                gender = (unit->unitFlags & UNIT_FLAG_MAGE) != 0 ? 's' : 'a';
            }
        }

        int32_t pos;
        if ((pos = this->role_name.Find("talk")) >= 0) {
            if (gender == 0) {
                gender = 't';
            }
            CString num = this->role_name.Mid(pos + 4, 5);
            while (num.GetLength() < 3) {
                num = "0" + num;
            }
            text.Format("talk\\%04d%c%sp%02d", id, gender, (const char*)num, this->line_no);
        } else if ((pos = this->role_name.Find("about")) >= 0) {
            text.Format("about\\%04dt000p%02d", id, this->line_no);
        } else if ((pos = this->role_name.Find("accept")) >= 0) {
            if (gender == 0) {
                gender = 't';
            }
            CString num = this->role_name.Mid(pos + 6, 5);
            while (num.GetLength() < 3) {
                num = "0" + num;
            }
            text.Format("accept\\%04d%c%sp%02d", id, gender, (const char*)num, this->line_no);
        } else if ((pos = this->role_name.Find("reject")) >= 0) {
            if (gender == 0) {
                gender = 't';
            }
            CString num = this->role_name.Mid(pos + 6, 5);
            while (num.GetLength() < 3) {
                num = "0" + num;
            }
            text.Format("reject\\%04d%c%sp%02d", id, gender, (const char*)num, this->line_no);
        } else if (this->role_name.Find("keeper") >= 0 || this->role_name.Find("man") >= 0 || this->role_name.Find("guard") >= 0) {
            int32_t len = this->role_name.GetLength();
            while (!isalpha(this->role_name[len - 1])) {
                len = len - 1;
            }
            if (gender == 0) {
                gender = 't';
            }
            CString num = this->role_name.Mid(len, 5);
            while (num.GetLength() < 3) {
                num = "0" + num;
            }
            text.Format("town\\%04d%c%sp%02d", id, gender, (const char*)num, this->line_no);
        }
    }

    if (text.GetLength() != 0) {
        this->speech_sample = new SfxSample("speech\\" + text + ".wav");
        this->speech_sample->LoadSample();
        this->speech_sample->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
    }

    if (this->is_npc != 0) {
        VisBitmap* portrait = (VisBitmap*)this->FindChild(12);
        CGameBitmap* bmp = portrait->GetBitmap();
        if (bmp != nullptr) {
            delete bmp;
        }
        portrait->SetBitmap(main_wnd->vis_map_context->BuildUnitPortrait(id));
    }

    if (music_id >= 0) {
        main_wnd->music_player->QueueTrack(music_id);
    }

    return true;
}


// 4e0b43
bool VisRoleKeyScreen::ParseSpeechLine(int32_t line_no, int32_t* out_id, CString* out_text, char** out_name, char** out_name_end, int32_t* out_music) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    char* p = this->text_buf;

    while (true) {
        while (*p != '<' && *p != '\0') {
            p++;
        }
        if (*p == '\0') {
            return false;
        }
        char* tag_start = p + 1;
        while (*p != '>' && *p != '\0') {
            p++;
        }
        if (*p == '\0') {
            return false;
        }
        char tag_close = *p;
        *p = '\0';
        CString tag(tag_start);
        tag.MakeLower();
        *p = tag_close;

        CString num;
        num.Format("part=%d", line_no);
        if (tag.Find(num) == -1) {
            if (*p == '\0') {
                return false;
            }
            continue;
        }

        int32_t pos = tag.Find("npc=");
        this->is_npc = pos >= 0;
        if (this->is_npc != 0) {
            num = tag.Mid(pos + 4, 5);
            sscanf(num, "%d", out_id);
        } else {
            *out_id = 0;
        }

        if (tag.Find("iamfemale") >= 0 || tag.Find("iammale") >= 0 || tag.Find("iammage") >= 0 || tag.Find("iamfighter") >= 0) {
            this->spoken_by_player = 1;
        } else {
            this->spoken_by_player = 0;
        }

        uint32_t unit_flags = main_wnd->vis_map_context->my_unit->unitFlags;
        if ((tag.Find("iamfemale") >= 0 && (unit_flags & 4) == 0) ||
            (tag.Find("iammale") >= 0 && (unit_flags & 4) != 0) ||
            (tag.Find("iammage") >= 0 && (unit_flags & 2) == 0) ||
            (tag.Find("iamfighter") >= 0 && (unit_flags & 2) != 0)) {
            if (*p == '\0') {
                return false;
            }
            continue;
        }

        pos = 0;
        while (true) {
            CString tail = tag.Mid(pos);
            pos = tail.Find("npcalive=");
            if (pos == -1) {
                break;
            }
            pos = pos + 9;
            num = tag.Mid(pos, 5);
            int32_t unit_id = 0;
            sscanf(num, "%d", &unit_id);
            CUnit* unit = main_wnd->vis_map_context->FindUnitByServerId(unit_id);
            if (unit == nullptr || pos < 0) {
                break;
            }
        }
        if (pos >= 0) {
            if (*p == '\0') {
                return false;
            }
            continue;
        }

        pos = 0;
        while (true) {
            CString tail = tag.Mid(pos);
            pos = tail.Find("npcdead=");
            if (pos == -1) {
                break;
            }
            pos = pos + 9;
            num = tag.Mid(pos, 5);
            int32_t unit_id = 0;
            sscanf(num, "%d", &unit_id);
            CUnit* unit = main_wnd->vis_map_context->FindUnitByServerId(unit_id);
            if (unit != nullptr || pos < 0) {
                break;
            }
        }
        if (pos >= 0) {
            if (*p == '\0') {
                return false;
            }
            continue;
        }

        pos = tag.Find("sound=");
        if (pos == -1) {
            *out_text = "";
        } else {
            pos = pos + 6;
            if (tag[pos] == '"') {
                *out_text = tag.Mid(pos + 1);
                *out_text = out_text->Left(out_text->Find('"'));
            } else {
                *out_text = tag.Mid(pos);
                *out_text = out_text->Left(out_text->Find(';'));
            }
        }

        pos = tag.Find("tune=");
        if (pos == -1) {
            *out_music = -1;
        } else {
            num = tag.Mid(pos + 5, 5);
            sscanf(num, "%d", out_music);
        }

        pos = tag.Find("tips=");
        if (pos >= 0) {
            num = tag.Mid(pos + 5, 5);
            sscanf(num, "%d", &this->tips_id);
        }

        while (*p != '\n' && *p != '\0') {
            p++;
        }
        if (*p == '\0') {
            return false;
        }
        p++;
        *out_name = p;
        while (*p != '<' && *p != '\0') {
            p++;
        }
        if (*p == '<') {
            while (*p != '\r') {
                p--;
            }
        }
        *out_name_end = p;
        return true;
    }
}


// 4e0978
void VisRoleKeyScreen::LoadText() {
    CString key;
    key.Format("#%s", (const char*)this->role_name);
    key.MakeLower();

    int32_t pos = g_MissionText.Find(key);
    if (pos == -1) {
        this->text_len = 1;
        this->text_buf = (char*)malloc(this->text_len);
        this->text_buf[0] = '\0';
        this->is_npc = 0;
        return;
    }

    CString body = g_MissionText.Mid(pos + key.GetLength() + 2);
    int32_t hash_pos = body.Find('#');
    if (hash_pos == -1) {
        hash_pos = body.GetLength();
    }
    body = body.Left(hash_pos);

    this->text_len = hash_pos + 1;
    this->text_buf = (char*)malloc(this->text_len);
    strcpy(this->text_buf, body);
    body.MakeLower();
    this->is_npc = body.Find("npc") >= 0;
}


// 4e3f60
VisRoleKeyScreen::~VisRoleKeyScreen()
{}

// 4dff5c
VisRoleKeyScreen::VisRoleKeyScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CString& text)
: VisWindow(_id, l, t, r, b, nullptr) {
    this->role_name = text;
    this->line_no = 0;
    this->speech_sample = nullptr;
    this->text_buf = nullptr;
    this->text_len = 0;
    this->tips_id = 0;
    this->LoadText();
}


VisMessageBox::~VisMessageBox() {
    //444ff3
}

int32_t VisMessageBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    //44500f
    if (msg >= 0x445 && msg <= 0x44b) {
        int32_t res = VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, wparam, lparam);
        this->exit_code = msg;
        this->VMethod31(msg);
        return res;
    } else {
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
}


void VisMessageBox::VMethod26() {
    //444086
    // client: 446656
    CRect r2;
    if (this->text && strlen(this->text) > 0) {
        if (g_font1->GetStrWidth(this->text) > this->rect.Width()) {
            VisMultiText* txt = new VisMultiText(0, 40, 32, this->rect.Width() - 50, 104, this->text, g_font1, p_clrsh_Black, 0);

            this->AddChild(txt);

            txt->SizesCheck();

            r2 = CRect(40, 116, this->rect.Width() - 40, this->rect.Height() - 112);
        } else {
            VisLabel* lbl = new VisLabel(1, 40, 32, this->rect.Width() - 40, 56, this->text, g_font1, p_clrsh_Black, 2);

            this->AddChild(lbl);

            r2 = CRect(40, 68, this->rect.Width() - 40, this->rect.Height() - 112);
        }
    } else {
        r2 = CRect(40, 56, this->rect.Width() - 40, this->rect.Height() - 88);
    }

    if (this->title && strlen(this->title) > 0) {
        r2.OffsetRect(0, 12);
        
        this->AddChild( new VisLabel(-1, r2.left, r2.top - 20, r2.right, r2.top - 4, this->title, g_font1, p_clrsh_Black, 0) );
    }

    CVisualObject* obj = this->VMethod30(this->payload, r2);
    this->rect.bottom = this->rect.top + 144 + obj->GetRect().bottom;
    this->UpdateWinRect();
#ifdef A2CLIENT
    if (this->button_types == 0x1000) {
        this->rect.OffsetRect(0, 0x84);
    }
#endif

    CRect local_20(this->rect.Width() / 2 - 48, this->rect.Height() - 60, this->rect.Width() / 2 + 48, this->rect.Height() - 36);
    CRect local_44(this->rect.Width() / 7, this->rect.Height() - 60, (this->rect.Width() * 3) / 7, this->rect.Height() - 36);
    CRect local_54((this->rect.Width() * 4) / 7, this->rect.Height() - 60, (this->rect.Width() * 6) / 7, this->rect.Height() - 36);
    CRect local_64((this->rect.Width() * 3) / 20, this->rect.Height() - 60, (this->rect.Width() * 7) / 20, this->rect.Height() - 36);
    CRect local_74((this->rect.Width() * 8) / 20, this->rect.Height() - 60, (this->rect.Width() * 12) / 20, this->rect.Height() - 36);
    CRect local_84((this->rect.Width() * 13) / 20, this->rect.Height() - 60, (this->rect.Width() * 17) / 20, this->rect.Height() - 36);

    switch (this->button_types) {
    case 0:
    case 0x1000: {
        //ok
        VisButton* btn = new VisButton(4, local_20, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "");
        this->AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);
    }
        break;

    case 1: {
        //ok cancel
        VisButton* btn = new VisButton(4, local_44, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "");
        this->AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);

        VisButton* btn2 = new VisButton(5, local_54, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
        this->AddChild(btn2);

        btn->SetRightObj(btn2);
    }
        break;

    case 2: {
        //cancel repeat ignore
        VisButton* btn = new VisButton(4, local_64, txt_dialogs.GetLine(2), g_font1, nullptr, 0x449, 0, "");
        this->AddChild(btn);

        VisButton* btn2 = new VisButton(5, local_74, txt_dialogs.GetLine(3), g_font1, nullptr, 0x44a, 0, "");
        this->AddChild(btn2);
        btn2->ChangeFlags(FLAG_10, true);

        VisButton* btn3 = new VisButton(6, local_84, txt_dialogs.GetLine(4), g_font1, nullptr, 0x44b, 0, "");
        this->AddChild(btn3);

        btn->SetRightObj(btn2);
        btn2->SetRightObj(btn3);
    }
        break;

    case 3: {
        //yes no cancel
        VisButton* btn = new VisButton(4, local_64, txt_main.GetLine(75), g_font1, nullptr, 0x447, 0, "");
        this->AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);

        VisButton* btn2 = new VisButton(5, local_74, txt_main.GetLine(76), g_font1, nullptr, 0x448, 0, "");
        this->AddChild(btn2);

        VisButton* btn3 = new VisButton(6, local_84, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
        this->AddChild(btn3);

        btn->SetRightObj(btn2);
        btn2->SetRightObj(btn3);
    }
        break;

    case 4: {
        //yes no
        VisButton* btn = new VisButton(4, local_44, txt_main.GetLine(75), g_font1, nullptr, 0x447, 0, "");
        this->AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);

        VisButton* btn2 = new VisButton(5, local_54, txt_main.GetLine(76), g_font1, nullptr, 0x448, 0, "");
        this->AddChild(btn2);

        btn->SetRightObj(btn2);
    }
        break;

    case 5: {
        //repeat cancel
        VisButton* btn = new VisButton(4, local_44, txt_dialogs.GetLine(3), g_font1, nullptr, 0x44a, 0, "");
        this->AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);

        VisButton* btn2 = new VisButton(5, local_54, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
        this->AddChild(btn2);

        btn->SetRightObj(btn2);
    }
        break;

    //from allods2.exe
    case 0x2000: {
        //ok
        VisButton* btn = new VisButton(4, local_20, txt_dialogs.GetLine(0), g_font1, nullptr, 0x447, 0, "");
        this->AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);
    }
        break;


    default:
        break;
    }

    CVisualObject* dobj = this->FindChild(5);
    if (dobj) {
        obj->SetDownObj(dobj);
    }

    dobj = this->FindChild(6);
    if (dobj) {
        obj->SetDownObj(dobj);
    }

    dobj = this->FindChild(4);
    if (dobj) {
        obj->SetDownObj(dobj);
    }
}


void VisMessageBox::VMethod31(int32_t code) {
    //445084
}

VisMessageBox::VisMessageBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const void* _payload, const char* str2, int32_t btypes, const char* str3)
: VisWindow(_id, l, t, r, b, nullptr) {
    //44402b
    this->title = str2;
    this->text = str3;
    this->payload = _payload;
    this->button_types = btypes;
}


CVisualObject* VisMessageBoxWithList::VMethod30(const void* str, const RECT& r) { //4450d4
    VisMultiText* txt = new VisMultiText(2, r, (const char*)str, g_font1, p_clrsh_Black, 0);
    this->AddChild(txt);
    txt->SizesCheck();
    return txt;
}

VisMessageBoxWithList::VisMessageBoxWithList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str1, const char* str2, int32_t btypes)
: VisMessageBox(_id, l, t, r, b, str1, str2, btypes, nullptr) { //445091
}






// 44D8F0
int32_t VisNetMapSelection::OnKeyDown(uint32_t wparam) {
    return this->VisWindow::OnKeyDown(wparam);
}


// 44aad6
UINT AFX_CDECL VisNetMapThreadProc(LPVOID pParam) {
    static_cast<VisNetMapSelection*>(pParam)->ScanMapsThread();
    return 0;
}


// 44AAE5
void VisNetMapSelection::ScanMapsThread() {
    char path[MAX_PATH];
    WIN32_FIND_DATAA find_data;
    AppGetWorkingDir(0x103, path);
    strcat(path, "\\*.alm");
    HANDLE find_handle = FindFirstFileA(path, &find_data);
    if (find_handle != INVALID_HANDLE_VALUE) {
        do {
            if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                NetMapInfo* info = new NetMapInfo;
                info->mapid = find_data.cFileName;
                Scenario* scenario = new Scenario;
                if (scenario->ReadAlmInfo(info)) {
                    bool name_printable = false;
                    const char* psz = (LPCTSTR)info->name;
                    if (psz != nullptr) {
                        for (; *psz != '\0'; psz++) {
                            if (isprint(*psz) || *psz < 0) {
                                name_printable = true;
                                break;
                            }
                        }
                    }
                    if (!name_printable) {
                        info->name = find_data.cFileName;
                    }
                    bool tooltip_printable = false;
                    psz = (LPCTSTR)info->tooltip;
                    if (psz != nullptr) {
                        for (; *psz != '\0'; psz++) {
                            if (isprint(*psz) || *psz < 0) {
                                tooltip_printable = true;
                                break;
                            }
                        }
                    }
                    if (!tooltip_printable) {
                        info->tooltip = g_EmptyStrMapTooltip;
                    }
                    info->name.OemToAnsi();
                    info->tooltip.OemToAnsi();
                    if (info->tooltip.IsEmpty()) {
                        info->tooltip = info->name;
                    }
                    delete scenario;
                    PostMessageA(g_MainWndHWND, 0x461, (WPARAM)info, 0);
                } else {
                    delete scenario;
                    delete info;
                }
            }
        } while (FindNextFileA(find_handle, &find_data) && WaitForSingleObject(this->stop_event, 0) != 0);
    }
    FindClose(find_handle);
}


// 44AF45
VisNetMapSelection::VisNetMapSelection(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CString* pMapName)
: VisWindow(_id, l, t, r, b, nullptr), thread(VisNetMapThreadProc, this) {
    this->p_mapname = pMapName;
}


// 44AFD8 (scalar deleting dtor ??_G at 44FEA0)
VisNetMapSelection::~VisNetMapSelection() {
    for (int32_t i = 0; i < this->avail_maps.GetSize(); i++) {
        delete this->avail_maps.GetAt(i);
    }
}


// 44AEEF
void VisNetMapSelection::StopThread() {
    SetEvent(this->stop_event);
    if (this->thread.m_hThread != nullptr) {
        WaitForSingleObject(this->thread.m_hThread, INFINITE);
    }
    CloseHandle(this->stop_event);
    this->stop_event = nullptr;
}


// 44D13D
void VisNetMapSelection::AddNetMapInfo(NetMapInfo* info) {
    this->avail_maps.Add(info);
    VisListBox* map_list = (VisListBox*)this->FindChild(1);
    VisScrollBar* scrollbar = (VisScrollBar*)this->FindChild(0xA);
    int32_t prev_sel = map_list->GetSelectedIndex();
    char buf[1024];
    sprintf(buf, "%s#%dx%d#%d#%d", (LPCTSTR)info->name, info->width - 0x10, info->height - 0x10,
            info->players, info->maplevel);
    map_list->AddItem(buf);
    if (prev_sel < 0) {
        prev_sel = prev_sel + 1;
        map_list->SetSelectedIndex(prev_sel);
    }
    scrollbar->SetPos(prev_sel, this->avail_maps.GetSize());
    map_list->VMethod9();
    if (prev_sel == 0) {
        PostMessageA(g_MainWndHWND, 0x46E, 1, 0);
    }
}


// 44C788
void VisNetMapSelection::VMethod26() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->mode = (main_wnd->sessionMode != SESSION_SINGLEPLAYER);

    this->AddChild(new VisLabel(0x19, 0x28, 0x16, this->rect.Width() - 0x28, 0x38,
                                txt_dialogs.GetLine(0x8A), g_font1, p_clrsh_Black, 2));
    this->AddChild(new VisLabel(0x1A, 0x28, 0x2C, this->rect.Width() - 0x28, 0x44,
                                txt_dialogs.GetLine(0x8B), g_font1, p_clrsh_Black, 0));
    VisNetMapList* map_list = new VisNetMapList(1, 0x28, 0x44, this->rect.Width() - 0x40, this->rect.Height() / 2,
                                                g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0xA,
                                                txt_dialogs.GetLine(0x76), &this->selected_map_index);
    this->AddChild(map_list);
    map_list->SetCaptionLabel((VisLabel*)this->FindChild(0x1A));
    CRect list_rc = map_list->GetRect();
    this->AddChild(new VisScrollBar(0xA, list_rc.right, list_rc.top, list_rc.right + 0x18, list_rc.bottom, nullptr));

    list_rc.top = 0x11C;
    list_rc.bottom = 0x180;
    list_rc.right = 0x15E;
    this->AddChild(new VisLabel(0x1B, list_rc.left, list_rc.top - 0x18, list_rc.right, list_rc.top - 6,
                                txt_dialogs.GetLine(0x8C), g_font1, p_clrsh_Black, 0));
    VisNetChatList* chat_list = new VisNetChatList(3, list_rc, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0xB,
                                                   txt_dialogs.GetLine(0x84));
    this->AddChild(chat_list);
    chat_list->SetCaptionLabel((VisLabel*)this->FindChild(0x1B));
    list_rc = chat_list->GetRect();
    this->AddChild(new VisScrollBar(0xB, list_rc.right, list_rc.top, list_rc.right + 0x18, list_rc.bottom, nullptr));

    CPoint btn_center(this->rect.Width() / 2, this->rect.Height() - 0x30);
    CRect btn_rc(btn_center.x - 0x30, btn_center.y - 0xC, btn_center.x + 0x30, btn_center.y + 0xC);
    OffsetRect(&btn_rc, -0x48, 0);
    VisButton* ok_btn = new VisButton(0x14, btn_rc, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "");
    this->AddChild(ok_btn);
    if (this->mode == 0) {
        ok_btn->ChangeFlags(1, 0);
    }

    list_rc.top = list_rc.bottom + 0xC;
    list_rc.bottom = list_rc.bottom + 0x24;
    VisNetChatTextBox* chat_txt = new VisNetChatTextBox(4, list_rc, g_font1, p_clrsh_Black,
                                                        txt_dialogs.GetLine(0x85));
    this->AddChild(chat_txt);
    chat_txt->SetUpObj(chat_list);

    list_rc.left = 0x181;
    list_rc.right = 0x20C;
    list_rc.top = 0x11C;
    list_rc.bottom = 0x1A4;
    this->AddChild(new VisLabel(0x1C, list_rc.left, list_rc.top - 0x18, list_rc.right, list_rc.top - 6,
                                txt_dialogs.GetLine(0x8D), g_font1, p_clrsh_Black, 0));
    VisNetPlayerList* player_list = new VisNetPlayerList(5, list_rc, g_font1, p_clrsh_Black, p_clrsh_Black, 0xC,
                                                         txt_dialogs.GetLine(0x89));
    this->AddChild(player_list);
    player_list->SetCaptionLabel((VisLabel*)this->FindChild(0x1C));
    PostMessageA(g_MainWndHWND, 0x460, 0, 0);
    list_rc = player_list->GetRect();
    this->AddChild(new VisScrollBar(0xC, list_rc.right, list_rc.top, list_rc.right + 0x18, list_rc.bottom, nullptr));

    OffsetRect(&btn_rc, 0x90, 0);
    VisButton* cancel_btn = new VisButton(0x15, btn_rc, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    this->AddChild(cancel_btn);
    cancel_btn->SetLeftObj(this->FindChild(0x14));

    this->stop_event = CreateEventA(nullptr, 0, 0, nullptr);
    if (this->stop_event == nullptr) {
        AfxThrowMemoryException();
        return;
    }
    this->map_context = main_wnd->vis_map_context;
    this->chat_log = &this->map_context->msglog;
    this->selected_map_index = 0;
    this->thread.CreateThread(0, 0, nullptr);
    this->thread.m_bAutoDelete = 0;
}


// 44D275
int32_t VisNetMapSelection::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    switch (msg) {
    case VIS_MSG_LIST_ACTIVATE:
        if (wparam != 1) {
            return 0;
        }
        // fallthrough
    case VIS_MSG_HIDE_SCREEN:
        if (this->FindChild(0x14)->TestFlags(1)) {
            this->StopThread();
            this->VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
            VisListBox* map_list = (VisListBox*)this->FindChild(1);
            NetMapInfo* info = this->avail_maps.GetAt(map_list->GetSelectedIndex());
            *this->p_mapname = info->mapid;
        }
        return 1;
    case VIS_MSG_CLOSE_SCREEN:
        this->StopThread();
        this->VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
        return 1;
    case VIS_MSG_NETMAP_APPLY:
        if (main_wnd->sessionMode == SESSION_SINGLEPLAYER) {
            this->StopThread();
            if (this->mode == 0) {
                VisListBox* map_list = (VisListBox*)this->FindChild(1);
                CString row = map_list->GetItem(this->selected_map_index);
                int32_t level = row[row.GetLength() - 1] - '1';
                if (level < 0) {
                    level = 0;
                }
                if (level > 3) {
                    level = 3;
                }
                bool can_join = (level + 1 >= main_wnd->m_GameSession.GetSkillTier1())
                    && (main_wnd->m_GameSession.GetSkillTier2() >= level + 1);
                this->VisScreen::MsgProc(can_join ? 0x445 : 0x446, 0, 0);
            } else {
                this->VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
            }
        }
        return 1;
    case VIS_MSG_NETMAP_CHAT_SYNC: {
        VisNetChatList* chat_list = (VisNetChatList*)this->FindChild(3);
        int32_t count = chat_list->GetItemCount();
        int32_t sel = chat_list->GetSelectedIndex();
        VisScrollBar* scrollbar = (VisScrollBar*)this->FindChild(0xB);
        scrollbar->SetPos(sel, count);
        int32_t size = this->chat_log->text.GetSize();
        chat_list->AddItem(this->chat_log->text[size - 1]);
        chat_list->colors.Add(this->chat_log->color.GetAt(size - 1));
        if (chat_list->GetItemCount() - 2 == chat_list->selected_index) {
            chat_list->SelectItem(chat_list->vis_start_index + chat_list->num_vis_entry);
        }
        chat_list->VMethod9();
        return 1;
    }
    case VIS_MSG_ROSTER_CHANGED: {
        BigStruct2* map_context = main_wnd->vis_map_context;
        VisNetPlayerList* player_list = (VisNetPlayerList*)this->FindChild(5);
        player_list->entries.RemoveAll();
        player_list->selected_index = -1;
        player_list->colors.RemoveAll();
        for (int32_t i = 0x10; i < map_context->map_players.GetSize(); i++) {
            MapPlayerData* pd = map_context->map_players.GetAt(i);
            if (pd != nullptr) {
                player_list->AddItem(pd->name);
                player_list->colors.Add(g_colors_human_pals + pd->color);
            }
        }
        player_list->VMethod9();
        return 1;
    }
    case VIS_MSG_NETMAP_ADD_INFO:
        this->AddNetMapInfo((NetMapInfo*)wparam);
        return 1;
    case VIS_MSG_SET_OPTION:
    case VIS_MSG_LIST_SELECT:
        if (wparam == 1) {
            if (lparam >= 0 && this->mode != 0) {
                this->selected_map_index = lparam;
                VisListBox* map_list = (VisListBox*)this->FindChild(1);
                map_list->VMethod9();
                NetMapInfo* info = this->avail_maps.GetAt(this->selected_map_index);
                this->map_context->SendMapAnnouncement((LPCTSTR)info->name);
                CString row = map_list->GetItem(lparam);
                int32_t level = row[row.GetLength() - 1] - '1';
                if (level < 0) {
                    level = 0;
                }
                if (level > 3) {
                    level = 3;
                }
                bool can_join = (level + 1 >= main_wnd->m_GameSession.GetSkillTier1())
                    && (main_wnd->m_GameSession.GetSkillTier2() >= level + 1);
                this->FindChild(0x14)->ChangeFlags(1, can_join);
                this->FindChild(0x14)->VMethod9();
            }
            return 1;
        }
        return 0;
    case VIS_MSG_NETMAP_FIND_MAP:
        if (this->mode == 0) {
            if (lparam >= 0 && lparam < (uint32_t)this->avail_maps.GetSize()) {
                for (int32_t i = 0; i < this->avail_maps.GetSize(); i++) {
                    if (_strcmpi((const char*)wparam, (LPCTSTR)this->avail_maps.GetAt(i)->name) == 0) {
                        this->selected_map_index = i;
                        break;
                    }
                }
            }
            this->FindChild(1)->VMethod9();
        }
        return 1;
    default:
        return this->VisScreen::MsgProc(msg, wparam, lparam);
    }
}


// 450080 (the CPtrArray `colors` member at 0x94 is member-constructed by the compiler)
VisNetChatList::VisNetChatList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
    : VisListBox(_id, r, _font, _clr1, _clr2, _scrollid, hint) {
}


// 450100 (deleting dtor ??_G; the complete dtor 450130 only destroys `colors` and the base)
VisNetChatList::~VisNetChatList() = default;


// 44c63d
void VisNetChatList::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    if (!this->IsValidIndex(idx)) {
        return;
    }

    const char* txt = this->entries[idx];

    CRect client;
    this->ClientRectToScreen(&client, this->rect);

    CRect saved;
    GetClipRect(&saved);

    CRect clip(pos.x, pos.y, client.right - 6, pos.y + this->font->GetHeight() + 2);
    SetClipRect(clip);

    this->font->DrawTextWithShadow(pos.x, pos.y, txt, 0, (uint16_t*)this->colors[idx], 1);

    SetClipRect(saved);
}


// 450180 (the CPtrArray `colors` member at 0x94 is member-constructed by the compiler)
VisNetPlayerList::VisNetPlayerList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
    : VisListBox(_id, r, _font, _clr1, _clr2, _scrollid, hint) {
}


// 450200 (deleting dtor ??_G; the complete dtor 450230 only destroys `colors` and the base)
VisNetPlayerList::~VisNetPlayerList() = default;


// 44c71d
void VisNetPlayerList::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    if (!this->IsValidIndex(idx)) {
        return;
    }

    const char* txt = this->entries[idx];
    this->font->DrawTextWithShadow(pos.x, pos.y, txt, 0, (uint16_t*)this->colors[idx], 1);
}


// 44ffd0
VisNetMapList::VisNetMapList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint, int32_t* p_selected_map_index)
    : VisListBox(_id, l, t, r, b, _font, _clr1, _clr2, _scrollid, hint) {
    this->p_selected_map_index = p_selected_map_index;
}


// 450030 (deleting dtor ??_G; the complete dtor 450060 only calls the base dtor)
VisNetMapList::~VisNetMapList() = default;


// 44b09e
void VisNetMapList::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    if (!this->IsValidIndex(idx)) {
        return;
    }

    if (idx == *this->p_selected_map_index) {
        g_bmp_server->Draw(pos.x, pos.y, 0, 0, 0);
    }

    CString& item = this->entries[idx];
    char* txt = (char*)(const char*)item;

    int32_t level = item[item.GetLength() - 1] - '1';
    if (level < 0) {
        level = 0;
    }
    if (level > 3) {
        level = 3;
    }

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->m_GameSession.GetSkillTier1() > level + 1
        || main_wnd->m_GameSession.GetSkillTier2() < level + 1) {
        clr = g_colors_human_pals[15];
    }

    pos.x += 0x1E;

    char* field = txt;
    char* sep = strchr(field, '#');
    *sep = '\0';
    this->font->DrawTextWithShadow(pos.x, pos.y, field, 0, clr, 1);
    *sep = '#';

    pos.x += 0x10E;
    field = sep + 1;
    sep = strchr(field, '#');
    *sep = '\0';
    this->font->DrawTextWithShadow(pos.x, pos.y, field, 0, clr, 1);
    *sep = '#';

    pos.x += 0x54;
    field = sep + 1;
    sep = strchr(field, '#');
    *sep = '\0';
    *sep = '#';
    this->font->DrawTextWithShadow(pos.x, pos.y, txt_patch.GetLine(level + 0x62), 0, clr, 1);
}

// 44b2bf
const char* VisNetMapList::GetHint() {
    CRect client;
    this->ClientRectToScreen(&client, this->rect);

    if (g_mousept.x < client.left + 0x12C) {
        int32_t idx = this->vis_start_index + this->YToIndex(g_mousept.y);
        if (!this->IsValidIndex(idx)) {
            return nullptr;
        }

        VisNetMapSelection* screen = (VisNetMapSelection*)this->parent;
        NetMapInfo* info = screen->avail_maps[idx];
        return info->tooltip;
    }

    if (g_mousept.x < client.left + 0x180) {
        return txt_dialogs.GetLine(0x86);
    }

    return txt_dialogs.GetLine(0x88);
}


// 450920
VisServerScreenRadio::VisServerScreenRadio(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
    : VisRadioType1(_id, l, t, r, b, _font, _clr, hint) {
}


// 450970 (deleting dtor ??_G; the complete dtor 4509a0 only calls the base dtor)
VisServerScreenRadio::~VisServerScreenRadio() = default;


// 4db3ad
void VisServerScreenRadio::VMethod7() {
    CRect r;
    this->ClientRectToScreen(&r, this->rect);
    r.bottom += 4;
    r.right += 4;

    int32_t txt_x = r.left + gfx_radiob->GetWidth(4) + 6;
    int32_t y = r.top;

    POINT cursor;
    GetCursorPos(&cursor);

    LockSurface2();

    this->parent->VMethod8(&r);

    r.bottom -= 4;
    r.right -= 4;

    for (int32_t i = 0; i < this->entries.GetSize(); i++) {
        int32_t flag = 1 << i;
        if (this->selection & flag) {
            gfx_radiob->DrawBrightened(r.left + 5, y + 4, 5, 4, 0);
            gfx_radiob->Draw(r.left + 1, y, 5, 0, 0);
        } else {
            gfx_radiob->DrawBrightened(r.left + 5, y + 4, 4, 4, 0);
            gfx_radiob->Draw(r.left + 1, y, 4, 0, 0);
        }

        CRect entry(r.left, y, r.right, y + gfx_radiob->GetHeight(4));
        if (entry.PtInRect(cursor)) {
            this->clr = clrsh_CharlieBrown;
        } else {
            this->clr = clrsh_DullGold;
        }

        this->font->DrawTextWithShadow(txt_x, y + 3, this->entries[i], 0, this->clr, 1);

        y += gfx_radiob->GetHeight(4);
    }

    if (!this->TestFlags(1)) {
        CRect r2;
        this->ClientRectToScreen(&r2, this->rect);
        ShadowRect(CRect(r2.left - 1, r2.top - 1, r2.right + 1, r2.bottom + 1), 3);
    }

    UnlockSurface2();
}

// 4db615
int32_t VisServerScreenRadio::GetIndex(int32_t y) {
    CRect client;
    this->ClientRectToScreen(&client, this->rect);
    return (y - client.top) / gfx_radiob->GetHeight(4);
}


// 450280
VisNetChatTextBox::VisNetChatTextBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr, const char* hint)
    : VisTextBox(_id, r, _font, _clr, hint) {
}


// 450350 (deleting dtor ??_G; the complete dtor 450380 only calls the base dtor)
VisNetChatTextBox::~VisNetChatTextBox() = default;


// 4502c0
int32_t VisNetChatTextBox::OnKeyDown(uint32_t wparam) {
    if (wparam == 0xD) {
        char buf[256];
        this->WriteData(buf);
        this->ReadData(g_EmptyStrNetChatText);
        this->VMethod9();

        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        main_wnd->vis_map_context->SendChatPacket(buf, CHAT_TYPE_GLOBAL, 0);
        return 1;
    }

    return VisTextBox::OnKeyDown(wparam);
}






VisMenuWnd::~VisMenuWnd() { //451a90
}

VisMenuWnd::VisMenuWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, uint32_t unk, const CRect& _r)
: VisWindow(_id, l, t, r, b, _bitmap) { //451990
    this->element_rect = _r;
    this->gap_0x68 = unk;
    this->element_rect.left = 40;
    this->element_rect.right = this->rect.Width() - 48;
}

int32_t VisMenuWnd::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) { //440e54
    if (msg == VIS_MSG_MENU_BUTTON_CLICK) {
        if (wparam == 0) {
            return VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
        }

        int32_t res = VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        AfxGetMainWnd()->PostMessage(wparam, 0, 0);
        return res;
    }

    return VisScreen::MsgProc(msg, wparam, lparam);
}

int32_t VisMenuWnd::OnKeyDown(uint32_t wparam) { //440ece
    if (wparam == VK_UP) {
        this->TabFocus(false, true);
        return 1;
    } else if (wparam == VK_DOWN) {
        this->TabFocus(true, true);
        return 1;
    }

    return VisWindow::OnKeyDown(wparam);
}


void VisMenuWnd::AddElement(CVisualObject* obj, int32_t height) { //440e11
    this->element_rect.top = this->element_rect.bottom;
    this->element_rect.bottom = this->element_rect.top + height;

    obj->SetRect(this->element_rect);
    this->AddChild(obj);
}


int32_t MenuButton::OnLButtonUp(uint32_t wparam, CPoint pos) { //4dd933
    CRect r = this->ClientRectToScreen(this->rect);

    if (this->downed == 0) {
        return 0;
    }

    this->SetDowned(false);

    if (r.PtInRect(pos) != 0) {
        AfxGetMainWnd()->PostMessage(0x47b, this->msgid, 0);
    }

    return 1;
}

int32_t MenuButton::OnChar(uint32_t wparam) { //4dd9b3
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    char ch = ToLowerChar(EncodeChar(wparam));

    if (ch != '\r' && ch != this->charid) {
        return 0;
    }

    AfxGetMainWnd()->PostMessage(0x47b, this->msgid, 0);
    return 1;
}

MenuButton::MenuButton(int32_t _id, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: VisButton(_id, 0, 0, 0, 0, _caption, _font, _clr, _msgid, _charid, hint) { //4509c0
}

MenuButton::MenuButton(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: VisButton(_id, r, _caption, _font, _clr, _msgid, _charid, hint) { //450bf0
}





const char* HintedListBox::GetHint() { //43ec67
    int32_t idx = this->YToIndex(g_mousept.GetY());
    if (idx >= this->num_vis_entry) {
        return nullptr;
    }
    
    idx += this->vis_start_index;

    if (idx >= this->entries.GetSize()) {
        return nullptr;
    }

    CString& str = this->hints->ElementAt(idx);
    if (str.IsEmpty()) {
        return nullptr;
    }
    
    return str;
}

HintedListBox::HintedListBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: VisListBox(_id, l, t, r, b, _font, _clr1, _clr2, _scrollid, hint) { //43ec05
}









void VisGlobalMap::RebuildScenarioLocations() { //47024a
    this->PopulateScenarioLocationFlags();
}

void VisGlobalMap::PopulateScenarioLocationFlags() { //47025d
    CList<ScenarioLocation*> *locs = ScenarioGetAllLocations();
    for (POSITION pos = locs->GetHeadPosition(); pos != nullptr;) {
        ScenarioLocation* loc = locs->GetNext(pos);
        this->locationPoints.Add(loc->GetRect().TopLeft());

        if (loc->GetRect().BottomRight() == CPoint(0, 0)) {
            this->locationAvailabilityFlags.Add(1);
        }
    }
}


// 4714E7
void VisGlobalMap::VMethod7() {
    CPoint screen_pt = this->rect.TopLeft();
    int32_t screen_x = screen_pt.x;
    int32_t screen_y = screen_pt.y;
    int32_t travel_arrived = 0;
    int32_t arrival_index = 0;

    if (this->renderActiveFlag == 0) {
        return;
    }

    LockSurface2();
    this->gmap->Draw(screen_x, screen_y, 0, 0, 0);

    if (this->umoirMapMode == 0) {
        if (this->travelRoutePoints.GetSize() == 0 && this->travelProgress == 0) {
            this->OnMapClick();
        }
    } else {
        if (this->travelProgress < 8) {
            ScenarioLocation* loc = ScenarioGetAvailableLocations()->GetHead();
            CPoint loc_pt = loc->GetRect().TopLeft();
            this->currentLocationPoint = loc_pt;
            this->targetLocationPoint = loc_pt;
            this->ComputeTravelRoute(this->currentLocationPoint.x, this->currentLocationPoint.y,
                this->targetLocationPoint.x, this->targetLocationPoint.y);
            this->travelProgress = 8;
        }
    }

    if (this->travelProgress == 0) {
        for (int32_t i = 0; i < this->travelRoutePoints.GetSize(); i++) {
            if (i % 8 == 0) {
                CPoint& pt = this->travelRoutePoints.ElementAt(i);
                int32_t w = this->ballmap->GetWidth(0);
                int32_t h = this->ballmap->GetHeight(0);
                this->ballmap->DrawSubRectMasked(screen_x + pt.x - w / 2, screen_y + pt.y - h / 2, 0, 0, w, h);
            }
        }
        for (POSITION pos = ScenarioGetAvailableLocations()->GetHeadPosition(); pos != nullptr;) {
            ScenarioLocation* loc = ScenarioGetAvailableLocations()->GetNext(pos);
            CPoint loc_pt = loc->GetRect().TopLeft();
            if (this->umoirMapMode == 0) {
                if (this->currentLocationPoint != CPoint(loc_pt.x, loc_pt.y)) {
                    if (this->targetLocationPoint == CPoint(loc_pt.x, loc_pt.y)) {
                        this->mission_flg->Draw(screen_x - 5 + loc_pt.x, screen_y - 0x29 + loc_pt.y,
                            this->mapFlagAnimationFrame, 0, 0);
                    } else {
                        this->flg_on_map->Draw(screen_x - 5 + loc_pt.x, screen_y - 0x25 + loc_pt.y,
                            this->mapFlagAnimationFrame, 0, 0);
                    }
                }
            } else {
                this->flag1_spr->Draw(screen_x - 4 + loc_pt.x, screen_y - 0x20 + loc_pt.y,
                    this->mapFlagAnimationFrame, 0, 0);
            }
        }
    } else {
        int32_t dot_count = this->travelProgress;
        if (dot_count >= this->travelRoutePoints.GetUpperBound()) {
            dot_count = this->travelRoutePoints.GetUpperBound();
        }
        for (int32_t i = 0; i < dot_count; i++) {
            if (i % 8 == 0) {
                CPoint& pt = this->travelRoutePoints.ElementAt(i);
                int32_t w = this->ballmap->GetWidth(0);
                int32_t h = this->ballmap->GetHeight(0);
                this->ballmap->DrawSubRectMasked(screen_x + pt.x - w / 2, screen_y + pt.y - h / 2, 0, 0, w, h);
            }
        }
        int32_t cross_frames = this->cross_spr->GetFrameCount();
        if (this->targetCrossAnimationFrame < cross_frames - 1) {
            this->cross_spr->Draw(screen_x - 10 + this->targetLocationPoint.x,
                screen_y - 0xC + this->targetLocationPoint.y, this->targetCrossAnimationFrame, 0, 0);
        } else {
            this->cross_spr->Draw(screen_x - 10 + this->targetLocationPoint.x,
                screen_y - 0xC + this->targetLocationPoint.y, cross_frames - 1, 0, 0);
        }
        this->targetCrossAnimationFrame++;
        this->travelProgress += 8;
        if (this->travelProgress < this->travelRoutePoints.GetSize()) {
            CSound& cur_snd = (this->routePointSoundIndex == 0) ? this->snd_point1 : this->snd_point2;
            if (!IsSfxPlaying(&cur_snd)) {
                this->routePointSoundIndex = (this->routePointSoundIndex + 1) & 1;
                CSound& new_snd = (this->routePointSoundIndex == 0) ? this->snd_point1 : this->snd_point2;
                CSound::Play(new_snd);
            }
        }
        if (this->travelProgress > this->travelRoutePoints.GetSize()
            && this->targetCrossAnimationFrame > this->cross_spr->GetFrameCount() + 1) {
            this->currentLocationPoint = this->targetLocationPoint;
            this->travelProgress = 0;
            this->travelRoutePoints.RemoveAll();
            int32_t i;
            for (i = 0; i < this->locationPoints.GetSize(); i++) {
                CPoint& pt = this->locationPoints.ElementAt(i);
                if (pt == this->targetLocationPoint) {
                    break;
                }
            }
            travel_arrived = 1;
            arrival_index = i;
        }
    }

    this->mapFlagAnimationFrame = (this->mapFlagAnimationFrame + 1) % this->flag1_spr->GetFrameCount();
    if (this->umoirMapMode != 0) {
        this->flag_spr->Draw(screen_x - 0xA + this->currentLocationPoint.x,
            screen_y - 0x18 + this->currentLocationPoint.y, this->partyFlagAnimationFrame, 0, 0);
    } else {
        this->yflag_spr->Draw(screen_x - 0x14 + this->currentLocationPoint.x,
            screen_y - 0x2C + this->currentLocationPoint.y, this->partyFlagAnimationFrame, 0, 0);
    }
    this->partyFlagAnimationFrame = (this->partyFlagAnimationFrame + 1) % this->flag_spr->GetFrameCount();

    if (this->umoirMapMode == 0 && !this->hoveredLocationTitle.IsEmpty()) {
        g_font4->DrawTxt(screen_x + 0xE6, screen_y + 0x1B, this->hoveredLocationTitle, 2,
            palette_brown_derby->GetPalette(0));
        int32_t upper = this->hoveredLocationLines.GetUpperBound();
        for (int32_t i = 0; i < upper; i++) {
            g_font2->DrawTxt(screen_x + 0xE6, screen_y + 0x30 + i * 10,
                this->hoveredLocationLines.GetAt(i), 2, clrsh_InvBarleyCorn);
        }
        const CString& last_line = this->hoveredLocationLines.GetAt(upper);
        CString trimmed = last_line.Left(last_line.GetLength() - 2);
        g_font2->DrawTxt(screen_x + 0xE6, screen_y + 0x30 + upper * 10, trimmed, 2, clrsh_InvBarleyCorn);
    }

    UnlockSurface2();
    CVisualObject::VMethod7();
    if (travel_arrived != 0) {
        this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        AfxGetMainWnd()->PostMessage(MAIN_MSG_ENTER_SCENARIO_LOCATION, 0, 0);
        CList<ScenarioLocation*>* locs = ScenarioGetAvailableLocations();
        if (this->umoirMapMode == 0) {
            for (POSITION pos = locs->GetHeadPosition(); pos != nullptr;) {
                ScenarioLocation* loc = locs->GetNext(pos);
                CPoint loc_pt = loc->GetRect().TopLeft();
                if (this->targetLocationPoint == CPoint(loc_pt.x, loc_pt.y)) {
                    ScenarioEnterLocation(loc);
                    break;
                }
            }
        } else {
            ScenarioEnterLocation(locs->GetHead());
        }
    }
    this->UpdateHoveredLocation(CPoint(g_mousept.GetX(), g_mousept.GetY()));
}


// 471367
void VisGlobalMap::DoClose(uint32_t code) {
    this->renderActiveFlag = 0;
    this->FreeBitmaps();
    this->FreeSamples();
    if (this->routeAdjacencyMatrix != nullptr) {
        void** rows = static_cast<void**>(this->routeAdjacencyMatrix);
        for (int32_t i = 0; i < this->graphNodePoints.GetSize(); i++) {
            CArray<CPoint>** row = static_cast<CArray<CPoint>**>(rows[i]);
            for (int32_t j = 0; j < this->graphNodePoints.GetSize(); j++) {
                if (row[j] != nullptr) {
                    delete row[j];
                }
            }
        }
        for (int32_t i = 0; i < this->graphNodePoints.GetSize(); i++) {
            operator delete(rows[i]);
        }
        operator delete(rows);
    }
    this->routeAdjacencyMatrix = nullptr;
    this->graphNodePoints.RemoveAll();
    this->travelRoutePoints.RemoveAll();
    VisScreen::DoClose(code);
}


// 473195
GMapThing::GMapThing() {
    for (int32_t x = 0; x < 640; x++) {
        for (int32_t y = 0; y < 480; y++) {
            this->visitedGrid[x][y] = 0;
        }
    }
    this->pathmap = nullptr;
    this->LoadPathmap();
    this->bmp_data = static_cast<uint8_t*>(this->pathmap->GetData());
    this->nodeCount = 0;
    for (int32_t x = 0; x < 640; x++) {
        for (int32_t y = 0; y < 480; y++) {
            int32_t offset = y * 640 + x;
            if (this->bmp_data[offset] == 2) {
                this->nodeCount++;
                this->nodePoints.Add(CPoint(x, y));
            }
        }
    }
    this->adjacencyMatrix = static_cast<CArray<CPoint>***>(operator new(this->nodeCount * 4));
    for (int32_t i = 0; i < this->nodeCount; i++) {
        this->adjacencyMatrix[i] = static_cast<CArray<CPoint>**>(operator new(this->nodeCount * 4));
    }
    for (int32_t i = 0; i < this->nodeCount; i++) {
        for (int32_t j = 0; j < this->nodeCount; j++) {
            this->adjacencyMatrix[i][j] = nullptr;
        }
    }
    this->BuildAdjacency();
}


// 47362d
void GMapThing::BuildAdjacency() {
    CArray<CPoint> foundPoints;
    CArray<CPoint> reverseBuf;
    for (int32_t i = 0; i < this->nodeCount; i++) {
        foundPoints.RemoveAll();
        foundPoints.Add(this->nodePoints[i]);
        CPoint node = this->nodePoints[i];
        this->visitedGrid[node.x][node.y] = 1;
        this->floodPos = node;
        int32_t size;
        do {
            this->FloodFill(this->floodPos.x, this->floodPos.y, 2, &foundPoints);
            size = foundPoints.GetSize();
            if (size != 1) {
                CPoint last = foundPoints.GetAt(foundPoints.GetUpperBound());
                for (int32_t j = 0; j < this->nodeCount; j++) {
                    if (this->nodePoints[j] == last) {
                        if (this->adjacencyMatrix[i][j] == nullptr) {
                            this->adjacencyMatrix[i][j] = new CArray<CPoint>();
                            this->adjacencyMatrix[i][j]->InsertAt(0, &foundPoints);
                            reverseBuf.RemoveAll();
                            while (foundPoints.GetSize() != 0) {
                                reverseBuf.InsertAt(0, foundPoints.GetAt(0), 1);
                                foundPoints.RemoveAt(0, 1);
                            }
                            this->adjacencyMatrix[j][i] = new CArray<CPoint>();
                            this->adjacencyMatrix[j][i]->InsertAt(0, &reverseBuf);
                            foundPoints.Add(this->nodePoints[i]);
                        }
                        break;
                    }
                }
            }
        } while (size > 1);
    }
}


// 4733cf
GMapThing::~GMapThing() {
    this->FreePathmap();
    if (this->adjacencyMatrix != nullptr) {
        for (int32_t i = 0; i < this->nodeCount; i++) {
            for (int32_t j = 0; j < this->nodeCount; j++) {
                CArray<CPoint>* cell = this->adjacencyMatrix[i][j];
                if (cell != nullptr) {
                    delete cell;
                }
            }
        }
        for (int32_t i = 0; i < this->nodeCount; i++) {
            operator delete(this->adjacencyMatrix[i]);
        }
        operator delete(this->adjacencyMatrix);
    }
}


// 47391f
int32_t GMapThing::FloodFill(int32_t x, int32_t y, uint16_t mark, CArray<CPoint>* foundPoints) {
    int32_t centerOffset = y * 640 + x;
    for (int32_t nx = x - 1; nx <= x + 1; nx++) {
        if (nx < 0 || nx >= 640) {
            continue;
        }
        for (int32_t ny = y - 1; ny <= y + 1; ny++) {
            if (ny < 0 || ny >= 480) {
                continue;
            }
            CPoint pt(nx, ny);
            if (pt == CPoint(x, y)) {
                continue;
            }
            int32_t offset = ny * 640 + nx;
            if (this->bmp_data[offset] == 1) {
                if (this->visitedGrid[nx][ny] == 0) {
                    this->visitedGrid[nx][ny] = mark;
                    foundPoints->Add(CPoint(nx, ny));
                    if (this->FloodFill(nx, ny, mark + 1, foundPoints) == 0) {
                        return 0;
                    }
                }
            } else if (this->bmp_data[offset] == 2) {
                if (this->floodPos != CPoint(nx, ny)) {
                    foundPoints->Add(CPoint(nx, ny));
                    return 0;
                }
            } else if (this->bmp_data[offset] == 1 && this->bmp_data[centerOffset] == 2 && this->visitedGrid[nx][ny] != 0) {
                // already-visited cell next to a node: nothing to do
            }
        }
    }
    return 0;
}


// 473508
void GMapThing::LoadPathmap() {
    MainWindow* mainWnd = (MainWindow*)AfxGetMainWnd();
    this->FreePathmap();
    if (mainWnd->vis_globalmap->umoirMapMode != 0) {
        this->pathmap = new CBmp256("graphics\\Global.Map\\UmoirPath.bmp");
    } else {
        this->pathmap = new CBmp256("graphics\\Global.Map\\PathMap.bmp");
    }
}


// 4735e2
void GMapThing::FreePathmap() {
    if (this->pathmap != nullptr) {
        delete this->pathmap;
    }
    this->pathmap = nullptr;
}


// 473b4a
void* GMapThing::TakeAdjacency(void* nodePoints) {
    CArray<CPoint>*** matrix = this->adjacencyMatrix;
    this->adjacencyMatrix = nullptr;
    static_cast<CArray<CPoint>*>(nodePoints)->InsertAt(0, &this->nodePoints);
    return matrix;
}


// 47115D
void VisGlobalMap::VMethod28() {
    g_mousept.DisableHint();
    this->LoadBitmaps();
    this->LoadSamples();
    ReadFileToString("main\\text\\globalmap.txt", &g_MissionText);
    this->routePointSoundIndex = 0;
    GMapThing* thing = new GMapThing();
    this->graphNodePoints.RemoveAll();
    this->routeAdjacencyMatrix = thing->TakeAdjacency(&this->graphNodePoints);
    delete thing;
    this->partyFlagAnimationFrame = 0;
    this->targetCrossAnimationFrame = 0;
    this->locationMetadata.RemoveAll();
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();
    VisScreen::VMethod28();
    g_Cursors[CURSOR_SELECT]->Use();
    this->renderActiveFlag = 1;
    g_mousept.EnableHint();
}


// 47031D
void VisGlobalMap::VMethod26() {
    this->hero_bmp = nullptr;
    this->routeAdjacencyMatrix = nullptr;
    this->ballmap = nullptr;
    this->flag1_spr = nullptr;
    this->flag_spr = nullptr;
    this->cross_spr = nullptr;
    this->flg_on_map = nullptr;
    this->yflag_spr = nullptr;
    this->scroll1_bmp = nullptr;
    this->scrollp1_bmp = nullptr;
    this->gmap = nullptr;
    this->scroll3_bmp = nullptr;
    this->mission_flg = nullptr;
    this->scrollp2_bmp = nullptr;
    this->snd_scrollup.sample = nullptr;
    this->snd_point1.sample = nullptr;
    this->snd_point2.sample = nullptr;
    this->scroll2_bmp = nullptr;
    this->scrollp3_bmp = nullptr;
    this->snd_scrolldn.sample = nullptr;
    this->partDetailsRect.SetRectEmpty();
    this->mapFlagAnimationFrame = 0;
    this->travelProgress = 0;
    this->hoveredLocationIndex = -1;
    this->heroBitmapSize = CSize(0xA, 0x14);

    this->AddChild(new VisButton(4, 0x21C, 0x1C2, 0x21C, 0x1C2, " ", g_font1, clrsh_TechBlack, 0x7FFF, 0, nullptr));
}


// 472729
const char* VisGlobalMap::GetHint() {
    if (this->renderActiveFlag == 0) {
        return nullptr;
    }
    if (!this->partDetailsRect.IsRectNull()) {
        return nullptr;
    }
    CPoint pt(g_mousept.GetX() - this->rect.left, g_mousept.GetY() - this->rect.top);
    for (int32_t i = 0; i < this->locationHitRects.GetSize(); i++) {
        if (this->locationHitRects.ElementAt(i).PtInRect(pt)) {
            if (this->locationAvailabilityFlags.ElementAt(i) == 0) {
                return nullptr;
            }
            return g_GlobalMapLocationHints.GetLine(i);
        }
    }
    return nullptr;
}


// 473077
int32_t VisGlobalMap::OnRButtonDown(uint32_t wparam, CPoint pos) {
    // Binary builds the popup rect from heroDrawPoint, which nothing ever
    // writes (stays (0,0) from the ctor) — vanilla quirk, kept faithfully.
    CRect popup_rect(CPoint(this->heroDrawPoint), this->heroBitmapSize);
    CPoint screen_pt = this->rect.TopLeft();
    pos -= CSize(screen_pt.x, screen_pt.y);
    if (popup_rect.PtInRect(pos)) {
        int32_t count = g_StructEnter.structure_list.GetSize();
        if (count < 2) {
            count = 2;
        }
        this->partDetailsRect = CRect(0x64, 0x64, 0x15E, count * 0x14 + 0x78);
    }
    return 1;
}


// Statics for VisGlobalMap::MsgProc (65FA30, 65FA68 in the binary).
static uint8_t gmap_repaint_init = 0;     // 65FA30 init-done flag
static uint32_t gmap_repaint_ts = 0;      // 65FA68 timestamp of the last throttled repaint


// 472CC5
int32_t VisGlobalMap::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if ((gmap_repaint_init & 1) == 0) {
        gmap_repaint_init |= 1;
        gmap_repaint_ts = timeGetTime() - 100;
    }
    if (msg == VIS_MSG_FRAME_TICK) {
        if (timeGetTime() - gmap_repaint_ts >= 100) {
            this->VMethod9();
            gmap_repaint_ts = timeGetTime();
        }
    }
    return VisScreen::MsgProc(msg, wparam, lparam);
}


// 473038
int32_t VisGlobalMap::OnLButtonDown(uint32_t wparam, CPoint pos) {
    if (this->travelProgress == 0) {
        this->travelProgress = 8;
    } else {
        this->FinishTravel();
    }
    return 1;
}


// 472D99
int32_t VisGlobalMap::OnMouseMove(uint32_t wparam, CPoint pos) {
    if (this->travelProgress == 0) {
        this->OnMapClick();
    }
    this->UpdateHoveredLocation(pos);
    return 0;
}


// 473175
int32_t VisGlobalMap::OnRButtonUp(uint32_t wparam, CPoint pos) {
    this->partDetailsRect.SetRectEmpty();
    return 1;
}


// 4726CC
int32_t VisGlobalMap::OnChar(uint32_t wparam) {
    this->FinishTravel();
    return 1;
}


// 4726E6
int32_t VisGlobalMap::OnKeyDown(uint32_t wparam) {
    this->FinishTravel();
    return 1;
}


// 474F10
int32_t VisGlobalMap::OnLButtonUp(uint32_t wparam, CPoint pos) {
    return 0;
}


// 471F08
void VisGlobalMap::VMethod8(CRect* rect) {
}


// 47001B (scalar deleting dtor ??_G at 473D70)
VisGlobalMap::~VisGlobalMap() {
    this->FreeBitmaps();
    this->locationHitRects.RemoveAll();
    this->locationPoints.RemoveAll();
    if (this->routeAdjacencyMatrix != nullptr) {
        void** rows = static_cast<void**>(this->routeAdjacencyMatrix);
        for (int32_t i = 0; i < this->graphNodePoints.GetSize(); i++) {
            CArray<CPoint>** row = static_cast<CArray<CPoint>**>(rows[i]);
            for (int32_t j = 0; j < this->graphNodePoints.GetSize(); j++) {
                if (row[j] != nullptr) {
                    delete row[j];
                }
            }
        }
        for (int32_t i = 0; i < this->graphNodePoints.GetSize(); i++) {
            operator delete(rows[i]);
        }
        operator delete(rows);
    }
    this->routeAdjacencyMatrix = nullptr;
    this->graphNodePoints.RemoveAll();
}


// 46FD7B
VisGlobalMap::VisGlobalMap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm)
    : VisScreen(_id, l, t, r, b, btm) {
    this->VMethod26();
}


// 4705C2
void VisGlobalMap::LoadBitmaps() {
    this->FreeBitmaps();
    if (this->umoirMapMode != 0) {
        this->gmap = new CBmp64("main\\graphics\\Global.Map\\Umoir.bmp");
    } else {
        this->gmap = new CBmp64("main\\graphics\\Global.Map\\GMap.bmp");
    }
    g_mousept.Update();
    this->flag1_spr = new CA16("graphics\\Global.Map\\Flag1\\sprites.16a");
    this->flag1_spr->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->flag_spr = new CA16("graphics\\Global.Map\\Flag\\sprites.16a");
    this->flag_spr->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->cross_spr = new CA16("graphics\\Global.Map\\Cross\\sprites.16a");
    this->cross_spr->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->ballmap = new CBmp64("graphics\\Global.Map\\BallMap.bmp");
    g_mousept.Update();
    this->hero_bmp = new CBmp64("graphics\\Global.Map\\Hero.bmp");
    g_mousept.Update();
    this->heroBitmapSize = CSize(this->hero_bmp->GetWidth(0), this->hero_bmp->GetHeight(0));
    this->flg_on_map = new CA16("graphics\\Global.Map\\FlagOnMap\\sprites.16a");
    this->flg_on_map->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->mission_flg = new CA16("graphics\\Global.Map\\MissionFlag\\sprites.16a");
    this->mission_flg->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->yflag_spr = new CA16("graphics\\Global.Map\\YourFlag\\sprites.16a");
    this->yflag_spr->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->scroll1_bmp = new CBmp64("graphics\\Global.Map\\Scroll01.bmp");
    g_mousept.Update();
    this->scroll2_bmp = new CBmp64("graphics\\Global.Map\\Scroll02.bmp");
    g_mousept.Update();
    this->scroll3_bmp = new CBmp64("graphics\\Global.Map\\Scroll03.bmp");
    g_mousept.Update();
    this->scrollp1_bmp = new CBmp64("graphics\\Global.Map\\ScrollP1.bmp");
    g_mousept.Update();
    this->scrollp3_bmp = new CBmp64("graphics\\Global.Map\\ScrollP3.bmp");
    g_mousept.Update();
    this->scrollp2_bmp = new CBmp64("graphics\\Global.Map\\ScrollP2.bmp");
    g_mousept.Update();
}


// 470CD9
void VisGlobalMap::FreeBitmaps() {
    if (this->gmap != nullptr) {
        delete this->gmap;
    }
    this->gmap = nullptr;
    if (this->hero_bmp != nullptr) {
        delete this->hero_bmp;
    }
    this->hero_bmp = nullptr;
    if (this->ballmap != nullptr) {
        delete this->ballmap;
    }
    this->ballmap = nullptr;
    if (this->flag1_spr != nullptr) {
        delete this->flag1_spr;
    }
    this->flag1_spr = nullptr;
    if (this->flag_spr != nullptr) {
        delete this->flag_spr;
    }
    this->flag_spr = nullptr;
    if (this->cross_spr != nullptr) {
        delete this->cross_spr;
    }
    this->cross_spr = nullptr;
    if (this->mission_flg != nullptr) {
        delete this->mission_flg;
    }
    this->mission_flg = nullptr;
    if (this->flg_on_map != nullptr) {
        delete this->flg_on_map;
    }
    this->flg_on_map = nullptr;
    if (this->yflag_spr != nullptr) {
        delete this->yflag_spr;
    }
    this->yflag_spr = nullptr;
    if (this->scroll1_bmp != nullptr) {
        delete this->scroll1_bmp;
    }
    this->scroll1_bmp = nullptr;
    if (this->scroll2_bmp != nullptr) {
        delete this->scroll2_bmp;
    }
    this->scroll2_bmp = nullptr;
    if (this->scroll3_bmp != nullptr) {
        delete this->scroll3_bmp;
    }
    this->scroll3_bmp = nullptr;
    if (this->scrollp1_bmp != nullptr) {
        delete this->scrollp1_bmp;
    }
    this->scrollp1_bmp = nullptr;
    if (this->scrollp3_bmp != nullptr) {
        delete this->scrollp3_bmp;
    }
    this->scrollp3_bmp = nullptr;
    if (this->scrollp2_bmp != nullptr) {
        delete this->scrollp2_bmp;
    }
    this->scrollp2_bmp = nullptr;
}


// 472820
void VisGlobalMap::ComputeTravelRoute(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY) {
    if (CPoint(fromX, fromY) != CPoint(toX, toY)) {
        int32_t from_node = 0;
        int32_t to_node = 0;
        for (int32_t i = 0; i < this->graphNodePoints.GetSize(); i++) {
            CPoint& node = this->graphNodePoints.ElementAt(i);
            if (CPoint(fromX, fromY) != CPoint(node.x, node.y)) {
                if (CPoint(toX, toY) == CPoint(node.x, node.y)) {
                    to_node = i;
                }
            } else {
                from_node = i;
            }
        }
        this->routeNodeIndices.RemoveAll();
        this->routeNodeIndices.routeCost = 0;
        this->bestRouteCost = 2000000000;
        GlobalMapRouteArray* route = new GlobalMapRouteArray();
        route->Add(static_cast<uint16_t>(from_node));
        this->SearchRoute(static_cast<uint16_t>(from_node), static_cast<uint16_t>(to_node), route);
        this->travelRoutePoints.RemoveAll();
        for (int32_t i = 1; i < this->routeNodeIndices.GetSize(); i++) {
            uint16_t prev = this->routeNodeIndices.GetAt(i - 1);
            uint16_t cur = this->routeNodeIndices.GetAt(i);
            CArray<CPoint>*** rows = static_cast<CArray<CPoint>***>(this->routeAdjacencyMatrix);
            this->travelRoutePoints.Append(*rows[prev][cur]);
        }
        delete route;
    } else {
        this->travelRoutePoints.RemoveAll();
        this->travelRoutePoints.Add(CPoint(fromX, fromY));
        this->travelRoutePoints.Add(CPoint(toX, toY));
    }
}


// 472DCA
void VisGlobalMap::OnMapClick() {
    CPoint old_target = this->targetLocationPoint;
    CPoint mouse(g_mousept.GetX(), g_mousept.GetY());
    CPoint screen_pt = this->rect.TopLeft();
    mouse -= CSize(screen_pt.x, screen_pt.y);
    int32_t best_dist = 0x7FFFFFFF;
    ScenarioLocation* best_loc = nullptr;
    for (POSITION pos = ScenarioGetAvailableLocations()->GetHeadPosition(); pos != nullptr;) {
        ScenarioLocation* loc = ScenarioGetAvailableLocations()->GetNext(pos);
        CPoint loc_pt = loc->GetRect().TopLeft();
        int32_t dist = (loc_pt.x - mouse.x) * (loc_pt.x - mouse.x)
            + (loc_pt.y - mouse.y) * (loc_pt.y - mouse.y);
        if (dist < best_dist) {
            best_dist = dist;
            this->targetLocationPoint = loc_pt;
            best_loc = loc;
        }
    }
    if (old_target != this->targetLocationPoint) {
        this->ComputeTravelRoute(this->currentLocationPoint.x, this->currentLocationPoint.y,
            this->targetLocationPoint.x, this->targetLocationPoint.y);
    }
    if (best_loc != nullptr) {
        MissionGetLocName(best_loc->GetKind(), best_loc->GetId(), &this->hoveredLocationTitle);
        this->hoveredLocationTitle = this->hoveredLocationTitle.Left(this->hoveredLocationTitle.GetLength() - 2);
        CString desc;
        MissionGetDescription(best_loc->GetKind(), best_loc->GetId(), &desc);
        this->hoveredLocationLines.Copy(g_font2->StringArrayForRect(CRect(0x46, 0x30, 0x186, 0xC8), desc));
    }
}


// 472D4B
void VisGlobalMap::UpdateHoveredLocation(CPoint pos) {
    CPoint screen_pt = this->rect.TopLeft();
    pos -= CSize(screen_pt.x, screen_pt.y);
    ApplyCursor(g_Cursors[CURSOR_SELECT]);
}


// 470504
void VisGlobalMap::LoadSamples() {
    this->FreeSamples();
    ReloadSfx(&this->snd_scrollup.sample, "SFX\\ScrollUp.wav");
    ReloadSfx(&this->snd_scrolldn.sample, "SFX\\ScrollDn.wav");
    ReloadSfx(&this->snd_point1.sample, "SFX\\Point1.wav");
    ReloadSfx(&this->snd_point2.sample, "SFX\\Point2.wav");
}


// 470571
void VisGlobalMap::FreeSamples() {
    FreeSfxSample(&this->snd_scrollup.sample);
    FreeSfxSample(&this->snd_scrolldn.sample);
    FreeSfxSample(&this->snd_point1.sample);
    FreeSfxSample(&this->snd_point2.sample);
}


// 472684
void VisGlobalMap::FinishTravel() {
    if (this->travelProgress != 0) {
        this->travelProgress = this->travelRoutePoints.GetSize() + 1;
        this->targetCrossAnimationFrame = this->cross_spr->GetFrameCount() + 1;
    }
}


// 472ADA
void VisGlobalMap::SearchRoute(uint16_t from, uint16_t to, GlobalMapRouteArray* route) {
    if (from == to) {
        if (route->routeCost < this->bestRouteCost) {
            this->bestRouteCost = route->routeCost;
            this->routeNodeIndices.Copy(*route);
        }
        return;
    }
    for (int32_t n = 0; n < this->graphNodePoints.GetSize(); n++) {
        if (n == from) {
            continue;
        }
        bool contained = false;
        for (int32_t k = 0; k < route->GetSize(); k++) {
            if (route->GetAt(k) == (uint16_t)n) {
                contained = true;
                break;
            }
        }
        if (contained) {
            continue;
        }
        CArray<CPoint>*** rows = static_cast<CArray<CPoint>***>(this->routeAdjacencyMatrix);
        CArray<CPoint>* segment = rows[from][n];
        if (segment == nullptr) {
            continue;
        }
        if (route->routeCost + segment->GetSize() >= this->bestRouteCost) {
            continue;
        }
        GlobalMapRouteArray* new_route = new GlobalMapRouteArray();
        new_route->Copy(*route);
        new_route->routeCost = route->routeCost;
        new_route->Add(static_cast<uint16_t>(n));
        new_route->routeCost += segment->GetSize();
        this->SearchRoute(static_cast<uint16_t>(n), to, new_route);
        delete new_route;
    }
}






































// 446567
void VisNetDlg::CreateSessionList() {
    int32_t x = (this->rect.Width() - 0xB0) / 2;
    int32_t y = this->rect.Height() - 0x90;
    CRect rc_list(0x28, 0x50, x + 0x28, y + 0x30);

    this->AddChild(new VisLabel(0x19, 0x28, 0x20, this->rect.Width() - 0x28, 0x38, txt_dialogs.GetLine(0x8E), g_font1, p_clrsh_Black, 2));

    VisListBox* server_list = new VisListBox(1, rc_list, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 2, txt_dialogs.GetLine(0x52));
    this->AddChild(server_list);

    VisScrollBar* server_scroll = new VisScrollBar(2, server_list->GetRect().left, server_list->GetRect().top, server_list->GetRect().right + 0x18, server_list->GetRect().bottom, nullptr);
    this->AddChild(server_scroll);

    this->AddChild(new VisLabel(4, server_list->GetRect().left, server_list->GetRect().top - 0x18, server_list->GetRect().right, server_list->GetRect().top, txt_dialogs.GetLine(0x53), g_font1, p_clrsh_Black, 0));
    server_list->SetCaptionLabel((VisLabel*)this->FindChild(4));

    CRect rc_players(server_scroll->GetRect().right + 0x18, server_list->GetRect().top, server_scroll->GetRect().right + x + 0x18, server_list->GetRect().bottom);
    VisListBox* player_list = new VisListBox(7, rc_players, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 8, txt_dialogs.GetLine(0x54));
    this->AddChild(player_list);

    this->AddChild(new VisLabel(6, player_list->GetRect().left, player_list->GetRect().top - 0x18, player_list->GetRect().right, player_list->GetRect().top, txt_dialogs.GetLine(0x55), g_font1, p_clrsh_Black, 0));
    player_list->SetCaptionLabel((VisLabel*)this->FindChild(6));

    VisScrollBar* player_scroll = new VisScrollBar(8, player_list->GetRect().left, player_list->GetRect().top, player_list->GetRect().right + 0x18, player_list->GetRect().bottom, nullptr);
    this->AddChild(player_scroll);

    int32_t btn_top = this->rect.Height() - 0x3C;
    int32_t btn_bottom = this->rect.Height() - 0x24;
    int32_t w = this->rect.Width();
    CRect rc_refresh(w * 3 / 26, btn_top, w * 7 / 26, btn_bottom);
    CRect rc_join(w * 8 / 26, btn_top, w * 12 / 26, btn_bottom);
    CRect rc_create(w * 13 / 26, btn_top, w * 17 / 26, btn_bottom);
    CRect rc_exit(w * 18 / 26, btn_top, w * 22 / 26, btn_bottom);

    VisButton* refresh_btn = new VisButton(0x11, rc_refresh, txt_dialogs.GetLine(0x53), g_font1, nullptr, 0x450, 0, "");
    this->AddChild(refresh_btn);

    VisButton* join_btn = new VisButton(0xF, rc_join, txt_dialogs.GetLine(0x57), g_font1, nullptr, 0x44F, 0, "");
    join_btn->ChangeFlags(0x10, true);
    this->AddChild(join_btn);
    join_btn->ChangeFlags(1, false);
    join_btn->SetLeftObj(this->FindChild(0x11));

    VisButton* create_btn = new VisButton(0xE, rc_create, txt_dialogs.GetLine(0x56), g_font1, nullptr, 0x44E, 0, "");
    this->AddChild(create_btn);
    if (g_IsCdPresent == 0) {
        create_btn->ChangeFlags(1, false);
    }
    create_btn->SetLeftObj(this->FindChild(0xF));

    VisButton* exit_btn = new VisButton(0x10, rc_exit, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    this->AddChild(exit_btn);
    exit_btn->SetLeftObj(this->FindChild(0xE));
}

static char* netdlg_selected_name = nullptr;    // 659960 "-COMPUTERNAME-" anchor used to sort sessions


// 44791E
static int __cdecl CompareSessionsByName(const void* elem1, const void* elem2) {
    if (strcmp((const char*)elem1, netdlg_selected_name) == 0) {
        return -1;
    }
    if (strcmp((const char*)elem2, netdlg_selected_name) == 0) {
        return 1;
    }
    return strcmp((const char*)elem1, (const char*)elem2);
}


// 447C27
void VisNetDlg::CachePlayerRows() {
    for (int32_t i = 0; i < this->cached_player_rows.GetSize(); i++) {
        CStringArray* row = this->cached_player_rows.GetAt(i);
        row->RemoveAll();
        if (row != nullptr) {
            delete row;
        }
    }
    this->cached_player_rows.RemoveAll();

    char* comp_name = new char[0x14];
    netdlg_selected_name = comp_name;
    comp_name[0] = '-';
    unsigned long name_size = 0x11;
    GetComputerNameA(comp_name + 1, &name_size);
    strcat(comp_name, "-");

    if (this->sessions->sessions != nullptr && this->sessions->num_sessions != 0) {
        qsort(this->sessions->sessions, this->sessions->num_sessions, sizeof(CLlNetSession), CompareSessionsByName);
    }
    delete[] comp_name;

    this->cached_player_rows.SetSize(this->sessions->num_sessions, -1);
    for (int32_t i = 0; i < this->sessions->num_sessions; i++) {
        CLlName* names = nullptr;
        int32_t num_names = 0;
        g_CLlDriver.EnumPlayers(&this->sessions->sessions[i], &names, &num_names);

        CStringArray* row = new CStringArray();
        row->SetSize(num_names, -1);
        for (int32_t j = 0; j < num_names; j++) {
            CString name(names[j].name);
            row->ElementAt(j) = name;
        }
        this->cached_player_rows.ElementAt(i) = row;
    }
}


// 44797E
void VisNetDlg::FillServerList() {
    VisListBox* server_list = (VisListBox*)this->FindChild(1);
    int32_t count = server_list->GetItemCount();

    CString sel_name;
    if (count > this->selected) {
        sel_name = server_list->GetItem(this->selected);
    } else {
        sel_name = "When night are cold and friends are few I sit alone and think of you";
    }
    while (count != 0) {
        count--;
        server_list->RemoveItem(count);
    }

    int32_t num_sessions = this->sessions->num_sessions;
    this->selected = 0;
    for (int32_t i = 0; i < num_sessions; i++) {
        server_list->AddItem(this->sessions->sessions[i].name);

        CString item(this->sessions->sessions[i].name);
        if (item == sel_name) {
            this->selected = i;
            server_list->SetSelectedIndex(i);
        }
    }

    VisButton* join_btn = (VisButton*)this->FindChild(0xF);
    if (num_sessions == 0) {
        join_btn->ChangeFlags(1, false);
        join_btn->SetDowned(false);
    } else {
        join_btn->ChangeFlags(1, true);
    }
    join_btn->VMethod9();
    server_list->VMethod9();
}


// 447B3C
void VisNetDlg::UpdatePlayerList() {
    VisListBox* player_list = (VisListBox*)this->FindChild(7);
    int32_t count = player_list->GetItemCount();
    while (count > 0) {
        count--;
        player_list->RemoveItem(count);
    }

    if (this->cached_player_rows.GetSize() > 0) {
        CStringArray* row = this->cached_player_rows.GetAt(this->selected);
        int32_t size = row->GetSize();
        int32_t i;
        for (i = 0; i < size; i++) {
            player_list->AddItem(row->ElementAt(i));
        }
        ((VisScrollBar*)this->FindChild(8))->SetPos(0, i);
    }
    player_list->VMethod9();
}


// 447105
static int EnumSessionsPump() {
    MSG msg;
    if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE) != 0) {
        if (msg.message == WM_QUIT) {
            return 0;
        }
        if (msg.message == VIS_MSG_CLOSE_SCREEN) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
            return 0;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    g_mousept.Update();
    return 1;
}


// 447173
void VisNetDlg::RefreshSessions() {
    uint32_t timeout = 0;
    int32_t provider = g_CLlDriver.GetProvider();
    if (provider == PROVIDER_DP_TCPIP) {
        timeout = 0x5DC;
    } else if (provider == PROVIDER_IPX) {
        timeout = 0x12C;
    } else if (provider == PROVIDER_MODEM) {
        timeout = 0xEA60;
    }

    g_CLlDriver.EnumSessions(&this->sessions->sessions, &this->sessions->num_sessions, EnumSessionsPump, timeout);
    this->CachePlayerRows();
    PostMessageA(g_MainWndHWND, 0x471, 0, 0);
}


// 447842
static char* ExtractFirstWord(const char* str) {
    const char* word = str;
    while (isalpha(*word) == 0) {
        if (*word == 0) {
            return nullptr;
        }
        word++;
    }
    const char* end = word;
    while (isalpha(*end) != 0) {
        end++;
    }
    int32_t len = (int32_t)(end - word);
    char* buf = new char[len + 1];
    memcpy(buf, word, len);
    buf[len] = 0;    // original writes buf[len + 1] (off-by-one); the caller only null-checks the result
    return buf;
}


// 447214
int32_t VisNetDlg::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    case VIS_MSG_LIST_ACTIVATE:
        if (this->FindChild(0xF)->TestFlags(1) == 0) {
            return 0;
        }
        // fall through
    case VIS_MSG_NET_CREATE:
    case VIS_MSG_NET_JOIN: {
        this->sessions->character_name = main_wnd->m_GameSession.character_name;
        VisListBox* server_list = (VisListBox*)this->FindChild(1);
        this->sessions->selected_index = server_list->GetSelectedIndex();
        this->DoClose(msg);
        main_wnd->PostMessage(MAIN_MSG_CHILD_CLOSED, (WPARAM)this, 0);

        for (int32_t i = 0; i < this->cached_player_rows.GetSize(); i++) {
            CStringArray* row = this->cached_player_rows.GetAt(i);
            if (row != nullptr) {
                delete row;
            }
        }
        this->cached_player_rows.RemoveAll();
        return 1;
    }
    case VIS_MSG_CLOSE_SCREEN: {
        VisScreen::MsgProc(msg, wparam, lparam);
        main_wnd->dialogsMask = DIALOG_MASK_NONE;

        int32_t provider = g_CLlDriver.GetProvider();
        if (provider == PROVIDER_MODEM) {
            g_CLlDriver.Close();
            PostMessageA(g_MainWndHWND, MAIN_MSG_SHOW_PHONE_BOOK, 0, 0);
        } else {
            provider = g_CLlDriver.GetProvider();
            if (provider == PROVIDER_SERIAL) {
                g_CLlDriver.Close();
                PostMessageA(g_MainWndHWND, MAIN_MSG_SHOW_SERIAL_SETTINGS, 0, 0);
            } else {
                provider = g_CLlDriver.GetProvider();
                if (provider == PROVIDER_DP_TCPIP) {
                    g_CLlDriver.Close();
                    if (main_wnd->net_session.internet_game == 0) {
                        PostMessageA(g_MainWndHWND, MAIN_MSG_SHOW_TCP_SETTINGS, 0, 0);
                    } else {
                        PostMessageA(g_MainWndHWND, MAIN_MSG_SHOW_HAT_BROWSER, 0, 0);
                    }
                } else {
                    g_CLlDriver.Close();
                    PostMessageA(g_MainWndHWND, MAIN_MSG_SHOW_CONNECT_DLG, 0, 0);
                }
            }
        }

        for (int32_t i = 0; i < this->cached_player_rows.GetSize(); i++) {
            CStringArray* row = this->cached_player_rows.GetAt(i);
            if (row != nullptr) {
                delete row;
            }
        }
        this->cached_player_rows.RemoveAll();
        return 1;
    }
    case VIS_MSG_NET_REFRESH:
        this->RefreshSessions();
        return 1;
    case VIS_MSG_SET_OPTION:
        if (wparam == 1) {
            this->selected = ((int32_t)lparam < 0) ? 0 : (int32_t)lparam;
            this->UpdatePlayerList();
        }
        return 1;
    case VIS_MSG_NET_APPLY_PROVIDER: {
        int32_t provider = g_CLlDriver.GetProvider();
        if (provider == PROVIDER_SERIAL || provider == PROVIDER_MODEM) {
            this->rect = this->net_rect;
            this->sessions->character_name = main_wnd->m_GameSession.character_name;
            this->sessions->selected_index = 0;

            if (this->sessions->num_sessions == 0) {
                this->RemoveAllChilds();

                this->AddChild(new VisLabel(1, 0x28, 0x30, 0xDC, 0x4A, txt_dialogs.GetLine(0x7C), g_font1, clrsh_TechBlack, 0));

                int32_t h = this->rect.Height();
                int32_t w = this->rect.Width();
                this->AddChild(new VisButton(2, w / 2 - 0x30, h / 2 + 0xC, w / 2 + 0x30, h / 2 + 0x24, txt_dialogs.GetLine(1), g_font1, clrsh_TechBlack, 0x446, 0, ""));
            } else {
                if (g_CLlDriver.GetProvider() == PROVIDER_MODEM) {
                    char* word = ExtractFirstWord(main_wnd->phone_book.phones.ElementAt(0));
                    if (word != nullptr) {
                        delete[] word;
                    }
                }
                this->DoClose(VIS_MSG_NET_JOIN);
                main_wnd->PostMessage(MAIN_MSG_CHILD_CLOSED, (WPARAM)this, 0);
                AfxGetMainWnd()->PostMessage(0x44F, 0, 0);
            }
        } else {
            this->FillServerList();
            this->UpdatePlayerList();
            this->sessions->selected_row = this->selected;
        }
        return 1;
    }
    case VIS_MSG_NET_NEW_SESSION:
        g_CLlDriver.SetEventNewSession();
        return 1;
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
}


// 446E97
void VisNetDlg::VMethod26() {
    int32_t provider = g_CLlDriver.GetProvider();
    if (provider == PROVIDER_SERIAL || provider == PROVIDER_MODEM) {
        this->net_rect = this->rect;
        this->rect = CRect(0, 0, 0x17C, 0xC4);
        this->UpdateWinRect();

        if (g_CLlDriver.GetProvider() == PROVIDER_MODEM) {
            this->AddChild(new VisLabel(1, 0x28, 0x30, 0xDC, 0x4A, txt_dialogs.GetLine(0x77), g_font1, p_clrsh_Black, 0));
        } else {
            this->AddChild(new VisLabel(1, 0x28, 0x30, 0xDC, 0x4A, txt_dialogs.GetLine(0xA3), g_font1, p_clrsh_Black, 0));
        }

        int32_t h = this->rect.Height();
        int32_t w = this->rect.Width();
        this->AddChild(new VisButton(2, w / 2 - 0x30, h / 2 + 0xC, w / 2 + 0x30, h / 2 + 0x24, txt_dialogs.GetLine(1), g_font1, nullptr, 0x47C, 0, ""));
    } else {
        this->CreateSessionList();
    }

    this->selected = 0;
    PostMessageA(g_MainWndHWND, 0x450, 0, 0);
}


// 4478E9
int32_t VisNetDlg::OnKeyDown(uint32_t wparam) {
    if (wparam == 0x1B) {
        return this->MsgProc(VIS_MSG_NET_NEW_SESSION, 0, 0);
    }
    return VisWindow::OnKeyDown(wparam);
}


// 4464C7
VisNetDlg::VisNetDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, AvailNetSession* _sessions)
: VisWindow(_id, l, t, r, b, nullptr) {
    this->sessions = _sessions;
    this->sessions->sessions = nullptr;
    this->sessions->num_sessions = 0;
}


// 44FA90
VisNetDlg::~VisNetDlg()
{}


// 4ADAB6
void VisFameDocument::VMethod26() {
    this->fame = nullptr;
    this->bmp_sheet = nullptr;
    this->rect_prev = CRect(CPoint(0, 200), CSize(0x38, 0x28));
    this->rect_next = CRect(CPoint(0x240, 200), CSize(0x3C, 0x28));
    this->rect_ok = CRect(CPoint(0x230, 0x1A0), CSize(0x2C, 0x20));
    this->bmp_cur_left = nullptr;
    this->bmp_cur_right = nullptr;
    this->bmp_cur_ok = nullptr;
    this->AddChild(new VisButton(4, 0, 0, 0, 0, "", g_font1, clrsh_TechBlack, 0x445, 0, nullptr));
    this->visible_flag = 0;
}


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
    this->bmp_bkg = new CBmp64("main\\graphics\\famehall\\hall.bmp");
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






// VisMainMenu









// 44DBE0
void VisServerScreen::VMethod7() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CString str;
    LockSurface2();
    FillRectColorSimple(0, 0, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    this->CVisualObject::VMethod7();
    g_font2->DrawTextWithShadow(0, 0, txt_patch.GetLine(0x39), 0, clrsh_ShockingBlack, 1);
    FillRectColorSimple(0, 0x10, this->rect.right, 0x11, 0xffff);
    FillRectColorSimple(0, 0x20, this->rect.right, 0x21, 0xffff);
    FillRectColorSimple(0, 0xC8, this->rect.right, 0xC9, 0xffff);
    FillRectColorSimple(0, 0xD8, this->rect.right, 0xD9, 0xffff);

    int32_t row_top = 0x10;
    int32_t row_bottom = 0xC8;
    int32_t y = 0x24;
    int32_t i = 0;
    POSITION it = g_NetStru1_main.client_stat.GetStartPosition();
    while (it != nullptr && i < this->scroll_pos + 0x10) {
        int32_t key;
        ConnStatInfo* stat;
        g_NetStru1_main.client_stat.GetNextAssoc(it, key, stat);
        if (i < this->scroll_pos) {
            i++;
            continue;
        }
        NetStru2* client = g_NetStru1_main.GetClientByLowUid((uint32_t)key >> 16);
        Player* player = g_PlayersList->FindById(client->player_id);
        if (player == nullptr) {
            i++;
            continue;
        }
        uint16_t* colosh = g_colors_human_pals[player->player_id & 0xF];
        int32_t bytes_per_sec = 0;
        if (stat->time != 0) {
            bytes_per_sec = stat->total_bytes / stat->time;
        }
        int32_t x = 0;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0xC;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3A), 2, clrsh_ShockingBlack);
        str.Format("%d", (int16_t)player->player_id);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0xC;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x30;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3B), 2, clrsh_ShockingBlack);
        g_font2->DrawTxt(x, y, player->name, 2, colosh);
        x += 0x30;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x24;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3C), 2, clrsh_ShockingBlack);
        str.Format("%d:%02d:%02d", stat->time / 0xE10, (stat->time % 0xE10) / 0x3C, stat->time % 0x3C);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0x24;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x18;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3D), 2, clrsh_ShockingBlack);
        str.Format("%d", stat->cur_bs);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0x18;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x18;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3E), 2, clrsh_ShockingBlack);
        str.Format("%d", bytes_per_sec);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0x18;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x18;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3F), 2, clrsh_ShockingBlack);
        str.Format("%d", stat->max_bs);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0x18;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x18;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x40), 2, clrsh_ShockingBlack);
        str.Format("%d", player->monster_kills);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0x18;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0xC;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x41), 2, clrsh_ShockingBlack);
        str.Format("%d", player->player_kills);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        if (i == this->scroll_pos) {
            FillRectColor(this->rect.right - 1, row_top, this->rect.right - 1, row_bottom, 0xffff);
        }
        y += 10;
    }

    int32_t avg_units_per_10_ticks = 0;
    if (g_Server->tic16 != 0) {
        avg_units_per_10_ticks = g_Server->total_ms / g_Server->tic16 / 10;
    }
    if (g_Server->srv_stru1->sack_list != nullptr) {
        str.Format(txt_patch.GetLine(0x42),
                   main_wnd->current_map_name,
                   g_PlayersList->CountHumanPlayers(),
                   g_ServerUnits->unit_list.GetCount(),
                   g_Server->srv_stru1->building_list->GetCount(),
                   g_Server->srv_stru1->units_list->unit_list.GetCount(),
                   g_Server->srv_stru1->sack_list->list.GetCount(),
                   g_Server->last_window_ms / 10,
                   avg_units_per_10_ticks);
        g_font2->DrawTxt(0, row_bottom + 4, str, 0, clrsh_ShockingBlack);
    }
    this->msg_log->Draw();
    UnlockSurface2();
}


// 44D909
void VisServerScreen::VMethod26() {
    this->scroll_pos = 0;

    this->AddChild(new VisButton(0x63, 0x140, g_ScreenSize.bottom - 0x1E, 0x1A4, g_ScreenSize.bottom - 0x0C,
                                 txt_dialogs.GetLine(0x0E), g_font2, nullptr, 0x445, 0, nullptr));
    this->AddChild(new VisButton(0x64, 0x1B8, g_ScreenSize.bottom - 0x1E, 0x21C, g_ScreenSize.bottom - 0x0C,
                                 txt_dialogs.GetLine(0x2A), g_font2, nullptr, 0x446, 0, nullptr));

    VisServerScreenRadio* save_on_server = new VisServerScreenRadio(0x65, 0, g_ScreenSize.bottom - 0x1C, 0x12C,
                                                                    g_ScreenSize.bottom - 0x0C, g_font2, nullptr, nullptr);
    save_on_server->AddEntry(txt_patch.GetLine(0x38));
    this->AddChild(save_on_server);

    int32_t checked = 1;
    if (strstr(afxCurrentWinApp->m_lpCmdLine, "-saveonserver") != nullptr
        || strstr(afxCurrentWinApp->m_lpCmdLine, "-internetserver") != nullptr) {
        g_Server->save_on_server = 0;
    }
    if (g_Server->save_on_server != 0) {
        checked = 0;
    }
    save_on_server->ReadData(&checked);

    this->AddChild(new VisTextBox(0x68, 4, g_ScreenSize.bottom - 0x38, g_ScreenSize.right - 4, g_ScreenSize.bottom - 0x28,
                                  g_font2, clrsh_ShockingBlack, nullptr));
}


// 44e4ce
void ExecuteAdminChatCommand(const char* cmd) {
    CString s(cmd);
    s.MakeLower();
    if (s.Find("disconnect") == 0) {
        CString arg = s.Mid(0xB);
        int32_t id = atoi(arg);
        NetStru2* cli = g_NetStru1_main.GetClientByPlayerID((uint16_t)id);
        Player* player = g_PlayersList->FindById((uint16_t)id);
        if (cli != nullptr && player != nullptr) {
            g_NetStru1_main.DisconnectClient(cli);
        }
    } else if (s.Find("curse") == 0) {
        CString arg = s.Mid(6);
        int32_t id = atoi(arg);
        Player* player = g_PlayersList->FindById((uint16_t)id);
        if (player != nullptr && player->main_unit != nullptr) {
            player->main_unit->experience = 0;
            player->main_unit->body = 10;
            player->main_unit->mind = 10;
            player->main_unit->reaction = 10;
            player->main_unit->spirit = 1;
            player->main_unit->RecalcStats();
            g_NetStru1_main.SendUnitUpdate(player->main_unit, nullptr, UNIT_UPD_ALL, EQUIP_SLOTS_ALL, 0, 0);
        }
    }
}


// 44E6DE
int32_t VisServerScreen::OnKeyDown(uint32_t wparam) {
    if (wparam == 0x0D) {  // Enter: send the chat text box content
        PacketJoin& pkt = PacketJoin::Inst;
        pkt.id = CHAT;
        pkt.to_player_id = 0;
        pkt.__as_dword = 0;

        CVisualObject* textbox = this->FindChild(0x68);
        textbox->WriteData(pkt.name);

        if (pkt.name[0] == '#') {
            ExecuteAdminChatCommand(&pkt.name[1]);
        } else {
            LogMessage(pkt.name);
            g_NetStru1_main.QueuePacketSend(&pkt);
        }

        CVisualObject* textbox_clear = this->FindChild(0x68);
        textbox_clear->ReadData(g_EmptyStrSrvScreenText);
        return 1;
    }

    CVisualObject* textbox = this->FindChild(0x68);
    return textbox->OnKeyDown(wparam);
}


// 44E7C8
int32_t VisServerScreen::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    switch (msg) {
    case VIS_MSG_CLOSE_SCREEN:  // "disconnect" button
        this->VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
        PostMessageA(g_MainWndHWND, MAIN_MSG_LEAVE_MISSION, 0, 0);
        return 1;

    case VIS_MSG_SET_OPTION:  // "save on server" checkbox changed
        if (wparam == 0x65) {
            g_Server->save_on_server = (lparam == 0) ? 1 : 0;
            return 1;
        }
        return this->VisScreen::MsgProc(msg, wparam, lparam);

    case 0x100:  // WM_KEYDOWN
        this->OnKeyDown(wparam);
        return 1;

    default:
        return this->VisScreen::MsgProc(msg, wparam, lparam);
    }
}


// 44DBAE
void VisServerScreen::VMethod8(CRect* rect) {
    FillRectColorSimple(rect->left, rect->top, rect->right, rect->bottom, 0);
}


// 44e469
VisServerScreen::VisServerScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, GM_a28* log)
: VisScreen(_id, l, t, r, b, nullptr) {
    this->msg_log = log;
}


// 450ab0 (deleting dtor ??_G; the complete dtor at 450AE0 only calls the base dtor)
VisServerScreen::~VisServerScreen() = default;


// 44878A
int32_t VisNetPhoneBook::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    switch (msg) {
    case VIS_MSG_LIST_ACTIVATE:
        PostMessageA(g_MainWndHWND, 0x46e, wparam, lparam);
        PostMessageA(g_MainWndHWND, 0x47d, 0, 0);
        return 1;
    case VIS_MSG_CLOSE_SCREEN: {
        int32_t res = this->VisScreen::MsgProc(msg, wparam, lparam);
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        main_wnd->PostMessage(MAIN_MSG_SHOW_CONNECT_DLG, 0, 0);
        return res;
    }
    case VIS_MSG_SET_OPTION:
        if (wparam == 1) {
            CVisualObject* edit = this->FindChild(3);
            const char* str;
            if ((int32_t)lparam < 0) {
                str = "";
            } else {
                str = this->phones->phones[lparam];
            }
            this->UpdateConnectButton(str);
            edit->ReadData(str);
            edit->VMethod9();
        }
        if (wparam == 3) {
            CVisualObject* edit = this->FindChild(3);
            char buf[64];
            edit->WriteData(buf);
            this->UpdateConnectButton(buf);
        }
        return 1;
    case VIS_MSG_PHONE_CONNECT: {
        CVisualObject* connect_btn = this->FindChild(10);
        if (connect_btn->TestFlags(1) == 0) {
            return 1;
        }
        char string[64];
        CVisualObject* edit = this->FindChild(3);
        edit->WriteData(string);
        this->phones->dial = string;
        CString dial_str(string);
        bool found = false;
        for (int32_t i = 0; i < this->phones->phones.GetSize(); i++) {
            CString name = this->phones->phones[i];
            if (name == this->phones->dial) {
                found = true;
            }
        }
        if (!found) {
            this->phones->phones.InsertAt(0, dial_str, 1);
        }
        VisListBox* list = (VisListBox*)this->FindChild(7);
        int32_t sel = list->GetSelectedIndex();
        CLlAddress* addr = this->enum_addresses + sel;
        if (g_CLlDriver.PrepareForConnect(this->phones->dial, addr)) {
            MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
            main_wnd->PostMessage(MAIN_MSG_SHOW_NET_SESSIONS, 0, 0);
            this->VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        }
        return 1;
    }
    case VIS_MSG_PHONE_DIAL: {
        this->VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        VisListBox* list = (VisListBox*)this->FindChild(7);
        int32_t sel = list->GetSelectedIndex();
        CLlAddress* addr = this->enum_addresses + sel;
        if (g_CLlDriver.StartServer(2, main_wnd->m_GameSession.character_name, addr) == 0) {
            VisMessageBoxWithList* msgbox = new VisMessageBoxWithList(
                1, 100, 100, 0x21c, 0x17c, TxtFile::AllLines.GetAt(0x9a), nullptr, 0);
            main_wnd->ModalScreen(msgbox);
            main_wnd->PostMessage(MAIN_MSG_SHOW_PHONE_BOOK, 0, 0);
        } else {
            main_wnd->PostMessage(MAIN_MSG_ENTER_CLIENT_MODE, 0, 0);
        }
        return 1;
    }
    default:
        return this->VisScreen::MsgProc(msg, wparam, lparam);
    }
}


// 447EF2
void VisNetPhoneBook::VMethod26() {
    g_CLlDriver.EnumAddresses(&this->enum_addresses, &this->enum_addresses_num);

    VisLabel* title = new VisLabel(-1, 0x28, 0x14, this->rect.Width() - 0x28, 0x2c,
        txt_dialogs.GetLine(0x93), g_font1, p_clrsh_Black, 2);
    this->AddChild(title);

    int32_t third_width = this->rect.Width() / 3;
    CRect rc(0x28, 0x50, third_width + 0x28, this->rect.Height() - 0x40);

    VisListBoxPhoneBook* phone_list = new VisListBoxPhoneBook(1, rc, g_font1, p_clrsh_Black,
        p_clrsh_ShockingBlack, 2, txt_dialogs.GetLine(0x5a), &this->phones->phones);
    this->AddChild(phone_list);

    for (int32_t i = 0; i < this->phones->phones.GetSize(); i++) {
        phone_list->AddItem(this->phones->phones[i]);
    }

    CRect rc2(phone_list->GetRect());

    VisScrollBar* scrollbar = new VisScrollBar(2, rc2.right, rc2.top, rc2.right + 0x18, rc2.bottom, nullptr);
    this->AddChild(scrollbar);

    VisLabel* phone_caption = new VisLabel(4, rc2.left + 0xa, rc2.top - 0x1e, rc2.right + 0xa, rc2.top - 0xa,
        txt_dialogs.GetLine(0x5c), g_font1, p_clrsh_Black, 2);
    this->AddChild(phone_caption);

    phone_list->SetCaptionLabel((VisLabel*)this->FindChild(4));

    VisLabel* addr_caption = new VisLabel(-2, rc2.right + 0x30, rc2.top - 0x1e, rc2.right + third_width * 3, rc2.top - 0xa,
        txt_dialogs.GetLine(0x98), g_font1, p_clrsh_Black, 0);
    this->AddChild(addr_caption);

    VisTextBox* edit = new VisTextBox(3, rc2.right + 0x30, rc2.top, rc2.right + (third_width * 3) / 2, rc2.top + 0x18,
        g_font1, p_clrsh_Black, txt_dialogs.GetLine(0x5b));
    this->AddChild(edit);

    CRect rc3(edit->GetRect());
    rc3.OffsetRect(0, rc3.Height() + 4);
    rc3.bottom = rc3.top + 0x30;

    VisListBox* addr_list = new VisListBox(7, rc3, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 8, txt_dialogs.GetLine(0x5d));
    this->AddChild(addr_list);

    for (int32_t i = 0; i < this->enum_addresses_num; i++) {
        addr_list->AddItem(this->enum_addresses[i].name);
    }

    rc3.OffsetRect(0, rc3.Height() + 4);
    rc3.bottom = rc3.top + 0x18;

    VisButton* connect_btn = new VisButton(0xa, rc3, txt_dialogs.GetLine(0x78), g_font1, nullptr, 0x47d, 0, txt_dialogs.GetLine(0x7e));
    this->AddChild(connect_btn);
    if (g_IsCdPresent == 0 || this->enum_addresses_num == 0) {
        connect_btn->ChangeFlags(1, false);
    }

    rc3.OffsetRect(0, rc3.Height() + 4);

    VisButton* host_btn = new VisButton(0xb, rc3, txt_dialogs.GetLine(0x79), g_font1, nullptr, 0x47e, 0, txt_dialogs.GetLine(0x7f));
    this->AddChild(host_btn);
    if (g_IsCdPresent == 0 || this->enum_addresses_num == 0) {
        host_btn->ChangeFlags(1, false);
    }

    rc3.OffsetRect(0, rc3.Height() + 4);

    VisButton* back_btn = new VisButton(0xc, rc3, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    this->AddChild(back_btn);

    const char* str = "";
    if (this->phones->phones.GetSize() != 0) {
        str = this->phones->phones[0];
        edit->ReadData(str);
    }
    this->UpdateConnectButton(str);
}


// 447ea8
VisNetPhoneBook::VisNetPhoneBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, PhoneBook* _book)
: VisWindow(_id, l, t, r, b, nullptr) {
    this->phones = _book;
    this->enum_addresses = nullptr;
}


// 44fae0 (deleting dtor ??_G; the complete dtor at 44FB10 only calls the base dtor)
VisNetPhoneBook::~VisNetPhoneBook() = default;


// 44FB30
VisListBoxPhoneBook::VisListBoxPhoneBook(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _entry_height, const char* hint, CStringArray* _bound_entries)
    : VisListBox(_id, r, _font, _clr1, _clr2, _entry_height, hint) {
    this->bound_entries = _bound_entries;
}

// 44fca0 (deleting dtor ??_G at 44FC70; the complete dtor only calls the base dtor)
VisListBoxPhoneBook::~VisListBoxPhoneBook() = default;

// 44FB80
int32_t VisListBoxPhoneBook::OnKeyDown(uint32_t wparam) {
    if (wparam == 0x2E) {
        if (this->selected_index >= 0) {
            this->RemoveItem(this->selected_index);
            this->bound_entries->RemoveAt(this->selected_index, 1);
            if (this->selected_index == this->entries.GetSize()) {
                this->selected_index--;
            }
            VisScrollBar* scrollbar = (VisScrollBar*)this->parent->FindChild(this->scrollbox_id);
            if (scrollbar != nullptr && this->selected_index != 0) {
                scrollbar->SetPos(this->selected_index, this->entries.GetSize());
            }
            this->VMethod9();
        }
        return 1;
    }
    return VisListBox::OnKeyDown(wparam);
}


// 448708
void VisNetPhoneBook::UpdateConnectButton(const char* str) {
    VisButton* connect_btn = (VisButton*)this->FindChild(10);
    if (str != nullptr && this->enum_addresses_num != 0) {
        connect_btn->ChangeFlags(1, true);
    } else {
        connect_btn->ChangeFlags(1, false);
        connect_btn->SetDowned(false);
    }
    connect_btn->VMethod9();
}


// 448cd5
VisNetSerialSettings::VisNetSerialSettings(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, ComSettings* _com)
: VisWindow(_id, l, t, r, b, nullptr) {
    this->pcom = _com;
}


// 44fcc0 (deleting dtor ??_G; the complete dtor 44fcf0 only calls the base dtor)
VisNetSerialSettings::~VisNetSerialSettings() = default;


// 449903
void VisNetSerialSettings::SaveComSettings() {
    char buffer[64];

    CVisualObject* obj = this->FindChild(1);
    obj->WriteData(buffer);
    this->pcom->index = atoi(buffer + 3);

    obj = this->FindChild(2);
    obj->WriteData(buffer);
    this->pcom->speed = atoi(buffer);

    obj = this->FindChild(3);
    obj->WriteData(buffer);
    if (strcmp(buffer, txt_dialogs.GetLine(0x62)) == 0) {
        this->pcom->stop_bits = 0;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x63)) == 0) {
        this->pcom->stop_bits = 1;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x64)) == 0) {
        this->pcom->stop_bits = 2;
    }

    obj = this->FindChild(4);
    obj->WriteData(buffer);
    if (strcmp(buffer, txt_dialogs.GetLine(0x5e)) == 0) {
        this->pcom->parity = 0;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x5f)) == 0) {
        this->pcom->parity = 1;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x60)) == 0) {
        this->pcom->parity = 2;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x61)) == 0) {
        this->pcom->parity = 3;
    }

    obj = this->FindChild(5);
    obj->WriteData(buffer);
    if (strcmp(buffer, txt_dialogs.GetLine(0x69)) == 0) {
        this->pcom->flow_control = 4;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x65)) == 0) {
        this->pcom->flow_control = 2;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x66)) == 0) {
        this->pcom->flow_control = 3;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x67)) == 0) {
        this->pcom->flow_control = 0;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x68)) == 0) {
        this->pcom->flow_control = 1;
    }
}


// 448d15
void VisNetSerialSettings::VMethod26() {
    VisLabel* title = new VisLabel(-1, 0x28, 0x14, this->rect.Width() - 0x28, 0x2c,
        txt_dialogs.GetLine(0x94), g_font1, p_clrsh_Black, 2);
    this->AddChild(title);

    CRect rc(0x28, 0x38, 0xd0, 0x3c);

    VisLabel* com_label = new VisLabel(0x1e, rc, txt_dialogs.GetLine(0x6a), g_font1, p_clrsh_Black, 0);
    this->AddChild(com_label);
    rc.OffsetRect(0, 0x28);

    VisLabel* speed_label = new VisLabel(0x1f, rc, txt_dialogs.GetLine(0x6b), g_font1, p_clrsh_Black, 0);
    this->AddChild(speed_label);
    rc.OffsetRect(0, 0x28);

    VisLabel* parity_label = new VisLabel(0x20, rc, txt_dialogs.GetLine(0x6c), g_font1, p_clrsh_Black, 0);
    this->AddChild(parity_label);
    rc.OffsetRect(0, 0x28);

    VisLabel* flow_label = new VisLabel(0x21, rc, txt_dialogs.GetLine(0x6d), g_font1, p_clrsh_Black, 0);
    this->AddChild(flow_label);
    rc.OffsetRect(0, 0x28);

    VisLabel* stop_label = new VisLabel(0x22, rc, txt_dialogs.GetLine(0x6e), g_font1, p_clrsh_Black, 0);
    this->AddChild(stop_label);

    VisComboBox* com_combo = new VisComboBox(1, CRect(0xe4, 0x38, this->rect.Width() - 0x2c, 0xb0),
        txt_dialogs.GetLine(0x6f));
    this->AddChild(com_combo);
    com_combo->AddItem("COM1");
    com_combo->AddItem("COM2");
    com_combo->AddItem("COM3");
    com_combo->AddItem("COM4");
    com_combo->SelectItem(this->pcom->index - 1);

    CRect rc2(com_combo->GetRect());
    rc2.OffsetRect(0, 0x28);
    rc2.bottom = rc2.top + 0xc0;

    VisComboBox* speed_combo = new VisComboBox(2, rc2, txt_dialogs.GetLine(0x70));
    this->AddChild(speed_combo);
    speed_combo->AddItem("14400");
    speed_combo->AddItem("19200");
    speed_combo->AddItem("38400");
    speed_combo->AddItem("56000");
    speed_combo->AddItem("57600");
    speed_combo->AddItem("115200");
    speed_combo->AddItem("128000");
    speed_combo->AddItem("256000");
    switch (this->pcom->speed) {
    case 14400:
        speed_combo->SelectItem(0);
        break;
    case 19200:
        speed_combo->SelectItem(1);
        break;
    case 38400:
        speed_combo->SelectItem(2);
        break;
    case 56000:
        speed_combo->SelectItem(3);
        break;
    case 57600:
        speed_combo->SelectItem(4);
        break;
    case 115200:
        speed_combo->SelectItem(5);
        break;
    case 128000:
        speed_combo->SelectItem(6);
        break;
    case 256000:
        speed_combo->SelectItem(7);
        break;
    default:
        speed_combo->SelectItem(0);
        break;
    }

    rc2.OffsetRect(0, 0x28);
    rc2.bottom = rc2.top + 0x78;

    VisComboBox* parity_combo = new VisComboBox(4, rc2, txt_dialogs.GetLine(0x71));
    this->AddChild(parity_combo);
    parity_combo->AddItem(txt_dialogs.GetLine(0x5e));
    parity_combo->AddItem(txt_dialogs.GetLine(0x5f));
    parity_combo->AddItem(txt_dialogs.GetLine(0x60));
    parity_combo->AddItem(txt_dialogs.GetLine(0x61));
    parity_combo->SelectItem(this->pcom->parity);

    rc2.OffsetRect(0, 0x28);
    rc2.bottom = rc2.top + 0x78;

    VisComboBox* flow_combo = new VisComboBox(5, rc2, txt_dialogs.GetLine(0x72));
    this->AddChild(flow_combo);
    flow_combo->AddItem(txt_dialogs.GetLine(0x67));
    flow_combo->AddItem(txt_dialogs.GetLine(0x68));
    flow_combo->AddItem(txt_dialogs.GetLine(0x65));
    flow_combo->AddItem(txt_dialogs.GetLine(0x66));
    flow_combo->AddItem(txt_dialogs.GetLine(0x69));
    flow_combo->SelectItem(this->pcom->flow_control);

    rc2.OffsetRect(0, 0x28);
    rc2.bottom = rc2.top + 0x48;

    VisComboBox* stop_combo = new VisComboBox(3, rc2, txt_dialogs.GetLine(0x73));
    this->AddChild(stop_combo);
    stop_combo->AddItem(txt_dialogs.GetLine(0x62));
    stop_combo->AddItem(txt_dialogs.GetLine(0x63));
    stop_combo->AddItem(txt_dialogs.GetLine(0x64));
    stop_combo->SelectItem(this->pcom->stop_bits);

    CRect btn_rc(60, 272, 321, 296);

    VisButton* join_btn = new VisButton(10, btn_rc, txt_dialogs.GetLine(0x7a), g_font1, nullptr, 0x480, 0,
        txt_dialogs.GetLine(0x80));
    this->AddChild(join_btn);
    btn_rc.OffsetRect(0, 0x1e);

    VisButton* host_btn = new VisButton(0xb, btn_rc, txt_dialogs.GetLine(0x7b), g_font1, nullptr, 0x481, 0,
        txt_dialogs.GetLine(0x81));
    this->AddChild(host_btn);
    if (g_IsCdPresent == 0) {
        host_btn->ChangeFlags(1, false);
    }
    btn_rc.OffsetRect(0, 0x1e);

    this->FindChild(0xb)->SetUpObj(this->FindChild(10));

    VisButton* reconnect_btn = new VisButton(0xc, btn_rc, txt_dialogs.GetLine(0x82), g_font1, nullptr, 0x483, 0,
        txt_dialogs.GetLine(0x83));
    this->AddChild(reconnect_btn);
    btn_rc.OffsetRect(0, 0x1e);

    this->FindChild(0xc)->SetUpObj(this->FindChild(0xb));

    VisButton* back_btn = new VisButton(0xd, btn_rc, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    this->AddChild(back_btn);

    this->FindChild(0xd)->SetUpObj(this->FindChild(0xc));
}


// 449c98
int32_t VisNetSerialSettings::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    switch (msg) {
    case VIS_MSG_CLOSE_SCREEN: {
        int32_t result = VisScreen::MsgProc(msg, wparam, lparam);
        AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_CONNECT_DLG, 0, 0);
        return result;
    }
    case VIS_MSG_SERIAL_CONNECT: {
        this->SaveComSettings();

        CLlAddress addr;
        addr.com = *this->pcom;
        if (g_CLlDriver.PrepareForConnect("", &addr) != 0) {
            AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_NET_SESSIONS, 0, 0);
            VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        }
        return 1;
    }
    case VIS_MSG_SERIAL_HOST: {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

        this->SaveComSettings();

        CLlAddress addr;
        addr.com = *this->pcom;
        if (g_CLlDriver.StartServer(2, main_wnd->m_GameSession.character_name, &addr) != 0) {
            VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
            main_wnd->PostMessage(MAIN_MSG_ENTER_CLIENT_MODE2, 0, 0);
        }
        return 1;
    }
    case VIS_MSG_SERIAL_CLEAR: {
        this->DestroyAllChilds();
        this->cursor_over_obj_last = nullptr;
        this->cursor_over_obj = nullptr;
        this->last_focus_obj = nullptr;
        this->focus_obj = nullptr;
        this->VMethod26();

        this->FocusTo(this->FindChild(0xc), false);
        this->VMethod9();
        return 1;
    }
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
}

// 44b37a
int32_t GetFieldPos(CString str, int32_t field) {
    int32_t pos = 0;
    for (int32_t i = 0; i < field; i++) {
        int32_t sep = str.Find('|');
        pos = pos + sep + 1;
        if (i == field - 1) {
            return pos;
        }
        str = str.Mid(sep + 1);
    }
    return -1;
}

// 44b460
int32_t VisHatBrowserList::IsItemEnabled(int32_t idx) {
    if (this->state == 0) {
        return 0;
    }

    if (!this->IsValidIndex(idx)) {
        return 0;
    }

    CString& item = this->entries[idx];

    int32_t pos = GetFieldPos(item, 5);
    if (pos == -1) {
        return 0;
    }

    int32_t level = item[pos] - '1';
    if (level < 0) {
        level = 0;
    }
    if (level > 3) {
        level = 3;
    }

    pos = GetFieldPos(item, 6);
    if (pos == -1) {
        return 0;
    }

    int32_t players = atoi(item.Mid(pos, 2));

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->net_session.internet_game == 0 || main_wnd->hat_settings.ishat == 0 || main_wnd->hat_settings.deathmatch == 0) {
        if (main_wnd->m_GameSession.GetSkillTier1() > level + 1
            || main_wnd->m_GameSession.GetSkillTier2() < level + 1
            || players == 16) {
            return 0;
        }
        return 1;
    }

    return players != 16 ? 1 : 0;
}

// 44ff20
VisHatBrowserList::VisHatBrowserList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: VisListBox(_id, l, t, r, b, _font, _clr1, _clr2, _scrollid, hint) {
    this->state = -1;
}

// 44ffb0
VisHatBrowserList::~VisHatBrowserList() {
}

// 44b60c
void VisHatBrowserList::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    if (this->state == 0 && idx == 0) {
        this->font->DrawTextWithShadow(pos.x + 5, pos.y, txt_patch.GetLine(0x71), 0, clrsh_ShockingBlack, 1);
        return;
    }

    if (this->state == -1 && idx == 0) {
        this->font->DrawTextWithShadow(pos.x + 5, pos.y, txt_patch.GetLine(0x72), 0, clrsh_ShockingBlack, 1);
        return;
    }

    if (this->state == 0 || !this->IsValidIndex(idx)) {
        return;
    }

    if (!this->IsItemEnabled(idx)) {
        clr = g_colors_human_pals[15];
    }

    CString& item = this->entries[idx];

    CRect saved_clip;
    GetClipRect(&saved_clip);

    pos.x += 5;

    CRect clip(saved_clip);
    clip.right = vis_scr_rect.left + 0xD9;
    SetClipRect(clip);

    CString text = item.Mid(GetFieldPos(item, 1));
    text = text.Left(text.Find('|'));
    this->font->DrawTextWithShadow(pos.x, pos.y, text, 0, clr, 1);

    text = item.Mid(GetFieldPos(item, 3));
    text = text.Left(text.Find('|'));
    clip.right = vis_scr_rect.left + 0x160;
    pos.x = vis_scr_rect.left + 0xDC;
    SetClipRect(clip);
    this->font->DrawTextWithShadow(pos.x, pos.y, text, 0, clr, 1);

    text = item.Mid(GetFieldPos(item, 4));
    text = text.Left(text.Find('|'));
    SetClipRect(saved_clip);
    pos.x = vis_scr_rect.left + 0x163;
    this->font->DrawTextWithShadow(pos.x, pos.y, text, 0, clr, 1);

    int32_t level = item[GetFieldPos(item, 5)] - '1';
    if (level < 0) {
        level = 0;
    }
    if (level > 3) {
        level = 3;
    }
    text = txt_patch.GetLine(level + 0x62);
    pos.x = vis_scr_rect.left + 0x1AE;
    this->font->DrawTextWithShadow(pos.x, pos.y, text, 0, clr, 1);

    text = item.Mid(GetFieldPos(item, 6));
    text = text.Left(text.Find('|'));
    pos.x = vis_scr_rect.left + 0x203;
    this->font->DrawTextWithShadow(pos.x, pos.y, text, 0, clr, 1);
}


// 44bc6c
// client: 44e774
void VisHatBrowserDlg::VMethod26() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    VisLabel* title_label = new VisLabel(0x19, 0x28, 0x16, this->rect.Width() - 0x28, 0x38, txt_patch.GetLine(0x6a), g_font1, p_clrsh_Black, 2);
    this->AddChild(title_label);

    const char* ip_text = main_wnd->hat_settings.hatip;
#ifdef A2CLIENT
    if (main_wnd->hat_settings.ishat != 0) {
        ip_text = main_wnd->hat_settings.hatprogip;
    }
#endif
    VisLabel* ip_label = new VisLabel(0x7e, 0x28, 0x2c, this->rect.Width() - 0x28, 0x44, ip_text, g_font1, p_clrsh_Black, 2);
    this->AddChild(ip_label);

#ifdef A2CLIENT
    int32_t cap_top = 0x30;
    int32_t cap_bottom = 0x44;
#else
    int32_t cap_top = 0x44;
    int32_t cap_bottom = 0x5c;
#endif

    VisLabel* caption_label = new VisLabel(0x1a, 0x28, cap_top, this->rect.Width() - 0x28, cap_bottom, txt_patch.GetLine(0x6b), g_font1, p_clrsh_Black, 0);
    this->AddChild(caption_label);

    VisHatBrowserList* list = nullptr;
    if (main_wnd->hat_settings.ishat != 0) {
        int32_t list_top = 0x44;
        int32_t list_bottom = this->rect.Height() - 0x60;
        if (main_wnd->hat_settings.deathmatch == 2) {
            list_bottom = this->rect.Height() - 0xc0;
        }
        list = new VisHatBrowserList(1, 0x28, list_top, this->rect.Width() - 0x40, list_bottom, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0xa, nullptr);
    } else {
#ifdef A2CLIENT
        int32_t list_bottom = this->rect.Height() - 0x50;
#else
        int32_t list_bottom = this->rect.Height() - 0x60;
#endif
        list = new VisHatBrowserList(1, 0x28, 0x5c, this->rect.Width() - 0x40, list_bottom, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0xa, nullptr);
    }

#ifdef A2CLIENT
    if (main_wnd->hat_settings.ishat != 0) {
        if (main_wnd->hat_settings.deathmatch == 2) {
            VisRadioType2* store_radio = new VisRadioType2(0x31, 0x28, this->rect.Height() - 0xb6, this->rect.Width() - 0x40, this->rect.Height() - 0x80, g_font1, p_clrsh_Black, nullptr);
            store_radio->AddEntry(txt_patch.GetLine(0x8b));
            store_radio->AddEntry(txt_patch.GetLine(0x8c));
            this->AddChild(store_radio);
            store_radio->ReadData(&main_wnd->hat_settings.store);
        }
    
        VisLabel* status1 = new VisLabel(0x2d, 0x28, this->rect.Height() - 0x64, this->rect.Width() - 0x28, this->rect.Height() - 0x50, " ", g_font1, p_clrsh_Black, 2);
        this->AddChild(status1);
        VisLabel* status2 = new VisLabel(0x2e, 0x28, this->rect.Height() - 0x54, this->rect.Width() - 0x28, this->rect.Height() - 0x40, " ", g_font1, p_clrsh_Black, 2);
        this->AddChild(status2);
    }
#endif

    this->AddChild(list);

    list->SetCaptionLabel((VisLabel*)this->FindChild(0x1a));

    CRect list_rect = list->GetRect();
    VisScrollBar* scrollbar = new VisScrollBar(0xa, list_rect.right, list_rect.top, list_rect.right + 0x18, list_rect.bottom, nullptr);
    this->AddChild(scrollbar);

    CPoint btn_center(0xc0, this->rect.Height() - 0x30);
    CRect btn_rect(btn_center.x - 0x30, btn_center.y - 0xc, btn_center.x + 0x30, btn_center.y + 0xc);
    btn_rect.OffsetRect(-0x48, 0);

    VisButton* ok_btn = new VisButton(0x14, btn_rect, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "");
    this->AddChild(ok_btn);
    ok_btn->ChangeFlags(1, false);

    btn_rect.OffsetRect(0x90, 0);
    VisButton* cancel_btn = new VisButton(0x15, btn_rect, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    this->AddChild(cancel_btn);
    cancel_btn->SetLeftObj(this->FindChild(0x14));

    btn_rect.OffsetRect(0x90, 0);
    VisButton* refresh_btn = new VisButton(0x16, btn_rect, txt_patch.GetLine(0x6c), g_font1, nullptr, 0x48b, 0, "");
    this->AddChild(refresh_btn);
    refresh_btn->SetLeftObj(this->FindChild(0x15));
}


// HatBrowserProgress implementation (struct declared in net.h)

HatBrowserProgress::HatBrowserProgress() { //4b18c0
    recv_addr_len = sizeof(send_addr);
    timeout = 1000;
    addr.S_un.S_addr = 0;
    send_serial = 0;
    sock = INVALID_SOCKET;
    wsa_started = 0;
    timestamp = 0;
    min_rtt = 10000;
    last_rtt = 0;
    max_rtt = 0;
    last_rtt = 0;
    total_rtt = 0;
    recv_serial = 0;
    timeouts = 0;
    error = 0;

    InitializeCriticalSection(&cs);

    if (WSAStartup(0x101, &wsa) == 0) {
        send_buf.resize(0x400);
        recv_buf.resize(0x400);
        if (send_buf.empty() || recv_buf.empty()) {
            error = -5;
        }
        else {
            wsa_started = 1;
        }
    }
    else {
        error = -1;
    }
}

HatBrowserProgress::~HatBrowserProgress() {
    if (wsa_started != 0) {
        if (sock != INVALID_SOCKET)
            closesocket(sock);
        WSACleanup();
    }
    DeleteCriticalSection(&cs);
}


void HatBrowserProgress::Init(const char* hst) { //4b1a9b
    if (wsa_started == 0)
        return;

    active = 0;

    EnterCriticalSection(&cs);

    min_rtt = 1000;
    last_rtt = 0;
    max_rtt = 0;
    last_rtt = 0;
    total_rtt = 0;
    recv_serial = 0;
    timeouts = 0;
    error = 0;

    LeaveCriticalSection(&cs);

    if (sock != INVALID_SOCKET)
        closesocket(sock);
    
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == INVALID_SOCKET) {
        error = -2;
        return;
    }

    err = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, 4);
    if (err == -1) {
        error = -3;
        return;
    }

    timeout = 1000;
    err = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, 4);
    if (err == -1) {
        error = -3;
        return;
    }

    memset(&send_addr, 0, sizeof(send_addr));
    //host_ent = gethostbyname(hst);

    if (GetAddrInfo(hst, nullptr, nullptr, &host_ent) == 0 || host_ent == nullptr) {
        inet_pton(AF_INET, hst, &addr.S_un); // addr.S_un.S_addr = inet_addr(hst);
    }

    if ((host_ent == nullptr) && addr.S_un.S_addr == 0xffffffff) {
        error = -4;
        return;
    }

    if (host_ent == nullptr)
        send_addr.sin_addr.S_un = addr.S_un;
    else
        memcpy(&send_addr.sin_addr, host_ent->ai_addr, host_ent->ai_addrlen);

    if (host_ent == nullptr)
        send_addr.sin_family = 2;
    else
        send_addr.sin_family = host_ent->ai_family;

    static char addrbuf[INET_ADDRSTRLEN+1];
    inet_ntop(AF_INET, &(send_addr.sin_addr), addrbuf, INET_ADDRSTRLEN); // inet_ntoa(send_addr.sin_addr);
    addr_str = addrbuf;
    send_len = 0x20;
    send_len += 0xc;

    memset(send_buf.data(), 0, send_buf.size());
    InitBuf(send_buf.data(), send_len);

    active = 1;
    timestamp = GetTickCount();
}


void HatBrowserProgress::PingServer() { //4b1dbb
    if (active == 0)
        return;

    error = 0;

    send_len = 0x20;
    send_len += 0xc;

    send_buf[2] = 0;
    send_buf[3] = 0;

    *(uint32_t *)(send_buf.data() + 8) = GetTickCount();
    *(uint16_t*)(send_buf.data() + 6) = send_serial;

    send_serial++;

    *(uint16_t*)(send_buf.data() + 2) = CalcCrc(send_buf.data(), send_len);

    int32_t res = sendto(sock, (char*)send_buf.data(), send_len, 0, (sockaddr*)&send_addr, 0x10);
    timestamp = GetTickCount();

    if (res == -1) {
        if (WSAGetLastError() == WSAETIMEDOUT)
            error = -6;
        else
            error = -7;
        return;
    }

    err = recvfrom(sock, (char*)recv_buf.data(), recv_buf.size(), 0, (sockaddr*)&recv_addr, &recv_addr_len);
    timestamp = GetTickCount();

    if (err == -1) {
        if (WSAGetLastError() == WSAETIMEDOUT) {
            EnterCriticalSection(&cs);
            timeouts++;
            error = -8;
            LeaveCriticalSection(&cs);
        }
        else
            error = -9;
        return;
    }
        
    uint32_t* rdat = (uint32_t*)(recv_buf.data() + (recv_buf[0] & 0xf) * 4);
    if (*(uint8_t*)rdat != 0) {
        error = -10;
        return;
    }

    if ((rdat[1] & 0xffff) != (GetCurrentProcessId() & 0xffff)) {
        error = -10;
        return;
    }

    EnterCriticalSection(&cs);
    last_rtt = timestamp - rdat[2];
    recv_serial++;
    total_rtt += last_rtt;

    if (min_rtt >= last_rtt)
        min_rtt = last_rtt;

    if (last_rtt >= max_rtt)
        max_rtt = last_rtt;
    LeaveCriticalSection(&cs);
}

int32_t HatBrowserProgress::GetPingError() { //453df0
    return error;
}


// 44c1a9
// client: 44f11d
int32_t VisHatBrowserDlg::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    case VIS_MSG_LIST_ACTIVATE:
        if (wparam != 1) {
            return VisScreen::MsgProc(msg, wparam, lparam);
        }
        // fallthrough
    case VIS_MSG_HIDE_SCREEN: {
        if (this->FindChild(0x14)->TestFlags(1) != 0) {
            VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
            CString item = list->GetItem(list->GetSelectedIndex());
            int32_t field_pos = GetFieldPos(item, 7);
            main_wnd->net_session.connect_address = item.Mid(field_pos);
            main_wnd->net_session.connect_address = main_wnd->net_session.connect_address.Left(main_wnd->net_session.connect_address.Find('|'));
#ifdef A2CLIENT
            if (main_wnd->hat_settings.deathmatch == 2) {
                ((VisRadioType2*)this->FindChild(0x31))->WriteData(&main_wnd->hat_settings.store);
                main_wnd->m_GameSession.color = (main_wnd->hat_settings.store != 0) ? 1 : 3;
            }
#endif
            VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
#ifdef A2CLIENT
            if (this->browser != nullptr) {
                SetEvent(this->exit_event);
            }
#endif
        }
        return 1;
    }
    case VIS_MSG_CLOSE_SCREEN:
        VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
#ifdef A2CLIENT
        if (this->browser != nullptr) {
            SetEvent(this->exit_event);
        }
#endif
        return 1;
    case VIS_MSG_SET_OPTION:
    case VIS_MSG_LIST_SELECT:
        if (wparam == 1) {
            if ((int32_t)lparam >= 0) {
                VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
                list->VMethod9();
                this->FindChild(0x14)->ChangeFlags(1, list->IsItemEnabled(list->GetSelectedIndex()));
                this->FindChild(0x14)->VMethod9();
#ifdef A2CLIENT
                if (this->browser) {
                    EnterCriticalSection(&this->connect_cs);
                    CString item = list->GetItem(list->GetSelectedIndex());
                    int32_t field_pos = GetFieldPos(item, 7);
                    this->connect_addr = item.Mid(field_pos);
                    this->connect_addr = this->connect_addr.Left(this->connect_addr.Find('|'));
                    LeaveCriticalSection(&this->connect_cs);
                }
#endif
            }
            return 1;
        }
        return 0;
    case VIS_MSG_HATBROWSER_REFRESH: {
        VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
        int32_t sel_index = list->GetSelectedIndex();
        int32_t vis_start = list->vis_start_index;
        list->entries.RemoveAll();
        list->selected_index = -1;
        list->state = wparam;
        if (list->state == 0) {
            list->vis_start_index = 0;
            list->SetSelectedIndex(0);
        } else {
            for (int32_t i = 0; i < g_WwwServerList.GetSize(); i++) {
                list->AddItem(g_WwwServerList[i]);
            }
            if (g_WwwServerList.GetSize() <= sel_index) {
                sel_index = g_WwwServerList.GetSize() - 1;
            }
#ifdef A2CLIENT
            if (g_WwwServerList.GetSize() > 0 && sel_index < 0) {
                sel_index = 0;
            }
#endif
            if (g_WwwServerList.GetSize() <= vis_start) {
                vis_start = g_WwwServerList.GetSize() - 1;
            }
            list->vis_start_index = vis_start;
            list->SetSelectedIndex(sel_index);
        }
        this->MsgProc(VIS_MSG_SET_OPTION, 1, list->GetSelectedIndex());
        list->VMethod9();
        return 1;
    }
    case VIS_MSG_HATBROWSER_ABORT: {
        VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
        list->state = -1;
        list->entries.RemoveAll();
        list->selected_index = -1;
        list->vis_start_index = 0;
        list->SetSelectedIndex(0);
        list->VMethod9();

        int32_t res = 1; // 44c55e: left uninitialized when hat_settings.ishat == 0
#ifdef A2CLIENT
        if (main_wnd->hat_settings.ishat == 0) {
            res = main_wnd->CheckWebVersion(main_wnd->hat_settings.hatip);
        } else if (main_wnd->LeaveServerMode() == 0) {
            g_CLlDriver.Close();
            main_wnd->PostMessage(0x446, 0, 0);
            return VisScreen::MsgProc(msg, wparam, lparam);
        }
#else
        if (main_wnd->hat_settings.ishat != 0) {
            if (main_wnd->LeaveServerMode() == 0) {
                g_CLlDriver.Close();
                main_wnd->PostMessage(0x446, 0, 0);
                return VisScreen::MsgProc(msg, wparam, lparam);
            }
            res = main_wnd->CheckWebVersion(main_wnd->hat_settings.hatip);
        }
#endif
        this->MsgProc(VIS_MSG_HATBROWSER_REFRESH, res, 0);
        return VisScreen::MsgProc(msg, wparam, lparam);
    }

#ifdef A2CLIENT
    case VIS_MSG_HATBROWSER_PROGRESS: {
        // client-only: progress/status pump posted by the worker thread. Requires the
        // a2mgr-installed browser object; without it the retail client passthroughs too.
        if (main_wnd->hat_settings.ishat && this->browser) {
            HatBrowserProgress* prog = (HatBrowserProgress*)this->browser;
            VisLabel* lbl1 = (VisLabel*)this->FindChild(0x2d);
            VisLabel* lbl2 = (VisLabel*)this->FindChild(0x2e);
            CString s1;
            CString s2;
            EnterCriticalSection(&prog->cs);
            int32_t step = (int32_t)wparam;
            if (prog->recv_serial + prog->timeouts != 0) {
                int32_t total = prog->recv_serial + prog->timeouts;
                s1.Format(txt_patch.GetLine(0x7f), prog->min_rtt, (prog->recv_serial != 0) ? prog->total_rtt / prog->recv_serial : 0, prog->max_rtt, (total != 0) ? prog->timeouts * 100 / total : 0);
                if (step == 0) {
                    s2.Format(txt_patch.GetLine(0x80), prog->last_rtt, total);
                }
                else {
                    s2 = txt_patch.GetLine(0x80 - step);
                }
            }
            else {
                if (step != 0) {
                    s1 = " ";
                    s2 = txt_patch.GetLine(0x80 - step);
                }
                else {
                    s1 = " ";
                    s2 = " ";
                }
            }
            LeaveCriticalSection(&prog->cs);
            lbl1->text = s1;
            lbl1->VMethod9();
            lbl2->text = s2;
            lbl2->VMethod9();
        }
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
#endif
    
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
}

// 44bc35
VisHatBrowserDlg::VisHatBrowserDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : VisWindow(_id, l, t, r, b, nullptr) {

    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    if (mwnd->hat_settings.ishat)
    {
        browser = new HatBrowserProgress();
        exit_event = CreateEventA(nullptr, 0, 0, nullptr);
        ack_event = CreateEventA(nullptr, 0, 0, nullptr);
        InitializeCriticalSection(&connect_cs);
        worker_thread = AfxBeginThread(ThreadProc, this, 0, 0, 0, 0);
        worker_thread->m_bAutoDelete = 1;
    }
    else
        browser = nullptr;
}

#ifdef A2CLIENT
// client: 4527dd
HatPlayerListWindow::HatPlayerListWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const void* _payload, int32_t _mode)
    : VisMessageBox(_id, l, t, r, b, _payload, nullptr, 0, txt_patch.GetLine(_mode + 0xd1)) {
    this->mode = _mode;
}

// client: 452837
HatPlayerListWindow::~HatPlayerListWindow() {
    // retail writes MainWindow+0x634 - the client layout's shutdown_pending
    // slot, repurposed as the hat player-list visibility flag
    ((MainWindow*)AfxGetMainWnd())->shutdown_pending = 0;
}

// client: 452a02
int32_t HatPlayerListWindow::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_HIDE_SCREEN && this->mode != 0) {
        MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
        wnd->vis_map_context->msglog.AddWithCheck(txt_patch.GetLine(0x45), clr_log_sblack, 10000);
    }
    return VisMessageBox::MsgProc(msg, wparam, lparam);
}

// client: 452897
CVisualObject* HatPlayerListWindow::VMethod30(const void* payload, const RECT& r) {
    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
    // hat player-list visibility flag (client MainWindow+0x634, see dtor above)
    wnd->shutdown_pending = 1;

    RECT list_r = r;
    list_r.right -= 0x18;

    HatPlayerScoreList* list = new HatPlayerScoreList(2, list_r, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0x29b, nullptr);
    list->ReadData(&payload);
    list->SetSelectedIndex(g_hat_selected_index);
    list->scores.SetSize(wnd->vis_map_context->hat_score_list.GetSize(), -1);
    for (int32_t i = 0; i < list->scores.GetSize(); i++) {
        list->scores[i] = wnd->vis_map_context->hat_score_list[i];
    }
    this->AddChild(list);

    this->AddChild(new VisScrollBar(0x29b, list_r.right, list_r.top, list_r.right + 0x18, list_r.bottom, nullptr));
    return list;
}

// client: 454970
HatPlayerScoreList::HatPlayerScoreList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
    : VisListBox(_id, r, _font, _clr1, _clr2, _scrollid, hint) {
}

// client: 454a20
HatPlayerScoreList::~HatPlayerScoreList() {
}

// client: 45265b
void HatPlayerScoreList::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    (void)clr;
    if (!this->IsValidIndex(idx)) {
        return;
    }

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
    bool is_me = (this->entries[idx] == wnd->vis_map_context->my_main_unit->name);
    uint16_t* color = is_me ? (uint16_t*)clrsh_ShockingBlack : g_colors_human_pals[15];

    CString str;
    str.Format("%d. %s", idx + 1, (const char*)this->entries[idx]);
    this->font->DrawTextWithShadow(pos.x + 5, pos.y, str, 0, color, 1);

    str.Format("%d", this->scores[idx]);
    int w = this->font->GetStrWidth(str);
    this->font->DrawTextWithShadow(pos.x + this->rect.Width() - 10 - w, pos.y, str, 1, color, 1);
}
#endif

// 44fed0
VisHatBrowserDlg::~VisHatBrowserDlg() {
}

UINT __cdecl VisHatBrowserDlg::ThreadProc(void* pv) { //44e391
    VisHatBrowserDlg* dlg = (VisHatBrowserDlg*)pv;

    while (WaitForSingleObject(dlg->exit_event, 0) != WAIT_OBJECT_0) {
        ResetEvent(dlg->ack_event);
        EnterCriticalSection(&dlg->connect_cs);

        CString addr = dlg->connect_addr;

        if (!addr.IsEmpty()) {
            dlg->connect_addr.Empty();
            LeaveCriticalSection(&dlg->connect_cs);

            int padr = addr.ReverseFind(':');
            if (padr != -1)
                addr = addr.Left(padr);

            dlg->browser->Init(addr);

            if (WaitForSingleObject(dlg->exit_event, 0) == 0) {
                SetEvent(dlg->ack_event);
                break;
            }

            PostMessageA(g_MainWndHWND, 0x48c, dlg->browser->GetPingError(), 0);
        }
        else
            LeaveCriticalSection(&dlg->connect_cs);

        dlg->browser->PingServer();
        PostMessageA(g_MainWndHWND, 0x48c, dlg->browser->GetPingError(), 0);
        SetEvent(dlg->ack_event);

        if (WaitForSingleObject(dlg->exit_event, 500) == 0)
            break;
    }
    return 0;
}


// 449F06
VisNetTcpIp::VisNetTcpIp(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CString* _ip)
    : VisMessageBox(_id, l, t, r, b, (const void*)_ip, txt_dialogs.GetLine(0x74), 1, txt_dialogs.GetLine(0x95)) {
    this->ip = _ip;
}

// 44FD10 (this address is the deleting dtor ??_G; the real dtor 44FD40 only calls the base dtor)
VisNetTcpIp::~VisNetTcpIp() = default;

// 449F67
void VisNetTcpIp::VMethod31(int32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (code == VIS_MSG_HIDE_SCREEN) {
        char buffer[256];
        this->FindChild(2)->WriteData(buffer);
        *this->ip = buffer;
        main_wnd->PostMessage(MAIN_MSG_SHOW_NET_SESSIONS, 0, 0);
        main_wnd->net_session.connect_address = buffer;
    } else {
        g_CLlDriver.Close();
        main_wnd->PostMessage(MAIN_MSG_SHOW_CONNECT_DLG, 0, 0);
    }
}

// 44A014
CVisualObject* VisNetTcpIp::VMethod30(const void* data, const RECT& r) {
    VisTextBox* textbox = new VisTextBox(2, r.left, r.top, r.right, r.top + 0x18, g_font1, clrsh_TechBlack, txt_dialogs.GetLine(0x75));
    this->AddChild(textbox);
    textbox->ReadData(*(CString*)data);
    return textbox;
}

// 44A140
// client: 44c815
void VisHatServerListDlg::VMethod31(int32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (code == VIS_MSG_HIDE_SCREEN) {
        this->FindChild(4)->WriteData(&this->hat_settings->ishat);
        char buffer[256];
        this->FindChild(2)->WriteData(buffer);
        if (this->hat_settings->ishat == 0) {
            this->hat_settings->hatip = buffer;
        } else {
            this->hat_settings->hatprogip = buffer;
        }
        this->FindChild(6)->WriteData(buffer);
        this->hat_settings->login = buffer;
        this->FindChild(8)->WriteData(buffer);
        this->hat_settings->password = buffer;
        this->FindChild(3)->WriteData(&this->hat_settings->deathmatch);
#ifdef A2CLIENT
        if (this->hat_settings->deathmatch == 1) {
            this->hat_settings->deathmatch = 3;
        }
#endif
        this->FindChild(9)->WriteData(&this->hat_settings->store);
#ifndef A2CLIENT
            main_wnd->net_session.internet_game = (this->hat_settings->ishat == 0) ? 1 : 0;
#else
            main_wnd->net_session.internet_game = 1;
#endif
        if (this->hat_settings->ishat == 0) {
            main_wnd->PostMessage(MAIN_MSG_SHOW_CHAR_LOADER, 0, 0);
        } else {
            main_wnd->PostMessage(MAIN_MSG_CONNECT_DONE, 0, 0);
        }
    } else {
        main_wnd->PostMessage(MAIN_MSG_MAIN_MENU, 0, 0);
    }
}

// 44A8A8
int32_t VisHatServerListDlg::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg != VIS_MSG_SET_OPTION) {
        return VisMessageBox::MsgProc(msg, wparam, lparam);
    }
    if (wparam != 4) {
        return 0;
    }

    CVisualObject* store_child = this->FindChild(9);
    if (store_child->TestFlags(1) == lparam || (int32_t)lparam > 1 || (int32_t)lparam < 0) {
        return 1;
    }

    store_child->ChangeFlags(1, lparam != 0);
    this->FindChild(3)->ChangeFlags(1, lparam != 0);
    this->FindChild(6)->ChangeFlags(1, lparam != 0);
    this->FindChild(8)->ChangeFlags(1, lparam != 0);

    char buffer[256];
    this->FindChild(2)->WriteData(buffer);
    if (lparam == 1) {
        this->FindChild(2)->ReadData(this->hat_settings->hatprogip);
        this->hat_settings->hatip = buffer;
    } else {
        this->FindChild(2)->ReadData(this->hat_settings->hatip);
        this->hat_settings->hatprogip = buffer;
    }
    this->VMethod9();
    return 1;
}

// 44A34D
// client: 44ca33
CVisualObject* VisHatServerListDlg::VMethod30(const void* data, const RECT& r) {
    HatSettings* hat = (HatSettings*)data;
    // the client lays out everything below the ip box 0x18 lower
#ifdef A2CLIENT
    const int32_t dy = 0x18;
#else
    const int32_t dy = 0;
#endif

    VisTextBox* ip_box = new VisTextBox(2, r.left, r.top, r.right, r.top + 0x18, g_font1, clrsh_TechBlack, txt_patch.GetLine(0x70));
    this->AddChild(ip_box);
    if (hat->ishat == 0) {
        ip_box->ReadData(hat->hatip);
    } else {
        ip_box->ReadData(hat->hatprogip);
    }

    VisRadioType2* deathmatch_radio = new VisRadioType2(3, r.left, 0x6c, r.right, 0x9c + dy, g_font1, p_clrsh_Black, nullptr);
    deathmatch_radio->AddEntry(txt_patch.GetLine(0x68));
#ifdef A2CLIENT
    deathmatch_radio->AddEntry(txt_patch.GetLine(0xd5));
    deathmatch_radio->AddEntry(txt_patch.GetLine(0x6d));
#else
    deathmatch_radio->AddEntry(txt_patch.GetLine(0x69));
#endif

    this->AddChild(deathmatch_radio);
    int32_t dm_display = hat->deathmatch;
#ifdef A2CLIENT
    if (dm_display == 3) {
        dm_display = 1;
    }
#endif

    deathmatch_radio->ReadData(&dm_display);
    if (hat->ishat == 0) {
        deathmatch_radio->ChangeFlags(1, false);
    }

    VisRadioType2* ishat_radio = new VisRadioType2(4, r.left, 0xa8 + dy, r.right, 0xd8 + dy, g_font1, p_clrsh_Black, nullptr);
    ishat_radio->AddEntry(txt_patch.GetLine(0x78));
    ishat_radio->AddEntry(txt_patch.GetLine(0x79));
    this->AddChild(ishat_radio);
    ishat_radio->ReadData(&hat->ishat);

    this->AddChild(new VisLabel(5, r.left, 0xe4 + dy, r.right, 0xfc + dy, txt_patch.GetLine(0x6e), g_font1, p_clrsh_Black, 0));

    VisTextBox* login_box = new VisTextBox(6, r.left, 0xfc + dy, r.right, 0x114 + dy, g_font1, p_clrsh_Black, nullptr);
    this->AddChild(login_box);
    login_box->ReadData(hat->login);
    if (hat->ishat == 0) {
        login_box->ChangeFlags(1, false);
    }

    this->AddChild(new VisLabel(7, r.left, 0x12c + dy, r.right, 0x144 + dy, txt_patch.GetLine(0x6f), g_font1, p_clrsh_Black, 0));

    VisTextBox* password_box = new VisTextBox(8, r.left, 0x144 + dy, r.right, 0x15c + dy, g_font1, p_clrsh_Black, nullptr);
    this->AddChild(password_box);
    password_box->ReadData(hat->password);
#ifdef A2CLIENT
    password_box->is_password = 1;
#endif
    if (hat->ishat == 0) {
        password_box->ChangeFlags(1, false);
    }

    VisRadioType1* store_radio = new VisRadioType1(9, r.left, 0x168 + dy, r.right, 0x180 + dy, g_font1, p_clrsh_Black, txt_patch.GetLine(0x7b));
    store_radio->AddEntry(txt_patch.GetLine(0x7a));
    this->AddChild(store_radio);
    store_radio->ReadData(&hat->store);
    if (hat->ishat == 0) {
        store_radio->ChangeFlags(1, false);
    }

    return password_box;
}

// 44A0E2
VisHatServerListDlg::VisHatServerListDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, HatSettings* hat)
    : VisMessageBox(_id, l, t, r, b, hat, txt_patch.GetLine(0x67), 1, txt_patch.GetLine(0x66)) {
    this->hat_settings = hat;
}

// 44FD90
VisHatServerListDlg::~VisHatServerListDlg() {
}


// 44604C
CVisualObject* VisConnectionDlg::VMethod30(const void* data, const RECT& r) {
    RECT& rect = const_cast<RECT&>(r);
    rect.right -= 0x18;

    VisListBox* list = new VisListBox(2, r, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 10, txt_dialogs.GetLine(0x51));
    this->AddChild(list);

    VisScrollBar* scrollbar = new VisScrollBar(10, list->GetRect().right, list->GetRect().top, list->GetRect().right + 0x18, list->GetRect().bottom, nullptr);
    this->AddChild(scrollbar);

    CStringArray* names = new CStringArray();

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    int32_t no_serial = (main_wnd->sessionMode == SESSION_SERVER) ? 1 : 0;

    int32_t count = 0;
    g_CLlDriver.EnumConnections(&this->protocols, &count);
    for (int32_t i = 0; i < count; i++) {
        if (this->protocols[i].typ == 4 || (no_serial != 0 && (this->protocols[i].typ == 1 || this->protocols[i].typ == 0))) {
            memmove(this->protocols + i, this->protocols + i + 1, (count - i - 1) * sizeof(CLlConn));
            count--;
            i--;
        } else {
            char enc[256];
            const char* name = this->protocols[i].name;
            uint32_t j = 0;
            for (; j < strlen(name); j++) {
                enc[j] = EncodeChar(name[j]);
            }
            enc[j] = '\0';
            names->Add(enc);
        }
    }
    list->ReadData(&names);

    if ((uint32_t)*this->pSelected >= (uint32_t)(list->GetItemCount() - 1)) {
        *this->pSelected = list->GetItemCount() - 1;
    }
    list->SetSelectedIndex(*this->pSelected);
    list->SetCaptionLabel((VisLabel*)this->FindChild(-1));

    delete names;
    return list;
}

// 445EBD
void VisConnectionDlg::VMethod31(int32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (code == VIS_MSG_HIDE_SCREEN) {
        VisListBox* list = (VisListBox*)this->FindChild(2);
        *this->pSelected = list->GetSelectedIndex();
        int32_t typ = this->protocols[*this->pSelected].typ;
        if (g_CLlDriver.ResetProvider(typ) == 0) {
            VisMessageBoxWithList* msg = new VisMessageBoxWithList(1, 100, 100, 0x21c, 0x17c, TxtFile::AllLines[0x98], nullptr, 0);
            main_wnd->ModalScreen(msg);
            main_wnd->PostMessage(MAIN_MSG_MAIN_MENU, 0, 0);
        } else {
            if (typ == 3) {
                main_wnd->PostMessage(MAIN_MSG_SHOW_TCP_SETTINGS, 0, 0);
            } else if (typ == 0) {
                main_wnd->PostMessage(MAIN_MSG_SHOW_SERIAL_SETTINGS, 0, 0);
            } else if (typ == 1) {
                main_wnd->PostMessage(MAIN_MSG_SHOW_PHONE_BOOK, 0, 0);
            } else if (typ == 2) {
                main_wnd->PostMessage(MAIN_MSG_SHOW_NET_SESSIONS, 0, 0);
            }
        }
    }
    if (code == VIS_MSG_CLOSE_SCREEN) {
        main_wnd->PostMessage(MAIN_MSG_MAIN_MENU, 0, 0);
    }
}

// 445E52
VisConnectionDlg::VisConnectionDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, int32_t* _payload)
    : VisMessageBox(_id, l, t, r, b, _payload, txt_dialogs.GetLine(0x50), 1, txt_dialogs.GetLine(0x92)) {
    this->protocols = nullptr;
    this->pSelected = _payload;
}

// 44FA40
VisConnectionDlg::~VisConnectionDlg() {
}



#ifdef A2CLIENT
VisHatSession::VisHatSession(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) {} //4e850e

void VisHatSession::VMethod7() { // 4e8545
    CDWordArray scores;
    CDWordArray colors;
    CStringArray names;
    int num = 0;

    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    if (mwnd->hat_settings.deathmatch == 3) {
        CString st = mwnd->vis_map_context->my_main_unit->name;
        for (int i = 0; i < mwnd->vis_map_context->hat_name_list.GetSize(); i++) {
            if (st == mwnd->vis_map_context->hat_name_list[i]) {
                scores.Add(mwnd->vis_map_context->hat_score_list[i]);
                colors.Add(mwnd->vis_map_context->my_main_unit->color);
                st.Format("%d/%d %s", i + 1, mwnd->vis_map_context->hat_name_list.GetSize(), mwnd->vis_map_context->my_main_unit->name);
                names.Add(st);
                num = 1;
                break;
            }
        }
    }
    else {
        for (int i = 0; i < 16; i++) {
            if (mwnd->vis_map_context->hat_cursor_slots[i] != 0xf0000003 && i + 16 < mwnd->vis_map_context->map_players.GetSize()) {
                MapPlayerData* pl = mwnd->vis_map_context->map_players[i + 16];
                if (pl) {
                    num++;

                    int j = 0;
                    while (j < scores.GetSize() && mwnd->vis_map_context->hat_cursor_slots[i] < scores[j])
                        j++;

                    scores.InsertAt(j, mwnd->vis_map_context->hat_cursor_slots[i]);
                    names.InsertAt(j, pl->name);
                    colors.InsertAt(j, pl->color);
                }
            }
        }
    }

    int32_t idoff = 0;
    if (mwnd->hat_settings.deathmatch == 2 && mwnd->vis_map_context->hat_item_slot12 != 0)
        idoff = 2;

    int32_t hsz = idoff + 2 + num;

    rect.bottom = rect.top + 72 + ((hsz + 2) / 3) * 32;
    CRect local_20 = ClientRectToScreen(rect);

    local_20.bottom -= 8;
    local_20.right -= 8;

    gfx_interface_lm->DrawBrightened(local_20.right - 24, local_20.top + 8,     12, 6, 0);
    gfx_interface_lm->DrawBrightened(local_20.left  + 8,  local_20.bottom - 24, 15, 6, 0);
    gfx_interface_lm->DrawBrightened(local_20.right - 24, local_20.bottom - 24, 17, 6, 0);
    
    for (int i = 0; i < (local_20.Width() - 64) / 48; i++)
        gfx_interface_lm->DrawBrightened(local_20.left + 40 + i * 48, local_20.bottom - 24, 16, 6, 0);

    for (int i = 0; i < (local_20.Height() - 64) / 32; i++)
        gfx_interface_lm->DrawBrightened(local_20.right - 24, local_20.top + 40 + i * 32, 14, 6, 0);

    gfx_interface_lm->Draw(local_20.left, local_20.top, 10, 0, 0);
    gfx_interface_lm->Draw(local_20.right - 32, local_20.top, 12, 0, 0);
    gfx_interface_lm->Draw(local_20.left, local_20.bottom - 32, 15, 0, 0);
    gfx_interface_lm->Draw(local_20.right - 32, local_20.bottom - 32, 17, 0, 0);

    for (int i = 0; i < (local_20.Width() - 64) / 48; i++) {
        gfx_interface_lm->Draw(local_20.left + 32 + i * 48, local_20.top, 11, 0, 0);
        gfx_interface_lm->Draw(local_20.left + 32 + i * 48, local_20.bottom - 32, 16, 0, 0);
    }

    for (int i = 0; i < (local_20.Height() - 64) / 32; i++) {
        gfx_interface_lm->Draw(local_20.left, local_20.top + 32 + i * 32, 13, 0, 0);
        gfx_interface_lm->Draw(local_20.right - 32, local_20.top + 32 + i * 32, 14, 0, 0);
    }

    for (int i = 0; i < (local_20.Width() - 64) / 48; i++) {
        for (int j = 0; j < (local_20.Height() - 64) / 32; j++) {
            gfx_interface_lm->Draw(local_20.left + 32 + i * 48, local_20.top + 32 + j * 32, 9, 0, 0);
        }
    }

    int32_t xoff = rect.left + 32;
    int32_t yoff = rect.top + 32;
    LockSurface2();
    if (mwnd->vis_map_context->hat_item_slot12 == 0) {
        g_font2->DrawTextWithShadow(rect.left + rect.Width() / 2, yoff, txt_patch.GetLine(181), 2, clrsh_TechBlack, 1);
    }
    else if (mwnd->vis_map_context->hat_player_count < 0x7fffffff) {
        CString local_88;
        local_88.Format(txt_patch.GetLine(186), mwnd->vis_map_context->hat_player_count);
        g_font2->DrawTextWithShadow(rect.left + rect.Width() / 2, yoff, local_88, 2, clrsh_TechBlack, 1);
    }
    else {
        g_font2->DrawTextWithShadow(rect.left + rect.Width() / 2, yoff, txt_patch.GetLine(180), 2, clrsh_TechBlack, 1);
    }

    yoff += 20;
    if (mwnd->hat_settings.deathmatch == 2 && mwnd->vis_map_context->hat_item_slot12 != 0) {
        CString local_8c;
        CString local_90;
        local_8c.Format("%d", mwnd->vis_map_context->hat_item_slot13);
        local_90.Format("%d", mwnd->vis_map_context->hat_item_slot14);

        int32_t vy1;
        int32_t vy2;

        if (mwnd->vis_map_context->hat_item_slot13 < mwnd->vis_map_context->hat_item_slot14) {
            vy2 = yoff;
            vy1 = yoff + 11;
        }
        else {
            vy1 = yoff;
            vy2 = yoff + 11;
        }

        g_font2->DrawTextWithShadow(xoff, vy1, txt_patch.GetLine(184), 0, g_colors_human_pals[2], 1);
        g_font2->DrawTextWithShadow(xoff, vy2, txt_patch.GetLine(185), 0, g_colors_human_pals[0], 1);
        g_font2->DrawTextWithShadow(rect.right - 32, vy1, local_8c, 1, g_colors_human_pals[2], 1);
        g_font2->DrawTextWithShadow(rect.right - 32, vy2, local_90, 1, g_colors_human_pals[0], 1);
    }

    for (int i = 0; i < num; i++) {
        g_font2->DrawTextWithShadow(xoff, yoff, names[i], 0, g_colors_human_pals[colors[i]], 1);
        CString st;
        if (mwnd->vis_map_context->hat_item_slot12 == 0) {
            st = txt_patch.GetLine(183 - scores[i]);
        }
        else {
            st.Format("%d", scores[i]);
        }
        g_font2->DrawTextWithShadow(rect.right - 32, yoff, st, 1, g_colors_human_pals[colors[i]], 1);

        yoff += 11;
    }
    UnlockSurface2();
}

#endif







