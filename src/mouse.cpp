#include "mouse.h"
#include "gfx.h"
#include "game_app.h"
#include "main_window.h"

CMousePointer g_mousept; //642c68
CCursor* g_Cursors[28]; //665420

//GetRuntimeClass 423b5f
IMPLEMENT_DYNAMIC(CMousePointer, CObject);

CMousePointer::~CMousePointer()
{
    //42693e
    if (saved_screen_rect)
        delete saved_screen_rect;

    if (saved_screen_rect2)
        delete saved_screen_rect2;

    if (saved_screen_rect_hint)
        delete saved_screen_rect_hint;

    for (int i = 0; i < 8; i++)
    {
        if (saved_rect_frame[i])
            delete saved_rect_frame[i];
    }

    if (hint_image)
        delete hint_image;
}

CMousePointer::CMousePointer()
{
	//4264eb
    cursor_sprite = nullptr;
    saved_screen_rect = nullptr;
    paint_call_count = 0;
    x = 0;
    y = 0;
    x_offset = 0;
    y_offset = 0;
    frame = 0;

    for (int i = 0; i < 8; i++)
    {
        if ((i & 2) == 0)
            saved_rect_frame[i] = new CBmp64(1600, 2);
        else
            saved_rect_frame[i] = new CBmp64(2, 1600);
    }

    hint_image = new CBmp64(640, 117);
    saved_screen_rect_hint = new CBmp64(640, 117);

    mouse_hint_time = 0;
    mouse_hint_ts = 0;
    hint_state = 0;
    hint_enable = 0;
}

void CMousePointer::CheckHintValues()
{
    //42709f
    CString ht = hint_text;
    hint_lines = 1;
    hint_max_width = 0;

    while (true)
    {
        int fpos = ht.Find('#');
        if (fpos < 0)
            break;

        int32_t sw = g_font2->GetStrWidth(ht.Left(fpos));
        if (hint_max_width < sw)
            hint_max_width = sw;

        ht = ht.Mid(fpos + 1);

        hint_lines++;
    }

    int32_t sw = g_font2->GetStrWidth(ht);
    if (hint_max_width < sw)
        hint_max_width = sw;
}

void CMousePointer::DrawToScreen()
{
    //426ce2
    uint32_t ts = timeGetTime();
    if (frame_delay < ts - frame_ts)
    {
        frame_ts = ts;
        frame++;
        if (frame >= frame_count)
            frame = 0;
    }

    LockSurface1();

    CopyScreenRectToBmp64(x - x_offset, y - y_offset, cursor_sprite->GetWidth(0), cursor_sprite->GetHeight(0), cursor_sprite->GetWidth(0), cursor_sprite->GetHeight(0), saved_screen_rect->GetData());
    cursor_sprite->VMethod2(x - x_offset, y - y_offset, frame, 0, 0);

    UnlockSurface1();
}

void CMousePointer::EraseCursor()
{
    //426df7
    LockSurface1();
    saved_screen_rect->VMethod9(x - x_offset, y - y_offset, 0, 0, cursor_sprite->GetWidth(0), cursor_sprite->GetHeight(0));
    UnlockSurface1();
}

void CMousePointer::ForceDisableHint()
{
    //4287d9
    if (hint_state)
    {
        hint_state = 0;
        RepaintHint();
    }
}

void CMousePointer::Paint()
{
    //426ca3
    if (g_IsServer)
        return;

    if (paint_call_count == 0 && cursor_sprite)
        DrawToScreen();
    
    paint_call_count++;
}

