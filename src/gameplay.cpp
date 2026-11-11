#include <winsock2.h>
#include <cmath>
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
#include "item.h"
#include "player_file.h"
#include "world.h"
#include "gameplay.h"
#include "vis_playing_area.h"
#include "take_damage.h"
#include "vis_inventory.h"
#include "vis_shop.h"
#include "vis_tavern.h"
#include "vis_hat.h"
#include "ingame.h"
#include "vis_char.h"

uint16_t* clr_log_sblack = clrsh_ShockingBlack; //62f88c
uint16_t* clr_log_tblack = clrsh_TechBlack; //62f890
uint16_t* clr_log_tok0 = clrsh_ShockingBlack; //62f894
uint16_t* clr_log_tok2 = clrsh_ShockingBlack; //62f898
uint16_t* clr_log_tok1 = clrsh_ShockingBlack; //62f89c
uint16_t* clr_log_tok3 = clrsh_ShockingBlack; //62f8a0

//476de7
void __cdecl SetMessageColors(uint32_t flag) {
	g_MessageColors = flag;
	if (flag == 0) {
		clr_log_sblack = clrsh_ShockingBlack;
		clr_log_tblack = clrsh_TechBlack;
	} else {
		clr_log_sblack = g_colors_human_pals[2];
		clr_log_tblack = g_colors_human_pals[7];
		clr_log_tok0 = clrsh_GoldenFizz;
		clr_log_tok1 = g_colors_human_pals[1];
		clr_log_tok2 = g_colors_human_pals[4];
		clr_log_tok3 = g_colors_human_pals[0];
	}
}


GM_a28::~GM_a28() = default; //4020ad

GM_a28::GM_a28() {
	//401f50
	this->elapsed_time = 0;
	this->dirty = 1;
}

#ifdef A2CLIENT_PATCH
// conv_double_tilde from my-a2mgr chat.cpp: double '~' so the game renderer
// shows a literal tilde instead of treating it as a color-code escape
static CString TangarDoubleTilde(const char* str)
{
	static char tilda_buf[4000];
	char* t = tilda_buf;
	while (*str && t < (tilda_buf + sizeof(tilda_buf) - 3)) {
		*t++ = *str;
		if (*str == '~') {
			*t++ = *str;
		}
		str++;
	}
	*t = 0;
	return CString(tilda_buf);
}
#endif

void GM_a28::Add(const char* msg, uint16_t* clr, uint32_t t) {
	//4021fe
	this->dirty = 1;

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	
	CGameFont* fnt = g_font1;
	if (wnd->sessionMode == SESSION_SERVER) {
		fnt = g_font2;
	}

	bool restart = this->text.GetSize() == 0;

#ifdef A2CLIENT_PATCH
	// CHAT_tilde hooks: escape tildes when adding chat text
	CString escaped = TangarDoubleTilde(msg);
	CStringArray& lines = fnt->StringArrayForRect(this->out_area, (const char*)escaped);
#else
	CStringArray& lines = fnt->StringArrayForRect(this->out_area, msg);
#endif
	for (int i = 0; i < lines.GetSize(); i++) {
		CString& s = lines[i];
		for (int j = 0; j < s.GetLength(); j++) {
			if (s[j] == '\r') {
				s.SetAt(j, ' ');
			}
		}

		this->text.Add(s);
		this->color.Add(clr);
		this->times.Add(t);
	}

	if (restart) {
		this->timestamp = timeGetTime();
		this->elapsed_time = 0;
	}

	while (this->text.GetSize() > this->max_size) {
		this->text.RemoveAt(0);
		this->color.RemoveAt(0);
		this->times.RemoveAt(0);
	}
}

void GM_a28::AddWithCheck(const char* msg, uint16_t* clr, uint32_t t) {
	//4023aa
	this->dirty = 1;

	CStringArray& lines = g_font1->StringArrayForRect(this->out_area, msg);
	for (int i = 0; i < lines.GetSize(); i++) {
		CString& s = lines[i];
		for (int j = 0; j < s.GetLength(); j++) {
			if (s[j] == '\r') {
				s.SetAt(j, ' ');
			}
		}

		if (lines.GetSize() == 1 && this->text.GetSize() != 0) {
			if (this->text[this->text.GetUpperBound()] == s) {
				return;
			}
		}

		this->text.Add(s);
		this->color.Add(clr);
		this->times.Add(t);
	}

	if (this->text.GetSize() == 1) {
		this->timestamp = timeGetTime();
		this->elapsed_time = 0;
	}

	while (this->text.GetSize() > this->max_size) {
		this->text.RemoveAt(0);
		this->color.RemoveAt(0);
		this->times.RemoveAt(0);
	}
}

void GM_a28::Clear() {
	//4021d2
	this->text.RemoveAll();
	this->color.RemoveAll();
	this->times.RemoveAll();
}

