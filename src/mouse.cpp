#include <algorithm>
#include "mouse.h"
#include "gfx.h"
#include "game_app.h"
#include "main_window.h"

#ifdef A2CLIENT_PATCH
constexpr int32_t kSelectFrameBufLen = 8192;

namespace {

constexpr int32_t kMaxDeferredMouseRects = 32;
int32_t g_mouse_present_batch_depth = 0;
int32_t g_deferred_mouse_rect_count = 0;
CRect g_deferred_mouse_rects[kMaxDeferredMouseRects];

int64_t PaddedRectCost(const CRect& r) {
    return static_cast<int64_t>(r.Width() + 6) * static_cast<int64_t>(r.Height() + 6);
}

void QueueOrPresentMouseRect(const CRect& rect) {
    CRect r;
    if (!r.IntersectRect(&rect, &g_ScreenSize)) {
        return;
    }
    if (g_mouse_present_batch_depth <= 0) {
        PresentPrimarySurfaceRect(r);
        return;
    }
    if (g_deferred_mouse_rect_count < kMaxDeferredMouseRects) {
        g_deferred_mouse_rects[g_deferred_mouse_rect_count++] = r;
    } else {
        g_deferred_mouse_rects[kMaxDeferredMouseRects - 1].UnionRect(
            &g_deferred_mouse_rects[kMaxDeferredMouseRects - 1], &r);
    }
}

void FlushDeferredMouseRects() {
    bool merged = true;
    while (merged) {
        merged = false;
        for (int32_t i = 0; i < g_deferred_mouse_rect_count && !merged; i++) {
            for (int32_t j = i + 1; j < g_deferred_mouse_rect_count; j++) {
                const CRect& a = g_deferred_mouse_rects[i];
                const CRect& b = g_deferred_mouse_rects[j];
                CRect u((std::min)(a.left, b.left), (std::min)(a.top, b.top),
                        (std::max)(a.right, b.right), (std::max)(a.bottom, b.bottom));
                if (PaddedRectCost(u) <= PaddedRectCost(a) + PaddedRectCost(b) + 512) {
                    g_deferred_mouse_rects[i] = u;
                    g_deferred_mouse_rects[j] = g_deferred_mouse_rects[--g_deferred_mouse_rect_count];
                    merged = true;
                    break;
                }
            }
        }
    }

    int32_t count = g_deferred_mouse_rect_count;
    g_deferred_mouse_rect_count = 0;
    for (int32_t i = 0; i < count; i++) {
        PresentPrimarySurfaceRect(g_deferred_mouse_rects[i]);
    }
}

struct ScopedMousePresentBatch {
    ScopedMousePresentBatch() { ++g_mouse_present_batch_depth; }
    ~ScopedMousePresentBatch() {
        if (--g_mouse_present_batch_depth == 0 && g_deferred_mouse_rect_count > 0) {
            FlushDeferredMouseRects();
        }
    }
};

} // namespace
#else
constexpr int32_t kSelectFrameBufLen = 1600;
#endif

CMousePointer g_mousept; //642c68
CCursor* g_Cursors[28]; //665420

//GetRuntimeClass 423b5f
IMPLEMENT_DYNAMIC(CMousePointer, CObject);

// 42693e (client: 42886c)
CMousePointer::~CMousePointer() {
    if (this->saved_screen_rect) {
        delete this->saved_screen_rect;
    }

    if (this->saved_screen_rect2) {
        delete this->saved_screen_rect2;
    }

    if (this->saved_screen_rect_hint) {
        delete this->saved_screen_rect_hint;
    }

    for (int i = 0; i < 8; i++) {
        if (this->saved_rect_frame[i]) {
            delete this->saved_rect_frame[i];
        }
    }

    if (this->hint_image) {
        delete this->hint_image;
    }
}

// 4264eb (client: 428419)
CMousePointer::CMousePointer() {
    this->cursor_sprite = nullptr;
    this->saved_screen_rect = nullptr;
    this->paint_call_count = 0;
    this->x = 0;
    this->y = 0;
    this->x_offset = 0;
    this->y_offset = 0;
    this->frame = 0;

    for (int32_t i = 0; i < 8; i++) {
        if ((i & 2) == 0) {
            this->saved_rect_frame[i] = new CBmp64(kSelectFrameBufLen, 2);
        } else {
            this->saved_rect_frame[i] = new CBmp64(2, kSelectFrameBufLen);
        }
    }

#ifdef A2CLIENT_PATCH
    // p10iti.xck (42859b, 4285dd -> 6e2213): enlarge tooltip backing bitmaps from 640x117 to 640x453
    this->hint_image = new CBmp64(640, 453);
    this->saved_screen_rect_hint = new CBmp64(640, 453);
#else
    this->hint_image = new CBmp64(640, 117);
    this->saved_screen_rect_hint = new CBmp64(640, 117);
#endif

    this->mouse_hint_time = 0;
    this->mouse_hint_ts = 0;
    this->hint_state = 0;
    this->hint_enable = 0;
}