void CMousePointer::PaintHint()
{
    //4282bb
    CRect r = hint_rect + CRect(1, 1, -1, -2);

    FillRectColorSimple(r.left, r.top, r.right, r.bottom, GetColorRGB(0x24, 0x2c, 0x27));

    uint32_t c1 = GetColorRGB(0xa0, 0x78, 0x32);
    uint32_t c2 = GetColorRGB(0x50, 0x3c, 0x18);

    gfx_ball->VMethod10(r.left - 1, r.top - 1, 0, 0, 4, 4);
    gfx_ball->VMethod10(r.right - 2, r.top - 1, 0, 0, 4, 4);
    gfx_ball->VMethod10(r.left - 1, r.bottom - 2, 0, 0, 4, 4);
    gfx_ball->VMethod10(r.right - 2, r.bottom - 2, 0, 0, 4, 4);

    FillRectColor(r.left + 2, r.top, r.right - 2, r.top, c1);
    FillRectColor(r.left + 2, r.top + 1, r.right - 2, r.top + 1, c2);
    FillRectColor(r.left + 2, r.bottom - 1, r.right - 2, r.bottom - 1, c1);
    FillRectColor(r.left + 2, r.bottom, r.right - 2, r.bottom, c2);
    FillRectColor(r.left, r.top + 2, r.left, r.bottom - 2, c1);
    FillRectColor(r.left + 1, r.top + 2, r.left + 1, r.bottom - 2, c2);
    FillRectColor(r.right - 1, r.top + 2, r.right - 1, r.bottom - 2, c1);
    FillRectColor(r.right, r.top + 2, r.right, r.bottom - 2, c2);

    CString ht = hint_text;
    
    int line = 0;

    while (true)
    {
        int fpos = ht.Find('#');
        if (fpos < 0)
            break;

        g_font2->DrawTextWithShadow(hint_rect.left + 5, hint_rect.top + 4 + line * 14, ht.Left(fpos), 0, clrsh_LuxorGold, 1);

        ht = ht.Mid(fpos + 1);

        line++;
    }

    g_font2->DrawTextWithShadow(hint_rect.left + 5, hint_rect.top + 4 + line * 14, ht, 0, clrsh_LuxorGold, 1);
}

void CMousePointer::PaintSelectFrame()
{
    //427e71
    if (select_frame_rect.Width() && select_frame_rect.Height())
    {
        CRect r = select_frame_rect;

        r.NormalizeRect();

        LockSurface1();

        if (r.Width() > 0)
        {
            if (r.Height() > 0)
            {
                CopyScreenRectToBmp64(r.left, r.top, r.Width(), 1, 1600, 2, saved_rect_frame[0]->GetData());
                CopyScreenRectToBmp64(r.left, r.bottom - 1, r.Width(), 1, 1600, 2, saved_rect_frame[1]->GetData());
            }

            if (r.Height() > 2)
            {
                CopyScreenRectToBmp64(r.left, r.top + 1, 2, r.Width() - 2, 2, 1600, saved_rect_frame[2]->GetData());
                CopyScreenRectToBmp64(r.right - 1, r.top + 1, 2, r.Width() - 2, 2, 1600, saved_rect_frame[3]->GetData());
            }
        }

        DrawRectangleFrame(r.left, r.top, r.right - 1, r.bottom - 1, GetColorRGB(0xff, 0xff, 0xff));

        UnlockSurface1();
    }
}

void CMousePointer::Repaint(CPoint point)
{
    //426eb6

    mouse_hint_time = 0;
    mouse_hint_ts = timeGetTime();

    if (hint_state != 0)
        RepaintHint();

    hint_state = 0;

    Unpaint();

    if (select_frame_state != 0)
        UnpaintSelectFrame();

    x = point.x;
    y = point.y;

    select_frame_rect.SetRect(select_frame_rect.left, select_frame_rect.top, x, y);

    if (select_frame_state != 0)
        PaintSelectFrame();
    
    Paint();
}

void CMousePointer::RepaintHint()
{
    //428779
    Unpaint();
    LockSurface1();

    hint_image->VMethod9(hint_rect.left, hint_rect.top, 0, 0, hint_rect.Width(), hint_rect.Height());

    UnlockSurface1();
    Paint();
}

void CMousePointer::ResetNext400msg()
{
    //42707e
    user_msg_ts = timeGetTime();
    user_msg_delay = 150;
}

