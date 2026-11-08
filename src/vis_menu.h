#ifndef A2_VIS_MENU_H
#define A2_VIS_MENU_H

#pragma once
#include "vis_core.h"

//60a3a8
class QuestObjectivesHeaderDialogVisualObject : public VisMessageBoxWithList {
public:
	QuestObjectivesHeaderDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b);  //445173 (client: 447841)

	virtual void VMethod26() override; //4451e1 (client: 4478af)
};
ASSERT_SIZE(QuestObjectivesHeaderDialogVisualObject, 0x78);

// VTable at 609b88
class VisCredits : public VisScreen {
public:
	virtual ~VisCredits(); // 43c3c6 (client: 43e7d6)

	virtual void VMethod7() override; // 43C60C (client: 43ea1c)
	virtual void VMethod8(CRect* rect) override; // 43c9a8 (client: 43edb8)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 43C595 (client: 43e9a5)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 43C5F5 (client: 43ea05)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 43C5D4 (client: 43e9e4)

	virtual void VMethod26() override; // 43C438 (client: 43e848)
	virtual void VMethod28() override; // 43C4D6 (client: 43e8e6)
	virtual void DoClose(uint32_t code) override; // 43C553 (client: 43e963)

	void LoadContent(); // 43c9b5 (client: 43edc5)
	void FreeContent(); // 43cb77 (client: 43ef87)

	VisCredits(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //43c337 (client: 43e747) in asm
public:
	CMapStringToOb bitmaps;
	TxtFile text;
	uint32_t flag;
	int32_t first_visible;
	int32_t scroll;
};
ASSERT_SIZE(VisCredits, 0xa0);

//60a170
class VisDropGold : public VisScreen {
public:
	virtual ~VisDropGold(); // 441c63 (client: 444232) (44F870 is the ??_G thunk)

	virtual void VMethod9() override; // 441F1F (client: 4444f1)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 441F3C (client: 44450e)

	virtual void VMethod26() override; // 441C7F (client: 44424e)
	virtual void VMethod28() override; // 441FA3 (client: 444575)
	virtual void DoClose(uint32_t code) override; // 441FEE (client: 4445c0)

	void SetSelection(int32_t idx); //4a7a30 (client: 4aba60) in asm

	VisDropGold(int32_t _id, int32_t _x, int32_t _y); // 441bec (client: 4441bb) in asm
public:
	int32_t selection;
	char amount_text[100];
};
ASSERT_SIZE(VisDropGold, 0xd0);

//60cd30
class VisMainMenu : public VisScreen {
public:
	virtual ~VisMainMenu(); // 4AAAB6 (client: 4aeae6)

	virtual void VMethod7() override; // 4AB28F (client: 4af2bf)
	virtual void VMethod8(CRect* rect) override; // 4ab411 (client: 4af441)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4AB41E (client: 4af44e)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4AB88C (client: 4af8bc)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4AB8AF (client: 4af8df)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4AB8F3 (client: 4af923)

	virtual void VMethod26() override; // 4AAB71 (client: 4aeba1)
	virtual void VMethod28() override; // 4ABA8D (client: 4afabd)
	virtual void DoClose(uint32_t code) override; // 4ABB14 (client: 4afb44)

	void UpdateButtonState(uint32_t wparam, CPoint mouse); // 4ab45d (client: 4af48d)
	void LoadGraphics(); // 4aada7 (client: 4aedd7)
	void LoadSfx(); // 4abb5e (client: 4afb8e)
	void FreeGraphics(); // 4ab07b (client: 4af0ab)
	void FreeSfx(); // 4abb87 (client: 4afbb7)

	VisMainMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4aa926 (client: 4ae956) in asm
public:
	CArray<CBmp64*> bmp_button;
	CArray<CBmp64*> bmp_pressed;
	CArray<CBmp64*> bmp_labels;
	CArray<CRect> rect_buttons;
	CArray<CRect> rect_labels;
	CBmp64* bmp_menu;
	CBmp256* bmp_menu_mask;
	SfxSample* snd_btn_click;
	CBmp64* bmp_active_button;
	CRect rect_active_button;
	int32_t over_button;
	int32_t active_button;
	int32_t pressed_button;
	uint32_t disable_mask;
	CA16* sprite;
};
ASSERT_SIZE(VisMainMenu, 0x100);

class VisStartGameTextBox : public CVisualObject {
public:
	VisStartGameTextBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisStartGame* screen); // 4329f9 (client: 434b2a)

