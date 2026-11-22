#include "vis_core.h"
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
#include "gameplay.h"
#include "vis_playing_area.h"
#include "vis_net.h"
#ifdef A2CLIENT_PATCH
#include "vis_menu.h"
#endif



char g_HintBuf[223]; //4c9a6f GetHint static buffer







// 4d6d40 (client: 4db650)
CVisualObject::CVisualObject() {
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

// 4d6f5f (client: 4db86f)
CVisualObject::CVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str) {
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

// 4d6e50 (client: 4db760)
CVisualObject::CVisualObject(int32_t _id, const RECT& r, const char* str) {
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


// 4d707a (client: 4db98a)
CVisualObject::~CVisualObject() {
    for (int32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        if (obj) {
            delete obj;
        }
    }
}


// 4d82b3 (client: 4dcbc3)
void CVisualObject::Dump(CDumpContext& dc) const {
    
    //dc << "CVisualObject";
}


// 4d7c15 (client: 4dc525)
const char* CVisualObject::GetHint() {

    if (this->TestFlags(FLAG_HIDDEN)) {
        return nullptr;
    }

    return this->hint;
}

// 4d7bf9 (client: 4dc509)
void CVisualObject::SetHint(const char* _name) {

    this->hint = _name;
}


// 4d7a03 (client: 4dc313)
void CVisualObject::ChangeFlags(uint32_t _flags, bool setunset) {

    if (setunset) {
        this->flags |= _flags;
    } else {
        this->flags &= ~_flags;
    }
}

// 4386e0 (client: 43ad20)
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


// 4d79ed (client: 4dc2fd)
uint32_t CVisualObject::TestFlags(uint32_t _flags) {

    return this->flags & _flags;
}


// 4d7a3a (client: 4dc34a)
void CVisualObject::SetCursorOver(bool isOver) {

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


// 4d7aa9 (client: 4dc3b9)
void CVisualObject::SetFocus(bool isFocus) {

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


// 4d7c40 (client: 4dc550)
void CVisualObject::VMethod7() {

    if (this->TestFlags(FLAG_HIDDEN)) {
        return;
    }

    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        if (!obj->TestFlags(FLAG_HIDDEN)) {
            obj->VMethod7();
        }
    }
}


// 41ed60 (client: 420190)
void CVisualObject::VMethod8(CRect* rect) {
}


// 4d7cc8 (client: 4dc5d8)
void CVisualObject::VMethod9() {
    if (this->TestFlags(FLAG_HIDDEN)) {
        return;
    }

    if (g_IsServer != 0) {
        return;
    }

    this->VMethod7();
    this->VMethod10();
}


// 4d7d03 (client: 4dc613)
void CVisualObject::VMethod10() {
    CRect r = this->ClientRectToScreen(this->rect);
    gfxFlushRect(r);
}


// 4d7d99 (client: 4dc6a9)
void CVisualObject::WriteData(void* buf) {

    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        obj->WriteData(buf);
        buf = (uint8_t*)buf + obj->DataSize();
    }
}


// 4d7d37 (client: 4dc647)
uint32_t CVisualObject::DataSize() {

    uint32_t sz = 0;
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        sz += this->childs[i]->DataSize();
    }
    return sz;
}


// 4d7e02 (client: 4dc712)
void CVisualObject::ReadData(const void* buf) {

    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        obj->ReadData(buf);
        buf = (const uint8_t*)buf + obj->DataSize();
    }
}


// 41ed70 (client: 420950)
int32_t CVisualObject::OnMouseMove(uint32_t wparam, CPoint pos) {
    return 0;
}

// 41ed80 (client: 4201a0)
int32_t CVisualObject::OnWmUser(uint32_t wparam, CPoint pos) {
    return 0;
}

// 438850 (client: 43a7c0)
int32_t CVisualObject::OnLButtonDown(uint32_t wparam, CPoint pos) {
    return 0;
}

// 438860 (client: 43a7d0)
int32_t CVisualObject::OnLButtonUp(uint32_t wparam, CPoint pos) {
    return 0;
}

// 438870 (client: 43a7e0)
int32_t CVisualObject::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    return 0;
}

// 438880 (client: 43a600)
int32_t CVisualObject::OnRButtonDown(uint32_t wparam, CPoint pos) {
    return 0;
}

// 438890 (client: 43a610)
int32_t CVisualObject::OnRButtonUp(uint32_t wparam, CPoint pos) {
    return 0;
}

// 41ed90 (client: 4201b0)
int32_t CVisualObject::OnRButtonDblClk(uint32_t wparam, CPoint pos) {
    return 0;
}

// 4388a0 (client: 43a620)
int32_t CVisualObject::OnKeyDown(uint32_t wparam) {
    return 0;
}

// 41eda0 (client: 4201c0)
int32_t CVisualObject::OnKeyUp(uint32_t wparam) {
    return 0;
}

// 4388b0 (client: 43a630)
int32_t CVisualObject::OnChar(uint32_t wparam) {
    return 0;
}