void CMousePointer::ResetStates()
{
    //427e23
    Unpaint();
    UnpaintSelectFrame();
    Paint();

    select_frame_state = 0;
    mouse_hint_time = 0;
    mouse_hint_ts = timeGetTime();
}

void CMousePointer::SaveScreenParts(const CRect& rect)
{
    //427912
    if (cursor_sprite && paint_call_count)
    {
        CRect tmp(x - x_offset, y - y_offset, (x - x_offset) + cursor_sprite->GetWidth(0), (y - y_offset) + cursor_sprite->GetHeight(0));

        CRect r = select_frame_rect;
        r.NormalizeRect();

        CRect intr;
        if (intr.IntersectRect(&rect, &tmp) != 0)
        {
            LockSurface2();

            saved_screen_rect2->VMethod9(x - x_offset, y - y_offset, 0, 0, cursor_sprite->GetWidth(0), cursor_sprite->GetHeight(0));
            CopyScreenRectToBmp64(x - x_offset, y - y_offset, 
                                  cursor_sprite->GetWidth(0), cursor_sprite->GetHeight(0), 
                                  cursor_sprite->GetWidth(0), cursor_sprite->GetHeight(0), 
                                  saved_screen_rect->GetData());
            UnlockSurface2();
        }


        if (intr.IntersectRect(&rect, &hint_rect) && hint_state)
        {
            LockSurface2();

            saved_screen_rect_hint->VMethod9(hint_rect.left, hint_rect.top, 0, 0, hint_rect.Width(), hint_rect.Height());
            CopyScreenRectToBmp64(hint_rect.left, hint_rect.top, 
                                    hint_rect.Width(), hint_rect.Height(), 
                                    hint_image->GetWidth(0), hint_image->GetHeight(0), 
                                    hint_image->GetData());
            UnlockSurface2();

            return;
        }

        if (intr.IntersectRect(&rect, &r) && select_frame_state)
        {
            LockSurface2();

            if (r.Width() > 0)
            {
                if (r.Height() > 0)
                {
                    saved_rect_frame[4]->VMethod9(r.left, r.top, 0, 0, r.Width(), 1);
                    saved_rect_frame[5]->VMethod9(r.left, r.bottom - 1, 0, 0, r.Width(), 1);
                }

                if (r.Height() > 2)
                {
                    saved_rect_frame[6]->VMethod9(r.left, r.top + 1, 0, 0, 1, r.Height() - 2);
                    saved_rect_frame[7]->VMethod9(r.right - 1, r.top + 1, 0, 0, 1, r.Height() - 2);
                }
            }

            CopyScreenRectToBmp64(r.left, r.top, r.Width(), 1, 1600, 2, saved_rect_frame[0]->GetData());
            CopyScreenRectToBmp64(r.left, r.bottom - 1, r.Width(), 1, 1600, 2, saved_rect_frame[1]->GetData());

            if (r.Height() > 2)
            {
                CopyScreenRectToBmp64(r.left, r.top + 1, 2, r.Height() - 2, 2, 1600, saved_rect_frame[2]->GetData());
                CopyScreenRectToBmp64(r.right - 1, r.top + 1, 2, r.Height() - 2, 2, 1600, saved_rect_frame[3]->GetData());
            }

            UnlockSurface2();
        }
    }
}

