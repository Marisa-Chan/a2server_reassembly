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
#include "vis_town.h"

// lizard animation frame sequences from the original data (634870/634898/6348C8/634930), -1 terminated
static const int32_t lizard_seq0[] = { 0x27, 0x28, 0x29, 0x2A, 0x29, 0x28, 0x27, 0x26, -1 };
static const int32_t lizard_seq1[] = { 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, -1 };
static const int32_t lizard_seq2[] = { 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, -1 };
static const int32_t lizard_seq3[] = { 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, -1 };
static const int32_t* const lizard_seqs[4] = { lizard_seq0, lizard_seq1, lizard_seq2, lizard_seq3 };

TownMenuListDialogVisualObject::TownMenuListDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CRect& _r)
: VisMenuWnd(_id, l, t, r, b, nullptr, 0, _r) { //44160e
    MenuButton* btn = new MenuButton(2, txt_dialogs.GetLine(34), g_font1, nullptr, 0x41a, 'S', ""); //Save the game
    this->AddElement(btn, 30);

    btn = new MenuButton(1, txt_dialogs.GetLine(35), g_font1, nullptr, 0x418, 'L', ""); //load game
    this->AddElement(btn, 30);

    btn = new MenuButton(3, txt_dialogs.GetLine(37), g_font1, nullptr, 0x422, 'N', ""); //Sound options
    this->AddElement(btn, 30);

    btn = new MenuButton(4, txt_dialogs.GetLine(77), g_font1, nullptr, 0x41c, 'E', ""); //exit
    this->AddElement(btn, 30);

    btn = new MenuButton(5, txt_dialogs.GetLine(40), g_font1, nullptr, 0x446, 'E', ""); //return to game
    this->AddElement(btn, 30);
}

// 4CDA5C
VisTown::VisTown(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) {
    this->VMethod26();
}


// 4D6B80, real body 4CDC78
VisTown::~VisTown() {
    this->VMethod37();
    this->VMethod31();
    if (this->tips != nullptr) {
        this->RemoveChild(this->tips);
        delete this->tips;
        this->tips = nullptr;
    }
}


// 4CDDAD
void VisTown::VMethod26() {
    for (int32_t i = 0; i < 14; i++) {
        this->sounds[i] = nullptr;
    }
    this->hover_snd_shop = 0;
    this->hover_snd_second = 0;
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
void VisTown::VMethod28() {
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
    } else {
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
void VisTown::VMethod8(CRect* rect) {
    (void)rect;
}


// 4CE4E5
int32_t VisTown::OnMouseMove(uint32_t wparam, CPoint pos) {
    this->VMethod34(pos);
    return 0;
}


// 4CE4AF
int32_t VisTown::OnKeyDown(uint32_t wparam) {
    if (wparam == 0x0D || wparam == 0x1B) {
        return 1;
    }
    return this->VisScreen::OnKeyDown(wparam);
}


// 4CE3B8
int32_t VisTown::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    MainWindow* mainwnd = (MainWindow*)AfxGetMainWnd();
    if (msg == VIS_MSG_FRAME_TICK) {
        if (mainwnd->dialogsMask == 0) {
            this->VMethod9();
        }
    } else if (msg == VIS_MSG_CLOSE_SCREEN) {
        return 1;
    } else if (msg == VIS_MSG_CLOSE_TIPS) {
        if (this->tips != nullptr) {
            this->RemoveChild(this->tips);
            delete this->tips;
            this->tips = nullptr;
        }
    } else if (msg == 0x100 && wparam == 0x0D) {
        return 1;
    }
    return this->VisScreen::MsgProc(msg, wparam, lparam);
}


// 4CE50A
int32_t VisTown::OnLButtonDown(uint32_t wparam, CPoint pos) {
    switch (this->VMethod33(pos)) {
    case 1:
        this->VMethod39();
        AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_SHOP, 0, 0);
        break;
    case 2:
        // tavern
        this->VMethod39();
        AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_INN, 0, 0);
        break;
    case 8:
        if (ScenarioGetVar(0x301) == 0) {
            ShowRoleKeyDialog("plagatguard");
        } else {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(MAIN_MSG_AUTOSAVE, 1, 0);
            AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_GLOBAL_MAP, 0, 0);
        }
        break;
    case 0x10:
        AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_TOWN_MENU, 0, 0);
        break;
    }
    return 1;
}