void GM_a28::Draw() {
	//40261a

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	int32_t out_x = 8;
	int32_t out_y = 8;
	CGameFont* fnt = g_font1;

	if (wnd->sessionMode == SESSION_SERVER) {
		out_x = 0;
		out_y = 220;
		fnt = g_font2;
	}

	for (int i = 0; i < this->text.GetSize(); i++) {
		CString s = this->text[i];

		while (true) {
			int rpos = s.Find('\r');

			if (rpos < 0) {
				fnt->DrawTextWithShadow(out_x, out_y, s, 0, (uint16_t*)this->color[i], 1);
				out_y += 2 + fnt->GetHeight();
				break;
			}
			
			fnt->DrawTextWithShadow(out_x, out_y, s.Left(rpos), 0, (uint16_t*)this->color[i], 1);
			out_y += 2 + fnt->GetHeight();
			s = s.Mid(rpos + 1);
		}
	}
}

void GM_a28::SetRect(const CRect& r) {
	//40212b
	this->out_area = r;

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	if (wnd->sessionMode == SESSION_SERVER) {
		this->max_size = (g_ScreenSize.bottom - 480) / (g_font2->GetHeight() + 2) + 14;
	} else {
		this->max_size = (this->out_area.Height() / (g_font1->GetHeight() + 2)) / 2;
	}
}

void GM_a28::Update() {
	//402585
	if (this->text.GetSize() == 0) {
		return;
	}

	uint32_t ts = timeGetTime();

	this->elapsed_time += (ts - this->timestamp);
	this->timestamp = ts;

	if (this->times[0] < this->elapsed_time) {
		this->elapsed_time = 0;

		this->text.RemoveAt(0);
		this->color.RemoveAt(0);
		this->times.RemoveAt(0);
	}
}





// 4AEE31
void VisRightPanel::VMethod7() {
    CVisualObject::VMethod7();
}

// 4AEDCF
VisRightPanel::VisRightPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : CVisualObject(_id, l, t, r, b, nullptr) {
}

// 4B46E0
VisRightPanel::~VisRightPanel() {
}

// 4AEE44
int32_t VisRightPanel::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == WM_MOUSEMOVE && g_mousept.GetSelectState() != 0) {
        g_mousept.ResetStates();
    }
    return CVisualObject::MsgProc(msg, wparam, lparam);
}

// 4B4019
void VisSideStatus::UpdateCursor() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CSprite256* cur_sprite = g_mousept.GetCursorSprite();

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    CCursor* new_cursor = nullptr;
    CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
    if (screen_rect.PtInRect(mouse_pt)) {
        if (g_mousept.GetX() >= g_ScreenSize.right - 2 && main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
            if (g_mousept.GetY() == 0) {
                new_cursor = g_Cursors[CURSOR_ARROW1];
            } else if (g_mousept.GetY() < g_ScreenSize.bottom - 2) {
                new_cursor = g_Cursors[CURSOR_ARROW2];
            } else {
                new_cursor = g_Cursors[CURSOR_ARROW3];
            }
        } else if (g_mousept.GetY() >= g_ScreenSize.bottom - 2 && main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
            new_cursor = g_Cursors[CURSOR_ARROW4];
        } else {
            if (screen_rect.PtInRect(mouse_pt)) {
                new_cursor = g_Cursors[CURSOR_DEFAULT];
            }
        }
        if (main_wnd->drag_entry != nullptr) {
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
void VisSideStatus::VMethod7() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->dialogsMask != DIALOG_MASK_INGAME) {
        return;
    }

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    VisPlayingArea* map = main_wnd->vis_map_context;
    LockSurface2();
    if (screen_rect.Height() >= main_wnd->vis_charinfo->GetRect().Height()) {
        screen_rect.OffsetRect(0, screen_rect.Height() - main_wnd->vis_charinfo->GetRect().Height());
        g_bmp_textbackr->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);

        CStructure* structure = nullptr;
        CUnit* unit = nullptr;
        uint16_t selected_id = (uint16_t)map->hovered_unit_id;
        CGameObject* found_obj = nullptr;
        int32_t found = 0;
        if (selected_id != 0 && main_wnd->drag_entry == nullptr) {
            found = map->objects.Lookup(selected_id, found_obj);
        }
        if (selected_id != 0 && found != 0) {
            if (found_obj->IsKindOf(RUNTIME_CLASS(CStructure))) {
                structure = (CStructure*)found_obj;
            } else {
                unit = (CUnit*)found_obj;
            }
        } else if (map->selected_object_count == 1) {
            if (map->primary_selected_object->IsKindOf(RUNTIME_CLASS(CStructure))) {
                structure = (CStructure*)map->primary_selected_object;
            } else {
                unit = (CUnit*)map->primary_selected_object;
            }
        }

        if (structure != nullptr) {
            g_font2->DrawTextWithShadow(screen_rect.right - 0x58, screen_rect.top + 0x1C, txt_building.GetLine(structure->typeId - 1), 2, clrsh_DullGold, 1);
            g_font2->DrawTextWithShadow(screen_rect.right - 0x58, screen_rect.top + 0x2C, TxtFile::AllLines[0x13], 2, clrsh_DullGold, 1);
            char hp_text[0x10];
            sprintf(hp_text, "%d/%d", structure->hp, structure->hp_max);
            g_font2->DrawTextWithShadow(screen_rect.right - 0x58, screen_rect.top + 0x36, hp_text, 2, clrsh_Oxley, 1);
        } else if (unit != nullptr) {
            unit->DrawStatsPanel(&screen_rect);
        }

        screen_rect.OffsetRect(0, main_wnd->vis_charinfo->GetRect().Height() - screen_rect.Height());
    }

    if (g_ScreenSize.bottom > 600) {
        g_bmp_extra1024r->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
    } else if (g_ScreenSize.bottom > 480) {
        g_bmp_extra800r->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
    }
    UnlockSurface2();
    this->dirty = 0;
}