// 42709f (client: 428fcd)
void CMousePointer::CheckHintValues() {
    CString ht = this->hint_text;
    this->hint_lines = 1;
    this->hint_max_width = 0;

    while (true) {
        int fpos = ht.Find('#');
        if (fpos < 0) {
            break;
        }

        int32_t sw = g_font2->GetStrWidth(ht.Left(fpos));
        if (this->hint_max_width < sw) {
            this->hint_max_width = sw;
        }

        ht = ht.Mid(fpos + 1);

        this->hint_lines++;
    }

    int32_t sw = g_font2->GetStrWidth(ht);
    if (this->hint_max_width < sw) {
        this->hint_max_width = sw;
    }
}

// 426ce2 (client: 428c10)
void CMousePointer::DrawToScreen() {
    uint32_t ts = timeGetTime();
    if (this->frame_delay < ts - this->frame_ts) {
        this->frame_ts = ts;
        this->frame++;
        if (this->frame >= this->frame_count) {
            this->frame = 0;
        }
    }

    LockPrimarySurface();

#ifdef A2CLIENT_PATCH
    int32_t cx = this->x - this->x_offset;
    int32_t cy = this->y - this->y_offset;
    int32_t cw = this->cursor_sprite->GetWidth(0);
    int32_t ch = this->cursor_sprite->GetHeight(0);
    CopyScreenRectToBmp64(cx, cy, cw, ch, cw, ch, (uint8_t*)this->saved_screen_rect->GetData());
    this->cursor_sprite->Draw(cx, cy, this->frame, 0, 0);

    UnlockPrimarySurface();
    QueueOrPresentMouseRect(CRect(cx, cy, cx + cw, cy + ch));
#else
    CopyScreenRectToBmp64(this->x - this->x_offset, this->y - this->y_offset, this->cursor_sprite->GetWidth(0), this->cursor_sprite->GetHeight(0), this->cursor_sprite->GetWidth(0), this->cursor_sprite->GetHeight(0), (uint8_t*)this->saved_screen_rect->GetData());
    this->cursor_sprite->Draw(this->x - this->x_offset, this->y - this->y_offset, this->frame, 0, 0);

    UnlockPrimarySurface();
#endif
}

// 426df7 (client: 428d25)
void CMousePointer::EraseCursor() {
    LockPrimarySurface();
#ifdef A2CLIENT_PATCH
    int32_t cx = this->x - this->x_offset;
    int32_t cy = this->y - this->y_offset;
    int32_t cw = this->cursor_sprite->GetWidth(0);
    int32_t ch = this->cursor_sprite->GetHeight(0);
    this->saved_screen_rect->DrawSubRect(cx, cy, 0, 0, cw, ch);
    UnlockPrimarySurface();
    QueueOrPresentMouseRect(CRect(cx, cy, cx + cw, cy + ch));
#else
    this->saved_screen_rect->DrawSubRect(this->x - this->x_offset, this->y - this->y_offset, 0, 0, this->cursor_sprite->GetWidth(0), this->cursor_sprite->GetHeight(0));
    UnlockPrimarySurface();
#endif
}

// 4287d9 (client: 42a6f5)
void CMousePointer::ForceDisableHint() {
    if (this->hint_state) {
        this->hint_state = 0;
        this->RepaintHint();
    }
}

// 426ca3 (client: 428bd1)
void CMousePointer::Paint() {
    if (g_IsServer) {
        return;
    }

    if (this->paint_call_count == 0 && this->cursor_sprite) {
        this->DrawToScreen();
    }
    
    this->paint_call_count++;
}

