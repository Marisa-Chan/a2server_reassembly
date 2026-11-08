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
#include "vis_menu.h"

char g_EmptyStrConfirmHint[4]; //659a54 "" hint string of the action menu confirm button
char g_EmptyStrCancelHint[4]; //659a58 "" hint string of the action menu cancel button
static int32_t startgame_tips_cycle_len = 1;      // 62CD00 frames per cycle for the active tips section (-1 = none)
// Main menu button layout. 60CC30 — [x, y, w, h] per button.
static const int32_t s_MainMenuButtonRects[8][4] = {
    {0xCC, 0x34, 0x68, 0x60}, {0x7C, 0x9C, 0x6C, 0x4C}, {0x7C, 0xFC, 0x60, 0x58},
    {0xD0, 0x154, 0x64, 0x64}, {0x154, 0x34, 0x58, 0x64}, {0x1A8, 0x98, 0x54, 0x58},
    {0x19C, 0x104, 0x60, 0x54}, {0x158, 0x15C, 0x48, 0x50},
};

// Main menu label layout. 60CCB0 — [x, y, w, h] per label.
static const int32_t s_MainMenuLabelRects[8][4] = {
    {0x74, 0x40, 0xD0, 0x8A}, {0x58, 0x58, 0xEC, 0x98}, {0x58, 0xEC, 0xEC, 0x98},
    {0x74, 0x110, 0xD0, 0x8C}, {0x140, 0x40, 0xD4, 0x8C}, {0x144, 0x58, 0xE8, 0x98},
    {0x144, 0xEC, 0xEC, 0x98}, {0x140, 0x110, 0xD4, 0x8C},
};

char g_EmptyStrLoadHint[4]; //659a4c "" hint string of the mission-failed load button
char g_EmptyStrRetryHint[4]; //659a50 "" hint string of the mission-failed retry button

// Statics for VisStartGame::VMethod7 (659514-659534 in the binary).
static uint8_t startgame_init_flags = 0;    // 659514 init-done flags for the three timestamps/delays below
static uint32_t startgame_blind_delay = 0;  // 659528 delay before the next blind-spawn event (ms)
static uint32_t startgame_blind_ts = 0;     // 65954C blind animation timestamp
static uint32_t startgame_frame_ts = 0;     // 659534 frame throttle timestamp

// Statics for VisStartGame::DrawTipsHighlight (65951C-659554, 62CCFC-62CD00 in the binary).
static uint8_t startgame_tips_init_flags = 0;     // 65951C init-done flags for the two timestamps below
static uint32_t startgame_tips_show_ts = 0;       // 659524 timestamp when the cursor entered the current hotspot
static uint32_t startgame_tips_frame_ts = 0;      // 65952C timestamp of the last highlight frame advance
static uint32_t startgame_tips_frame = 0;         // 659554 current highlight animation frame
static int32_t startgame_tips_last_hotspot = -1;  // 62CCFC hotspot id the highlight is currently locked to

const int32_t VisStartGame::PortraitPlayerTypes[4] = {0, 2, 3, 1};

// 4DCEBA
VisListBoxDiplomacy::~VisListBoxDiplomacy() {
    delete this->enemy_radios;
    delete this->ally_radios;
    delete this->mute_radios;
    // see_radios is intentionally not deleted, matching the original dtor.
}


// 4DC9FB
VisListBoxDiplomacy::VisListBoxDiplomacy(int32_t _id, const RECT& r, CArray<DiplomacyEntry*>* _diplomacy, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _entry_height)
: VisListBox(_id, r, _font, _clr1, _clr2, -1, nullptr) {
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
void VisListBoxDiplomacy::WriteRadioState() {
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
void VisListBoxDiplomacy::UpdateScrollBar() {
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
void VisListBoxDiplomacy::RestoreRect() {
    if (this->scrollbox_id >= 0) {
        this->rect.right += 0x1A;
    }
}


// 445bcb
VisDiplomacy::VisDiplomacy(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CArray<DiplomacyEntry*>* _payload)
: VisMessageBox(_id, l, t, r, b, *(CArray<DiplomacyEntry*>**)_payload, txt_dialogs.GetLine(0x4F), 1, txt_dialogs.GetLine(0x91)) {
    this->diplomacy = (CArray<DiplomacyEntry*>**)_payload;
}


// 44F9E0
VisDiplomacy::~VisDiplomacy() {
}


// 445D34
void VisDiplomacy::ReadData(const void* buf) {
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
CVisualObject* VisDiplomacy::VMethod30(const void* data, const RECT& r) {
    ((RECT&)r).top = 0x50;

    VisListBoxDiplomacy* listbox = new VisListBoxDiplomacy(2, r, (CArray<DiplomacyEntry*>*)data, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0);
    this->AddChild(listbox);
    listbox->UpdateScrollBar();

    listbox->SetCaptionLabel((VisLabel*)this->FindChild(-1));
    return listbox;
}


// 44E875
VisMissionFailed::VisMissionFailed(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CString& _txt)
    : VisMessageBoxWithList(_id, l, t, r, b, _txt, TxtFile::AllLines[0x8D], 0xFFFF) {
}

// 450B00 (this address is the deleting dtor ??_G; the real dtor 450B30 only calls the base dtor)
VisMissionFailed::~VisMissionFailed() = default;

// 44E8CA
void VisMissionFailed::VMethod26() {
    VisMessageBox::VMethod26();
    this->rect.bottom += 0x40;

    CRect retry_rect(this->rect.Width() / 8, this->rect.Height() - 0x60, this->rect.Width() * 7 / 8, this->rect.Height() - 0x48);
    CRect load_rect(this->rect.Width() / 8, this->rect.Height() - 0x48, this->rect.Width() * 7 / 8, this->rect.Height() - 0x30);

    VisButton* retry_btn = new VisButton(4, retry_rect, txt_dialogs.GetLine(0x2C), g_font1, nullptr, 0x445, 0, g_EmptyStrLoadHint);
    this->AddChild(retry_btn);
    VisButton* load_btn = new VisButton(5, load_rect, txt_dialogs.GetLine(0x23), g_font1, nullptr, 0x446, 0, g_EmptyStrRetryHint);
    this->AddChild(load_btn);

    this->SetUpObj(this->FindChild(4));
    if (AppHasAnySaveFile() == 0) {
        load_btn->ChangeFlags(1, false);
    }
}

// 44EABB
VisCutScenesDlg::VisCutScenesDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CStringArray& _payload)
    : VisMessageBox(_id, l, t, r, b, (const void*)&_payload, nullptr, 1, txt_dialogs.GetLine(0x99)) {
}

// 450B50 (this address is the deleting dtor ??_G; the real dtor 450B80 only calls the base dtor)
VisCutScenesDlg::~VisCutScenesDlg() = default;

// 44EB08
CVisualObject* VisCutScenesDlg::VMethod30(const void* data, const RECT& r) {
    RECT& rect = const_cast<RECT&>(r);
    rect.right -= 0x18;

    VisListBox* list = new VisListBox(2, r, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0x29B, nullptr);
    list->ReadData(&data);
    list->SetSelectedIndex(g_SelCutsceneIndex);
    this->AddChild(list);

    VisScrollBar* scrollbar = new VisScrollBar(0x29B, rect.right, rect.top, rect.right + 0x18, rect.bottom, nullptr);
    this->AddChild(scrollbar);

    return list;
}


// 445CF1
void VisDiplomacy::VMethod31(int32_t code) {
    if (code == VIS_MSG_HIDE_SCREEN) {
        ((VisListBoxDiplomacy*)this->FindChild(2))->WriteRadioState();
    }
}


QuestObjectivesHeaderDialogVisualObject::QuestObjectivesHeaderDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisMessageBoxWithList(_id, l, t, r, b, g_MissionBriefing, txt_dialogs.GetLine(68), 0xffff)
{} //445173

void QuestObjectivesHeaderDialogVisualObject::VMethod26() { //4451e1
    VisMessageBox::VMethod26();

    CStringArray sarr;

    CRect r(0, 0, this->rect.Width() - 128, 480);
    int32_t pos = this->FindChild(2)->GetRect().bottom + 16;

    for (int i = 0; i < g_MissionSubjs.GetSize(); i++) {
        sarr.Copy(g_font1->StringArrayForRect(r, g_MissionSubjs[i]));

        CRect opos(48, pos - 3, 72, pos + 21);
        uint8_t bits = ScenarioGetVar(752 + i);
        if (bits) {
            uint16_t* colorsh = nullptr;
            CGameBitmap* bitmap = nullptr;
            VisBitmap* vis_bitmap = nullptr;

            if (bits & 2) {
                colorsh = clrsh_ShockingBlack;
                bitmap = new CSprite256("graphics\\interface\\subobj.256");
                bitmap->ResetPalette(1, 1, 0);
                vis_bitmap = new VisBitmap(pos + 1, r, bitmap, 11);
            } else if (bits & 4) {
                colorsh = clrsh_CoralRed;
                bitmap = new CSprite256("graphics\\interface\\subobj.256");
                bitmap->ResetPalette(1, 1, 0);
                vis_bitmap = new VisBitmap(pos + 1, r, bitmap, 12);
            } else if (bits & 1) {
                colorsh = clrsh_TechBlack;
                bitmap = new CSprite256("graphics\\interface\\subobj.256");
                bitmap->ResetPalette(1, 1, 0);
                vis_bitmap = new VisBitmap(pos + 1, r, bitmap, 10);
            }

            this->AddChild(new VisMultiText(pos, 80, pos, this->rect.Width() - 48, pos + sarr.GetSize() * (g_font1->GetHeight() + 4), g_MissionSubjs[i], g_font1, colorsh, 0));
            if (vis_bitmap) {
                this->AddChild(vis_bitmap);
            }

            int32_t dy = 10 + sarr.GetSize() * (g_font1->GetHeight() + 4);
            pos += dy;
            this->rect.bottom += dy;
        }
    }

    this->rect.bottom += 40;
    VisWindow::UpdateWinRect();

    int32_t rh = this->rect.Height();
    int32_t rw = this->rect.Width();

    CRect cr(rw / 2 - 48, rh - 60, rw / 2 + 48, rh - 36);
    this->AddChild(new VisButton(4, cr, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "")); //accept
}


// 4E2F76
void VisQuestStatus::VMethod7() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    QuestMap* quest_map = main_wnd->vis_map_context->quest_map;

    this->rect.bottom = this->rect.top + 0x48 + quest_map->GetQuestCount() * 0x20;

    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    screen_rect.right -= 8;
    screen_rect.bottom -= 8;

    LockSurface2();

    gfx_interface_lm->DrawBrightened(screen_rect.right - 0x18, screen_rect.top + 8, 0xC, 6, 0);
    gfx_interface_lm->DrawBrightened(screen_rect.left + 8, screen_rect.bottom - 0x18, 0xF, 6, 0);
    gfx_interface_lm->DrawBrightened(screen_rect.right - 0x18, screen_rect.bottom - 0x18, 0x11, 6, 0);
    for (int32_t i = 0; i < (screen_rect.Width() - 0x40) / 0x30; i++) {
        gfx_interface_lm->DrawBrightened(screen_rect.left + 0x28 + i * 0x30, screen_rect.bottom - 0x18, 0x10, 6, 0);
    }
    for (int32_t i = 0; i < (screen_rect.Height() - 0x40) / 0x20; i++) {
        gfx_interface_lm->DrawBrightened(screen_rect.right - 0x18, screen_rect.top + 0x28 + i * 0x20, 0xE, 6, 0);
    }

    gfx_interface_lm->Draw(screen_rect.left, screen_rect.top, 0xA, 0, 0);
    gfx_interface_lm->Draw(screen_rect.right - 0x20, screen_rect.top, 0xC, 0, 0);
    gfx_interface_lm->Draw(screen_rect.left, screen_rect.bottom - 0x20, 0xF, 0, 0);
    gfx_interface_lm->Draw(screen_rect.right - 0x20, screen_rect.bottom - 0x20, 0x11, 0, 0);
    for (int32_t i = 0; i < (screen_rect.Width() - 0x40) / 0x30; i++) {
        gfx_interface_lm->Draw(screen_rect.left + 0x20 + i * 0x30, screen_rect.top, 0xB, 0, 0);
        gfx_interface_lm->Draw(screen_rect.left + 0x20 + i * 0x30, screen_rect.bottom - 0x20, 0x10, 0, 0);
    }
    for (int32_t i = 0; i < (screen_rect.Height() - 0x40) / 0x20; i++) {
        gfx_interface_lm->Draw(screen_rect.left, screen_rect.top + 0x20 + i * 0x20, 0xD, 0, 0);
        gfx_interface_lm->Draw(screen_rect.right - 0x20, screen_rect.top + 0x20 + i * 0x20, 0xE, 0, 0);
    }
    for (int32_t i = 0; i < (screen_rect.Width() - 0x40) / 0x30; i++) {
        for (int32_t j = 0; j < (screen_rect.Height() - 0x40) / 0x20; j++) {
            gfx_interface_lm->Draw(screen_rect.left + 0x20 + i * 0x30, screen_rect.top + 0x20 + j * 0x20, 9, 0, 0);
        }
    }

    int32_t col_x = this->rect.left + 0x20;
    int32_t row_y = this->rect.top + 0x28;

    if (quest_map->GetQuestCount() != 0) {
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
            FillRectBrightness(col_x + 0x16, row_y - 2, col_x + this->rect.Width() - 0x46, row_y + 0x18, 4);
        }

        CString unit_name;
        switch (quest->Kind()) {
        case 1:
        case 4:
        case 0xB: {
            CGameObject* obj;
            if (main_wnd->vis_map_context->objects.Lookup((uint16_t)quest->GetObj(), obj)) {
                CUnit* unit = (CUnit*)obj;
                if ((unit->unitFlags & (UNIT_FLAG_PLAYER_CLASS | UNIT_FLAG_CREATURE)) == 0) {
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
        if (main_wnd->vis_map_context->objects.Lookup((uint16_t)quest->GetLandmarkId(), landmark)) {
            building_name = txt_building.GetLine(landmark->typeId - 1);
            int32_t cell_x = ((landmark->tileX - 8) * 5) / (main_wnd->vis_map_context->map_width - 0x10);
            int32_t cell_y = ((landmark->tileY - 8) * 5) / (main_wnd->vis_map_context->map_height - 0x10);
            area_name = TxtFile::AllLines[cell_x + 0x13D + cell_y * 5];
        }

        CString quest_text;
        switch (quest->Kind()) {
        case 1:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], (const char*)unit_name, (const char*)area_name, (const char*)building_name);
            break;
        case 2:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->GetLimit(), (const char*)unit_name);
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
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->GetLimit() / 0x3C0, (quest->GetLimit() >> 4) % 0x3C, (const char*)area_name);
            break;
        case 8:
        case 9:
        case 0xA:
            quest_text.Format(TxtFile::AllLines[quest->Kind() + 0x11C], quest->GetLimit());
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
        this->icon->Draw(col_x - 6, row_y - 2, icon_index, 0, 0);

        row_y += 0x20;
    }

    UnlockSurface2();
}


// 4E4530
int32_t VisQuestStatus::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    return 0;
}


// 4E2E3D
VisQuestStatus::VisQuestStatus(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) {
    this->icon = new CSprite256("graphics\\interface\\subobj.256");
    this->icon->ResetPalette(1, 1, 0);
}


// 4E2EF4
VisQuestStatus::~VisQuestStatus() {
    delete this->icon;
}


// 4e27cd
void VisTipsDialog::VMethod8(CRect* rect) {
    CRect r;
    this->ClientRectToScreen(&r, this->rect);

    r.right -= 8;
    r.bottom -= 8;

    CRect tmp;
    GetClipRect(&tmp);
    SetClipRect(*rect);
    LockSurface2();

    gfx_interface_lm->DrawBrightened(r.right - 24, r.top + 8, 0xC, 6, 0);
    gfx_interface_lm->DrawBrightened(r.left + 8, r.bottom - 24, 0xF, 6, 0);
    gfx_interface_lm->DrawBrightened(r.right - 24, r.bottom - 24, 0x11, 6, 0);

    for (int32_t i = 0; i < (r.Width() - 0x40) / 0x30; i++) {
        gfx_interface_lm->DrawBrightened(r.left + 0x28 + i * 0x30, r.bottom - 0x18, 0x10, 6, 0);
    }

    for (int32_t i = 0; i < (r.Height() - 0x40) / 0x20; i++) {
        gfx_interface_lm->DrawBrightened(r.right - 0x18, r.top + 0x28 + i * 0x20, 0xE, 6, 0);
    }

    gfx_interface_lm->Draw(r.left, r.top, 0xA, 0, 0);
    gfx_interface_lm->Draw(r.right - 0x20, r.top, 0xC, 0, 0);
    gfx_interface_lm->Draw(r.left, r.bottom - 0x20, 0xF, 0, 0);
    gfx_interface_lm->Draw(r.right - 0x20, r.bottom - 0x20, 0x11, 0, 0);

    for (int32_t i = 0; i < (r.Width() - 0x40) / 0x30; i++) {
        gfx_interface_lm->Draw(r.left + 0x20 + i * 0x30, r.top, 0xB, 0, 0);
        gfx_interface_lm->Draw(r.left + 0x20 + i * 0x30, r.bottom - 0x20, 0x10, 0, 0);
    }

    for (int32_t i = 0; i < (r.Height() - 0x40) / 0x20; i++) {
        gfx_interface_lm->Draw(r.left, r.top + 0x20 + i * 0x20, 0xD, 0, 0);
        gfx_interface_lm->Draw(r.right - 0x20, r.top + 0x20 + i * 0x20, 0xE, 0, 0);
    }

    for (int32_t i = 0; i < (r.Width() - 0x40) / 0x30; i++) {
        for (int32_t j = 0; j < (r.Height() - 0x40) / 0x20; j++) {
            gfx_interface_lm->Draw(r.left + 0x20 + i * 0x30, r.top + 0x20 + j * 0x20, 9, 0, 0);
        }
    }

    UnlockSurface2();
    SetClipRect(tmp);
}


// 4e2b73
void VisTipsDialog::SetText(const char* text) {
    VisMultiText* mt = (VisMultiText*)this->FindChild(MAP_CHILD_TIPS_TEXT);
    mt->SetText(text);
}


// 4e2b95
int32_t VisTipsDialog::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    // client 4e7519: the radio id is build-shifted (0xF server, 0x10 client).
    if (msg == VIS_MSG_SET_OPTION && wparam == MAP_CHILD_TIPS_SHOW_RADIO) {
        g_settings.TipsMode = lparam;
    }

    if (msg == 0x100 || msg == VIS_MSG_HIDE_SCREEN || msg == VIS_MSG_CLOSE_SCREEN) {
        return 0;
    }

    if (msg == 0x202) {
        CPoint pos(lparam & 0xFFFF, lparam >> 0x10);
        this->OnLButtonUp(wparam, pos);
    }

    return VisScreen::MsgProc(msg, wparam, lparam);
}