	virtual const char* GetHint() override; // 432EA3 (client: 434fdb)
	virtual void VMethod7() override; // 432D58 (client: 434e90)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 432C1F (client: 434d56)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 432C9E (client: 434dd5)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 432CE3 (client: 434e1a)
	virtual int32_t OnChar(uint32_t wparam) override; // 432D1B (client: 434e52)

	CString GetText(); // 438d80 (client: 43add0)
	void SetText(const CString& text); // 432c03 (client: 434d3a)

	void BackspaceChar(); //432b80 (client: 434cb7) in asm
	void AcceptChar(uint8_t ch); //432b33 (client: 434c63) in asm
	void InitTextBox(); //432ad1 (client: 434c02) in asm

public:
	VisStartGame* screen;
	CString text;
	uint16_t* clr;
	uint16_t* clr_active;
	uint16_t* clr_inactive;
	int32_t is_active;
	uint32_t ts;
};
ASSERT_SIZE(VisStartGameTextBox, 0x78);

//609998
class VisStartGame : public VisScreen {
public:
	virtual ~VisStartGame(); // 4386B0 (client: 43ac90)

	virtual const char* GetHint() override; // 437E13 (client: 439f45)
	virtual void VMethod7() override; // 435D72 (client: 437ea4)
	virtual void VMethod8(CRect* rect) override; // 438dc0 (client: 43ac80)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 437571 (client: 4396a3)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 437664 (client: 439796)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 43769C (client: 4397ce)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 437617 (client: 439749)

	virtual void VMethod26() override; // 4333B9 (client: 4354f1)
	virtual void VMethod28() override; // 433E2D (client: 435f65)
	virtual void DoClose(uint32_t code) override; // 4341EB (client: 436323)

	uint32_t GetHotspotId(int32_t x, int32_t y); // 4356c2 (client: 4377fa)
	uint32_t DrawTipsHighlight(uint32_t hotspot); // 436e4b (client: 438f7d)
	uint32_t UpdateHotspots(int32_t x, int32_t y, uint32_t pressed); // 435867 (client: 43799b)
	void UpdateTipsProgress(uint32_t hotspot); // 437c25 (client: 439d57)
	void Accept(); // 43817d (client: 43a2af)
	void Cancel(); // 4382a9 (client: 43a5a4)

	void PlayLabelInputSound(); //4382d9 (client: 43a5d4) in asm

	void ClearDifficultySelectedFlags(); // 435743 (client: 43787b)
	void ClearPortraitSelectedFlags(); // 43578c (client: 4378c3)
	void ClearDifficultyHoverFlags(); // 4357d5 (client: 43790b)
	void ClearPortraitHoverFlags(); // 43581e (client: 437953)

	void LoadBitmaps(); // 43438b (client: 4364c3)
	void LoadSamples(); // 437ff4 (client: 43a126)
	void FreeSamples(); // 4380d3 (client: 43a205)
	void ResetHoverState(); // 435d26 (client: 437e58)
	void FreeBitmaps(); // 435097 (client: 4371cf)