// 4282bb (client: 42a1e9)
void CMousePointer::PaintHint() {
    CRect r = this->hint_rect;
    r.DeflateRect(1, 1, 1, 2);

    FillRectColorSimple(r.left, r.top, r.right, r.bottom, GetColorRGB(0x24, 0x2c, 0x27));

    uint32_t c1 = GetColorRGB(0xa0, 0x78, 0x32);
    uint32_t c2 = GetColorRGB(0x50, 0x3c, 0x18);

    gfx_ball->DrawSubRectMasked(r.left - 1, r.top - 1, 0, 0, 4, 4);
    gfx_ball->DrawSubRectMasked(r.right - 2, r.top - 1, 0, 0, 4, 4);
    gfx_ball->DrawSubRectMasked(r.left - 1, r.bottom - 2, 0, 0, 4, 4);
    gfx_ball->DrawSubRectMasked(r.right - 2, r.bottom - 2, 0, 0, 4, 4);

    FillRectColor(r.left + 2, r.top, r.right - 2, r.top, c1);
    FillRectColor(r.left + 2, r.top + 1, r.right - 2, r.top + 1, c2);
    FillRectColor(r.left + 2, r.bottom - 1, r.right - 2, r.bottom - 1, c1);
    FillRectColor(r.left + 2, r.bottom, r.right - 2, r.bottom, c2);
    FillRectColor(r.left, r.top + 2, r.left, r.bottom - 2, c1);
    FillRectColor(r.left + 1, r.top + 2, r.left + 1, r.bottom - 2, c2);
    FillRectColor(r.right - 1, r.top + 2, r.right - 1, r.bottom - 2, c1);
    FillRectColor(r.right, r.top + 2, r.right, r.bottom - 2, c2);

    CString ht = this->hint_text;
    
    int line = 0;

    while (true) {
        int fpos = ht.Find('#');
        if (fpos < 0) {
            break;
        }

        g_font2->DrawTextWithShadow(this->hint_rect.left + 5, this->hint_rect.top + 4 + line * 14, ht.Left(fpos), CGameFont::ALIGN_LEFT_TOP, clrsh_LuxorGold, 1);

        ht = ht.Mid(fpos + 1);

        line++;
    }

    g_font2->DrawTextWithShadow(this->hint_rect.left + 5, this->hint_rect.top + 4 + line * 14, ht, CGameFont::ALIGN_LEFT_TOP, clrsh_LuxorGold, 1);
}

// 427e71 (client: 429d9f)
void CMousePointer::PaintSelectFrame() {
    if (this->select_frame_rect.Width() && this->select_frame_rect.Height()) {
        CRect r = this->select_frame_rect;

        r.NormalizeRect();

        LockPrimarySurface();

        if (r.Width() > 0) {
#ifdef A2CLIENT_PATCH
            int32_t hl = (std::max)(r.left, g_ScreenSize.left);
            int32_t hr = (std::min)(r.right, g_ScreenSize.right);
            int32_t vt = (std::max)(r.top + 1, g_ScreenSize.top);
            int32_t vb = (std::min)(r.bottom - 1, g_ScreenSize.bottom);
            if (r.Height() > 0 && hr > hl) {
                CopyScreenRectToBmp64(hl, r.top, hr - hl, 1, kSelectFrameBufLen, 2, (uint8_t*)this->saved_rect_frame[0]->GetData());
                CopyScreenRectToBmp64(hl, r.bottom - 1, hr - hl, 1, kSelectFrameBufLen, 2, (uint8_t*)this->saved_rect_frame[1]->GetData());
            }

            if (vb > vt) {
                CopyScreenRectToBmp64(r.left, vt, 2, vb - vt, 2, kSelectFrameBufLen, (uint8_t*)this->saved_rect_frame[2]->GetData());
                CopyScreenRectToBmp64(r.right - 1, vt, 2, vb - vt, 2, kSelectFrameBufLen, (uint8_t*)this->saved_rect_frame[3]->GetData());
            }
#else
            if (r.Height() > 0) {
                CopyScreenRectToBmp64(r.left, r.top, r.Width(), 1, 1600, 2, (uint8_t*)this->saved_rect_frame[0]->GetData());
                CopyScreenRectToBmp64(r.left, r.bottom - 1, r.Width(), 1, 1600, 2, (uint8_t*)this->saved_rect_frame[1]->GetData());
            }

            if (r.Height() > 2) {
                CopyScreenRectToBmp64(r.left, r.top + 1, 2, r.Height() - 2, 2, 1600, (uint8_t*)this->saved_rect_frame[2]->GetData());
                CopyScreenRectToBmp64(r.right - 1, r.top + 1, 2, r.Height() - 2, 2, 1600, (uint8_t*)this->saved_rect_frame[3]->GetData());
            }
#endif
        }

        DrawRectangleFrame(r.left, r.top, r.right - 1, r.bottom - 1, GetColorRGB(0xff, 0xff, 0xff));

        UnlockPrimarySurface();

#ifdef A2CLIENT_PATCH
        if (r.Width() > 0 && r.Height() > 0) {
            QueueOrPresentMouseRect(CRect(r.left, r.top, r.right, r.top + 1));
            if (r.Height() > 1) {
                QueueOrPresentMouseRect(CRect(r.left, r.bottom - 1, r.right, r.bottom));
            }
            if (r.Height() > 2) {
                QueueOrPresentMouseRect(CRect(r.left, r.top + 1, r.left + 1, r.bottom - 1));
                if (r.Width() > 1) {
                    QueueOrPresentMouseRect(CRect(r.right - 1, r.top + 1, r.right, r.bottom - 1));
                }
            }
        }
#endif
    }
}