// 4B4490
int32_t VisSideStatus::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    int32_t result = CVisualObject::MsgProc(msg, wparam, lparam);

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    int32_t moved_up = (screen_rect.Height() >= main_wnd->vis_charinfo->GetRect().Height()) ? 1 : 0;

    if (result == 0) {
        switch (msg) {
        case VIS_MSG_FRAME_TICK:
            if (main_wnd->dialogsMask == DIALOG_MASK_INGAME && moved_up != 0 && this->dirty != 0) {
                this->VMethod9();
            }
            if (main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
                this->UpdateCursor();
            }
            break;
        case VIS_MSG_SET_MAP_CONTEXT:
        case VIS_MSG_DATA_REFRESH:
        case VIS_MSG_STATUS_REFRESH:
            this->dirty = 1;
            break;
        }
    }
    return result;
}

// 4B45BB
const char* VisSideStatus::GetHint() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        return nullptr;
    }

    VisPlayingArea* map = main_wnd->vis_map_context;
    CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    CUnit* unit = nullptr;
    int32_t moved_up = (screen_rect.Height() >= main_wnd->vis_charinfo->GetRect().Height()) ? 1 : 0;
    if (map->selected_object_count == 1) {
        unit = (CUnit*)map->primary_selected_object;
    }
    if (unit == nullptr || moved_up == 0) {
        return nullptr;
    }

    int32_t hint_y = mouse_pt.y - screen_rect.TopLeft().y - 2 - (this->rect.Height() - main_wnd->vis_charinfo->GetRect().Height());
    int32_t hint_x = mouse_pt.x - screen_rect.TopLeft().x;
    return unit->GetStatsHint(hint_x, hint_y);
}

// 4B3FA3
VisSideStatus::VisSideStatus(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : CVisualObject(_id, l, t, r, b, nullptr) {
    this->dirty = 0;
}

// 4B4810
VisSideStatus::~VisSideStatus() {
}

// 4B0C84
int32_t VisMiniMap::OnLButtonUp(uint32_t wparam, CPoint pos) {
    (void)wparam;
    (void)pos;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        g_Cursors[0]->Use();
        main_wnd->ResetItemCursor();
    }
    return 1;
}


// 4B0C2F
int32_t VisMiniMap::OnMouseMove(uint32_t wparam, CPoint pos) {
    if ((wparam & 1) != 0) {
        return this->OnLButtonDown(wparam, pos);
    }
    if ((wparam & 2) != 0) {
        return this->OnRButtonDown(wparam, pos);
    }
    return 0;
}


