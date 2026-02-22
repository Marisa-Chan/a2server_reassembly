#include "visual.h"
#include "server.h"


CGameFont::CGameFont()
{
    //45e0d0
    bitmap = nullptr;
    char_widths = nullptr;
}

CGameFont::~CGameFont()
{
    //45e0f9
    if (char_widths)
        free(char_widths);

    if (bitmap)
        delete bitmap;
}

void CGameFont::DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colosh)
{
    //460f60
}

uint16_t* CGameFont::GetShadowColors()
{
    //460f70
    return nullptr;
}

void CGameFont::DrawTextWithShadow(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colorsh, int32_t shadow_dxy)
{
    //45ebda
    uint16_t* shdow = GetShadowColors();
    DrawTxt(x + shadow_dxy, y + shadow_dxy, txt, align, shdow);
    DrawTxt(x, y, txt, align, colorsh);
}






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
    name = "";
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
        name = str;

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
        name = str;

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


const char* CVisualObject::GetName()
{
    //4d7c15

    if (TestFlags(FLAG_20))
        return nullptr;

    return name;
}

void CVisualObject::SetName(const char* _name)
{
    //4d7bf9

    name = _name;
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


void CVisualObject::VMethod8(RECT* rect)
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
    CRect r;
    ClientRectToScreen(&r, rect);
    FUN_00454c74(&r);
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


void CVisualObject::ReadData(void* buf)
{
    //4d7e02

    for (uint32_t i = 0; i < childs.GetSize(); i++)
    {
        CVisualObject* obj = childs[i];
        obj->ReadData(buf);
        buf = (uint8_t*)buf + obj->DataSize();
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

        CRect r;
        obj->ClientRectToScreen(&r, obj->rect);

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

    CRect r;
    ClientRectToScreen(&r, rect);
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
        CRect local_14;
        ClientRectToScreen(&local_14, rect);

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