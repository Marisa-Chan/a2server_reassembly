#include "visual.h"
#include "server.h"
#include "main_window.h"
#include "gfx.h"
#include "game_app.h"
#include "util.h"




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
                last_focus_obj = nullptr;

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
        CRect local_14 = ClientRectToScreen(rect);

        LockSurface2();

        parent->VMethod8(&local_14);

        int32_t outx = 0;
        int32_t outy = 0;

        if (align_flags & 1)
            outx = local_14.right;
        else if (align_flags & 2)
            outx = (local_14.left + local_14.right) / 2;
        else
            outx = local_14.left;

        if (align_flags & 4)
            outy = local_14.bottom;
        else if (align_flags & 8)
            outy = (local_14.top + local_14.bottom) / 2;
        else
            outy = local_14.top;

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
    int32_t p = val_max * (x - rt.left + 2 + rt.Height()) / (rt.Width() - (rt.Height() * 2 - 8) - 12);
    if (p < 0)
        p = 0;
    if (p > val_max)
        p = val_max;
    return p;
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

/*
void VisMessageBox::VMethod26()
{
    //444086
    CRect r2;
    if (field_0x6c && strlen(field_0x6c) > 0)
    {

    }
}*/


void VisMessageBox::VMethod31(int32_t code)
{
    //445084
}

VisMessageBox::VisMessageBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str1, const char* str2, int32_t btypes, const char* str3)
: VisWindow(_id, l, t, r, b, nullptr)
{
    //44402b
    field_0x70 = str2;
    field_0x6c = str3;
    field_0x68 = str1;
    button_types = btypes;
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
            if (parent_screen->vis_list->field_0xd8 == 0)
#ifdef A2CLIENT
            {
                int32_t idx = parent_screen->vis_list->field_0xd0;
                if (idx == parent_screen->pCharacters->GetStringArray1Size() - 1 &&
                    mwnd->field_0x3f0 != 0 && mwnd->field_0x3b8 != 0 &&
                    parent_screen->pCharacters->field_0x130.GetSize() > 15)
                {
                    VisScreen* mbox = new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(141), nullptr, 0x2000);
                    mwnd->field_0x3dc = mbox;
                    mwnd->ModalScreen(mbox);
                }
                else if (idx == parent_screen->pCharacters->GetStringArray1Size() - 1 &&
                    mwnd->field_0x3f0 != 0 && mwnd->field_0x3b8 != 0 && mwnd->field_0x3c4 != 0)
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
            if (parent_screen->vis_list->field_0xd8 == 0)
            {
                int32_t idx = parent_screen->vis_list->field_0xd0;
                if (idx != parent_screen->pCharacters->GetStringArray1Size() - 1)
                {
                    CString str = CString(txt_patch.GetLine(93)) + CString(parent_screen->pCharacters->character_name) + CString(txt_patch.GetLine(94));
                    VisScreen* box = new VisMessageBoxWithList(1, 64, 100, 380, 594, str, nullptr, 4);
                    mwnd->field_0x3dc = box;
                    mwnd->ModalScreen(box);

                    if (box->GetCloseCode() == 0x447)
                    {
                        parent_screen->pCharacters->FUN_00493cd8();

                        if (idx < parent_screen->pCharacters->GetStringArray1Size() - 1)
                        {
                            parent_screen->pCharacters->FUN_00492c66(idx);
                            parent_screen->vis_stats->FUN_0042f6f3();
                            parent_screen->FUN_00432655(parent_screen->field_0x84);
                        }
                        else
                        {
                            parent_screen->vis_list->field_0xd0 = parent_screen->pCharacters->GetStringArray1Size() - 1;
                            parent_screen->field_0x84->VMethod1(0);
                            parent_screen->gameinterface->FUN_00416cf7();
                        }
                    }
                }
            }
            break;

        case 2:
            if (parent_screen->vis_list->field_0xd8 == 0)
            {
                parent_screen->OpenRenameWindow();
                CSound::Play(parent_screen->field_0x8c);
            }
            break;

        case 3:
            if (parent_screen->vis_list->field_0xd8 == 0)
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