// 4d7fc9 (client: 4dc8d9)
int32_t CVisualObject::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {

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


// 4d7e6b (client: 4dc77b)
int32_t CVisualObject::MsgProcOnChilds(uint32_t msg, uint32_t wparam, uint32_t lparam) {
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

// split from 4d71f3 (client: 4dbb03)
void CVisualObject::ClientPtToScreen(CPoint* out, const CPoint& _point) {
    *out = _point;
    for (CVisualObject* pobj = this->parent; pobj; pobj = pobj->parent) {
        *out += pobj->rect.TopLeft();
    }
}

// split from 4d7283 (client: 4dbb93)
void CVisualObject::ClientRectToScreen(CRect* out, const CRect& _rect) {
    this->ClientPtToScreen(&(out->TopLeft()), _rect.TopLeft());
    this->ClientPtToScreen(&(out->BottomRight()), _rect.BottomRight());
}

// split from 4d71f3 (client: 4dbb03)
CPoint CVisualObject::ClientPtToScreen(const CPoint& _point) {
    CPoint out = _point;
    for (CVisualObject* pobj = this->parent; pobj; pobj = pobj->parent) {
        out += pobj->rect.TopLeft();
    }
    return out;
}

// split from 4d7283 (client: 4dbb93)
CRect CVisualObject::ClientRectToScreen(const CRect& _rect) {
    CRect out;
    out.TopLeft() = this->ClientPtToScreen(_rect.TopLeft());
    out.BottomRight() = this->ClientPtToScreen(_rect.BottomRight());
    return out;
}


// 4d72c4 (client: 4dbbd4)
void CVisualObject::AddChild(CVisualObject* obj) {
    this->childs.Add(obj);
    obj->parent = this;
}

// 4d72e9 (client: 4dbbf9)
void CVisualObject::RemoveChild(CVisualObject* obj) {
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

// 4d7393 (client: 4dbca3)
void CVisualObject::RemoveChildById(int32_t _id) {
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

// 4d78d4 (client: 4dc1e4)
void CVisualObject::RemoveAllChilds() {
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        this->childs[i]->parent = nullptr;
    }
    this->childs.RemoveAll();

    this->cursor_over_obj = nullptr;
    this->focus_obj = nullptr;
    this->cursor_over_obj_last = nullptr;
    this->last_focus_obj = nullptr;
}

// 4d7430 (client: 4dbd40)
void CVisualObject::DestroyChild(CVisualObject* obj) {
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

// 4d74cf (client: 4dbddf)
void CVisualObject::DestroyChildById(int32_t _id) {
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

// 4d7950 (client: 4dc260)
void CVisualObject::DestroyAllChilds() {
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        delete this->childs[i];
    }
    this->childs.RemoveAll();

    this->cursor_over_obj = nullptr;
    this->focus_obj = nullptr;
    this->cursor_over_obj_last = nullptr;
    this->last_focus_obj = nullptr;
}

// 4d7873 (client: 4dc183)
CVisualObject* CVisualObject::FindChild(int32_t _id) {
    for (uint32_t i = 0; i < this->childs.GetSize(); i++) {
        CVisualObject* obj = this->childs[i];
        if (obj->id == _id) {
            return obj;
        }
    }
    return nullptr;
}


// 4d7b48 (client: 4dc458)
CVisualObject* CVisualObject::GetChildAt(POINT pt) {
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

// 4d82cd (client: 4dcbdd)
void CVisualObject::SetLeftObj(CVisualObject* obj) {
    this->left_obj = obj;
    obj->right_obj = this;
}

// 4d82ec (client: 4dcbfc)
void CVisualObject::SetRightObj(CVisualObject* obj) {
    this->right_obj = obj;
    obj->left_obj = this;
}

// 4d830b (client: 4dcc1b)
void CVisualObject::SetUpObj(CVisualObject* obj) {
    this->up_obj = obj;
    obj->down_obj = this;
}

// 4d832a (client: 4dcc3a)
void CVisualObject::SetDownObj(CVisualObject* obj) {
    this->down_obj = obj;
    obj->up_obj = this;
}

// 4d8349 (client: 4dcc59)
void CVisualObject::SetCaptionLabel(VisLabel* obj) {
    this->caption_obj = obj;
}

// 4d7816 (client: 4dc126)
void CVisualObject::FocusTo(CVisualObject* obj, bool update) {
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

// 4d7674 (client: 4dbf84)
void CVisualObject::TabFocus(bool forward, bool update) {
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
        if (obj->TestFlags(FLAG_ENABLED | FLAG_CAN_FOCUS) == (FLAG_ENABLED | FLAG_CAN_FOCUS)) {
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

// split from 4d7178 (client: 4dba88)
void CVisualObject::SetRect(RECT r) {
    this->rect = r;
}

// split from 4d7178 (client: 4dba88)
void CVisualObject::SetRect(const RECT* r) {
    this->rect = *r;
}

// split from 4d7178 (client: 4dba88)
void CVisualObject::SetRect(int32_t l, int32_t t, int32_t r, int32_t b) {
    this->rect.left = l;
    this->rect.top = t;
    this->rect.right = r;
    this->rect.bottom = b;
}






// 4d835f (client: 4dcc6f)
VisLabel::VisLabel() {
}

// 4d8453 (client: 4dcd63)
VisLabel::VisLabel(int32_t _id, const RECT& r, const char* _text, CGameFont* _font, uint16_t* colorsh, uint32_t align)
    : CVisualObject(_id, r, nullptr) {

    this->text = _text;
    this->font = _font;
    this->color_sh = colorsh;
    this->align_flags = align;
}

// 4d83b6 (client: 4dccc6)
VisLabel::VisLabel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _text, CGameFont* _font, uint16_t* colorsh, uint32_t align)
    : CVisualObject(_id, l, t, r, b, nullptr) {

    this->text = _text;
    this->font = _font;
    this->color_sh = colorsh;
    this->align_flags = align;
}

// 4e3ba0 (client: 453f10)
VisLabel::~VisLabel() {
}


// 4d84e4 (client: 4dcdf4)
void VisLabel::VMethod7() {
    if (this->parent) {
        CRect res = this->ClientRectToScreen(this->rect);

        LockSurface2();

        this->parent->VMethod8(&res);

        int32_t outx = 0;
        int32_t outy = 0;

        if (this->align_flags & CGameFont::ALIGN_RIGHT) {
            outx = res.right;
        } else if (this->align_flags & CGameFont::ALIGN_HCENTER) {
            outx = (res.left + res.right) / 2;
        } else {
            outx = res.left;
        }

        if (this->align_flags & CGameFont::ALIGN_BOTTOM) {
            outy = res.bottom;
        } else if (this->align_flags & CGameFont::ALIGN_VCENTER) {
            outy = (res.top + res.bottom) / 2;
        } else {
            outy = res.top;
        }

        this->font->DrawTextWithShadow(outx, outy, this->text, this->align_flags, this->color_sh, 1);

        UnlockSurface2();
    }
}

// 4d85e6 (client: 4dcef6)
void VisLabel::SetActiveColor(bool isActive) {
    if (isActive) {
        this->color_sh = p_clrsh_Gold;
    } else {
        this->color_sh = p_clrsh_Black;
    }
    this->VMethod9();
}


// 450a40 (client: 453330)
VisButton::~VisButton() {
}

// 4d8f7e (client: 4dd88a)
void VisButton::VMethod7() {
    if (!this->parent) {
        return;
    }

    CRect r2 = this->ClientRectToScreen(this->rect);
    
    LockSurface2();
    this->parent->VMethod8(&r2);

    r2.right -= 1;
    r2.bottom -= 1;

    POINT pp;
#ifdef A2CLIENT_PATCH
    GetLogicalCursorPos(&pp);
#else
    GetCursorPos(&pp);
#endif

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
            this->font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, this->caption, CGameFont::ALIGN_CENTER, p_clrsh_Black, dd);
        } else {
            this->font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, this->caption, CGameFont::ALIGN_CENTER, p_clrsh_Gold, dd);
        }
    } else if (this->mouse_on == 0) {
        this->font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, this->caption, CGameFont::ALIGN_CENTER, clrsh_DullGold, dd);
    } else {
        this->font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, this->caption, CGameFont::ALIGN_CENTER, clrsh_CharlieBrown, dd);
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
        ShadowRect(t, shadow_level::disabled);
    }
    UnlockSurface2();
}

// 4d958f (client: 4dde8b)
int32_t VisButton::OnMouseMove(uint32_t wparam, CPoint pos) {
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

// 4d96a1 (client: 4ddf9d)
int32_t VisButton::OnLButtonDown(uint32_t wparam, CPoint pos) {
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (this->downed) {
        return 0;
    }
    
    this->SetDowned(true);
    return 1;
}

// 4d96e8 (client: 4ddfe4)
int32_t VisButton::OnLButtonUp(uint32_t wparam, CPoint pos) {
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

// 4d9783 (client: 4de07f)
int32_t VisButton::OnKeyDown(uint32_t wparam) {
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (wparam != VK_RETURN) {
        return 0;
    }

    AfxGetMainWnd()->PostMessage(this->msgid, 0, 0);
    return 1;
}

// 4d97d4 (client: 4de0d0)
int32_t VisButton::OnChar(uint32_t wparam) {
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }

    if (EncodeChar(wparam) != this->charid) {
        return 0;
    }

    AfxGetMainWnd()->PostMessage(this->msgid, 0, 0);
    return 1;
}

// 4d8e2d (client: 4dd73b)
VisButton::VisButton(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: CVisualObject(_id, r, hint) {
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
    this->flags |= FLAG_CAN_FOCUS;
}

// 4d8cd0 (client: 4dd5e0)
VisButton::VisButton(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: CVisualObject(_id, l, t, r, b, hint) {
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
    this->flags |= FLAG_CAN_FOCUS;
}

// 4d94b5 (client: 4dddb1)
void VisButton::SetDowned(bool down) {
    if (!down) {
        this->downed = 0;
        if (this->TestFlags(FLAG_OVERCURSOR)) {
            this->SetCursorOver(false);
        }

        this->VMethod9();
    } else {
        g_SfxArray[sfx_id::ui_button]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xdc, 0);

        this->downed = 1;
        if (this->TestFlags(FLAG_OVERCURSOR) == 0) {
            this->SetCursorOver(true);
        }

        this->VMethod9();
    }
}



// 4e3e80 (client: 4e9260)
VisScrollBar::~VisScrollBar() {
}

// 4de6df (client: 4e3060)
void VisScrollBar::SetCursorOver(bool isOver) {
    CVisualObject::SetCursorOver(isOver);
    this->is_mouse_over = isOver;
}

// 4de12c (client: 4e2aad)
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

        ShadowRect(tr, shadow_level::disabled);
    }
    UnlockSurface2();
}


// 4de0d0 (client: 4e2a51)
void VisScrollBar::VMethod9() {
    if (g_IsServer == 0) {
        this->VMethod7();
        CRect tr = this->ClientRectToScreen(this->rect);
        tr.BottomRight() += CPoint(4, 4);
        gfxFlushRect(tr);
    }
}



// 4e3d90 (client: 4e9bc0)
void VisScrollBar::WriteData(void* buf) {
    VisScrollBar::Data* dat = (VisScrollBar::Data*)buf;
    dat->v = this->val;
    dat->vmax = this->val_max;
}


// 4e3dc0 (client: 4e9bf0)
void VisScrollBar::ReadData(const void* buf) {
    const VisScrollBar::Data* dat = (const VisScrollBar::Data*)buf;
    this->val = dat->v;
    this->val_max = dat->vmax;
}

// 4df055 (client: 4e39d6)
int32_t VisScrollBar::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_SCROLLBAR_RESIZE) {
        this->UpdateRects();
    }

    return CVisualObject::MsgProc(msg, wparam, lparam);
}