// 4B042E
void VisMiniMap::UpdateCursor() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CSprite256* cur_sprite = g_mousept.GetCursorSprite();
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    CCursor* new_cursor = nullptr;
    CPoint mpt(g_mousept.GetX(), g_mousept.GetY());
    if (rc.PtInRect(mpt)) {
        if (g_mousept.GetX() < g_ScreenSize.right - 2 || main_wnd->dialogsMask != DIALOG_MASK_INGAME) {
            if (g_mousept.GetY() == 0 && main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
                new_cursor = g_Cursors[9];
            } else {
                VisPlayingArea* mc = this->map_context;
                Scenario* scen = mc->scenario;
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
                    || mc->map_width - 0x10 < (pt2.x - ox) >> z
                    || mc->map_height - 0x10 < (pt2.y - oy) >> z) {
                    new_cursor = g_Cursors[0x11];
                } else if (mc->selected_object_count == 0) {
                    new_cursor = g_Cursors[0x11];
                } else {
                    new_cursor = g_Cursors[0x12];
                    switch (mc->order_mode) {
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
                    if ((mc->selection_flags & 0x24) != 0) {
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
        if (main_wnd->drag_entry != nullptr) {
            new_cursor = main_wnd->item_cursor;
        }
        if (new_cursor != nullptr && cur_sprite != new_cursor->GetSprite()) {
            new_cursor->Use();
        }
    }
}


// 4AF006
int32_t VisMiniMap::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    int32_t res = this->CVisualObject::MsgProc(msg, wparam, lparam);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    if (res == 0) {
        switch (msg) {
        case VIS_MSG_FRAME_TICK:
            if (main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
                this->VMethod9();
                this->UpdateCursor();
            }
            break;
        case VIS_MSG_SET_MAP_CONTEXT:
            this->map_context = (VisPlayingArea*)wparam;
            break;
        case VIS_MSG_MINIMAP_REBUILD:
            this->RebuildMap();
            res = 1;
            break;
        case VIS_MSG_DATA_REFRESH:
            this->viewer = -100;
            break;
        }
    }
    return res;
}


// 4B0AAD
int32_t VisMiniMap::OnRButtonDown(uint32_t wparam, CPoint pos) {
    VisPlayingArea* mc = this->map_context;
    (void)AfxGetMainWnd();
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    int32_t z = this->zoom;
    int32_t mode = 1;
    if (z < 0) {
        z = 0;
        mode = 2;
    }
    Scenario* scen = mc->scenario;
    int32_t x, y;
    if (mode == 1) {
        int32_t ox = 0x48 - ((scen->GetWidth() - 0x10 << z) >> 1);
        int32_t oy = 0x52 - ((scen->GetHeight() - 0x10 << z) >> 1);
        x = ((pos.x - ox) - rc.left >> z) - mc->view_tiles_x / 2;
        y = ((pos.y - oy) - rc.top >> z) - mc->view_tiles_y / 2;
    } else {
        int32_t ox = 0x48 - (scen->GetWidth() - 0x10 >> 2);
        int32_t oy = 0x52 - (scen->GetHeight() - 0x10 >> 2);
        x = ((pos.x - ox) - rc.left) * 2 - mc->view_tiles_x / 2;
        y = ((pos.y - oy) - rc.top) * 2 - mc->view_tiles_y / 2;
    }
    x = x + 8;
    y = y + 8;
    mc->MsgProc(VIS_MSG_SCROLL_VIEW, x, y);
    return 0;
}


// 4B0788
int32_t VisMiniMap::OnLButtonDown(uint32_t wparam, CPoint pos) {
    VisPlayingArea* mc = this->map_context;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    int32_t z = this->zoom;
    int32_t mode = 1;
    if (z < 0) {
        z = 0;
        mode = 2;
    }
    Scenario* scen = mc->scenario;
    int32_t ox, oy, x, y;
    if (mode == 1) {
        ox = 0x48 - ((scen->GetWidth() - 0x10 << z) >> 1);
        oy = 0x52 - ((scen->GetHeight() - 0x10 << z) >> 1);
        x = ((pos.x - ox) - rc.left >> z) - mc->view_tiles_x / 2;
        y = ((pos.y - oy) - rc.top >> z) - mc->view_tiles_y / 2;
    } else {
        ox = 0x48 - (scen->GetWidth() - 0x10 >> 2);
        oy = 0x52 - (scen->GetHeight() - 0x10 >> 2);
        x = ((pos.x - ox) - rc.left) * 2 - mc->view_tiles_x / 2;
        y = ((pos.y - oy) - rc.top) * 2 - mc->view_tiles_y / 2;
    }
    x = x + 8;
    y = y + 8;
    CSprite256* cur_sprite = g_mousept.GetCursorSprite();
    if (cur_sprite == g_Cursors[0x11]->GetSprite()) {
        mc->MsgProc(VIS_MSG_SCROLL_VIEW, x, y);
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
            mc->SendGroupMove((uint16_t)x, (uint16_t)y);
        } else if (cur_sprite == g_Cursors[0x13]->GetSprite()) {
            uint32_t id = mc->FindObjectAtTile(x, y);
            if ((id & 0xffff) == 0) {
                mc->SendGroupAttackMove((uint16_t)x, (uint16_t)y);
            } else {
                mc->SendGroupAttack((uint16_t)id);
            }
        } else if (cur_sprite == g_Cursors[0x14]->GetSprite()) {
            uint32_t id = mc->FindObjectAtTile(x, y);
            if ((id & 0xffff) != 0) {
                mc->SendGroupAttackTarget((uint16_t)id);
            }
        } else if (cur_sprite == g_Cursors[0x16]->GetSprite()) {
            mc->FindObjectAtTile(x, y);
        } else if (cur_sprite == g_Cursors[0x15]->GetSprite()) {
            mc->SendGroupPatrol((uint16_t)x, (uint16_t)y);
        }
    }
    mc->order_mode = 0;
    main_wnd->vis_ordertoolbar->MsgProc(VIS_MSG_ORDERS_CLEAR, 0, 0);
    return 0;
}


// 4AF0DC
void VisMiniMap::RebuildMap() {
    Scenario* scen = this->map_context->scenario;
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
    uint8_t* heights = scen->GetOverlayMap();
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
void VisMiniMap::VMethod7() {
    VisPlayingArea* mc = this->map_context;
    if (mc == nullptr) {
        return;
    }
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if ((main_wnd->dialogsMask & DIALOG_MASK_SHOP) != 0 || (main_wnd->dialogsMask & DIALOG_MASK_TAVERN) != 0) {
        return;
    }
    if (abs((int)(this->viewer - mc->anim_tick)) <= 10) {
        return;
    }
    this->viewer = mc->anim_tick;
    CRect rc;
    this->ClientRectToScreen(&rc, this->rect);
    Scenario* scen = mc->scenario;
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
    g_bmp_crystalr->Draw(rc.left, rc.top, 0, 0, 0);
    if (mc->minimap_dirty == 0) {
        this->bitmap2->Draw(rc.left + ox, rc.top + oy, 0, 0, 0);
    } else {
        this->bitmap1->Draw(rc.left + ox, rc.top + oy, 0, 0, 0);
    }
    if (mc->minimap_dirty != 0) {
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
        mc->minimap_dirty = 0;
    }
    POSITION pos = mc->objects.GetStartPosition();
    while (pos != nullptr) {
        uint16_t key;
        CGameObject* obj;
        mc->objects.GetNextAssoc(pos, key, obj);
        obj->DrawMinimapRect(rc.left + ox, rc.top + oy, this->zoom);
    }
    if (mode == 1) {
        DrawRectangleFrame((mc->view_x - 8) * step + rc.left + ox,
            (mc->view_y - 8) * step + rc.top + oy,
            (mc->view_x - 8) * step + rc.left + ox - 1 + mc->view_tiles_x * step,
            (mc->view_y - 8) * step + rc.top + oy - 1 + mc->view_tiles_y * step,
            (uint16_t)(((0xFF >> (8 - g_RBits & 0x1F)) << (g_RBitShift & 0x1F))
                | ((0xFF >> (8 - g_GBits & 0x1F)) << (g_GBitShift & 0x1F))
                | ((0xFF >> (8 - g_BBits & 0x1F)) << (g_BBitShift & 0x1F))));
    } else {
        DrawRectangleFrame((mc->view_x - 8) / 2 + rc.left + ox,
            (mc->view_y - 8) / 2 + rc.top + oy,
            (mc->view_x - 8) / 2 + rc.left + ox - 1 + mc->view_tiles_x / 2,
            (mc->view_y - 8) / 2 + rc.top + oy - 1 + mc->view_tiles_y / 2,
            (uint16_t)(((0xFF >> (8 - g_RBits & 0x1F)) << (g_RBitShift & 0x1F))
                | ((0xFF >> (8 - g_GBits & 0x1F)) << (g_GBitShift & 0x1F))
                | ((0xFF >> (8 - g_BBits & 0x1F)) << (g_BBitShift & 0x1F))));
    }
    UnlockSurface2();
}


// 4AEEC3
VisMiniMap::VisMiniMap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: CVisualObject(_id, l, t, r, b, nullptr) {
    this->bitmap1 = nullptr;
    this->bitmap2 = nullptr;
}


// 4AEF4D (scalar deleting dtor ??_G at 4B4730)
VisMiniMap::~VisMiniMap() {
    if (this->bitmap1 != nullptr) {
        delete this->bitmap1;
    }
    if (this->bitmap2 != nullptr) {
        delete this->bitmap2;
    }
}

// 43AB28
Vis1200obj::Vis1200obj(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
    : CVisualObject(_id, l, t, r, b, hint) {
    this->font = _font;
    this->clr = _clr;
    this->cursor_visible = 1;
    this->match_player_index = -1;
    this->match_name_length = 0;
    this->flags |= 2;
}

// 43BFA0 (deleting dtor; the complete dtor 43AC08 only sets the vtable and calls the member dtors)
Vis1200obj::~Vis1200obj() = default;

// 43B32F
void Vis1200obj::VMethod7() {
    CString line;
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    for (int32_t i = 0; i < this->history.GetSize(); i++) {
        if (i == 0) {
            this->DrawInputLine(&this->history[0]);
        } else {
            this->font->DrawTxt(screen_rect.left, screen_rect.bottom - this->font->GetHeight() * (this->history.GetSize() - i + 1), this->history[i], 0, this->clr);
        }
    }

    if (this->cursor_visible != 0) {
        line = this->input_line + '_';
    } else {
        line = this->input_line;
    }

    if (this->history.GetSize() == 0) {
        this->DrawInputLine(&line);
    } else {
        this->font->DrawTxt(screen_rect.left, screen_rect.bottom - this->font->GetHeight(), line, 0, this->clr);
    }
}

// 43B159
int32_t Vis1200obj::OnKeyDown(uint32_t wparam) {
    int32_t result = 0;
    switch (wparam) {
    case 8:
        if (g_kbControlState == 0) {
            this->BackspaceInputChar();
        } else {
            this->DeletePreviousWord();
        }
        result = 1;
        break;
    case 9:
        this->CompletePlayerName();
        result = 1;
        break;
    case 0xD:
        if (this->input_line.GetLength() != 0) {
            this->history.Add(this->input_line);
            this->input_line.Empty();
        }
        // fall through
    case 0x1B:
        return CVisualObject::OnKeyDown(wparam);
    }
    return result;
}

// 43B283
int32_t Vis1200obj::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_FRAME_TICK) {
        this->blink_now_tick = timeGetTime();
        if (this->blink_now_tick - this->blink_last_tick > 500) {
            this->blink_last_tick = this->blink_now_tick;
            if (this->cursor_visible == 0) {
                this->cursor_visible = 1;
            } else {
                this->cursor_visible = 0;
            }
        }
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        main_wnd->vis_map_context->full_redraw_requested = 1;
    }
    return CVisualObject::MsgProc(msg, wparam, lparam);
}

// 43B240
// client: 43d64b
int32_t Vis1200obj::OnChar(uint32_t wparam) {
#ifdef A2CLIENT
    if (wparam < ' ' || wparam == 0x7F || wparam == '~') {
#else
    if (wparam < ' ' || wparam == 0x7F) {
#endif
        return CVisualObject::OnChar(wparam);
    }
    this->AppendInputChar(EncodeChar((uint8_t)wparam));
    return 1;
}

// 43AD35
void Vis1200obj::WriteData(void* buf) {
    CString* out = (CString*)buf;
    for (int32_t i = 0; i < this->history.GetSize(); i++) {
        *out += this->history[i];
    }
}

// 43ACCA
void Vis1200obj::ReadData(const void* buf) {
    this->input_line = (const char*)buf;
}

// 43C020
uint32_t Vis1200obj::DataSize() {
    return 4;
}

// 497330
int32_t Vis1200obj::GetMatchNameLength() {
    return this->match_name_length;
}

// 4972F0
int32_t Vis1200obj::GetMatchPlayerIndex() {
    return this->match_player_index;
}

// 43AD86
void Vis1200obj::SaveHistoryBackup() {
    this->history_backup.Copy(this->history);
}

// 43AC8C
void Vis1200obj::ResetInputState() {
    this->match_player_index = -1;
    this->match_name_length = 0;
    this->history.RemoveAll();
    this->input_line.Empty();
}

// 43ADA3
void Vis1200obj::RecallHistoryPrev() {
    this->history.Copy(this->history_backup);
    this->input_line.Empty();
    if (this->history.GetSize() != 0) {
        this->input_line = this->history[this->history.GetUpperBound()];
        this->history.RemoveAt(this->history.GetUpperBound(), 1);
    }
}

// 43AE1C
void Vis1200obj::AppendInputChar(uint8_t ch) {
    if (this->font->GetStrWidth(this->input_line) + this->font->GetStrWidth("_") < this->rect.Width()) {
        this->input_line += (char)ch;
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        main_wnd->vis_map_context->full_redraw_requested = 1;
    } else if (this->history.GetSize() < 2) {
        int32_t pos = this->input_line.ReverseFind(' ');
        if (pos < 10) {
            this->history.Add(this->input_line);
            this->input_line.Empty();
        } else {
            CString left = this->input_line.Left(pos + 1);
            this->history.Add(left);
            this->input_line = this->input_line.Right(this->input_line.GetLength() - pos - 1);
        }
        this->input_line.TrimLeft();
        this->AppendInputChar(ch);
    }
}

// 43B03A
void Vis1200obj::BackspaceInputChar() {
    this->input_line = this->input_line.Left(this->input_line.GetLength() - 1);
    this->input_line.TrimLeft();
    if (this->input_line.GetLength() == 0 && this->history.GetSize() > 0) {
        this->input_line = this->history[this->history.GetUpperBound()];
        this->history.RemoveAt(this->history.GetUpperBound(), 1);
        this->input_line.TrimLeft();
    }
}

// 43aae0
int32_t IsWordSeparator(char ch) {
    if (ch == ' ' || ch == ',' || ch == '.' || ch == ':' || ch == ';') {
        return 1;
    }
    return 0;
}

// 476236
void MakeLowerDosCp(CString* str) {
    CString lowered(*str);
    lowered.MakeLower();

    if (g_isDosCP == 1) {
        for (int32_t i = 0; i < str->GetLength(); i++) {
            uint8_t c = (*str)[i];
            if (c >= 0x80 && c < 0x90) {
                str->SetAt(i, c + 0x20);
            } else if (c >= 0x90 && c < 0xA0) {
                str->SetAt(i, c + 0x50);
            } else if (c < 0x80) {
                str->SetAt(i, lowered[i]);
            }
        }
    } else {
        *str = lowered;
    }
}

// 476987
void InsertThousandsSeparators(CString* str) {
    CString work(*str);
    int32_t len = work.GetLength();
    str->Empty();

    while (len > 3) {
        if (len == 4) {
            char first = work[0];
            if (first == '-' || first == '+') {
                break;
            }
        }
        *str = ',' + work.Right(3) + *str;
        work = work.Left(len - 3);
        len = work.GetLength();
    }

    *str = work + *str;
}

// 43B724
void Vis1200obj::CompletePlayerName() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisPlayingArea* map = main_wnd->vis_map_context;
    int32_t pos = this->input_line.GetLength() - 1;
    if (pos == -1) {
        return;
    }
    int32_t start = 0;
    if (this->input_line[0] == '=' || this->input_line[0] == '-') {
        start = 1;
    }
    if (IsWordSeparator(this->input_line[pos]) != 0) {
        return;
    }
    while (pos >= start && IsWordSeparator(this->input_line[pos]) == 0) {
        pos--;
    }
    CString word = this->input_line.Mid(pos + 1);
    if (word.IsEmpty()) {
        return;
    }
    int32_t word_end = pos;
    MakeLowerDosCp(&word);
    CStringArray candidates;
    for (int32_t i = 0; i < map->map_players.GetSize(); i++) {
        MapPlayerData* player = map->map_players[i];
        if (player != 0 && (player->flags & 1) == 0) {
            candidates.Add(player->name);
        }
    }
    for (int32_t i = 0; i < word.GetLength(); i++) {
        int32_t j = 0;
        while (j < candidates.GetSize()) {
            CString cand = candidates[j];
            MakeLowerDosCp(&cand);
            if (i >= cand.GetLength() || word[i] != cand[i]) {
                candidates.RemoveAt(j, 1);
            } else {
                j++;
            }
        }
        if (candidates.GetSize() == 0) {
            return;
        }
    }
    this->input_line = this->input_line.Left(word_end + 1) + candidates[0];
    map->full_redraw_requested = 1;
}

// 43BA75
void Vis1200obj::DeletePreviousWord() {
    int32_t pos = this->input_line.GetLength() - 1;
    if (pos == -1) {
        return;
    }
    while (pos > 0 && IsWordSeparator(this->input_line[pos]) == 0) {
        this->BackspaceInputChar();
        pos = this->input_line.GetLength() - 1;
    }
    this->BackspaceInputChar();
}

// 43B504
void Vis1200obj::DrawInputLine(CString* str) {
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    this->match_player_index = -1;
    this->match_name_length = 0;
    this->font->DrawTxt(screen_rect.left, screen_rect.bottom - this->font->GetHeight() * (this->history.GetSize() + 1), *str, 0, this->clr);
    if (!str->IsEmpty() && (*str)[0] == '-') {
        CString name_part = str->Mid(1);
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        VisPlayingArea* map = main_wnd->vis_map_context;
        for (int32_t i = 0; i < map->map_players.GetSize(); i++) {
            MapPlayerData* player = map->map_players[i];
            if (player != 0) {
                CString pname = player->name;
                if (pname == name_part.Left(pname.GetLength())) {
                    this->match_player_index = i;
                    this->match_name_length = pname.GetLength() + 1;
                    break;
                }
            }
        }
    }
}

// 43BE9F
void Vis1200::RepositionAboveDialogs() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    VisPlayingArea* map = (VisPlayingArea*)this->parent;
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
void Vis1200::VMethod26() {
    this->text_block = new Vis1200obj(4, 0, 0, this->rect.Width(), g_font1->GetHeight() * 3, g_font1, clrsh_ShockingBlack, nullptr);
    this->rect.BottomRight().y = this->rect.TopLeft().y + g_font1->GetHeight() * 3 + 4;
    this->AddChild(this->text_block);
    this->active = 0;
}

// 43BDEC
int32_t Vis1200::OnKeyDown(uint32_t wparam) {
    if (wparam == 0xd) {
        this->text_block->SaveHistoryBackup();
        this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        return 1;
    }
    if (wparam == 0x1b) {
        this->MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
        return 1;
    }
    if (wparam == 0x26) {
        this->text_block->RecallHistoryPrev();
    }
    return VisScreen::OnKeyDown(wparam);
}

// 43BD2E
void Vis1200::VMethod28() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->console_active = 1;
    memset(this->buffer, 0, 0x100);
    this->text_block->ResetInputState();
    VisScreen::VMethod28();
    this->active = 1;
    this->text_block->blink_last_tick = timeGetTime();
}

