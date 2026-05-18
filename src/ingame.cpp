#include "ingame.h"
#include "asm_mfc.h"
#include "visual.h"
#include "gameobj.h"
#include "net.h"
#include "packet.h"
#include "main_window.h"
#include "txtfile.h"
#include "gfx.h"
#include "game_app.h"
#include "unit.h"
#include "mouse.h"
#include "item.h"
#include "player_file.h"
#include "map_stuff.h"
#include "quest.h"
#include "quest_map.h"
#include "file.h"
#include "spell.h"


uint16_t* clr_log_sblack = clrsh_ShockingBlack; //62f88c
uint16_t* clr_log_tblack = clrsh_TechBlack; //62f890
uint16_t* clr_log_tok0 = clrsh_ShockingBlack; //62f894
uint16_t* clr_log_tok2 = clrsh_ShockingBlack; //62f898
uint16_t* clr_log_tok1 = clrsh_ShockingBlack; //62f89c
uint16_t* clr_log_tok3 = clrsh_ShockingBlack; //62f8a0

int INT_6362e8[16] = { 0, 0, 1, 0,   1, 0, 1, 0,   0, 0, -1, 0,   -1, 0, -1, 0 };
int INT_636328[16] = { -1, 0, -1, 0,   0, 0, 1, 0,   1, 0, 1, 0,   0, 0, -1, 0 };

CArray<GfxFile*> g_GfxFiles; //661100
CArray<GfxObject*> g_GfxObjects; //665330
CArray<StructureInfo*> g_StructuresInfo; //661098
CArray<ProjectileInfo*> g_ProjectileInfos; //6610b0

CGamePalette* g_pal_projectiles; //665490
CGamePalette* g_pal_projectile_; //665494

CA16* g_spr_smoke[2]; //6610f8

extern CUnit g_CUnitStatic; //642a48
extern CStringArray DAT_00666a00; //666a00



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




