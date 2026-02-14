#include "main_window.h"
#include "asm_mfc.h"
#include <stdio.h>


//495033
CListBox2::CListBox2() = default;

//495052
CListBox2::~CListBox2() = default;

const AFX_MSGMAP* CListBox2::GetMessageMap() const
{
	//49506e
	return &CListBox2::messageMap;
}

//60bfa8
AFX_DATADEF const AFX_MSGMAP CListBox2::messageMap =
{ &CListBox::messageMap, &CListBox2::_messageEntries[0] };

const AFX_MSGMAP_ENTRY CListBox2::_messageEntries[] =
{
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	{0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 }
};

void CListBox2::OnKeyDown(UINT, UINT, UINT)
{
	printf("CListBox2 OnKeyDown\n");
	Default();
}

void CListBox2::OnLButtonDblClk(UINT, CPoint)
{
	printf("CListBox2 OnLButtonDblClk\n");
	Default();
}