// 4e2c42
int32_t VisTipsDialog::OnLButtonUp(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->drag_entry != nullptr) {
        if ((main_wnd->dialogsMask & DIALOG_MASK_SHOP) != 0) {
            CVisualObject* obj = main_wnd->vis_root->FindChild(1000);
            if (obj != nullptr) {
                ((VisShop*)obj)->shop_compass->OnLButtonUp(wparam, pos);
                return 1;
            }
        } else {
            if (main_wnd->dialogsMask == DIALOG_MASK_INGAME) {
                main_wnd->vis_map_context->OnLButtonUp(wparam, pos);
                return 1;
            }
        }
    }
    return 0;
}


// 4e2541
VisTipsDialog::VisTipsDialog(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* txt)
: VisScreen(_id, l, t, r, b, nullptr) {
    this->InitTipsDialog(l, t, r, b, txt);
}

// 4e25c1
void VisTipsDialog::InitTipsDialog(int32_t l, int32_t t, int32_t r, int32_t b, const char* txt) {
    VisMultiText* multitext = new VisMultiText(MAP_CHILD_TIPS_TEXT, 0x14, 0x18, this->rect.Width() - 0x1C, this->rect.Height() - 0x24, txt, g_font2, clrsh_DullGold, 0);
    this->AddChild(multitext);

    VisButton* ok_btn = new VisButton(MAP_CHILD_TIPS_OK_BUTTON, this->rect.Width() - 0x78, this->rect.Height() - 0x28, this->rect.Width() - 0x28, this->rect.Height() - 0x16, TxtFile::AllLines[0x7F], g_font2, clrsh_DullGold, 0x45A, 0, nullptr);
    this->AddChild(ok_btn);

    VisServerScreenRadio* radio = new VisServerScreenRadio(MAP_CHILD_TIPS_SHOW_RADIO, 0x28, this->rect.Height() - 0x28, this->rect.Width() - 0x7C, this->rect.Height() - 0x18, g_font2, clrsh_DullGold, nullptr);
    radio->AddEntry(TxtFile::AllLines[0x80]);
    this->AddChild(radio);

    int32_t selected = 1;
    radio->ReadData(&selected);
}

// 4e4510
VisTipsDialog::~VisTipsDialog() {
}

IngameMenu::IngameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, uint32_t unk, const CRect& _r)
: VisMenuWnd(_id, l, t, r, b, _bitmap, unk, _r) { //451990
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    //Save game
    MenuButton* btn = new MenuButton(1, txt_dialogs.GetLine(34), g_font1, 0, 0x41a, 'S', "");

    if (mwnd->sessionMode == SESSION_SINGLEPLAYER || mwnd->sessionMode == SESSION_CLIENT) {
        btn->ChangeFlags(FLAG_ENABLED, false);
    }

    this->AddElement(btn, 30);

    if (mwnd->sessionMode == SESSION_LOCAL_BATTLE) {
        //Load game
        btn = new MenuButton(2, txt_dialogs.GetLine(35), g_font1, 0, 0x418, 'L', "");

        if (AppHasAnySaveFile() == 0) {
            btn->ChangeFlags(FLAG_ENABLED, false);
        }
    } else {
        //Diplomacy
        btn = new MenuButton(7, txt_dialogs.GetLine(76), g_font1, 0, 0x43c, 'D', "");
    }

    this->AddElement(btn, 30);


    //options
    btn = new MenuButton(3, txt_dialogs.GetLine(36), g_font1, 0, 0x41b, 'O', "");
    this->AddElement(btn, 30);

    //sound options
    btn = new MenuButton(4, txt_dialogs.GetLine(37), g_font1, 0, 0x422, 'N', "");

    if (mwnd->music_player->GetState() == 5) {
        btn->ChangeFlags(FLAG_ENABLED, false);
    }

    this->AddElement(btn, 30);


    //objectives
    if (mwnd->net_session.internet_game == 0 || mwnd->hat_settings.ishat == 0 || mwnd->hat_settings.deathmatch != 3)
        btn = new MenuButton(5, txt_dialogs.GetLine(38), g_font1, 0, 0x420, 'M', "");
    else
        btn = new MenuButton(5, txt_patch.GetLine(209), g_font1, 0, 0x420, 'C', "");

    this->AddElement(btn, 30);


    //end mission
    btn = new MenuButton(6, txt_dialogs.GetLine(39), g_font1, 0, 0x41c, 'E', "");

    this->AddElement(btn, 30);


    //resume
    btn = new MenuButton(8, txt_dialogs.GetLine(40), g_font1, 0, 0x446, 'R', "");

    this->AddElement(btn, 30);
}



int32_t LoadGameWindow::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) { //43f63c
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);

    case VIS_MSG_LIST_ACTIVATE:
    case VIS_MSG_CONFIRM: {
        if (this->selected_index < 0) {
            return VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
        }

        CStringArray arr;
        ((VisListBox*)this->FindChild(3))->WriteData(&arr);

        strcpy(this->out_info->title, arr[this->selected_index]);
        strcpy(this->out_info->filename, this->file_names[this->selected_index]);

        return VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
    }

    case VIS_MSG_SET_OPTION:
        if (wparam == 3) {
            this->selected_index = lparam;
        }
        return 1;

    case VIS_MSG_LIST_SECONDARY: {
        VisListBox* lb = (VisListBox*)this->FindChild(3);
        int32_t idx = lb->GetSelectedIndex();
        if (idx > -1) {
            CString str = txt_patch.GetLine(50) + lb->GetItem(idx) + txt_patch.GetLine(51); // want to delete save ### ?
            mwnd->vis_msgbox_list = new VisMessageBoxWithList(1, 64, 100, 380, 594, str, nullptr, 4);
            mwnd->ModalScreen(mwnd->vis_msgbox_list);
            if (mwnd->vis_msgbox_list->GetCloseCode() == VIS_MSG_BOX_OK) {
                DeleteFileA(this->file_names[idx]);
                this->file_names.RemoveAt(idx);
                this->save_times.RemoveAt(idx);

                lb->RemoveItem(idx);
                lb->SelectItem(idx);

                this->MsgProc(VIS_MSG_SET_OPTION, lb->GetId(), lb->GetSelectedIndex());

                if (this->file_names.GetSize() == 0) {
                    CVisualObject* obj = this->FindChild(4);
                    obj->ChangeFlags(FLAG_ENABLED, false);
                    obj->VMethod9();

                    obj = this->FindChild(6);
                    obj->ChangeFlags(FLAG_ENABLED, false);
                    obj->VMethod9();
                }

            }
        }
        return 1;
    }

    case VIS_MSG_CANCEL:
        VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
        return 1;
    }

    return 1;
}