// 43BDCB
int32_t Vis1200::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    return VisScreen::MsgProc(msg, wparam, lparam);
}

// 43BD96
void Vis1200::DoClose(uint32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->console_active = 0;
    this->active = 0;
    VisScreen::DoClose(code);
}

// 43BE80
void Vis1200::VMethod7() {
    if (this->active != 0) {
        VisScreen::VMethod7();
    }
}

// 4972D0
int32_t Vis1200::GetMatchPlayerIndex() {
    return this->text_block->GetMatchPlayerIndex();
}

// 497310
int32_t Vis1200::GetMatchNameLength() {
    return this->text_block->GetMatchNameLength();
}

// 43BE73
void Vis1200::VMethod8(CRect* rect) {
}

// 43BB70
Vis1200::Vis1200(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : VisScreen(_id, l, t, r, b, nullptr) {
    this->VMethod26();
}

// 43C000
Vis1200::~Vis1200() {
}

// 4B12E3
void VisOrderToolbar::VMethod7() {
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
        LockSurface2();
        if (this->enabled == 0) {
            g_bmp_headsr->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
        } else {
            g_bmp_cmdbarr->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
            for (int32_t i = 0; i < 8; i++) {
                if ((this->avail_orders_mask & (1 << i)) == 0) {
                    int32_t x = (i & 3) * 0x22 + 8;
                    int32_t y = (i >> 2) * 0x22 + 7;
                    g_bmp_cmdempr->DrawSubRect(screen_rect.left + x, screen_rect.top + y, x, y, x + 0x22, y + 0x22);
                }
            }
            int32_t sel = (int32_t)this->selected_order;
            if (sel >= 0) {
                int32_t x = (sel & 3) * 0x22 + 8;
                int32_t y = (sel >> 2) * 0x22 + 7;
                g_bmp_cmddnr->DrawSubRect(screen_rect.left + x, screen_rect.top + y, x, y, x + 0x22, y + 0x22);
            }
        }
        UnlockSurface2();
        this->dirty = 0;
    }
}

