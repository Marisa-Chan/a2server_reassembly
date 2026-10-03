#include "visual.h"
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
#include "unit_list.h"
#include "buildings_list.h"
#include "packet.h"


extern "C" char byte_666590[223]; //4c9a6f GetHint static buffer
extern "C" char unk_659A48[4]; //659a48 zeroed buffer used to clear the server screen text box
extern "C" char byte_659A34[4]; //659a34 "" hint string of the hat browser OK button
extern "C" char byte_659A38[4]; //659a38 "" hint string of the hat browser cancel button
extern "C" char byte_659A3C[4]; //659a3c "" hint string of the hat browser refresh button

// Spellbook pressed-position spell table. 62f8a8
uint32_t DAT_0062F8A8[24] = {1,0,0,1, 1,1,1,1, 1,1,1,1, 1,0,0,1, 1,0,0,1, 1,1,0,1};

// Spellbook selected-position spell table. 62fa28
uint32_t DAT_0062FA28[24] = {1,0,0,1, 1,0,0,1, 0,1,1,1, 1,0,0,1, 0,1,1,1, 1,0,0,1};

// Spellbook position castable-from-book flag. 62f968
uint32_t DAT_0062F968[24] = {1,1,1,0, 1,0,0,1, 0,0,1,1, 1,1,1,0, 1,0,0,0, 0,1,1,1};

// Spellbook position double-click-castable flag. 62f9c8
uint32_t DAT_0062F9C8[24] = {0,0,0,1, 1,1,1,0, 1,1,0,0, 0,0,0,1, 1,0,0,1, 1,0,0,0};


const int32_t VisStartGame::DWORD_0060bd60[4] = {0, 2, 3, 1};


CVisualObject::CVisualObject()
{
	//4d6d40
    parent = nullptr;
    cursor_over_obj = nullptr;
    focus_obj = nullptr;
    cursor_over_obj_last = nullptr;
    last_focus_obj = nullptr;
    flags = 1;
    id = -1;
    hint = "";
    rect = CRect(0, 0, 0, 0);
    down_obj = nullptr;
    up_obj = nullptr;
    left_obj = nullptr;
    right_obj = nullptr;
    caption_obj = nullptr;
}

CVisualObject::CVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str)
{
    //4d6f5f
    parent = nullptr;
    cursor_over_obj = nullptr;
    focus_obj = nullptr;
    cursor_over_obj_last = nullptr;
    last_focus_obj = nullptr;
    flags = 1;
    id = _id;

    if (str)
        hint = str;

    rect = CRect(l, t, r, b);
    down_obj = nullptr;
    up_obj = nullptr;
    left_obj = nullptr;
    right_obj = nullptr;
    caption_obj = nullptr;
}

CVisualObject::CVisualObject(int32_t _id, const RECT& r, const char* str)
{
    //4d6e50
    parent = nullptr;
    cursor_over_obj = nullptr;
    focus_obj = nullptr;
    cursor_over_obj_last = nullptr;
    last_focus_obj = nullptr;
    flags = 1;
    id = _id;

    if (str)
        hint = str;

    rect = r;

    down_obj = nullptr;
    up_obj = nullptr;
    left_obj = nullptr;
    right_obj = nullptr;
    caption_obj = nullptr;
}


CVisualObject::~CVisualObject()
{
    //4d707a
    for (int32_t i = 0; i < childs.GetSize(); i++)
    {
        CVisualObject* obj = childs[i];
        if (obj)
            delete obj;
    }
}


void CVisualObject::Dump(CDumpContext& dc) const
{
    //4d82b3
    
    //dc << "CVisualObject";
}


const char* CVisualObject::GetHint()
{
    //4d7c15

    if (TestFlags(FLAG_20))
        return nullptr;

    return hint;
}

void CVisualObject::SetHint(const char* _name)
{
    //4d7bf9

    hint = _name;
}


void CVisualObject::ChangeFlags(uint32_t _flags, bool setunset)
{
    //4d7a03

    if (setunset)
        flags |= _flags;
    else
        flags &= ~_flags;
}


uint32_t CVisualObject::TestFlags(uint32_t _flags)
{
    //4d79ed

    return flags & _flags;
}


void CVisualObject::SetCursorOver(bool isOver)
{
    //4d7a3a

    ChangeFlags(FLAG_OVERCURSOR, isOver);

    if (parent)
    {
        if (isOver)
        {
            parent->cursor_over_obj_last = parent->cursor_over_obj;
            parent->cursor_over_obj = this;
        }
        else if (parent->cursor_over_obj == this)
        {
            parent->cursor_over_obj = parent->cursor_over_obj_last;
        }
    }
}


void CVisualObject::SetFocus(bool isFocus)
{
    //4d7aa9

    ChangeFlags(FLAG_FOCUS, isFocus);

    if (parent)
    {
        if (isFocus)
        {
            parent->last_focus_obj = parent->focus_obj;
            parent->focus_obj = this;
            if (caption_obj)
                caption_obj->SetActiveColor(true);
        }
        else
        {
            if (parent->focus_obj == this)
                parent->focus_obj = parent->last_focus_obj;

            if (caption_obj)
                caption_obj->SetActiveColor(false);
        }
    }
}


void CVisualObject::VMethod7()
{
    //4d7c40

    if (TestFlags(FLAG_20))
        return;

    for (uint32_t i = 0; i < childs.GetSize(); i++)
    {
        CVisualObject* obj = childs[i];
        if (!obj->TestFlags(FLAG_20))
            obj->VMethod7();
    }
}


void CVisualObject::VMethod8(CRect* rect)
{
    //41ed60
}


void CVisualObject::VMethod9()
{
    //4d7cc8
    if (TestFlags(FLAG_20))
        return;

    if (g_IsServer != 0)
        return;

    VMethod7();
    VMethod10();
}


void CVisualObject::VMethod10()
{
    //4d7d03
    CRect r = ClientRectToScreen(rect);
    gfxFlushRect(r);
}


void CVisualObject::WriteData(void* buf)
{
    //4d7d99

    for (uint32_t i = 0; i < childs.GetSize(); i++)
    {
        CVisualObject* obj = childs[i];
        obj->WriteData(buf);
        buf = (uint8_t*)buf + obj->DataSize();
    }
}


uint32_t CVisualObject::DataSize()
{
    //4d7d37

    uint32_t sz = 0;
    for (uint32_t i = 0; i < childs.GetSize(); i++)
        sz += childs[i]->DataSize();
    return sz;
}


void CVisualObject::ReadData(const void* buf)
{
    //4d7e02

    for (uint32_t i = 0; i < childs.GetSize(); i++)
    {
        CVisualObject* obj = childs[i];
        obj->ReadData(buf);
        buf = (const uint8_t*)buf + obj->DataSize();
    }
}


int32_t CVisualObject::OnMouseMove(uint32_t wparam, CPoint pos)
{
    //41ed70
    return 0;
}

int32_t CVisualObject::OnWmUser(uint32_t wparam, CPoint pos)
{
    //41ed80
    return 0;
}

int32_t CVisualObject::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    //438850
    return 0;
}

int32_t CVisualObject::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    //438860
    return 0;
}

int32_t CVisualObject::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    //438870
    return 0;
}

int32_t CVisualObject::OnRButtonDown(uint32_t wparam, CPoint pos)
{
    //438880
    return 0;
}

int32_t CVisualObject::OnRButtonUp(uint32_t wparam, CPoint pos)
{
    //438890
    return 0;
}

int32_t CVisualObject::OnRButtonDblClk(uint32_t wparam, CPoint pos)
{
    //41ed90
    return 0;
}

int32_t CVisualObject::OnKeyDown(uint32_t wparam)
{
    //4388a0
    return 0;
}

int32_t CVisualObject::OnKeyUp(uint32_t wparam)
{
    //41eda0
    return 0;
}

int32_t CVisualObject::OnChar(uint32_t wparam)
{
    //4388b0
    return 0;
}


int32_t CVisualObject::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    //4d7fc9

    int32_t res = 0;
    if (msg >= WM_KEYDOWN && msg <= WM_CHAR)
    {
        if (focus_obj)
            res = focus_obj->MsgProc(msg, wparam, lparam);
        
        if (!res)
            res = MsgProcOnChilds(msg, wparam, lparam);
    }
    else if ((msg >= WM_MOUSEMOVE && msg <= WM_RBUTTONDBLCLK) || msg == WM_USER)
    {
        if (cursor_over_obj)
            res = cursor_over_obj->MsgProc(msg, wparam, lparam);
            
        if (!res)
            res = MsgProcOnChilds(msg, wparam, lparam);
    }
    else
        res = MsgProcOnChilds(msg, wparam, lparam);

    if (!res)
    {
        CPoint mouse(lparam & 0xffff, lparam >> 16);

        if (msg == WM_KEYDOWN)
            res = OnKeyDown(wparam);
        else if (msg == WM_KEYUP)
            res = OnKeyUp(wparam);
        else if (msg == WM_CHAR)
            res = OnChar(wparam);
        else if (msg == WM_MOUSEMOVE)
            res = OnMouseMove(wparam, mouse);
        else if (msg == WM_LBUTTONDOWN)
            res = OnLButtonDown(wparam, mouse);
        else if (msg == WM_LBUTTONUP)
            res = OnLButtonUp(wparam, mouse);
        else if (msg == WM_LBUTTONDBLCLK)
            res = OnLButtonDblClk(wparam, mouse);
        else if (msg == WM_RBUTTONDOWN)
            res = OnRButtonDown(wparam, mouse);
        else if (msg == WM_RBUTTONUP)
            res = OnRButtonUp(wparam, mouse);
        else if (msg == WM_RBUTTONDBLCLK)
            res = OnRButtonDblClk(wparam, mouse);
        else if (msg == WM_USER)
            res = OnWmUser(wparam, mouse);
    }

    return res;
}


int32_t CVisualObject::MsgProcOnChilds(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    //4d7e6b
    CPoint mouse(lparam & 0xffff, lparam >> 16);
    
    bool isMouseEvent = false;
    if ((msg >= WM_MOUSEMOVE && msg <= WM_RBUTTONDBLCLK) || msg == WM_USER)
        isMouseEvent = true;

    for (uint32_t i = 0; i < childs.GetSize(); i++)
    {
        CVisualObject* obj = childs[i];

        CRect r = obj->ClientRectToScreen(obj->rect);

        if (!isMouseEvent || r.PtInRect(mouse))
        {
            int32_t res = obj->MsgProc(msg, wparam, lparam);
            if (res)
                return res;

            if (isMouseEvent) // is mouse event, mouse in rect and res == 0 then break and return 0
                return 0;
        }
    }
    return 0;
}

void CVisualObject::ClientPtToScreen(CPoint* out, const CPoint& _point)
{
    //4d71f3
    *out = _point;
    for (CVisualObject* pobj = parent; pobj; pobj = pobj->parent)
        *out += pobj->rect.TopLeft();
}

void CVisualObject::ClientRectToScreen(CRect* out, const CRect& _rect)
{
    //4d7283
    ClientPtToScreen(&(out->TopLeft()), _rect.TopLeft());
    ClientPtToScreen(&(out->BottomRight()), _rect.BottomRight());
}

CPoint CVisualObject::ClientPtToScreen(const CPoint& _point)
{
    //4d71f3
    CPoint out = _point;
    for (CVisualObject* pobj = parent; pobj; pobj = pobj->parent)
        out += pobj->rect.TopLeft();
    return out;
}

CRect CVisualObject::ClientRectToScreen(const CRect& _rect)
{
    //4d7283
    CRect out;
    out.TopLeft() = ClientPtToScreen(_rect.TopLeft());
    out.BottomRight() = ClientPtToScreen(_rect.BottomRight());
    return out;
}


void CVisualObject::AddChild(CVisualObject* obj)
{
    //4d72c4
    childs.Add(obj);
    obj->parent = this;
}

void CVisualObject::RemoveChild(CVisualObject* obj)
{
    //4d72e9
    for (uint32_t i = 0; i < childs.GetSize(); i++)
    {
        if (childs[i] == obj)
        {
            childs.RemoveAt(i);

            obj->parent = nullptr;

            if (cursor_over_obj == obj)
                cursor_over_obj = nullptr;

            if (focus_obj == obj)
                focus_obj = last_focus_obj;
            
            if (last_focus_obj == obj)
            {
                last_focus_obj = nullptr;
                focus_obj = nullptr;
            }

            break;
        }
    }
}

void CVisualObject::RemoveChildById(int32_t _id)
{
    //4d7393
    for (uint32_t i = 0; i < childs.GetSize(); i++)
    {
        CVisualObject* obj = childs[i];
        if (obj->id == _id)
        {
            childs.RemoveAt(i);

            obj->parent = nullptr;

            if (cursor_over_obj == obj)
                cursor_over_obj = nullptr;

            /*if (focus_obj == obj)
                focus_obj = last_focus_obj;

            if (last_focus_obj == obj)
                last_focus_obj = nullptr;
            */
            break;
        }
    }
}

void CVisualObject::RemoveAllChilds()
{
    //4d78d4
    for (uint32_t i = 0; i < childs.GetSize(); i++)
        childs[i]->parent = nullptr;
    childs.RemoveAll();

    cursor_over_obj = nullptr;
    focus_obj = nullptr;
    cursor_over_obj_last = nullptr;
    last_focus_obj = nullptr;
}

void CVisualObject::DestroyChild(CVisualObject* obj)
{
    //4d7430
    for (uint32_t i = 0; i < childs.GetSize(); i++)
    {
        if (childs[i] == obj)
        {
            childs.RemoveAt(i);

            if (cursor_over_obj == obj)
                cursor_over_obj = nullptr;
            
            /*if (focus_obj == obj)
                focus_obj = last_focus_obj;

            if (last_focus_obj == obj)
                last_focus_obj = nullptr;*/

            delete obj;
            break;
        }
    }
}

void CVisualObject::DestroyChildById(int32_t _id)
{
    //4d74cf
    for (uint32_t i = 0; i < childs.GetSize(); i++)
    {
        CVisualObject* obj = childs[i];
        if (obj->id == _id)
        {
            childs.RemoveAt(i);

            if (cursor_over_obj == obj)
                cursor_over_obj = nullptr;

            /*if (focus_obj == obj)
                focus_obj = last_focus_obj;

            if (last_focus_obj == obj)
                last_focus_obj = nullptr;
            */

            delete obj;
            break;
        }
    }
}

void CVisualObject::DestroyAllChilds()
{
    //4d7950
    for (uint32_t i = 0; i < childs.GetSize(); i++)
        delete childs[i];
    childs.RemoveAll();

    cursor_over_obj = nullptr;
    focus_obj = nullptr;
    cursor_over_obj_last = nullptr;
    last_focus_obj = nullptr;
}

CVisualObject* CVisualObject::FindChild(int32_t _id)
{
    //4d7873
    for (uint32_t i = 0; i < childs.GetSize(); i++)
    {
        CVisualObject* obj = childs[i];
        if (obj->id == _id)
            return obj;
    }
    return nullptr;
}


CVisualObject* CVisualObject::GetChildAt(POINT pt)
{
    //4d7b48
    for (int32_t i = childs.GetSize() - 1; i >= 0; i--)
    {
        CVisualObject* ret = childs[i]->GetChildAt(pt);
        if (ret)
            return ret;
    }

    CRect r = ClientRectToScreen(rect);
    if (r.PtInRect(pt))
        return this;
    return nullptr;
}

void CVisualObject::SetLeftObj(CVisualObject* obj)
{
    //4d82cd
    left_obj = obj;
    obj->right_obj = this;
}

void CVisualObject::SetRightObj(CVisualObject* obj)
{
    //4d82ec
    right_obj = obj;
    obj->left_obj = this;
}

void CVisualObject::SetUpObj(CVisualObject* obj)
{
    //4d830b
    up_obj = obj;
    obj->down_obj = this;
}

void CVisualObject::SetDownObj(CVisualObject* obj)
{
    //4d832a
    down_obj = obj;
    obj->up_obj = this;
}

void CVisualObject::SetCaptionLabel(VisLabel* obj)
{
    //4d8349
    caption_obj = obj;
}

void CVisualObject::FocusTo(CVisualObject* obj, bool update)
{
    //4d7816
    if (focus_obj)
    {
        CVisualObject* oldo = focus_obj;
        oldo->SetFocus(false);
        if (update)
            oldo->VMethod9();
    }

    obj->SetFocus(true);
    if (update)
        obj->VMethod9();
}

void CVisualObject::TabFocus(bool forward, bool update)
{
    //4d7674
    int inc = 1;
    if (forward)
        inc = 1;
    else
        inc = -1;

    int32_t indx = -1;
    if (focus_obj)
    {
        for (int32_t i = 0; i < childs.GetSize(); i++)
        {
            if (childs[i] == focus_obj)
            {
                indx = i;
                break;
            }
        }
    }

    if (!childs.GetSize())
        return;

    int32_t next_indx = indx + inc;
    int32_t first_check_id = -1;
    bool idIsSet = false;

    while (true)
    {
        next_indx += inc;

        if (next_indx >= childs.GetSize())
            next_indx = 0;
        else if (next_indx < 0)
            next_indx = childs.GetSize() - 1;

        if (idIsSet)
        {
            if (next_indx == first_check_id)
                return; //full loop
        }
        else
        {
            first_check_id = next_indx;
            idIsSet = true;
        }

        CVisualObject* obj = childs[next_indx];
        if (obj->TestFlags(FLAG_ENABLED | FLAG_NOTFOCUS) == (FLAG_ENABLED | FLAG_NOTFOCUS))
        {
            if (focus_obj)
            {
                CVisualObject* old_focus = focus_obj;
                focus_obj->SetFocus(false);
                if (update)
                    old_focus->VMethod9();
            }
            obj->SetFocus(true);
            if (update)
                obj->VMethod9();
            break;
        }
    }
}

void CVisualObject::SetRect(RECT r)
{
    //4d7178
    rect = r;
}

void CVisualObject::SetRect(const RECT* r)
{
    //4d7140
    rect = *r;
}

void CVisualObject::SetRect(int32_t l, int32_t t, int32_t r, int32_t b)
{
    rect.left = l;
    rect.top = t;
    rect.right = r;
    rect.bottom = b;
}






VisLabel::VisLabel()
{
    //4d835f
}

VisLabel::VisLabel(int32_t _id, const RECT& r, const char* _text, CGameFont* _font, uint16_t* colorsh, uint32_t align)
    : CVisualObject(_id, r, nullptr)
{
    //4d8453

    text = _text;
    font = _font;
    color_sh = colorsh;
    align_flags = align;
}

VisLabel::VisLabel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _text, CGameFont* _font, uint16_t* colorsh, uint32_t align)
    : CVisualObject(_id, l, t, r, b, nullptr)
{
    //4d83b6

    text = _text;
    font = _font;
    color_sh = colorsh;
    align_flags = align;
}

VisLabel::~VisLabel()
{
    //4e3ba0
}


void VisLabel::VMethod7()
{
    //4d84e4
    if (parent)
    {
        CRect res = ClientRectToScreen(rect);

        LockSurface2();

        parent->VMethod8(&res);

        int32_t outx = 0;
        int32_t outy = 0;

        if (align_flags & 1)
            outx = res.right;
        else if (align_flags & 2)
            outx = (res.left + res.right) / 2;
        else
            outx = res.left;

        if (align_flags & 4)
            outy = res.bottom;
        else if (align_flags & 8)
            outy = (res.top + res.bottom) / 2;
        else
            outy = res.top;

        font->DrawTextWithShadow(outx, outy, text, align_flags, color_sh, 1);

        UnlockSurface2();
    }
}

void VisLabel::SetActiveColor(bool isActive)
{
    //4d85e6
    if (isActive)
        color_sh = clrsh_DullGold;
    else
        color_sh = clrsh_TechBlack;
}


VisButton::~VisButton()
{
    //450a40
}

void VisButton::VMethod7()
{
    //4d8f7e
    if (!parent)
        return;

    CRect r2 = ClientRectToScreen(rect);
    
    LockSurface2();
    parent->VMethod8(&r2);

    r2.right -= 1;
    r2.bottom -= 1;

    POINT pp;
    GetCursorPos(&pp);

    uint32_t clr1;
    uint32_t clr2;
    uint32_t dd;
    if (r2.PtInRect(pp) == 0 || downed == 0)
    {
        clr1 = GetColorRGB(0x29, 0x45, 0x3f);
        clr2 = GetColorRGB(7, 12, 9);
        dd = 2;
    }
    else
    {
        clr1 = GetColorRGB(7, 12, 9);
        clr2 = GetColorRGB(0x29, 0x45, 0x3f);
        dd = 4;
    }

    if (!clr)
    {
        if ((mouse_on == 0 && TestFlags(FLAG_FOCUS) == 0) || TestFlags(FLAG_ENABLED) == 0)
            font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, caption, 8 | 2, p_clrsh_Black, dd);
        else
            font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, caption, 8 | 2, p_clrsh_Gold, dd);
    }
    else if (mouse_on == 0)
        font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, caption, 8 | 2, clrsh_DullGold, dd);
    else
        font->DrawTextWithShadow(r2.left + 1 + r2.Width() / 2, r2.top + r2.Height() / 2, caption, 8 | 2, clrsh_CharlieBrown, dd);

    FillRectColor(r2.right, r2.top + 2, r2.right, r2.bottom - 2, clr2);
    FillRectColor(r2.right - 1, r2.top + 1, r2.right - 1, r2.bottom - 1, clr2);
    FillRectColor(r2.left + 2, r2.bottom, r2.right - 2, r2.bottom, clr2);
    FillRectColor(r2.left + 1, r2.bottom - 1, r2.right - 1, r2.bottom - 1, clr2);
    FillRectColor(r2.left + 2, r2.top, r2.right - 2, r2.top, clr1);
    FillRectColor(r2.left, r2.top + 2, r2.left, r2.bottom - 2, clr1);

    SetPixelColor(r2.left + 1, r2.top + 1, clr1);
    SetPixelColor(r2.right - 2, r2.bottom - 2, clr2);

    if (TestFlags(FLAG_ENABLED) == 0)
    {
        CRect t = ClientRectToScreen(rect);
        ShadowRect(t, 3);
    }
    UnlockSurface2();
}

int32_t VisButton::OnMouseMove(uint32_t wparam, CPoint pos)
{
    //4d958f
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    if (TestFlags(FLAG_FOCUS) == 0)
        parent->FocusTo(this, true);

    CRect t = ClientRectToScreen(rect);
    
    if (t.PtInRect(pos))
    {
        if (TestFlags(FLAG_OVERCURSOR) == 0)
            SetCursorOver(true);
        if (mouse_on == 0)
        {
            mouse_on = 1;
            VMethod9();
        }
    }
    else
    {
        if (TestFlags(FLAG_OVERCURSOR) != 0 && downed == 0)
            SetCursorOver(false);

        if (mouse_on == 1)
        {
            mouse_on = 0;
            VMethod9();
        }
    }
    return 0;
}

int32_t VisButton::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    //4d96a1
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    if (downed)
        return 0;
    
    SetDowned(true);
    return 1;
}

int32_t VisButton::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    //4d96e8
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    if (downed == 0)
        return 0;

    CRect t = ClientRectToScreen(rect);

    SetDowned(false);

    if (t.PtInRect(pos))
        AfxGetMainWnd()->PostMessage(msgid, 0, 0);
    return 1;
}

int32_t VisButton::OnKeyDown(uint32_t wparam)
{
    //4d9783
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    if (wparam != VK_RETURN)
        return 0;

    AfxGetMainWnd()->PostMessage(msgid, 0, 0);
    return 1;
}

int32_t VisButton::OnChar(uint32_t wparam)
{
    //4d97d4
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    if (EncodeChar(wparam) != charid)
        return 0;

    AfxGetMainWnd()->PostMessage(msgid, 0, 0);
    return 1;
}

VisButton::VisButton(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: CVisualObject(_id, r, hint)
{
    //4d8e2d
    caption = _caption;
    font = _font;
    msgid = _msgid;
    clr = _clr;
    charid = _charid;

    for (int i = 1; i < caption.GetLength(); i++)
    {
        char c0 = caption[i - 1];
        char c1 = caption[i];
        if (c1 == '~' && c0 == '~')
            i += 2;
        else if (c1 != '~' && c0 == '~')
        {
            charid = ToLowerChar(c1);
            break;
        }
    }

    mouse_on = 0;
    downed = 0;
    flags |= FLAG_NOTFOCUS;
}

VisButton::VisButton(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: CVisualObject(_id, l, t, r, b, hint)
{
    //4d8cd0
    caption = _caption;
    font = _font;
    msgid = _msgid;
    clr = _clr;
    charid = _charid;

    for (int i = 1; i < caption.GetLength(); i++)
    {
        char c0 = caption[i - 1];
        char c1 = caption[i];
        if (c1 == '~' && c0 == '~')
            i += 2;
        else if (c1 != '~' && c0 == '~')
        {
            charid = ToLowerChar(c1);
            break;
        }
    }

    mouse_on = 0;
    downed = 0;
    flags |= FLAG_NOTFOCUS;
}

void VisButton::SetDowned(bool down)
{
    //4d94b5
    if (!down)
    {
        downed = 0;
        if (TestFlags(FLAG_OVERCURSOR))
            SetCursorOver(false);

        VMethod9();
    }
    else
    {
        g_SfxArray[2]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xdc, 0);

        downed = 1;
        if (TestFlags(FLAG_OVERCURSOR) == 0)
            SetCursorOver(true);

        VMethod9();
    }
}



VisScrollBar::~VisScrollBar()
{
    //4e3e80
}

void VisScrollBar::SetCursorOver(bool isOver)
{
    //4de6df
    CVisualObject::SetCursorOver(isOver);
    is_mouse_over = isOver;
}

void VisScrollBar::VMethod7()
{
    if (!parent)
        return;

    CRect tr = ClientRectToScreen(rect);
    tr.BottomRight() += CPoint(4, 4);

    LockSurface2();
    parent->VMethod8(&tr);

    tr = ClientRectToScreen(rect);

    if (tr.Width() < tr.Height())
    {
        int32_t val_pos = 0;

        if (val_max < 2)
            val_pos = 0;
        else
            val_pos = (val * (tr.Height() - 3 * tr.Width() + 8)) / (val_max - 1);

        if (mouse_on_minus == 0)
        {
            gfx_scrollbars->VMethod3(tr.left + 4, tr.top + 4, 18, 4, 0);
            gfx_scrollbars->VMethod2(tr.left, tr.top, 18, 0, 0);
        }
        else
        {
            gfx_scrollbars->VMethod3(tr.left + 4, tr.top + 4, 21, 4, 0);
            gfx_scrollbars->VMethod2(tr.left, tr.top, 21, 0, 0);
        }

        for (int32_t i = 1; i < tr.Height() / tr.Width(); i++)
        {
            gfx_scrollbars->VMethod3(tr.left + 4, tr.top + 4 + i * tr.Width(), 19, 4, 0);
            gfx_scrollbars->VMethod2(tr.left, tr.top + i * tr.Width(), 19, 0, 0);
        }

        gfx_scrollbars->VMethod3(tr.left + 4, tr.top + tr.Width() + val_pos, 22, 0, 0);

        if (mouse_on_plus == 0)
        {
            gfx_scrollbars->VMethod3(tr.left + 4, tr.bottom - tr.Width() + 4, 20, 4, 0);
            gfx_scrollbars->VMethod2(tr.left, tr.bottom - tr.Width(), 20, 0, 0);
        }
        else
        {
            gfx_scrollbars->VMethod3(tr.left + 4, tr.bottom - tr.Width() + 4, 23, 4, 0);
            gfx_scrollbars->VMethod2(tr.left, tr.bottom - tr.Width(), 23, 0, 0);
        }

        gfx_scrollbars->VMethod2(tr.left, tr.top + tr.Width() - 4 + val_pos, 22, 0, 0);
    }
    else
    {
        UpdateHBoxes();

        if (mouse_on_minus == 0)
        {
            gfx_scrollbars->VMethod3(tr.left + 4, tr.top + 4, 0, 4, 0);
            gfx_scrollbars->VMethod2(tr.left, tr.top, 0, 0, 0);
        }
        else
        {
            gfx_scrollbars->VMethod3(tr.left + 4, tr.top + 4, 3, 4, 0);
            gfx_scrollbars->VMethod2(tr.left, tr.top, 3, 0, 0);
        }

        for (int32_t i = 1; i < tr.Width() / tr.Height(); i++)
        {
            gfx_scrollbars->VMethod3(tr.left + 4 + i * tr.Height(), tr.top + 4, 7, 4, 0);
            gfx_scrollbars->VMethod2(tr.left + i * tr.Height(), tr.top, 7, 0, 0);
        }

        gfx_scrollbars->VMethod3(horiz_box_pos.left + 5, tr.top + 4, 10, 4, 0);

        if (mouse_on_plus == 0)
        {
            gfx_scrollbars->VMethod3(tr.right - tr.Height() + 4, tr.top + 4, 8, 4, 0);
            gfx_scrollbars->VMethod2(tr.right - tr.Height(), tr.top, 8, 0, 0);
        }
        else
        {
            gfx_scrollbars->VMethod3(tr.right - tr.Height() + 4, tr.top + 4, 11, 4, 0);
            gfx_scrollbars->VMethod2(tr.right - tr.Height(), tr.top, 11, 0, 0);
        }

        gfx_scrollbars->VMethod2(horiz_box_pos.left + 1, tr.top, 10, 0, 0);
    }
    
    if (TestFlags(FLAG_ENABLED) == 0)
    {
        tr = ClientRectToScreen(rect);
        tr.InflateRect(CRect(1, 1, 1, 1));

        ShadowRect(tr, 3);
    }
    UnlockSurface2();
}


void VisScrollBar::VMethod9()
{
    //4de0d0
    if (g_IsServer == 0)
    {
        VMethod7();
        CRect tr = ClientRectToScreen(rect);
        tr.BottomRight() += CPoint(4, 4);
        gfxFlushRect(tr);
    }
}



void VisScrollBar::WriteData(void* buf)
{
    //4e3d90
    VisScrollBar::Data* dat = (VisScrollBar::Data*)buf;
    dat->v = val;
    dat->vmax = val_max;
}


void VisScrollBar::ReadData(const void* buf)
{
    //4e3dc0
    const VisScrollBar::Data* dat = (const VisScrollBar::Data*)buf;
    val = dat->v;
    val_max = dat->vmax;
}

int32_t VisScrollBar::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    //4df055
    if (msg == 0x46f)
        UpdateRects();

    return CVisualObject::MsgProc(msg, wparam, lparam);
}

int32_t VisScrollBar::OnMouseMove(uint32_t wparam, CPoint pos)
{
    //4deb32
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    CRect tr = ClientRectToScreen(rect);

    if (btn_minus.PtInRect(pos))
    {
        if (mouse_on_minus == 0)
        {
            mouse_on_minus = 1;
            VMethod9();
        }

        if (TestFlags(FLAG_OVERCURSOR) == 0)
            CVisualObject::SetCursorOver(true); //do not set   is_mouse_over
    }
    else
    {
        if (mouse_on_minus != 0)
        {
            CVisualObject::SetCursorOver(false); //do not set   is_mouse_over
            mouse_on_minus = 0;
            VMethod9();
        }
    }

    if (btn_plus.PtInRect(pos))
    {
        if (mouse_on_plus == 0)
        {
            mouse_on_plus = 1;
            VMethod9();
        }

        if (TestFlags(FLAG_OVERCURSOR) == 0)
            CVisualObject::SetCursorOver(true); //do not set   is_mouse_over
    } 
    else
    {
        if (mouse_on_plus != 0)
        {
            CVisualObject::SetCursorOver(false); //do not set   is_mouse_over
            mouse_on_plus = 0;
            VMethod9();
        }
    }

    if (TestFlags(FLAG_FOCUS) == 0)
    {
        if (tr.Height() < tr.Width())
            parent->FocusTo(parent, true);
    }

    if (TestFlags(FLAG_OVERCURSOR) == 0 || is_mouse_over == 0)
    {
        if ((wparam & 1) != 0)
            OnLButtonDown(wparam, pos);
        return 0;
    }

    if (tr.Width() < tr.Height())
    {
        int32_t val_pos = val_max - 1;
        if (val_max >= 2)
            val_pos = ((val_max - 1) * (pos.y - tr.top - 24)) / (tr.Height() + (tr.Width() - 4) * -3);

        if (val_pos < 0)
            val_pos = 0;

        if (val_pos > val_max - 1)
            val_pos = val_max - 1;

        parent->MsgProc(0x469, id, val_pos);

        if (pos.x < tr.left - tr.Width() ||
            pos.x > tr.right + tr.Width() ||
            pos.y < tr.top - tr.Width() ||
            pos.y > tr.bottom + tr.Width() )
            SetCursorOver(false);
    }
    else if (mousedown_on_hbox != 0)
    {
        val = GetValHPos(pos.x);
        VMethod9();
        parent->MsgProc(0x46e, id, val);
    }
    return 0;
}


int32_t VisScrollBar::OnWmUser(uint32_t wparam, CPoint pos)
{
    //4deebd
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    if (rect.Width() < rect.Height())
    {
        if (TestFlags(FLAG_OVERCURSOR) &&
            (btn_minus.PtInRect(pos) || btn_plus.PtInRect(pos)))
            return OnLButtonDown(wparam | 1, pos);

        if (wparam == 1)
            return OnMouseMove(wparam, pos);
    }
    return 1;
}


int32_t VisScrollBar::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    //4de701
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    CRect rt = ClientRectToScreen(rect);
    if (rt.Width() < rt.Height())
    {
        int32_t val_pos = val_max - 1;
        if (val_max >= 2)
            val_pos = (val * (rt.Height() + rt.Width() * -3 + 8)) / (val_max - 1);

        if (pos.y - rt.top < rt.Width())
            parent->MsgProc(0x46a, id, 0);
        else if (rt.bottom - pos.y < rt.Width() - 4)
            parent->MsgProc(0x46b, id, 0);
        else if (pos.y - rt.top - rt.Width() < val_pos)
        {
            if (down_on_part != 2)
            {
                parent->MsgProc(0x46c, id, 0);
                down_on_part = 1;
            }
        }
        else if ((pos.y - rt.top) - (rt.Width() * 2 - 4) < val_pos)
        {
            SetCursorOver(true);
        }
        else if (down_on_part != 1)
        {
            parent->MsgProc(0x46d, id, 0);
            down_on_part = 2;
        }
    }
    else
    {
        if (btn_minus.PtInRect(pos))
        {
            int32_t d = val_max / 16;
            if (d < 1)
                d = 1;

            if (val - d < 0)
                val = 0;
            else
                val -= d;
        }
        else if (btn_plus.PtInRect(pos))
        {
            int32_t d = val_max / 16;
            if (d < 1)
                d = 1;

            if (val + d > val_max)
                val = val_max;
            else
                val += d;
        }
        else
        {
            val = GetValHPos(pos.x);
            UpdateHBoxes();

            if (mousedown_on_hbox == 0)
            {
                mousedown_on_hbox = horiz_box_pos.PtInRect(pos);

                if (mousedown_on_hbox != 0 && TestFlags(FLAG_OVERCURSOR) == 0)
                    SetCursorOver(true);
            }            
        }

        VMethod9();
        parent->MsgProc(0x46e, id, val);
    }
    return 1;
}


int32_t VisScrollBar::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    //4defc7
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    down_on_part = 0;

    if (is_mouse_over)
    {
        parent->MsgProc(0x474, id, 0);
        SetCursorOver(false);
        mousedown_on_hbox = 0;
    }
    return 0;
}


int32_t VisScrollBar::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    //4e3df0
    if (OnLButtonDown(wparam, pos) != 0 && OnLButtonUp(wparam, pos) != 0)
        return 1;
    return 0;
}


int32_t VisScrollBar::OnKeyDown(uint32_t wparam)
{
    //4df087
    if (TestFlags(FLAG_FOCUS) == 0 || rect.Height() >= rect.Width())
        return CVisualObject::OnKeyDown(wparam);

    int32_t d = val_max / 16;
    if (d == 0)
        d = 1;

    if (wparam == VK_LEFT)
    {
        if (val - d < 0)
            val = 0;
        else
            val -= d;

        VMethod9();
        parent->MsgProc(0x46e, id, val);
        return 1;
    }
    
    if (wparam == VK_RIGHT)
    {
        if (val + d >= val_max)
            val = val_max;
        else
            val += d;

        VMethod9();
        parent->MsgProc(0x46e, id, val);
        return 1;
    }
    return 0;
}

VisScrollBar::VisScrollBar(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* hint)
: CVisualObject(_id, l, t, r, b, hint)
{
    //4dde6a
    field_0x5c = 0;
    val = 0;
    val_max = 0;
    mouse_on_minus = 0;
    mouse_on_plus = 0;
    is_mouse_over = 0;
    mousedown_on_hbox = 0;
    caption_obj = nullptr;

    AfxGetMainWnd()->PostMessage(0x46f, 0, 0);
}


void VisScrollBar::SetPos(int32_t pos, int32_t pos_max)
{
    //4de086
    if (pos_max > -1)
        val_max = pos_max;

    if (pos > -1 && pos < val_max)
        val = pos;

    VMethod9();
}


void VisScrollBar::UpdateRects()
{
    //4ddacf
    field_0x5c = 1;

    CRect rt = ClientRectToScreen(rect);
    if (rt.Width() < rt.Height())
    {
        btn_minus = CRect(rt.left, rt.top, rt.right, rt.top + rt.Width() - 4);
        btn_plus = CRect(rt.left, rt.bottom - rt.Width() + 4, rt.right, rt.bottom);
    }
    else
    {
        btn_minus = CRect(rt.left, rt.top, rt.left + rt.Height() - 4, rt.bottom);
        btn_plus = CRect(rt.right - rt.Height() + 4, rt.top, rt.right, rt.bottom);

        flags |= FLAG_NOTFOCUS;
    }
    UpdateHBoxes();
}


void VisScrollBar::UpdateHBoxes()
{
    //4ddc41

    CRect rt = ClientRectToScreen(rect);
    if (rt.Height() < rt.Width())
    {
        int32_t p = rt.left + rt.Height() - 4;
        if (val_max != 0)
            p += (rt.Width() - (rt.Height() * 2 - 8) - 12) * val / val_max;

        horiz_box_pos = CRect(p, rt.top, p + 16, rt.bottom);
        rect4 = CRect(btn_minus.right, btn_minus.top, horiz_box_pos.left, horiz_box_pos.bottom);
        rect5 = CRect(horiz_box_pos.right, horiz_box_pos.top, btn_plus.left, btn_plus.bottom);
    }
}


int32_t VisScrollBar::GetValHPos(int32_t x)
{
    //4dddbe
    CRect rt = ClientRectToScreen(rect);
    int32_t p = val_max * (x - (rt.left + 2 + rt.Height())) / (rt.Width() - (rt.Height() * 2 - 8) - 12);
    if (p < 0)
        p = 0;
    if (p > val_max)
        p = val_max;
    return p;
}






VisListBox::~VisListBox() = default; //44f430

void VisListBox::VMethod7()
{
    //4dc11d
    if (!parent)
        return;

    CRect rt = ClientRectToScreen(rect);

    LockSurface2();

    parent->VMethod8(&rt);

    int32_t y = rt.top + 2;
    for (int32_t i = vis_start_index; i < vis_start_index + num_vis_entry; i++)
    {
        if (i >= vis_start_index)
        {
            entry_height_full -= 2;

            FillRectColor(rt.left + 1, y - 2, rt.right - 5, y - 2, GetColorRGB(8, 8, 8));
            FillRectColor(rt.left + 1, y - 1, rt.left, y - 3 + entry_height_full, GetColorRGB(8, 8, 8));

            FillRectColor(rt.left + 1, y - 2 + entry_height_full, rt.right - 5, y - 2 + entry_height_full, GetColorRGB(0x5e, 0x73, 0x65));
            FillRectColor(rt.right - 4, y - 1, rt.right - 4, y - 3 + entry_height_full, GetColorRGB(0x5e, 0x73, 0x65));

            entry_height_full += 2;
        }

        if (selected_index == i)
        {
            VMethod30(CPoint(rt.left, y), rt);

            if (TestFlags(FLAG_FOCUS) == 0)
                DrawItem(i, CPoint(rt.left + 4, y - 2), p_clrsh_ShockingBlack);
            else
                DrawItem(i, CPoint(rt.left + 4, y - 2), p_clrsh_Gold);
        }
        else
            DrawItem(i, CPoint(rt.left + 4, y - 2), p_clrsh_Black);

        y += entry_height_full;
    }
    UnlockSurface2();
}

void VisListBox::WriteData(void* buf)
{
    //4dc0bc
    ((CStringArray*)buf)->Copy(entries);
}

uint32_t VisListBox::DataSize()
{
    //4507e0
    return 4; //???
}

void VisListBox::ReadData(const void* buf)
{
    //4dc0ea
    entries.Copy(**(const CStringArray**)buf);
}

int32_t VisListBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    //4dc62f
    int32_t res = CVisualObject::MsgProc(msg, wparam, lparam);
    if (res != 0)
        return res;

    switch (msg)
    {
    case 0x469:
        if (wparam == scrollbox_id)
        {
            SelectItem(lparam);
            return 1;
        }
        break;

    case 0x46a:
        if (wparam == scrollbox_id)
        {
            Up();
            return 1;
        }
        break;

    case 0x46b:
        if (wparam == scrollbox_id)
        {
            Down();
            return 1;
        }
        break;

    case 0x46c:
        if (wparam == scrollbox_id)
        {
            PageUp();
            return 1;
        }
        break;

    case 0x46d:
        if (wparam == scrollbox_id)
        {
            PageDown();
            return 1;
        }
    }

    return 0;
}

int32_t VisListBox::OnMouseMove(uint32_t wparam, CPoint pos)
{
    //4dc8a4
    if (TestFlags(FLAG_FOCUS) == 0)
        parent->FocusTo(this, true);

    if ((wparam & 1) != 0)
        OnLButtonDown(wparam, pos);

    return 0;
}


int32_t VisListBox::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    //4dc73f

    int32_t idx = YToIndex(pos.y);
    if (idx < num_vis_entry)
    {
        if (vis_start_index + idx < entries.GetSize() - 1)
            idx += vis_start_index;
        else
            idx = entries.GetSize() - 1;
    }

    selected_index = idx;

    VMethod9();

    VisScrollBar* scrollbar = (VisScrollBar*)parent->FindChild(scrollbox_id);
    if (scrollbar)
        scrollbar->SetPos(selected_index, entries.GetSize());

    parent->MsgProc(0x46e, id, selected_index);

    return 1;
}

int32_t VisListBox::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    //4dc832
    parent->MsgProc(0x473, id, selected_index);
    return 1;
}


int32_t VisListBox::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    //4dc86b
    parent->MsgProc(0x444, id, selected_index);
    return 1;
}


int32_t VisListBox::OnKeyDown(uint32_t wparam)
{
    //4dc8f6

    if (TestFlags(FLAG_FOCUS) == 0)
        return CVisualObject::OnKeyDown(wparam);

    switch (wparam)
    {
    case VK_PRIOR:
        PageUp();
        break;

    case VK_NEXT:
        PageDown();
        break;

    case VK_UP:
        Up();
        break;

    case VK_DOWN:
        Down();
        break;

    default:
        break;
    }

    switch (wparam)
    {
    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
        parent->MsgProc(0x46e, id, selected_index);
        return 1;

    default:
        break;
    }

    return 0;
}


int32_t VisListBox::IsValidIndex(int32_t idx)
{
    //4dc528
    if (idx > -1 && idx < entries.GetSize())
        return 1;
    return 0;
}

void VisListBox::DrawItem(int32_t idx, CPoint pos, uint16_t* clr)
{
    //4dc560
    if (!IsValidIndex(idx))
        return;

    CRect tmp;
    GetClipRect(&tmp);

    CRect rt = ClientRectToScreen(rect);

    CRect clip(pos.x, pos.y, rt.right - 6, pos.y + 2 + font->GetHeight());
    SetClipRect(clip);

    font->DrawTextWithShadow(pos.x, pos.y, entries[idx], 0, clr, 1);

    SetClipRect(tmp);
}

void VisListBox::SelectItem(int32_t idx)
{
    //4dbe7f

    if (idx < 0)
        idx = 0;

    if (idx >= entries.GetSize())
        idx = entries.GetSize() - 1;

    if (idx < 0)
        vis_start_index = 0;
    else if (idx < vis_start_index)
        vis_start_index = idx;
    else if (idx >= vis_start_index + num_vis_entry)
        vis_start_index = (idx - num_vis_entry) + 1;

    selected_index = idx;

    VMethod9();

    VisScrollBar* scrollbar = (VisScrollBar*)parent->FindChild(scrollbox_id);
    if (scrollbar)
        scrollbar->SetPos(selected_index, entries.GetSize());

    parent->MsgProc(0x46e, id, selected_index);
}

int32_t VisListBox::GetItemCount()
{
    //4507c0
    return entries.GetSize();
}

void VisListBox::VMethod30(CPoint pos, const CRect& r)
{
    //4dc4ee
    CRect rt;
    rt.top = pos.y - 1;
    rt.left = pos.x;
    rt.right = r.right - 4;
    rt.bottom = pos.y - 3 + entry_height_full;

    ShadowRect(rt, 10);
}


VisListBox::VisListBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: CVisualObject(_id, r, hint)
{
    //4dbd31

    entry_height = _font->GetHeight();
    entry_height_full = entry_height + 4;

    flags |= FLAG_NOTFOCUS;

    selected_index = -1;
    vis_start_index = 0;
    font = _font;
    clr1 = _clr1;
    clr2 = _clr2;
    scrollbox_id = _scrollid;

    num_vis_entry = rect.Height() / entry_height_full;

    rect.bottom = rect.top + 2 + num_vis_entry * entry_height_full;
}


VisListBox::VisListBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: CVisualObject(_id, l, t, r, b, hint)
{
    //4dbc11

    entry_height = _font->GetHeight();
    entry_height_full = entry_height + 4;

    flags |= FLAG_NOTFOCUS;

    selected_index = -1;
    vis_start_index = 0;
    font = _font;
    clr1 = _clr1;
    clr2 = _clr2;
    scrollbox_id = _scrollid;

    num_vis_entry = rect.Height() / entry_height_full;

    rect.bottom = rect.top + 2 + num_vis_entry * entry_height_full;
}


void VisListBox::Down()
{
    //4dbfbb
    SelectItem(selected_index + 1);
}

void VisListBox::Up()
{
    //4dbf89
    if (selected_index > 0)
        SelectItem(selected_index - 1);
}

void VisListBox::PageDown()
{
    //4dc03b
    if (selected_index == vis_start_index + num_vis_entry - 1)
        SelectItem(vis_start_index + num_vis_entry + num_vis_entry - 1);
    else
        SelectItem(vis_start_index + num_vis_entry - 1);
}

void VisListBox::PageUp()
{
    //4dbfe1
    if (selected_index == vis_start_index)
        SelectItem(vis_start_index - num_vis_entry);
    else
        SelectItem(vis_start_index);
}


int32_t VisListBox::YToIndex(int32_t y)
{
    //4dbe45
    return (y - ClientPtToScreen(rect.TopLeft()).y - 1) / entry_height_full;
}

void VisListBox::AddItem(const char* str)
{
    //4507f0
    entries.Add(str);
    if (selected_index < 0)
        selected_index++;
}

void VisListBox::SetSelectedIndex(int32_t idx)
{ 
    //4507a0
    if (idx < 0)
        idx = 0;
    else if (idx >= entries.GetSize())
        idx = entries.GetSize() - 1;
    selected_index = idx;
}

CString& VisListBox::GetItem(int32_t idx)
{
    //4508b0
    if (idx < 0)
        return entries[0];

    if (idx < entries.GetSize())
        return entries[idx];

    return entries[entries.GetSize() - 1];
}

void VisListBox::RemoveItem(int32_t idx)
{ //450830
    entries.RemoveAt(idx);
    selected_index -= (entries.GetSize() == 0); // WAT?
}




VisTextBox::~VisTextBox() = default; //4503a0

void VisTextBox::VMethod7()
{
    //4d9a21
    if (!parent)
        return;

    CRect rt = ClientRectToScreen(rect);

    LockSurface2();

    parent->VMethod8(&rt);

    FillRectColor(rt.left + 1, rt.top, rt.right - 1, rt.top, GetColorRGB(8, 8, 8));
    FillRectColor(rt.left, rt.top + 1, rt.left, rt.bottom - 1, GetColorRGB(8, 8, 8));
    FillRectColor(rt.right, rt.top + 1, rt.right, rt.bottom - 1, GetColorRGB(0x5e, 0x73, 0x65));
    FillRectColor(rt.left + 1, rt.bottom, rt.right - 1, rt.bottom, GetColorRGB(0x5e, 0x73, 0x65));

    if (select_start != select_end)
    {
        CRect rshd;
        rshd.bottom = rt.bottom - 2;

        CString s = text.Left(select_end);
        
        rshd.right = rt.left + 4 + font->GetStrWidth(s);
        rshd.top = rt.top + 2;

        s = text.Left(select_start);

        rshd.left = rt.left + 4 + font->GetStrWidth(s);

        ShadowRect(rshd, 12);
    }

    font->DrawTextWithShadow(rt.left + 4, rt.top + rt.Height() / 2, text, 8, clr, 1);

    if (TestFlags(FLAG_FOCUS) != 0 && cursor_blink != 0)
    {
        CRect blink;
        blink.left = rt.left + 4 + font->GetStrWidth(text.Left(cursor_pos));
        blink.right = blink.left + 2;
        blink.top = rt.top + 2;
        blink.bottom = rt.bottom - 2;
        FillRectColorSimple(blink.left, blink.top, blink.right, blink.bottom, GetColorRGB(255, 255, 255));
    }
    UnlockSurface2();
}

void VisTextBox::WriteData(void* buf)
{
    //450440
    strcpy((char*)buf, text);
}


uint32_t VisTextBox::DataSize()
{
    //450430
    return 4;
}


void VisTextBox::ReadData(const void* buf)
{
    //450410
    text = (const char*)buf;
}


int32_t VisTextBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    //4daa2b
    if (msg == 0x462 && (timeGetTime() - cursor_blink_ts) > 500)
    {
        cursor_blink_ts = timeGetTime();
        cursor_blink = 1 - cursor_blink;

        if (TestFlags(FLAG_ENABLED))
            VMethod9();
    }

    return CVisualObject::MsgProc(msg, wparam, lparam);
}


int32_t VisTextBox::OnMouseMove(uint32_t wparam, CPoint pos)
{
    //4da22e
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    if (TestFlags(FLAG_FOCUS) == 0)
        parent->FocusTo(this, true);

    cursor_blink_ts = timeGetTime();

    if ((wparam & 1) == 0)
        return 0;

    parent->FocusTo(this, true);

    int32_t cpos = GetCursorPosByX(pos.x);

    if (cpos < cursor_pos)
    {
        if (select_end == select_start)
        {
            select_end = cursor_pos;
            select_start = cpos;
        }
        else if (select_start < cpos)
            select_end = cpos;
        else
            select_start = cpos;
    }
    else if (cpos > cursor_pos)
    {
        if (select_end == select_start) {
            select_start = cursor_pos;
            select_end = cpos;
        }
        else if (cpos < select_end)
            select_start = cpos;
        else
            select_end = cpos;
    }

    cursor_pos = cpos;

    ResetBlink();

    VMethod9();

    return 0;
}


int32_t VisTextBox::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    //4da37c
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    cursor_blink_ts = timeGetTime();

    cursor_pos = GetCursorPosByX(pos.x);;

    select_end = cursor_pos;
    select_start = select_end;

    VMethod9();

    return 1;
}

int32_t VisTextBox::OnKeyDown(uint32_t wparam)
{
    //4da3ed
    if (TestFlags(FLAG_FOCUS) == 0)
        return 0;

    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    cursor_blink_ts = timeGetTime();

    int32_t res = 0;

    switch (wparam)
    {
    case VK_BACK:
        if (cursor_pos != 0)
        {
            cursor_pos--;

            text = text.Left(cursor_pos) + text.Right(text.GetLength() - cursor_pos - 1);
            
            ResetBlink();

            VMethod9();
            res = 1;
        }
        else
            res = 0;
        break;

    case VK_END:
        if (cursor_pos < text.GetLength())
        {
            if (g_kbShiftState == 0)
                select_end = select_start;
            else
            {
                if (select_start == select_end)
                    select_start = cursor_pos;
                else if (cursor_pos < select_end)
                    select_start = select_end;

                select_end = text.GetLength();
            }

            cursor_pos = text.GetLength();

            ResetBlink();

            VMethod9();
            res = 1;
        }
        break;

    case VK_HOME:
        if (cursor_pos != 0)
        {
            if (g_kbShiftState == 0)
                select_end = select_start;
            else
            {
                if (select_start == select_end)
                    select_end = cursor_pos;
                else if (select_start < cursor_pos)
                    select_end = select_start;

                select_start = 0;
            }

            cursor_pos = 0;

            ResetBlink();

            VMethod9();
            res = 1;
        }
        break;

    case VK_LEFT:
        if (cursor_pos != 0)
        {
            cursor_pos--;

            if (g_kbShiftState == 0)
                select_end = select_start;
            else if (select_end == select_start)
            {
                select_end = cursor_pos + 1;
                select_start = cursor_pos;
            }
            else if (cursor_pos + 1 == select_start)
                select_start--;
            else
                select_end--;

            ResetBlink();

            VMethod9();
            res = 1;
        }
        break;

    case VK_RIGHT:
        if (cursor_pos < text.GetLength())
        {
            cursor_pos++;
            if (g_kbShiftState == 0)
                select_end = select_start;
            else if (select_end == select_start)
            {
                select_start = cursor_pos - 1;
                select_end = cursor_pos;
            }
            else if (cursor_pos - 1 == select_end)
                select_end++;
            else
                select_start++;

            ResetBlink();

            VMethod9();
            res = 1;
        }
        break;

    case VK_DELETE:
        if (select_end == select_start || (select_end - select_start) < 0)
        {
            if (cursor_pos < text.GetLength())
                text = text.Left(cursor_pos) + text.Right((text.GetLength() - cursor_pos) - 1);
        }
        else
            DelSelection();

        ResetBlink();

        VMethod9();
        res = 1;
        break;

    default:
        break;
    }

    parent->MsgProc(0x46e, id, 0);

    return res;
}


int32_t VisTextBox::OnChar(uint32_t wparam)
{
    //4da96a

    if (isalnum(wparam) != 0 && TestFlags(FLAG_FOCUS) == 0)
        parent->FocusTo(this, true);

    if (TestFlags(FLAG_FOCUS) == 0 || !parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    cursor_blink_ts = timeGetTime();

    if (wparam >= ' ')
        InsertChar(wparam);

    VMethod9();

    parent->MsgProc(0x46e, id, 0);

    return 1;
}


VisTextBox::VisTextBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: CVisualObject(_id, l, t, r, b, hint)
{
    //4d9891
    font = _font;
    clr = _clr;
    text = "";
    select_start = 0;
    select_end = 0;
    cursor_pos = 0;

    flags |= FLAG_NOTFOCUS;

    cursor_blink = 1;
}


VisTextBox::VisTextBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr, const char* hint)
: CVisualObject(_id, r, hint)
{
    //4d995f
    font = _font;
    clr = _clr;
    text = "";
    select_start = 0;
    select_end = 0;
    cursor_pos = 0;

    flags |= FLAG_NOTFOCUS;

    cursor_blink = 1;
}

void VisTextBox::DelSelection()
{
    //4da067
    text = text.Left(select_start) + text.Right(text.GetLength() - select_end);
    select_end = select_start;
    cursor_pos = select_start;
}

void VisTextBox::InsertChar(int32_t chr)
{
    //4d9ef2
    if (select_end != select_start && (select_end - select_start) > -1)
        DelSelection();

    CString tmp = text.Left(cursor_pos) + (char)EncodeChar(chr) + text.Right(text.GetLength() - cursor_pos);
    if (font->GetStrWidth(tmp) + 8 < rect.Width())
    {
        text = tmp;
        cursor_pos++;
    }
    ResetBlink();
}

void VisTextBox::ResetBlink()
{
    //4daab3
    cursor_blink_ts = timeGetTime();
    cursor_blink = 1;
}

int32_t VisTextBox::GetCursorPosByX(int32_t x)
{
    //4da153
    CRect rt = ClientRectToScreen(rect);
    for (int32_t i = 0; i < text.GetLength(); i++)
    {
        CString str = text.Left(i + 1);
        if (rt.left + 4 + font->GetStrWidth(str) >= x)
            return i;
    }
    return text.GetLength();
}


VisRadioBase::~VisRadioBase() = default; //4505b0

void VisRadioBase::SetCursorOver(bool isOver)
{
    //4daccd
    CVisualObject::SetCursorOver(isOver);
    mouse_over = isOver;
}

void VisRadioBase::WriteData(void* buf)
{
    //4504f0
    *(int32_t*)buf = selection;
}

uint32_t VisRadioBase::DataSize()
{
    //4504e0
    return 4;
}

void VisRadioBase::ReadData(const void* buf)
{
    //4506c0
    selection = *(const int32_t*)buf;
    selected = selection;
}

int32_t VisRadioBase::OnMouseMove(uint32_t wparam, CPoint pos)
{
    //4dacef
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    if (TestFlags(FLAG_FOCUS) == 0)
        parent->FocusTo(this, true);

    if (wparam & 1)
        OnLButtonDown(wparam, pos);
    else
    {
        CRect rt = ClientRectToScreen(rect);

        if (rt.PtInRect(pos))
        {
            if (TestFlags(FLAG_OVERCURSOR) == 0)
                SetCursorOver(true);
        }
        else
            SetCursorOver(false);

        VMethod9();
    }
    return 0;
}

int32_t VisRadioBase::GetIndex(int32_t y)
{
    //4dac81
    CPoint t = ClientPtToScreen(rect.TopLeft());
    return (y - t.y) / gfx_radiob->GetHeight(0);
}

VisRadioBase::VisRadioBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: CVisualObject(_id, l, t, r, b, hint)
{
    //4dab2b
    font = _font;
    clr = _clr;
    selection = 0;
    flags |= FLAG_NOTFOCUS;
    selected = 0;
}

void VisRadioBase::AddEntry(const char* etext)
{
    //450470
    entries.Add(etext);
}



VisRadioType1::~VisRadioType1() = default; //450590

void VisRadioType1::VMethod7()
{
    //4dadf8
    CRect rt = ClientRectToScreen(rect);
    rt.bottom += 4;
    rt.right += 4;

    int32_t x = rt.left + 6 + gfx_radiob->GetWidth(0);
    int32_t y = rt.top;

    LockSurface2();

    parent->VMethod8(&rt);

    rt.bottom -= 4;
    rt.right -= 4;

    for (int i = 0; i < entries.GetSize(); i++)
    {
        if ((selection & (1 << i)) == 0)
        {
            gfx_radiob->VMethod3(rt.left + 5, y + 4, 2, 4, 0);
            gfx_radiob->VMethod2(rt.left + 1, y, 2, 0, 0);
        }
        else
        {
            gfx_radiob->VMethod3(rt.left + 5, y + 4, 3, 4, 0);
            gfx_radiob->VMethod2(rt.left + 1, y, 3, 0, 0);
        }

        if (TestFlags(FLAG_FOCUS) == 0)
            clr = p_clrsh_Black;
        else
            clr = p_clrsh_Gold;

        font->DrawTextWithShadow(x, y + 5, entries[i], 0, clr, 1);

        y += gfx_radiob->GetHeight(0);
    }

    if (TestFlags(FLAG_ENABLED) == 0)
    {
        rt = ClientRectToScreen(rect);
        rt.InflateRect(CRect(1, 1, 1, 1));
        ShadowRect(rt, 3);
    }

    UnlockSurface2();
}

void VisRadioType1::ReadData(const void* buf)
{
    //450510
    selection = *(const int32_t*)buf;
}

int32_t VisRadioType1::OnMouseMove(uint32_t wparam, CPoint pos)
{
    //4db134
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    if (TestFlags(FLAG_FOCUS) == 0)
        parent->FocusTo(this, true);

    if (wparam & 1)
        OnLButtonDown(wparam, pos);
    else
    {
        CRect rt = ClientRectToScreen(rect);
        rt.bottom += 4;
        rt.right += 4;

        if (rt.PtInRect(pos))
        {
            if (TestFlags(FLAG_OVERCURSOR) == 0)
                SetCursorOver(true);
        }
        else
            SetCursorOver(false);

        VMethod9();
    }
    return 0;
}

int32_t VisRadioType1::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    //4db062
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    selected = GetIndex(pos.y);

    uint32_t bit = 1 << selected;

    if ((selection & bit) == 0)
        selection |= bit;
    else
        selection &= ~bit;

    VMethod9();
    parent->MsgProc(0x46e, id, selection);
    return 1;
}


int32_t VisRadioType1::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    //450530
    return OnLButtonDown(wparam, pos);
}

int32_t VisRadioType1::OnKeyDown(uint32_t wparam)
{
    //4db2d3
    if (wparam == VK_DOWN && TestFlags(FLAG_FOCUS) != 0)
    {
        int32_t num = entries.GetSize();
        if (num <= 0)
            return 0;

        if (selected < num - 1)
            selected++;

        VMethod9();
        return 1;
    }

    if (wparam == VK_UP && TestFlags(FLAG_FOCUS) != 0)
    {
        int32_t num = entries.GetSize();
        if (num <= 0)
            return 0;

        if (selected > 0)
            selected--;

        VMethod9();
        return 1;
    }

    return CVisualObject::OnKeyDown(wparam);
}


int32_t VisRadioType1::OnChar(uint32_t wparam)
{
    //4db20d
    if (wparam != ' ' || TestFlags(FLAG_FOCUS) == 0)
        return CVisualObject::OnChar(wparam);

    uint32_t bit = 1 << selected;
    if ((selection & bit) == 0)
        selection |= bit;
    else 
        selection &= ~bit;
    
    VMethod9();
    parent->MsgProc(0x46e, id, selection);
    return 1;
}

VisRadioType1::VisRadioType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
 : VisRadioBase(_id, l, t, r, b, _font, _clr, hint)
{
    //450490
}


VisRadioType2::~VisRadioType2() = default; //450720


void VisRadioType2::VMethod7()
{
    //4db680
    CRect rt = ClientRectToScreen(rect);
    rt.bottom += 4;
    rt.right += 4;

    int32_t x = rt.left + 6 + gfx_radiob->GetWidth(0);
    int32_t y = rt.top;

    LockSurface2();

    parent->VMethod8(&rt);

    rt.bottom -= 4;
    rt.right -= 4;

    for (int i = 0; i < entries.GetSize(); i++)
    {
        if (selection == i)
        {
            gfx_radiob->VMethod3(rt.left + 5, y + 4, 1, 4, 0);
            gfx_radiob->VMethod2(rt.left + 1, y, 1, 0, 0);

            if (TestFlags(FLAG_FOCUS) == 0)
                clr = p_clrsh_Black;
            else
                clr = p_clrsh_Gold;
        }
        else
        {
            gfx_radiob->VMethod3(rt.left + 5, y + 4, 0, 4, 0);
            gfx_radiob->VMethod2(rt.left + 1, y, 0, 0, 0);

            clr = p_clrsh_Black;
        }

        font->DrawTextWithShadow(x, y + 5, entries[i], 0, clr, 1);

        y += gfx_radiob->GetHeight(0);
    }

    if (TestFlags(FLAG_ENABLED) == 0)
    {
        rt = ClientRectToScreen(rect);
        rt.InflateRect(CRect(1, 1, 1, 1));
        ShadowRect(rt, 3);
    }

    UnlockSurface2();
}

int32_t VisRadioType2::OnMouseMove(uint32_t wparam, CPoint pos)
{
    //4dbb7b
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;
    return VisRadioBase::OnMouseMove(wparam, pos);
}

int32_t VisRadioType2::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    //4db8e7
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    int32_t idx = GetIndex(pos.y);
    if (idx < 0)
        return 0;
    if (idx >= entries.GetSize())
        return 0;

    selection = idx;
    selected = idx;
    VMethod9();
    parent->MsgProc(0x46e, id, selection);
    return 1;
}

int32_t VisRadioType2::OnKeyDown(uint32_t wparam)
{
    //4dba0e
    if (wparam == VK_DOWN && TestFlags(FLAG_FOCUS) != 0)
    {
        int32_t num = entries.GetSize();
        if (num <= 0)
            return 0;

        if (selection < num - 1)
            selection++;

        selected = selection;
        parent->MsgProc(0x46e, id, selection);
        VMethod9();
        return 1;
    }

    if (wparam == VK_UP && TestFlags(FLAG_FOCUS) != 0)
    {
        int32_t num = entries.GetSize();
        if (num <= 0)
            return 0;

        if (selection > 0)
            selection--;

        selected = selection;
        parent->MsgProc(0x46e, id, selection);
        VMethod9();
        return 1;
    }

    return CVisualObject::OnKeyDown(wparam);
}

int32_t VisRadioType2::OnChar(uint32_t wparam)
{
    //4db993
    if (wparam != ' ' || TestFlags(FLAG_FOCUS) == 0)
        return CVisualObject::OnChar(wparam);

    selection = selected;

    VMethod9();
    parent->MsgProc(0x46e, id, selection);
    return 1;
}

VisRadioType2::VisRadioType2(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: VisRadioBase(_id, l, t, r, b, _font, _clr, hint)
{
    //450670
}



VisBitmap::~VisBitmap()
{
    //4df2c3
    if (bitmap)
        delete bitmap;
}

void VisBitmap::VMethod7()
{
    //4df345
    if (!bitmap)
        return;

    CRect rt = ClientRectToScreen(rect);
    LockSurface2();

    if (draw_type == 1)
    {
        parent->VMethod8(&rt);
        CBmp64* bmp64 = (CBmp64*)bitmap;
        bmp64->VMethod10(rt.left, rt.top, 0, 0, bmp64->GetWidth(0), bmp64->GetHeight(0));
    }
    else if (draw_type == 2)
    {
        parent->VMethod8(&rt);

        FillRectColorSimple(rt.left + 8, rt.top + 7, rt.left + 80, rt.top + 101, 0);

        CBmp64* bmp64 = (CBmp64*)bitmap;
        bmp64->VMethod10(rt.left, rt.top, 0, 0, bmp64->GetWidth(0), bmp64->GetHeight(0));
    }
    else if (draw_type < 11)
        bitmap->VMethod2(rt.left, rt.top, 0, 0, 0);
    else
        bitmap->VMethod2(rt.left, rt.top, draw_type - 10, 0, 0);

    UnlockSurface2();
}

VisBitmap::VisBitmap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, int32_t dtype)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    //4df23d
    bitmap = _bitmap;
    draw_type = dtype;
}

VisBitmap::VisBitmap(int32_t _id, const RECT& r, CGameBitmap* _bitmap, int32_t dtype)
: CVisualObject(_id, r, nullptr)
{
    //4df286
    bitmap = _bitmap;
    draw_type = dtype;
}

CGameBitmap* VisBitmap::GetBitmap()
{
    //4e3fb0
    return bitmap;
}

void VisBitmap::SetBitmap(CGameBitmap* bmp)
{
    //4e3fd0
    bitmap = bmp;
}





void VisComboBoxText::VMethod7()
{
    //4e4170
    CRect rt = ClientRectToScreen(rect);

    LockSurface2();

    parent->VMethod8(&rt);

    FillRectColor(rt.left + 1, rt.top, rt.right - 1, rt.top, GetColorRGB(8, 8, 8));
    FillRectColor(rt.left, rt.top + 1, rt.left, rt.bottom - 1, GetColorRGB(8, 8, 8));
    FillRectColor(rt.right, rt.top + 1, rt.right, rt.bottom - 1, GetColorRGB(0x5e, 0x73, 0x65));
    FillRectColor(rt.left + 1, rt.bottom, rt.right - 1, rt.bottom, GetColorRGB(0x5e, 0x73, 0x65));

    uint16_t *clr = p_clrsh_Gold;
    if (parent->TestFlags(FLAG_FOCUS) == 0)
        clr = p_clrsh_Black;

    font->DrawTextWithShadow(rt.left + 4, rt.top + rt.Height() / 2, text, 8, clr, 1);

    UnlockSurface2();
}

VisComboBoxText::VisComboBoxText(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
: VisTextBox(_id, l, t, r, b, _font, _clr, hint)
{
    //4e4120
}



int32_t VisComboBoxList::OnKeyDown(uint32_t wparam)
{
    //4e4090
    if (wparam == VK_RETURN)
    {
        ((VisComboBox*)parent)->ProcSelectList();
        return 1;
    }
    return VisListBox::OnKeyDown(wparam);
}

VisComboBoxList::VisComboBoxList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: VisListBox(_id, l, t, r, b, _font, _clr1, _clr2, _scrollid, hint)
{
    //4e4040
}


void VisComboBoxButton::SetCursorOver(bool isOver)
{
    //4e1cb6
    CVisualObject::SetCursorOver(isOver);

    if (isOver)
    {
        if (parent->TestFlags(FLAG_OVERCURSOR) == 0)
            parent->SetCursorOver(true);
    }
    else
    {
        if (TestFlags(FLAG_OVERCURSOR) && ((VisComboBox*)parent)->list_showed == 0)
            parent->SetCursorOver(false);
    }
}


void VisComboBoxButton::VMethod7()
{
    //4e1d7d
    CRect rt = ClientRectToScreen(rect);
    parent->VMethod8(&rt);

    if (mouse_on)
        bitmap->VMethod2(rt.left, rt.top, frm + 1, 0, 0);
    else
        bitmap->VMethod2(rt.left, rt.top, frm, 0, 0);
}

int32_t VisComboBoxButton::OnMouseMove(uint32_t wparam, CPoint pos)
{
    //4e1d5c
    return VisButton::OnMouseMove(wparam, pos);
}

int32_t VisComboBoxButton::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    //4e1d3b
    return VisButton::OnLButtonDown(wparam, pos);
}

int32_t VisComboBoxButton::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    //4e1c16
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    if (downed == 0)
        return 0;

    CRect rt = ClientRectToScreen(rect);
    VisButton::SetDowned(false);

    if (rt.PtInRect(pos))
        parent->MsgProc(msgid, 0, 0);

    return 1;
}

VisComboBoxButton::VisComboBoxButton(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, int32_t _frm, int32_t _msgid, int32_t _charid, const char* hint)
: VisButton(_id, l, t, r, b, "", g_font1, clrsh_TechBlack, _msgid, _charid, hint)
{
    //4e1b5c
    bitmap = _bitmap;
    frm = _frm;
}



void VisComboBox::VMethod8(CRect* rect)
{
    //4e2481
    parent->VMethod8(rect);
}

void VisComboBox::VMethod9()
{
    //4e201e
    CVisualObject::VMethod9();
}

void VisComboBox::WriteData(void* buf)
{
    //4e2031
    textbox->WriteData(buf);
}

int32_t VisComboBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    //4e22d8
    if (msg == 0x456)
    {
        ToggleList();
        return 1;
    }
    else if (msg == WM_LBUTTONDOWN)
    {
        CRect rt = ClientRectToScreen(rect);
        CPoint cpt(g_mousept.x, g_mousept.y);
        if (rt.PtInRect(cpt))
        {
            parent->FocusTo(this, true);
            return CVisualObject::MsgProc(WM_LBUTTONDOWN, wparam, lparam);
        }
        else
        {
            HideList();
            parent->OnLButtonDown(1, cpt);
            return 1;
        }
    }
    else if (msg == 0x473)
    {
        if (wparam == listbox->GetId())
        {
            ProcSelectList();
            return 1;
        }
    }
    else if (msg == WM_MOUSEMOVE)
        return CVisualObject::MsgProc(WM_MOUSEMOVE, wparam, lparam);

    return CVisualObject::MsgProc(msg, wparam, lparam);
}

int32_t VisComboBox::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    //4e2410
    return 1;
}

int32_t VisComboBox::OnKeyDown(uint32_t wparam)
{
    //4e2422
    if (wparam != VK_DOWN || TestFlags(FLAG_FOCUS) == 0)
        return CVisualObject::OnKeyDown(wparam);

    //VK_DOWN
    SetFocus(true);
    listbox->SetFocus(true);
    ToggleList();
    return 1;
}

VisComboBox::VisComboBox(int32_t _id, CRect r, const char* hint)
: CVisualObject(_id, r.left, r.top, r.right, r.top + 24, hint)
{
    //4e1e10
    textbox = new VisComboBoxText(1, 0, 0, (r.right - r.left) - 24, 24, g_font1, clrsh_TechBlack, hint);
    textbox->ChangeFlags(FLAG_ENABLED, false);

    AddChild(textbox);

    listbox = new VisComboBoxList(2, 0, 24, (r.right - r.left), (r.bottom - r.top), g_font1, clrsh_TechBlack, clrsh_ShockingBlack, 10, hint);
    listbox->ChangeFlags(FLAG_20, true);

    AddChild(listbox);

    VisComboBoxButton* btn = new VisComboBoxButton(3, (r.right - r.left) - 22, 2, (r.right - r.left) - 2, 22, gfx_scrollbars, 24, 0x456, 0, "");
    AddChild(btn);

    isEmpty = 1;
    list_showed = 0;
    flags |= FLAG_NOTFOCUS;
}

void VisComboBox::AddItem(const char* str)
{
    //4e2053
    if (isEmpty)
    {
        textbox->ReadData(str);
        isEmpty = 0;
    }

    listbox->AddItem(str);
}

void VisComboBox::ToggleList()
{
    //4e2150
    if (list_showed)
    {
        HideList();
        return;
    }

    list_showed = 1;

    if (TestFlags(FLAG_OVERCURSOR) == 0)
        SetCursorOver(true);

    listbox->ChangeFlags(FLAG_20, false);
    listbox->SetFocus(true);

    rect.bottom = rect.top + 24 + listbox->GetRect().Height();

    CRect rt = ClientRectToScreen(rect);
    rt.top += 24;

    parent->VMethod8(&rt);
    VMethod9();
}

void VisComboBox::HideList()
{
    //4e20db
    SetCursorOver(false);

    list_showed = 0;

    listbox->ChangeFlags(FLAG_20, true);
    listbox->SetFocus(false);

    rect.bottom = rect.top + 24;

    parent->VMethod9();
    VMethod9();
}

void VisComboBox::ProcSelectList()
{
    //4e222a
    if (list_showed != 0)
    {
        SetCursorOver(false);
        list_showed = 0;
    }

    listbox->SetFocus(false);
    listbox->ChangeFlags(FLAG_20, true);

    textbox->ReadData( listbox->GetItem(listbox->GetSelectedIndex()) );

    rect.bottom = rect.top + 24;

    parent->VMethod9();
    VMethod9();
}

void VisComboBox::SelectItem(int32_t index)
{
    //4e2097
    textbox->ReadData(listbox->GetItem(index));
    listbox->SetSelectedIndex(index);
}




void VisMultiText::VMethod7()
{
    //4d8994
    if (!parent)
        return;

    CRect rt = ClientRectToScreen(rect);
    parent->VMethod8(&rt);

    LockSurface2();
    font->DrawTextLinesShadow(rt, vis_start_index, vis_start_index + num_vis_entry, entries, clr1, entry_height_full);
    UnlockSurface2();
}

int32_t VisMultiText::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    //4d8b99
    return VisListBox::MsgProc(msg, wparam, lparam);
}

int32_t VisMultiText::OnMouseMove(uint32_t wparam, CPoint pos)
{
    //4d8bba
    return CVisualObject::OnMouseMove(wparam, pos);
}

int32_t VisMultiText::OnWmUser(uint32_t wparam, CPoint pos)
{
    //4d8bdb
    if ((wparam & 1) == 0 || scrollbox_id == 0)
        return 1;

    CRect rt = ClientRectToScreen(rect);
    if ((rt.top + rt.bottom) / 2 < pos.y)
        VisListBox::Down();
    else
        VisListBox::Up();

    return 1;
}

int32_t VisMultiText::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    //4d8c3f
    return CVisualObject::OnLButtonDown(wparam, pos);
}

int32_t VisMultiText::OnKeyDown(uint32_t wparam)
{
    //4d8c60
    return VisListBox::OnKeyDown(wparam);
}

void VisMultiText::SelectItem(int32_t idx)
{
    //4d889d
    if (idx < 0)
        idx = 0;

    if (idx < entries.GetSize() - num_vis_entry)
    {
        selected_index = idx;
        vis_start_index = idx;
    }
    else
    {
        selected_index = entries.GetSize() - num_vis_entry;
        vis_start_index = selected_index;
    }

    VMethod9();

    VisScrollBar* scroll = (VisScrollBar*)parent->FindChild(scrollbox_id);
    if (scroll)
        scroll->SetPos(vis_start_index, (entries.GetSize() - num_vis_entry) + 1);

    parent->MsgProc(0x46e, id, vis_start_index);
}


VisMultiText::VisMultiText(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str, CGameFont* _font, uint16_t* _clr, int32_t dy)
: VisListBox(_id, l, t, r, b, _font, _clr, nullptr, 0, nullptr)
{
    text = str;

    if (dy == 0)
        dy = entry_height + 2;

    entry_height_full = dy;
    entries.Copy( font->StringArrayForRect(rect, str) );

    if (rect.Height() / entry_height_full < entries.GetSize())
        num_vis_entry = rect.Height() / entry_height_full;
    else
        num_vis_entry = entries.GetSize();
}

VisMultiText::VisMultiText(int32_t _id, const RECT& r, const char* str, CGameFont* _font, uint16_t* _clr, int32_t dy)
: VisListBox(_id, r, _font, _clr, nullptr, 0, nullptr)
{
    text = str;

    if (dy == 0)
        dy = entry_height + 2;

    entry_height_full = dy;
    entries.Copy(font->StringArrayForRect(rect, str));

    if (rect.Height() / entry_height_full < entries.GetSize())
        num_vis_entry = rect.Height() / entry_height_full;
    else
        num_vis_entry = entries.GetSize();
}

void VisMultiText::SetText(const char* _text)
{
    //4d8816
    text = _text;

    entries.Copy(font->StringArrayForRect(rect, _text));

    if (rect.Height() / entry_height_full < entries.GetSize())
        num_vis_entry = rect.Height() / entry_height_full;
    else
        num_vis_entry = entries.GetSize();
}

void VisMultiText::SizesCheck()
{
    //4d8a3b
    num_vis_entry = entries.GetSize();

    if (entry_height * num_vis_entry <= rect.Height())
    {
        rect.bottom = rect.top + num_vis_entry * entry_height_full;
        return;
    }

    rect.right = rect.right - 26;

    entries.Copy(font->StringArrayForRect(rect, text));
    
    scrollbox_id = 0xdf23;

    VisScrollBar* scroll = new VisScrollBar(scrollbox_id, rect.right, rect.top, rect.right + 24, rect.bottom, nullptr);
    parent->AddChild(scroll);

    num_vis_entry = rect.Height() / entry_height_full;
}


//4df4d1
VisScreen::VisScreen() = default;

//4df4fa
VisScreen::VisScreen(int32_t _id, const RECT& r, CGameBitmap* _bitmap)
: CVisualObject(_id, r, nullptr)
{
    bitmap = _bitmap;
    VisScreen::VMethod26();
}

//4df56d
VisScreen::VisScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    bitmap = _bitmap;
    VisScreen::VMethod26();
}

//4df5ec
VisScreen::~VisScreen()
{
    VisScreen::VMethod27();
}

//4df63d
void VisScreen::VMethod7()
{
    CRect r = ClientRectToScreen(rect);

    VMethod8(&r);

    CVisualObject::VMethod7();
}

//4df63d
void VisScreen::VMethod8(CRect* r)
{
    r->IntersectRect(r, &g_ScreenSize);

    if (bitmap)
    {
        //CRect r2;
        //ClientRectToScreen(&r2, rect);

        CRect tmp;
        GetClipRect(&tmp);
        SetClipRect(*r);
        LockSurface2();

        bitmap->VMethod2(rect.left, rect.top, 0, 0, 0);

        UnlockSurface2();
        SetClipRect(tmp);
    }
    else
    {
        CRect r2 = ClientRectToScreen(rect);

        r2.right -= 8;
        r2.bottom -= 8;

        CRect tmp;
        GetClipRect(&tmp);
        SetClipRect(*r);
        LockSurface2();

        gfx_interface_lm->VMethod3(r2.right - 40, r2.top + 8, 3, 6, 0);
        gfx_interface_lm->VMethod3(r2.left + 8, r2.bottom - 40, 6, 6, 0);
        gfx_interface_lm->VMethod3(r2.right - 40, r2.bottom - 40, 8, 6, 0);

        for (int x = 0; x < (r2.Width() - 96); x += 96)
            gfx_interface_lm->VMethod3(r2.left + 56 + x, r2.bottom - 40, 7, 6, 0);

        for (int y = 0; y < (r2.Height() - 96); y += 64)
            gfx_interface_lm->VMethod3(r2.right - 40, r2.top + 56 + y, 5, 6, 0);


        gfx_interface_lm->VMethod2(r2.left, r2.top, 1, 0, 0);
        gfx_interface_lm->VMethod2(r2.right - 48, r2.top, 3, 0, 0);
        gfx_interface_lm->VMethod2(r2.left, r2.bottom - 48, 6, 0, 0);
        gfx_interface_lm->VMethod2(r2.right - 48, r2.bottom - 48, 8, 0, 0);

        for (int x = 0; x < (r2.Width() - 96); x += 96)
        {
            gfx_interface_lm->VMethod2(r2.left + 48 + x, r2.top, 2, 0, 0);
            gfx_interface_lm->VMethod2(r2.left + 48 + x, r2.bottom - 48, 7, 0, 0);
        }

        for (int y = 0; y < (r2.Height() - 96); y += 64)
        {
            gfx_interface_lm->VMethod2(r2.left, r2.top + 48 + y, 4, 0, 0);
            gfx_interface_lm->VMethod2(r2.right - 48, r2.top + 48 + y, 5, 0, 0);
        }

        for (int x = 0; x < (r2.Width() - 96); x += 96)
        {
            for (int y = 0; y < (r2.Height() - 96); y += 64)
            {
                gfx_interface_lm->VMethod2(r2.left + 48 + x, r2.top + 48 + y, 0, 0, 0);
            }
        }

        UnlockSurface2();
        SetClipRect(tmp);
    }
}


int32_t VisScreen::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    //4dfbcd
    if (msg == 0x445 || msg == 0x446)
    {
        if (is_active)
        {
            DoClose(msg);
            AfxGetMainWnd()->PostMessageA(0x44c, (WPARAM)this, 0);
        }
        return 1;
    }
    else
        return CVisualObject::MsgProc(msg, wparam, lparam);
}

int32_t VisScreen::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    //4dfee7
    if (cursor_over_obj_last == cursor_over_obj)
    {
        cursor_over_obj_last = nullptr;
        CVisualObject* obj = GetChildAt(pos);
        if (obj && obj != this)
        {
            return obj->OnLButtonDown(wparam, pos);
        }
    }

    return CVisualObject::OnLButtonDown(wparam, pos);
}

int32_t VisScreen::OnKeyDown(uint32_t wparam)
{
    //4dfc43
    switch (wparam)
    {
    case VK_TAB:
        TabFocus(g_kbShiftState == 0, true);
        return 1;

    case VK_LEFT:
        if (focus_obj && focus_obj->left_obj)
        {
            FocusTo(focus_obj->left_obj, true);
            return 1;
        }
        break;

    case VK_UP:
        if (focus_obj && focus_obj->up_obj)
        {
            FocusTo(focus_obj->up_obj, true);
            return 1;
        }
        break;

    case VK_RIGHT:
        if (focus_obj && focus_obj->right_obj)
        {
            FocusTo(focus_obj->right_obj, true);
            return 1;
        }
        break;

    case VK_DOWN:
        if (focus_obj && focus_obj->down_obj)
        {
            FocusTo(focus_obj->down_obj, true);
            return 1;
        }
        break;

    default:
        break;
    }

    return 0;
}


void VisScreen::VMethod26()
{
    //450900
}

void VisScreen::VMethod27()
{
    //4388c0
}

void VisScreen::VMethod28()
{
    //4dfb4f
    is_active = 1;
    SetCursorOver(true);
    SetFocus(true);
    TabFocus(true, false);
}

void VisScreen::DoClose(uint32_t code)
{
    //4dfb8a 29 method
    if (is_active)
    {
        is_active = 0;
        SetCursorOver(false);
        SetFocus(false);
        exit_code = code;
    }
}

int32_t VisScreen::GetCloseCode()
{
    //4388d0
    return exit_code;
}

void VisScreen::CloseOk()
{
    //4327b5
    MsgProc(0x445, 0, 0);
}

void VisScreen::CloseCancel()
{
    //4327d4
    MsgProc(0x446, 0, 0);
}


VisWindow::~VisWindow()  //451a70
{}

int32_t VisWindow::OnKeyDown(uint32_t wparam)
{
    //4dfeb2
    if (wparam == VK_ESCAPE)
        return MsgProc(0x446, 0, 0);
    else
        return VisScreen::OnKeyDown(wparam);
}

VisWindow::VisWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap)
:VisScreen(_id, l, t, r, b, _bitmap)
{
    //4dfdab
    VisWindow::UpdateWinRect();
}

void VisWindow::UpdateWinRect()
{
    //4dfe19
    int32_t w = ((rect.Width() - 8) / 96) * 96 + 8;
    int32_t h = ((rect.Height() - 104) / 64) * 64 + 104;

    rect.left = (g_ScreenSize.right - w) / 2;
    rect.top = (g_ScreenSize.bottom - h) / 2;
    rect.right = rect.left + w;
    rect.bottom = rect.top + h;
}


VisMessageBox::~VisMessageBox()
{
    //444ff3
}

int32_t VisMessageBox::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    //44500f
    if (msg >= 0x445 && msg <= 0x44b)
    {
        int32_t res = VisScreen::MsgProc(0x446, wparam, lparam);
        exit_code = msg;
        VMethod31(msg);
        return res;
    }
    else
        return VisScreen::MsgProc(msg, wparam, lparam);
}


void VisMessageBox::VMethod26()
{
    //444086
    CRect r2;
    if (field_0x6c && strlen(field_0x6c) > 0)
    {
        if (g_font1->GetStrWidth(field_0x6c) > rect.Width())
        {
            VisMultiText* txt = new VisMultiText(0, 40, 32, rect.Width() - 50, 104, field_0x6c, g_font1, p_clrsh_Black, 0);

            AddChild(txt);

            txt->SizesCheck();

            r2 = CRect(40, 116, rect.Width() - 40, rect.Height() - 112);
        }
        else
        {
            VisLabel* lbl = new VisLabel(1, 40, 32, rect.Width() - 40, 56, field_0x6c, g_font1, p_clrsh_Black, 2);

            AddChild(lbl);

            r2 = CRect(40, 68, rect.Width() - 40, rect.Height() - 112);
        }
    }
    else
        r2 = CRect(40, 56, rect.Width() - 40, rect.Height() - 88);

    if (field_0x70 && strlen(field_0x70) > 0)
    {
        r2.OffsetRect(0, 12);
        
        AddChild( new VisLabel(-1, r2.left, r2.top - 20, r2.right, r2.top - 4, field_0x70, g_font1, p_clrsh_Black, 0) );
    }

    CVisualObject* obj = VMethod30(payload, r2);
    rect.bottom = rect.top + 144 + obj->GetRect().bottom;
    UpdateWinRect();

    CRect local_20(rect.Width() / 2 - 48, rect.Height() - 60, rect.Width() / 2 + 48, rect.Height() - 36);
    CRect local_44(rect.Width() / 7, rect.Height() - 60, (rect.Width() * 3) / 7, rect.Height() - 36);
    CRect local_54((rect.Width() * 4) / 7, rect.Height() - 60, (rect.Width() * 6) / 7, rect.Height() - 36);
    CRect local_64((rect.Width() * 3) / 20, rect.Height() - 60, (rect.Width() * 7) / 20, rect.Height() - 36);
    CRect local_74((rect.Width() * 8) / 20, rect.Height() - 60, (rect.Width() * 12) / 20, rect.Height() - 36);
    CRect local_84((rect.Width() * 13) / 20, rect.Height() - 60, (rect.Width() * 17) / 20, rect.Height() - 36);

    switch (button_types)
    {
    case 0:
    case 0x1000:
    {
        //ok
        VisButton* btn = new VisButton(4, local_20, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "");
        AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);
    }
        break;

    case 1:
    {
        //ok cancel
        VisButton* btn = new VisButton(4, local_44, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "");
        AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);

        VisButton* btn2 = new VisButton(5, local_54, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
        AddChild(btn2);

        btn->SetRightObj(btn2);
    }
        break;

    case 2:
    {
        //cancel repeat ignore
        VisButton* btn = new VisButton(4, local_64, txt_dialogs.GetLine(2), g_font1, nullptr, 0x449, 0, "");
        AddChild(btn);

        VisButton* btn2 = new VisButton(5, local_74, txt_dialogs.GetLine(3), g_font1, nullptr, 0x44a, 0, "");
        AddChild(btn2);
        btn2->ChangeFlags(FLAG_10, true);

        VisButton* btn3 = new VisButton(6, local_84, txt_dialogs.GetLine(4), g_font1, nullptr, 0x44b, 0, "");
        AddChild(btn3);

        btn->SetRightObj(btn2);
        btn2->SetRightObj(btn3);
    }
        break;

    case 3:
    {
        //yes no cancel
        VisButton* btn = new VisButton(4, local_64, txt_main.GetLine(75), g_font1, nullptr, 0x447, 0, "");
        AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);

        VisButton* btn2 = new VisButton(5, local_74, txt_main.GetLine(76), g_font1, nullptr, 0x448, 0, "");
        AddChild(btn2);

        VisButton* btn3 = new VisButton(6, local_84, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
        AddChild(btn3);

        btn->SetRightObj(btn2);
        btn2->SetRightObj(btn3);
    }
        break;

    case 4:
    {
        //yes no
        VisButton* btn = new VisButton(4, local_44, txt_main.GetLine(75), g_font1, nullptr, 0x447, 0, "");
        AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);

        VisButton* btn2 = new VisButton(5, local_54, txt_main.GetLine(76), g_font1, nullptr, 0x448, 0, "");
        AddChild(btn2);

        btn->SetRightObj(btn2);
    }
        break;

    case 5:
    {
        //repeat cancel
        VisButton* btn = new VisButton(4, local_44, txt_dialogs.GetLine(3), g_font1, nullptr, 0x44a, 0, "");
        AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);

        VisButton* btn2 = new VisButton(5, local_54, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
        AddChild(btn2);

        btn->SetRightObj(btn2);
    }
        break;

    //from allods2.exe
    case 0x2000:
    {
        //ok
        VisButton* btn = new VisButton(4, local_20, txt_dialogs.GetLine(0), g_font1, nullptr, 0x447, 0, "");
        AddChild(btn);
        btn->ChangeFlags(FLAG_10, true);
    }
        break;


    default:
        break;
    }

    CVisualObject* dobj = FindChild(5);
    if (dobj)
        obj->SetDownObj(dobj);

    dobj = FindChild(6);
    if (dobj)
        obj->SetDownObj(dobj);

    dobj = FindChild(4);
    if (dobj)
        obj->SetDownObj(dobj);
}


void VisMessageBox::VMethod31(int32_t code)
{
    //445084
}

VisMessageBox::VisMessageBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const void* _payload, const char* str2, int32_t btypes, const char* str3)
: VisWindow(_id, l, t, r, b, nullptr)
{
    //44402b
    field_0x70 = str2;
    field_0x6c = str3;
    payload = _payload;
    button_types = btypes;
}


CVisualObject* VisMessageBoxWithList::VMethod30(const void* str, const RECT& r)
{ //4450d4
    VisMultiText* txt = new VisMultiText(2, r, (const char*)str, g_font1, p_clrsh_Black, 0);
    AddChild(txt);
    txt->SizesCheck();
    return txt;
}

VisMessageBoxWithList::VisMessageBoxWithList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str1, const char* str2, int32_t btypes)
: VisMessageBox(_id, l, t, r, b, str1, str2, btypes, nullptr)
{ //445091
}


// 4DCEBA
VisListBoxDiplomacy::~VisListBoxDiplomacy()
{
    delete this->enemy_radios;
    delete this->ally_radios;
    delete this->mute_radios;
    // see_radios is intentionally not deleted, matching the original dtor.
}


// 4DC9FB
VisListBoxDiplomacy::VisListBoxDiplomacy(int32_t _id, const RECT& r, CArray<DiplomacyEntry*>* _diplomacy, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _entry_height)
: VisListBox(_id, r, _font, _clr1, _clr2, -1, nullptr)
{
    this->diplomacy = _diplomacy;
    if (_entry_height == 0) {
        _entry_height = this->entry_height;
    }
    if ((uint32_t)_entry_height <= 0x18) {
        _entry_height = 0x18;
    }
    this->entry_height_full = _entry_height + 8;
    this->num_vis_entry = this->rect.Height() / this->entry_height_full;

    this->enemy_radios = new CArray<VisRadioType1*>();
    this->ally_radios = new CArray<VisRadioType1*>();
    this->see_radios = new CArray<VisRadioType1*>();
    this->mute_radios = new CArray<VisRadioType1*>();

    int32_t count = this->diplomacy->GetSize();
    this->enemy_radios->SetSize(count);
    this->ally_radios->SetSize(count);
    this->see_radios->SetSize(count);
    this->mute_radios->SetSize(count);

    for (int32_t i = 0; i < count; i++) {
        DiplomacyEntry* entry = this->diplomacy->ElementAt(i);

        VisRadioType1* radio = new VisRadioType1(i, 0, 0, 0, 0, _font, _clr1, nullptr);
        this->enemy_radios->ElementAt(i) = radio;
        radio->AddEntry(" ");
        radio->ReadData(&entry->enemy);
        this->AddChild(radio);

        radio = new VisRadioType1(i + count, 0, 0, 0, 0, _font, _clr1, nullptr);
        this->ally_radios->ElementAt(i) = radio;
        radio->AddEntry(" ");
        radio->ReadData(&entry->ally);
        this->AddChild(radio);

        radio = new VisRadioType1(i + count * 2, 0, 0, 0, 0, _font, _clr1, nullptr);
        this->see_radios->ElementAt(i) = radio;
        radio->AddEntry(" ");
        radio->ReadData(&entry->see);
        this->AddChild(radio);

        radio = new VisRadioType1(i + count * 3, 0, 0, 0, 0, _font, _clr1, nullptr);
        this->mute_radios->ElementAt(i) = radio;
        radio->AddEntry(" ");
        radio->ReadData(&entry->mute);
        this->AddChild(radio);
    }

    this->UpdateRadioPositions(0);
}


// 4DD1B2
void VisListBoxDiplomacy::WriteRadioState()
{
    this->diplomacy->SetSize(this->enemy_radios->GetSize(), -1);
    for (int32_t i = 0; i < this->enemy_radios->GetSize(); i++) {
        DiplomacyEntry* entry = this->diplomacy->ElementAt(i);
        this->enemy_radios->ElementAt(i)->WriteData(&entry->enemy);
        this->ally_radios->ElementAt(i)->WriteData(&entry->ally);
        this->see_radios->ElementAt(i)->WriteData(&entry->see);
        this->mute_radios->ElementAt(i)->WriteData(&entry->mute);
    }
}


// 4DD2C7
void VisListBoxDiplomacy::UpdateScrollBar()
{
    if (this->parent == nullptr) {
        return;
    }

    this->num_vis_entry = this->rect.Height() / this->entry_height_full;
    int32_t content_height = this->entry_height_full * this->diplomacy->GetSize();
    int32_t height = this->rect.Height();
    if (content_height <= height) {
        return;
    }

    if (this->scrollbox_id < 0) {
        this->rect.right -= 0x1A;
        this->scrollbox_id = this->parent->childs.GetSize() + 1;
        VisScrollBar* scrollbar = new VisScrollBar(this->scrollbox_id, this->rect.right, this->rect.top, this->rect.right + 0x18, this->rect.bottom, nullptr);
        this->parent->AddChild(scrollbar);
    } else {
        VisScrollBar* scrollbar = (VisScrollBar*)this->FindChild(this->scrollbox_id);
        scrollbar->SetPos(this->selected_index, this->diplomacy->GetSize());
    }
}


// 4DD424
void VisListBoxDiplomacy::RestoreRect()
{
    if (this->scrollbox_id >= 0) {
        this->rect.right += 0x1A;
    }
}


// 44402B
VisDiplomacy::VisDiplomacy(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CArray<DiplomacyEntry*>* _payload)
: VisMessageBox(_id, l, t, r, b, *(CArray<DiplomacyEntry*>**)_payload, txt_dialogs.GetLine(0x4F), 1, txt_dialogs.GetLine(0x91))
{
    this->diplomacy = (CArray<DiplomacyEntry*>**)_payload;
}


// 44F9E0
VisDiplomacy::~VisDiplomacy()
{
}


// 445D34
void VisDiplomacy::ReadData(const void* buf)
{
    this->diplomacy = (CArray<DiplomacyEntry*>**)buf;

    VisListBoxDiplomacy* old_listbox = (VisListBoxDiplomacy*)this->FindChild(2);
    old_listbox->RestoreRect();
    CRect rc = old_listbox->GetRect();
    this->DestroyChild(old_listbox);

    VisListBoxDiplomacy* listbox = new VisListBoxDiplomacy(2, rc, *this->diplomacy, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0);

    this->cursor_over_obj_last = nullptr;
    this->cursor_over_obj = nullptr;
    this->last_focus_obj = nullptr;
    this->focus_obj = nullptr;

    this->AddChild(listbox);
    listbox->UpdateScrollBar();
    listbox->SetCaptionLabel((VisLabel*)this->FindChild(-1));
}


// 445C2E
CVisualObject* VisDiplomacy::VMethod30(const void* data, const RECT& r)
{
    ((RECT&)r).top = 0x50;

    VisListBoxDiplomacy* listbox = new VisListBoxDiplomacy(2, r, (CArray<DiplomacyEntry*>*)data, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0);
    this->AddChild(listbox);
    listbox->UpdateScrollBar();

    listbox->SetCaptionLabel((VisLabel*)this->FindChild(-1));
    return listbox;
}


// 445CF1
void VisDiplomacy::VMethod31(int32_t code)
{
    if (code == 0x445) {
        ((VisListBoxDiplomacy*)this->FindChild(2))->WriteRadioState();
    }
}


QuestObjectivesHeaderDialogVisualObject::QuestObjectivesHeaderDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisMessageBoxWithList(_id, l, t, r, b, g_MissionBriefing, txt_dialogs.GetLine(68), 0xffff)
{} //445173

void QuestObjectivesHeaderDialogVisualObject::VMethod26()
{ //4451e1
    VisMessageBox::VMethod26();

    CStringArray sarr;

    CRect r(0, 0, rect.Width() - 128, 480);
    int32_t pos = FindChild(2)->GetRect().bottom + 16;

    for (int i = 0; i < g_MissionSubjs.GetSize(); i++)
    {
        sarr.Copy(g_font1->StringArrayForRect(r, g_MissionSubjs[i]));

        CRect opos(48, pos - 3, 72, pos + 21);
        uint8_t bits = ScenarioGetVar(752 + i);
        if (bits)
        {
            uint16_t* colorsh = nullptr;
            CGameBitmap* bitmap = nullptr;
            VisBitmap* vis_bitmap = nullptr;

            if (bits & 2)
            {
                colorsh = clrsh_ShockingBlack;
                bitmap = new CSprite256("graphics\\interface\\subobj.256");
                bitmap->ResetPalette(1, 1, 0);
                vis_bitmap = new VisBitmap(pos + 1, r, bitmap, 11);
            }
            else if (bits & 4)
            {
                colorsh = clrsh_CoralRed;
                bitmap = new CSprite256("graphics\\interface\\subobj.256");
                bitmap->ResetPalette(1, 1, 0);
                vis_bitmap = new VisBitmap(pos + 1, r, bitmap, 12);
            }
            else if (bits & 1)
            {
                colorsh = clrsh_TechBlack;
                bitmap = new CSprite256("graphics\\interface\\subobj.256");
                bitmap->ResetPalette(1, 1, 0);
                vis_bitmap = new VisBitmap(pos + 1, r, bitmap, 10);
            }

            AddChild(new VisMultiText(pos, 80, pos, rect.Width() - 48, pos + sarr.GetSize() * (g_font1->GetHeight() + 4), g_MissionSubjs[i], g_font1, colorsh, 0));
            if (vis_bitmap)
                AddChild(vis_bitmap);

            int32_t dy = 10 + sarr.GetSize() * (g_font1->GetHeight() + 4);
            pos += dy;
            rect.bottom += dy;
        }
    }

    rect.bottom += 40;
    VisWindow::UpdateWinRect();

    int32_t rh = rect.Height();
    int32_t rw = rect.Width();

    CRect cr(rw / 2 - 48, rh - 60, rw / 2 + 48, rh - 36);
    AddChild(new VisButton(4, cr, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "")); //accept
}


// 4E2F76
void VisQuestStatus::VMethod7()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    QuestMap* quest_map = main_wnd->vis_map_context->field_0x4970;

    this->rect.bottom = this->rect.top + 0x48 + quest_map->FUN_0041ec00() * 0x20;

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    screen_rect.right -= 8;
    screen_rect.bottom -= 8;

    LockSurface2();

    gfx_interface_lm->VMethod3(screen_rect.right - 0x18, screen_rect.top + 8, 0xC, 6, 0);
    gfx_interface_lm->VMethod3(screen_rect.left + 8, screen_rect.bottom - 0x18, 0xF, 6, 0);
    gfx_interface_lm->VMethod3(screen_rect.right - 0x18, screen_rect.bottom - 0x18, 0x11, 6, 0);
    for (int32_t i = 0; i < (screen_rect.Width() - 0x40) / 0x30; i++) {
        gfx_interface_lm->VMethod3(screen_rect.left + 0x28 + i * 0x30, screen_rect.bottom - 0x18, 0x10, 6, 0);
    }
    for (int32_t i = 0; i < (screen_rect.Height() - 0x40) / 0x20; i++) {
        gfx_interface_lm->VMethod3(screen_rect.right - 0x18, screen_rect.top + 0x28 + i * 0x20, 0xE, 6, 0);
    }

    gfx_interface_lm->VMethod2(screen_rect.left, screen_rect.top, 0xA, 0, 0);
    gfx_interface_lm->VMethod2(screen_rect.right - 0x20, screen_rect.top, 0xC, 0, 0);
    gfx_interface_lm->VMethod2(screen_rect.left, screen_rect.bottom - 0x20, 0xF, 0, 0);
    gfx_interface_lm->VMethod2(screen_rect.right - 0x20, screen_rect.bottom - 0x20, 0x11, 0, 0);
    for (int32_t i = 0; i < (screen_rect.Width() - 0x40) / 0x30; i++) {
        gfx_interface_lm->VMethod2(screen_rect.left + 0x20 + i * 0x30, screen_rect.top, 0xB, 0, 0);
        gfx_interface_lm->VMethod2(screen_rect.left + 0x20 + i * 0x30, screen_rect.bottom - 0x20, 0x10, 0, 0);
    }
    for (int32_t i = 0; i < (screen_rect.Height() - 0x40) / 0x20; i++) {
        gfx_interface_lm->VMethod2(screen_rect.left, screen_rect.top + 0x20 + i * 0x20, 0xD, 0, 0);
        gfx_interface_lm->VMethod2(screen_rect.right - 0x20, screen_rect.top + 0x20 + i * 0x20, 0xE, 0, 0);
    }
    for (int32_t i = 0; i < (screen_rect.Width() - 0x40) / 0x30; i++) {
        for (int32_t j = 0; j < (screen_rect.Height() - 0x40) / 0x20; j++) {
            gfx_interface_lm->VMethod2(screen_rect.left + 0x20 + i * 0x30, screen_rect.top + 0x20 + j * 0x20, 9, 0, 0);
        }
    }

    int32_t col_x = this->rect.left + 0x20;
    int32_t row_y = this->rect.top + 0x28;

    if (quest_map->FUN_0041ec00() != 0) {
        g_font2->DrawTextWithShadow(this->rect.left + this->rect.Width() / 2, row_y - 0xE, TxtFile::AllLines[0x156], 2, clrsh_TechBlack, 1);
    } else {
        g_font2->DrawTextWithShadow(this->rect.left + this->rect.Width() / 2, row_y - 0xE, TxtFile::AllLines[0x157], 2, clrsh_TechBlack, 1);
    }

    POSITION quest_it = quest_map->quests_map.GetStartPosition();
    uint32_t quest_key;
    Quest* quest;
    while (quest_it != nullptr) {
        quest_map->quests_map.GetNextAssoc(quest_it, quest_key, quest);

        bool is_shown = (
            main_wnd->vis_map_context->quest_some_id_2 == quest->GetSomeId() ||
            main_wnd->vis_map_context->quest_landmark_some_id == quest->GetSomeId() ||
            main_wnd->vis_map_context->quest_building_some_id == quest->GetSomeId() ||
            main_wnd->vis_map_context->quest_some_id == quest->GetSomeId()
        );

        if (is_shown) {
            sub_457C5D(col_x + 0x16, row_y - 2, col_x + this->rect.Width() - 0x46, row_y + 0x18, 4);
        }

        CString unit_name;
        switch (quest->Kind()) {
        case 1:
        case 4:
        case 0xB: {
            CGameObject* obj;
            if (main_wnd->vis_map_context->field_0x9d0.Lookup((uint16_t)quest->GetObj(), obj)) {
                CUnit* unit = (CUnit*)obj;
                if ((unit->unitFlags & 0x11) == 0) {
                    if (unit->typeId < 0x52 || unit->typeId > 0x66) {
                        unit_name.Format("%s[%d]", txt_unitname.GetLine(unit->typeId), unit->face);
                    } else {
                        unit_name.Format("%s", txt_unitname.GetLine(unit->typeId));
                    }
                } else {
                    unit_name.Format("%s", txt_npcnames.GetLine(unit->serverId - 1));
                }
            }
            break;
        }
        case 2: {
            if ((quest->GetObj() & 0xFF) < 0x52 || (quest->GetObj() & 0xFF) > 0x66) {
                unit_name.Format("%s[%d]", txt_unitname.GetLine(quest->GetObj() & 0xFF), quest->GetObj() >> 8);
            } else {
                unit_name.Format("%s", txt_unitname.GetLine(quest->GetObj() & 0xFF));
            }
            break;
        }
        default:
            break;
        }

        CString area_name;
        CString building_name;
        CGameObject* landmark;
        if (main_wnd->vis_map_context->field_0x9d0.Lookup((uint16_t)quest->GetLandmarkId(), landmark)) {
            building_name = txt_building.GetLine(landmark->typeId - 1);
            int32_t cell_x = ((landmark->tileX - 8) * 5) / (main_wnd->vis_map_context->field_0x84 - 0x10);
            int32_t cell_y = ((landmark->tileY - 8) * 5) / (main_wnd->vis_map_context->field_0x88 - 0x10);
            area_name = TxtFile::AllLines[cell_x + 0x13D + cell_y * 5];
        }

        CString quest_text;
        switch (quest->Kind()) {
        case 1:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], (const char*)unit_name, (const char*)area_name, (const char*)building_name);
            break;
        case 2:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->FUN_004a4780(), (const char*)unit_name);
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
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->FUN_004a4780() / 0x3C0, (quest->FUN_004a4780() >> 4) % 0x3C, (const char*)area_name);
            break;
        case 8:
        case 9:
        case 0xA:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->FUN_004a4780());
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

        CRect text_rect(CPoint(col_x + 0x18, row_y), CSize(this->rect.Width() - 0x60, 0x20));

        uint16_t* clr = clrsh_TechBlack;
        int32_t icon_index = 0xA;
        if (quest->quest_data.state == 0) {
            clr = clrsh_TechBlack;
            icon_index = 0;
        } else if (quest->quest_data.state == 1) {
            clr = clrsh_ShockingBlack;
            icon_index = 1;
        } else if (quest->quest_data.state == 2) {
            clr = clrsh_CoralRed;
            icon_index = 2;
        }

        g_font2->DrawTextJustifyInRectShadow(text_rect, quest_text, clr, 0xA);
        this->icon->VMethod2(col_x - 6, row_y - 2, icon_index, 0, 0);

        row_y += 0x20;
    }

    UnlockSurface2();
}


// 4E4530
int32_t VisQuestStatus::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    return 0;
}


// 4E2E3D
VisQuestStatus::VisQuestStatus(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr)
{
    this->icon = new CSprite256("graphics\\interface\\subobj.256");
    this->icon->ResetPalette(1, 1, 0);
}


// 4E2EF4
VisQuestStatus::~VisQuestStatus()
{
    delete this->icon;
}


int32_t VisCharSellectButtons::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    //4303c8
    MainWindow* mainwnd = (MainWindow*)AfxGetMainWnd();

    if (mouse_down_box > -1 && mouse_down_box < 4 &&
        GetMouseOnBox(pos) == mouse_down_box)
    {
        int32_t box = mouse_down_box;
        mouse_down_box = -1;
        UpdateMouseOverBox(wparam, pos);

        MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

        switch (box)
        {
        case 0:
            if (parent_screen->roster_list->field_0xd8 == 0)
#ifdef A2CLIENT
            {
                int32_t idx = parent_screen->roster_list->field_0xd0;
                if (idx == parent_screen->session->GetStringArray1Size() - 1 &&
                    mwnd->field_0x3e0.field_10 != 0 && mwnd->hat_settings.ishat != 0 &&
                    parent_screen->session->field_0x130.GetSize() > 15)
                {
                    VisScreen* mbox = new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(141), nullptr, 0x2000);
                    mwnd->field_0x3dc = mbox;
                    mwnd->ModalScreen(mbox);
                }
                else if (idx == parent_screen->session->GetStringArray1Size() - 1 &&
                    mwnd->field_0x3e0.field_10 != 0 && mwnd->hat_settings.ishat != 0 && mwnd->hat_settings.deathmatch != 0)
                {
                    VisScreen* mbox = new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(142), nullptr, 0x2000);
                    mwnd->field_0x3dc = mbox;
                    mwnd->ModalScreen(mbox);
                }
                else
                    parent_screen->CloseOk();
            }
#else
                parent_screen->CloseOk();
#endif
            break;

        case 1:
            if (parent_screen->roster_list->field_0xd8 == 0)
            {
                int32_t idx = parent_screen->roster_list->field_0xd0;
                if (idx != parent_screen->session->GetStringArray1Size() - 1)
                {
                    CString str = CString(txt_patch.GetLine(93)) + CString(parent_screen->session->character_name) + CString(txt_patch.GetLine(94));
                    VisScreen* box = new VisMessageBoxWithList(1, 64, 100, 380, 594, str, nullptr, 4);
                    mwnd->field_0x3dc = box;
                    mwnd->ModalScreen(box);

                    if (box->GetCloseCode() == 0x447)
                    {
                        parent_screen->session->FUN_00493cd8();

                        if (idx < parent_screen->session->GetStringArray1Size() - 1)
                        {
                            parent_screen->session->LoadCharacterRosterEntry(idx);
                            parent_screen->vis_stats->ReadUnitStats();
                            parent_screen->FUN_00432655(parent_screen->selected_unit);
                        }
                        else
                        {
                            parent_screen->roster_list->field_0xd0 = parent_screen->session->GetStringArray1Size() - 1;
                            parent_screen->selected_unit->VMethod1(0);
                            parent_screen->map_context->UpdateSelectionState();
                        }
                    }
                }
            }
            break;

        case 2:
            if (parent_screen->roster_list->field_0xd8 == 0)
            {
                parent_screen->OpenRenameWindow();
                if (parent_screen->snd_rename)
                    parent_screen->snd_rename->Play();
            }
            break;

        case 3:
            if (parent_screen->roster_list->field_0xd8 == 0)
                parent_screen->CloseCancel();
            break;
            
        default:
            break;
        }
    }
    return 1;
}

int VisCharSellectButtons::GetMouseOnBox(CPoint pos)
{
    pos -= parent_screen->rect.TopLeft();

    for (int i = 0; i < 4; i++)
    {
        if (areas[i].PtInRect(pos))
            return i;
    }
    return -1;
}

void VisCharSellectButtons::UpdateMouseOverBox(uint32_t wparam, CPoint pos)
{
    int over = GetMouseOnBox(pos);
    if (over >= 0 && (wparam & 1) == 0)
        mouse_over_box = over;
    else if (over >= 0 && mouse_down_box == over && (wparam & 1) != 0)
        mouse_over_box = over;
    else
        mouse_over_box = -1;
}


// 43005F
void VisCharSellectButtons::VMethod7()
{
    CPoint top_left = this->parent_screen->rect.TopLeft();
    if (this->parent_screen->active_flag != 0) {
        LockSurface2();
        this->bmp_area->VMethod2(top_left.x + this->rect.left, top_left.y + this->rect.top, 0, 0, 0);
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
                                 this->field_0x60[i], 10, pal);
            } else {
                this->buttons_bmp[i]->VMethod10(top_left.x + this->areas[i].left, top_left.y + this->areas[i].top,
                                                0, 0, this->areas[i].Width(), this->areas[i].Height());
                g_font4->DrawTxt(top_left.x + this->areas[i].left + this->areas[i].Width() / 2,
                                 top_left.y + this->areas[i].top + 1 + this->areas[i].Height() / 2,
                                 this->field_0x60[i], 10, pal);
            }
        }
        UnlockSurface2();
    }
}


// 430a1a
void VisCharSellectButtons::FreeBitmaps()
{
    for (int32_t i = 0; i < 4; i++) {
        if (this->buttons_bmp[i] != nullptr) {
            delete this->buttons_bmp[i];
        }
        this->buttons_bmp[i] = nullptr;
        // Never assigned anywhere; kept faithful — dispatch is virtual so the
        // static type only satisfies the compiler.
        if (this->field_0x84[i] != nullptr) {
            delete static_cast<CBmp64*>(this->field_0x84[i]);
        }
        this->field_0x84[i] = nullptr;
    }
    if (this->bmp_area != nullptr) {
        delete this->bmp_area;
    }
    this->bmp_area = nullptr;
}


// 430850
void VisCharSellectButtons::LoadBitmaps()
{
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
int32_t VisCharSellectButtons::OnLButtonDown(uint32_t wparam, CPoint pos)
{
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
int32_t VisCharSellectButtons::OnMouseMove(uint32_t wparam, CPoint pos)
{
    this->UpdateMouseOverBox(wparam, pos);
    return 0;
}


// 43072B
void VisCharSellectButtons::ResetMouseBoxes()
{
    this->mouse_down_box = -1;
    this->mouse_over_box = -1;
}


// 42FE62
void VisCharSellectButtons::Init()
{
    this->mouse_down_box = -1;
    this->mouse_over_box = -1;
    this->areas[0] = CRect(0x1EE, 0xF, 0x266, 0x43);
    this->areas[1] = CRect(0x1E3, 0x43, 0x26F, 0x71);
    this->areas[2] = CRect(0x1E3, 0x72, 0x26F, 0xA0);
    this->areas[3] = CRect(0x1EE, 0xA0, 0x266, 0xD4);
    for (int32_t i = 0; i < 4; i++) {
        this->buttons_bmp[i] = nullptr;
        this->field_0x84[i] = nullptr;
    }
    this->bmp_area = nullptr;
    this->field_0x60.SetSize(4, -1);
    this->field_0x60[0] = TxtFile::AllLines[0xEE];
    this->field_0x60[1] = TxtFile::AllLines[0xF0];
    this->field_0x60[2] = TxtFile::AllLines[0xF1];
    this->field_0x60[3] = TxtFile::AllLines[0x4E];
    this->flags |= FLAG_NOTFOCUS;
}


// 42FD5C
VisCharSellectButtons::VisCharSellectButtons(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    this->parent_screen = parent_screen;
    this->Init();
}


// 42FDF8 (scalar deleting dtor ??_G at 4385B0)
VisCharSellectButtons::~VisCharSellectButtons()
{
    this->FreeBitmaps();
    this->parent_screen = nullptr;
}


// 430FEC
void VisCharSellectList::VMethod7()
{
    if (this->parent_screen->active_flag != 0) {
        CPoint top_left = this->parent_screen->rect.TopLeft();
        int32_t visible_end = this->field_0xa0 + this->field_0x70.Height() / this->field_0x60.Height();
        if (visible_end > this->parent_screen->session->GetStringArray1Size()) {
            visible_end = this->parent_screen->session->GetStringArray1Size();
        }
        LockSurface2();
        this->field_0xa4->VMethod2(top_left.x + this->rect.left, top_left.y + this->rect.top, 0, 0, 0);
        if (this->field_0xb8 != nullptr) {
            this->field_0xb8->VMethod2(top_left.x + 0x78 + this->rect.left, top_left.y + this->rect.top, 0, 0, 0);
        }
        if (this->field_0xbc != nullptr) {
            this->field_0xbc->VMethod2(top_left.x + 0x78 + this->rect.left, top_left.y + 0x188 + this->rect.top, 0, 0, 0);
        }
        for (int32_t i = this->field_0xa0; i < visible_end; i++) {
            uint16_t* pal;
            if (i == this->field_0xd4) {
                pal = palette_paris_daisy->GetPalette(0);
            } else {
                pal = palette_husk->GetPalette(0);
            }
            if (i == this->field_0xd0) {
                CRect rc(top_left.x + this->field_0x70.left,
                         top_left.y + this->field_0x70.top + this->field_0x60.Height() * ((i - this->field_0xa0) + 1) - 4,
                         top_left.x + this->field_0x70.right,
                         top_left.y + this->field_0x70.top + this->field_0x60.Height() * ((i - this->field_0xa0) + 2));
                ShadowRect(rc, 8);
            }
            if (this->field_0xd0 != i || this->field_0xd8 == 0) {
                g_font4->DrawTextWithShadow(top_left.x + this->field_0x70.left + this->field_0x70.Width() / 2,
                                            top_left.y + this->field_0x70.top + this->field_0x60.Height() * ((i - this->field_0xa0) + 1),
                                            this->parent_screen->session->characterRosterNames[i], 2, pal, 1);
            }
        }
        UnlockSurface2();
        this->CVisualObject::VMethod7();
    }
}


// 43148A
void VisCharSellectList::FreeBitmaps()
{
    if (this->field_0xa4 != nullptr) {
        delete this->field_0xa4;
    }
    this->field_0xa4 = nullptr;
    if (this->field_0xa8 != nullptr) {
        delete this->field_0xa8;
    }
    this->field_0xa8 = nullptr;
    if (this->field_0xac != nullptr) {
        delete this->field_0xac;
    }
    this->field_0xac = nullptr;
    if (this->field_0xb0 != nullptr) {
        delete this->field_0xb0;
    }
    this->field_0xb0 = nullptr;
    if (this->field_0xb4 != nullptr) {
        delete this->field_0xb4;
    }
    this->field_0xb4 = nullptr;
}


// 4312B7
void VisCharSellectList::LoadBitmaps()
{
    this->FreeBitmaps();
    this->field_0xa4 = new CBmp64("graphics\\interface\\chrgen\\loader\\CenterArea.bmp");
    g_mousept.Update();
    this->field_0xa8 = new CBmp64("graphics\\interface\\chrgen\\loader\\up\\shine.bmp");
    g_mousept.Update();
    this->field_0xac = new CBmp64("graphics\\interface\\chrgen\\loader\\up\\shine_on.bmp");
    g_mousept.Update();
    this->field_0xb0 = new CBmp64("graphics\\interface\\chrgen\\loader\\down\\shine.bmp");
    g_mousept.Update();
    this->field_0xb4 = new CBmp64("graphics\\interface\\chrgen\\loader\\down\\shine_on.bmp");
    g_mousept.Update();
}


// 4316C4
void VisCharSellectList::SelectRow(int32_t row)
{
    if (row != -1) {
        if (row == this->parent_screen->session->GetStringArray1Size() - 1) {
            this->parent_screen->selected_unit->VMethod1(0);
            this->parent_screen->map_context->UpdateSelectionState();
        } else {
            this->parent_screen->session->LoadCharacterRosterEntry(row);
            this->parent_screen->vis_stats->ReadUnitStats();
            this->parent_screen->FUN_00432655(this->parent_screen->selected_unit);
        }
        this->field_0xd0 = row;
    }
}


// 431872
int32_t VisCharSellectList::HitTest(CPoint pos)
{
    int32_t visible_end = this->field_0xa0 + this->field_0x70.Height() / this->field_0x60.Height();
    CPoint top_left = this->parent_screen->rect.TopLeft();
    if (visible_end > this->parent_screen->session->GetStringArray1Size()) {
        visible_end = this->parent_screen->session->GetStringArray1Size();
    }
    for (int32_t i = this->field_0xa0; i < visible_end; i++) {
        CPoint origin(top_left.x + this->field_0x70.left,
                      top_left.y + this->field_0x70.top + this->field_0x60.Height() * (i - this->field_0xa0 + 1));
        CRect row_rect(origin, this->field_0x60.Size());
        if (row_rect.PtInRect(pos)) {
            return i;
        }
    }
    return -1;
}


// 431A05
int32_t VisCharSellectList::SelectPrevRow()
{
    if (this->field_0xd0 - 1 < 0) {
        return 0;
    }
    if (this->field_0xd0 - 1 < this->field_0xa0) {
        this->field_0xa0 = this->field_0xa0 - 1;
        this->field_0xd0 = this->field_0xd0 - 1;
    } else {
        this->field_0xd0 = this->field_0xd0 - 1;
    }
    this->SelectRow(this->field_0xd0);
    return 1;
}


// 431A93
int32_t VisCharSellectList::SelectNextRow()
{
    int32_t visible_end = this->field_0xa0 + this->field_0x70.Height() / this->field_0x60.Height();
    if (visible_end > this->parent_screen->session->GetStringArray1Size()) {
        visible_end = this->parent_screen->session->GetStringArray1Size();
    }
    if (this->field_0xd0 + 1 >= this->parent_screen->session->GetStringArray1Size()) {
        return 0;
    }
    if (this->field_0xd0 + 1 > visible_end - 1) {
        this->field_0xa0 = this->field_0xa0 + 1;
        this->field_0xd0 = this->field_0xd0 + 1;
    } else {
        this->field_0xd0 = this->field_0xd0 + 1;
    }
    this->SelectRow(this->field_0xd0);
    return 1;
}


// 431C97
void VisCharSellectList::UpdateArrows(CPoint pos, bool is_down)
{
    pos -= this->parent_screen->rect.TopLeft();
    if (is_down) {
        if (this->field_0xd0 - 1 < 0 || !this->field_0x80.PtInRect(pos)) {
            this->field_0xb8 = nullptr;
        } else {
            this->field_0xb8 = this->field_0xac;
        }
        if (this->field_0xd0 + 1 < this->parent_screen->session->GetStringArray1Size()
            && this->field_0x90.PtInRect(pos)) {
            this->field_0xbc = this->field_0xb4;
        } else {
            this->field_0xbc = nullptr;
        }
    } else {
        if (this->field_0xd0 - 1 < 0 || !this->field_0x80.PtInRect(pos)) {
            this->field_0xb8 = nullptr;
        } else {
            this->field_0xb8 = this->field_0xa8;
        }
        if (this->field_0xd0 + 1 < this->parent_screen->session->GetStringArray1Size()
            && this->field_0x90.PtInRect(pos)) {
            this->field_0xbc = this->field_0xb0;
        } else {
            this->field_0xbc = nullptr;
        }
    }
}


// 431609
int32_t VisCharSellectList::OnMouseMove(uint32_t wparam, CPoint pos)
{
    if (this->field_0xd8 != 0) {
        return this->CVisualObject::OnMouseMove(wparam, pos);
    }
    this->field_0xd4 = this->HitTest(pos);
    if (this->field_0xd4 == -1) {
        g_Cursors[0]->Use();
    } else {
        g_Cursors[5]->Use();
    }
    this->UpdateArrows(pos, (wparam & 1) != 0);
    return this->CVisualObject::OnMouseMove(wparam, pos);
}


// 431764
int32_t VisCharSellectList::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    if (this->field_0xd8 != 0) {
        return this->CVisualObject::OnLButtonDown(wparam, pos);
    }
    int32_t row = this->HitTest(pos);
    this->SelectRow(row);
    if (this->field_0x90.PtInRect(pos)) {
        this->SelectNextRow();
    } else if (this->field_0x80.PtInRect(pos)) {
        this->SelectPrevRow();
    }
    this->UpdateArrows(pos, true);
    return 1;
}


// 43182C
int32_t VisCharSellectList::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    if (this->field_0xd8 != 0) {
        return this->CVisualObject::OnLButtonUp(wparam, pos);
    }
    this->UpdateArrows(pos, false);
    return 1;
}


// 4316A0
int32_t VisCharSellectList::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    return this->OnLButtonDown(wparam, pos);
}


// 431B95
int32_t VisCharSellectList::OnKeyDown(uint32_t wparam)
{
    switch (wparam) {
    case 0x26:
        this->SelectPrevRow();
        return 1;
    case 0x28:
        this->SelectNextRow();
        return 1;
    case 0x0D:
        if (this->field_0xd8 != 0) {
            char buf[100];
            this->parent_screen->rename_txt->WriteData(buf);
            this->parent_screen->session->FUN_0049381c(buf);
            this->parent_screen->CloseRenameWindow();
        } else {
            this->parent_screen->CloseOk();
        }
        return 1;
    case 0x1B:
        if (this->field_0xd8 != 0) {
            this->parent_screen->CloseRenameWindow();
        }
        return 0;
    default:
        return 0;
    }
}


// 430FAD
void VisCharSellectList::FUN_00430fad()
{
    this->field_0xa0 = 0;
    this->field_0xd0 = 0;
    this->field_0xd4 = -1;
    this->field_0xd8 = 0;
}


// 430CB3
void VisCharSellectList::Init()
{
    this->field_0x60 = CRect(CPoint(0x38, 0x38), CSize(0xCE, g_font4->GetHeight()));
    this->field_0x70 = CRect(CPoint(this->rect.left + 0x38, this->rect.top + 0x38), CSize(0xCE, 0xEC));
    this->field_0xa4 = nullptr;
    this->field_0xa8 = nullptr;
    this->field_0xac = nullptr;
    this->field_0xb0 = nullptr;
    this->field_0xb4 = nullptr;
    this->field_0xbc = nullptr;
    this->field_0xb8 = nullptr;
    CPoint top_left = this->rect.TopLeft();
    this->field_0x80 = CRect(CPoint(0x78, 0), CSize(0x50, 0x3C)) + top_left;
    this->field_0x90 = CRect(CPoint(0x78, 0x188), CSize(0x50, 0x34)) + top_left;
    this->FUN_00430fad();
}


// 430BBB
VisCharSellectList::VisCharSellectList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    this->parent_screen = parent_screen;
    this->Init();
}


// 430C62 (scalar deleting dtor ??_G at 4385E0)
VisCharSellectList::~VisCharSellectList()
{
    this->FreeBitmaps();
}


// 44D8F0
int32_t VisNetMapSelection::OnKeyDown(uint32_t wparam)
{
    return this->VisWindow::OnKeyDown(wparam);
}


// 44AF45
VisNetMapSelection::VisNetMapSelection(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CString* pMapName)
: VisWindow(_id, l, t, r, b, nullptr), thread(VisNetMapThreadProc, this)
{
    this->p_mapname = pMapName;
}


// 44AFD8 (scalar deleting dtor ??_G at 44FEA0)
VisNetMapSelection::~VisNetMapSelection()
{
    for (int32_t i = 0; i < this->avail_maps.GetSize(); i++) {
        delete this->avail_maps.GetAt(i);
    }
}


// 44AEEF
void VisNetMapSelection::StopThread()
{
    SetEvent(this->stop_event);
    if (this->thread.m_hThread != nullptr) {
        WaitForSingleObject(this->thread.m_hThread, INFINITE);
    }
    CloseHandle(this->stop_event);
    this->stop_event = nullptr;
}


// 44D13D
void VisNetMapSelection::AddNetMapInfo(NetMapInfo* info)
{
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
void VisNetMapSelection::VMethod26()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->mode = (main_wnd->sessionMode != 0);

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
    this->AddChild(new VisScrollBar(0xA, list_rc.right, list_rc.top, list_rc.right + 0x10, list_rc.bottom, nullptr));

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
    this->AddChild(new VisScrollBar(0xB, list_rc.right, list_rc.top, list_rc.right + 0x10, list_rc.bottom, nullptr));

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
    this->AddChild(new VisScrollBar(0xC, list_rc.right, list_rc.top, list_rc.right + 0x10, list_rc.bottom, nullptr));

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
int32_t VisNetMapSelection::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    switch (msg) {
    case 0x444:
        if (wparam != 1) {
            return 0;
        }
        // fallthrough
    case 0x445:
        if (this->FindChild(0x14)->TestFlags(1)) {
            this->StopThread();
            this->VisScreen::MsgProc(0x445, 0, 0);
            VisListBox* map_list = (VisListBox*)this->FindChild(1);
            NetMapInfo* info = this->avail_maps.GetAt(map_list->GetSelectedIndex());
            *this->p_mapname = info->mapid;
        }
        return 1;
    case 0x446:
        this->StopThread();
        this->VisScreen::MsgProc(0x446, 0, 0);
        return 1;
    case 0x45E:
        if (main_wnd->sessionMode == 0) {
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
                bool can_join = (level + 1 >= main_wnd->m_GameSession.FUN_004200f0())
                    && (main_wnd->m_GameSession.FUN_00420110() >= level + 1);
                this->VisScreen::MsgProc(can_join ? 0x445 : 0x446, 0, 0);
            } else {
                this->VisScreen::MsgProc(0x445, 0, 0);
            }
        }
        return 1;
    case 0x45F: {
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
    case 0x460: {
        BigStruct2* map_context = main_wnd->vis_map_context;
        VisNetPlayerList* player_list = (VisNetPlayerList*)this->FindChild(5);
        player_list->entries.RemoveAll();
        player_list->selected_index = -1;
        player_list->colors.RemoveAll();
        for (int32_t i = 0x10; i < map_context->field_0x9b8.GetSize(); i++) {
            MapPlayerData* pd = map_context->field_0x9b8.GetAt(i);
            if (pd != nullptr) {
                player_list->AddItem(pd->name);
                player_list->colors.Add(g_colors_human_pals + pd->color);
            }
        }
        player_list->VMethod9();
        return 1;
    }
    case 0x461:
        this->AddNetMapInfo((NetMapInfo*)wparam);
        return 1;
    case 0x46E:
    case 0x473:
        if (wparam == 1) {
            if (lparam >= 0 && this->mode != 0) {
                this->selected_map_index = lparam;
                VisListBox* map_list = (VisListBox*)this->FindChild(1);
                map_list->VMethod9();
                NetMapInfo* info = this->avail_maps.GetAt(this->selected_map_index);
                this->map_context->FUN_0041cda3((LPCTSTR)info->name);
                CString row = map_list->GetItem(lparam);
                int32_t level = row[row.GetLength() - 1] - '1';
                if (level < 0) {
                    level = 0;
                }
                if (level > 3) {
                    level = 3;
                }
                bool can_join = (level + 1 >= main_wnd->m_GameSession.FUN_004200f0())
                    && (main_wnd->m_GameSession.FUN_00420110() >= level + 1);
                this->FindChild(0x14)->ChangeFlags(1, can_join);
                this->FindChild(0x14)->VMethod9();
            }
            return 1;
        }
        return 0;
    case 0x484:
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


// 42F61A
void VisCharSellectStats::FreeBitmaps()
{
    if (this->field_0x60 != nullptr) {
        delete this->field_0x60;
    }
    this->field_0x60 = nullptr;
    if (this->field_0x64 != nullptr) {
        delete this->field_0x64;
    }
    this->field_0x64 = nullptr;
    if (this->field_0x8c != nullptr) {
        delete this->field_0x8c;
    }
    this->field_0x8c = nullptr;
}


// 42F4DF
void VisCharSellectStats::LoadBitmaps()
{
    this->FreeBitmaps();
    this->field_0x60 = new CBmp64("graphics\\Interface\\chrgen\\FullStatsL.bmp");
    g_mousept.Update();
    this->field_0x64 = new CBmp64("graphics\\interface\\chrgen\\loader\\LeftUp.bmp");
    g_mousept.Update();
    this->field_0x8c = new CA16("graphics\\interface\\chrgen\\cube\\sprites.16a");
    this->field_0x8c->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
}


// 42F6F3
void VisCharSellectStats::ReadUnitStats()
{
    if (this->parent_screen->selected_unit == nullptr) {
        this->field_0x7c = 0;
        this->field_0x80 = 0;
        this->field_0x84 = 0;
        this->field_0x88 = 0;
    } else {
        this->field_0x7c = this->parent_screen->selected_unit->body;
        this->field_0x80 = this->parent_screen->selected_unit->reaction;
        this->field_0x84 = this->parent_screen->selected_unit->mind;
        this->field_0x88 = this->parent_screen->selected_unit->spirit;
    }
}


// 42F7B9
void VisCharSellectStats::VMethod7()
{
    CPoint top_left = this->parent_screen->rect.TopLeft();
    if (this->parent_screen->active_flag != 0) {
        LockSurface2();
        this->field_0x64->VMethod2(top_left.x, top_left.y, 0, 0, 0);
        this->field_0x60->VMethod2(top_left.x, top_left.y + 0xEE, 0, 0, 0);
        if (this->parent_screen->roster_list->field_0xd0 == this->parent_screen->session->GetStringArray1Size() - 1) {
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
        this->parent_screen->selected_unit->FUN_0046c124(&rc);
        UnlockSurface2();
    }
}


// 42FC61
const char* VisCharSellectStats::GetHint()
{
    if (this->parent_screen->active_flag == 0) {
        return nullptr;
    }
    CRect rc = this->ClientRectToScreen(this->rect);
    CPoint pos(g_mousept.GetX() - rc.left - 0xC, g_mousept.GetY() - rc.top - 0xEE);
    return this->parent_screen->selected_unit->FUN_0046d0f7(pos.x, pos.y);
}


// 42F1E8
void VisCharSellectStats::Init()
{
    this->field_0x60 = nullptr;
    this->field_0x64 = nullptr;
    this->field_0x8c = nullptr;
    this->field_0x68.SetSize(4, -1);
    this->field_0x68[0] = TxtFile::AllLines[0xF];
    this->field_0x68[1] = TxtFile::AllLines[0x10];
    this->field_0x68[2] = TxtFile::AllLines[0x11];
    this->field_0x68[3] = TxtFile::AllLines[0x12];
    this->field_0x90[0] = CRect(CPoint(0x70, 0x31), CSize(0x20, 0x20));
    this->field_0x90[1] = CRect(CPoint(0x70, 0x52), CSize(0x20, 0x20));
    this->field_0x90[2] = CRect(CPoint(0x70, 0x73), CSize(0x20, 0x20));
    this->field_0x90[3] = CRect(CPoint(0x70, 0x94), CSize(0x20, 0x20));
    this->field_0x7c = 0x1E;
    this->field_0x80 = 0x1F;
    this->field_0x84 = 0x20;
    this->field_0x88 = 0x21;
}


// 42F0EC
VisCharSellectStats::VisCharSellectStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    this->parent_screen = parent_screen;
    this->Init();
}


// 42F188 (scalar deleting dtor ??_G at 438580)
VisCharSellectStats::~VisCharSellectStats()
{
    this->FreeBitmaps();
}


// 431FDF
void VisCharSelect::VMethod26()
{
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
    CRect rc = this->roster_list->field_0x60;
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
CUnit* VisCharSelect::GetSelectedMapUnit()
{
    return this->map_context->GetUnit_3f6c();
}


// 432280
void VisCharSelect::LoadSfx()
{
    this->FreeSfx();
    FUN_00438e40(&this->snd_ok, "SFX\\Click_Ok.wav");
    FUN_00438e40(&this->snd_rename, "SFX\\Rename.wav");
    FUN_00438e40(&this->snd_delete, "SFX\\Delete.wav");
    FUN_00438e40(&this->snd_cancel, "SFX\\Undo.wav");
}


// 4322ED
void VisCharSelect::FreeSfx()
{
    FUN_00438dd0(&this->snd_ok);
    FUN_00438dd0(&this->snd_rename);
    FUN_00438dd0(&this->snd_delete);
    FUN_00438dd0(&this->snd_cancel);
}


// 432933
void VisCharSelect::CloseRenameWindow()
{
    if (this->roster_list->field_0xd8 != 0) {
        this->roster_list->field_0xd8 = 0;
        this->roster_list->RemoveChild(this->rename_txt);
        this->roster_list->focus_obj = nullptr;
        this->roster_list->cursor_over_obj = nullptr;
    }
}


// 43233E
void VisCharSelect::VMethod28()
{
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
        this->FUN_00432655(this->selected_unit);
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
    this->roster_list->FUN_00430fad();
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
void VisCharSelect::DoClose(uint32_t code)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->VMethod9();
    if (this->roster_list->field_0xd0 == this->session->GetStringArray1Size() - 1 && code == 0x445) {
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
void VisCharSelect::OpenRenameWindow()
{
    if (this->session->characterRosterNames.GetSize() == this->roster_list->field_0xd0) {
        return;
    }
    int32_t row_height = this->roster_list->field_0x60.Height();
    int32_t sel = this->roster_list->field_0xd0;
    int32_t first_row = this->roster_list->field_0xa0;
    int32_t list_top = this->roster_list->field_0x70.top;
    CRect& rc = this->rename_txt->GetRect();
    rc.top = list_top + row_height * (sel + 1 - first_row) - 4;
    rc.bottom = list_top + row_height * (sel + 2 - first_row);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->rename_txt->ReadData(main_wnd->vis_map_context->field_0x3f6c->str2);
    this->roster_list->AddChild(this->rename_txt);
    this->FocusTo(this->rename_txt, 1);
    this->roster_list->field_0xd8 = 1;
}


// 4326EA
int32_t VisCharSelect::OnMouseMove(uint32_t wparam, CPoint pos)
{
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
int32_t VisCharSelect::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    if (msg == 0x402) {
        this->VMethod9();
        return 1;
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 432655
void VisCharSelect::FUN_00432655(CUnit* unit)
{
    if (unit != nullptr) {
        unit->VMethod1(1);
        unit->unitFlags |= 8;
    }
    this->map_context->field_0x138 = unit;
    this->map_context->UpdateSelectionState();
}


// 438D50
void VisCharSelect::VMethod8(CRect* rect)
{
    (void)rect;
}


// 431EAC
VisCharSelect::VisCharSelect(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr)
{
    this->VMethod26();
}


// 431F18
VisCharSelect::~VisCharSelect()
{
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




VisMenuWnd::~VisMenuWnd()
{ //451a90
}

VisMenuWnd::VisMenuWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, uint32_t unk, const CRect& _r)
: VisWindow(_id, l, t, r, b, _bitmap)
{ //451990
    field_0x6c = _r;
    field_0x68 = unk;
    field_0x6c.left = 40;
    field_0x6c.right = rect.Width() - 48;
}

int32_t VisMenuWnd::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{ //440e54
    if (msg == 0x47b)
    {
        if (wparam == 0)
            return VisScreen::MsgProc(0x446, 0, 0);

        int32_t res = VisScreen::MsgProc(0x445, 0, 0);
        AfxGetMainWnd()->PostMessage(wparam, 0, 0);
        return res;
    }

    return VisScreen::MsgProc(msg, wparam, lparam);
}

int32_t VisMenuWnd::OnKeyDown(uint32_t wparam)
{ //440ece
    if (wparam == VK_UP)
    {
        TabFocus(false, true);
        return 1;
    }
    else if (wparam == VK_DOWN)
    {
        TabFocus(true, true);
        return 1;
    }

    return VisWindow::OnKeyDown(wparam);
}


void VisMenuWnd::AddElement(CVisualObject* obj, int32_t height)
{ //440e11
    field_0x6c.top = field_0x6c.bottom;
    field_0x6c.bottom = field_0x6c.top + height;

    obj->SetRect(field_0x6c);
    AddChild(obj);
}


int32_t MenuButton::OnLButtonUp(uint32_t wparam, CPoint pos)
{ //4dd933
    CRect r = ClientRectToScreen(rect);

    if (downed == 0)
        return 0;

    SetDowned(false);

    if (r.PtInRect(pos) != 0)
        AfxGetMainWnd()->PostMessage(0x47b, msgid, 0);

    return 1;
}

int32_t MenuButton::OnChar(uint32_t wparam)
{ //4dd9b3
    if (!parent || TestFlags(FLAG_ENABLED) == 0)
        return 0;

    char ch = ToLowerChar(EncodeChar(wparam));

    if (ch != '\r' && ch != charid)
        return 0;

    AfxGetMainWnd()->PostMessage(0x47b, msgid, 0);
    return 1;
}

MenuButton::MenuButton(int32_t _id, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: VisButton(_id, 0, 0, 0, 0, _caption, _font, _clr, _msgid, _charid, hint)
{ //4509c0
}

MenuButton::MenuButton(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
: VisButton(_id, r, _caption, _font, _clr, _msgid, _charid, hint)
{ //450bf0
}


IngameMenu::IngameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, uint32_t unk, const CRect& _r)
: VisMenuWnd(_id, l, t, r, b, _bitmap, unk, _r)
{ //451990
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    //Save game
    MenuButton* btn = new MenuButton(1, txt_dialogs.GetLine(34), g_font1, 0, 0x41a, 'S', "");

    if (mwnd->sessionMode == 0 || mwnd->sessionMode == 1)
        btn->ChangeFlags(FLAG_ENABLED, false);

    AddElement(btn, 30);

    if (mwnd->sessionMode == 2)
    {
        //Load game
        btn = new MenuButton(2, txt_dialogs.GetLine(35), g_font1, 0, 0x418, 'L', "");

        if (AppHasAnySaveFile() == 0)
            btn->ChangeFlags(FLAG_ENABLED, false);
    }
    else
    {
        //Diplomacy
        btn = new MenuButton(7, txt_dialogs.GetLine(76), g_font1, 0, 0x43c, 'D', "");
    }

    AddElement(btn, 30);


    //options
    btn = new MenuButton(3, txt_dialogs.GetLine(36), g_font1, 0, 0x41b, 'O', "");
    AddElement(btn, 30);

    //sound options
    btn = new MenuButton(4, txt_dialogs.GetLine(37), g_font1, 0, 0x422, 'N', "");

    if (mwnd->music_player->GetState() == 5)
        btn->ChangeFlags(FLAG_ENABLED, false);

    AddElement(btn, 30);


    //objectives
    btn = new MenuButton(5, txt_dialogs.GetLine(38), g_font1, 0, 0x420, 'M', "");

    AddElement(btn, 30);


    //end mission
    btn = new MenuButton(6, txt_dialogs.GetLine(39), g_font1, 0, 0x41c, 'E', "");

    AddElement(btn, 30);


    //resume
    btn = new MenuButton(8, txt_dialogs.GetLine(40), g_font1, 0, 0x446, 'R', "");

    AddElement(btn, 30);
}



int32_t LoadGameWindow::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{ //43f63c
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    switch (msg)
    {
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);

    case 0x444:
    case 0x479:
    {
        if (field_0x94 < 0)
            return VisScreen::MsgProc(0x446, 0, 0);

        CStringArray arr;
        ((VisListBox*)FindChild(3))->WriteData(&arr);

        strcpy(field_0x68->title, arr[field_0x94]);
        strcpy(field_0x68->filename, file_names[field_0x94]);

        return VisScreen::MsgProc(0x445, 0, 0);
    }

    case 0x46e:
        if (wparam == 3)
            field_0x94 = lparam;
        return 1;

    case 0x475:
    {
        VisListBox* lb = (VisListBox*)FindChild(3);
        int32_t idx = lb->GetSelectedIndex();
        if (idx > -1)
        {
            CString str = txt_patch.GetLine(50) + lb->GetItem(idx) + txt_patch.GetLine(51); // want to delete save ### ?
            mwnd->field_0x3dc = new VisMessageBoxWithList(1, 64, 100, 380, 594, str, nullptr, 4);
            mwnd->ModalScreen(mwnd->field_0x3dc);
            if (mwnd->field_0x3dc->GetCloseCode() == 0x447)
            {
                DeleteFileA(file_names[idx]);
                file_names.RemoveAt(idx);
                save_times.RemoveAt(idx);

                lb->RemoveItem(idx);
                lb->SelectItem(idx);

                MsgProc(0x46e, lb->GetId(), lb->GetSelectedIndex());

                if (file_names.GetSize() == 0)
                {
                    CVisualObject* obj = FindChild(4);
                    obj->ChangeFlags(FLAG_ENABLED, false);
                    obj->VMethod9();

                    obj = FindChild(6);
                    obj->ChangeFlags(FLAG_ENABLED, false);
                    obj->VMethod9();
                }

            }
        }
        return 1;
    }

    case 0x47a:
        VisScreen::MsgProc(0x446, 0, 0);
        return 1;
    }

    return 1;
}

void LoadGameWindow::VMethod26()
{ //43ed94

    AddChild(new VisLabel(1, 40, 32, rect.Width() - 40, 56, txt_dialogs.GetLine(151), g_font1, p_clrsh_Black, 2)); //restore saved game

    VisLabel* caption = new VisLabel(2, 40, 68, rect.Width() - 40, 92, txt_dialogs.GetLine(24), g_font1, p_clrsh_Black, 0);//saved games
    AddChild(caption); 

    HintedListBox* list = new HintedListBox(3, 40, 92, rect.Width() - 64, 284, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 10, txt_dialogs.GetLine(25)); //select game for restore

    CArray<WIN32_FIND_DATAA> files;
    AppFindSavesList(&files, 1);

    for (int i = 0; i < files.GetSize(); i++)
    {
        WIN32_FIND_DATAA* inf = &files[i];
        FILE* f = fopen(inf->cFileName, "rb"); //WAT , LOL!

        uint32_t headers[2];
        fread(headers, 4, 2, f);
        if (headers[0] == 0x26677342)
        {
            fseek(f, headers[1], SEEK_SET);

            char savename[257];
            fread(savename, 1, 256, f);
            savename[256] = 0;

            list->AddItem(savename);
            file_names.Add(inf->cFileName);

            CTime t(inf->ftLastWriteTime);
            CString str;
            str.Format("%s  %02d.%02d.%d  %02d:%02d:%02d", savename, t.GetDay(), t.GetMonth(), t.GetYear(), t.GetHour(), t.GetMinute(), t.GetSecond());
            save_times.Add(str);
        }
        else
            inf->cFileName[0] = 0; // disable this save

        fclose(f);

        g_mousept.Update(); // do not freeze
    }

    //useless nival, useless remove
    /*
    for (int i = files.GetSize() - 1; i > 0; i--)
    {
        if (files[i].cFileName[0] == 0)
            files.RemoveAt(i);
    }
    */
    CRect& lr = list->GetRect();
    VisScrollBar* scrl = new VisScrollBar(10, lr.right, lr.top, lr.right + 24, lr.bottom, nullptr);

    AddChild(scrl);
    AddChild(list);
    list->SetCaptionLabel(caption);
    list->SetHints(&save_times);

    CRect local_17c((rect.Width() * 3) / 20, lr.bottom + 24, (rect.Width() * 7) / 20, lr.bottom + 48);
    CRect local_190((rect.Width() * 8) / 20, lr.bottom + 24, (rect.Width() * 12) / 20, lr.bottom + 48);
    CRect local_1a0((rect.Width() * 13) / 20, lr.bottom + 24, (rect.Width() * 17) / 20, lr.bottom + 48);

    VisButton* btn = new VisButton(4, local_17c, txt_dialogs.GetLine(0), g_font1, nullptr, 0x479, 0, txt_dialogs.GetLine(26));
    AddChild(btn);

    btn->ChangeFlags(FLAG_10, true);

    if (file_names.GetSize() == 0)
        btn->ChangeFlags(FLAG_ENABLED, false);

    VisButton* btn2 = new VisButton(6, local_190, txt_dialogs.GetLine(157), g_font1, nullptr, 0x475, 0, txt_dialogs.GetLine(158));
    AddChild(btn2);

    VisButton* btn3 = new VisButton(5, local_1a0, txt_dialogs.GetLine(1), g_font1, nullptr, 0x47a, 0, txt_dialogs.GetLine(27));
    AddChild(btn3);

    btn->SetRightObj(btn3);
    btn->SetLeftObj(btn2);
    btn->SetUpObj(list);
    btn3->SetUpObj(list);
}

LoadGameWindow::LoadGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk)
: VisWindow(_id, l, t, r, b, _bitmap)
{ //43ecfb
    field_0x68 = unk;
    field_0x94 = 0;
}



const char* HintedListBox::GetHint()
{ //43ec67
    int32_t idx = YToIndex(g_mousept.GetY());
    if (idx >= num_vis_entry)
        return nullptr;
    
    idx += vis_start_index;

    if (idx >= entries.GetSize())
        return nullptr;

    CString& str = hints->ElementAt(idx);
    if (str.IsEmpty())
        return nullptr;
    
    return str;
}

HintedListBox::HintedListBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: VisListBox(_id, l, t, r, b, _font, _clr1, _clr2, _scrollid, hint)
{ //43ec05
}



int32_t SaveGameWindow::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{ //440726
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    switch (msg)
    {
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);

    case 0x444:
    case 0x479:
    {
        if (FindChild(4)->TestFlags(FLAG_ENABLED) != 0)
        {
            char buf[256];
            FindChild(1)->WriteData(buf);

            strcpy(field_0x68->title, buf);

            CStringArray arr;
            ((VisListBox*)FindChild(3))->WriteData(&arr);

            for (int i = 1; i < arr.GetSize(); i++)
            {
                if (strcmp(arr[i], buf) == 0)
                {
                    field_0x94 = i;
                    break;
                }
            }

            strcpy(field_0x68->filename, file_names[field_0x94]);

            VisScreen::MsgProc(0x445, 0, 0);
        }
        return 1;
    }

    case 0x46e:
        if (wparam == 3)
        {
            field_0x94 = lparam;
            CStringArray local_158;

            const char* str = "";
            if (field_0x94 >= 1)
            {
                FindChild(3)->WriteData(&local_158);
                str = local_158[field_0x94];
            }
            CVisualObject* obj = FindChild(1);
            obj->ReadData(str);
            obj->VMethod9();
            CheckInput();
            return 1;
        }
        else if (wparam == 1)
            CheckInput();
        return VisScreen::MsgProc(msg, wparam, lparam);

    case 0x475:
    {
        VisListBox* lb = (VisListBox*)FindChild(3);
        int32_t idx = lb->GetSelectedIndex();
        if (idx != 0)
        {
            CString str = txt_patch.GetLine(50) + lb->GetItem(idx) + txt_patch.GetLine(51); // want to delete save ### ?
            mwnd->field_0x3dc = new VisMessageBoxWithList(1, 64, 100, 380, 594, str, nullptr, 4);
            mwnd->ModalScreen(mwnd->field_0x3dc);
            if (mwnd->field_0x3dc->GetCloseCode() == 0x447)
            {
                DeleteFileA(file_names[idx]);
                file_names.RemoveAt(idx);
                save_times.RemoveAt(idx);

                lb->RemoveItem(idx);
                lb->SelectItem(idx);

                MsgProc(0x46e, lb->GetId(), lb->GetSelectedIndex());
            }
        }
        return 1;
    }

    case 0x47a:
        VisScreen::MsgProc(0x446, 0, 0);
        return 1;
    }

    return 1;
}

int __cdecl SaveGameWindow::Compare(void const* a, void const* b)
{ //43faf4
    if (*(const int*)a > *(const int*)b)
        return 1;
    else if (*(const int*)a < *(const int*)b)
        return -1;
    return 0;
}

void SaveGameWindow::VMethod26()
{ //43fb2c

    AddChild(new VisLabel(2, 40, 32, rect.Width() - 40, 56, txt_dialogs.GetLine(29), g_font1, p_clrsh_Black, 2));

    VisLabel* caption = new VisLabel(0, 40, 104, rect.Width() - 40, 128, txt_dialogs.GetLine(144), g_font1, p_clrsh_Black, 0);//saved games
    AddChild(caption);

    VisTextBox* textbox = new VisTextBox(1, 40, 68, rect.Width() - 40, 92, g_font1, p_clrsh_Black, txt_dialogs.GetLine(30));
    AddChild(textbox);

    HintedListBox* list = new HintedListBox(3, 40, 128, rect.Width() - 64, 272, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 10, txt_dialogs.GetLine(31)); //saved games

    textbox->SetDownObj(list);

    list->AddItem(field_0x98);
    file_names.Add("");
    save_times.Add("");

    CArray<WIN32_FIND_DATAA> files;
    AppFindSavesList(&files, 0);

    for (int i = 0; i < files.GetSize(); i++)
    {
        WIN32_FIND_DATAA* inf = &files[i];
        FILE* f = fopen(inf->cFileName, "rb"); //WAT , LOL!

        uint32_t headers[2];
        fread(headers, 4, 2, f);
        if (headers[0] == 0x26677342)
        {
            fseek(f, headers[1], SEEK_SET);

            char savename[257];
            fread(savename, 1, 256, f);
            savename[256] = 0;

            list->AddItem(savename);
            file_names.Add(inf->cFileName);

            CTime t(inf->ftLastWriteTime);
            CString str;
            str.Format("%s  %02d.%02d.%d  %02d:%02d:%02d", savename, t.GetDay(), t.GetMonth(), t.GetYear(), t.GetHour(), t.GetMinute(), t.GetSecond());
            save_times.Add(str);
        }
        else
            inf->cFileName[0] = 0; // disable this save

        fclose(f);

        g_mousept.Update(); // do not freeze
    }

    //useless nival, useless remove
    /*
    for (int i = files.GetSize() - 1; i > 0; i--)
    {
        if (files[i].cFileName[0] == 0)
            files.RemoveAt(i);
    }
    */

    int next_save_id = 0;
    int sz = file_names.GetSize() - 1;
    if (sz >= 1)
    {
        int* ids = new int[sz];
        for (int i = 0; i < sz; i++)
        {
            CString& s = file_names[i + 1];
            ids[i] = (s[4] - '0') * 1000 + (s[5] - '0') * 100 + (s[6] - '0') * 10 + (s[7] - '0');
        }
        qsort(ids, sz, sizeof(int), Compare);
        next_save_id = ids[sz - 1] + 1;
        for (int i = 0; i < sz - 1; i++)
        {
            if (ids[i] != i)
            {
                next_save_id = i;
                break;
            }
        }
        delete[] ids;
    }

    file_names[0].Format("game%04ld.sav", next_save_id);

    CRect& lr = list->GetRect();
    VisScrollBar* scrl = new VisScrollBar(10, lr.right, lr.top, lr.right + 24, lr.bottom, nullptr);

    AddChild(scrl);
    AddChild(list);
    list->SetCaptionLabel(caption);
    list->SetHints(&save_times);

    CRect local_17c((rect.Width() * 3) / 20, lr.bottom + 24, (rect.Width() * 7) / 20, lr.bottom + 48);
    CRect local_190((rect.Width() * 8) / 20, lr.bottom + 24, (rect.Width() * 12) / 20, lr.bottom + 48);
    CRect local_1a0((rect.Width() * 13) / 20, lr.bottom + 24, (rect.Width() * 17) / 20, lr.bottom + 48);

    VisButton* btn = new VisButton(4, local_17c, txt_dialogs.GetLine(0), g_font1, nullptr, 0x479, 0, txt_dialogs.GetLine(26));
    AddChild(btn);

    btn->ChangeFlags(FLAG_10, true);

    if (file_names.GetSize() == 0)
        btn->ChangeFlags(FLAG_ENABLED, false);

    VisButton* btn2 = new VisButton(6, local_190, txt_dialogs.GetLine(157), g_font1, nullptr, 0x475, 0, txt_dialogs.GetLine(158));
    AddChild(btn2);

    VisButton* btn3 = new VisButton(5, local_1a0, txt_dialogs.GetLine(1), g_font1, nullptr, 0x47a, 0, txt_dialogs.GetLine(27));
    AddChild(btn3);

    btn->SetRightObj(btn3);
    btn->SetLeftObj(btn2);
    btn->SetUpObj(list);
    btn3->SetUpObj(list);

    CheckInput();
}

SaveGameWindow::SaveGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk)
    : VisWindow(_id, l, t, r, b, _bitmap)
{ //43fa42
    field_0x68 = unk;
    field_0x94 = 0;
    field_0x98 = txt_dialogs.GetLine(28); //empty cell
}

void SaveGameWindow::CheckInput()
{ //440cc4
    char buf[256];
    FindChild(1)->WriteData(buf);

    CVisualObject* obj = FindChild(4);
    if (strlen(buf) == 0)
        obj->ChangeFlags(FLAG_ENABLED, false);
    else
        obj->ChangeFlags(FLAG_ENABLED, true);

    obj->VMethod9();
}


int32_t GameOptionsWindow::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{ //443cd7
    int32_t oldshtfl = g_settings.ShowTimeFlow;

    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    if (msg == 0x445)
    {
        VisScrollBar::Data dat;
        FindChild(2)->WriteData(&dat);
        mwnd->SetSpeed(dat.v);

        if (mwnd->sessionMode == 2)
            FindChild(13)->WriteData(&g_settings.TipsMode);
        else
            FindChild(13)->WriteData(&g_settings.ClanNames);

        FindChild(12)->WriteData(&g_settings.ShowTimeFlow);
        FindChild(3)->WriteData(&g_settings.Smoothing);
        FindChild(31)->WriteData(&g_Shadows);
        FindChild(32)->WriteData(&g_Lightning);
        FindChild(33)->WriteData(&g_Animation);

        if (g_Animation == 0)
            g_Lightning = 0;

        FindChild(4)->WriteData(g_settings.pShowAllHitPoints);
        FindChild(5)->WriteData(g_settings.pShowFlyingHP);
        FindChild(41)->WriteData(&g_MessageColors);

        SetMessageColors(g_MessageColors);

        FindChild(7)->WriteData(g_settings.pFormationMode);
        FindChild(9)->WriteData(g_settings.pWimpyMode);

        g_settings.AutoCasting = 8;

        int32_t sel;
        FindChild(141)->WriteData(&sel);

        if (sel == 1)
            g_settings.AutoCasting |= 0x18;
        else if (sel == 2)
            g_settings.AutoCasting |= 0x38;

        FindChild(131)->WriteData(&sel);
        if (sel != 0)
            g_settings.AutoCasting |= 1;

        FindChild(132)->WriteData(&sel);
        if (sel != 0)
            g_settings.AutoCasting |= 2;

        FindChild(133)->WriteData(&sel);
        if (sel != 0)
            g_settings.AutoCasting |= 4;


        mwnd->vis_map_context->FUN_0041abd2(*g_settings.pFormationMode % 3);
        mwnd->vis_map_context->FUN_0041aaaa(*g_settings.pWimpyMode % 3);
        mwnd->vis_map_context->FUN_0041ab74();
    }
    if (g_settings.ShowTimeFlow != oldshtfl)
        mwnd->vis_map_context->FUN_0041d97e(1);

    return VisScreen::MsgProc(msg, wparam, lparam);
}

void GameOptionsWindow::VMethod26()
{ //442646
    
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    AddChild( new VisLabel(-1, 40, 20, rect.Width() - 40, 40, txt_dialogs.GetLine(150), g_font1, p_clrsh_Black, 2) );

    VisLabel* caption = new VisLabel(1, 40, 56, 232, 80, txt_dialogs.GetLine(50), g_font1, p_clrsh_Black, 2);
    AddChild(caption);

    VisScrollBar* scrl = new VisScrollBar(2, 40, 84, 232, 108, txt_dialogs.GetLine(51));
    AddChild(scrl);

    if (mwnd->sessionMode == 0)
        scrl->ChangeFlags(FLAG_ENABLED, false);

    VisScrollBar::Data sd;
    sd.v = *g_settings.pGameSpeed;
    sd.vmax = 8;
    scrl->ReadData(&sd);

    scrl->SetCaptionLabel(caption);

    CRect& scr = scrl->GetRect();

    VisRadioType1* daych = new VisRadioType1(12, scr.left, scr.bottom + 12, scr.right, scr.bottom + 36, g_font1, p_clrsh_Black, nullptr);
    daych->AddEntry(txt_dialogs.GetLine(55));
    AddChild(daych);

    daych->ReadData(&g_settings.ShowTimeFlow);
    daych->SetUpObj(scrl);

    VisRadioType1* smooth = new VisRadioType1(3, scr.left, scr.bottom + 40, scr.right, scr.bottom + 64, g_font1, p_clrsh_Black, nullptr);
    smooth->AddEntry(txt_dialogs.GetLine(53));
    AddChild(smooth);

    smooth->ReadData(&g_settings.Smoothing);
    smooth->SetUpObj(daych);

    VisRadioType1* shd = new VisRadioType1(31, scr.left, scr.bottom + 68, scr.right, scr.bottom + 92, g_font1, p_clrsh_Black, nullptr);
    shd->AddEntry(txt_patch.GetLine(52));
    AddChild(shd);

    shd->ReadData(&g_Shadows);
    shd->SetUpObj(smooth);

    VisRadioType1* light = new VisRadioType1(32, scr.left, scr.bottom + 96, scr.right, scr.bottom + 120, g_font1, p_clrsh_Black, nullptr);
    light->AddEntry(txt_patch.GetLine(53));
    AddChild(light);

    light->ReadData(&g_Lightning);
    light->SetUpObj(shd);

    VisRadioType1* anims = new VisRadioType1(33, scr.left, scr.bottom + 124, scr.right, scr.bottom + 148, g_font1, p_clrsh_Black, nullptr);
    anims->AddEntry(txt_patch.GetLine(54));
    AddChild(anims);

    anims->ReadData(&g_Animation);
    anims->SetUpObj(light);

    int ypoint = caption->GetRect().top - 15;

    VisRadioType1* showhit = new VisRadioType1(4, scr.right + 48, ypoint, scr.right + 264, ypoint + 24, g_font1, p_clrsh_Black, nullptr);
    showhit->AddEntry(txt_dialogs.GetLine(57));
    AddChild(showhit);

    showhit->ReadData(g_settings.pShowAllHitPoints);
    showhit->SetLeftObj(scrl);

    VisRadioType1* showhp = new VisRadioType1(5, scr.right + 48, ypoint + 28, scr.right + 264, ypoint + 52, g_font1, p_clrsh_Black, nullptr);
    showhp->AddEntry(txt_dialogs.GetLine(78));
    AddChild(showhp);

    showhp->ReadData(g_settings.pShowFlyingHP);
    showhp->SetUpObj(showhit);

    VisRadioType1* clantips = new VisRadioType1(13, scr.right + 48, ypoint + 56, scr.right + 264, ypoint + 80, g_font1, p_clrsh_Black, nullptr);

    if (mwnd->sessionMode == 2)
        clantips->AddEntry(txt_dialogs.GetLine(156)); //tips
    else
        clantips->AddEntry(txt_dialogs.GetLine(175)); //clans

    AddChild(clantips);

    if (mwnd->sessionMode == 2)
        clantips->ReadData(&g_settings.TipsMode);
    else
        clantips->ReadData(&g_settings.ClanNames);
    clantips->SetUpObj(showhp);


    VisRadioType1* altclr = new VisRadioType1(41, scr.right + 48, ypoint + 84, scr.right + 264, ypoint + 108, g_font1, p_clrsh_Black, nullptr);

    altclr->AddEntry(txt_patch.GetLine(87)); //alt color
    AddChild(altclr);

    altclr->ReadData(&g_MessageColors);
    altclr->SetUpObj(clantips);

    
    AddChild(new VisLabel(15, 280, caption->GetRect().bottom + 74, 472, caption->GetRect().bottom + 98, txt_dialogs.GetLine(166), g_font1, p_clrsh_Black, 0));


    VisRadioType1* ochar = new VisRadioType1(131, scr.right + 48, scr.bottom + 68, scr.right + 264, scr.bottom + 92, g_font1, p_clrsh_Black, nullptr);

    ochar->AddEntry(txt_dialogs.GetLine(167)); //own chars
    AddChild(ochar);

    int32_t aflg = (g_settings.AutoCasting & 1) != 0;
    ochar->ReadData(&aflg);
    ochar->SetUpObj(altclr);


    VisRadioType1* alli = new VisRadioType1(132, scr.right + 48, scr.bottom + 96, scr.right + 264, scr.bottom + 120, g_font1, p_clrsh_Black, nullptr);

    alli->AddEntry(txt_dialogs.GetLine(168)); //allies
    AddChild(alli);

    aflg = (g_settings.AutoCasting & 2) != 0;
    alli->ReadData(&aflg);
    alli->SetUpObj(ochar);


    VisRadioType1* neutral = new VisRadioType1(133, scr.right + 48, scr.bottom + 124, scr.right + 264, scr.bottom + 148, g_font1, p_clrsh_Black, nullptr);

    neutral->AddEntry(txt_dialogs.GetLine(169)); //neutral
    AddChild(neutral);

    aflg = (g_settings.AutoCasting & 4) != 0;
    neutral->ReadData(&aflg);
    neutral->SetUpObj(alli);


    VisLabel* lbl = new VisLabel(6, 40, scr.bottom + 156, 208, scr.bottom + 180, txt_dialogs.GetLine(58), g_font1, p_clrsh_Black, 0);
    AddChild(lbl);

    CRect* lr = &lbl->GetRect();
    
    //formation
    VisRadioType2* formation = new VisRadioType2(7, 40, lr->bottom, 208, lr->bottom + 72, g_font1, p_clrsh_Black, txt_dialogs.GetLine(59));
    formation->AddEntry(txt_dialogs.GetLine(60)); //off
    formation->AddEntry(txt_dialogs.GetLine(61)); //auto
    formation->AddEntry(txt_dialogs.GetLine(62)); //on
    AddChild(formation);

    formation->ReadData(g_settings.pFormationMode);
    anims->SetDownObj(formation);


    lr = &lbl->GetRect();

    lbl = new VisLabel(8, lr->right, lr->top, lr->right + 168, lr->bottom, txt_dialogs.GetLine(63), g_font1, p_clrsh_Black, 0);
    AddChild(lbl);


    lr = &formation->GetRect();

    //autoretreat
    VisRadioType2* retreat = new VisRadioType2(9, lr->right, lr->top, lr->right + 168, lr->bottom, g_font1, p_clrsh_Black, txt_dialogs.GetLine(64));
    retreat->AddEntry(txt_dialogs.GetLine(65)); //off
    retreat->AddEntry(txt_dialogs.GetLine(66)); //norm
    retreat->AddEntry(txt_dialogs.GetLine(67)); //panic
    AddChild(retreat);

    retreat->ReadData(g_settings.pWimpyMode);

    formation->SetRightObj(retreat);
    retreat->SetUpObj(neutral);
    neutral->SetDownObj(retreat);

    lr = &lbl->GetRect();
    lbl = new VisLabel(151, lr->right, lr->top, lr->right + 168, lr->bottom, txt_dialogs.GetLine(170), g_font1, p_clrsh_Black, 0);
    AddChild(lbl);

    lr = &retreat->GetRect();
    VisRadioType2* acast = new VisRadioType2(141, lr->right, lr->top, lr->right + 168, lr->bottom, g_font1, p_clrsh_Black, txt_dialogs.GetLine(174));
    acast->AddEntry(txt_dialogs.GetLine(171)); //min
    acast->AddEntry(txt_dialogs.GetLine(172)); //mid
    acast->AddEntry(txt_dialogs.GetLine(173)); //max
    AddChild(acast);

    aflg = (g_settings.AutoCasting & 0x10) != 0;
    if ((g_settings.AutoCasting & 0x20) != 0)
        aflg = 2;

    acast->ReadData(&aflg);

    acast->SetLeftObj(retreat);
    acast->SetUpObj(neutral);
    neutral->SetDownObj(acast);

    CRect local_30(rect.Width() / 7, acast->GetRect().bottom + 8, (rect.Width() * 3) / 7, acast->GetRect().bottom + 32);
    CRect local_40((rect.Width() * 4) / 7, acast->GetRect().bottom + 8, (rect.Width() * 6) / 7, acast->GetRect().bottom + 32);

    VisButton* acpt = new VisButton(10, local_30, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "");
    AddChild(acpt);
    acpt->ChangeFlags(FLAG_10, true);

    VisButton* cncl = new VisButton(11, local_40, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    AddChild(cncl);

    acpt->SetRightObj(cncl);
}




EndGameMenu::EndGameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitm, uint32_t unk, const CRect& _r, int32_t unk2)
 : VisMenuWnd(_id, l, t, r, b, _bitm, unk, _r)
{ //441357
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    MenuButton* btn = nullptr;

    if (mwnd->sessionMode == 1 || mwnd->sessionMode == 3 || mwnd->sessionMode == 0)
        btn = new MenuButton(1, txt_dialogs.GetLine(42), g_font1, nullptr, 0x41d, 'C', ""); //restart mission
    else
        btn = new MenuButton(1, txt_dialogs.GetLine(43), g_font1, nullptr, 0x41d, 'V', ""); //win
    AddElement(btn, 30);

    if (unk2 == 0 && mwnd->sessionMode == 2)
        btn->ChangeFlags(FLAG_ENABLED, false);
    
    if (g_CLlDriver.GetProvider() == 4)
        btn->ChangeFlags(FLAG_ENABLED, false);

    btn = new MenuButton(2, txt_dialogs.GetLine(44), g_font1, nullptr, 0x41e, 'E', ""); //exit to main
    AddElement(btn, 30);

    btn = new MenuButton(3, txt_dialogs.GetLine(45), g_font1, nullptr, WM_CLOSE, 'W', ""); //exit to win
    AddElement(btn, 30);

    btn = new MenuButton(4, txt_dialogs.GetLine(40), g_font1, nullptr, 0x446, 'R', ""); //return to game
    AddElement(btn, 30);
}


ExitGameMenu::ExitGameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CRect& _r)
: VisMenuWnd(_id, l, t, r, b, nullptr, 0, _r)
{ //44186e
    MenuButton* btn = new MenuButton(2, txt_dialogs.GetLine(44), g_font1, nullptr, 0x41e, 'E', ""); //exit to main
    AddElement(btn, 30);

    btn = new MenuButton(3, txt_dialogs.GetLine(45), g_font1, nullptr, WM_CLOSE, 'W', ""); //exit to win
    AddElement(btn, 30);

    btn = new MenuButton(4, txt_dialogs.GetLine(40), g_font1, nullptr, 0x446, 'R', ""); //return to game
    AddElement(btn, 30);
}

TownMenuListDialogVisualObject::TownMenuListDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CRect& _r)
: VisMenuWnd(_id, l, t, r, b, nullptr, 0, _r)
{ //44160e
    MenuButton* btn = new MenuButton(2, txt_dialogs.GetLine(34), g_font1, nullptr, 0x41a, 'S', ""); //Save the game
    AddElement(btn, 30);

    btn = new MenuButton(1, txt_dialogs.GetLine(35), g_font1, nullptr, 0x418, 'L', ""); //load game
    AddElement(btn, 30);

    btn = new MenuButton(3, txt_dialogs.GetLine(37), g_font1, nullptr, 0x422, 'N', ""); //Sound options
    AddElement(btn, 30);

    btn = new MenuButton(4, txt_dialogs.GetLine(77), g_font1, nullptr, 0x41c, 'E', ""); //exit
    AddElement(btn, 30);

    btn = new MenuButton(5, txt_dialogs.GetLine(40), g_font1, nullptr, 0x446, 'E', ""); //return to game
    AddElement(btn, 30);
}


SoundPreferencesDialogVisualObject::SoundPreferencesDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SoundSettings *pset)
: VisWindow(_id, l, t, r, b, _bitmap)
{ //43d917
    settings = pset;
}


int32_t SoundPreferencesDialogVisualObject::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{ //43e6f0
    int32_t result = 0;
    switch (msg)
    {
    case 0x467:
        result = 1;
        break;

    case 0x46e:
        switch (wparam)
        {
        case 2:
            settings->random = lparam;
            music_player->SetRandom(lparam);
            result = 1;
            break;

        case 3:
            track_index = lparam;
            result = 1;
            break;

        case 6:
        {
            int32_t vol = ConvSliderToVolume(lparam, settings->field_0xc);
            int32_t cur_vol = music_player->GetVolume();
            if (vol != cur_vol)
            {
                settings->mus_pos = vol;
                if (music_player->GetState() != 2)
                    music_player->SetVolume(vol);
            }
            result = 1;
        }   break;


        case 7:
            settings->sfx_pos = ConvSliderToVolume(lparam, settings->field_0x14);
            PostMessage(g_MainWndHWND, 0x485, 0, 0);
            result = 1;
            break;

        case 8:
            settings->speech_pos = ConvSliderToVolume(lparam, settings->field_0x1c);
            result = 1;
            break;
        default:
            result = 1;
            break;
        }
        break;

    case 0x474:
        if (wparam == 7)
            g_SfxArray[100]->Play(settings->sfx_pos, 0, 0, 220, 0);
        else if (wparam == 8)
            SoundBank_fighter[0].select[1]->Play(settings->sfx_pos, 0, 0, 220, 0);
        result = 1;
        break;

    case 0x476:
        music_player->SetPlayNotify(0);
        FindChild(40)->WriteData(&g_settings.Acknowledgement);
        VisScreen::MsgProc(0x445, 0, 0);
        result = 1;
        break;

    case 0x477:
        settings->music_enabled = 1;
        music_player->SetFadeout(0);
        if (track_index == music_player->GetCurrentTrackIndex())
        {
            music_player->SetVolume(settings->mus_pos);
            music_player->Play();
        }
        else
        {
            music_player->OnEndTrack();
            music_player->StartPlayTrack(track_index);
            music_player->SetVolume(settings->mus_pos);
            music_player->Play();
        }
        result = 1;
        break;

    case 0x478:
        if (music_player->GetState() == 2)
            music_player->OnEndTrack();
        if (music_player->GetState() == 1)
            music_player->BeginFadeOut(2000, 8000);

        settings->music_enabled = 0;
        result = 1;
        break;

    default:
        result = VisScreen::MsgProc(msg, wparam, lparam);
        break;
    }
    return result;
}

void SoundPreferencesDialogVisualObject::VMethod26()
{ //43d959
    MainWindow *mwnd = (MainWindow*)AfxGetMainWnd();

    music_player = mwnd->music_player;
    track_index = 0;

    AddChild(new VisLabel(555, 40, 20, rect.Width() - 40, 45, txt_dialogs.GetLine(7), g_font1, p_clrsh_Black, 2)); //Sound options

    VisLabel* lbl_melodies = new VisLabel(557, 40, 60, rect.Width() - 40, 78, txt_dialogs.GetLine(143), g_font1, p_clrsh_Black, 0); //melodies
    AddChild(lbl_melodies);

    VisButton* btn_acpt = new VisButton(1, 40, 290, 252, 314, txt_dialogs.GetLine(0), g_font1, nullptr, 0x476, 0, txt_dialogs.GetLine(8)); //Accept
    btn_acpt->ChangeFlags(FLAG_10, true);
    AddChild(btn_acpt);

    VisRadioType1* btn_raport = new VisRadioType1(40, 40, 190, 252, 214, g_font1, p_clrsh_Black, txt_dialogs.GetLine(165)); //Raports
    btn_raport->AddEntry(txt_dialogs.GetLine(165));
    btn_raport->ReadData(&g_settings.Acknowledgement);
    AddChild(btn_raport);

    if (mwnd->sessionMode == 0 || mwnd->sessionMode == 1)
        btn_raport->ChangeFlags(FLAG_ENABLED, false);


    VisListBox* lst_melody = new VisListBox(3, 40, 80, rect.Width() - 64, 170, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 10, txt_dialogs.GetLine(11)); //Select melody
    lst_melody->SetSelectedIndex(track_index);

    for (int i = 0; i < music_player->GetPlaylistSize(); i++)
    {
        CString trk = music_player->GetPlaylistEntry(i);
        trk.MakeLower();
        trk = trk.Mid(6); // music/

        CString name;
        g_TunesMap.Lookup(trk, name);
        lst_melody->AddItem(name);
    }
    AddChild(lst_melody);

    lst_melody->SetCaptionLabel(lbl_melodies);


    CRect r = lst_melody->GetRect();
    AddChild(new VisScrollBar(10, r.right, r.top, r.right + 24, r.bottom, nullptr));

    VisButton* btn_play = new VisButton(4, 40, 256, 140, 280, txt_dialogs.GetLine(12), g_font1, nullptr, 0x477, 0, txt_dialogs.GetLine(13)); //Play
    if (settings->field_0x20 == 0)
    {
        btn_play->ChangeFlags(FLAG_ENABLED, false);
        btn_play->SetHint(txt_dialogs.GetLine(23)); //Music disabled
    }
    if (music_player->GetPlaylistSize() == 0)
    {
        btn_play->ChangeFlags(FLAG_ENABLED, 0);
        btn_play->SetHint(txt_dialogs.GetLine(75)); //Music opts now unav
    }
    AddChild(btn_play);


    VisButton* btn_stop = new VisButton(5, 150, 256, 252, 280, txt_dialogs.GetLine(14), g_font1, nullptr, 0x478, 0, txt_dialogs.GetLine(15)); //stop
    if (settings->field_0x20 == 0)
    {
        btn_stop->ChangeFlags(FLAG_ENABLED, 0);
        btn_stop->SetHint(txt_dialogs.GetLine(23)); //Music unav
    }
    if (music_player->GetPlaylistSize() == 0)
    {
        btn_stop->ChangeFlags(FLAG_ENABLED, 0);
        btn_stop->SetHint(txt_dialogs.GetLine(75)); //Music opts now unav
    }
    AddChild(btn_stop);


    btn_stop->SetUpObj(btn_raport);
    btn_play->SetUpObj(btn_raport);

    btn_stop->SetDownObj(btn_acpt);
    btn_play->SetDownObj(btn_acpt);

    btn_play->SetRightObj(btn_stop);

    VisLabel* lbl_mvol = new VisLabel(26, 258, 175, rect.Width() - 40, 190, txt_dialogs.GetLine(16), g_font1, p_clrsh_Black, 2);
    AddChild(lbl_mvol);

    VisScrollBar* scrl_mvol = new VisScrollBar(6, 258, 190, rect.Width() - 40, 214, txt_dialogs.GetLine(19));
    AddChild(scrl_mvol);

    scrl_mvol->SetCaptionLabel(lbl_mvol);

    VisScrollBar::Data sd;
    sd.v = ConvVolumeToSlider(settings->mus_pos, settings->field_0xc);
    sd.vmax = settings->field_0xc;

    scrl_mvol->ReadData(&sd);

    if (settings->field_0x20 == 0)
    {
        scrl_mvol->ChangeFlags(FLAG_ENABLED, false);
        scrl_mvol->SetHint(txt_dialogs.GetLine(23)); //Music unav
    }
    if (music_player->GetPlaylistSize() == 0)
    {
        scrl_mvol->ChangeFlags(FLAG_ENABLED, 0);
        scrl_mvol->SetHint(txt_dialogs.GetLine(75)); //Music opts now unav
    }


    VisLabel* lbl_efvol = new VisLabel(27, 258, 224, rect.Width() - 40, 239, txt_dialogs.GetLine(17), g_font1, p_clrsh_Black, 2); //effects vol
    AddChild(lbl_efvol);

    VisScrollBar* scrl_efvol = new VisScrollBar(7, 258, 240, rect.Width() - 40, 264, txt_dialogs.GetLine(20));
    AddChild(scrl_efvol);

    sd.v = ConvVolumeToSlider(settings->sfx_pos, settings->field_0x14);
    sd.vmax = settings->field_0x14;

    scrl_efvol->ReadData(&sd);
    scrl_efvol->SetCaptionLabel(lbl_efvol);


    VisLabel* lbl_svol = new VisLabel(28, 258, 275, rect.Width() - 40, 290, txt_dialogs.GetLine(18), g_font1, p_clrsh_Black, 2); //speech vol
    AddChild(lbl_svol);

    VisScrollBar* scrl_svol = new VisScrollBar(8, 258, 290, rect.Width() - 40, 314, txt_dialogs.GetLine(21));
    AddChild(scrl_svol);

    sd.v = ConvVolumeToSlider(settings->speech_pos, settings->field_0x1c);
    sd.vmax = settings->field_0x14;

    scrl_svol->ReadData(&sd);
    scrl_svol->SetCaptionLabel(lbl_svol);


    scrl_svol->SetLeftObj(btn_acpt);

    scrl_mvol->SetDownObj(scrl_efvol);
    scrl_efvol->SetDownObj(scrl_svol);

    music_player->SetPlayNotify(1);
}



void VisGlobalMap::RebuildScenarioLocations()
{ //47024a
    PopulateScenarioLocationFlags();
}

void VisGlobalMap::PopulateScenarioLocationFlags()
{ //47025d
    CList<ScenarioLocation*> *locs = ScenarioGetAllLocations();
    for (POSITION pos = locs->GetHeadPosition(); pos != nullptr;)
    {
        ScenarioLocation* loc = locs->GetNext(pos);
        locationPoints.Add(loc->GetRect().TopLeft());

        if (loc->GetRect().BottomRight() == CPoint(0, 0))
            locationAvailabilityFlags.Add(1);
    }
}


// 4714E7
void VisGlobalMap::VMethod7()
{
    CPoint screen_pt = this->rect.TopLeft();
    int32_t screen_x = screen_pt.x;
    int32_t screen_y = screen_pt.y;
    int32_t travel_arrived = 0;
    int32_t arrival_index = 0;

    if (this->renderActiveFlag == 0) {
        return;
    }

    LockSurface2();
    this->gmap->VMethod2(screen_x, screen_y, 0, 0, 0);

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
                this->ballmap->VMethod10(screen_x + pt.x - w / 2, screen_y + pt.y - h / 2, 0, 0, w, h);
            }
        }
        for (POSITION pos = ScenarioGetAvailableLocations()->GetHeadPosition(); pos != nullptr;) {
            ScenarioLocation* loc = ScenarioGetAvailableLocations()->GetNext(pos);
            CPoint loc_pt = loc->GetRect().TopLeft();
            if (this->umoirMapMode == 0) {
                if (this->currentLocationPoint != CPoint(loc_pt.x, loc_pt.y)) {
                    if (this->targetLocationPoint == CPoint(loc_pt.x, loc_pt.y)) {
                        this->mission_flg->VMethod2(screen_x - 5 + loc_pt.x, screen_y - 0x29 + loc_pt.y,
                            this->mapFlagAnimationFrame, 0, 0);
                    } else {
                        this->flg_on_map->VMethod2(screen_x - 5 + loc_pt.x, screen_y - 0x25 + loc_pt.y,
                            this->mapFlagAnimationFrame, 0, 0);
                    }
                }
            } else {
                this->flag1_spr->VMethod2(screen_x - 4 + loc_pt.x, screen_y - 0x20 + loc_pt.y,
                    this->mapFlagAnimationFrame, 0, 0);
            }
        }
    } else {
        int32_t dot_count = this->travelProgress;
        if (dot_count >= this->travelRoutePoints.GetSize()) {
            dot_count = this->travelRoutePoints.GetSize();
        }
        for (int32_t i = 0; i < dot_count; i++) {
            if (i % 8 == 0) {
                CPoint& pt = this->travelRoutePoints.ElementAt(i);
                int32_t w = this->ballmap->GetWidth(0);
                int32_t h = this->ballmap->GetHeight(0);
                this->ballmap->VMethod10(screen_x + pt.x - w / 2, screen_y + pt.y - h / 2, 0, 0, w, h);
            }
        }
        int32_t cross_frames = this->cross_spr->GetFrameCount();
        if (this->targetCrossAnimationFrame < cross_frames - 1) {
            this->cross_spr->VMethod2(screen_x - 10 + this->targetLocationPoint.x,
                screen_y - 0xC + this->targetLocationPoint.y, this->targetCrossAnimationFrame, 0, 0);
        } else {
            this->cross_spr->VMethod2(screen_x - 10 + this->targetLocationPoint.x,
                screen_y - 0xC + this->targetLocationPoint.y, cross_frames - 1, 0, 0);
        }
        this->targetCrossAnimationFrame++;
        this->travelProgress += 8;
        if (this->travelProgress < this->travelRoutePoints.GetSize()) {
            CSound& cur_snd = (this->routePointSoundIndex == 0) ? this->snd_point1 : this->snd_point2;
            if (!FUN_00475110(&cur_snd)) {
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
        this->flag_spr->VMethod2(screen_x - 0xA + this->currentLocationPoint.x,
            screen_y - 0x18 + this->currentLocationPoint.y, this->partyFlagAnimationFrame, 0, 0);
    } else {
        this->yflag_spr->VMethod2(screen_x - 0x14 + this->currentLocationPoint.x,
            screen_y - 0x2C + this->currentLocationPoint.y, this->partyFlagAnimationFrame, 0, 0);
    }
    this->partyFlagAnimationFrame = (this->partyFlagAnimationFrame + 1) % this->flag_spr->GetFrameCount();

    if (this->umoirMapMode == 0 && !this->hoveredLocationTitle.IsEmpty()) {
        g_font4->DrawTxt(screen_x + 0xE6, screen_y + 0x1B, this->hoveredLocationTitle, 2,
            palette_brown_derby->GetPalette(0));
        for (int32_t i = 0; i < this->hoveredLocationLines.GetSize(); i++) {
            g_font2->DrawTxt(screen_x + 0xE6, screen_y + 0x30 + i * 10,
                this->hoveredLocationLines.GetAt(i), 2, clrsh_InvBarleyCorn);
        }
        int32_t n = this->hoveredLocationLines.GetSize();
        CString last_line = this->hoveredLocationLines.GetAt(n);
        CString trimmed = last_line.Left(last_line.GetLength() - 2);
        g_font2->DrawTxt(screen_x + 0xE6, screen_y + 0x30 + n * 10, trimmed, 2, clrsh_InvBarleyCorn);
    }

    UnlockSurface2();
    CVisualObject::VMethod7();
    if (travel_arrived != 0) {
        this->MsgProc(0x445, 0, 0);
        AfxGetMainWnd()->PostMessage(0x468, 0, 0);
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
void VisGlobalMap::DoClose(uint32_t code)
{
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


// 47115D
void VisGlobalMap::VMethod28()
{
    g_mousept.DisableHint();
    this->LoadBitmaps();
    this->LoadSamples();
    ReadFileToString("main\\text\\globalmap.txt", &g_MissionText);
    this->routePointSoundIndex = 0;
    GMapThing* thing = static_cast<GMapThing*>(operator new(0x9602C))->Init();
    this->graphNodePoints.RemoveAll();
    this->routeAdjacencyMatrix = thing->TakeAdjacency(&this->graphNodePoints);
    thing->Destroy(1);
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
void VisGlobalMap::VMethod26()
{
    this->hero_bmp = nullptr;
    this->snd_point2.sample = nullptr;
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
const char* VisGlobalMap::GetHint()
{
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
            return TxtFile_00660e88.GetLine(i);
        }
    }
    return nullptr;
}


// 473077
int32_t VisGlobalMap::OnRButtonDown(uint32_t wparam, CPoint pos)
{
    CRect popup_rect(CPoint(this->targetLocationPoint), this->heroBitmapSize);
    CPoint screen_pt = this->rect.TopLeft();
    pos -= CSize(screen_pt.x, screen_pt.y);
    if (popup_rect.PtInRect(pos)) {
        int32_t count = g_StructEnter.field_0x0.GetSize();
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
int32_t VisGlobalMap::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    if ((gmap_repaint_init & 1) == 0) {
        gmap_repaint_init |= 1;
        gmap_repaint_ts = timeGetTime() - 100;
    }
    if (msg == 0x402) {
        if (timeGetTime() - gmap_repaint_ts >= 100) {
            this->VMethod9();
            gmap_repaint_ts = timeGetTime();
        }
    }
    return VisScreen::MsgProc(msg, wparam, lparam);
}


// 473038
int32_t VisGlobalMap::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    if (this->travelProgress == 0) {
        this->travelProgress = 8;
    } else {
        this->FinishTravel();
    }
    return 1;
}


// 472D99
int32_t VisGlobalMap::OnMouseMove(uint32_t wparam, CPoint pos)
{
    if (this->travelProgress == 0) {
        this->OnMapClick();
    }
    this->UpdateHoveredLocation(pos);
    return 0;
}


// 473175
int32_t VisGlobalMap::OnRButtonUp(uint32_t wparam, CPoint pos)
{
    this->partDetailsRect.SetRectEmpty();
    return 1;
}


// 4726CC
int32_t VisGlobalMap::OnChar(uint32_t wparam)
{
    this->FinishTravel();
    return 1;
}


// 4726E6
int32_t VisGlobalMap::OnKeyDown(uint32_t wparam)
{
    this->FinishTravel();
    return 1;
}


// 474F10
int32_t VisGlobalMap::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    return 0;
}


// 471F08
void VisGlobalMap::VMethod8(CRect* rect)
{
}


// 47001B (scalar deleting dtor ??_G at 473D70)
VisGlobalMap::~VisGlobalMap()
{
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
    : VisScreen(_id, l, t, r, b, btm)
{
    this->VMethod26();
}


// 4705C2
void VisGlobalMap::LoadBitmaps()
{
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
void VisGlobalMap::FreeBitmaps()
{
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
void VisGlobalMap::ComputeTravelRoute(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY)
{
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
            CArray<CPoint>** row = static_cast<CArray<CPoint>**>(this->routeAdjacencyMatrix);
            this->travelRoutePoints.Append(row[prev][cur]);
        }
        delete route;
    } else {
        this->travelRoutePoints.RemoveAll();
        this->travelRoutePoints.Add(CPoint(fromX, fromY));
        this->travelRoutePoints.Add(CPoint(toX, toY));
    }
}


// 472DCA
void VisGlobalMap::OnMapClick()
{
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
void VisGlobalMap::UpdateHoveredLocation(CPoint pos)
{
    CPoint screen_pt = this->rect.TopLeft();
    pos -= CSize(screen_pt.x, screen_pt.y);
    ApplyCursor(g_Cursors[CURSOR_SELECT]);
}


// 470504
void VisGlobalMap::LoadSamples()
{
    this->FreeSamples();
    FUN_00438e40(&this->snd_scrollup.sample, "SFX\\ScrollUp.wav");
    FUN_00438e40(&this->snd_scrolldn.sample, "SFX\\ScrollDn.wav");
    FUN_00438e40(&this->snd_point1.sample, "SFX\\Point1.wav");
    FUN_00438e40(&this->snd_point2.sample, "SFX\\Point2.wav");
}


// 470571
void VisGlobalMap::FreeSamples()
{
    FUN_00438dd0(&this->snd_scrollup.sample);
    FUN_00438dd0(&this->snd_scrolldn.sample);
    FUN_00438dd0(&this->snd_point1.sample);
    FUN_00438dd0(&this->snd_point2.sample);
}


// 472684
void VisGlobalMap::FinishTravel()
{
    if (this->travelProgress != 0) {
        this->travelProgress = this->travelRoutePoints.GetSize() + 1;
        this->targetCrossAnimationFrame = this->cross_spr->GetFrameCount() + 1;
    }
}


// 472ADA
void VisGlobalMap::SearchRoute(uint16_t from, uint16_t to, GlobalMapRouteArray* route)
{
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


// 42F021
void VisCharGen::FreeSamples()
{
    FUN_00438dd0(&this->snd_stat);
    FUN_00438dd0(&this->snd_ok);
    FUN_00438dd0(&this->snd_reset);
    FUN_00438dd0(&this->snd_back);
}


// 42EFB4
void VisCharGen::LoadSamples()
{
    this->FreeSamples();
    FUN_00438e40(&this->snd_stat, "SFX\\ChrGen\\+_-.wav");
    FUN_00438e40(&this->snd_ok, "SFX\\Click_Ok.wav");
    FUN_00438e40(&this->snd_reset, "SFX\\Sbros.wav");
    FUN_00438e40(&this->snd_back, "SFX\\Back.wav");
}


// 42EBAB
void VisCharGen::OnClickBack()
{
    this->MsgProc(0x446, 0, 0);
}


// 42EB8C
void VisCharGen::OnClickFwd()
{
    this->MsgProc(0x445, 0, 0);
}


// 42EA87
void VisCharGen::ShowTipHint()
{
    if (g_settings.TipsMode != 0 && this->tips_step == 0 && this->tips != nullptr) {
        CString str;
        MissionGetTips(7, &str);
        this->tips->SetText(str);
        this->tips_step = this->tips_step + 1;
    }
}


// 42E92A
void VisCharGen::RollStats()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->stats_panel->field_0x1f0 = 100;
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
void VisCharGen::OnNextFace()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode == 2) {
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
    this->current_char->unitFlags |= 8;
    main_wnd->m_GameSession.face = this->current_char->face;
}


// 42EDC2
void VisCharGen::OnPrevFace()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode == 2) {
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
    this->current_char->unitFlags |= 8;
    main_wnd->m_GameSession.face = this->current_char->face;
}


// 42DCD9
VisCharGen::VisCharGen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr)
{
    this->VMethod26();
}


// 42DD66
VisCharGen::~VisCharGen()
{
    this->RemoveChild(this->info_panel);
    this->info_panel = nullptr;
    this->FreeSamples();
}


// 42DDEC
void VisCharGen::VMethod26()
{
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
    this->field14_0x9c = -1;
    this->mage_flag = 0;
    this->female_face = 0;
    this->selected_face = 0;
    this->active_flag = 0;
}


// 42EB29
void VisCharGen::VMethod7()
{
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
void VisCharGen::VMethod8(CRect* rect)
{
    (void)rect;
}


// 42EA01
int32_t VisCharGen::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    if (this->fwd_btn == 0) {
        return this->CVisualObject::OnLButtonUp(wparam, pos);
    }
    this->stats_panel->OnLButtonUp(wparam, pos);
    return 0;
}


// 42EA4F
int32_t VisCharGen::OnKeyDown(uint32_t wparam)
{
    if (wparam == 0xD) {
        this->OnClickFwd();
        return 1;
    }
    return this->VisScreen::OnKeyDown(wparam);
}


// 42E196
int32_t VisCharGen::OnMouseMove(uint32_t wparam, CPoint pos)
{
    CRect area = this->action_panel->GetRect() + this->rect.TopLeft();
    if (!area.PtInRect(pos)) {
        this->action_panel->ResetMouseBoxes();
    }
    return this->CVisualObject::OnMouseMove(wparam, pos);
}


// 42E058
int32_t VisCharGen::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    switch (msg) {
    case 0x402:
        this->VMethod9();
        break;
    case 0x414:
        this->OnPrevFace();
        break;
    case 0x415:
        this->OnNextFace();
        break;
    case 0x45A:
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
void VisCharGen::DoClose(uint32_t code)
{
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
void VisCharGen::VMethod28()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    g_mousept.DisableHint();
    this->info_panel = main_wnd->vis_charinfo;
    this->map_visual = main_wnd->vis_map_context;
    this->current_char = this->map_visual->GetUnit_3f6c();
    this->stats_panel->ReadUnitStats();
    this->selected_face = this->current_char->face - 1;
    this->mage_flag = (this->current_char->unitFlags & 2) << 5;
    this->female_face = (this->current_char->unitFlags & 4) << 6;
    main_wnd->vis_right_panel->RemoveChild(this->info_panel);
    CRect& rc = this->info_panel->GetRect();
    OffsetRect(&rc, 0x280 - rc.Width(), 0);
    this->info_panel->SetRect(&rc);
    this->AddChild(this->info_panel);
    if (main_wnd->sessionMode == 0) {
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
        if (this->current_char->unitFlags & 2) {
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
    this->field14_0x9c = -1;
    this->stats_panel->LoadBitmaps();
    this->fullstats_panel->LoadBitmaps();
    this->action_panel->LoadBitmaps();
    this->skills_panel->LoadBitmaps(this->current_char->unitFlags & 2);
    this->skills_panel->LoadSamples(this->current_char->unitFlags & 2);
    this->LoadSamples();
    this->stats_panel->field_0x1f0 = 0;
    this->stats_panel->HandleClick(0, CPoint(0, 0));
    for (int32_t i = 0; i < 5; i++) {
        this->skills_panel->field_0x10c[i] = 0;
    }
    this->skills_panel->selected_slot = main_wnd->m_GameSession.main_sphere - 1;
    this->skills_panel->field_0x10c[this->skills_panel->selected_slot] = 1;
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


void VisCharGenStats::LoadBitmaps()
{ //429475
    this->FreeBitmaps();
    this->bmp = new CBmp64("main\\graphics\\chrgen\\leftup.bmp");
    g_mousept.Update();
    this->field_0x194[0] = new CBmp64("graphics\\interface\\chrgen\\buttons\\plon.bmp");
    g_mousept.Update();
    this->field_0x194[1] = new CBmp64("graphics\\interface\\chrgen\\buttons\\ploff.bmp");
    g_mousept.Update();
    this->field_0x194[2] = new CBmp64("graphics\\interface\\chrgen\\buttons\\pnlon.bmp");
    g_mousept.Update();
    this->field_0x194[3] = new CBmp64("graphics\\interface\\chrgen\\buttons\\pnloff.bmp");
    g_mousept.Update();
    this->field_0x194[4] = new CBmp64("graphics\\interface\\chrgen\\buttons\\pdisable.bmp");
    g_mousept.Update();
    this->field_0x194[5] = new CBmp64("graphics\\interface\\chrgen\\buttons\\mlon.bmp");
    g_mousept.Update();
    this->field_0x194[6] = new CBmp64("graphics\\interface\\chrgen\\buttons\\mloff.bmp");
    g_mousept.Update();
    this->field_0x194[7] = new CBmp64("graphics\\interface\\chrgen\\buttons\\mnlon.bmp");
    g_mousept.Update();
    this->field_0x194[8] = new CBmp64("graphics\\interface\\chrgen\\buttons\\mnloff.bmp");
    g_mousept.Update();
    this->field_0x194[9] = new CBmp64("graphics\\interface\\chrgen\\buttons\\mdisable.bmp");
    g_mousept.Update();
    for (int32_t i = 0; i < 4; i++) {
        this->field_0x174[i * 2] = this->field_0x194[4];
        this->field_0x174[i * 2 + 1] = this->field_0x194[8];
    }
}


void VisCharGenStats::FreeBitmaps()
{ //4298b6
    if (this->bmp != nullptr) {
        delete this->bmp;
    }
    this->bmp = nullptr;
    for (int32_t i = 0; i < 10; i++) {
        if (this->field_0x194[i] != nullptr) {
            delete this->field_0x194[i];
        }
        this->field_0x194[i] = nullptr;
    }
    for (int32_t i = 0; i < 4; i++) {
        this->field_0x174[i * 2] = nullptr;
        this->field_0x174[i * 2 + 1] = nullptr;
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
        if ((this->field_0x64[i] + topleft).PtInRect(pt)) {
            return TxtFile::AllLines[i + 0x9B];
        }
        if ((this->field_0xa4 + topleft).PtInRect(pt)) {
            return TxtFile::AllLines[0x111];
        }
        if ((this->areas[i * 3] + topleft).PtInRect(pt)) {
            str.Format("%s = %d", (LPCTSTR)this->texts.ElementAt(i), (&this->stat_body)[i]);
            return str;
        }
        if ((this->areas[i * 3 + 1] + topleft).PtInRect(pt)) {
            str.Format("%+d", -this->StatUpCost((&this->stat_body)[i]));
            FUN_00476987(&str);
            return str;
        }
        if ((this->areas[i * 3 + 2] + topleft).PtInRect(pt)) {
            str.Format("%+d", this->StatDownRefund((&this->stat_body)[i]));
            FUN_00476987(&str);
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
    this->bmp->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
    for (int32_t i = 0; i < 4; i++) {
        str.Format("%d", (&this->stat_body)[i]);
        uint16_t* pal = palette_husk->GetPalette(0);
        g_font4->DrawTextWithShadow(screen_rect.left + this->areas[i * 3].left + 2,
                                    screen_rect.top + this->areas[i * 3].top + 4,
                                    str, 0, pal, 1);
        CBmp64* plus_bmp = this->field_0x174[i * 2];
        plus_bmp->VMethod10(screen_rect.left + this->areas[i * 3 + 1].left,
                            screen_rect.top + this->areas[i * 3 + 1].top,
                            0, 0, plus_bmp->GetWidth(0), plus_bmp->GetHeight(0));
        CBmp64* minus_bmp = this->field_0x174[i * 2 + 1];
        minus_bmp->VMethod10(screen_rect.left + this->areas[i * 3 + 2].left,
                             screen_rect.top + this->areas[i * 3 + 2].top,
                             0, 0, minus_bmp->GetWidth(0), minus_bmp->GetHeight(0));
        str.Format("%d", this->field_0x1f0);
        FUN_00476987(&str);
        pal = palette_husk->GetPalette(0);
        g_font4->DrawTextWithShadow(
            screen_rect.left + this->field_0xa4.left + this->field_0xa4.Width() / 2,
            screen_rect.top + this->field_0xa4.top + this->field_0xa4.Height() / 2 + 2,
            str, 10, pal, 1);
    }
    UnlockSurface2();
}


// 42a02a
int32_t VisCharGenStats::HitTest(CPoint pt) {
    CPoint topleft;
    this->ClientPtToScreen(&topleft, this->rect.TopLeft());
    for (int32_t i = 0; i < 4; i++) {
        if ((this->areas[i * 3] + topleft).PtInRect(pt)) {
            return i << 8;
        }
        if ((this->areas[i * 3 + 1] + topleft).PtInRect(pt)) {
            return (i << 8) | 1;
        }
        if ((this->areas[i * 3 + 2] + topleft).PtInRect(pt)) {
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
        this->field_0x174[i * 2] = this->field_0x194[3];
        if (this->field_0x1f0 >= up_cost && (&this->stat_body)[i] < 0x2D) {
            if (i == idx && button == 1) {
                if (mode & 1) {
                    this->field_0x174[i * 2] = this->field_0x194[0];
                } else {
                    this->field_0x174[i * 2] = this->field_0x194[1];
                }
            }
        } else {
            this->field_0x174[i * 2] = this->field_0x194[4];
        }
        this->StatDownRefund((&this->stat_body)[i]);
        this->field_0x174[i * 2 + 1] = this->field_0x194[8];
        if ((&this->stat_body)[i] > 0xF) {
            if (i == idx && button == 2) {
                if (mode & 1) {
                    this->field_0x174[i * 2 + 1] = this->field_0x194[5];
                } else {
                    this->field_0x174[i * 2 + 1] = this->field_0x194[6];
                }
            }
        } else {
            this->field_0x174[i * 2 + 1] = this->field_0x194[9];
        }
    }
    return hit;
}


// 42a778
int32_t VisCharGenStats::OnStatUp(int32_t idx) {
    int32_t cost = this->StatUpCost((&this->stat_body)[idx]);
    if (this->field_0x1f0 < cost) {
        return 0;
    }
    if ((&this->stat_body)[idx] >= 0x2D) {
        return 0;
    }
    (&this->stat_body)[idx] += 1;
    this->field_0x1f0 -= cost;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->m_GameSession.SetCharacterStats(this->stat_body, this->stat_reaction, this->stat_mind, this->stat_spirit,
                                              this->parent_screen->skills_panel->selected_slot + 1);
    FUN_00438f20(&this->parent_screen->snd_stat);
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
    this->field_0x1f0 += refund;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->m_GameSession.SetCharacterStats(this->stat_body, this->stat_reaction, this->stat_mind, this->stat_spirit,
                                              this->parent_screen->skills_panel->selected_slot + 1);
    FUN_00438f20(&this->parent_screen->snd_stat);
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
        this->field_0x194[i] = nullptr;
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
        this->areas[i * 3] = CRect(CPoint(0x52, (i << 5) + 0x36), CSize(0x14, 0x14));
        this->areas[i * 3 + 1] = CRect(CPoint(0x6B, (i << 5) + 0x36), CSize(0x14, 0x14));
        this->areas[i * 3 + 2] = CRect(CPoint(0x84, (i << 5) + 0x36), CSize(0x14, 0x14));
    }
    for (int32_t i = 0; i < 4; i++) {
        this->field_0x64[i] = CRect(CPoint(0x10, i * 0x21 + 0x39),
                                    CSize(g_font4->GetStrWidth(this->texts.ElementAt(i)), g_font4->GetHeight()));
    }
    this->field_0xa4 = CRect(CPoint(0x2E, 0xB5), CSize(0x4D, 0x16));
    this->field_0x1f0 = 0;
}

// 428f12
VisCharGenStats::~VisCharGenStats() {
    this->FreeBitmaps();
    this->parent_screen = nullptr;
}

// 428e51
VisCharGenStats::VisCharGenStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    this->Init();
    this->parent_screen = parent;
}


// 42abea
void VisCharGenFullStats::VMethod7() {
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    LockSurface2();
    this->bmp->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
    CRect unit_rect = screen_rect;
    unit_rect.left += 0xC;
    unit_rect.right += 0xC;
    this->parent_screen->current_char->FUN_0046c124(&unit_rect);
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
    return this->parent_screen->current_char->FUN_0046d0f7(pt.x, pt.y);
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
: CVisualObject(_id, l, t, r, b, nullptr)
{
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
    this->area_bmp->VMethod2(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, 0);
    for (int32_t i = 0; i < 3; i++) {
        uint16_t* pal;
        if (this->mouse_over_box == i) {
            pal = palette_paris_daisy->GetPalette(0);
        } else {
            pal = palette_husk->GetPalette(0);
        }
        if (this->mouse_over_box < 0 || this->mouse_down_box != this->mouse_over_box || this->mouse_over_box != i) {
            this->btn_off[i]->VMethod2(topleft.x + this->areas[i].left, topleft.y + this->areas[i].top, 0, 0, 0);
            g_font4->DrawTxt(topleft.x + this->areas[i].left + this->areas[i].Width() / 2,
                             topleft.y + this->areas[i].top + this->areas[i].Height() / 2,
                             this->texts.ElementAt(i), 10, pal);
        } else {
            this->btn_on[i]->VMethod2(topleft.x + this->areas[i].left, topleft.y + this->areas[i].top, 0, 0, 0);
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
: CVisualObject(_id, l, t, r, b, nullptr)
{
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
        this->field_0xb4[0] = CPoint(0xF8 + topleft.x, 0x5D + topleft.y);
        this->field_0xdc[0] = CPoint(0x8C, 0x2F);
        this->bmp_on[1] = new CBmp64("graphics\\interface\\chrgen\\fighter\\axe\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[1] = new CBmp64("graphics\\interface\\chrgen\\fighter\\axe\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[1] = new CBmp64("graphics\\interface\\chrgen\\fighter\\axe\\shine_on.bmp");
        g_mousept.Update();
        this->field_0xb4[1] = CPoint(0xFC + topleft.x, 0x7E + topleft.y);
        this->field_0xdc[1] = CPoint(0x84, 0x39);
        this->bmp_on[2] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Mace\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[2] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Mace\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[2] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Mace\\shine_on.bmp");
        g_mousept.Update();
        this->field_0xb4[2] = CPoint(0xF8 + topleft.x, 0xB6 + topleft.y);
        this->field_0xdc[2] = CPoint(0x8C, 0x2E);
        this->bmp_on[3] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Pike\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[3] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Pike\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[3] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Pike\\shine_on.bmp");
        g_mousept.Update();
        this->field_0xb4[3] = CPoint(0xF4 + topleft.x, 0xE1 + topleft.y);
        this->field_0xdc[3] = CPoint(0x94, 0x1C);
        this->bmp_on[4] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Bow\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[4] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Bow\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[4] = new CBmp64("graphics\\interface\\chrgen\\fighter\\Bow\\shine_on.bmp");
        g_mousept.Update();
        this->field_0xb4[4] = CPoint(0xF8 + topleft.x, 0xFA + topleft.y);
        this->field_0xdc[4] = CPoint(0x8C, 0x28);
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
        this->field_0xb4[0] = CPoint(0x168 + topleft.x, 0x96 + topleft.y);
        this->field_0xdc[0] = CPoint(0x2C, 0x34);
        this->bmp_on[1] = new CBmp64("graphics\\interface\\chrgen\\mag\\water\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[1] = new CBmp64("graphics\\interface\\chrgen\\mag\\water\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[1] = new CBmp64("graphics\\interface\\chrgen\\mag\\water\\shine_on.bmp");
        g_mousept.Update();
        this->field_0xb4[1] = CPoint(0xE8 + topleft.x, 0xA5 + topleft.y);
        this->field_0xdc[1] = CPoint(0x30, 0x24);
        this->bmp_on[2] = new CBmp64("graphics\\interface\\chrgen\\mag\\air\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[2] = new CBmp64("graphics\\interface\\chrgen\\mag\\air\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[2] = new CBmp64("graphics\\interface\\chrgen\\mag\\air\\shine_on.bmp");
        g_mousept.Update();
        this->field_0xb4[2] = CPoint(0x124 + topleft.x, 0x62 + topleft.y);
        this->field_0xdc[2] = CPoint(0x30, 0x26);
        this->bmp_on[3] = new CBmp64("graphics\\interface\\chrgen\\mag\\earth\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[3] = new CBmp64("graphics\\interface\\chrgen\\mag\\earth\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[3] = new CBmp64("graphics\\interface\\chrgen\\mag\\earth\\shine_on.bmp");
        g_mousept.Update();
        this->field_0xb4[3] = CPoint(0x12C + topleft.x, 0xE4 + topleft.y);
        this->field_0xdc[3] = CPoint(0x30, 0x26);
        this->bmp_on[4] = new CBmp64("graphics\\interface\\chrgen\\mag\\astral\\on.bmp");
        g_mousept.Update();
        this->bmp_shine_off[4] = new CBmp64("graphics\\interface\\chrgen\\mag\\astral\\shine_off.bmp");
        g_mousept.Update();
        this->bmp_shine_on[4] = new CBmp64("graphics\\interface\\chrgen\\mag\\astral\\shine_on.bmp");
        g_mousept.Update();
        this->field_0xb4[4] = CPoint(0x128 + topleft.x, 0x9E + topleft.y);
        this->field_0xdc[4] = CPoint(0x30, 0x2D);
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
    this->field_0x10c[4] |= 1;
    if (this->parent_screen->active_flag != 0) {
        LockSurface2();
        this->bmp_column->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
        for (int32_t i = 0; i < 4; i++) {
            int32_t state = this->field_0x10c[i];
            if (state == 1) {
                this->bmp_on[i]->VMethod10(this->field_0xb4[i].x, this->field_0xb4[i].y, 0, 0,
                                           this->field_0xdc[i].x, this->field_0xdc[i].y);
            } else if (state == 2) {
                this->bmp_shine_off[i]->VMethod10(this->field_0xb4[i].x, this->field_0xb4[i].y, 0, 0,
                                                  this->field_0xdc[i].x, this->field_0xdc[i].y);
            } else if (state == 3) {
                this->bmp_shine_on[i]->VMethod10(this->field_0xb4[i].x, this->field_0xb4[i].y, 0, 0,
                                                 this->field_0xdc[i].x, this->field_0xdc[i].y);
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
        this->field_0x10c[i] &= ~1;
    }
    this->field_0x10c[hit] |= 1;
    this->selected_slot = hit;
    CSound::Play((CSound&)this->field_0x120[hit]);
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
        FUN_00438e40(&this->field_0x120[0], "SFX\\ChrGen\\Skill\\FSword.wav");
        FUN_00438e40(&this->field_0x120[1], "SFX\\ChrGen\\Skill\\FAxe.wav");
        FUN_00438e40(&this->field_0x120[2], "SFX\\ChrGen\\Skill\\FClub.wav");
        FUN_00438e40(&this->field_0x120[3], "SFX\\ChrGen\\Skill\\FPike.wav");
        FUN_00438e40(&this->field_0x120[4], "SFX\\ChrGen\\Skill\\FBow.wav");
    } else {
        FUN_00438e40(&this->field_0x120[0], "SFX\\ChrGen\\Skill\\MFire.wav");
        FUN_00438e40(&this->field_0x120[1], "SFX\\ChrGen\\Skill\\MWater.wav");
        FUN_00438e40(&this->field_0x120[2], "SFX\\ChrGen\\Skill\\MAir.wav");
        FUN_00438e40(&this->field_0x120[3], "SFX\\ChrGen\\Skill\\MEarth.wav");
        FUN_00438e40(&this->field_0x120[4], "SFX\\ChrGen\\Skill\\MAstral.wav");
    }
}


// 42bfcc
int32_t VisCharGenSkills::OnMouseMove(uint32_t wparam, CPoint pos) {
    for (int32_t i = 0; i < 5; i++) {
        this->field_0x10c[i] &= ~2;
    }
    int32_t hit = this->HitTest(pos);
    if (hit == -1) {
        return this->CVisualObject::OnMouseMove(wparam, pos);
    }
    this->field_0x10c[hit] |= 2;
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
    FUN_00438dd0(&this->field_0x120[0]);
    FUN_00438dd0(&this->field_0x120[1]);
    FUN_00438dd0(&this->field_0x120[2]);
    FUN_00438dd0(&this->field_0x120[3]);
    FUN_00438dd0(&this->field_0x120[4]);
}


// 42c1ab
int32_t VisCharGenSkills::OnLButtonUp(uint32_t wparam, CPoint pos) {
    return this->CVisualObject::OnLButtonUp(wparam, pos);
}


// Blink-cycle state (binary: 659530, 659518, 62CCF4, 62CCF8, 659550, 65953C)
static uint32_t blink_hover_time = 0;
static uint32_t blink_cycle_time = 0;
static int32_t blink_hover_slot = -1;
static int32_t blink_cycle_len = 4;
static int32_t blink_slot = 0;
static uint8_t blink_inited = 0;

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
    if (this->field_0x10c[blink_slot] == 1) {
        this->bmp_shine_on[blink_slot]->VMethod10(this->field_0xb4[blink_slot].x, this->field_0xb4[blink_slot].y, 0, 0,
                                                  this->field_0xdc[blink_slot].x, this->field_0xdc[blink_slot].y);
    } else {
        this->bmp_shine_off[blink_slot]->VMethod10(this->field_0xb4[blink_slot].x, this->field_0xb4[blink_slot].y, 0, 0,
                                                   this->field_0xdc[blink_slot].x, this->field_0xdc[blink_slot].y);
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
        this->field_0x10c[i] = 0;
        this->field_0x120[i] = nullptr;
    }
    this->selected_slot = 0;
    this->field_0x10c[this->selected_slot] = 1;
}


// 42ba30
VisCharGenSkills::~VisCharGenSkills() {
    this->FreeBitmaps();
}

// 42b98c
VisCharGenSkills::VisCharGenSkills(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    this->parent_screen = parent;
    this->Init();
}


VisLogoWnd::VisLogoWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr)
{ //4cd4d0
    VisLogoWnd::VMethod26();
}

VisLogoWnd::~VisLogoWnd()
{ //4cd53c
    if (logoBitmap)
        delete logoBitmap;
}

void VisLogoWnd::VMethod7()
{ //4cd8d1
    drawPendingFlag = 0;

    LockSurface2();

    if (logoBitmap)
        logoBitmap->VMethod2(rect.left, rect.top, 0, 0, 0);

    UnlockSurface2();
}

int32_t VisLogoWnd::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{ //4cd72e

    static const char* logos_files[] =
    {
        "graphics\\interface\\logo\\SSS256.bmp",
        "graphics\\interface\\logo\\Buka256.bmp",
        "graphics\\interface\\logo\\Nival256.bmp",
        "graphics\\interface\\logo\\Allods.bmp",
        "graphics\\interface\\logo\\Intro.bmp",
        "graphics\\interface\\logo\\Web.bmp",
        "graphics\\interface\\logo\\splash1.bmp",
        "graphics\\interface\\logo\\splash2.bmp",
        "graphics\\interface\\logo\\splash3.bmp",
        "graphics\\interface\\logo\\splash4.bmp"
    };

    switch (msg)
    {
    case 0x402:
        if (drawPendingFlag)
            VMethod9();

        if (timeoutTicks < timeGetTime() - timeoutStart)
            AfxGetMainWnd()->PostMessage(0x438, step, 0);
        break;

    case 0x408:
        drawPendingFlag = 1;
        break;

    case 0x439:
        timeoutTicks = wparam;
        timeoutStart = timeGetTime();
        break;

    case 0x43a:
        step = wparam;
        logoBitmap = new CBmp256(logos_files[wparam]);
        logoBitmap->ResetPalette(1, 1, 0);
        break;
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
    return 1;
}

int32_t VisLogoWnd::OnLButtonDown(uint32_t wparam, CPoint pos)
{ //4cd6c2
    AfxGetMainWnd()->PostMessage(0x438, step, 0);
    timeoutTicks = 0x7fffffff;
    return 1;
}

int32_t VisLogoWnd::OnKeyDown(uint32_t wparam)
{ //4cd6f8
    AfxGetMainWnd()->PostMessage(0x438, step, 0);
    timeoutTicks = 0x7fffffff;
    return 1;
}

void VisLogoWnd::VMethod26()
{ //4cd5be
    timeoutTicks = 0x7fffffff;
    logoBitmap = nullptr;
    AddChild(new VisButton(4, 0, 0, 0, 0, "", g_font1, clrsh_TechBlack, 0x445, 0, nullptr)); // WTF?
}

void VisLogoWnd::DoClose(uint32_t code)
{ //4cd666 29 method   
    if (logoBitmap)
    {
        delete logoBitmap;
        logoBitmap = nullptr;
    }
    VisScreen::DoClose(code);
}

// 4B2B3D
void VisCharInfo::VMethod7()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if ((main_wnd->dialogsMask & 0x627) == 0) {
        return;
    }

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    int32_t moved_up;
    if (g_ScreenSize.bottom - screen_rect.bottom > screen_rect.Height() && main_wnd->dialogsMask == 1) {
        moved_up = 1;
    } else {
        moved_up = 0;
    }

    BigStruct2* map = this->map_context;
    LockSurface2();
    if (moved_up != 0) {
        this->info_mode = 1;
    }

    if (this->info_mode != 0) {
        g_bmp_humanbackr->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
    } else {
        g_bmp_textbackr->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
    }

    if ((main_wnd->dialogsMask & 3) != 0 && (main_wnd->dialogsMask & 4) == 0) {
        if (map->IsBookOpen()) {
            g_bmp_bookopened->VMethod10(screen_rect.left, screen_rect.top, 0, 0, 0x1C, 0x26);
        } else {
            g_bmp_bookclosed->VMethod10(screen_rect.left, screen_rect.top + 4, 0, 0, 0x1C, 0x25);
        }
    }

    if ((main_wnd->dialogsMask & 0x226) != 0) {
        if ((main_wnd->dialogsMask & 0x200) == 0 || main_wnd->sessionMode != 2) {
            g_bmp_ar1->VMethod10(screen_rect.left + 1, screen_rect.top + 0xCD, 0, 0, 0x20, 0x20);
            g_bmp_ar2->VMethod10(screen_rect.left + 0x77, screen_rect.top + 0xCD, 0, 0, 0x20, 0x20);
        }
    } else if ((main_wnd->dialogsMask & 0x400) == 0) {
        if (map->IsBagOpen()) {
            g_bmp_backpackop->VMethod10(screen_rect.left, screen_rect.top + 0xD0, 0, 0, 0x20, 0x1F);
        } else {
            g_bmp_backpackcl->VMethod10(screen_rect.left + 1, screen_rect.top + 0xC9, 0, 0, 0x1C, 0x1E);
        }
        g_bmp_diskette->VMethod10(screen_rect.left + 0x7E, screen_rect.top + 0xCE, 0, 0, 0x20, 0x20);
    }

    if ((main_wnd->dialogsMask & 0x600) == 0 && moved_up == 0) {
        if (this->info_mode != 0) {
            g_bmp_humanmode->VMethod10(screen_rect.left + 0x80, screen_rect.top + 4, 0, 0, 0x1C, 0x20);
        } else {
            g_bmp_textmode->VMethod10(screen_rect.left + 0x80, screen_rect.top + 4, 0, 0, 0x1C, 0x20);
        }
    }

    CStructure* structure = nullptr;
    CUnit* unit = nullptr;
    uint16_t selected_id = (uint16_t)map->field_0x9a8;
    CGameObject* found_obj = nullptr;
    int32_t found = 0;
    if (selected_id != 0 && main_wnd->field_0x408 == nullptr && main_wnd->dialogsMask == 1) {
        found = map->field_0x9d0.Lookup(selected_id, found_obj);
    }
    if (selected_id != 0 && found != 0) {
        if (found_obj->IsKindOf(RUNTIME_CLASS(CStructure))) {
            structure = (CStructure*)found_obj;
        } else {
            unit = (CUnit*)found_obj;
        }
    } else if (map->field_0x140 == 1) {
        if (map->field_0x138->IsKindOf(RUNTIME_CLASS(CStructure))) {
            structure = (CStructure*)map->field_0x138;
        } else {
            unit = (CUnit*)map->field_0x138;
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
            this->bitmap->VMethod10(screen_rect.left, screen_rect.top + 2, 0, 0, 0xA0, 0xF0);
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
            unit->FUN_0046c124(&screen_rect);
        } else {
            if ((unit->unitFlags & 0x11) != 0) {
                char temp_path[0x100];
                char fname[0x100];
                char full_path[0x100];
                GetTempPathA(0x100, temp_path);
                sprintf(fname, "allods-2-%d.$$$", unit->unit_id);
                sprintf(full_path, "%s%s", temp_path, fname);
                if (strcmp(this->picturename, fname) != 0) {
                    strcpy(this->picturename, fname);
                    if ((unit->unitFlags & 8) != 0) {
                        UnlockSurface2();
                        unit->VMethod30(full_path, this->bitmap, this->hitmap);
                        LockSurface2();
                    } else {
                        this->bitmap->LoadFile(fname, this->hitmap);
                    }
                } else if ((unit->unitFlags & 8) != 0) {
                    UnlockSurface2();
                    unit->VMethod30(full_path, this->bitmap, this->hitmap);
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
            this->bitmap->VMethod10(screen_rect.left, screen_rect.top + 2, 0, 0, 0xA0, 0xF0);
            if (map->quest_some_id_2 != 0 || map->quest_some_id != 0) {
                g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x36, TxtFile::AllLines[0x159], 2, clrsh_DullGold, 1);
                g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x42, TxtFile::AllLines[0x15A], 2, clrsh_DullGold, 1);
            }
        }
    } else {
        if (map->field_0x140 > 1) {
            g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x36, TxtFile::AllLines[0x31], 2, clrsh_DullGold, 1);
            g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x42, TxtFile::AllLines[0x32], 2, clrsh_DullGold, 1);
            char count_text[0x10];
            sprintf(count_text, "%d", map->field_0x140);
            g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x4E, count_text, 2, clrsh_DullGold, 1);
        } else if (map->field_0x140 == 0) {
            g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x36, TxtFile::AllLines[0x2F], 2, clrsh_DullGold, 1);
            g_font2->DrawTextWithShadow(screen_rect.left + 0x48, screen_rect.top + 0x42, TxtFile::AllLines[0x30], 2, clrsh_DullGold, 1);
        }
    }

    UnlockSurface2();
    this->dirty = 0;
}

// 4B19B0
const char* VisCharInfo::GetHint()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        return nullptr;
    }
    if ((main_wnd->dialogsMask & 8) != 0) {
        return nullptr;
    }

    BigStruct2* map = this->map_context;
    CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    int32_t moved_up;
    if (g_ScreenSize.bottom - screen_rect.bottom > screen_rect.Height() && main_wnd->dialogsMask == 1) {
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

    if ((main_wnd->dialogsMask & 3) != 0 && (main_wnd->dialogsMask & 4) == 0) {
        if (book_rect.PtInRect(mouse_pt)) {
            if (map->IsBookOpen()) {
                return TxtFile::AllLines[9];
            }
            return TxtFile::AllLines[8];
        }
    }

    if ((main_wnd->dialogsMask & 1) != 0) {
        if (backpack_rect.PtInRect(mouse_pt)) {
            if ((main_wnd->dialogsMask & 2) == 0 && (main_wnd->dialogsMask & 4) == 0) {
                if (map->IsBagOpen()) {
                    return TxtFile::AllLines[0xB];
                }
                return TxtFile::AllLines[0xA];
            }
        }
    }

    if ((main_wnd->dialogsMask & 0x600) == 0) {
        if (mode_btn_rect.PtInRect(mouse_pt) && moved_up == 0) {
            if (this->info_mode != 0) {
                return TxtFile::AllLines[0xD];
            }
            return TxtFile::AllLines[0xC];
        }
    }

    if ((main_wnd->dialogsMask & 0x226) != 0) {
        if ((main_wnd->dialogsMask & 0x200) == 0 || main_wnd->sessionMode != 2) {
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
    } else if ((main_wnd->dialogsMask & 0x400) == 0) {
        if (diskette_rect.PtInRect(mouse_pt)) {
            return TxtFile::AllLines[0xE];
        }
    }

    CGameObject* sel = nullptr;
    if (map->field_0x140 == 1) {
        sel = map->field_0x138;
    }
    if (sel == nullptr) {
        return nullptr;
    }
    if ((main_wnd->dialogsMask & 8) != 0) {
        return nullptr;
    }

    CUnit* unit = (CUnit*)sel;
    if (unit->map_player != nullptr && unit->map_player->index != 0) {
        if (map->my_main_unit->FUN_0041ee50(unit->map_player->index) == 0) {
            if (this->info_mode != 0) {
                return nullptr;
            }
            return unit->FUN_0046d0f7(mouse_pt.x - screen_rect.left, mouse_pt.y - screen_rect.top - 2);
        }
    }

    if (this->info_mode == 0) {
        return unit->FUN_0046d0f7(mouse_pt.x - screen_rect.left, mouse_pt.y - screen_rect.top - 2);
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
    return entry->FUN_00439973();
}

// 4B3A0D
int32_t VisCharInfo::VMethod27(int32_t a)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    int32_t disallow = 0;
    if (main_wnd->field_0x408 == nullptr) {
        disallow = 1;
    }

    BigStruct2* map = this->map_context;
    if (map->field_0x140 != 1) {
        disallow = 1;
    }

    CUnit* unit = (CUnit*)map->field_0x138;
    if (unit->map_player != map->my_main_unit) {
        return 0;
    }

    if (a < 2 && (unit->last_action == 3 || unit->last_action == 7 || unit->last_action == 8)) {
        disallow = 1;
    }

    TokenEntry* token = main_wnd->field_0x408;
    if ((token->flg & 6) == 0 && token->item_id == 0xE4D) {
        main_wnd->PostMessageA(0x463, 0, 0);
    }

    if (unit->FUN_0046c0c9(token) == 0) {
        disallow = 1;
    }
    if (token->sub_43A6D5() == 0) {
        disallow = 1;
    }

    int32_t spell_id = token->GetAttribute(0x2A);
    if (spell_id != 0) {
        if ((unit->unitFlags & 2) != 0) {
            if (((1 << spell_id) & unit->spells) == 0) {
                disallow = 1;
            }
        } else {
            disallow = 1;
        }
    }

    if (disallow != 0) {
        main_wnd->vis_invtype1->VMethod37(main_wnd->field_0x40c);
        return 0;
    }

    if ((token->flg & 0x10) != 0 && (token->flg & 1) != 0) {
        int32_t removed = main_wnd->vis_invtype1->VMethod37(main_wnd->field_0x40c);
        main_wnd->vis_spellbook->sub_4CA925(removed);
        map->sub_418F93(0xA);
        return 0;
    }

    if ((token->flg & 1) != 0) {
        TokenEntry* split = nullptr;
        if (token->field_0x10 > 1) {
            split = new TokenEntry(token);
            split->field_0x10 -= 1;
            token->field_0x10 = 1;
        }

        int32_t amount = token->field_0x10;
        int32_t mode = main_wnd->OnCommand(a + 1, 0);
        map->sub_41A7C7(main_wnd->field_0x410, main_wnd->field_0x40c, mode, a + 1, amount);

        if (split != nullptr) {
            main_wnd->vis_invtype1->VMethod37(main_wnd->field_0x40c);
            return 0;
        }
        if (main_wnd->field_0x408 != nullptr) {
            delete main_wnd->field_0x408;
        }
        main_wnd->sub_48CD44();
        return 0;
    }

    TokenEntry* old_token = unit->equipmentTokens[a];
    if (old_token != nullptr) {
        main_wnd->vis_invtype1->VMethod26(old_token, main_wnd->field_0x40c);
        main_wnd->vis_invtype1->VMethod33(unit);
    }

    if (a == 0) {
        if (token->sub_4396FB() != 0 && unit->equipmentTokens[1] != nullptr) {
            main_wnd->vis_invtype1->VMethod26(unit->equipmentTokens[1], main_wnd->field_0x40c);
            main_wnd->vis_invtype1->VMethod33(unit);
            unit->equipmentTokens[1] = nullptr;
        }
    }

    if (a == 1) {
        if (unit->equipmentTokens[0] != nullptr && unit->equipmentTokens[0]->sub_4396FB() != 0) {
            main_wnd->vis_invtype1->VMethod26(unit->equipmentTokens[0], main_wnd->field_0x40c);
            main_wnd->vis_invtype1->VMethod33(unit);
            unit->equipmentTokens[0] = nullptr;
        }
    }

    unit->unitFlags |= 8;
    this->MsgProc(0x408, 0, 0);
    unit->equipmentTokens[a] = main_wnd->field_0x408;

    int32_t amount = main_wnd->field_0x408->field_0x10;
    int32_t mode = main_wnd->OnCommand(a + 1, 0);
    map->sub_41A7C7(main_wnd->field_0x410, main_wnd->field_0x40c, mode, a + 1, amount);

    main_wnd->sub_48CD44();
    unit->ReloadSprite();
    main_wnd->vis_root->MsgProc(0x46E, (uint32_t)main_wnd->m_hWnd, 0);
    return (int32_t)old_token;
}

// 4B2346
int32_t VisCharInfo::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    CRect screen_rect;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->ClientRectToScreen(&screen_rect, this->rect);

    int32_t moved_up;
    if (g_ScreenSize.bottom - screen_rect.bottom > screen_rect.Height() && main_wnd->dialogsMask == 1) {
        moved_up = 1;
    } else {
        moved_up = 0;
    }

    BigStruct2* map = this->map_context;

    CRect backpack_rect(screen_rect.left, screen_rect.bottom - 0x28, screen_rect.left + 0x1C, screen_rect.bottom);
    CRect ar1_rect(screen_rect.left + 1, screen_rect.top + 0xCD, screen_rect.left + 0x21, screen_rect.top + 0xED);
    CRect ar2_rect(screen_rect.left + 0x77, screen_rect.top + 0xCD, screen_rect.left + 0x97, screen_rect.top + 0xED);
    CRect diskette_rect(screen_rect.left + 0x7E, screen_rect.top + 0xCE, screen_rect.left + 0x9E, screen_rect.top + 0xEE);

    if (main_wnd->field_0x408 != nullptr) {
        TokenEntry* token = main_wnd->field_0x408;
        if (token->field_0x18 == 2 || token->field_0x18 == 1) {
            g_Cursors[0]->Use();
            this->VMethod27(token->GetType() - 1);
            return 1;
        }
        if (token->field_0x18 >= 5 && token->field_0x18 <= 8) {
            ((VisShop*)main_wnd->vis_root->FindChild(0x3E8))->sub_4BC97B();
            return 1;
        }
    }

    if (main_wnd->dialogsMask == 1) {
        if (backpack_rect.PtInRect(pos)) {
            this->MsgProc(0x40E, 0, 0);
            map->MsgProc(0x40E, 0, 0);
        }
    }

    CRect book_rect(screen_rect.left, screen_rect.top, screen_rect.left + 0x1C, screen_rect.top + 0x24);
    if ((main_wnd->dialogsMask & 3) != 0 && (main_wnd->dialogsMask & 4) == 0) {
        if (book_rect.PtInRect(pos)) {
            this->MsgProc(0x40F, 0, 0);
            map->MsgProc(0x40F, 0, 0);
            if ((main_wnd->dialogsMask & 2) != 0) {
                main_wnd->vis_root->FindChild(0x3E8)->MsgProc(0x40F, 0, 0);
            }
        }
    }

    CRect mode_btn_rect(screen_rect.left + 0x80, screen_rect.top, screen_rect.right, screen_rect.top + 0x24);
    if ((main_wnd->dialogsMask & 0x600) == 0) {
        if (mode_btn_rect.PtInRect(pos) && moved_up == 0) {
            this->MsgProc(0x412, 0, 0);
            map->field_0xe0 = 1;
        }
    }

    if ((main_wnd->dialogsMask & 0x226) != 0) {
        if (ar1_rect.PtInRect(pos)) {
            g_SfxArray[1]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xDC, 0);
            main_wnd->vis_root->MsgProc(0x414, 0, 0);
        }
        if (ar2_rect.PtInRect(pos)) {
            g_SfxArray[1]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xDC, 0);
            main_wnd->vis_root->MsgProc(0x415, 0, 0);
        }
    }

    if ((main_wnd->dialogsMask & 1) != 0) {
        if (diskette_rect.PtInRect(pos)) {
            g_SfxArray[1]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xDC, 0);
            main_wnd->PostMessageA(0x416, 0, 0);
        }
    }
    return 1;
}

// 4B289E
int32_t VisCharInfo::OnMouseMove(uint32_t wparam, CPoint pos)
{
    if ((wparam & 1) == 0) {
        return 0;
    }

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    BigStruct2* map = this->map_context;

    if (main_wnd->field_0x408 != nullptr) {
        return 0;
    }
    if (this->info_mode == 0) {
        return 0;
    }

    CUnit* unit = (CUnit*)map->field_0x138;
    if (map->field_0x140 != 1) {
        return 0;
    }
    if ((unit->unitFlags & 1) == 0) {
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

    CString name = main_wnd->field_0x408->FUN_004394f3();
    CString path = "graphics\\inventory\\" + name + ".16a";
    main_wnd->sub_48CCA1(main_wnd->field_0x408, main_wnd->field_0x40c, path, main_wnd->field_0x410);

    unit->unitFlags |= 8;
    this->MsgProc(0x408, 0, 0);
    this->parent->MsgProc(0x46E, this->id, 0);
    unit->m_bSelectionDirty = 1;
    return 0;
}

// 4B38AF
int32_t VisCharInfo::VMethod26(int32_t a)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        return 0;
    }
    if ((main_wnd->dialogsMask & 2) == 0 && main_wnd->dialogsMask != 1) {
        return 0;
    }

    BigStruct2* map = this->map_context;
    if (map->field_0x140 != 1) {
        return 0;
    }

    CUnit* unit = (CUnit*)map->field_0x138;
    if (unit->map_player != map->my_main_unit) {
        return 0;
    }

    if (a < 2 && (unit->last_action == 3 || unit->last_action == 7 || unit->last_action == 8)) {
        return 0;
    }

    unit->unitFlags |= 8;
    this->MsgProc(0x408, 0, 0);
    main_wnd->field_0x408 = unit->equipmentTokens[a];
    unit->equipmentTokens[a] = nullptr;
    main_wnd->field_0x40c = a;
    main_wnd->field_0x410 = main_wnd->OnCommand(0, 0);
    unit->ReloadSprite();
    main_wnd->vis_root->MsgProc(0x46E, (uint32_t)main_wnd->m_hWnd, 0);
    return (int32_t)main_wnd->field_0x408;
}

// 4B2239
int32_t VisCharInfo::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    uint8_t* data = (uint8_t*)this->hitmap->GetData();
    BigStruct2* map = this->map_context;

    uint8_t color = data[(pos.y - screen_rect.top - 2) * 0xA0 + (pos.x - screen_rect.left)];
    if (color == 0) {
        return 1;
    }

    CUnit* unit = (CUnit*)map->field_0x138;
    if (map->field_0x140 != 1) {
        return 0;
    }
    if ((unit->unitFlags & 1) == 0) {
        return 0;
    }

    if (this->VMethod26(color - 1) != 0) {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        int32_t slot = main_wnd->vis_invtype1->FUN_0046fb90();
        main_wnd->vis_invtype1->VMethod37(slot);
    }
    return 1;
}

// 4B2AD5
int32_t VisCharInfo::OnKeyDown(uint32_t wparam)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (wparam != 9) {
        return 0;
    }

    if ((main_wnd->dialogsMask & 0x400) == 0 && (main_wnd->dialogsMask & 0x200) == 0) {
        this->MsgProc(0x412, 0, 0);
    }
    return 1;
}

// 4B2857
int32_t VisCharInfo::OnRButtonUp(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    BigStruct2* map = this->map_context;
    if (main_wnd->dialogsMask == 1) {
        return map->MsgProc(0x405, 0, 0);
    }
    return 1;
}

// 4B2215
int32_t VisCharInfo::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    return 1;
}

// 4B2227
int32_t VisCharInfo::OnRButtonDown(uint32_t wparam, CPoint pos)
{
    return 1;
}

// 4B2334
int32_t VisCharInfo::OnRButtonDblClk(uint32_t wparam, CPoint pos)
{
    return 1;
}

// 4B4830
int32_t VisCharInfo::VMethod28()
{
    return 1;
}

// 4B16C3
VisCharInfo::VisCharInfo(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : CVisualObject(_id, l, t, r, b, nullptr)
{
    this->field_64 = 0;
    this->dirty = 0;
    this->selection_panel_state = 0;
    this->spell_panel_state = 0;
    this->info_mode = 1;
    this->bitmap = new CBmp64(0xa0, 0xf0);
    this->picturename[0] = 0;
    this->hitmap = new CBmp256(0xa0, 0xf0);
}

// 4B1909
VisCharInfo::~VisCharInfo()
{
    delete this->bitmap;
    delete this->hitmap;
}

// 4B201C
int32_t VisCharInfo::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    int32_t result = CVisualObject::MsgProc(msg, wparam, lparam);

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    int32_t moved_up;
    if (g_ScreenSize.bottom - screen_rect.bottom > screen_rect.Height() && main_wnd->dialogsMask == 1) {
        moved_up = 1;
    } else {
        moved_up = 0;
    }

    if (result == 0) {
        switch (msg) {
        case 0x403:
            this->map_context = (BigStruct2*)wparam;
            this->dirty = 1;
            break;
        case 0x408:
            this->dirty = 1;
            break;
        case 0x402:
            if (main_wnd->dialogsMask == 1 || main_wnd->dialogsMask == 3 || main_wnd->dialogsMask == 5) {
                if (this->dirty != 0) {
                    this->VMethod9();
                }
            }
            if ((main_wnd->dialogsMask & 3) != 0) {
                this->sub_4B36B4();
            }
            break;
        case 0x40E:
            this->dirty = 1;
            this->selection_panel_state = (this->selection_panel_state == 0);
            break;
        case 0x40F:
            this->dirty = 1;
            this->spell_panel_state = (this->spell_panel_state == 0);
            break;
        case 0x410:
            this->dirty = 1;
            break;
        case 0x412:
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
void VisCharInfo::sub_4B36B4()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CSprite256* cur_sprite = g_mousept.GetCursorSprite();

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    CCursor* new_cursor = nullptr;
    CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
    if (!screen_rect.PtInRect(mouse_pt)) {
        return;
    }

    if ((main_wnd->dialogsMask & 2) != 0) {
        if (main_wnd->field_0x408 != nullptr) {
            VisShop* shop = (VisShop*)main_wnd->vis_root->FindChild(0x3E8);
            TokenEntry* token = main_wnd->field_0x408;
            if (shop->placement_lock != 0 || token->field_0x18 == 1 || token->field_0x18 == 2) {
                new_cursor = main_wnd->item_cursor;
            } else {
                new_cursor = g_Cursors[CURSOR_CANTPUT];
            }
        }
    } else {
        if (mouse_pt.x >= g_ScreenSize.right - 2 && main_wnd->dialogsMask == 1) {
            if (mouse_pt.y == 0) {
                new_cursor = g_Cursors[CURSOR_ARROW1];
            } else if (mouse_pt.y >= g_ScreenSize.bottom - 2) {
                new_cursor = g_Cursors[CURSOR_ARROW3];
            } else {
                new_cursor = g_Cursors[CURSOR_ARROW2];
            }
        } else if (mouse_pt.y >= g_ScreenSize.bottom - 2 && main_wnd->dialogsMask == 1) {
            new_cursor = g_Cursors[CURSOR_ARROW4];
        } else {
            if (screen_rect.PtInRect(mouse_pt)) {
                new_cursor = g_Cursors[CURSOR_DEFAULT];
            }
        }
        if (main_wnd->field_0x408 != nullptr) {
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

// 4B4190
void VisSideStatus::VMethod7()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->dialogsMask != 1) {
        return;
    }

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    BigStruct2* map = main_wnd->vis_map_context;
    LockSurface2();
    if (screen_rect.Height() >= main_wnd->vis_charinfo->GetRect().Height()) {
        screen_rect.OffsetRect(0, screen_rect.Height() - main_wnd->vis_charinfo->GetRect().Height());
        g_bmp_textbackr->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);

        CStructure* structure = nullptr;
        CUnit* unit = nullptr;
        uint16_t selected_id = (uint16_t)map->field_0x9a8;
        CGameObject* found_obj = nullptr;
        int32_t found = 0;
        if (selected_id != 0 && main_wnd->field_0x408 == nullptr) {
            found = map->field_0x9d0.Lookup(selected_id, found_obj);
        }
        if (selected_id != 0 && found != 0) {
            if (found_obj->IsKindOf(RUNTIME_CLASS(CStructure))) {
                structure = (CStructure*)found_obj;
            } else {
                unit = (CUnit*)found_obj;
            }
        } else if (map->field_0x140 == 1) {
            if (map->field_0x138->IsKindOf(RUNTIME_CLASS(CStructure))) {
                structure = (CStructure*)map->field_0x138;
            } else {
                unit = (CUnit*)map->field_0x138;
            }
        }

        if (structure != nullptr) {
            g_font2->DrawTextWithShadow(screen_rect.right - 0x58, screen_rect.top + 0x1C, txt_building.GetLine(structure->typeId - 1), 2, clrsh_DullGold, 1);
            g_font2->DrawTextWithShadow(screen_rect.right - 0x58, screen_rect.top + 0x2C, TxtFile::AllLines[0x13], 2, clrsh_DullGold, 1);
            char hp_text[0x10];
            sprintf(hp_text, "%d/%d", structure->hp, structure->hp_max);
            g_font2->DrawTextWithShadow(screen_rect.right - 0x58, screen_rect.top + 0x36, hp_text, 2, clrsh_Oxley, 1);
        } else if (unit != nullptr) {
            unit->FUN_0046c124(&screen_rect);
        }

        screen_rect.OffsetRect(0, main_wnd->vis_charinfo->GetRect().Height() - screen_rect.Height());
    }

    if (g_ScreenSize.bottom > 600) {
        g_bmp_extra1024r->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
    } else if (g_ScreenSize.bottom > 480) {
        g_bmp_extra800r->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
    }
    UnlockSurface2();
    this->dirty = 0;
}

// 4B4490
int32_t VisSideStatus::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    int32_t result = CVisualObject::MsgProc(msg, wparam, lparam);

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    int32_t moved_up = (screen_rect.Height() >= main_wnd->vis_charinfo->GetRect().Height()) ? 1 : 0;

    if (result == 0) {
        switch (msg) {
        case 0x402:
            if (main_wnd->dialogsMask == 1 && moved_up != 0 && this->dirty != 0) {
                this->VMethod9();
            }
            if (main_wnd->dialogsMask == 1) {
                this->UpdateCursor();
            }
            break;
        case 0x403:
        case 0x408:
        case 0x410:
            this->dirty = 1;
            break;
        }
    }
    return result;
}

// 4B45BB
const char* VisSideStatus::GetHint()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        return nullptr;
    }

    BigStruct2* map = main_wnd->vis_map_context;
    CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    CUnit* unit = nullptr;
    int32_t moved_up = (screen_rect.Height() >= main_wnd->vis_charinfo->GetRect().Height()) ? 1 : 0;
    if (map->field_0x140 == 1) {
        unit = (CUnit*)map->field_0x138;
    }
    if (unit == nullptr || moved_up == 0) {
        return nullptr;
    }

    int32_t hint_y = mouse_pt.y - screen_rect.TopLeft().y - 2 - (this->rect.Height() - main_wnd->vis_charinfo->GetRect().Height());
    int32_t hint_x = mouse_pt.x - screen_rect.TopLeft().x;
    return unit->FUN_0046d0f7(hint_x, hint_y);
}

// 4B3FA3
VisSideStatus::VisSideStatus(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : CVisualObject(_id, l, t, r, b, nullptr)
{
    this->dirty = 0;
}

// 4B4810
VisSideStatus::~VisSideStatus()
{
}

// 4BA5A0
VisShop::~VisShop()
{
    this->sub_4BB4FB();
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
void VisShop::FUN_004bcd02()
{
    if (this->spell_panel != nullptr) {
        this->sub_4BCD79();
        this->gameplay->FUN_0041b636();
    }
    this->sub_4BCC38();
    this->MsgProc(0x445, 0, 0);
}


// 4BA832
int32_t VisShop::OnKeyDown(uint32_t wparam)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (wparam == 0xd) {
        return 1;
    }
    if (wparam == 0x1b) {
        this->FUN_004bcd02();
        if (main_wnd->dialogsMask == 2) {
            main_wnd->PostMessage(0x42e, 0, 0);
        }
        return 1;
    }
    return 0;
}


// 4BB045
int32_t VisShop::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr && this->spell_panel != nullptr) {
        if (this->spell_panel->GetRect().PtInRect(pos)) {
            this->sub_4BC97B();
            return 1;
        }
    }
    if (main_wnd->field_0x408 != nullptr && !this->rect.PtInRect(pos)) {
        this->sub_4BC97B();
        return 1;
    }
    return CVisualObject::OnLButtonUp(wparam, pos);
}


// 4BB79A
void VisShop::VMethod8(CRect* rect)
{
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
    : VisScreen(_id, l, t, r, b, btm)
{
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
void VisShop::VMethod31()
{
    this->VMethod32();
    FUN_00438e40(&this->snd_notif, "SFX\\Town\\Shop\\nofit.wav");
    FUN_00438e40(&this->snd_step1, "SFX\\Town\\Shop\\step1.wav");
    FUN_00438e40(&this->snd_step2, "SFX\\Town\\Shop\\step2.wav");
    FUN_00438e40(&this->snd_breath, "SFX\\Town\\Shop\\breath.wav");
    FUN_00438e40(&this->snd_depart, "SFX\\Town\\Shop\\depart.wav");
    FUN_00438e40(&this->snd_buy, "SFX\\Town\\buy.wav");
    FUN_00438e40(&this->snd_sell, "SFX\\Town\\sell.wav");
    FUN_00438e40(&this->snd_enter, "SFX\\Town\\Shop\\enter.wav");
    FUN_00438e40(&this->snd_start, "SFX\\Town\\Shop\\start.wav");
    FUN_00438e40(&this->snd_pov1, "SFX\\Town\\Shop\\Povorot1.wav");
    FUN_00438e40(&this->snd_pov2, "SFX\\Town\\Shop\\Povorot2.wav");
    FUN_00438e40(&this->snd_inshop, "SFX\\Town\\Shop\\InShop.wav");
    FUN_00438e40(&this->snd_out, "SFX\\Out.wav");
    FUN_00438e40(&this->snd_undo, "SFX\\Undo.wav");
}


// 4BCF2F
void VisShop::VMethod30()
{
    static uint8_t snd_timing_init = 0; // byte_665DA8 in asm
    static uint32_t last_snd_time; // dword_665DA0 in asm

    if ((snd_timing_init & 1) == 0) {
        snd_timing_init |= 1;
        last_snd_time = timeGetTime();
    }
    uint32_t now = timeGetTime();
    uint32_t delay = GetRandS16(30000) + 30000;
    uint32_t compass_flags = this->shop_compass->state;
    if ((compass_flags & 0x10) == 0 && (compass_flags & 0x20) == 0 && (compass_flags & 0x40) == 0) {
        if (now - last_snd_time > delay && !FUN_00475110(&this->to_buy->field_0x20b0)) {
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
void VisShop::DoClose(uint32_t code)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    this->VMethod9();
    this->dialog_active = 0;
    this->VMethod32();

    if (main_wnd->field_0x408 != nullptr) {
        this->sub_4BC97B();
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
    this->sub_4BB4FB();
    this->assortiment->sub_4B7859();
    this->assortiment->sub_4B4C1C();
    this->to_sell->sub_4B4C1C();
    this->to_buy->sub_4B4C1C();

    VisScreen::DoClose(code);

    if (main_wnd->sessionMode == 2) {
        ScenarioLeaveShop();
    }
}


// 4BADCB
int32_t VisShop::OnMouseMove(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    pos -= CPoint(this->rect.left, this->rect.top);

    if (main_wnd->field_0x408 != nullptr) {
        TokenEntry* item = main_wnd->field_0x408;
        bool reset = false;

        if ((this->to_sell->GetRect().PtInRect(pos) || this->select_info_panel->GetRect().PtInRect(pos))
            && (int32_t)item->field_0x18 >= 5 && (int32_t)item->field_0x18 <= 8) {
            reset = true;
        }
        if (!reset && this->assortiment->GetRect().PtInRect(pos)
            && (item->field_0x18 == 2 || (int32_t)item->field_0x18 <= 1)) {
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
        this->sub_4BCEA4();
    }

    return 0;
}


// 4BBD75
void VisShop::VMethod26()
{
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
int32_t VisShop::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    case 0x402:
        if ((main_wnd->dialogsMask & 8) == 0) {
            this->sub_4BC8D7();
            this->shop_compass->VMethod9();
            this->to_sell->VMethod9();
            this->assortiment->VMethod9();
            if (this->spell_panel != nullptr) {
                this->spell_panel->VMethod9();
            } else {
                this->to_buy->VMethod9();
            }
            this->select_info_panel->VMethod9();
            this->sub_4BCDA0();
            this->buttons->VMethod9();
        }
        break;
    case 0x40F:
        if (this->spell_panel != nullptr) {
            this->sub_4BCD79();
        } else {
            this->sub_4BCD4B();
        }
        break;
    case 0x413:
        if (lparam == 0) {
            this->to_sell->sub_4B4D33();
            this->assortiment->sub_4B4D33();
            this->to_buy->sub_4B4D33();
            ((CUnit*)this->gameplay->field_0x138)->unitFlags |= 8;
            this->dirty |= 0x28;
        } else {
            switch (wparam) {
            case 1:
                ((CUnit*)this->gameplay->field_0x138)->unitFlags |= 8;
                this->dirty |= 8;
                break;
            case 2:
                this->to_sell->VMethod40((CArray<TokenEntry*>*)lparam);
                this->to_sell->sub_4B4D33();
                break;
            case 4:
                this->to_buy->VMethod40((CArray<TokenEntry*>*)lparam);
                this->to_buy->sub_4B4D33();
                this->sub_4BCDA0();
                this->dirty |= 0x20;
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                this->sub_4BBBD6(wparam - 5, (CArray<TokenEntry*>*)lparam);
                this->assortiment->sub_4B4D33();
                break;
            }
            ((CArray<TokenEntry*>*)lparam)->RemoveAll();
        }
        break;
    case 0x414:
        this->sub_4BBA06();
        this->dirty |= 1;
        break;
    case 0x415:
        this->sub_4BB895();
        this->dirty |= 1;
        break;
    case 0x45A:
        if (this->tips != nullptr) {
            if (this->shop_compass != nullptr) {
                this->shop_compass->RemoveChild(this->tips);
            }
            delete this->tips;
            this->tips = nullptr;
        }
        break;
    case 0x46E:
        if (wparam == 7) {
            this->to_sell->sub_4B4D33();
            this->to_buy->sub_4B4D33();
        } else {
            switch (wparam) {
            case 1:
            case 2:
                this->to_sell->sub_4B4D33();
                break;
            case 4:
                this->to_buy->sub_4B4D33();
                this->sub_4BCDA0();
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                this->assortiment->sub_4B4D33();
                break;
            }
        }
        break;
    }

    return VisScreen::MsgProc(msg, wparam, lparam);
}


// 4BC32B
void VisShop::VMethod28()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    g_mousept.DisableHint();
    this->buttons->ResetSelected();
    this->to_buy->VMethod41();
    this->assortiment->VMethod41();
    this->shop_compass->VMethod26();
    this->buttons->LoadBmp();
    this->sub_4BB102();
    this->VMethod31();

    this->gameplay = main_wnd->vis_map_context;
    this->select_info_panel = main_wnd->vis_charinfo;

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == 2) {
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

    g_StructEnter.FUN_00473b80();
    this->selected_units.Copy(g_StructEnter.field_0x0);
    this->select_index = g_StructEnter.FUN_00473d10();

    this->to_sell->visible_startref = &this->selected_units[0]->shopInventoryVisibleStart;
    *this->assortiment->visible_startref = 0;
    this->select_category = 100;
    this->shop_compass->VMethod37(0);
    this->to_sell->VMethod32(&this->selected_units[this->select_index]->tokenEntries);
    this->assortiment->sub_4B73E4(0);
    this->assortiment->sub_4B7859();
    this->to_buy->sub_4B970E();
    this->gameplay->MsgProc(0x405, 0, 0);
    this->selected_units[this->select_index]->VMethod1(1);
    this->gameplay->UpdateSelectionState();
    this->selected_units[this->select_index]->unitFlags |= 8;
    this->to_sell->sub_4B4D33();
    this->assortiment->sub_4B4D33();
    this->to_buy->sub_4B4D33();
    this->dirty |= 0x2F;
    this->assortiment->sub_4B4BC5();
    this->to_sell->sub_4B4BC5();
    this->to_buy->sub_4B4BC5();

    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();

    VisScreen::VMethod28();
    this->dialog_active = 1;
    ApplyCursor(g_Cursors[0]);
    this->snd_enter->Play();
    FUN_004a4740(&this->snd_inshop);

    if (this->snd_start != nullptr && this->snd_start->FindPlayingChannel() == nullptr) {
        this->snd_start->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
    }

    if (main_wnd->sessionMode == 2) {
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
void VisShop::sub_4BCD79()
{
    this->spell_panel = nullptr;
    this->AddChild(this->to_buy);
}


// 4BCD4B
void VisShop::sub_4BCD4B()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->spell_panel = main_wnd->vis_spellbook;
    this->RemoveChild(this->to_buy);
}


// 4BCEA4
void VisShop::sub_4BCEA4()
{
    this->buttons->ResetSelected();
    this->dirty &= ~0x200;
    this->dirty &= ~0x400;
    this->dirty &= ~0x80;
    this->dirty &= ~0x100;
    this->dirty |= 0x20;
}


// 4BC8D7
void VisShop::sub_4BC8D7()
{
    if (this->tips != nullptr) {
        if (this->to_buy->grid_source->GetSize() != 0 && this->tips_update_flag == 0) {
            this->tips_update_flag = 1;
            CString tips_text;
            MissionGetTips(4, &tips_text);
            ((VisMultiText*)this->tips->FindChild(13))->SetText(tips_text);
        }
    }
}


// 4BCC38
void VisShop::sub_4BCC38()
{
    TokenEntry* removed = nullptr;
    this->sub_4BCDA0();
    while (this->to_buy->grid_source->GetSize() != 0) {
        TokenEntry* first = (*this->to_buy->grid_source)[0];
        removed = this->to_buy->VMethod36(0, first->field_0x10);
        if (removed != nullptr) {
            if (removed->field_0x18 == 2) {
                this->to_sell->VMethod37(-1);
            } else {
                this->assortiment->VMethod37(-1);
            }
        }
    }
}


// 4BD251
void VisShop::VMethod32()
{
    FUN_00438dd0(&this->snd_notif);
    FUN_00438dd0(&this->snd_step1);
    FUN_00438dd0(&this->snd_step2);
    FUN_00438dd0(&this->snd_breath);
    FUN_00438dd0(&this->snd_depart);
    FUN_00438dd0(&this->snd_buy);
    FUN_00438dd0(&this->snd_sell);
    FUN_00438dd0(&this->snd_enter);
    FUN_00438dd0(&this->snd_start);
    FUN_00438dd0(&this->snd_pov1);
    FUN_00438dd0(&this->snd_pov2);
    FUN_00438dd0(&this->snd_inshop);
    FUN_00438dd0(&this->snd_out);
    FUN_00438dd0(&this->snd_undo);
}


// 4C6B90
CString VisShop::VMethod33()
{
    return CString();
}


// 4BCDA0
void VisShop::sub_4BCDA0()
{
    this->current_gold = this->gameplay->my_main_unit->gold;
    this->sell_gold = 0;
    this->buy_gold = 0;
    for (int32_t i = 0; i < this->to_buy->grid_source->GetSize(); i++) {
        TokenEntry* entry = (*this->to_buy->grid_source)[i];
        if (entry->field_0x18 == 2) {
            this->sell_gold += (int32_t)(entry->GetAttribute(1) + 1) / 2 * entry->field_0x10;
        } else {
            this->buy_gold -= entry->GetAttribute(1) * entry->field_0x10;
        }
    }
    this->result_gold = this->current_gold + this->buy_gold + this->sell_gold;
}


// 4BB895
void VisShop::sub_4BB895()
{
    this->gameplay->MsgProc(0x405, 0, 0);
    this->select_index++;
    if (this->select_index >= this->selected_units.GetSize()) {
        this->select_index = 0;
    }
    CUnit* unit = this->selected_units[this->select_index];
    unit->VMethod1(1);
    this->gameplay->UpdateSelectionState();
    this->to_sell->VMethod32(&this->selected_units[this->select_index]->tokenEntries);
    this->to_sell->visible_startref = &this->selected_units[this->select_index]->shopInventoryVisibleStart;
    this->to_sell->sub_4B4D33();
    this->to_sell->sub_4B4FD1();
    this->selected_units[this->select_index]->unitFlags |= 8;
    this->dirty |= 9;
}


// 4BBA06
void VisShop::sub_4BBA06()
{
    this->gameplay->MsgProc(0x405, 0, 0);
    if (this->select_index != 0) {
        this->select_index--;
    } else {
        this->select_index = this->selected_units.GetUpperBound();
    }
    CUnit* unit = this->selected_units[this->select_index];
    unit->VMethod1(1);
    this->gameplay->UpdateSelectionState();
    this->to_sell->VMethod32(&this->selected_units[this->select_index]->tokenEntries);
    this->to_sell->visible_startref = &this->selected_units[this->select_index]->shopInventoryVisibleStart;
    this->to_sell->sub_4B4D33();
    this->to_sell->sub_4B4FD1();
    this->selected_units[this->select_index]->unitFlags |= 8;
    this->dirty |= 9;
}


// 4BC97B
void VisShop::sub_4BC97B()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    ApplyCursor(g_Cursors[0]);
    switch (main_wnd->field_0x410) {
    case 1:
        ((VisCharInfo*)this->select_info_panel)->VMethod27(main_wnd->field_0x40c);
        this->dirty |= 8;
        break;
    case 2:
        this->to_sell->VMethod37(main_wnd->field_0x40c);
        this->to_sell->sub_4B4D33();
        this->dirty |= 1;
        break;
    case 4:
        this->to_buy->VMethod37(main_wnd->field_0x40c);
        this->to_buy->sub_4B4D33();
        if (this->spell_panel == nullptr) {
            this->dirty |= 4;
        }
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        this->assortiment->VMethod37(main_wnd->field_0x40c);
        this->assortiment->sub_4B4D33();
        this->dirty |= 2;
        break;
    }
    main_wnd->ResetItemCursor();
}


// 4BBBD6
void VisShop::sub_4BBBD6(int32_t category, CArray<TokenEntry*>* arr)
{
    AfxGetMainWnd();
    for (int32_t i = 0; i < arr->GetSize(); i++) {
        (*arr)[i]->field_0x18 = category + 5;
        (*arr)[i]->field_0x14 = category;
    }
    CArray<TokenEntry*>& cat_arr = this->assortiment->field_0x2100[category];
    for (int32_t i = 0; i < cat_arr.GetSize(); i++) {
        if (cat_arr[i] != nullptr) {
            delete cat_arr[i];
        }
        cat_arr[i] = nullptr;
    }
    cat_arr.RemoveAll();
    for (int32_t i = 0; i < arr->GetSize(); i++) {
        (*arr)[i]->field_0x20 = i;
    }
    FUN_004ba1cc(arr);
    cat_arr.Copy(*arr);
    arr->RemoveAll();
}


// 4BB4FB
void VisShop::sub_4BB4FB()
{
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
void VisShop::sub_4BB102()
{
    char fname[0x400];

    this->sub_4BB4FB();
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


// 4BD495
VisShopCompass::VisShopCompass(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : CVisualObject(_id, l, t, r, b, nullptr)
{
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
void VisShopCompass::VMethod7()
{
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
                this->sub_4BF63F(i);
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
        this->frame_sprite->VMethod2(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, 0);
    }
    if (this->bkg_bmp != nullptr) {
        this->bkg_bmp->VMethod2(topleft.x + this->rect.left + 5, topleft.y + this->rect.top + 8, 0, 0, 0);
    }
    for (int32_t i = 0; i < 4; i++) {
        if ((this->state & (1 << i)) != 0 && this->direction_frames[i][this->dir_frm[i]] != nullptr) {
            this->direction_frames[i][this->dir_frm[i]]->VMethod2(topleft.x + this->outer_rects[i].left, topleft.y + this->outer_rects[i].top, 0, 0, 0);
        }
    }
    if (this->state & 0x10) {
        this->trigger_bmp->VMethod2(topleft.x + this->rect.left + 0x71, topleft.y + this->rect.top + 0x70, 0, 0, 0);
    } else if (this->state & 0x20) {
        this->fwd_frames[this->center_frm]->VMethod2(topleft.x + this->rect.left + 0x71, topleft.y + this->rect.top + 0x70, 0, 0, 0);
    } else if (this->state & 0x40) {
        this->ret_frames[this->center_frm]->VMethod2(topleft.x + this->rect.left + 0x71, topleft.y + this->rect.top + 0x70, 0, 0, 0);
    } else if (this->idle_bmp != nullptr) {
        this->idle_bmp->VMethod2(topleft.x + this->rect.left + 0x71, topleft.y + this->rect.top + 0x70, 0, 0, 0);
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
int32_t VisShopCompass::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CPoint topleft = this->shop->rect.TopLeft();
    if (this->state & 0x80) {
        CRect yes_rect = this->confirm_yes_rect + this->confirm_rect.TopLeft() + topleft;
        CRect no_rect = this->confirm_no_rect + this->confirm_rect.TopLeft() + topleft;
        if (yes_rect.PtInRect(pos) || no_rect.PtInRect(pos)) {
            this->state &= ~0x80;
            this->shop->sub_4BC97B();
            main_wnd->ResetItemCursor();
        }
    } else {
        for (int32_t i = 0; i < 4; i++) {
            CRect hit_rect = this->outer_rects[i] + topleft;
            if (hit_rect.PtInRect(pos) && this->VMethod37(i) != 0) {
                this->state |= 0x20;
                this->VMethod28();
                this->shop->assortiment->sub_4B73E4((uint16_t)this->shop->select_category);
                this->shop->assortiment->VMethod10();
                break;
            }
        }
    }
    return 0;
}


// 4BEA64
const char* VisShopCompass::GetHint()
{
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
void VisShopCompass::VMethod26()
{
    this->VMethod27();
    this->frame_sprite = new CSprite256("graphics\\interface\\ShopFrame.256");
    this->frame_sprite->ResetPalette(1, 1, 0);
    g_mousept.Update();
    this->bkg_bmp = new CBmp64("graphics\\interface\\shopanim\\ShopMain.bmp");
    this->idle_bmp = new CBmp64("movies\\shopanim\\Pose2-3\\1.bmp");
    g_mousept.Update();
}


// 4BE5DE
void VisShopCompass::VMethod28()
{
    char fname[52];

    this->VMethod33();
    this->fwd_frames[0] = new CBmp64("movies\\shopanim\\Pose2-3\\1.bmp");
    for (int32_t i = 1; i < 12; i++) {
        sprintf(fname, "movies\\shopanim\\Yes\\%d.bmp", i + 1);
        this->fwd_frames[i] = new CBmp64(fname);
    }
}


// 4BE6DF
void VisShopCompass::VMethod29()
{
    char fname[52];

    this->VMethod34();
    this->ret_frames[0] = new CBmp64("movies\\shopanim\\Pose2-3\\1.bmp");
    for (int32_t i = 1; i < 12; i++) {
        sprintf(fname, "movies\\shopanim\\No\\%d.bmp", i + 1);
        this->ret_frames[i] = new CBmp64(fname);
    }
}


// 4BF4E8
int32_t VisShopCompass::VMethod37(int32_t arg)
{
    if ((uint16_t)this->shop->select_category == (uint16_t)arg) {
        return 0;
    }
    FUN_00438f20(&this->shop->snd_depart);
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
void VisShopCompass::VMethod31(int32_t arg)
{
    char fname[52];

    this->VMethod32(arg);
    for (int32_t i = 0; i < 11; i++) {
        sprintf(fname, "graphics\\interface\\shopanim\\%.2d\\%d.bmp", 4 - arg, i + 1);
        this->direction_frames[arg][i] = new CBmp64(fname);
    }
    this->dir_frm[arg] = 0;
}


// 4BE372
void VisShopCompass::VMethod27()
{
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
void VisShopCompass::VMethod30()
{
    char fname[52];

    this->VMethod35();
    this->center_frm = (this->center_frm + 1) % 30;
    sprintf(fname, "movies\\shopanim\\Pose2-3\\%d.bmp", this->center_frm + 1);
    this->trigger_bmp = new CBmp64(fname);
}


// 4BE541
void VisShopCompass::VMethod32(int32_t arg)
{
    for (int32_t i = 0; i < 11; i++) {
        if (this->direction_frames[arg][i] != nullptr) {
            delete this->direction_frames[arg][i];
        }
        this->direction_frames[arg][i] = nullptr;
    }
}


// 4BE8B0
void VisShopCompass::VMethod33()
{
    for (int32_t i = 0; i < 12; i++) {
        if (this->fwd_frames[i] != nullptr) {
            delete this->fwd_frames[i];
        }
        this->fwd_frames[i] = nullptr;
    }
}


// 4BE92D
void VisShopCompass::VMethod34()
{
    for (int32_t i = 0; i < 12; i++) {
        if (this->ret_frames[i] != nullptr) {
            delete this->ret_frames[i];
        }
        this->ret_frames[i] = nullptr;
    }
}


// 4BEA01
void VisShopCompass::VMethod36()
{
    for (int32_t i = 0; i < 4; i++) {
        this->VMethod32(i);
    }
    this->VMethod33();
    this->VMethod34();
    this->VMethod35();
}


// 4BF6CE
int32_t VisShopCompass::OnMouseMove(uint32_t wparam, CPoint pos)
{
    if (this->shop->hovered_region != this->VMethod38()) {
        this->shop->hovered_region = this->VMethod38();
        this->shop->buttons->ResetSelected();
        this->shop->dirty |= 0x2F;
    }
    return 0;
}


// 4BE9AA
void VisShopCompass::VMethod35()
{
    if (this->trigger_bmp != nullptr) {
        delete this->trigger_bmp;
    }
    this->trigger_bmp = nullptr;
}


// 4BE1DF
VisShopCompass::~VisShopCompass()
{
    this->VMethod27();
    this->VMethod36();
}


// 4BFA01
int32_t VisShopCompass::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        this->shop->placement_lock = 0;
        ApplyCursor(g_Cursors[0]);
        this->shop->sub_4BC97B();
        main_wnd->ResetItemCursor();
    }
    return 1;
}


// 4C6BC0
int32_t VisShopCompass::VMethod38()
{
    return 0x64;
}

// 4bf63f
void VisShopCompass::sub_4BF63F(int32_t dir) {
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
    : VisShopCompass(_id, l, t, r, b, shop)
{
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


// 4C4693
VisShopCompassKaarg::VisShopCompassKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisShopCompass(_id, l, t, r, b, shop)
{
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


// 4BFA5A
VisShopButtons::VisShopButtons(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : CVisualObject(_id, l, t, r, b, nullptr)
{
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
VisShopButtons::~VisShopButtons()
{
    this->shop = nullptr;
    this->ReleaseBmp();
    this->pressed_btn = -1;
}


// 4C6BD0
int32_t VisShopButtons::VMethod30()
{
    return 0x65;
}


// 4C10C4
int32_t VisShopButtons::OnMouseMove(uint32_t wparam, CPoint pos)
{
    if (this->shop->hovered_region != this->VMethod30()) {
        this->shop->hovered_region = this->VMethod30();
        this->shop->dirty |= 0x2F;
    }
    this->UpdateHoveredState(wparam, pos);
    return 0;
}


// 4C1134
int32_t VisShopButtons::OnLButtonDown(uint32_t wparam, CPoint pos)
{
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
int32_t VisShopButtons::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        this->shop->sub_4BC97B();
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
            this->shop->sub_4BCCE1();
            break;
        case 1:
            this->shop->sub_4BCB63();
            break;
        case 2:
            this->shop->sub_4BCAF1();
            break;
        case 3:
            this->shop->FUN_004bcd02();
            if (main_wnd->dialogsMask == 2) {
                main_wnd->PostMessageA(0x42E, 0, 0);
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
void VisShopButtons::VMethod7()
{
    CPoint topleft = this->shop->rect.TopLeft();
    CString str;
    if (this->shop->dialog_active == 0) {
        return;
    }
    if (this->button_bmps[0] == nullptr || this->button_bmps[1] == nullptr || this->button_bmps[2] == nullptr || this->button_bmps[3] == nullptr || this->menu_bmp == nullptr) {
        return;
    }
    LockSurface2();
    this->menu_bmp->VMethod10(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, this->rect.Width(), this->rect.Height());
    uint16_t* pal;
    if (this->hovered_btn == 0) {
        pal = palette_paris_daisy->GetPalette(0);
    } else {
        pal = palette_husk->GetPalette(0);
    }
    str = TxtFile::AllLines[0x48];
    g_font4->DrawTxt(topleft.x + this->outer_rects[0].left + this->outer_rects[0].Width() / 2, topleft.y + this->outer_rects[0].top + 6 + this->outer_rects[0].Height() / 4, str, 10, pal);
    str.Format("%d", this->shop->current_gold);
    FUN_00476987(&str);
    g_font4->DrawTxt(topleft.x + this->outer_rects[0].left + this->outer_rects[0].Width() / 2, topleft.y + this->outer_rects[0].top - 2 + this->outer_rects[0].Height() * 3 / 4, str, 10, pal);
    if (this->hovered_btn == 1) {
        pal = palette_paris_daisy->GetPalette(0);
    } else {
        pal = palette_husk->GetPalette(0);
    }
    str = TxtFile::AllLines[0x46];
    g_font4->DrawTxt(topleft.x + this->outer_rects[1].left + this->outer_rects[1].Width() / 2, topleft.y + this->outer_rects[1].top + 6 + this->outer_rects[1].Height() / 4, str, 10, pal);
    str.Format("%d", this->shop->buy_gold);
    FUN_00476987(&str);
    g_font4->DrawTxt(topleft.x + this->outer_rects[1].left + this->outer_rects[1].Width() / 2, topleft.y + this->outer_rects[1].top - 2 + this->outer_rects[1].Height() * 3 / 4, str, 10, pal);
    if (this->hovered_btn == 2) {
        pal = palette_paris_daisy->GetPalette(0);
    } else {
        pal = palette_husk->GetPalette(0);
    }
    str = TxtFile::AllLines[0x47];
    g_font4->DrawTxt(topleft.x + this->outer_rects[2].left + this->outer_rects[2].Width() / 2, topleft.y + this->outer_rects[2].top + 8 + this->outer_rects[2].Height() / 4, str, 10, pal);
    str.Format("%d", this->shop->sell_gold);
    FUN_00476987(&str);
    g_font4->DrawTxt(topleft.x + this->outer_rects[2].left + this->outer_rects[2].Width() / 2, topleft.y + this->outer_rects[2].top + this->outer_rects[2].Height() * 3 / 4, str, 10, pal);
    if (this->hovered_btn == 3) {
        pal = palette_paris_daisy->GetPalette(0);
    } else {
        pal = palette_husk->GetPalette(0);
    }
    str = TxtFile::AllLines[0x49];
    g_font4->DrawTxt(topleft.x + this->outer_rects[3].left + this->outer_rects[3].Width() / 2, topleft.y + this->outer_rects[3].top + 6 + this->outer_rects[3].Height() / 4, str, 10, pal);
    str.Format("%d", this->shop->result_gold);
    FUN_00476987(&str);
    g_font4->DrawTxt(topleft.x + this->outer_rects[3].left + this->outer_rects[3].Width() / 2, topleft.y + this->outer_rects[3].top - 2 + this->outer_rects[3].Height() * 3 / 4, str, 10, pal);
    if (this->hovered_btn >= 0 && this->pressed_btn >= 0 && this->pressed_btn == this->hovered_btn) {
        CRect& rect = this->outer_rects[this->hovered_btn];
        this->button_bmps[this->hovered_btn]->VMethod10(topleft.x + rect.left, topleft.y + rect.top, 0, 0, rect.Width(), rect.Height());
        pal = palette_paris_daisy->GetPalette(0);
        switch (this->hovered_btn) {
        case 0:
            str = TxtFile::AllLines[0x48];
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top + 7 + rect.Height() / 4, str, 10, pal);
            str.Format("%d", this->shop->current_gold);
            FUN_00476987(&str);
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top - 1 + rect.Height() * 3 / 4, str, 10, pal);
            break;
        case 1:
            str = TxtFile::AllLines[0x46];
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top + 7 + rect.Height() / 4, str, 10, pal);
            str.Format("%d", this->shop->buy_gold);
            FUN_00476987(&str);
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top - 1 + rect.Height() * 3 / 4, str, 10, pal);
            break;
        case 2:
            str = TxtFile::AllLines[0x47];
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top + 9 + rect.Height() / 4, str, 10, pal);
            str.Format("%d", this->shop->sell_gold);
            FUN_00476987(&str);
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top + 1 + rect.Height() * 3 / 4, str, 10, pal);
            break;
        case 3:
            str = TxtFile::AllLines[0x49];
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top + 7 + rect.Height() / 4, str, 10, pal);
            str.Format("%d", this->shop->result_gold);
            FUN_00476987(&str);
            g_font4->DrawTxt(topleft.x + rect.left + rect.Width() / 2, topleft.y + rect.top - 1 + rect.Height() * 3 / 4, str, 10, pal);
            break;
        }
    }
    UnlockSurface2();
}


// 4C1358
void VisShopButtons::ResetSelected()
{
    this->pressed_btn = -1;
    this->hovered_btn = -1;
}


// 4C0352
void VisShopButtons::ReleaseBmp()
{
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
void VisShopButtons::LoadBmp()
{
    this->ReleaseBmp();
    CString base = "graphics\\interface\\";
    if (this->shop != nullptr)
        base += this->shop->VMethod33();

    for (int32_t i = 0; i < 4; i++) {
        CString fname = base + "ShopButton" + CString((char)('1' + i), 1) + ".bmp";
        this->button_bmps[i] = new CBmp64(fname);
        g_mousept.Update();
    }
    this->menu_bmp = new CBmp64(base + "ShopMenu.bmp");
    g_mousept.Update();
}

void VisShopButtons::UpdateHoveredState(uint32_t wparam, CPoint pos)
{ //4c14c4
    int32_t idx = GetButtonAt(pos);
    if (idx < 0 || (wparam & 1) != 0)
    {
        if (idx < 0 || idx != pressed_btn || (wparam & 1) == 0)
            hovered_btn = -1;
        else
            hovered_btn = idx;
    }
    else
        hovered_btn = idx;

    shop->dirty |= 0x20;
}

int32_t VisShopButtons::GetButtonAt(CPoint pos)
{ //4c137d
    CPoint pt = pos - shop->rect.TopLeft();
    int32_t idx = -1;
    for (int i = 0; i < 4; i++)
    {
        if (outer_rects[i].PtInRect(pt))
        {
            idx = i;
            break;
        }
    }

    if (idx < 0)
        return -1;

    if (idx == 0)
    {
        if (inner_rects[0].PtInRect(pt) || inner_rects[1].PtInRect(pt))
            return 0;
        return -1;
    }

    if (idx != 3)
        return idx;

    if (inner_rects[2].PtInRect(pt) || inner_rects[3].PtInRect(pt))
        return 3;

    return -1;
}


// 4C69E0
VisShopDruid::~VisShopDruid()
{
}


// 4C15AD
VisShopDruid::VisShopDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm)
    : VisShop(_id, l, t, r, b, btm)
{
}


// 4C15E6
void VisShopDruid::VMethod26()
{
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
}


// 4C1B8B
void VisShopDruid::VMethod31()
{
    this->VMethod32();
    FUN_00438e40(&this->snd_notif, "SFX\\Town\\Shop\\nofit.wav");
    FUN_00438e40(&this->snd_depart, "SFX\\Town_Druid\\Shop\\Dotdel.wav");
    FUN_00438e40(&this->snd_buy, "SFX\\Town\\buy.wav");
    FUN_00438e40(&this->snd_sell, "SFX\\Town\\sell.wav");
    FUN_00438e40(&this->snd_enter, "SFX\\Town_Druid\\Shop\\Din2.wav");
    FUN_00438e40(&this->snd_pov1, "SFX\\Town_Druid\\Shop\\Ddruid5.wav");
    FUN_00438e40(&this->snd_pov2, "SFX\\Town_Druid\\Shop\\Ddruid6.wav");
    FUN_00438e40(&this->snd_inshop, "SFX\\Town_Druid\\Inn\\Dforest2.wav");
    FUN_00438e40(&this->snd_out, "SFX\\Out.wav");
    FUN_00438e40(&this->snd_undo, "SFX\\Undo.wav");
    FUN_00438e40(&this->snd_bird[0], "SFX\\Town_druid\\Inn\\Dbird4.wav");
    FUN_00438e40(&this->snd_bird[1], "SFX\\Town_druid\\Inn\\Dbird41.wav");
    FUN_00438e40(&this->snd_bird[2], "SFX\\Town_druid\\Inn\\Dbird42.wav");
    FUN_00438e40(&this->snd_tool[0], "SFX\\Town_druid\\shop\\Dtools1.wav");
    FUN_00438e40(&this->snd_tool[1], "SFX\\Town_druid\\shop\\Dtools2.wav");
    FUN_00438e40(&this->snd_tool[2], "SFX\\Town_druid\\shop\\Dtools3.wav");
    FUN_00438e40(&this->snd_tool[3], "SFX\\Town_druid\\shop\\Dtools4.wav");
}


// 4C1958
void VisShopDruid::VMethod30()
{
    static uint8_t snd_timing_init = 0; // byte_665D9C in asm
    static uint32_t last_snd_time; // dword_665DD4 in asm

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
void VisShopDruid::VMethod28()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    g_mousept.DisableHint();
    this->buttons->ResetSelected();
    this->to_buy->VMethod41();
    this->assortiment->VMethod41();
    this->shop_compass->VMethod26();
    this->buttons->LoadBmp();
    this->sub_4BB102();
    this->VMethod31();

    this->gameplay = main_wnd->vis_map_context;
    this->select_info_panel = main_wnd->vis_charinfo;

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == 2) {
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

    g_StructEnter.FUN_00473b80();
    this->selected_units.Copy(g_StructEnter.field_0x0);
    this->select_index = g_StructEnter.FUN_00473d10();

    this->to_sell->visible_startref = &this->selected_units[0]->shopInventoryVisibleStart;
    *this->assortiment->visible_startref = 0;
    this->select_category = 100;
    this->shop_compass->VMethod37(0);
    this->to_sell->VMethod32(&this->selected_units[this->select_index]->tokenEntries);
    this->assortiment->sub_4B73E4(0);
    this->assortiment->sub_4B7859();
    this->to_buy->sub_4B970E();
    this->gameplay->MsgProc(0x405, 0, 0);
    this->selected_units[this->select_index]->VMethod1(1);
    this->gameplay->UpdateSelectionState();
    this->selected_units[this->select_index]->unitFlags |= 8;
    this->to_sell->sub_4B4D33();
    this->assortiment->sub_4B4D33();
    this->to_buy->sub_4B4D33();
    this->dirty |= 0x2F;
    this->assortiment->sub_4B4BC5();
    this->to_sell->sub_4B4BC5();
    this->to_buy->sub_4B4BC5();

    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();

    VisScreen::VMethod28();
    this->dialog_active = 1;
    ApplyCursor(g_Cursors[0]);
    this->snd_enter->Play();
    FUN_004a4740(&this->snd_inshop);

    this->next_bird = GetRandS16(2000) + 2000;
    this->bird_tick = timeGetTime();
    this->next_tool = GetRandS16(2000) + 2000;
    this->tool_tick = timeGetTime();

    if (this->snd_start != nullptr && this->snd_start->FindPlayingChannel() == nullptr) {
        this->snd_start->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
    }

    if (main_wnd->sessionMode == 2) {
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
void VisShopDruid::VMethod32()
{
    FUN_00438dd0(&this->snd_notif);
    FUN_00438dd0(&this->snd_depart);
    FUN_00438dd0(&this->snd_buy);
    FUN_00438dd0(&this->snd_sell);
    FUN_00438dd0(&this->snd_enter);
    FUN_00438dd0(&this->snd_pov1);
    FUN_00438dd0(&this->snd_pov2);
    FUN_00438dd0(&this->snd_inshop);
    FUN_00438dd0(&this->snd_out);
    FUN_00438dd0(&this->snd_undo);
    FUN_00438dd0(&this->snd_bird[0]);
    FUN_00438dd0(&this->snd_bird[1]);
    FUN_00438dd0(&this->snd_bird[2]);
    FUN_00438dd0(&this->snd_tool[0]);
    FUN_00438dd0(&this->snd_tool[1]);
    FUN_00438dd0(&this->snd_tool[2]);
    FUN_00438dd0(&this->snd_tool[3]);
}


// 4C6BE0
CString VisShopDruid::VMethod33()
{
    return "shop_druid\\";
}


// 4C453D
void VisShopKaarg::VMethod32()
{
    FUN_00438dd0(&this->snd_notif);
    FUN_00438dd0(&this->snd_depart);
    FUN_00438dd0(&this->snd_buy);
    FUN_00438dd0(&this->snd_sell);
    FUN_00438dd0(&this->snd_enter);
    FUN_00438dd0(&this->snd_pov1);
    FUN_00438dd0(&this->snd_pov2);
    FUN_00438dd0(&this->snd_inshop);
    FUN_00438dd0(&this->snd_out);
    FUN_00438dd0(&this->snd_undo);
    FUN_00438dd0(&this->snd_voice[0]);
    FUN_00438dd0(&this->snd_voice[1]);
    FUN_00438dd0(&this->snd_voice[2]);
    FUN_00438dd0(&this->snd_tool[0]);
    FUN_00438dd0(&this->snd_tool[1]);
    FUN_00438dd0(&this->snd_tool[2]);
    FUN_00438dd0(&this->snd_tool[3]);
}


// 4C6C10
CString VisShopKaarg::VMethod33()
{
    return "shop_kaarg\\";
}


// 4C6A50
VisShopKaarg::~VisShopKaarg()
{
}


// 4C37C5
VisShopKaarg::VisShopKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm)
    : VisShop(_id, l, t, r, b, btm)
{
}


// 4C43A3
void VisShopKaarg::VMethod31()
{
    this->VMethod32();
    FUN_00438e40(&this->snd_notif, "SFX\\Town\\Shop\\nofit.wav");
    FUN_00438e40(&this->snd_depart, "SFX\\Town_kaarg\\Shop\\Kotdel.wav");
    FUN_00438e40(&this->snd_buy, "SFX\\Town\\buy.wav");
    FUN_00438e40(&this->snd_sell, "SFX\\Town\\sell.wav");
    FUN_00438e40(&this->snd_enter, "SFX\\Town_kaarg\\Shop\\Kin2.wav");
    FUN_00438e40(&this->snd_pov1, "SFX\\Town_kaarg\\Shop\\Kman4.wav");
    FUN_00438e40(&this->snd_pov2, "SFX\\Town_kaarg\\Shop\\Kman4.wav");
    FUN_00438e40(&this->snd_inshop, "SFX\\Town_kaarg\\Inn\\Kvox5.wav");
    FUN_00438e40(&this->snd_out, "SFX\\Out.wav");
    FUN_00438e40(&this->snd_undo, "SFX\\Undo.wav");
    FUN_00438e40(&this->snd_voice[0], "SFX\\Town_kaarg\\Inn\\Kvox6.wav");
    FUN_00438e40(&this->snd_voice[1], "SFX\\Town_kaarg\\Inn\\Kvox7.wav");
    FUN_00438e40(&this->snd_voice[2], "SFX\\Town_kaarg\\Inn\\Kvox8.wav");
    FUN_00438e40(&this->snd_tool[0], "SFX\\Town_kaarg\\shop\\Ktools1.wav");
    FUN_00438e40(&this->snd_tool[1], "SFX\\Town_kaarg\\shop\\Ktools2.wav");
    FUN_00438e40(&this->snd_tool[2], "SFX\\Town_kaarg\\shop\\Ktools3.wav");
    FUN_00438e40(&this->snd_tool[3], "SFX\\Town_kaarg\\shop\\Ktools4.wav");
}


// 4C4170
void VisShopKaarg::VMethod30()
{
    static uint8_t snd_timing_init = 0; // byte_665DA4 in asm
    static uint32_t last_snd_time; // dword_665DB8 in asm

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
void VisShopKaarg::VMethod26()
{
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
}


// 4C3B70
void VisShopKaarg::VMethod28()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    g_mousept.DisableHint();
    this->buttons->ResetSelected();
    this->to_buy->VMethod41();
    this->assortiment->VMethod41();
    this->shop_compass->VMethod26();
    this->buttons->LoadBmp();
    this->sub_4BB102();
    this->VMethod31();

    this->gameplay = main_wnd->vis_map_context;
    this->select_info_panel = main_wnd->vis_charinfo;

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == 2) {
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

    g_StructEnter.FUN_00473b80();
    this->selected_units.Copy(g_StructEnter.field_0x0);
    this->select_index = g_StructEnter.FUN_00473d10();

    this->to_sell->visible_startref = &this->selected_units[0]->shopInventoryVisibleStart;
    *this->assortiment->visible_startref = 0;
    this->select_category = 100;
    this->shop_compass->VMethod37(0);
    this->to_sell->VMethod32(&this->selected_units[this->select_index]->tokenEntries);
    this->assortiment->sub_4B73E4(0);
    this->assortiment->sub_4B7859();
    this->to_buy->sub_4B970E();
    this->gameplay->MsgProc(0x405, 0, 0);
    this->selected_units[this->select_index]->VMethod1(1);
    this->gameplay->UpdateSelectionState();
    this->selected_units[this->select_index]->unitFlags |= 8;
    this->to_sell->sub_4B4D33();
    this->assortiment->sub_4B4D33();
    this->to_buy->sub_4B4D33();
    this->dirty |= 0x2F;
    this->assortiment->sub_4B4BC5();
    this->to_sell->sub_4B4BC5();
    this->to_buy->sub_4B4BC5();

    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();

    VisScreen::VMethod28();
    this->dialog_active = 1;
    ApplyCursor(g_Cursors[0]);
    this->snd_enter->Play();
    FUN_004a4740(&this->snd_inshop);

    this->next_voice = GetRandS16(2000) + 2000;
    this->voice_tick = timeGetTime();
    this->next_tool = GetRandS16(2000) + 2000;
    this->tool_tick = timeGetTime();

    if (this->snd_start != nullptr && this->snd_start->FindPlayingChannel() == nullptr) {
        this->snd_start->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
    }

    if (main_wnd->sessionMode == 2) {
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
void VisShop::sub_4BCCE1()
{
    this->sub_4BCC38();
    this->gameplay->sub_41A9F6();
}


// 4BCAF1
void VisShop::sub_4BCAF1()
{
    this->sub_4BCDA0();
    if (this->sell_gold != 0) {
        this->shop_compass->VMethod28();
        this->shop_compass->state |= 0x20;
        this->gameplay->sub_41A99C();
        this->snd_sell->Play();
    }
}

// 4BCB63
int VisShop::sub_4BCB63()
{
    this->sub_4BCDA0();

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
    this->snd_notif->Play();
    this->gameplay->sub_41A942();
    this->snd_buy->Play();
    return 1;
}


// 49E34F
void VisTav::VMethod28()
{
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

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == 2) {
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

    if (main_wnd->sessionMode == 2) {
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
            this->avail_entries.Add(this->map_context->FUN_0041dfa6(unit_id));
        } else {
            this->reserved_entries.Add(this->map_context->FUN_0041dfa6(unit_id));
        }
    }

    if (this->avail_entries.GetSize() + this->reserved_entries.GetSize() != 0) {
        this->selection_index = 0;
    } else {
        this->selection_index = -1;
    }

    g_StructEnter.FUN_00473b80();
    this->selected_entries.Copy(g_StructEnter.field_0x0);
    this->select_party = g_StructEnter.FUN_00473d10();

    this->map_context->MsgProc(0x405, 0, 0);
    this->selected_entries[this->select_party]->VMethod1(1);
    this->map_context->UpdateSelectionState();
    this->selected_entries[this->select_party]->unitFlags |= 8;

    this->scene->VMethod26();

    CStringArray names;
    CString name;

    for (int32_t i = 0; i < 10; i++) {
        name.Format("graphics\\interface\\inn\\candle\\t%.4d.bmp", i);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        this->scene->anims[0].FUN_004010ee(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 0x15; i++) {
        name.Format("graphics\\interface\\inn\\cauldron\\t%.4d.bmp", i);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        this->scene->anims[1].FUN_004010ee(&names);
    }
    names.RemoveAll();

    for (int32_t i = 1; i <= 0x18; i++) {
        name.Format("graphics\\interface\\inn\\tender\\breath\\br%.4d.bmp", i);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        this->scene->anims[2].FUN_004010ee(&names);
    }
    names.RemoveAll();

    for (int32_t i = 1; i <= 0x28; i++) {
        name.Format("graphics\\interface\\inn\\tender\\drink\\dr%.4d.bmp", i);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        this->scene->anims[3].FUN_004010ee(&names);
    }
    names.RemoveAll();

    this->right_panel->FUN_00499a67();
    this->right_panel->FUN_0049a84e();
    this->scene->VMethod28();
    this->left_panel->FUN_004995d1();
    this->right_panel->FUN_0049a973();
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
    this->field_0x13c = 0;
}


// 49EB6C
void VisTav::DoClose(uint32_t code)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    this->VMethod9();
    if (main_wnd->sessionMode == 2) {
        ScenarioLeaveInn();
    }

    if (this->field_0x13c != 0) {
        main_wnd->vis_map_context->FUN_0041ae1c(0xAAAAAAAA);
    } else {
        main_wnd->vis_map_context->FUN_0041ae1c(this->quest_selection);
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

    this->right_panel->FUN_00499cdf();
    this->scene->VMethod29();
    this->scene->VMethod27();
    this->left_panel->FUN_004996ab();

    this->scene->anims[0].FUN_004014f2();
    this->scene->anims[1].FUN_004014f2();
    this->scene->anims[3].FUN_004014f2();
    this->scene->anims[2].FUN_004014f2();

    this->VMethod31();

    this->avail_entries.RemoveAll();
    this->selected_entries.RemoveAll();
    this->reserved_entries.RemoveAll();

    VisScreen::DoClose(code);
    this->quest_map->sub_55ECFE(0);

    while (this->rewards.GetSize() != 0) {
        TokenEntry* entry = this->rewards[0];
        if (entry != nullptr) {
            delete entry;
        }
        this->rewards.RemoveAt(0, 1);
    }
}


// 49DDAF
void VisTav::VMethod26()
{
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
int32_t VisTav::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    case 0x402:
        if (this->rewards.GetSize() != 0 || this->quest_map->FUN_0041ec00() != 0) {
            if (this->right_panel->texts[0] == " ") {
                this->right_panel->FUN_0049a973();
            }
        }
        if (main_wnd->dialogsMask == 4 || main_wnd->dialogsMask == 5) {
            this->VMethod9();
        }
        break;
    case 0x414:
        this->FUN_0049eecd();
        break;
    case 0x415:
        this->FUN_0049ee36();
        break;
    case 0x45A:
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
void VisTav::VMethod30()
{
    this->VMethod31();
    FUN_00438e40(&this->sounds[0].sample, "SFX\\Town\\Inn\\drink.wav");
    FUN_00438e40(&this->sounds[1].sample, "SFX\\Town\\Inn\\glotok.wav");
    FUN_00438e40(&this->sounds[2].sample, "SFX\\Town\\Inn\\steam.wav");
    FUN_00438e40(&this->sounds[3].sample, "SFX\\Town\\Inn\\water.wav");
    FUN_00438e40(&this->sounds[4].sample, "SFX\\Town\\Inn\\chair.wav");
    FUN_00438e40(&this->sounds[5].sample, "SFX\\Add.wav");
    FUN_00438e40(&this->sounds[6].sample, "SFX\\NoAdd.wav");
    FUN_00438e40(&this->sounds[7].sample, "SFX\\Town\\Shop\\nofit.wav");
    FUN_00438e40(&this->sounds[8].sample, "SFX\\Town\\Inn\\enter.wav");
    FUN_00438e40(&this->sounds[9].sample, "SFX\\Town\\Inn\\Helper.wav");
    FUN_00438e40(&this->sounds[10].sample, "SFX\\Town\\Shop\\Breath.wav");
    FUN_00438e40(&this->sounds[11].sample, "SFX\\Out.wav");
    FUN_00438e40(&this->sounds[12].sample, "SFX\\Talk.wav");
}


// 49F3D7
void VisTav::VMethod31()
{
    for (int32_t i = 0; i < 13; i++) {
        FUN_00438dd0(&this->sounds[i].sample);
    }
}


// 49DB79
VisTav::VisTav(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr)
{
}


// 49E261
int32_t VisTav::OnMouseMove(uint32_t wparam, CPoint pos)
{
    CRect r = this->right_panel->GetRect() + this->rect.TopLeft();
    if (!r.PtInRect(pos)) {
        this->right_panel->FUN_0049a84e();
    }

    return CVisualObject::OnMouseMove(wparam, pos);
}


// 49EECD
void VisTav::FUN_0049eecd()
{
    this->map_context->MsgProc(0x405, 0, 0);

    if (this->select_party != 0) {
        this->select_party--;
    } else {
        this->select_party = this->selected_entries.GetUpperBound();
    }

    this->selected_entries[this->select_party]->VMethod1(1);
    this->map_context->UpdateSelectionState();
}


// 49EE36
void VisTav::FUN_0049ee36()
{
    this->map_context->MsgProc(0x405, 0, 0);

    this->select_party++;
    if (this->select_party >= this->selected_entries.GetSize()) {
        this->select_party = 0;
    }

    this->selected_entries[this->select_party]->VMethod1(1);
    this->map_context->UpdateSelectionState();
}


// 49EF63
void VisTav::FUN_0049ef63()
{
    CUnit* unit = this->avail_entries[this->selection_index];
    int32_t entry = this->FUN_0049e2e3(unit);
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
void VisTav::FUN_0049f179()
{
    CUnit* unit = this->reserved_entries[this->selection_index - this->avail_entries.GetSize()];
    int32_t entry = this->FUN_0049e2e3(unit);
    uint32_t v = this->entrie_id[entry];

    CString text;
    text.Format("npc%dtalk%d", v & 0xFFFF, (v >> 0x10) & 0xFFF);
    ShowRoleKeyDialog(text);
    ScenarioTalkTo(this->entrie_id[entry]);

    CSound::Play(this->sounds[12]);
}


// 49F0DA
void VisTav::FUN_0049f0da()
{
    CUnit* unit = this->avail_entries[this->selection_index];
    int32_t entry = this->FUN_0049e2e3(unit);
    this->entrie_id[entry] &= 0x7FFFFFFF;

    unit = this->avail_entries[this->selection_index];
    entry = this->FUN_0049e2e3(unit);
    ScenarioTalkTo(this->entrie_id[entry]);

    CSound::Play(this->sounds[6]);
}


// 49E2E3
int32_t VisTav::FUN_0049e2e3(CUnit* unit)
{
    for (int32_t i = 0; i < this->entrie_id.GetSize(); i++) {
        if (unit->serverId == (this->entrie_id[i] & 0xFFFF)) {
            return i;
        }
    }
    return -1;
}


// 49EDEC
void VisTav::FUN_0049edec()
{
    this->MsgProc(0x445, 0, 0);

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
    if (wnd->dialogsMask == 4) {
        AfxGetMainWnd()->PostMessageA(0x42E, 0, 0);
    }
}


// 49E233
int32_t VisTav::OnKeyDown(uint32_t wparam)
{
    if (wparam == VK_ESCAPE) {
        this->FUN_0049edec();
        return 1;
    }
    return 0;
}


// 49E044
void VisTav::VMethod7()
{
    if (this->dialog_active != 0) {
        VisScreen::VMethod7();
    }

    FUN_004a4740(&this->sounds[3].sample);
}


// 49E075
void VisTav::VMethod8(CRect* rect)
{
}


// 49DC33
VisTav::~VisTav()
{
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
const char* VisTavLeftPanel::GetHint()
{
    if (this->vis_tav->dialog_active == 0) {
        return nullptr;
    }
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != 2) {
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
        if (unit->unitFlags & 0x40) {
            return nullptr;
        }
        return unit->FUN_0046d0f7(pt.x, pt.y);
    }

    client_rect.TopLeft().x -= 4;
    uint8_t* data = (uint8_t*)this->field_0x164->GetData();
    CPoint client_topleft = client_rect.TopLeft();
    uint8_t index = data[(mouse_pt.y - client_topleft.y - 0xF0) * 0xA0 + (mouse_pt.x - client_topleft.x) - 0x10];
    if (index == 0) {
        return nullptr;
    }
    return unit->equipmentTokens[index - 1]->FUN_00439973();
}


// 4995D1
void VisTavLeftPanel::FUN_004995d1()
{
    this->FUN_004996ab();
    this->field_0x168 = new CBmp64("graphics\\Interface\\Inn\\LeftStats.bmp");
    g_mousept.Update();
    this->field_0x16c = new CBmp64("graphics\\Interface\\Inn\\LeftPicture.bmp");
    g_mousept.Update();
}


// 4996AB
void VisTavLeftPanel::FUN_004996ab()
{
    if (this->field_0x168 != nullptr) {
        delete this->field_0x168;
    }
    this->field_0x168 = nullptr;
    if (this->field_0x16c != nullptr) {
        delete this->field_0x16c;
    }
    this->field_0x16c = nullptr;
}


// 497990
VisTavLeftPanel::VisTavLeftPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    this->vis_tav = tav;
    this->FUN_00497ace();
}


// 4A3E10
VisTavLeftPanel::~VisTavLeftPanel()
{
    if (this->field_0x160 != nullptr) {
        delete this->field_0x160;
    }
    if (this->field_0x164 != nullptr) {
        delete this->field_0x164;
    }
}


// 497ACE
void VisTavLeftPanel::FUN_00497ace()
{
    this->flags |= 2;
    CRect client_rect;
    this->ClientRectToScreen(&client_rect, this->rect);
    this->field_0x170 = CRect(CPoint(client_rect.left + 0x11, client_rect.top + 0x1BB), CSize(0x20, 0x20));
    this->field_0x180 = CRect(CPoint(client_rect.left + 0x87, client_rect.top + 0x1BB), CSize(0x20, 0x20));
    this->field_0x164 = new CBmp256(0xA0, 0xF0);
    this->field_0x160 = new CBmp64(0xA0, 0xF0);
    this->field_0x60[0] = 0;
    this->field_0x16c = nullptr;
    this->field_0x168 = nullptr;
}


// 497EC1
int32_t VisTavLeftPanel::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    return 0;
}


// 497ED0
void VisTavLeftPanel::VMethod7()
{
    if (this->vis_tav->selection_index < 0) {
        this->FUN_00497f82(nullptr);
    } else if (this->vis_tav->selection_index < this->vis_tav->avail_entries.GetSize()) {
        this->FUN_00497f82(this->vis_tav->avail_entries[this->vis_tav->selection_index]);
    } else {
        this->FUN_00497f82(this->vis_tav->reserved_entries[this->vis_tav->selection_index - this->vis_tav->avail_entries.GetSize()]);
    }
}


// 497F82
void VisTavLeftPanel::FUN_00497f82(CUnit* unit)
{
    CRect screen_rect;
    CRect rect_1C(CPoint(0xC, 0), CSize(0xA0, 0xEE));
    this->ClientRectToScreen(&screen_rect, this->rect);
    LockSurface2();
    this->field_0x168->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
    this->field_0x16c->VMethod2(screen_rect.left, screen_rect.top + 0xEE, 0, 0, 0);

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode == 2) {
        if (unit == nullptr) {
            g_font2->DrawTextWithShadow(screen_rect.left + 0x58, screen_rect.top + 0x36, TxtFile::AllLines[0x2F], 2, clrsh_DullGold, 1);
            g_font2->DrawTextWithShadow(screen_rect.left + 0x58, screen_rect.top + 0x42, TxtFile::AllLines[0x30], 2, clrsh_DullGold, 1);
        } else {
            if ((unit->unitFlags & 0x40) == 0) {
                CRect unit_rect = rect_1C + screen_rect.TopLeft();
                unit->FUN_0046c124(&unit_rect);
            }
            if ((unit->unitFlags & 0x11) == 0) {
                char picture[0x100];
                char suffix[0x50];
                strcpy(picture, g_VFX_info[unit->typeId]->info_picture);
                sprintf(suffix, "%d", unit->face);
                if (unit->face > 1) {
                    strcat(picture, suffix);
                }
                if (strcmp(this->field_0x60, picture) != 0) {
                    strcpy(this->field_0x60, picture);
                    char path[0x100];
                    sprintf(path, "graphics\\infowindow\\%s.bmp", this->field_0x60);
                    this->field_0x160->LoadFile(path, nullptr);
                    memset(this->field_0x164->GetData(), 0, this->field_0x164->GetWidth() * this->field_0x164->GetHeight());
                }
            } else {
                char temp_path[0x100];
                char fname[0x100];
                char full_path[0x100];
                GetTempPathA(0x100, temp_path);
                sprintf(fname, "allods-2-%d.$$$", unit->unit_id);
                sprintf(full_path, "%s%s", temp_path, fname);
                if (strcmp(this->field_0x60, fname) != 0) {
                    strcpy(this->field_0x60, fname);
                    if ((unit->unitFlags & 8) != 0) {
                        UnlockSurface2();
                        unit->VMethod30(full_path, this->field_0x160, this->field_0x164);
                        LockSurface2();
                    } else {
                        this->field_0x160->LoadFile(fname, this->field_0x164);
                    }
                } else if ((unit->unitFlags & 8) != 0) {
                    UnlockSurface2();
                    unit->VMethod30(full_path, this->field_0x160, this->field_0x164);
                    LockSurface2();
                }
            }
            this->field_0x160->VMethod10(screen_rect.left + 0xB, screen_rect.top + 0xF0, 0, 0, 0xA0, 0xF0);
        }
    }

    uint32_t quest_result = main_wnd->vis_map_context->field_0x4970->VMethod1(0x11, main_wnd->vis_map_context->my_main_unit->index, this->vis_tav->field_0x138);
    Quest* quest;
    int32_t quest_found;
    if (quest_result != 0) {
        quest_found = main_wnd->vis_map_context->field_0x4970->FUN_004a47a0(quest_result, &quest);
    } else {
        quest_found = this->vis_tav->quest_map->FUN_004a47a0(this->vis_tav->quest_id, &quest);
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
                found = main_wnd->vis_map_context->FUN_0041e2af(quest->GetObj(), (CUnit**)&obj);
            } else {
                found = main_wnd->vis_map_context->field_0x9d0.Lookup((uint16_t)quest->GetObj(), obj);
            }
            CUnit* target = (CUnit*)obj;
            if (found != 0) {
                if ((target->unitFlags & 0x11) != 0) {
                    char temp_path[0x100];
                    char fname[0x100];
                    char full_path[0x100];
                    GetTempPathA(0x100, temp_path);
                    sprintf(fname, "allods-2-%d.$$$", target->unit_id);
                    sprintf(full_path, "%s%s", temp_path, fname);
                    if (strcmp(this->field_0x60, fname) != 0) {
                        strcpy(this->field_0x60, fname);
                        if ((target->unitFlags & 8) != 0) {
                            UnlockSurface2();
                            target->VMethod30(full_path, this->field_0x160, this->field_0x164);
                            LockSurface2();
                        } else {
                            this->field_0x160->LoadFile(fname, this->field_0x164);
                        }
                    } else if ((target->unitFlags & 8) != 0) {
                        UnlockSurface2();
                        target->VMethod30(full_path, this->field_0x160, this->field_0x164);
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
                    if (strcmp(this->field_0x60, picture) != 0) {
                        strcpy(this->field_0x60, picture);
                        char path[0x100];
                        sprintf(path, "graphics\\infowindow\\%s.bmp", this->field_0x60);
                        this->field_0x160->LoadFile(path, nullptr);
                        memset(this->field_0x164->GetData(), 0, this->field_0x164->GetWidth() * this->field_0x164->GetHeight());
                    }
                    if (obj->typeId >= 0x52 && obj->typeId <= 0x66) {
                        unit_name.Format("%s", txt_unitname.GetLine(obj->typeId));
                    } else {
                        unit_name.Format("%s[%d]", txt_unitname.GetLine(obj->typeId), target->face);
                    }
                }
                this->field_0x160->VMethod10(screen_rect.left + 0xB, screen_rect.top + 0xF0, 0, 0, 0xA0, 0xF0);
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
                if (strcmp(this->field_0x60, picture) != 0) {
                    strcpy(this->field_0x60, picture);
                    char path[0x100];
                    sprintf(path, "graphics\\infowindow\\%s.bmp", this->field_0x60);
                    this->field_0x160->LoadFile(path, nullptr);
                    memset(this->field_0x164->GetData(), 0, this->field_0x164->GetWidth() * this->field_0x164->GetHeight());
                }
                this->field_0x160->VMethod10(screen_rect.left + 0xB, screen_rect.top + 0xF0, 0, 0, 0xA0, 0xF0);
            }
            break;
        }
        default:
            break;
        }

        CString area_name;
        CString building_name;
        CGameObject* landmark;
        if (main_wnd->vis_map_context->field_0x9d0.Lookup((uint16_t)quest->GetLandmarkId(), landmark)) {
            building_name = txt_building.GetLine(landmark->typeId - 1);
            int32_t cell_x = ((landmark->tileX - 8) * 5) / (main_wnd->vis_map_context->field_0x84 - 0x10);
            int32_t cell_y = ((landmark->tileY - 8) * 5) / (main_wnd->vis_map_context->field_0x88 - 0x10);
            area_name = TxtFile::AllLines[cell_x + 0x13D + cell_y * 5];
        }

        CString quest_text;
        switch (quest->Kind()) {
        case 1:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], (const char*)unit_name, (const char*)area_name, (const char*)building_name);
            break;
        case 2:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->FUN_004a4780(), (const char*)unit_name);
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
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->FUN_004a4780() / 0x3C0, (quest->FUN_004a4780() >> 4) % 0x3C, (const char*)area_name);
            break;
        case 8:
        case 9:
        case 0xA:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->FUN_004a4780());
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
            strcpy(picture, g_VFX_info[this->vis_tav->rewards[this->vis_tav->quest_id]->field_0x10 & 0xFF]->info_picture);
            sprintf(suffix, "%d", (int32_t)this->vis_tav->rewards[this->vis_tav->quest_id]->field_0x10 >> 8);
            if ((int32_t)this->vis_tav->rewards[this->vis_tav->quest_id]->field_0x10 >> 8 > 1) {
                strcat(picture, suffix);
            }
            if (picture[0] != 0) {
                if (strcmp(this->field_0x60, picture) != 0) {
                    strcpy(this->field_0x60, picture);
                    char path[0x100];
                    sprintf(path, "graphics\\infowindow\\%s.bmp", this->field_0x60);
                    this->field_0x160->LoadFile(path, nullptr);
                    memset(this->field_0x164->GetData(), 0, this->field_0x164->GetWidth() * this->field_0x164->GetHeight());
                }
                this->field_0x160->VMethod10(screen_rect.left + 0xB, screen_rect.top + 0xF0, 0, 0, 0xA0, 0xF0);
            }
        }
    }

    UnlockSurface2();
}


// 497E06
void VisTavLeftPanel::FUN_00497e06(int32_t idx)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (main_wnd->sessionMode == 2) {
        this->vis_tav->selection_index = idx;
        this->vis_tav->right_panel->FUN_0049a973();
    } else {
        this->vis_tav->quest_id = idx;
    }

    CSound::Play(this->vis_tav->sounds[9]);
}


// 49A30A
int32_t VisTavRightPanel::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (this->field_0xc0 >= 0 && this->field_0xc0 < 3 && this->FUN_0049a873(pos) == this->field_0xc0) {
        int32_t button = this->field_0xc0;
        this->field_0xc0 = -1;
        this->FUN_0049a8fa(wparam, pos);

        if (main_wnd->sessionMode == 2) {
            if (button == 0) {
                if (this->vis_tav->selection_index < this->vis_tav->avail_entries.GetSize()) {
                    CUnit* unit = this->vis_tav->avail_entries[this->vis_tav->selection_index];
                    if (this->vis_tav->entrie_id[this->vis_tav->FUN_0049e2e3(unit)] & 0x80000000) {
                        this->vis_tav->FUN_0049f0da();
                    } else {
                        this->vis_tav->FUN_0049ef63();
                    }
                    this->FUN_0049a973();
                }
            } else if (button == 1) {
                if (this->vis_tav->selection_index < this->vis_tav->avail_entries.GetSize()) {
                    CUnit* unit = this->vis_tav->avail_entries[this->vis_tav->selection_index];
                    CString str;
                    str.Format("npc%dabout", unit->serverId);
                    ShowRoleKeyDialog(str);
                    CSound::Play(this->vis_tav->sounds[12]);
                } else {
                    this->vis_tav->FUN_0049f179();
                }
            } else if (button == 2) {
                this->vis_tav->FUN_0049edec();
            }
        } else {
            if (button == 0) {
                if (main_wnd->vis_map_context->field_0x4970->VMethod1(0x11, main_wnd->vis_map_context->my_main_unit->index, this->vis_tav->field_0x138) != 0) {
                    this->vis_tav->field_0x13c = (this->vis_tav->field_0x13c == 0);
                } else {
                    if (this->vis_tav->quest_map->FUN_0041ec00() != 0 || this->vis_tav->rewards.GetSize() != 0) {
                        if (this->vis_tav->quest_selection == this->vis_tav->quest_id) {
                            this->vis_tav->quest_selection = -1;
                        } else {
                            this->vis_tav->quest_selection = this->vis_tav->quest_id;
                        }
                    }
                }
            } else if (button == 1) {
                if (this->vis_tav->quest_map->FUN_0041ec00() != 0) {
                    Quest* quest;
                    if (this->vis_tav->quest_map->FUN_004a47a0(this->vis_tav->quest_id, &quest) != 0) {
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
                this->vis_tav->FUN_0049edec();
            }
        }
    }

    this->field_0xc0 = -1;
    this->FUN_0049a8fa(wparam, pos);
    return 1;
}


// 499A67
void VisTavRightPanel::FUN_00499a67()
{
    this->FUN_00499cdf();
    this->field_0x74[0] = new CBmp64("graphics\\interface\\Inn\\button1on.bmp");
    g_mousept.Update();
    this->field_0x74[1] = new CBmp64("graphics\\interface\\Inn\\button2on.bmp");
    g_mousept.Update();
    this->field_0x74[2] = new CBmp64("graphics\\interface\\Inn\\button3on.bmp");
    g_mousept.Update();
    this->field_0x80[0] = new CBmp64("graphics\\interface\\Inn\\button1off.bmp");
    g_mousept.Update();
    this->field_0x80[1] = new CBmp64("graphics\\interface\\Inn\\button2off.bmp");
    g_mousept.Update();
    this->field_0x80[2] = new CBmp64("graphics\\interface\\Inn\\button3off.bmp");
    g_mousept.Update();
    this->field_0x8c = new CBmp64("graphics\\interface\\Inn\\ButtonsArea.bmp");
    g_mousept.Update();
}


// 49A8FA
void VisTavRightPanel::FUN_0049a8fa(uint32_t wparam, CPoint pos)
{
    int32_t index = this->FUN_0049a873(pos);
    if (index < 0 || (wparam & 1) != 0) {
        if (index < 0 || index != this->field_0xc0 || (wparam & 1) == 0) {
            this->field_0xc4 = -1;
        } else {
            this->field_0xc4 = index;
        }
    } else {
        this->field_0xc4 = index;
    }
}


// 49A873
int32_t VisTavRightPanel::FUN_0049a873(CPoint pos)
{
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    CPoint pt(pos.x - tav_topleft.x, pos.y - tav_topleft.y);

    for (int32_t i = 0; i < 3; i++) {
        if (this->field_0x90[i].PtInRect(pt)) {
            return i;
        }
    }
    return -1;
}


// 4998C2
void VisTavRightPanel::FUN_004998c2()
{
    this->field_0xc0 = -1;
    this->field_0xc4 = -1;
    this->field_0x90[0] = CRect(0x1E4, 0x2C, 0x270, 0x5A);
    this->field_0x90[1] = CRect(0x1E4, 0x5B, 0x270, 0x89);
    this->field_0x90[2] = CRect(0x1E4, 0x8A, 0x270, 0xB8);
    for (int32_t i = 0; i < 3; i++) {
        this->field_0x74[i] = nullptr;
        this->field_0x80[i] = nullptr;
    }
    this->field_0x8c = nullptr;

    this->texts.SetSize(3, -1);
    this->texts[0] = TxtFile::AllLines[0xF3];
    this->texts[1] = TxtFile::AllLines[0xF2];
    this->texts[2] = TxtFile::AllLines[0xE8];
    this->flags |= 2;
}


// 4997C6
VisTavRightPanel::VisTavRightPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    this->vis_tav = tav;
    this->FUN_004998c2();
}


// 4A3E40
VisTavRightPanel::~VisTavRightPanel()
{
    this->FUN_00499cdf();
}


// 4A4710
const char* VisTavRightPanel::GetHint()
{
    return nullptr;
}


// 49A84E
void VisTavRightPanel::FUN_0049a84e()
{
    this->field_0xc0 = -1;
    this->field_0xc4 = -1;
}


// 49A26E
int32_t VisTavRightPanel::OnMouseMove(uint32_t wparam, CPoint pos)
{
    this->FUN_0049a8fa(wparam, pos);
    return 0;
}


// 49A2E6
int32_t VisTavRightPanel::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    return this->OnLButtonDown(wparam, pos);
}


// 49A291
int32_t VisTavRightPanel::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    this->field_0xc0 = this->FUN_0049a873(pos);
    if (this->field_0xc0 == 2) {
        CSound::Play(this->vis_tav->sounds[11]);
    }
    return 1;
}


// 499CDF
void VisTavRightPanel::FUN_00499cdf()
{
    for (int32_t i = 0; i < 3; i++) {
        if (this->field_0x74[i] != nullptr) {
            delete this->field_0x74[i];
        }
        this->field_0x74[i] = nullptr;
        if (this->field_0x80[i] != nullptr) {
            delete this->field_0x80[i];
        }
        this->field_0x80[i] = nullptr;
    }

    if (this->field_0x8c != nullptr) {
        delete this->field_0x8c;
    }
    this->field_0x8c = nullptr;
}


// 49A973
void VisTavRightPanel::FUN_0049a973()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (main_wnd->sessionMode == 2) {
        if (this->vis_tav->avail_entries.GetUpperBound() < this->vis_tav->selection_index || this->vis_tav->selection_index == -1) {
            this->texts[0] = "";
        } else {
            CUnit* unit = this->vis_tav->avail_entries[this->vis_tav->selection_index];
            if (this->vis_tav->entrie_id[this->vis_tav->FUN_0049e2e3(unit)] & 0x80000000) {
                this->texts[0] = TxtFile::AllLines[0x103];
            } else {
                this->texts[0] = TxtFile::AllLines[0x102];
            }
        }
        this->texts[1] = TxtFile::AllLines[0xF2];
    } else {
        if (this->vis_tav->quest_map->FUN_0041ec00() != 0) {
            this->texts[0] = TxtFile::AllLines[0x15F];
            this->texts[1] = TxtFile::AllLines[0x160];
        } else {
            if (this->vis_tav->rewards.GetSize() != 0) {
                this->texts[0] = TxtFile::AllLines[0x164];
                this->texts[1] = TxtFile::AllLines[0x165];
            } else {
                if (main_wnd->vis_map_context->field_0x4970->VMethod1(0x11, main_wnd->vis_map_context->my_main_unit->index, this->vis_tav->field_0x138) != 0) {
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
void VisTavRightPanel::VMethod7()
{
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    if (this->vis_tav->dialog_active == 0) {
        return;
    }

    LockSurface2();
    this->field_0x8c->VMethod2(tav_topleft.x + this->rect.left, tav_topleft.y + this->rect.top, 0, 0, 0);

    uint16_t* pal;
    if (this->field_0xc4 == 1) {
        pal = palette_paris_daisy->GetPalette(0);
    } else {
        pal = palette_husk->GetPalette(0);
    }

    if (this->field_0xc4 < 0 || this->field_0xc0 != this->field_0xc4 || this->field_0xc4 != 1) {
        this->field_0x80[1]->VMethod2(tav_topleft.x + this->field_0x90[1].left, tav_topleft.y + this->field_0x90[1].top, 0, 0, 0);
        g_font4->DrawTxt(tav_topleft.x + this->field_0x90[1].left + this->field_0x90[1].Width() / 2, tav_topleft.y + this->field_0x90[1].top + this->field_0x90[1].Height() / 2, this->texts[1], 10, pal);
    } else {
        this->field_0x74[1]->VMethod2(tav_topleft.x + this->field_0x90[1].left, tav_topleft.y + this->field_0x90[1].top, 0, 0, 0);
        g_font4->DrawTxt(tav_topleft.x + this->field_0x90[1].left + this->field_0x90[1].Width() / 2, tav_topleft.y + this->field_0x90[1].top + 1 + this->field_0x90[1].Height() / 2, this->texts[1], 10, pal);
    }

    for (int32_t i = 0; i < 3; i++) {
        if (i == 1) {
            continue;
        }
        if (this->field_0xc4 == i) {
            pal = palette_paris_daisy->GetPalette(0);
        } else {
            pal = palette_husk->GetPalette(0);
        }
        if (this->field_0xc4 < 0 || this->field_0xc0 != this->field_0xc4 || this->field_0xc4 != i) {
            this->field_0x80[i]->VMethod2(tav_topleft.x + this->field_0x90[i].left, tav_topleft.y + this->field_0x90[i].top, 0, 0, 0);
            g_font4->DrawTxt(tav_topleft.x + this->field_0x90[i].left + this->field_0x90[i].Width() / 2, tav_topleft.y + this->field_0x90[i].top + this->field_0x90[i].Height() / 2, this->texts[i], 10, pal);
        } else {
            this->field_0x74[i]->VMethod2(tav_topleft.x + this->field_0x90[i].left, tav_topleft.y + this->field_0x90[i].top, 0, 0, 0);
            g_font4->DrawTxt(tav_topleft.x + this->field_0x90[i].left + this->field_0x90[i].Width() / 2, tav_topleft.y + this->field_0x90[i].top + 1 + this->field_0x90[i].Height() / 2, this->texts[i], 10, pal);
        }
    }

    UnlockSurface2();
}


// 4010EE
void VisTavSceneAnim::FUN_004010ee(CStringArray* names)
{
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
void VisTavSceneAnim::FUN_004014f2()
{
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
bool VisTavSceneAnim::StepForward()
{
    if (this->frame_idx == this->frames.GetUpperBound()) {
        return false;
    }
    this->frame_idx++;
    this->current_frame = this->frames.GetAt(this->frame_idx);
    return this->current_frame != nullptr;
}


// 401613
bool VisTavSceneAnim::StepBackward()
{
    if (this->frame_idx == 0) {
        return false;
    }
    this->frame_idx--;
    this->current_frame = this->frames.GetAt(this->frame_idx);
    return this->current_frame != nullptr;
}


// 401659
void VisTavSceneAnim::NextFrame()
{
    this->frame_idx = (this->frame_idx + 1) % this->frames.GetUpperBound();
    this->current_frame = this->frames.GetAt(this->frame_idx);
}


// 4016EF
void VisTavSceneAnim::Draw(int32_t x, int32_t y)
{
    this->current_frame->VMethod2(x, y, 0, 0, 0);
}


// Statics for VisTavScene::VMethod7 (665D60-665D84 in the binary).
static bool tavscene_statics_inited = false;
static uint32_t tavscene_anim_delay = 0;    // 665D78 delay before tavern animation; bit 0 selects which one plays
static uint32_t tavscene_frame_ts = 0;      // 665D3C
static uint32_t tavscene_anims_ts = 0;      // 665D68
static uint32_t tavscene_state_ts = 0;      // 665D38
static uint32_t tavscene_ambient_ts = 0;    // 665D7C
static int32_t tavscene_anim_state = 0;     // 665D80 0=none, 1=anims[3], 2=anims[2]
static int32_t tavscene_anim_substate = 0;  // 665D84 1=playing forward, -1=playing backward


// 49B22F
void VisTavScene::VMethod7()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (!tavscene_statics_inited) {
        tavscene_statics_inited = true;
        tavscene_anim_delay = 3000 + rand() % 16;
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

    this->field_0x27c->VMethod2(x, y, 0, 0, 0);

    if (main_wnd->sessionMode == 2) {
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
                        tavscene_anim_delay = 3000 + rand() % 16;
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
                    tavscene_anim_delay = 3000 + rand() % 16;
                    this->anims[2].frame_idx = 0;
                }
                tavscene_state_ts = now;
            }
        }

        int32_t avail_count = this->vis_tav->avail_entries.GetSize();
        for (int32_t i = 0; i < avail_count; i++) {
            int32_t row = i / 6;
            int32_t col = i % 6;
            CPoint pt = this->field_0x60[row * 6 + col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            this->field_0x274->VMethod2(x + pt.x, y + pt.y, 0, 0, 0);
            CSprite256* sprite = this->field_0x224[i];
            sprite->VMethod2(x + pt.x, y + pt.y, this->field_0x24c[i], 0, 0);
            if (this->vis_tav->selection_index == i && now - tavscene_frame_ts > 0x7D) {
                this->field_0x24c[i] = (this->field_0x24c[i] + 1) % sprite->GetFrameCount();
                tavscene_frame_ts = now;
            }

            CUnit* unit = this->vis_tav->avail_entries[i];
            int32_t entry = this->vis_tav->FUN_0049e2e3(unit);
            if (this->vis_tav->entrie_id[entry] & 0x80000000) {
                CRect& r = this->field_0x60[row * 6 + col];
                g_font2->DrawTextWithShadow(x + r.left + r.Width() / 2, y + r.top + r.Height() / 2, TxtFile::AllLines[0x101], 10, clrsh_CoralRed, 1);
            }
        }

        for (int32_t i = 0; i < this->vis_tav->reserved_entries.GetSize(); i++) {
            int32_t row = (avail_count + i) / 6;
            int32_t col = (avail_count + i) % 6;
            CPoint pt = this->field_0x60[row * 6 + col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            if (this->vis_tav->selection_index == avail_count + i && now - tavscene_frame_ts > 0x7D) {
                this->field_0x260[i] = (this->field_0x260[i] + 1) % this->field_0x238[i]->GetFrameCount();
                tavscene_frame_ts = now;
            }
            this->field_0x278->VMethod2(x + pt.x, y + pt.y, 0, 0, 0);
            this->field_0x238[i]->VMethod2(x + pt.x, y + pt.y, this->field_0x260[i], 0, 0);
        }
    } else {
        this->FUN_0049bc23();
    }

    this->field_0x340->VMethod10(x + 0xA0, y, 0, 0, 0x10, 0xEE);
    this->field_0x344->VMethod10(x + 0xA0, y + 0xEE, 0, 0, 0x10, 0xF2);
    this->field_0x348->VMethod10(x + 0x1D0, y, 0, 0, 0x10, 0xEE);
    if (this->vis_tav->info_panel->info_mode) {
        g_bmp_humanbackl->VMethod10(x + 0x1D0, y + 0xEE, 0, 0, 0x10, 0xF2);
    } else {
        g_bmp_textbackl->VMethod10(x + 0x1D0, y + 0xEE, 0, 0, 0x10, 0xF2);
    }
    UnlockSurface2();
    CVisualObject::VMethod7();
}


// 49D5F9
void VisTavScene::VMethod29()
{
    delete this->field_0x274;
    this->field_0x274 = nullptr;
    delete this->field_0x278;
    this->field_0x278 = nullptr;
    delete this->field_0x27c;
    this->field_0x27c = nullptr;
    delete this->field_0x340;
    this->field_0x340 = nullptr;
    delete this->field_0x344;
    this->field_0x344 = nullptr;
    delete this->field_0x348;
    this->field_0x348 = nullptr;

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != 2) {
        for (int32_t i = 0; i < 13; i++) {
            delete this->field_0x34c[i];
            this->field_0x34c[i] = nullptr;
        }
    }

    for (int32_t i = 0; i < this->field_0x210.GetSize(); i++) {
        if (this->field_0x210[i] != nullptr) {
            delete this->field_0x210[i];
            this->field_0x210[i] = nullptr;
        }
    }
    this->field_0x210.RemoveAll();
}


// 49CED4
void VisTavScene::VMethod26()
{
    this->field_0x224.SetSize(this->vis_tav->avail_entries.GetSize(), -1);
    this->field_0x24c.SetSize(this->vis_tav->avail_entries.GetSize(), -1);
    this->field_0x238.SetSize(this->vis_tav->reserved_entries.GetSize(), -1);
    this->field_0x260.SetSize(this->vis_tav->reserved_entries.GetSize(), -1);

    CString name;
    for (int32_t i = 0; i < this->vis_tav->avail_entries.GetSize(); i++) {
        CUnit* unit = this->vis_tav->avail_entries[i];
        name.Format("graphics\\interface\\inn\\Unit%d\\sprites.16a", unit->typeId);
        this->field_0x224[i] = new CA16(name);
        this->field_0x224[i]->ResetPalette(0x10, 4, 0);
    }

    for (int32_t i = 0; i < this->vis_tav->reserved_entries.GetSize(); i++) {
        CUnit* unit = this->vis_tav->reserved_entries[i];
        if (unit->unitFlags & 1) {
            if (unit->unitFlags & 2) {
                name.Format("graphics\\interface\\inn\\HeroMage\\sprites.16a");
            } else {
                name.Format("graphics\\interface\\inn\\HeroFighter\\sprites.16a");
            }
        } else {
            name.Format("graphics\\interface\\inn\\Unit%d\\sprites.16a", unit->typeId);
        }
        this->field_0x238[i] = new CA16(name);
        this->field_0x238[i]->ResetPalette(0x10, 4, 0);
    }
}


// 49D2BB
void VisTavScene::VMethod28()
{
    this->VMethod29();
    this->field_0x274 = new CBmp64("graphics\\Interface\\Inn\\manback.bmp");
    g_mousept.Update();
    this->field_0x278 = new CBmp64("graphics\\Interface\\Inn\\ManBackTalk.bmp");
    g_mousept.Update();
    this->field_0x27c = new CBmp64("graphics\\Interface\\Inn\\CenterArea.bmp");
    g_mousept.Update();
    this->field_0x340 = new CBmp64("graphics\\interface\\inn\\LUOver.bmp");
    g_mousept.Update();
    this->field_0x344 = new CBmp64("graphics\\interface\\inn\\LDOver.bmp");
    g_mousept.Update();
    this->field_0x348 = new CBmp64("graphics\\interface\\inn\\RUOver.bmp");
    g_mousept.Update();

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != 2) {
        CString name;
        for (int32_t i = 0; i < 13; i++) {
            name.Format("graphics\\interface\\inn\\quests\\%02d.bmp", i + 1);
            this->field_0x34c[i] = new CBmp64(name);
        }
    }
}


// 49CD2B
const char* VisTavScene::GetHint()
{
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    CPoint pt = CPoint(g_mousept.GetX(), g_mousept.GetY()) - tav_topleft;

    for (int32_t i = 0; i < this->vis_tav->rewards.GetSize(); i++) {
        int32_t row = i / 3;
        int32_t col = i % 3;
        if (this->field_0x180[row * 3 + col].PtInRect(pt)) {
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
            return entry->FUN_00439973();
        }
    }
    return nullptr;
}


// 49D8E8
int32_t VisTavScene::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (main_wnd->sessionMode != 2) {
        if (main_wnd->vis_map_context->field_0x4970->VMethod1(0x11, main_wnd->vis_map_context->my_main_unit->index, this->vis_tav->field_0x138) != 0) {
            this->vis_tav->field_0x13c = (this->vis_tav->field_0x13c == 0) ? 1 : 0;
            return 1;
        }
    }

    if (this->OnLButtonDown(wparam, pos) == 0) {
        return 0;
    }

    if (main_wnd->sessionMode == 2) {
        if (this->vis_tav->selection_index < this->vis_tav->avail_entries.GetSize()) {
            CUnit* unit = this->vis_tav->avail_entries[this->vis_tav->selection_index];
            int32_t entry = this->vis_tav->FUN_0049e2e3(unit);
            if (this->vis_tav->entrie_id[entry] & 0x80000000) {
                this->vis_tav->FUN_0049f0da();
            } else {
                this->vis_tav->FUN_0049ef63();
            }
            this->vis_tav->right_panel->FUN_0049a973();
        } else {
            this->vis_tav->FUN_0049f179();
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
void VisTavScene::VMethod27()
{
    for (int32_t i = 0; i < this->field_0x224.GetSize(); i++) {
        if (this->field_0x224[i] != nullptr) {
            delete this->field_0x224[i];
        }
    }
    this->field_0x224.RemoveAll();
    this->field_0x24c.RemoveAll();

    for (int32_t i = 0; i < this->field_0x238.GetSize(); i++) {
        if (this->field_0x238[i] != nullptr) {
            delete this->field_0x238[i];
        }
    }
    this->field_0x238.RemoveAll();
    this->field_0x260.RemoveAll();
}


// 49DA98
int32_t VisTavScene::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    int32_t idx = this->FUN_0049cab8(&pos);
    if (idx == -1) {
        return 0;
    }
    this->vis_tav->left_panel->FUN_00497e06(idx);
    return 1;
}


// 49D8D9
int32_t VisTavScene::OnMouseMove(uint32_t wparam, CPoint pos)
{
    return 0;
}


// 4A3E70
VisTavScene::~VisTavScene()
{
    this->VMethod29();
}


// 49AD56
VisTavScene::VisTavScene(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    this->vis_tav = tav;
    this->FUN_0049af8c();
}


// 49BC23
void VisTavScene::FUN_0049bc23()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();

    int32_t quest_found = main_wnd->vis_map_context->field_0x4970->VMethod1(0x11, main_wnd->vis_map_context->my_main_unit->index, this->vis_tav->field_0x138);
    if (quest_found != 0) {
        Quest* quest = nullptr;
        this->vis_tav->quest_map->FUN_004a47a0(quest_found, &quest);
        CPoint pt = this->field_0x60[0].TopLeft();
        int32_t kind = quest->Kind();
        this->field_0x34c[kind - 1]->VMethod2(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
        if (this->vis_tav->field_0x13c == 0) {
            CRect& r = this->field_0x60[0];
            g_font2->DrawTextWithShadow(tav_topleft.x + r.left + r.Width() / 2, tav_topleft.y + r.top + r.Height() / 2, TxtFile::AllLines[0x158], 10, clrsh_CoralRed, 1);
        }
        return;
    }

    if (this->vis_tav->quest_map->FUN_0041ec00() != 0) {
        POSITION it = this->vis_tav->quest_map->quests_map.GetStartPosition();
        int32_t i = 0;
        while (it != nullptr) {
            uint32_t key;
            Quest* quest;
            this->vis_tav->quest_map->quests_map.GetNextAssoc(it, key, quest);
            int32_t row = i / 6;
            int32_t col = i % 6;
            CPoint pt = this->field_0x60[row * 6 + col].TopLeft();
            int32_t kind = quest->Kind();
            this->field_0x34c[kind - 1]->VMethod2(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            if (this->vis_tav->quest_selection == key) {
                CRect& r = this->field_0x60[row * 6 + col];
                g_font2->DrawTextWithShadow(tav_topleft.x + r.left + r.Width() / 2, tav_topleft.y + r.top + r.Height() / 2, TxtFile::AllLines[0x158], 10, clrsh_CoralRed, 1);
            }
            i++;
        }
        return;
    }

    for (int32_t i = 0; i < this->vis_tav->rewards.GetSize(); i++) {
        int32_t row = i / 3;
        int32_t col = i % 3;
        CPoint pt = this->field_0x180[row * 3 + col].TopLeft();
        g_bmp_backinv->VMethod2(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
        if (this->vis_tav->quest_id == i) {
            sub_457C5D(tav_topleft.x + pt.x, tav_topleft.y + pt.y, tav_topleft.x + pt.x + 0x50, tav_topleft.y + pt.y + 0x50, 2);
        }
        if (this->field_0x210.GetSize() <= i) {
            this->field_0x210.SetSize(i + 1, -1);
        }

        TokenEntry* entry = this->vis_tav->rewards[i];
        if (entry->item_id == 0xFFFD) {
            g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 0x28, tav_topleft.y + pt.y + 0x23, TxtFile::AllLines[0x166], 2, clrsh_DullGold, 1);
        } else if (entry->item_id == 0xFFFE) {
            g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 0x28, tav_topleft.y + pt.y + 0x23, TxtFile::AllLines[0x2E], 2, clrsh_DullGold, 1);
            CString text;
            text.Format("%d", entry->field_0x10 * 250);
            FUN_00476987(&text);
            g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 3, tav_topleft.y + pt.y + 0x42, text, 0, clrsh_DullGold, 1);
        } else if (entry->item_id == 0xFFFF) {
            g_ca16_money->VMethod2(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            CString text;
            text.Format("%d", entry->field_0x10 * 250);
            FUN_00476987(&text);
            g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 3, tav_topleft.y + pt.y + 0x42, text, 0, clrsh_DullGold, 1);
        } else {
            if (this->field_0x210[i] == nullptr) {
                if (this->field_0x210.GetSize() <= i) {
                    this->field_0x210.SetSize(i + 1, -1);
                }
                CString path = "graphics\\inventory\\" + entry->FUN_004394f3() + ".16a";
                this->field_0x210[i] = new CA16(path);
                this->field_0x210[i]->ResetPalette(0x10, 4, 0);
            }
            this->field_0x210[i]->VMethod2(tav_topleft.x + pt.x, tav_topleft.y + pt.y, 0, 0, 0);
            if (entry->field_0x10 > 1) {
                char buf[80];
                sprintf(buf, "%d", entry->field_0x10);
                g_font2->DrawTextWithShadow(tav_topleft.x + pt.x + 3, tav_topleft.y + pt.y + 0x42, buf, 0, clrsh_DullGold, 1);
            }

            if (entry->flg & 0x20) {
                uint32_t t = GetTickCount() / 120;
                int32_t cell_x = tav_topleft.x + pt.x;
                int32_t cell_y = tav_topleft.y + pt.y;
                static const uint32_t sparkle_alpha[7] = {0x3F, 0x7F, 0xBF, 0xFF, 0xBF, 0x7F, 0x3F};
                for (uint32_t k = 0; k < 7; k++) {
                    if (t >= k) {
                        sub_4588EC(cell_x + main_wnd->vis_invtype1->random_offsets1[(t - k) & 0x3FF],
                                   cell_y + main_wnd->vis_invtype1->random_offsets2[(t - k) & 0x3FF],
                                   0xFF, 0, 0xFF, sparkle_alpha[k]);
                    }
                }
                main_wnd->vis_invtype1->sub_4A5FAB(cell_x, cell_y, i);
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
void VisTavScene::FUN_0049af8c()
{
    CPoint base(this->rect.left + 0x10, this->rect.bottom);

    for (int32_t i = 0; i < 3; i++) {
        for (int32_t j = 0; j < 6; j++) {
            CPoint pt = base + CPoint(j * 0x30, -(i + 1) * 0x40);
            this->field_0x60[i * 6 + j] = CRect(pt, CSize(0x30, 0x40));
        }
    }

    for (int32_t i = 0; i < 3; i++) {
        for (int32_t j = 0; j < 3; j++) {
            CPoint pt = base + CPoint(j * 0x50 + 0x18, -(i + 1) * 0x50);
            this->field_0x180[i * 3 + j] = CRect(pt, CSize(0x50, 0x50));
        }
    }

    this->field_0x274 = nullptr;
    this->field_0x278 = nullptr;
    this->field_0x27c = nullptr;
    this->field_0x340 = nullptr;
    this->field_0x344 = nullptr;
    this->field_0x348 = nullptr;
    for (int32_t i = 0; i < 13; i++) {
        this->field_0x34c[i] = nullptr;
    }
}


// 49CAB8
int32_t VisTavScene::FUN_0049cab8(CPoint* pos)
{
    CPoint tav_topleft = this->vis_tav->rect.TopLeft();
    CPoint pt = *pos - tav_topleft;

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (main_wnd->sessionMode == 2) {
        int32_t count = this->vis_tav->avail_entries.GetSize() + this->vis_tav->reserved_entries.GetSize();
        for (int32_t i = 0; i < count; i++) {
            int32_t row = i / 6;
            int32_t col = i % 6;
            if (this->field_0x60[row * 6 + col].PtInRect(pt)) {
                return i;
            }
        }
        return -1;
    }

    if (this->vis_tav->quest_map->FUN_0041ec00() != 0) {
        POSITION it = this->vis_tav->quest_map->quests_map.GetStartPosition();
        int32_t i = 0;
        while (it != nullptr) {
            uint32_t key;
            Quest* quest;
            this->vis_tav->quest_map->quests_map.GetNextAssoc(it, key, quest);
            int32_t row = i / 6;
            int32_t col = i % 6;
            if (this->field_0x60[row * 6 + col].PtInRect(pt)) {
                return quest->GetSomeId();
            }
            i++;
        }
        return -1;
    }

    for (int32_t i = 0; i < this->vis_tav->rewards.GetSize(); i++) {
        int32_t row = i / 3;
        int32_t col = i % 3;
        if (this->field_0x180[row * 3 + col].PtInRect(pt)) {
            return i;
        }
    }
    return -1;
}


// Statics for VisTavSceneDruid::VMethod7 (665D44-665D6C, 631FB0 in the binary).
static bool tavscene_druid_statics_inited = false;  // 665D64 init bitmask
static uint32_t tavscene_druid_anim_delay = 0;      // 665D50 delay before druid animation; low bits select which one plays
static uint32_t tavscene_druid_frame_ts = 0;        // 665D58
static uint32_t tavscene_druid_anims_ts = 0;        // 665D5C
static uint32_t tavscene_druid_state_ts = 0;        // 665D44
static uint32_t tavscene_druid_ambient_ts = 0;      // 665D6C (written, never read)
static int32_t tavscene_druid_anim_state = -1;      // 631FB0 -1=idle, 1=anims_druid[1], 2=anims_druid[2], 3/4=field_0x3cc flash


// 4A0673
void VisTavSceneDruid::VMethod7()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisTavDruid* tav_druid = (VisTavDruid*)this->vis_tav;

    if (!tavscene_druid_statics_inited) {
        tavscene_druid_statics_inited = true;
        tavscene_druid_anim_delay = 3200 + rand() % 16;
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

    this->field_0x27c->VMethod2(x, y, 0, 0, 0);

    if (main_wnd->sessionMode == 2) {
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
            this->field_0x3cc->VMethod2(x + 0x68, y + 0x98, 0, 0, 0);
            break;
        }

        if (tavscene_druid_anim_state != -1 && now - tavscene_druid_state_ts > 100) {
            switch (tavscene_druid_anim_state) {
            case 1:
                if (!this->anims_druid[1].StepForward()) {
                    tavscene_druid_anim_state = -1;
                    tavscene_druid_anim_delay = 3200 + rand() % 16;
                    this->anims_druid[1].frame_idx = 0;
                }
                tavscene_druid_state_ts = now;
                break;
            case 2:
                if (!this->anims_druid[2].StepForward()) {
                    tavscene_druid_anim_state = -1;
                    tavscene_druid_anim_delay = 3200 + rand() % 16;
                    this->anims_druid[2].frame_idx = 0;
                }
                tavscene_druid_state_ts = now;
                break;
            case 3:
                tavscene_druid_anim_state++;
                tavscene_druid_anim_delay = 1000 + rand() % 16;
                tavscene_druid_state_ts = now;
                break;
            case 4:
                tavscene_druid_anim_state = -1;
                break;
            }
        }

        int32_t avail_count = this->vis_tav->avail_entries.GetSize();
        for (int32_t i = 0; i < avail_count; i++) {
            int32_t row = i / 6;
            int32_t col = i % 6;
            CPoint pt = this->field_0x60[row * 6 + col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            this->field_0x274->VMethod2(x + pt.x, y + pt.y, 0, 0, 0);
            CSprite256* sprite = this->field_0x224[i];
            sprite->VMethod2(x + pt.x, y + pt.y, this->field_0x24c[i], 0, 0);
            if (this->vis_tav->selection_index == i && now - tavscene_druid_frame_ts > 0x7D) {
                this->field_0x24c[i] = (this->field_0x24c[i] + 1) % sprite->GetFrameCount();
                tavscene_druid_frame_ts = now;
            }

            CUnit* unit = this->vis_tav->avail_entries[i];
            int32_t entry = this->vis_tav->FUN_0049e2e3(unit);
            if (this->vis_tav->entrie_id[entry] & 0x80000000) {
                CRect& r = this->field_0x60[row * 6 + col];
                g_font2->DrawTextWithShadow(x + r.left + r.Width() / 2, y + r.top + r.Height() / 2, TxtFile::AllLines[0x101], 10, clrsh_CoralRed, 1);
            }
        }

        for (int32_t i = 0; i < this->vis_tav->reserved_entries.GetSize(); i++) {
            int32_t row = (avail_count + i) / 6;
            int32_t col = (avail_count + i) % 6;
            CPoint pt = this->field_0x60[row * 6 + col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            if (this->vis_tav->selection_index == avail_count + i && now - tavscene_druid_frame_ts > 0x7D) {
                this->field_0x260[i] = (this->field_0x260[i] + 1) % this->field_0x238[i]->GetFrameCount();
                tavscene_druid_frame_ts = now;
            }
            this->field_0x278->VMethod2(x + pt.x, y + pt.y, 0, 0, 0);
            this->field_0x238[i]->VMethod2(x + pt.x, y + pt.y, this->field_0x260[i], 0, 0);
        }
    } else {
        this->FUN_0049bc23();
    }

    this->field_0x340->VMethod10(x + 0xA0, y, 0, 0, 0x10, 0xEE);
    this->field_0x344->VMethod10(x + 0xA0, y + 0xEE, 0, 0, 0x10, 0xF2);
    this->field_0x348->VMethod10(x + 0x1D0, y, 0, 0, 0x10, 0xEE);
    if (this->vis_tav->info_panel->info_mode) {
        g_bmp_humanbackl->VMethod10(x + 0x1D0, y + 0xEE, 0, 0, 0x10, 0xF2);
    } else {
        g_bmp_textbackl->VMethod10(x + 0x1D0, y + 0xEE, 0, 0, 0x10, 0xF2);
    }
    UnlockSurface2();
    CVisualObject::VMethod7();

    if (main_wnd->sessionMode == 2 && now - this->field_0x460 > this->field_0x464) {
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
        this->field_0x464 = GetRandS16(2000) + 2000;
        this->field_0x460 = timeGetTime();
    }
}


// 4A3FC0
VisTavSceneDruid::~VisTavSceneDruid() {}


// 4A05A0
VisTavSceneDruid::VisTavSceneDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav)
: VisTavScene(_id, l, t, r, b, tav)
{
    this->field_0x460 = timeGetTime();
    this->field_0x464 = GetRandS16(2000) + 2000;
    this->field_0x3cc = nullptr;
}


// 4A158F
void VisTavSceneDruid::VMethod29()
{
    delete this->field_0x274;
    this->field_0x274 = nullptr;
    delete this->field_0x278;
    this->field_0x278 = nullptr;
    delete this->field_0x27c;
    this->field_0x27c = nullptr;
    delete this->field_0x340;
    this->field_0x340 = nullptr;
    delete this->field_0x344;
    this->field_0x344 = nullptr;
    delete this->field_0x348;
    this->field_0x348 = nullptr;
    delete this->field_0x3cc;
    this->field_0x3cc = nullptr;

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != 2) {
        for (int32_t i = 0; i < 13; i++) {
            delete this->field_0x34c[i];
            this->field_0x34c[i] = nullptr;
        }
    }

    for (int32_t i = 0; i < this->field_0x210.GetSize(); i++) {
        if (this->field_0x210[i] != nullptr) {
            delete this->field_0x210[i];
            this->field_0x210[i] = nullptr;
        }
    }
    this->field_0x210.RemoveAll();
}


// 4A11E3
void VisTavSceneDruid::VMethod28()
{
    this->VMethod29();
    this->field_0x274 = new CBmp64("graphics\\Interface\\Inn\\manback.bmp");
    g_mousept.Update();
    this->field_0x278 = new CBmp64("graphics\\Interface\\Inn\\ManBackTalk.bmp");
    g_mousept.Update();
    this->field_0x27c = new CBmp64("graphics\\Interface\\Inn_druid\\TavernMain.bmp");
    g_mousept.Update();
    this->field_0x340 = new CBmp64("graphics\\interface\\inn\\LUOver.bmp");
    g_mousept.Update();
    this->field_0x344 = new CBmp64("graphics\\interface\\inn\\LDOver.bmp");
    g_mousept.Update();
    this->field_0x348 = new CBmp64("graphics\\interface\\inn\\RUOver.bmp");
    g_mousept.Update();
    this->field_0x3cc = new CBmp64("graphics\\interface\\inn_druid\\taverner\\a30001.bmp");
    g_mousept.Update();

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != 2) {
        CString name;
        for (int32_t i = 0; i < 13; i++) {
            name.Format("graphics\\interface\\inn\\quests\\%02d.bmp", i + 1);
            this->field_0x34c[i] = new CBmp64(name);
        }
    }
}


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


// 4A2BB8
void VisTavSceneKaarg::VMethod7()
{
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

    this->field_0x27c->VMethod2(x, y, 0, 0, 0);

    if (main_wnd->sessionMode == 2) {
        if (tavscene_kaarg_anim_state == -1) {
            this->field_0x4bc = 0;
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
            this->anims_kaarg[4].frames[kaarg_frame_script[this->field_0x4bc]]->VMethod2(x + 0x58, y + 0x54, 0, 0, 0);
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
                this->field_0x4bc++;
                if (kaarg_frame_script[this->field_0x4bc] == -1) {
                    tavscene_kaarg_anim_state = -1;
                    this->anims_kaarg[4].frame_idx = 0;
                }
                tavscene_kaarg_state_ts = now;
                break;
            }
        }

        int32_t avail_count = this->vis_tav->avail_entries.GetSize();
        for (int32_t i = 0; i < avail_count; i++) {
            int32_t row = i / 6;
            int32_t col = i % 6;
            CPoint pt = this->field_0x60[row * 6 + col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            this->field_0x274->VMethod2(x + pt.x, y + pt.y, 0, 0, 0);
            CSprite256* sprite = this->field_0x224[i];
            sprite->VMethod2(x + pt.x, y + pt.y, this->field_0x24c[i], 0, 0);
            if (this->vis_tav->selection_index == i && now - tavscene_kaarg_frame_ts > 0x7D) {
                this->field_0x24c[i] = (this->field_0x24c[i] + 1) % sprite->GetFrameCount();
                tavscene_kaarg_frame_ts = now;
            }

            CUnit* unit = this->vis_tav->avail_entries[i];
            int32_t entry = this->vis_tav->FUN_0049e2e3(unit);
            if (this->vis_tav->entrie_id[entry] & 0x80000000) {
                CRect& r = this->field_0x60[row * 6 + col];
                g_font2->DrawTextWithShadow(x + r.left + r.Width() / 2, y + r.top + r.Height() / 2, TxtFile::AllLines[0x101], 10, clrsh_CoralRed, 1);
            }
        }

        for (int32_t i = 0; i < this->vis_tav->reserved_entries.GetSize(); i++) {
            int32_t row = (avail_count + i) / 6;
            int32_t col = (avail_count + i) % 6;
            CPoint pt = this->field_0x60[row * 6 + col].TopLeft();
            if (this->vis_tav->selection_index == -1) {
                continue;
            }

            if (this->vis_tav->selection_index == avail_count + i && now - tavscene_kaarg_frame_ts > 0x7D) {
                this->field_0x260[i] = (this->field_0x260[i] + 1) % this->field_0x238[i]->GetFrameCount();
                tavscene_kaarg_frame_ts = now;
            }
            this->field_0x278->VMethod2(x + pt.x, y + pt.y, 0, 0, 0);
            this->field_0x238[i]->VMethod2(x + pt.x, y + pt.y, this->field_0x260[i], 0, 0);
        }
    } else {
        this->FUN_0049bc23();
    }

    this->field_0x340->VMethod10(x + 0xA0, y, 0, 0, 0x10, 0xEE);
    this->field_0x344->VMethod10(x + 0xA0, y + 0xEE, 0, 0, 0x10, 0xF2);
    this->field_0x348->VMethod10(x + 0x1D0, y, 0, 0, 0x10, 0xEE);
    if (this->vis_tav->info_panel->info_mode) {
        g_bmp_humanbackl->VMethod10(x + 0x1D0, y + 0xEE, 0, 0, 0x10, 0xF2);
    } else {
        g_bmp_textbackl->VMethod10(x + 0x1D0, y + 0xEE, 0, 0, 0x10, 0xF2);
    }
    UnlockSurface2();

    if (main_wnd->sessionMode == 2 && now - this->field_0x4c8 > this->field_0x4cc) {
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
        this->field_0x4cc = GetRandS16(2000) + 2000;
        this->field_0x4c8 = timeGetTime();
    }

    if (main_wnd->sessionMode == 2 && now - this->field_0x4c0 > this->field_0x4c4) {
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
        this->field_0x4c4 = GetRandS16(2000) + 2000;
        this->field_0x4c0 = timeGetTime();
    }

    CVisualObject::VMethod7();
}


// 4A2AA4
VisTavSceneKaarg::VisTavSceneKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav)
: VisTavScene(_id, l, t, r, b, tav)
{
    this->field_0x4c0 = timeGetTime();
    this->field_0x4c4 = GetRandS16(2000) + 2000;
    this->field_0x4c8 = timeGetTime();
    this->field_0x4cc = GetRandS16(2000) + 2000;
}


// 4A40C0
VisTavSceneKaarg::~VisTavSceneKaarg() {}


// 4A3B25
void VisTavSceneKaarg::VMethod29()
{
    delete this->field_0x274;
    this->field_0x274 = nullptr;
    delete this->field_0x278;
    this->field_0x278 = nullptr;
    delete this->field_0x27c;
    this->field_0x27c = nullptr;
    delete this->field_0x340;
    this->field_0x340 = nullptr;
    delete this->field_0x344;
    this->field_0x344 = nullptr;
    delete this->field_0x348;
    this->field_0x348 = nullptr;

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != 2) {
        for (int32_t i = 0; i < 13; i++) {
            delete this->field_0x34c[i];
            this->field_0x34c[i] = nullptr;
        }
    }

    for (int32_t i = 0; i < this->field_0x210.GetSize(); i++) {
        if (this->field_0x210[i] != nullptr) {
            delete this->field_0x210[i];
            this->field_0x210[i] = nullptr;
        }
    }
    this->field_0x210.RemoveAll();
}


// 4A37E7
void VisTavSceneKaarg::VMethod28()
{
    this->VMethod29();
    this->field_0x274 = new CBmp64("graphics\\Interface\\Inn\\manback.bmp");
    g_mousept.Update();
    this->field_0x278 = new CBmp64("graphics\\Interface\\Inn\\ManBackTalk.bmp");
    g_mousept.Update();
    this->field_0x27c = new CBmp64("graphics\\Interface\\inn_kaarg\\TavernMain.bmp");
    g_mousept.Update();
    this->field_0x340 = new CBmp64("graphics\\interface\\inn\\LUOver.bmp");
    g_mousept.Update();
    this->field_0x344 = new CBmp64("graphics\\interface\\inn\\LDOver.bmp");
    g_mousept.Update();
    this->field_0x348 = new CBmp64("graphics\\interface\\inn\\RUOver.bmp");
    g_mousept.Update();

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode != 2) {
        CString name;
        for (int32_t i = 0; i < 13; i++) {
            name.Format("graphics\\interface\\inn\\quests\\%02d.bmp", i + 1);
            this->field_0x34c[i] = new CBmp64(name);
        }
    }
}


// 49FAAA
void VisTavDruid::VMethod28()
{
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

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == 2) {
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

    if (main_wnd->sessionMode == 2) {
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
            this->avail_entries.Add(this->map_context->FUN_0041dfa6(unit_id));
        } else {
            this->reserved_entries.Add(this->map_context->FUN_0041dfa6(unit_id));
        }
    }

    if (this->avail_entries.GetSize() + this->reserved_entries.GetSize() != 0) {
        this->selection_index = 0;
    } else {
        this->selection_index = -1;
    }

    g_StructEnter.FUN_00473b80();
    this->selected_entries.Copy(g_StructEnter.field_0x0);
    this->select_party = g_StructEnter.FUN_00473d10();

    this->map_context->MsgProc(0x405, 0, 0);
    this->selected_entries[this->select_party]->VMethod1(1);
    this->map_context->UpdateSelectionState();
    this->selected_entries[this->select_party]->unitFlags |= 8;

    this->scene->VMethod26();

    VisTavSceneDruid* scene_druid = (VisTavSceneDruid*)this->scene;

    CStringArray names;
    CString name;

    for (int32_t i = 0; i < 10; i++) {
        name.Format("graphics\\interface\\inn_druid\\waterdrop\\d%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        scene_druid->anims_druid[0].FUN_004010ee(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 0x28; i++) {
        name.Format("graphics\\interface\\inn_druid\\taverner\\a1%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        scene_druid->anims_druid[1].FUN_004010ee(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 0x1E; i++) {
        name.Format("graphics\\interface\\inn_druid\\taverner\\a2%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        scene_druid->anims_druid[2].FUN_004010ee(&names);
    }
    names.RemoveAll();

    this->right_panel->FUN_00499a67();
    this->right_panel->FUN_0049a84e();
    this->scene->VMethod28();
    this->left_panel->FUN_004995d1();
    this->right_panel->FUN_0049a973();
    this->VMethod30();

    this->dialog_active = 1;

    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();

    VisScreen::VMethod28();
    CSound::Play(this->sounds[8]);
    FUN_004a4740(&this->snd_druid[7]);
    g_Cursors[0]->Use();
    g_mousept.EnableHint();

    this->quest_id = 0;
    this->quest_selection = -1;
    this->field_0x13c = 0;
}


// 4A0278
void VisTavDruid::DoClose(uint32_t code)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    this->VMethod9();
    if (main_wnd->sessionMode == 2) {
        ScenarioLeaveInn();
    }

    if (this->field_0x13c != 0) {
        main_wnd->vis_map_context->FUN_0041ae1c(0xAAAAAAAA);
    } else {
        main_wnd->vis_map_context->FUN_0041ae1c(this->quest_selection);
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

    this->right_panel->FUN_00499cdf();
    this->scene->VMethod29();
    this->scene->VMethod27();
    this->left_panel->FUN_004996ab();

    VisTavSceneDruid* scene_druid = (VisTavSceneDruid*)this->scene;
    scene_druid->anims_druid[0].FUN_004014f2();
    scene_druid->anims_druid[1].FUN_004014f2();
    scene_druid->anims_druid[2].FUN_004014f2();

    this->VMethod31();

    this->avail_entries.RemoveAll();
    this->selected_entries.RemoveAll();
    this->reserved_entries.RemoveAll();

    VisScreen::DoClose(code);
    this->quest_map->sub_55ECFE(0);

    while (this->rewards.GetSize() != 0) {
        TokenEntry* entry = this->rewards[0];
        if (entry != nullptr) {
            delete entry;
        }
        this->rewards.RemoveAt(0, 1);
    }
}


// 49F51B
void VisTavDruid::VMethod26()
{
    this->dialog_active = 0;
    for (int32_t i = 0; i < 13; i++) {
        this->sounds[i].sample = nullptr;
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
void VisTavDruid::VMethod30()
{
    this->VMethod31();
    FUN_00438e40(&this->sounds[5].sample, "SFX\\Add.wav");
    FUN_00438e40(&this->sounds[6].sample, "SFX\\NoAdd.wav");
    FUN_00438e40(&this->sounds[7].sample, "SFX\\Town\\Shop\\nofit.wav");
    FUN_00438e40(&this->sounds[8].sample, "SFX\\Town_druid\\Inn\\din1.wav");
    FUN_00438e40(&this->sounds[9].sample, "SFX\\Town\\Inn\\Helper.wav");
    FUN_00438e40(&this->sounds[11].sample, "SFX\\Out.wav");
    FUN_00438e40(&this->sounds[12].sample, "SFX\\Talk.wav");
    FUN_00438e40(&this->snd_druid[0], "SFX\\Town_druid\\Inn\\DWater1.wav");
    FUN_00438e40(&this->snd_druid[1], "SFX\\Town_druid\\Inn\\DWater2.wav");
    FUN_00438e40(&this->snd_druid[2], "SFX\\Town_druid\\Inn\\DWater3.wav");
    FUN_00438e40(&this->snd_druid[3], "SFX\\Town_druid\\Inn\\DWater4.wav");
    FUN_00438e40(&this->snd_druid[4], "SFX\\Town_druid\\Inn\\DDruid3.wav");
    FUN_00438e40(&this->snd_druid[5], "SFX\\Town_druid\\Inn\\Ddruid4.wav");
    FUN_00438e40(&this->snd_druid[6], "SFX\\Town_druid\\Inn\\Ddruid41.wav");
    FUN_00438e40(&this->snd_druid[7], "SFX\\Town_druid\\Inn\\Dforest2.wav");
    FUN_00438e40(&this->snd_druid[8], "SFX\\Town_druid\\Inn\\Dbird4.wav");
    FUN_00438e40(&this->snd_druid[9], "SFX\\Town_druid\\Inn\\Dbird41.wav");
    FUN_00438e40(&this->snd_druid[10], "SFX\\Town_druid\\Inn\\Dbird42.wav");
}


// 49F961
void VisTavDruid::VMethod31()
{
    FUN_00438dd0(&this->sounds[5].sample);
    FUN_00438dd0(&this->sounds[6].sample);
    FUN_00438dd0(&this->sounds[7].sample);
    FUN_00438dd0(&this->sounds[8].sample);
    FUN_00438dd0(&this->sounds[9].sample);
    FUN_00438dd0(&this->sounds[11].sample);
    FUN_00438dd0(&this->sounds[12].sample);
    for (int32_t i = 0; i < 11; i++) {
        FUN_00438dd0(&this->snd_druid[i]);
    }
}


// 4A3F70
VisTavDruid::~VisTavDruid()
{
}


// 49F4E6
VisTavDruid::VisTavDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisTav(_id, l, t, r, b)
{
}


// 4A1BA2
void VisTavKaarg::VMethod28()
{
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

    if (g_settings.TipsMode != 0 && main_wnd->sessionMode == 2) {
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

    if (main_wnd->sessionMode == 2) {
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
            this->avail_entries.Add(this->map_context->FUN_0041dfa6(unit_id));
        } else {
            this->reserved_entries.Add(this->map_context->FUN_0041dfa6(unit_id));
        }
    }

    if (this->avail_entries.GetSize() + this->reserved_entries.GetSize() != 0) {
        this->selection_index = 0;
    } else {
        this->selection_index = -1;
    }

    g_StructEnter.FUN_00473b80();
    this->selected_entries.Copy(g_StructEnter.field_0x0);
    this->select_party = g_StructEnter.FUN_00473d10();

    this->map_context->MsgProc(0x405, 0, 0);
    this->selected_entries[this->select_party]->VMethod1(1);
    this->map_context->UpdateSelectionState();
    this->selected_entries[this->select_party]->unitFlags |= 8;

    this->scene->VMethod26();

    VisTavSceneKaarg* scene_kaarg = (VisTavSceneKaarg*)this->scene;

    CStringArray names;
    CString name;

    for (int32_t i = 0; i < 0xF; i++) {
        name.Format("graphics\\interface\\inn_kaarg\\taverner\\a1%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        scene_kaarg->anims_kaarg[0].FUN_004010ee(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 0x19; i++) {
        name.Format("graphics\\interface\\inn_kaarg\\taverner\\a2%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        scene_kaarg->anims_kaarg[1].FUN_004010ee(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 3; i++) {
        name.Format("graphics\\interface\\inn_kaarg\\taverner\\a3%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        scene_kaarg->anims_kaarg[2].FUN_004010ee(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 7; i++) {
        name.Format("graphics\\interface\\inn_kaarg\\taverner\\a4%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        scene_kaarg->anims_kaarg[3].FUN_004010ee(&names);
    }
    names.RemoveAll();

    for (int32_t i = 0; i < 0x1D; i++) {
        name.Format("graphics\\interface\\inn_kaarg\\taverner\\a5%.4d.bmp", i + 1);
        names.Add(name);
    }
    if (main_wnd->sessionMode == 2) {
        scene_kaarg->anims_kaarg[4].FUN_004010ee(&names);
    }
    names.RemoveAll();

    this->right_panel->FUN_00499a67();
    this->right_panel->FUN_0049a84e();
    this->scene->VMethod28();
    this->left_panel->FUN_004995d1();
    this->right_panel->FUN_0049a973();
    this->VMethod30();

    this->dialog_active = 1;

    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();

    VisScreen::VMethod28();
    CSound::Play(this->sounds[8]);
    FUN_004a4740(&this->snd_kaarg[6]);
    g_Cursors[0]->Use();
    g_mousept.EnableHint();

    this->quest_id = 0;
    this->quest_selection = -1;
    this->field_0x13c = 0;
}


// 4A2448
void VisTavKaarg::DoClose(uint32_t code)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    this->VMethod9();
    if (main_wnd->sessionMode == 2) {
        ScenarioLeaveInn();
    }

    if (this->field_0x13c != 0) {
        main_wnd->vis_map_context->FUN_0041ae1c(0xAAAAAAAA);
    } else {
        main_wnd->vis_map_context->FUN_0041ae1c(this->quest_selection);
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

    this->right_panel->FUN_00499cdf();
    this->scene->VMethod29();
    this->scene->VMethod27();
    this->left_panel->FUN_004996ab();

    VisTavSceneKaarg* scene_kaarg = (VisTavSceneKaarg*)this->scene;
    scene_kaarg->anims_kaarg[0].FUN_004014f2();
    scene_kaarg->anims_kaarg[1].FUN_004014f2();
    scene_kaarg->anims_kaarg[2].FUN_004014f2();
    scene_kaarg->anims_kaarg[3].FUN_004014f2();
    scene_kaarg->anims_kaarg[4].FUN_004014f2();

    this->VMethod31();

    this->avail_entries.RemoveAll();
    this->selected_entries.RemoveAll();
    this->reserved_entries.RemoveAll();

    VisScreen::DoClose(code);
    this->quest_map->sub_55ECFE(0);

    while (this->rewards.GetSize() != 0) {
        TokenEntry* entry = this->rewards[0];
        if (entry != nullptr) {
            delete entry;
        }
        this->rewards.RemoveAt(0, 1);
    }
}


// 4A190D
void VisTavKaarg::VMethod26()
{
    this->dialog_active = 0;
    for (int32_t i = 0; i < 13; i++) {
        this->sounds[i].sample = nullptr;
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
void VisTavKaarg::VMethod30()
{
    this->VMethod31();
    FUN_00438e40(&this->sounds[5].sample, "SFX\\Add.wav");
    FUN_00438e40(&this->sounds[6].sample, "SFX\\NoAdd.wav");
    FUN_00438e40(&this->sounds[7].sample, "SFX\\Town\\Shop\\nofit.wav");
    FUN_00438e40(&this->sounds[8].sample, "SFX\\Town_kaarg\\Inn\\Kin1.wav");
    FUN_00438e40(&this->sounds[9].sample, "SFX\\Town\\Inn\\Helper.wav");
    FUN_00438e40(&this->sounds[11].sample, "SFX\\Out.wav");
    FUN_00438e40(&this->sounds[12].sample, "SFX\\Talk.wav");
    FUN_00438e40(&this->snd_kaarg[0], "SFX\\Town_kaarg\\Inn\\Kdish1.wav");
    FUN_00438e40(&this->snd_kaarg[1], "SFX\\Town_kaarg\\Inn\\Kdish2.wav");
    FUN_00438e40(&this->snd_kaarg[2], "SFX\\Town_kaarg\\Inn\\Kdish3.wav");
    FUN_00438e40(&this->snd_kaarg[3], "SFX\\Town_kaarg\\Inn\\Kdish4.wav");
    FUN_00438e40(&this->snd_kaarg[4], "SFX\\Town_kaarg\\Inn\\Kman2.wav");
    FUN_00438e40(&this->snd_kaarg[5], "SFX\\Town_kaarg\\Inn\\Kman3.wav");
    FUN_00438e40(&this->snd_kaarg[6], "SFX\\Town_kaarg\\Inn\\Kvox5.wav");
    FUN_00438e40(&this->snd_kaarg[7], "SFX\\Town_kaarg\\Inn\\Kvox6.wav");
    FUN_00438e40(&this->snd_kaarg[8], "SFX\\Town_kaarg\\Inn\\Kvox7.wav");
    FUN_00438e40(&this->snd_kaarg[9], "SFX\\Town_kaarg\\Inn\\Kvox8.wav");
}


// 4A2873
void VisTavKaarg::VMethod31()
{
    FUN_00438dd0(&this->sounds[5].sample);
    FUN_00438dd0(&this->sounds[6].sample);
    FUN_00438dd0(&this->sounds[7].sample);
    FUN_00438dd0(&this->sounds[8].sample);
    FUN_00438dd0(&this->sounds[9].sample);
    FUN_00438dd0(&this->sounds[11].sample);
    FUN_00438dd0(&this->sounds[12].sample);
    for (int32_t i = 0; i < 10; i++) {
        FUN_00438dd0(&this->snd_kaarg[i]);
    }
}


// 4A18D8
VisTavKaarg::VisTavKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisTav(_id, l, t, r, b)
{
}


// 4A4070
VisTavKaarg::~VisTavKaarg()
{
}


// 4A5FAB
void VisInvBase::sub_4A5FAB(int32_t x, int32_t y, int32_t idx)
{
    static const int32_t trail_alpha[7] = {0x3F, 0x7F, 0xBF, 0xFF, 0xBF, 0x7F, 0x3F};

    int32_t i = this->anim_frames.GetAt(idx) >> 1;
    for (int32_t k = 0; k < 7; k++) {
        if (i < k) {
            break;
        }
        int32_t j = (i - k) % 1024;
        sub_4588EC(x + this->random_offsets1[j], y + this->random_offsets2[j], 0xFF, 0, 0xFF, trail_alpha[k]);
    }
}


// 4A58DC
TokenEntry* VisInvBase::VMethod29(TokenEntry* o, int32_t num)
{
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (!o->sub_4A7900(entry)) {
            continue;
        }
        if (entry->sub_4A7880(num)) {
            o = new TokenEntry();
            *o = *entry;
            o->field_0x10 = num;
            return o;
        }
        if (entry->FUN_0041f0d0()) {
            o = new TokenEntry();
            *o = *entry;
            entry->field_0x10 = 0;
            return o;
        }
        o = entry;
        this->grid_source->RemoveAt(i, 1);
        o->field_0x10 = num;
        return o;
    }
    return nullptr;
}


// 4A554F
int32_t VisInvBase::VMethod26(TokenEntry* o, int32_t idx)
{
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (!o->sub_4A7900(entry)) {
            continue;
        }
        entry->sub_4A7850(o->field_0x10);
        delete o;
        return i;
    }
    if (idx >= 0 && idx < this->grid_source->GetSize()) {
        this->grid_source->InsertAt(idx, o, 1);
        return idx;
    }
    if (this->grid_source->GetSize() != 0 && this->grid_source->GetAt(this->grid_source->GetUpperBound())->FUN_0041f0d0()) {
        this->grid_source->InsertAt(this->grid_source->GetUpperBound(), o, 1);
        return this->grid_source->GetUpperBound() - 1;
    }
    this->grid_source->Add(o);
    return this->grid_source->GetUpperBound();
}


// 4A57A1
TokenEntry* VisInvBase::VMethod28(uint32_t id)
{
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (id != entry->field_0x4) {
            continue;
        }
        if (entry->sub_4A7880(1)) {
            TokenEntry* result = new TokenEntry();
            *result = *entry;
            result->field_0x10 = 1;
            return result;
        }
        this->grid_source->RemoveAt(i, 1);
        entry->field_0x10 = 1;
        return entry;
    }
    return nullptr;
}


// 4A56C3
int32_t VisInvBase::VMethod27(TokenEntry* o)
{
    if (this->grid_source == nullptr) {
        return -1;
    }
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (!entry->sub_4A7900(o)) {
            continue;
        }
        entry->sub_4A7850(o->field_0x10);
        delete o;
        return i;
    }
    this->grid_source->Add(o);
    return this->grid_source->GetUpperBound();
}


// 4A5DE0
int32_t VisInvBase::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    if (msg == 0x401) {
        this->FUN_004a5e12();
    }
    return CVisualObject::MsgProc(msg, wparam, lparam);
}


// 46FB90
int32_t VisInvBase::FUN_0046fb90()
{
    return *this->visible_startref;
}


// 4A79A0
int32_t VisInvBase::VMethod30(int32_t x, int32_t y)
{
    return -1;
}


// 4A7990
int32_t VisInvBase::VMethod31(const CPoint* pt)
{
    return -1;
}


// 4A4CF8
const char* VisInvBase::GetHint()
{
    return nullptr;
}


// 4A4938
VisInvBase::VisInvBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    this->visible_columns = 0;
    this->visible_rows = 0;
    this->visible_startref = nullptr;
    this->grid_source = nullptr;
    this->sub_4A4BCB();
}


// 4a4bcb
void VisInvBase::sub_4A4BCB()
{
    for (int32_t i = 0; i < 1024; i++) {
        this->random_offsets1[i] = rand() / 0x1FF + 8;
        this->random_offsets2[i] = rand() / 0x1FF + 8;
    }
}


// 4A4ACC
VisInvBase::~VisInvBase()
{
    for (int32_t i = 0; i < this->visible_columns * this->visible_rows; i++) {
        CSprite256* sprite = this->spr_cells.GetAt(i);
        if (sprite != nullptr) {
            delete sprite;
        }
    }
    this->spr_cells.RemoveAll();
}


// 4A5C39
int32_t VisInvBase::VMethod37(int32_t idx)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 == nullptr) {
        return -1;
    }
    if (main_wnd->field_0x408->FUN_0041f0d0()) {
        main_wnd->vis_map_context->my_main_unit->gold += main_wnd->field_0x408->field_0x10;
    }
    int32_t count = main_wnd->field_0x408->field_0x10;

    int32_t same_tab;
    int32_t tab = this->VMethod38();
    if (tab >= 5 && tab <= 8 && tab == main_wnd->field_0x410) {
        same_tab = 0;
    } else {
        same_tab = 1;
    }

    idx = this->VMethod26(main_wnd->field_0x408, idx);
    if (this->VMethod38() == 2) {
        this->VMethod33(main_wnd->vis_map_context->field_0x138);
    } else {
        this->VMethod32(this->grid_source);
    }
    if (same_tab != 0) {
        main_wnd->vis_map_context->sub_41A7C7(main_wnd->field_0x410, main_wnd->field_0x40c, this->VMethod38(), idx, count);
    }
    main_wnd->ResetItemCursor();
    main_wnd->vis_root->MsgProc(0x46E, this->id, 0);
    return idx;
}


// 4A5AAE
TokenEntry* VisInvBase::VMethod36(int32_t idx, int32_t num)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr || this->grid_source->GetSize() <= idx) {
        return nullptr;
    }
    CGameObject* selected = main_wnd->vis_map_context->field_0x138;
    if (selected->map_player != main_wnd->vis_map_context->my_main_unit) {
        return nullptr;
    }
    TokenEntry* entry = this->grid_source->GetAt(idx);
    if (entry == nullptr) {
        return nullptr;
    }
    if (entry->GetType() < 2 && (selected->last_action == 3 || selected->last_action == 7 || selected->last_action == 8)) {
        return nullptr;
    }
    main_wnd->field_0x408 = this->VMethod29(entry, num);
    main_wnd->field_0x40c = idx;
    main_wnd->field_0x410 = this->VMethod38();
    if (this->VMethod38() == 2) {
        this->VMethod33(selected);
    } else {
        this->VMethod32(this->grid_source);
    }
    main_wnd->vis_root->MsgProc(0x46E, this->id, 0);
    return main_wnd->field_0x408;
}


// 4A5E12
void VisInvBase::FUN_004a5e12()
{
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
        if (main_wnd->field_0x408 == entry && main_wnd->field_0x408 != nullptr && main_wnd->field_0x408->field_0x10 == 1) {
            continue;
        }
        if (entry == nullptr || !(entry->flg & 0x20)) {
            continue;
        }
        this->anim_frames.ElementAt(i)++;
        this->has_anim_visible_cells = 1;
    }
}


// 4A4C2F
void VisInvBase::VMethod35()
{
    int32_t total = this->visible_columns * this->visible_rows;
    this->spr_cells.SetSize(total, -1);
    this->cell_update_counter.SetSize(total, -1);
    this->anim_frames.SetSize(total, -1);
    for (int32_t i = 0; i < total; i++) {
        this->spr_cells.ElementAt(i) = nullptr;
        this->cell_update_counter.ElementAt(i) = 0;
    }
}


// 4A4D05
void VisInvBase::VMethod33(CGameObject* uni)
{
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
    this->VMethod34();
}


// 4A4DA9
void VisInvBase::VMethod32(CArray<TokenEntry*>* arr)
{
    if (arr == nullptr) {
        this->grid_source = nullptr;
    } else {
        this->grid_source = arr;
    }
    int32_t total = this->visible_columns * this->visible_rows;
    for (int32_t i = 0; i < total; i++) {
        this->cell_update_counter.ElementAt(i) = 0;
    }
    this->VMethod34();
}


// 4A4E28
void VisInvBase::VMethod34()
{
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


// 4A79B0
int32_t VisInvBase::VMethod38()
{
    return -1;
}


// 4A4EBC
void VisInvBase::FUN_004a4ebc()
{
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
        if (entry->FUN_0041f0d0() != 0 || entry->FUN_004a78c0() != 0) {
            this->cell_update_counter.ElementAt(i) = 1;
            continue;
        }
        CSprite256* sprite = this->spr_cells.GetAt(i);
        if (sprite != nullptr) {
            delete sprite;
        }
        CString name = entry->FUN_004394f3();
        sprite = new CA16("graphics\\inventory\\" + name + ".16a");
        this->spr_cells.ElementAt(i) = sprite;
        sprite->ResetPalette(0x10, 4, 0);
        this->cell_update_counter.ElementAt(i) = 1;
    }
}


// 4a51c9
void VisInvBase::sub_4A51C9()
{
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
    this->FUN_004a4ebc();
}


// 4a5350
void VisInvBase::sub_4A5350()
{
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
    this->FUN_004a4ebc();
}

// 41f940
int32_t VisInvBase::sub_41F940() {
    return this->has_anim_visible_cells;
} 


// 4A6449
void VisInvType1::VMethod7()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->FUN_004a4ebc();

    CRect screen_rect = this->ClientRectToScreen(this->rect);

    int32_t edge_width = ((g_ScreenSize.right - 0xf0) % 0x50) / 2;

    g_bmp_invframe->VMethod10(screen_rect.left + edge_width, screen_rect.top, 0, 0, 0x110, g_bmp_invframe->GetHeight(0));

    for (int32_t i = 0; i < (g_ScreenSize.right - 0x280) / 0x50; i++) {
        g_bmp_invframe->VMethod10(screen_rect.left + edge_width + 0x110 + i * 0x50, screen_rect.top, 0xc0, 0, 0x110, g_bmp_invframe->GetHeight(0));
    }

    if (edge_width != 0 && g_ScreenSize.bottom > 600) {
        g_bmp_inv1024l->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
        g_bmp_inv1024r->VMethod10(screen_rect.right - edge_width - 0x10, screen_rect.top, 0, 0, g_bmp_inv1024r->GetWidth(0), g_bmp_inv1024r->GetHeight(0));
    }

    int32_t frame_w = g_bmp_invframe->GetWidth(0);
    int32_t frame_h = g_bmp_invframe->GetHeight(0);
    g_bmp_invframe->VMethod10(screen_rect.right - edge_width - g_bmp_invframe->GetWidth(0) + 0x110, screen_rect.top, 0x110, 0, frame_w, frame_h);

    CRect left_arrow_rect(screen_rect.left, screen_rect.top, screen_rect.left + 0x20 + edge_width, screen_rect.bottom);
    CRect right_arrow_rect(screen_rect.left + edge_width + 0x20 + this->visible_columns * 0x50, screen_rect.top,
        screen_rect.left + 0x40 + edge_width * 2 + this->visible_columns * 0x50, screen_rect.bottom);

    CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());

    if (this->grid_source != nullptr) {
        if (*this->visible_startref != 0) {
            if (left_arrow_rect.PtInRect(mouse_pt)) {
                g_bmp_invarrow3->VMethod10(screen_rect.left + edge_width, screen_rect.top + 2, 0, 0, 0x20, 0x58);
            } else {
                g_bmp_invarrow1->VMethod10(screen_rect.left + edge_width, screen_rect.top + 2, 0, 0, 0x20, 0x58);
            }
        }
        if (this->grid_source->GetSize() > this->visible_columns + *this->visible_startref) {
            if (right_arrow_rect.PtInRect(mouse_pt)) {
                g_bmp_invarrow4->VMethod10(screen_rect.left + edge_width + 0x20 + this->visible_columns * 0x50, screen_rect.top + 2, 0, 0, 0x20, 0x58);
            } else {
                g_bmp_invarrow2->VMethod10(screen_rect.left + edge_width + 0x20 + this->visible_columns * 0x50, screen_rect.top + 2, 0, 0, 0x20, 0x58);
            }
        }
    }

    if (this->grid_source == nullptr) {
        g_font2->DrawTextWithShadow(edge_width + (screen_rect.left + screen_rect.right - 0x10) / 2, (screen_rect.top + screen_rect.bottom) / 2,
            TxtFile::AllLines[0x33], 10, clrsh_DullGold, 1);
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
        if (main_wnd->field_0x408 == entry && main_wnd->field_0x408 != nullptr && main_wnd->field_0x408->field_0x10 == 1) {
            continue;
        }

        entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry == nullptr) {
            continue;
        }

        int32_t cell_x = edge_width + screen_rect.left + 0x20 + i * 0x50;
        int32_t cell_y = screen_rect.top + 6;

        if (entry->FUN_0041f0d0() != 0) {
            g_bmp_backinv->VMethod2(cell_x, cell_y, 0, 0, 0);
            g_ca16_money->VMethod2(cell_x, cell_y, 0, 0, 0);

            CString text;
            text.Format("%d", main_wnd->vis_map_context->my_main_unit->gold);
            FUN_00476987(&text);
            g_font2->DrawTextWithShadow(cell_x + 3, screen_rect.bottom - 0xe, text, 0, clrsh_DullGold, 1);
            continue;
        }

        g_bmp_backinv->VMethod2(cell_x, cell_y, 0, 0, 0);
        CSprite256* sprite = this->spr_cells.GetAt(i);
        sprite->VMethod2(cell_x, cell_y, 0, 0, 0);

        entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry->field_0x10 > 1) {
            char buf[0x50];
            sprintf(buf, "%d", entry->field_0x10);
            g_font2->DrawTextWithShadow(cell_x + 3, screen_rect.bottom - 0xe, buf, 0, clrsh_DullGold, 1);
        }

        entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry->flg & 0x20) {
            this->sub_4A5FAB(cell_x, cell_y, i);
        }

        for (int32_t j = 0; j < 9; j++) {
            entry = this->grid_source->GetAt(i + *this->visible_startref);
            if (main_wnd->m_GameSession.shortcuts[j].FUN_0041e3af(entry) != 0) {
                char key_buf[0x50];
                sprintf(key_buf, "F%d", j + 4);
                g_font3->DrawTextWithShadow(cell_x + 3, screen_rect.top + 8, key_buf, 0, clrsh_ShockingBlack, 1);
                break;
            }
        }
    }
}


// 4A716F
int32_t VisInvType1::OnMouseMove(uint32_t wparam, CPoint pos)
{
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
    if (main_wnd->field_0x408 != nullptr) {
        return 0;
    }

    int32_t idx = this->VMethod30(pos.x, -pos.y);
    if (idx == -1) {
        return 0;
    }

    int32_t count;
    if (this->grid_source->GetAt(idx)->FUN_0041f0d0() != 0) {
        if (main_wnd->vis_map_context->my_main_unit->gold < 1) {
            count = 0;
        } else if (g_kbShiftState == 0) {
            count = 1000;
        } else {
            count = this->grid_source->GetAt(idx)->field_0x10;
        }
    } else {
        if (g_kbShiftState == 0) {
            count = 1;
        } else {
            count = this->grid_source->GetAt(idx)->field_0x10;
        }
    }

    if (this->grid_source->GetAt(idx)->sub_43A5E5() != 0) {
        return 0;
    }

    if (this->VMethod36(idx, count) == 0) {
        return 0;
    }

    if (main_wnd->field_0x408->FUN_0041f0d0() != 0) {
        main_wnd->vis_map_context->my_main_unit->gold -= main_wnd->field_0x408->field_0x10;
        main_wnd->sub_48CCA1(main_wnd->field_0x408, main_wnd->field_0x40c, "graphics\\interface\\money\\money.16a", main_wnd->field_0x410);
    } else {
        CString name = main_wnd->field_0x408->FUN_004394f3();
        CString path = "graphics\\inventory\\" + name + ".16a";
        main_wnd->sub_48CCA1(main_wnd->field_0x408, main_wnd->field_0x40c, path, main_wnd->field_0x410);
    }

    return 0;
}


// 4A7441
int32_t VisInvType1::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (msg == 0x417) {
        if (lparam != 0) {
            CRect screen_rect = this->ClientRectToScreen(this->rect);
            CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
            if (screen_rect.PtInRect(mouse_pt)) {
                int32_t idx = this->VMethod30(g_mousept.GetX(), -g_mousept.GetY());
                if (idx >= 0) {
                    main_wnd->m_GameSession.shortcuts[wparam].sub_41E343(this->grid_source->GetAt(idx));
                    for (int32_t i = 0; i < 9; i++) {
                        if (i != wparam && main_wnd->m_GameSession.shortcuts[i].FUN_0041e3af(this->grid_source->GetAt(idx)) != 0) {
                            main_wnd->m_GameSession.shortcuts[i].kind = 0;
                        }
                    }
                    main_wnd->m_GameSession.FUN_004948b2();
                }
            }
        } else {
            if (this->grid_source != nullptr) {
                for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
                    if (main_wnd->m_GameSession.shortcuts[wparam].FUN_0041e3af(this->grid_source->GetAt(i)) != 0) {
                        TokenEntry* entry = this->VMethod36(i, 1);
                        if (entry != nullptr) {
                            main_wnd->vis_charinfo->VMethod27(entry->GetType() - 1);
                        }
                        break;
                    }
                }
            }
        }
    }

    return VisInvBase::MsgProc(msg, wparam, lparam);
}


// 4A765D
const char* VisInvType1::GetHint()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        return nullptr;
    }
    if (main_wnd->dialogsMask != 1) {
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
    if (entry->FUN_0041f0d0() != 0) {
        return TxtFile::AllLines[0x4a];
    }
    return entry->FUN_00439973();
}


// 4A6D11
int32_t VisInvType1::VMethod30(int32_t x, int32_t y)
{
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


// 4A6E57
int32_t VisInvType1::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    CRect screen_rect = this->ClientRectToScreen(this->rect);

    int32_t edge_width = ((g_ScreenSize.right - 0xf0) % 0x50) / 2;

    CRect left_rect(screen_rect.left, screen_rect.top, screen_rect.left + 0x20 + edge_width, screen_rect.bottom);
    CRect right_rect(screen_rect.right - 0x30 - edge_width, screen_rect.top, screen_rect.right - 0x10, screen_rect.bottom);

    if (left_rect.PtInRect(pos)) {
        this->sub_4A5350();
    }
    if (right_rect.PtInRect(pos)) {
        this->sub_4A51C9();
    }

    return 1;
}


// 4A70EC
int32_t VisInvType1::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (this->grid_source != nullptr) {
        int32_t idx = this->VMethod30(pos.x, pos.y);
        if (main_wnd->field_0x408 != nullptr) {
            ApplyCursor(g_Cursors[0]);
            this->VMethod37(idx);
        }
    }
    return 1;
}


// 4A741D
int32_t VisInvType1::OnWmUser(uint32_t wparam, CPoint pos)
{
    return this->OnLButtonDown(wparam, pos);
}


// 4A6F32
int32_t VisInvType1::OnRButtonDown(uint32_t wparam, CPoint pos)
{
    return 1;
}


// 4A715D
int32_t VisInvType1::OnRButtonUp(uint32_t wparam, CPoint pos)
{
    return 1;
}


// 4A70DA
int32_t VisInvType1::OnRButtonDblClk(uint32_t wparam, CPoint pos)
{
    return 1;
}


// 4A79C0
int32_t VisInvType1::VMethod27(TokenEntry* o)
{
    o->field_0x18 = 2;
    return VisInvBase::VMethod27(o);
}


// 4A79F0
int32_t VisInvType1::VMethod26(TokenEntry* o, int32_t idx)
{
    o->field_0x18 = 2;
    return VisInvBase::VMethod26(o, idx);
}


// 4A7800
VisInvType1::~VisInvType1()
{
}


// 4A79B0
int32_t VisInvType1::VMethod38()
{
    return 2;
}


// 4A630B
VisInvType1::VisInvType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : VisInvBase(_id, l, t, r, b)
{
    this->visible_columns = this->rect.Width() / 0x50 - 1;
    this->visible_rows = this->rect.Height() / 0x50;
    this->VMethod35();
}


// 4A6F44
int32_t VisInvType1::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    if (this->grid_source == nullptr) {
        return 1;
    }

    int32_t idx = this->VMethod30(pos.x, -pos.y);
    if (idx == -1) {
        return 1;
    }

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (this->grid_source->GetAt(idx)->FUN_0041f0d0() != 0) {
        if (main_wnd->vis_map_context->my_main_unit->gold > 0) {
            main_wnd->dialogsMask |= 8;
            main_wnd->vis_root->AddChild(main_wnd->vis_dropgold);
            main_wnd->vis_dropgold->FUN_004a7a30(idx);
            main_wnd->vis_dropgold->VMethod28();
            main_wnd->vis_root->VMethod9();
            ApplyCursor(g_Cursors[0]);
            if (g_mousept.GetSelectState() != 0) {
                g_mousept.ResetStates();
            }
            main_wnd->field_0x460 = 0;
        }
    } else {
        if (this->grid_source->GetAt(idx)->sub_43A6D5() == 0) {
            TokenEntry* entry = this->VMethod36(idx, 1);
            if (entry != nullptr) {
                main_wnd->vis_charinfo->VMethod27(entry->GetType() - 1);
            }
        }
    }

    return 1;
}


// 4B5072
void VisInvExtBase::VMethod7()
{
    VisShop* shop = this->field_0x20ac;
    int32_t off_x = shop->rect.left;
    int32_t off_y = shop->rect.top;

    if (shop->dialog_active == 0) {
        return;
    }

    this->FUN_004a4ebc();

    if (this->grid_source == nullptr) {
        return;
    }

    int32_t total_cells = this->visible_columns * this->visible_rows;
    int32_t draw_count = this->grid_source->GetSize();
    if (draw_count >= total_cells) {
        draw_count = total_cells;
    }

    LockSurface2();
    this->FUN_004a5e12();

    for (int32_t i = 0; i < total_cells; i++) {
        CPoint pos(this->field_0x20c4[i].left + off_x, this->field_0x20c4[i].top + off_y);
        if (i >= draw_count) {
            continue;
        }
        if (this->cell_update_counter.GetAt(i) == 0) {
            continue;
        }
        TokenEntry* entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry->FUN_0041f0d0() != 0) {
            if (shop->gameplay != nullptr) {
                entry->field_0x10 = shop->gameplay->my_main_unit->gold;
            }
            g_bmp_backinv->VMethod2(pos.x, pos.y + 1, 0, 0, 0);
            g_ca16_money->VMethod2(pos.x, pos.y + 1, 0, 0, 0);
        } else if (entry->FUN_004a78c0() != 0) {
            shop->bmp_backinvg->VMethod2(pos.x, pos.y + 1, 0, 0, 0);
        } else {
            CUnit* unit = shop->selected_units[shop->select_index];
            int32_t compatible = unit->FUN_0046c0c9(entry);
            if (entry->field_0x18 >= 5 && entry->field_0x18 <= 8) {
                int32_t price = entry->GetAttribute(1);
                if (shop->current_gold < price || compatible == 0) {
                    shop->bmp_backinvg->VMethod2(pos.x, pos.y + 1, 0, 0, 0);
                } else {
                    shop->bmp_backinvs->VMethod2(pos.x, pos.y + 1, 0, 0, 0);
                }
            } else {
                if (compatible != 0) {
                    g_bmp_backinv->VMethod2(pos.x, pos.y + 1, 0, 0, 0);
                } else {
                    shop->bmp_backinvg->VMethod2(pos.x, pos.y + 1, 0, 0, 0);
                }
            }
            CSprite256* sprite = this->spr_cells.GetAt(i);
            sprite->VMethod2(pos.x, pos.y + 1, 0, 0, 0);
            if (entry->flg & 0x20) {
                this->sub_4A5FAB(pos.x, pos.y + 1, i);
            }
        }

        entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry->field_0x10 > 1) {
            CPoint bottom_right(this->field_0x20c4[i].right + off_x, this->field_0x20c4[i].bottom + off_y);
            CString text;
            text.Format("%d", entry->field_0x10);
            FUN_00476987(&text);
            g_font2->DrawTextWithShadow(pos.x + 10, bottom_right.y - 15, text, 0, clrsh_DullGold, 1);
        }

        entry = this->grid_source->GetAt(i + *this->visible_startref);
        if (entry->FUN_0041f0d0() == 0 && entry->FUN_004a78c0() == 0) {
            CPoint cost_pos(this->field_0x20c4[i].right + off_x, this->field_0x20c4[i].top + off_y + 1);
            CArray<CBmp64*>* cost_arr;
            if (entry->field_0x18 == 2) {
                cost_arr = &shop->bmp_cost_medium;
            } else {
                cost_arr = &shop->bmp_cost_small;
            }
            int32_t digits = (int32_t)log10((double)entry->GetAttribute(1));
            if (digits > cost_arr->GetUpperBound()) {
                digits = cost_arr->GetUpperBound();
            }
            CBmp64* cost_bmp = cost_arr->GetAt(digits);
            cost_bmp->VMethod3(cost_pos.x - 3, cost_pos.y + 2, 0, 8, 0);
            int32_t bmp_w = cost_bmp->GetWidth(0);
            int32_t bmp_h = cost_bmp->GetHeight(0);
            cost_bmp->VMethod10(cost_pos.x, cost_pos.y, 0, 0, bmp_w, bmp_h);
            int32_t amount;
            if (entry->field_0x18 == 2) {
                amount = (entry->GetAttribute(1) + 1) / 2;
            } else {
                amount = entry->GetAttribute(1);
            }
            CString cost_text;
            cost_text.Format("%d", amount);
            FUN_00476987(&cost_text);
            g_font2->DrawTextWithShadow(cost_pos.x - 6 + cost_bmp->GetWidth(0), cost_pos.y, cost_text, 1, clrsh_DullGold, 1);
        }
    }

    shop->sub_4BAD1D();
    UnlockSurface2();
    shop->dirty |= 0x40;
}


// 4B5F3F
int32_t VisInvExtBase::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    VisShop* shop = this->field_0x20ac;
    int32_t idx = this->VMethod31(&pos);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (idx >= 0 && this->grid_source->GetSize() > idx) {
        TokenEntry* entry = this->grid_source->GetAt(idx);
        if (entry->sub_43A6D5() == 0) {
            this->VMethod36(idx, 1);
            if (main_wnd->field_0x408 != nullptr) {
                if (main_wnd->field_0x410 == 4) {
                    switch (main_wnd->field_0x408->field_0x18) {
                    case 1:
                        ((VisCharInfo*)shop->select_info_panel)->VMethod27(main_wnd->field_0x40c);
                        shop->dirty |= 8;
                        break;
                    case 2:
                        shop->to_sell->VMethod37(*shop->to_sell->visible_startref);
                        shop->to_sell->sub_4B4D33();
                        break;
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                        shop->assortiment->grid_source = &shop->assortiment->field_0x2100[main_wnd->field_0x408->field_0x18 - 5];
                        shop->assortiment->VMethod37(-1);
                        shop->assortiment->grid_source = &shop->assortiment->field_0x2100[shop->select_category];
                        shop->assortiment->sub_4B4D33();
                        break;
                    }
                    this->sub_4B4D33();
                    shop->sub_4BCDA0();
                    shop->dirty |= 0x20;
                } else if (main_wnd->field_0x410 == 2) {
                    ((VisCharInfo*)shop->select_info_panel)->VMethod27(main_wnd->field_0x408->GetType() - 1);
                } else {
                    if (shop->to_buy->sub_4B91F9(main_wnd->field_0x408) == 0) {
                        shop->to_buy->VMethod37(*shop->to_buy->visible_startref);
                        this->sub_4B4D33();
                        this->VMethod9();
                        shop->to_buy->sub_4B4D33();
                        shop->sub_4BCDA0();
                        shop->dirty |= 0x20;
                    }
                }
            }
        }
    }
    return 1;
}


// 4B5D5C
int32_t VisInvExtBase::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    VisShop* shop = this->field_0x20ac;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 == nullptr) {
        return 1;
    }
    ApplyCursor(g_Cursors[0]);
    if (main_wnd->field_0x408->GetAttribute(1) == 0 && this->VMethod38() != 2) {
        shop->placement_lock = 0;
    }
    if (shop->placement_lock == 0) {
        shop->sub_4BC97B();
        CSound::Play((CSound&)shop->snd_notif);
        return 1;
    }
    int32_t region = this->VMethod38();
    if (region == 4) {
        this->VMethod37(this->VMethod31(&pos));
        shop->sub_4BCDA0();
        shop->dirty |= 0x20;
    } else if (region < 4 || region > 8) {
        this->VMethod37(this->VMethod31(&pos));
    } else {
        if (this->VMethod38() == main_wnd->field_0x408->field_0x18) {
            this->VMethod37(this->VMethod31(&pos));
        } else {
            shop->sub_4BC97B();
        }
    }
    this->sub_4B5B81();
    this->sub_4B4D33();
    this->VMethod9();
    return 1;
}


// 4B5BBE
int32_t VisInvExtBase::OnMouseMove(uint32_t wparam, CPoint pos)
{
    VisShop* shop = this->field_0x20ac;
    if (shop->hovered_region != this->VMethod38()) {
        shop->hovered_region = this->VMethod38();
        shop->buttons->ResetSelected();
    }
    if (wparam & 1) {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        if (main_wnd->field_0x40c >= 0 && main_wnd->field_0x40c < this->grid_source->GetSize() && main_wnd->field_0x408 == nullptr) {
            int32_t count;
            if (g_kbShiftState != 0) {
                count = this->grid_source->GetAt(main_wnd->field_0x40c)->field_0x10;
            } else {
                count = 1;
            }
            if (this->grid_source->GetAt(main_wnd->field_0x40c)->sub_43A5E5() == 0) {
                if (this->VMethod36(main_wnd->field_0x40c, count) != 0) {
                    this->sub_4B5A6C(main_wnd->field_0x40c);
                    this->sub_4B4FD1();
                    this->sub_4B4D33();
                    this->VMethod9();
                }
            }
        }
    }
    return 0;
}


// 4B4E88
int32_t VisInvExtBase::VMethod31(const CPoint* pt)
{
    VisShop* shop = this->field_0x20ac;
    CPoint rel = *pt - CPoint(shop->rect.left, shop->rect.top);
    for (int32_t i = 0; i < this->visible_columns * this->visible_rows; i++) {
        if (this->field_0x20c4[i].PtInRect(rel)) {
            int32_t idx = i + *this->visible_startref;
            if (idx <= this->grid_source->GetUpperBound() && this->grid_source->GetAt(idx)->item_id != 0xffff) {
                return idx;
            }
        }
    }
    return -1;
}


// 4B4C64
const char* VisInvExtBase::GetHint()
{
    VisShop* shop = this->field_0x20ac;
    if (shop->dialog_active == 0) {
        return nullptr;
    }
    int32_t idx = this->VMethod30(g_mousept.GetX(), g_mousept.GetY());
    if (this->grid_source->GetAt(idx) == nullptr) {
        return nullptr;
    }
    if (this->grid_source->GetAt(idx)->FUN_0041f0d0() != 0) {
        return TxtFile::AllLines.GetAt(0x4A);
    }
    if (this->grid_source->GetAt(idx)->FUN_004a78c0() != 0) {
        return nullptr;
    }
    return this->grid_source->GetAt(idx)->FUN_00439973();
}


// 4B4FD1
void VisInvExtBase::sub_4B4FD1()
{
    int32_t size = this->grid_source->GetSize();
    if (size - *this->visible_startref < this->visible_columns * this->visible_rows) {
        int32_t start = size - this->visible_columns * this->visible_rows;
        if (start <= 0) {
            start = 0;
        }
        *this->visible_startref = start;
    }
}


// 4B4D33
void VisInvExtBase::sub_4B4D33()
{
    this->sub_4B4FD1();
    for (int32_t i = 0; i < this->visible_columns * this->visible_rows; i++) {
        this->cell_update_counter.ElementAt(i) = 0;
    }
    this->FUN_004a4ebc();
}


// 4B4BC5
void VisInvExtBase::sub_4B4BC5()
{
    this->sub_4B4C1C();
    FUN_00438e40(&this->field_0x20b4.sample, "SFX\\Put_On.wav");
    FUN_00438e40(&this->field_0x20b8.sample, "SFX\\Put_Off.wav");
    FUN_00438e40(&this->field_0x20bc.sample, "SFX\\Scroll.wav");
}


// 4B5D19
int32_t VisInvExtBase::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 == nullptr) {
        main_wnd->field_0x40c = this->VMethod31(&pos);
    }
    return 1;
}


// 4B4BA9
void VisInvExtBase::sub_4B4BA9()
{
    FUN_00438dd0(&this->field_0x20b0.sample);
}


// 4B4C1C
void VisInvExtBase::sub_4B4C1C()
{
    this->sub_4B4BA9();
    FUN_00438dd0(&this->field_0x20b4.sample);
    FUN_00438dd0(&this->field_0x20b8.sample);
    FUN_00438dd0(&this->field_0x20bc.sample);
}


// 4B4FA3
int32_t VisInvExtBase::VMethod30(int32_t x, int32_t y)
{
    CPoint pt(x, y);
    return this->VMethod31(&pt);
}


// 4B4D91
void VisInvExtBase::VMethod32(CArray<TokenEntry*>* arr)
{
    this->grid_source = arr;
}


// 4B9E40
int32_t VisInvExtBase::VMethod38()
{
    return -1;
}


// 4B9E30
void VisInvExtBase::VMethod39()
{
}


// 4B4DAA
void VisInvExtBase::VMethod40(CArray<TokenEntry*>* arr)
{
    if (this->VMethod38() != 4) {
        for (int32_t i = 0; i < arr->GetSize(); i++) {
            arr->GetAt(i)->field_0x18 = this->VMethod38();
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


// 4B9E50
void VisInvExtBase::VMethod41()
{
}


// 4B9E60
void VisInvExtBase::VMethod42()
{
}


// 4B62A2
int32_t VisInvExtBase::OnRButtonDown(uint32_t wparam, CPoint pos)
{
    return 1;
}


// 4B62B4
int32_t VisInvExtBase::OnRButtonUp(uint32_t wparam, CPoint pos)
{
    return 1;
}


// 4B62C6
int32_t VisInvExtBase::OnRButtonDblClk(uint32_t wparam, CPoint pos)
{
    return 1;
}


// 4B4A0C
VisInvExtBase::VisInvExtBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
: VisInvBase(_id, l, t, r, b)
{
    this->grid_source = nullptr;
    this->field_0x20ac = shop;
    this->flags |= 2;
    this->visible_startref = new int32_t(0);
    this->field_0x20b0.sample = nullptr;
    this->field_0x20b4.sample = nullptr;
    this->field_0x20b8.sample = nullptr;
    this->field_0x20bc.sample = nullptr;
    // field_0x20c4 is deliberately left uninitialized here, as in the original
}


// 4B4AEC
VisInvExtBase::~VisInvExtBase()
{
    if (this->field_0x20c4 != nullptr) {
        delete this->field_0x20c4;
    }
    this->field_0x20ac = nullptr;
    this->grid_source = nullptr;
    if (this->visible_startref != nullptr) {
        delete this->visible_startref;
    }
    this->sub_4B4BA9();
    this->sub_4B4C1C();
}


// 4B5A6C
int32_t VisInvExtBase::sub_4B5A6C(int32_t idx)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    TokenEntry* entry = main_wnd->field_0x408;
    int32_t region = this->VMethod38();
    CString name = entry->FUN_004394f3();
    CString cs1 = "graphics\\inventory\\" + name;
    CString cs2 = cs1 + ".16a";
    main_wnd->sub_48CCA1(entry, idx, cs2, region);
    ApplyCursor(main_wnd->item_cursor);
    this->field_0x20ac->placement_lock = 1;
    return 1;
}


// 4B5B81
int32_t VisInvExtBase::sub_4B5B81()
{
    ApplyCursor(g_Cursors[0]);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->ResetItemCursor();
    this->field_0x20ac->placement_lock = 0;
    return 1;
}


// 4B7A9F
TokenEntry* VisInvExtType1::VMethod29(TokenEntry* o, int32_t num)
{
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (!o->sub_4A7900(entry)) {
            continue;
        }
        if (entry->FUN_004a78c0()) {
            return nullptr;
        }
        if (entry->sub_4A7880(num)) {
            o = new TokenEntry();
            *o = *entry;
            o->field_0x10 = num;
        } else if (entry->FUN_0041f0d0()) {
            o = new TokenEntry();
            *o = *entry;
            entry->field_0x10 = 0;
        } else {
            o = new TokenEntry();
            *o = *entry;
            o->field_0x10 = num;
            entry->field_0x10 = 0;
            entry->item_id = 0;
        }
        o->field_0x1c = i;
        o->field_0x18 = this->VMethod38();
        return o;
    }
    return nullptr;
}


// 4B70C0
void VisInvExtType1::VMethod7()
{
    VisShop* shop = this->field_0x20ac;
    CPoint topleft = shop->rect.TopLeft();

    if (shop->dialog_active == 0 || this->shop_inv == nullptr) {
        return;
    }

    LockSurface2();
    this->shop_inv->VMethod2(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, 0);
    UnlockSurface2();
    VisInvExtBase::VMethod7();

    if (*this->visible_startref > 0) {
        CPoint pos(g_mousept.GetX(), g_mousept.GetY());
        CRect rc = this->field_0x20cc;
        rc.OffsetRect(topleft.x, topleft.y);
        if (rc.PtInRect(pos)) {
            this->sub_4B7264();
        } else {
            this->sub_4B72C1();
        }
    }
    if (this->grid_source->GetSize() - *this->visible_startref > this->visible_columns * this->visible_rows) {
        CPoint pos(g_mousept.GetX(), g_mousept.GetY());
        CRect rc = this->field_0x20dc;
        rc.OffsetRect(topleft.x, topleft.y);
        if (rc.PtInRect(pos)) {
            this->sub_4B731E();
        } else {
            this->sub_4B7381();
        }
    }
}


// 4B790D
int32_t VisInvExtType1::VMethod26(TokenEntry* o, int32_t idx)
{
    o->field_0x18 = o->field_0x14 + 5;
    if (o->field_0x18 >= 5 && o->field_0x18 <= 8 && o->field_0x1c >= 0 && o->field_0x1c < this->grid_source->GetSize()) {
        TokenEntry* entry = this->grid_source->GetAt(o->field_0x1c);
        if (entry->FUN_004a78c0()) {
            *entry = *o;
        } else {
            entry->sub_4A7850(o->field_0x10);
        }
        return o->field_0x1c;
    }
    if (idx >= 0 && idx < this->grid_source->GetSize()) {
        this->grid_source->InsertAt(idx, o, 1);
        return idx;
    }
    if (this->grid_source->GetSize() != 0 && this->grid_source->GetAt(this->grid_source->GetUpperBound())->FUN_0041f0d0()) {
        this->grid_source->InsertAt(this->grid_source->GetUpperBound(), o, 1);
        return this->grid_source->GetUpperBound() - 1;
    }
    this->grid_source->Add(o);
    return this->grid_source->GetUpperBound();
}


// 4B6F8D
const char* VisInvExtType1::GetHint()
{
    VisShop* shop = this->field_0x20ac;
    if (shop->dialog_active == 0) {
        return nullptr;
    }
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        return nullptr;
    }
    CPoint mouse(g_mousept.GetX(), g_mousept.GetY());
    CPoint topleft = shop->rect.TopLeft();
    CPoint rel = mouse - topleft;
    int32_t region = this->VMethod31(&rel);
    if (region >= 0) {
        return VisInvExtBase::GetHint();
    }
    if (this->field_0x20cc.PtInRect(rel)) {
        return TxtFile::AllLines[0x38];
    }
    if (this->field_0x20dc.PtInRect(rel)) {
        return TxtFile::AllLines[0x39];
    }
    return TxtFile::AllLines[0x3C];
}


// 4B7562
int32_t VisInvExtType1::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    VisShop* shop = this->field_0x20ac;
    CPoint topleft = shop->rect.TopLeft();
    CPoint pt = pos - topleft;
    if (this->field_0x20cc.PtInRect(pt)) {
        this->sub_4A5350();
        this->sub_4A5350();
        FUN_00438f20(&this->field_0x20bc.sample);
        CSound::Play(this->field_0x20bc);
        return 1;
    }
    if (this->field_0x20dc.PtInRect(pt)) {
        this->sub_4A51C9();
        this->sub_4A51C9();
        FUN_00438f20(&this->field_0x20bc.sample);
        CSound::Play(this->field_0x20bc);
        return 1;
    }
    return VisInvExtBase::OnLButtonDown(wparam, pos);
}


// 4B7433
int32_t VisInvExtType1::OnWmUser(uint32_t wparam, CPoint pos)
{
    VisShop* shop = this->field_0x20ac;
    CPoint topleft = shop->rect.TopLeft();
    CPoint pt = pos - topleft;
    if (wparam & 1) {
        if (this->field_0x20cc.PtInRect(pt)) {
            this->sub_4A5350();
            this->sub_4A5350();
            FUN_00438f20(&this->field_0x20bc.sample);
            CSound::Play(this->field_0x20bc);
            return 1;
        }
        if (this->field_0x20dc.PtInRect(pt)) {
            this->sub_4A51C9();
            this->sub_4A51C9();
            FUN_00438f20(&this->field_0x20bc.sample);
            CSound::Play(this->field_0x20bc);
            return 1;
        }
    }
    return 1;
}


// 4B7859
void VisInvExtType1::sub_4B7859()
{
    for (int32_t i = 0; i < 4; i++) {
        CArray<TokenEntry*>& arr = this->field_0x2100[i];
        for (int32_t j = 0; j < arr.GetSize(); j++) {
            TokenEntry* entry = arr[j];
            if (entry != nullptr) {
                delete entry;
            }
        }
        arr.RemoveAll();
    }
}


// 4B777C
int32_t VisInvExtType1::OnKeyDown(uint32_t wparam)
{
    if (wparam == 0x21) {
        this->sub_4A5350();
        this->sub_4A5350();
        this->sub_4A5350();
        this->sub_4A5350();
        this->sub_4A5350();
        this->sub_4A5350();
        FUN_00438f20(&this->field_0x20bc.sample);
        CSound::Play(this->field_0x20bc);
        return 1;
    }
    if (wparam == 0x22) {
        this->sub_4A51C9();
        this->sub_4A51C9();
        this->sub_4A51C9();
        this->sub_4A51C9();
        this->sub_4A51C9();
        this->sub_4A51C9();
        FUN_00438f20(&this->field_0x20bc.sample);
        CSound::Play(this->field_0x20bc);
        return 1;
    }
    return 0;
}


// 4B7690
int32_t VisInvExtType1::OnMouseMove(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        switch (main_wnd->field_0x408->field_0x18) {
        case 5:
        case 6:
        case 7:
        case 8:
            this->field_0x20ac->placement_lock = 1;
            ApplyCursor(main_wnd->item_cursor);
            break;
        default:
            ApplyCursor(g_Cursors[23]);
            this->field_0x20ac->placement_lock = 0;
            break;
        }
    }
    return VisInvExtBase::OnMouseMove(wparam, pos);
}


// 4B9CB0
VisInvExtType1::~VisInvExtType1()
{
    for (int32_t i = 0; i < 4; i++) {
        CArray<TokenEntry*>& arr = this->field_0x2100[i];
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


// 4B7412
int32_t VisInvExtType1::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    return VisInvExtBase::OnLButtonUp(wparam, pos);
}


// 4B73E4
void VisInvExtType1::sub_4B73E4(int32_t category)
{
    this->grid_source = &this->field_0x2100[category];
    this->sub_4B4D33();
}


// 4B63F7
VisInvExtType1::VisInvExtType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisInvExtBase(_id, l, t, r, b, shop)
{
    this->visible_columns = this->rect.Width() / 0x50;
    this->visible_rows = this->rect.Height() / 0x50;
    this->VMethod39();
    this->VisInvBase::VMethod35();
    this->arrow1 = nullptr;
    this->arrow3 = nullptr;
    this->arrow2 = nullptr;
    this->arrow4 = nullptr;
    this->shop_inv = nullptr;
}


// 4C6C40
VisInvExtType1Druid::VisInvExtType1Druid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisInvExtType1(_id, l, t, r, b, shop)
{
}


// 4C6CD0
VisInvExtType1Kaarg::VisInvExtType1Kaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisInvExtType1(_id, l, t, r, b, shop)
{
}


// 4B6D46
void VisInvExtType1::VMethod39()
{
    int32_t total_cells = this->visible_columns * this->visible_rows;
    this->field_0x20c4 = new CRect[total_cells];

    CPoint topleft = this->rect.TopLeft();
    this->field_0x20cc = CRect(topleft.x + 0x2E, topleft.y, topleft.x + 0x76, topleft.y + 0x20);
    this->field_0x20dc = CRect(topleft.x + 0x2E, topleft.y + 0x10F, topleft.x + 0x76, topleft.y + 0x12F);
    CPoint origin(this->rect.left, this->field_0x20cc.bottom);

    for (int32_t i = 0; i < this->visible_columns; i++) {
        for (int32_t j = 0; j < this->visible_rows; j++) {
            this->field_0x20c4[j * this->visible_columns + i] = CRect(
                origin.x + i * 0x50 + 1,
                origin.y + j * 0x50 - 1,
                origin.x + i * 0x50 + 0x51,
                origin.y + j * 0x50 + 0x4F);
        }
    }
}


// 4B663C
void VisInvExtType1::VMethod41()
{
    this->VMethod42();
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


// 4B69EE
void VisInvExtType1Druid::VMethod41()
{
    this->VMethod42();
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


// 4B6815
void VisInvExtType1Kaarg::VMethod41()
{
    this->VMethod42();
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


// 4B6BC7
void VisInvExtType1::VMethod42()
{
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


// 4B7D1C
int32_t VisInvExtType1::VMethod37(int32_t idx)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 == nullptr) {
        return -1;
    }
    int32_t count = main_wnd->field_0x408->field_0x10;
    int32_t tab_before = this->VMethod38();
    int32_t new_idx = this->VMethod26(main_wnd->field_0x408, idx);
    TokenEntry* entry = this->grid_source->GetAt(new_idx);
    if (this->VMethod38() == 2) {
        this->VisInvBase::VMethod33(main_wnd->vis_map_context->field_0x138);
    } else {
        this->VMethod32(this->grid_source);
    }
    if (tab_before != main_wnd->field_0x410) {
        main_wnd->vis_map_context->sub_41A7C7(main_wnd->field_0x410, main_wnd->field_0x40c, this->VMethod38(), entry->field_0x20, count);
    }
    main_wnd->ResetItemCursor();
    main_wnd->vis_root->MsgProc(0x46E, this->id, 0);
    return new_idx;
}


// 4B7264
void VisInvExtType1::sub_4B7264()
{
    CPoint topleft = this->field_0x20ac->rect.TopLeft();
    if (this->arrow1 != nullptr) {
        this->arrow1->VMethod2(topleft.x + 0x2E, topleft.y, 0, 0, 0);
    }
}


// 4B72C1
void VisInvExtType1::sub_4B72C1()
{
    CPoint topleft = this->field_0x20ac->rect.TopLeft();
    if (this->arrow3 != nullptr) {
        this->arrow3->VMethod2(topleft.x + 0x2E, topleft.y, 0, 0, 0);
    }
}


// 4B731E
void VisInvExtType1::sub_4B731E()
{
    CPoint topleft = this->field_0x20ac->rect.TopLeft();
    if (this->arrow2 != nullptr) {
        this->arrow2->VMethod2(topleft.x + 0x2E, topleft.y + 0x10F, 0, 0, 0);
    }
}


// 4B7381
void VisInvExtType1::sub_4B7381()
{
    CPoint topleft = this->field_0x20ac->rect.TopLeft();
    if (this->arrow4 != nullptr) {
        this->arrow4->VMethod2(topleft.x + 0x2E, topleft.y + 0x10F, 0, 0, 0);
    }
}


// 4B9E70
int32_t VisInvExtType1::VMethod38()
{
    return (uint16_t)this->field_0x20ac->select_category + 5;
}


// 4B846C
void VisInvExtType2::VMethod7()
{
    VisShop* shop = this->field_0x20ac;
    CPoint topleft = shop->rect.TopLeft();

    if (shop->dialog_active == 0) {
        return;
    }

    LockSurface2();
    g_bmp_invframe->VMethod10(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, this->rect.Width(), this->rect.Height());
    UnlockSurface2();
    VisInvExtBase::VMethod7();

    if (*this->visible_startref > 0) {
        CPoint pos(g_mousept.GetX(), g_mousept.GetY());
        CRect rc = this->field_0x20cc;
        rc.OffsetRect(topleft.x, topleft.y);
        if (rc.PtInRect(pos)) {
            this->sub_4B860E();
        } else {
            this->sub_4B8666();
        }
    }
    if (this->grid_source->GetSize() - *this->visible_startref > this->visible_columns * this->visible_rows) {
        CPoint pos(g_mousept.GetX(), g_mousept.GetY());
        CRect rc = this->field_0x20dc;
        rc.OffsetRect(topleft.x, topleft.y);
        if (rc.PtInRect(pos)) {
            this->sub_4B86BE();
        } else {
            this->sub_4B871C();
        }
    }
}


// 4B8339
const char* VisInvExtType2::GetHint()
{
    VisShop* shop = this->field_0x20ac;
    if (shop->dialog_active == 0) {
        return nullptr;
    }
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        return nullptr;
    }
    CPoint mouse(g_mousept.GetX(), g_mousept.GetY());
    CPoint topleft = shop->rect.TopLeft();
    CPoint rel = mouse - topleft;
    int32_t region = this->VMethod31(&mouse);
    if (region < 0) {
        if (this->field_0x20cc.PtInRect(rel)) {
            return TxtFile::AllLines.GetAt(0x36);
        }
        if (this->field_0x20dc.PtInRect(rel)) {
            return TxtFile::AllLines.GetAt(0x37);
        }
        return TxtFile::AllLines.GetAt(0x3A);
    }
    return VisInvExtBase::GetHint();
}


// 4B87CE
int32_t VisInvExtType2::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    VisShop* shop = this->field_0x20ac;
    CPoint topleft = shop->rect.TopLeft();
    CPoint rel = pos - topleft;

    if (this->field_0x20cc.PtInRect(rel)) {
        this->sub_4A5350();
        FUN_00438f20(&this->field_0x20bc.sample);
        CSound::Play(this->field_0x20bc);
        return 1;
    }
    if (this->field_0x20dc.PtInRect(rel)) {
        this->sub_4A51C9();
        FUN_00438f20(&this->field_0x20bc.sample);
        CSound::Play(this->field_0x20bc);
        return 1;
    }
    return VisInvExtBase::OnLButtonDown(wparam, pos);
}


// 4B88EC
int32_t VisInvExtType2::OnMouseMove(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        switch (main_wnd->field_0x408->field_0x18) {
        case 1:
        case 2:
            this->field_0x20ac->placement_lock = 1;
            ApplyCursor(main_wnd->item_cursor);
            break;
        default:
            ApplyCursor(g_Cursors[23]);
            this->field_0x20ac->placement_lock = 0;
            break;
        }
    }
    return VisInvExtBase::OnMouseMove(wparam, pos);
}


// 4B877A
int32_t VisInvExtType2::OnWmUser(uint32_t wparam, CPoint pos)
{
    if (wparam & 1) {
        this->OnLButtonDown(wparam, pos);
    }
    return 1;
}


// 4B87AD
int32_t VisInvExtType2::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    return VisInvExtBase::OnLButtonUp(wparam, pos);
}


// 4B9EA0
int32_t VisInvExtType2::VMethod27(TokenEntry* o)
{
    o->field_0x18 = 2;
    return VisInvBase::VMethod27(o);
}


// 4B9ED0
int32_t VisInvExtType2::VMethod26(TokenEntry* o, int32_t idx)
{
    o->field_0x18 = 2;
    return VisInvBase::VMethod26(o, idx);
}


// 4B9457
int32_t VisInvExtType3::VMethod26(TokenEntry* o, int32_t idx)
{
    if (o->field_0x18 == 1) {
        o->field_0x18 = 2;
    }
    return VisInvBase::VMethod26(o, idx);
}


// 4B95C5
TokenEntry* VisInvExtType3::VMethod43(int32_t id1, int32_t id2)
{
    if (this->grid_source == nullptr) {
        return nullptr;
    }
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (entry->field_0x4 != id1) {
            continue;
        }
        if (entry->field_0x18 != id2) {
            continue;
        }
        if (entry->sub_4A7880(1) == 0) {
            this->grid_source->RemoveAt(i, 1);
            entry->field_0x10 = 1;
            this->sub_4B4FD1();
            return entry;
        }
        TokenEntry* copy = new TokenEntry();
        *copy = *entry;
        copy->field_0x10 = 1;
        this->sub_4B4FD1();
        return copy;
    }
    this->sub_4B4FD1();
    return nullptr;
}


// 4B970E
void VisInvExtType3::sub_4B970E()
{
    while (this->field_0x20f4.GetSize() != 0) {
        if (this->field_0x20f4.GetAt(0) != nullptr) {
            delete this->field_0x20f4.GetAt(0);
        }
        this->field_0x20f4.RemoveAt(0, 1);
    }
}


// 4B90CF
const char* VisInvExtType3::GetHint()
{
    VisShop* shop = this->field_0x20ac;
    if (shop->dialog_active == 0) {
        return nullptr;
    }
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        return nullptr;
    }
    CPoint mouse(g_mousept.GetX(), g_mousept.GetY());
    int32_t region = this->VMethod31(&mouse);
    if (region < 0) {
        return TxtFile::AllLines.GetAt(0x3B);
    }
    return VisInvExtBase::GetHint();
}


// 4B8E43
void VisInvExtType3::VMethod42()
{
    if (this->shoptable != nullptr) {
        delete this->shoptable;
    }
    this->shoptable = nullptr;
}


// 4B9B2F
int32_t VisInvExtType3::sub_4B9B2F(int32_t idx)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 == nullptr) {
        return -1;
    }
    TokenEntry* entry = main_wnd->field_0x408;
    int32_t amount = entry->field_0x10;
    int32_t grade = -1;
    if (main_wnd->field_0x410 >= 5 && main_wnd->field_0x410 <= 8) {
        grade = entry->field_0x20;
    } else {
        grade = main_wnd->field_0x40c;
    }
    int32_t result = this->VMethod26(entry, idx);
    this->VMethod32(this->grid_source);
    uint8_t kind = this->VMethod38();
    main_wnd->vis_map_context->sub_41A7C7(main_wnd->field_0x410, grade, kind, result, amount);
    main_wnd->ResetItemCursor();
    main_wnd->vis_root->MsgProc(0x46E, this->id, 0);
    return result;
}


// 4B8CC5
VisInvExtType3::~VisInvExtType3()
{
    this->grid_source->RemoveAll();
    this->grid_source = nullptr;
    this->sub_4B4C1C();
}


// 4B8BBA
VisInvExtType3::VisInvExtType3(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisInvExtBase(_id, l, t, r, b, shop)
{
    this->visible_columns = this->rect.Width() / 0x50 - 1;
    this->visible_rows = this->rect.Height() / 0x50;
    this->grid_source = &this->field_0x20f4;
    this->field_0x20f4.RemoveAll();
    this->VMethod39();
    this->VisInvBase::VMethod35();
    this->shoptable = nullptr;
}


// 4B8D43
void VisInvExtType3::VMethod41()
{
    this->VMethod42();
    CString path = "graphics\\interface\\";
    if (this->field_0x20ac != nullptr) {
        path += this->field_0x20ac->VMethod33();
    }
    path += "ShopTable.bmp";
    this->shoptable = new CBmp64(path);
}


// 4B8E9A
void VisInvExtType3::VMethod39()
{
    this->field_0x20c4 = new CRect[this->visible_columns * this->visible_rows];

    CPoint topleft = this->rect.TopLeft();
    CPoint bottomright = this->rect.BottomRight();

    this->field_0x20cc = CRect(topleft.x, topleft.y, topleft.x + 0x20, bottomright.y);
    this->field_0x20dc = CRect(topleft.x + 0x1B0, topleft.y, bottomright.x, bottomright.y);

    CPoint start(this->field_0x20cc.right, this->field_0x20cc.top);
    for (int32_t col = 0; col < this->visible_columns; col++) {
        for (int32_t row = 0; row < this->visible_rows; row++) {
            this->field_0x20c4[row * this->visible_columns + col] = CRect(
                start.x + col * 0x50,
                start.y + row * 0x50,
                start.x + col * 0x50 + 0x50,
                start.y + row * 0x50 + 0x50);
        }
    }
}


// 4B9F10
int32_t VisInvExtType3::VMethod38()
{
    return 4;
}


// 4B9790
int32_t VisInvExtType3::VMethod37(int32_t idx)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (GetRandS16(100) < 0x1E && main_wnd->sessionMode == 2 && main_wnd->field_0x408 != nullptr) {
        TokenEntry* entry = main_wnd->field_0x408;
        if (entry->field_0x18 > 4 && entry->field_0x18 < 9) {
            char shopname[80];
            if (main_wnd->vis_shopkaarg == this->field_0x20ac) {
                strcpy(shopname, "shop_kaarg");
            } else if (main_wnd->vis_shopdruid == this->field_0x20ac) {
                strcpy(shopname, "shop_druid");
            } else {
                strcpy(shopname, "shop");
            }
            char path[1024];
            int32_t effect = entry->sub_43988E();
            if (effect != 0) {
                sprintf(path, "speech\\%s\\effects\\%.2d.wav", shopname, effect);
            } else if (entry->GetAttribute(0x2A) != 0) {
                sprintf(path, "speech\\%s\\books\\%.2d.wav", shopname, entry->GetAttribute(0x2A));
            } else {
                sprintf(path, "speech\\%s\\s%.2di%.2dp%d.wav", shopname, entry->GetType(), entry->GetId(), GetRandS16(3) + 1);
            }
            if (this->field_0x20b0.sample != nullptr) {
                if (this->field_0x20b0.sample->FindPlayingChannel() == nullptr) {
                    delete this->field_0x20b0.sample;
                    this->field_0x20b0.sample = nullptr;
                    this->field_0x20b0.sample = new SfxSample(path);
                    this->field_0x20b0.sample->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
                }
            } else {
                this->field_0x20b0.sample = new SfxSample(path);
                this->field_0x20b0.sample->Play(g_SoundSettings.speech_pos, 0, 0, 0x80, 0);
            }
        }
    }
    FUN_00438f20(&this->field_0x20b4.sample);
    CSound::Play(this->field_0x20b4);
    return this->sub_4B9B2F(idx);
}


// 4B9C3C
TokenEntry* VisInvExtType3::VMethod36(int32_t idx, int32_t num)
{
    FUN_00438f20(&this->field_0x20b8.sample);
    CSound::Play(this->field_0x20b8);
    return VisInvBase::VMethod36(idx, num);
}


// 4B9436
int32_t VisInvExtType3::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    return VisInvExtBase::OnLButtonDblClk(wparam, pos);
}


// 4B93AE
int32_t VisInvExtType3::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    return VisInvExtBase::OnLButtonDown(wparam, pos);
}


// 4B93CF
int32_t VisInvExtType3::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 == nullptr) {
        return 1;
    }
    if (this->sub_4B91F9(main_wnd->field_0x408) != 0) {
        this->field_0x20ac->placement_lock = 0;
    }
    return VisInvExtBase::OnLButtonUp(wparam, pos);
}


// 4B92F9
int32_t VisInvExtType3::OnMouseMove(uint32_t wparam, CPoint pos)
{
    if (wparam & 1) {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        if (main_wnd->field_0x408 != nullptr) {
            if (this->sub_4B91F9(main_wnd->field_0x408) == 0) {
                if (main_wnd->field_0x408->GetAttribute(1) > 0) {
                    this->field_0x20ac->placement_lock = 1;
                    ApplyCursor(main_wnd->item_cursor);
                }
            } else {
                this->field_0x20ac->placement_lock = 0;
                ApplyCursor(g_Cursors[23]);
            }
        }
    }
    return VisInvExtBase::OnMouseMove(wparam, pos);
}


// 4B9155
void VisInvExtType3::VMethod7()
{
    VisShop* shop = this->field_0x20ac;
    CPoint topleft = shop->rect.TopLeft();

    if (shop->dialog_active == 0) {
        return;
    }
    if (this->shoptable == nullptr) {
        return;
    }

    LockSurface2();
    this->shoptable->VMethod10(topleft.x + this->rect.left, topleft.y + this->rect.top, 0, 0, this->rect.Width(), this->rect.Height());
    UnlockSurface2();
    VisInvExtBase::VMethod7();
}


// 4B91F9
int32_t VisInvExtType3::sub_4B91F9(TokenEntry* o)
{
    TokenEntry copy(o);
    if (o->field_0x18 == 1) {
        copy.field_0x18 = 2;
    }
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (copy.sub_4A7900(entry)) {
            return 0;
        }
    }
    return this->grid_source->GetSize() >= this->visible_columns * this->visible_rows;
}


// 4B949A
int32_t VisInvExtType3::VMethod27(TokenEntry* o)
{
    if (this->grid_source == nullptr) {
        return -1;
    }
    for (int32_t i = 0; i < this->grid_source->GetSize(); i++) {
        TokenEntry* entry = this->grid_source->GetAt(i);
        if (entry->field_0x4 != o->field_0x4) {
            continue;
        }
        if (entry->field_0x18 != o->field_0x18) {
            continue;
        }
        entry->sub_4A7850(o->field_0x10);
        delete o;
        return i;
    }
    if (this->grid_source->GetSize() < this->visible_columns * this->visible_rows) {
        this->grid_source->Add(o);
    }
    return this->grid_source->GetSize() - 1;
}


// 4B80D9
VisInvExtType2::~VisInvExtType2()
{
    this->visible_startref = nullptr;
}


// 4B7FE5
VisInvExtType2::VisInvExtType2(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop)
    : VisInvExtBase(_id, l, t, r, b, shop)
{
    this->visible_columns = this->rect.Width() / 0x50 - 1;
    this->visible_rows = this->rect.Height() / 0x50;
    delete this->visible_startref;
    this->visible_startref = nullptr;
    this->VMethod39();
    this->VisInvBase::VMethod35();
}


// 4B8102
void VisInvExtType2::VMethod39()
{
    VisShop* shop = this->field_0x20ac;
    CPoint topleft = shop->rect.TopLeft();
    CPoint bottomright = shop->rect.BottomRight();

    this->field_0x20c4 = new CRect[this->visible_columns * this->visible_rows];
    this->field_0x20cc = CRect(topleft.x, topleft.y, topleft.x + 0x20, bottomright.y);
    this->field_0x20dc = CRect(topleft.x + 0x1B0, topleft.y, bottomright.x, bottomright.y);

    CPoint start(this->field_0x20cc.right, this->field_0x20cc.top);
    for (int32_t col = 0; col < this->visible_columns; col++) {
        for (int32_t row = 0; row < this->visible_rows; row++) {
            this->field_0x20c4[row * this->visible_columns + col] = CRect(
                start.x + col * 0x50,
                start.y + row * 0x50 + 5,
                start.x + col * 0x50 + 0x50,
                start.y + row * 0x50 + 0x55);
        }
    }
}


// 4B860E
void VisInvExtType2::sub_4B860E()
{
    CPoint topleft = this->field_0x20ac->rect.TopLeft();
    if (g_bmp_invarrow1 != nullptr) {
        g_bmp_invarrow3->VMethod10(topleft.x, topleft.y + 0x188, 0, 0, 0x20, 0x58);
    }
}


// 4B8666
void VisInvExtType2::sub_4B8666()
{
    CPoint topleft = this->field_0x20ac->rect.TopLeft();
    if (g_bmp_invarrow3 != nullptr) {
        g_bmp_invarrow1->VMethod10(topleft.x, topleft.y + 0x188, 0, 0, 0x20, 0x58);
    }
}


// 4B86BE
void VisInvExtType2::sub_4B86BE()
{
    CPoint topleft = this->field_0x20ac->rect.TopLeft();
    if (g_bmp_invarrow2 != nullptr) {
        g_bmp_invarrow4->VMethod10(topleft.x + 0x1B0, topleft.y + 0x188, 0, 0, 0x20, 0x58);
    }
}


// 4B871C
void VisInvExtType2::sub_4B871C()
{
    CPoint topleft = this->field_0x20ac->rect.TopLeft();
    if (g_bmp_invarrow4 != nullptr) {
        g_bmp_invarrow2->VMethod10(topleft.x + 0x1B0, topleft.y + 0x188, 0, 0, 0x20, 0x58);
    }
}


// 4B9F00
int32_t VisInvExtType2::VMethod38()
{
    return 2;
}

// Statics for VisStartGame::VMethod7 (659514-659534 in the binary).
static uint8_t startgame_init_flags = 0;    // 659514 init-done flags for the three timestamps/delays below
static uint32_t startgame_blind_delay = 0;  // 659528 delay before the next blind-spawn event (ms)
static uint32_t startgame_blind_ts = 0;     // 65954C blind animation timestamp
static uint32_t startgame_frame_ts = 0;     // 659534 frame throttle timestamp


// 435D72
void VisStartGame::VMethod7()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if ((startgame_init_flags & 1) == 0) {
        startgame_init_flags |= 1;
        startgame_blind_delay = rand() / 0x41 + 500;
    }
    if ((startgame_init_flags & 2) == 0) {
        startgame_init_flags |= 2;
        startgame_blind_ts = timeGetTime();
    }
    uint32_t now = timeGetTime();
    CPoint screen_pt = this->rect.TopLeft();
    int32_t screen_x = screen_pt.x;
    int32_t screen_y = screen_pt.y;

    if ((startgame_init_flags & 4) == 0) {
        startgame_init_flags |= 4;
        startgame_frame_ts = timeGetTime();
    }
    if (timeGetTime() - startgame_frame_ts <= 0x43) {
        return;
    }
    startgame_frame_ts = timeGetTime();

    if (g_mousept.GetCursorSprite() != g_Cursors[CURSOR_SELECT]->GetSprite()) {
        g_Cursors[CURSOR_SELECT]->Use();
    }

    CRect old_clip;
    GetClipRect(&old_clip);
    SetClipRect(this->rect);
    LockSurface2();

    this->mainAreaBitmap->VMethod2(this->rect.left, this->rect.top, 0, 0, 0);

    if (this->returnToGameHoverBitmap != nullptr) {
        this->returnToGameHoverBitmap->VMethod10(
            screen_x + this->returnToGameButtonRect.left,
            screen_y + this->returnToGameButtonRect.top,
            0, 0,
            this->returnToGameButtonRect.Width(),
            this->returnToGameButtonRect.Height());
    }
    if (this->acceptHoverBitmap != nullptr) {
        this->acceptHoverBitmap->VMethod10(
            screen_x + this->acceptButtonRect.left,
            screen_y + this->acceptButtonRect.top,
            0, 0,
            this->acceptButtonRect.Width(),
            this->acceptButtonRect.Height());
    }
    if (main_wnd->sessionMode == 2) {
        for (int32_t i = 0; i < 3; i++) {
            if (this->difficultyStateFlags.ElementAt(i) == 2) {
                const CRect& r = this->difficultyRects.ElementAt(i);
                this->difficultyHoverBitmaps.GetAt(i)->VMethod10(
                    screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
            }
            if (this->difficultyStateFlags.ElementAt(i) == 1) {
                const CRect& r = this->difficultyRects.ElementAt(i);
                this->difficultySelectedBitmaps.GetAt(i)->VMethod10(
                    screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
            }
            if (this->difficultyStateFlags.ElementAt(i) == 3) {
                const CRect& r = this->difficultyRects.ElementAt(i);
                this->difficultySelectedHoverBitmaps.GetAt(i)->VMethod10(
                    screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
            }
        }
    }
    if (this->portraitStateFlags.ElementAt(0) & 1) {
        if ((this->portraitStateFlags.ElementAt(1) & 2) == 0) {
            this->portraitHoverBitmaps.GetAt(0)->VMethod10(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        } else {
            this->portraitSelectedHoverBitmaps.GetAt(0)->VMethod10(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        }
        if (this->portraitStateFlags.ElementAt(2) & 2) {
            const CRect& r = this->portraitRects.ElementAt(2);
            this->portraitSelectedBitmaps.GetAt(2)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
        if (this->portraitStateFlags.ElementAt(3) & 2) {
            const CRect& r = this->portraitRects.ElementAt(3);
            this->portraitSelectedBitmaps.GetAt(3)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
    }
    if (this->portraitStateFlags.ElementAt(1) & 1) {
        if ((this->portraitStateFlags.ElementAt(0) & 2) == 0) {
            this->portraitHoverBitmaps.GetAt(1)->VMethod10(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        } else {
            this->portraitSelectedHoverBitmaps.GetAt(1)->VMethod10(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        }
        if (this->portraitStateFlags.ElementAt(2) & 2) {
            const CRect& r = this->portraitRects.ElementAt(2);
            this->portraitSelectedBitmaps.GetAt(2)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
        if (this->portraitStateFlags.ElementAt(3) & 2) {
            const CRect& r = this->portraitRects.ElementAt(3);
            this->portraitSelectedBitmaps.GetAt(3)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
    }
    if (this->portraitStateFlags.ElementAt(2) & 1) {
        if ((this->portraitStateFlags.ElementAt(3) & 2) == 0) {
            this->portraitHoverBitmaps.GetAt(2)->VMethod10(screen_x + 0x104, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        } else {
            this->portraitSelectedHoverBitmaps.GetAt(2)->VMethod10(screen_x + 0x104, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        }
        if (this->portraitStateFlags.ElementAt(0) & 2) {
            const CRect& r = this->portraitRects.ElementAt(0);
            this->portraitSelectedBitmaps.GetAt(0)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
        if (this->portraitStateFlags.ElementAt(1) & 2) {
            const CRect& r = this->portraitRects.ElementAt(1);
            this->portraitSelectedBitmaps.GetAt(1)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
    }
    if (this->portraitStateFlags.ElementAt(3) & 1) {
        if ((this->portraitStateFlags.ElementAt(2) & 2) == 0) {
            this->portraitHoverBitmaps.GetAt(3)->VMethod10(screen_x + 0x104, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        } else {
            this->portraitSelectedHoverBitmaps.GetAt(3)->VMethod10(screen_x + 0x104, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        }
        if (this->portraitStateFlags.ElementAt(0) & 2) {
            const CRect& r = this->portraitRects.ElementAt(0);
            this->portraitSelectedBitmaps.GetAt(0)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
        if (this->portraitStateFlags.ElementAt(1) & 2) {
            const CRect& r = this->portraitRects.ElementAt(1);
            this->portraitSelectedBitmaps.GetAt(1)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
    }
    if (this->returnToGameHoverBitmap != nullptr) {
        this->returnToGameHoverBitmap->VMethod10(
            screen_x + this->returnToGameButtonRect.left,
            screen_y + this->returnToGameButtonRect.top,
            0, 0,
            this->returnToGameButtonRect.Width(),
            this->returnToGameButtonRect.Height());
    }
    if (this->acceptHoverBitmap != nullptr) {
        this->acceptHoverBitmap->VMethod10(
            screen_x + this->acceptButtonRect.left,
            screen_y + this->acceptButtonRect.top,
            0, 0,
            this->acceptButtonRect.Width(),
            this->acceptButtonRect.Height());
    }
    if (g_settings.TipsMode != 0 && this->tipsPrompt != nullptr) {
        uint32_t hotspot = this->GetHotspotId(g_mousept.GetX(), g_mousept.GetY());
        this->DrawTipsHighlight(hotspot);
    }

    if (this->blindAnimationFrame == 0) {
        if (now - startgame_blind_ts > startgame_blind_delay) {
            int32_t idx = rand() / (0x7FFF / this->blindSpawnRects.GetSize());
            CRect spawn_rect = this->blindSpawnRects.ElementAt(idx);
            int32_t x_off = rand() / (0x7FFF / spawn_rect.Width());
            int32_t y_off = rand() / (0x7FFF / spawn_rect.Height());
            this->blindAnimationPosition = spawn_rect.TopLeft() + CPoint(x_off, y_off);
            startgame_blind_delay = rand() / 0x41 + 500;
            this->blindAnimationFrame = 1;
        }
    } else {
        this->blindAnimation->VMethod2(
            screen_x + this->blindAnimationPosition.x,
            screen_y + this->blindAnimationPosition.y,
            this->blindAnimationFrame, 0, 0);
        if (now - startgame_blind_ts > 0x3F) {
            this->blindAnimationFrame = (this->blindAnimationFrame + 1) % (int32_t)this->blindAnimation->GetFrameCount();
            startgame_blind_ts = now;
        }
    }

    CVisualObject* obj = this->FindChild(0x464);
    if (obj != nullptr) {
        CPoint pt = obj->GetRect().TopLeft();
        g_font4->DrawTxt(screen_x + pt.x - 10, screen_y + pt.y, TxtFile::AllLines.GetAt(0x16D), 1, palette_husk->GetPalette(0));
    }
    obj = this->FindChild(0x465);
    if (obj != nullptr) {
        CPoint pt = obj->GetRect().TopLeft();
        g_font4->DrawTxt(screen_x + pt.x - 10, screen_y + pt.y, TxtFile::AllLines.GetAt(0x16E), 1, palette_husk->GetPalette(0));
    }

    this->torchFrameTick = this->torchFrameTick + 1;
    this->leftTorchFrames.GetAt(this->torchFrameTick % 0xF)->VMethod2(screen_x + 4, screen_y + 200, 0, 0, 0);
    this->rightTorchFrames.GetAt((this->torchFrameTick + 8) % 0xF)->VMethod2(screen_x + 0x24C, screen_y + 200, 0, 0, 0);

    UnlockSurface2();
    SetClipRect(old_clip);
    VisScreen::VMethod7();
}


// 4333B9
void VisStartGame::VMethod26()
{
    this->dialogActiveFlag = 0;
    this->blindAnimationPosition = CPoint(0, 0);
    this->blindAnimationFrame = 0;
    this->mainAreaBitmap = nullptr;
    this->hotspotMaskBitmap = nullptr;
    this->returnToGameButtonBitmap = nullptr;
    this->acceptButtonBitmap = nullptr;
    this->blindAnimation = nullptr;
    this->tableauBitmap = nullptr;
    this->returnToGameHoverBitmap = nullptr;
    this->acceptHoverBitmap = nullptr;
    this->field_0x1bc = 0;
    this->field_0x1c0 = 0;
    this->difficultyLevel1Sound.sample = nullptr;
    this->difficultyLevel2Sound.sample = nullptr;
    this->difficultyLevel3Sound.sample = nullptr;
    this->portraitSelectSound.sample = nullptr;
    this->acceptSound.sample = nullptr;
    this->returnSound.sample = nullptr;
    this->labelInputSound1.sample = nullptr;
    this->labelInputSound2.sample = nullptr;
    this->labelInputSound3.sample = nullptr;
    this->labelInputSoundIndex = 0;

    this->portraitHoverBitmaps.SetSize(4, -1);
    this->portraitSelectedBitmaps.SetSize(4, -1);
    this->portraitSelectedHoverBitmaps.SetSize(4, -1);
    this->difficultySelectedBitmaps.SetSize(3, -1);
    this->difficultyHoverBitmaps.SetSize(3, -1);
    this->difficultySelectedHoverBitmaps.SetSize(3, -1);
    this->portraitRects.SetSize(4, -1);
    this->difficultyRects.SetSize(3, -1);
    this->portraitStateFlags.SetSize(4, -1);
    this->difficultyStateFlags.SetSize(3, -1);

    this->difficultyRects.ElementAt(0) = CRect(CPoint(8, 0), CSize(0x30, 0x48));
    this->difficultyRects.ElementAt(1) = CRect(CPoint(0x128, 0), CSize(0x30, 0x48));
    this->difficultyRects.ElementAt(2) = CRect(CPoint(0x244, 0), CSize(0x30, 0x48));
    this->portraitRects.ElementAt(0) = CRect(CPoint(0x74, 0x2C), CSize(0x40, 0xF4));
    this->portraitRects.ElementAt(1) = CRect(CPoint(0xB4, 0x2C), CSize(0x40, 0xF4));
    this->portraitRects.ElementAt(2) = CRect(CPoint(0x188, 0x2C), CSize(0x40, 0xF4));
    this->portraitRects.ElementAt(3) = CRect(CPoint(0x1C8, 0x2C), CSize(0x40, 0xF4));
    this->returnToGameButtonRect = CRect(CPoint(0x10, 400), CSize(0x40, 0x4C));
    this->acceptButtonRect = CRect(CPoint(0x224, 400), CSize(0x50, 0x4C));

    this->blindSpawnRects.SetSize(this->difficultyRects.GetSize() + 2, -1);
    for (int32_t i = 0; i < this->difficultyRects.GetSize(); i++) {
        this->blindSpawnRects.ElementAt(i) = this->difficultyRects.ElementAt(i);
    }
    this->blindSpawnRects.ElementAt(this->difficultyRects.GetSize()) = this->returnToGameButtonRect;
    this->blindSpawnRects.ElementAt(this->difficultyRects.GetSize() + 1) = this->acceptButtonRect;

    this->char_name = "Master Oberic";

    this->networkNameLabel = new VisStartGameTextBox(0x464, 0x12C, 0x1B1, 0x1D0, 0x1C1, this);
    this->nameLabel = new VisStartGameTextBox(0x464, 0x12C, 0x1A6, 0x1D0, 0x1B6, this);
    this->clanLabel = new VisStartGameTextBox(0x465, 0x12C, 0x1BC, 0x1D0, 0x1CC, this);
    this->tipsPrompt = nullptr;
    this->tipsProgress = 0;
    this->torchFrameTick = 0;
}


// 43769C
int32_t VisStartGame::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    VisStartGameTextBox* label;
    if (main_wnd->sessionMode == 2) {
        label = this->networkNameLabel;
    } else {
        label = this->nameLabel;
    }
    uint32_t hotspot = this->UpdateHotspots(pos.x, pos.y, 1);
    this->UpdateTipsProgress(hotspot);
    switch (hotspot) {
    case 0x14:
        if (main_wnd->sessionMode == 2) {
            this->committedDifficultyIndex = 0;
            FUN_00438f20(&this->difficultyLevel1Sound.sample);
            CSound::Play(this->difficultyLevel1Sound);
        }
        break;
    case 0x28:
        if (main_wnd->sessionMode == 2) {
            this->committedDifficultyIndex = 1;
            FUN_00438f20(&this->difficultyLevel2Sound.sample);
            CSound::Play(this->difficultyLevel2Sound);
        }
        break;
    case 0x3C:
        if (main_wnd->sessionMode == 2) {
            this->committedDifficultyIndex = 2;
            FUN_00438f20(&this->difficultyLevel3Sound.sample);
            CSound::Play(this->difficultyLevel3Sound);
        }
        break;
    case 0x50:
        if (this->committedPortraitIndex != 0) {
            CString text = label->GetText();
            if (IsDefaultNpcName(text)) {
                label->SetText(txt_npcnames.GetLine(0x17));
            }
        }
        this->committedPortraitIndex = 0;
        FUN_00438f20(&this->portraitSelectSound.sample);
        CSound::Play(this->portraitSelectSound);
        break;
    case 100:
        if (this->committedPortraitIndex != 1) {
            CString text = label->GetText();
            if (IsDefaultNpcName(text)) {
                label->SetText(txt_npcnames.GetLine(0x18));
            }
        }
        this->committedPortraitIndex = 1;
        FUN_00438f20(&this->portraitSelectSound.sample);
        CSound::Play(this->portraitSelectSound);
        break;
    case 0x78:
        if (this->committedPortraitIndex != 2) {
            CString text = label->GetText();
            if (IsDefaultNpcName(text)) {
                label->SetText(txt_npcnames.GetLine(0x1A));
            }
        }
        this->committedPortraitIndex = 2;
        FUN_00438f20(&this->portraitSelectSound.sample);
        CSound::Play(this->portraitSelectSound);
        break;
    case 0x8C:
        if (this->committedPortraitIndex != 3) {
            CString text = label->GetText();
            if (IsDefaultNpcName(text)) {
                label->SetText(txt_npcnames.GetLine(0x19));
            }
        }
        this->committedPortraitIndex = 3;
        FUN_00438f20(&this->portraitSelectSound.sample);
        CSound::Play(this->portraitSelectSound);
        break;
    case 0xA0:
        this->Cancel();
        break;
    case 0xB4:
        this->Accept();
        break;
    }
    return VisScreen::OnLButtonDown(wparam, pos);
}


// Statics for VisStartGame::DrawTipsHighlight (65951C-659554, 62CCFC-62CD00 in the binary).
static uint8_t startgame_tips_init_flags = 0;     // 65951C init-done flags for the two timestamps below
static uint32_t startgame_tips_show_ts = 0;       // 659524 timestamp when the cursor entered the current hotspot
static uint32_t startgame_tips_frame_ts = 0;      // 65952C timestamp of the last highlight frame advance
static uint32_t startgame_tips_frame = 0;         // 659554 current highlight animation frame
static int32_t startgame_tips_last_hotspot = -1;  // 62CCFC hotspot id the highlight is currently locked to
static int32_t startgame_tips_cycle_len = 1;      // 62CD00 frames per cycle for the active tips section (-1 = none)


// 436E4B
uint32_t VisStartGame::DrawTipsHighlight(uint32_t hotspot)
{
    CPoint screen_pt = this->rect.TopLeft();
    int32_t screen_x = screen_pt.x;
    int32_t screen_y = screen_pt.y;

    if ((startgame_tips_init_flags & 1) == 0) {
        startgame_tips_init_flags |= 1;
        startgame_tips_show_ts = timeGetTime();
    }
    if ((startgame_tips_init_flags & 2) == 0) {
        startgame_tips_init_flags |= 2;
        startgame_tips_frame_ts = timeGetTime();
    }
    uint32_t now = timeGetTime();
    if (now - startgame_tips_show_ts < 500) {
        startgame_tips_frame_ts = now;
        return now;
    }

    switch (startgame_tips_last_hotspot) {
    case 0x14:
        startgame_tips_frame = 1;
        break;
    case 0x28:
        startgame_tips_frame = 2;
        break;
    case 0x3C:
        startgame_tips_frame = 3;
        break;
    case 0x50:
        startgame_tips_frame = 1;
        break;
    case 0x64:
        startgame_tips_frame = 2;
        break;
    case 0x78:
        startgame_tips_frame = 3;
        break;
    case 0x8C:
        startgame_tips_frame = 4;
        break;
    case 0xA0:
        startgame_tips_frame = 1;
        break;
    case 0xB4:
        startgame_tips_frame = 2;
        break;
    }

    if (this->tipsProgress == 0) {
        if (hotspot == 0x50 || hotspot == 0x64 || hotspot == 0x8C || hotspot == 0x78) {
            startgame_tips_show_ts = now;
            startgame_tips_frame_ts = now;
            startgame_tips_last_hotspot = hotspot;
            return now;
        }
        startgame_tips_cycle_len = 4;
        startgame_tips_frame = startgame_tips_frame % 4;
        if (startgame_tips_frame == 1) {
            this->portraitSelectedHoverBitmaps.GetAt(0)->VMethod10(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        } else {
            this->portraitHoverBitmaps.GetAt(0)->VMethod10(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        }
        if (startgame_tips_frame == 2) {
            const CRect& r = this->portraitRects.ElementAt(2);
            this->portraitSelectedBitmaps.GetAt(2)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
        if (startgame_tips_frame == 3) {
            const CRect& r = this->portraitRects.ElementAt(3);
            this->portraitSelectedBitmaps.GetAt(3)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
    } else if (this->tipsProgress == 1) {
        if (hotspot == 0x14 || hotspot == 0x28 || hotspot == 0x3C) {
            startgame_tips_show_ts = now;
            startgame_tips_frame_ts = now;
            startgame_tips_last_hotspot = hotspot;
            return now;
        }
        startgame_tips_cycle_len = 3;
        startgame_tips_frame = startgame_tips_frame % 3;
        if (this->difficultyStateFlags.ElementAt(startgame_tips_frame) == 1) {
            const CRect& r = this->difficultyRects.ElementAt(startgame_tips_frame);
            this->difficultySelectedHoverBitmaps.GetAt(startgame_tips_frame)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        } else {
            const CRect& r = this->difficultyRects.ElementAt(startgame_tips_frame);
            this->difficultyHoverBitmaps.GetAt(startgame_tips_frame)->VMethod10(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
    } else if (this->tipsProgress == 2) {
        if (hotspot == 0xA0 || hotspot == 0xB4) {
            startgame_tips_show_ts = now;
            startgame_tips_frame_ts = now;
            startgame_tips_last_hotspot = hotspot;
            return now;
        }
        startgame_tips_cycle_len = 2;
        startgame_tips_frame = startgame_tips_frame % 2;
        if (startgame_tips_frame == 0) {
            this->returnToGameButtonBitmap->VMethod10(
                screen_x + this->returnToGameButtonRect.left,
                screen_y + this->returnToGameButtonRect.top,
                0, 0,
                this->returnToGameButtonRect.Width(),
                this->returnToGameButtonRect.Height());
        } else if (startgame_tips_frame == 1) {
            this->acceptButtonBitmap->VMethod10(
                screen_x + this->acceptButtonRect.left,
                screen_y + this->acceptButtonRect.top,
                0, 0,
                this->acceptButtonRect.Width(),
                this->acceptButtonRect.Height());
        }
    } else {
        startgame_tips_cycle_len = -1;
    }

    startgame_tips_last_hotspot = -1;
    uint32_t elapsed = now - startgame_tips_frame_ts;
    if (elapsed > 0x12C && startgame_tips_cycle_len != -1) {
        uint32_t next_frame = startgame_tips_frame + 1;
        startgame_tips_frame = next_frame % startgame_tips_cycle_len;
        startgame_tips_frame_ts = now;
        return next_frame / startgame_tips_cycle_len;
    }
    return elapsed;
}


// 435867
uint32_t VisStartGame::UpdateHotspots(int32_t x, int32_t y, uint32_t pressed)
{
    uint32_t hotspot = this->GetHotspotId(x, y);
    this->ClearPortraitHoverFlags();
    this->ClearDifficultyHoverFlags();
    if (pressed != 0) {
        switch (hotspot) {
        case 0x14:
            this->ClearDifficultySelectedFlags();
            this->difficultyStateFlags.ElementAt(0) |= 1;
            this->selectedDifficultyIndex = 0;
            break;
        case 0x28:
            this->ClearDifficultySelectedFlags();
            this->difficultyStateFlags.ElementAt(1) |= 1;
            this->selectedDifficultyIndex = 1;
            break;
        case 0x3C:
            this->ClearDifficultySelectedFlags();
            this->difficultyStateFlags.ElementAt(2) |= 1;
            this->selectedDifficultyIndex = 2;
            break;
        case 0x50:
            this->ClearPortraitSelectedFlags();
            this->portraitStateFlags.ElementAt(0) |= 1;
            this->selectedPortraitIndex = 0;
            break;
        case 0x64:
            this->ClearPortraitSelectedFlags();
            this->portraitStateFlags.ElementAt(1) |= 1;
            this->selectedPortraitIndex = 1;
            break;
        case 0x78:
            this->ClearPortraitSelectedFlags();
            this->portraitStateFlags.ElementAt(2) |= 1;
            this->selectedPortraitIndex = 2;
            break;
        case 0x8C:
            this->ClearPortraitSelectedFlags();
            this->portraitStateFlags.ElementAt(3) |= 1;
            this->selectedPortraitIndex = 3;
            break;
        }
    }

    this->acceptHoverBitmap = nullptr;
    this->returnToGameHoverBitmap = nullptr;
    switch (hotspot) {
    case 0x14:
        this->difficultyStateFlags.ElementAt(0) |= 2;
        break;
    case 0x28:
        this->difficultyStateFlags.ElementAt(1) |= 2;
        break;
    case 0x3C:
        this->difficultyStateFlags.ElementAt(2) |= 2;
        break;
    case 0x50:
        this->portraitStateFlags.ElementAt(0) |= 2;
        break;
    case 0x64:
        this->portraitStateFlags.ElementAt(1) |= 2;
        break;
    case 0x78:
        this->portraitStateFlags.ElementAt(2) |= 2;
        break;
    case 0x8C:
        this->portraitStateFlags.ElementAt(3) |= 2;
        break;
    case 0xA0:
        this->returnToGameHoverBitmap = this->returnToGameButtonBitmap;
        break;
    case 0xB4:
        this->acceptHoverBitmap = this->acceptButtonBitmap;
        break;
    }
    return hotspot;
}


// 433E2D
void VisStartGame::VMethod28()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    g_mousept.DisableHint();
    this->torchFrameTick = 0;
    this->LoadBitmaps();
    this->LoadSamples();
    this->ResetHoverState();
    this->blindAnimationPosition = CPoint(0, 0);
    this->blindAnimationFrame = 0;

    for (int32_t i = 0; i < this->portraitStateFlags.GetSize(); i++) {
        this->portraitStateFlags.ElementAt(i) = 0;
    }
    for (int32_t i = 0; i < this->difficultyStateFlags.GetSize(); i++) {
        this->difficultyStateFlags.ElementAt(i) = 0;
    }

    this->selectedPortraitIndex = 0;
    this->portraitStateFlags.ElementAt(this->selectedPortraitIndex) = 1;
    this->difficultyStateFlags.ElementAt(this->selectedDifficultyIndex) = 1;

    if (IsDefaultNpcName(this->char_name)) {
        this->char_name = txt_npcnames.GetLine(0x14);
    }
    this->committedPortraitIndex = this->selectedPortraitIndex;
    this->committedDifficultyIndex = this->selectedDifficultyIndex;

    this->RemoveChildById(0x464);
    this->RemoveChildById(0x465);

    if (main_wnd->sessionMode == 2) {
        this->AddChild(this->networkNameLabel);
        this->networkNameLabel->SetText(this->char_name);
    } else {
        this->AddChild(this->nameLabel);
        this->nameLabel->SetText(this->char_name);
        this->AddChild(this->clanLabel);
        this->clanLabel->SetText(this->clan_name);
    }

    if (g_settings.TipsMode != 0) {
        CString tips;
        MissionGetTips(8, &tips);
        this->tipsPrompt = new VisTipsDialog(0x467, 0xE8, 0x30, 0x280, 0xB8, tips);
        this->AddChild(this->tipsPrompt);
    } else {
        if (this->tipsPrompt != nullptr) {
            this->RemoveChild(this->tipsPrompt);
            delete this->tipsPrompt;
            this->tipsPrompt = nullptr;
        }
    }

    this->tipsProgress = 0;
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();
    VisScreen::VMethod28();
    g_Cursors[CURSOR_SELECT]->Use();
    this->dialogActiveFlag = 1;
    g_mousept.EnableHint();
}


// 4341EB
void VisStartGame::DoClose(uint32_t code)
{
    this->VMethod9();
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->dialogActiveFlag = 0;
    this->FreeBitmaps();
    this->FreeSamples();
    if (main_wnd->sessionMode == 2) {
        this->char_name = this->networkNameLabel->GetText();
    } else {
        this->char_name = this->nameLabel->GetText();
        this->clan_name = this->clanLabel->GetText();
    }
    if (this->tipsPrompt != nullptr) {
        this->RemoveChild(this->tipsPrompt);
        delete this->tipsPrompt;
        this->tipsPrompt = nullptr;
    }
    VisScreen::DoClose(code);
}


// 437E13
const char* VisStartGame::GetHint()
{
    if (this->dialogActiveFlag == 0) {
        return nullptr;
    }

    uint32_t hotspot = this->GetHotspotId(g_mousept.GetX(), g_mousept.GetY());
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode == 2) {
        switch (hotspot) {
        case 0x14:
            return TxtFile::AllLines.GetAt(0xF7);
        case 0x28:
            return TxtFile::AllLines.GetAt(0xF8);
        case 0x3C:
            return TxtFile::AllLines.GetAt(0xF9);
        }
    }

    switch (hotspot) {
    case 0x50:
        return TxtFile::AllLines.GetAt(0xFA);
    case 0x8C:
        return TxtFile::AllLines.GetAt(0xFB);
    case 0x64:
        return TxtFile::AllLines.GetAt(0xFC);
    case 0x78:
        return TxtFile::AllLines.GetAt(0xFD);
    case 0xB4:
        return TxtFile::AllLines.GetAt(0xFE);
    case 0xA0:
        return TxtFile::AllLines.GetAt(0xFF);
    }
    return nullptr;
}


// 437C25
void VisStartGame::UpdateTipsProgress(uint32_t hotspot)
{
    if (g_settings.TipsMode == 0 || this->tipsPrompt == nullptr) {
        return;
    }

    switch (hotspot) {
    case 0x50:
    case 0x64:
    case 0x78:
    case 0x8C:
        if (this->tipsProgress == 0) {
            this->tipsProgress++;
            CString tips;
            MissionGetTips(9, &tips);
            static_cast<VisTipsDialog*>(this->tipsPrompt)->SetText(tips);
        }
        break;
    case 0x14:
    case 0x28:
    case 0x3C:
        if (this->tipsProgress == 1) {
            this->tipsProgress++;
            CString tips;
            MissionGetTips(10, &tips);
            static_cast<VisTipsDialog*>(this->tipsPrompt)->SetText(tips);
        }
        break;
    }
}


// 43817D
void VisStartGame::Accept()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode == 2) {
        if (this->networkNameLabel->GetText().GetLength() > 0) {
            CSound::Play(this->acceptSound);
            this->MsgProc(0x445, 0, 0);
        }
    } else {
        if (this->nameLabel->GetText().GetLength() > 0) {
            CSound::Play(this->acceptSound);
            this->MsgProc(0x445, 0, 0);
        }
    }
}


// 437571
int32_t VisStartGame::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    if (msg == 0x402) {
        this->VMethod9();
    } else if (msg == 0x45A && this->tipsPrompt != nullptr) {
        this->RemoveChild(this->tipsPrompt);
        delete this->tipsPrompt;
        this->tipsPrompt = nullptr;
    }
    return VisScreen::MsgProc(msg, wparam, lparam);
}


// 4356C2
uint32_t VisStartGame::GetHotspotId(int32_t x, int32_t y)
{
    CPoint pt(x, y);
    if (!this->rect.PtInRect(pt)) {
        return 0xFFFFFFFF;
    }
    CPoint top_left = this->rect.TopLeft();
    pt.x -= top_left.x;
    pt.y -= top_left.y;
    int32_t index = pt.y * 0x280 + pt.x;
    uint8_t* data = static_cast<uint8_t*>(this->hotspotMaskBitmap->GetData());
    return data[index];
}


// 437617
int32_t VisStartGame::OnKeyDown(uint32_t wparam)
{
    if (wparam == VK_RETURN) {
        this->Accept();
        return 1;
    }
    if (wparam == VK_ESCAPE) {
        this->Cancel();
        return 1;
    }
    return VisScreen::OnKeyDown(wparam);
}


// 437664
int32_t VisStartGame::OnMouseMove(uint32_t wparam, CPoint pos)
{
    this->UpdateHotspots(pos.x, pos.y, wparam & 1);
    return CVisualObject::OnMouseMove(wparam, pos);
}


// 4382A9
void VisStartGame::Cancel()
{
    CSound::Play(this->returnSound);
    this->MsgProc(0x446, 0, 0);
}


// 438DC0
void VisStartGame::VMethod8(CRect* rect)
{
}


// 4386B0
VisStartGame::~VisStartGame()
{
    this->FreeBitmaps();
    if (this->tipsPrompt != nullptr) {
        this->RemoveChild(this->tipsPrompt);
        delete this->tipsPrompt;
        this->tipsPrompt = nullptr;
    }
}


// 43305B
VisStartGame::VisStartGame(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : VisScreen(_id, l, t, r, b, nullptr)
{
    this->VMethod26();
}


// 435D26
void VisStartGame::ResetHoverState()
{
    this->field_0x204 = 0;
    this->returnToGameHoverBitmap = nullptr;
    this->acceptHoverBitmap = nullptr;
    this->field_0x1bc = 0;
    this->field_0x1c0 = 0;
}


// 435743
void VisStartGame::ClearDifficultySelectedFlags()
{
    for (int32_t i = 0; i < 3; i++) {
        this->difficultyStateFlags.ElementAt(i) &= ~1u;
    }
}


// 43578C
void VisStartGame::ClearPortraitSelectedFlags()
{
    for (int32_t i = 0; i < 4; i++) {
        this->portraitStateFlags.ElementAt(i) &= ~1u;
    }
}


// 4357D5
void VisStartGame::ClearDifficultyHoverFlags()
{
    for (int32_t i = 0; i < 3; i++) {
        this->difficultyStateFlags.ElementAt(i) &= ~2u;
    }
}


// 43581E
void VisStartGame::ClearPortraitHoverFlags()
{
    for (int32_t i = 0; i < 4; i++) {
        this->portraitStateFlags.ElementAt(i) &= ~2u;
    }
}


// 4380D3
void VisStartGame::FreeSamples()
{
    FUN_00438dd0(&this->difficultyLevel1Sound.sample);
    FUN_00438dd0(&this->difficultyLevel2Sound.sample);
    FUN_00438dd0(&this->difficultyLevel3Sound.sample);
    FUN_00438dd0(&this->portraitSelectSound.sample);
    FUN_00438dd0(&this->acceptSound.sample);
    FUN_00438dd0(&this->labelInputSound1.sample);
    FUN_00438dd0(&this->labelInputSound2.sample);
    FUN_00438dd0(&this->labelInputSound3.sample);
    FUN_00438dd0(&this->returnSound.sample);
}


// 437FF4
void VisStartGame::LoadSamples()
{
    this->FreeSamples();
    FUN_00438e40(&this->difficultyLevel1Sound.sample, "SFX\\ChrGen\\Level1.wav");
    FUN_00438e40(&this->difficultyLevel2Sound.sample, "SFX\\ChrGen\\Level2.wav");
    FUN_00438e40(&this->difficultyLevel3Sound.sample, "SFX\\ChrGen\\Level3.wav");
    FUN_00438e40(&this->portraitSelectSound.sample, "SFX\\ChrGen\\Char.wav");
    FUN_00438e40(&this->acceptSound.sample, "SFX\\ChrGen\\Ok.wav");
    FUN_00438e40(&this->labelInputSound1.sample, "SFX\\Letter1.wav");
    FUN_00438e40(&this->labelInputSound2.sample, "SFX\\Letter2.wav");
    FUN_00438e40(&this->labelInputSound3.sample, "SFX\\Letter3.wav");
    FUN_00438e40(&this->returnSound.sample, "SFX\\ChrGen\\Ok.wav");
}


// 435097
void VisStartGame::FreeBitmaps()
{
    if (this->blindAnimation != nullptr) {
        delete this->blindAnimation;
    }
    this->blindAnimation = nullptr;
    if (this->mainAreaBitmap != nullptr) {
        delete this->mainAreaBitmap;
    }
    this->mainAreaBitmap = nullptr;
    if (this->hotspotMaskBitmap != nullptr) {
        delete this->hotspotMaskBitmap;
    }
    this->hotspotMaskBitmap = nullptr;
    if (this->acceptButtonBitmap != nullptr) {
        delete this->acceptButtonBitmap;
    }
    this->acceptButtonBitmap = nullptr;
    if (this->returnToGameButtonBitmap != nullptr) {
        delete this->returnToGameButtonBitmap;
    }
    this->returnToGameButtonBitmap = nullptr;
    if (this->tableauBitmap != nullptr) {
        delete this->tableauBitmap;
    }
    this->tableauBitmap = nullptr;
    this->returnToGameHoverBitmap = nullptr;
    this->acceptHoverBitmap = nullptr;
    this->field_0x1bc = 0;
    this->field_0x1c0 = 0;

    for (int32_t i = 0; i < this->portraitHoverBitmaps.GetSize(); i++) {
        if (this->portraitHoverBitmaps.GetAt(i) != nullptr) {
            delete this->portraitHoverBitmaps.GetAt(i);
        }
        this->portraitHoverBitmaps.ElementAt(i) = nullptr;
        if (this->portraitSelectedBitmaps.GetAt(i) != nullptr) {
            delete this->portraitSelectedBitmaps.GetAt(i);
        }
        this->portraitSelectedBitmaps.ElementAt(i) = nullptr;
        if (this->portraitSelectedHoverBitmaps.GetAt(i) != nullptr) {
            delete this->portraitSelectedHoverBitmaps.GetAt(i);
        }
        this->portraitSelectedHoverBitmaps.ElementAt(i) = nullptr;
    }

    for (int32_t i = 0; i < this->difficultySelectedBitmaps.GetSize(); i++) {
        if (this->difficultySelectedBitmaps.GetAt(i) != nullptr) {
            delete this->difficultySelectedBitmaps.GetAt(i);
        }
        this->difficultySelectedBitmaps.ElementAt(i) = nullptr;
        if (this->difficultyHoverBitmaps.GetAt(i) != nullptr) {
            delete this->difficultyHoverBitmaps.GetAt(i);
        }
        this->difficultyHoverBitmaps.ElementAt(i) = nullptr;
        if (this->difficultySelectedHoverBitmaps.GetAt(i) != nullptr) {
            delete this->difficultySelectedHoverBitmaps.GetAt(i);
        }
        this->difficultySelectedHoverBitmaps.ElementAt(i) = nullptr;
    }

    for (int32_t i = 0; i < this->leftTorchFrames.GetSize(); i++) {
        if (this->leftTorchFrames.GetAt(i) != nullptr) {
            delete this->leftTorchFrames.GetAt(i);
        }
        this->leftTorchFrames.ElementAt(i) = nullptr;
        if (this->rightTorchFrames.GetAt(i) != nullptr) {
            delete this->rightTorchFrames.GetAt(i);
        }
        this->rightTorchFrames.ElementAt(i) = nullptr;
    }

    this->leftTorchFrames.RemoveAll();
    this->rightTorchFrames.RemoveAll();
}


// 43438B
void VisStartGame::LoadBitmaps()
{
    this->FreeBitmaps();
    this->hotspotMaskBitmap = new CBmp256("graphics\\interface\\chrgen\\PreCreate\\Mask.bmp");
    g_mousept.Update();
    this->mainAreaBitmap = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\MainArea.bmp");
    g_mousept.Update();
    this->returnToGameButtonBitmap = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\cancell.bmp");
    g_mousept.Update();
    this->acceptButtonBitmap = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Okl.bmp");
    g_mousept.Update();
    this->tableauBitmap = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\tablol.bmp");
    g_mousept.Update();
    this->blindAnimation = new CA16("graphics\\interface\\chrgen\\PreCreate\\Blind\\sprites.16a");
    this->blindAnimation->ResetPalette(0x10, 4, 0);
    g_mousept.Update();

    this->portraitHoverBitmaps.ElementAt(0) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h1sel.bmp");
    g_mousept.Update();
    this->portraitHoverBitmaps.ElementAt(3) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h4sel.bmp");
    g_mousept.Update();
    this->portraitHoverBitmaps.ElementAt(1) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h2sel.bmp");
    g_mousept.Update();
    this->portraitHoverBitmaps.ElementAt(2) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h3sel.bmp");
    g_mousept.Update();
    this->portraitSelectedBitmaps.ElementAt(0) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h1on.bmp");
    g_mousept.Update();
    this->portraitSelectedBitmaps.ElementAt(3) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h4on.bmp");
    g_mousept.Update();
    this->portraitSelectedBitmaps.ElementAt(1) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h2on.bmp");
    g_mousept.Update();
    this->portraitSelectedBitmaps.ElementAt(2) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h3on.bmp");
    g_mousept.Update();
    this->portraitSelectedHoverBitmaps.ElementAt(0) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h1sel2.bmp");
    g_mousept.Update();
    this->portraitSelectedHoverBitmaps.ElementAt(3) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h4sel3.bmp");
    g_mousept.Update();
    this->portraitSelectedHoverBitmaps.ElementAt(1) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h2sel1.bmp");
    g_mousept.Update();
    this->portraitSelectedHoverBitmaps.ElementAt(2) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Heroes\\h3sel4.bmp");
    g_mousept.Update();
    this->difficultySelectedBitmaps.ElementAt(0) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Levels\\level0on.bmp");
    g_mousept.Update();
    this->difficultySelectedBitmaps.ElementAt(1) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Levels\\level1on.bmp");
    g_mousept.Update();
    this->difficultySelectedBitmaps.ElementAt(2) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Levels\\level2on.bmp");
    g_mousept.Update();
    this->difficultyHoverBitmaps.ElementAt(0) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Levels\\level0l.bmp");
    g_mousept.Update();
    this->difficultyHoverBitmaps.ElementAt(1) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Levels\\level1l.bmp");
    g_mousept.Update();
    this->difficultyHoverBitmaps.ElementAt(2) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Levels\\level2l.bmp");
    g_mousept.Update();
    this->difficultySelectedHoverBitmaps.ElementAt(0) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Levels\\level0lon.bmp");
    g_mousept.Update();
    this->difficultySelectedHoverBitmaps.ElementAt(1) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Levels\\level1lon.bmp");
    g_mousept.Update();
    this->difficultySelectedHoverBitmaps.ElementAt(2) = new CBmp64("graphics\\interface\\chrgen\\PreCreate\\Levels\\level2lon.bmp");
    g_mousept.Update();

    for (int32_t i = 0; i < 0xF; i++) {
        CString name;
        name.Format("graphics\\interface\\chrgen\\PreCreate\\torch1\\t1%04d.bmp", i);
        this->leftTorchFrames.Add(new CBmp64(name));
        name.Format("graphics\\interface\\chrgen\\PreCreate\\torch2\\t2%04d.bmp", i);
        this->rightTorchFrames.Add(new CBmp64(name));
    }
}


// 4CDA5C
VisTown::VisTown(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr)
{
    this->VMethod26();
}


// 4D6B80, real body 4CDC78
VisTown::~VisTown()
{
    this->VMethod37();
    this->VMethod31();
    if (this->tips != nullptr) {
        this->RemoveChild(this->tips);
        delete this->tips;
        this->tips = nullptr;
    }
}


// 4CDDAD
void VisTown::VMethod26()
{
    for (int32_t i = 0; i < 14; i++) {
        this->sounds[i] = nullptr;
    }
    this->hover_snd_shop = 0;
    this->bmp_bkg = nullptr;
    this->bmp_hover_mask = nullptr;
    this->bmp_bird_overlay = nullptr;
    this->spr_guard = nullptr;
    this->spr_tavern = nullptr;
    this->bmp_cur_sign = nullptr;
    this->bmp_cur_door = nullptr;
    this->bmp_cur_stars = nullptr;
    this->spr_fighter = nullptr;
    this->spr_mage = nullptr;
    this->spr_shop = nullptr;
    this->bmp_cur_flugel = nullptr;
    this->spr_cur_horse = nullptr;
    this->spr_cur_bbird = nullptr;
    this->spr_dervish = nullptr;
    this->bmp_tavern_hover = nullptr;
    this->bmp_trainer_hover = nullptr;
    this->bmp_shop_hover = nullptr;
    this->tips = nullptr;
    this->dialog_active = 0;
    this->AddChild(new VisButton(4, 0, 0, 0, 0, "", g_font1, clrsh_TechBlack, 0x445, 0, nullptr));
}


// 4CE002
void VisTown::VMethod28()
{
    g_mousept.DisableHint();
    this->VMethod36();
    this->VMethod30();
    this->town_anim = 0;
    this->door_open_flag = 0;
    this->guard_sound = 0;
    if (g_settings.TipsMode == 0) {
        if (this->tips != nullptr) {
            this->RemoveChild(this->tips);
            delete this->tips;
            this->tips = nullptr;
        }
    }
    else {
        CString str;
        MissionGetTips(1, &str);
        this->tips = new VisTipsDialog(0x467, 0x148, 0, 0x280, 0xC8, str);
        this->AddChild(this->tips);
    }
    this->last_bird = timeGetTime();
    this->guard_frame = this->spr_guard->GetFrameCount() - 1;
    this->guard_frame_step = 0;
    this->town_anim |= 0x400;
    this->dervish_frame = 0;
    this->bbird_last_tick = timeGetTime();
    this->bbird_delay = GetRandS16(2000) + 2000;
    this->spr_cur_bbird = this->spr_bbird.ElementAt(0);
    this->bbird_frame = -1;
    this->horse_last_tick = timeGetTime();
    this->horse_delay = GetRandS16(2000) + 2000;
    this->spr_cur_horse = this->spr_horse.ElementAt(0);
    this->horse_frame = -1;
    this->hovered_action_mask = -1;
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();
    this->VisScreen::VMethod28();
    this->dialog_active = 1;
    this->VMethod9();
    this->VMethod32();
    g_mousept.EnableHint();
}


// 4CE719
void VisTown::VMethod8(CRect* rect)
{
    (void)rect;
}


// 4CE4E5
int32_t VisTown::OnMouseMove(uint32_t wparam, CPoint pos)
{
    this->VMethod34(pos);
    return 0;
}


// 4CE4AF
int32_t VisTown::OnKeyDown(uint32_t wparam)
{
    if (wparam == 0x0D || wparam == 0x1B) {
        return 1;
    }
    return this->VisScreen::OnKeyDown(wparam);
}


// 4CE3B8
int32_t VisTown::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    MainWindow* mainwnd = (MainWindow*)AfxGetMainWnd();
    if (msg == 0x402) {
        if (mainwnd->dialogsMask == 0) {
            this->VMethod9();
        }
    }
    else if (msg == 0x446) {
        return 1;
    }
    else if (msg == 0x45A) {
        if (this->tips != nullptr) {
            this->RemoveChild(this->tips);
            delete this->tips;
            this->tips = nullptr;
        }
    }
    else if (msg == 0x100 && wparam == 0x0D) {
        return 1;
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 4CE50A
int32_t VisTown::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    switch (this->VMethod33(pos)) {
    case 1:
        this->VMethod39();
        AfxGetMainWnd()->PostMessage(0x42A, 0, 0);
        break;
    case 2:
        // tavern
        this->VMethod39();
        AfxGetMainWnd()->PostMessage(0x42B, 0, 0);
        break;
    case 8:
        if (ScenarioGetVar(0x301) == 0) {
            ShowRoleKeyDialog("plagatguard");
        }
        else {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(0x442, 1, 0);
            AfxGetMainWnd()->PostMessage(0x42D, 0, 0);
        }
        break;
    case 0x10:
        AfxGetMainWnd()->PostMessage(0x41F, 0, 0);
        break;
    }
    return 1;
}


// 4CE310
void VisTown::DoClose(uint32_t code)
{
    this->dialog_active = 0;
    if (this->tips != nullptr) {
        this->RemoveChild(this->tips);
        delete this->tips;
        this->tips = nullptr;
    }
    this->VMethod37();
    this->VMethod31();
    this->VisScreen::DoClose(code);
}


// 4D1723
void VisTown::VMethod30()
{
    this->VMethod31();
    FUN_00438e40(&this->sounds[0], "SFX\\Town\\Crowd.wav");
    FUN_00438e40(&this->sounds[3], "SFX\\Town\\Birds1.wav");
    FUN_00438e40(&this->sounds[4], "SFX\\Town\\Birds2.wav");
    FUN_00438e40(&this->sounds[5], "SFX\\Town\\Flugel.wav");
    FUN_00438e40(&this->sounds[6], "SFX\\Town\\Flag.wav");
    FUN_00438e40(&this->sounds[7], "SFX\\Town\\Point.wav");
    FUN_00438e40(&this->sounds[8], "SFX\\Town\\Shop\\enter.wav");
    FUN_00438e40(&this->sounds[9], "SFX\\Town\\School\\Point.wav");
    FUN_00438e40(&this->sounds[10], "SFX\\Town\\Stars.wav");
    FUN_00438e40(&this->sounds[11], "SFX\\Town\\Horse2.wav");
    FUN_00438e40(&this->sounds[12], "SFX\\Town\\Horse3.wav");
    FUN_00438e40(&this->sounds[13], "SFX\\Town\\Horse1.wav");
}


// 4D184A
void VisTown::VMethod31()
{
    for (int32_t i = 0; i < 14; i++) {
        FUN_00438dd0(&this->sounds[i]);
    }
}


// 4D1602
const char* VisTown::GetHint()
{
    if (this->dialog_active == 0) {
        return nullptr;
    }
    CPoint pt(g_mousept.GetX(), g_mousept.GetY());
    switch (this->VMethod33(pt)) {
    case 1:
        return TxtFile::AllLines[0xE9];
    case 2:
        return TxtFile::AllLines[0xEC];
    case 4:
        return TxtFile::AllLines[0xEA];
    case 8:
        return TxtFile::AllLines[0xED];
    case 0x10:
        return TxtFile::AllLines[0xEB];
    }
    return nullptr;
}


// 4CE64B
int32_t VisTown::FUN_004ce64b()
{
    this->town_anim |= 0x80;
    this->bird_group_index = GetRandS16(3);
    this->active_bird = GetRandS16(3) + 1;
    if (this->active_bird == 1) {
        CSound::Play((CSound&)this->sounds[3]);
    }
    else {
        CSound::Play((CSound&)this->sounds[4]);
    }
    int32_t delay = GetRandS16(2000) + 1000;
    this->last_bird = timeGetTime();
    this->bird_frame0 = 0;
    this->bird_frame1 = 0;
    this->bird_frame2 = 0;
    return delay;
}


// 4CE726
void VisTown::FUN_004ce726()
{
    int32_t done_count = 0;
    if ((this->town_anim & 0x80) != 0) {
        for (int32_t i = 0; i < this->active_bird; i++) {
            CA16* spr = this->spr_birds.ElementAt(this->bird_group_index * 3 + i);
            int32_t frame_count = spr->GetFrameCount();
            if ((&this->bird_frame0)[i] < frame_count) {
                spr->VMethod2(this->rect.left, this->rect.top, (&this->bird_frame0)[i], 0, 0);
            }
            else {
                done_count++;
            }
        }
        this->bmp_bird_overlay->VMethod10(this->rect.left, this->rect.top, 0, 0, this->bmp_bird_overlay->GetWidth(), this->bmp_bird_overlay->GetHeight());
        if (done_count == this->active_bird) {
            this->town_anim &= ~0x80u;
        }
        this->last_bird = timeGetTime();
    }
}


// 4CE886
void VisTown::VMethod7()
{
    static uint8_t anim_init_flags;   // byte_6669BC in asm
    static uint32_t hover_tick;       // dword_66699C in asm
    static uint32_t bird_anim_delay;  // dword_6669C4 in asm

    if (this->dialog_active == 0) {
        return;
    }
    if ((anim_init_flags & 1) == 0) {
        anim_init_flags |= 1;
        hover_tick = timeGetTime();
    }
    if ((anim_init_flags & 2) == 0) {
        anim_init_flags |= 2;
        bird_anim_delay = (uint32_t)(GetRandS16(2000) + 1000);
    }
    uint32_t now = timeGetTime();
    if ((now - hover_tick) > 0x43) {
        CPoint pt(g_mousept.GetX(), g_mousept.GetY());
        this->VMethod34(pt);
        if (GetRandS16(100) > 0x5E) {
            this->town_anim |= 0x40;
        }
        if (GetRandS16(100) > 0x61) {
            this->town_anim |= 0x20;
        }
        this->VMethod38();
        hover_tick = timeGetTime();
    }
    this->VMethod35();
    if ((now - this->last_bird) > bird_anim_delay && (this->town_anim & 0x80) == 0) {
        bird_anim_delay = (uint32_t)this->FUN_004ce64b();
    }
    LockSurface2();
    if (this->bmp_bkg != nullptr) {
        this->bmp_bkg->VMethod2(this->rect.left, this->rect.top, 0, 0, 0);
    }
    this->FUN_004ce726();
    if (this->hovered_action_mask == 1) {
        this->bmp_shop_hover->VMethod2(this->rect.left + 0x108, this->rect.top + 0x108, 0, 0, 0);
    }
    if (this->hovered_action_mask == 2) {
        this->bmp_tavern_hover->VMethod2(this->rect.left + 0x90, this->rect.top + 0x14C, 0, 0, 0);
    }
    if (this->spr_tavern != nullptr) {
        this->spr_tavern->VMethod2(this->rect.left + 0x7C, this->rect.top + 0x138, this->tavern_frame, 0, 0);
    }
    if (this->bmp_cur_sign != nullptr) {
        this->bmp_cur_sign->VMethod2(this->rect.left + 0x168, this->rect.top + 0xE8, 0, 0, 0);
    }
    if (this->bmp_cur_door != nullptr) {
        this->bmp_cur_door->VMethod2(this->rect.left + 0xB4, this->rect.top + 0x94, 0, 0, 0);
    }
    if (this->bmp_cur_stars != nullptr) {
        this->bmp_cur_stars->VMethod2(this->rect.left + 0x154, this->rect.top + 0x120, 0, 0, 0);
    }
    if (this->spr_shop != nullptr) {
        this->spr_shop->VMethod2(this->rect.left + 0x114, this->rect.top + 0x128, this->shop_frame, 0, 0);
    }
    if (this->bmp_cur_flugel != nullptr) {
        this->bmp_cur_flugel->VMethod2(this->rect.left + 0x134, this->rect.top + 0x40, 0, 0, 0);
    }
    if (this->spr_guard != nullptr) {
        this->spr_guard->VMethod2(this->rect.left + 0xB8, this->rect.top + 0x9E, this->guard_frame, 0, 0);
    }
    if (this->spr_cur_horse != nullptr) {
        if (this->horse_anim_index == 0) {
            if (this->horse_frame == 0xE) {
                CSound::Play((CSound&)this->sounds[11]);
            }
        }
        else if (this->horse_anim_index == 1) {
            if (this->horse_frame == 8) {
                CSound::Play((CSound&)this->sounds[11]);
            }
            if (this->horse_frame == 0xE) {
                CSound::Play((CSound&)this->sounds[11]);
            }
        }
        else if (this->horse_anim_index == 2) {
            if (this->horse_frame == 1) {
                CSound::Play((CSound&)this->sounds[13]);
            }
            if (this->horse_frame == 0xE) {
                CSound::Play((CSound&)this->sounds[11]);
            }
        }
        if (this->horse_frame == -1) {
            this->spr_cur_horse->VMethod2(this->rect.left + this->horse_position.x, this->rect.top + this->horse_position.y, 0, 0, 0);
        }
        else {
            this->spr_cur_horse->VMethod2(this->rect.left + this->horse_position.x, this->rect.top + this->horse_position.y, this->horse_frame, 0, 0);
        }
    }
    if (this->spr_cur_bbird != nullptr) {
        if (this->bbird_frame == -1) {
            this->spr_cur_bbird->VMethod2(this->rect.left + this->bbird_position.x, this->rect.top + this->bbird_position.y, 0, 0, 0);
        }
        else {
            this->spr_cur_bbird->VMethod2(this->rect.left + this->bbird_position.x, this->rect.top + this->bbird_position.y, this->bbird_frame, 0, 0);
        }
    }
    if (this->spr_dervish != nullptr) {
        this->spr_dervish->VMethod2(this->rect.left + this->dervish_position.x, this->rect.top + this->dervish_position.y, this->dervish_frame, 0, 0);
    }
    UnlockSurface2();
    this->VisScreen::VMethod7();
}


// 4D1709
void VisTown::VMethod32()
{
    FUN_004a4740(this->sounds);
}


// 4D15E3
void VisTown::VMethod39()
{
    this->MsgProc(0x445, 0, 0);
}


// 4D10F0
int32_t VisTown::VMethod33(CPoint pos)
{
    if (!this->rect.PtInRect(pos) || this->bmp_hover_mask == nullptr) {
        return -1;
    }
    pos -= this->rect.TopLeft();
    int32_t idx = pos.x + pos.y * 0x280;
    uint8_t* data = (uint8_t*)this->bmp_hover_mask->GetData();
    switch (data[idx]) {
    case 0x20:
        return 0x400;
    case 0x40:
        return 0x800;
    case 0x50:
        return 0x1000;
    case 0x60:
        return 0x200;
    case 0x80:
        return 2;
    case 0x90:
        return 1;
    case 0xA0:
        return 8;
    case 0xB0:
        return 0x10;
    case 0xC0:
        return 4;
    }
    return -1;
}


// 4D1429
void VisTown::VMethod34(CPoint pos)
{
    int32_t mask = this->VMethod33(pos);
    this->guard_frame_step = 1;
    this->hovered_action_mask = mask;
    switch (mask + 1) {
    case 0:
        ApplyCursor(g_Cursors[0]);
        this->hover_snd_second = 0;
        this->hover_snd_shop = 0;
        break;
    case 2:
        if (rand() % 100 > 0x5F && (this->town_anim & 1) == 0) {
            this->town_anim |= mask;
        }
        if (this->hover_snd_shop == 0) {
            FUN_00438f20(&this->sounds[9]);
            CSound::Play((CSound&)this->sounds[8]);
            this->hover_snd_shop = 1;
        }
        this->hover_snd_second = 0;
        break;
    case 5:
        break;
    case 9:
        if (ScenarioGetVar(0x301) == 0) {
            this->guard_frame_step = -1;
        }
        else {
            this->guard_frame_step = 1;
        }
        this->hover_snd_second = 0;
        this->hover_snd_shop = 0;
        break;
    default:
        this->town_anim |= mask;
        this->hover_snd_second = 0;
        this->hover_snd_shop = 0;
        break;
    }
}


// 4D12CB
void VisTown::VMethod35()
{
    uint32_t now = timeGetTime();
    if ((now - this->bbird_last_tick) > (uint32_t)this->bbird_delay) {
        this->bbird_delay = GetRandS16(5000) + 2000;
        this->bbird_last_tick = now;
        this->bbird_frame = 0;
        this->spr_cur_bbird = this->spr_bbird.GetAt(GetRandS16(this->spr_bbird.GetSize()));
        this->town_anim |= 0x200;
    }
    if ((now - this->horse_last_tick) > (uint32_t)this->horse_delay) {
        this->horse_delay = GetRandS16(5000) + 2000;
        this->horse_last_tick = now;
        this->horse_frame = 0;
        this->horse_anim_index = GetRandS16(this->spr_horse.GetSize());
        this->spr_cur_horse = this->spr_horse.GetAt(this->horse_anim_index);
        this->town_anim |= 0x100;
    }
}


// 4D06AA
void VisTown::FUN_004d06aa()
{
    this->guard_frame += this->guard_frame_step;
    if (this->guard_frame_step == 1 && this->guard_sound == 0) {
        FUN_00438e40(&this->sounds[2], "SFX\\Town\\Guard2.wav");
        CSound::Play((CSound&)this->sounds[2]);
        this->guard_sound = 1;
    }
    else if (this->guard_frame_step == -1 && this->guard_sound != 0) {
        FUN_00438e40(&this->sounds[2], "SFX\\Town\\Guard1.wav");
        CSound::Play((CSound&)this->sounds[2]);
        this->guard_sound = 0;
    }
    if (this->guard_frame < 0) {
        this->guard_frame_step = 0;
        this->guard_frame = 0;
        FUN_00438dd0(&this->sounds[2]);
    }
    else {
        if (this->spr_guard->GetFrameCount() <= this->guard_frame) {
            this->guard_frame_step = 0;
            this->guard_frame = this->spr_guard->GetFrameCount() - 1;
            FUN_00438dd0(&this->sounds[2]);
        }
    }
}


// 4D07E8
void VisTown::FUN_004d07e8()
{
    if (this->tavern_frame == 0) {
        FUN_00438f20(&this->sounds[8]);
        FUN_00438f20(&this->sounds[9]);
        CSound::Play((CSound&)this->sounds[7]);
    }
    this->tavern_frame++;
    if (this->tavern_frame == this->spr_tavern->GetFrameCount()) {
        this->tavern_frame = 0;
        this->town_anim &= ~2u;
    }
}


// 4D0884
void VisTown::FUN_004d0884()
{
    if (this->sign_frame == 0) {
        CSound::Play((CSound&)this->sounds[6]);
    }
    this->sign_frame++;
    if (this->sign_frame == 10) {
        this->sign_frame = 0;
        this->town_anim &= ~0x40u;
    }
    this->bmp_cur_sign = this->bmp_sign.GetAt(this->sign_frame);
}


// 4D0913
void VisTown::FUN_004d0913()
{
    if (ScenarioGetVar(0x301) == 0) {
        this->door_frame = 8;
        this->bmp_cur_door = this->bmp_door.GetAt(this->door_frame);
        return;
    }
    CPoint pt(g_mousept.GetX(), g_mousept.GetY());
    if (this->VMethod33(pt) != 8) {
        if (this->door_open_flag != 0) {
            FUN_00438e40(&this->sounds[1], "SFX\\Town\\GateDn.wav");
            CSound::Play((CSound&)this->sounds[1]);
        }
        this->door_open_flag = 0;
        this->door_frame++;
        if (this->door_frame > 7) {
            this->door_frame = 8;
            this->town_anim &= ~0x08u;
        }
    }
    else {
        if (this->door_open_flag == 0) {
            FUN_00438e40(&this->sounds[1], "SFX\\Town\\GateUp.wav");
            CSound::Play((CSound&)this->sounds[1]);
        }
        this->door_open_flag = 1;
        this->door_frame--;
        if (this->door_frame < 1) {
            this->door_frame = 0;
            this->town_anim &= ~0x08u;
        }
    }
    this->bmp_cur_door = this->bmp_door.GetAt(this->door_frame);
}


// 4D0AD7
void VisTown::FUN_004d0ad7()
{
    static int32_t stars_repeat; // dword_6669d0 in asm
    if (this->stars_frame == 0) {
        CSound::Play((CSound&)this->sounds[10]);
    }
    this->stars_frame++;
    if (this->stars_frame < 9) {
        this->bmp_cur_stars = this->bmp_stars.GetAt(this->stars_frame);
    }
    else {
        stars_repeat++;
        if (stars_repeat == 10) {
            this->stars_frame = 0;
            stars_repeat = 0;
        }
        this->bmp_cur_stars = nullptr;
        this->town_anim &= ~0x10u;
    }
}


// 4D0B95
void VisTown::FUN_004d0b95()
{
    static int32_t fighter_dir; // dword_6669d4 in asm
    if (this->fighter_frame < 1 && fighter_dir == 0) {
        fighter_dir = (rand() % 100 > 0x5F) ? 1 : 0;
    }
    else if (this->fighter_frame == 10) {
        fighter_dir = (rand() % 100 > 0x5F) ? -1 : 0;
    }
    else if (this->fighter_frame == 0 && fighter_dir == -1) {
        this->fighter_frame = 0;
        fighter_dir = 0;
        this->town_anim &= ~4u;
    }
    this->fighter_frame += fighter_dir;
}


// 4D0C6E
void VisTown::FUN_004d0c6e()
{
    static int32_t mage_dir; // dword_6669d8 in asm
    if (this->mage_frame < 1 && mage_dir == 0) {
        mage_dir = (rand() % 100 > 0x5F) ? 1 : 0;
    }
    else if (this->mage_frame == 10) {
        mage_dir = (rand() % 100 > 0x5F) ? -1 : 0;
    }
    else if (this->mage_frame == 0 && mage_dir == -1) {
        this->mage_frame = 0;
        mage_dir = 0;
        this->town_anim &= ~4u;
    }
    this->mage_frame += mage_dir;
}


// 4D0D47
void VisTown::FUN_004d0d47()
{
    this->shop_frame++;
    if (this->shop_frame == this->spr_shop->GetFrameCount()) {
        this->shop_frame = 0;
        this->town_anim &= ~1u;
    }
}


// 4D0DA2
void VisTown::FUN_004d0da2()
{
    if (this->flugel_frame == 0) {
        CSound::Play((CSound&)this->sounds[5]);
    }
    this->flugel_frame++;
    if (this->flugel_frame == 8) {
        this->flugel_frame = 0;
        this->town_anim &= ~0x20u;
    }
    this->bmp_cur_flugel = this->bmp_flugel.GetAt(this->flugel_frame);
}


// 4D0F66
void VisTown::FUN_004d0f66()
{
    if (this->spr_cur_bbird != nullptr && (this->town_anim & 0x200) != 0 && this->bbird_frame != -1) {
        this->bbird_frame++;
        this->bbird_last_tick = timeGetTime();
        if (this->spr_cur_bbird->GetFrameCount() <= this->bbird_frame) {
            this->bbird_frame = -1;
            this->town_anim &= ~0x200u;
        }
    }
}


// 4D1002
void VisTown::FUN_004d1002()
{
    if (this->spr_cur_horse != nullptr && (this->town_anim & 0x100) != 0 && this->horse_frame != -1) {
        this->horse_frame++;
        this->horse_last_tick = timeGetTime();
        if (this->spr_cur_horse->GetFrameCount() <= this->horse_frame) {
            this->horse_frame = -1;
            this->town_anim &= ~0x100u;
        }
    }
}


// 4D109E
void VisTown::FUN_004d109e()
{
    if (this->spr_dervish == nullptr) {
        this->dervish_frame = -1;
    }
    else {
        this->dervish_frame = (this->dervish_frame + 1) % this->spr_dervish->GetFrameCount();
    }
}


// 4D0E31
void VisTown::VMethod38()
{
    if ((this->town_anim & 1) != 0) {
        this->FUN_004d0d47();
    }
    if ((this->town_anim & 2) != 0) {
        this->FUN_004d07e8();
    }
    this->FUN_004d0913();
    if ((this->town_anim & 0x10) != 0) {
        this->FUN_004d0ad7();
    }
    if ((this->town_anim & 0x40) != 0) {
        this->FUN_004d0884();
    }
    if ((this->town_anim & 0x20) != 0) {
        this->FUN_004d0da2();
    }
    if ((this->town_anim & 0x80) != 0) {
        this->bird_frame0++;
        this->bird_frame1++;
        this->bird_frame2++;
    }
    if ((this->town_anim & 0x400) != 0) {
        this->FUN_004d109e();
    }
    if ((this->town_anim & 0x200) != 0) {
        this->FUN_004d0f66();
    }
    if ((this->town_anim & 0x100) != 0) {
        this->FUN_004d1002();
    }
    this->FUN_004d06aa();
}


// 4CFD0E
void VisTown::VMethod37()
{
    if (this->bmp_bkg != nullptr) {
        delete this->bmp_bkg;
        this->bmp_bkg = nullptr;
    }
    if (this->bmp_hover_mask != nullptr) {
        delete this->bmp_hover_mask;
        this->bmp_hover_mask = nullptr;
    }
    if (this->bmp_bird_overlay != nullptr) {
        delete this->bmp_bird_overlay;
        this->bmp_bird_overlay = nullptr;
    }
    if (this->spr_guard != nullptr) {
        delete this->spr_guard;
        this->spr_guard = nullptr;
    }
    if (this->bmp_tavern_hover != nullptr) {
        delete this->bmp_tavern_hover;
        this->bmp_tavern_hover = nullptr;
    }
    if (this->bmp_trainer_hover != nullptr) {
        delete this->bmp_trainer_hover;
        this->bmp_trainer_hover = nullptr;
    }
    if (this->bmp_shop_hover != nullptr) {
        delete this->bmp_shop_hover;
        this->bmp_shop_hover = nullptr;
    }
    if (this->spr_tavern != nullptr) {
        delete this->spr_tavern;
        this->spr_tavern = nullptr;
    }
    for (int32_t i = 0; i < this->bmp_sign.GetSize(); i++) {
        if (this->bmp_sign.GetAt(i) != nullptr) {
            delete this->bmp_sign.GetAt(i);
        }
    }
    this->bmp_sign.RemoveAll();
    for (int32_t i = 0; i < this->bmp_door.GetSize(); i++) {
        if (this->bmp_door.GetAt(i) != nullptr) {
            delete this->bmp_door.GetAt(i);
        }
    }
    this->bmp_door.RemoveAll();
    for (int32_t i = 0; i < this->bmp_stars.GetSize(); i++) {
        if (this->bmp_stars.GetAt(i) != nullptr) {
            delete this->bmp_stars.GetAt(i);
        }
    }
    this->bmp_stars.RemoveAll();
    if (this->spr_fighter != nullptr) {
        delete this->spr_fighter;
        this->spr_fighter = nullptr;
    }
    if (this->spr_mage != nullptr) {
        delete this->spr_mage;
        this->spr_mage = nullptr;
    }
    if (this->spr_shop != nullptr) {
        delete this->spr_shop;
        this->spr_shop = nullptr;
    }
    for (int32_t i = 0; i < this->bmp_flugel.GetSize(); i++) {
        if (this->bmp_flugel.GetAt(i) != nullptr) {
            delete this->bmp_flugel.GetAt(i);
        }
    }
    this->bmp_flugel.RemoveAll();
    for (int32_t i = 0; i < this->spr_birds.GetSize(); i++) {
        if (this->spr_birds.GetAt(i) != nullptr) {
            delete this->spr_birds.GetAt(i);
        }
        this->spr_birds.ElementAt(i) = nullptr;
    }
    this->spr_birds.RemoveAll();
    this->spr_cur_horse = nullptr;
    for (int32_t i = 0; i < this->spr_horse.GetSize(); i++) {
        if (this->spr_horse.GetAt(i) != nullptr) {
            delete this->spr_horse.GetAt(i);
        }
        this->spr_horse.ElementAt(i) = nullptr;
    }
    this->spr_horse.RemoveAll();
    this->spr_cur_bbird = nullptr;
    for (int32_t i = 0; i < this->spr_bbird.GetSize(); i++) {
        if (this->spr_bbird.GetAt(i) != nullptr) {
            delete this->spr_bbird.GetAt(i);
        }
        this->spr_bbird.ElementAt(i) = nullptr;
    }
    this->spr_bbird.RemoveAll();
    if (this->spr_dervish != nullptr) {
        delete this->spr_dervish;
        this->spr_dervish = nullptr;
    }
    this->spr_tavern = nullptr;
    this->bmp_cur_sign = nullptr;
    this->bmp_cur_door = nullptr;
    this->bmp_cur_stars = nullptr;
    this->spr_fighter = nullptr;
    this->spr_mage = nullptr;
    this->spr_shop = nullptr;
    this->bmp_cur_flugel = nullptr;
}


// 4CEEEF
void VisTown::VMethod36()
{
    // spawn-point tables from the original data (634808/634830/634850)
    static const int32_t horse_spawn_tbl[5][2] = {
        { 0x68, 0x194 }, { 0x68, 0x194 }, { 0x100, 0x158 }, { 0x1C0, 0x190 }, { 0x8C, 0x190 },
    };
    static const int32_t bbird_spawn_tbl[4][2] = {
        { 0xD8, 0x16C }, { 0x134, 0x1A8 }, { 0x180, 0x1A8 }, { 0x244, 0x180 },
    };
    static const int32_t dervish_spawn_tbl[4][2] = {
        { 0xE0, 0x16C }, { 0x144, 0x1A8 }, { 0x188, 0x1A4 }, { 0x250, 0x184 },
    };

    this->VMethod37();
    this->bmp_hover_mask = new CBmp256("graphics\\interface\\town\\townmask.bmp");
    g_mousept.Update();
    this->bmp_bkg = new CBmp64("graphics\\interface\\town\\townmain.bmp");
    g_mousept.Update();
    this->bmp_tavern_hover = new CBmp64("graphics\\interface\\town\\Tavern_l.bmp");
    this->bmp_trainer_hover = new CBmp64("graphics\\interface\\town\\Trener_l.bmp");
    this->bmp_shop_hover = new CBmp64("graphics\\interface\\town\\Shop_l.bmp");
    g_mousept.Update();
    this->spr_tavern = new CA16("graphics\\interface\\townbirds\\tavern\\sprites.16a");
    this->spr_tavern->ResetPalette(0x10, 4, 0);
    for (int32_t i = 0; i < 10; i++) {
        CString name;
        name.Format("graphics\\interface\\town\\sign\\V%.2d.bmp", i);
        this->bmp_sign.Add(new CBmp64(name));
        g_mousept.Update();
    }
    for (int32_t i = 0; i < 9; i++) {
        CString name;
        name.Format("graphics\\interface\\town\\door\\T%.2d.bmp", i);
        this->bmp_door.Add(new CBmp64(name));
        g_mousept.Update();
        name.Format("graphics\\interface\\town\\stars\\S%.2d.bmp", i);
        this->bmp_stars.Add(new CBmp64(name));
        g_mousept.Update();
    }
    this->spr_fighter = new CA16("graphics\\interface\\townbirds\\fighter\\sprites.16a");
    this->spr_fighter->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->spr_mage = new CA16("graphics\\interface\\townbirds\\mage\\sprites.16a");
    this->spr_mage->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    for (int32_t i = 0; i < 0xB; i++) {
        // empty spin loop, present in the original
    }
    this->spr_shop = new CA16("graphics\\interface\\townbirds\\shopie\\sprites.16a");
    this->spr_shop->ResetPalette(0x10, 4, 0);
    for (int32_t i = 0; i < 0x1F; i++) {
        // empty spin loop, present in the original
    }
    for (int32_t i = 0; i < 8; i++) {
        CString name;
        name.Format("graphics\\interface\\town\\fluger\\F%.2d.bmp", i);
        this->bmp_flugel.Add(new CBmp64(name));
        g_mousept.Update();
    }
    this->spr_birds.SetSize(9, -1);
    for (int32_t i = 0; i < 9; i++) {
        CString name;
        name.Format("graphics\\interface\\TownBirds\\Birds%d\\sprites.16a", i + 1);
        CA16* spr = new CA16(name);
        this->spr_birds.ElementAt(i) = spr;
        spr->ResetPalette(0x10, 4, 0);
        g_mousept.Update();
    }
    this->bmp_bird_overlay = new CBmp64("graphics\\interface\\Town\\Town_add.bmp");
    g_mousept.Update();
    this->spr_guard = new CA16("graphics\\interface\\TownBirds\\Guards\\sprites.16a");
    this->spr_guard->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->spr_horse.SetSize(3, -1);
    int32_t horse_group = GetRandS16(5);
    this->horse_position.x = horse_spawn_tbl[horse_group][0];
    this->horse_position.y = horse_spawn_tbl[horse_group][1];
    for (int32_t i = 0; i < this->spr_horse.GetSize(); i++) {
        CString name;
        name.Format("graphics\\interface\\TownBirds\\HORSE%d\\A%d\\sprites.16a", horse_group + 1, i + 1);
        CA16* spr = new CA16(name);
        this->spr_horse.ElementAt(i) = spr;
        spr->ResetPalette(0x10, 4, 0);
        g_mousept.Update();
    }
    this->spr_bbird.SetSize(2, -1);
    int32_t bbird_group = GetRandS16(4);
    this->bbird_position.x = bbird_spawn_tbl[bbird_group][0];
    this->bbird_position.y = bbird_spawn_tbl[bbird_group][1];
    for (int32_t i = 0; i < this->spr_bbird.GetSize(); i++) {
        CString name;
        name.Format("graphics\\interface\\TownBirds\\BABA%d\\A%d\\sprites.16a", bbird_group + 1, i + 1);
        CA16* spr = new CA16(name);
        this->spr_bbird.ElementAt(i) = spr;
        spr->ResetPalette(0x10, 4, 0);
        g_mousept.Update();
    }
    int32_t bbird_group_saved = bbird_group;
    while (bbird_group_saved == bbird_group) {
        bbird_group = GetRandS16(4);
    }
    this->dervish_position.x = dervish_spawn_tbl[bbird_group][0];
    this->dervish_position.y = dervish_spawn_tbl[bbird_group][1];
    CString name;
    name.Format("graphics\\interface\\TownBirds\\DERVISH%d\\sprites.16a", bbird_group + 1);
    this->spr_dervish = new CA16(name);
    this->spr_dervish->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->tavern_frame = -1;
    this->FUN_004d07e8();
    this->sign_frame = -1;
    this->FUN_004d0884();
    this->door_frame = 9;
    this->FUN_004d0913();
    this->stars_frame = -1;
    this->FUN_004d0ad7();
    this->fighter_frame = 0;
    this->FUN_004d0b95();
    this->mage_frame = 0;
    this->FUN_004d0c6e();
    this->shop_frame = -1;
    this->FUN_004d0d47();
    this->flugel_frame = -1;
    this->FUN_004d0da2();
    this->bbird_frame = -1;
    this->horse_frame = -1;
    this->dervish_frame = -1;
}


// 4D423A
VisTownKaarg::VisTownKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisTown(_id, l, t, r, b)
{
    for (int32_t i = 0; i < 3; i++) {
        this->snd_voice[i] = nullptr;
    }
    for (int32_t i = 0; i < 4; i++) {
        this->snd_bird[i] = nullptr;
    }
    this->snd_voice1 = nullptr;
    this->snd_shop_enter = nullptr;
    this->snd_tavern_enter = nullptr;
    this->snd_dervish = nullptr;
    for (int32_t i = 0; i < 6; i++) {
        this->snd_guard[i] = nullptr;
    }
}


// 4D6C90
VisTownKaarg::~VisTownKaarg()
{
}


// 4D54B5
void VisTownKaarg::VMethod32()
{
    FUN_004a4740(this->sounds);
}


// 4D54CF
void VisTownKaarg::VMethod30()
{
    FUN_00438e40(&this->snd_voice[0], "sfx\\town_kaarg\\Kvox2.wav");
    FUN_00438e40(&this->snd_voice[1], "sfx\\town_kaarg\\Kvox3.wav");
    FUN_00438e40(&this->snd_voice[2], "sfx\\town_kaarg\\Kvox4.wav");
    FUN_00438e40(&this->snd_bird[0], "sfx\\town_kaarg\\Kbird1.wav");
    FUN_00438e40(&this->snd_bird[1], "sfx\\town_kaarg\\Kbird2.wav");
    FUN_00438e40(&this->snd_bird[2], "sfx\\town_kaarg\\Kbird3.wav");
    FUN_00438e40(&this->snd_bird[3], "sfx\\town_kaarg\\Kbird4.wav");
    FUN_00438e40(&this->snd_voice1, "sfx\\town_kaarg\\Kvox1.wav");
    FUN_00438e40(&this->snd_shop_enter, "sfx\\town_kaarg\\Kenter2.wav");
    FUN_00438e40(&this->snd_tavern_enter, "sfx\\town_kaarg\\Kenter1.wav");
    FUN_00438e40(&this->snd_dervish, "sfx\\town_kaarg\\Kman1.wav");
    FUN_00438e40(&this->snd_guard[0], "sfx\\town_kaarg\\Ksteps2.wav");
    FUN_00438e40(&this->snd_guard[1], "sfx\\town_kaarg\\Ksteps21.wav");
    FUN_00438e40(&this->snd_guard[2], "sfx\\town_kaarg\\Ksteps1.wav");
    FUN_00438e40(&this->snd_guard[3], "sfx\\town_kaarg\\Ksteps11.wav");
    FUN_00438e40(&this->snd_guard[4], "sfx\\town_kaarg\\Ksteps3.wav");
    FUN_00438e40(&this->snd_guard[5], "sfx\\town_kaarg\\Ksteps31.wav");
    this->tavern_snd_flag = 0;
    this->hover_snd_shop = 0;
    this->exit_snd_flag = 0;
}


// 4D5682
void VisTownKaarg::VMethod31()
{
    for (int32_t i = 0; i < 3; i++) {
        FUN_00438dd0(&this->snd_voice[i]);
    }
    for (int32_t i = 0; i < 4; i++) {
        FUN_00438dd0(&this->snd_bird[i]);
    }
    FUN_00438dd0(&this->snd_voice1);
    FUN_00438dd0(&this->snd_shop_enter);
    FUN_00438dd0(&this->snd_tavern_enter);
    FUN_00438dd0(&this->snd_dervish);
    for (int32_t i = 0; i < 6; i++) {
        FUN_00438dd0(&this->snd_guard[i]);
    }
}


// 4D5C9A
void VisTownKaarg::VMethod34(CPoint pos)
{
    int32_t mask = this->VMethod33(pos);
    this->guard_frame_step = 1;
    this->hovered_action_mask = mask;
    if (mask < 9) {
        if (mask == 8) {
            this->tavern_snd_flag = 0;
            this->hover_snd_shop = 0;
            if (this->exit_snd_flag == 0) {
                FUN_00438f20(&this->snd_shop_enter);
                FUN_00438f20(&this->snd_tavern_enter);
                this->exit_snd_flag = 1;
            }
        }
        else {
            switch (mask + 1) {
            case 0:
                ApplyCursor(g_Cursors[0]);
                this->tavern_snd_flag = 0;
                this->hover_snd_shop = 0;
                this->exit_snd_flag = 0;
                break;
            case 2:
                if (this->hover_snd_shop == 0) {
                    FUN_00438f20(&this->snd_tavern_enter);
                    CSound::Play((CSound&)this->snd_shop_enter);
                    this->hover_snd_shop = 1;
                }
                this->tavern_snd_flag = 0;
                this->exit_snd_flag = 0;
                break;
            case 3:
                if (this->tavern_snd_flag == 0) {
                    FUN_00438f20(&this->snd_shop_enter);
                    CSound::Play((CSound&)this->snd_tavern_enter);
                    this->tavern_snd_flag = 1;
                }
                this->hover_snd_shop = 0;
                this->exit_snd_flag = 0;
                break;
            case 5:
                break;
            default:
                this->town_anim |= mask;
                this->tavern_snd_flag = 0;
                this->hover_snd_shop = 0;
                this->exit_snd_flag = 0;
                break;
            }
        }
    }
    else {
        if (mask != 0x200 && mask != 0x400 && mask != 0x800) {
            this->town_anim |= mask;
            this->tavern_snd_flag = 0;
            this->hover_snd_shop = 0;
            this->exit_snd_flag = 0;
        }
    }
}


// 4D4866
void VisTownKaarg::VMethod36()
{
    this->VMethod37();
    this->bmp_hover_mask = new CBmp256("graphics\\interface\\town_kaarg\\townmask.bmp");
    g_mousept.Update();
    this->bmp_bkg = new CBmp64("graphics\\interface\\town_kaarg\\townmain.bmp");
    g_mousept.Update();
    this->bmp_tavern_hover = new CBmp64("graphics\\interface\\town_kaarg\\hili_tavern.bmp");
    this->bmp_shop_hover = new CBmp64("graphics\\interface\\town_kaarg\\hili_shop.bmp");
    g_mousept.Update();
    for (int32_t i = 0; i < 0x1E; i++) {
        CString name;
        name.Format("graphics\\interface\\town_kaarg\\dervish\\d%04d.bmp", i);
        File2 f;
        if (!f.Open(name, 0, nullptr)) {
            break;
        }
        f.Close();
        this->bmp_dervish.Add(new CBmp64(name));
        g_mousept.Update();
    }
    for (int32_t i = 0; i < 0x37; i++) {
        CString name;
        name.Format("graphics\\interface\\town_kaarg\\guard\\g%04d.bmp", i);
        File2 f;
        if (!f.Open(name, 0, nullptr)) {
            break;
        }
        f.Close();
        this->bmp_guard.Add(new CBmp64(name));
        g_mousept.Update();
    }
    for (int32_t g = 0; g < 2; g++) {
        for (int32_t i = g; i < 0x1F; i++) {
            CString name;
            name.Format("graphics\\interface\\town_kaarg\\girl1\\g%d%03d.bmp", g + 1, i);
            File2 f;
            if (!f.Open(name, 0, nullptr)) {
                break;
            }
            f.Close();
            this->bmp_girl1[g].Add(new CBmp64(name));
            g_mousept.Update();
        }
    }
    for (int32_t g = 0; g < 2; g++) {
        for (int32_t i = g; i < 0x1F; i++) {
            CString name;
            name.Format("graphics\\interface\\town_kaarg\\girl2\\g%d%03d.bmp", g + 1, i);
            File2 f;
            if (!f.Open(name, 0, nullptr)) {
                break;
            }
            f.Close();
            this->bmp_girl2[g].Add(new CBmp64(name));
            g_mousept.Update();
        }
    }
    for (int32_t i = 0; i <= 10; i++) {
        CString name;
        name.Format("graphics\\interface\\town_kaarg\\maingates\\m%04d.bmp", i);
        File2 f;
        if (!f.Open(name, 0, nullptr)) {
            break;
        }
        f.Close();
        this->bmp_gate.Add(new CBmp64(name));
        g_mousept.Update();
    }
    this->tavern_frame = -1;
    this->sign_frame = -1;
    this->door_frame = 0;
    this->stars_frame = -1;
    this->fighter_frame = -1;
    this->mage_frame = 0;
    this->shop_frame = -1;
    this->flugel_frame = -1;
    this->bbird_frame = -1;
    this->girl1_group = -1;
    this->horse_frame = -1;
    this->dervish_frame = -1;
}


// 4D5112
void VisTownKaarg::VMethod37()
{
    if (this->bmp_bkg != nullptr) {
        delete this->bmp_bkg;
        this->bmp_bkg = nullptr;
    }
    if (this->bmp_hover_mask != nullptr) {
        delete this->bmp_hover_mask;
        this->bmp_hover_mask = nullptr;
    }
    if (this->bmp_tavern_hover != nullptr) {
        delete this->bmp_tavern_hover;
        this->bmp_tavern_hover = nullptr;
    }
    if (this->bmp_shop_hover != nullptr) {
        delete this->bmp_shop_hover;
        this->bmp_shop_hover = nullptr;
    }
    for (int32_t i = 0; i < this->bmp_gate.GetSize(); i++) {
        if (this->bmp_gate.GetAt(i) != nullptr) {
            delete this->bmp_gate.GetAt(i);
        }
    }
    this->bmp_gate.RemoveAll();
    for (int32_t i = 0; i < this->bmp_guard.GetSize(); i++) {
        if (this->bmp_guard.GetAt(i) != nullptr) {
            delete this->bmp_guard.GetAt(i);
        }
    }
    this->bmp_guard.RemoveAll();
    for (int32_t g = 0; g < 2; g++) {
        for (int32_t i = 0; i < this->bmp_girl1[g].GetSize(); i++) {
            if (this->bmp_girl1[g].GetAt(i) != nullptr) {
                delete this->bmp_girl1[g].GetAt(i);
            }
        }
        this->bmp_girl1[g].RemoveAll();
        for (int32_t i = 0; i < this->bmp_girl2[g].GetSize(); i++) {
            if (this->bmp_girl2[g].GetAt(i) != nullptr) {
                delete this->bmp_girl2[g].GetAt(i);
            }
        }
        this->bmp_girl2[g].RemoveAll();
    }
    for (int32_t i = 0; i < this->bmp_dervish.GetSize(); i++) {
        if (this->bmp_dervish.GetAt(i) != nullptr) {
            delete this->bmp_dervish.GetAt(i);
        }
    }
    this->bmp_dervish.RemoveAll();
}


// 4D6480
void VisTownKaarg::FUN_004d6480()
{
    CPoint pt(g_mousept.GetX(), g_mousept.GetY());
    if (this->VMethod33(pt) != 8) {
        if (this->door_open_flag != 0) {
            FUN_00438e40(&this->sounds[1], "SFX\\Town_kaarg\\Kdoor2.wav");
            CSound::Play((CSound&)this->sounds[1]);
        }
        this->door_open_flag = 0;
        this->door_frame--;
        if (this->door_frame < 1) {
            this->door_frame = 0;
            this->town_anim &= ~0x08u;
        }
    }
    else {
        if (this->door_open_flag == 0) {
            FUN_00438e40(&this->sounds[1], "SFX\\Town_kaarg\\Kdoor1.wav");
            CSound::Play((CSound&)this->sounds[1]);
        }
        this->door_open_flag = 1;
        this->door_frame++;
        if (this->door_frame > 9) {
            this->door_frame = 10;
            this->town_anim &= ~0x08u;
        }
    }
}


// 4D65DD
void VisTownKaarg::FUN_004d65dd()
{
    this->bbird_frame++;
    if (this->bbird_frame == this->bmp_girl1[this->girl1_group].GetSize()) {
        this->bbird_frame = 0;
        this->girl1_group = -1;
        this->town_anim &= ~0x200u;
    }
}


// 4D6655
void VisTownKaarg::FUN_004d6655()
{
    this->girl2_frame++;
    if (this->girl2_frame == this->bmp_girl2[this->girl2_group].GetSize()) {
        this->girl2_frame = 0;
        this->girl2_group = -1;
        this->town_anim &= ~4u;
    }
}


// 4D66CA
void VisTownKaarg::FUN_004d66ca()
{
    this->fighter_frame++;
    if (this->fighter_frame == this->bmp_guard.GetSize()) {
        this->fighter_frame = 0;
        this->town_anim &= ~0x800u;
    }
}


// 4D6728
void VisTownKaarg::FUN_004d6728()
{
    this->dervish_frame++;
    if (this->dervish_frame == this->bmp_dervish.GetSize()) {
        this->dervish_frame = 0;
        this->town_anim &= ~0x400u;
    }
}


// 4D6786
void VisTownKaarg::VMethod38()
{
    if ((this->town_anim & 0x200) != 0) {
        this->FUN_004d65dd();
    }
    if ((this->town_anim & 4) != 0) {
        this->FUN_004d6655();
    }
    if ((this->town_anim & 0x400) != 0) {
        this->FUN_004d6728();
    }
    if ((this->town_anim & 0x800) != 0) {
        this->FUN_004d66ca();
    }
    this->FUN_004d6480();
}


// 4D5EF0
void VisTownKaarg::VMethod35()
{
    static uint8_t timer_init_flags; // byte_6669AC in asm
    static uint32_t voice_delay;     // dword_666994 in asm
    static uint32_t bird_delay;      // dword_6669B0 in asm

    uint32_t now = timeGetTime();
    if ((now - this->dervish_tick) > (uint32_t)this->dervish_delay) {
        this->dervish_delay = GetRandS16(500) + 0xE74;
        this->dervish_tick = now;
        this->dervish_frame = 0;
        this->town_anim |= 0x400;
    }
    if ((now - this->bbird_last_tick) > (uint32_t)this->bbird_delay) {
        this->bbird_delay = GetRandS16(5000) + 0xDAC;
        this->bbird_last_tick = now;
        this->bbird_frame = 0;
        this->girl1_group = GetRandS16(2);
        this->town_anim |= 0x200;
    }
    if ((now - this->girl2_tick) > (uint32_t)this->girl2_delay) {
        this->girl2_delay = GetRandS16(5000) + 0xDAC;
        this->girl2_tick = now;
        this->girl2_frame = 0;
        this->girl2_group = GetRandS16(2);
        this->town_anim |= 4;
    }
    if ((now - this->guard_tick) > (uint32_t)this->guard_delay) {
        this->guard_delay = GetRandS16(5000) + 0x1D4C;
        this->guard_tick = now;
        this->fighter_frame = 0;
        this->town_anim |= 0x800;
    }
    if ((timer_init_flags & 1) == 0) {
        timer_init_flags |= 1;
        voice_delay = (uint32_t)(GetRandS16(2000) + 2000);
    }
    if ((timer_init_flags & 2) == 0) {
        timer_init_flags |= 2;
        bird_delay = (uint32_t)(GetRandS16(2000) + 2000);
    }
    if ((now - this->last_bird) > voice_delay) {
        this->active_bird = GetRandS16(3) + 1;
        if (this->active_bird == 1) {
            CSound::Play((CSound&)this->snd_voice[0]);
        }
        else if (this->active_bird == 2) {
            CSound::Play((CSound&)this->snd_voice[1]);
        }
        else if (this->active_bird == 3) {
            CSound::Play((CSound&)this->snd_voice[2]);
        }
        voice_delay = (uint32_t)(GetRandS16(2000) + 2000);
        this->last_bird = timeGetTime();
    }
    if ((now - this->bird_snd_tick) > bird_delay) {
        switch (GetRandS16(4)) {
        case 0:
            CSound::Play((CSound&)this->snd_bird[0]);
            break;
        case 1:
            CSound::Play((CSound&)this->snd_bird[1]);
            break;
        case 2:
            CSound::Play((CSound&)this->snd_bird[2]);
            break;
        case 3:
            CSound::Play((CSound&)this->snd_bird[3]);
            break;
        }
        bird_delay = (uint32_t)(GetRandS16(2000) + 2000);
        this->bird_snd_tick = timeGetTime();
    }
    if ((now - this->dervish_snd_tick) > 45000) {
        CSound::Play((CSound&)this->snd_dervish);
        this->dervish_snd_tick = timeGetTime();
    }
    if ((this->town_anim & 0x800) != 0) {
        switch (this->fighter_frame) {
        case 5:
        case 0xD:
        case 0x15:
        case 0x2F:
        case 0x37:
        case 0x3F:
            if (this->guard_snd_flag == 0) {
                if (GetRandS16(4) == 0) {
                    CSound::Play((CSound&)this->snd_guard[1]);
                }
                else {
                    CSound::Play((CSound&)this->snd_guard[0]);
                }
                this->guard_snd_flag = 1;
            }
            break;
        case 9:
        case 0x11:
        case 0x17:
        case 0x33:
        case 0x3B:
            if (this->guard_snd_flag == 0) {
                if (GetRandS16(4) == 0) {
                    CSound::Play((CSound&)this->snd_guard[3]);
                }
                else {
                    CSound::Play((CSound&)this->snd_guard[2]);
                }
                this->guard_snd_flag = 1;
            }
            break;
        case 0x1F:
        case 0x45:
            if (this->guard_snd_flag == 0) {
                if (GetRandS16(4) == 0) {
                    CSound::Play((CSound&)this->snd_guard[5]);
                }
                else {
                    CSound::Play((CSound&)this->snd_guard[4]);
                }
                this->guard_snd_flag = 1;
            }
            break;
        default:
            this->guard_snd_flag = 0;
            break;
        }
    }
}


// 4D6A1C
const char* VisTownKaarg::GetHint()
{
    if (this->dialog_active == 0) {
        return nullptr;
    }
    CPoint pt(g_mousept.GetX(), g_mousept.GetY());
    switch (this->VMethod33(pt)) {
    case 1:
        return TxtFile::AllLines[0xE9];
    case 2:
        return TxtFile::AllLines[0xEC];
    case 4:
        return TxtFile::AllLines[0x168];
    case 8:
        return TxtFile::AllLines[0xED];
    case 0x10:
        return TxtFile::AllLines[0xEB];
    case 0x200:
        return TxtFile::AllLines[0x167];
    case 0x400:
        return TxtFile::AllLines[0x16A];
    case 0x800:
        return TxtFile::AllLines[0x169];
    }
    return nullptr;
}


// 4D6802
int32_t VisTownKaarg::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    int32_t mask = this->VMethod33(pos);
    if (mask < 9) {
        if (mask == 8) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(0x442, 1, 0);
            AfxGetMainWnd()->PostMessage(0x42D, 0, 0);
        }
        else if (mask == 1) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(0x42A, 0, 0);
        }
        else if (mask == 2) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(0x42B, 0, 0);
        }
        else if (mask == 4) {
            CString name;
            name.Format("kaargwoman%d", ScenarioGetVar(0x300));
            ShowRoleKeyDialog(name);
        }
    }
    else if (mask < 0x201) {
        if (mask == 0x200) {
            CString name;
            name.Format("kaargwoman%d", ScenarioGetVar(0x300));
            ShowRoleKeyDialog(name);
        }
        else if (mask == 0x10) {
            AfxGetMainWnd()->PostMessage(0x41F, 0, 0);
        }
    }
    else if (mask == 0x400) {
        CString name;
        name.Format("kaargman%d", ScenarioGetVar(0x300));
        ShowRoleKeyDialog(name);
    }
    else if (mask == 0x800) {
        CString name;
        name.Format("kaargguard%d", ScenarioGetVar(0x300));
        ShowRoleKeyDialog(name);
    }
    return 1;
}


// 4D4592
void VisTownKaarg::VMethod28()
{
    g_mousept.DisableHint();
    this->VMethod36();
    this->VMethod30();
    this->town_anim = 0;
    this->door_open_flag = 0;
    this->guard_sound = 0;
    if (g_settings.TipsMode == 0) {
        if (this->tips != nullptr) {
            this->RemoveChild(this->tips);
            delete this->tips;
            this->tips = nullptr;
        }
    }
    else {
        CString str;
        MissionGetTips(0xC, &str);
        this->tips = new VisTipsDialog(0x467, 0x148, 0, 0x280, 0xC8, str);
        this->AddChild(this->tips);
    }
    this->girl2_frame = -1;
    this->girl2_group = -1;
    this->bbird_last_tick = timeGetTime();
    this->bbird_delay = GetRandS16(2000) + 2000;
    this->girl2_tick = timeGetTime();
    this->girl2_delay = GetRandS16(2000) + 2000;
    this->guard_tick = timeGetTime();
    this->guard_delay = GetRandS16(2000) + 2000;
    this->dervish_tick = timeGetTime();
    this->dervish_delay = GetRandS16(500) + 4000;
    this->hovered_action_mask = -1;
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();
    this->VisScreen::VMethod28();
    this->dialog_active = 1;
    this->VMethod9();
    FUN_004a4740(&this->snd_voice1);
    g_mousept.EnableHint();
}


// 4D57B9
void VisTownKaarg::VMethod7()
{
    static uint8_t anim_init_flags;  // byte_6669C0 in asm
    static uint32_t hover_tick;      // dword_6669B8 in asm
    static uint32_t bird_anim_delay; // dword_6669CC in asm

    if (this->dialog_active == 0) {
        return;
    }
    if ((anim_init_flags & 1) == 0) {
        anim_init_flags |= 1;
        hover_tick = timeGetTime();
    }
    if ((anim_init_flags & 2) == 0) {
        anim_init_flags |= 2;
        bird_anim_delay = (uint32_t)(GetRandS16(2000) + 1000);
    }
    uint32_t now = timeGetTime();
    if ((now - hover_tick) > 100) {
        CPoint pt(g_mousept.GetX(), g_mousept.GetY());
        this->VMethod34(pt);
        this->VMethod38();
        hover_tick = timeGetTime();
    }
    this->VMethod35();
    LockSurface2();
    if (this->bmp_bkg != nullptr) {
        this->bmp_bkg->VMethod2(this->rect.left, this->rect.top, 0, 0, 0);
    }
    if (this->hovered_action_mask == 1) {
        this->bmp_shop_hover->VMethod2(this->rect.left + 0x148, this->rect.top + 0x100, 0, 0, 0);
    }
    if (this->hovered_action_mask == 2) {
        this->bmp_tavern_hover->VMethod2(this->rect.left + 0x1E0, this->rect.top + 0xC4, 0, 0, 0);
    }
    if ((this->town_anim & 0x200) == 0) {
        this->bmp_girl1[0].GetAt(0)->VMethod2(this->rect.left + 0xD8, this->rect.top + 0x11C, 0, 0, 0);
    }
    else {
        this->bmp_girl1[this->girl1_group].GetAt(this->bbird_frame)->VMethod2(this->rect.left + 0xD8, this->rect.top + 0x11C, 0, 0, 0);
    }
    if ((this->town_anim & 4) == 0) {
        this->bmp_girl2[0].GetAt(0)->VMethod2(this->rect.left + 0x104, this->rect.top + 0x11C, 0, 0, 0);
    }
    else {
        this->bmp_girl2[this->girl2_group].GetAt(this->girl2_frame)->VMethod2(this->rect.left + 0x104, this->rect.top + 0x11C, 0, 0, 0);
    }
    if ((this->town_anim & 0x800) == 0) {
        this->bmp_guard.GetAt(0)->VMethod2(this->rect.left + 0xB8, this->rect.top + 0x9C, 0, 0, 0);
    }
    else if (this->fighter_frame < 0xD || this->fighter_frame > 0x25) {
        this->bmp_guard.GetAt(this->fighter_frame)->VMethod2(this->rect.left + 0xB8, this->rect.top + 0x9C, 0, 0, 0);
    }
    else {
        this->bmp_guard.GetAt(this->fighter_frame)->VMethod2(this->rect.left + 0x8C, this->rect.top + 0x98, 0, 0, 0);
    }
    if ((this->town_anim & 0x400) == 0) {
        this->bmp_dervish.GetAt(0)->VMethod2(this->rect.left + 0x1A0, this->rect.top + 0x148, 0, 0, 0);
    }
    else {
        this->bmp_dervish.GetAt(this->dervish_frame)->VMethod2(this->rect.left + 0x1A0, this->rect.top + 0x148, 0, 0, 0);
    }
    this->bmp_gate.GetAt(this->door_frame)->VMethod2(this->rect.left + 0x98, this->rect.top + 0x100, 0, 0, 0);
    UnlockSurface2();
    this->VisScreen::VMethod7();
}


// 4D1AF8
VisTownDruid::VisTownDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisTown(_id, l, t, r, b)
{
    for (int32_t i = 0; i < 4; i++) {
        this->snd_lizard[i] = nullptr;
    }
    for (int32_t i = 0; i < 3; i++) {
        this->snd_bug[i] = nullptr;
    }
    for (int32_t i = 0; i < 3; i++) {
        this->snd_bird[i] = nullptr;
    }
    for (int32_t i = 0; i < 4; i++) {
        this->snd_tree[i] = nullptr;
    }
    this->snd_forest = nullptr;
    this->snd_shop = nullptr;
    this->snd_tavern = nullptr;
    this->snd_shop_enter = nullptr;
    this->snd_tavern_enter = nullptr;
    this->snd_town_exit = nullptr;
    this->snd_wolf = nullptr;
    this->spr_bug = nullptr;
    this->spr_lizard = nullptr;
}


// 4D6BE0
VisTownDruid::~VisTownDruid()
{
}


// 4D2AE3
void VisTownDruid::VMethod32()
{
    FUN_004a4740(&this->snd_forest);
}


// 4D2AFF
void VisTownDruid::VMethod30()
{
    this->VMethod31();
    FUN_00438e40(&this->snd_lizard[0], "sfx\\town_druid\\Dlizard1.wav");
    FUN_00438e40(&this->snd_lizard[1], "sfx\\town_druid\\Dlizard2.wav");
    FUN_00438e40(&this->snd_lizard[2], "sfx\\town_druid\\Dlizard3.wav");
    FUN_00438e40(&this->snd_lizard[3], "sfx\\town_druid\\Dlizard4.wav");
    FUN_00438e40(&this->snd_bug[0], "sfx\\town_druid\\Dbug1.wav");
    FUN_00438e40(&this->snd_bug[1], "sfx\\town_druid\\Dbug2.wav");
    FUN_00438e40(&this->snd_bug[2], "sfx\\town_druid\\Dbug3.wav");
    FUN_00438e40(&this->snd_bird[0], "sfx\\town_druid\\Dbird1.wav");
    FUN_00438e40(&this->snd_bird[1], "sfx\\town_druid\\Dbird2.wav");
    FUN_00438e40(&this->snd_bird[2], "sfx\\town_druid\\Dbird3.wav");
    FUN_00438e40(&this->snd_tree[0], "sfx\\town_druid\\Dtree1.wav");
    FUN_00438e40(&this->snd_tree[1], "sfx\\town_druid\\Dtree2.wav");
    FUN_00438e40(&this->snd_tree[2], "sfx\\town_druid\\Dtree3.wav");
    FUN_00438e40(&this->snd_tree[3], "sfx\\town_druid\\Dtree4.wav");
    FUN_00438e40(&this->snd_forest, "sfx\\town_druid\\Dforest1.wav");
    FUN_00438e40(&this->snd_shop, "sfx\\town_druid\\Ddruid1.wav");
    FUN_00438e40(&this->snd_tavern, "sfx\\town_druid\\Ddruid2.wav");
    FUN_00438e40(&this->snd_shop_enter, "sfx\\town_druid\\Denter2.wav");
    FUN_00438e40(&this->snd_tavern_enter, "sfx\\town_druid\\Denter1.wav");
    FUN_00438e40(&this->snd_town_exit, "sfx\\town_druid\\Dout.wav");
    FUN_00438e40(&this->snd_wolf, "sfx\\town_druid\\Dwolf1.wav");
    this->tavern_hover_snd_flag = 0;
    this->hover_snd_shop = 0;
    this->mission_exit_hover_snd_flag = 0;
}


// 4D2D1B
void VisTownDruid::VMethod31()
{
    for (int32_t i = 0; i < 4; i++) {
        FUN_00438dd0(&this->snd_lizard[i]);
    }
    for (int32_t i = 0; i < 3; i++) {
        FUN_00438dd0(&this->snd_bug[i]);
    }
    for (int32_t i = 0; i < 3; i++) {
        FUN_00438dd0(&this->snd_bird[i]);
    }
    for (int32_t i = 0; i < 4; i++) {
        FUN_00438dd0(&this->snd_tree[i]);
    }
    FUN_00438dd0(&this->snd_forest);
    FUN_00438dd0(&this->snd_shop);
    FUN_00438dd0(&this->snd_tavern);
    FUN_00438dd0(&this->snd_shop_enter);
    FUN_00438dd0(&this->snd_tavern_enter);
    FUN_00438dd0(&this->snd_town_exit);
    FUN_00438dd0(&this->snd_wolf);
}


// lizard animation frame sequences from the original data (634870/634898/6348C8/634930), -1 terminated
static const int32_t lizard_seq0[] = { 0x27, 0x28, 0x29, 0x2A, 0x29, 0x28, 0x27, 0x26, -1 };
static const int32_t lizard_seq1[] = { 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, -1 };
static const int32_t lizard_seq2[] = { 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, -1 };
static const int32_t lizard_seq3[] = { 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, -1 };
static const int32_t* const lizard_seqs[4] = { lizard_seq0, lizard_seq1, lizard_seq2, lizard_seq3 };


// 4D3384
void VisTownDruid::FUN_004d3384()
{
    if (this->tavern_frame == 0) {
        FUN_00438f20(&this->sounds[8]);
        CSound::Play((CSound&)this->sounds[7]);
    }
    if (this->tavern_group >= 0) {
        this->tavern_frame++;
        if (this->tavern_frame == this->tavern_frame_group[this->tavern_group].GetSize()) {
            this->tavern_frame = 0;
            this->tavern_group = -1;
            this->town_anim &= ~2u;
        }
    }
}


// 4D3435
void VisTownDruid::FUN_004d3435()
{
    if (this->shop_group >= 0) {
        this->shop_frame++;
        if (this->shop_frame == this->shop_frame_group[this->shop_group].GetSize()) {
            this->shop_frame = 0;
            this->shop_group = -1;
            this->town_anim &= ~1u;
        }
    }
}


// 4D34B6
void VisTownDruid::FUN_004d34b6()
{
    if (this->bug_variant >= 0) {
        this->bug_frame++;
        if (this->bug_frame == 0x3D) {
            this->bug_frame = 0;
            this->bug_variant = -1;
            this->town_anim &= ~0x80u;
        }
    }
}


// 4D3520
void VisTownDruid::FUN_004d3520()
{
    if (this->lizard_variant >= 0) {
        this->lizard_frame++;
        if (lizard_seqs[this->lizard_variant][this->lizard_frame] == -1) {
            this->lizard_frame = -1;
            this->lizard_variant = -1;
            this->town_anim &= ~0x100u;
        }
    }
}


// 4D2153
void VisTownDruid::VMethod36()
{
    this->VMethod37();
    this->bmp_hover_mask = new CBmp256("graphics\\interface\\town_druid\\townmask.bmp");
    g_mousept.Update();
    this->bmp_bkg = new CBmp64("graphics\\interface\\town_druid\\townmain.bmp");
    g_mousept.Update();
    this->bmp_tavern_hover = new CBmp64("graphics\\interface\\town_druid\\hili_tavern.bmp");
    this->bmp_shop_hover = new CBmp64("graphics\\interface\\town_druid\\hili_shop.bmp");
    g_mousept.Update();
    for (int32_t g = 0; g < 3; g++) {
        for (int32_t i = 1; i < 0x15; i++) {
            CString name;
            name.Format("graphics\\interface\\town_druid\\woman\\a%d%04d.bmp", g + 1, i);
            File2 f;
            if (!f.Open(name, 0, nullptr)) {
                break;
            }
            f.Close();
            this->tavern_frame_group[g].Add(new CBmp64(name));
            g_mousept.Update();
        }
    }
    for (int32_t g = 0; g < 3; g++) {
        for (int32_t i = 1; i < 0x15; i++) {
            CString name;
            name.Format("graphics\\interface\\town_druid\\man\\a%d%04d.bmp", g + 1, i);
            File2 f;
            if (!f.Open(name, 0, nullptr)) {
                break;
            }
            f.Close();
            this->shop_frame_group[g].Add(new CBmp64(name));
            g_mousept.Update();
        }
    }
    this->spr_bug = new CA16("graphics\\interface\\town_druid\\bug\\sprites.16a");
    this->spr_bug->ResetPalette(0x10, 4, 0);
    this->spr_lizard = new CA16("graphics\\interface\\town_druid\\Lizard\\sprites.16a");
    this->spr_lizard->ResetPalette(0x10, 4, 0);
    this->tavern_frame = -1;
    this->tavern_group = -1;
    this->FUN_004d3384();
    this->sign_frame = -1;
    this->door_frame = 9;
    this->stars_frame = -1;
    this->fighter_frame = 0;
    this->mage_frame = 0;
    this->shop_frame = -1;
    this->shop_group = -1;
    this->FUN_004d3435();
    this->flugel_frame = -1;
    this->lizard_variant = -1;
    this->lizard_frame = -1;
    this->bbird_frame = -1;
    this->horse_frame = -1;
    this->dervish_frame = -1;
    this->bug_variant = -1;
}


// 4D27FF
void VisTownDruid::VMethod37()
{
    if (this->bmp_bkg != nullptr) {
        delete this->bmp_bkg;
        this->bmp_bkg = nullptr;
    }
    if (this->bmp_hover_mask != nullptr) {
        delete this->bmp_hover_mask;
        this->bmp_hover_mask = nullptr;
    }
    if (this->bmp_tavern_hover != nullptr) {
        delete this->bmp_tavern_hover;
        this->bmp_tavern_hover = nullptr;
    }
    if (this->bmp_shop_hover != nullptr) {
        delete this->bmp_shop_hover;
        this->bmp_shop_hover = nullptr;
    }
    if (this->spr_bug != nullptr) {
        delete this->spr_bug;
        this->spr_bug = nullptr;
    }
    if (this->spr_lizard != nullptr) {
        delete this->spr_lizard;
        this->spr_lizard = nullptr;
    }
    for (int32_t g = 0; g < 3; g++) {
        for (int32_t i = 0; i < this->shop_frame_group[g].GetSize(); i++) {
            if (this->shop_frame_group[g].GetAt(i) != nullptr) {
                delete this->shop_frame_group[g].GetAt(i);
            }
        }
        this->shop_frame_group[g].RemoveAll();
        for (int32_t i = 0; i < this->tavern_frame_group[g].GetSize(); i++) {
            if (this->tavern_frame_group[g].GetAt(i) != nullptr) {
                delete this->tavern_frame_group[g].GetAt(i);
            }
        }
        this->tavern_frame_group[g].RemoveAll();
    }
}


// 4D3EA7
void VisTownDruid::VMethod38()
{
    if ((this->town_anim & 1) != 0) {
        this->FUN_004d3435();
    }
    if ((this->town_anim & 2) != 0) {
        this->FUN_004d3384();
    }
    if ((this->town_anim & 0x80) != 0) {
        this->FUN_004d34b6();
    }
    if ((this->town_anim & 0x100) != 0) {
        this->FUN_004d3520();
    }
}


// 4D359C
void VisTownDruid::VMethod34(CPoint pos)
{
    int32_t mask = this->VMethod33(pos);
    this->guard_frame_step = 1;
    int32_t old_mask = this->hovered_action_mask;
    this->hovered_action_mask = mask;
    if (mask < 9) {
        if (mask == 8) {
            this->tavern_hover_snd_flag = 0;
            this->hover_snd_shop = 0;
            if (this->mission_exit_hover_snd_flag == 0) {
                FUN_00438f20(&this->snd_shop_enter);
                FUN_00438f20(&this->snd_tavern_enter);
                CSound::Play((CSound&)this->snd_town_exit);
                this->mission_exit_hover_snd_flag = 1;
            }
        }
        else {
            switch (mask + 1) {
            case 0:
                ApplyCursor(g_Cursors[0]);
                this->tavern_hover_snd_flag = 0;
                this->hover_snd_shop = 0;
                this->mission_exit_hover_snd_flag = 0;
                break;
            case 2:
                if (this->shop_group != 2 && old_mask != this->hovered_action_mask) {
                    FUN_00438f20(&this->snd_shop);
                    this->shop_tick = timeGetTime();
                    this->shop_group = 2;
                    this->shop_frame = -1;
                    this->town_anim |= mask;
                    this->FUN_004d3435();
                    CSound::Play((CSound&)this->snd_shop);
                }
                if (this->hover_snd_shop == 0) {
                    FUN_00438f20(&this->snd_tavern_enter);
                    FUN_00438f20(&this->snd_town_exit);
                    CSound::Play((CSound&)this->snd_shop_enter);
                    this->hover_snd_shop = 1;
                }
                this->tavern_hover_snd_flag = 0;
                this->mission_exit_hover_snd_flag = 0;
                break;
            case 3:
                if (this->tavern_group != 2 && old_mask != this->hovered_action_mask) {
                    FUN_00438f20(&this->snd_tavern);
                    this->tavern_tick = timeGetTime();
                    this->tavern_group = 2;
                    this->tavern_frame = -1;
                    this->town_anim |= mask;
                    this->FUN_004d3384();
                    CSound::Play((CSound&)this->snd_tavern);
                }
                if (this->tavern_hover_snd_flag == 0) {
                    FUN_00438f20(&this->snd_shop_enter);
                    FUN_00438f20(&this->snd_town_exit);
                    CSound::Play((CSound&)this->snd_tavern_enter);
                    this->tavern_hover_snd_flag = 1;
                }
                this->hover_snd_shop = 0;
                this->mission_exit_hover_snd_flag = 0;
                break;
            case 5:
                break;
            default:
                this->town_anim |= mask;
                this->tavern_hover_snd_flag = 0;
                this->hover_snd_shop = 0;
                this->mission_exit_hover_snd_flag = 0;
                break;
            }
        }
    }
    else {
        if (mask != 0x200 && mask != 0x1000) {
            this->town_anim |= mask;
            this->tavern_hover_snd_flag = 0;
            this->hover_snd_shop = 0;
            this->mission_exit_hover_snd_flag = 0;
        }
    }
}


// 4D3922
void VisTownDruid::VMethod35()
{
    static uint8_t timer_init_flags; // byte_666998 in asm
    static uint32_t bird_delay;      // dword_6669A4 in asm
    static uint32_t tree_delay;      // dword_6669B4 in asm

    uint32_t now = timeGetTime();
    CPoint pt(g_mousept.GetX(), g_mousept.GetY());
    int32_t mask = this->VMethod33(pt);
    if ((now - this->shop_tick) > (uint32_t)this->shop_delay) {
        this->shop_delay = GetRandS16(5000) + 0xDAC;
        this->shop_tick = now;
        if (this->shop_group == -1) {
            this->shop_frame = 0;
            if (mask == 1) {
                this->shop_group = 2;
            }
            else {
                this->shop_group = GetRandS16(2);
            }
            this->town_anim |= 1;
            FUN_00438f20(&this->snd_shop);
            CSound::Play((CSound&)this->snd_shop);
        }
    }
    if ((now - this->tavern_tick) > (uint32_t)this->tavern_delay) {
        this->tavern_delay = GetRandS16(5000) + 0xDAC;
        this->tavern_tick = now;
        if (this->tavern_group == -1) {
            this->tavern_frame = 0;
            if (mask == 2) {
                this->tavern_group = 2;
            }
            else {
                this->tavern_group = GetRandS16(2);
            }
            this->town_anim |= 2;
            FUN_00438f20(&this->snd_tavern);
            CSound::Play((CSound&)this->snd_tavern);
        }
    }
    if ((timer_init_flags & 1) == 0) {
        timer_init_flags |= 1;
        bird_delay = (uint32_t)(GetRandS16(2000) + 2000);
    }
    if ((timer_init_flags & 2) == 0) {
        timer_init_flags |= 2;
        tree_delay = (uint32_t)(GetRandS16(2000) + 2000);
    }
    if ((now - this->last_bird) > bird_delay) {
        this->active_bird = GetRandS16(3) + 1;
        if (this->active_bird == 1) {
            CSound::Play((CSound&)this->snd_bird[0]);
        }
        else if (this->active_bird == 2) {
            CSound::Play((CSound&)this->snd_bird[1]);
        }
        else if (this->active_bird == 3) {
            CSound::Play((CSound&)this->snd_bird[2]);
        }
        bird_delay = (uint32_t)(GetRandS16(2000) + 2000);
        this->last_bird = timeGetTime();
    }
    if ((now - this->tree_tick) > tree_delay) {
        int32_t tree_index = GetRandS16(4);
        CSound::Play((CSound&)this->snd_tree[tree_index]);
        tree_delay = (uint32_t)(GetRandS16(2000) + 2000);
        this->tree_tick = timeGetTime();
    }
    if ((now - this->wolf_tick) > 60000) {
        CSound::Play((CSound&)this->snd_wolf);
        this->wolf_tick = timeGetTime();
    }
    if ((now - this->bug_tick) > (uint32_t)this->bug_delay) {
        int32_t bug_index = GetRandS16(3) + 1;
        if (bug_index == 1) {
            CSound::Play((CSound&)this->snd_bug[0]);
        }
        else if (bug_index == 2) {
            CSound::Play((CSound&)this->snd_bug[1]);
        }
        else if (bug_index == 3) {
            CSound::Play((CSound&)this->snd_bug[2]);
        }
        this->bug_variant = bug_index - 1;
        this->town_anim |= 0x80;
        this->bug_delay = GetRandS16(10000) + 10000;
        this->bug_tick = timeGetTime();
    }
    if (this->lizard_variant == -1) {
        int32_t roll = GetRandS16(0xF) + 1;
        if (roll == 1) {
            this->lizard_variant = 1;
            CSound::Play((CSound&)this->snd_lizard[1]);
        }
        else if (roll == 2) {
            this->lizard_variant = 2;
            CSound::Play((CSound&)this->snd_lizard[2]);
        }
        else if (roll == 3) {
            this->lizard_variant = 3;
            CSound::Play((CSound&)this->snd_lizard[3]);
        }
        else {
            this->lizard_variant = 0;
            CSound::Play((CSound&)this->snd_lizard[0]);
        }
        this->town_anim |= 0x100;
        this->lizard_frame = 0;
    }
}


// 4D2E99
void VisTownDruid::VMethod7()
{
    static uint8_t anim_init_flags;  // byte_6669A0 in asm
    static uint32_t hover_tick;      // dword_6669A8 in asm
    static uint32_t bird_anim_delay; // dword_6669C8 in asm

    if (this->dialog_active == 0) {
        return;
    }
    if ((anim_init_flags & 1) == 0) {
        anim_init_flags |= 1;
        hover_tick = timeGetTime();
    }
    if ((anim_init_flags & 2) == 0) {
        anim_init_flags |= 2;
        bird_anim_delay = (uint32_t)(GetRandS16(2000) + 1000);
    }
    uint32_t now = timeGetTime();
    if ((now - hover_tick) > 100) {
        CPoint pt(g_mousept.GetX(), g_mousept.GetY());
        this->VMethod34(pt);
        this->VMethod38();
        hover_tick = timeGetTime();
    }
    this->VMethod35();
    LockSurface2();
    if (this->bmp_bkg != nullptr) {
        this->bmp_bkg->VMethod2(this->rect.left, this->rect.top, 0, 0, 0);
    }
    if (this->hovered_action_mask == 1 || this->shop_group == 2) {
        this->bmp_shop_hover->VMethod2(this->rect.left + 0x1A4, this->rect.top + 0xE0, 0, 0, 0);
    }
    if (this->shop_group < 0) {
        if (this->hovered_action_mask == 1) {
            this->shop_frame_group[2].GetAt(0)->VMethod2(this->rect.left + 0x154, this->rect.top + 0xE0, 0, 0, 0);
        }
        else {
            this->shop_frame_group[0].GetAt(0)->VMethod2(this->rect.left + 0x150, this->rect.top + 0xF4, 0, 0, 0);
        }
    }
    else {
        this->shop_frame_group[this->shop_group].GetAt(this->shop_frame)->VMethod2(
            this->rect.left + 0x150 + (this->shop_group / 2) * 4,
            this->rect.top + 0xF4 - (this->shop_group / 2) * 0x14, 0, 0, 0);
    }
    if (this->hovered_action_mask == 2 || this->tavern_group == 2) {
        this->bmp_tavern_hover->VMethod2(this->rect.left, this->rect.top + 0xB8, 0, 0, 0);
    }
    if (this->tavern_group < 0) {
        if (this->hovered_action_mask == 2) {
            this->tavern_frame_group[2].GetAt(0)->VMethod2(this->rect.left + 0x98, this->rect.top + 0xB8, 0, 0, 0);
        }
        else {
            this->tavern_frame_group[0].GetAt(0)->VMethod2(this->rect.left + 0xA4, this->rect.top + 0xC8, 0, 0, 0);
        }
    }
    else {
        this->tavern_frame_group[this->tavern_group].GetAt(this->tavern_frame)->VMethod2(
            this->rect.left + 0xA4 - (this->tavern_group / 2) * 0xC,
            this->rect.top + 0xC8 - (this->tavern_group / 2) * 0x10, 0, 0, 0);
    }
    if (this->lizard_variant < 0 || this->lizard_frame < 0) {
        this->spr_lizard->VMethod2(this->rect.left, this->rect.top + 300, 0, 0, 0);
    }
    else {
        this->spr_lizard->VMethod2(this->rect.left, this->rect.top + 300, lizard_seqs[this->lizard_variant][this->lizard_frame] - 1, 0, 0);
    }
    if (this->bug_variant >= 0 && this->bug_frame >= 0) {
        this->spr_bug->VMethod2(this->rect.left, this->rect.top + 0xD4, this->bug_variant * 0x3D + this->bug_frame, 0, 0);
    }
    UnlockSurface2();
    this->VisScreen::VMethod7();
}


// 4D1E80
void VisTownDruid::VMethod28()
{
    g_mousept.DisableHint();
    this->VMethod36();
    this->VMethod30();
    this->town_anim = 0;
    this->door_open_flag = 0;
    this->guard_sound = 0;
    if (g_settings.TipsMode == 0) {
        if (this->tips != nullptr) {
            this->RemoveChild(this->tips);
            delete this->tips;
            this->tips = nullptr;
        }
    }
    else {
        CString str;
        MissionGetTips(0xB, &str);
        this->tips = new VisTipsDialog(0x467, 0x148, 0, 0x280, 0xC8, str);
        this->AddChild(this->tips);
    }
    this->shop_tick = timeGetTime();
    this->shop_delay = GetRandS16(2000) + 2000;
    this->tavern_tick = timeGetTime();
    this->tavern_delay = GetRandS16(2000) + 2000;
    this->last_bird = timeGetTime();
    this->tree_tick = timeGetTime();
    this->wolf_tick = timeGetTime();
    this->bug_delay = GetRandS16(5000) + 7000;
    this->bug_tick = timeGetTime();
    this->bug_frame = -1;
    this->lizard_frame = -1;
    this->hovered_action_mask = -1;
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();
    this->VisScreen::VMethod28();
    this->dialog_active = 1;
    this->VMethod9();
    this->VMethod32();
    g_mousept.EnableHint();
}


// 4D3F18
int32_t VisTownDruid::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    int32_t mask = this->VMethod33(pos);
    if (mask < 9) {
        if (mask == 8) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(0x442, 1, 0);
            AfxGetMainWnd()->PostMessage(0x42D, 0, 0);
        }
        else if (mask == 1) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(0x42A, 0, 0);
        }
        else if (mask == 2) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(0x42B, 0, 0);
        }
    }
    else if (mask == 0x10) {
        AfxGetMainWnd()->PostMessage(0x41F, 0, 0);
    }
    else if (mask == 0x200) {
        CString name;
        name.Format("druidinnkeeper%d", ScenarioGetVar(0x300));
        ShowRoleKeyDialog(name);
    }
    else if (mask == 0x1000) {
        CString name;
        name.Format("druidshopkeeper%d", ScenarioGetVar(0x300));
        ShowRoleKeyDialog(name);
    }
    return 1;
}


// 446567
void VisNetDlg::CreateSessionList()
{
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
static int __cdecl CompareSessionsByName(const void* elem1, const void* elem2)
{
    if (strcmp((const char*)elem1, netdlg_selected_name) == 0) {
        return -1;
    }
    if (strcmp((const char*)elem2, netdlg_selected_name) == 0) {
        return 1;
    }
    return strcmp((const char*)elem1, (const char*)elem2);
}


// 447C27
void VisNetDlg::CachePlayerRows()
{
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
void VisNetDlg::FillServerList()
{
    VisListBox* server_list = (VisListBox*)this->FindChild(1);
    int32_t count = server_list->GetItemCount();

    CString sel_name;
    if (count > this->selected) {
        sel_name = server_list->GetItem(this->selected);
    }
    else {
        sel_name = "When night are cold and friends are few";
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
    }
    else {
        join_btn->ChangeFlags(1, true);
    }
    join_btn->VMethod9();
    server_list->VMethod9();
}


// 447B3C
void VisNetDlg::UpdatePlayerList()
{
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
static int EnumSessionsPump()
{
    MSG msg;
    if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE) != 0) {
        if (msg.message == WM_QUIT) {
            return 0;
        }
        if (msg.message == 0x446) {
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
void VisNetDlg::RefreshSessions()
{
    uint32_t timeout = 0;
    int32_t provider = g_CLlDriver.GetProvider();
    if (provider == 3) {
        timeout = 0x5DC;
    }
    else if (provider == 2) {
        timeout = 0x12C;
    }
    else if (provider == 1) {
        timeout = 0xEA60;
    }

    g_CLlDriver.EnumSessions(&this->sessions->sessions, &this->sessions->num_sessions, EnumSessionsPump, timeout);
    this->CachePlayerRows();
    PostMessageA(g_MainWndHWND, 0x471, 0, 0);
}


// 447842
static char* ExtractFirstWord(const char* str)
{
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
int32_t VisNetDlg::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    case 0x444:
        if (this->FindChild(0xF)->TestFlags(1) == 0) {
            return 0;
        }
        // fall through
    case 0x44E:
    case 0x44F: {
        this->sessions->character_name = main_wnd->m_GameSession.character_name;
        VisListBox* server_list = (VisListBox*)this->FindChild(1);
        this->sessions->selected_index = server_list->GetSelectedIndex();
        this->DoClose(msg);
        main_wnd->PostMessage(0x44C, (WPARAM)this, 0);

        for (int32_t i = 0; i < this->cached_player_rows.GetSize(); i++) {
            CStringArray* row = this->cached_player_rows.GetAt(i);
            if (row != nullptr) {
                delete row;
            }
        }
        this->cached_player_rows.RemoveAll();
        return 1;
    }
    case 0x446: {
        VisScreen::MsgProc(msg, wparam, lparam);
        main_wnd->dialogsMask = 0;

        int32_t provider = g_CLlDriver.GetProvider();
        if (provider == 1) {
            g_CLlDriver.Close();
            PostMessageA(g_MainWndHWND, 0x455, 0, 0);
        }
        else {
            provider = g_CLlDriver.GetProvider();
            if (provider == 0) {
                g_CLlDriver.Close();
                PostMessageA(g_MainWndHWND, 0x454, 0, 0);
            }
            else {
                provider = g_CLlDriver.GetProvider();
                if (provider == 3) {
                    g_CLlDriver.Close();
                    if (main_wnd->field_0x3e0.field_10 == 0) {
                        PostMessageA(g_MainWndHWND, 0x453, 0, 0);
                    }
                    else {
                        PostMessageA(g_MainWndHWND, 0x489, 0, 0);
                    }
                }
                else {
                    g_CLlDriver.Close();
                    PostMessageA(g_MainWndHWND, 0x451, 0, 0);
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
    case 0x450:
        this->RefreshSessions();
        return 1;
    case 0x46E:
        if (wparam == 1) {
            this->selected = ((int32_t)lparam < 0) ? 0 : (int32_t)lparam;
            this->UpdatePlayerList();
        }
        return 1;
    case 0x471: {
        int32_t provider = g_CLlDriver.GetProvider();
        if (provider == 0 || provider == 1) {
            this->rect = this->net_rect;
            this->sessions->character_name = main_wnd->m_GameSession.character_name;
            this->sessions->selected_index = 0;

            if (this->sessions->num_sessions == 0) {
                this->RemoveAllChilds();

                this->AddChild(new VisLabel(1, 0x28, 0x30, 0xDC, 0x4A, txt_dialogs.GetLine(0x7C), g_font1, clrsh_TechBlack, 0));

                int32_t h = this->rect.Height();
                int32_t w = this->rect.Width();
                this->AddChild(new VisButton(2, w / 2 - 0x30, h / 2 + 0xC, w / 2 + 0x30, h / 2 + 0x24, txt_dialogs.GetLine(1), g_font1, clrsh_TechBlack, 0x446, 0, ""));
            }
            else {
                if (g_CLlDriver.GetProvider() == 1) {
                    char* word = ExtractFirstWord(main_wnd->phone_book.phones.ElementAt(0));
                    if (word != nullptr) {
                        delete[] word;
                    }
                }
                this->DoClose(0x44F);
                main_wnd->PostMessage(0x44C, (WPARAM)this, 0);
                AfxGetMainWnd()->PostMessage(0x44F, 0, 0);
            }
        }
        else {
            this->FillServerList();
            this->UpdatePlayerList();
            this->sessions->field_0x4 = this->selected;
        }
        return 1;
    }
    case 0x47C:
        g_CLlDriver.SetEventNewSession();
        return 1;
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
}


// 446E97
void VisNetDlg::VMethod26()
{
    int32_t provider = g_CLlDriver.GetProvider();
    if (provider == 0 || provider == 1) {
        this->net_rect = this->rect;
        this->rect = CRect(0, 0, 0x17C, 0xC4);
        this->UpdateWinRect();

        if (g_CLlDriver.GetProvider() == 1) {
            this->AddChild(new VisLabel(1, 0x28, 0x30, 0xDC, 0x4A, txt_dialogs.GetLine(0x77), g_font1, p_clrsh_Black, 0));
        }
        else {
            this->AddChild(new VisLabel(1, 0x28, 0x30, 0xDC, 0x4A, txt_dialogs.GetLine(0xA3), g_font1, p_clrsh_Black, 0));
        }

        int32_t h = this->rect.Height();
        int32_t w = this->rect.Width();
        this->AddChild(new VisButton(2, w / 2 - 0x30, h / 2 + 0xC, w / 2 + 0x30, h / 2 + 0x24, txt_dialogs.GetLine(1), g_font1, nullptr, 0x47C, 0, ""));
    }
    else {
        this->CreateSessionList();
    }

    this->selected = 0;
    PostMessageA(g_MainWndHWND, 0x450, 0, 0);
}


// 4478E9
int32_t VisNetDlg::OnKeyDown(uint32_t wparam)
{
    if (wparam == 0x1B) {
        return this->MsgProc(0x47C, 0, 0);
    }
    return VisWindow::OnKeyDown(wparam);
}


// 4464C7
VisNetDlg::VisNetDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, AvailNetSession* _sessions)
: VisWindow(_id, l, t, r, b, nullptr)
{
    this->sessions = _sessions;
    this->sessions->sessions = nullptr;
    this->sessions->num_sessions = 0;
}


// 44FA90
VisNetDlg::~VisNetDlg()
{}


// 4ADAB6
void VisFameDocument::VMethod26()
{
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
void VisFameDocument::VMethod7()
{
    CPoint top_left = this->rect.TopLeft();
    if (this->visible_flag != 0) {
        LockSurface2();
        FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
        if (this->bmp_sheet != nullptr) {
            this->bmp_sheet->VMethod2(top_left.x, top_left.y, 0, 0, 0);
        }
        if (this->bmp_cur_left != nullptr) {
            this->bmp_cur_left->VMethod2(top_left.x + this->rect_prev.left, top_left.y + this->rect_prev.top, 0, 0, 0);
        }
        if (this->bmp_cur_right != nullptr) {
            this->bmp_cur_right->VMethod2(top_left.x + this->rect_next.left, top_left.y + this->rect_next.top, 0, 0, 0);
        }
        if (this->bmp_cur_ok != nullptr) {
            this->bmp_cur_ok->VMethod2(top_left.x + this->rect_ok.left, top_left.y + this->rect_ok.top, 0, 0, 0);
        }
        Fame2& doc = this->fame->m_Documents[this->selected_doc];
        doc.FUN_004abfb1(top_left.x, top_left.y);
        UnlockSurface2();
        this->VisScreen::VMethod7();
    }
}


// 4ADD24
void VisFameDocument::LoadBitmaps()
{
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
void VisFameDocument::FreeBitmaps()
{
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
void VisFameDocument::ClearDocs()
{
    if (this->fame != nullptr) {
        for (int32_t i = 0; i < this->fame->m_Documents.GetSize(); i++) {
            this->fame->m_Documents[i].Clear();
        }
    }
}


// 4AE3C6
void VisFameDocument::LoadDocs()
{
    this->ClearDocs();
    for (int32_t i = 0; i < this->fame->m_Documents.GetSize(); i++) {
        this->fame->m_Documents[i].FUN_004ac0af();
    }
}


// 4AE952
int32_t VisFameDocument::UpdateButtons(CPoint pos, bool is_down)
{
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
void VisFameDocument::UpdateArrowStates()
{
    if (this->selected_doc == 0) {
        if (this->fame->m_Documents[0].field_x38 == 0) {
            this->bmp_cur_left = this->bmp_leftarrow[0];
        }
    }
    int32_t last = this->fame->m_Documents.GetUpperBound();
    if (this->selected_doc == last) {
        Fame2& doc = this->fame->m_Documents[this->selected_doc];
        if (doc.str_arr.GetSize() <= (int32_t)doc.field_x38 + 0x15) {
            this->bmp_cur_right = this->bmp_rightarrow[0];
        }
    }
}


// 4AE638
void VisFameDocument::NextDoc()
{
    Fame2& doc = this->fame->m_Documents[this->selected_doc];
    if (!doc.FUN_004abf35()) {
        this->selected_doc = this->selected_doc + 1;
        int32_t last = this->fame->m_Documents.GetUpperBound();
        if (this->selected_doc >= last) {
            this->selected_doc = last;
        }
    }
    this->UpdateArrowStates();
}


// 4AE6B3
void VisFameDocument::PrevDoc()
{
    Fame2& doc = this->fame->m_Documents[this->selected_doc];
    if (doc.FUN_004abf72() == 0) {
        this->selected_doc = this->selected_doc - 1;
        if (this->selected_doc < 1) {
            this->selected_doc = 0;
        }
    }
    this->UpdateArrowStates();
}


// 4AE57F
int32_t VisFameDocument::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    if (msg == 0x402) {
        this->VMethod9();
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 4AE5BE
int32_t VisFameDocument::OnMouseMove(uint32_t wparam, CPoint pos)
{
    int32_t hit = this->UpdateButtons(pos, (wparam & 1) != 0);
    this->UpdateArrowStates();
    if (hit != -1) {
        this->VMethod9();
        FlushScreen();
    }
    return 0;
}


// 4AE607
int32_t VisFameDocument::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    this->UpdateButtons(pos, (wparam & 1) != 0);
    this->UpdateArrowStates();
    return 1;
}


// 4AE717
int32_t VisFameDocument::OnLButtonUp(uint32_t wparam, CPoint pos)
{
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
void VisFameDocument::VMethod28()
{
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
void VisFameDocument::DoClose(uint32_t code)
{
    this->VMethod9();
    this->visible_flag = 0;
    this->FreeBitmaps();
    this->ClearDocs();
    this->fame = nullptr;
    this->VisScreen::DoClose(code);
}


// 4AE791
int32_t VisFameDocument::OnKeyDown(uint32_t wparam)
{
    (void)wparam;
    return 1;
}


// 4AE926
void VisFameDocument::VMethod8(CRect* rect)
{
    (void)rect;
}


// 4AEDA0
const char* VisFameDocument::GetHint()
{
    return nullptr;
}


// 4AD961
VisFameDocument::VisFameDocument(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr)
{
    this->VMethod26();
}


// 4ADA2A (scalar deleting dtor ??_G at 4AED50)
VisFameDocument::~VisFameDocument()
{
    this->FreeBitmaps();
    this->ClearDocs();
}


// 45D7F9
void VisFameHall::VMethod7()
{
    CPoint top_left = this->rect.TopLeft();
    CString str;
    if (this->visible_flag != 0) {
        LockSurface2();
        this->bmp_bkg->VMethod2(top_left.x, top_left.y, 0, 0, 0);
        if (this->bmp_cur_close != nullptr) {
            this->bmp_cur_close->VMethod2(top_left.x + this->close_rect.left, top_left.y + this->close_rect.top, 0, 0, 0);
        }
        for (int32_t i = 0; i < this->name_rects.GetSize(); i++) {
            str.Format("%d.", i + 1);
            g_font4->DrawTextWithShadow(top_left.x + this->rank_rects[i].left, top_left.y + this->rank_rects[i].top, str, 0, palette_brown_derby->GetPalette(0), 1);
            Fame1& entry = this->fame->m_Entries[i];
            g_font4->DrawTextWithShadow(top_left.x + this->name_rects[i].left, top_left.y + this->name_rects[i].top, entry.str, 0, palette_brown_derby->GetPalette(0), 1);
            str.Format("%d", entry.field_x4);
            FUN_00476987(&str);
            g_font4->DrawTextWithShadow(top_left.x + this->score_rects[i].right, top_left.y + this->score_rects[i].top, str, 1, palette_tawny_port->GetPalette(0), 1);
        }
        UnlockSurface2();
    }
    this->VisScreen::VMethod7();
}


// 45CE6E
void VisFameHall::VMethod26()
{
    this->bmp_bkg = nullptr;
    this->bmp_close_off = nullptr;
    this->bmp_close_on = nullptr;
    this->snd_close = nullptr;
    this->close_rect = CRect(CPoint(0x230, 0x1A0), CSize(0x2C, 0x20));
    this->AddChild(new VisButton(4, 0, 0, 0, 0, "", g_font1, clrsh_TechBlack, 0x445, 0, nullptr));
    this->visible_flag = 0;
}


// 45D5F0
int32_t VisFameHall::OnMouseMove(uint32_t wparam, CPoint pos)
{
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
int32_t VisFameHall::OnLButtonDown(uint32_t wparam, CPoint pos)
{
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
int32_t VisFameHall::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    CPoint top_left = this->rect.TopLeft();
    if ((this->close_rect + top_left).PtInRect(pos)) {
        this->MsgProc(0x445, 0, 0);
    }
    return this->CVisualObject::OnLButtonUp(wparam, pos);
}


// 45D222
void VisFameHall::VMethod28()
{
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
int32_t VisFameHall::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    if (msg == 0x402) {
        this->CVisualObject::VMethod9();
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 45D2D9
void VisFameHall::DoClose(uint32_t code)
{
    this->visible_flag = 0;
    this->FreeBitmaps();
    this->FreeSounds();
    this->fame = nullptr;
    this->name_rects.RemoveAll();
    this->VisScreen::DoClose(code);
    g_mousept.EnableHint();
}


// 45D5BE
int32_t VisFameHall::OnKeyDown(uint32_t wparam)
{
    return this->VisScreen::OnKeyDown(wparam);
}


// 45D5D7
int32_t VisFameHall::OnChar(uint32_t wparam)
{
    return this->CVisualObject::OnChar(wparam);
}


// 45DB50
const char* VisFameHall::GetHint()
{
    return nullptr;
}


// 45DA6A
void VisFameHall::VMethod8(CRect* rect)
{
    (void)rect;
}


// 45CFAE
void VisFameHall::UpdateRects()
{
    for (int32_t i = 0; i < this->name_rects.GetSize(); i++) {
        int32_t font_h = g_font4->GetHeight();
        int32_t y = (int32_t)(font_h * (i * 1.5)) + 0x91;
        this->rank_rects[i] = CRect(CPoint(0x91, y), CSize(0x19, font_h));
        this->name_rects[i] = CRect(CPoint(0xAF, y), CSize(200, font_h));
        this->score_rects[i] = CRect(CPoint(0x181, y), CSize(100, font_h));
    }
}


// 45D32E
void VisFameHall::LoadBitmaps()
{
    this->FreeBitmaps();
    this->bmp_bkg = new CBmp64("main\\graphics\\famehall\\hall.bmp");
    this->bmp_close_off = new CBmp64("graphics\\interface\\Docs\\OK\\Ok_l_off.bmp");
    this->bmp_close_on = new CBmp64("graphics\\interface\\Docs\\OK\\Ok_l_on.bmp");
    this->bmp_cur_close = nullptr;
}


// 45D44A
void VisFameHall::FreeBitmaps()
{
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
void VisFameHall::LoadSounds()
{
    FUN_00438e40(&this->snd_close, "SFX\\ChrGen\\Ok.wav");
}


// 45D563
void VisFameHall::FreeSounds()
{
    FUN_00438dd0(&this->snd_close);
}


// 45CD35
VisFameHall::VisFameHall(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr)
{
    this->VMethod26();
}


// 45CDE2 (complete dtor ??1; deleting dtor ??_G at 45DA90)
VisFameHall::~VisFameHall()
{
    this->FreeBitmaps();
    this->FreeSounds();
}


// 4B0C84
int32_t VisMiniMap::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    (void)wparam;
    (void)pos;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        g_Cursors[0]->Use();
        main_wnd->ResetItemCursor();
    }
    return 1;
}


// 4B0C2F
int32_t VisMiniMap::OnMouseMove(uint32_t wparam, CPoint pos)
{
    if ((wparam & 1) != 0) {
        return this->OnLButtonDown(wparam, pos);
    }
    if ((wparam & 2) != 0) {
        return this->OnRButtonDown(wparam, pos);
    }
    return 0;
}


// 4B042E
void VisMiniMap::UpdateCursor()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CSprite256* cur_sprite = g_mousept.GetCursorSprite();
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    CCursor* new_cursor = nullptr;
    CPoint mpt(g_mousept.GetX(), g_mousept.GetY());
    if (rc.PtInRect(mpt)) {
        if (g_mousept.GetX() < g_ScreenSize.right - 2 || main_wnd->dialogsMask != 1) {
            if (g_mousept.GetY() == 0 && main_wnd->dialogsMask == 1) {
                new_cursor = g_Cursors[9];
            } else {
                BigStruct2* mc = this->map_context;
                Scenario* scen = mc->field_0x80;
                CRect rc2;
                this->ClientRectToScreen(&rc2, this->rect);
                int32_t z = this->zoom;
                int32_t mode = 1;
                if (z < 0) {
                    z = 0;
                    mode = 2;
                }
                int32_t step = 1 << z;
                int32_t ox, oy;
                if (mode == 1) {
                    ox = (scen->GetWidth() - 0x10 << z) >> 1;
                    oy = (scen->GetHeight() - 0x10 << z) >> 1;
                } else {
                    ox = scen->GetWidth() - 0x10 >> 2;
                    oy = scen->GetHeight() - 0x10 >> 2;
                }
                oy = 0x52 - oy;
                ox = 0x48 - ox;
                CPoint pt2(g_mousept.GetX(), g_mousept.GetY());
                if (pt2.x - ox < 0 || pt2.y - oy < 0
                    || mc->field_0x84 - 0x10 < (pt2.x - ox) >> z
                    || mc->field_0x88 - 0x10 < (pt2.y - oy) >> z) {
                    new_cursor = g_Cursors[0x11];
                } else if (mc->field_0x140 == 0) {
                    new_cursor = g_Cursors[0x11];
                } else {
                    new_cursor = g_Cursors[0x12];
                    switch (mc->field_0x9b4) {
                    case 1:
                    case 6:
                        new_cursor = g_Cursors[0x13];
                        break;
                    case 2:
                        break;
                    case 4:
                        new_cursor = g_Cursors[0x14];
                        break;
                    case 5:
                        new_cursor = g_Cursors[0x16];
                        break;
                    case 8:
                        new_cursor = g_Cursors[0x15];
                        break;
                    }
                    if ((mc->field_0x144 & 0x24) != 0) {
                        new_cursor = g_Cursors[0x11];
                    }
                }
            }
        } else {
            if (g_mousept.GetY() == 0) {
                new_cursor = g_Cursors[0xF];
            } else if (g_mousept.GetY() < g_ScreenSize.bottom - 2) {
                new_cursor = g_Cursors[0xC];
            } else {
                new_cursor = g_Cursors[0x10];
            }
        }
        if (main_wnd->field_0x408 != nullptr) {
            new_cursor = main_wnd->item_cursor;
        }
        if (new_cursor != nullptr && cur_sprite != new_cursor->GetSprite()) {
            new_cursor->Use();
        }
    }
}


// 4AF006
int32_t VisMiniMap::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    int32_t res = this->CVisualObject::MsgProc(msg, wparam, lparam);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    if (res == 0) {
        switch (msg) {
        case 0x402:
            if (main_wnd->dialogsMask == 1) {
                this->VMethod9();
                this->UpdateCursor();
            }
            break;
        case 0x403:
            this->map_context = (BigStruct2*)wparam;
            break;
        case 0x404:
            this->RebuildMap();
            res = 1;
            break;
        case 0x408:
            this->viewer = -100;
            break;
        }
    }
    return res;
}


// 4B0AAD
int32_t VisMiniMap::OnRButtonDown(uint32_t wparam, CPoint pos)
{
    BigStruct2* mc = this->map_context;
    (void)AfxGetMainWnd();
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    int32_t z = this->zoom;
    int32_t mode = 1;
    if (z < 0) {
        z = 0;
        mode = 2;
    }
    Scenario* scen = mc->field_0x80;
    int32_t x, y;
    if (mode == 1) {
        int32_t ox = 0x48 - ((scen->GetWidth() - 0x10 << z) >> 1);
        int32_t oy = 0x52 - ((scen->GetHeight() - 0x10 << z) >> 1);
        x = ((pos.x - ox) - rc.left >> z) - mc->field_0x64 / 2;
        y = ((pos.y - oy) - rc.top >> z) - mc->field_0x68 / 2;
    } else {
        int32_t ox = 0x48 - (scen->GetWidth() - 0x10 >> 2);
        int32_t oy = 0x52 - (scen->GetHeight() - 0x10 >> 2);
        x = ((pos.x - ox) - rc.left) * 2 - mc->field_0x64 / 2;
        y = ((pos.y - oy) - rc.top) * 2 - mc->field_0x68 / 2;
    }
    x = x + 8;
    y = y + 8;
    mc->MsgProc(0x406, x, y);
    return 0;
}


// 4B0788
int32_t VisMiniMap::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    BigStruct2* mc = this->map_context;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    int32_t z = this->zoom;
    int32_t mode = 1;
    if (z < 0) {
        z = 0;
        mode = 2;
    }
    Scenario* scen = mc->field_0x80;
    int32_t ox, oy, x, y;
    if (mode == 1) {
        ox = 0x48 - ((scen->GetWidth() - 0x10 << z) >> 1);
        oy = 0x52 - ((scen->GetHeight() - 0x10 << z) >> 1);
        x = ((pos.x - ox) - rc.left >> z) - mc->field_0x64 / 2;
        y = ((pos.y - oy) - rc.top >> z) - mc->field_0x68 / 2;
    } else {
        ox = 0x48 - (scen->GetWidth() - 0x10 >> 2);
        oy = 0x52 - (scen->GetHeight() - 0x10 >> 2);
        x = ((pos.x - ox) - rc.left) * 2 - mc->field_0x64 / 2;
        y = ((pos.y - oy) - rc.top) * 2 - mc->field_0x68 / 2;
    }
    x = x + 8;
    y = y + 8;
    CSprite256* cur_sprite = g_mousept.GetCursorSprite();
    if (cur_sprite == g_Cursors[0x11]->GetSprite()) {
        mc->MsgProc(0x406, x, y);
    } else {
        if (mode == 1) {
            x = ((pos.x - ox) - rc.left) >> z;
            y = ((pos.y - oy) - rc.top) >> z;
        } else {
            x = ((pos.x - ox) - rc.left) * 2;
            y = ((pos.y - oy) - rc.top) * 2;
        }
        x = x + 8;
        y = y + 8;
        if (cur_sprite == g_Cursors[0x12]->GetSprite()) {
            mc->sub_419154((uint16_t)x, (uint16_t)y);
        } else if (cur_sprite == g_Cursors[0x13]->GetSprite()) {
            uint32_t id = mc->sub_40C0A8(x, y);
            if ((id & 0xffff) == 0) {
                mc->sub_41930D((uint16_t)x, (uint16_t)y);
            } else {
                mc->sub_419246((uint16_t)id);
            }
        } else if (cur_sprite == g_Cursors[0x14]->GetSprite()) {
            uint32_t id = mc->sub_40C0A8(x, y);
            if ((id & 0xffff) != 0) {
                mc->sub_41955C((uint16_t)id);
            }
        } else if (cur_sprite == g_Cursors[0x16]->GetSprite()) {
            mc->sub_40C0A8(x, y);
        } else if (cur_sprite == g_Cursors[0x15]->GetSprite()) {
            mc->sub_41965D((uint16_t)x, (uint16_t)y);
        }
    }
    mc->field_0x9b4 = 0;
    main_wnd->vis_ordertoolbar->MsgProc(0x40B, 0, 0);
    return 0;
}


// 4AF0DC
void VisMiniMap::RebuildMap()
{
    Scenario* scen = this->map_context->field_0x80;
    uint16_t mask = (uint16_t)(((0x3F >> (8 - g_RBits & 0x1F)) << (g_RBitShift & 0x1F))
        | ((0x3F >> (8 - g_GBits & 0x1F)) << (g_GBitShift & 0x1F))
        | ((0x3F >> (8 - g_BBits & 0x1F)) << (g_BBitShift & 0x1F)));
    if (this->bitmap1 != nullptr) {
        delete this->bitmap1;
    }
    if (this->bitmap2 != nullptr) {
        delete this->bitmap2;
    }
    CBmp64* full = new CBmp64(scen->GetWidth() - 0x10, scen->GetHeight() - 0x10);
    uint16_t* full_data = (uint16_t*)full->GetData();
    uint16_t* landscape = scen->GetLandscape();
    uint16_t* minimap_data = (uint16_t*)g_bmp_minimapdata->GetData();
    for (int32_t y = 8; y < scen->GetHeight() - 8; y++) {
        for (int32_t x = 8; x < scen->GetWidth() - 8; x++) {
            uint16_t v = landscape[x + y * scen->GetWidth()];
            int32_t mm_w = g_bmp_minimapdata->GetWidth(0);
            int32_t mm_h = g_bmp_minimapdata->GetHeight(0);
            int32_t full_w = full->GetWidth(0);
            full_data[(x - 8) + (y - 8) * full_w] =
                minimap_data[((v & 0x1FFF) >> 4) + mm_w * ((mm_h - 1) - (v & 0xF))];
        }
    }
    if (scen->GetHeight() < scen->GetWidth()) {
        if (scen->GetWidth() < 0x91) {
            if (scen->GetWidth() < 0x51) {
                if (scen->GetWidth() < 0x31) {
                    this->zoom = 2;
                } else {
                    this->zoom = 1;
                }
            } else {
                this->zoom = 0;
            }
        } else {
            this->zoom = -1;
        }
    } else {
        if (scen->GetHeight() < 0x91) {
            if (scen->GetHeight() < 0x51) {
                if (scen->GetHeight() < 0x31) {
                    this->zoom = 2;
                } else {
                    this->zoom = 1;
                }
            } else {
                this->zoom = 0;
            }
        } else {
            this->zoom = -1;
        }
    }
    int32_t w, h;
    if (this->zoom < 0) {
        w = (scen->GetWidth() - 0x10) >> 1;
        h = (scen->GetHeight() - 0x10) >> 1;
    } else {
        w = (scen->GetWidth() - 0x10) << this->zoom;
        h = (scen->GetHeight() - 0x10) << this->zoom;
    }
    this->bitmap1 = new CBmp64(w, h);
    this->bitmap2 = new CBmp64(w, h);
    uint16_t* b1data = (uint16_t*)this->bitmap1->GetData();
    if (this->zoom < 0) {
        for (int32_t y = 0; y < h; y++) {
            const uint16_t* prow = &full_data[(2 * (h - 1 - y)) * w];
            for (int32_t x = 0; x < w; x++) {
                b1data[x + y * w] = (uint16_t)(((prow[x] >> 2) & mask)
                    + ((prow[x + 1] >> 2) & mask)
                    + ((prow[w + x] >> 2) & mask)
                    + ((prow[w + x + 1] >> 2) & mask));
            }
        }
    } else {
        for (int32_t y = 0; y < h; y++) {
            for (int32_t x = 0; x < w; x++) {
                b1data[x + y * w] = full_data[(x >> this->zoom)
                    + ((full->GetHeight(0) - 1) - (y >> this->zoom)) * full->GetWidth(0)];
            }
        }
    }
    int32_t z = this->zoom;
    int32_t step_dir = 1;
    if (z < 0) {
        z = 0;
        step_dir = 2;
    }
    int32_t step = 1 << z;
    uint8_t* heights = scen->FUN_0041eee0();
    int32_t dst_off = 0;
    for (int32_t x = 8; x < scen->GetWidth() - 8; x += step_dir) {
        int32_t dst_y = 0;
        for (int32_t y = 8; y < scen->GetHeight() - 8; y += step_dir) {
            int32_t land_idx = x + y * scen->GetWidth();
            int32_t b1h = this->bitmap1->GetHeight(0);
            int32_t b1w = this->bitmap1->GetWidth(0);
            int32_t b1_idx = dst_off + ((b1h - 1) - dst_y) * b1w;
            uint8_t h00 = heights[land_idx];
            uint8_t h01 = heights[land_idx + 1];
            uint8_t h10 = heights[land_idx + scen->GetWidth()];
            uint8_t h11 = heights[land_idx + scen->GetWidth() + 1];
            uint16_t shade = 0x60 - (uint16_t)(((uint32_t)h00 + h01 + h10 + h11) >> 2);
            uint16_t px = b1data[b1_idx];
            uint32_t r = (((uint32_t)(uint16_t)((px >> (g_RBitShift & 0x1F)) & ((1 << g_RBits) - 1)) * shade) >> 5) << (8 - g_RBits & 0x1F);
            uint32_t rc = r & 0xFFFF;
            if (rc > 0xFF) {
                rc = 0xFF;
            }
            uint32_t g = (((uint32_t)(uint16_t)((px >> (g_GBitShift & 0x1F)) & ((1 << g_GBits) - 1)) * shade) >> 5) << (8 - g_GBits & 0x1F);
            uint32_t gc = g & 0xFFFF;
            if (gc > 0xFF) {
                gc = 0xFF;
            }
            uint32_t b = (((uint32_t)(uint16_t)((px >> (g_BBitShift & 0x1F)) & ((1 << g_BBits) - 1)) * shade) >> 5) << (8 - g_BBits & 0x1F);
            uint32_t bc = b & 0xFFFF;
            if (bc > 0xFF) {
                bc = 0xFF;
            }
            for (int32_t j = 0; j < step; j++) {
                for (int32_t i = 0; i < step; i++) {
                    b1data[b1_idx + i - j * this->bitmap1->GetWidth(0)] =
                        (uint16_t)(((rc >> (8 - g_RBits & 0x1F)) << (g_RBitShift & 0x1F))
                            | ((gc >> (8 - g_GBits & 0x1F)) << (g_GBitShift & 0x1F))
                            | ((bc >> (8 - g_BBits & 0x1F)) << (g_BBitShift & 0x1F)));
                }
            }
            dst_y += step;
        }
        dst_off += step;
    }
    delete full;
}


// 4AFB8E
void VisMiniMap::VMethod7()
{
    BigStruct2* mc = this->map_context;
    if (mc == nullptr) {
        return;
    }
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if ((main_wnd->dialogsMask & 2) != 0 || (main_wnd->dialogsMask & 4) != 0) {
        return;
    }
    if (abs((int)(this->viewer - mc->field_0xa88)) <= 10) {
        return;
    }
    this->viewer = mc->field_0xa88;
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    Scenario* scen = mc->field_0x80;
    if (scen == nullptr) {
        return;
    }
    LockSurface2();
    int32_t z = this->zoom;
    int32_t mode = 1;
    if (z < 0) {
        z = 0;
        mode = 2;
    }
    int32_t step = 1 << z;
    uint16_t half_mask = (uint16_t)(((0x7F >> (8 - g_RBits & 0x1F)) << (g_RBitShift & 0x1F))
        | ((0x7F >> (8 - g_GBits & 0x1F)) << (g_GBitShift & 0x1F))
        | ((0x7F >> (8 - g_BBits & 0x1F)) << (g_BBitShift & 0x1F)));
    int32_t ox, oy;
    if (mode == 1) {
        ox = ((scen->GetWidth() - 0x10) << z) >> 1;
        oy = ((scen->GetHeight() - 0x10) << z) >> 1;
    } else {
        ox = (scen->GetWidth() - 0x10) >> 2;
        oy = (scen->GetHeight() - 0x10) >> 2;
    }
    oy = 0x52 - oy;
    ox = 0x48 - ox;
    g_bmp_crystalr->VMethod2(rc.left, rc.top, 0, 0, 0);
    if (mc->field_0xdc == 0) {
        this->bitmap2->VMethod2(rc.left + ox, rc.top + oy, 0, 0, 0);
    } else {
        this->bitmap1->VMethod2(rc.left + ox, rc.top + oy, 0, 0, 0);
    }
    if (mc->field_0xdc != 0) {
        uint16_t* landscape = scen->GetLandscape();
        uint16_t* cryst_data = (uint16_t*)g_bmp_crystalr->GetData();
        int32_t cryst_w = g_bmp_crystalr->GetWidth(0);
        uint16_t* mm2_data = (uint16_t*)this->bitmap2->GetData();
        int32_t total;
        if (mode == 1) {
            total = ((scen->GetWidth() - 0x10) * (scen->GetHeight() - 0x10)) << ((this->zoom << 1) & 0x1F);
        } else {
            total = (scen->GetHeight() - 0x10) * (scen->GetWidth() - 0x10);
            total = (total + (total >> 31 & 3)) >> 2;
        }
        uint16_t* mm2_tail = mm2_data + total;
        int32_t dst_pix = rc.top * g_selDrawBitmap.dwWidth + rc.left;
        int32_t cryst_h = g_bmp_crystalr->GetHeight(0);
        int32_t src_row = ((cryst_h - 1) - oy) * cryst_w;
        int32_t dst_row = oy * g_selDrawBitmap.dwWidth;
        int32_t sy = 8;
        while (sy < scen->GetHeight() - 8) {
            int32_t cx = ox;
            int32_t sx = 8;
            while (sx < scen->GetWidth() - 8) {
                int32_t lidx = sx + sy * scen->GetWidth();
                uint16_t flags = (landscape[lidx] & 0xC000)
                    | (landscape[lidx + 1] & 0xC000)
                    | (landscape[lidx + scen->GetWidth()] & 0xC000)
                    | (landscape[lidx + scen->GetWidth() + 1] & 0xC000);
                uint16_t* src = &cryst_data[src_row + cx];
                uint16_t* dst = (uint16_t*)((uint16_t*)g_selDrawBitmap.lpSurface + dst_pix + dst_row + cx);
                if (flags == 0) {
                    for (int32_t j = step; j != 0; j--) {
                        for (int32_t i = step; i != 0; i--) {
                            *dst = *src;
                            src++;
                            dst++;
                        }
                        dst += g_selDrawBitmap.dwWidth - step;
                        src += -step - cryst_w;
                    }
                } else if (flags == 0x8000) {
                    for (int32_t j = step; j != 0; j--) {
                        for (int32_t i = step; i != 0; i--) {
                            uint16_t s = *src++;
                            *dst = (uint16_t)(((s >> 1) & half_mask) + ((*dst >> 1) & half_mask));
                            dst++;
                        }
                        dst += g_selDrawBitmap.dwWidth - step;
                        src += -(cryst_w + step);
                    }
                }
                cx += step;
                sx += mode;
            }
            if (mode == 1) {
                for (int32_t i = 0; i < step; i++) {
                    mm2_tail -= (scen->GetWidth() - 0x10) << this->zoom;
                    memcpy(mm2_tail,
                        (uint16_t*)g_selDrawBitmap.lpSurface + g_selDrawBitmap.dwWidth * i
                            + dst_pix + dst_row + ox,
                        (scen->GetWidth() - 0x10) * (2 << this->zoom));
                }
            } else {
                mm2_tail -= (scen->GetWidth() - 0x10) >> 1;
                memcpy(mm2_tail,
                    (uint16_t*)g_selDrawBitmap.lpSurface + dst_pix + dst_row + ox,
                    scen->GetWidth() - 0x10);
            }
            src_row -= cryst_w * step;
            dst_row += g_selDrawBitmap.dwWidth * step;
            sy += mode;
        }
        mc->field_0xdc = 0;
    }
    POSITION pos = mc->field_0x9d0.GetStartPosition();
    while (pos != nullptr) {
        uint16_t key;
        CGameObject* obj;
        mc->field_0x9d0.GetNextAssoc(pos, key, obj);
        obj->VMethod9(rc.left + ox, rc.top + oy, this->zoom);
    }
    if (mode == 1) {
        DrawRectangleFrame((mc->view_x - 8) * step + rc.left + ox,
            (mc->view_y - 8) * step + rc.top + oy,
            (mc->view_x - 8) * step + rc.left + ox - 1 + mc->field_0x64 * step,
            (mc->view_y - 8) * step + rc.top + oy - 1 + mc->field_0x68 * step,
            (uint16_t)(((0xFF >> (8 - g_RBits & 0x1F)) << (g_RBitShift & 0x1F))
                | ((0xFF >> (8 - g_GBits & 0x1F)) << (g_GBitShift & 0x1F))
                | ((0xFF >> (8 - g_BBits & 0x1F)) << (g_BBitShift & 0x1F))));
    } else {
        DrawRectangleFrame((mc->view_x - 8) / 2 + rc.left + ox,
            (mc->view_y - 8) / 2 + rc.top + oy,
            (mc->view_x - 8) / 2 + rc.left + ox - 1 + mc->field_0x64 / 2,
            (mc->view_y - 8) / 2 + rc.top + oy - 1 + mc->field_0x68 / 2,
            (uint16_t)(((0xFF >> (8 - g_RBits & 0x1F)) << (g_RBitShift & 0x1F))
                | ((0xFF >> (8 - g_GBits & 0x1F)) << (g_GBitShift & 0x1F))
                | ((0xFF >> (8 - g_BBits & 0x1F)) << (g_BBitShift & 0x1F))));
    }
    UnlockSurface2();
}


// 4AEEC3
VisMiniMap::VisMiniMap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    this->bitmap1 = nullptr;
    this->bitmap2 = nullptr;
}


// 4AEF4D (scalar deleting dtor ??_G at 4B4730)
VisMiniMap::~VisMiniMap()
{
    if (this->bitmap1 != nullptr) {
        delete this->bitmap1;
    }
    if (this->bitmap2 != nullptr) {
        delete this->bitmap2;
    }
}


// 41F9B0
int32_t VisSpellBook::sub_41F9B0()
{
    if (this->pressed >= 0) {
        return this->pressed;
    }
    return this->spell;
}


// 41F9E0
int32_t VisSpellBook::sub_41F9E0()
{
    return this->pressed >= 0;
}


// 4CA89B
int32_t VisSpellBook::sub_4CA89B(int32_t id)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    return (main_wnd->vis_map_context->field_0x148 & (1 << (id & 0x1F))) != 0;
}


// 4CA8E0
int32_t VisSpellBook::sub_4CA8E0(int32_t id)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    return (main_wnd->vis_map_context->field_0x150 & (1 << (id & 0x1F))) != 0;
}


// 4CA925
void VisSpellBook::sub_4CA925(int32_t idx)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    BigStruct2* mc = main_wnd->vis_map_context;
    this->spell = idx;
    TokenEntry* entry = mc->field_0x138->tokenEntries[this->spell];
    this->selected = (int32_t)entry->GetCastSpellId() - 1;
}


// 4CAAA7
int32_t VisSpellBook::sub_4CAAA7()
{
    if (this->selected < 0) {
        return DAT_0062F8A8[this->pressed];
    }
    return DAT_0062FA28[this->selected];
}


// 4CAA69
void VisSpellBook::FUN_004caa69()
{
    (void)AfxGetMainWnd();
    if (this->selected >= 0) {
        this->selected = -1;
        this->spell = -1;
    }
}


// 4CA7C2
int32_t VisSpellBook::FUN_004ca7c2(CPoint* pos)
{
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    *pos += CPoint(-5 - rc.left, -5 - rc.top);
    int32_t left = vis_scr_rect.left;
    if ((main_wnd->dialogsMask & 2) != 0) {
        left = 0;
    }
    *pos += CPoint(-left, 0);
    if (pos->x < 0) {
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


// 4CADD2
int32_t VisSpellBook::OnRButtonDown(uint32_t wparam, CPoint pos)
{
    (void)wparam;
    (void)pos;
    this->pressed = -1;
    return 1;
}


// 4CAF70
int32_t VisSpellBook::OnRButtonUp(uint32_t wparam, CPoint pos)
{
    (void)wparam;
    (void)pos;
    this->pressed = -1;
    return 1;
}


// 4CAE94
int32_t VisSpellBook::OnRButtonDblClk(uint32_t wparam, CPoint pos)
{
    (void)wparam;
    (void)pos;
    return 1;
}


// 4CAD6A
int32_t VisSpellBook::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    (void)wparam;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    BigStruct2* mc = main_wnd->vis_map_context;
    int32_t idx = this->FUN_004ca7c2(&pos);
    if (idx >= 0
        && ((mc->field_0x148 | mc->field_0x150) & (1 << (idx & 0x1F))) != 0) {
        this->pressed = idx;
    }
    return 1;
}


// 4CAF8C
int32_t VisSpellBook::OnMouseMove(uint32_t wparam, CPoint pos)
{
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


// 4CAFF5
int32_t VisSpellBook::OnWmUser(uint32_t wparam, CPoint pos)
{
    return this->OnLButtonDown(wparam, pos);
}


// 4CAEA6
int32_t VisSpellBook::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    (void)wparam;
    (void)pos;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->field_0x408 != nullptr) {
        g_Cursors[0]->Use();
        if ((main_wnd->dialogsMask & 2) == 0) {
            if (main_wnd->field_0x410 == 2) {
                main_wnd->vis_invtype1->VMethod37(main_wnd->field_0x40c);
            } else if (main_wnd->field_0x410 == 1) {
                main_wnd->vis_invtype1->VMethod37(main_wnd->vis_invtype1->FUN_0046fb90());
            }
        } else {
            CVisualObject* obj = main_wnd->vis_root->FindChild(1000);
            ((VisShop*)obj)->sub_4BC97B();
        }
    }
    return 1;
}


// 4CADEE
int32_t VisSpellBook::OnLButtonDblClk(uint32_t wparam, CPoint pos)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    BigStruct2* mc = main_wnd->vis_map_context;
    int32_t idx = this->FUN_004ca7c2(&pos);
    if (idx < 0
        || ((mc->field_0x148 | mc->field_0x150) & (1 << (idx & 0x1F))) == 0
        || DAT_0062F9C8[idx] == 0) {
        this->OnLButtonDown(wparam, pos);
    } else {
        mc->FUN_0041a001(idx + 1);
        this->OnLButtonDown(wparam, pos);
    }
    return 1;
}


// 4CAC9B
int32_t VisSpellBook::OnKeyDown(uint32_t wparam)
{
    if (g_kbControlState != 0 && (wparam == 0x61 || wparam == 0x41)) {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        BigStruct2* mc = main_wnd->vis_map_context;
        uint32_t spell_id = 0;
        if (this->pressed == -1) {
            CPoint pt;
            GetCursorPos(&pt);
            int32_t idx = this->FUN_004ca7c2(&pt);
            if (idx >= 0 && DAT_0062F968[idx] != 0) {
                spell_id = BOOK_POS_TO_SPELL_ID[idx + 1];
            }
        } else if (DAT_0062F968[this->pressed] != 0) {
            spell_id = BOOK_POS_TO_SPELL_ID[this->pressed + 1];
        }
        if (spell_id != 0) {
            mc->FUN_0041ace2((int32_t)spell_id);
            mc->UpdateSelectionState();
        }
    }
    return 0;
}


// 4CAAD7
int32_t VisSpellBook::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (msg == 0x411) {
        this->pressed = -1;
    } else if (msg == 0x417) {
        if (lparam == 0) {
            if (main_wnd->m_GameSession.shortcuts[wparam].kind == 1) {
                this->pressed = main_wnd->m_GameSession.shortcuts[wparam].item_id;
            }
        } else {
            if (this->pressed == -1) {
                CPoint pt;
                GetCursorPos(&pt);
                int32_t idx = this->FUN_004ca7c2(&pt);
                if (idx >= 0) {
                    main_wnd->m_GameSession.shortcuts[wparam].FUN_0041e323((short)idx);
                    for (int32_t i = 0; i < 9; i++) {
                        if (i != (int32_t)wparam
                            && main_wnd->m_GameSession.shortcuts[i].FUN_0041e456((short)idx)) {
                            main_wnd->m_GameSession.shortcuts[i].SetNull();
                        }
                    }
                }
            } else {
                main_wnd->m_GameSession.shortcuts[wparam].FUN_0041e323((short)this->pressed);
                for (int32_t i = 0; i < 9; i++) {
                    if (i != (int32_t)wparam
                        && main_wnd->m_GameSession.shortcuts[i].FUN_0041e456((short)this->pressed)) {
                        main_wnd->m_GameSession.shortcuts[i].SetNull();
                    }
                }
            }
            main_wnd->m_GameSession.FUN_004948b2();
        }
    }
    return this->CVisualObject::MsgProc(msg, wparam, lparam);
}


// 4CA2A1
void VisSpellBook::VMethod7()
{
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    LockSurface2();
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    BigStruct2* mc = main_wnd->vis_map_context;
    int32_t left = vis_scr_rect.left;
    if ((main_wnd->dialogsMask & 2) != 0) {
        left = 0;
    }
    if (left != 0) {
        if (g_ScreenSize.bottom < 0x259) {
            if (g_ScreenSize.bottom > 0x1E0) {
                g_bmp_spb800l->VMethod10(rc.left, rc.top, 0, 0,
                    g_bmp_spb800l->GetWidth(0), g_bmp_spb800l->GetHeight(0));
                g_bmp_spb800r->VMethod10(rc.left + g_bmp_spellbook->GetWidth(0) - 0x10 + left, rc.top, 0, 0,
                    g_bmp_spb800r->GetWidth(0), g_bmp_spb800r->GetHeight(0));
            }
        } else {
            g_bmp_spb1024l->VMethod10(rc.left, rc.top, 0, 0,
                g_bmp_spb1024l->GetWidth(0), g_bmp_spb1024l->GetHeight(0));
            g_bmp_spb1024r->VMethod10(rc.left + g_bmp_spellbook->GetWidth(0) - 0x10 + left, rc.top, 0, 0,
                g_bmp_spb1024r->GetWidth(0), g_bmp_spb1024r->GetHeight(0));
        }
    }
    g_bmp_spellbook->VMethod10(rc.left + left, rc.top, 0, 0,
        g_bmp_spellbook->GetWidth(0), g_bmp_spellbook->GetHeight(0));
    for (int32_t i = 0; i < 24; i++) {
        if (((mc->field_0x148 | mc->field_0x150) & (1 << (i & 0x1F))) == 0) {
            g_bmp_spellback->VMethod2(left + rc.left + 6 + (i % 0xC) * 0x26,
                rc.top + 6 + (i / 0xC) * 0x26, 0, 0, 0);
        }
    }
    if (this->pressed >= 0
        && ((mc->field_0x148 | mc->field_0x150) & (1 << (this->pressed & 0x1F))) != 0) {
        int32_t bx = (this->pressed % 0xC) * 0x26 + 6;
        int32_t by = (this->pressed / 0xC) * 0x26 + 6;
        sub_457C5D(left + rc.left + bx, rc.top + by,
            left + rc.left + 0x24 + bx, rc.top + 0x24 + by, 4);
    }
    for (int32_t i = 0; i < 0x18; i++) {
        if (((mc->field_0x148 | mc->field_0x150) & (1 << (i & 0x1F))) != 0) {
            int32_t fkey = 0;
            for (int32_t s = 0; s < 9; s++) {
                if (main_wnd->m_GameSession.shortcuts[s].FUN_0041e456((short)i)) {
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
                    rc.top + 8 + off + (i / 0xC) * 0x26, buf, 0, clrsh_TechBlack, 1);
            }
        }
        if ((mc->field_0x14c & (1 << (i & 0x1F))) != 0) {
            char buf[80];
            sprintf(buf, "A");
            int32_t off = 0;
            if (this->pressed == i) {
                off = 2;
            }
            g_font3->DrawTextWithShadow(left + rc.left + 8 + off + (i % 0xC) * 0x26,
                rc.top + 0x22 + off + (i / 0xC) * 0x26, buf, 0, clrsh_TechBlack, 1);
        }
    }
    UnlockSurface2();
}


// 4C9A6F
const char* VisSpellBook::GetHint()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    BigStruct2* mc = main_wnd->vis_map_context;
    CPoint mpt(g_mousept.GetX(), g_mousept.GetY());
    int32_t idx = this->FUN_004ca7c2(&mpt);
    if ((mc->field_0x148 & (1 << (idx & 0x1F))) == 0) {
        return nullptr;
    }
    if ((main_wnd->dialogsMask & 2) == 0 && main_wnd->dialogsMask != 1) {
        return nullptr;
    }
    byte_666590[0] = 0;
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
    sprintf(byte_666590, "%s%s%s%s%s", (LPCTSTR)str, (LPCTSTR)dmg_str,
        (LPCTSTR)range_str, (LPCTSTR)power_str, (LPCTSTR)vals_str);
    return byte_666590;
}


// 4C99C7
VisSpellBook::VisSpellBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: CVisualObject(_id, l, t, r, b, nullptr)
{
    this->field_0x5c = 0;
    this->pressed = -1;
    this->selected = -1;
    this->spell = -1;
}


// 4CB020 (scalar deleting dtor ??_G at 4CB020; complete dtor FUN_004cb050 has no custom cleanup)
VisSpellBook::~VisSpellBook()
{
}


// VisMainMenu


// 4AB41E
int32_t VisMainMenu::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    if (msg == 0x402) {
        this->VMethod9();
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 4AB411
void VisMainMenu::VMethod8(CRect* rect)
{
    (void)rect;
}


// 4AB28F
void VisMainMenu::VMethod7()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    if (main_wnd->dialogsMask != 0x80) {
        return;
    }
    LockSurface2();
    this->bmp_menu->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
    if (this->bmp_active_button != nullptr) {
        this->bmp_active_button->VMethod2(this->rect_active_button.left, this->rect_active_button.top, 0, 0, 0);
        if (this->pressed_button >= 0) {
            this->bmp_labels.GetAt(this->pressed_button)->VMethod2(screen_rect.left + 0xE8, screen_rect.top + 200, 0, 0, 0);
        }
    } else {
        if (this->active_button >= 0) {
            this->bmp_labels.GetAt(this->active_button)->VMethod2(screen_rect.left + 0xE8, screen_rect.top + 200, 0, 0, 0);
        }
    }
    this->sprite->VMethod2(screen_rect.left + 0x1E0, screen_rect.top + 0x168, 0, 0, 0);
    UnlockSurface2();
}


// 4AB88C
int32_t VisMainMenu::OnMouseMove(uint32_t wparam, CPoint pos)
{
    this->UpdateButtonState(wparam, pos);
    return 0;
}


// 4AB8AF
int32_t VisMainMenu::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    this->UpdateButtonState(wparam, pos);
    if (this->pressed_button != -1) {
        CSound::Play((CSound&)this->snd_btn_click);
    }
    return 1;
}


// 4AB8F3
int32_t VisMainMenu::OnLButtonUp(uint32_t wparam, CPoint pos)
{
    uint32_t msg = 0;
    switch (this->active_button) {
    case 0:
        if ((((uint8_t)this->disable_mask) & 1) == 0) {
            msg = 0x425;
        }
        break;
    case 1:
        if ((((uint8_t)this->disable_mask) & 2) == 0) {
            msg = 0x426;
        }
        break;
    case 2:
        if ((((uint8_t)this->disable_mask) & 4) == 0) {
            msg = 0x43B;
        }
        break;
    case 3:
        if ((((uint8_t)this->disable_mask) & 8) == 0) {
            msg = 0x428;
        }
        break;
    case 4:
        if ((((uint8_t)this->disable_mask) & 0x10) == 0) {
            msg = 0x418;
        }
        break;
    case 5:
        if ((((uint8_t)this->disable_mask) & 0x20) == 0) {
            msg = 0x487;
        }
        break;
    case 6:
        if ((((uint8_t)this->disable_mask) & 0x40) == 0) {
            msg = 0x429;
        }
        break;
    case 7:
        if ((((uint8_t)this->disable_mask) & 0x80) == 0) {
            msg = 0x10;
        }
        break;
    }
    if (msg != 0) {
        AfxGetMainWnd()->PostMessage(msg, 0, 0);
    }
    this->pressed_button = -1;
    this->UpdateButtonState(wparam, pos);
    if (msg != 0) {
        this->VMethod9();
        this->DoClose(msg);
    }
    return 1;
}


// Main menu button layout. 60CC30 — [x, y, w, h] per button.
static const int32_t DWORD_0060CC30[8][4] = {
    {0xCC, 0x34, 0x68, 0x60}, {0x7C, 0x9C, 0x6C, 0x4C}, {0x7C, 0xFC, 0x60, 0x58},
    {0xD0, 0x154, 0x64, 0x64}, {0x154, 0x34, 0x58, 0x64}, {0x1A8, 0x98, 0x54, 0x58},
    {0x19C, 0x104, 0x60, 0x54}, {0x158, 0x15C, 0x48, 0x50},
};

// Main menu label layout. 60CCB0 — [x, y, w, h] per label.
static const int32_t DWORD_0060CCB0[8][4] = {
    {0x74, 0x40, 0xD0, 0x8A}, {0x58, 0x58, 0xEC, 0x98}, {0x58, 0xEC, 0xEC, 0x98},
    {0x74, 0x110, 0xD0, 0x8C}, {0x140, 0x40, 0xD4, 0x8C}, {0x144, 0x58, 0xE8, 0x98},
    {0x144, 0xEC, 0xEC, 0x98}, {0x140, 0x110, 0xD4, 0x8C},
};


// 4AAB71
void VisMainMenu::VMethod26()
{
    CRect screen_rect;
    CRect r;
    this->ClientRectToScreen(&screen_rect, this->rect);
    CPoint top_left = screen_rect.TopLeft();
    this->bmp_menu = nullptr;
    this->bmp_menu_mask = nullptr;
    this->bmp_active_button = nullptr;
    this->snd_btn_click = nullptr;
    this->sprite = nullptr;
    this->rect_buttons.RemoveAll();
    this->rect_labels.RemoveAll();
    for (int32_t i = 0; i < 8; i++) {
        r.SetRect(top_left.x + DWORD_0060CC30[i][0], top_left.y + DWORD_0060CC30[i][1],
            top_left.x + DWORD_0060CC30[i][0] + DWORD_0060CC30[i][2],
            top_left.y + DWORD_0060CC30[i][1] + DWORD_0060CC30[i][3]);
        this->rect_buttons.Add(r);
        r.SetRect(top_left.x + DWORD_0060CCB0[i][0], top_left.y + DWORD_0060CCB0[i][1],
            top_left.x + DWORD_0060CCB0[i][0] + DWORD_0060CCB0[i][2],
            top_left.y + DWORD_0060CCB0[i][1] + DWORD_0060CCB0[i][3]);
        this->rect_labels.Add(r);
    }
    this->disable_mask = 0;
    VisButton* btn = new VisButton(4, 0, 0, 0, 0, "", g_font1, clrsh_TechBlack, 0x7fff, 0, nullptr);
    this->AddChild(btn);
}


// 4ABA8D
void VisMainMenu::VMethod28()
{
    this->LoadGraphics();
    this->LoadSfx();
    g_mousept.DisableHint();
    this->active_button = -1;
    this->pressed_button = -1;
    this->over_button = -1;
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();
    this->VisScreen::VMethod28();
}


// 4ABB14
void VisMainMenu::DoClose(uint32_t code)
{
    this->FreeGraphics();
    this->FreeSfx();
    this->VisScreen::DoClose(code);
    AfxGetMainWnd()->SendMessage(0x44C, (WPARAM)this, 0);
    g_mousept.EnableHint();
}


// 4AB45D
void VisMainMenu::UpdateButtonState(uint32_t wparam, CPoint mouse)
{
    if (!this->rect.PtInRect(mouse)) {
        return;
    }
    CPoint top_left = this->rect.TopLeft();
    mouse.x -= top_left.x;
    mouse.y -= top_left.y;
    int32_t idx = mouse.x + mouse.y * 640;
    this->bmp_active_button = nullptr;
    this->over_button = -1;
    uint8_t cell = ((uint8_t*)this->bmp_menu_mask->GetData())[idx];
    switch (cell) {
    case 0x80:
        this->over_button = 0;
        break;
    case 0x90:
        this->over_button = 1;
        break;
    case 0xA0:
        this->over_button = 2;
        break;
    case 0xB0:
        this->over_button = 3;
        break;
    case 0xC0:
        this->over_button = 4;
        break;
    case 0xD0:
        this->over_button = 5;
        break;
    case 0xE0:
        this->over_button = 6;
        break;
    case 0xF0:
        this->over_button = 7;
        break;
    }
    if (wparam == 1) {
        if (this->pressed_button == -1) {
            this->pressed_button = this->over_button;
            this->active_button = this->pressed_button;
        } else if (this->pressed_button != this->over_button) {
            this->active_button = -1;
        } else {
            this->active_button = this->pressed_button;
        }
    } else {
        if (this->pressed_button == -1) {
            this->active_button = this->over_button;
        }
    }
    if (this->pressed_button == -1) {
        if (this->active_button != -1) {
            this->bmp_active_button = this->bmp_button.GetAt(this->active_button);
            this->rect_active_button = this->rect_buttons.GetAt(this->active_button);
        }
    } else if (this->over_button == this->pressed_button) {
        this->bmp_active_button = this->bmp_pressed.GetAt(this->pressed_button);
        this->rect_active_button = this->rect_buttons.GetAt(this->pressed_button);
        this->active_button = this->pressed_button;
    }
    if ((((uint8_t)this->disable_mask) & (1u << (this->over_button & 0x1f))) != 0) {
        this->bmp_active_button = nullptr;
    }
}


// 4ABB5E
void VisMainMenu::LoadSfx()
{
    this->FreeSfx();
    FUN_00438e40(&this->snd_btn_click, "SFX\\ChrGen\\Ok.wav");
}


// 4ABB87
void VisMainMenu::FreeSfx()
{
    FUN_00438dd0(&this->snd_btn_click);
}


// 4AADA7
void VisMainMenu::LoadGraphics()
{
    CString str;
    this->FreeGraphics();
    this->bmp_menu_mask = new CBmp256("main\\graphics\\MainMenu\\MenuMask.bmp");
    g_mousept.Update();
    this->bmp_menu = new CBmp64("main\\graphics\\MainMenu\\menu_.bmp");
    g_mousept.Update();
    this->sprite = new CA16("graphics\\interface\\sprites.16a");
    this->sprite->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    for (int32_t i = 0; i < 8; i++) {
        str.Format("main\\graphics\\MainMenu\\button%dp.bmp", i + 1);
        this->bmp_pressed.Add(new CBmp64(str));
        g_mousept.Update();
        str.Format("main\\graphics\\MainMenu\\button%d.bmp", i + 1);
        this->bmp_button.Add(new CBmp64(str));
        g_mousept.Update();
        str.Format("main\\graphics\\MainMenu\\text%d.bmp", i + 1);
        this->bmp_labels.Add(new CBmp64(str));
        g_mousept.Update();
    }
    this->bmp_active_button = nullptr;
}


// 4AB07B
void VisMainMenu::FreeGraphics()
{
    if (this->bmp_menu != nullptr) {
        delete this->bmp_menu;
    }
    this->bmp_menu = nullptr;
    if (this->bmp_menu_mask != nullptr) {
        delete this->bmp_menu_mask;
    }
    this->bmp_menu_mask = nullptr;
    if (this->sprite != nullptr) {
        delete this->sprite;
    }
    this->sprite = nullptr;
    while (this->bmp_pressed.GetSize() != 0) {
        if (this->bmp_pressed.GetAt(0) != nullptr) {
            delete this->bmp_pressed.GetAt(0);
        }
        this->bmp_pressed.RemoveAt(0, 1);
        if (this->bmp_button.GetAt(0) != nullptr) {
            delete this->bmp_button.GetAt(0);
        }
        this->bmp_button.RemoveAt(0, 1);
        if (this->bmp_labels.GetAt(0) != nullptr) {
            delete this->bmp_labels.GetAt(0);
        }
        this->bmp_labels.RemoveAt(0, 1);
    }
}


// 4AA926
VisMainMenu::VisMainMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr)
{
    this->VMethod26();
}


// 4AAAB6 (deleting dtor thunk ??_G at 4ABBB0 is compiler-generated)
VisMainMenu::~VisMainMenu()
{
    this->FreeGraphics();
    this->FreeSfx();
}


// 44DBE0
void VisServerScreen::VMethod7()
{
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
        Player* player = g_PlayersList->sub_535B50(client->player_id);
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
        avg_units_per_10_ticks = g_Server->field44_0x1bc / g_Server->tic16 / 10;
    }
    if (g_Server->srv_stru1->sack_list != nullptr) {
        str.Format(txt_patch.GetLine(0x42),
                   g_PlayersList->CountHumanPlayers(),
                   dword_6CDB3C->unit_list.GetCount(),
                   g_Server->srv_stru1->building_list->GetCount(),
                   g_Server->srv_stru1->units_list->unit_list.GetCount(),
                   g_Server->srv_stru1->sack_list->list.GetCount(),
                   g_Server->field42_0x1b4 / 10,
                   avg_units_per_10_ticks,
                   main_wnd->current_map_name);
        g_font2->DrawTxt(0, row_bottom + 4, str, 0, clrsh_ShockingBlack);
    }
    this->msg_log->Draw();
    UnlockSurface2();
}


// 44D909
void VisServerScreen::VMethod26()
{
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
        g_Server->field39_0x1a8 = 0;
    }
    if (g_Server->field39_0x1a8 != 0) {
        checked = 0;
    }
    save_on_server->ReadData(&checked);

    this->AddChild(new VisTextBox(0x68, 4, g_ScreenSize.bottom - 0x38, g_ScreenSize.right - 4, g_ScreenSize.bottom - 0x28,
                                  g_font2, clrsh_ShockingBlack, nullptr));
}


// 44E6DE
int32_t VisServerScreen::OnKeyDown(uint32_t wparam)
{
    if (wparam == 0x0D) {  // Enter: send the chat text box content
        PacketJoin& pkt = PacketJoin::Inst;
        pkt.id = 0x91;
        pkt.to_player_id = 0;
        pkt.__field_0xa = 0;

        CVisualObject* textbox = this->FindChild(0x68);
        textbox->WriteData(pkt.name);

        if (pkt.name[0] == '#') {
            sub_44E4CE(&pkt.name[1]);
        } else {
            LogMessage(pkt.name);
            g_NetStru1_main.QueuePacketSend(&pkt);
        }

        CVisualObject* textbox_clear = this->FindChild(0x68);
        textbox_clear->ReadData(unk_659A48);
        return 1;
    }

    CVisualObject* textbox = this->FindChild(0x68);
    return textbox->OnKeyDown(wparam);
}


// 44E7C8
int32_t VisServerScreen::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    switch (msg) {
    case 0x446:  // "disconnect" button
        this->VisScreen::MsgProc(0x446, 0, 0);
        PostMessageA(g_MainWndHWND, 0x41D, 0, 0);
        return 1;

    case 0x46E:  // "save on server" checkbox changed
        if (wparam == 0x65) {
            g_Server->field39_0x1a8 = (lparam == 0) ? 1 : 0;
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
void VisServerScreen::VMethod8(CRect* rect)
{
    FillRectColorSimple(rect->left, rect->top, rect->right, rect->bottom, 0);
}


// 44e469
VisServerScreen::VisServerScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, GM_a28* log)
: VisScreen(_id, l, t, r, b, nullptr)
{
    this->msg_log = log;
}


// 450ab0 (deleting dtor ??_G; the complete dtor at 450AE0 only calls the base dtor)
VisServerScreen::~VisServerScreen() = default;


// 44878A
int32_t VisNetPhoneBook::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    switch (msg) {
    case 0x444:
        PostMessageA(g_MainWndHWND, 0x46e, wparam, lparam);
        PostMessageA(g_MainWndHWND, 0x47d, 0, 0);
        return 1;
    case 0x446: {
        int32_t res = this->VisScreen::MsgProc(msg, wparam, lparam);
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        main_wnd->PostMessage(0x451, 0, 0);
        return res;
    }
    case 0x46e:
        if (wparam == 1) {
            CVisualObject* edit = this->FindChild(3);
            const char* str;
            if ((int32_t)lparam < 0) {
                str = "";
            } else {
                str = this->phones->phones[lparam];
            }
            this->sub_448708(str);
            edit->ReadData(str);
            edit->VMethod9();
        }
        if (wparam == 3) {
            CVisualObject* edit = this->FindChild(3);
            char buf[64];
            edit->WriteData(buf);
            this->sub_448708(buf);
        }
        return 1;
    case 0x47d: {
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
            main_wnd->PostMessage(0x452, 0, 0);
            this->VisScreen::MsgProc(0x445, 0, 0);
        }
        return 1;
    }
    case 0x47e: {
        this->VisScreen::MsgProc(0x445, 0, 0);
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        VisListBox* list = (VisListBox*)this->FindChild(7);
        int32_t sel = list->GetSelectedIndex();
        CLlAddress* addr = this->enum_addresses + sel;
        if (g_CLlDriver.StartServer(2, main_wnd->m_GameSession.character_name, addr) == 0) {
            VisMessageBoxWithList* msgbox = new VisMessageBoxWithList(
                1, 100, 100, 0x21c, 0x17c, TxtFile::AllLines.GetAt(0x9a), nullptr, 0);
            main_wnd->ModalScreen(msgbox);
            main_wnd->PostMessage(0x455, 0, 0);
        } else {
            main_wnd->PostMessage(0x47f, 0, 0);
        }
        return 1;
    }
    default:
        return this->VisScreen::MsgProc(msg, wparam, lparam);
    }
}


// 447EF2
void VisNetPhoneBook::VMethod26()
{
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
    this->sub_448708(str);
}


// 447ea8
VisNetPhoneBook::VisNetPhoneBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, PhoneBook* _book)
: VisWindow(_id, l, t, r, b, nullptr)
{
    this->phones = _book;
    this->enum_addresses = nullptr;
}


// 44fae0 (deleting dtor ??_G; the complete dtor at 44FB10 only calls the base dtor)
VisNetPhoneBook::~VisNetPhoneBook() = default;


// 448708
void VisNetPhoneBook::sub_448708(const char* str)
{
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
: VisWindow(_id, l, t, r, b, nullptr)
{
    this->pcom = _com;
}


// 44fcc0 (deleting dtor ??_G; the complete dtor sub_44fcf0 only calls the base dtor)
VisNetSerialSettings::~VisNetSerialSettings() = default;


// 449903
void VisNetSerialSettings::SaveComSettings()
{
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
void VisNetSerialSettings::VMethod26()
{
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
int32_t VisNetSerialSettings::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    switch (msg) {
    case 0x446: {
        int32_t result = VisScreen::MsgProc(msg, wparam, lparam);
        AfxGetMainWnd()->PostMessage(0x451, 0, 0);
        return result;
    }
    case 0x480: {
        this->SaveComSettings();

        CLlAddress addr;
        addr.com = *this->pcom;
        if (g_CLlDriver.PrepareForConnect("", &addr) != 0) {
            AfxGetMainWnd()->PostMessage(0x452, 0, 0);
            VisScreen::MsgProc(0x445, 0, 0);
        }
        return 1;
    }
    case 0x481: {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

        this->SaveComSettings();

        CLlAddress addr;
        addr.com = *this->pcom;
        if (g_CLlDriver.StartServer(2, main_wnd->m_GameSession.character_name, &addr) != 0) {
            VisScreen::MsgProc(0x445, 0, 0);
            main_wnd->PostMessage(0x482, 0, 0);
        }
        return 1;
    }
    case 0x483: {
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

// 44bc6c
void VisHatBrowserDlg::VMethod26()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    VisLabel* title_label = new VisLabel(0x19, 0x28, 0x16, this->rect.Width() - 0x28, 0x38, txt_patch.GetLine(0x6a), g_font1, p_clrsh_Black, 2);
    this->AddChild(title_label);

    VisLabel* ip_label = new VisLabel(0x7e, 0x28, 0x2c, this->rect.Width() - 0x28, 0x44, main_wnd->hat_settings.hatip, g_font1, p_clrsh_Black, 2);
    this->AddChild(ip_label);

    VisLabel* caption_label = new VisLabel(0x1a, 0x28, 0x44, this->rect.Width() - 0x28, 0x5c, txt_patch.GetLine(0x6b), g_font1, p_clrsh_Black, 0);
    this->AddChild(caption_label);

    VisHatBrowserList* list = new VisHatBrowserList(1, 0x28, 0x5c, this->rect.Width() - 0x40, this->rect.Height() - 0x60, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0xa, nullptr);
    this->AddChild(list);

    list->SetCaptionLabel((VisLabel*)this->FindChild(0x1a));

    CRect list_rect = list->GetRect();
    VisScrollBar* scrollbar = new VisScrollBar(0xa, list_rect.right, list_rect.top, list_rect.right + 0x18, list_rect.bottom, nullptr);
    this->AddChild(scrollbar);

    CPoint btn_center(0xc0, this->rect.Height() - 0x30);
    CRect btn_rect(btn_center.x - 0x30, btn_center.y - 0xc, btn_center.x + 0x30, btn_center.y + 0xc);
    btn_rect.OffsetRect(-0x48, 0);

    VisButton* ok_btn = new VisButton(0x14, btn_rect, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, byte_659A34);
    this->AddChild(ok_btn);
    ok_btn->ChangeFlags(1, false);

    btn_rect.OffsetRect(0x90, 0);
    VisButton* cancel_btn = new VisButton(0x15, btn_rect, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, byte_659A38);
    this->AddChild(cancel_btn);
    cancel_btn->SetLeftObj(this->FindChild(0x14));

    btn_rect.OffsetRect(0x90, 0);
    VisButton* refresh_btn = new VisButton(0x16, btn_rect, txt_patch.GetLine(0x6c), g_font1, nullptr, 0x48b, 0, byte_659A3C);
    this->AddChild(refresh_btn);
    refresh_btn->SetLeftObj(this->FindChild(0x15));
}

// 44c1a9
int32_t VisHatBrowserDlg::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    case 0x444:
        if (wparam != 1) {
            return VisScreen::MsgProc(msg, wparam, lparam);
        }
        // fallthrough
    case 0x445: {
        if (this->FindChild(0x14)->TestFlags(1) != 0) {
            VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
            CString item = list->GetItem(list->GetSelectedIndex());
            int32_t field_pos = GetFieldPos(item, 7);
            main_wnd->field_0x3e0.field_14 = item.Mid(field_pos);
            main_wnd->field_0x3e0.field_14 = main_wnd->field_0x3e0.field_14.Left(main_wnd->field_0x3e0.field_14.Find('|'));
            VisScreen::MsgProc(0x445, 0, 0);
        }
        return 1;
    }
    case 0x446:
        VisScreen::MsgProc(0x446, 0, 0);
        return 1;
    case 0x46e:
    case 0x473:
        if (wparam == 1) {
            if ((int32_t)lparam >= 0) {
                VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
                list->VMethod9();
                int32_t enabled = list->IsItemEnabled(list->GetSelectedIndex());
                this->FindChild(0x14)->ChangeFlags(1, enabled != 0);
                this->FindChild(0x14)->VMethod9();
            }
            return 1;
        }
        return 0;
    case 0x48a: {
        VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
        int32_t sel_index = list->GetSelectedIndex();
        int32_t vis_start = list->vis_start_index;
        list->entries.RemoveAll();
        list->selected_index = -1;
        list->state = wparam;
        if (list->state == 0) {
            list->vis_start_index = 0;
            list->SetSelectedIndex(0);
        }
        else {
            for (int32_t i = 0; i < DAT_00666a00.GetSize(); i++) {
                list->AddItem(DAT_00666a00[i]);
            }
            if (DAT_00666a00.GetSize() <= sel_index) {
                sel_index = DAT_00666a00.GetSize() - 1;
            }
            if (DAT_00666a00.GetSize() <= vis_start) {
                vis_start = DAT_00666a00.GetSize() - 1;
            }
            list->vis_start_index = vis_start;
            list->SetSelectedIndex(sel_index);
        }
        this->MsgProc(0x46e, 1, list->GetSelectedIndex());
        list->VMethod9();
        return 1;
    }
    case 0x48b: {
        VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
        list->state = -1;
        list->entries.RemoveAll();
        list->selected_index = -1;
        list->vis_start_index = 0;
        list->SetSelectedIndex(0);
        list->VMethod9();

        int32_t res; // 44c55e: left uninitialized when hat_settings.ishat == 0
        if (main_wnd->hat_settings.ishat != 0) {
            if (main_wnd->FUN_00490eb3() == 0) {
                g_CLlDriver.Close();
                main_wnd->PostMessage(0x446, 0, 0);
                return VisScreen::MsgProc(msg, wparam, lparam);
            }
            res = main_wnd->FUN_004e5466(main_wnd->hat_settings.hatip);
        }
        this->MsgProc(0x48a, res, 0);
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
}

// 44bc35
VisHatBrowserDlg::VisHatBrowserDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : VisWindow(_id, l, t, r, b, nullptr)
{
}

// 44fed0
VisHatBrowserDlg::~VisHatBrowserDlg()
{
}

// 43C60C
void VisCredits::VMethod7()
{
    if (this->flag == 0) {
        return;
    }

    static uint8_t credits_timing_init = 0;  // byte_65995C in asm
    static uint32_t credits_scroll_time = 0; // dword_659958 in asm
    if (credits_timing_init == 0) {
        credits_timing_init = 1;
        credits_scroll_time = timeGetTime();
    }

    uint32_t now = timeGetTime();
    int32_t line_height = g_font1->GetHeight();
    int32_t left = this->rect.TopLeft().x;
    int32_t top = this->rect.TopLeft().y;
    CRect saved_clip;
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    if (now - credits_scroll_time < 0x17) {
        return;
    }

    this->scroll -= 1;
    if (this->scroll < 0) {
        this->first_visible = abs(this->scroll) / line_height;
    } else {
        this->first_visible = 0;
    }

    GetClipRect(&saved_clip);
    SetClipRect(screen_rect);
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);

    int32_t last = this->first_visible + 1 + 0x1e0 / line_height;
    if (last > this->text.GetCount()) {
        last = this->text.GetCount();
    }
    int32_t first = (this->first_visible - 10 < 0) ? 0 : this->first_visible - 10;

    for (int32_t i = first; i < last; i++) {
        char* line = this->text.GetLine(i);
        if (*line == '"') {
            CObject* value = nullptr;
            if (this->bitmaps.Lookup(this->text.GetLine(i), value) != 0) {
                CBmp64* bmp = (CBmp64*)value;
                int32_t width = bmp->GetWidth();
                bmp->VMethod2(left + 0x140 - width / 2, top + this->scroll + i * line_height, 0, 0, 0);
            }
        } else if (i == 0) {
            g_font1->DrawTxt(left + 0x140, top + this->scroll + i * line_height, this->text.GetLine(0), 2, clrsh_ShockingBlack);
        } else {
            if (strlen(this->text.GetLine(i - 1)) == 0) {
                g_font1->DrawTxt(left + 0x140, top + this->scroll + i * line_height, this->text.GetLine(i), 2, clrsh_ShockingBlack);
            } else {
                g_font1->DrawTxt(left + 0x140, top + this->scroll + i * line_height, this->text.GetLine(i), 2, clrsh_TechBlack);
            }
        }
    }

    UnlockSurface2();
    SetClipRect(saved_clip);

    if (this->first_visible >= last) {
        this->MsgProc(0x445, 0, 0);
    }

    credits_scroll_time = timeGetTime();
}

// 43C438
void VisCredits::VMethod26()
{
    this->bitmaps.RemoveAll();
    this->AddChild(new VisButton(4, 0, 0, 0, 0, " ", g_font1, clrsh_TechBlack, 0x445, 0, nullptr));
}

// 43C595
int32_t VisCredits::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    if (msg == 0x402) {
        this->VMethod9();
    }
    return VisScreen::MsgProc(msg, wparam, lparam);
}

// 43C4D6
void VisCredits::VMethod28()
{
    g_mousept.DisableHint();
    this->scroll = 0x1e0;
    this->LoadContent();
    this->flag = 1;
    g_mousept.Unpaint();
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();
    VisScreen::VMethod28();
}

// 43C9B5
void VisCredits::LoadContent()
{
    this->FreeContent();
    this->text.LoadChunkFile("main\\text\\credits.txt");
    for (int32_t i = 0; i < this->text.GetCount(); i++) {
        char* line = this->text.GetLine(i);
        if (*line != '"') {
            continue;
        }
        CString name(line);
        name = name.Mid(1);
        name = name.Left(name.GetLength() - 1);
        name = "main\\graphics\\logo\\" + name;
        this->bitmaps.SetAt(this->text.GetLine(i), new CBmp64(name));
    }
}

// 43CB77
void VisCredits::FreeContent()
{
    POSITION it = this->bitmaps.GetStartPosition();
    CString key;
    while (it != nullptr) {
        CObject* value = nullptr;
        this->bitmaps.GetNextAssoc(it, key, value);
        if (value != nullptr) {
            delete value;
        }
    }
    this->bitmaps.RemoveAll();
    this->text.Free();
}

// 43C553
void VisCredits::DoClose(uint32_t code)
{
    this->FreeContent();
    g_mousept.Paint();
    this->flag = 0;
    VisScreen::DoClose(code);
    g_mousept.EnableHint();
}

// 43C5D4
int32_t VisCredits::OnKeyDown(uint32_t wparam)
{
    this->MsgProc(0x445, 0, 0);
    return VisScreen::OnKeyDown(wparam);
}

// 43C5F5
int32_t VisCredits::OnLButtonDown(uint32_t wparam, CPoint pos)
{
    this->MsgProc(0x445, 0, 0);
    return 0;
}

// 43C9A8
void VisCredits::VMethod8(CRect* rect)
{
}

// 43C337
VisCredits::VisCredits(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : VisScreen(_id, l, t, r, b, nullptr)
{
    this->VMethod26();
}

// 43C3C6
VisCredits::~VisCredits()
{
    this->FreeContent();
}

// 44A140
void VisHatServerListDlg::VMethod31(int32_t code)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (code == 0x445) {
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
        this->FindChild(9)->WriteData(&this->hat_settings->store);
        main_wnd->field_0x3e0.field_10 = (this->hat_settings->ishat == 0) ? 1 : 0;
        if (this->hat_settings->ishat == 0) {
            main_wnd->PostMessage(0x426, 0, 0);
        } else {
            main_wnd->PostMessage(0x488, 0, 0);
        }
    } else {
        main_wnd->PostMessage(0x421, 0, 0);
    }
}

// 44A8A8
int32_t VisHatServerListDlg::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    if (msg != 0x46e) {
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
CVisualObject* VisHatServerListDlg::VMethod30(const void* data, const RECT& r)
{
    HatSettings* hat = (HatSettings*)data;

    VisTextBox* ip_box = new VisTextBox(2, r.left, r.top, r.right, r.top + 0x18, g_font1, clrsh_TechBlack, txt_patch.GetLine(0x70));
    this->AddChild(ip_box);
    if (hat->ishat == 0) {
        ip_box->ReadData(hat->hatip);
    } else {
        ip_box->ReadData(hat->hatprogip);
    }

    VisRadioType2* deathmatch_radio = new VisRadioType2(3, r.left, 0x6c, r.right, 0x9c, g_font1, p_clrsh_Black, nullptr);
    deathmatch_radio->AddEntry(txt_patch.GetLine(0x68));
    deathmatch_radio->AddEntry(txt_patch.GetLine(0x69));
    this->AddChild(deathmatch_radio);
    deathmatch_radio->ReadData(&hat->deathmatch);
    if (hat->ishat == 0) {
        deathmatch_radio->ChangeFlags(1, false);
    }

    VisRadioType2* ishat_radio = new VisRadioType2(4, r.left, 0xa8, r.right, 0xd8, g_font1, p_clrsh_Black, nullptr);
    ishat_radio->AddEntry(txt_patch.GetLine(0x78));
    ishat_radio->AddEntry(txt_patch.GetLine(0x79));
    this->AddChild(ishat_radio);
    ishat_radio->ReadData(&hat->ishat);

    this->AddChild(new VisLabel(5, r.left, 0xe4, r.right, 0xfc, txt_patch.GetLine(0x6e), g_font1, p_clrsh_Black, 0));

    VisTextBox* login_box = new VisTextBox(6, r.left, 0xfc, r.right, 0x114, g_font1, p_clrsh_Black, nullptr);
    this->AddChild(login_box);
    login_box->ReadData(hat->login);
    if (hat->ishat == 0) {
        login_box->ChangeFlags(1, false);
    }

    this->AddChild(new VisLabel(7, r.left, 0x12c, r.right, 0x144, txt_patch.GetLine(0x6f), g_font1, p_clrsh_Black, 0));

    VisTextBox* password_box = new VisTextBox(8, r.left, 0x144, r.right, 0x15c, g_font1, p_clrsh_Black, nullptr);
    this->AddChild(password_box);
    password_box->ReadData(hat->password);
    if (hat->ishat == 0) {
        password_box->ChangeFlags(1, false);
    }

    VisRadioType1* store_radio = new VisRadioType1(9, r.left, 0x168, r.right, 0x180, g_font1, p_clrsh_Black, txt_patch.GetLine(0x7b));
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
    : VisMessageBox(_id, l, t, r, b, hat, txt_patch.GetLine(0x67), 1, txt_patch.GetLine(0x66))
{
    this->hat_settings = hat;
}

// 44FD90
VisHatServerListDlg::~VisHatServerListDlg()
{
}

// 43BE9F
void Vis1200::FUN_0043be9f()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    BigStruct2* map = (BigStruct2*)this->parent;
    CSize size = this->rect.Size();
    int32_t total_height = this->rect.Height();
    if (map->IsBookOpen() != 0) {
        total_height += main_wnd->vis_spellbook->GetRect().Height();
    }
    if (map->IsBagOpen() != 0) {
        total_height += main_wnd->vis_invtype1->GetRect().Height();
    }
    CPoint top_left(0, map->GetRect().BottomRight().y - total_height);
    this->rect = CRect(top_left, size);
}

// 43BC3C
void Vis1200::VMethod26()
{
    this->text_block = new Vis1200obj(4, 0, 0, this->rect.Width(), g_font1->GetHeight() * 3, g_font1, clrsh_ShockingBlack, nullptr);
    this->rect.BottomRight().y = this->rect.TopLeft().y + g_font1->GetHeight() * 3 + 4;
    this->AddChild(this->text_block);
    this->active = 0;
}

// 43BDEC
int32_t Vis1200::OnKeyDown(uint32_t wparam)
{
    if (wparam == 0xd) {
        this->text_block->FUN_0043ad86();
        this->MsgProc(0x445, 0, 0);
        return 1;
    }
    if (wparam == 0x1b) {
        this->MsgProc(0x446, 0, 0);
        return 1;
    }
    if (wparam == 0x26) {
        this->text_block->FUN_0043ada3();
    }
    return VisScreen::OnKeyDown(wparam);
}

// 43BD2E
void Vis1200::VMethod28()
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->field_0xc0 = 1;
    memset(this->buffer, 0, 0x100);
    this->text_block->FUN_0043ac8c();
    VisScreen::VMethod28();
    this->active = 1;
    this->text_block->field_0x98 = timeGetTime();
}

// 43BDCB
int32_t Vis1200::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam)
{
    return VisScreen::MsgProc(msg, wparam, lparam);
}

// 43BD96
void Vis1200::DoClose(uint32_t code)
{
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->field_0xc0 = 0;
    this->active = 0;
    VisScreen::DoClose(code);
}

// 43BE80
void Vis1200::VMethod7()
{
    if (this->active != 0) {
        VisScreen::VMethod7();
    }
}

// 4972D0
int32_t Vis1200::FUN_004972d0()
{
    return this->text_block->FUN_004972f0();
}

// 497310
int32_t Vis1200::FUN_00497310()
{
    return this->text_block->FUN_00497330();
}

// 43BE73
void Vis1200::VMethod8(CRect* rect)
{
}

// 43BB70
Vis1200::Vis1200(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : VisScreen(_id, l, t, r, b, nullptr)
{
    this->VMethod26();
}

// 43C000
Vis1200::~Vis1200()
{
}

// 4B12E3
void VisOrderToolbar::VMethod7()
{
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (main_wnd->dialogsMask == 1) {
        LockSurface2();
        if (this->enabled == 0) {
            g_bmp_headsr->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
        } else {
            g_bmp_cmdbarr->VMethod2(screen_rect.left, screen_rect.top, 0, 0, 0);
            for (int32_t i = 0; i < 8; i++) {
                if ((this->avail_orders_mask & (1 << i)) == 0) {
                    int32_t x = (i & 3) * 0x22 + 8;
                    int32_t y = (i >> 2) * 0x22 + 7;
                    g_bmp_cmdempr->VMethod9(screen_rect.left + x, screen_rect.top + y, x, y, x + 0x22, y + 0x22);
                }
            }
            int32_t sel = (int32_t)this->selected_order;
            if (sel >= 0) {
                int32_t x = (sel & 3) * 0x22 + 8;
                int32_t y = (sel >> 2) * 0x22 + 7;
                g_bmp_cmddnr->VMethod9(screen_rect.left + x, screen_rect.top + y, x, y, x + 0x22, y + 0x22);
            }
        }
        UnlockSurface2();
        this->dirty = 0;
    }
}