void LoadGameWindow::VMethod26() { //43ed94

    this->AddChild(new VisLabel(1, 40, 32, this->rect.Width() - 40, 56, txt_dialogs.GetLine(151), g_font1, p_clrsh_Black, 2)); //restore saved game

    VisLabel* caption = new VisLabel(2, 40, 68, this->rect.Width() - 40, 92, txt_dialogs.GetLine(24), g_font1, p_clrsh_Black, 0);//saved games
    this->AddChild(caption); 

    HintedListBox* list = new HintedListBox(3, 40, 92, this->rect.Width() - 64, 284, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 10, txt_dialogs.GetLine(25)); //select game for restore

    CArray<WIN32_FIND_DATAA> files;
    AppFindSavesList(&files, 1);

    for (int i = 0; i < files.GetSize(); i++) {
        WIN32_FIND_DATAA* inf = &files[i];
        FILE* f = fopen(inf->cFileName, "rb"); //WAT , LOL!

        uint32_t headers[2];
        fread(headers, 4, 2, f);
        if (headers[0] == 0x26677342) {
            fseek(f, headers[1], SEEK_SET);

            char savename[257];
            fread(savename, 1, 256, f);
            savename[256] = 0;

            list->AddItem(savename);
            this->file_names.Add(inf->cFileName);

            CTime t(inf->ftLastWriteTime);
            CString str;
            str.Format("%s  %02d.%02d.%d  %02d:%02d:%02d", savename, t.GetDay(), t.GetMonth(), t.GetYear(), t.GetHour(), t.GetMinute(), t.GetSecond());
            this->save_times.Add(str);
        } else {
            inf->cFileName[0] = 0; // disable this save
        }

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

    this->AddChild(scrl);
    this->AddChild(list);
    list->SetCaptionLabel(caption);
    list->SetHints(&this->save_times);

    CRect local_17c((this->rect.Width() * 3) / 20, lr.bottom + 24, (this->rect.Width() * 7) / 20, lr.bottom + 48);
    CRect local_190((this->rect.Width() * 8) / 20, lr.bottom + 24, (this->rect.Width() * 12) / 20, lr.bottom + 48);
    CRect local_1a0((this->rect.Width() * 13) / 20, lr.bottom + 24, (this->rect.Width() * 17) / 20, lr.bottom + 48);

    VisButton* btn = new VisButton(4, local_17c, txt_dialogs.GetLine(0), g_font1, nullptr, 0x479, 0, txt_dialogs.GetLine(26));
    this->AddChild(btn);

    btn->ChangeFlags(FLAG_10, true);

    if (this->file_names.GetSize() == 0) {
        btn->ChangeFlags(FLAG_ENABLED, false);
    }

    VisButton* btn2 = new VisButton(6, local_190, txt_dialogs.GetLine(157), g_font1, nullptr, 0x475, 0, txt_dialogs.GetLine(158));
    this->AddChild(btn2);

    VisButton* btn3 = new VisButton(5, local_1a0, txt_dialogs.GetLine(1), g_font1, nullptr, 0x47a, 0, txt_dialogs.GetLine(27));
    this->AddChild(btn3);

    btn->SetRightObj(btn3);
    btn->SetLeftObj(btn2);
    btn->SetUpObj(list);
    btn3->SetUpObj(list);
}

LoadGameWindow::LoadGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk)
: VisWindow(_id, l, t, r, b, _bitmap) { //43ecfb
    this->out_info = unk;
    this->selected_index = 0;
}

int32_t SaveGameWindow::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) { //440726
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);

    case VIS_MSG_LIST_ACTIVATE:
    case VIS_MSG_CONFIRM: {
        if (this->FindChild(4)->TestFlags(FLAG_ENABLED) != 0) {
            char buf[256];
            this->FindChild(1)->WriteData(buf);

            strcpy(this->out_info->title, buf);

            CStringArray arr;
            ((VisListBox*)this->FindChild(3))->WriteData(&arr);

            for (int i = 1; i < arr.GetSize(); i++) {
                if (strcmp(arr[i], buf) == 0) {
                    this->selected_index = i;
                    break;
                }
            }

            strcpy(this->out_info->filename, this->file_names[this->selected_index]);

            VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        }
        return 1;
    }

    case VIS_MSG_SET_OPTION:
        if (wparam == 3) {
            this->selected_index = lparam;
            CStringArray local_158;

            const char* str = "";
            if (this->selected_index >= 1) {
                this->FindChild(3)->WriteData(&local_158);
                str = local_158[this->selected_index];
            }
            CVisualObject* obj = this->FindChild(1);
            obj->ReadData(str);
            obj->VMethod9();
            this->CheckInput();
            return 1;
        } else if (wparam == 1) {
            this->CheckInput();
        }
        return VisScreen::MsgProc(msg, wparam, lparam);

    case VIS_MSG_LIST_SECONDARY: {
        VisListBox* lb = (VisListBox*)this->FindChild(3);
        int32_t idx = lb->GetSelectedIndex();
        if (idx != 0) {
            CString str = txt_patch.GetLine(50) + lb->GetItem(idx) + txt_patch.GetLine(51); // want to delete save ### ?
            mwnd->vis_msgbox_list = new VisMessageBoxWithList(1, 64, 100, 380, 594, str, nullptr, 4);
            mwnd->ModalScreen(mwnd->vis_msgbox_list);
            if (mwnd->vis_msgbox_list->GetCloseCode() == VIS_MSG_BOX_OK) {
                DeleteFileA(this->file_names[idx]);
                this->file_names.RemoveAt(idx);
                this->save_times.RemoveAt(idx);

                lb->RemoveItem(idx);
                lb->SelectItem(idx);

                this->MsgProc(VIS_MSG_SET_OPTION, lb->GetId(), lb->GetSelectedIndex());
            }
        }
        return 1;
    }

    case VIS_MSG_CANCEL:
        VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
        return 1;
    }

    return 1;
}

int __cdecl SaveGameWindow::Compare(void const* a, void const* b) { //43faf4
    if (*(const int*)a > *(const int*)b) {
        return 1;
    } else if (*(const int*)a < *(const int*)b) {
        return -1;
    }
    return 0;
}

void SaveGameWindow::VMethod26() { //43fb2c

    this->AddChild(new VisLabel(2, 40, 32, this->rect.Width() - 40, 56, txt_dialogs.GetLine(29), g_font1, p_clrsh_Black, 2));

    VisLabel* caption = new VisLabel(0, 40, 104, this->rect.Width() - 40, 128, txt_dialogs.GetLine(144), g_font1, p_clrsh_Black, 0);//saved games
    this->AddChild(caption);

    VisTextBox* textbox = new VisTextBox(1, 40, 68, this->rect.Width() - 40, 92, g_font1, p_clrsh_Black, txt_dialogs.GetLine(30));
    this->AddChild(textbox);

    HintedListBox* list = new HintedListBox(3, 40, 128, this->rect.Width() - 64, 272, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 10, txt_dialogs.GetLine(31)); //saved games

    textbox->SetDownObj(list);

    list->AddItem(this->empty_cell_text);
    this->file_names.Add("");
    this->save_times.Add("");

    CArray<WIN32_FIND_DATAA> files;
    AppFindSavesList(&files, 0);

    for (int i = 0; i < files.GetSize(); i++) {
        WIN32_FIND_DATAA* inf = &files[i];
        FILE* f = fopen(inf->cFileName, "rb"); //WAT , LOL!

        uint32_t headers[2];
        fread(headers, 4, 2, f);
        if (headers[0] == 0x26677342) {
            fseek(f, headers[1], SEEK_SET);

            char savename[257];
            fread(savename, 1, 256, f);
            savename[256] = 0;

            list->AddItem(savename);
            this->file_names.Add(inf->cFileName);

            CTime t(inf->ftLastWriteTime);
            CString str;
            str.Format("%s  %02d.%02d.%d  %02d:%02d:%02d", savename, t.GetDay(), t.GetMonth(), t.GetYear(), t.GetHour(), t.GetMinute(), t.GetSecond());
            this->save_times.Add(str);
        } else {
            inf->cFileName[0] = 0; // disable this save
        }

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
    int sz = this->file_names.GetSize() - 1;
    if (sz >= 1) {
        int* ids = new int[sz];
        for (int i = 0; i < sz; i++) {
            CString& s = this->file_names[i + 1];
            ids[i] = (s[4] - '0') * 1000 + (s[5] - '0') * 100 + (s[6] - '0') * 10 + (s[7] - '0');
        }
        qsort(ids, sz, sizeof(int), Compare);
        next_save_id = ids[sz - 1] + 1;
        for (int i = 0; i < sz - 1; i++) {
            if (ids[i] != i) {
                next_save_id = i;
                break;
            }
        }
        delete[] ids;
    }

    this->file_names[0].Format("game%04ld.sav", next_save_id);

    CRect& lr = list->GetRect();
    VisScrollBar* scrl = new VisScrollBar(10, lr.right, lr.top, lr.right + 24, lr.bottom, nullptr);

    this->AddChild(scrl);
    this->AddChild(list);
    list->SetCaptionLabel(caption);
    list->SetHints(&this->save_times);

    CRect local_17c((this->rect.Width() * 3) / 20, lr.bottom + 24, (this->rect.Width() * 7) / 20, lr.bottom + 48);
    CRect local_190((this->rect.Width() * 8) / 20, lr.bottom + 24, (this->rect.Width() * 12) / 20, lr.bottom + 48);
    CRect local_1a0((this->rect.Width() * 13) / 20, lr.bottom + 24, (this->rect.Width() * 17) / 20, lr.bottom + 48);

    VisButton* btn = new VisButton(4, local_17c, txt_dialogs.GetLine(0), g_font1, nullptr, 0x479, 0, txt_dialogs.GetLine(32));
    this->AddChild(btn);

    btn->ChangeFlags(FLAG_10, true);

    if (this->file_names.GetSize() == 0) {
        btn->ChangeFlags(FLAG_ENABLED, false);
    }

    VisButton* btn2 = new VisButtonSaveGame(6, local_190, txt_dialogs.GetLine(157), g_font1, nullptr, 0x475, 0, txt_dialogs.GetLine(158));
    this->AddChild(btn2);

    VisButton* btn3 = new VisButtonSaveGame(5, local_1a0, txt_dialogs.GetLine(1), g_font1, nullptr, 0x47a, 0, txt_dialogs.GetLine(33));
    this->AddChild(btn3);

    btn->SetRightObj(btn3);
    btn->SetLeftObj(btn2);
    btn->SetUpObj(list);
    btn3->SetUpObj(list);

    this->CheckInput();
}

SaveGameWindow::SaveGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk)
    : VisWindow(_id, l, t, r, b, _bitmap) { //43fa42
    this->out_info = unk;
    this->selected_index = 0;
    this->empty_cell_text = txt_dialogs.GetLine(28); //empty cell
}

void SaveGameWindow::CheckInput() { //440cc4
    char buf[256];
    this->FindChild(1)->WriteData(buf);

    CVisualObject* obj = this->FindChild(4);
    if (strlen(buf) == 0) {
        obj->ChangeFlags(FLAG_ENABLED, false);
    } else {
        obj->ChangeFlags(FLAG_ENABLED, true);
    }

    obj->VMethod9();
}


int32_t VisButtonSaveGame::OnKeyDown(uint32_t wparam) { //44f5f0
    if (this->TestFlags(FLAG_FOCUS) == 0) {
        return 0;
    }
    return VisButton::OnKeyDown(wparam);
}

VisButtonSaveGame::VisButtonSaveGame(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint)
    : VisButton(_id, r, _caption, _font, _clr, _msgid, _charid, hint) { //44f5a0
}

VisButtonSaveGame::~VisButtonSaveGame() { //44f650
}