// 4B107B
int32_t VisOrderToolbar::OnLButtonDown(uint32_t wparam, CPoint pos) {
    CRect screen_rect;
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        return 1;
    }
    this->ClientRectToScreen(&screen_rect, this->rect);
    VisPlayingArea* map = this->map_context;
    if (this->enabled != 0) {
        for (int32_t i = 0; i < 8; i++) {
            if ((this->avail_orders_mask & (1 << i)) != 0) {
                int32_t x = screen_rect.left + 8 + (i & 3) * 0x22;
                int32_t y = screen_rect.top + 7 + (i >> 2) * 0x22;
                CRect rc(x, y, x + 0x22, y + 0x22);
                if (rc.PtInRect(pos) != 0) {
                    if (this->selected_order == (uint32_t)i) {
                        return 1;
                    }
                    if (i == 4 && map->IsBookOpen() != 0) {
                        return 1;
                    }
                    this->selected_order = i;
                    this->VMethod9();
                    map->MsgProc(VIS_MSG_ORDER_MODE_CHANGED, i, 0);
                    return 1;
                }
            }
        }
        this->selected_order = 0xffffffff;
        this->VMethod9();
        map->MsgProc(VIS_MSG_ORDER_MODE_CHANGED, 0xffffffff, 0);
    }
    return 1;
}

