#pragma once

#include "asm_mfc.h"
#include <inttypes.h>


class CSprite256;
class CBmp64;
class CMousePointer;

extern CMousePointer g_mousept; //642c68


class CMousePointer : public CObject
{
	DECLARE_DYNAMIC(CMousePointer);
public:
	virtual ~CMousePointer();

	CMousePointer(); //4264eb

	void DisableHint() { hint_enable = 0; } //438830
	void EnableHint() { hint_enable = 1; } //438810
	CSprite256* GetCursorSprite() { return cursor_sprite; } //438810
	CRect& GetSelectFrame() { return select_frame_rect; } //41ed20
	int32_t GetSelectState() { return select_frame_state; } //41ed00
	int32_t GetX() { return x; } //41ecc0
	int32_t GetY() { return y; } //41ece0

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
