#include "visual.h"
#include "gameobj.h"
#include "ingame.h"
#include "net.h"
#include "packet.h"
#include "main_window.h"
#include "txtfile.h"
#include "gfx.h"
#include "game_app.h"
#include "unit.h"


uint16_t* clr_log_tblack = clrsh_TechBlack; //62f890
uint16_t* clr_log_sblack = clrsh_ShockingBlack; //62f88c


int INT_6362e8[16] = { 0, 0, 1, 0,   1, 0, 1, 0,   0, 0, -1, 0,   -1, 0, -1, 0 };
int INT_636328[16] = { -1, 0, -1, 0,   0, 0, 1, 0,   1, 0, 1, 0,   0, 0, -1, 0 };

extern CUnit g_CUnitStatic; //642a48




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
	char buf[128];
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
		PacketAbility* packet_abil = reinterpret_cast<PacketAbility*>(pkt);
		PacketUnitUpdate* packet_unit = reinterpret_cast<PacketUnitUpdate*>(pkt);
		PacketMount* packet_mount = reinterpret_cast<PacketMount*>(pkt);

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
			

		case 0x6a:
		{
			CUnit* unit;
			if (field_0x9d0.Lookup(packet_word->value, unit))
			{
				field_0x9d0.RemoveKey(packet_word->value);
				delete unit;
			}
		}
			break;

		case 0x6b:
		{
			CUnit* unit;
			if (!field_0x9d0.Lookup(packet_abil->field_0xa, unit))
			{
				sprintf(buf, "Invalid unit #%d. Command Move.", packet_abil->field_0xa);
				if (g_EnableTrace)
					msglog.Add(buf, clrsh_TechBlack, 5000);
			}
			else
			{
				if (unit->action_segments)
				{
					switch (unit->action)
					{
					case 1:
						sprintf(buf, "Overriding \'Move\' by \'Move\'. %d segments lost.", unit->action_segments);
						break;

					case 3:
						sprintf(buf, "Overriding \'Attack\' by \'Move\'. %d segments lost.", unit->action_segments);
						break;

					case 5:
						buf[0] = 0;
						break;

					case 7:
						sprintf(buf, "Overriding \'Shoot\' by \'Move\'. %d segments lost.", unit->action_segments);
						break;

					case 8:
						sprintf(buf, "Overriding \'Cast\' by \'Move\'. %d segments lost.", unit->action_segments);
						break;

					default:
						sprintf(buf, "Overriding <Command %d> by \'Move\'. %d segments lost.", unit->action, unit->action_segments);
						break;
					}

					if (g_EnableTrace)
						msglog.Add(buf, clrsh_TechBlack, 5000);
				}

				if (unit->hp > 0)
				{
					unit->action_segments = packet_abil->field_0xd;
					unit->action = 1;
					unit->action_dir = packet_abil->field_0xc;
					unit->action_x = INT_6362e8[unit->action_dir] << 8;
					unit->action_y = INT_636328[unit->action_dir] << 8;
				}
			}
		}
			break;

		case 0x6c:
		case 0x6e:
		case 0x6f:
		case 0x70:
		{

			if (packet_unit->unit_id == 0)
			{
				if (g_EnableTrace)
				{
					sprintf(buf, "Invalid index id=%d", packet_unit->unit_id);
					msglog.Add(buf, clrsh_TechBlack, 30000);
				}
			}
			else
			{
				uint8_t* pdata = packet_unit->data;
				if ((packet_unit->flags_mask & 1) != 0)
				{
					g_CUnitStatic.hp = *(uint16_t*)pdata;
					pdata += 2;
				}
				if ((packet_unit->flags_mask & 2) != 0)
				{
					g_CUnitStatic.mp = *(uint16_t*)pdata;
					pdata += 2;
				}
				if ((packet_unit->flags_mask & 4) != 0)
				{
					g_CUnitStatic.skill_levels[0] = pdata[0];
					g_CUnitStatic.skill_levels[1] = pdata[1];
					g_CUnitStatic.skill_levels[2] = pdata[2];
					g_CUnitStatic.skill_levels[3] = pdata[3];
					g_CUnitStatic.skill_levels[4] = pdata[4];
					pdata += 5;
				}
				if ((packet_unit->flags_mask & 8) != 0)
				{
					g_CUnitStatic.hp_max = *(int16_t*)pdata;
					pdata += 2;
					g_CUnitStatic.mp_max = *(int16_t*)pdata;
					pdata += 2;
				}
				if ((packet_unit->flags_mask & 0x10) != 0)
				{
					g_CUnitStatic.last_action = *pdata;
					pdata ++;
					g_CUnitStatic.x_pos = *pdata;
					pdata ++;
					g_CUnitStatic.y_pos = *pdata;
					pdata ++;
				}
				if ((packet_unit->flags_mask & 0x20) != 0)
				{
					g_CUnitStatic.typeId = *pdata;
					pdata ++;
					g_CUnitStatic.face = *pdata;
					pdata ++;
				}

				uint8_t ownerid = 0;
				if ((packet_unit->flags_mask & 0x40) != 0)
				{
					ownerid = *pdata;
					pdata ++;
				}

				if ((packet_unit->flags_mask & 0x80) != 0)
				{
					g_CUnitStatic.serverId = *(uint16_t*)pdata;
					pdata += 2;
				}

				if (ownerid >= field_0x9b8.GetSize())
				{
					if (g_EnableTrace != 0)
					{
						sprintf(buf, "Invalid owner. Unit(ext) id=%d", packet_unit->unit_id);
						msglog.Add(buf, clrsh_TechBlack, 30000);
					}
				}
				else
				{
					MapPlayerData* mpl = field_0x9b8[ownerid];
					if (!mpl)
					{
						if (g_EnableTrace != 0)
						{
							sprintf(buf, "Invalid owner. Unit(ext) id=%d", packet_unit->unit_id);
							msglog.Add(buf, clrsh_TechBlack, 30000);
						}
					}
					else
					{
						bool groundunit = false;
						Unit* uni = nullptr;

						if (packet_unit->flags_mask & 0x20)
						{
							if (g_CUnitStatic.typeId >= 0x20 && g_CUnitStatic.typeId < 0x40)
								groundunit = true;
						}
						else if (packet_unit->flags_mask & 0x80)
						{
							uni = new Unit();
							uni->itemDataID = g_GameDataRes.FUN_00512625(g_CUnitStatic.serverId);
							uni->monster_info = &g_GameDataRes.monsters[uni->itemDataID];
							uni->FUN_0052ec7a(uni->monster_info->values);
							uni->server_id = g_CUnitStatic.serverId;

							if (wnd->field_0x640 == 2 && g_Server && g_Server->field22_0xd8 != 2)
							{
								if (g_Server->field22_0xd8 == 1)
									uni->hp_max = uni->hp_max / 2.0;
								else
								{
									uni->hit_values.attack += 50;
									uni->protections.defense += 50;
									uni->hp_max = uni->hp_max * 1.5;
								}
								uni->hp = uni->hp_max;
							}

							if (packet_unit->flags_mask & 8)
							{
								auto tmp_hp = g_CUnitStatic.hp;
								auto tmp_hpmax = g_CUnitStatic.hp_max;
								g_CUnitStatic.FUN_0046b0d7(*uni);
								g_CUnitStatic.hp = tmp_hp;
								g_CUnitStatic.hp_max = tmp_hpmax;
							}
							else
								g_CUnitStatic.FUN_0046b0d7(*uni);
						}

						CUnit* ct = nullptr;
						bool ct_exist = true;

						if (field_0x9d0.Lookup(packet_unit->unit_id, ct) == 0)
						{
							ct_exist = false;

							if ((packet_unit->flags_mask & (0x80 | 0x20)) == 0)
								break;

							if (groundunit || g_VFX_info[g_CUnitStatic.typeId]->z == 0)
								ct = new CUnit();
							else
								ct = new CAirUnit();

							if (uni)
								ct->FUN_0046b0d7(*uni);
							else
							{
								ct->typeId = g_CUnitStatic.typeId;
								ct->FUN_0046b7d2(g_CUnitStatic.face);
								ct->FUN_0046b91c();
							}

							field_0x9d0.SetAt(packet_unit->unit_id, ct);
							ct->unit_id = packet_unit->unit_id;
						}

						if (packet_unit->flags_mask & 0x40)
							ct->field_0x14 = mpl;

						if (packet_unit->flags_mask & 1)
							ct->hp = g_CUnitStatic.hp;

						if (packet_unit->flags_mask & 2)
							ct->mp = g_CUnitStatic.mp;

						if (packet_unit->flags_mask & 4)
						{
							ct->skill_levels[0] = g_CUnitStatic.skill_levels[0];
							ct->skill_levels[1] = g_CUnitStatic.skill_levels[1];
							ct->skill_levels[2] = g_CUnitStatic.skill_levels[2];
							ct->skill_levels[3] = g_CUnitStatic.skill_levels[3];
							ct->skill_levels[4] = g_CUnitStatic.skill_levels[4];
						}

						if (packet_unit->flags_mask & 8)
						{
							ct->hp_max = g_CUnitStatic.hp_max;
							ct->mp_max = g_CUnitStatic.mp_max;
						}

						uint8_t sval = ct->field_0x180[4];

						if (packet_unit->flags_mask & 0x10)
						{
							ct->field_0x70 = g_CUnitStatic.last_action;
							ct->x_pos = g_CUnitStatic.x_pos;
							ct->y_pos = g_CUnitStatic.y_pos;
							ct->x_pos = ct->x_pos * 0x100 + 0x80;
							ct->y_pos = ct->y_pos * 0x100 + 0x80;
							ct->x_pos2 = ct->x_pos;
							ct->y_pos2 = ct->y_pos;
							ct->field_0x10 = 0;
						}

						if (packet_unit->flags_mask & 0x80)
							ct->serverId = g_CUnitStatic.serverId;

						if (packet_unit->flags_mask & 0x10000)
						{
							ct->attack = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & 0x20000)
						{
							ct->hand_dmg_min = pdata[0];
							ct->hand_dmg_spread = pdata[1];
							pdata += 2;
						}

						if (packet_unit->flags_mask & 0x4000)
						{
							ct->defence = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & 0x8000)
						{
							ct->absorb = *pdata;
							pdata++;
						}

						if (packet_unit->flags_mask & 0x40000)
						{
							ct->scan_range = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & 0x2000)
						{
							ct->carrying_weight_100g = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & 0x1000000)
						{
							ct->magic_protect[0] = pdata[0];
							ct->magic_protect[1] = pdata[1];
							ct->magic_protect[2] = pdata[2];
							ct->magic_protect[3] = pdata[3];
							ct->magic_protect[4] = pdata[4];
							pdata += 5;
						}

						if (packet_unit->flags_mask & 0x2000000)
						{
							wnd->some_struc.field_0x3c = *(uint32_t*)(pdata);
							wnd->some_struc.field_0x40 = *(uint32_t*)(pdata + 4);
							wnd->some_struc.field_0x48 = *(uint32_t*)(pdata + 8);
							wnd->some_struc.field_0x44 = *(uint32_t*)(pdata + 12);
							pdata += 16;
						}

						if (packet_unit->flags_mask & 0x80000)
						{
							ct->speed = *pdata;
							pdata++;
						}

						if (packet_unit->flags_mask & 0x100000)
						{
							ct->field_0x1c = *(int32_t*)pdata;
							pdata += 4;
						}

						if (packet_unit->flags_mask & 0x100000)
						{
							ct->field_0x118 = *pdata;
							pdata++;
						}

						if (packet_unit->flags_mask & 0x100)
						{
							ct->experience_per_sphere[0] = *(int32_t*)pdata;
							pdata += 4;
						}

						if (packet_unit->flags_mask & 0x200)
						{
							ct->experience_per_sphere[1] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & 0x400)
						{
							ct->experience_per_sphere[2] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & 0x800)
						{
							ct->experience_per_sphere[3] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & 0x1000)
						{
							ct->experience_per_sphere[4] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & 0x20000000)
						{
							ct->body = pdata[0];
							ct->reaction = pdata[1];
							ct->mind = pdata[2];
							ct->spirit = pdata[3];

							ct->field_0x180[0] = pdata[4];
							ct->field_0x180[1] = pdata[5];
							ct->field_0x180[2] = pdata[6];
							ct->field_0x180[3] = pdata[7];

							pdata += 8;
						}

						ct->exp_summary = ct->experience_per_sphere[0] + ct->experience_per_sphere[1] + ct->experience_per_sphere[2] + ct->experience_per_sphere[3] + ct->experience_per_sphere[4];

						if ((packet_unit->flags_mask & 0x10) != 0 && ct->unit_id > 0x5fff)
							ct->field_0x1b8 &= ~0x80;

						if (packet_unit->flags_mask & 0x80000000)
						{
							memset(ct->field_0xec, 0, sizeof(ct->field_0xec));
							char* dpos = strchr((char*)pdata, '|');
							if (!dpos)
							{
								memcpy(ct->field_0xec, pdata, 11);
								ct->field_0xf8[0] = 0;
							}
							else
							{
								memcpy(ct->field_0xec, pdata, dpos - (char*)pdata);
								memcpy(ct->field_0xf8, dpos, 12);
							}
							pdata += 12 + 12;
						}

						if (wnd->field_0x640 == 2)
						{
							if (ct->serverId == 0x15)
							{
								field_0x3f6c = ct;
								field_0x3f6c->field_0x1b8 |= 0x20;
							}
						}
						else if (ct->field_0x14 == my_main_unit && (ct->field_0x1b8 & 1) != 0)
						{
							field_0x3f6c = ct;
							field_0x3f6c->field_0x1b8 |= 0x20;
						}

						if (ct->field_0xec[0] == 0 && ct->serverId != 0)
							strcpy(ct->field_0xec, txt_npcnames.GetLine(ct->serverId - 1));

						if (packet_unit->flags_mask & 0x800000)
							ct->field_0x130.RemoveAll();

						if (ct_exist && (wnd->field_0x418 & 0x200) != 0)
						{
							if ((packet_unit->flags_mask & 0x20) != 0)
								ct->face = g_CUnitStatic.face;

							field_0x3f6c->field_0x1b8 |= 8;
						}

						if (ct->hp < -600)
							ct->field_0x180[4] = 5;
						else if (ct->hp < -40)
							ct->field_0x180[4] = 4;
						else if (ct->hp < -20)
							ct->field_0x180[4] = 3;
						else if (ct->hp < -10)
							ct->field_0x180[4] = 2;
						else if (ct->hp < 1)
							ct->field_0x180[4] = 1;
						else
							ct->field_0x180[4] = 0;

						g_CUnitStatic.field_0x180[4] = ct->field_0x180[4];

						if (ct->serverId != 0 && ct->serverId < 21 && ct->field_0x180[4] > 1)
							ScenarioSetVar(ct->serverId + 0x213, 0);

						if (wnd->field_0x640 != 2 && (ct->field_0x1b8 & 0x20) != 0)
						{
							if ((packet_unit->flags_mask & 0x22100000) == 0)
							{
								if ((packet_unit->flags_mask & 0x1f00) != 0)
									wnd->some_struc.FUN_004946ae();
							}
							else
								wnd->some_struc.FUN_004946e0();
						}

						switch (ct->field_0x180[4])
						{
						case 0:
							if (sval != 0)
							{
								ct->action = 0;
								if (ct->field_0x1b8 & 1)
									ct->FUN_0046b91c();
							}
							break;

						case 1:
							if (sval == 0)
							{
								UnitVFXUnfo* dying = g_VFX_info[ g_VFX_info[ct->typeId]->dying ];
								ct->action_segments = dying->dying_phases * 2;
								ct->action = 6;
								ct->action_dir = ct->field_0x70;
								ct->action_phase = 0;
								ct->field_0xa4 = 0;
								ct->field_0xa0 = 0;
								ct->action_x = ct->x_pos;
								ct->action_y = ct->y_pos;
								ct->VMethod25(3);
								ct->FUN_0046b91c();
							}
							else if (sval == 1)
							{
								UnitVFXUnfo* dying = g_VFX_info[g_VFX_info[ct->typeId]->dying];
								ct->action_segments = 4;
								ct->action = 6;
								ct->action_dir = ct->field_0x70;
								ct->action_phase = dying->dying_phases * 2 - 4;
								ct->field_0xa4 = 0;
								ct->field_0xa0 = 0;
								ct->action_x = ct->x_pos;
								ct->action_y = ct->y_pos;
								ct->VMethod25(2);
								ct->FUN_0046b91c();
							}
							break;

						case 2:
						case 3:
						case 4:
							if (wnd->field_0x640 != 2 && ct == field_0x3f6c && ct->field_0x180[4] == 4 && sval < 4)
							{
								msglog.Add(txt_patch.GetLine(0x44), clr_log_sblack, 10000);
								msglog.Add(txt_patch.GetLine(0x45), clr_log_sblack, 10000);
							}

							if (sval < 2 && my_main_unit->FUN_0041ee20(ct->field_0x14->index) != 0)
								wnd->field_0x5e8.FUN_00420050();

							ct->field_0x88 = 0;
							ct->field_0x130.RemoveAll();
							ct->field_0x11c.RemoveAll();
							if (ct->FUN_0041f110() != 0)
							{
								ct->VMethod1(0);
								FUN_00416cf7();
							}

							break;

						case 5:
							ct->field_0x88 = 0;
							ct->field_0x130.RemoveAll();
							ct->field_0x11c.RemoveAll();
							if (ct->FUN_0041f110() != 0)
							{
								ct->VMethod1(0);
								FUN_00416cf7();
							}

							if (ct->unit_id < 0x6000)
							{
								field_0x9d0.RemoveKey(packet_unit->unit_id);
								if (ct)
									delete ct;
							}
							break;
						}
						if (g_CUnitStatic.field_0x180[4] < 5)
						{
							if ((wnd->field_0x418 & 1) != 0 && field_0x80)
								ct->FUN_0046190d();

							if (field_0x138 == ct)
							{
								wnd->field_0xe0->MsgProc(0x408, 0, 0);
								wnd->field_0xe4->MsgProc(0x408, 0, 0);
							}

							if (ct->FUN_0041f110() != 0 && ((ct->field_0x1b8 & 2) != 0 || (packet_unit->flags_mask & 0x100000) != 0))
								FUN_00416cf7();

							ct->field_0x114 = 1;
						}
					}
				}
			}
		}
			break;

		case 0x6d:
		{
			CUnit* ct = nullptr;
			if (field_0x9d0.Lookup(packet_abil->field_0xa, ct) == 0)
			{
				if (g_EnableTrace != 0)
				{
					sprintf(buf, "Invalid unit #%d. Command Turn.", packet_abil->field_0xa);
					msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			}
			else
			{
				if (ct->action_segments != 0)
				{
					if (g_EnableTrace)
					{
						switch (ct->action) {
						case 1:
							sprintf(buf, "Overriding \'Move\' by \'Turn\'. %d segments lost.", ct->action_segments);
							break;
						case 3:
							sprintf(buf, "Overriding \'Attack\' by \'Turn\'. %d segments lost.", ct->action_segments);
							break;
						case 5:
							buf[0] = 0;
							break;
						case 7:
							sprintf(buf, "Overriding \'Shoot\' by \'Turn\'. %d segments lost.", ct->action_segments);
							break;
						case 8:
							sprintf(buf, "Overriding \'Cast\' by \'Turn\'. %d segments lost.", ct->action_segments);
							break;

						default:
							sprintf(buf, "Overriding <Command %d> by \'Turn\'. %d segments lost.", ct->action, ct->action_segments);
							break;
						}
						msglog.Add(buf, clrsh_TechBlack, 5000);
					}
				}
				if (ct->hp > 0)
				{
					ct->action_segments = packet_abil->field_0xd;
					ct->action = 5;
					ct->action_dir = packet_abil->field_0xc;
					ct->field_0xc4 = ct->field_0x70 * 16;
					ct->action_phase = 0;
					ct->field_0xa4 = 0;
					ct->field_0xa0 = 0;
				}
			}
		}
			break;

		case 0x71:
		{
			CUnit* ct = nullptr;
			if (field_0x9d0.Lookup(packet_abil->field_0xa, ct) == 0)
			{
				if (g_EnableTrace != 0)
				{
					sprintf(buf, "Invalid unit #%d. Command Attack.", packet_abil->field_0xa);
					msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			}
			else
			{
				if (ct->action_segments == 0)
				{
					UnitVFXUnfo* vfx = g_VFX_info[ct->typeId];
					if (vfx->attack_phases != 0 && ct->hp > 0)
					{
						ct->action_segments = vfx->attack_anim_frame_cnt;
						ct->action = 3;
						ct->action_dir = packet_abil->field_0xc;
						ct->action_phase = 0;
						ct->field_0xa4 = 0;
						ct->field_0xa0 = 0;
						break;
					}
				}
				
				if (ct->action_segments != 0)
				{
					if (g_EnableTrace)
					{
						switch (ct->action)
						{
						case 1:
							sprintf(buf, "Overriding \'Move\' by \'Attack\'. %d segments lost.", ct->action_segments);
							break;
						case 3:
							sprintf(buf, "Overriding \'Attack\' by \'Attack\'. %d segments lost.", ct->action_segments);
							break;
						case 5:
							buf[0] = 0;
							break;
						case 7:
							sprintf(buf, "Overriding \'Shoot\' by \'Attack\'. %d segments lost.", ct->action_segments);
							break;
						case 8:
							sprintf(buf, "Overriding \'Cast\' by \'Attack\'. %d segments lost.", ct->action_segments);
							break;
						default:
							sprintf(buf, "Overriding <Command %d> by \'Attack\'. %d segments lost.", ct->action, ct->action_segments);
							break;
						}

						msglog.Add(buf, clrsh_TechBlack, 5000);
					}
				}
			}
		}
			break;

		case 0x72:
		{
			CUnit* ct = nullptr;
			if (field_0x9d0.Lookup(packet_mount->field_0xa, ct) == 0)
			{
				if (g_EnableTrace)
				{
					sprintf(buf, "Invalid unit #%d. Command Ranged Attack.", packet_mount->field_0xa);
					msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			}
			else if (ct->action_segments == 0 && ct->hp > 0)
			{
				ct->action_segments = g_VFX_info[ct->typeId]->attack_anim_frame_cnt;
				ct->action = 7;
				ct->action_phase = 0;
				ct->field_0xa4 = 0;
				ct->field_0xa0 = 0;
				ct->action_target = packet_mount->unit_id;
				ct->action_dir = ct->field_0x70;
			}
			else if (ct->action_segments != 0) 
			{
				if (g_EnableTrace)
				{
					switch (ct->action)
					{
					case 1:
						sprintf(buf, "Overriding \'Move\' by \'Shoot\'. %d segments lost.", ct->action_segments);
						break;
					case 3:
						sprintf(buf, "Overriding \'Attack\' by \'Shoot\'. %d segments lost.", ct->action_segments);
						break;
					case 5:
						buf[0] = 0;
						break;
					case 7:
						sprintf(buf, "Overriding \'Shoot\' by \'Shoot\'. %d segments lost.", ct->action_segments);
						break;
					case 8:
						sprintf(buf, "Overriding \'Cast\' by \'Shoot\'. %d segments lost.", ct->action_segments);
						break;
					default:
						sprintf(buf, "Overriding <Command %d> by \'Shoot\'. %d segments lost.", ct->action, ct->action_segments);
						break;
					}

					msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			}
		}
			break;
		
		}

		if (pkt->id == breakid)
			return 1;
	}
	return 1;
}