int32_t GameOptionsWindow::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) { //443cd7
    int32_t oldshtfl = g_settings.ShowTimeFlow;

    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    if (msg == VIS_MSG_HIDE_SCREEN) {
        VisScrollBar::Data dat;
        this->FindChild(2)->WriteData(&dat);
        mwnd->SetSpeed(dat.v);

        if (mwnd->sessionMode == SESSION_LOCAL_BATTLE) {
            this->FindChild(13)->WriteData(&g_settings.TipsMode);
        } else {
            this->FindChild(13)->WriteData(&g_settings.ClanNames);
        }

        this->FindChild(12)->WriteData(&g_settings.ShowTimeFlow);
        this->FindChild(3)->WriteData(&g_settings.Smoothing);
        this->FindChild(31)->WriteData(&g_Shadows);
        this->FindChild(32)->WriteData(&g_Lightning);
        this->FindChild(33)->WriteData(&g_Animation);

        if (g_Animation == 0) {
            g_Lightning = 0;
        }

        this->FindChild(4)->WriteData(g_settings.pShowAllHitPoints);
        this->FindChild(5)->WriteData(g_settings.pShowFlyingHP);
        this->FindChild(41)->WriteData(&g_MessageColors);

        SetMessageColors(g_MessageColors);

        this->FindChild(7)->WriteData(g_settings.pFormationMode);
        this->FindChild(9)->WriteData(g_settings.pWimpyMode);

        g_settings.AutoCasting = 8;

        int32_t sel;
        this->FindChild(141)->WriteData(&sel);

        if (sel == 1) {
            g_settings.AutoCasting |= 0x18;
        } else if (sel == 2) {
            g_settings.AutoCasting |= 0x38;
        }

        this->FindChild(131)->WriteData(&sel);
        if (sel != 0) {
            g_settings.AutoCasting |= 1;
        }

        this->FindChild(132)->WriteData(&sel);
        if (sel != 0) {
            g_settings.AutoCasting |= 2;
        }

        this->FindChild(133)->WriteData(&sel);
        if (sel != 0) {
            g_settings.AutoCasting |= 4;
        }


        mwnd->vis_map_context->SendFormationMode(*g_settings.pFormationMode % 3);
        mwnd->vis_map_context->SendWimpyMode(*g_settings.pWimpyMode % 3);
        mwnd->vis_map_context->SendAutoCastSetting();
    }
    if (g_settings.ShowTimeFlow != oldshtfl) {
        mwnd->vis_map_context->UpdateDayNightCycle(1);
    }

    return VisScreen::MsgProc(msg, wparam, lparam);
}

void GameOptionsWindow::VMethod26() { //442646
    
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    this->AddChild( new VisLabel(-1, 40, 20, this->rect.Width() - 40, 40, txt_dialogs.GetLine(150), g_font1, p_clrsh_Black, 2) );

    VisLabel* caption = new VisLabel(1, 40, 56, 232, 80, txt_dialogs.GetLine(50), g_font1, p_clrsh_Black, 2);
    this->AddChild(caption);

    VisScrollBar* scrl = new VisScrollBar(2, 40, 84, 232, 108, txt_dialogs.GetLine(51));
    this->AddChild(scrl);

    if (mwnd->sessionMode == SESSION_SINGLEPLAYER) {
        scrl->ChangeFlags(FLAG_ENABLED, false);
    }

    VisScrollBar::Data sd;
    sd.v = *g_settings.pGameSpeed;
    sd.vmax = 8;
    scrl->ReadData(&sd);

    scrl->SetCaptionLabel(caption);

    CRect& scr = scrl->GetRect();

    VisRadioType1* daych = new VisRadioType1(12, scr.left, scr.bottom + 12, scr.right, scr.bottom + 36, g_font1, p_clrsh_Black, nullptr);
    daych->AddEntry(txt_dialogs.GetLine(55));
    this->AddChild(daych);

    daych->ReadData(&g_settings.ShowTimeFlow);
    daych->SetUpObj(scrl);

    VisRadioType1* smooth = new VisRadioType1(3, scr.left, scr.bottom + 40, scr.right, scr.bottom + 64, g_font1, p_clrsh_Black, nullptr);
    smooth->AddEntry(txt_dialogs.GetLine(53));
    this->AddChild(smooth);

    smooth->ReadData(&g_settings.Smoothing);
    smooth->SetUpObj(daych);

    VisRadioType1* shd = new VisRadioType1(31, scr.left, scr.bottom + 68, scr.right, scr.bottom + 92, g_font1, p_clrsh_Black, nullptr);
    shd->AddEntry(txt_patch.GetLine(52));
    this->AddChild(shd);

    shd->ReadData(&g_Shadows);
    shd->SetUpObj(smooth);

    VisRadioType1* light = new VisRadioType1(32, scr.left, scr.bottom + 96, scr.right, scr.bottom + 120, g_font1, p_clrsh_Black, nullptr);
    light->AddEntry(txt_patch.GetLine(53));
    this->AddChild(light);

    light->ReadData(&g_Lightning);
    light->SetUpObj(shd);

    VisRadioType1* anims = new VisRadioType1(33, scr.left, scr.bottom + 124, scr.right, scr.bottom + 148, g_font1, p_clrsh_Black, nullptr);
    anims->AddEntry(txt_patch.GetLine(54));
    this->AddChild(anims);

    anims->ReadData(&g_Animation);
    anims->SetUpObj(light);

    int ypoint = caption->GetRect().top - 15;

    VisRadioType1* showhit = new VisRadioType1(4, scr.right + 48, ypoint, scr.right + 264, ypoint + 24, g_font1, p_clrsh_Black, nullptr);
    showhit->AddEntry(txt_dialogs.GetLine(57));
    this->AddChild(showhit);

    showhit->ReadData(g_settings.pShowAllHitPoints);
    showhit->SetLeftObj(scrl);

    VisRadioType1* showhp = new VisRadioType1(5, scr.right + 48, ypoint + 28, scr.right + 264, ypoint + 52, g_font1, p_clrsh_Black, nullptr);
    showhp->AddEntry(txt_dialogs.GetLine(78));
    this->AddChild(showhp);

    showhp->ReadData(g_settings.pShowFlyingHP);
    showhp->SetUpObj(showhit);

    VisRadioType1* clantips = new VisRadioType1(13, scr.right + 48, ypoint + 56, scr.right + 264, ypoint + 80, g_font1, p_clrsh_Black, nullptr);

    if (mwnd->sessionMode == SESSION_LOCAL_BATTLE) {
        clantips->AddEntry(txt_dialogs.GetLine(156)); //tips
    } else {
        clantips->AddEntry(txt_dialogs.GetLine(175)); //clans
    }

    this->AddChild(clantips);

    if (mwnd->sessionMode == SESSION_LOCAL_BATTLE) {
        clantips->ReadData(&g_settings.TipsMode);
    } else {
        clantips->ReadData(&g_settings.ClanNames);
    }
    clantips->SetUpObj(showhp);


    VisRadioType1* altclr = new VisRadioType1(41, scr.right + 48, ypoint + 84, scr.right + 264, ypoint + 108, g_font1, p_clrsh_Black, nullptr);

    altclr->AddEntry(txt_patch.GetLine(87)); //alt color
    this->AddChild(altclr);

    altclr->ReadData(&g_MessageColors);
    altclr->SetUpObj(clantips);

    
    this->AddChild(new VisLabel(15, 280, caption->GetRect().bottom + 74, 472, caption->GetRect().bottom + 98, txt_dialogs.GetLine(166), g_font1, p_clrsh_Black, 0));


    VisRadioType1* ochar = new VisRadioType1(131, scr.right + 48, scr.bottom + 68, scr.right + 264, scr.bottom + 92, g_font1, p_clrsh_Black, nullptr);

    ochar->AddEntry(txt_dialogs.GetLine(167)); //own chars
    this->AddChild(ochar);

    int32_t aflg = (g_settings.AutoCasting & 1) != 0;
    ochar->ReadData(&aflg);
    ochar->SetUpObj(altclr);


    VisRadioType1* alli = new VisRadioType1(132, scr.right + 48, scr.bottom + 96, scr.right + 264, scr.bottom + 120, g_font1, p_clrsh_Black, nullptr);

    alli->AddEntry(txt_dialogs.GetLine(168)); //allies
    this->AddChild(alli);

    aflg = (g_settings.AutoCasting & 2) != 0;
    alli->ReadData(&aflg);
    alli->SetUpObj(ochar);


    VisRadioType1* neutral = new VisRadioType1(133, scr.right + 48, scr.bottom + 124, scr.right + 264, scr.bottom + 148, g_font1, p_clrsh_Black, nullptr);

    neutral->AddEntry(txt_dialogs.GetLine(169)); //neutral
    this->AddChild(neutral);

    aflg = (g_settings.AutoCasting & 4) != 0;
    neutral->ReadData(&aflg);
    neutral->SetUpObj(alli);


    VisLabel* lbl = new VisLabel(6, 40, scr.bottom + 156, 208, scr.bottom + 180, txt_dialogs.GetLine(58), g_font1, p_clrsh_Black, 0);
    this->AddChild(lbl);

    CRect* lr = &lbl->GetRect();
    
    //formation
    VisRadioType2* formation = new VisRadioType2(7, 40, lr->bottom, 208, lr->bottom + 72, g_font1, p_clrsh_Black, txt_dialogs.GetLine(59));
    formation->AddEntry(txt_dialogs.GetLine(60)); //off
    formation->AddEntry(txt_dialogs.GetLine(61)); //auto
    formation->AddEntry(txt_dialogs.GetLine(62)); //on
    this->AddChild(formation);

    formation->ReadData(g_settings.pFormationMode);
    anims->SetDownObj(formation);


    lr = &lbl->GetRect();

    lbl = new VisLabel(8, lr->right, lr->top, lr->right + 168, lr->bottom, txt_dialogs.GetLine(63), g_font1, p_clrsh_Black, 0);
    this->AddChild(lbl);


    lr = &formation->GetRect();

    //autoretreat
    VisRadioType2* retreat = new VisRadioType2(9, lr->right, lr->top, lr->right + 168, lr->bottom, g_font1, p_clrsh_Black, txt_dialogs.GetLine(64));
    retreat->AddEntry(txt_dialogs.GetLine(65)); //off
    retreat->AddEntry(txt_dialogs.GetLine(66)); //norm
    retreat->AddEntry(txt_dialogs.GetLine(67)); //panic
    this->AddChild(retreat);

    retreat->ReadData(g_settings.pWimpyMode);

    formation->SetRightObj(retreat);
    retreat->SetUpObj(neutral);
    neutral->SetDownObj(retreat);

    lr = &lbl->GetRect();
    lbl = new VisLabel(151, lr->right, lr->top, lr->right + 168, lr->bottom, txt_dialogs.GetLine(170), g_font1, p_clrsh_Black, 0);
    this->AddChild(lbl);

    lr = &retreat->GetRect();
    VisRadioType2* acast = new VisRadioType2(141, lr->right, lr->top, lr->right + 168, lr->bottom, g_font1, p_clrsh_Black, txt_dialogs.GetLine(174));
    acast->AddEntry(txt_dialogs.GetLine(171)); //min
    acast->AddEntry(txt_dialogs.GetLine(172)); //mid
    acast->AddEntry(txt_dialogs.GetLine(173)); //max
    this->AddChild(acast);

    aflg = (g_settings.AutoCasting & 0x10) != 0;
    if ((g_settings.AutoCasting & 0x20) != 0) {
        aflg = 2;
    }

    acast->ReadData(&aflg);

    acast->SetLeftObj(retreat);
    acast->SetUpObj(neutral);
    neutral->SetDownObj(acast);

    CRect local_30(this->rect.Width() / 7, acast->GetRect().bottom + 8, (this->rect.Width() * 3) / 7, acast->GetRect().bottom + 32);
    CRect local_40((this->rect.Width() * 4) / 7, acast->GetRect().bottom + 8, (this->rect.Width() * 6) / 7, acast->GetRect().bottom + 32);

    VisButton* acpt = new VisButton(10, local_30, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "");
    this->AddChild(acpt);
    acpt->ChangeFlags(FLAG_10, true);

    VisButton* cncl = new VisButton(11, local_40, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    this->AddChild(cncl);

    acpt->SetRightObj(cncl);
}




EndGameMenu::EndGameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitm, uint32_t unk, const CRect& _r, int32_t unk2)
 : VisMenuWnd(_id, l, t, r, b, _bitm, unk, _r) { //441357
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    MenuButton* btn = nullptr;
    if (mwnd->sessionMode == SESSION_SINGLEPLAYER && mwnd->net_session.internet_game && mwnd->hat_settings.ishat && mwnd->hat_settings.deathmatch == 0)
        btn = new MenuButton(1, txt_patch.GetLine(208), g_font1, nullptr, 0x41d, 'R', "");
    else if (mwnd->sessionMode == SESSION_CLIENT || mwnd->sessionMode == SESSION_SERVER || mwnd->sessionMode == SESSION_SINGLEPLAYER) {
        btn = new MenuButton(1, txt_dialogs.GetLine(42), g_font1, nullptr, 0x41d, 'C', ""); //restart mission
    } else {
        btn = new MenuButton(1, txt_dialogs.GetLine(43), g_font1, nullptr, 0x41d, 'V', ""); //win
    }
    this->AddElement(btn, 30);

    if (unk2 == 0 && mwnd->sessionMode == SESSION_LOCAL_BATTLE) {
        btn->ChangeFlags(FLAG_ENABLED, false);
    }
    
    if (g_CLlDriver.GetProvider() == PROVIDER_HAT_TCP && mwnd->hat_settings.deathmatch != 0) {
        btn->ChangeFlags(FLAG_ENABLED, false);
    }

    btn = new MenuButton(2, txt_dialogs.GetLine(44), g_font1, nullptr, 0x41e, 'E', ""); //exit to main
    this->AddElement(btn, 30);

    btn = new MenuButton(3, txt_dialogs.GetLine(45), g_font1, nullptr, WM_CLOSE, 'W', ""); //exit to win
    this->AddElement(btn, 30);

    btn = new MenuButton(4, txt_dialogs.GetLine(40), g_font1, nullptr, 0x446, 'R', ""); //return to game
    this->AddElement(btn, 30);
}