//4216b0
void __cdecl ReadKillStats(uint8_t* data, int32_t datasz, uint8_t** out, int32_t* outsz)
{
	int32_t sz = *(int32_t*)data;
	data += 4;

	*outsz = sz;

	uint8_t* buf = new uint8_t[sz];

	*out = buf;

	uint8_t* dst = buf;

	int32_t readcount = 4;
	while (readcount < datasz)
	{
		if ((*data & 0x80) == 0)
		{
			int num = *data;
			data++;
			readcount++;

			memcpy(dst, data, num);
			dst += num;

			data += num;
			readcount += num;
		}
		else
		{
			int num = data[0] & 0x7f;
			int val = data[1];

			memset(dst, val, num);
			dst += num;

			data += 2;
			readcount += 2;
		}
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
		PacketPing* packet_ping = reinterpret_cast<PacketPing*>(pkt);
		PacketUnitStateVec* packet_state = reinterpret_cast<PacketUnitStateVec*>(pkt);
		PacketEight* packet_eight = reinterpret_cast<PacketEight*>(pkt);
		PacketSync* packet_sync = reinterpret_cast<PacketSync*>(pkt);
		PacketMoveCmd* packet_move = reinterpret_cast<PacketMoveCmd*>(pkt);
		PacketAoeZone* packet_aoe = reinterpret_cast<PacketAoeZone*>(pkt);
		PacketEffect* packet_effect = reinterpret_cast<PacketEffect*>(pkt);
		PacketTerrain* packet_terrain = reinterpret_cast<PacketTerrain*>(pkt);
		PacketUnitProperties* packet_props = reinterpret_cast<PacketUnitProperties*>(pkt);
		PacketPlayerInfo* packet_player = reinterpret_cast<PacketPlayerInfo*>(pkt);

		if (!pkt)
		{
			if (breakid != 0 || (wnd->field_0x418 & 1) != 0)
				continue;
			return 0;
		}

		//printf("Client receive packet 0x%x\n", pkt->id);

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
				sprintf(buf, "%s %d %s", TxtFile::AllLines[0x58], packet_info->field_0xa - my_main_unit->gold, TxtFile::AllLines[0x59]);
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
			if (field_0x9d0.Lookup(packet_word->value, *(CGameObject**)&unit))
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
			if (field_0x9d0.Lookup(packet_word->value, *(CGameObject**)&unit))
			{
				field_0x9d0.RemoveKey(packet_word->value);
				delete unit;
			}
		}
			break;

		case 0x6b:
		{
			CUnit* unit;
			if (!field_0x9d0.Lookup(packet_abil->field_0xa, *(CGameObject**)&unit))
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

						if (field_0x9d0.Lookup(packet_unit->unit_id, *(CGameObject**)&ct) == 0)
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
							ct->dir = g_CUnitStatic.last_action;
							ct->x_pos = g_CUnitStatic.x_pos;
							ct->y_pos = g_CUnitStatic.y_pos;
							ct->x_pos = ct->x_pos * 0x100 + 0x80;
							ct->y_pos = ct->y_pos * 0x100 + 0x80;
							ct->x_pos2 = ct->x_pos;
							ct->y_pos2 = ct->y_pos;
							ct->z_pos = 0;
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
							wnd->some_struc.monster_killed = *(uint32_t*)(pdata);
							wnd->some_struc.player_killed = *(uint32_t*)(pdata + 4);
							wnd->some_struc.death_count = *(uint32_t*)(pdata + 8);
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
							ct->spells = *(int32_t*)pdata;
							pdata += 4;
						}

						if (packet_unit->flags_mask & 0x100000)
						{
							ct->active_spell = *pdata;
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

						if ((packet_unit->flags_mask & 0x100000) != 0)
						{
							ct->field_0x18 = 0;
							if (ct->field_0x14 == my_main_unit && ct->spells != 0)
							{
								for (int i = 0; i < 32; i++)
								{
									if ((ct->spells & (1 << i)) != 0)
										ct->field_0x18 |= SpellBtB[i];
								}
							}
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
								ct->action_dir = ct->dir;
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
								ct->action_dir = ct->dir;
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
			if (field_0x9d0.Lookup(packet_abil->field_0xa, *(CGameObject**)&ct) == 0)
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
					ct->field_0xc4 = ct->dir * 16;
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
			if (field_0x9d0.Lookup(packet_abil->field_0xa, *(CGameObject**)&ct) == 0)
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
			if (field_0x9d0.Lookup(packet_mount->field_0xa, *(CGameObject**)&ct) == 0)
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
				ct->action_dir = ct->dir;
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

		case 0x73:
		{
			CUnit* ct = nullptr;
			if (field_0x9d0.Lookup(packet_ping->field_0xa, *(CGameObject**)&ct) == 0)
			{
				if (g_EnableTrace)
				{
					sprintf(buf, "Invalid unit #%d. Command Take damage.", packet_ping->field_0xa);
					msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			}
			else
			{
				if (flying_hp && packet_ping->field_0xc < ct->hp)
				{
					bool not_me = ct->field_0x14 != my_main_unit;

					int32_t a = ct->VMethod4();
					int32_t dx = a * 16;

					if (not_me)
						dx = -dx;

					TakeDamage::AddDamage(&damage_labels, TakeDamage(ct->hp - packet_ping->field_0xc, nullptr, not_me, dx, a * -48, ct) );
				}
				if (ct->hp == packet_ping->field_0xc)
					ct->VMethod25(0);
				else if (ct->hp > -10)
				{
					if (ct->hp < ct->hp_max / 2)
						ct->VMethod25(2);
					else
						ct->VMethod25(1);
				}

				ct->hp = packet_ping->field_0xc;
				if (field_0x138 == ct)
				{
					wnd->field_0xe0->MsgProc(0x408, 0, 0);
					wnd->field_0xe4->MsgProc(0x408, 0, 0);
				}
				ct->field_0x114 = 1;
			}
		}
			break;
		

		case 0x76:
		{
			if (wnd->field_0x408 && packet_state->field_0xc == wnd->field_0x410)
			{
				g_Cursors[CURSOR_DEFAULT]->Use();
				if (wnd->field_0x408)
					delete wnd->field_0x408;
				wnd->ResetItemCursor();
			}
			CUnit* ct = nullptr;
			switch (packet_state->field_0xc & 0x7f)
			{
			case 1:
				if (field_0x9d0.Lookup(packet_state->field_0xa, *(CGameObject**)&ct) != 0)
				{
					uint8_t *pdata = packet_state->data;
					for (int i = 0; i < 12; i++)
					{
						if ((packet_state->field_0xf & (1 << i)) != 0)
						{
							if (ct->field_0x188[i])
								delete ct->field_0x188[i];

							GO_d0* itm = new GO_d0(&pdata, 0);
							if (itm->item_id == 0)
							{
								delete itm;
								ct->field_0x188[i] = nullptr;
							}
							else 
							{
								itm->field_0x18 = 1;
								ct->field_0x188[i] = itm;

								if (g_EnableTrace)
								{
									CString logstr = "Invalid item weared " + ct->field_0x188[i]->FUN_004394f3();

									if (ct->field_0x188[i]->FUN_004396d6() == 0 && g_EnableTrace != 0)
									{
										CStdioFile f;
										f.Open("error.log", CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite);
										f.SeekToEnd();
										f.WriteString(CTime::GetCurrentTime().Format("%d.%m.%y %H:%M:%S ") + logstr + "\n");
										f.Close();
										wnd->field_0xd0->msglog.Add(logstr, clrsh_TechBlack, 30000);
									}
								}
							}
						}
					}

					ct->field_0x1b8 |= 8;

					if ((wnd->field_0x418 & 2) != 0)
						wnd->vis_root->FindChild(1000)->MsgProc(0x413, packet_state->field_0xc & 0x7f, 0);

					ct->FUN_0046b91c();

					ct->field_0x114 = 1;

					if ((ct->field_0x1b8 & 0x2U) != 0 && wnd->field_0x640 != 2)
					{
						uint8_t* pdata = packet_state->data;
						for (int i = 0; i < 12; i++)
						{
							if ((packet_state->field_0xf & (1 << i)) != 0)
							{
								if (field_0x4988[i])
									delete field_0x4988[i];

								field_0x4988[i] = sub_4F499B(&pdata);
							}
						}

						packet_state->data_size = 0;

						for (int i = 0; i < 12; i++)
						{
							if (!field_0x4988[i])
							{
								Item item;
								item.StoreToPacket(packet_state, 0);
							}
							else
								field_0x4988[i]->StoreToPacket(packet_state, 0);
						}

						CString fname = wnd->some_struc.FUN_00420070();
						WritePlayerFile_4F53EA(fname, nullptr, nullptr, nullptr, packet_state, nullptr, nullptr, 0);
					}
				}
				break;

			case 2:
			{
				if (field_0x9d0.Lookup(packet_state->field_0xa, *(CGameObject**)&ct) != 0)
				{
					if ((packet_state->field_0xc & 0x80) == 0)
					{
						ct->field_0xd0.RemoveAll();
					}
					else
					{
						int num = ct->field_0xd0.GetSize();

						if ((ct->field_0x1b8 & 0x20) != 0)
							num--;

						if (num <= packet_state->field_0xf)
						{
							if (g_EnableTrace)
							{
								CString logstr = "Invalid inventory update! all items ignored.";

								CStdioFile f;
								f.Open("error.log", CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite);
								f.SeekToEnd();
								f.WriteString(CTime::GetCurrentTime().Format("%d.%m.%y %H:%M:%S ") + logstr + "\n");
								f.Close();
								wnd->field_0xd0->msglog.Add(logstr, clrsh_TechBlack, 30000);
							}
							break;
						}

						if (num < (packet_state->field_0xf + packet_state->entry_count))
						{
							if (g_EnableTrace)
							{
								CString logstr = "Invalid inventory update! some items ignored.";

								CStdioFile f;
								f.Open("error.log", CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite);
								f.SeekToEnd();
								f.WriteString(CTime::GetCurrentTime().Format("%d.%m.%y %H:%M:%S ") + logstr + "\n");
								f.Close();
								wnd->field_0xd0->msglog.Add(logstr, clrsh_TechBlack, 30000);
							}
							packet_state->entry_count = num - packet_state->field_0xf;
						}

						for (int i = 0; i < packet_state->entry_count; i++)
						{
							if ((i + packet_state->field_0xf) < ct->field_0xd0.GetSize())
							{
								GO_d0* o = ct->field_0xd0[i + packet_state->field_0xf];
								if (o)
								{
									delete o;
									ct->field_0xd0[i + packet_state->field_0xf] = nullptr;
								}
							}
						}

						ct->field_0xd0.RemoveAt(packet_state->field_0xf, packet_state->entry_count);

						if ((ct->field_0x1b8 & 0x20) != 0)
							ct->field_0xd0.RemoveAt(ct->field_0xd0.GetUpperBound());
					}

					uint8_t* pdata = packet_state->data;
					for (int i = 0; i < packet_state->entry_count; i++)
					{
						GO_d0* obj = new GO_d0(&pdata, 0);
						obj->field_0x18 = 2;

						if (obj->FUN_004396d6())
							ct->field_0xd0.InsertAt(i + packet_state->field_0xf, obj, 1);

						if (g_EnableTrace)
						{
							CString logstr = "Invalid item in inventory " + obj->FUN_004394f3();

							CStdioFile f;
							f.Open("error.log", CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite);
							f.SeekToEnd();
							f.WriteString(CTime::GetCurrentTime().Format("%d.%m.%y %H:%M:%S ") + logstr + "\n");
							f.Close();
							wnd->field_0xd0->msglog.Add(logstr, clrsh_TechBlack, 30000);
						}
						
						if ((obj->flg & 0x40) != 0)
						{
							obj->flg = obj->flg & 0xbf;

							char* itmname = nullptr;
							unk_660DA8.Lookup(obj->item_id, *(void **)&itmname);

							if (obj->field_0x10 < 2)
								sprintf(buf, "%s %s", TxtFile::AllLines[85], itmname);
							else
								sprintf(buf, "%s %s (%s %d %s)", TxtFile::AllLines[85], itmname, TxtFile::AllLines[86], obj->field_0x10, TxtFile::AllLines[87]);
							msglog.Add(buf, clr_log_tblack, 3000);
						}
					}

					if ((ct->field_0x1b8 & 0x20) != 0)
					{
						if (wnd->field_0x640 != 2)
						{
							uint8_t *pdata = packet_state->data;
							for (int i = 0; i < packet_state->entry_count; i++)
							{								
								if (i + packet_state->field_0xf < field_0x4974.GetSize())
								{
									Item* itm = field_0x4974[i + packet_state->field_0xf];
									if (itm)
										delete itm;
								}

								Item * itm = sub_4F499B(&pdata);
								field_0x4974.SetAtGrow(i + packet_state->field_0xf, itm);
							}

							if ((packet_state->field_0xc & 0x80) == 0)
								field_0x4974.SetSize(packet_state->entry_count);
							
							packet_state->data_size = 0;

							for (int i = 0; i < field_0x4974.GetSize(); i++)
								field_0x4974[i]->StoreToPacket(packet_state, 0);


							CString fname = wnd->some_struc.FUN_00420070();
							WritePlayerFile_4F53EA(fname, nullptr, nullptr, nullptr, packet_state, nullptr, nullptr, 0);
						}

						GO_d0* gt = new GO_d0(0);
						gt->item_id = 0xffff;
						gt->field_0x10 = my_main_unit->gold;
						gt->flg = 0;
						gt->field_0x9 = 0;
						gt->field_0x18 = 2;
						ct->field_0xd0.Add(gt);
					}

					FUN_00416cf7();

					if (wnd->field_0x418 & 2)
						wnd->vis_root->FindChild(1000)->MsgProc(0x413, packet_state->field_0xc & 0x7f, 0);
					else
					{
						if (field_0x140 == 1 && field_0x138 == ct)
							wnd->field_0xe8->VMethod33(ct);
					}
					
				}
			}
				break;

			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			{
				CArray<GO_d0*> tmp;

				uint8_t* pdata = packet_state->data;

				for (int i = 0; i < packet_state->entry_count; i++)
				{
					GO_d0* d0 = nullptr;
					if ((packet_state->field_0xc & 0x7f) == 4)
						d0 = new GO_d0(&pdata, 1);
					else
						d0 = new GO_d0(&pdata, 0);
					
					if (d0->FUN_004396d6())
						tmp.Add(d0);
					else
					{
						if (g_EnableTrace)
						{
							CString logstr = "Invalid item in shop " + d0->FUN_004394f3();

							CStdioFile f;
							f.Open("error.log", CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite);
							f.SeekToEnd();
							f.WriteString(CTime::GetCurrentTime().Format("%d.%m.%y %H:%M:%S ") + logstr + "\n");
							f.Close();
							wnd->field_0xd0->msglog.Add(logstr, clrsh_TechBlack, 30000);
						}
					}
				}

				if ((wnd->field_0x418 & 2) == 0)
				{
					for (int i = 0; i < tmp.GetSize(); i++)
					{
						if (tmp[i])
							delete tmp[i];
					}
					tmp.RemoveAll();
				}
				else
					wnd->vis_root->FindChild(1000)->MsgProc(0x413, packet_state->field_0xc & 0x7f, (uint32_t)&tmp);
			}
				break;

			case 9:
			{
				if ((wnd->field_0x418 & 4) != 0)
				{
					VisTav* tavern = (VisTav*) wnd->vis_root->FindChild(0x44c);
					uint8_t* pdata = packet_state->data;
					for (int i = 0; i < packet_state->entry_count; i++)
					{
						GO_d0* d0 = new GO_d0(&pdata, 0);
						if (d0->FUN_004396d6() || d0->item_id > 0xfff0)
							tavern->field_0x124.Add(d0);
						//WAT ?  else - delete ???
					}
				}
			}
				break;
			}
		}
			break;


		case 0x7a:
		{
			CGameObject* ct = nullptr;
			if (field_0x9d0.Lookup(packet_eight->unit_id, ct))
			{
				ct->typeId = packet_eight->type_id;
				if (ct->typeId > 5)
					ct->typeId = 5;
			}
			else
			{
				CBackPack* pak = new CBackPack();
				field_0x9d0[packet_eight->unit_id] = pak;

				pak->SetVals(packet_eight->unit_id, 1, packet_eight->xpos, packet_eight->ypos, 0, 0, 0, 0, 1);

				pak->field_0x14 = my_main_unit;
				pak->typeId = packet_eight->type_id;
				if (pak->typeId > 5)
					pak->typeId = 5;

				if ((wnd->field_0x418 & 1) != 0 && field_0x80 != nullptr)
					pak->FUN_0046190d();
			}

		}
			break;

		case 0x82:
		{
			CUnit* ct = nullptr;
			if (field_0x9d0.Lookup(packet_sync->field_0xa, *(CGameObject**)&ct))
			{
				ct->hp = packet_sync->field_0xc;
				if (field_0x138 == ct)
				{
					wnd->field_0xe0->MsgProc(0x408, 0, 0);
					wnd->field_0xe4->MsgProc(0x408, 0, 0);
				}
			}
		}
			break;

		case 0x83:
			wnd->PostMessage(0x42a, packet_info->field_0xa, 0);
			break;

		case 0x84:
			wnd->PostMessage(0x42b, packet_info->field_0xa, packet_info->field_0xe);
			break;

		case 0x86:
			if ((packet_move->field_0xc & 1) == 0 || packet_move->field_0xc >= g_ProjectileInfos.GetSize() || g_ProjectileInfos[packet_move->field_0xc] == nullptr)
			{
				CUnit* ct = nullptr;
				if (field_0x9d0.Lookup(packet_move->field_0xa, *(CGameObject**)&ct) == 0)
				{
					if (g_EnableTrace != 0)
					{
						sprintf(buf, "Invalid unit #%d. Command Cast spell.", packet_move->field_0xa);
						msglog.Add(buf, clrsh_TechBlack, 5000);
					}
				}
				else if (ct->action_segments == 0 && g_VFX_info[ct->typeId]->attack_phases != 0)
				{
					ct->action_segments = g_VFX_info[ct->typeId]->attack_anim_frame_cnt;
					ct->action = 8;
					ct->action_phase = 0;

					if (packet_move->field_0xc < g_ProjectileInfos.GetSize() && g_ProjectileInfos[packet_move->field_0xc] != nullptr)
					{
						if (g_ProjectileInfos[packet_move->field_0xc]->homing == 0)
						{
							ct->action_x = packet_move->field_0xd * 0x100 + 0x80;
							ct->action_y = packet_move->field_0xe * 0x100 + 0x80;
							ct->action_target = 0;
						}
						else
							ct->action_target = packet_move->field_0xd | (packet_move->field_0xe << 8);
					}
					else
					{
						ct->action_x = ct->x_pos;
						ct->action_y = ct->y_pos;
						ct->action_target = 0;
					}
					ct->action_spell = packet_move->field_0xc;
				}
			}
			else
			{
				CProjectile* pj = new CProjectile();
				pj->typeId = packet_move->field_0xc;
				pj->x_pos = packet_move->field_0xd * 0x100 + 0x80;
				pj->y_pos = packet_move->field_0xe * 0x100 + 0x80;
				pj->z_pos = 0;
				pj->x_pos2 = pj->x_pos;
				pj->y_pos2 = pj->y_pos;
				pj->action_x = pj->x_pos;
				pj->action_y = pj->y_pos;
				pj->action_z = pj->z_pos;
				pj->action_target = 0;
				pj->action_phase = -1;
				pj->field_0x14 = my_main_unit;
				pj->action_segments = packet_move->field_0xf;
				pj->action = 1;
				pj->field_0xe8 = this;

				pj->FUN_0046190d();

				field_0x9ec[field_0xa24] = pj;
				field_0xa24++;

				SfxSample* snd = g_SfxArray[packet_move->field_0xc + 500];
				if (packet_move->field_0xc != 23 && snd)
				{
					int32_t vol = 0;
					int32_t pan = 0;
					FUN_0041b7b7(pj->x_pos, pj->y_pos, &vol, &pan);

					int priority = (10000 - ::abs(vol)) / 100;

					snd->Play(vol + g_SoundSettings.sfx_pos, pan, 0, priority, 0);
				}
			}
			break;

		case 0x87:
			FUN_0041c74b(packet_aoe->field_0xb,
							packet_aoe->field_0xc,
							packet_aoe->field_0xd,
							packet_aoe->field_0xe,
							(packet_aoe->field_0xa / 2) - 4,
							packet_aoe->data,
							packet_aoe->field_0xf);
			if (packet_aoe->field_0xf == 0)
				field_0xe0 = 1;
			break;

		case 0x88:
		{
			CGameObject* ct = nullptr;
			if (field_0x9d0.Lookup(packet_effect->field_0xa, ct) != 0)
			{
				int32_t idx = ct->FUN_00462405(packet_effect->effect_type);
				if (idx < 0)
					ct->field_0x130.Add((packet_effect->effect_type << 0x16) | 0xffff);
				else
					ct->field_0x130[idx] = (packet_effect->effect_type << 0x16) | 0xffff;

				ct->field_0x114 = 1;
			}
		}
			break;

		case 0x89:
		{
			CGameObject* ct = nullptr;
			if (field_0x9d0.Lookup(packet_effect->field_0xa, ct) != 0)
			{
				int32_t idx = ct->FUN_00462405(packet_effect->effect_type);
				if (idx > -1)
					ct->field_0x130.RemoveAt(idx);

				ct->field_0x114 = 1;
			}
		}
		break;

		case 0x8a:
		{
			CGameObject* ct = nullptr;
			field_0x9d0.Lookup(packet_terrain->buf[0], ct);
			if (ct->action_segments == 0 && g_VFX_info[ct->typeId]->attack_phases != 0)
			{
				ct->action_segments = g_VFX_info[ct->typeId]->attack_anim_frame_cnt;
				ct->action = 8;
				ct->action_phase = 0;

				ct->field_0xb0.RemoveAll();
				for (int i = 1; i < packet_terrain->count; i++)
					ct->field_0xb0.Add(packet_terrain->buf[i]);

				ct->action_target = packet_terrain->buf[1];
				ct->action_spell = 0x1e;
			}
		}
		break;

		case 0x8b:
		{
			CProjectile* pj = new CProjectile();
			pj->typeId = packet_move->field_0xc;
			pj->x_pos = (packet_move->field_0xa & 0xFF) * 0x100 + 0x80;
			pj->y_pos = (packet_move->field_0xa >> 8) * 0x100 + 0x80;
			pj->z_pos = 0;
			pj->x_pos2 = pj->x_pos;
			pj->y_pos2 = pj->y_pos;
			
			if (g_ProjectileInfos[packet_move->field_0xc]->homing == 0)
			{
				pj->action_x = packet_move->field_0xd * 0x100 + 0x80;
				pj->action_y = packet_move->field_0xe * 0x100 + 0x80;
				pj->action_target = 0;
			}
			else
				pj->action_target = packet_move->field_0xd | (packet_move->field_0xe << 8);

			pj->action_z = pj->z_pos;
			pj->action_phase = -1;
			pj->field_0x14 = my_main_unit;
			pj->action_segments = packet_move->field_0xf;
			pj->action = 1;
			pj->field_0xe8 = this;

			pj->FUN_0046190d();
			
			field_0x9ec[field_0xa24] = pj;
			field_0xa24++;

			SfxSample* snd = g_SfxArray[packet_move->field_0xc + 500];
			if (packet_move->field_0xc != 23 && snd)
			{
				int32_t vol = 0;
				int32_t pan = 0;
				FUN_0041b7b7(pj->x_pos, pj->y_pos, &vol, &pan);

				int priority = (10000 - ::abs(vol)) / 100;

				snd->Play(vol + g_SoundSettings.sfx_pos, pan, 0, priority, 0);
			}
		}
		break;

		case 0x8c:
		{
			CProjectile* pj = new CProjectile();
			pj->typeId = 0x1e;
			pj->x_pos = (packet_terrain->buf[0] & 0xFF) * 0x100 + 0x80;
			pj->y_pos = (packet_terrain->buf[0] & 0xFF00) /* * 0x100*/ + 0x80;
			pj->z_pos = 0;
			pj->x_pos2 = pj->x_pos;
			pj->y_pos2 = pj->y_pos;

			pj->field_0xb0.RemoveAll();

			for (int i = 1; i < packet_terrain->count; i++)
				pj->field_0xb0.Add(packet_terrain->buf[i]);

			pj->action_target = packet_terrain->buf[1];
			pj->action_z = pj->z_pos;
			pj->action_phase = -1;
			pj->field_0x14 = my_main_unit;
			pj->action_segments = 13;
			pj->action = 1;
			pj->field_0xe8 = this;

			pj->FUN_0046190d();

			field_0x9ec[field_0xa24] = pj;
			field_0xa24++;

			SfxSample* snd = g_SfxArray[pj->typeId + 500];
			if (pj->typeId != 23 && snd)
			{
				int32_t vol = 0;
				int32_t pan = 0;
				FUN_0041b7b7(pj->x_pos, pj->y_pos, &vol, &pan);

				int priority = (10000 - ::abs(vol)) / 100;

				snd->Play(vol + g_SoundSettings.sfx_pos, pan, 0, priority, 0);
			}
		}
		break;

		case 0x91:
		{
			if (packet_join->player_id == 0 || packet_join->player_id >= field_0x9b8.GetSize() || field_0x9b8[packet_join->player_id] == nullptr)
			{
				msglog.Add(packet_join->name, clr_log_sblack, 10000);
			}
			else if (my_main_unit->FUN_0041edf0(packet_join->player_id) == 0 || packet_join->token_id == 4)
			{
				MapPlayerData* dat = field_0x9b8[packet_join->player_id];

				if (g_MessageColors == 0 || (wnd->field_0x418 & 1) == 0)
				{
					msglog.Add(CString(dat->name) + ": " + packet_join->name, g_colors_human_pals[dat->color], 10000);
				}
				else
				{
					uint16_t* clr = nullptr;
					switch (packet_join->token_id)
					{
					case 0:
					case 4:
						clr = clr_log_tok0;
						break;
					case 1:
						clr = clr_log_tok1;
						break;
					case 2:
						clr = clr_log_tok2;
						break;
					case 3:
						clr = clr_log_tok3;
						break;
					}

					msglog.Add(CString(dat->name) + ": " + packet_join->name, clr, 10000);
				}

				g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				
			}
			wnd->vis_root->MsgProc(0x45f, 0, 0);
		}
		break;

		case 0x92:
		{
			switch (packet_info->field_0xa)
			{
			case 1:
				if (g_EnableTrace)
					msglog.Add(TxtFile::AllLines[0x81], clr_log_sblack, 3000);
				break;

			case 2:
			{
				int32_t texid = packet_info->field_0xe & 0xffff;
				int32_t serv_id = (packet_info->field_0xe >> 16) & 0xffff;

				CString txt;

				if ((field_0x3f6c->field_0x1b8 & 2) == 0)
					txt = TxtFile::AllLines[texid + 0x81];
				else
					txt = TxtFile::AllLines[texid + 0x86];

				if (wnd->field_0x640 == 2)
				{
					CUnit* ct = FUN_0041df23(serv_id);
					if (ct)
					{
						if ((ct->field_0x1b8 & 2) == 0)
							txt = TxtFile::AllLines[texid + 0x81] + CString(" (") + ct->field_0xec + ")";
						else
							txt = TxtFile::AllLines[texid + 0x86] + CString(" (") + ct->field_0xec + ")";
					}
				}

				msglog.Add(txt, clr_log_sblack, 3000);

				if ((wnd->field_0x418 & 1) != 0)
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
			}
				break;

			case 3:
				if (packet_info->field_0xe != my_main_unit->index)
				{
					sprintf(buf, "%s %s %s", TxtFile::AllLines[0xcc], field_0x9b8[packet_info->field_0xe]->name, TxtFile::AllLines[0xcd]);
					msglog.Add(buf, clr_log_sblack, 5000);
				}
				break;

			case 4:
				if (packet_info->field_0xe == my_main_unit->index)
					sprintf(buf, "%s %s %s", TxtFile::AllLines[0xd0], field_0x9b8[packet_info->field_0xe]->name, TxtFile::AllLines[0xd1]);
				else
					sprintf(buf, "%s %s %s", TxtFile::AllLines[0xce], field_0x9b8[packet_info->field_0xe]->name, TxtFile::AllLines[0xcf]);
				msglog.Add(buf, clr_log_sblack, 5000);
				break;

			case 5:
				sprintf(buf, "%s %s %s", TxtFile::AllLines[0xdd], field_0x9b8[packet_info->field_0xe]->name, TxtFile::AllLines[0xde]);
				msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->field_0x418 & 1) != 0)
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				break;

			case 6:
				sprintf(buf, "%s %s %s", TxtFile::AllLines[0xdf], field_0x9b8[packet_info->field_0xe]->name, TxtFile::AllLines[0xe0]);
				msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->field_0x418 & 1) != 0)
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				break;

			case 7:
				sprintf(buf, "%s %s %s", TxtFile::AllLines[0xe1], field_0x9b8[packet_info->field_0xe]->name, TxtFile::AllLines[0xe2]);
				msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->field_0x418 & 1) != 0)
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				break;

			case 8:
				sprintf(buf, "%s %d %s", TxtFile::AllLines[0x53], packet_info->field_0xe, TxtFile::AllLines[0x54]);
				msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->field_0x418 & 1) != 0)
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				break;

			case 0x10:
				msglog.Add(txt_patch.GetLine(0x58), clr_log_sblack, 5000);

				if ((wnd->field_0x418 & 1) != 0)
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				break;

			case 0x20:
				msglog.Add(txt_patch.GetLine(0x59), clr_log_sblack, 5000);
				msglog.Add(txt_patch.GetLine(0x5a), clr_log_sblack, 5000);

				if ((wnd->field_0x418 & 1) != 0)
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				break;

			case 0x40:
				msglog.Add(txt_patch.GetLine(0x5b), clr_log_sblack, 5000);

				if ((wnd->field_0x418 & 1) != 0)
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				break;

			case 0x80:
				msglog.Add(txt_patch.GetLine(0x5c), clr_log_sblack, 5000);

				if ((wnd->field_0x418 & 1) != 0)
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				break;
			}
		}
		break;

		case 0x93:
		{
			msglog.Add(txt_patch.GetLine(0x4e) + CString(" ") + packet_join->name + CString(" ") + txt_patch.GetLine(0x4f), clr_log_sblack, 5000);
			g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);

			if (packet_join->__field_0xa == my_main_unit->index)
			{
				if ((wnd->field_0x418 & 1) == 0)
				{
					while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
					{}
					g_CLlDriver.Close();
					return 0;
				}

				wnd->PostMessage(0x45c, 0, 0);
				g_CLlDriver.Close();
			}
		}
			break;

		case 0x94:
			if (packet_info->field_0xa != 0 && 
				packet_info->field_0xa < field_0x9b8.GetSize() && field_0x9b8[packet_info->field_0xa] != nullptr &&
				packet_info->field_0xe != 0 &&
				packet_info->field_0xe < field_0x9b8.GetSize() && field_0x9b8[packet_info->field_0xe] != nullptr)
			{
				CString str;
				str.Format("%s %s %s %s %s", txt_patch.GetLine(0x50), field_0x9b8[packet_info->field_0xa]->name, txt_patch.GetLine(0x51), field_0x9b8[packet_info->field_0xe]->name, txt_patch.GetLine(0x52));
				msglog.Add(str, clr_log_sblack, 5000);
			}
			break;

		case 0x96:
		{
			bool isin = packet_join->player_id < field_0x9b8.GetSize();
			if (isin)
			{
				if (field_0x9b8[packet_join->player_id] == nullptr)
					isin = false;
			}
			MapPlayerData* mp = nullptr;
			if (!isin)
			{
				mp = new MapPlayerData(packet_join->player_id, packet_join->field_0xc);
				mp->field2_0x8 = packet_join->token_id;
			}
			else
			{
				mp = field_0x9b8[packet_join->player_id];
			}

			if ((packet_join->flags & 1) == 0)
				mp->flags &= ~1;
			else
				mp->flags |= 1;

			strncpy(mp->name, packet_join->name, 31);
			mp->name[31] = 0;

			if (field_0x9b8.GetSize() == 1)
			{
				wnd->some_struc.field_0x10c = (packet_join->flags & 2) != 0;
				my_main_unit = mp;
				my_main_unit->diplomacy.SetAtGrow(0, 0);
				my_main_unit->diplomacy.SetAtGrow(mp->index, 0x3a);
			}
			else if ((mp->flags & 1) == 0)
			{
				if (mp != my_main_unit)
					my_main_unit->diplomacy.SetAtGrow(mp->index, 0);
			}
			else
				my_main_unit->diplomacy.SetAtGrow(mp->index, 1);

			field_0x9b8.SetAtGrow(mp->index, mp);

			wnd->FUN_00494a9e();

			if (wnd->field_0x144)
			{
				wnd->field_0x144->ReadData(&wnd->field_0x348);
				wnd->field_0x144->VMethod9();
			}

			wnd->vis_root->MsgProc(0x460, 0, 0);
		}
			break;

		case 0x97:
		{
			if (packet_info->field_0xa == my_main_unit->index)
			{
				wnd->PostMessage(0x45c, 0, 0);
				g_CLlDriver.Close();
				break;
			}

			CArray<uint16_t> tmp;
			for (POSITION it = field_0x9d0.GetStartPosition(); it != nullptr;)
			{
				uint16_t key;
				CGameObject* obj;
				field_0x9d0.GetNextAssoc(it, key, obj);
				
				if (obj->field_0x14 == field_0x9b8[packet_info->field_0xa])
					tmp.Add(key);
			}

			for (int i = 0; i < tmp.GetSize(); i++)
			{
				CGameObject* obj;
				field_0x9d0.Lookup(tmp[i], obj);
				field_0x9d0.RemoveKey(tmp[i]);

				if (obj->FUN_0041f110() != 0)
				{
					field_0x9b4 = 0;
					wnd->field_0xec->FUN_004caa69();
					wnd->field_0xdc->MsgProc(0x40b, 0, 0);
				}

				delete obj;
			}

			FUN_00416cf7();

			field_0x9b8[packet_info->field_0xa] = nullptr; //WAT ? not delete?
			wnd->vis_root->MsgProc(0x460, 0, 0);

			wnd->FUN_00494a9e();

			if (wnd->field_0x144)
			{
				wnd->field_0x144->ReadData(&wnd->field_0x348);
				wnd->field_0x144->VMethod9();
			}
		}
		break;

		case 0x9b:
		{
			CWordArray tmp;
			tmp.SetSize(packet_terrain->count);
			memcpy(tmp.GetData(), packet_terrain->buf, packet_terrain->count * 2);
			FUN_0041c630(&tmp);
		}
			break;

		case 0x9c:
		{
			CUnit* ct = nullptr;
			if (packet_props->field_0xa != 0 && field_0x9d0.Lookup(packet_props->field_0xa, *(CGameObject**)&ct) != 0)
			{
				int pr = 0;
				for (int i = 0; i < 12; i++)
				{
					if ((packet_props->flags & (1 << i)) != 0)
					{
						if (ct->field_0x188[i])
							delete ct->field_0x188[i];

						if (packet_props->prop[pr] == 0)
							ct->field_0x188[i] = nullptr;
						else
						{
							GO_d0* itm = new GO_d0(0);
							itm->item_id = packet_props->prop[pr];
							itm->field_0x10 = 1;
							itm->flg = 0;
							itm->field_0x9 = 0;
							itm->field_0xa = 0;
							itm->field_0x18 = 1;

							ct->field_0x188[i] = itm;
						}
						pr++;
					}
				}

				ct->field_0x1b8 |= 8;
				ct->FUN_0046b91c();
			}
		}
			break;

		case 0xaa:
		{
			switch(packet_info->field_0xa)
			{
			case 0:
				INT_00660f74 = 0;
				break;

			case 1:
			{
				INT_00660f74 = 1;
				uint16_t* landscape = field_0x80->GetLandscape();

				for (int i = 0; i < field_0x80->GetWidth(); i++)
				{
					for (int j = 0; j < field_0x80->GetHeight(); j++)
						landscape[i + j * field_0x80->GetWidth()] |= 0xc000;
				}
			}
				break;
			case 2:
				wnd->PostMessage(0x430, 0, 0);
				break;
			}
		}
			break;

		case 0xab:
			if (wnd->field_0x404 == 0)
				MsgProc(0x406, packet_info->field_0xa - (field_0x64 / 2), packet_info->field_0xe - (field_0x68 / 2));
			break;

		case 0xae:
			MsgProc(0x484, (uint32_t)&(packet_join->__field_0xa), 0); ///???????????
			break;

		case 0xaf:
			if (wnd->field_0x640 == 0)
			{
				if ((wnd->field_0x418 & 1) == 0)
				{
					while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
					{}
					g_CLlDriver.Close();
					return 0;
				}
				else
				{
					g_CLlDriver.Close();
					if (pkt)
						delete pkt; // WAT!?
					return 1;
				}
			}
			break;

		case 0xb3:
			kill_stats[packet_dword->value & 0xffff] = (packet_dword->value >> 16) & 0xff;

			if (wnd->field_0x640 != 2)
				wnd->some_struc.FUN_00494982();
			break;

		case 0xb4:
			wnd->PostMessage(0x433, 255, packet_info->field_0xa);
			break;

		case 0xb5:
			wnd->PostMessage(0x430, 0, 0);
			break;

		case 0xb6:
			wnd->PostMessage(0x433, packet_info->field_0xa, 0);
			break;

		case 0xb7:
			if (wnd->field_0x640 == 0)
			{
				wnd->field_0x454 = 1;
				wnd->PostMessage(0x45e, 0, 0);
			}
			break;

		case 0xb8:
			if (wnd->field_0x640 == 0)
			{
				if ((wnd->field_0x418 & 2) != 0)
				{
					VisShop* shop = (VisShop*)wnd->vis_root->FindChild(1000);
					shop->FUN_004bcd02();
					wnd->vis_root->RemoveChild(shop);
				}

				if ((wnd->field_0x418 & 4) != 0)
				{
					VisTav* tavern = (VisTav*)wnd->vis_root->FindChild(1100);
					tavern->FUN_0049edec();
					wnd->vis_root->RemoveChild(tavern);
				}
				wnd->vis_root->MsgProc(0x446, 0, 0);
				wnd->field_0x454 = 0;
				wnd->PostMessage(0x45d, 0, 0);
			}
			break;

		case 0xb9:
			for (int i = 16; i < packet_terrain->count; i++)
			{
				MapPlayerData* mp = field_0x9b8[i];
				if (mp && (mp->flags & 1) == 0 && i < my_main_unit->diplomacy.GetSize())
				{
					buf[0] = 0;

					uint16_t& diplo = my_main_unit->diplomacy[i];

					if ((diplo & 8) != 0 && (packet_terrain->buf[i] & 8) == 0)
						sprintf(buf, "%s %s %s", txt_patch.GetLine(0x48), mp->name, txt_patch.GetLine(0x49));
					else if ((diplo & 8) == 0 && (packet_terrain->buf[i] & 8) != 0)
						sprintf(buf, "%s %s %s", txt_patch.GetLine(0x46), mp->name, txt_patch.GetLine(0x47));

					if (buf[0] != 0)
					{
						msglog.Add(buf, clr_log_sblack, 3000);
						g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
					}

					buf[0] = 0;

					if ((diplo & 0x40) == 0 && (packet_terrain->buf[i] & 0x40) != 0)
						sprintf(buf, "%s %s %s", txt_patch.GetLine(0x94), mp->name, txt_patch.GetLine(0x95));
					else if ((diplo & 0x40) != 0 && (packet_terrain->buf[i] & 0x40) == 0)
						sprintf(buf, "%s %s %s", txt_patch.GetLine(0x92), mp->name, txt_patch.GetLine(0x93));

					if (buf[0] != 0)
					{
						msglog.Add(buf, clr_log_sblack, 3000);
						g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
					}

					buf[0] = 0;

					if ((diplo & 0x20) == 0 && (packet_terrain->buf[i] & 0x20) != 0)
						sprintf(buf, "%s %s %s", txt_patch.GetLine(0x8e), mp->name, txt_patch.GetLine(0x8f));
					else if ((diplo & 0x20) != 0 && (packet_terrain->buf[i] & 0x20) == 0)
						sprintf(buf, "%s %s %s", txt_patch.GetLine(0x90), mp->name, txt_patch.GetLine(0x91));

					if (buf[0] != 0)
					{
						msglog.Add(buf, clr_log_sblack, 3000);
						g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
					}
				}
			}

			my_main_unit->diplomacy.SetSize(packet_terrain->count);
			memcpy(my_main_unit->diplomacy.GetData(), packet_terrain->buf, packet_terrain->count * 2);

			wnd->FUN_00494a9e();

			if (wnd->field_0x144)
			{
				wnd->field_0x144->ReadData(&wnd->field_0x348);
				wnd->field_0x144->VMethod9();
			}
			break;

		case 0xba:
		{
			uint8_t* stats = nullptr;
			int32_t stats_sz = 0;

			ReadKillStats(packet_data->data, packet_data->count, &stats, &stats_sz);
			memcpy(kill_stats.data(), stats, stats_sz);

			delete[] stats;

			if (wnd->field_0x640 != 2)
				wnd->some_struc.FUN_00494982();
		}
			break;

		case 0xbb:
		{
			bool isnull = field_0x4970->FUN_0041ec00() == 0;

			field_0x4970->sub_55ECFE(0);

			uint16_t* dat = packet_terrain->buf + 1;

			for (int i = 0; i < packet_terrain->buf[0]; i++)
			{
				Quest* qwa = field_0x4970->FUN_0055ee42(*dat);
				dat++;

				qwa->CopyFields((QuestData*)dat);
				dat = (uint16_t*)(((QuestData*)dat) + 1);

				field_0x4970->sub_55E24A(qwa);
			}

			if (isnull && field_0x4970->FUN_0041ec00() != 0)
				wnd->PostMessage(0x466, 1, 0);
		}
			break;

		case 0xbc:
			if ((wnd->field_0x418 & 4) != 0)
			{
				VisTav* tavern = (VisTav*)wnd->vis_root->FindChild(1100);
				if (tavern)
				{
					tavern->quest_map->sub_55ECFE(0);

					uint16_t* dat = packet_terrain->buf + 1;
					for (int i = 0; i < packet_terrain->buf[0]; i++)
					{
						Quest* qwa = tavern->quest_map->FUN_0055ee42(*dat);
						dat++;

						qwa->CopyFields((QuestData*)dat);
						dat = (uint16_t*)(((QuestData*)dat) + 1);

						tavern->quest_map->sub_55E24A(qwa);
					}
				}
			}
			break;

		case 0xbf:
			if (packet_join->__field_0xa == sub_4F1D0D(packet_join->name))
			{
				wnd->field_0x458 = 1;
				wnd->field_0x45c = 0;
			}
			else
			{
				FUN_0041cb67(0);
				wnd->field_0x45c = 1;
			}
			break;

		case 0xc0:
		{
			CString fname = CString((const char*)packet_player->preamble) + ".tmp";
			CFile fl;

			try {
				if (packet_player->offset == 0)
				{
					LockSurface2();
					FillRectColorSimple(0, 0, g_ScreenSize.right, g_ScreenSize.bottom, 0);

					CString cstr = CString(TxtFile::AllLines[0x16b]) + " " + (const char*)packet_player->preamble;
					g_font1->DrawTxt(g_ScreenSize.right / 2, g_ScreenSize.bottom / 2, cstr, 8 | 2, clrsh_ShockingBlack);

					UnlockSurface2();

					if (wnd->field_0xbc != 0)
						FlushScreen();

					fl.Open(fname, CFile::modeCreate | CFile::modeWrite);
				}
				else
					fl.Open(fname, CFile::modeWrite);

				fl.Seek(packet_player->offset, 0);
				fl.Write(packet_player->var_data, packet_player->count);
				fl.Close();

				LockSurface2();
				FillRectColorSimple(0, g_ScreenSize.bottom / 2 + 20, g_ScreenSize.right, g_ScreenSize.bottom / 2 + 40, 0);

				g_font1->DrawTxt(g_ScreenSize.right / 2, g_ScreenSize.bottom / 2 + 20, TxtFile::AllLines[0x16c], 2, clrsh_ShockingBlack);

				UnlockSurface2();

				if (wnd->field_0xbc != 0)
					gfxFlushRect(CRect(0, g_ScreenSize.bottom / 2 + 20, g_ScreenSize.right, g_ScreenSize.bottom / 2 + 40));

			}
			catch (...)
			{}

			if (packet_player->offset + packet_player->count < packet_player->total_length)
				FUN_0041cb67(packet_player->offset + packet_player->count);
			else
			{
				
				try
				{
					CFile::Remove((const char*)packet_player->preamble);
				}
				catch(...)
				{}

				try
				{
					CFile::Rename(fname, (const char*)packet_player->preamble);
				}
				catch (...)
				{}

				FUN_0041cb21();
			}
		}
			break;

		case 0xc1:
		{
			NetStru2* conn = g_NetStru1_local.FUN_0041f520();
			if (conn)
				g_CLlDriver.SetLatency(conn->GetUid(), packet_dword->value);
		}
			break;

		case 0xcd:
		{
			DAT_00666a00.RemoveAll();

			char* chpos = strstr((char*)packet_data->data, "CURRENTCOUNT");
			if (!chpos)
				return 0;

			chpos = strchr((char*)packet_data->data, '|');
			if (!chpos)
				return 0;

			chpos++;
			int32_t num = atoi(chpos);

			chpos = strchr((char*)packet_data->data, '\n') + 1;
			chpos = strchr(chpos, '\n') + 1;
			chpos = strchr(chpos, '\n');

			for (int i = 0; i < num; i++)
			{
				while( isprint(*chpos) == 0 )
				{
					chpos++;
				}

				char bbuf[1024];
				char* outpos = bbuf;
				while (*chpos >= 0x20)
				{
					*outpos = *chpos;
					outpos++;
					chpos++;
				}
				*outpos = 0;

				DAT_00666a00.Add( CString('|') + bbuf );
			}


			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{}
			return 1;
		}
			break;

		case 0xce:
		{
			wnd->field_0x3e0.field_0c = *(int32_t*)packet_data->data;
			wnd->some_struc.field_0x130.RemoveAll();

			HatCharId* chars = (HatCharId*)(packet_data->data + 4);
			for (int i = 0; i < packet_data->count / 8; i++)
				wnd->some_struc.field_0x130.Add(chars[i]);
		}
			break;

		case 0xcf:
		{
			HatCharId* chr = (HatCharId*)packet_data->data;

			CString fname;
			fname.Format("%u%u.a2c", chr->id1, chr->id2);

			CFile fl;
			if (fl.Open(fname, CFile::modeCreate | CFile::modeWrite))
			{
				fl.Write(packet_data->data + 8, packet_data->count - 8);
				fl.Close();
			}
			else
				INT_00660f8c = 0x1011;

			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{}
			return 1;
		}
			break;

		case 0xd0:
			wnd->some_struc.field_0x148 = packet_info->field_0xa;
			wnd->some_struc.field_0x14c = packet_info->field_0xe;

			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{}
			return 1;

		}

		if (pkt->id == breakid)
			return 1;
	}
	return 1;
}






