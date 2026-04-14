#include "visual.h"
#include "gameobj.h"
#include "ingame.h"
#include "net.h"
#include "packet.h"
#include "main_window.h"
#include "txtfile.h"
#include "gfx.h"


uint16_t* clr_log_tblack = clrsh_TechBlack; //62f890



GM_a28::~GM_a28() = default; //4020ad

GM_a28::GM_a28()
{
	//401f50
	elapsed_time = 0;
	field_0x5c = 1;
}

void GM_a28::Add(const char* msg, uint16_t* clr, uint32_t t)
{
	//4021fe
	field_0x5c = 1;

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	
	CGameFont* fnt = g_font1;
	if (wnd->field_0x640 == 3)
		fnt = g_font2;

	bool restart = text.GetSize() == 0;

	CStringArray& lines = fnt->StringArrayForRect(out_area, msg);
	for (int i = 0; i < lines.GetSize(); i++)
	{
		CString& s = lines[i];
		for (int j = 0; j < s.GetLength(); j++)
		{
			if (s[j] == '\r')
				s.SetAt(j, ' ');
		}

		text.Add(s);
		color.Add(clr);
		times.Add(t);
	}

	if (restart)
	{
		timestamp = timeGetTime();
		elapsed_time = 0;
	}

	while (text.GetSize() > max_size)
	{
		text.RemoveAt(0);
		color.RemoveAt(0);
		times.RemoveAt(0);
	}
}

void GM_a28::AddWithCheck(const char* msg, uint16_t* clr, uint32_t t)
{
	//4023aa
	field_0x5c = 1;

	CStringArray& lines = g_font1->StringArrayForRect(out_area, msg);
	for (int i = 0; i < lines.GetSize(); i++)
	{
		CString& s = lines[i];
		for (int j = 0; j < s.GetLength(); j++)
		{
			if (s[j] == '\r')
				s.SetAt(j, ' ');
		}

		if (text[text.GetUpperBound()] == s)
			return;

		text.Add(s);
		color.Add(clr);
		times.Add(t);
	}

	if (text.GetSize() == 1)
	{
		timestamp = timeGetTime();
		elapsed_time = 0;
	}

	while (text.GetSize() > max_size)
	{
		text.RemoveAt(0);
		color.RemoveAt(0);
		times.RemoveAt(0);
	}
}

void GM_a28::Clear()
{
	//4021d2
	text.RemoveAll();
	color.RemoveAll();
	times.RemoveAll();
}

void GM_a28::Draw()
{
	//40261a

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	int32_t out_x = 8;
	int32_t out_y = 8;
	CGameFont* fnt = g_font1;

	if (wnd->field_0x640 == 3)
	{
		out_x = 0;
		out_y = 220;
		fnt = g_font2;
	}

	for (int i = 0; i < text.GetSize(); i++)
	{
		CString s = text[i];

		while (true)
		{
			int rpos = s.Find('\r');

			if (rpos < 0)
			{
				fnt->DrawTextWithShadow(out_x, out_y, s, 0, (uint16_t*)color[i], 1);
				out_y += 2 + fnt->GetHeight();
				break;
			}
			
			fnt->DrawTextWithShadow(out_x, out_y, s.Left(rpos), 0, (uint16_t*)color[i], 1);
			out_y += 2 + fnt->GetHeight();
			s = s.Mid(rpos + 1);
		}
	}
}

void GM_a28::SetRect(const CRect& r)
{
	//40212b
	out_area = r;

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	if (wnd->field_0x640 == 3)
		max_size = (g_ScreenSize.bottom - 480) / (g_font2->GetHeight() + 2) + 14;
	else
		max_size = (out_area.Height() / (g_font1->GetHeight() + 2)) / 2;
}

void GM_a28::Update()
{
	//402585
	if (text.GetSize() == 0)
		return;

	uint32_t ts = timeGetTime();

	elapsed_time += (ts - timestamp);
	timestamp = ts;

	if (times[0] < elapsed_time)
	{
		elapsed_time = 0;

		text.RemoveAt(0);
		color.RemoveAt(0);
		times.RemoveAt(0);
	}
}



int32_t BigStruct2::ProcessPackets(uint8_t breakid)
{
	//40da14
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	bool loop = true;
	while (loop)
	{
		Packet* pkt = g_NetStru1_local.ReceiveAnyPacket();

		Packet3Dwords* packet_3d = reinterpret_cast<Packet3Dwords*>(pkt);
		PacketJoin* packet_join = reinterpret_cast<PacketJoin*>(pkt);
		PacketCmd* packet_cmd = reinterpret_cast<PacketCmd*>(pkt);
		PacketInfo* packet_info = reinterpret_cast<PacketInfo*>(pkt);
		PacketDword* packet_dword = reinterpret_cast<PacketDword*>(pkt);
		PacketData* packet_data = reinterpret_cast<PacketData*>(pkt);
		PacketWord* packet_word = reinterpret_cast<PacketWord*>(pkt);

		if (!pkt)
		{
			if (breakid != 0 || (wnd->field_0x418 & 1) != 0)
				continue;
			return 0;
		}

		switch (pkt->id)
		{
		case 3:
			break;

		case 6:
			if (!field_0x80)
				FUN_0041c4a1(packet_join->name);

			if (!field_0x80)
			{
				INT_00660f8c = 0x1004;
				return 0;
			}
			INT_00660f90 = 0;
			break;

		case 0xb:
		{
			INT_00660f8c = packet_info->field_0xa | 0x1000;

			if ((wnd->field_0x418 & 1) == 0)
			{
				while( g_NetStru1_local.ReceiveAnyPacket() != nullptr)
				{}
				g_CLlDriver.Close();
				return 0;
			}

			wnd->PostMessage(0x45c, 0, 0);
			g_CLlDriver.Close();
			break;
		}

		case 0xe:
		{
			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{}
			return 1;
		}

		case 0x64:
			wnd->field_0x41c = packet_dword->value;
			break;

		case 0x67:
		{
			if (wnd->field_0x418 == 1 && my_main_unit->gold < packet_info->field_0xa && packet_info->field_0xe == 0)
			{
				char buf[128];
				sprintf(buf, "%s %d %s", TxtFile::AllLines.GetAt(0x58), packet_info->field_0xa - my_main_unit->gold, TxtFile::AllLines.GetAt(0x59));
				msglog.Add(buf, clr_log_tblack, 3000);
			}
			my_main_unit->gold = packet_info->field_0xa;

			CUnit* u = GetUnit_3f6c();
			if (u)
			{
				wnd->some_struc.FUN_004946e0();
				if (u->field_0xd0.GetSize())
				{
					GO_d0* gitem = u->field_0xd0[u->field_0xd0.GetUpperBound()];
					if (gitem->FUN_0041f0d0())
						gitem->field_0x10 = my_main_unit->gold;
				}
			}
		}
			break;

		case 0x69:
		case 0x74:
		{
			CUnit* unit;
			if (field_0x9d0.Lookup(packet_word->value, unit))
			{
				if (pkt->id == 0x74)
					unit->field_0x1b8 |= 0x80;
				else
					unit->field_0x1b8 &= ~0x80;
			}
		}
			break;
			

		}

		if (pkt->id == breakid)
			return 1;
	}
	return 1;
}