// 4CE310
void VisTown::DoClose(uint32_t code) {
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
void VisTown::VMethod30() {
    this->VMethod31();
    ReloadSfx(&this->sounds[0], "SFX\\Town\\Crowd.wav");
    ReloadSfx(&this->sounds[3], "SFX\\Town\\Birds1.wav");
    ReloadSfx(&this->sounds[4], "SFX\\Town\\Birds2.wav");
    ReloadSfx(&this->sounds[5], "SFX\\Town\\Flugel.wav");
    ReloadSfx(&this->sounds[6], "SFX\\Town\\Flag.wav");
    ReloadSfx(&this->sounds[7], "SFX\\Town\\Point.wav");
    ReloadSfx(&this->sounds[8], "SFX\\Town\\Shop\\enter.wav");
    ReloadSfx(&this->sounds[9], "SFX\\Town\\School\\Point.wav");
    ReloadSfx(&this->sounds[10], "SFX\\Town\\Stars.wav");
    ReloadSfx(&this->sounds[11], "SFX\\Town\\Horse2.wav");
    ReloadSfx(&this->sounds[12], "SFX\\Town\\Horse3.wav");
    ReloadSfx(&this->sounds[13], "SFX\\Town\\Horse1.wav");
}


// 4D184A
void VisTown::VMethod31() {
    for (int32_t i = 0; i < 14; i++) {
        FreeSfxSample(&this->sounds[i]);
    }
}


// 4D1602
const char* VisTown::GetHint() {
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
int32_t VisTown::StartBirdFlight() {
    this->town_anim |= 0x80;
    this->bird_group_index = GetRandS16(3);
    this->active_bird = GetRandS16(3) + 1;
    if (this->active_bird == 1) {
        CSound::Play((CSound&)this->sounds[3]);
    } else {
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
void VisTown::DrawBirdFlock() {
    int32_t done_count = 0;
    if ((this->town_anim & 0x80) != 0) {
        for (int32_t i = 0; i < this->active_bird; i++) {
            CA16* spr = this->spr_birds.ElementAt(this->bird_group_index * 3 + i);
            int32_t frame_count = spr->GetFrameCount();
            if ((&this->bird_frame0)[i] < frame_count) {
                spr->Draw(this->rect.left, this->rect.top, (&this->bird_frame0)[i], 0, 0);
            } else {
                done_count++;
            }
        }
        this->bmp_bird_overlay->DrawSubRectMasked(this->rect.left, this->rect.top, 0, 0, this->bmp_bird_overlay->GetWidth(), this->bmp_bird_overlay->GetHeight());
        if (done_count == this->active_bird) {
            this->town_anim &= ~0x80u;
        }
        this->last_bird = timeGetTime();
    }
}


// 4CE886
void VisTown::VMethod7() {
    static uint8_t anim_init_flags;   // 6669BC in asm
    static uint32_t hover_tick;       // 66699C in asm
    static uint32_t bird_anim_delay;  // 6669C4 in asm

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
        bird_anim_delay = (uint32_t)this->StartBirdFlight();
    }
    LockSurface2();
    if (this->bmp_bkg != nullptr) {
        this->bmp_bkg->Draw(this->rect.left, this->rect.top, 0, 0, 0);
    }
    this->DrawBirdFlock();
    if (this->hovered_action_mask == 1) {
        this->bmp_shop_hover->Draw(this->rect.left + 0x108, this->rect.top + 0x108, 0, 0, 0);
    }
    if (this->hovered_action_mask == 2) {
        this->bmp_tavern_hover->Draw(this->rect.left + 0x90, this->rect.top + 0x14C, 0, 0, 0);
    }
    if (this->spr_tavern != nullptr) {
        this->spr_tavern->Draw(this->rect.left + 0x7C, this->rect.top + 0x138, this->tavern_frame, 0, 0);
    }
    if (this->bmp_cur_sign != nullptr) {
        this->bmp_cur_sign->Draw(this->rect.left + 0x168, this->rect.top + 0xE8, 0, 0, 0);
    }
    if (this->bmp_cur_door != nullptr) {
        this->bmp_cur_door->Draw(this->rect.left + 0xB4, this->rect.top + 0x94, 0, 0, 0);
    }
    if (this->bmp_cur_stars != nullptr) {
        this->bmp_cur_stars->Draw(this->rect.left + 0x154, this->rect.top + 0x120, 0, 0, 0);
    }
    if (this->spr_shop != nullptr) {
        this->spr_shop->Draw(this->rect.left + 0x114, this->rect.top + 0x128, this->shop_frame, 0, 0);
    }
    if (this->bmp_cur_flugel != nullptr) {
        this->bmp_cur_flugel->Draw(this->rect.left + 0x134, this->rect.top + 0x40, 0, 0, 0);
    }
    if (this->spr_guard != nullptr) {
        this->spr_guard->Draw(this->rect.left + 0xB8, this->rect.top + 0x9E, this->guard_frame, 0, 0);
    }
    if (this->spr_cur_horse != nullptr) {
        if (this->horse_anim_index == 0) {
            if (this->horse_frame == 0xE) {
                CSound::Play((CSound&)this->sounds[11]);
            }
        } else if (this->horse_anim_index == 1) {
            if (this->horse_frame == 8) {
                CSound::Play((CSound&)this->sounds[11]);
            }
            if (this->horse_frame == 0xE) {
                CSound::Play((CSound&)this->sounds[11]);
            }
        } else if (this->horse_anim_index == 2) {
            if (this->horse_frame == 1) {
                CSound::Play((CSound&)this->sounds[13]);
            }
            if (this->horse_frame == 0xE) {
                CSound::Play((CSound&)this->sounds[11]);
            }
        }
        if (this->horse_frame == -1) {
            this->spr_cur_horse->Draw(this->rect.left + this->horse_position.x, this->rect.top + this->horse_position.y, 0, 0, 0);
        } else {
            this->spr_cur_horse->Draw(this->rect.left + this->horse_position.x, this->rect.top + this->horse_position.y, this->horse_frame, 0, 0);
        }
    }
    if (this->spr_cur_bbird != nullptr) {
        if (this->bbird_frame == -1) {
            this->spr_cur_bbird->Draw(this->rect.left + this->bbird_position.x, this->rect.top + this->bbird_position.y, 0, 0, 0);
        } else {
            this->spr_cur_bbird->Draw(this->rect.left + this->bbird_position.x, this->rect.top + this->bbird_position.y, this->bbird_frame, 0, 0);
        }
    }
    if (this->spr_dervish != nullptr) {
        this->spr_dervish->Draw(this->rect.left + this->dervish_position.x, this->rect.top + this->dervish_position.y, this->dervish_frame, 0, 0);
    }
    UnlockSurface2();
    this->VisScreen::VMethod7();
}


// 4D1709
void VisTown::VMethod32() {
    PlayAmbientSfx(this->sounds);
}


// 4D15E3
void VisTown::VMethod39() {
    this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
}


// 4D10F0
int32_t VisTown::VMethod33(CPoint pos) {
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
void VisTown::VMethod34(CPoint pos) {
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
            StopSfx(&this->sounds[9]);
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
        } else {
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
void VisTown::VMethod35() {
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
void VisTown::AdvanceGuardAnim() {
    this->guard_frame += this->guard_frame_step;
    if (this->guard_frame_step == 1 && this->guard_sound == 0) {
        ReloadSfx(&this->sounds[2], "SFX\\Town\\Guard2.wav");
        CSound::Play((CSound&)this->sounds[2]);
        this->guard_sound = 1;
    } else if (this->guard_frame_step == -1 && this->guard_sound != 0) {
        ReloadSfx(&this->sounds[2], "SFX\\Town\\Guard1.wav");
        CSound::Play((CSound&)this->sounds[2]);
        this->guard_sound = 0;
    }
    if (this->guard_frame < 0) {
        this->guard_frame_step = 0;
        this->guard_frame = 0;
        FreeSfxSample(&this->sounds[2]);
    } else {
        if (this->spr_guard->GetFrameCount() <= this->guard_frame) {
            this->guard_frame_step = 0;
            this->guard_frame = this->spr_guard->GetFrameCount() - 1;
            FreeSfxSample(&this->sounds[2]);
        }
    }
}


// 4D07E8
void VisTown::AdvanceTavernAnim() {
    if (this->tavern_frame == 0) {
        StopSfx(&this->sounds[8]);
        StopSfx(&this->sounds[9]);
        CSound::Play((CSound&)this->sounds[7]);
    }
    this->tavern_frame++;
    if (this->tavern_frame == this->spr_tavern->GetFrameCount()) {
        this->tavern_frame = 0;
        this->town_anim &= ~2u;
    }
}


// 4D0884
void VisTown::AdvanceSignAnim() {
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
void VisTown::AdvanceGateAnim() {
    if (ScenarioGetVar(0x301) == 0) {
        this->door_frame = 8;
        this->bmp_cur_door = this->bmp_door.GetAt(this->door_frame);
        return;
    }
    CPoint pt(g_mousept.GetX(), g_mousept.GetY());
    if (this->VMethod33(pt) != 8) {
        if (this->door_open_flag != 0) {
            ReloadSfx(&this->sounds[1], "SFX\\Town\\GateDn.wav");
            CSound::Play((CSound&)this->sounds[1]);
        }
        this->door_open_flag = 0;
        this->door_frame++;
        if (this->door_frame > 7) {
            this->door_frame = 8;
            this->town_anim &= ~0x08u;
        }
    } else {
        if (this->door_open_flag == 0) {
            ReloadSfx(&this->sounds[1], "SFX\\Town\\GateUp.wav");
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
void VisTown::AdvanceStarsAnim() {
    static int32_t stars_repeat; // 6669d0 in asm
    if (this->stars_frame == 0) {
        CSound::Play((CSound&)this->sounds[10]);
    }
    this->stars_frame++;
    if (this->stars_frame < 9) {
        this->bmp_cur_stars = this->bmp_stars.GetAt(this->stars_frame);
    } else {
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
void VisTown::AdvanceFighterAnim() {
    static int32_t fighter_dir; // 6669d4 in asm
    if (this->fighter_frame < 1 && fighter_dir == 0) {
        fighter_dir = (rand() % 100 > 0x5F) ? 1 : 0;
    } else if (this->fighter_frame == 10) {
        fighter_dir = (rand() % 100 > 0x5F) ? -1 : 0;
    } else if (this->fighter_frame == 0 && fighter_dir == -1) {
        this->fighter_frame = 0;
        fighter_dir = 0;
        this->town_anim &= ~4u;
    }
    this->fighter_frame += fighter_dir;
}


// 4D0C6E
void VisTown::AdvanceMageAnim() {
    static int32_t mage_dir; // 6669d8 in asm
    if (this->mage_frame < 1 && mage_dir == 0) {
        mage_dir = (rand() % 100 > 0x5F) ? 1 : 0;
    } else if (this->mage_frame == 10) {
        mage_dir = (rand() % 100 > 0x5F) ? -1 : 0;
    } else if (this->mage_frame == 0 && mage_dir == -1) {
        this->mage_frame = 0;
        mage_dir = 0;
        this->town_anim &= ~4u;
    }
    this->mage_frame += mage_dir;
}


// 4D0D47
void VisTown::AdvanceShopAnim() {
    this->shop_frame++;
    if (this->shop_frame == this->spr_shop->GetFrameCount()) {
        this->shop_frame = 0;
        this->town_anim &= ~1u;
    }
}


// 4D0DA2
void VisTown::AdvanceFlugelAnim() {
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
void VisTown::AdvanceBlackBirdAnim() {
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
void VisTown::AdvanceHorseAnim() {
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
void VisTown::AdvanceDervishAnim() {
    if (this->spr_dervish == nullptr) {
        this->dervish_frame = -1;
    } else {
        this->dervish_frame = (this->dervish_frame + 1) % this->spr_dervish->GetFrameCount();
    }
}


// 4D0E31
void VisTown::VMethod38() {
    if ((this->town_anim & 1) != 0) {
        this->AdvanceShopAnim();
    }
    if ((this->town_anim & 2) != 0) {
        this->AdvanceTavernAnim();
    }
    this->AdvanceGateAnim();
    if ((this->town_anim & 0x10) != 0) {
        this->AdvanceStarsAnim();
    }
    if ((this->town_anim & 0x40) != 0) {
        this->AdvanceSignAnim();
    }
    if ((this->town_anim & 0x20) != 0) {
        this->AdvanceFlugelAnim();
    }
    if ((this->town_anim & 0x80) != 0) {
        this->bird_frame0++;
        this->bird_frame1++;
        this->bird_frame2++;
    }
    if ((this->town_anim & 0x400) != 0) {
        this->AdvanceDervishAnim();
    }
    if ((this->town_anim & 0x200) != 0) {
        this->AdvanceBlackBirdAnim();
    }
    if ((this->town_anim & 0x100) != 0) {
        this->AdvanceHorseAnim();
    }
    this->AdvanceGuardAnim();
}


// 4CFD0E
void VisTown::VMethod37() {
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
void VisTown::VMethod36() {
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
    this->AdvanceTavernAnim();
    this->sign_frame = -1;
    this->AdvanceSignAnim();
    this->door_frame = 9;
    this->AdvanceGateAnim();
    this->stars_frame = -1;
    this->AdvanceStarsAnim();
    this->fighter_frame = 0;
    this->AdvanceFighterAnim();
    this->mage_frame = 0;
    this->AdvanceMageAnim();
    this->shop_frame = -1;
    this->AdvanceShopAnim();
    this->flugel_frame = -1;
    this->AdvanceFlugelAnim();
    this->bbird_frame = -1;
    this->horse_frame = -1;
    this->dervish_frame = -1;
}


// 4D423A
VisTownKaarg::VisTownKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisTown(_id, l, t, r, b) {
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
VisTownKaarg::~VisTownKaarg() {
}


// 4D54B5
void VisTownKaarg::VMethod32() {
    PlayAmbientSfx(this->sounds);
}


// 4D54CF
void VisTownKaarg::VMethod30() {
    ReloadSfx(&this->snd_voice[0], "sfx\\town_kaarg\\Kvox2.wav");
    ReloadSfx(&this->snd_voice[1], "sfx\\town_kaarg\\Kvox3.wav");
    ReloadSfx(&this->snd_voice[2], "sfx\\town_kaarg\\Kvox4.wav");
    ReloadSfx(&this->snd_bird[0], "sfx\\town_kaarg\\Kbird1.wav");
    ReloadSfx(&this->snd_bird[1], "sfx\\town_kaarg\\Kbird2.wav");
    ReloadSfx(&this->snd_bird[2], "sfx\\town_kaarg\\Kbird3.wav");
    ReloadSfx(&this->snd_bird[3], "sfx\\town_kaarg\\Kbird4.wav");
    ReloadSfx(&this->snd_voice1, "sfx\\town_kaarg\\Kvox1.wav");
    ReloadSfx(&this->snd_shop_enter, "sfx\\town_kaarg\\Kenter2.wav");
    ReloadSfx(&this->snd_tavern_enter, "sfx\\town_kaarg\\Kenter1.wav");
    ReloadSfx(&this->snd_dervish, "sfx\\town_kaarg\\Kman1.wav");
    ReloadSfx(&this->snd_guard[0], "sfx\\town_kaarg\\Ksteps2.wav");
    ReloadSfx(&this->snd_guard[1], "sfx\\town_kaarg\\Ksteps21.wav");
    ReloadSfx(&this->snd_guard[2], "sfx\\town_kaarg\\Ksteps1.wav");
    ReloadSfx(&this->snd_guard[3], "sfx\\town_kaarg\\Ksteps11.wav");
    ReloadSfx(&this->snd_guard[4], "sfx\\town_kaarg\\Ksteps3.wav");
    ReloadSfx(&this->snd_guard[5], "sfx\\town_kaarg\\Ksteps31.wav");
    this->tavern_snd_flag = 0;
    this->hover_snd_shop = 0;
    this->exit_snd_flag = 0;
}


// 4D5682
void VisTownKaarg::VMethod31() {
    for (int32_t i = 0; i < 3; i++) {
        FreeSfxSample(&this->snd_voice[i]);
    }
    for (int32_t i = 0; i < 4; i++) {
        FreeSfxSample(&this->snd_bird[i]);
    }
    FreeSfxSample(&this->snd_voice1);
    FreeSfxSample(&this->snd_shop_enter);
    FreeSfxSample(&this->snd_tavern_enter);
    FreeSfxSample(&this->snd_dervish);
    for (int32_t i = 0; i < 6; i++) {
        FreeSfxSample(&this->snd_guard[i]);
    }
}


// 4D5C9A
void VisTownKaarg::VMethod34(CPoint pos) {
    int32_t mask = this->VMethod33(pos);
    this->guard_frame_step = 1;
    this->hovered_action_mask = mask;
    if (mask < 9) {
        if (mask == 8) {
            this->tavern_snd_flag = 0;
            this->hover_snd_shop = 0;
            if (this->exit_snd_flag == 0) {
                StopSfx(&this->snd_shop_enter);
                StopSfx(&this->snd_tavern_enter);
                this->exit_snd_flag = 1;
            }
        } else {
            switch (mask + 1) {
            case 0:
                ApplyCursor(g_Cursors[0]);
                this->tavern_snd_flag = 0;
                this->hover_snd_shop = 0;
                this->exit_snd_flag = 0;
                break;
            case 2:
                if (this->hover_snd_shop == 0) {
                    StopSfx(&this->snd_tavern_enter);
                    CSound::Play((CSound&)this->snd_shop_enter);
                    this->hover_snd_shop = 1;
                }
                this->tavern_snd_flag = 0;
                this->exit_snd_flag = 0;
                break;
            case 3:
                if (this->tavern_snd_flag == 0) {
                    StopSfx(&this->snd_shop_enter);
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
    } else {
        if (mask != 0x200 && mask != 0x400 && mask != 0x800) {
            this->town_anim |= mask;
            this->tavern_snd_flag = 0;
            this->hover_snd_shop = 0;
            this->exit_snd_flag = 0;
        }
    }
}


// 4D4866
void VisTownKaarg::VMethod36() {
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
void VisTownKaarg::VMethod37() {
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
void VisTownKaarg::AdvanceGateAnim() {
    CPoint pt(g_mousept.GetX(), g_mousept.GetY());
    if (this->VMethod33(pt) != 8) {
        if (this->door_open_flag != 0) {
            ReloadSfx(&this->sounds[1], "SFX\\Town_kaarg\\Kdoor2.wav");
            CSound::Play((CSound&)this->sounds[1]);
        }
        this->door_open_flag = 0;
        this->door_frame--;
        if (this->door_frame < 1) {
            this->door_frame = 0;
            this->town_anim &= ~0x08u;
        }
    } else {
        if (this->door_open_flag == 0) {
            ReloadSfx(&this->sounds[1], "SFX\\Town_kaarg\\Kdoor1.wav");
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
void VisTownKaarg::AdvanceGirl1Anim() {
    this->bbird_frame++;
    if (this->bbird_frame == this->bmp_girl1[this->girl1_group].GetSize()) {
        this->bbird_frame = 0;
        this->girl1_group = -1;
        this->town_anim &= ~0x200u;
    }
}


// 4D6655
void VisTownKaarg::AdvanceGirl2Anim() {
    this->girl2_frame++;
    if (this->girl2_frame == this->bmp_girl2[this->girl2_group].GetSize()) {
        this->girl2_frame = 0;
        this->girl2_group = -1;
        this->town_anim &= ~4u;
    }
}


// 4D66CA
void VisTownKaarg::AdvanceGuardAnim() {
    this->fighter_frame++;
    if (this->fighter_frame == this->bmp_guard.GetSize()) {
        this->fighter_frame = 0;
        this->town_anim &= ~0x800u;
    }
}


// 4D6728
void VisTownKaarg::AdvanceDervishAnim() {
    this->dervish_frame++;
    if (this->dervish_frame == this->bmp_dervish.GetSize()) {
        this->dervish_frame = 0;
        this->town_anim &= ~0x400u;
    }
}


// 4D6786
void VisTownKaarg::VMethod38() {
    if ((this->town_anim & 0x200) != 0) {
        this->AdvanceGirl1Anim();
    }
    if ((this->town_anim & 4) != 0) {
        this->AdvanceGirl2Anim();
    }
    if ((this->town_anim & 0x400) != 0) {
        this->AdvanceDervishAnim();
    }
    if ((this->town_anim & 0x800) != 0) {
        this->AdvanceGuardAnim();
    }
    this->AdvanceGateAnim();
}


// 4D5EF0
void VisTownKaarg::VMethod35() {
    static uint8_t timer_init_flags; // 6669AC in asm
    static uint32_t voice_delay;     // 666994 in asm
    static uint32_t bird_delay;      // 6669B0 in asm

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
        } else if (this->active_bird == 2) {
            CSound::Play((CSound&)this->snd_voice[1]);
        } else if (this->active_bird == 3) {
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
                } else {
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
                } else {
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
                } else {
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
const char* VisTownKaarg::GetHint() {
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
    case VIS_MSG_MOUSE:
        return TxtFile::AllLines[0x16A];
    case 0x800:
        return TxtFile::AllLines[0x169];
    }
    return nullptr;
}


// 4D6802
int32_t VisTownKaarg::OnLButtonDown(uint32_t wparam, CPoint pos) {
    int32_t mask = this->VMethod33(pos);
    if (mask < 9) {
        if (mask == 8) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(MAIN_MSG_AUTOSAVE, 1, 0);
            AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_GLOBAL_MAP, 0, 0);
        } else if (mask == 1) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_SHOP, 0, 0);
        } else if (mask == 2) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_INN, 0, 0);
        } else if (mask == 4) {
            CString name;
            name.Format("kaargwoman%d", ScenarioGetVar(0x300));
            ShowRoleKeyDialog(name);
        }
    } else if (mask < 0x201) {
        if (mask == 0x200) {
            CString name;
            name.Format("kaargwoman%d", ScenarioGetVar(0x300));
            ShowRoleKeyDialog(name);
        } else if (mask == 0x10) {
            AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_TOWN_MENU, 0, 0);
        }
    } else if (mask == 0x400) {
        CString name;
        name.Format("kaargman%d", ScenarioGetVar(0x300));
        ShowRoleKeyDialog(name);
    } else if (mask == 0x800) {
        CString name;
        name.Format("kaargguard%d", ScenarioGetVar(0x300));
        ShowRoleKeyDialog(name);
    }
    return 1;
}


// 4D4592
void VisTownKaarg::VMethod28() {
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
    } else {
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
    PlayAmbientSfx(&this->snd_voice1);
    g_mousept.EnableHint();
}


// 4D57B9
void VisTownKaarg::VMethod7() {
    static uint8_t anim_init_flags;  // 6669C0 in asm
    static uint32_t hover_tick;      // 6669B8 in asm
    static uint32_t bird_anim_delay; // 6669CC in asm

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
        this->bmp_bkg->Draw(this->rect.left, this->rect.top, 0, 0, 0);
    }
    if (this->hovered_action_mask == 1) {
        this->bmp_shop_hover->Draw(this->rect.left + 0x148, this->rect.top + 0x100, 0, 0, 0);
    }
    if (this->hovered_action_mask == 2) {
        this->bmp_tavern_hover->Draw(this->rect.left + 0x1E0, this->rect.top + 0xC4, 0, 0, 0);
    }
    if ((this->town_anim & 0x200) == 0) {
        this->bmp_girl1[0].GetAt(0)->Draw(this->rect.left + 0xD8, this->rect.top + 0x11C, 0, 0, 0);
    } else {
        this->bmp_girl1[this->girl1_group].GetAt(this->bbird_frame)->Draw(this->rect.left + 0xD8, this->rect.top + 0x11C, 0, 0, 0);
    }
    if ((this->town_anim & 4) == 0) {
        this->bmp_girl2[0].GetAt(0)->Draw(this->rect.left + 0x104, this->rect.top + 0x11C, 0, 0, 0);
    } else {
        this->bmp_girl2[this->girl2_group].GetAt(this->girl2_frame)->Draw(this->rect.left + 0x104, this->rect.top + 0x11C, 0, 0, 0);
    }
    if ((this->town_anim & 0x800) == 0) {
        this->bmp_guard.GetAt(0)->Draw(this->rect.left + 0xB8, this->rect.top + 0x9C, 0, 0, 0);
    } else if (this->fighter_frame < 0xD || this->fighter_frame > 0x25) {
        this->bmp_guard.GetAt(this->fighter_frame)->Draw(this->rect.left + 0xB8, this->rect.top + 0x9C, 0, 0, 0);
    } else {
        this->bmp_guard.GetAt(this->fighter_frame)->Draw(this->rect.left + 0x8C, this->rect.top + 0x98, 0, 0, 0);
    }
    if ((this->town_anim & 0x400) == 0) {
        this->bmp_dervish.GetAt(0)->Draw(this->rect.left + 0x1A0, this->rect.top + 0x148, 0, 0, 0);
    } else {
        this->bmp_dervish.GetAt(this->dervish_frame)->Draw(this->rect.left + 0x1A0, this->rect.top + 0x148, 0, 0, 0);
    }
    this->bmp_gate.GetAt(this->door_frame)->Draw(this->rect.left + 0x98, this->rect.top + 0x100, 0, 0, 0);
    UnlockSurface2();
    this->VisScreen::VMethod7();
}


// 4D1AF8
VisTownDruid::VisTownDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisTown(_id, l, t, r, b) {
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
VisTownDruid::~VisTownDruid() {
}


// 4D2AE3
void VisTownDruid::VMethod32() {
    PlayAmbientSfx(&this->snd_forest);
}


// 4D2AFF
void VisTownDruid::VMethod30() {
    this->VMethod31();
    ReloadSfx(&this->snd_lizard[0], "sfx\\town_druid\\Dlizard1.wav");
    ReloadSfx(&this->snd_lizard[1], "sfx\\town_druid\\Dlizard2.wav");
    ReloadSfx(&this->snd_lizard[2], "sfx\\town_druid\\Dlizard3.wav");
    ReloadSfx(&this->snd_lizard[3], "sfx\\town_druid\\Dlizard4.wav");
    ReloadSfx(&this->snd_bug[0], "sfx\\town_druid\\Dbug1.wav");
    ReloadSfx(&this->snd_bug[1], "sfx\\town_druid\\Dbug2.wav");
    ReloadSfx(&this->snd_bug[2], "sfx\\town_druid\\Dbug3.wav");
    ReloadSfx(&this->snd_bird[0], "sfx\\town_druid\\Dbird1.wav");
    ReloadSfx(&this->snd_bird[1], "sfx\\town_druid\\Dbird2.wav");
    ReloadSfx(&this->snd_bird[2], "sfx\\town_druid\\Dbird3.wav");
    ReloadSfx(&this->snd_tree[0], "sfx\\town_druid\\Dtree1.wav");
    ReloadSfx(&this->snd_tree[1], "sfx\\town_druid\\Dtree2.wav");
    ReloadSfx(&this->snd_tree[2], "sfx\\town_druid\\Dtree3.wav");
    ReloadSfx(&this->snd_tree[3], "sfx\\town_druid\\Dtree4.wav");
    ReloadSfx(&this->snd_forest, "sfx\\town_druid\\Dforest1.wav");
    ReloadSfx(&this->snd_shop, "sfx\\town_druid\\Ddruid1.wav");
    ReloadSfx(&this->snd_tavern, "sfx\\town_druid\\Ddruid2.wav");
    ReloadSfx(&this->snd_shop_enter, "sfx\\town_druid\\Denter2.wav");
    ReloadSfx(&this->snd_tavern_enter, "sfx\\town_druid\\Denter1.wav");
    ReloadSfx(&this->snd_town_exit, "sfx\\town_druid\\Dout.wav");
    ReloadSfx(&this->snd_wolf, "sfx\\town_druid\\Dwolf1.wav");
    this->tavern_hover_snd_flag = 0;
    this->hover_snd_shop = 0;
    this->mission_exit_hover_snd_flag = 0;
}


// 4D2D1B
void VisTownDruid::VMethod31() {
    for (int32_t i = 0; i < 4; i++) {
        FreeSfxSample(&this->snd_lizard[i]);
    }
    for (int32_t i = 0; i < 3; i++) {
        FreeSfxSample(&this->snd_bug[i]);
    }
    for (int32_t i = 0; i < 3; i++) {
        FreeSfxSample(&this->snd_bird[i]);
    }
    for (int32_t i = 0; i < 4; i++) {
        FreeSfxSample(&this->snd_tree[i]);
    }
    FreeSfxSample(&this->snd_forest);
    FreeSfxSample(&this->snd_shop);
    FreeSfxSample(&this->snd_tavern);
    FreeSfxSample(&this->snd_shop_enter);
    FreeSfxSample(&this->snd_tavern_enter);
    FreeSfxSample(&this->snd_town_exit);
    FreeSfxSample(&this->snd_wolf);
}

// 4D3384
void VisTownDruid::AdvanceTavernAnim() {
    if (this->tavern_frame == 0) {
        StopSfx(&this->sounds[8]);
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
void VisTownDruid::AdvanceShopAnim() {
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
void VisTownDruid::AdvanceBugAnim() {
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
void VisTownDruid::AdvanceLizardAnim() {
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
void VisTownDruid::VMethod36() {
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
            name.Format("graphics\\interface\\town_druid\\man\\a%d%04d.bmp", g + 1, i);
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
            name.Format("graphics\\interface\\town_druid\\woman\\a%d%04d.bmp", g + 1, i);
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
    this->AdvanceTavernAnim();
    this->sign_frame = -1;
    this->door_frame = 9;
    this->stars_frame = -1;
    this->fighter_frame = 0;
    this->mage_frame = 0;
    this->shop_frame = -1;
    this->shop_group = -1;
    this->AdvanceShopAnim();
    this->flugel_frame = -1;
    this->lizard_variant = -1;
    this->lizard_frame = -1;
    this->bbird_frame = -1;
    this->horse_frame = -1;
    this->dervish_frame = -1;
    this->bug_variant = -1;
}


// 4D27FF
void VisTownDruid::VMethod37() {
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
void VisTownDruid::VMethod38() {
    if ((this->town_anim & 1) != 0) {
        this->AdvanceShopAnim();
    }
    if ((this->town_anim & 2) != 0) {
        this->AdvanceTavernAnim();
    }
    if ((this->town_anim & 0x80) != 0) {
        this->AdvanceBugAnim();
    }
    if ((this->town_anim & 0x100) != 0) {
        this->AdvanceLizardAnim();
    }
}


// 4D359C
void VisTownDruid::VMethod34(CPoint pos) {
    int32_t mask = this->VMethod33(pos);
    this->guard_frame_step = 1;
    int32_t old_mask = this->hovered_action_mask;
    this->hovered_action_mask = mask;
    if (mask < 9) {
        if (mask == 8) {
            this->tavern_hover_snd_flag = 0;
            this->hover_snd_shop = 0;
            if (this->mission_exit_hover_snd_flag == 0) {
                StopSfx(&this->snd_shop_enter);
                StopSfx(&this->snd_tavern_enter);
                CSound::Play((CSound&)this->snd_town_exit);
                this->mission_exit_hover_snd_flag = 1;
            }
        } else {
            switch (mask + 1) {
            case 0:
                ApplyCursor(g_Cursors[0]);
                this->tavern_hover_snd_flag = 0;
                this->hover_snd_shop = 0;
                this->mission_exit_hover_snd_flag = 0;
                break;
            case 2:
                if (this->shop_group != 2 && old_mask != this->hovered_action_mask) {
                    StopSfx(&this->snd_shop);
                    this->shop_tick = timeGetTime();
                    this->shop_group = 2;
                    this->shop_frame = -1;
                    this->town_anim |= mask;
                    this->AdvanceShopAnim();
                    CSound::Play((CSound&)this->snd_shop);
                }
                if (this->hover_snd_shop == 0) {
                    StopSfx(&this->snd_tavern_enter);
                    StopSfx(&this->snd_town_exit);
                    CSound::Play((CSound&)this->snd_shop_enter);
                    this->hover_snd_shop = 1;
                }
                this->tavern_hover_snd_flag = 0;
                this->mission_exit_hover_snd_flag = 0;
                break;
            case 3:
                if (this->tavern_group != 2 && old_mask != this->hovered_action_mask) {
                    StopSfx(&this->snd_tavern);
                    this->tavern_tick = timeGetTime();
                    this->tavern_group = 2;
                    this->tavern_frame = -1;
                    this->town_anim |= mask;
                    this->AdvanceTavernAnim();
                    CSound::Play((CSound&)this->snd_tavern);
                }
                if (this->tavern_hover_snd_flag == 0) {
                    StopSfx(&this->snd_shop_enter);
                    StopSfx(&this->snd_town_exit);
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
    } else {
        if (mask != 0x200 && mask != 0x1000) {
            this->town_anim |= mask;
            this->tavern_hover_snd_flag = 0;
            this->hover_snd_shop = 0;
            this->mission_exit_hover_snd_flag = 0;
        }
    }
}


// 4D3922
void VisTownDruid::VMethod35() {
    static uint8_t timer_init_flags; // 666998 in asm
    static uint32_t bird_delay;      // 6669A4 in asm
    static uint32_t tree_delay;      // 6669B4 in asm

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
            } else {
                this->shop_group = GetRandS16(2);
            }
            this->town_anim |= 1;
            StopSfx(&this->snd_shop);
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
            } else {
                this->tavern_group = GetRandS16(2);
            }
            this->town_anim |= 2;
            StopSfx(&this->snd_tavern);
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
        } else if (this->active_bird == 2) {
            CSound::Play((CSound&)this->snd_bird[1]);
        } else if (this->active_bird == 3) {
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
        } else if (bug_index == 2) {
            CSound::Play((CSound&)this->snd_bug[1]);
        } else if (bug_index == 3) {
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
        } else if (roll == 2) {
            this->lizard_variant = 2;
            CSound::Play((CSound&)this->snd_lizard[2]);
        } else if (roll == 3) {
            this->lizard_variant = 3;
            CSound::Play((CSound&)this->snd_lizard[3]);
        } else {
            this->lizard_variant = 0;
            CSound::Play((CSound&)this->snd_lizard[0]);
        }
        this->town_anim |= 0x100;
        this->lizard_frame = 0;
    }
}


// 4D2E99
void VisTownDruid::VMethod7() {
    static uint8_t anim_init_flags;  // 6669A0 in asm
    static uint32_t hover_tick;      // 6669A8 in asm
    static uint32_t bird_anim_delay; // 6669C8 in asm

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
        this->bmp_bkg->Draw(this->rect.left, this->rect.top, 0, 0, 0);
    }
    if (this->hovered_action_mask == 1 || this->shop_group == 2) {
        this->bmp_shop_hover->Draw(this->rect.left + 0x1A4, this->rect.top + 0xE0, 0, 0, 0);
    }
    if (this->shop_group < 0) {
        if (this->hovered_action_mask == 1) {
            this->shop_frame_group[2].GetAt(0)->Draw(this->rect.left + 0x154, this->rect.top + 0xE0, 0, 0, 0);
        } else {
            this->shop_frame_group[0].GetAt(0)->Draw(this->rect.left + 0x150, this->rect.top + 0xF4, 0, 0, 0);
        }
    } else {
        this->shop_frame_group[this->shop_group].GetAt(this->shop_frame)->Draw(
            this->rect.left + 0x150 + (this->shop_group / 2) * 4,
            this->rect.top + 0xF4 - (this->shop_group / 2) * 0x14, 0, 0, 0);
    }
    if (this->hovered_action_mask == 2 || this->tavern_group == 2) {
        this->bmp_tavern_hover->Draw(this->rect.left, this->rect.top + 0xB8, 0, 0, 0);
    }
    if (this->tavern_group < 0) {
        if (this->hovered_action_mask == 2) {
            this->tavern_frame_group[2].GetAt(0)->Draw(this->rect.left + 0x98, this->rect.top + 0xB8, 0, 0, 0);
        } else {
            this->tavern_frame_group[0].GetAt(0)->Draw(this->rect.left + 0xA4, this->rect.top + 0xC8, 0, 0, 0);
        }
    } else {
        this->tavern_frame_group[this->tavern_group].GetAt(this->tavern_frame)->Draw(
            this->rect.left + 0xA4 - (this->tavern_group / 2) * 0xC,
            this->rect.top + 0xC8 - (this->tavern_group / 2) * 0x10, 0, 0, 0);
    }
    if (this->lizard_variant < 0 || this->lizard_frame < 0) {
        this->spr_lizard->Draw(this->rect.left, this->rect.top + 300, 0, 0, 0);
    } else {
        this->spr_lizard->Draw(this->rect.left, this->rect.top + 300, lizard_seqs[this->lizard_variant][this->lizard_frame] - 1, 0, 0);
    }
    if (this->bug_variant >= 0 && this->bug_frame >= 0) {
        this->spr_bug->Draw(this->rect.left, this->rect.top + 0xD4, this->bug_variant * 0x3D + this->bug_frame, 0, 0);
    }
    UnlockSurface2();
    this->VisScreen::VMethod7();
}


// 4D1E80
void VisTownDruid::VMethod28() {
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
    } else {
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
int32_t VisTownDruid::OnLButtonDown(uint32_t wparam, CPoint pos) {
    int32_t mask = this->VMethod33(pos);
    if (mask < 9) {
        if (mask == 8) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(MAIN_MSG_AUTOSAVE, 1, 0);
            AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_GLOBAL_MAP, 0, 0);
        } else if (mask == 1) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_SHOP, 0, 0);
        } else if (mask == 2) {
            this->VMethod39();
            AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_INN, 0, 0);
        }
    } else if (mask == 0x10) {
        AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_TOWN_MENU, 0, 0);
    } else if (mask == 0x200) {
        CString name;
        name.Format("druidinnkeeper%d", ScenarioGetVar(0x300));
        ShowRoleKeyDialog(name);
    } else if (mask == 0x1000) {
        CString name;
        name.Format("druidshopkeeper%d", ScenarioGetVar(0x300));
        ShowRoleKeyDialog(name);
    }
    return 1;
}