TakeDamage::TakeDamage() = default; //45db60
TakeDamage::TakeDamage(const TakeDamage& ref) = default;
TakeDamage::~TakeDamage() = default; //45dd08

TakeDamage::TakeDamage(int32_t _dmg, uint16_t* clr, int32_t unk, int32_t _dx, int32_t _dy, CUnit* ct)
{
	//45db8a
	dmg = _dmg;

	txt.Format("%d", dmg);

	field_0x18 = unk;
	dx = _dx;
	dy = _dy;
	cunit = ct;

	if (!clr)
		color = g_colors_human_pals[ct->field_0x14->color];
	else
		color = clr;

	timestamp = timeGetTime();

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	field_0x20 = wnd->field_0x41c;
}



void TakeDamage::Update()
{
	//45dd1b
	if (field_0x18 == 0)
	{
		dx += 1;
		dy -= 2;
	}
	else
	{
		dx -= 1;
		dy -= 2;
	}
}

int TakeDamage::Draw()
{
	//45dd6d
	uint32_t t = timeGetTime();
	if (t - timestamp > 1000)
		return 0;

	if (cunit->field_0x7c == 0)
		g_font2->DrawTextWithShadow(cunit->screen_x + dx, cunit->screen_y + dy - cunit->field_0x6c, txt, 0, color, 1);
	return 1;
}