void CMousePointer::SaveScreenParts2(const CRect& rect)
{
    //42751c
    if (cursor_sprite && paint_call_count)
    {
        CRect tmp(x - x_offset, y - y_offset, (x - x_offset) + cursor_sprite->GetWidth(0), (y - y_offset) + cursor_sprite->GetHeight(0));

        CRect r = select_frame_rect;
        r.NormalizeRect();

        CRect intr;
        if (intr.IntersectRect(&rect, &hint_rect) && hint_state)
        {
            LockSurface2();

            CopyScreenRectToBmp64(hint_rect.left, hint_rect.top, hint_rect.Width(), hint_rect.Height(),
                                    saved_screen_rect_hint->GetWidth(0), saved_screen_rect_hint->GetHeight(0),
                                    saved_screen_rect_hint->GetData());

            UnlockSurface2();
        }
        else if (intr.IntersectRect(&rect, &r) && select_frame_state)
        {
            LockSurface2();

            if (r.Width() > 0)
            {
                if (r.Height() > 0)
                {
                    CopyScreenRectToBmp64(r.left, r.top, r.Width(), 1, 1600, 2, saved_rect_frame[4]->GetData());
                    CopyScreenRectToBmp64(r.left, r.bottom - 1, r.Width(), 1, 1600, 2, saved_rect_frame[5]->GetData());
                }

                if (r.Height() > 2)
                {
                    CopyScreenRectToBmp64(r.left, r.top + 1, 2, r.Height() - 2, 2, 1600, saved_rect_frame[6]->GetData());
                    CopyScreenRectToBmp64(r.right - 1, r.top + 1, 2, r.Height() - 2, 2, 1600, saved_rect_frame[7]->GetData());
                }
            }

            DrawRectangleFrame(r.left, r.top, r.right, r.bottom, GetColorRGB(0xff, 0xff, 0xff));

            UnlockSurface2();
        }

        if (intr.IntersectRect(&rect, &tmp))
        {
            LockSurface2();
            CopyScreenRectToBmp64(x - x_offset, y - y_offset, cursor_sprite->GetWidth(0), cursor_sprite->GetHeight(0),
                                    cursor_sprite->GetWidth(0), cursor_sprite->GetHeight(0),
                                    saved_screen_rect2->GetData());
            cursor_sprite->VMethod2(x - x_offset, y - y_offset, frame, 0, 0);
            UnlockSurface2();
        }
    }
}

void CMousePointer::SetCursor(CSprite256* spr, int32_t off_x, int32_t off_y, int32_t count, int32_t delay)
{
    //426aa5
    if (g_IsServer)
        return;

    Unpaint();

    if (saved_screen_rect)
        delete saved_screen_rect;

    if (saved_screen_rect2)
        delete saved_screen_rect2;

    cursor_sprite = spr;
    x_offset = off_x;
    y_offset = off_y;
    frame = 0;
    frame_ts = 0;
    user_msg_ts = 0;
    frame_delay = delay;
    frame_count = count;

    saved_screen_rect = new CBmp64(spr->GetWidth(0), spr->GetHeight(0));
    saved_screen_rect2 = new CBmp64(spr->GetWidth(0), spr->GetHeight(0));

    Paint();
}

void CMousePointer::StartSelect(const CPoint& point)
{
    //427db1
    select_frame_state = 1;
    select_frame_rect.SetRect(point, point);

    if (hint_state)
    {
        hint_state = 0;
        RepaintHint();
    }

    Unpaint();
    PaintSelectFrame();
    Paint();
}
void CMousePointer::Unpaint()
{
    //426c64
    if (g_IsServer)
        return;

    paint_call_count--;

    if (paint_call_count == 0 && cursor_sprite)
        EraseCursor();
}

void CMousePointer::UnpaintSelectFrame()
{
    //428051
    if (select_frame_rect.Width() == 0 || select_frame_rect.Height() == 0)
        return;

    CRect r = select_frame_rect;
    r.NormalizeRect();

    LockSurface1();

    if (r.Width() > 0)
    {
        if (r.Height() > 0)
        {
            saved_rect_frame[0]->VMethod9(r.left, r.top, 0, 0, r.Width(), 1);
            saved_rect_frame[1]->VMethod9(r.left, r.bottom - 1, 0, 0, r.Width(), 1);
        }

        if (r.Height() > 2)
        {
            saved_rect_frame[2]->VMethod9(r.left, r.top + 1, 0, 0, 1, r.Height() - 2);
            saved_rect_frame[3]->VMethod9(r.right - 1, r.top + 1, 0, 0, 1, r.Height() - 2);
        }
    }

    UnlockSurface1();
}

