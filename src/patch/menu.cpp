// Tangar custom main menu, ported from my-a2mgr menu.cpp. The patched menu
// replaces the retail eight-button menu with a menunew.png background and
// three custom buttons (Play / Authors / Quit) drawn over the inn button
// art, dispatching through the retail button codes 5 / 3 / 7.

#include "menu.h"

#include "../vis_menu.h"
#include "../gfx.h"
#include "../sound.h"
#include "client_config.h"
#include "image.h"

namespace tangar_menu {

static PatchImage* img_menu_new = nullptr;
static PatchImage* img_buttons_up[3] = { nullptr, nullptr, nullptr };
static PatchImage* img_buttons_down[3] = { nullptr, nullptr, nullptr };

static int32_t buttonpos[] = { 251, 258,
                               251, 304,
                               251, 351 };

static int32_t buttonhover = -1;
static int32_t buttonclick = -1;

// Button labels: patched string-table lines 238-240 (locale/patch.txt,
// 0-based; "Play", "Authors", "Quit"). The RU table is CP866, which is what
// the retail RU font expects, so the RU literals are CP866 byte strings.
static const char* ButtonLabel(int32_t i) {
	static const char* labels_en[3] = { "Play", "Authors", "Quit" };
	static const char* labels_ru[3] = {
		"\x88\xA3\xE0\xA0\xE2\xEC", // Играть
		"\x80\xA2\xE2\xAE\xE0\xEB", // Авторы
		"\x82\xEB\xE5\xAE\xE4",     // Выход
	};
	return tangar_cfg::z_russian ? labels_ru[i] : labels_en[i];
}

void Load() {
	if (img_menu_new == nullptr) {
		char path[MAX_PATH];
		sprintf_s(path, "data\\locale\\%s\\graphics\\mainmenu\\menunew.png", tangar_cfg::z_russian ? "ru" : "en");
		img_menu_new = new PatchImage(path);
	}
	for (int32_t i = 0; i < 3; i++) {
		if (img_buttons_up[i] == nullptr) {
			char path[MAX_PATH];
			sprintf_s(path, "data\\graphics\\interface\\inn\\button%doff.bmp", i + 1);
			img_buttons_up[i] = new PatchImage(path);
		}
		if (img_buttons_down[i] == nullptr) {
			char path[MAX_PATH];
			sprintf_s(path, "data\\graphics\\interface\\inn\\button%don.bmp", i + 1);
			img_buttons_down[i] = new PatchImage(path);
		}
	}
}

void Draw() {
	LockSurface2();

	const CRect& pRect = g_ScreenSize;

	int32_t xMin = pRect.right / 2 - 320;
	int32_t yMin = pRect.bottom / 2 - 240;
	img_menu_new->Display((int16_t)xMin, (int16_t)yMin);

	// display buttons
	for (int32_t i = 0; i < 3; i++) {
		int32_t bx = xMin + buttonpos[i * 2];
		int32_t by = yMin + buttonpos[i * 2 + 1];

		// draw off for now
		if (buttonclick == i) {
			img_buttons_down[i]->Display((int16_t)bx, (int16_t)by);
		} else {
			img_buttons_up[i]->Display((int16_t)bx, (int16_t)by);
		}

		// display text
		uint16_t* col = palette_husk->GetPalette(0); // FONT_EXTCOLOR_GOLD

		int32_t tx = bx + 70;
		int32_t ty = by + 16;

		if (buttonclick == i) {
			ty += 1;
			col = palette_paris_daisy->GetPalette(0); // FONT_EXTCOLOR_GOLDBRIGHT
		} else if (buttonhover == i) {
			col = palette_paris_daisy->GetPalette(0);
		}

		g_font4->DrawTextWithShadow(tx, ty, ButtonLabel(i), 2 /* FONT_ALIGN_CENTER */, col, 1);
	}

	UnlockSurface2();
}

void Hover(VisMainMenu* menu, int32_t x, int32_t y, uint32_t buttons) {
	const CRect& pRect = g_ScreenSize;

	int32_t xMin = pRect.right / 2 - 320;
	int32_t yMin = pRect.bottom / 2 - 240;

	// for whatever reason, this also gets called on click (null hover)
	buttonhover = -1;
	if (buttons == 0) {
		buttonclick = -1;
	}
	for (int32_t i = 0; i < 3; i++) {
		int32_t bx = xMin + buttonpos[i * 2];
		int32_t by = yMin + buttonpos[i * 2 + 1];
		int32_t bw = 140;
		int32_t bh = 46;

		// buttons is 1 or 2
		if (x >= bx && x < bx + bw && y >= by && y < by + bh) {
			buttonhover = i;
			if (buttonclick < 0 && buttons == 1) {
				buttonclick = i;
				// play menu sound
				CSound::Play((CSound&)menu->snd_btn_click);
			}
		}
	}

	// Map the custom buttons onto the retail main-menu selection codes
	// (OnLButtonUp dispatches these): Play -> 5 (HAT server list),
	// Authors -> 3 (credits), Quit -> 7 (WM_CLOSE).
	if (buttonclick == buttonhover) {
		switch (buttonclick) {
		case 0:
			menu->active_button = 5;
			break;
		case 1:
			menu->active_button = 3;
			break;
		case 2:
			menu->active_button = 7;
			break;
		default:
			menu->active_button = -1;
			break;
		}
	} else {
		menu->active_button = -1;
	}
}

} // namespace tangar_menu