void __cdecl TakeDamage::AddDamage(CArray<TakeDamage>* arr, const TakeDamage& dmg)
{
	//45dea1

	for (int i = 0; i < arr->GetSize(); i++)
	{
		TakeDamage& b = arr->ElementAt(i);
		if (b.cunit == dmg.cunit && b.field_0x20 == dmg.field_0x20)
		{
			b.dmg += dmg.dmg;
			b.txt.Format("%d", b.dmg);
			return;
		}
	}

	arr->Add(dmg);
}

void __cdecl TakeDamage::DrawDamages(CArray<TakeDamage>* arr)
{
	//45ddf6
	if (arr->GetSize() == 0)
		return;

	LockSurface2();
	for (int i = 0; i < arr->GetSize(); i++)
	{
		if (arr->ElementAt(i).Draw() == 0)
		{
			arr->RemoveAt(i);
			i--;
		}
	}
	UnlockSurface2();
}

void __cdecl TakeDamage::UpdateDamages(CArray<TakeDamage>* arr)
{
	//45de65
	for (int i = 0; i < arr->GetSize(); i++)
	{
		arr->ElementAt(i).Update();
	}
}



void BigStruct2::FUN_0041b2a4(const char* nam, int32_t token, int32_t playerid)
{
	//41b2a4

	PacketJoin* join = &PacketJoin::Inst;
	join->id = 0x91;
	join->field_0x5 = my_main_unit->index;
	join->to_player_id = 0;

	strcpy(join->name, nam);

	if (playerid == 0)
	{
		join->player_id = 0;
		join->token_id = 0;
		join->field_0xc = 0;
		join->flags = 0;
	}
	else
		join->__field_0xa = field_0x9b8[playerid]->index;

	join->__field_0xa |= token << 8;

	if (g_NetStru1_local.IsActive() == 0)
	{
		if (g_NetStru1_main.IsActive() && g_Server)
		{
			g_NetStru1_main.QueuePacketSend(join);
			g_NetStru1_main.SendAllData();
		}
	}
	else
		g_NetStru1_local.QueuePacketSend(join);
}