void CMousePointer::Update()
{
    //4271e6
    if (g_IsServer || GetLockCountSurf2())
        return;

    uint32_t ts = timeGetTime();

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

    if (GetAsyncKeyState(VK_RBUTTON) > -1 && wnd->field_0xbc != 0)
    {
        POINT pt;
        GetCursorPos(&pt);

        if ((pt.x != x  || pt.y != y) && cursor_sprite )
            Repaint(pt);
    }

    if (user_msg_delay < ts - user_msg_ts && g_LButtonDown != 0)
    {
        wnd->PostMessage(0x400, 1, (x & 0xFFFF) | (y << 16));

        user_msg_ts = ts;
        user_msg_delay = 66;
    }

    if (frame_delay < ts - frame_ts && frame_count > 1 && wnd && wnd->field_0xbc != 0)
    {
        frame_ts = ts;
        frame++;

        if (frame >= frame_count)
            frame = 0;

        Unpaint();
        Paint();
    }

    int32_t prev_time = mouse_hint_time;

    mouse_hint_time += ts - mouse_hint_ts;
    mouse_hint_ts = ts;

    if (mouse_hint_time >= 500 && mouse_hint_time < 25500 && 
        prev_time < 500 &&
        select_frame_state == 0 && 
        hint_enable != 0 &&
        wnd && wnd->vis_root && wnd->field_0xbc != 0)
    {
        CVisualObject* vobj = wnd->vis_root->GetChildAt(CPoint(x, y));
        if (vobj)
        {
            hint_text = vobj->GetHint();
            if (hint_text != "")
            {
                hint_state = 1;
                CheckHintValues();
                UpdateHint();
            }
        }
    }

    if (mouse_hint_time > 25499 && hint_state != 0 && 
        wnd && wnd->field_0xbc != 0)
    {
        hint_state = 0;
        RepaintHint();
    }

    if (wnd->music_update_proc)
        wnd->music_update_proc();
}

void CMousePointer::UpdateHint()
{
    //42819d
    hint_rect.SetRect(x, y - 5 - hint_lines * 14, x + 11 + hint_max_width, y);

    if (hint_rect.right > g_ScreenSize.right)
        hint_rect.OffsetRect(g_ScreenSize.right - hint_rect.right, 0);

    if (hint_rect.top < g_ScreenSize.top)
        hint_rect.OffsetRect(0, g_ScreenSize.top - hint_rect.top);

    Unpaint();
    LockSurface1();
    CopyScreenRectToBmp64(hint_rect.left, hint_rect.top, hint_rect.Width(), hint_rect.Height(), hint_image->GetWidth(0), hint_image->GetHeight(0), hint_image->GetData());
    PaintHint();
    UnlockSurface1();

    Paint();
}


void CMousePointer::DisableHint() { hint_enable = 0; } //438830
void CMousePointer::EnableHint() { hint_enable = 1; } //438810
CSprite256* CMousePointer::GetCursorSprite() { return cursor_sprite; } //438810
CRect& CMousePointer::GetSelectFrame() { return select_frame_rect; } //41ed20
int32_t CMousePointer::GetSelectState() { return select_frame_state; } //41ed00
int32_t CMousePointer::GetX() { return x; } //41ecc0
int32_t CMousePointer::GetY() { return y; } //41ece0


CCursor::~CCursor()
{
    //47cf99
    if (sprite)
        delete sprite;
    sprite = nullptr;
}

CCursor::CCursor(const char* fname, int32_t xof, int32_t yof, int32_t _delay)
{
    //47ce66
    if (strstr(fname, "16a") == nullptr)
    {
        sprite = new CSprite256(fname);
        sprite->ResetPalette(1, 1, 0);
    }
    else
    {
        sprite = new CA16(fname);
        sprite->ResetPalette(16, 4, 0);
    }

    x_off = xof;
    y_off = yof;
    delay = _delay;
    frames_count = sprite->GetFrameCount();
}

void CCursor::Use()
{
    //47d025
    g_mousept.SetCursor(sprite, x_off, y_off, frames_count, delay);
}



