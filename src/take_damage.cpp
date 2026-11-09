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

TakeDamage::TakeDamage() = default; //45db60
TakeDamage::TakeDamage(const TakeDamage& ref) = default;
TakeDamage::~TakeDamage() = default; //45dd08

TakeDamage::TakeDamage(int32_t _dmg, uint16_t* clr, int32_t unk, int32_t _dx, int32_t _dy, CUnit* ct) {
	//45db8a
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



void TakeDamage::Update() {
	//45dd1b
	if (this->drift_left == 0) {
		this->dx += 1;
		this->dy -= 2;
	} else {
		this->dx -= 1;
		this->dy -= 2;
	}
}

int TakeDamage::Draw() {
	//45dd6d
	uint32_t t = timeGetTime();
	if (t - this->timestamp > 1000) {
		return 0;
	}

	if (this->cunit->bIsBlocked == 0) {
		g_font2->DrawTextWithShadow(this->cunit->centerScreenX + this->dx, this->cunit->centerScreenY + this->dy - this->cunit->terrainHeightOffset, this->txt, 0, this->color, 1);
	}
	return 1;
}

int32_t TakeDamage::GetScreenX() { //41f620
    return this->dx + this->cunit->centerScreenX;
}

int32_t TakeDamage::GetScreenY() { //41f640
    return this->dy + this->cunit->centerScreenY - this->cunit->terrainHeightOffset;
}


void __cdecl TakeDamage::AddDamage(CArray<TakeDamage>* arr, const TakeDamage& dmg) {
	//45dea1

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

void __cdecl TakeDamage::DrawDamages(CArray<TakeDamage>* arr) {
	//45ddf6
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

void __cdecl TakeDamage::UpdateDamages(CArray<TakeDamage>* arr) {
	//45de65
	for (int i = 0; i < arr->GetSize(); i++) {
		arr->ElementAt(i).Update();
	}
}