void BigStruct2::FUN_0041a8cc()
{
	//41a8cc
	PacketWord* pkt = &PacketWord::Inst;

	pkt->id = 0x36;
	pkt->field_0x5 = my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->value = GetUnit_3f6c()->unit_id;

	g_NetStru1_local.QueuePacketSend(pkt);

	((MainWindow*)AfxGetMainWnd())->field_0x418 &= ~2;
}


int32_t BigStruct2::FUN_0040d7f3()
{
	//40d7f3
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	if (wnd->field_0x640 == 0)
	{
		g_NetStru1_local.sub_51C7CC(g_CmdLatency, 0);

		NetStru2* conn = g_NetStru1_local.FUN_0041f520();
		if (conn)
			g_CLlDriver.SetLatency(conn->GetUid(), g_CmdLatency);
	}

	Packet3Dwords *pkt = &Packet3Dwords::Inst;

	pkt->id = 2;
	strcpy(pkt->field_0x16, wnd->some_struc.character_name);
	memcpy(&pkt->field_0xa, &wnd->some_struc.field_0x8, 8);

	pkt->field_0x12 = g_IsCdPresent | (wnd->some_struc.color << 8) | (wnd->some_struc.FUN_004200f0() << 0x10) | (wnd->some_struc.FUN_00420110() << 0x18);
	pkt->to_player_id = 0;

	field_0x9d0.RemoveAll();

	if (field_0x3f6c)
		delete field_0x3f6c;

	field_0x3f6c = nullptr;

	FUN_00416cf7();

	g_NetStru1_local.QueuePacketSend(pkt);

	if (wnd->field_0x63c != 0)
		g_Server->FUN_0050907e();
	
	uint32_t stime = timeGetTime();
	while (true)
	{
		if (g_NetStru1_local.GetClientsPktNum())
		{
			if (ProcessPackets(0x64) == 0)
				return 0;

			if (field_0x9b8.GetSize() < 2)
			{
				INT_00660f8c = 0x1007;
				return 0;
			}

			return 1;
		}


		MSG msg;
		if (PeekMessageA(&msg, NULL, 0, 0, 1) != 0)
		{
			if (msg.message == WM_QUIT)
				return 0;

			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		if (g_CmdTimeout < timeGetTime() - stime)
			break;

		g_mousept.Update();
		g_NetStru1_local.ProcessConnections();
	}

	INT_00660f8c = 0x1009;
	return 0;
}


int BigStruct2::FUN_0040d4e2()
{
	//40d4e2
	INT_00660f8c = 0;

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	if (wnd->field_0x640 == 1 || wnd->field_0x640 == 2)
	{
		g_NetStru1_main.SetLinkedHLDriver(&g_NetStru1_local);
		g_NetStru1_local.SetLinkedHLDriver(&g_NetStru1_main);
		g_NetStru1_main.ProcessConnections();
		g_NetStru1_local.ProcessConnections();
	}
	else
	{
		CLlNetSession* session = nullptr;
		if (wnd->net_sessions.sessions)
			session = wnd->net_sessions.sessions + wnd->net_sessions.selected_index;
		
		if (g_CLlDriver.Connect(wnd->some_struc.character_name, session) == 0)
		{
			wnd->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, TxtFile::AllLines.GetAt(0x9a), nullptr, 0));
			return 0;
		}
	}

	g_NetStru1_local.ProcessConnections();

	if (FUN_0040d7f3())
		return 1;

	if ((INT_00660f8c & 0xff) < 11)
		wnd->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, TxtFile::AllLines.GetAt((INT_00660f8c & 0xff) + 0xc0), nullptr, 0));
	else
		wnd->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, TxtFile::AllLines.GetAt(INT_00660f8c & 0xff), nullptr, 0));
	return 0;
}