ExitGameMenu::ExitGameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CRect& _r)
: VisMenuWnd(_id, l, t, r, b, nullptr, 0, _r) { //44186e
    MenuButton* btn = new MenuButton(2, txt_dialogs.GetLine(44), g_font1, nullptr, 0x41e, 'E', ""); //exit to main
    this->AddElement(btn, 30);

    btn = new MenuButton(3, txt_dialogs.GetLine(45), g_font1, nullptr, WM_CLOSE, 'W', ""); //exit to win
    this->AddElement(btn, 30);

    btn = new MenuButton(4, txt_dialogs.GetLine(40), g_font1, nullptr, 0x446, 'R', ""); //return to game
    this->AddElement(btn, 30);
}

SoundPreferencesDialogVisualObject::SoundPreferencesDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SoundSettings *pset)
: VisWindow(_id, l, t, r, b, _bitmap) { //43d917
    this->settings = pset;
}


int32_t SoundPreferencesDialogVisualObject::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) { //43e6f0
    int32_t result = 0;
    switch (msg) {
    case VIS_MSG_MUSIC_POSITION:
        result = 1;
        break;

    case VIS_MSG_SET_OPTION:
        switch (wparam) {
        case 2:
            this->settings->random = lparam;
            this->music_player->SetRandom(lparam);
            result = 1;
            break;

        case 3:
            this->track_index = lparam;
            result = 1;
            break;

        case 6: {
            int32_t vol = this->ConvSliderToVolume(lparam, this->settings->mus_pos_max);
            int32_t cur_vol = this->music_player->GetVolume();
            if (vol != cur_vol) {
                this->settings->mus_pos = vol;
                if (this->music_player->GetState() != 2) {
                    this->music_player->SetVolume(vol);
                }
            }
            result = 1;
        }   break;


        case 7:
            this->settings->sfx_pos = this->ConvSliderToVolume(lparam, this->settings->sfx_pos_max);
            PostMessage(g_MainWndHWND, 0x485, 0, 0);
            result = 1;
            break;

        case 8:
            this->settings->speech_pos = this->ConvSliderToVolume(lparam, this->settings->speech_pos_max);
            result = 1;
            break;
        default:
            result = 1;
            break;
        }
        break;

    case VIS_MSG_SFX_TEST:
        if (wparam == 7) {
            g_SfxArray[100]->Play(this->settings->sfx_pos, 0, 0, 220, 0);
        } else if (wparam == 8) {
            SoundBank_fighter[0].select[1]->Play(this->settings->sfx_pos, 0, 0, 220, 0);
        }
        result = 1;
        break;

    case VIS_MSG_ACKNOWLEDGE:
        this->music_player->SetPlayNotify(0);
        this->FindChild(40)->WriteData(&g_settings.Acknowledgement);
        VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        result = 1;
        break;

    case VIS_MSG_MUSIC_ENABLE:
        this->settings->music_enabled = 1;
        this->music_player->SetFadeout(0);
        if (this->track_index == this->music_player->GetCurrentTrackIndex()) {
            this->music_player->SetVolume(this->settings->mus_pos);
            this->music_player->Play();
        } else {
            this->music_player->OnEndTrack();
            this->music_player->StartPlayTrack(this->track_index);
            this->music_player->SetVolume(this->settings->mus_pos);
            this->music_player->Play();
        }
        result = 1;
        break;

    case VIS_MSG_MUSIC_DISABLE:
        if (this->music_player->GetState() == 2) {
            this->music_player->OnEndTrack();
        }
        if (this->music_player->GetState() == 1) {
            this->music_player->BeginFadeOut(2000, 8000);
        }

        this->settings->music_enabled = 0;
        result = 1;
        break;

    default:
        result = VisScreen::MsgProc(msg, wparam, lparam);
        break;
    }
    return result;
}

void SoundPreferencesDialogVisualObject::VMethod26() { //43d959
    MainWindow *mwnd = (MainWindow*)AfxGetMainWnd();

    this->music_player = mwnd->music_player;
    this->track_index = 0;

    this->AddChild(new VisLabel(555, 40, 20, this->rect.Width() - 40, 45, txt_dialogs.GetLine(7), g_font1, p_clrsh_Black, 2)); //Sound options

    VisLabel* lbl_melodies = new VisLabel(557, 40, 60, this->rect.Width() - 40, 78, txt_dialogs.GetLine(143), g_font1, p_clrsh_Black, 0); //melodies
    this->AddChild(lbl_melodies);

    VisButton* btn_acpt = new VisButton(1, 40, 290, 252, 314, txt_dialogs.GetLine(0), g_font1, nullptr, 0x476, 0, txt_dialogs.GetLine(8)); //Accept
    btn_acpt->ChangeFlags(FLAG_10, true);
    this->AddChild(btn_acpt);

    VisRadioType1* btn_raport = new VisRadioType1(40, 40, 190, 252, 214, g_font1, p_clrsh_Black, txt_dialogs.GetLine(165)); //Raports
    btn_raport->AddEntry(txt_dialogs.GetLine(165));
    btn_raport->ReadData(&g_settings.Acknowledgement);
    this->AddChild(btn_raport);

    if (mwnd->sessionMode == SESSION_SINGLEPLAYER || mwnd->sessionMode == SESSION_CLIENT) {
        btn_raport->ChangeFlags(FLAG_ENABLED, false);
    }


    VisListBox* lst_melody = new VisListBox(3, 40, 80, this->rect.Width() - 64, 170, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 10, txt_dialogs.GetLine(11)); //Select melody
    lst_melody->SetSelectedIndex(this->track_index);

    for (int i = 0; i < this->music_player->GetPlaylistSize(); i++) {
        CString trk = this->music_player->GetPlaylistEntry(i);
        trk.MakeLower();
        trk = trk.Mid(6); // music/

        CString name;
        g_TunesMap.Lookup(trk, name);
        lst_melody->AddItem(name);
    }
    this->AddChild(lst_melody);

    lst_melody->SetCaptionLabel(lbl_melodies);


    CRect r = lst_melody->GetRect();
    this->AddChild(new VisScrollBar(10, r.right, r.top, r.right + 24, r.bottom, nullptr));

    VisButton* btn_play = new VisButton(4, 40, 256, 140, 280, txt_dialogs.GetLine(12), g_font1, nullptr, 0x477, 0, txt_dialogs.GetLine(13)); //Play
    if (this->settings->music_allowed == 0) {
        btn_play->ChangeFlags(FLAG_ENABLED, false);
        btn_play->SetHint(txt_dialogs.GetLine(23)); //Music disabled
    }
    if (this->music_player->GetPlaylistSize() == 0) {
        btn_play->ChangeFlags(FLAG_ENABLED, 0);
        btn_play->SetHint(txt_dialogs.GetLine(75)); //Music opts now unav
    }
    this->AddChild(btn_play);


    VisButton* btn_stop = new VisButton(5, 150, 256, 252, 280, txt_dialogs.GetLine(14), g_font1, nullptr, 0x478, 0, txt_dialogs.GetLine(15)); //stop
    if (this->settings->music_allowed == 0) {
        btn_stop->ChangeFlags(FLAG_ENABLED, 0);
        btn_stop->SetHint(txt_dialogs.GetLine(23)); //Music unav
    }
    if (this->music_player->GetPlaylistSize() == 0) {
        btn_stop->ChangeFlags(FLAG_ENABLED, 0);
        btn_stop->SetHint(txt_dialogs.GetLine(75)); //Music opts now unav
    }
    this->AddChild(btn_stop);


    btn_stop->SetUpObj(btn_raport);
    btn_play->SetUpObj(btn_raport);

    btn_stop->SetDownObj(btn_acpt);
    btn_play->SetDownObj(btn_acpt);

    btn_play->SetRightObj(btn_stop);

    VisLabel* lbl_mvol = new VisLabel(26, 258, 175, this->rect.Width() - 40, 190, txt_dialogs.GetLine(16), g_font1, p_clrsh_Black, 2);
    this->AddChild(lbl_mvol);

    VisScrollBar* scrl_mvol = new VisScrollBar(6, 258, 190, this->rect.Width() - 40, 214, txt_dialogs.GetLine(19));
    this->AddChild(scrl_mvol);

    scrl_mvol->SetCaptionLabel(lbl_mvol);

    VisScrollBar::Data sd;
    sd.v = this->ConvVolumeToSlider(this->settings->mus_pos, this->settings->mus_pos_max);
    sd.vmax = this->settings->mus_pos_max;

    scrl_mvol->ReadData(&sd);

    if (this->settings->music_allowed == 0) {
        scrl_mvol->ChangeFlags(FLAG_ENABLED, false);
        scrl_mvol->SetHint(txt_dialogs.GetLine(23)); //Music unav
    }
    if (this->music_player->GetPlaylistSize() == 0) {
        scrl_mvol->ChangeFlags(FLAG_ENABLED, 0);
        scrl_mvol->SetHint(txt_dialogs.GetLine(75)); //Music opts now unav
    }


    VisLabel* lbl_efvol = new VisLabel(27, 258, 224, this->rect.Width() - 40, 239, txt_dialogs.GetLine(17), g_font1, p_clrsh_Black, 2); //effects vol
    this->AddChild(lbl_efvol);

    VisScrollBar* scrl_efvol = new VisScrollBar(7, 258, 240, this->rect.Width() - 40, 264, txt_dialogs.GetLine(20));
    this->AddChild(scrl_efvol);

    sd.v = this->ConvVolumeToSlider(this->settings->sfx_pos, this->settings->sfx_pos_max);
    sd.vmax = this->settings->sfx_pos_max;

    scrl_efvol->ReadData(&sd);
    scrl_efvol->SetCaptionLabel(lbl_efvol);


    VisLabel* lbl_svol = new VisLabel(28, 258, 275, this->rect.Width() - 40, 290, txt_dialogs.GetLine(18), g_font1, p_clrsh_Black, 2); //speech vol
    this->AddChild(lbl_svol);

    VisScrollBar* scrl_svol = new VisScrollBar(8, 258, 290, this->rect.Width() - 40, 314, txt_dialogs.GetLine(21));
    this->AddChild(scrl_svol);

    sd.v = this->ConvVolumeToSlider(this->settings->speech_pos, this->settings->speech_pos_max);
    sd.vmax = this->settings->sfx_pos_max;

    scrl_svol->ReadData(&sd);
    scrl_svol->SetCaptionLabel(lbl_svol);


    scrl_svol->SetLeftObj(btn_acpt);

    scrl_mvol->SetDownObj(scrl_efvol);
    scrl_efvol->SetDownObj(scrl_svol);

    this->music_player->SetPlayNotify(1);
}

VisLogoWnd::VisLogoWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) { //4cd4d0
    VisLogoWnd::VMethod26();
}

VisLogoWnd::~VisLogoWnd() { //4cd53c
    if (this->logoBitmap) {
        delete this->logoBitmap;
    }
}

void VisLogoWnd::VMethod7() { //4cd8d1
    this->drawPendingFlag = 0;

    LockSurface2();

    if (this->logoBitmap) {
        this->logoBitmap->Draw(this->rect.left, this->rect.top, 0, 0, 0);
    }

    UnlockSurface2();
}