	VisStartGame(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //43305b (client: 435193) in asm

	CString GetName() { return this->char_name; } // 4973f0 (client: 43add0)
	CString GetClan() { return this->clan_name; } // 497430 (client: 49aa50)
	int32_t GetSelectedDifficulty() { return this->selectedDifficultyIndex; } //4973d0 (client: 49a9f0)
	void SetSelectedDifficulty(int diff) { this->selectedDifficultyIndex = diff; } //497370 (client: 49ab80)
	int32_t GetSelectedPortraitPlayerType() { return PortraitPlayerTypes[this->selectedPortraitIndex]; } //4973b0 (client: 49a9d0)
	void SetSelectedPortraitPlayerType(int id) { this->selectedPortraitIndex = id; } //497350 (client: 49ab60)
	void SetCharName(const CString& name) { this->char_name = name; } //497390 (client: 434d3a)

public:
	CBmp64* mainAreaBitmap; //0x68
	CBmp256* hotspotMaskBitmap; // 0x6c;
	CA16* blindAnimation; // 0x70;
	int32_t blindAnimationFrame; // 0x74;
	CPoint blindAnimationPosition; // 0x78;
	CArray<CRect> blindSpawnRects; // 0x80;
	CArray<CBmp64*> portraitSelectedBitmaps; // 0x94;
	CArray<CBmp64*> portraitHoverBitmaps; // 0xa8;
	CArray<CBmp64*> portraitSelectedHoverBitmaps; // 0xbc;
	CArray<CBmp64*> difficultySelectedBitmaps; // 0xd0;
	CArray<CBmp64*> difficultyHoverBitmaps; // 0xe4;
	CArray<CBmp64*> difficultySelectedHoverBitmaps; // 0xf8;
	CArray<CRect> portraitRects; // 0x10c;
	CArray<CRect> difficultyRects; // 0x120;
	CUIntArray portraitStateFlags; // 0x134;
	CUIntArray difficultyStateFlags; // 0x148;
	CBmp64* returnToGameButtonBitmap; // 0x15c;
	CBmp64* acceptButtonBitmap; // 0x160;
	CBmp64* tableauBitmap; // 0x164;
	CArray<CBmp64*> leftTorchFrames; // 0x168;
	CArray<CBmp64*> rightTorchFrames; // 0x17c;
	CRect returnToGameButtonRect; // 0x190;
	CRect acceptButtonRect; // 0x1a0;
	int32_t torchFrameTick; // 0x1b0;
	CBmp64* returnToGameHoverBitmap; // 0x1b4;
	CBmp64* acceptHoverBitmap; // 0x1b8;
	int32_t gap_0x1bc;
	int32_t gap_0x1c0;
	CSound difficultyLevel1Sound; // 0x1c4;
	CSound difficultyLevel2Sound; // 0x1c8;
	CSound difficultyLevel3Sound; // 0x1cc;
	CSound portraitSelectSound; // 0x1d0;
	CSound acceptSound; // 0x1d4;
	CSound returnSound; // 0x1d8;
	CSound labelInputSound1; // 0x1dc;
	CSound labelInputSound2; // 0x1e0;
	CSound labelInputSound3; // 0x1e4;
	int32_t labelInputSoundIndex;
	VisStartGameTextBox* networkNameLabel; // 0x1ec;
	VisStartGameTextBox* nameLabel; // 0x1f0;
	VisStartGameTextBox* clanLabel; // 0x1f4;
	VisScreen* tipsPrompt; // 0x1f8;
	CString char_name; // 0x1fc
	CString clan_name; // 0x200
	int32_t gap_0x204;
	int32_t selectedPortraitIndex; // 0x208;
	int32_t selectedDifficultyIndex; // 0x20c;
	int32_t committedDifficultyIndex; // 0x210;
	int32_t committedPortraitIndex; // 0x214;
	int32_t tipsProgress; // 0x218;
	int32_t dialogActiveFlag; // 0x21c;

	static const int32_t PortraitPlayerTypes[4];
};
ASSERT_SIZE(VisStartGame, 0x220);

//609f50
class IngameMenu : public VisMenuWnd {
public:
	IngameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, uint32_t unk, const CRect& _r); //451990 (client: 453430)
};
ASSERT_SIZE(IngameMenu, 0x7c);

//609dc8
class LoadGameWindow : public VisWindow {
public:
	virtual ~LoadGameWindow() {} //44f4b0 (client: 452fd0)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //43f63c (client: 441aa0)
	virtual void VMethod26() override; //43ed94 (client: 4411f8)

	LoadGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk); //43ecfb (client: 44115f)
public:
	SaveFileInfo* out_info;
	CStringArray file_names;
	CStringArray save_times;
	int32_t selected_index;
};
ASSERT_SIZE(LoadGameWindow, 0x98);

//609e50
class SaveGameWindow : public VisWindow {
public:
	virtual ~SaveGameWindow() {} //44f540 (client: 452fd0)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //440726 (client: 442b8d)
	virtual void VMethod26() override; //43fb2c (client: 441f90)

	SaveGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk); //43fa42 (client: 441ea6)
	void CheckInput(); //440cc4 (client: 44312b)

	static int __cdecl Compare(void const* a, void const* b); //43faf4 (client: 441f58)
