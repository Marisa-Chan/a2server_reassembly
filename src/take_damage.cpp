#include <winsock2.h>
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
#include "take_damage.h"

// 45db60 (client: 461780)
TakeDamage::TakeDamage() = default;
TakeDamage::TakeDamage(const TakeDamage& ref) = default;
// 45dd08 (client: 461928)
TakeDamage::~TakeDamage() = default;

// 45db8a (client: 4617aa)
TakeDamage::TakeDamage(int32_t _dmg, uint16_t* clr, int32_t unk, int32_t _dx, int32_t _dy, CUnit* ct) {
    this->dmg = _dmg;

    this->txt.Format("%d", this->dmg);

    this->drift_left = unk;
    this->dx = _dx;
    this->dy = _dy;
    this->cunit = ct;

    if (!clr) {
        this->color = g_colors_human_pals[ct->map_player->color];
    } else {
        this->color = clr;
    }

    this->timestamp = timeGetTime();

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
    this->burst_id = wnd->serverLoopCounter;
}



// 45dd1b (client: 46193b)
void TakeDamage::Update() {
    if (this->drift_left == 0) {
        this->dx += 1;
        this->dy -= 2;
    } else {
        this->dx -= 1;
        this->dy -= 2;
    }
}

// 45dd6d (client: 46198d)
int TakeDamage::Draw() {
    uint32_t t = timeGetTime();
    if (t - this->timestamp > 1000) {
        return 0;
    }

    if (this->cunit->bIsBlocked == 0) {
        g_font2->DrawTextWithShadow(this->cunit->centerScreenX + this->dx, this->cunit->centerScreenY + this->dy - this->cunit->terrainHeightOffset, this->txt, CGameFont::ALIGN_LEFT_TOP, this->color, 1);
    }
    return 1;
}

// 41f620 (client: 4203d0)
int32_t TakeDamage::GetScreenX() {
    return this->dx + this->cunit->centerScreenX;
}

// 41f640 (client: 4203f0)
int32_t TakeDamage::GetScreenY() {
    return this->dy + this->cunit->centerScreenY - this->cunit->terrainHeightOffset;
}


// 45dea1 (client: 461ac1)
void __cdecl TakeDamage::AddDamage(CArray<TakeDamage>* arr, const TakeDamage& dmg) {

    for (int i = 0; i < arr->GetSize(); i++) {
        TakeDamage& b = arr->ElementAt(i);
        if (b.cunit == dmg.cunit && b.burst_id == dmg.burst_id) {
            b.dmg += dmg.dmg;
            b.txt.Format("%d", b.dmg);
            return;
        }
    }

    arr->Add(dmg);
}

// 45ddf6 (client: 461a16)
void __cdecl TakeDamage::DrawDamages(CArray<TakeDamage>* arr) {
    if (arr->GetSize() == 0) {
        return;
    }

    LockSurface2();
    for (int i = 0; i < arr->GetSize(); i++) {
        if (arr->ElementAt(i).Draw() == 0) {
            arr->RemoveAt(i);
            i--;
        }
    }
    UnlockSurface2();
}

// 45de65 (client: 461a85)
void __cdecl TakeDamage::UpdateDamages(CArray<TakeDamage>* arr) {
    for (int i = 0; i < arr->GetSize(); i++) {
        arr->ElementAt(i).Update();
    }
}

#ifdef A2CLIENT_PATCH
// Retail bug fix: drop every label that references a unit which is about to
// be deleted, so Draw() cannot dereference a freed CUnit*.
void __cdecl TakeDamage::RemoveDamagesFor(CArray<TakeDamage>* arr, CUnit* ct) {
    for (int i = arr->GetSize() - 1; i >= 0; i--) {
        if (arr->ElementAt(i).cunit == ct) {
            arr->RemoveAt(i);
        }
    }
}
#endif