// 4B0ED1
int32_t VisOrderToolbar::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    int32_t result = CVisualObject::MsgProc(msg, wparam, lparam);
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    if (result == 0) {
        switch (msg) {
        case VIS_MSG_FRAME_TICK:
            if (main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
                if (this->dirty != 0) {
                    this->VMethod9();
                }
                this->UpdateCursor();
            }
            break;
        case VIS_MSG_SET_MAP_CONTEXT:
            this->map_context = (VisPlayingArea*)wparam;
            break;
        case VIS_MSG_ORDERS_ENABLE:
            if (this->enabled == 0 || this->avail_orders_mask != wparam) {
                this->enabled = 1;
                this->dirty = 1;
                this->avail_orders_mask = wparam;
            }
            result = 1;
            break;
        case VIS_MSG_ORDERS_DISABLE:
            if (this->enabled != 0) {
                this->enabled = 0;
                this->dirty = 1;
            }
            result = 1;
            break;
        case VIS_MSG_ORDERS_CLEAR:
            if (this->enabled != 0) {
                this->selected_order = 0xffffffff;
                this->dirty = 1;
            }
            break;
        case VIS_MSG_TOOLBAR_ORDER_SELECT:
            if (this->enabled != 0 && this->selected_order != wparam && ((1 << wparam) & this->avail_orders_mask) != 0) {
                this->selected_order = wparam;
                this->dirty = 1;
            }
            result = 1;
            break;
        }
    }
    return result;
}