void BigStruct2::FUN_0041cbb8()
{
	//0041cbb8
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	PacketInfo* pkt = &PacketInfo::Inst;
	pkt->id = 0x48;
	pkt->field_0x5 = my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->field_a1 = wnd->some_struc.field_0x4c;
	pkt->field_a2 = wnd->some_struc.field_0x50;
	pkt->field_a3 = wnd->some_struc.field_0x54;
	pkt->field_a4 = wnd->some_struc.field_0x58;
	pkt->field_e1 = wnd->some_struc.main_sphere;
	pkt->field_e2 = wnd->some_struc.face | wnd->some_struc.field_0x34;
	pkt->field_e3 = wnd->some_struc.color;

	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041cc78(const CString& fname)
{
	//41cc78
	PacketData* pkt = &PacketData::Inst;

	pkt->id = 0xbe;
	pkt->field_0x5 = my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;

	CFile fl;
	if (fl.Open(fname, 0))
	{
		int32_t len = fl.GetLength();
		if (len < 0x8000)
		{
			fl.Read(pkt->data, len);
			pkt->count = len;
		}
		fl.Close();
	}

	g_NetStru1_local.QueuePacketSend(pkt);
}



void BigStruct2::FUN_0041d2da(int32_t arg)
{ // 41d2da
	field_0x3f6c = nullptr;

	if (!field_0x9d0.IsEmpty())
	{
		POSITION it = field_0x9d0.GetStartPosition();
		while (it != nullptr)
		{
			uint16_t key;
			CGameObject* obj;
			field_0x9d0.GetNextAssoc(it, key, obj);
			if (obj)
				delete obj;
		}

		field_0x9d0.RemoveAll();
	}

	if (!field_0x9ec.IsEmpty())
	{
		POSITION it = field_0x9ec.GetStartPosition();
		while (it != nullptr)
		{
			uint16_t key;
			CGameObject* obj;
			field_0x9ec.GetNextAssoc(it, key, obj);
			if (obj)
				delete obj;
		}

		field_0x9ec.RemoveAll();
	}

	damage_labels.RemoveAll();

	if (arg == 0)
	{
		for (int i = 1; i < 17; i++)
		{
			MapPlayerData* mp = field_0x9b8[i];
			if (mp && (mp->flags & 1) != 0)
			{
				delete mp;
				field_0x9b8[i] = nullptr;
			}
		}
	}
	else
	{
		for (int i = 1; i < field_0x9b8.GetSize(); i++)
		{
			MapPlayerData* mp = field_0x9b8[i];
			if (mp)
				delete mp;
		}
		if (field_0x9b8.GetSize())
			field_0x9b8.SetSize(1);
	}

	field_0xa08.RemoveAll();

	if (field_0x80)
	{
		delete field_0x80;
		field_0x80 = nullptr;
	}

	while (g_NetStru1_local.ReceiveAnyPacket())
	{}

	if (g_dsound_channels)
	{
		for (int i = 0; i < g_dsound_channel_num; i++)
			g_dsound_channels[i].Clear();
	}

	if (g_SfxArray[14])
		g_SfxArray[14]->Release();

	if (g_SfxArray[16])
		g_SfxArray[16]->Release();

	for (int i = 0; i < g_mapmusicinfos.GetSize(); i++)
		delete g_mapmusicinfos[i];

	g_mapmusicinfos.RemoveAll();
}

int32_t BigStruct2::IsBookOpen()
{ // 41b4b0
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->field_0x418 & 2)
		return FindChild(1000)->FindChild(3) != nullptr;

	return FindChild(3) != nullptr;
}