public:
	SaveFileInfo* out_info;
	CStringArray file_names;
	CStringArray save_times;
	int32_t selected_index;
	char* empty_cell_text;
};
ASSERT_SIZE(SaveGameWindow, 0x9c);

//609ed8
class VisButtonSaveGame : public VisButton {
public:
	virtual ~VisButtonSaveGame(); //44f650 (client: 453310)

	virtual int32_t OnKeyDown(uint32_t wparam) override; //44f5f0 (client: 4532b0)

	VisButtonSaveGame(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint); //44f5a0 (client: 452b50)
};
ASSERT_SIZE(VisButtonSaveGame, 0x78);

//60a200
class GameOptionsWindow : public VisWindow {
public:
	virtual ~GameOptionsWindow() {} //443cbb (client: 406f89)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //443cd7 (client: 4462a9)
	virtual void VMethod26() override; //442646 (client: 444c18)

	GameOptionsWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b) : VisWindow(_id, l, t, r, b, nullptr) {}; //44260f (client: 444be1)
};
ASSERT_SIZE(GameOptionsWindow, 0x68);

//609fd8
class EndGameMenu : public VisMenuWnd {
public:
	EndGameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitm, uint32_t unk, const CRect& _r, int32_t unk2); //441357 (client: 453430)
};
ASSERT_SIZE(EndGameMenu, 0x7c);

//60a0e8
class ExitGameMenu : public VisMenuWnd {
public:
	ExitGameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CRect& _r); //44186e (client: 443e3d)
};
ASSERT_SIZE(ExitGameMenu, 0x7c);

//609cb0
class SoundPreferencesDialogVisualObject : public VisWindow {
public:
	SoundPreferencesDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SoundSettings* pset); //43d917 (client: 43fd7b)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //43e6f0 (client: 440b54)
	virtual void VMethod26() override; //43d959 (client: 43fdbd)

	int32_t ConvVolumeToSlider(int32_t vol, int32_t max_val) { return max_val - (sqrt((float)-vol / (float)max_val) * (float)max_val); } //45ca10 (client: 46063e)
	int32_t ConvSliderToVolume(int32_t pos, int32_t max_val) { //45ca45 (client: 460673)
		float v = (float)(pos - max_val) / (float)max_val;
		return max_val * -(v * v);
	}
public:
	MusicPlayer* music_player;
	int32_t track_index;
	SoundSettings* settings;
};
ASSERT_SIZE(SoundPreferencesDialogVisualObject, 0x74);

//60db28
class VisLogoWnd : public VisScreen {
public:
	VisLogoWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4cd4d0 (client: 4d1e20)
	~VisLogoWnd(); //4cd53c (client: 4d1e8c)

	virtual void VMethod7() override; //4cd8d1 (client: 4d2221)
	virtual void VMethod8(CRect* rect) override {}; //4cd91e (client: 4d226e)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //4cd72e (client: 4d207e)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; //4cd6c2 (client: 4d2012)
	virtual int32_t OnKeyDown(uint32_t wparam) override; //4cd6f8 (client: 4d2048)

	virtual void VMethod26(); //4cd5be (client: 4d1f0e)
	virtual void DoClose(uint32_t code); //4cd666 (client: 4d1fb6) 29 method   

public:
	CBmp256* logoBitmap;
	int32_t step;
	uint32_t timeoutStart;
	uint32_t timeoutTicks;
	uint32_t drawPendingFlag;
};
ASSERT_SIZE(VisLogoWnd, 0x7c);

//60b098
class Vis2Action : public VisMenuWnd {
public:
	virtual ~Vis2Action(); // 450bd0 (client: 4548d0)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44F30D (client: 452632)
	virtual void VMethod26() override; // 44F0E8 (client: 45240d)

	Vis2Action(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _txt, const CRect& _r); //44f0a2 (client: 4523c7)

public:
	const char* txt;
};
ASSERT_SIZE(Vis2Action, 0x80);

//60af78
class VisMissionFailed : public VisMessageBoxWithList {
public:
	virtual ~VisMissionFailed(); // 450B00 (client: 454800)
	virtual void VMethod26() override; // 44E8CA (client: 451bef)

	VisMissionFailed(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CString& _txt); //44e875 (client: 451b9a)
};
ASSERT_SIZE(VisMissionFailed, 0x78);

