#pragma once

#include "asm_mfc.h"
#include <inttypes.h>


class CSprite256;
class CBmp64;
class CMousePointer;
class CCursor;

extern CMousePointer g_mousept; //642c68
extern CCursor* g_Cursors[28]; //665420

enum {
	CURSOR_DEFAULT = 0,
	CURSOR_MOVE = 1,
	CURSOR_SWARM = 2,
	CURSOR_ATTACK = 3,
	CURSOR_DEFEND = 4,
	CURSOR_SELECT = 5,
	CURSOR_PATROL = 6,
	CURSOR_CAST = 7,
	CURSOR_PICKUP = 8,
	CURSOR_ARROW0 = 9,
	CURSOR_ARROW4 = 10,
	CURSOR_ARROW6 = 11,
	CURSOR_ARROW2 = 12,
	CURSOR_ARROW7 = 13,
	CURSOR_ARROW5 = 14,
	CURSOR_ARROW1 = 15,
	CURSOR_ARROW3 = 16,
	CURSOR_SDEFAULT = 17,
	CURSOR_SMOVE = 18,
	CURSOR_SATTACK = 19,
	CURSOR_SDEFEND = 20,
	CURSOR_SPATROL = 21,
	CURSOR_SCAST = 22,
	CURSOR_CANTPUT = 23,
	CURSOR_TOWN = 24,
	CURSOR_DICE = 25,
	CURSOR_WAIT = 26,
	CURSOR_BACKPACK = 27
};


class CMousePointer : public CObject
{
	DECLARE_DYNAMIC(CMousePointer);
public:
	virtual ~CMousePointer();

	CMousePointer(); //4264eb

	void DisableHint(); //438830
	void EnableHint(); //438810
	CSprite256* GetCursorSprite(); //438810
	CRect& GetSelectFrame(); //41ed20
	int32_t GetSelectState(); //41ed00
	int32_t GetX(); //41ecc0
	int32_t GetY(); //41ece0

	void CheckHintValues(); //42709f
	void DrawToScreen(); //426ce2
	void EraseCursor(); //426df7
	void ForceDisableHint(); //4287d9
	void Paint(); //426ca3
	void PaintHint(); //4282bb
	void PaintSelectFrame(); //427e71
	void Repaint(CPoint point); //426eb6
	void RepaintHint(); //428779
	void ResetNext400msg(); //42707e
	void ResetStates(); //427e23
	void SaveScreenParts(const CRect& rect); //427912
	void SaveScreenParts2(const CRect& rect); //42751c
	void SetCursor(CSprite256* spr, int32_t off_x, int32_t off_y, int32_t count, int32_t delay); //426aa5
	void StartSelect(const CPoint& point); //427db1
	void Unpaint(); //426c64
	void UnpaintSelectFrame(); //428051
	void Update(); //4271e6
	void UpdateHint(); //42819d

public:
	CSprite256* cursor_sprite;
	CBmp64* saved_screen_rect;
	CBmp64* saved_screen_rect2;
	int32_t x;
	int32_t y;
	int32_t x_offset;
	int32_t y_offset;
	int32_t paint_call_count;
	int32_t frame;
	int32_t frame_count;
	uint32_t frame_delay;
	uint32_t frame_ts;
	uint32_t user_msg_ts;
	uint32_t user_msg_delay;
	CString  hint_text;
	uint32_t hint_lines;
	uint32_t hint_max_width;
	CRect	 hint_rect;
	int32_t mouse_hint_time;
	uint32_t mouse_hint_ts;
	uint32_t hint_state;
	CBmp64* hint_image;
	CBmp64* saved_screen_rect_hint;
	CRect    select_frame_rect;
	CBmp64* saved_rect_frame[8];
	uint32_t select_frame_state;
	uint32_t hint_enable;
};

ASSERT_SIZE(CMousePointer, 0xa4);


class CCursor : public CObject
{
public:
	virtual ~CCursor(); //47cf99
	CCursor() = default; //47ce1f
	CCursor(const char* fname, int32_t xof, int32_t yof, int32_t delay); //47ce66

	void Use(); //47d025

	CSprite256* GetSprite() const { return sprite; }; //41f8d0
public:
	CSprite256* sprite = nullptr;
	int32_t x_off = 0;
	int32_t y_off = 0;
	int32_t frames_count = 0;
	int32_t delay = 2000000000;
};
ASSERT_SIZE(CCursor, 0x18);


void LoadCursors(); //47d05d
void DeleteCursors(); //47daf0
int __cdecl ApplyCursor(CCursor* cur); //4750d0
