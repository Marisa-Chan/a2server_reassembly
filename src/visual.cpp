#include "visual.h"
#include "server.h"
#include "main_window.h"
#include "gfx.h"
#include "mouse.h"
#include "game_app.h"
#include "quest_map.h"
#include "util.h"


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
                            parent_screen->vis_stats->FUN_0042f6f3();
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