// 4B0D9A
const char* VisOrderToolbar::GetHint() {
    if (this->enabled != 0) {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        if ((main_wnd->dialogsMask & (DIALOG_MASK_POPUP | DIALOG_MASK_TAVERN | DIALOG_MASK_SHOP)) == 0 && main_wnd->drag_entry == nullptr) {
            CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
            CRect screen_rect;
            this->ClientRectToScreen(&screen_rect, this->rect);
            for (int32_t i = 0; i < 8; i++) {
                if ((this->avail_orders_mask & (1 << i)) != 0) {
                    int32_t x = screen_rect.left + 8 + (i & 3) * 0x22;
                    int32_t y = screen_rect.top + 7 + (i >> 2) * 0x22;
                    CRect rc(x, y, x + 0x22, y + 0x22);
                    if (rc.PtInRect(mouse_pt) != 0) {
                        return TxtFile::AllLines[i];
                    }
                }
            }
        }
    }
    return nullptr;
}

// 4B0CFC
VisOrderToolbar::VisOrderToolbar(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : CVisualObject(_id, l, t, r, b, nullptr) {
    this->selected_order = 0xffffffff;
    this->enabled = 0;
    this->dirty = 0;
}

// 4B4790
VisOrderToolbar::~VisOrderToolbar() {
}

// 4B12B3
int32_t VisOrderToolbar::OnMouseMove(uint32_t wparam, CPoint pos) {
    if ((wparam & 1) != 0) {
        this->OnLButtonDown(wparam, pos);
    }
    return 0;
}

// 4B1213
int32_t VisOrderToolbar::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
    return this->OnLButtonDown(wparam | 1, pos);
}

// 4B124C
int32_t VisOrderToolbar::OnLButtonUp(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        g_Cursors[0]->Use();
        main_wnd->ResetItemCursor();
    }
    return 1;
}

// 4B1287
int32_t VisOrderToolbar::OnRButtonUp(uint32_t wparam, CPoint pos) {
    return this->map_context->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);
}

// 4B1201
int32_t VisOrderToolbar::OnRButtonDown(uint32_t wparam, CPoint pos) {
    return 1;
}

// 4B123A
int32_t VisOrderToolbar::OnRButtonDblClk(uint32_t wparam, CPoint pos) {
    return 1;
}

// 4B1468
void VisOrderToolbar::UpdateCursor() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CSprite256* cur_sprite = g_mousept.GetCursorSprite();

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);

    CCursor* new_cursor = nullptr;
    CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
    if (screen_rect.PtInRect(mouse_pt)) {
        if (g_mousept.GetX() >= g_ScreenSize.right - 2 && main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
            if (g_mousept.GetY() == 0) {
                new_cursor = g_Cursors[CURSOR_ARROW1];
            } else if (g_mousept.GetY() < g_ScreenSize.bottom - 2) {
                new_cursor = g_Cursors[CURSOR_ARROW2];
            } else {
                new_cursor = g_Cursors[CURSOR_ARROW3];
            }
        } else {
            if (screen_rect.PtInRect(mouse_pt)) {
                new_cursor = g_Cursors[CURSOR_DEFAULT];
            }
        }
        if (main_wnd->drag_entry != nullptr) {
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




