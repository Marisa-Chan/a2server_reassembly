#ifndef VISUALOBJ_H
#define VISUALOBJ_H

#include "assert_offset.h"
#include "asm_mfc.h"
#include "mfc_templ.h"


class VisLabel;
class CGameFont;

class CVisualObject : public CObject
{
public:
	~CVisualObject();
	virtual void Dump(CDumpContext& dc) const override;

	virtual const char* GetName();
	virtual void SetName(const char *_name);
	virtual void ChangeFlags(uint32_t _flags, bool setunset);
	virtual uint32_t TestFlags(uint32_t _flags);
	virtual void SetCursorOver(bool isOver);
	virtual void SetFocus(bool isFocus);
	virtual void VMethod7();
	virtual void VMethod8(RECT* rect);
	virtual void VMethod9();
	virtual void VMethod10();
	virtual void WriteData(void* buf);
	virtual uint32_t DataSize();
	virtual void ReadData(void* buf);
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam);
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos);
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos);
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos);
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos);
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos);
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos);
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos);
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos);
	virtual int32_t OnKeyDown(uint32_t wparam);
	virtual int32_t OnKeyUp(uint32_t wparam);
	virtual int32_t OnChar(uint32_t wparam);

public:
	CVisualObject();
	CVisualObject(int32_t _id, const RECT& r, const char* str);
	CVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str);

	int32_t MsgProcOnChilds(uint32_t msg, uint32_t wparam, uint32_t lparam);

	void AddChild(CVisualObject* obj);

	void RemoveChild(CVisualObject* obj);
	void RemoveChildById(CVisualObject* obj);
	void RemoveAllChilds();

	void DestroyChild(CVisualObject* obj);
	void DestroyChildById(CVisualObject* obj);
	void DestroyAllChilds();

	CVisualObject* FindChild(int32_t _id);
	CVisualObject* GetChildAt(POINT pt);
	
	void ClientPtToScreen(CPoint* out, const CPoint& _point);
	void ClientRectToScreen(CRect* out, const CRect& _rect);

	void SetLeftObj(CVisualObject* obj);
	void SetRightObj(CVisualObject* obj);
	void SetUpObj(CVisualObject* obj);
	void SetDownObj(CVisualObject* obj);

	void SetCaptionLabel(VisLabel* obj);

	void FocusTo(CVisualObject* obj, bool update);
	void TabFocus(bool forward, bool update);

	void SetRect(RECT r);
	void SetRect(const RECT* r);

public:
	int32_t id;
	CRect rect;
	uint32_t flags;
	CArray<CVisualObject*> childs;
	CVisualObject* parent;
	CVisualObject* cursor_over_obj;
	CVisualObject* focus_obj;
	CString name;
	CVisualObject* cursor_over_obj_last;
	CVisualObject* last_focus_obj;
	CVisualObject* up_obj;
	CVisualObject* down_obj;
	CVisualObject* left_obj;
	CVisualObject* right_obj;
	VisLabel* caption_obj;
};

ASSERT_SIZE(CVisualObject, 0x5C);



class VisLabel : public CVisualObject
{
public:
	~VisLabel();

	virtual void VMethod7() override;

public:
	VisLabel();
	VisLabel(int32_t _id, const RECT& r, const char* text, CGameFont* font, uint16_t* colorsh, uint32_t align);
	VisLabel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* text, CGameFont *font, uint16_t *colorsh, uint32_t align);

	void SetActiveColor(bool isActive);
public:

	CString text;
	CGameFont* font;
	uint16_t* color_sh;
	uint32_t align_flags;
};

ASSERT_SIZE(VisLabel, 0x6C);


#endif