// 426eb6 (client: 428de4)
void CMousePointer::Repaint(CPoint point) {
#ifdef A2CLIENT_PATCH
    ScopedMousePresentBatch present_batch;
#endif

    this->mouse_hint_time = 0;
    this->mouse_hint_ts = timeGetTime();

    if (this->hint_state != 0) {
        this->RepaintHint();
    }

    this->hint_state = 0;

    this->Unpaint();

    if (this->select_frame_state != 0) {
        this->UnpaintSelectFrame();
    }

    this->x = point.x;
    this->y = point.y;

    this->select_frame_rect.SetRect(this->select_frame_rect.left, this->select_frame_rect.top, this->x, this->y);

    if (this->select_frame_state != 0) {
        this->PaintSelectFrame();
    }
    
    this->Paint();
}

// 428779 (client: 42a695)
void CMousePointer::RepaintHint() {
#ifdef A2CLIENT_PATCH
    ScopedMousePresentBatch present_batch;
#endif
    this->Unpaint();
    LockPrimarySurface();

    this->hint_image->DrawSubRect(this->hint_rect.left, this->hint_rect.top, 0, 0, this->hint_rect.Width(), this->hint_rect.Height());

    UnlockPrimarySurface();
#ifdef A2CLIENT_PATCH
    QueueOrPresentMouseRect(CRect(this->hint_rect.left, this->hint_rect.top, this->hint_rect.right + 1, this->hint_rect.bottom + 1));
#endif
    this->Paint();
}

// 42707e (client: 428fac)
void CMousePointer::ResetUserMsgTimer() {
    this->user_msg_ts = timeGetTime();
    this->user_msg_delay = 150;
}

// 427e23 (client: 429d51)
void CMousePointer::ResetStates() {
#ifdef A2CLIENT_PATCH
    ScopedMousePresentBatch present_batch;
#endif
    this->Unpaint();
    this->UnpaintSelectFrame();
    this->Paint();

    this->select_frame_state = 0;
    this->mouse_hint_time = 0;
    this->mouse_hint_ts = timeGetTime();
}

// 427912 (client: 429840)
void CMousePointer::RestoreAfterFlush(const CRect& rect) {
    if (this->cursor_sprite && paint_call_count) {
        CRect tmp(this->x - this->x_offset, this->y - this->y_offset, (this->x - this->x_offset) + this->cursor_sprite->GetWidth(0), (this->y - this->y_offset) + this->cursor_sprite->GetHeight(0));

        CRect r = this->select_frame_rect;
        r.NormalizeRect();

        CRect intr;
        if (intr.IntersectRect(&rect, &tmp) != 0) {
            LockBackSurface();

            this->saved_screen_rect2->DrawSubRect(this->x - this->x_offset, this->y - this->y_offset, 0, 0, this->cursor_sprite->GetWidth(0), this->cursor_sprite->GetHeight(0));
            CopyScreenRectToBmp64(this->x - this->x_offset, this->y - this->y_offset, 
                                  this->cursor_sprite->GetWidth(0), this->cursor_sprite->GetHeight(0), 
                                  this->cursor_sprite->GetWidth(0), this->cursor_sprite->GetHeight(0), 
                                  (uint8_t*)this->saved_screen_rect->GetData());
            UnlockBackSurface();
        }


        if (intr.IntersectRect(&rect, &this->hint_rect) && this->hint_state) {
            LockBackSurface();

            this->saved_screen_rect_hint->DrawSubRect(this->hint_rect.left, this->hint_rect.top, 0, 0, this->hint_rect.Width(), this->hint_rect.Height());
            CopyScreenRectToBmp64(this->hint_rect.left, this->hint_rect.top, 
                                    this->hint_rect.Width(), this->hint_rect.Height(), 
                                    this->hint_image->GetWidth(0), this->hint_image->GetHeight(0), 
                                    (uint8_t*)this->hint_image->GetData());
            UnlockBackSurface();

            return;
        }

        if (intr.IntersectRect(&rect, &r) && this->select_frame_state) {
            LockBackSurface();

#ifdef A2CLIENT_PATCH
            int32_t hl = (std::max)(r.left, g_ScreenSize.left);
            int32_t hr = (std::min)(r.right, g_ScreenSize.right);
            int32_t vt = (std::max)(r.top + 1, g_ScreenSize.top);
            int32_t vb = (std::min)(r.bottom - 1, g_ScreenSize.bottom);
            if (r.Width() > 0) {
                if (r.Height() > 0 && hr > hl) {
                    this->saved_rect_frame[4]->DrawSubRect(hl, r.top, 0, 0, hr - hl, 1);
                    this->saved_rect_frame[5]->DrawSubRect(hl, r.bottom - 1, 0, 0, hr - hl, 1);
                }

                if (vb > vt) {
                    this->saved_rect_frame[6]->DrawSubRect(r.left, vt, 0, 0, 1, vb - vt);
                    this->saved_rect_frame[7]->DrawSubRect(r.right - 1, vt, 0, 0, 1, vb - vt);
                }
            }

            if (r.Height() > 0 && hr > hl) {
                CopyScreenRectToBmp64(hl, r.top, hr - hl, 1, kSelectFrameBufLen, 2, (uint8_t*)this->saved_rect_frame[0]->GetData());
                CopyScreenRectToBmp64(hl, r.bottom - 1, hr - hl, 1, kSelectFrameBufLen, 2, (uint8_t*)this->saved_rect_frame[1]->GetData());
            }

            if (vb > vt) {
                CopyScreenRectToBmp64(r.left, vt, 2, vb - vt, 2, kSelectFrameBufLen, (uint8_t*)this->saved_rect_frame[2]->GetData());
                CopyScreenRectToBmp64(r.right - 1, vt, 2, vb - vt, 2, kSelectFrameBufLen, (uint8_t*)this->saved_rect_frame[3]->GetData());
            }
#else
            if (r.Width() > 0) {
                if (r.Height() > 0) {
                    this->saved_rect_frame[4]->DrawSubRect(r.left, r.top, 0, 0, r.Width(), 1);
                    this->saved_rect_frame[5]->DrawSubRect(r.left, r.bottom - 1, 0, 0, r.Width(), 1);
                }

                if (r.Height() > 2) {
                    this->saved_rect_frame[6]->DrawSubRect(r.left, r.top + 1, 0, 0, 1, r.Height() - 2);
                    this->saved_rect_frame[7]->DrawSubRect(r.right - 1, r.top + 1, 0, 0, 1, r.Height() - 2);
                }
            }

            CopyScreenRectToBmp64(r.left, r.top, r.Width(), 1, 1600, 2, (uint8_t*)this->saved_rect_frame[0]->GetData());
            CopyScreenRectToBmp64(r.left, r.bottom - 1, r.Width(), 1, 1600, 2, (uint8_t*)this->saved_rect_frame[1]->GetData());

            if (r.Height() > 2) {
                CopyScreenRectToBmp64(r.left, r.top + 1, 2, r.Height() - 2, 2, 1600, (uint8_t*)this->saved_rect_frame[2]->GetData());
                CopyScreenRectToBmp64(r.right - 1, r.top + 1, 2, r.Height() - 2, 2, 1600, (uint8_t*)this->saved_rect_frame[3]->GetData());
            }
#endif

            UnlockBackSurface();
        }
    }
}