// 4deb32 (client: 4e34b3)
int32_t VisScrollBar::OnMouseMove(uint32_t wparam, CPoint pos) {
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


// 4deebd (client: 4e383e)
int32_t VisScrollBar::OnWmUser(uint32_t wparam, CPoint pos) {
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


// 4de701 (client: 4e3082)
int32_t VisScrollBar::OnLButtonDown(uint32_t wparam, CPoint pos) {
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


// 4defc7 (client: 4e3948)
int32_t VisScrollBar::OnLButtonUp(uint32_t wparam, CPoint pos) {
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


// 4e3df0 (client: 4e9c20)
int32_t VisScrollBar::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    if (this->OnLButtonDown(wparam, pos) != 0 && this->OnLButtonUp(wparam, pos) != 0) {
        return 1;
    }
    return 0;
}


// 4df087 (client: 4e3a08)
int32_t VisScrollBar::OnKeyDown(uint32_t wparam) {
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

// 4dde6a (client: 4e27eb)
VisScrollBar::VisScrollBar(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* hint)
: CVisualObject(_id, l, t, r, b, hint) {
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


// 4de086 (client: 4e2a07)
void VisScrollBar::SetPos(int32_t pos, int32_t pos_max) {
    if (pos_max > -1) {
        this->val_max = pos_max;
    }

    if (pos > -1 && pos < this->val_max) {
        this->val = pos;
    }

    this->VMethod9();
}


// 4ddacf (client: 4e2450)
void VisScrollBar::UpdateRects() {
    this->dirty = 1;

    CRect rt = this->ClientRectToScreen(this->rect);
    if (rt.Width() < rt.Height()) {
        this->btn_minus = CRect(rt.left, rt.top, rt.right, rt.top + rt.Width() - 4);
        this->btn_plus = CRect(rt.left, rt.bottom - rt.Width() + 4, rt.right, rt.bottom);
    } else {
        this->btn_minus = CRect(rt.left, rt.top, rt.left + rt.Height() - 4, rt.bottom);
        this->btn_plus = CRect(rt.right - rt.Height() + 4, rt.top, rt.right, rt.bottom);

        this->flags |= FLAG_CAN_FOCUS;
    }
    this->UpdateHBoxes();
}


// 4ddc41 (client: 4e25c2)
void VisScrollBar::UpdateHBoxes() {

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


// 4dddbe (client: 4e273f)
int32_t VisScrollBar::GetValHPos(int32_t x) {
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






// 44f430 (client: 452f50)
VisListBox::~VisListBox() = default;

// 4dc11d (client: 4e0aa0)
void VisListBox::VMethod7() {
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

// 4dc0bc (client: 4e0a3f)
void VisListBox::WriteData(void* buf) {
    ((CStringArray*)buf)->Copy(this->entries);
}

// 4507e0 (client: 452ef0)
uint32_t VisListBox::DataSize() {
    return 4; //???
}

// 4dc0ea (client: 4e0a6d)
void VisListBox::ReadData(const void* buf) {
    this->entries.Copy(**(const CStringArray**)buf);
}

// 4dc62f (client: 4e0fb2)
int32_t VisListBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
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

// 4dc8a4 (client: 4e1227)
int32_t VisListBox::OnMouseMove(uint32_t wparam, CPoint pos) {
    if (this->TestFlags(FLAG_FOCUS) == 0) {
        this->parent->FocusTo(this, true);
    }

    if ((wparam & 1) != 0) {
        this->OnLButtonDown(wparam, pos);
    }

    return 0;
}


// 4dc73f (client: 4e10c2)
int32_t VisListBox::OnLButtonDown(uint32_t wparam, CPoint pos) {

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

// 4dc832 (client: 4e11b5)
int32_t VisListBox::OnLButtonUp(uint32_t wparam, CPoint pos) {
    this->parent->MsgProc(VIS_MSG_LIST_SELECT, this->id, this->selected_index);
    return 1;
}


// 4dc86b (client: 4e11ee)
int32_t VisListBox::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    this->parent->MsgProc(VIS_MSG_LIST_ACTIVATE, this->id, this->selected_index);
    return 1;
}


// 4dc8f6 (client: 4e1279)
int32_t VisListBox::OnKeyDown(uint32_t wparam) {

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


// 4dc528 (client: 4e0eab)
int32_t VisListBox::IsValidIndex(int32_t idx) {
    if (idx > -1 && idx < this->entries.GetSize()) {
        return 1;
    }
    return 0;
}

// 4dc560 (client: 4e0ee3)
void VisListBox::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    if (!this->IsValidIndex(idx)) {
        return;
    }

    CRect tmp;
    GetClipRect(&tmp);

    CRect rt = this->ClientRectToScreen(this->rect);

    CRect clip(pos.x, pos.y, rt.right - 6, pos.y + 2 + this->font->GetHeight());
    SetClipRect(clip);

    this->font->DrawTextWithShadow(pos.x, pos.y, this->entries[idx], CGameFont::ALIGN_LEFT_TOP, clr, 1);

    SetClipRect(tmp);
}

// 4dbe7f (client: 4e0802)
void VisListBox::SelectItem(int32_t idx) {
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

// 4507c0 (client: 452ed0)
int32_t VisListBox::GetItemCount() {
    return this->entries.GetSize();
}

// 4dc4ee (client: 4e0e71)
void VisListBox::VMethod30(CPoint pos, const CRect& r) {
    CRect rt;
    rt.top = pos.y - 1;
    rt.left = pos.x;
    rt.right = r.right - 4;
    rt.bottom = pos.y - 3 + this->entry_height_full;

    ShadowRect(rt, shadow_level::list_selected);
}


// 4dbd31 (client: 4e06b5)
VisListBox::VisListBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: CVisualObject(_id, r, hint) {

    this->entry_height = _font->GetHeight();
    this->entry_height_full = this->entry_height + 4;

    this->flags |= FLAG_CAN_FOCUS;

    this->selected_index = -1;
    this->vis_start_index = 0;
    this->font = _font;
    this->clr1 = _clr1;
    this->clr2 = _clr2;
    this->scrollbox_id = _scrollid;

    this->num_vis_entry = this->rect.Height() / this->entry_height_full;

    this->rect.bottom = this->rect.top + 2 + this->num_vis_entry * entry_height_full;
}


// 4dbc11 (client: 4e0596)
VisListBox::VisListBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: CVisualObject(_id, l, t, r, b, hint) {

    this->entry_height = _font->GetHeight();
    this->entry_height_full = this->entry_height + 4;

    this->flags |= FLAG_CAN_FOCUS;

    this->selected_index = -1;
    this->vis_start_index = 0;
    this->font = _font;
    this->clr1 = _clr1;
    this->clr2 = _clr2;
    this->scrollbox_id = _scrollid;

    this->num_vis_entry = this->rect.Height() / this->entry_height_full;

    this->rect.bottom = this->rect.top + 2 + this->num_vis_entry * entry_height_full;
}


// 4dbfbb (client: 4e093e)
void VisListBox::Down() {
    this->SelectItem(this->selected_index + 1);
}

// 4dbf89 (client: 4e090c)
void VisListBox::Up() {
    if (this->selected_index > 0) {
        this->SelectItem(this->selected_index - 1);
    }
}

// 4dc03b (client: 4e09be)
void VisListBox::PageDown() {
    if (this->selected_index == this->vis_start_index + this->num_vis_entry - 1) {
        this->SelectItem(this->vis_start_index + this->num_vis_entry + this->num_vis_entry - 1);
    } else {
        this->SelectItem(this->vis_start_index + this->num_vis_entry - 1);
    }
}

// 4dbfe1 (client: 4e0964)
void VisListBox::PageUp() {
    if (this->selected_index == this->vis_start_index) {
        this->SelectItem(this->vis_start_index - this->num_vis_entry);
    } else {
        this->SelectItem(this->vis_start_index);
    }
}


// 4dbe45 (client: 4e07c8) - Get index of y coordinate
int32_t VisListBox::YToIndex(int32_t y) {
    return (y - this->ClientPtToScreen(this->rect.TopLeft()).y - 1) / this->entry_height_full;
}

// 4507f0 (client: 452ce0)
void VisListBox::AddItem(const char* str) {
    this->entries.Add(str);
    if (this->selected_index < 0) {
        this->selected_index++;
    }
}

// 4507a0 (client: 452cc0)
void VisListBox::SetSelectedIndex(int32_t idx) {
    // Note: vanilla stores the raw index with no clamping.
    if (idx < 0) {
        idx = 0;
    } else if (idx >= this->entries.GetSize()) {
        idx = this->entries.GetSize() - 1;
    }

    this->selected_index = idx;
}

// 4508b0 (client: 453180)
CString& VisListBox::GetItem(int32_t idx) {
    // Vanilla: i = (idx < GetSize()) ? idx : GetSize(); no negative guard.
    if (idx < 0) {
        return this->entries[0];
    }

    if (idx < this->entries.GetSize()) {
        return this->entries[idx];
    }

    return this->entries[this->entries.GetSize() - 1];
}

// 450830 (client: 453130)
void VisListBox::RemoveItem(int32_t idx) {
    this->entries.RemoveAt(idx);
    this->selected_index -= (this->entries.GetSize() == 0); // WAT?
}




// 4503a0 (client: 454620)
VisTextBox::~VisTextBox() = default;

// 4d9a21 (client: 4de337)
void VisTextBox::VMethod7() {
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

        ShadowRect(rshd, shadow_level::text_select);
    }

    this->font->DrawTextWithShadow(rt.left + 4, rt.top + rt.Height() / 2, s, CGameFont::ALIGN_VCENTER, this->clr, 1);

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
        ShadowRect(rt, shadow_level::disabled);
    }
#endif
    UnlockSurface2();
}

// 450440 (client: 454510)
void VisTextBox::WriteData(void* buf) {
    strcpy((char*)buf, this->text);
}


// 450430 (client: 454500)
uint32_t VisTextBox::DataSize() {
    return 4;
}


// 450410 (client: 4544e0)
void VisTextBox::ReadData(const void* buf) {
    this->text = (const char*)buf;
}


// 4daa2b (client: 4df3b0)
int32_t VisTextBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_CURSOR_BLINK && (timeGetTime() - this->cursor_blink_ts) > 500) {
        this->cursor_blink_ts = timeGetTime();
        this->cursor_blink = 1 - this->cursor_blink;

        if (this->TestFlags(FLAG_ENABLED)) {
            this->VMethod9();
        }
    }

    return CVisualObject::MsgProc(msg, wparam, lparam);
}


// 4da22e (client: 4debaf)
int32_t VisTextBox::OnMouseMove(uint32_t wparam, CPoint pos) {
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


// 4da37c (client: 4decfd)
int32_t VisTextBox::OnLButtonDown(uint32_t wparam, CPoint pos) {
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

// 4da3ed (client: 4ded6e)
int32_t VisTextBox::OnKeyDown(uint32_t wparam) {
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


// 4da96a (client: 4df2eb)
int32_t VisTextBox::OnChar(uint32_t wparam) {

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


// 4d9891 (client: 4de195)
VisTextBox::VisTextBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: CVisualObject(_id, l, t, r, b, hint) {
    this->font = _font;
    this->clr = _clr;
    this->text = "";
    this->select_start = 0;
    this->select_end = 0;
    this->cursor_pos = 0;

    this->flags |= FLAG_CAN_FOCUS;

    this->cursor_blink = 1;
#ifdef A2CLIENT
    this->is_password = 0;
#endif
}


// 4d995f (client: 4de26c)
VisTextBox::VisTextBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr, const char* hint)
: CVisualObject(_id, r, hint) {
    this->font = _font;
    this->clr = _clr;
    this->text = "";
    this->select_start = 0;
    this->select_end = 0;
    this->cursor_pos = 0;

    this->flags |= FLAG_CAN_FOCUS;

    this->cursor_blink = 1;
#ifdef A2CLIENT
    this->is_password = 0;
#endif
}

// 4da067 (client: 4de9e8)
void VisTextBox::DelSelection() {
    this->text = this->text.Left(this->select_start) + this->text.Right(this->text.GetLength() - this->select_end);
    this->select_end = this->select_start;
    this->cursor_pos = this->select_start;
}

// 4d9ef2 (client: 4de875)
void VisTextBox::InsertChar(int32_t chr) {
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

// 4daab3 (client: 4df438)
void VisTextBox::ResetBlink() {
    this->cursor_blink_ts = timeGetTime();
    this->cursor_blink = 1;
}

// 4da153 (client: 4dead4)
int32_t VisTextBox::GetCursorPosByX(int32_t x) {
    CRect rt = this->ClientRectToScreen(this->rect);
#if defined(A2CLIENT_PATCH) && defined(A2CLIENT)
    // Tangar GUI_textField_MeasurePosition (4DEAD4 entry takeover): more
    // accurate cursor placement, password-field asterisk measurement, and
    // correct positions for fields not starting at the screen left edge.
    if (this->is_password != 0) {
        char* ctx = this->text.GetBuffer(0);
        size_t len = strlen(ctx);
        for (size_t i = 0; i < len; i++) {
            ctx[i] = '*';
        }
        this->text.ReleaseBuffer();
    }

    const char* ctx = (const char*)this->text;
    int32_t inner_pos = x - rt.left;
    int32_t pos = this->text.GetLength();
    for (int32_t i = 0; ctx[i] != 0; i++) {
        int32_t wd = this->font->GetStrWidth(this->text.Left(i + 1));
        if (wd >= inner_pos) {
            char thissym[2] = { ctx[i], 0 };
            int32_t wdx = (this->font->GetStrWidth(thissym) - 4) / 2;
            if (wd - wdx >= inner_pos) {
                pos = i;
            } else {
                pos = i + 1;
            }
            break;
        }
    }
    return pos;
#else
    for (int32_t i = 0; i < this->text.GetLength(); i++) {
        CString str = this->text.Left(i + 1);
        if (rt.left + 4 + this->font->GetStrWidth(str) >= x) {
            return i;
        }
    }
    return this->text.GetLength();
#endif
}


// 4505b0 (client: 452c70)
VisRadioBase::~VisRadioBase() = default;

// 4daccd (client: 4df652)
void VisRadioBase::SetCursorOver(bool isOver) {
    CVisualObject::SetCursorOver(isOver);
    this->mouse_over = isOver;
}

// 4504f0 (client: 452bb0)
void VisRadioBase::WriteData(void* buf) {
    *(int32_t*)buf = this->selection;
}

// 4504e0 (client: 452ba0)
uint32_t VisRadioBase::DataSize() {
    return 4;
}

// 4506c0 (client: 453740)
void VisRadioBase::ReadData(const void* buf) {
    this->selection = *(const int32_t*)buf;
    this->selected = this->selection;
}

// 4dacef (client: 4df674)
int32_t VisRadioBase::OnMouseMove(uint32_t wparam, CPoint pos) {
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

// 4dac81 (client: 4df606)
int32_t VisRadioBase::GetIndex(int32_t y) {
    CPoint t = this->ClientPtToScreen(this->rect.TopLeft());
    return (y - t.y) / gfx_radiob->GetHeight(0);
}

// 4dab2b (client: 4df4b0)
VisRadioBase::VisRadioBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: CVisualObject(_id, l, t, r, b, hint) {
    this->font = _font;
    this->clr = _clr;
    this->selection = 0;
    this->flags |= FLAG_CAN_FOCUS;
    this->selected = 0;
}

// 450470 (client: 452b30)
void VisRadioBase::AddEntry(const char* etext) {
    this->entries.Add(etext);
}



// 450590 (client: 452c50)
VisRadioType1::~VisRadioType1() = default;

// 4dadf8 (client: 4df77d)
void VisRadioType1::VMethod7() {
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

        this->font->DrawTextWithShadow(x, y + 5, this->entries[i], CGameFont::ALIGN_LEFT_TOP, this->clr, 1);

        y += gfx_radiob->GetHeight(0);
    }

    if (this->TestFlags(FLAG_ENABLED) == 0) {
        rt = this->ClientRectToScreen(this->rect);
        rt.InflateRect(CRect(1, 1, 1, 1));
        ShadowRect(rt, shadow_level::disabled);
    }

    UnlockSurface2();
}

// 450510 (client: 452bd0)
void VisRadioType1::ReadData(const void* buf) {
    this->selection = *(const int32_t*)buf;
}

// 4db134 (client: 4dfab9)
int32_t VisRadioType1::OnMouseMove(uint32_t wparam, CPoint pos) {
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

// 4db062 (client: 4df9e7)
int32_t VisRadioType1::OnLButtonDown(uint32_t wparam, CPoint pos) {
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


// 450530 (client: 452bf0)
int32_t VisRadioType1::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    return this->OnLButtonDown(wparam, pos);
}

// 4db2d3 (client: 4dfc58)
int32_t VisRadioType1::OnKeyDown(uint32_t wparam) {
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


// 4db20d (client: 4dfb92)
int32_t VisRadioType1::OnChar(uint32_t wparam) {
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

// 450490 (client: 452b50)
VisRadioType1::VisRadioType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
 : VisRadioBase(_id, l, t, r, b, _font, _clr, hint) {
}


// 450720 (client: 4537a0)
VisRadioType2::~VisRadioType2() = default;


// 4db680 (client: 4e0005)
void VisRadioType2::VMethod7() {
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

        this->font->DrawTextWithShadow(x, y + 5, this->entries[i], CGameFont::ALIGN_LEFT_TOP, this->clr, 1);

        y += gfx_radiob->GetHeight(0);
    }

    if (this->TestFlags(FLAG_ENABLED) == 0) {
        rt = this->ClientRectToScreen(this->rect);
        rt.InflateRect(CRect(1, 1, 1, 1));
        ShadowRect(rt, shadow_level::disabled);
    }

    UnlockSurface2();
}

// 4dbb7b (client: 4e0500)
int32_t VisRadioType2::OnMouseMove(uint32_t wparam, CPoint pos) {
    if (!this->parent || this->TestFlags(FLAG_ENABLED) == 0) {
        return 0;
    }
    return VisRadioBase::OnMouseMove(wparam, pos);
}

// 4db8e7 (client: 4e026c)
int32_t VisRadioType2::OnLButtonDown(uint32_t wparam, CPoint pos) {
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

// 4dba0e (client: 4e0393)
int32_t VisRadioType2::OnKeyDown(uint32_t wparam) {
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

// 4db993 (client: 4e0318)
int32_t VisRadioType2::OnChar(uint32_t wparam) {
    if (wparam != ' ' || this->TestFlags(FLAG_FOCUS) == 0) {
        return CVisualObject::OnChar(wparam);
    }

    this->selection = this->selected;

    this->VMethod9();
    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->selection);
    return 1;
}

// 450670 (client: 4536f0)
VisRadioType2::VisRadioType2(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: VisRadioBase(_id, l, t, r, b, _font, _clr, hint) {
}



// 4df2c3 (client: 4e3c44)
VisBitmap::~VisBitmap() {
    if (this->bitmap) {
        delete this->bitmap;
    }
}

// 4df345 (client: 4e3cc6)
void VisBitmap::VMethod7() {
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

// 4df23d (client: 4e3bbe)
VisBitmap::VisBitmap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, int32_t dtype)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->bitmap = _bitmap;
    this->draw_type = dtype;
}

// 4df286 (client: 4e3c07)
VisBitmap::VisBitmap(int32_t _id, const RECT& r, CGameBitmap* _bitmap, int32_t dtype)
: CVisualObject(_id, r, nullptr) {
    this->bitmap = _bitmap;
    this->draw_type = dtype;
}

// 4e3fb0 (client: 4e9c80)
CGameBitmap* VisBitmap::GetBitmap() {
    return this->bitmap;
}

// 4e3fd0 (client: 4e9ca0)
void VisBitmap::SetBitmap(CGameBitmap* bmp) {
    this->bitmap = bmp;
}





// 4e4170 (client: 4e9510)
void VisComboBoxText::VMethod7() {
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

    this->font->DrawTextWithShadow(rt.left + 4, rt.top + rt.Height() / 2, this->text, CGameFont::ALIGN_VCENTER, clr, 1);

    UnlockSurface2();
}

// 4e4120 (client: 4e94c0)
VisComboBoxText::VisComboBoxText(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: VisTextBox(_id, l, t, r, b, _font, _clr, hint) {
}



// 4e4090 (client: 4e9430)
int32_t VisComboBoxList::OnKeyDown(uint32_t wparam) {
    if (wparam == VK_RETURN) {
        ((VisComboBox*)this->parent)->ProcSelectList();
        return 1;
    }
    return VisListBox::OnKeyDown(wparam);
}

// 4e4040 (client: 4e93e0)
VisComboBoxList::VisComboBoxList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: VisListBox(_id, l, t, r, b, _font, _clr1, _clr2, _scrollid, hint) {
}


// 4e1cb6 (client: 4e6637)
void VisComboBoxButton::SetCursorOver(bool isOver) {
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


// 4e1d7d (client: 4e66fe)
void VisComboBoxButton::VMethod7() {
    CRect rt = this->ClientRectToScreen(this->rect);
    this->parent->VMethod8(&rt);

    if (this->mouse_on) {
        this->bitmap->Draw(rt.left, rt.top, this->frm + 1, 0, 0);
    } else {
        this->bitmap->Draw(rt.left, rt.top, this->frm, 0, 0);
    }
}

// 4e1d5c (client: 4e66dd)
int32_t VisComboBoxButton::OnMouseMove(uint32_t wparam, CPoint pos) {
    return VisButton::OnMouseMove(wparam, pos);
}

// 4e1d3b (client: 4e66bc)
int32_t VisComboBoxButton::OnLButtonDown(uint32_t wparam, CPoint pos) {
    return VisButton::OnLButtonDown(wparam, pos);
}

// 4e1c16 (client: 4e6597)
int32_t VisComboBoxButton::OnLButtonUp(uint32_t wparam, CPoint pos) {
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

// 4e1b5c (client: 4e64dd)
VisComboBoxButton::VisComboBoxButton(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, int32_t _frm, int32_t _msgid, int32_t _charid, const char* hint)
: VisButton(_id, l, t, r, b, "", g_font1, clrsh_TechBlack, _msgid, _charid, hint) {
    this->bitmap = _bitmap;
    this->frm = _frm;
}



// 4e2481 (client: 4e6e05)
void VisComboBox::VMethod8(CRect* rect) {
    this->parent->VMethod8(rect);
}

// 4e201e (client: 4e69a2)
void VisComboBox::VMethod9() {
    CVisualObject::VMethod9();
}

// 4e2031 (client: 4e69b5)
void VisComboBox::WriteData(void* buf) {
    this->textbox->WriteData(buf);
}

// 4e22d8 (client: 4e6c5c)
int32_t VisComboBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
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

// 4e2410 (client: 4e6d94)
int32_t VisComboBox::OnLButtonDown(uint32_t wparam, CPoint pos) {
    return 1;
}

// 4e2422 (client: 4e6da6)
int32_t VisComboBox::OnKeyDown(uint32_t wparam) {
    if (wparam != VK_DOWN || this->TestFlags(FLAG_FOCUS) == 0) {
        return CVisualObject::OnKeyDown(wparam);

    //VK_DOWN
    }
    this->SetFocus(true);
    this->listbox->SetFocus(true);
    this->ToggleList();
    return 1;
}

// 4e1e10 (client: 4e6791)
VisComboBox::VisComboBox(int32_t _id, CRect r, const char* hint)
: CVisualObject(_id, r.left, r.top, r.right, r.top + 24, hint) {
    this->textbox = new VisComboBoxText(1, 0, 0, (r.right - r.left) - 24, 24, g_font1, clrsh_TechBlack, hint);
    this->textbox->ChangeFlags(FLAG_ENABLED, false);

    this->AddChild(this->textbox);

    this->listbox = new VisComboBoxList(2, 0, 24, (r.right - r.left), (r.bottom - r.top), g_font1, clrsh_TechBlack, clrsh_ShockingBlack, 10, hint);
    this->listbox->ChangeFlags(FLAG_HIDDEN, true);

    this->AddChild(this->listbox);

    VisComboBoxButton* btn = new VisComboBoxButton(3, (r.right - r.left) - 22, 2, (r.right - r.left) - 2, 22, gfx_scrollbars, 24, 0x456, 0, "");
    this->AddChild(btn);

    this->isEmpty = 1;
    this->list_showed = 0;
    this->flags |= FLAG_CAN_FOCUS;
}

// 4e2053 (client: 4e69d7)
void VisComboBox::AddItem(const char* str) {
    if (this->isEmpty) {
        this->textbox->ReadData(str);
        this->isEmpty = 0;
    }

    this->listbox->AddItem(str);
}

// 4e2150 (client: 4e6ad4)
void VisComboBox::ToggleList() {
    if (this->list_showed) {
        this->HideList();
        return;
    }

    this->list_showed = 1;

    if (this->TestFlags(FLAG_OVERCURSOR) == 0) {
        this->SetCursorOver(true);
    }

    this->listbox->ChangeFlags(FLAG_HIDDEN, false);
    this->listbox->SetFocus(true);

    this->rect.bottom = this->rect.top + 24 + this->listbox->GetRect().Height();

    CRect rt = this->ClientRectToScreen(this->rect);
    rt.top += 24;

    this->parent->VMethod8(&rt);
    this->VMethod9();
}

// 4e20db (client: 4e6a5f)
void VisComboBox::HideList() {
    this->SetCursorOver(false);

    this->list_showed = 0;

    this->listbox->ChangeFlags(FLAG_HIDDEN, true);
    this->listbox->SetFocus(false);

    this->rect.bottom = this->rect.top + 24;

    this->parent->VMethod9();
    this->VMethod9();
}

// 4e222a (client: 4e6bae)
void VisComboBox::ProcSelectList() {
    if (this->list_showed != 0) {
        this->SetCursorOver(false);
        this->list_showed = 0;
    }

    this->listbox->SetFocus(false);
    this->listbox->ChangeFlags(FLAG_HIDDEN, true);

    this->textbox->ReadData( this->listbox->GetItem(this->listbox->GetSelectedIndex()) );

    this->rect.bottom = this->rect.top + 24;

    this->parent->VMethod9();
    this->VMethod9();
}

// 4e2097 (client: 4e6a1b)
void VisComboBox::SelectItem(int32_t index) {
    this->textbox->ReadData(this->listbox->GetItem(index));
    this->listbox->SetSelectedIndex(index);
}




// 4d8994 (client: 4dd2a4)
void VisMultiText::VMethod7() {
    if (!this->parent) {
        return;
    }

    CRect rt = this->ClientRectToScreen(this->rect);
    this->parent->VMethod8(&rt);

    LockSurface2();
    this->font->DrawTextLinesShadow(rt, this->vis_start_index, this->vis_start_index + this->num_vis_entry, this->entries, this->clr1, this->entry_height_full);
    UnlockSurface2();
}

// 4d8b99 (client: 4dd4a9)
int32_t VisMultiText::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    return VisListBox::MsgProc(msg, wparam, lparam);
}

// 4d8bba (client: 4dd4ca)
int32_t VisMultiText::OnMouseMove(uint32_t wparam, CPoint pos) {
    return CVisualObject::OnMouseMove(wparam, pos);
}

// 4d8bdb (client: 4dd4eb)
int32_t VisMultiText::OnWmUser(uint32_t wparam, CPoint pos) {
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

// 4d8c3f (client: 4dd54f)
int32_t VisMultiText::OnLButtonDown(uint32_t wparam, CPoint pos) {
    return CVisualObject::OnLButtonDown(wparam, pos);
}

// 4d8c60 (client: 4dd570)
int32_t VisMultiText::OnKeyDown(uint32_t wparam) {
    return VisListBox::OnKeyDown(wparam);
}

// 4d889d (client: 4dd1ad)
void VisMultiText::SelectItem(int32_t idx) {
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


// 4d86fb (client: 4dd00b)
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

// 4d8629 (client: 4dcf39)
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

// 4d8816 (client: 4dd126)
void VisMultiText::SetText(const char* _text) {
    // Note: vanilla does not store _text into the text member here (only the ctor does);
    // entries are rebuilt from _text but text keeps its ctor value.
    this->entries.Copy(this->font->StringArrayForRect(this->rect, _text));

    if (this->rect.Height() / this->entry_height_full < this->entries.GetSize()) {
        this->num_vis_entry = this->rect.Height() / this->entry_height_full;
    } else {
        this->num_vis_entry = this->entries.GetSize();
    }
}

// 4d8a3b (client: 4dd34b)
void VisMultiText::SizesCheck() {
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


// 4df4d1 (client: 4e3e52)
VisScreen::VisScreen() = default;

// 4df4fa (client: 4e3e7b)
VisScreen::VisScreen(int32_t _id, const RECT& r, CGameBitmap* _bitmap)
: CVisualObject(_id, r, nullptr) {
    this->bitmap = _bitmap;
    VisScreen::VMethod26();
}

// 4df56d (client: 4e3eee)
VisScreen::VisScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->bitmap = _bitmap;
    VisScreen::VMethod26();
}

// 4df5ec (client: 4e3f6d)
VisScreen::~VisScreen() {
    VisScreen::VMethod27();
}

// 4df63d (client: 4e3fbe)
void VisScreen::VMethod7() {
    CRect r = this->ClientRectToScreen(this->rect);

    this->VMethod8(&r);

    CVisualObject::VMethod7();
}

// 4df67c (client: 4e3ffd)
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


// 4dfbcd (client: 4e454e)
int32_t VisScreen::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
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

// 4dfee7 (client: 4e4868)
int32_t VisScreen::OnLButtonDown(uint32_t wparam, CPoint pos) {
    if (this->cursor_over_obj_last == this->cursor_over_obj) {
        this->cursor_over_obj_last = nullptr;
        CVisualObject* obj = this->GetChildAt(pos);
        if (obj && obj != this) {
            return obj->OnLButtonDown(wparam, pos);
        }
    }

    return CVisualObject::OnLButtonDown(wparam, pos);
}

// 4dfc43 (client: 4e45c4)
int32_t VisScreen::OnKeyDown(uint32_t wparam) {
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


// 450900 (client: 453380)
void VisScreen::VMethod26() {
}

// 4388c0 (client: 43a9e0)
void VisScreen::VMethod27() {
}

// 4dfb4f (client: 4e44d0)
void VisScreen::VMethod28() {
    this->is_active = 1;
    this->SetCursorOver(true);
    this->SetFocus(true);
    this->TabFocus(true, false);
}

// 4dfb8a (client: 4e450b) - 29 method
void VisScreen::DoClose(uint32_t code) {
    //4dfb8a 29 method
    if (this->is_active) {
        this->is_active = 0;
        this->SetCursorOver(false);
        this->SetFocus(false);
        this->exit_code = code;
    }
}

// 4388d0 (client: 43ab60)
int32_t VisScreen::GetCloseCode() {
    return this->exit_code;
}

// 4327b5 (client: 4348e6)
void VisScreen::CloseOk() {
    this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
}

// 4327d4 (client: 434905)
void VisScreen::CloseCancel() {
    this->MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
}


// 451a70 (client: 452a70)
VisWindow::~VisWindow()
{}

// 4dfeb2 (client: 449fb8)
int32_t VisWindow::OnKeyDown(uint32_t wparam) {
    if (wparam == VK_ESCAPE) {
        return this->MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
    } else {
        return VisScreen::OnKeyDown(wparam);
    }
}

// 4dfdab (client: 4e472c)
VisWindow::VisWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap)
:VisScreen(_id, l, t, r, b, _bitmap) {
    VisWindow::UpdateWinRect();
}

// 4dfe19 (client: 4e479a)
void VisWindow::UpdateWinRect() {
    int32_t w = ((this->rect.Width() - 8) / 96) * 96 + 8;
    int32_t h = ((this->rect.Height() - 104) / 64) * 64 + 104;

    this->rect.left = (g_ScreenSize.right - w) / 2;
    this->rect.top = (g_ScreenSize.bottom - h) / 2;
    this->rect.right = this->rect.left + w;
    this->rect.bottom = this->rect.top + h;
}


// 4e0071 (client: 4e49f2)
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


// 4e0027 (client: 4e49a8)
int32_t VisRoleKeyScreen::OnKeyDown(uint32_t wparam) {
    if (wparam == VK_RETURN || wparam == VK_ESCAPE) {
        this->MsgProc(VIS_MSG_SPEECH_ADVANCE, 0, 0);
        return 1;
    }
    return this->VisScreen::OnKeyDown(wparam);
}


// 4e012d (client: 4e4aae)
void VisRoleKeyScreen::VMethod28() {
    this->AdvanceSpeechLine();
    this->VisScreen::VMethod28();
}


// 4e0148 (client: 4e4ac9) - (helper 4e0b20 inlined)
void VisRoleKeyScreen::DoClose(uint32_t code) {
    if (this->text_buf != nullptr) {
        free(this->text_buf);
    }
    this->VisScreen::DoClose(code);
}


// 4e0169 (client: 4e4aea) - advance to next talk line
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


// 4e0b43 (client: 4e54c4)
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


// 4e0978 (client: 4e52f9)
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


// 4e3f60 (client: 4e9340)
VisRoleKeyScreen::~VisRoleKeyScreen()
{}

// 4dff5c (client: 4e48dd)
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


// 444ff3 (client: 4476c1)
VisMessageBox::~VisMessageBox() {
}

// 44500f (client: 4476dd)
int32_t VisMessageBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg >= 0x445 && msg <= 0x44b) {
        int32_t res = VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, wparam, lparam);
        this->exit_code = msg;
        this->VMethod31(msg);
        return res;
    } else {
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
}


// 444086 (client: 446656)
void VisMessageBox::VMethod26() {
    CRect r2;
    if (this->text && strlen(this->text) > 0) {
        if (g_font1->GetStrWidth(this->text) > this->rect.Width()) {
            VisMultiText* txt = new VisMultiText(0, 40, 32, this->rect.Width() - 50, 104, this->text, g_font1, p_clrsh_Black, 0);

            this->AddChild(txt);

            txt->SizesCheck();

            r2 = CRect(40, 116, this->rect.Width() - 40, this->rect.Height() - 112);
        } else {
            VisLabel* lbl = new VisLabel(1, 40, 32, this->rect.Width() - 40, 56, this->text, g_font1, p_clrsh_Black, CGameFont::ALIGN_HCENTER);

            this->AddChild(lbl);

#ifdef A2CLIENT_PATCH
            // long_dip.txt (446875): single-line header payload bottom Height-112 -> Height-80
            r2 = CRect(40, 68, this->rect.Width() - 40, this->rect.Height() - 80);
#else
            r2 = CRect(40, 68, this->rect.Width() - 40, this->rect.Height() - 112);
#endif
        }
    } else {
        r2 = CRect(40, 56, this->rect.Width() - 40, this->rect.Height() - 88);
    }

    if (this->title && strlen(this->title) > 0) {
        r2.OffsetRect(0, 12);
        
        this->AddChild( new VisLabel(-1, r2.left, r2.top - 20, r2.right, r2.top - 4, this->title, g_font1, p_clrsh_Black, CGameFont::ALIGN_LEFT_TOP) );
    }

    CVisualObject* obj = this->VMethod30(this->payload, r2);
    this->rect.bottom = this->rect.top + 144 + obj->GetRect().bottom;
#ifdef A2CLIENT_PATCH
    // long_dip.txt (446a03..446a15): vtable check for VisDiplomacy shrinks bottom margin by 32
    if (dynamic_cast<VisDiplomacy*>(this) != nullptr) {
        this->rect.bottom -= 32;
    }
#endif
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
        btn->ChangeFlags(FLAG_DEFAULT_BUTTON, true);
    }
        break;

    case 1: {
        //ok cancel
        VisButton* btn = new VisButton(4, local_44, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "");
        this->AddChild(btn);
        btn->ChangeFlags(FLAG_DEFAULT_BUTTON, true);

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
        btn2->ChangeFlags(FLAG_DEFAULT_BUTTON, true);

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
        btn->ChangeFlags(FLAG_DEFAULT_BUTTON, true);

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
        btn->ChangeFlags(FLAG_DEFAULT_BUTTON, true);

        VisButton* btn2 = new VisButton(5, local_54, txt_main.GetLine(76), g_font1, nullptr, 0x448, 0, "");
        this->AddChild(btn2);

        btn->SetRightObj(btn2);
    }
        break;

    case 5: {
        //repeat cancel
        VisButton* btn = new VisButton(4, local_44, txt_dialogs.GetLine(3), g_font1, nullptr, 0x44a, 0, "");
        this->AddChild(btn);
        btn->ChangeFlags(FLAG_DEFAULT_BUTTON, true);

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
        btn->ChangeFlags(FLAG_DEFAULT_BUTTON, true);
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


// 445084 (client: 447752)
void VisMessageBox::VMethod31(int32_t code) {
}

// 44402b (client: 4465fb)
VisMessageBox::VisMessageBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const void* _payload, const char* str2, int32_t btypes, const char* str3)
: VisWindow(_id, l, t, r, b, nullptr) {
    this->title = str2;
    this->text = str3;
    this->payload = _payload;
    this->button_types = btypes;
}


// 4450d4 (client: 4477a2)
CVisualObject* VisMessageBoxWithList::VMethod30(const void* str, const RECT& r) {
    VisMultiText* txt = new VisMultiText(2, r, (const char*)str, g_font1, p_clrsh_Black, 0);
    this->AddChild(txt);
    txt->SizesCheck();
    return txt;
}

// 445091 (client: 44775f)
VisMessageBoxWithList::VisMessageBoxWithList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str1, const char* str2, int32_t btypes)
: VisMessageBox(_id, l, t, r, b, str1, str2, btypes, nullptr) {
}








// 44aad6 (client: 44d203)
UINT AFX_CDECL VisNetMapThreadProc(LPVOID pParam) {
    static_cast<VisNetMapSelection*>(pParam)->ScanMapsThread();
    return 0;
}




// 450130 (client: 454350) - (complete dtor 450130) - (deleting dtor ??_G; the complete dtor 450130 only destroys `colors` and the base)
VisNetChatList::~VisNetChatList() = default;




// 450230 (client: 454450) - (complete dtor 450230) - (deleting dtor ??_G; the complete dtor 450230 only destroys `colors` and the base)
VisNetPlayerList::~VisNetPlayerList() = default;




// 450060 (client: 454280) - (complete dtor 450060) - (deleting dtor ??_G; the complete dtor 450060 only calls the base dtor)
VisNetMapList::~VisNetMapList() = default;




// 4509a0 (client: 454760) - (complete dtor 4509a0) - (deleting dtor ??_G; the complete dtor 4509a0 only calls the base dtor)
VisServerScreenRadio::~VisServerScreenRadio() = default;




// 450380 (client: 454600) - (complete dtor 450380) - (deleting dtor ??_G; the complete dtor 450380 only calls the base dtor)
VisNetChatTextBox::~VisNetChatTextBox() = default;








// 451a90 (client: 453670)
VisMenuWnd::~VisMenuWnd() {
}

// 451990 (client: 453430)
VisMenuWnd::VisMenuWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, uint32_t unk, const CRect& _r)
: VisWindow(_id, l, t, r, b, _bitmap) {
    this->element_rect = _r;
    this->gap_0x68 = unk;
    this->element_rect.left = 40;
    this->element_rect.right = this->rect.Width() - 48;
}

// 440e54 (client: 4432bb)
int32_t VisMenuWnd::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
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

// 440ece (client: 443335)
int32_t VisMenuWnd::OnKeyDown(uint32_t wparam) {
    if (wparam == VK_UP) {
        this->TabFocus(false, true);
        return 1;
    } else if (wparam == VK_DOWN) {
        this->TabFocus(true, true);
        return 1;
    }

    return VisWindow::OnKeyDown(wparam);
}


// 440e11 (client: 443278)
void VisMenuWnd::AddElement(CVisualObject* obj, int32_t height) {
    this->element_rect.top = this->element_rect.bottom;
    this->element_rect.bottom = this->element_rect.top + height;

    obj->SetRect(this->element_rect);
    this->AddChild(obj);
}


// 4dd933 (client: 4e22b6)
int32_t MenuButton::OnLButtonUp(uint32_t wparam, CPoint pos) {
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

// 4dd9b3 (client: 4e2336)
int32_t MenuButton::OnChar(uint32_t wparam) {
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

// 4509c0 (client: 453390)
MenuButton::MenuButton(int32_t _id, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: VisButton(_id, 0, 0, 0, 0, _caption, _font, _clr, _msgid, _charid, hint) {
}

// 450bf0 (client: 4548f0)
MenuButton::MenuButton(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: VisButton(_id, r, _caption, _font, _clr, _msgid, _charid, hint) {
}





// 43ec67 (client: 4410cb)
const char* HintedListBox::GetHint() {
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

// 43ec05 (client: 441069)
HintedListBox::HintedListBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: VisListBox(_id, l, t, r, b, _font, _clr1, _clr2, _scrollid, hint) {
}






























































// VisMainMenu











// 44e4ce (client: 4517f9)
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




// 450ae0 (client: 4547e0) - (deleting dtor ??_G; the complete dtor at 450AE0 only calls the base dtor)
VisServerScreen::~VisServerScreen() = default;




// 44fb10 (client: 453a10) - (deleting dtor ??_G; the complete dtor at 44FB10 only calls the base dtor)
VisNetPhoneBook::~VisNetPhoneBook() = default;




// 44fcf0 (client: 453bf0) - (deleting dtor ??_G; the complete dtor 44fcf0 only calls the base dtor)
VisNetSerialSettings::~VisNetSerialSettings() = default;



// 44b37a (client: 44daa7)
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










#ifdef A2CLIENT

#endif