int32_t BigStruct2::IsBagOpen()
{ // 41b495
	return FindChild(2) != nullptr;
}

void BigStruct2::FUN_0041afcf(CString str)
{ //41afcf
	PacketJoin* pkt = &PacketJoin::Inst;
	pkt->id = 7;
	pkt->field_0x5 = my_main_unit->index;
	pkt->to_player_id = 0;
	strcpy(pkt->name, str);

	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041a735()
{ //41a735
	PacketTerrain* pkt = &PacketTerrain::Inst;
	pkt->id = 0x45;
	pkt->field_0x5 = my_main_unit->index;
	pkt->to_player_id = 0;
	memcpy(pkt->buf, my_main_unit->diplomacy.GetData(), my_main_unit->diplomacy.GetSize() * 2);
	pkt->count = my_main_unit->diplomacy.GetSize();
	g_NetStru1_local.QueuePacketSend(pkt);
}


GfxFile::GfxFile(const char* _fname)
{ //47ae53
	spr = nullptr;
	spr_b = nullptr;
	fname = CString("graphics\\objects\\") + _fname;
	inited = 0;
}

GfxFile::~GfxFile()
{ //47aed9
	Deinit();
}

void GfxFile::Init()
{ // 47af39
	spr = new CSprite256(fname + ".256");
	spr_b = new CSprite256(fname + "b.256");
	spr->ResetPalette(0x10, 2, 1);
	inited = 1;
}

void GfxFile::Deinit()
{ //47b0ab
	if (inited)
	{
		if (spr)
		{
			delete spr;
			spr = nullptr;
		}
		if (spr_b)
		{
			delete spr_b;
			spr_b = nullptr;
		}
		inited = 0;
	}
}




StructureInfo::StructureInfo(const char* _fname)
{ //47eca2
	inited = 0;
	fname = CString("graphics\\structures\\") + _fname;
}


StructureInfo::~StructureInfo()
{ //47ed4e
	Deinit();
	if (anim_mask)
	{
		delete[] anim_mask;
		anim_mask = nullptr;
	}
}


void StructureInfo::Init()
{ //47edd8
	spr = new CSprite256(fname + ".256");
	spr_b = new CSprite256(fname + "b.256");
	spr->ResetPalette(0x10, 2, 1);
	inited = 1;
}


void StructureInfo::Deinit()
{ //47ef52
	if (inited)
	{
		if (spr)
		{
			delete spr;
			spr = nullptr;
		}
		if (spr_b)
		{
			delete spr_b;
			spr_b = nullptr;
		}
		inited = 0;
	}
}


ProjectileInfo::ProjectileInfo(const char* fname, int32_t _a16)
{ //47e1ec
	a16 = _a16;
	filename = fname;
	inited = 0;
}

ProjectileInfo::~ProjectileInfo()
{ //47e50b
	Deinit();
}


void ProjectileInfo::Init()
{ //47e26b
	if (inited)
		return;

	CString str = CString("graphics\\projectiles\\") + filename;
	if (a16)
	{
		sprite1 = new CA16(str + ".16a");
		sprite2 = nullptr;
	}
	else
	{
		sprite1 = new CSprite256(str + ".256");
		sprite2 = new CSprite256(str + "b.256");
	}

	if (palette)
	{
		if (a16)
			sprite1->ResetPalette(16, 4, 0);
		else
		{
			sprite1->ResetPalette(16, 2, 1);
			sprite2->ResetPalette(16, 2, 1);
		}
	}

	inited = 1;
}

void ProjectileInfo::Deinit()
{ //47e56b
	if (!inited)
		return;

	if (sprite1)
	{
		delete sprite1;
		sprite1 = nullptr;
	}

	if (sprite2)
	{
		delete sprite2;
		sprite2 = nullptr;
	}

	inited = 0;
}


void BigStruct2::FUN_0041cad0(int32_t m)
{ //41cad0
	PacketInfo *pkt = &PacketInfo::Inst;
	pkt->id = 4;
	pkt->field_0x5 = my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->field_0xa = m;
	
	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041aaaa(int32_t wim)
{ //41aaaa
	PacketInfo* pkt = &PacketInfo::Inst;
	pkt->id = 0x46;
	pkt->field_0x5 = my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->field_0xa = 1;
	pkt->field_0xe = wim;

	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041abd2(int32_t form)
{ // 41abd2
	PacketInfo* pkt = &PacketInfo::Inst;
	pkt->id = 0x46;
	pkt->field_0x5 = my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->field_0xa = 2;
	pkt->field_0xe = form;

	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041ab74()
{ //41ab74
	PacketInfo* pkt = &PacketInfo::Inst;
	pkt->id = 0x46;
	pkt->field_0x5 = my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->field_0xa = 3;
	pkt->field_0xe = g_settings.AutoCasting;

	g_NetStru1_local.QueuePacketSend(pkt);
}


void BigStruct2::FUN_0041b6e0()
{ //41b6e0
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	int32_t btm = rect.bottom & (~0x1f);
	CVisualObject* obj2 = FindChild(2);
	CVisualObject* obj3 = FindChild(3);

	if (obj2)
		btm = obj2->GetRect().top;

	if (obj3)
		btm = obj3->GetRect().top;

	btm = ((btm - 1) & (~0x1f)) + 0x20;
	field_0x68 = btm / 32;
	
	int32_t vy = FUN_0041f6b0();
	if (vy < view_y)
		view_y = vy;

	if (CVisualObject::FindChild(1200))
		mwnd->field_0x138->FUN_0043be9f();

	field_0x74 = 1;
}

void BigStruct2::FUN_0041b636()
{ //41b636
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if ((mwnd->field_0x418 & 2) == 0)
		RemoveChild(mwnd->field_0xec);
	else
		mwnd->vis_root->FindChild(1000)->RemoveChild(mwnd->field_0xec);

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);

	mwnd->field_0xd4->MsgProc(0x408, 0, 0);

	FUN_0041b6e0();
}


void BigStruct2::FUN_0041b509()
{ // 41b509

	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if (FUN_0041b495() == 0)
		mwnd->field_0xec->SetRect(0, rect.bottom - 85, rect.right, rect.bottom);
	else
		mwnd->field_0xec->SetRect(0, rect.bottom - 175, rect.right, rect.bottom - 90);

	if ((mwnd->field_0x418 & 2) == 0)
		AddChild(mwnd->field_0xec);
	else
	{
		mwnd->field_0xec->SetRect(0, 305, 480, 390);
		mwnd->vis_root->FindChild(1000)->AddChild(mwnd->field_0xec);
	}

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);

	mwnd->field_0xd4->MsgProc(0x408, 0, 0);

	FUN_0041b6e0();
}

void BigStruct2::FUN_0041b40e()
{ //41b40e
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if (IsBookOpen())
		mwnd->field_0xec->SetRect(0, rect.bottom - 85, rect.right, rect.bottom);

	RemoveChild(mwnd->field_0xe8);

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);

	FUN_0041b6e0();
}

void BigStruct2::FUN_0041b381()
{ // 41b381
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if (IsBookOpen())
		mwnd->field_0xec->SetRect(0, rect.bottom - 175, rect.right, rect.bottom - 90);
	
	AddChild(mwnd->field_0xe8);

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);
	FUN_0041b6e0();
}

void BigStruct2::FUN_0041b10f()
{
	Packet* pkt = &Packet::Inst;
	pkt->id = 0x4a;
	pkt->field_0x5 = my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041b064(int32_t arg1, int32_t arg2)
{
	PacketInfo* pkt = &PacketInfo::Inst;
	pkt->id = 0x3f;
	pkt->field_0xa = arg1;
	pkt->field_0xe = arg2;
	pkt->field_0x5 = my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}