// 42751c (client: 42944a)
void CMousePointer::PrepareFlushOverlays(const CRect& rect) {
    if (this->cursor_sprite && paint_call_count) {
        CRect tmp(this->x - this->x_offset, this->y - this->y_offset, (this->x - this->x_offset) + this->cursor_sprite->GetWidth(0), (this->y - this->y_offset) + this->cursor_sprite->GetHeight(0));

        CRect r = this->select_frame_rect;
        r.NormalizeRect();

        CRect intr;
        if (intr.IntersectRect(&rect, &this->hint_rect) && this->hint_state) {
            LockBackSurface();

            CopyScreenRectToBmp64(this->hint_rect.left, this->hint_rect.top, this->hint_rect.Width(), this->hint_rect.Height(),
                                    this->saved_screen_rect_hint->GetWidth(0), this->saved_screen_rect_hint->GetHeight(0),
                                    (uint8_t*)this->saved_screen_rect_hint->GetData());
            this->PaintHint();

            UnlockBackSurface();
        } else if (intr.IntersectRect(&rect, &r) && this->select_frame_state) {
            LockBackSurface();

            if (r.Width() > 0) {
#ifdef A2CLIENT_PATCH
                int32_t hl = (std::max)(r.left, g_ScreenSize.left);
                int32_t hr = (std::min)(r.right, g_ScreenSize.right);
                int32_t vt = (std::max)(r.top + 1, g_ScreenSize.top);
                int32_t vb = (std::min)(r.bottom - 1, g_ScreenSize.bottom);
                if (r.Height() > 0 && hr > hl) {
                    CopyScreenRectToBmp64(hl, r.top, hr - hl, 1, kSelectFrameBufLen, 2, (uint8_t*)this->saved_rect_frame[4]->GetData());
                    CopyScreenRectToBmp64(hl, r.bottom - 1, hr - hl, 1, kSelectFrameBufLen, 2, (uint8_t*)this->saved_rect_frame[5]->GetData());
                }

                if (vb > vt) {
                    CopyScreenRectToBmp64(r.left, vt, 2, vb - vt, 2, kSelectFrameBufLen, (uint8_t*)this->saved_rect_frame[6]->GetData());
                    CopyScreenRectToBmp64(r.right - 1, vt, 2, vb - vt, 2, kSelectFrameBufLen, (uint8_t*)this->saved_rect_frame[7]->GetData());
                }
#else
                if (r.Height() > 0) {
                    CopyScreenRectToBmp64(r.left, r.top, r.Width(), 1, 1600, 2, (uint8_t*)this->saved_rect_frame[4]->GetData());
                    CopyScreenRectToBmp64(r.left, r.bottom - 1, r.Width(), 1, 1600, 2, (uint8_t*)this->saved_rect_frame[5]->GetData());
                }

                if (r.Height() > 2) {
                    CopyScreenRectToBmp64(r.left, r.top + 1, 2, r.Height() - 2, 2, 1600, (uint8_t*)this->saved_rect_frame[6]->GetData());
                    CopyScreenRectToBmp64(r.right - 1, r.top + 1, 2, r.Height() - 2, 2, 1600, (uint8_t*)this->saved_rect_frame[7]->GetData());
                }
#endif
            }

            DrawRectangleFrame(r.left, r.top, r.right - 1, r.bottom - 1, GetColorRGB(0xff, 0xff, 0xff));

            UnlockBackSurface();
        }

        if (intr.IntersectRect(&rect, &tmp)) {
            LockBackSurface();
            CopyScreenRectToBmp64(this->x - this->x_offset, this->y - this->y_offset, this->cursor_sprite->GetWidth(0), this->cursor_sprite->GetHeight(0),
                                    this->cursor_sprite->GetWidth(0), this->cursor_sprite->GetHeight(0),
                                    (uint8_t*)this->saved_screen_rect2->GetData());
            this->cursor_sprite->Draw(this->x - this->x_offset, this->y - this->y_offset, this->frame, 0, 0);
            UnlockBackSurface();
        }
    }
}