//60b008
class VisCutScenesDlg : public VisMessageBox {
public:
	virtual ~VisCutScenesDlg(); // 450B50 (client: 454850)

	VisCutScenesDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CStringArray& _payload); //44eabb (client: 451de0)


	virtual CVisualObject* VMethod30(const void* data, const RECT& r) override; // 44EB08 (client: 451e2d)
};
ASSERT_SIZE(VisCutScenesDlg, 0x78);

//60e140
class VisListBoxDiplomacy : public VisListBox {
public:
	virtual ~VisListBoxDiplomacy(); // 4dceba (client: 4e183d)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4dd7f3 (client: 4e2176)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4dd44a (client: 4e1dcd)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4dd8c5 (client: 4e2248)
	virtual int32_t IsValidIndex(int32_t idx) override; // 4dd098 (client: 4e1a1b)
	virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 4dd0bd (client: 4e1a40)
	virtual void SelectItem(int32_t idx) override; // 4dcf98 (client: 4e191b)
	virtual void VMethod30(CPoint pos, const CRect& r) override; // 4e3d30 (client: 4e9cc0)

	VisListBoxDiplomacy(int32_t _id, const RECT& r, CArray<DiplomacyEntry*>* _diplomacy, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _entry_height); //4dc9fb (client: 4e137e)

	void WriteRadioState(); //4dd1b2 (client: 4e1b35)
	void UpdateScrollBar(); //4dd2c7 (client: 4e1c4a)
	void RestoreRect(); //4dd424 (client: 4e1da7)
	void UpdateRadioPositions(int32_t vis_start_index); //4dd54c (client: 4e1ecf)

public:
	CArray<DiplomacyEntry*>* diplomacy; //0x94
	CArray<VisRadioType1*>* enemy_radios; //0x98
	CArray<VisRadioType1*>* ally_radios; //0x9c
	CArray<VisRadioType1*>* see_radios; //0xa0
	CArray<VisRadioType1*>* mute_radios; //0xa4
};
ASSERT_SIZE(VisListBoxDiplomacy, 0xa8);

//60a4c8
class VisDiplomacy : public VisMessageBox {
public:
	virtual ~VisDiplomacy(); // 44F9E0 (client: 4538d0)

	VisDiplomacy(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CArray<DiplomacyEntry*>* _payload); //445bcb (client: 44829c)

	virtual void ReadData(const void* buf) override; // 445D34 (client: 448405)
	virtual CVisualObject* VMethod30(const void* data, const RECT& r) override; // 445C2E (client: 4482ff)
	virtual void VMethod31(int32_t code) override; // 445CF1 (client: 4483c2)
public:
	// Points at MainWindow's CArray<DiplomacyEntry*>* variable (the binary puns it
	// through a const CArray* ctor parameter), so ReadData re-reads the current array.
	CArray<DiplomacyEntry*>** diplomacy; //0x78
};
ASSERT_SIZE(VisDiplomacy, 0x7c);

//60e650
class VisTipsDialog : public VisScreen {
public:
	virtual ~VisTipsDialog(); // 4e4510 (client: 4e98b0)

	virtual void VMethod8(CRect* rect) override; // 4E27CD (client: 4e7151)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4E2B95 (client: 4e7519)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4E2C42 (client: 4e75c6)
	virtual int32_t OnKeyDown(uint32_t wparam) override { return 0; } // 4e2c33 (client: 4e75b7)

	VisTipsDialog(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* txt); //4e2541 (client: 4e6ec5)

	void InitTipsDialog(int32_t l, int32_t t, int32_t r, int32_t b, const char* txt); // 4e25c1 (client: 4e6f45)
	void SetText(const char* text); // 4e2b73 (client: 4e74f7)

public:
	uint8_t __unused__[12];
};
ASSERT_SIZE(VisTipsDialog, 0x74);

//60e6d8
class VisQuestStatus : public VisScreen {
public:
	virtual ~VisQuestStatus(); // 4E4540 (client: 4e98d0)

	virtual void VMethod7() override; // 4E2F76 (client: 4e78fa)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4e4530 (client: 4e9cd0)

	VisQuestStatus(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4e2e3d (client: 4e77c1)

public:
	CSprite256* icon;
};
ASSERT_SIZE(VisQuestStatus, 0x6c);

#endif