int32_t VisLogoWnd::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) { //4cd72e

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

    switch (msg) {
    case VIS_MSG_FRAME_TICK:
        if (this->drawPendingFlag) {
            this->VMethod9();
        }

        if (this->timeoutTicks < timeGetTime() - this->timeoutStart) {
            AfxGetMainWnd()->PostMessage(MAIN_MSG_ADVANCE_LOGO, this->step, 0);
        }
        break;

    case VIS_MSG_DATA_REFRESH:
        this->drawPendingFlag = 1;
        break;

    case VIS_MSG_LOGO_SET_TIMEOUT:
        this->timeoutTicks = wparam;
        this->timeoutStart = timeGetTime();
        break;

    case VIS_MSG_LOGO_SET_STEP:
        this->step = wparam;
        this->logoBitmap = new CBmp256(logos_files[wparam]);
        this->logoBitmap->ResetPalette(1, 1, 0);
        break;
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
    return 1;
}

int32_t VisLogoWnd::OnLButtonDown(uint32_t wparam, CPoint pos) { //4cd6c2
    AfxGetMainWnd()->PostMessage(MAIN_MSG_ADVANCE_LOGO, this->step, 0);
    this->timeoutTicks = 0x7fffffff;
    return 1;
}

int32_t VisLogoWnd::OnKeyDown(uint32_t wparam) { //4cd6f8
    AfxGetMainWnd()->PostMessage(MAIN_MSG_ADVANCE_LOGO, this->step, 0);
    this->timeoutTicks = 0x7fffffff;
    return 1;
}

void VisLogoWnd::VMethod26() { //4cd5be
    this->timeoutTicks = 0x7fffffff;
    this->logoBitmap = nullptr;
    this->AddChild(new VisButton(4, 0, 0, 0, 0, "", g_font1, clrsh_TechBlack, 0x445, 0, nullptr)); // WTF?
}

void VisLogoWnd::DoClose(uint32_t code) { //4cd666 29 method
    if (this->logoBitmap) {
        delete this->logoBitmap;
        this->logoBitmap = nullptr;
    }
    VisScreen::DoClose(code);
}

// 435D72
void VisStartGame::VMethod7() {
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

    this->mainAreaBitmap->Draw(this->rect.left, this->rect.top, 0, 0, 0);

    if (this->returnToGameHoverBitmap != nullptr) {
        this->returnToGameHoverBitmap->DrawSubRectMasked(
            screen_x + this->returnToGameButtonRect.left,
            screen_y + this->returnToGameButtonRect.top,
            0, 0,
            this->returnToGameButtonRect.Width(),
            this->returnToGameButtonRect.Height());
    }
    if (this->acceptHoverBitmap != nullptr) {
        this->acceptHoverBitmap->DrawSubRectMasked(
            screen_x + this->acceptButtonRect.left,
            screen_y + this->acceptButtonRect.top,
            0, 0,
            this->acceptButtonRect.Width(),
            this->acceptButtonRect.Height());
    }
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        for (int32_t i = 0; i < 3; i++) {
            if (this->difficultyStateFlags.ElementAt(i) == 2) {
                const CRect& r = this->difficultyRects.ElementAt(i);
                this->difficultyHoverBitmaps.GetAt(i)->DrawSubRectMasked(
                    screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
            }
            if (this->difficultyStateFlags.ElementAt(i) == 1) {
                const CRect& r = this->difficultyRects.ElementAt(i);
                this->difficultySelectedBitmaps.GetAt(i)->DrawSubRectMasked(
                    screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
            }
            if (this->difficultyStateFlags.ElementAt(i) == 3) {
                const CRect& r = this->difficultyRects.ElementAt(i);
                this->difficultySelectedHoverBitmaps.GetAt(i)->DrawSubRectMasked(
                    screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
            }
        }
    }
    if (this->portraitStateFlags.ElementAt(0) & 1) {
        if ((this->portraitStateFlags.ElementAt(1) & 2) == 0) {
            this->portraitHoverBitmaps.GetAt(0)->DrawSubRectMasked(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        } else {
            this->portraitSelectedHoverBitmaps.GetAt(0)->DrawSubRectMasked(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        }
        if (this->portraitStateFlags.ElementAt(2) & 2) {
            const CRect& r = this->portraitRects.ElementAt(2);
            this->portraitSelectedBitmaps.GetAt(2)->DrawSubRectMasked(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
        if (this->portraitStateFlags.ElementAt(3) & 2) {
            const CRect& r = this->portraitRects.ElementAt(3);
            this->portraitSelectedBitmaps.GetAt(3)->DrawSubRectMasked(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
    }
    if (this->portraitStateFlags.ElementAt(1) & 1) {
        if ((this->portraitStateFlags.ElementAt(0) & 2) == 0) {
            this->portraitHoverBitmaps.GetAt(1)->DrawSubRectMasked(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        } else {
            this->portraitSelectedHoverBitmaps.GetAt(1)->DrawSubRectMasked(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        }
        if (this->portraitStateFlags.ElementAt(2) & 2) {
            const CRect& r = this->portraitRects.ElementAt(2);
            this->portraitSelectedBitmaps.GetAt(2)->DrawSubRectMasked(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
        if (this->portraitStateFlags.ElementAt(3) & 2) {
            const CRect& r = this->portraitRects.ElementAt(3);
            this->portraitSelectedBitmaps.GetAt(3)->DrawSubRectMasked(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
    }
    if (this->portraitStateFlags.ElementAt(2) & 1) {
        if ((this->portraitStateFlags.ElementAt(3) & 2) == 0) {
            this->portraitHoverBitmaps.GetAt(2)->DrawSubRectMasked(screen_x + 0x104, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        } else {
            this->portraitSelectedHoverBitmaps.GetAt(2)->DrawSubRectMasked(screen_x + 0x104, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        }
        if (this->portraitStateFlags.ElementAt(0) & 2) {
            const CRect& r = this->portraitRects.ElementAt(0);
            this->portraitSelectedBitmaps.GetAt(0)->DrawSubRectMasked(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
        if (this->portraitStateFlags.ElementAt(1) & 2) {
            const CRect& r = this->portraitRects.ElementAt(1);
            this->portraitSelectedBitmaps.GetAt(1)->DrawSubRectMasked(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
    }
    if (this->portraitStateFlags.ElementAt(3) & 1) {
        if ((this->portraitStateFlags.ElementAt(2) & 2) == 0) {
            this->portraitHoverBitmaps.GetAt(3)->DrawSubRectMasked(screen_x + 0x104, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        } else {
            this->portraitSelectedHoverBitmaps.GetAt(3)->DrawSubRectMasked(screen_x + 0x104, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        }
        if (this->portraitStateFlags.ElementAt(0) & 2) {
            const CRect& r = this->portraitRects.ElementAt(0);
            this->portraitSelectedBitmaps.GetAt(0)->DrawSubRectMasked(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
        if (this->portraitStateFlags.ElementAt(1) & 2) {
            const CRect& r = this->portraitRects.ElementAt(1);
            this->portraitSelectedBitmaps.GetAt(1)->DrawSubRectMasked(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
    }
    if (this->returnToGameHoverBitmap != nullptr) {
        this->returnToGameHoverBitmap->DrawSubRectMasked(
            screen_x + this->returnToGameButtonRect.left,
            screen_y + this->returnToGameButtonRect.top,
            0, 0,
            this->returnToGameButtonRect.Width(),
            this->returnToGameButtonRect.Height());
    }
    if (this->acceptHoverBitmap != nullptr) {
        this->acceptHoverBitmap->DrawSubRectMasked(
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
        this->blindAnimation->Draw(
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
    this->leftTorchFrames.GetAt(this->torchFrameTick % 0xF)->Draw(screen_x + 4, screen_y + 200, 0, 0, 0);
    this->rightTorchFrames.GetAt((this->torchFrameTick + 8) % 0xF)->Draw(screen_x + 0x24C, screen_y + 200, 0, 0, 0);

    UnlockSurface2();
    SetClipRect(old_clip);
    VisScreen::VMethod7();
}


// 4333B9
void VisStartGame::VMethod26() {
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
    this->gap_0x1bc = 0;
    this->gap_0x1c0 = 0;
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
int32_t VisStartGame::OnLButtonDown(uint32_t wparam, CPoint pos) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    VisStartGameTextBox* label;
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        label = this->networkNameLabel;
    } else {
        label = this->nameLabel;
    }
    uint32_t hotspot = this->UpdateHotspots(pos.x, pos.y, 1);
    this->UpdateTipsProgress(hotspot);
    switch (hotspot) {
    case 0x14:
        if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
            this->committedDifficultyIndex = 0;
            StopSfx(&this->difficultyLevel1Sound.sample);
            CSound::Play(this->difficultyLevel1Sound);
        }
        break;
    case 0x28:
        if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
            this->committedDifficultyIndex = 1;
            StopSfx(&this->difficultyLevel2Sound.sample);
            CSound::Play(this->difficultyLevel2Sound);
        }
        break;
    case 0x3C:
        if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
            this->committedDifficultyIndex = 2;
            StopSfx(&this->difficultyLevel3Sound.sample);
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
        StopSfx(&this->portraitSelectSound.sample);
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
        StopSfx(&this->portraitSelectSound.sample);
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
        StopSfx(&this->portraitSelectSound.sample);
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
        StopSfx(&this->portraitSelectSound.sample);
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

// 436E4B
uint32_t VisStartGame::DrawTipsHighlight(uint32_t hotspot) {
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
            this->portraitSelectedHoverBitmaps.GetAt(0)->DrawSubRectMasked(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        } else {
            this->portraitHoverBitmaps.GetAt(0)->DrawSubRectMasked(screen_x + 0x70, screen_y + 0x2C, 0, 0, 0x10C, 0x154);
        }
        if (startgame_tips_frame == 2) {
            const CRect& r = this->portraitRects.ElementAt(2);
            this->portraitSelectedBitmaps.GetAt(2)->DrawSubRectMasked(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        }
        if (startgame_tips_frame == 3) {
            const CRect& r = this->portraitRects.ElementAt(3);
            this->portraitSelectedBitmaps.GetAt(3)->DrawSubRectMasked(
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
            this->difficultySelectedHoverBitmaps.GetAt(startgame_tips_frame)->DrawSubRectMasked(
                screen_x + r.left, screen_y + r.top, 0, 0, r.Width(), r.Height());
        } else {
            const CRect& r = this->difficultyRects.ElementAt(startgame_tips_frame);
            this->difficultyHoverBitmaps.GetAt(startgame_tips_frame)->DrawSubRectMasked(
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
            this->returnToGameButtonBitmap->DrawSubRectMasked(
                screen_x + this->returnToGameButtonRect.left,
                screen_y + this->returnToGameButtonRect.top,
                0, 0,
                this->returnToGameButtonRect.Width(),
                this->returnToGameButtonRect.Height());
        } else if (startgame_tips_frame == 1) {
            this->acceptButtonBitmap->DrawSubRectMasked(
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
uint32_t VisStartGame::UpdateHotspots(int32_t x, int32_t y, uint32_t pressed) {
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
void VisStartGame::VMethod28() {
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

    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
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
void VisStartGame::DoClose(uint32_t code) {
    this->VMethod9();
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->dialogActiveFlag = 0;
    this->FreeBitmaps();
    this->FreeSamples();
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
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
const char* VisStartGame::GetHint() {
    if (this->dialogActiveFlag == 0) {
        return nullptr;
    }

    uint32_t hotspot = this->GetHotspotId(g_mousept.GetX(), g_mousept.GetY());
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
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
void VisStartGame::UpdateTipsProgress(uint32_t hotspot) {
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
// client: 43a2af
void VisStartGame::Accept() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->sessionMode == SESSION_LOCAL_BATTLE) {
        if (this->networkNameLabel->GetText().GetLength() > 0) {
            CSound::Play(this->acceptSound);
            this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        }
    } else {
#ifdef A2CLIENT
        int32_t min_len = 1;
        if (main_wnd->net_session.internet_game != 0 && main_wnd->hat_settings.ishat != 0) {
            min_len = 3;
        }
        if (this->nameLabel->GetText().GetLength() >= min_len) {
            if (this->nameLabel->GetText()[0] != ' ') {
                if (main_wnd->hat_settings.ishat != 0 && main_wnd->net_session.internet_game != 0) {
                    main_wnd->SendHatJoinName(this->nameLabel->GetText());
                }
                CSound::Play(this->acceptSound);
                this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
            } else {
                VisScreen* mbox = new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(0x1b), nullptr, 0x2000);
                main_wnd->vis_msgbox_list = mbox;
                main_wnd->ModalScreen(mbox);
            }
        } else if (min_len > 1) {
            VisScreen* mbox = new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(0x1c), nullptr, 0x2000);
            main_wnd->vis_msgbox_list = mbox;
            main_wnd->ModalScreen(mbox);
        }
#else
        if (this->nameLabel->GetText().GetLength() > 0) {
            CSound::Play(this->acceptSound);
            this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        }
#endif
    }
}


// 437571
int32_t VisStartGame::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_FRAME_TICK) {
        this->VMethod9();
    } else if (msg == VIS_MSG_CLOSE_TIPS && this->tipsPrompt != nullptr) {
        this->RemoveChild(this->tipsPrompt);
        delete this->tipsPrompt;
        this->tipsPrompt = nullptr;
    }
    return VisScreen::MsgProc(msg, wparam, lparam);
}


// 4356C2
uint32_t VisStartGame::GetHotspotId(int32_t x, int32_t y) {
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
int32_t VisStartGame::OnKeyDown(uint32_t wparam) {
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
int32_t VisStartGame::OnMouseMove(uint32_t wparam, CPoint pos) {
    this->UpdateHotspots(pos.x, pos.y, wparam & 1);
    return CVisualObject::OnMouseMove(wparam, pos);
}


// 4382A9
void VisStartGame::Cancel() {
    CSound::Play(this->returnSound);
    this->MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
}

// 4382D9
void VisStartGame::PlayLabelInputSound() {
    CSound::Play((&this->labelInputSound1)[this->labelInputSoundIndex]);
}

// 438DC0
void VisStartGame::VMethod8(CRect* rect) {
}


// 4386B0
VisStartGame::~VisStartGame() {
    this->FreeBitmaps();
    if (this->tipsPrompt != nullptr) {
        this->RemoveChild(this->tipsPrompt);
        delete this->tipsPrompt;
        this->tipsPrompt = nullptr;
    }
}


// 43305B
VisStartGame::VisStartGame(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : VisScreen(_id, l, t, r, b, nullptr) {
    this->VMethod26();
}


// 435D26
void VisStartGame::ResetHoverState() {
    this->gap_0x204 = 0;
    this->returnToGameHoverBitmap = nullptr;
    this->acceptHoverBitmap = nullptr;
    this->gap_0x1bc = 0;
    this->gap_0x1c0 = 0;
}


// 435743
void VisStartGame::ClearDifficultySelectedFlags() {
    for (int32_t i = 0; i < 3; i++) {
        this->difficultyStateFlags.ElementAt(i) &= ~1u;
    }
}


// 43578C
void VisStartGame::ClearPortraitSelectedFlags() {
    for (int32_t i = 0; i < 4; i++) {
        this->portraitStateFlags.ElementAt(i) &= ~1u;
    }
}


// 4357D5
void VisStartGame::ClearDifficultyHoverFlags() {
    for (int32_t i = 0; i < 3; i++) {
        this->difficultyStateFlags.ElementAt(i) &= ~2u;
    }
}


// 43581E
void VisStartGame::ClearPortraitHoverFlags() {
    for (int32_t i = 0; i < 4; i++) {
        this->portraitStateFlags.ElementAt(i) &= ~2u;
    }
}


// 4380D3
void VisStartGame::FreeSamples() {
    FreeSfxSample(&this->difficultyLevel1Sound.sample);
    FreeSfxSample(&this->difficultyLevel2Sound.sample);
    FreeSfxSample(&this->difficultyLevel3Sound.sample);
    FreeSfxSample(&this->portraitSelectSound.sample);
    FreeSfxSample(&this->acceptSound.sample);
    FreeSfxSample(&this->labelInputSound1.sample);
    FreeSfxSample(&this->labelInputSound2.sample);
    FreeSfxSample(&this->labelInputSound3.sample);
    FreeSfxSample(&this->returnSound.sample);
}


// 437FF4
void VisStartGame::LoadSamples() {
    this->FreeSamples();
    ReloadSfx(&this->difficultyLevel1Sound.sample, "SFX\\ChrGen\\Level1.wav");
    ReloadSfx(&this->difficultyLevel2Sound.sample, "SFX\\ChrGen\\Level2.wav");
    ReloadSfx(&this->difficultyLevel3Sound.sample, "SFX\\ChrGen\\Level3.wav");
    ReloadSfx(&this->portraitSelectSound.sample, "SFX\\ChrGen\\Char.wav");
    ReloadSfx(&this->acceptSound.sample, "SFX\\ChrGen\\Ok.wav");
    ReloadSfx(&this->labelInputSound1.sample, "SFX\\Letter1.wav");
    ReloadSfx(&this->labelInputSound2.sample, "SFX\\Letter2.wav");
    ReloadSfx(&this->labelInputSound3.sample, "SFX\\Letter3.wav");
    ReloadSfx(&this->returnSound.sample, "SFX\\ChrGen\\Ok.wav");
}


// 435097
void VisStartGame::FreeBitmaps() {
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
    this->gap_0x1bc = 0;
    this->gap_0x1c0 = 0;

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
void VisStartGame::LoadBitmaps() {
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

// 4AB41E
int32_t VisMainMenu::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_FRAME_TICK) {
        this->VMethod9();
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 4AB411
void VisMainMenu::VMethod8(CRect* rect) {
    (void)rect;
}


// 4AB28F
void VisMainMenu::VMethod7() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CRect screen_rect;
    this->ClientRectToScreen(&screen_rect, this->rect);
    if (main_wnd->dialogsMask != DIALOG_MASK_MAIN_MENU) {
        return;
    }
    LockSurface2();
    this->bmp_menu->Draw(screen_rect.left, screen_rect.top, 0, 0, 0);
    if (this->bmp_active_button != nullptr) {
        this->bmp_active_button->Draw(this->rect_active_button.left, this->rect_active_button.top, 0, 0, 0);
        if (this->active_button >= 0) {
            this->bmp_labels.GetAt(this->active_button)->Draw(screen_rect.left + 0xE8, screen_rect.top + 200, 0, 0, 0);
        }
    } else {
        if (this->over_button >= 0) {
            this->bmp_labels.GetAt(this->over_button)->Draw(screen_rect.left + 0xE8, screen_rect.top + 200, 0, 0, 0);
        }
    }
    this->sprite->Draw(screen_rect.left + 0x1E0, screen_rect.top + 0x168, 0, 0, 0);
    UnlockSurface2();
}


// 4AB88C
int32_t VisMainMenu::OnMouseMove(uint32_t wparam, CPoint pos) {
    this->UpdateButtonState(wparam, pos);
    return 0;
}


// 4AB8AF
int32_t VisMainMenu::OnLButtonDown(uint32_t wparam, CPoint pos) {
    this->UpdateButtonState(wparam, pos);
    if (this->pressed_button != -1) {
        CSound::Play((CSound&)this->snd_btn_click);
    }
    return 1;
}


// 4AB8F3
int32_t VisMainMenu::OnLButtonUp(uint32_t wparam, CPoint pos) {
    uint32_t msg = 0;
    switch (this->active_button) {
    case 0:
        if ((((uint8_t)this->disable_mask) & 1) == 0) {
            msg = MAIN_MSG_NEW_CAMPAIGN;
        }
        break;
    case 1:
        if ((((uint8_t)this->disable_mask) & 2) == 0) {
            msg = MAIN_MSG_SHOW_CHAR_LOADER;
        }
        break;
    case 2:
        if ((((uint8_t)this->disable_mask) & 4) == 0) {
            msg = 0x43B;
        }
        break;
    case 3:
        if ((((uint8_t)this->disable_mask) & 8) == 0) {
            msg = MAIN_MSG_SHOW_CREDITS;
        }
        break;
    case 4:
        if ((((uint8_t)this->disable_mask) & 0x10) == 0) {
            msg = MAIN_MSG_SHOW_LOAD_GAME;
        }
        break;
    case 5:
        if ((((uint8_t)this->disable_mask) & 0x20) == 0) {
            msg = MAIN_MSG_SHOW_HAT_SERVER_LIST;
        }
        break;
    case 6:
        if ((((uint8_t)this->disable_mask) & 0x40) == 0) {
            msg = MAIN_MSG_SHOW_FAME_HALL;
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

// 4AAB71
void VisMainMenu::VMethod26() {
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
        r.SetRect(top_left.x + s_MainMenuButtonRects[i][0], top_left.y + s_MainMenuButtonRects[i][1],
            top_left.x + s_MainMenuButtonRects[i][0] + s_MainMenuButtonRects[i][2],
            top_left.y + s_MainMenuButtonRects[i][1] + s_MainMenuButtonRects[i][3]);
        this->rect_buttons.Add(r);
        r.SetRect(top_left.x + s_MainMenuLabelRects[i][0], top_left.y + s_MainMenuLabelRects[i][1],
            top_left.x + s_MainMenuLabelRects[i][0] + s_MainMenuLabelRects[i][2],
            top_left.y + s_MainMenuLabelRects[i][1] + s_MainMenuLabelRects[i][3]);
        this->rect_labels.Add(r);
    }
    this->disable_mask = 0;
    VisButton* btn = new VisButton(4, 0, 0, 0, 0, "", g_font1, clrsh_TechBlack, 0x7fff, 0, nullptr);
    this->AddChild(btn);
}


// 4ABA8D
void VisMainMenu::VMethod28() {
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
void VisMainMenu::DoClose(uint32_t code) {
    this->FreeGraphics();
    this->FreeSfx();
    this->VisScreen::DoClose(code);
    AfxGetMainWnd()->SendMessage(MAIN_MSG_CHILD_CLOSED, (WPARAM)this, 0);
    g_mousept.EnableHint();
}


// 4AB45D
void VisMainMenu::UpdateButtonState(uint32_t wparam, CPoint mouse) {
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
void VisMainMenu::LoadSfx() {
    this->FreeSfx();
    ReloadSfx(&this->snd_btn_click, "SFX\\ChrGen\\Ok.wav");
}


// 4ABB87
void VisMainMenu::FreeSfx() {
    FreeSfxSample(&this->snd_btn_click);
}


// 4AADA7
void VisMainMenu::LoadGraphics() {
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
void VisMainMenu::FreeGraphics() {
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
: VisScreen(_id, l, t, r, b, nullptr) {
    this->VMethod26();
}


// 4AAAB6 (deleting dtor thunk ??_G at 4ABBB0 is compiler-generated)
VisMainMenu::~VisMainMenu() {
    this->FreeGraphics();
    this->FreeSfx();
}

// 44f0e8
void Vis2Action::VMethod26() {
    VisLabel* label = new VisLabel(1, 0x28, 0x20, 0xE8, 0x50, this->txt, g_font1, p_clrsh_Black, 2);
    this->AddChild(label);

    CRect ok_rect(
        this->rect.Width() / 8,
        this->rect.Height() - 0x60,
        this->rect.Width() * 7 / 8,
        this->rect.Height() - 0x48);

    CRect cancel_rect(
        this->rect.Width() / 8,
        this->rect.Height() - 0x48,
        this->rect.Width() * 7 / 8,
        this->rect.Height() - 0x30);

    MenuButton* ok_btn = new MenuButton(2, ok_rect, txt_dialogs.GetLine(0x2B), g_font1, nullptr, 0x41D, 0x56, g_EmptyStrConfirmHint);
    this->AddElement(ok_btn, ok_rect.Height());

    MenuButton* cancel_btn = new MenuButton(3, cancel_rect, txt_dialogs.GetLine(0x9A), g_font1, nullptr, 0x446, 0x43, g_EmptyStrCancelHint);
    this->AddElement(cancel_btn, cancel_rect.Height());
}

// 44f30d
int32_t Vis2Action::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    return VisMenuWnd::MsgProc(msg, wparam, lparam);
}

// 44f0a2
Vis2Action::Vis2Action(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _txt, const CRect& _r)
: VisMenuWnd(_id, l, t, r, b, nullptr, 0, _r) {
    this->txt = _txt;
}

// 450bd0
Vis2Action::~Vis2Action() {
}

// 43C60C
void VisCredits::VMethod7() {
    if (this->flag == 0) {
        return;
    }

    static uint8_t credits_timing_init = 0;  // 65995C in asm
    static uint32_t credits_scroll_time = 0; // 659958 in asm
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
                bmp->Draw(left + 0x140 - width / 2, top + this->scroll + i * line_height, 0, 0, 0);
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
        this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
    }

    credits_scroll_time = timeGetTime();
}

// 43C438
void VisCredits::VMethod26() {
    this->bitmaps.RemoveAll();
    this->AddChild(new VisButton(4, 0, 0, 0, 0, " ", g_font1, clrsh_TechBlack, 0x445, 0, nullptr));
}

// 43C595
int32_t VisCredits::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_FRAME_TICK) {
        this->VMethod9();
    }
    return VisScreen::MsgProc(msg, wparam, lparam);
}

// 43C4D6
void VisCredits::VMethod28() {
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
void VisCredits::LoadContent() {
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
void VisCredits::FreeContent() {
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
void VisCredits::DoClose(uint32_t code) {
    this->FreeContent();
    g_mousept.Paint();
    this->flag = 0;
    VisScreen::DoClose(code);
    g_mousept.EnableHint();
}

// 43C5D4
int32_t VisCredits::OnKeyDown(uint32_t wparam) {
    this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
    return VisScreen::OnKeyDown(wparam);
}

// 43C5F5
int32_t VisCredits::OnLButtonDown(uint32_t wparam, CPoint pos) {
    this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
    return 0;
}

// 43C9A8
void VisCredits::VMethod8(CRect* rect) {
}

// 43C337
VisCredits::VisCredits(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : VisScreen(_id, l, t, r, b, nullptr) {
    this->VMethod26();
}

// 43C3C6
VisCredits::~VisCredits() {
    this->FreeContent();
}

// 441C7F
void VisDropGold::VMethod26() {
    VisTextBox* textbox = new VisTextBox(10000005, 30, 65, 266, 85, g_font1, clrsh_TechBlack, nullptr);
    this->AddChild(textbox);

    VisButton* btn_ok = new VisButton(10000001, 68, this->rect.Height() - 50, 138, this->rect.Height() - 30, txt_dialogs.GetLine(0), g_font1, clrsh_TechBlack, 0x445, 0, txt_dialogs.GetLine(0x2e));
    this->AddChild(btn_ok);

    VisButton* btn_cancel = new VisButton(10000002, 158, this->rect.Height() - 50, 228, this->rect.Height() - 30, txt_dialogs.GetLine(1), g_font1, clrsh_TechBlack, 0x446, 0, txt_dialogs.GetLine(0x2f));
    this->AddChild(btn_cancel);

    VisLabel* lbl_title = new VisLabel(10000003, 20, 20, 276, 40, txt_dialogs.GetLine(0x30), g_font1, clrsh_TechBlack, 2);
    this->AddChild(lbl_title);

    VisLabel* lbl_subtitle = new VisLabel(10000004, 20, 40, 276, 60, txt_dialogs.GetLine(0x31), g_font1, clrsh_TechBlack, 2);
    this->AddChild(lbl_subtitle);
}

// 441FEE
void VisDropGold::DoClose(uint32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (code == VIS_MSG_HIDE_SCREEN) {
        CVisualObject* textbox = this->FindChild(10000005);
        textbox->WriteData(this->amount_text);

        float amount;
        sscanf(this->amount_text, "%f", &amount);

        if (amount < 0.0f) {
            amount = 0.0f;
        }

        if ((float)(int32_t)main_wnd->vis_map_context->my_main_unit->gold < amount) {
            amount = (float)(int32_t)main_wnd->vis_map_context->my_main_unit->gold;
        }

        TokenEntry* entry = main_wnd->vis_invtype1->VMethod36(this->selection, (int32_t)amount);
        main_wnd->vis_map_context->my_main_unit->gold -= entry->count;
        main_wnd->vis_map_context->SendDropGold(entry->count, main_wnd->vis_map_context->my_unit->tileX | (main_wnd->vis_map_context->my_unit->tileY << 8));
        main_wnd->ResetItemCursor();

        if (entry != nullptr) {
            delete entry;
        }
    }

    VisScreen::DoClose(code);
}

// 441F3C
int32_t VisDropGold::OnKeyDown(uint32_t wparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (wparam == 0xd) {
        main_wnd->PostMessage(0x445, 0, 0);
        return 1;
    }

    if (wparam == 0x1b) {
        main_wnd->PostMessage(0x446, 0, 0);
        return 1;
    }

    return VisScreen::OnKeyDown(wparam);
}

// 441FA3
void VisDropGold::VMethod28() {
    strcpy(this->amount_text, "0");

    CVisualObject* textbox = this->FindChild(10000005);
    textbox->ReadData(this->amount_text);

    VisScreen::VMethod28();
}

// 441F1F
void VisDropGold::VMethod9() {
    LockSurface2();
    CVisualObject::VMethod9();
    UnlockSurface2();
}

// 4A7A30
void VisDropGold::SetSelection(int32_t idx) {
    this->selection = idx;
}

// 441BEC
VisDropGold::VisDropGold(int32_t _id, int32_t _x, int32_t _y)
    : VisScreen(_id, _x, _y, _x + 0x128, _y + 0xA8, nullptr) {
    this->VMethod26();
}

// 441C63
VisDropGold::~VisDropGold() {
}

// 432D58
void VisStartGameTextBox::VMethod7() {
    CPoint tl = this->screen->GetRect().TopLeft();
    uint32_t now = timeGetTime();

    CString str;

    LockSurface2();
    if (this->is_active != 0 && this->TestFlags(FLAG_FOCUS)) {
        str = this->text + '|';
    } else {
        str = this->text;
    }

    g_font4->DrawTxt(tl.x + this->rect.left, tl.y + this->rect.bottom - g_font4->GetHeight(), str, 0, this->clr);

    if ((now - this->ts) > 500) {
        this->ts = now;
        this->is_active = this->is_active ^ 1;
    }
    UnlockSurface2();
}

// 432C1F
int32_t VisStartGameTextBox::OnMouseMove(uint32_t wparam, CPoint pos) {
    CPoint tl = this->screen->GetRect().TopLeft();

    if ((this->rect + tl).PtInRect(pos)) {
        this->clr = this->clr_active;
    } else {
        this->clr = this->clr_inactive;
    }

    return 0;
}

// 432C9E
int32_t VisStartGameTextBox::OnLButtonDown(uint32_t wparam, CPoint pos) {
    if (this->rect.PtInRect(pos)) {
        this->screen->FocusTo(this, true);
    }

    return 0;
}

// 432CE3
int32_t VisStartGameTextBox::OnKeyDown(uint32_t wparam) {
    if (wparam == VK_BACK) {
        this->BackspaceChar();
        return 1;
    }

    return CVisualObject::OnKeyDown(wparam);
}

// 432D1B
// client: 434e52
int32_t VisStartGameTextBox::OnChar(uint32_t wparam) {
    int32_t len = this->text.GetLength();
    if (len < 10) {
        this->AcceptChar(EncodeChar((uint8_t)wparam));
    }
#ifdef A2CLIENT
    return 1;
#else
    return len < 10;
#endif   
}

// 432EA3
const char* VisStartGameTextBox::GetHint() {
    if (this->screen->dialogActiveFlag == 0) {
        return nullptr;
    }

    return TxtFile::AllLines[0x100];
}

// 438D80
CString VisStartGameTextBox::GetText() {
    return this->text;
}

// 432C03
void VisStartGameTextBox::SetText(const CString& text) {
    this->text = text;
}

// 432B80
void VisStartGameTextBox::BackspaceChar() {
    this->text = this->text.Left(this->text.GetLength() - 1);
    this->screen->PlayLabelInputSound();
}

// 432AD1
void VisStartGameTextBox::InitTextBox() {
    this->clr_active = palette_paris_daisy->GetPalette(0);
    this->clr_inactive = palette_paris_daisy->GetPalette(0);
    this->clr = this->clr_inactive;
    this->is_active = 1;
    this->ts = timeGetTime();
    this->flags |= 2;
}

// 432B33
// client: 434c63 (client variant also rejects '~')
void VisStartGameTextBox::AcceptChar(uint8_t ch) {
    this->is_active = 1;
    this->ts = timeGetTime();
#ifdef A2CLIENT
    if (ch >= ' ' && ch != '~') {
#else
    if (ch >= ' ') {
#endif
        this->screen->PlayLabelInputSound();
        this->text += (char)ch;
    }
}

// 4329F9
VisStartGameTextBox::VisStartGameTextBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisStartGame* screen)
    : CVisualObject(_id, l, t, r, b, nullptr) {
    this->screen = screen;
    this->InitTextBox();
}

// 4DD54C
void VisListBoxDiplomacy::UpdateRadioPositions(int32_t vis_start_index) {
    for (int32_t i = 0; i < this->diplomacy->GetSize(); i++) {
        bool hidden;
        if (i < this->vis_start_index || this->vis_start_index + this->num_vis_entry <= i) {
            hidden = true;
        } else {
            hidden = false;
        }

        this->enemy_radios->ElementAt(i)->ChangeFlags(FLAG_20, hidden);
        this->ally_radios->ElementAt(i)->ChangeFlags(FLAG_20, hidden);
        this->see_radios->ElementAt(i)->ChangeFlags(FLAG_20, hidden);
        this->mute_radios->ElementAt(i)->ChangeFlags(FLAG_20, hidden);

        int32_t top = (i - vis_start_index) * this->entry_height_full;
        int32_t bottom = top + 0x18;
        this->enemy_radios->ElementAt(i)->SetRect(200, top, 0xfa, bottom);
        this->ally_radios->ElementAt(i)->SetRect(0x10e, top, 0x140, bottom);
        this->see_radios->ElementAt(i)->SetRect(0x154, top, 0x186, bottom);
        this->mute_radios->ElementAt(i)->SetRect(0x19a, top, 0x1cc, bottom);
    }
}

// 4DD0BD
void VisListBoxDiplomacy::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    if (this->IsValidIndex(idx)) {
        DiplomacyEntry* entry = this->diplomacy->GetAt(idx);
        this->font->DrawTextWithShadow(pos.x, pos.y + (this->entry_height_full - this->entry_height) / 2, entry->name, 0, clr, 1);

        this->enemy_radios->ElementAt(idx)->VMethod7();
        this->ally_radios->ElementAt(idx)->VMethod7();
        this->see_radios->ElementAt(idx)->VMethod7();
        this->mute_radios->ElementAt(idx)->VMethod7();
    }
}

// 4DD7F3
int32_t VisListBoxDiplomacy::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    int32_t count = this->diplomacy->GetSize();

    if (msg == VIS_MSG_SET_OPTION) {
        if ((int32_t)wparam < count * 2) {
            CVisualObject* radio = this->FindChild(wparam);
            CVisualObject* other_radio = this->FindChild((wparam + count) % (count << 1));

            int32_t state = 0;
            int32_t other_state = 0;
            radio->WriteData(&state);
            other_radio->WriteData(&other_state);

            if (state != 0 && other_state != 0) {
                other_state = 0;
                other_radio->ReadData(&other_state);
                other_radio->VMethod9();
            }

            return 1;
        }

        return 0;
    }

    return VisListBox::MsgProc(msg, wparam, lparam);
}

// 4DCF98
void VisListBoxDiplomacy::SelectItem(int32_t idx) {
    if (idx < 0) {
        idx = 0;
    }

    int32_t count = this->diplomacy->GetSize();

    int32_t start;
    if (idx < count - this->num_vis_entry) {
        start = idx;
    } else {
        start = count - this->num_vis_entry;
    }

    this->vis_start_index = start;
    this->selected_index = start;

    this->UpdateRadioPositions(this->vis_start_index);
    this->VMethod9();

    VisScrollBar* scrollbar = (VisScrollBar*)this->parent->FindChild(this->scrollbox_id);
    if (scrollbar != nullptr) {
        scrollbar->SetPos(this->vis_start_index, (count - this->num_vis_entry) + 1);
    }

    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->vis_start_index);
}

// 4DD44A
int32_t VisListBoxDiplomacy::OnLButtonDown(uint32_t wparam, CPoint pos) {
    int32_t idx = this->YToIndex(pos.y);
    if (idx < this->num_vis_entry) {
        if (this->vis_start_index + idx < this->diplomacy->GetSize() - 1) {
            idx = this->vis_start_index + idx;
        } else {
            idx = this->diplomacy->GetSize() - 1;
        }
        this->selected_index = idx;
    }

    this->VMethod9();

    if (0 < this->scrollbox_id) {
        VisScrollBar* scrollbar = (VisScrollBar*)this->parent->FindChild(this->scrollbox_id);
        if (scrollbar != nullptr) {
            scrollbar->SetPos(this->selected_index, this->diplomacy->GetSize());
        }
    }

    this->parent->MsgProc(VIS_MSG_SET_OPTION, this->id, this->selected_index);
    return 1;
}

// 4DD098
int32_t VisListBoxDiplomacy::IsValidIndex(int32_t idx) {
    return idx < this->enemy_radios->GetSize();
}

// 4DD8C5
int32_t VisListBoxDiplomacy::OnKeyDown(uint32_t wparam) {
    return CVisualObject::OnKeyDown(wparam);
}

// 4E3D30
void VisListBoxDiplomacy::VMethod30(CPoint pos, const CRect& r) {
}