// 426aa5 (client: 4289d3)
void CMousePointer::SetCursor(CSprite256* spr, int32_t off_x, int32_t off_y, int32_t count, int32_t delay) {
    if (g_IsServer) {
        return;
    }

#ifdef A2CLIENT_PATCH
    ScopedMousePresentBatch present_batch;
#endif
    this->Unpaint();

    if (this->saved_screen_rect) {
        delete this->saved_screen_rect;
    }

    if (this->saved_screen_rect2) {
        delete this->saved_screen_rect2;
    }

    this->cursor_sprite = spr;
    this->x_offset = off_x;
    this->y_offset = off_y;
    this->frame = 0;
    this->frame_ts = 0;
    this->user_msg_ts = 0;
    this->frame_delay = delay;
    this->frame_count = count;

    this->saved_screen_rect = new CBmp64(spr->GetWidth(0), spr->GetHeight(0));
    this->saved_screen_rect2 = new CBmp64(spr->GetWidth(0), spr->GetHeight(0));

    this->Paint();
}

// 427db1 (client: 429cdf)
void CMousePointer::StartSelect(const CPoint& point) {
#ifdef A2CLIENT_PATCH
    ScopedMousePresentBatch present_batch;
#endif
    this->select_frame_state = 1;
    this->select_frame_rect.SetRect(point, point);

    if (this->hint_state) {
        this->hint_state = 0;
        this->RepaintHint();
    }

    this->Unpaint();
    this->PaintSelectFrame();
    this->Paint();
}
// 426c64 (client: 428b92)
void CMousePointer::Unpaint() {
    if (g_IsServer) {
        return;
    }

    this->paint_call_count--;

    if (this->paint_call_count == 0 && this->cursor_sprite) {
        this->EraseCursor();
    }
}