void LoadCursors()
{
    //47d05d
    g_mousept.Update();

    g_Cursors[0] = new CCursor("graphics\\cursors\\default\\sprites.16a", 4, 4, 2000000000);
    g_Cursors[1] = new CCursor("graphics\\cursors\\move\\sprites.16a", 2, 3, 100);
    g_Cursors[2] = new CCursor("graphics\\cursors\\swarm\\sprites.16a", 2, 3, 100);
    g_Cursors[3] = new CCursor("graphics\\cursors\\attack\\sprites.16a", 3, 3, 100);
    g_Cursors[4] = new CCursor("graphics\\cursors\\defend\\sprites.16a", 16, 16, 100);
    g_Cursors[5] = new CCursor("graphics\\cursors\\select\\sprites.16a", 3, 3, 100);
    g_Cursors[6] = new CCursor("graphics\\cursors\\patrol\\sprites.16a", 8, 25, 100);
    g_Cursors[7] = new CCursor("graphics\\cursors\\cast\\sprites.16a", 16, 16, 100);
    g_Cursors[8] = new CCursor("graphics\\cursors\\pickup\\sprites.16a", 16, 12, 66);
    g_Cursors[9] = new CCursor("graphics\\cursors\\arrow0\\sprites.16a", 16, 6, 2000000000);
    g_Cursors[10] = new CCursor("graphics\\cursors\\arrow4\\sprites.16a", 16, 28, 2000000000);
    g_Cursors[11] = new CCursor("graphics\\cursors\\arrow6\\sprites.16a", 5, 16, 2000000000);
    g_Cursors[12] = new CCursor("graphics\\cursors\\arrow2\\sprites.16a", 26, 16, 2000000000);
    g_Cursors[13] = new CCursor("graphics\\cursors\\arrow7\\sprites.16a", 8, 9, 2000000000);
    g_Cursors[14] = new CCursor("graphics\\cursors\\arrow5\\sprites.16a", 8, 24, 2000000000);
    g_Cursors[15] = new CCursor("graphics\\cursors\\arrow1\\sprites.16a", 23, 9, 2000000000);
    g_Cursors[16] = new CCursor("graphics\\cursors\\arrow3\\sprites.16a", 23, 24, 2000000000);
    g_Cursors[17] = new CCursor("graphics\\cursors\\sdefault\\sprites.16a", 1, 1, 2000000000);
    g_Cursors[18] = new CCursor("graphics\\cursors\\smove.256", 0, 0, 2000000000);
    g_Cursors[19] = new CCursor("graphics\\cursors\\sattack.256", 0, 0, 2000000000);
    g_Cursors[20] = new CCursor("graphics\\cursors\\sdefend.256", 0, 0, 2000000000);
    g_Cursors[21] = new CCursor("graphics\\cursors\\spatrol.256", 0, 0, 2000000000);
    g_Cursors[22] = new CCursor("graphics\\cursors\\scast.256", 0, 0, 2000000000);
    g_Cursors[23] = new CCursor("graphics\\cursors\\cantput\\sprites.16a", 38, 36, 2000000000);
    g_Cursors[24] = new CCursor("graphics\\cursors\\town\\sprites.16a", 16, 16, 2000000000);
    g_Cursors[25] = new CCursor("graphics\\cursors\\dice\\sprites.16a", 16, 16, 100);
    g_Cursors[26] = new CCursor("graphics\\cursors\\wait\\sprites.16a", 16, 16, 100);
    g_Cursors[27] = new CCursor("graphics\\cursors\\backpack\\sprites.16a", 16, 16, 100);

    g_mousept.Update();
}

void DeleteCursors()
{
    //47daf0
    for (int i = 0; i < 28; i++)
        delete g_Cursors[i];
}

int __cdecl ApplyCursor(CCursor* cur)
{
    //4750d0
    if (!cur)
        return 0;

    if (g_mousept.GetCursorSprite() == cur->GetSprite())
        return 0;
    
    cur->Use();
    return 1;
}