// 428051 (client: 429f7f)
void CMousePointer::UnpaintSelectFrame() {
    if (this->select_frame_rect.Width() == 0 || this->select_frame_rect.Height() == 0) {
        return;
    }

    CRect r = this->select_frame_rect;
    r.NormalizeRect();

    LockPrimarySurface();

    if (r.Width() > 0) {
#ifdef A2CLIENT_PATCH
        int32_t hl = (std::max)(r.left, g_ScreenSize.left);
        int32_t hr = (std::min)(r.right, g_ScreenSize.right);
        int32_t vt = (std::max)(r.top + 1, g_ScreenSize.top);
        int32_t vb = (std::min)(r.bottom - 1, g_ScreenSize.bottom);
        if (r.Height() > 0 && hr > hl) {
            this->saved_rect_frame[0]->DrawSubRect(hl, r.top, 0, 0, hr - hl, 1);
            this->saved_rect_frame[1]->DrawSubRect(hl, r.bottom - 1, 0, 0, hr - hl, 1);
        }

        if (vb > vt) {
            this->saved_rect_frame[2]->DrawSubRect(r.left, vt, 0, 0, 1, vb - vt);
            this->saved_rect_frame[3]->DrawSubRect(r.right - 1, vt, 0, 0, 1, vb - vt);
        }
#else
        if (r.Height() > 0) {
            this->saved_rect_frame[0]->DrawSubRect(r.left, r.top, 0, 0, r.Width(), 1);
            this->saved_rect_frame[1]->DrawSubRect(r.left, r.bottom - 1, 0, 0, r.Width(), 1);
        }

        if (r.Height() > 2) {
            this->saved_rect_frame[2]->DrawSubRect(r.left, r.top + 1, 0, 0, 1, r.Height() - 2);
            this->saved_rect_frame[3]->DrawSubRect(r.right - 1, r.top + 1, 0, 0, 1, r.Height() - 2);
        }
#endif
    }

    UnlockPrimarySurface();

#ifdef A2CLIENT_PATCH
    if (r.Width() > 0 && r.Height() > 0) {
        QueueOrPresentMouseRect(CRect(r.left, r.top, r.right, r.top + 1));
        if (r.Height() > 1) {
            QueueOrPresentMouseRect(CRect(r.left, r.bottom - 1, r.right, r.bottom));
        }
        if (r.Height() > 2) {
            QueueOrPresentMouseRect(CRect(r.left, r.top + 1, r.left + 1, r.bottom - 1));
            if (r.Width() > 1) {
                QueueOrPresentMouseRect(CRect(r.right - 1, r.top + 1, r.right, r.bottom - 1));
            }
        }
    }
#endif
}

// 4271e6 (client: 429114)
void CMousePointer::Update() {
    if (g_IsServer || GetBackSurfaceLockCount()) {
        return;
    }

    uint32_t ts = timeGetTime();

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

    if (GetAsyncKeyState(VK_RBUTTON) > -1 && wnd->has_focus != 0) {
        POINT pt;
#ifdef A2CLIENT_PATCH
        GetLogicalCursorPos(&pt);
#else
        GetCursorPos(&pt);
#endif

        if ((pt.x != this->x  || pt.y != this->y) && this->cursor_sprite ) {
            this->Repaint(pt);
        }
    }

    if (this->user_msg_delay < ts - this->user_msg_ts && g_LButtonDown != 0) {
        wnd->PostMessage(MAIN_MSG_MOUSE, 1, (this->x & 0xFFFF) | (this->y << 16));

        this->user_msg_ts = ts;
        this->user_msg_delay = 66;
    }

    if (this->frame_delay < ts - this->frame_ts && frame_count > 1 && wnd && wnd->has_focus != 0) {
        this->frame_ts = ts;
        this->frame++;

        if (this->frame >= frame_count) {
            this->frame = 0;
        }

#ifdef A2CLIENT_PATCH
        ScopedMousePresentBatch present_batch;
#endif
        this->Unpaint();
        this->Paint();
    }

    int32_t prev_time = this->mouse_hint_time;

    this->mouse_hint_time += ts - this->mouse_hint_ts;
    this->mouse_hint_ts = ts;

    if (this->mouse_hint_time >= 500 && this->mouse_hint_time < 25500 && 
        prev_time < 500 &&
        this->select_frame_state == 0 && 
        this->hint_enable != 0 &&
        wnd && wnd->vis_root && wnd->has_focus != 0) {
        CVisualObject* vobj = wnd->vis_root->GetChildAt(CPoint(this->x, this->y));
        if (vobj) {
            this->hint_text = vobj->GetHint();
            if (this->hint_text != "") {
                this->hint_state = 1;
                this->CheckHintValues();
                this->UpdateHint();
            }
        }
    }

    if (this->mouse_hint_time > 25499 && this->hint_state != 0 && 
        wnd && wnd->has_focus != 0) {
        this->hint_state = 0;
        this->RepaintHint();
    }

    if (wnd->music_update_proc) {
        wnd->music_update_proc();
    }
}

// 42819d (client: 42a0cb)
void CMousePointer::UpdateHint() {
#ifdef A2CLIENT_PATCH
    ScopedMousePresentBatch present_batch;
#endif
    this->hint_rect.SetRect(this->x, this->y - 5 - this->hint_lines * 14, this->x + 11 + this->hint_max_width, this->y);

    if (this->hint_rect.right > g_ScreenSize.right) {
        this->hint_rect.OffsetRect(g_ScreenSize.right - this->hint_rect.right, 0);
    }

    if (this->hint_rect.top < g_ScreenSize.top) {
        this->hint_rect.OffsetRect(0, g_ScreenSize.top - this->hint_rect.top);
    }

    this->Unpaint();
    LockPrimarySurface();
    CopyScreenRectToBmp64(this->hint_rect.left, this->hint_rect.top, this->hint_rect.Width(), this->hint_rect.Height(), this->hint_image->GetWidth(0), this->hint_image->GetHeight(0), (uint8_t*)this->hint_image->GetData());
    this->PaintHint();
    UnlockPrimarySurface();
#ifdef A2CLIENT_PATCH
    QueueOrPresentMouseRect(CRect(this->hint_rect.left, this->hint_rect.top, this->hint_rect.right + 1, this->hint_rect.bottom + 1));
#endif

    this->Paint();
}


// 438830 (client: 43aa60)
void CMousePointer::DisableHint() { this->hint_enable = 0; }
// 438810 (client: 43aa40)
void CMousePointer::EnableHint() { this->hint_enable = 1; }
// 41eca0 (client: 420820)
CSprite256* CMousePointer::GetCursorSprite() { return this->cursor_sprite; }
// 41ed20 (client: 4208a0)
CRect& CMousePointer::GetSelectFrame() { return this->select_frame_rect; }
// 41ed00 (client: 420880)
int32_t CMousePointer::GetSelectState() { return this->select_frame_state; }
// 41ecc0 (client: 420840)
int32_t CMousePointer::GetX() { return this->x; }
// 41ece0 (client: 420860)
int32_t CMousePointer::GetY() { return this->y; }


// 47cf99 (client: 480e53)
CCursor::~CCursor() {
    if (this->sprite) {
        delete this->sprite;
    }
    this->sprite = nullptr;
}

// 47ce66 (client: 480d20)
CCursor::CCursor(const char* fname, int32_t xof, int32_t yof, int32_t _delay) {
    if (strstr(fname, "16a") == nullptr) {
        this->sprite = new CSprite256(fname);
        this->sprite->ResetPalette(1, 1, 0);
    } else {
        this->sprite = new CA16(fname);
        this->sprite->ResetPalette(16, 4, 0);
    }

    this->x_off = xof;
    this->y_off = yof;
    this->delay = _delay;
    this->frames_count = this->sprite->GetFrameCount();
}

// 47d025 (client: 480edf)
void CCursor::Use() {
    g_mousept.SetCursor(this->sprite, this->x_off, this->y_off, this->frames_count, this->delay);
}



// 47d05d (client: 480f17)
void LoadCursors() {
    g_mousept.Update();

    g_Cursors[0] = new CCursor("graphics\\cursors\\default\\sprites.16a", 4, 4, 2000000000);
    g_Cursors[1] = new CCursor("graphics\\cursors\\move\\sprites.16a", 2, 3, 100);
    g_Cursors[2] = new CCursor("graphics\\cursors\\swarm\\sprites.16a", 2, 3, 100);
    g_Cursors[3] = new CCursor("graphics\\cursors\\attack\\sprites.16a", 3, 3, 100);
    g_Cursors[4] = new CCursor("graphics\\cursors\\defend\\sprites.16a", 16, 16, 100);
    g_Cursors[5] = new CCursor("graphics\\cursors\\select\\sprites.16a", 3, 3, 100);
    g_Cursors[6] = new CCursor("graphics\\cursors\\patrol\\sprites.16a", 8, 25, 100);
    g_Cursors[7] = new CCursor("graphics\\cursors\\cast\\sprites.16a", 16, 16, 100);
    g_mousept.Update();
    g_Cursors[8] = new CCursor("graphics\\cursors\\pickup\\sprites.16a", 16, 12, 66);
    g_Cursors[9] = new CCursor("graphics\\cursors\\arrow0\\sprites.16a", 16, 6, 2000000000);
    g_Cursors[10] = new CCursor("graphics\\cursors\\arrow4\\sprites.16a", 16, 28, 2000000000);
    g_Cursors[11] = new CCursor("graphics\\cursors\\arrow6\\sprites.16a", 5, 16, 2000000000);
    g_Cursors[12] = new CCursor("graphics\\cursors\\arrow2\\sprites.16a", 26, 16, 2000000000);
    g_Cursors[13] = new CCursor("graphics\\cursors\\arrow7\\sprites.16a", 8, 9, 2000000000);
    g_Cursors[14] = new CCursor("graphics\\cursors\\arrow5\\sprites.16a", 8, 24, 2000000000);
    g_Cursors[15] = new CCursor("graphics\\cursors\\arrow1\\sprites.16a", 23, 9, 2000000000);
    g_Cursors[16] = new CCursor("graphics\\cursors\\arrow3\\sprites.16a", 23, 24, 2000000000);
    g_mousept.Update();
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

// 47daf0 (client: 4819aa)
void DeleteCursors() {
    for (int i = 0; i < 28; i++) {
        delete g_Cursors[i];
    }
}

// 4750d0 (client: 477ed0)
int __cdecl ApplyCursor(CCursor* cur) {
    if (!cur) {
        return 0;
    }

    if (g_mousept.GetCursorSprite() == cur->GetSprite()) {
        return 0;
    }
    
    cur->Use();
    return 1;
}
