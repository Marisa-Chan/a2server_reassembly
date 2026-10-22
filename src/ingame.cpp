#include "ingame.h"
#include <afx.h>
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
#include "resource.h"
#include "sound.h"

#include <cmath>


uint16_t* clr_log_sblack = clrsh_ShockingBlack; //62f88c
uint16_t* clr_log_tblack = clrsh_TechBlack; //62f890
uint16_t* clr_log_tok0 = clrsh_ShockingBlack; //62f894
uint16_t* clr_log_tok2 = clrsh_ShockingBlack; //62f898
uint16_t* clr_log_tok1 = clrsh_ShockingBlack; //62f89c
uint16_t* clr_log_tok3 = clrsh_ShockingBlack; //62f8a0

//476de7
void __cdecl SetMessageColors(uint32_t flag) {
	g_MessageColors = flag;
	if (flag == 0) {
		clr_log_sblack = clrsh_ShockingBlack;
		clr_log_tblack = clrsh_TechBlack;
	} else {
		clr_log_sblack = g_colors_human_pals[2];
		clr_log_tblack = g_colors_human_pals[7];
		clr_log_tok0 = clrsh_GoldenFizz;
		clr_log_tok1 = g_colors_human_pals[1];
		clr_log_tok2 = g_colors_human_pals[4];
		clr_log_tok3 = g_colors_human_pals[0];
	}
}

int INT_6362e8[16] = { 0, 0, 1, 0,   1, 0, 1, 0,   0, 0, -1, 0,   -1, 0, -1, 0 };
int INT_636328[16] = { -1, 0, -1, 0,   0, 0, 1, 0,   1, 0, 1, 0,   0, 0, -1, 0 };

// 62f908 - is the spell offensive, based on the spellbook position.
int32_t dword_62F908[24] = {1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1};

CArray<UnitVFXUnfo*> g_VFX_info; //6610e0
CArray<GfxFile*> g_GfxFiles; //661100
CArray<GfxObject*> g_GfxObjects; //665330
CArray<StructureInfo*> g_StructuresInfo; //661098
CArray<ProjectileInfo*> g_ProjectileInfos; //6610b0
CArray<UnitGfxFile*> g_UnitGfxFiles; //665118

CGamePalette* g_pal_projectiles; //665490

int32_t DAT_00660f68 = 0; //660f68
int32_t DAT_00660f6c = 0; //660f6c
int32_t INT_00660f8c = 0; //660f8c
int32_t INT_00660f90 = 0; //660f90
int32_t DAT_00660f70 = 0; //660f70
CGamePalette* g_pal_projectile_; //665494

CA16* g_spr_smoke[2]; //6610f8

CUnit g_CUnitStatic; //642a48
CStringArray DAT_00666a00; // 666a00



GM_a28::~GM_a28() = default; //4020ad

GM_a28::GM_a28() {
	//401f50
	this->elapsed_time = 0;
	this->field_0x5c = 1;
}

void GM_a28::Add(const char* msg, uint16_t* clr, uint32_t t) {
	//4021fe
	this->field_0x5c = 1;

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	
	CGameFont* fnt = g_font1;
	if (wnd->sessionMode == 3) {
		fnt = g_font2;
	}

	bool restart = this->text.GetSize() == 0;

	CStringArray& lines = fnt->StringArrayForRect(this->out_area, msg);
	for (int i = 0; i < lines.GetSize(); i++) {
		CString& s = lines[i];
		for (int j = 0; j < s.GetLength(); j++) {
			if (s[j] == '\r') {
				s.SetAt(j, ' ');
			}
		}

		this->text.Add(s);
		this->color.Add(clr);
		this->times.Add(t);
	}

	if (restart) {
		this->timestamp = timeGetTime();
		this->elapsed_time = 0;
	}

	while (this->text.GetSize() > this->max_size) {
		this->text.RemoveAt(0);
		this->color.RemoveAt(0);
		this->times.RemoveAt(0);
	}
}

void GM_a28::AddWithCheck(const char* msg, uint16_t* clr, uint32_t t) {
	//4023aa
	this->field_0x5c = 1;

	CStringArray& lines = g_font1->StringArrayForRect(this->out_area, msg);
	for (int i = 0; i < lines.GetSize(); i++) {
		CString& s = lines[i];
		for (int j = 0; j < s.GetLength(); j++) {
			if (s[j] == '\r') {
				s.SetAt(j, ' ');
			}
		}

		if (lines.GetSize() == 1 && this->text.GetSize() != 0) {
			if (this->text[this->text.GetUpperBound()] == s) {
				return;
			}
		}

		this->text.Add(s);
		this->color.Add(clr);
		this->times.Add(t);
	}

	if (this->text.GetSize() == 1) {
		this->timestamp = timeGetTime();
		this->elapsed_time = 0;
	}

	while (this->text.GetSize() > this->max_size) {
		this->text.RemoveAt(0);
		this->color.RemoveAt(0);
		this->times.RemoveAt(0);
	}
}

void GM_a28::Clear() {
	//4021d2
	this->text.RemoveAll();
	this->color.RemoveAll();
	this->times.RemoveAll();
}

void GM_a28::Draw() {
	//40261a

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	int32_t out_x = 8;
	int32_t out_y = 8;
	CGameFont* fnt = g_font1;

	if (wnd->sessionMode == 3) {
		out_x = 0;
		out_y = 220;
		fnt = g_font2;
	}

	for (int i = 0; i < this->text.GetSize(); i++) {
		CString s = this->text[i];

		while (true) {
			int rpos = s.Find('\r');

			if (rpos < 0) {
				fnt->DrawTextWithShadow(out_x, out_y, s, 0, (uint16_t*)this->color[i], 1);
				out_y += 2 + fnt->GetHeight();
				break;
			}
			
			fnt->DrawTextWithShadow(out_x, out_y, s.Left(rpos), 0, (uint16_t*)this->color[i], 1);
			out_y += 2 + fnt->GetHeight();
			s = s.Mid(rpos + 1);
		}
	}
}

void GM_a28::SetRect(const CRect& r) {
	//40212b
	this->out_area = r;

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	if (wnd->sessionMode == 3) {
		this->max_size = (g_ScreenSize.bottom - 480) / (g_font2->GetHeight() + 2) + 14;
	} else {
		this->max_size = (this->out_area.Height() / (g_font1->GetHeight() + 2)) / 2;
	}
}

void GM_a28::Update() {
	//402585
	if (this->text.GetSize() == 0) {
		return;
	}

	uint32_t ts = timeGetTime();

	this->elapsed_time += (ts - this->timestamp);
	this->timestamp = ts;

	if (this->times[0] < this->elapsed_time) {
		this->elapsed_time = 0;

		this->text.RemoveAt(0);
		this->color.RemoveAt(0);
		this->times.RemoveAt(0);
	}
}




//4216b0
void __cdecl ReadKillStats(uint8_t* data, int32_t datasz, uint8_t** out, int32_t* outsz) {
	int32_t sz = *(int32_t*)data;
	data += 4;

	*outsz = sz;

	uint8_t* buf = new uint8_t[sz];

	*out = buf;

	uint8_t* dst = buf;

	int32_t readcount = 4;
	while (readcount < datasz) {
		if ((*data & 0x80) == 0) {
			int num = *data;
			data++;
			readcount++;

			memcpy(dst, data, num);
			dst += num;

			data += num;
			readcount += num;
		} else {
			int num = data[0] & 0x7f;
			int val = data[1];

			memset(dst, val, num);
			dst += num;

			data += 2;
			readcount += 2;
		}
	}
}


int32_t BigStruct2::ProcessPackets(uint8_t breakid) {
	//40da14
	char buf[128];
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	bool loop = true;
	while (loop) {
		Packet* pkt = g_NetStru1_local.ReceiveAnyPacket();

		PacketEnterGame* packet_enter_game = reinterpret_cast<PacketEnterGame*>(pkt);
		PacketJoin* packet_join = reinterpret_cast<PacketJoin*>(pkt);
		PacketItemTransfer* packet_transfer = reinterpret_cast<PacketItemTransfer*>(pkt);
		Packet2Dwords* packet_2dwords = reinterpret_cast<Packet2Dwords*>(pkt);
		PacketDword* packet_dword = reinterpret_cast<PacketDword*>(pkt);
		PacketData* packet_data = reinterpret_cast<PacketData*>(pkt);
		PacketWord* packet_word = reinterpret_cast<PacketWord*>(pkt);
		PacketUnitAction* packet_action = reinterpret_cast<PacketUnitAction*>(pkt);
		PacketUnitUpdate* packet_unit = reinterpret_cast<PacketUnitUpdate*>(pkt);
		PacketRangedAttack* packet_ranged_attack = reinterpret_cast<PacketRangedAttack*>(pkt);
		PacketUnitHp* packet_unit_hp = reinterpret_cast<PacketUnitHp*>(pkt);
		PacketUnitStateVec* packet_state = reinterpret_cast<PacketUnitStateVec*>(pkt);
		PacketSack* packet_sack = reinterpret_cast<PacketSack*>(pkt);
		PacketBuildingHp* packet_building_hp = reinterpret_cast<PacketBuildingHp*>(pkt);
		PacketMoveBroadcast* packet_move = reinterpret_cast<PacketMoveBroadcast*>(pkt);
		PacketAoeZone* packet_aoe = reinterpret_cast<PacketAoeZone*>(pkt);
		PacketEffect* packet_effect = reinterpret_cast<PacketEffect*>(pkt);
		PacketTerrain* packet_terrain = reinterpret_cast<PacketTerrain*>(pkt);
		PacketUnitEquipment* packet_equip = reinterpret_cast<PacketUnitEquipment*>(pkt);
		PacketFileChunk* packet_file = reinterpret_cast<PacketFileChunk*>(pkt);

		if (!pkt) {
			if (breakid != 0 || (wnd->dialogsMask & 1) != 0) {
				continue;
			}
			return 0;
		}

		//printf("Client receive packet 0x%x\n", pkt->id);

		switch (pkt->id) {
		case 3:
			break;

		case 6:
			if (!this->field_0x80) {
				this->FUN_0041c4a1(packet_join->name);
			}

			if (!this->field_0x80) {
				INT_00660f8c = 0x1004;
				return 0;
			}
			INT_00660f90 = 0;
			break;

		case 0xb: {
			INT_00660f8c = packet_2dwords->param1 | 0x1000;

			if ((wnd->dialogsMask & 1) == 0) {
				while( g_NetStru1_local.ReceiveAnyPacket() != nullptr)
				{}
				g_CLlDriver.Close();
				return 0;
			}

			wnd->PostMessage(0x45c, 0, 0);
			g_CLlDriver.Close();
			break;
		}

		case 0xe: {
			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{}
			return 1;
		}

		case 0x64:
			wnd->serverLoopCounter = packet_dword->value;
			break;

		case 0x67: {
			if (wnd->dialogsMask == 1 && this->my_main_unit->gold < packet_2dwords->param1 && packet_2dwords->param2 == 0) {
				sprintf(buf, "%s %d %s", TxtFile::AllLines[0x58], packet_2dwords->param1 - this->my_main_unit->gold, TxtFile::AllLines[0x59]);
				this->msglog.Add(buf, clr_log_tblack, 3000);
			}
			this->my_main_unit->gold = packet_2dwords->param1;

			CUnit* u = this->GetUnit_3f6c();
			if (u) {
				wnd->m_GameSession.AutosaveCharacter();
				if (u->tokenEntries.GetSize()) {
					TokenEntry* gitem = u->tokenEntries[u->tokenEntries.GetUpperBound()];
					if (gitem->IsEmpty()) {
						gitem->count = this->my_main_unit->gold;
					}
				}
			}
		}
			break;

		case 0x69:
		case 0x74: {
			CUnit* unit;
			if (this->field_0x9d0.Lookup(packet_word->value, *(CGameObject**)&unit)) {
				if (pkt->id == 0x74) {
					unit->unitFlags |= 0x80;
				} else {
					unit->unitFlags &= ~0x80;
				}
			}
		}
			break;
			

		case 0x6a: {
			CUnit* unit;
			if (this->field_0x9d0.Lookup(packet_word->value, *(CGameObject**)&unit)) {
				this->field_0x9d0.RemoveKey(packet_word->value);
				delete unit;
			}
		}
			break;

		case 0x6b: {
			CUnit* unit;
			if (!this->field_0x9d0.Lookup(packet_action->unit_id, *(CGameObject**)&unit)) {
				sprintf(buf, "Invalid unit #%d. Command Move.", packet_action->unit_id);
				if (g_EnableTrace) {
					this->msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			} else {
				if (unit->action_segments) {
					switch (unit->action) {
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

					if (g_EnableTrace) {
						this->msglog.Add(buf, clrsh_TechBlack, 5000);
					}
				}

				if (unit->hp > 0) {
					unit->action_segments = packet_action->segments;
					unit->action = 1;
					unit->action_dir = packet_action->angle;
					unit->action_x = INT_6362e8[unit->action_dir] << 8;
					unit->action_y = INT_636328[unit->action_dir] << 8;
				}
			}
		}
			break;

		case 0x6c:
		case 0x6e:
		case 0x6f:
		case 0x70: {

			if (packet_unit->unit_id == 0) {
				if (g_EnableTrace) {
					sprintf(buf, "Invalid index id=%d", packet_unit->unit_id);
					this->msglog.Add(buf, clrsh_TechBlack, 30000);
				}
			} else {
				uint8_t* pdata = packet_unit->data;
				if ((packet_unit->flags_mask & 1) != 0) {
					g_CUnitStatic.hp = *(uint16_t*)pdata;
					pdata += 2;
				}
				if ((packet_unit->flags_mask & 2) != 0) {
					g_CUnitStatic.mp = *(uint16_t*)pdata;
					pdata += 2;
				}
				if ((packet_unit->flags_mask & 4) != 0) {
					g_CUnitStatic.skill_levels[0] = pdata[0];
					g_CUnitStatic.skill_levels[1] = pdata[1];
					g_CUnitStatic.skill_levels[2] = pdata[2];
					g_CUnitStatic.skill_levels[3] = pdata[3];
					g_CUnitStatic.skill_levels[4] = pdata[4];
					pdata += 5;
				}
				if ((packet_unit->flags_mask & 8) != 0) {
					g_CUnitStatic.hp_max = *(int16_t*)pdata;
					pdata += 2;
					g_CUnitStatic.mp_max = *(int16_t*)pdata;
					pdata += 2;
				}
				if ((packet_unit->flags_mask & 0x10) != 0) {
					g_CUnitStatic.last_action = *pdata;
					pdata ++;
					g_CUnitStatic.x_pos = *pdata;
					pdata ++;
					g_CUnitStatic.y_pos = *pdata;
					pdata ++;
				}
				if ((packet_unit->flags_mask & 0x20) != 0) {
					g_CUnitStatic.typeId = *pdata;
					pdata ++;
					g_CUnitStatic.face = *pdata;
					pdata ++;
				}

				uint8_t ownerid = 0;
				if ((packet_unit->flags_mask & 0x40) != 0) {
					ownerid = *pdata;
					pdata ++;
				}

				if ((packet_unit->flags_mask & 0x80) != 0) {
					g_CUnitStatic.serverId = *(uint16_t*)pdata;
					pdata += 2;
				}

				if (ownerid >= this->field_0x9b8.GetSize()) {
					if (g_EnableTrace != 0) {
						sprintf(buf, "Invalid owner. Unit(ext) id=%d", packet_unit->unit_id);
						this->msglog.Add(buf, clrsh_TechBlack, 30000);
					}
				} else {
					MapPlayerData* mpl = this->field_0x9b8[ownerid];
					if (!mpl) {
						if (g_EnableTrace != 0) {
							sprintf(buf, "Invalid owner. Unit(ext) id=%d", packet_unit->unit_id);
							this->msglog.Add(buf, clrsh_TechBlack, 30000);
						}
					} else {
						bool groundunit = false;
						Unit* uni = nullptr;

						if (packet_unit->flags_mask & 0x20) {
							if (g_CUnitStatic.typeId >= 0x20 && g_CUnitStatic.typeId < 0x40) {
								groundunit = true;
							}
						} else if (packet_unit->flags_mask & 0x80) {
							uni = new Unit();
							uni->itemDataID = g_GameDataRes.FindMonsterByServerId(g_CUnitStatic.serverId);
							uni->monster_info = &g_GameDataRes.monsters[uni->itemDataID];
							uni->ApplyMonsterInfo(uni->monster_info->Values());
							uni->server_id = g_CUnitStatic.serverId;

							if (wnd->sessionMode == 2 && g_Server && g_Server->field22_0xd8 != 2) {
								if (g_Server->field22_0xd8 == 1) {
									uni->hp_max = uni->hp_max / 2.0;
								} else {
									uni->hit_values.attack += 50;
									uni->protections.defense += 50;
									uni->hp_max = uni->hp_max * 1.5;
								}
								uni->hp = uni->hp_max;
							}

							if (packet_unit->flags_mask & 8) {
								auto tmp_hp = g_CUnitStatic.hp;
								auto tmp_hpmax = g_CUnitStatic.hp_max;
								g_CUnitStatic.CopyFromUnit(*uni);
								g_CUnitStatic.hp = tmp_hp;
								g_CUnitStatic.hp_max = tmp_hpmax;
							} else {
								g_CUnitStatic.CopyFromUnit(*uni);
							}
						}

						CUnit* ct = nullptr;
						bool ct_exist = true;

						if (this->field_0x9d0.Lookup(packet_unit->unit_id, *(CGameObject**)&ct) == 0) {
							ct_exist = false;

							if ((packet_unit->flags_mask & (0x80 | 0x20)) == 0) {
								break;
							}

							if (groundunit || g_VFX_info[g_CUnitStatic.typeId]->z == 0) {
								ct = new CUnit();
							} else {
								ct = new CAirUnit();
							}

							if (uni) {
								ct->CopyFromUnit(*uni);
							} else {
								ct->typeId = g_CUnitStatic.typeId;
								ct->ApplyFace(g_CUnitStatic.face);
								ct->ReloadSprite();
							}

							this->field_0x9d0.SetAt(packet_unit->unit_id, ct);
							ct->unit_id = packet_unit->unit_id;
						}

						if (packet_unit->flags_mask & 0x40) {
							ct->map_player = mpl;
						}

						if (packet_unit->flags_mask & 1) {
							ct->hp = g_CUnitStatic.hp;
						}

						if (packet_unit->flags_mask & 2) {
							ct->mp = g_CUnitStatic.mp;
						}

						if (packet_unit->flags_mask & 4) {
							ct->skill_levels[0] = g_CUnitStatic.skill_levels[0];
							ct->skill_levels[1] = g_CUnitStatic.skill_levels[1];
							ct->skill_levels[2] = g_CUnitStatic.skill_levels[2];
							ct->skill_levels[3] = g_CUnitStatic.skill_levels[3];
							ct->skill_levels[4] = g_CUnitStatic.skill_levels[4];
						}

						if (packet_unit->flags_mask & 8) {
							ct->hp_max = g_CUnitStatic.hp_max;
							ct->mp_max = g_CUnitStatic.mp_max;
						}

						uint8_t sval = ct->death_state;

						if (packet_unit->flags_mask & 0x10) {
							ct->dir = g_CUnitStatic.last_action;
							ct->x_pos = g_CUnitStatic.x_pos;
							ct->y_pos = g_CUnitStatic.y_pos;
							ct->x_pos = ct->x_pos * 0x100 + 0x80;
							ct->y_pos = ct->y_pos * 0x100 + 0x80;
							ct->x_pos2 = ct->x_pos;
							ct->y_pos2 = ct->y_pos;
							ct->z_pos = 0;
						}

						if (packet_unit->flags_mask & 0x80) {
							ct->serverId = g_CUnitStatic.serverId;
						}

						if (packet_unit->flags_mask & 0x10000) {
							ct->attack = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & 0x20000) {
							ct->hand_dmg_min = pdata[0];
							ct->hand_dmg_spread = pdata[1];
							pdata += 2;
						}

						if (packet_unit->flags_mask & 0x4000) {
							ct->defence = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & 0x8000) {
							ct->absorb = *pdata;
							pdata++;
						}

						if (packet_unit->flags_mask & 0x40000) {
							ct->scan_range = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & 0x2000) {
							ct->carrying_weight_100g = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & 0x1000000) {
							ct->magic_protect[0] = pdata[0];
							ct->magic_protect[1] = pdata[1];
							ct->magic_protect[2] = pdata[2];
							ct->magic_protect[3] = pdata[3];
							ct->magic_protect[4] = pdata[4];
							pdata += 5;
						}

						if (packet_unit->flags_mask & 0x2000000) {
							wnd->m_GameSession.monster_killed = *(uint32_t*)(pdata);
							wnd->m_GameSession.player_killed = *(uint32_t*)(pdata + 4);
							wnd->m_GameSession.death_count = *(uint32_t*)(pdata + 8);
							wnd->m_GameSession.fragCount = *(uint32_t*)(pdata + 12);

							pdata += 16;
						}

						if (packet_unit->flags_mask & 0x80000) {
							ct->speed = *pdata;
							pdata++;
						}

						if (packet_unit->flags_mask & 0x100000) {
							ct->spells = *(int32_t*)pdata;
							pdata += 4;
						}

						if (packet_unit->flags_mask & 0x100000) {
							ct->active_spell = *pdata;
							pdata++;
						}

						if (packet_unit->flags_mask & 0x100) {
							ct->experience_per_sphere[0] = *(int32_t*)pdata;
							pdata += 4;
						}

						if (packet_unit->flags_mask & 0x200) {
							ct->experience_per_sphere[1] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & 0x400) {
							ct->experience_per_sphere[2] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & 0x800) {
							ct->experience_per_sphere[3] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & 0x1000) {
							ct->experience_per_sphere[4] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & 0x20000000) {
							ct->body = pdata[0];
							ct->reaction = pdata[1];
							ct->mind = pdata[2];
							ct->spirit = pdata[3];

							ct->stat_body = pdata[4];
							ct->stat_reaction = pdata[5];
							ct->stat_mind = pdata[6];
							ct->stat_spirit = pdata[7];

							pdata += 8;
						}

						ct->exp_summary = ct->experience_per_sphere[0] + ct->experience_per_sphere[1] + ct->experience_per_sphere[2] + ct->experience_per_sphere[3] + ct->experience_per_sphere[4];

						if ((packet_unit->flags_mask & 0x10) != 0 && ct->unit_id > 0x5fff) {
							ct->unitFlags &= ~0x80;
						}

						if (packet_unit->flags_mask & 0x80000000) {
							memset(ct->str1, 0, sizeof(ct->str1));
							char* dpos = strchr((char*)pdata, '|');
							if (!dpos) {
								memcpy(ct->str1, pdata, 11);
								ct->str2[0] = 0;
							} else {
								memcpy(ct->str1, pdata, dpos - (char*)pdata);
								memcpy(ct->str2, dpos, 12);
							}
							pdata += 12 + 12;
						}

						if (wnd->sessionMode == 2) {
							if (ct->serverId == 0x15) {
								this->field_0x3f6c = ct;
								this->field_0x3f6c->unitFlags |= 0x20;
							}
						} else if (ct->map_player == this->my_main_unit && (ct->unitFlags & 1) != 0) {
							this->field_0x3f6c = ct;
							this->field_0x3f6c->unitFlags |= 0x20;
						}

						if (ct->str1[0] == 0 && ct->serverId != 0) {
							strcpy(ct->str1, txt_npcnames.GetLine(ct->serverId - 1));
						}

						if (packet_unit->flags_mask & 0x800000) {
							ct->effect_timers.RemoveAll();
						}

						if (ct_exist && (wnd->dialogsMask & 0x200) != 0) {
							if ((packet_unit->flags_mask & 0x20) != 0) {
								ct->face = g_CUnitStatic.face;
							}

							this->field_0x3f6c->unitFlags |= 8;
						}

						if ((packet_unit->flags_mask & 0x100000) != 0) {
							ct->availableSpellMask = 0;
							if (ct->map_player == this->my_main_unit && ct->spells != 0) {
								for (int i = 0; i < 32; i++) {
									if ((ct->spells & (1 << i)) != 0) {
										ct->availableSpellMask |= SpellBtB[i];
									}
								}
							}
						}

						if (ct->hp < -600) {
							ct->death_state = 5;
						} else if (ct->hp < -40) {
							ct->death_state = 4;
						} else if (ct->hp < -20) {
							ct->death_state = 3;
						} else if (ct->hp < -10) {
							ct->death_state = 2;
						} else if (ct->hp < 1) {
							ct->death_state = 1;
						} else {
							ct->death_state = 0;
						}

						g_CUnitStatic.death_state = ct->death_state;

						if (ct->serverId != 0 && ct->serverId < 21 && ct->death_state > 1) {
							ScenarioSetVar(ct->serverId + 0x213, 0);
						}

						if (wnd->sessionMode != 2 && (ct->unitFlags & 0x20) != 0) {
							if ((packet_unit->flags_mask & 0x22100000) == 0) {
								if ((packet_unit->flags_mask & 0x1f00) != 0) {
									wnd->m_GameSession.AutosaveTick();
								}
							} else {
								wnd->m_GameSession.AutosaveCharacter();
							}
						}

						switch (ct->death_state) {
						case 0:
							if (sval != 0) {
								ct->action = 0;
								if (ct->unitFlags & 1) {
									ct->ReloadSprite();
								}
							}
							break;

						case 1:
							if (sval == 0) {
								UnitVFXUnfo* dying = g_VFX_info[ g_VFX_info[ct->typeId]->dying ];
								ct->action_segments = dying->dying_phases * 2;
								ct->action = 6;
								ct->action_dir = ct->dir;
								ct->action_phase = 0;
								ct->move_accum_y = 0;
								ct->move_accum_x = 0;
								ct->action_x = ct->x_pos;
								ct->action_y = ct->y_pos;
								ct->PlayHitSound(3);
								ct->ReloadSprite();
							} else if (sval == 1) {
								UnitVFXUnfo* dying = g_VFX_info[g_VFX_info[ct->typeId]->dying];
								ct->action_segments = 4;
								ct->action = 6;
								ct->action_dir = ct->dir;
								ct->action_phase = dying->dying_phases * 2 - 4;
								ct->move_accum_y = 0;
								ct->move_accum_x = 0;
								ct->action_x = ct->x_pos;
								ct->action_y = ct->y_pos;
								ct->PlayHitSound(2);
								ct->ReloadSprite();
							}
							break;

						case 2:
						case 3:
						case 4:
							if (wnd->sessionMode != 2 && ct == this->field_0x3f6c && ct->death_state == 4 && sval < 4) {
								this->msglog.Add(txt_patch.GetLine(0x44), clr_log_sblack, 10000);
								this->msglog.Add(txt_patch.GetLine(0x45), clr_log_sblack, 10000);
							}

							if (sval < 2 && this->my_main_unit->IsAllied(ct->map_player->index) != 0) {
								wnd->m_FameHall.AddDifficultyPoint();
							}

							ct->controlGroupMask = 0;
							ct->effect_timers.RemoveAll();
							ct->transientVisualElements.RemoveAll();
							if (ct->IsSelected() != 0) {
								ct->SetSelected(0);
								this->UpdateSelectionState();
							}

							break;

						case 5:
							ct->controlGroupMask = 0;
							ct->effect_timers.RemoveAll();
							ct->transientVisualElements.RemoveAll();
							if (ct->IsSelected() != 0) {
								ct->SetSelected(0);
								this->UpdateSelectionState();
							}

							if (ct->unit_id < 0x6000) {
								this->field_0x9d0.RemoveKey(packet_unit->unit_id);
								if (ct) {
									delete ct;
								}
							}
							break;
						}
						if (g_CUnitStatic.death_state < 5) {
							if ((wnd->dialogsMask & 1) != 0 && this->field_0x80) {
								ct->UpdateMapPosition();
							}

							if (this->field_0x138 == ct) {
								wnd->vis_charinfo->MsgProc(0x408, 0, 0);
								wnd->vis_sidestatus->MsgProc(0x408, 0, 0);
							}

							if (ct->IsSelected() != 0 && ((ct->unitFlags & 2) != 0 || (packet_unit->flags_mask & 0x100000) != 0)) {
								this->UpdateSelectionState();
							}

							ct->m_bSelectionDirty = 1;
						}
					}
				}
			}
		}
			break;

		case 0x6d: {
			CUnit* ct = nullptr;
			if (this->field_0x9d0.Lookup(packet_action->unit_id, *(CGameObject**)&ct) == 0) {
				if (g_EnableTrace != 0) {
					sprintf(buf, "Invalid unit #%d. Command Turn.", packet_action->unit_id);
					this->msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			} else {
				if (ct->action_segments != 0) {
					if (g_EnableTrace) {
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
						this->msglog.Add(buf, clrsh_TechBlack, 5000);
					}
				}
				if (ct->hp > 0) {
					ct->action_segments = packet_action->segments;
					ct->action = 5;
					ct->action_dir = packet_action->angle;
					ct->turn_angle = ct->dir * 16;
					ct->action_phase = 0;
					ct->move_accum_y = 0;
					ct->move_accum_x = 0;
				}
			}
		}
			break;

		case 0x71: {
			CUnit* ct = nullptr;
			if (this->field_0x9d0.Lookup(packet_action->unit_id, *(CGameObject**)&ct) == 0) {
				if (g_EnableTrace != 0) {
					sprintf(buf, "Invalid unit #%d. Command Attack.", packet_action->unit_id);
					this->msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			} else {
				if (ct->action_segments == 0) {
					UnitVFXUnfo* vfx = g_VFX_info[ct->typeId];
					if (vfx->attack_phases != 0 && ct->hp > 0) {
						ct->action_segments = vfx->attack_anim_frame_cnt;
						ct->action = 3;
						ct->action_dir = packet_action->angle;
						ct->action_phase = 0;
						ct->move_accum_y = 0;
						ct->move_accum_x = 0;
						break;
					}
				}
				
				if (ct->action_segments != 0) {
					if (g_EnableTrace) {
						switch (ct->action) {
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

						this->msglog.Add(buf, clrsh_TechBlack, 5000);
					}
				}
			}
		}
			break;

		case 0x72: {
			CUnit* ct = nullptr;
			if (this->field_0x9d0.Lookup(packet_ranged_attack->unit_id, *(CGameObject**)&ct) == 0) {
				if (g_EnableTrace) {
					sprintf(buf, "Invalid unit #%d. Command Ranged Attack.", packet_ranged_attack->unit_id);
					this->msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			} else if (ct->action_segments == 0 && ct->hp > 0) {
				ct->action_segments = g_VFX_info[ct->typeId]->attack_anim_frame_cnt;
				ct->action = 7;
				ct->action_phase = 0;
				ct->move_accum_y = 0;
				ct->move_accum_x = 0;
				ct->action_target = packet_ranged_attack->target_id;
				ct->action_dir = ct->dir;
			} else if (ct->action_segments != 0) {
				if (g_EnableTrace) {
					switch (ct->action) {
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

					this->msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			}
		}
			break;

		case 0x73: {
			CUnit* ct = nullptr;
			if (this->field_0x9d0.Lookup(packet_unit_hp->unit_id, *(CGameObject**)&ct) == 0) {
				if (g_EnableTrace) {
					sprintf(buf, "Invalid unit #%d. Command Take damage.", packet_unit_hp->unit_id);
					this->msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			} else {
				if (this->flying_hp && packet_unit_hp->hp < ct->hp) {
					bool not_me = ct->map_player != this->my_main_unit;

					int32_t a = ct->GetTileWidth();
					int32_t dx = a * 16;

					if (not_me) {
						dx = -dx;
					}

					TakeDamage::AddDamage(&this->damage_labels, TakeDamage(ct->hp - packet_unit_hp->hp, nullptr, not_me, dx, a * -48, ct) );
				}
				if (ct->hp == packet_unit_hp->hp) {
					ct->PlayHitSound(0);
				} else if (ct->hp > -10) {
					if (ct->hp < ct->hp_max / 2) {
						ct->PlayHitSound(2);
					} else {
						ct->PlayHitSound(1);
					}
				}

				ct->hp = packet_unit_hp->hp;
				if (this->field_0x138 == ct) {
					wnd->vis_charinfo->MsgProc(0x408, 0, 0);
					wnd->vis_sidestatus->MsgProc(0x408, 0, 0);
				}
				ct->m_bSelectionDirty = 1;
			}
		}
			break;
		

		case 0x76: {
			if (wnd->drag_entry && packet_state->mode == wnd->drag_src_tab) {
				g_Cursors[CURSOR_DEFAULT]->Use();
				if (wnd->drag_entry) {
					delete wnd->drag_entry;
				}
				wnd->ResetItemCursor();
			}
			CUnit* ct = nullptr;
			switch (packet_state->mode & 0x7f) {
			case 1:
				if (this->field_0x9d0.Lookup(packet_state->building_id, *(CGameObject**)&ct) != 0) {
					uint8_t *pdata = packet_state->data;
					for (int i = 0; i < 12; i++) {
						if ((packet_state->start_index & (1 << i)) != 0) {
							if (ct->equipmentTokens[i]) {
								delete ct->equipmentTokens[i];
							}

							TokenEntry* itm = new TokenEntry(&pdata, 0);
							if (itm->item_id == 0) {
								delete itm;
								ct->equipmentTokens[i] = nullptr;
							} else {
								itm->slot_code = 1;
								ct->equipmentTokens[i] = itm;

								if (g_EnableTrace) {
									CString logstr = "Invalid item weared " + ct->equipmentTokens[i]->GetSpriteName();

									if (ct->equipmentTokens[i]->HasKnownName() == 0 && g_EnableTrace != 0) {
										CStdioFile f;
										f.Open("error.log", CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite);
										f.SeekToEnd();
										f.WriteString(CTime::GetCurrentTime().Format("%d.%m.%y %H:%M:%S ") + logstr + "\n");
										f.Close();
										wnd->vis_map_context->msglog.Add(logstr, clrsh_TechBlack, 30000);
									}
								}
							}
						}
					}

					ct->unitFlags |= 8;

					if ((wnd->dialogsMask & 2) != 0) {
						wnd->vis_root->FindChild(1000)->MsgProc(0x413, packet_state->mode & 0x7f, 0);
					}

					ct->ReloadSprite();

					ct->m_bSelectionDirty = 1;

					if ((ct->unitFlags & 0x2U) != 0 && wnd->sessionMode != 2) {
						uint8_t* pdata = packet_state->data;
						for (int i = 0; i < 12; i++) {
							if ((packet_state->start_index & (1 << i)) != 0) {
								if (this->field_0x4988[i]) {
									delete this->field_0x4988[i];
								}

								this->field_0x4988[i] = ReadItem(&pdata);
							}
						}

						packet_state->data_size = 0;

						for (int i = 0; i < 12; i++) {
							if (!this->field_0x4988[i]) {
								Item item;
								item.StoreToPacket(packet_state, 0);
							} else {
								this->field_0x4988[i]->StoreToPacket(packet_state, 0);
							}
						}

						CString fname = wnd->m_GameSession.GetSelectedRosterPath();
						WritePlayerFile_4F53EA(fname, nullptr, nullptr, nullptr, packet_state, nullptr, nullptr, 0);
					}
				}
				break;

			case 2: {
				if (this->field_0x9d0.Lookup(packet_state->building_id, *(CGameObject**)&ct) != 0) {
					if ((packet_state->mode & 0x80) == 0) {
						ct->tokenEntries.RemoveAll();
					} else {
						int num = ct->tokenEntries.GetSize();

						if ((ct->unitFlags & 0x20) != 0) {
							num--;
						}

						if (num <= packet_state->start_index) {
							if (g_EnableTrace) {
								CString logstr = "Invalid inventory update! all items ignored.";

								CStdioFile f;
								f.Open("error.log", CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite);
								f.SeekToEnd();
								f.WriteString(CTime::GetCurrentTime().Format("%d.%m.%y %H:%M:%S ") + logstr + "\n");
								f.Close();
								wnd->vis_map_context->msglog.Add(logstr, clrsh_TechBlack, 30000);
							}
							break;
						}

						if (num < (packet_state->start_index + packet_state->entry_count)) {
							if (g_EnableTrace) {
								CString logstr = "Invalid inventory update! some items ignored.";

								CStdioFile f;
								f.Open("error.log", CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite);
								f.SeekToEnd();
								f.WriteString(CTime::GetCurrentTime().Format("%d.%m.%y %H:%M:%S ") + logstr + "\n");
								f.Close();
								wnd->vis_map_context->msglog.Add(logstr, clrsh_TechBlack, 30000);
							}
							packet_state->entry_count = num - packet_state->start_index;
						}

						for (int i = 0; i < packet_state->entry_count; i++) {
							if ((i + packet_state->start_index) < ct->tokenEntries.GetSize()) {
								TokenEntry* o = ct->tokenEntries[i + packet_state->start_index];
								if (o) {
									delete o;
									ct->tokenEntries[i + packet_state->start_index] = nullptr;
								}
							}
						}

						ct->tokenEntries.RemoveAt(packet_state->start_index, packet_state->entry_count);

						if ((ct->unitFlags & 0x20) != 0) {
							ct->tokenEntries.RemoveAt(ct->tokenEntries.GetUpperBound());
						}
					}

					uint8_t* pdata = packet_state->data;
					for (int i = 0; i < packet_state->entry_count; i++) {
						TokenEntry* obj = new TokenEntry(&pdata, 0);
						obj->slot_code = 2;

						if (obj->HasKnownName()) {
							ct->tokenEntries.InsertAt(i + packet_state->start_index, obj, 1);
						}

						if (g_EnableTrace) {
							CString logstr = "Invalid item in inventory " + obj->GetSpriteName();

							CStdioFile f;
							f.Open("error.log", CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite);
							f.SeekToEnd();
							f.WriteString(CTime::GetCurrentTime().Format("%d.%m.%y %H:%M:%S ") + logstr + "\n");
							f.Close();
							wnd->vis_map_context->msglog.Add(logstr, clrsh_TechBlack, 30000);
						}
						
						if ((obj->flg & 0x40) != 0) {
							obj->flg = obj->flg & 0xbf;

							char* itmname = nullptr;
							unk_660DA8.Lookup(obj->item_id, *(void **)&itmname);

							if (obj->count < 2) {
								sprintf(buf, "%s %s", TxtFile::AllLines[85], itmname);
							} else {
								sprintf(buf, "%s %s (%s %d %s)", TxtFile::AllLines[85], itmname, TxtFile::AllLines[86], obj->count, TxtFile::AllLines[87]);
							}
							this->msglog.Add(buf, clr_log_tblack, 3000);
						}
					}

					if ((ct->unitFlags & 0x20) != 0) {
						if (wnd->sessionMode != 2) {
							uint8_t *pdata = packet_state->data;
							for (int i = 0; i < packet_state->entry_count; i++) {
								if (i + packet_state->start_index < this->field_0x4974.GetSize()) {
									Item* itm = this->field_0x4974[i + packet_state->start_index];
									if (itm) {
										delete itm;
									}
								}

								Item * itm = ReadItem(&pdata);
								this->field_0x4974.SetAtGrow(i + packet_state->start_index, itm);
							}

							if ((packet_state->mode & 0x80) == 0) {
								this->field_0x4974.SetSize(packet_state->entry_count);
							}
							
							packet_state->data_size = 0;

							for (int i = 0; i < this->field_0x4974.GetSize(); i++) {
								this->field_0x4974[i]->StoreToPacket(packet_state, 0);
							}


							CString fname = wnd->m_GameSession.GetSelectedRosterPath();
							WritePlayerFile_4F53EA(fname, nullptr, nullptr, nullptr, packet_state, nullptr, nullptr, 0);
						}

						TokenEntry* gt = new TokenEntry(0);
						gt->item_id = 0xffff;
						gt->count = this->my_main_unit->gold;
						gt->flg = 0;
						gt->mods_count = 0;
						gt->slot_code = 2;
						ct->tokenEntries.Add(gt);
					}

					this->UpdateSelectionState();

					if (wnd->dialogsMask & 2) {
						wnd->vis_root->FindChild(1000)->MsgProc(0x413, packet_state->mode & 0x7f, 0);
					} else {
						if (this->field_0x140 == 1 && this->field_0x138 == ct) {
							wnd->vis_invtype1->VMethod33(ct);
						}
					}
					
				}
			}
				break;

			case 4:
			case 5:
			case 6:
			case 7:
			case 8: {
				CArray<TokenEntry*> tmp;

				uint8_t* pdata = packet_state->data;

				for (int i = 0; i < packet_state->entry_count; i++) {
					TokenEntry* d0 = nullptr;
					if ((packet_state->mode & 0x7f) == 4) {
						d0 = new TokenEntry(&pdata, 1);
					} else {
						d0 = new TokenEntry(&pdata, 0);
					}
					
					if (d0->HasKnownName()) {
						tmp.Add(d0);
					} else {
						if (g_EnableTrace) {
							CString logstr = "Invalid item in shop " + d0->GetSpriteName();

							CStdioFile f;
							f.Open("error.log", CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite);
							f.SeekToEnd();
							f.WriteString(CTime::GetCurrentTime().Format("%d.%m.%y %H:%M:%S ") + logstr + "\n");
							f.Close();
							wnd->vis_map_context->msglog.Add(logstr, clrsh_TechBlack, 30000);
						}
					}
				}

				if ((wnd->dialogsMask & 2) == 0) {
					for (int i = 0; i < tmp.GetSize(); i++) {
						if (tmp[i]) {
							delete tmp[i];
						}
					}
					tmp.RemoveAll();
				} else {
					wnd->vis_root->FindChild(1000)->MsgProc(0x413, packet_state->mode & 0x7f, (uint32_t)&tmp);
				}
			}
				break;

			case 9: {
				if ((wnd->dialogsMask & 4) != 0) {
					VisTav* tavern = (VisTav*) wnd->vis_root->FindChild(0x44c);
					uint8_t* pdata = packet_state->data;
					for (int i = 0; i < packet_state->entry_count; i++) {
						TokenEntry* d0 = new TokenEntry(&pdata, 0);
						if (d0->HasKnownName() || d0->item_id > 0xfff0) {
							tavern->rewards.Add(d0);
						//WAT ?  else - delete ???
						}
					}
				}
			}
				break;
			}
		}
			break;


		case 0x7a: {
			CGameObject* ct = nullptr;
			if (this->field_0x9d0.Lookup(packet_sack->unit_id, ct)) {
				ct->typeId = packet_sack->type_id;
				if (ct->typeId > 5) {
					ct->typeId = 5;
				}
			} else {
				CBackPack* pak = new CBackPack();
				this->field_0x9d0[packet_sack->unit_id] = pak;

				pak->SetVals(packet_sack->unit_id, 1, packet_sack->xpos, packet_sack->ypos, 0, 0, 0, 0, 1);

				pak->map_player = this->my_main_unit;
				pak->typeId = packet_sack->type_id;
				if (pak->typeId > 5) {
					pak->typeId = 5;
				}

				if ((wnd->dialogsMask & 1) != 0 && this->field_0x80 != nullptr) {
					pak->UpdateMapPosition();
				}
			}

		}
			break;

		case 0x82: {
			CUnit* ct = nullptr;
			if (this->field_0x9d0.Lookup(packet_building_hp->building_id, *(CGameObject**)&ct)) {
				ct->hp = packet_building_hp->hp;
				if (this->field_0x138 == ct) {
					wnd->vis_charinfo->MsgProc(0x408, 0, 0);
					wnd->vis_sidestatus->MsgProc(0x408, 0, 0);
				}
			}
		}
			break;

		case 0x83:
			wnd->PostMessage(0x42a, packet_2dwords->param1, 0);
			break;

		case 0x84:
			wnd->PostMessage(0x42b, packet_2dwords->param1, packet_2dwords->param2);
			break;

		case 0x86:
			if ((packet_move->type_id & 1) == 0 || packet_move->type_id >= g_ProjectileInfos.GetSize() || g_ProjectileInfos[packet_move->type_id] == nullptr) {
				CUnit* ct = nullptr;
				if (this->field_0x9d0.Lookup(packet_move->unit_id, *(CGameObject**)&ct) == 0) {
					if (g_EnableTrace != 0) {
						sprintf(buf, "Invalid unit #%d. Command Cast spell.", packet_move->unit_id);
						this->msglog.Add(buf, clrsh_TechBlack, 5000);
					}
				} else if (ct->action_segments == 0 && g_VFX_info[ct->typeId]->attack_phases != 0) {
					ct->action_segments = g_VFX_info[ct->typeId]->attack_anim_frame_cnt;
					ct->action = 8;
					ct->action_phase = 0;

					if (packet_move->type_id < g_ProjectileInfos.GetSize() && g_ProjectileInfos[packet_move->type_id] != nullptr) {
						if (g_ProjectileInfos[packet_move->type_id]->homing == 0) {
							ct->action_x = packet_move->x * 0x100 + 0x80;
							ct->action_y = packet_move->y * 0x100 + 0x80;
							ct->action_target = 0;
						} else {
							ct->action_target = packet_move->x | (packet_move->y << 8);
						}
					} else {
						ct->action_x = ct->x_pos;
						ct->action_y = ct->y_pos;
						ct->action_target = 0;
					}
					ct->action_spell = packet_move->type_id;
				}
			} else {
				CProjectile* pj = new CProjectile();
				pj->typeId = packet_move->type_id;
				pj->x_pos = packet_move->x * 0x100 + 0x80;
				pj->y_pos = packet_move->y * 0x100 + 0x80;
				pj->z_pos = 0;
				pj->x_pos2 = pj->x_pos;
				pj->y_pos2 = pj->y_pos;
				pj->action_x = pj->x_pos;
				pj->action_y = pj->y_pos;
				pj->action_z = pj->z_pos;
				pj->action_target = 0;
				pj->action_phase = -1;
				pj->map_player = this->my_main_unit;
				pj->action_segments = packet_move->segments;
				pj->action = 1;
				pj->pMapObject = this;

				pj->UpdateMapPosition();

				this->field_0x9ec[this->field_0xa24] = pj;
				this->field_0xa24++;

				SfxSample* snd = g_SfxArray[packet_move->type_id + 500];
				if (packet_move->type_id != 23 && snd) {
					int32_t vol = 0;
					int32_t pan = 0;
					this->FUN_0041b7b7(pj->x_pos, pj->y_pos, &vol, &pan);

					int priority = (10000 - ::abs(vol)) / 100;

					snd->Play(vol + g_SoundSettings.sfx_pos, pan, 0, priority, 0);
				}
			}
			break;

		case 0x87:
			this->FUN_0041c74b(packet_aoe->field_0xb,
							packet_aoe->field_0xc,
							packet_aoe->field_0xd,
							packet_aoe->field_0xe,
							(packet_aoe->type_id / 2) - 4,
							packet_aoe->data,
							packet_aoe->set_flag);
			if (packet_aoe->set_flag == 0) {
				this->field_0xe0 = 1;
			}
			break;

		case 0x88: {
			CGameObject* ct = nullptr;
			if (this->field_0x9d0.Lookup(packet_effect->unit_id, ct) != 0) {
				int32_t idx = ct->FindEffectTimer(packet_effect->effect_type);
				if (idx < 0) {
					ct->effect_timers.Add((packet_effect->effect_type << 0x10) | 0xffff);
				} else {
					ct->effect_timers[idx] = (packet_effect->effect_type << 0x10) | 0xffff;
				}

				ct->m_bSelectionDirty = 1;
			}
		}
			break;

		case 0x89: {
			CGameObject* ct = nullptr;
			if (this->field_0x9d0.Lookup(packet_effect->unit_id, ct) != 0) {
				int32_t idx = ct->FindEffectTimer(packet_effect->effect_type);
				if (idx > -1) {
					ct->effect_timers.RemoveAt(idx);
				}

				ct->m_bSelectionDirty = 1;
			}
		}
		break;

		case 0x8a: {
			CGameObject* ct = nullptr;
			this->field_0x9d0.Lookup(packet_terrain->buf[0], ct);
			if (ct->action_segments == 0 && g_VFX_info[ct->typeId]->attack_phases != 0) {
				ct->action_segments = g_VFX_info[ct->typeId]->attack_anim_frame_cnt;
				ct->action = 8;
				ct->action_phase = 0;

				ct->actionTargets.RemoveAll();
				for (int i = 1; i < packet_terrain->count; i++) {
					ct->actionTargets.Add(packet_terrain->buf[i]);
				}

				ct->action_target = packet_terrain->buf[1];
				ct->action_spell = 0x1e;
			}
		}
		break;

		case 0x8b: {
			CProjectile* pj = new CProjectile();
			pj->typeId = packet_move->type_id;
			pj->x_pos = (packet_move->unit_id & 0xFF) * 0x100 + 0x80;
			pj->y_pos = (packet_move->unit_id >> 8) * 0x100 + 0x80;
			pj->z_pos = 0;
			pj->x_pos2 = pj->x_pos;
			pj->y_pos2 = pj->y_pos;
			
			if (g_ProjectileInfos[packet_move->type_id]->homing == 0) {
				pj->action_x = packet_move->x * 0x100 + 0x80;
				pj->action_y = packet_move->y * 0x100 + 0x80;
				pj->action_target = 0;
			} else {
				pj->action_target = packet_move->x | (packet_move->y << 8);
			}

			pj->action_z = pj->z_pos;
			pj->action_phase = -1;
			pj->map_player = this->my_main_unit;
			pj->action_segments = packet_move->segments;
			pj->action = 1;
			pj->pMapObject = this;

			pj->UpdateMapPosition();
			
			this->field_0x9ec[this->field_0xa24] = pj;
			this->field_0xa24++;

			SfxSample* snd = g_SfxArray[packet_move->type_id + 500];
			if (packet_move->type_id != 23 && snd) {
				int32_t vol = 0;
				int32_t pan = 0;
				this->FUN_0041b7b7(pj->x_pos, pj->y_pos, &vol, &pan);

				int priority = (10000 - ::abs(vol)) / 100;

				snd->Play(vol + g_SoundSettings.sfx_pos, pan, 0, priority, 0);
			}
		}
		break;

		case 0x8c: {
			CProjectile* pj = new CProjectile();
			pj->typeId = 0x1e;
			pj->x_pos = (packet_terrain->buf[0] & 0xFF) * 0x100 + 0x80;
			pj->y_pos = (packet_terrain->buf[0] & 0xFF00) /* * 0x100*/ + 0x80;
			pj->z_pos = 0;
			pj->x_pos2 = pj->x_pos;
			pj->y_pos2 = pj->y_pos;

			pj->actionTargets.RemoveAll();

			for (int i = 1; i < packet_terrain->count; i++) {
				pj->actionTargets.Add(packet_terrain->buf[i]);
			}

			pj->action_target = packet_terrain->buf[1];
			pj->action_z = pj->z_pos;
			pj->action_phase = -1;
			pj->map_player = this->my_main_unit;
			pj->action_segments = 13;
			pj->action = 1;
			pj->pMapObject = this;

			pj->UpdateMapPosition();

			this->field_0x9ec[this->field_0xa24] = pj;
			this->field_0xa24++;

			SfxSample* snd = g_SfxArray[pj->typeId + 500];
			if (pj->typeId != 23 && snd) {
				int32_t vol = 0;
				int32_t pan = 0;
				this->FUN_0041b7b7(pj->x_pos, pj->y_pos, &vol, &pan);

				int priority = (10000 - ::abs(vol)) / 100;

				snd->Play(vol + g_SoundSettings.sfx_pos, pan, 0, priority, 0);
			}
		}
		break;

		case 0x91: {
			if (packet_join->player_id == 0 || packet_join->player_id >= this->field_0x9b8.GetSize() || this->field_0x9b8[packet_join->player_id] == nullptr) {
				this->msglog.Add(packet_join->name, clr_log_sblack, 10000);
			} else if (this->my_main_unit->KnowsPlayer(packet_join->player_id) == 0 || packet_join->token_id == 4) {
				MapPlayerData* dat = this->field_0x9b8[packet_join->player_id];

				if (g_MessageColors == 0 || (wnd->dialogsMask & 1) == 0) {
					this->msglog.Add(CString(dat->name) + ": " + packet_join->name, g_colors_human_pals[dat->color], 10000);
				} else {
					uint16_t* clr = nullptr;
					switch (packet_join->token_id) {
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

					this->msglog.Add(CString(dat->name) + ": " + packet_join->name, clr, 10000);
				}

				g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				
			}
			wnd->vis_root->MsgProc(0x45f, 0, 0);
		}
		break;

		case 0x92: {
			switch (packet_2dwords->param1) {
			case 1:
				if (g_EnableTrace) {
					this->msglog.Add(TxtFile::AllLines[0x81], clr_log_sblack, 3000);
				}
				break;

			case 2: {
				int32_t texid = packet_2dwords->param2 & 0xffff;
				int32_t serv_id = (packet_2dwords->param2 >> 16) & 0xffff;

				CString txt;

				if ((this->field_0x3f6c->unitFlags & 2) == 0) {
					txt = TxtFile::AllLines[texid + 0x81];
				} else {
					txt = TxtFile::AllLines[texid + 0x86];
				}

				if (wnd->sessionMode == 2) {
					CUnit* ct = this->FUN_0041df23(serv_id);
					if (ct) {
						if ((ct->unitFlags & 2) == 0) {
							txt = TxtFile::AllLines[texid + 0x81] + CString(" (") + ct->str1 + ")";
						} else {
							txt = TxtFile::AllLines[texid + 0x86] + CString(" (") + ct->str1 + ")";
						}
					}
				}

				this->msglog.Add(txt, clr_log_sblack, 3000);

				if ((wnd->dialogsMask & 1) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
			}
				break;

			case 3:
				if (packet_2dwords->param2 != this->my_main_unit->index) {
					sprintf(buf, "%s %s %s", TxtFile::AllLines[0xcc], this->field_0x9b8[packet_2dwords->param2]->name, TxtFile::AllLines[0xcd]);
					this->msglog.Add(buf, clr_log_sblack, 5000);
				}
				break;

			case 4:
				if (packet_2dwords->param2 == this->my_main_unit->index) {
					sprintf(buf, "%s %s %s", TxtFile::AllLines[0xd0], this->field_0x9b8[packet_2dwords->param2]->name, TxtFile::AllLines[0xd1]);
				} else {
					sprintf(buf, "%s %s %s", TxtFile::AllLines[0xce], this->field_0x9b8[packet_2dwords->param2]->name, TxtFile::AllLines[0xcf]);
				}
				this->msglog.Add(buf, clr_log_sblack, 5000);
				break;

			case 5:
				sprintf(buf, "%s %s %s", TxtFile::AllLines[0xdd], this->field_0x9b8[packet_2dwords->param2]->name, TxtFile::AllLines[0xde]);
				this->msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->dialogsMask & 1) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case 6:
				sprintf(buf, "%s %s %s", TxtFile::AllLines[0xdf], this->field_0x9b8[packet_2dwords->param2]->name, TxtFile::AllLines[0xe0]);
				this->msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->dialogsMask & 1) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case 7:
				sprintf(buf, "%s %s %s", TxtFile::AllLines[0xe1], this->field_0x9b8[packet_2dwords->param2]->name, TxtFile::AllLines[0xe2]);
				this->msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->dialogsMask & 1) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case 8:
				sprintf(buf, "%s %d %s", txt_patch.GetLine(0x53), packet_2dwords->param2, txt_patch.GetLine(0x54));
				this->msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->dialogsMask & 1) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case 0x10:
				this->msglog.Add(txt_patch.GetLine(0x58), clr_log_sblack, 5000);

				if ((wnd->dialogsMask & 1) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case 0x20:
				this->msglog.Add(txt_patch.GetLine(0x59), clr_log_sblack, 5000);
				this->msglog.Add(txt_patch.GetLine(0x5a), clr_log_sblack, 5000);

				if ((wnd->dialogsMask & 1) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case 0x40:
				this->msglog.Add(txt_patch.GetLine(0x5b), clr_log_sblack, 5000);

				if ((wnd->dialogsMask & 1) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case 0x80:
				this->msglog.Add(txt_patch.GetLine(0x5c), clr_log_sblack, 5000);

				if ((wnd->dialogsMask & 1) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;
			}
		}
		break;

		case 0x93: {
			this->msglog.Add(txt_patch.GetLine(0x4e) + CString(" ") + packet_join->name + CString(" ") + txt_patch.GetLine(0x4f), clr_log_sblack, 5000);
			g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);

			if (packet_join->__field_0xa == this->my_main_unit->index) {
				if ((wnd->dialogsMask & 1) == 0) {
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
			if (packet_2dwords->param1 != 0 && 
				packet_2dwords->param1 < this->field_0x9b8.GetSize() && this->field_0x9b8[packet_2dwords->param1] != nullptr &&
				packet_2dwords->param2 != 0 &&
				packet_2dwords->param2 < this->field_0x9b8.GetSize() && this->field_0x9b8[packet_2dwords->param2] != nullptr) {
				CString str;
				str.Format("%s %s %s %s %s", txt_patch.GetLine(0x50), this->field_0x9b8[packet_2dwords->param1]->name, txt_patch.GetLine(0x51), this->field_0x9b8[packet_2dwords->param2]->name, txt_patch.GetLine(0x52));
				this->msglog.Add(str, clr_log_sblack, 5000);
			}
			break;

		case 0x96: {
			bool isin = packet_join->player_id < this->field_0x9b8.GetSize();
			if (isin) {
				if (this->field_0x9b8[packet_join->player_id] == nullptr) {
					isin = false;
				}
			}
			MapPlayerData* mp = nullptr;
			if (!isin) {
				mp = new MapPlayerData(packet_join->player_id, packet_join->color);
				mp->field2_0x8 = packet_join->token_id;
			} else {
				mp = this->field_0x9b8[packet_join->player_id];
			}

			if ((packet_join->flags & 1) == 0) {
				mp->flags &= ~1;
			} else {
				mp->flags |= 1;
			}

			strncpy(mp->name, packet_join->name, 31);
			mp->name[31] = 0;

			if (this->field_0x9b8.GetSize() == 1) {
				wnd->m_GameSession.phased_in = (packet_join->flags & 2) != 0;
				this->my_main_unit = mp;
				this->my_main_unit->diplomacy.SetAtGrow(0, 0);
				this->my_main_unit->diplomacy.SetAtGrow(mp->index, 0x3a);
			} else if ((mp->flags & 1) == 0) {
				if (mp != this->my_main_unit) {
					this->my_main_unit->diplomacy.SetAtGrow(mp->index, 0);
				}
			} else {
				this->my_main_unit->diplomacy.SetAtGrow(mp->index, 1);
			}

			this->field_0x9b8.SetAtGrow(mp->index, mp);

			wnd->ClearDiplomacyEntries();

			if (wnd->vis_msgbox) {
				wnd->vis_msgbox->ReadData(&wnd->diplomacy_entries);
				wnd->vis_msgbox->VMethod9();
			}

			wnd->vis_root->MsgProc(0x460, 0, 0);
		}
			break;

		case 0x97: {
			if (packet_2dwords->param1 == this->my_main_unit->index) {
				wnd->PostMessage(0x45c, 0, 0);
				g_CLlDriver.Close();
				break;
			}

			CArray<uint16_t> tmp;
			for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
				uint16_t key;
				CGameObject* obj;
				this->field_0x9d0.GetNextAssoc(it, key, obj);
				
				if (obj->map_player == this->field_0x9b8[packet_2dwords->param1]) {
					tmp.Add(key);
				}
			}

			for (int i = 0; i < tmp.GetSize(); i++) {
				CGameObject* obj;
				this->field_0x9d0.Lookup(tmp[i], obj);
				this->field_0x9d0.RemoveKey(tmp[i]);

				if (obj->IsSelected() != 0) {
					this->field_0x9b4 = 0;
					wnd->vis_spellbook->FUN_004caa69();
					wnd->vis_ordertoolbar->MsgProc(0x40b, 0, 0);
				}

				delete obj;
			}

			this->UpdateSelectionState();

			this->field_0x9b8[packet_2dwords->param1] = nullptr; //WAT ? not delete?
			wnd->vis_root->MsgProc(0x460, 0, 0);

			wnd->ClearDiplomacyEntries();

			if (wnd->vis_msgbox) {
				wnd->vis_msgbox->ReadData(&wnd->diplomacy_entries);
				wnd->vis_msgbox->VMethod9();
			}
		}
		break;

		case 0x9b: {
			CWordArray tmp;
			tmp.SetSize(packet_terrain->count);
			memcpy(tmp.GetData(), packet_terrain->buf, packet_terrain->count * 2);
			this->FUN_0041c630(&tmp);
		}
			break;

		case 0x9c: {
			CUnit* ct = nullptr;
			if (packet_equip->unit_id != 0 && this->field_0x9d0.Lookup(packet_equip->unit_id, *(CGameObject**)&ct) != 0) {
				int pr = 0;
				for (int i = 0; i < 12; i++) {
					if ((packet_equip->flags & (1 << i)) != 0) {
						if (ct->equipmentTokens[i]) {
							delete ct->equipmentTokens[i];
						}

						if (packet_equip->prop[pr] == 0) {
							ct->equipmentTokens[i] = nullptr;
						} else {
							TokenEntry* itm = new TokenEntry(0);
							itm->item_id = packet_equip->prop[pr];
							itm->count = 1;
							itm->flg = 0;
							itm->mods_count = 0;
							itm->mods_size = 0;
							itm->slot_code = 1;

							ct->equipmentTokens[i] = itm;
						}
						pr++;
					}
				}

				ct->unitFlags |= 8;
				ct->ReloadSprite();
			}
		}
			break;

		case 0xaa: {
			switch(packet_2dwords->param1) {
			case 0:
				INT_00660f74 = 0;
				break;

			case 1: {
				INT_00660f74 = 1;
				uint16_t* landscape = this->field_0x80->GetLandscape();

				for (int i = 0; i < this->field_0x80->GetWidth(); i++) {
					for (int j = 0; j < this->field_0x80->GetHeight(); j++) {
						landscape[i + j * field_0x80->GetWidth()] |= 0xc000;
					}
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
			if (wnd->pending_game_mode == 0) {
				this->MsgProc(0x406, packet_2dwords->param1 - (this->view_tiles_x / 2), packet_2dwords->param2 - (this->view_tiles_y / 2));
			}
			break;

		case 0xae:
			this->MsgProc(0x484, (uint32_t)&(packet_join->__field_0xa), 0); ///???????????
			break;

		case 0xaf:
			if (wnd->sessionMode == 0) {
				if ((wnd->dialogsMask & 1) == 0) {
					while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
					{}
					g_CLlDriver.Close();
					return 0;
				} else {
					g_CLlDriver.Close();
					if (pkt) {
						delete pkt; // WAT!?
					}
					return 1;
				}
			}
			break;

		case 0xb3:
			this->kill_stats[packet_dword->value & 0xffff] = (packet_dword->value >> 16) & 0xff;

			if (wnd->sessionMode != 2) {
				wnd->m_GameSession.SaveKillStats();
			}
			break;

		case 0xb4:
			wnd->PostMessage(0x433, 255, packet_2dwords->param1);
			break;

		case 0xb5:
			wnd->PostMessage(0x430, 0, 0);
			break;

		case 0xb6:
			wnd->PostMessage(0x433, packet_2dwords->param1, 0);
			break;

		case 0xb7:
			if (wnd->sessionMode == 0) {
				wnd->inside_building = 1;
				wnd->PostMessage(0x45e, 0, 0);
			}
			break;

		case 0xb8:
			if (wnd->sessionMode == 0) {
				if ((wnd->dialogsMask & 2) != 0) {
					VisShop* shop = (VisShop*)wnd->vis_root->FindChild(1000);
					shop->FUN_004bcd02();
					wnd->vis_root->RemoveChild(shop);
				}

				if ((wnd->dialogsMask & 4) != 0) {
					VisTav* tavern = (VisTav*)wnd->vis_root->FindChild(1100);
					tavern->FUN_0049edec();
					wnd->vis_root->RemoveChild(tavern);
				}
				wnd->vis_root->MsgProc(0x446, 0, 0);
				wnd->inside_building = 0;
				wnd->PostMessage(0x45d, 0, 0);
			}
			break;

		case 0xb9:
			for (int i = 16; i < packet_terrain->count; i++) {
				MapPlayerData* mp = this->field_0x9b8[i];
				if (mp && (mp->flags & 1) == 0 && i < this->my_main_unit->diplomacy.GetSize()) {
					buf[0] = 0;

					uint16_t& diplo = this->my_main_unit->diplomacy[i];

					if ((diplo & 8) != 0 && (packet_terrain->buf[i] & 8) == 0) {
						sprintf(buf, "%s %s %s", txt_patch.GetLine(0x48), mp->name, txt_patch.GetLine(0x49));
					} else if ((diplo & 8) == 0 && (packet_terrain->buf[i] & 8) != 0) {
						sprintf(buf, "%s %s %s", txt_patch.GetLine(0x46), mp->name, txt_patch.GetLine(0x47));
					}

					if (buf[0] != 0) {
						this->msglog.Add(buf, clr_log_sblack, 3000);
						g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
					}

					buf[0] = 0;

					if ((diplo & 0x40) == 0 && (packet_terrain->buf[i] & 0x40) != 0) {
						sprintf(buf, "%s %s %s", TxtFile::AllLines[0x94], mp->name, TxtFile::AllLines[0x95]);
					} else if ((diplo & 0x40) != 0 && (packet_terrain->buf[i] & 0x40) == 0) {
						sprintf(buf, "%s %s %s", TxtFile::AllLines[0x92], mp->name, TxtFile::AllLines[0x93]);
					}

					if (buf[0] != 0) {
						this->msglog.Add(buf, clr_log_sblack, 3000);
						g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
					}

					buf[0] = 0;

					if ((diplo & 0x20) == 0 && (packet_terrain->buf[i] & 0x20) != 0) {
						sprintf(buf, "%s %s %s", TxtFile::AllLines[0x8e], mp->name, TxtFile::AllLines[0x8f]);
					} else if ((diplo & 0x20) != 0 && (packet_terrain->buf[i] & 0x20) == 0) {
						sprintf(buf, "%s %s %s", TxtFile::AllLines[0x90], mp->name, TxtFile::AllLines[0x91]);
					}

					if (buf[0] != 0) {
						this->msglog.Add(buf, clr_log_sblack, 3000);
						g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
					}
				}
			}

			this->my_main_unit->diplomacy.SetSize(packet_terrain->count);
			memcpy(this->my_main_unit->diplomacy.GetData(), packet_terrain->buf, packet_terrain->count * 2);

			wnd->ClearDiplomacyEntries();

			if (wnd->vis_msgbox) {
				wnd->vis_msgbox->ReadData(&wnd->diplomacy_entries);
				wnd->vis_msgbox->VMethod9();
			}
			break;

		case 0xba: {
			uint8_t* stats = nullptr;
			int32_t stats_sz = 0;

			ReadKillStats(packet_data->data, packet_data->count, &stats, &stats_sz);
			memcpy(this->kill_stats.data(), stats, stats_sz);

			delete[] stats;

			if (wnd->sessionMode != 2) {
				wnd->m_GameSession.SaveKillStats();
			}
		}
			break;

		case 0xbb: {
			bool isnull = this->field_0x4970->GetQuestCount() == 0;

			this->field_0x4970->RemovePlayerQuests(0);

			uint16_t* dat = packet_terrain->buf + 1;

			for (int i = 0; i < packet_terrain->buf[0]; i++) {
				Quest* qwa = this->field_0x4970->CreateQuest(*dat);
				dat++;

				qwa->CopyFields((QuestData*)dat);
				dat = (uint16_t*)(((QuestData*)dat) + 1);

				this->field_0x4970->RegisterQuest(qwa);
			}

			if (isnull && field_0x4970->GetQuestCount() != 0) {
				wnd->PostMessage(0x466, 1, 0);
			}
		}
			break;

		case 0xbc:
			if ((wnd->dialogsMask & 4) != 0) {
				VisTav* tavern = (VisTav*)wnd->vis_root->FindChild(1100);
				if (tavern) {
					tavern->quest_map->RemovePlayerQuests(0);

					uint16_t* dat = packet_terrain->buf + 1;
					for (int i = 0; i < packet_terrain->buf[0]; i++) {
						Quest* qwa = tavern->quest_map->CreateQuest(*dat);
						dat++;

						qwa->CopyFields((QuestData*)dat);
						dat = (uint16_t*)(((QuestData*)dat) + 1);

						tavern->quest_map->RegisterQuest(qwa);
					}
				}
			}
			break;

		case 0xbf:
			if (packet_join->__field_0xa == CalcFileChecksum(packet_join->name)) {
				wnd->enter_ack_received = 1;
				wnd->input_locked = 0;
			} else {
				this->FUN_0041cb67(0);
				wnd->input_locked = 1;
			}
			break;

		case 0xc0: {
			CString fname = CString((const char*)packet_file->preamble) + ".tmp";
			CFile fl;

			try {
				if (packet_file->offset == 0) {
					LockSurface2();
					FillRectColorSimple(0, 0, g_ScreenSize.right, g_ScreenSize.bottom, 0);

					CString cstr = CString(TxtFile::AllLines[0x16b]) + " " + (const char*)packet_file->preamble;
					g_font1->DrawTxt(g_ScreenSize.right / 2, g_ScreenSize.bottom / 2, cstr, 8 | 2, clrsh_ShockingBlack);

					UnlockSurface2();

					if (wnd->has_focus != 0) {
						FlushScreen();
					}

					fl.Open(fname, CFile::modeCreate | CFile::modeWrite);
				} else {
					fl.Open(fname, CFile::modeWrite);
				}

				fl.Seek(packet_file->offset, 0);
				fl.Write(packet_file->var_data, packet_file->count);
				fl.Close();

				LockSurface2();
				FillRectColorSimple(0, g_ScreenSize.bottom / 2 + 20, g_ScreenSize.right, g_ScreenSize.bottom / 2 + 40, 0);

				g_font1->DrawTxt(g_ScreenSize.right / 2, g_ScreenSize.bottom / 2 + 20, TxtFile::AllLines[0x16c], 2, clrsh_ShockingBlack);

				UnlockSurface2();

				if (wnd->has_focus != 0) {
					gfxFlushRect(CRect(0, g_ScreenSize.bottom / 2 + 20, g_ScreenSize.right, g_ScreenSize.bottom / 2 + 40));
				}

			}
			catch (...)
			{}

			if (packet_file->offset + packet_file->count < packet_file->total_length) {
				this->FUN_0041cb67(packet_file->offset + packet_file->count);
			} else {
				
				try {
					CFile::Remove((const char*)packet_file->preamble);
				}
				catch(...)
				{}

				try {
					CFile::Rename(fname, (const char*)packet_file->preamble);
				}
				catch (...)
				{}

				this->FUN_0041cb21();
			}
		}
			break;

		case 0xc1: {
			NetStru2* conn = g_NetStru1_local.GetFirstClient();
			if (conn) {
				g_CLlDriver.SetLatency(conn->GetUid(), packet_dword->value);
			}
		}
			break;

		case 0xcd: {
			DAT_00666a00.RemoveAll();

			char* chpos = strstr((char*)packet_data->data, "CURRENTCOUNT");
			if (!chpos) {
				return 0;
			}

			chpos = strchr((char*)packet_data->data, '|');
			if (!chpos) {
				return 0;
			}

			chpos++;
			int32_t num = atoi(chpos);

			chpos = strchr((char*)packet_data->data, '\n') + 1;
			chpos = strchr(chpos, '\n') + 1;
			chpos = strchr(chpos, '\n');

			for (int i = 0; i < num; i++) {
				while( isprint(*chpos) == 0 ) {
					chpos++;
				}

				char bbuf[1024];
				char* outpos = bbuf;
				while (*chpos >= 0x20) {
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

		case 0xce: {
			wnd->field_0x3e0.hat_id = *(int32_t*)packet_data->data;
			wnd->m_GameSession.hat_char_ids.RemoveAll();

			HatCharId* chars = (HatCharId*)(packet_data->data + 4);
			for (int i = 0; i < packet_data->count / 8; i++) {
				wnd->m_GameSession.hat_char_ids.Add(chars[i]);
			}
		}
			break;

		case 0xcf: {
			HatCharId* chr = (HatCharId*)packet_data->data;

			CString fname;
			fname.Format("%u%u.a2c", chr->id1, chr->id2);

			CFile fl;
			if (fl.Open(fname, CFile::modeCreate | CFile::modeWrite)) {
				fl.Write(packet_data->data + 8, packet_data->count - 8);
				fl.Close();
			} else {
				INT_00660f8c = 0x1011;
			}

			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{}
			return 1;
		}
			break;

		case 0xd0:
			wnd->m_GameSession.field_0x148 = packet_2dwords->param1;
			wnd->m_GameSession.field_0x14c = packet_2dwords->param2;

			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{}
			return 1;

		}

		if (pkt->id == breakid) {
			return 1;
		}
	}
	return 1;
}






TakeDamage::TakeDamage() = default; //45db60
TakeDamage::TakeDamage(const TakeDamage& ref) = default;
TakeDamage::~TakeDamage() = default; //45dd08

TakeDamage::TakeDamage(int32_t _dmg, uint16_t* clr, int32_t unk, int32_t _dx, int32_t _dy, CUnit* ct) {
	//45db8a
	this->dmg = _dmg;

	this->txt.Format("%d", this->dmg);

	this->field_0x18 = unk;
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
	this->field_0x20 = wnd->serverLoopCounter;
}



void TakeDamage::Update() {
	//45dd1b
	if (this->field_0x18 == 0) {
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

int32_t TakeDamage::FUN_0041f620() { //41f620
    return this->dx + this->cunit->centerScreenX;
}

int32_t TakeDamage::FUN_0041f640() { //41f640
    return this->dy + this->cunit->centerScreenY - this->cunit->terrainHeightOffset;
}


void __cdecl TakeDamage::AddDamage(CArray<TakeDamage>* arr, const TakeDamage& dmg) {
	//45dea1

	for (int i = 0; i < arr->GetSize(); i++) {
		TakeDamage& b = arr->ElementAt(i);
		if (b.cunit == dmg.cunit && b.field_0x20 == dmg.field_0x20) {
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



void BigStruct2::FUN_0041b2a4(const char* nam, int32_t token, int32_t playerid) {
	//41b2a4

	PacketJoin* join = &PacketJoin::Inst;
	join->id = 0x91;
	join->from_player_id = this->my_main_unit->index;
	join->to_player_id = 0;

	strcpy(join->name, nam);

	if (playerid == 0) {
		join->player_id = 0;
		join->token_id = 0;
		join->color = 0;
		join->flags = 0;
	} else {
		join->__field_0xa = this->field_0x9b8[playerid]->index;
	}

	join->__field_0xa |= token << 8;

	if (g_NetStru1_local.IsActive() == 0) {
		if (g_NetStru1_main.IsActive() && g_Server) {
			g_NetStru1_main.QueuePacketSend(join);
			g_NetStru1_main.SendAllData();
		}
	} else {
		g_NetStru1_local.QueuePacketSend(join);
	}
}

void BigStruct2::FUN_0041a8cc() {
	//41a8cc
	PacketWord* pkt = &PacketWord::Inst;

	pkt->id = 0x36;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->value = this->GetUnit_3f6c()->unit_id;

	g_NetStru1_local.QueuePacketSend(pkt);

	((MainWindow*)AfxGetMainWnd())->dialogsMask &= ~2;
}


int32_t BigStruct2::FUN_0040d7f3() {
	//40d7f3
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	if (wnd->sessionMode == 0) {
		g_NetStru1_local.SendLatency(g_CmdLatency, 0);

		NetStru2* conn = g_NetStru1_local.GetFirstClient();
		if (conn) {
			g_CLlDriver.SetLatency(conn->GetUid(), g_CmdLatency);
		}
	}

	PacketEnterGame *pkt = &PacketEnterGame::Inst;

	pkt->id = 2;
	strcpy(pkt->text, wnd->m_GameSession.character_name);
	memcpy(&pkt->value1, &wnd->m_GameSession.sessionKeyPart1, 8);

	pkt->value3 = g_IsCdPresent | (wnd->m_GameSession.color << 8) | (wnd->m_GameSession.GetSkillTier1() << 0x10) | (wnd->m_GameSession.GetSkillTier2() << 0x18);
	pkt->to_player_id = 0;

	this->field_0x9d0.RemoveAll();

	if (this->field_0x3f6c) {
		delete this->field_0x3f6c;
	}

	this->field_0x3f6c = nullptr;

	this->UpdateSelectionState();

	g_NetStru1_local.QueuePacketSend(pkt);

	if (wnd->serverBootstrapEnabled != 0) {
		g_Server->PumpNetwork();
	}
	
	uint32_t stime = timeGetTime();
	while (true) {
		if (g_NetStru1_local.GetClientsPktNum()) {
			if (this->ProcessPackets(0x64) == 0) {
				return 0;
			}

			if (this->field_0x9b8.GetSize() < 2) {
				INT_00660f8c = 0x1007;
				return 0;
			}

			return 1;
		}


		MSG msg;
		if (PeekMessageA(&msg, NULL, 0, 0, 1) != 0) {
			if (msg.message == WM_QUIT) {
				return 0;
			}

			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		if (g_CmdTimeout < timeGetTime() - stime) {
			break;
		}

		g_mousept.Update();
		g_NetStru1_local.ProcessConnections();
	}

	INT_00660f8c = 0x1009;
	return 0;
}


int BigStruct2::ConnectAndJoinSession() {
	//40d4e2
	INT_00660f8c = 0;

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	if (wnd->sessionMode == 1 || wnd->sessionMode == 2) {
		g_NetStru1_main.SetLinkedHLDriver(&g_NetStru1_local);
		g_NetStru1_local.SetLinkedHLDriver(&g_NetStru1_main);
		g_NetStru1_main.ProcessConnections();
		g_NetStru1_local.ProcessConnections();
	} else {
		CLlNetSession* session = nullptr;
		if (wnd->net_sessions.sessions) {
			session = wnd->net_sessions.sessions + wnd->net_sessions.selected_index;
		}
		
		if (g_CLlDriver.Connect(wnd->m_GameSession.character_name, session) == 0) {
			wnd->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, TxtFile::AllLines.GetAt(0x9a), nullptr, 0));
			return 0;
		}
	}

	g_NetStru1_local.ProcessConnections();

	if (this->FUN_0040d7f3()) {
		return 1;
	}

	if ((INT_00660f8c & 0xff) < 11) {
		wnd->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, TxtFile::AllLines.GetAt((INT_00660f8c & 0xff) + 0xc0), nullptr, 0));
	} else {
		wnd->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(INT_00660f8c & 0xff), nullptr, 0));
	}
	return 0;
}



void BigStruct2::FUN_0041cbb8() {
	//0041cbb8
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x48;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1_b0 = wnd->m_GameSession.body;
	pkt->param1_b1 = wnd->m_GameSession.reaction;
	pkt->param1_b2 = wnd->m_GameSession.mind;
	pkt->param1_b3 = wnd->m_GameSession.spirit;
	pkt->param2_b0 = wnd->m_GameSession.main_sphere;
	pkt->param2_b1 = wnd->m_GameSession.face | wnd->m_GameSession.type;
	pkt->param2_b2 = wnd->m_GameSession.color;

	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041cc78(const CString& fname) {
	//41cc78
	PacketData* pkt = &PacketData::Inst;

	pkt->id = 0xbe;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;

	CFile fl;
	if (fl.Open(fname, 0)) {
		int32_t len = fl.GetLength();
		if (len < 0x8000) {
			fl.Read(pkt->data, len);
			pkt->count = len;
		}
		fl.Close();
	}

	g_NetStru1_local.QueuePacketSend(pkt);
}



void BigStruct2::FUN_0041d2da(int32_t arg) { // 41d2da
	this->field_0x3f6c = nullptr;

	if (!this->field_0x9d0.IsEmpty()) {
		POSITION it = this->field_0x9d0.GetStartPosition();
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->field_0x9d0.GetNextAssoc(it, key, obj);
			if (obj) {
				delete obj;
			}
		}

		this->field_0x9d0.RemoveAll();
	}

	if (!this->field_0x9ec.IsEmpty()) {
		POSITION it = this->field_0x9ec.GetStartPosition();
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->field_0x9ec.GetNextAssoc(it, key, obj);
			if (obj) {
				delete obj;
			}
		}

		this->field_0x9ec.RemoveAll();
	}

	this->damage_labels.RemoveAll();

	if (arg == 0) {
		for (int i = 1; i < 17; i++) {
			MapPlayerData* mp = this->field_0x9b8[i];
			if (mp && (mp->flags & 1) != 0) {
				delete mp;
				this->field_0x9b8[i] = nullptr;
			}
		}
	} else {
		for (int i = 1; i < this->field_0x9b8.GetSize(); i++) {
			MapPlayerData* mp = this->field_0x9b8[i];
			if (mp) {
				delete mp;
			}
		}
		if (this->field_0x9b8.GetSize()) {
			this->field_0x9b8.SetSize(1);
		}
	}

	this->field_0xa08.RemoveAll();

	if (this->field_0x80) {
		delete this->field_0x80;
		this->field_0x80 = nullptr;
	}

	while (g_NetStru1_local.ReceiveAnyPacket())
	{}

	if (g_dsound_channels) {
		for (int i = 0; i < g_dsound_channel_num; i++) {
			g_dsound_channels[i].Clear();
		}
	}

	if (g_SfxArray[14]) {
		g_SfxArray[14]->Release();
	}

	if (g_SfxArray[16]) {
		g_SfxArray[16]->Release();
	}

	for (int i = 0; i < g_mapmusicinfos.GetSize(); i++) {
		delete g_mapmusicinfos[i];
	}

	g_mapmusicinfos.RemoveAll();
}

int32_t BigStruct2::IsBookOpen() { // 41b4b0
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->dialogsMask & 2) {
		return wnd->vis_root->FindChild(1000)->FindChild(3) != nullptr;
	}

	return this->FindChild(3) != nullptr;
}

int32_t BigStruct2::IsBagOpen() { // 41b495
	return this->FindChild(2) != nullptr;
}

void BigStruct2::FUN_0041afcf(CString str) { //41afcf
	PacketJoin* pkt = &PacketJoin::Inst;
	pkt->id = 7;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	strcpy(pkt->name, str);

	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041a735() { //41a735
	PacketTerrain* pkt = &PacketTerrain::Inst;
	pkt->id = 0x45;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	memcpy(pkt->buf, this->my_main_unit->diplomacy.GetData(), this->my_main_unit->diplomacy.GetSize() * 2);
	pkt->count = this->my_main_unit->diplomacy.GetSize();
	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041c39c() { //41c39c
	this->field_0x49b8 = -1;
	this->field_0x49bc = -1;
	SoundChannel* ch1 = g_SfxArray[50]->FindPlayingChannel();
	SoundChannel* ch2 = g_SfxArray[90]->FindPlayingChannel();
	if (ch1) {
		ch1->Stop();
	}
	if (ch2) {
		ch2->Stop();
	}
}


void BigStruct2::CleanupCompletedMissionMapState() { //41ce5a
	CDWordArray arr;
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);

		if (obj->IsKindOf(RUNTIME_CLASS(CUnit)) == 0) {
			arr.Add(key);
		} else {
			CUnit* uni = (CUnit*)obj;
			if ((uni->unitFlags & 1) != 0 && uni->hp >= -10 && uni->map_player == this->my_main_unit) {
				uni->death_state = 0;
				uni->action = 0;
				uni->action_segments = 0;
				uni->SetSelected(0);
			} else {
				arr.Add(key);
			}
		}
	}

	this->field_0x3f6c->SetSelected(1);

	//Remove it here or while iteration
	for (int i = 0; i < arr.GetSize(); i++) {
		delete this->field_0x9d0[arr[i]];
		this->field_0x9d0.RemoveKey(arr[i]);
	}
	arr.RemoveAll();

	for (POSITION it = this->field_0x9ec.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9ec.GetNextAssoc(it, key, obj);

		delete obj;
		this->field_0x9ec.RemoveKey(key);
	}

	this->damage_labels.RemoveAll();

	if (this->field_0x80) {
		delete this->field_0x80;
	}
	this->field_0x80 = nullptr;

	for (int i = 0; i < this->field_0x9b8.GetSize(); i++) {
		if (this->field_0x9b8[i] != this->my_main_unit) {
			delete this->field_0x9b8[i];
		}
	}
	this->field_0x9b8.RemoveAll();

	this->field_0x9b8.SetAtGrow(this->my_main_unit->index, this->my_main_unit);

	this->field_0xa08.RemoveAll();

	while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
	{}

	if (g_dsound_channels) {
		for (int i = 0; i < g_dsound_channel_num; i++) {
			g_dsound_channels[i].Clear();
		}
	}

	if (g_SfxArray[14]) {
		g_SfxArray[14]->Release();
	}

	if (g_SfxArray[16]) {
		g_SfxArray[16]->Release();
	}

	this->UpdateSelectionState();

	for (int i = 0; i < g_mapmusicinfos.GetSize(); i++) {
		delete g_mapmusicinfos[i];
	}
	g_mapmusicinfos.RemoveAll();
}

void BigStruct2::UpdateSelectionState() { //416cf7

	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
	this->field_0x140 = 0;
	this->field_0x144 = 0;
	this->field_0x148 = 0;
	this->field_0x150 = 0;
	this->field_0x14c = 0;

	for (int i = 0; i < 24; i++) {
		this->spell_damage_min[i] = 0;
		this->spell_damage_max[i] = 0;
		this->spell_range_min[i] = 0xffff;
		this->spell_range_max[i] = 0;
		this->spell_mana_cost1[i] = 0xffff;
		this->spell_mana_cost2[i] = 0;
		this->spell_power_min[i] = 0xffff;
		this->spell_power_max[i] = 0;
		this->spell_val1_min[i] = 0xffff;
		this->spell_val1_max[i] = -0xffff;
		this->spell_val2_min[i] = 0xffff;
		this->spell_val2_max[i] = 0;
		this->spell_val3_min[i] = 0xffff;
		this->spell_val3_max[i] = -0xffff;
		this->spell_val4_min[i] = 0xffff;
		this->spell_val4_max[i] = 0;
		this->spell_val5_min[i] = 0xffff;
		this->spell_val5_max[i] = 0;
		this->spell_val6_min[i] = 0xffff;
		this->spell_val6_max[i] = 0;
		this->spell_val7_min[i] = 0xffff;
		this->spell_val7_max[i] = 0;
	}

	bool firstsel = false;

	for (POSITION it = this->field_0x9d0.GetStartPosition(); it;) {
		POSITION curpos = it;
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);

		if (obj->IsSelected() == 0) {
			continue;

		// if selected
		}
		this->field_0x140++;
		this->field_0x148 |= obj->availableSpellMask;
		if (obj->active_spell != 0) {
			this->field_0x14c |= SpellBtB[obj->active_spell];
		}

		if (!firstsel) {
			this->field_0x138 = obj;
			this->field_0x13c = curpos;
			firstsel = true;
		}

		if (mwnd->dialogsMask == 1 || (mwnd->dialogsMask & 2) != 0) {
			if (obj->GetRuntimeClass() == RUNTIME_CLASS(CUnit)) {
				CUnit* unit = (CUnit*)obj;

				this->field_0x144 |= 1;

				if ((unit->typeId == 0x17 || unit->typeId == 0x18) && unit->availableSpellMask != 0) {
					this->field_0x144 |= 0x200;
					this->UpdateSpellModifiers(unit);
				}

				if (unit->unitFlags & 1) {
					this->field_0x144 |= 8;
				}

				this->UpdateSpellEffects(unit);
			} else if (obj->GetRuntimeClass() == RUNTIME_CLASS(CAirUnit)) {
				this->field_0x144 |= 2;
			} else if (obj->GetRuntimeClass() == RUNTIME_CLASS(CStructure)) {
				this->field_0x144 |= 0x20;
			} else if (g_IsServer != 0) {
				this->field_0x144 = 8;
			}
		}
#ifdef A2CLIENT
		// client: 41916c — the gate-failure edge takes the "unknown class" path instead
		else if (g_IsServer == 0) {
			this->field_0x144 = 8;
		}
#endif
	}

	if (this->field_0x140 > 0 && this->field_0x138->map_player != this->my_main_unit) {
		this->field_0x144 |= 4;
	}
	
	if (this->field_0x140 == 0 || (this->field_0x144 & 0x24) != 0) {
		mwnd->vis_right_panel->MsgProc(CVisualObject::MSG_40a, 0, 0);
	} else {
		mwnd->vis_right_panel->MsgProc(CVisualObject::MSG_409, this->AvailableOrderMask(), 0);
	}

	mwnd->vis_charinfo->MsgProc(CVisualObject::MSG_410, 0, 0);
	mwnd->vis_sidestatus->MsgProc(CVisualObject::MSG_410, 0, 0);
	if (this->field_0x140 == 1 && (this->field_0x144 & 8) != 0) {
		mwnd->vis_invtype1->VMethod33(this->field_0x138);
	} else {
		mwnd->vis_invtype1->VMethod33(nullptr);
	}

}

void BigStruct2::UpdateSpellEffects(CUnit* unit) { //from 416cf7
	for (int i = 0; i < unit->tokenEntries.GetSize(); i++) {
		TokenEntry* t = unit->tokenEntries[i];
		if (!t || t->GetType() != TokenEntry::TYPE_SPELL) {
			continue;
		}

		uint32_t cast_spell = t->GetCastSpellId();
		if (t == 0) {
			continue;
		}

		unit->activeSpellEffectMask |= SpellBtB[cast_spell];
		this->field_0x150 |= SpellBtB[cast_spell];
	}
}

void BigStruct2::UpdateSpellModifiers(CUnit* unit) { //from 416cf7
	for (int i = 0; i < 24; i++) {
		if ((unit->availableSpellMask & (1 << i)) == 0) {
			continue;
		}

		Spell s(BOOK_POS_TO_SPELL_ID[i + 1]);

		const uint8_t spellbase = unit->skill_levels[s.spell_info->Values()[0].sphere - 1];
		int32_t v = (spellbase - 30) + unit->mind;
		if (v < 0) {
			v = 0;
		}
		if (v >= 256) {
			v = 255;
		}

		s.SetPower(v);

		this->spell_damage_min[i] += s.damage_min;
		this->spell_damage_max[i] += s.damage_min + s.damage_spread;
		this->spell_mana_cost1[i] = s.mana_cost;
		this->spell_mana_cost2[i] = s.mana_cost;

		if (this->spell_range_min[i] > s.max_range) {
			this->spell_range_min[i] = s.max_range;
		}

		if (this->spell_range_max[i] < s.max_range) {
			this->spell_range_max[i] = s.max_range;
		}

		if (this->spell_power_min[i] > s.spell_power) {
			this->spell_power_min[i] = s.spell_power;
		}
		
		if (this->spell_power_max[i] < s.spell_power) {
			this->spell_power_max[i] = s.spell_power;
		}
		
		if (BOOK_POS_TO_SPELL_ID[i + 1] == 0x12) { // STONE_CURSE
			this->spell_power_min[i] = 0;
		}

		const uint8_t spd = s.GetSpeed(v);
		if (spd != 0) {
			if (this->spell_val1_min[i] > spd) {
				this->spell_val1_min[i] = spd;
			}
			if (this->spell_val1_max[i] < spd) {
				this->spell_val1_max[i] = spd;
			}
		}

		const uint8_t res = s.GetResistance(v);
		if (this->spell_val2_min[i] > res) {
			this->spell_val2_min[i] = res;
		}
		if (this->spell_val2_max[i] < res) {
			this->spell_val2_max[i] = res;
		}

		const int16_t sigh = s.GetSight(v);
		if (sigh != 0) {
			if (this->spell_val3_min[i] > sigh) {
				this->spell_val3_min[i] = sigh;
			}
			if (this->spell_val3_max[i] < sigh) {
				this->spell_val3_max[i] = sigh;
			}
		}

		const uint8_t mxprob = s.GetMaximumDamageProbability(v);
		if (this->spell_val4_min[i] > mxprob) {
			this->spell_val4_min[i] = mxprob;
		}
		if (this->spell_val4_max[i] < mxprob) {
			this->spell_val4_max[i] = mxprob;
		}

		const uint8_t mnprob = s.GetMinimumDamageProbability(v);
		if (this->spell_val5_min[i] > mnprob) {
			this->spell_val5_min[i] = mnprob;
		}
		if (this->spell_val5_max[i] < mnprob) {
			this->spell_val5_max[i] = mnprob;
		}

		const uint8_t rs = s.GetRays(v);
		if (this->spell_val6_min[i] > rs) {
			this->spell_val6_min[i] = rs;
		}
		if (this->spell_val6_max[i] < rs) {
			this->spell_val6_max[i] = rs;
		}

		const uint8_t bn5 = s.GetBonusType5(v);
		if (this->spell_val7_min[i] > bn5) {
			this->spell_val7_min[i] = bn5;
		}
		if (this->spell_val7_max[i] < bn5) {
			this->spell_val7_max[i] = bn5;
		}
	}
}

uint32_t BigStruct2::AvailableOrderMask() const {
	if ((this->field_0x144 & 4) == 0) {
		if (this->field_0x148 != 0 || this->field_0x150 != 0) {
			return 0xff; // ~0
		}

		return 0xef;  // ~(0x10)
	}

	return 0;
}




GfxFile::GfxFile(const char* _fname) { //47ae53
	this->spr = nullptr;
	this->spr_b = nullptr;
	this->fname = CString("graphics\\objects\\") + _fname;
	this->inited = 0;
}

GfxFile::~GfxFile() { //47aed9
	this->Deinit();
}

void GfxFile::Init() { // 47af39
	this->spr = new CSprite256(this->fname + ".256");
	this->spr_b = new CSprite256(this->fname + "b.256");
	this->spr->ResetPalette(0x10, 2, 1);
	this->inited = 1;
}

void GfxFile::Deinit() { //47b0ab
	if (this->inited) {
		if (this->spr) {
			delete this->spr;
			this->spr = nullptr;
		}
		if (this->spr_b) {
			delete this->spr_b;
			this->spr_b = nullptr;
		}
		this->inited = 0;
	}
}

CSprite256* GfxFile::GetSprite() { //41f870
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr;
}

CSprite256* GfxFile::GetSpriteB() { //41f8a0
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr_b;
}


UnitGfxFile::UnitGfxFile(const char* _fname) { //47c85d
	this->spr = nullptr;
	this->spr_b = nullptr;
	this->fname = CString("graphics\\units\\") + _fname;
	this->inited = 0;
}

UnitGfxFile::~UnitGfxFile() { //47c8e3
	this->Deinit();
}

void UnitGfxFile::Init() { // 47c943
	this->spr = new CSprite256(this->fname + ".256");
	this->spr_b = new CSprite256(this->fname + "b.256");
	this->spr->ResetPalette(0x10, 2, 1);
	this->inited = 1;
}

void UnitGfxFile::Deinit() { //47caa4
	if (this->inited) {
		if (this->spr) {
			delete this->spr;
			this->spr = nullptr;
		}
		if (this->spr_b) {
			delete this->spr_b;
			this->spr_b = nullptr;
		}
		this->inited = 0;
	}
}

CSprite256* UnitGfxFile::GetSprite() { //46f700
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr;
}

CSprite256* UnitGfxFile::GetSpriteB() { //46f730
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr_b;
}

CGamePalette* UnitVFXUnfo::GetPalette(int32_t idx) { //46f680
    uint16_t* pal = this->palettes[idx]->GetPalette(0);
    if (pal == nullptr) {
        this->palettes[idx]->SetPalette((RGBQUAD*)this->palette_datas[idx], 0x10, 2, 1);
    }
    return this->palettes[idx];
}

uint8_t* UnitVFXUnfo::GetPaletteData(int32_t idx) { //46f6e0
    return this->palette_datas[idx];
}


StructureInfo::StructureInfo(const char* _fname) { //47eca2
	this->inited = 0;
	this->fname = CString("graphics\\structures\\") + _fname;
}


StructureInfo::~StructureInfo() { //47ed4e
	this->Deinit();
	if (this->anim_mask) {
		delete[] this->anim_mask;
		this->anim_mask = nullptr;
	}
}


void StructureInfo::Init() { //47edd8
	this->spr = new CSprite256(this->fname + ".256");
	this->spr_b = new CSprite256(this->fname + "b.256");
	this->spr->ResetPalette(0x10, 2, 1);
	this->inited = 1;
}


void StructureInfo::Deinit() { //47ef52
	if (this->inited) {
		if (this->spr) {
			delete this->spr;
			this->spr = nullptr;
		}
		if (this->spr_b) {
			delete this->spr_b;
			this->spr_b = nullptr;
		}
		this->inited = 0;
	}
}

CSprite256* StructureInfo::GetSprite() { //46f790
	if (this->inited == 0) {
		this->Init();
	}
	return this->spr;
}

CSprite256* StructureInfo::GetSpriteB() { //46f7c0
	if (this->inited == 0) {
		this->Init();
	}
	return this->spr_b;
}


ProjectileInfo::ProjectileInfo(const char* fname, int32_t _a16) { //47e1ec
	this->a16 = _a16;
	this->filename = fname;
	this->inited = 0;
}

ProjectileInfo::~ProjectileInfo() { //47e50b
	this->Deinit();
}


void ProjectileInfo::Init() { //47e26b
	if (this->inited) {
		return;
	}

	CString str = CString("graphics\\projectiles\\") + this->filename;
	if (this->a16) {
		this->sprite1 = new CA16(str + ".16a");
		this->sprite2 = nullptr;
	} else {
		this->sprite1 = new CSprite256(str + ".256");
		this->sprite2 = new CSprite256(str + "b.256");
	}

	if (this->palette) {
		if (this->a16) {
			this->sprite1->ResetPalette(16, 4, 0);
		} else {
			this->sprite1->ResetPalette(16, 2, 1);
			this->sprite2->ResetPalette(16, 2, 1);
		}
	}

	this->inited = 1;
}

void ProjectileInfo::Deinit() { //47e56b
	if (!this->inited) {
		return;
	}

	if (this->sprite1) {
		delete this->sprite1;
		this->sprite1 = nullptr;
	}

	if (this->sprite2) {
		delete this->sprite2;
		this->sprite2 = nullptr;
	}

	this->inited = 0;
}

CSprite256* ProjectileInfo::GetSprite() { //41f8f0
    if (this->inited == 0) {
        this->Init();
    }
    return this->sprite1;
}

CSprite256* ProjectileInfo::GetSpriteB() { //46f760
    if (this->inited == 0) {
        this->Init();
    }
    return this->sprite2;
}


void BigStruct2::FUN_0041cad0(int32_t m) { //41cad0
	Packet2Dwords *pkt = &Packet2Dwords::Inst;
	pkt->id = 4;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = m;
	
	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041aaaa(int32_t wim) { //41aaaa
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x46;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = 1;
	pkt->param2 = wim;

	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041abd2(int32_t form) { // 41abd2
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x46;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = 2;
	pkt->param2 = form;

	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041ab74() { //41ab74
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x46;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = 3;
	pkt->param2 = g_settings.AutoCasting;

	g_NetStru1_local.QueuePacketSend(pkt);
}


void BigStruct2::FUN_0041b6e0() { //41b6e0
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	int32_t btm = this->rect.bottom & (~0x1f);
	CVisualObject* obj2 = this->FindChild(2);
	CVisualObject* obj3 = this->FindChild(3);

	if (obj2) {
		btm = obj2->GetRect().top;
	}

	if (obj3) {
		btm = obj3->GetRect().top;
	}

	btm = ((btm - 1) & (~0x1f)) + 0x20;
	this->view_tiles_y = btm / 32;
	
	int32_t vy = this->MapMaxY();
	if (vy < this->view_y) {
		this->view_y = vy;
	}

	if (CVisualObject::FindChild(1200)) {
		mwnd->vis_1200->FUN_0043be9f();
	}

	this->force_full_redraw = 1;
}

void BigStruct2::FUN_0041b636() { //41b636
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if ((mwnd->dialogsMask & 2) == 0) {
		this->RemoveChild(mwnd->vis_spellbook);
	} else {
		mwnd->vis_root->FindChild(1000)->RemoveChild(mwnd->vis_spellbook);
	}

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);

	mwnd->vis_right_panel->MsgProc(0x408, 0, 0);

	this->FUN_0041b6e0();
}


void BigStruct2::FUN_0041b509() { // 41b509

	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if (this->FUN_0041b495() == 0) {
		mwnd->vis_spellbook->SetRect(0, this->rect.bottom - 85, this->rect.right, this->rect.bottom);
	} else {
		mwnd->vis_spellbook->SetRect(0, this->rect.bottom - 175, this->rect.right, this->rect.bottom - 90);
	}

	if ((mwnd->dialogsMask & 2) == 0) {
		this->AddChild(mwnd->vis_spellbook);
	} else {
		mwnd->vis_spellbook->SetRect(0, 305, 480, 390);
		mwnd->vis_root->FindChild(1000)->AddChild(mwnd->vis_spellbook);
	}

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);

	mwnd->vis_right_panel->MsgProc(0x408, 0, 0);

	this->FUN_0041b6e0();
}

void BigStruct2::FUN_0041b40e() { //41b40e
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if (this->IsBookOpen()) {
		mwnd->vis_spellbook->SetRect(0, this->rect.bottom - 85, this->rect.right, this->rect.bottom);
	}

	this->RemoveChild(mwnd->vis_invtype1);

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);

	this->FUN_0041b6e0();
}

void BigStruct2::FUN_0041b381() { // 41b381
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if (this->IsBookOpen()) {
		mwnd->vis_spellbook->SetRect(0, this->rect.bottom - 175, this->rect.right, this->rect.bottom - 90);
	}
	
	this->AddChild(mwnd->vis_invtype1);

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);
	this->FUN_0041b6e0();
}

void BigStruct2::FUN_0041b10f() {
	Packet* pkt = &Packet::Inst;
	pkt->id = 0x4a;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::FUN_0041b064(int32_t arg1, int32_t arg2) {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x3f;
	pkt->param1 = arg1;
	pkt->param2 = arg2;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::SendAdjustPlayerGoldAction(int32_t goldDelta) const { //41b0be
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x3e;
	pkt->param1 = goldDelta;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::ScrollMapX(int32_t dx) { //403f4f
	this->scroll_dx += dx;
	if (this->scroll_dx + this->view_x < this->MapMinX()) {
		this->scroll_dx = this->MapMinX() - this->view_x;
	}

	if (this->scroll_dx + this->view_x > this->MapMaxX()) {
		this->scroll_dx = this->MapMaxX() - this->view_x;
	}
}

void BigStruct2::ScrollMapY(int32_t dy) { //403fc5
	this->scroll_dy += dy;
	if (this->scroll_dy + this->view_y < this->MapMinY()) {
		this->scroll_dy = this->MapMinY() - this->view_y;
	}

	if (this->scroll_dy + this->view_y > this->MapMaxY()) {
		this->scroll_dy = this->MapMaxY() - this->view_y;
	}
}

void BigStruct2::OnOpenShopDialog() { //41a856
	PacketWord* pkt = &PacketWord::Inst;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->id = 0x32;
	pkt->value = this->GetUnit_3f6c()->unit_id;
	g_NetStru1_local.QueuePacketSend(pkt);

	((MainWindow*)AfxGetMainWnd())->dialogsMask |= 2;
}

void BigStruct2::NetOnOpenInnDialog() { //41adbb
	PacketWord* pkt = &PacketWord::Inst;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->id = 0x38;
	pkt->value = this->GetUnit_3f6c()->unit_id;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41F830
CUnit* BigStruct2::GetUnit_3f6c() {
	return this->field_0x3f6c;
}

// 41F6D0
void BigStruct2::sub_41F6D0() {
	this->formation = (this->formation + 1) % 3;
	this->FUN_0041abd2(this->formation);
}

// 41F720
void BigStruct2::sub_41F720() {
	this->wimpy = (this->wimpy + 1) % 3;
	this->FUN_0041aaaa(this->wimpy);
}

// 41F770
void BigStruct2::sub_41F770() {
	this->show_hp = this->show_hp == 0;
}

// 41F7B0
void BigStruct2::sub_41F7B0() {
	this->flying_hp = this->flying_hp == 0;
}

// 41F7F0
int32_t BigStruct2::sub_41F7F0() {
	return this->wimpy;
}

// 41F810
int32_t BigStruct2::sub_41F810() {
	return this->formation;
}

// 416983
void BigStruct2::sub_416983(int32_t digit) {
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsInControlGroup(digit)) {
			obj->SetSelected(1);
		} else if (g_kbShiftState == 0) {
			obj->SetSelected(0);
		}
	}
	this->UpdateSelectionState();
}

// 416A02
void BigStruct2::sub_416A02(int32_t digit) {
	if (this->field_0x144 & 4) {
		return;
	}
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsInControlGroup(digit)) {
			obj->RemoveControlGroup(digit);
		}
		if (obj->IsSelected() && obj->map_player == this->my_main_unit) {
			obj->SetControlGroup(digit);
		}
	}
	this->UpdateSelectionState();
}

// 416AA6
void BigStruct2::sub_416AA6(int32_t digit) {
	if (this->field_0x144 & 4) {
		return;
	}
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsInControlGroup(digit)) {
			obj->RemoveControlGroup(digit);
		}
		if (obj->IsSelected() && obj->map_player == this->my_main_unit) {
			obj->AddControlGroup(digit);
		}
	}
	this->UpdateSelectionState();
}

// 416B4A
void BigStruct2::sub_416B4A(int32_t digit) {
	int32_t sum_x = 0;
	int32_t sum_y = 0;
	int32_t count = 0;
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsInControlGroup(digit)) {
			obj->SetSelected(1);
			count++;
			sum_x += obj->x_pos >> 8;
			sum_y += obj->y_pos >> 8;
		} else {
			obj->SetSelected(0);
		}
	}
	this->UpdateSelectionState();
	if (count > 0) {
		int32_t avg_x = sum_x / count;
		int32_t avg_y = sum_y / count;
		this->MsgProc(0x406, avg_x - this->view_tiles_x / 2, avg_y - this->view_tiles_y / 2);
	}
}

// 416C4C
void BigStruct2::sub_416C4C() {
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsKindOf(RUNTIME_CLASS(CUnit)) && obj->map_player == this->my_main_unit &&
			((CUnit*)obj)->death_state <= 1 && !(((CUnit*)obj)->unitFlags & 0x80)) {
			obj->SetSelected(1);
		} else {
			obj->SetSelected(0);
		}
	}
	this->UpdateSelectionState();
}

// 4168BD
void BigStruct2::sub_4168BD() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (this->field_0x9b4 != 0) {
		if (this->field_0x9b4 == 5 && this->IsBookOpen()) {
			this->FUN_0041b636();
		}
		wnd->vis_spellbook->FUN_004caa69();
		this->field_0x9b4 = 0;
		wnd->vis_ordertoolbar->MsgProc(0x40B, 0, 0);
	} else {
		for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
			uint16_t key;
			CGameObject* obj;
			this->field_0x9d0.GetNextAssoc(it, key, obj);
			obj->SetSelected(0);
		}
		this->UpdateSelectionState();
	}
}

// 4167F7
void BigStruct2::sub_4167F7(int32_t wparam, int32_t lparam) {
	if (this->field_0x80 == nullptr) {
		return;
	}
	if (wparam < this->MapMinX()) {
		wparam = this->MapMinX();
	}
	if (lparam < this->MapMinY()) {
		lparam = this->MapMinY();
	}
	if (wparam > this->MapMaxX()) {
		wparam = this->MapMaxX();
	}
	if (lparam > this->MapMaxY()) {
		lparam = this->MapMaxY();
	}
	this->scroll_dx = wparam - this->view_x;
	this->scroll_dy = lparam - this->view_y;
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	wnd->vis_minimap->MsgProc(0x408, 0, 0);
}

// 403F08
void BigStruct2::sub_403F08(Scenario* scen) {
	this->field_0x80 = scen;
	this->map_width = this->field_0x80->GetWidth();
	this->map_height = this->field_0x80->GetHeight();
}

// 418F93
void BigStruct2::sub_418F93(int32_t arg) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	uint32_t avail = this->AvailableOrderMask();
	int32_t mode = arg;
	if (arg == 9) {
		mode = 5;
	}
	if (arg == 0xA) {
		mode = 5;
	}
	if (this->field_0x9b4 == 5 && mode != 5 && this->IsBookOpen()) {
		this->FUN_0041b636();
		wnd->vis_spellbook->FUN_004caa69();
	}
	if ((avail & (1 << (mode - 1))) != 0 || arg == 0xA) {
		this->field_0x9b4 = mode;
		wnd->vis_ordertoolbar->MsgProc(0x40D, mode - 1, 0);
		switch (arg) {
		case 3:
			this->sub_4193E8();
			this->field_0x9b4 = 0;
			wnd->vis_ordertoolbar->MsgProc(0x40B, 0, 0);
			break;
		case 7:
			this->sub_4194A2();
			this->field_0x9b4 = 0;
			wnd->vis_ordertoolbar->MsgProc(0x40B, 0, 0);
			break;
		case 8:
			this->sub_41A425();
			this->field_0x9b4 = 0;
			wnd->vis_ordertoolbar->MsgProc(0x40B, 0, 0);
			break;
		case 5:
			if (!this->IsBookOpen()) {
				this->FUN_0041b509();
			}
			break;
		}
	}
}

// 419154
void BigStruct2::sub_419154(uint16_t x, uint16_t y) {
	POSITION it = this->field_0x9d0.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x16;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->x = x;
	int32_t tile_y = this->sub_4184B8(g_mousept.x, g_mousept.y) + this->view_y;
	pkt->y = tile_y;
	pkt->y_high = tile_y >> 8;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr && unit->last_action != 1) {
		unit->PlayMoveVoice();
	}
}

// 419246
void BigStruct2::sub_419246(uint16_t id) {
	POSITION it = this->field_0x9d0.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x19;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param = id;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr) {
		unit->PlayAttackVoice();
	}
}

// 41930D
void BigStruct2::sub_41930D(uint16_t x, uint16_t y) {
	POSITION it = this->field_0x9d0.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x1A;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->x = x;
	pkt->y = y;
	pkt->y_high = y >> 8;
	pkt->to_player_id = 0;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr && unit->last_action != 1) {
		unit->PlaySwarmVoice();
	}
}

// 4193E8
void BigStruct2::sub_4193E8() {
	POSITION it = this->field_0x9d0.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x17;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr) {
		unit->PlayDefendVoice();
	}
}

// 4194A2
void BigStruct2::sub_4194A2() {
	POSITION it = this->field_0x9d0.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x18;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr) {
		unit->PlayDefendVoice();
	}
}

// 41955C
void BigStruct2::sub_41955C(uint16_t id) {
	POSITION it = this->field_0x9d0.GetStartPosition();
	CGameObject* obj;
	if (!this->field_0x9d0.Lookup(id, obj) || !obj->IsKindOf(RUNTIME_CLASS(CUnit))) {
		return;
	}
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x1B;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param = id;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr) {
		unit->PlayDefendVoice();
	}
}

// 41965D
void BigStruct2::sub_41965D(uint16_t x, uint16_t y) {
	POSITION it = this->field_0x9d0.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x1D;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->x = x;
	pkt->y = y;
	pkt->y_high = y >> 8;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr) {
		unit->PlayMoveVoice();
	}
}

// 41972F
void BigStruct2::sub_41972F(uint16_t x, uint16_t y, int32_t spell) {
	POSITION it = this->field_0x9d0.GetStartPosition();
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x1F;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->x = x;
	pkt->y = y;
	pkt->y_high = y >> 8;
	pkt->count = 0;
	if (wnd->vis_spellbook->sub_4CA89B(spell - 1) != 0 && wnd->vis_spellbook->spell == -1) {
		pkt->id = 0x1F;
		pkt->slot = spell;
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->field_0x9d0.GetNextAssoc(it, key, obj);
			if (obj->IsSelected() && (obj->availableSpellMask & (1 << (spell - 1))) != 0) {
				pkt->AppendWord(obj->unit_id);
			}
		}
		g_NetStru1_local.QueuePacketSend(pkt);
	}
	if (wnd->vis_spellbook->spell >= 0 && this->field_0x140 == 1 && this->field_0x138 != nullptr) {
		pkt->id = 0x26;
		pkt->count = 0;
		pkt->AppendWord(this->field_0x138->unit_id);
		uint16_t slot = wnd->vis_spellbook->spell;
		pkt->slot = slot;
		pkt->slot_high = slot >> 8;
		g_NetStru1_local.QueuePacketSend(pkt);
		TokenEntry* entry = this->field_0x138->tokenEntries[slot];
		if (entry->count == 1) {
			this->field_0x138->tokenEntries.RemoveAt(slot, 1);
			delete entry;
		} else {
			entry->count--;
		}
		wnd->vis_spellbook->spell = -1;
		wnd->vis_spellbook->selected = -1;
	} else {
		if (wnd->vis_spellbook->sub_4CA8E0(spell - 1) != 0) {
			for (it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
				uint16_t key;
				CGameObject* obj;
				this->field_0x9d0.GetNextAssoc(it, key, obj);
				if (!obj->IsSelected() || (obj->activeSpellEffectMask & (1 << (spell - 1))) == 0) {
					continue;
				}
				pkt->count = 0;
				pkt->AppendWord(obj->unit_id);
				uint16_t slot = 0xFFFF;
				if (wnd->vis_spellbook->spell >= 0) {
					slot = wnd->vis_spellbook->spell;
				} else {
					for (int32_t i = 0; i < obj->tokenEntries.GetSize(); i++) {
						TokenEntry* entry = obj->tokenEntries[i];
						if (entry != nullptr && (entry->flg & 0x10) != 0 && entry->GetCastSpellId() == BOOK_POS_TO_SPELL_ID[spell]) {
							slot = i;
							break;
						}
					}
				}
				pkt->slot = slot;
				pkt->slot_high = slot >> 8;
				if (slot == 0xFFFF) {
					continue;
				}
				pkt->id = 0x26;
				g_NetStru1_local.QueuePacketSend(pkt);
				TokenEntry* entry = obj->tokenEntries[slot];
				if (entry->count == 1) {
					obj->tokenEntries.RemoveAt(slot, 1);
					delete entry;
				} else {
					entry->count--;
				}
				wnd->vis_spellbook->spell = -1;
				wnd->vis_spellbook->selected = -1;
			}
		}
	}
	this->UpdateSelectionState();
}

// 419BA1
void BigStruct2::sub_419BA1(uint16_t id, int32_t spell) {
	POSITION it = this->field_0x9d0.GetStartPosition();
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param = id;
	pkt->count = 0;
	if (wnd->vis_spellbook->sub_4CA89B(spell - 1) != 0 && wnd->vis_spellbook->spell == -1) {
		pkt->id = 0x1E;
		pkt->slot = spell;
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->field_0x9d0.GetNextAssoc(it, key, obj);
			if (obj->IsSelected() && (obj->availableSpellMask & (1 << (spell - 1))) != 0) {
				pkt->AppendWord(obj->unit_id);
			}
		}
		g_NetStru1_local.QueuePacketSend(pkt);
	}
	if (wnd->vis_spellbook->spell >= 0 && this->field_0x140 == 1 && this->field_0x138 != nullptr) {
		pkt->count = 0;
		pkt->AppendWord(this->field_0x138->unit_id);
		uint16_t slot = wnd->vis_spellbook->spell;
		pkt->slot = slot;
		pkt->slot_high = slot >> 8;
		pkt->id = 0x25;
		g_NetStru1_local.QueuePacketSend(pkt);
		TokenEntry* entry = this->field_0x138->tokenEntries[slot];
		if (entry->count == 1) {
			this->field_0x138->tokenEntries.RemoveAt(slot, 1);
			delete entry;
		} else {
			entry->count--;
		}
		wnd->vis_spellbook->spell = -1;
		wnd->vis_spellbook->selected = -1;
	} else {
		if (wnd->vis_spellbook->sub_4CA8E0(spell - 1) != 0) {
			for (it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
				uint16_t key;
				CGameObject* obj;
				this->field_0x9d0.GetNextAssoc(it, key, obj);
				if (!obj->IsSelected() || (obj->activeSpellEffectMask & (1 << (spell - 1))) == 0) {
					continue;
				}
				pkt->count = 0;
				pkt->AppendWord(obj->unit_id);
				uint16_t slot = 0xFFFF;
				if (wnd->vis_spellbook->spell >= 0) {
					slot = wnd->vis_spellbook->spell;
				} else {
					for (int32_t i = 0; i < obj->tokenEntries.GetSize(); i++) {
						TokenEntry* entry = obj->tokenEntries[i];
						if (entry != nullptr && (entry->flg & 0x10) != 0 && entry->GetCastSpellId() == BOOK_POS_TO_SPELL_ID[spell]) {
							slot = i;
							break;
						}
					}
				}
				pkt->slot = slot;
				pkt->slot_high = slot >> 8;
				if (slot == 0xFFFF) {
					continue;
				}
				pkt->id = 0x25;
				g_NetStru1_local.QueuePacketSend(pkt);
				TokenEntry* entry = obj->tokenEntries[slot];
				if (entry->count == 1) {
					obj->tokenEntries.RemoveAt(slot, 1);
					delete entry;
				} else {
					entry->count--;
				}
				wnd->vis_spellbook->spell = -1;
				wnd->vis_spellbook->selected = -1;
			}
		}
	}
	this->UpdateSelectionState();
}

// 40328E
void BigStruct2::sub_40328E() {
	free(this->tile_units);
	free(this->tile_air_units);
	free(this->tile_structures);
	free(this->tile_backpacks);
	free(this->tile_special_units);
	free(this->field_0xa0);
	free(this->field_0xa4);
	free(this->field_0xa8);
	free(this->field_0xac);
	free(this->tile_light);
	free(this->tile_corner_y);
	free(this->tile_top_y);
	free(this->tile_bottom_y);
	free(this->height_cells);
}

// 40403B
void BigStruct2::sub_40403B() {
	this->field_0xf4.SetRectEmpty();
	memset(this->tile_units, 0, this->tile_grid_bytes);
	memset(this->tile_air_units, 0, this->tile_grid_bytes);
	memset(this->tile_structures, 0, this->tile_grid_bytes);
	memset(this->tile_backpacks, 0, this->tile_grid_bytes);
	memset(this->tile_special_units, 0, this->tile_grid_bytes);
	if (g_DWORD_00659d18 == 2) {
		memcpy(this->field_0xac, this->field_0xa8, this->corner_grid_bytes / 4);
	}
	memset(this->field_0xa8, 0xFF, this->corner_grid_bytes / 4);
	this->field_0x108 = 0;
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		obj->UpdateMapPosition();
		obj->UpdateTileLayer();
		obj->UpdateLight();
	}
	memset(this->field_0xa0, 0, this->corner_grid_bytes);
	uint16_t* landscape = this->field_0x80->GetLandscape();
	int32_t src_idx = (this->view_y - 3) * this->map_width + this->view_x - 3;
	int32_t out_idx = 0;
	for (int32_t y = -3; y <= this->view_tiles_y + 7; y++) {
		for (int32_t x = -3; x <= this->view_tiles_x + 3; x++) {
			int32_t h_idx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
			if (x == this->view_tiles_x + 3) {
				h_idx--;
			}
			if (y == this->view_tiles_y + 7) {
				h_idx -= this->view_tiles_x + 6;
			}
			uint16_t tile = landscape[src_idx] & 0xC000;
			src_idx++;
			if (tile == 0xC000) {
				if (g_Animation != 0) {
					CRect rect((x - 3) * 0x20, this->tile_top_y[h_idx] - 0x60, (x + 3) * 0x20, this->tile_bottom_y[h_idx] + 0x60);
					this->field_0xf4 |= rect;
				} else if (this->field_0xdc != 0) {
					CRect rect((x - 1) * 0x20, this->tile_top_y[h_idx] - 0x60, (x + 2) * 0x20, this->tile_bottom_y[h_idx] + 0x60);
					this->field_0xf4 |= rect;
				}
				this->field_0xa0[out_idx] = 0;
			} else if (tile == 0x8000) {
				this->field_0xa0[out_idx] = 8;
			} else {
				this->field_0xa0[out_idx] = 0x10;
			}
			out_idx++;
		}
		src_idx += this->map_width - this->view_tiles_x - 7;
	}
	memcpy(this->field_0xa4, this->field_0xa0, this->corner_grid_bytes);
	for (POSITION it = this->field_0x9ec.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9ec.GetNextAssoc(it, key, obj);
		obj->UpdateMapPosition();
		obj->UpdateLight();
	}
	for (POSITION it = this->field_0xa94.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		uint32_t value;
		this->field_0xa94.GetNextAssoc(it, key, value);
		int32_t brightness = -1;
		switch (value >> 1) {
		case 0:
			brightness = 0x24;
			break;
		case 1:
			brightness = 0x18;
			break;
		case 2:
			brightness = 0xC;
			break;
		case 3:
			brightness = 0;
			break;
		case 4:
			brightness = 0xC;
			break;
		case 5:
			brightness = 0;
			break;
		case 6:
			brightness = 0xC;
			break;
		case 7:
			brightness = 0;
			break;
		case 8:
			brightness = 0x14;
			break;
		case 9:
			brightness = 0x1C;
			break;
		case 10:
			brightness = 0x24;
			break;
		}
		int32_t lx = key & 0xFF;
		int32_t ly = key >> 8;
		if (brightness >= 0) {
			this->ApplyDynamicLight(lx, ly, 1, brightness);
		}
		if (g_Animation == 0) {
			lx -= this->view_x;
			ly -= this->view_y;
			CRect rect((lx - 1) * 0x20, (ly - 2) * 0x20, (lx + 2) * 0x20, (ly + 2) * 0x20);
			this->field_0xf4 |= rect;
		}
	}
	for (POSITION it = this->field_0xa08.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		uint32_t value;
		this->field_0xa08.GetNextAssoc(it, key, value);
		int32_t lx = key & 0xFF;
		int32_t ly = key >> 8;
		if (this->view_x - 3 <= lx && lx < this->view_x + 3 + this->view_tiles_x &&
			this->view_y - 3 <= ly && ly < this->view_y + 7 + this->view_tiles_y && value != 0) {
			lx -= this->view_x;
			ly -= this->view_y;
			if (g_Lightning == 0) {
				CRect rect((lx - 1) * 0x20, (ly - 3) * 0x20, (lx + 2) * 0x20, (ly + 2) * 0x20);
				this->field_0xf4 |= rect;
			} else {
				CRect rect((lx - 3) * 0x20, (ly - 3) * 0x20, (lx + 4) * 0x20, (ly + 4) * 0x20);
				this->field_0xf4 |= rect;
			}
		}
	}
	for (int32_t i = 0; i < this->damage_labels.GetSize(); i++) {
		TakeDamage& dmg = this->damage_labels.ElementAt(i);
		int32_t left = (dmg.FUN_0041f620() - 0x30) & ~0x1F;
		int32_t top = dmg.FUN_0041f640() - 0x30;
		int32_t right = ((dmg.FUN_0041f620() + 0x30) & ~0x1F) + 0x20;
		int32_t bottom = dmg.FUN_0041f640() + 0x30;
		CRect rect(left, top, right, bottom);
		this->field_0xf4 |= rect;
	}
	this->sub_404A2C();
}

// 404912
void BigStruct2::sub_404912() {
	uint8_t* buf = this->field_0x80->GetOverlayMap();
	for (POSITION it = this->field_0xa08.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		uint32_t value;
		this->field_0xa08.GetNextAssoc(it, key, value);
		int32_t x = key & 0xFF;
		int32_t y = key >> 8;
		int32_t idx = y * this->map_width + x;
		if (value & 0x8000) {
			buf[idx] = 0;
			buf[idx + 1] = 0;
			buf[idx + this->map_width] = 0;
			buf[idx + this->map_width + 1] = 0;
		}
		if (value & 0x4000) {
			buf[idx] = 0x50;
			buf[idx + 1] = 0x50;
			buf[idx + this->map_width] = 0x50;
			buf[idx + this->map_width + 1] = 0x50;
		}
	}
}

// 403B8F
int32_t BigStruct2::sub_403B8F(int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4) {
	int32_t idx;
	if (arg2 > 0) {
		idx = arg2 * this->map_width + arg1;
	} else {
		idx = arg1 - (-arg2) * this->map_width;
	}
	int32_t nx = arg1 + (int8_t)this->field_0xac0[arg1][arg2][0];
	int32_t ny = arg2 + (int8_t)this->field_0xac0[arg1][arg2][1];
	uint8_t* heights = this->field_0x80->GetMapHeights();
	int32_t base = (int8_t)heights[arg3 + idx] - arg4;
	int32_t val = (int32_t)this->field_0x17e4[nx][ny] - ((int16_t)this->field_3228[arg1][arg2] + base);
	this->field_0x17e4[arg1][arg2] = val;
	return val <= 0;
}

// 41B155
void BigStruct2::sub_41B155() {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x4B;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41CE14
void BigStruct2::sub_41CE14() {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x4C;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41AC2D
void BigStruct2::sub_41AC2D(int32_t id) {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x46;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = 0x80;
	pkt->param2 = id;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41AC88
void BigStruct2::sub_41AC88(int32_t arg1, int32_t arg2) {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x23;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = arg1;
	pkt->param2 = arg2;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 404A2C
void BigStruct2::sub_404A2C() {
	int32_t delta = g_DeltaCLR.brightness;
	memset(this->tile_light, delta >> 2, this->tile_grid_bytes / 4);
	for (POSITION it = this->field_0xa08.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		uint32_t value;
		this->field_0xa08.GetNextAssoc(it, key, value);
		int32_t x = key & 0xFF;
		int32_t y = key >> 8;
		if (x < this->view_x - 3 || y < this->view_y - 3 ||
			x >= this->view_x + this->view_tiles_x + 3 || y >= this->view_y + this->view_tiles_y + 7) {
			continue;
		}
		int32_t idx = (x - this->view_x) + ((y - this->view_y) + 3) * (this->view_tiles_x + 6) + 3;
		if ((value & 0x8000) != 0) {
			this->tile_light[idx] = 0;
		}
		if ((value & 0x4000) != 0) {
			this->tile_light[idx] = 0xC;
		}
		if ((value & 8) != 0) {
			int32_t brightness = (abs(this->field_0xa88 / 2 + x * y) % 5 & 1) != 0 ? 0xC : 0;
			this->ApplyDynamicLight(x, y, 1, brightness);
		}
	}
	for (int32_t ty = 0; ty < this->view_tiles_y + 10; ty++) {
		for (int32_t tx = 0; tx < this->view_tiles_x + 6; tx++) {
			int32_t idx = ty * (this->view_tiles_x + 7) + tx;
			int32_t idx_next = idx + this->view_tiles_x + 7;
			int32_t v0 = this->field_0xa8[idx] - 0x20;
			if (v0 < 0) {
				v0 = 0;
			}
			int32_t v1 = this->field_0xa8[idx + 1] - 0x20;
			if (v1 < 0) {
				v1 = 0;
			}
			int32_t v2 = this->field_0xa8[idx_next] - 0x20;
			if (v2 < 0) {
				v2 = 0;
			}
			int32_t v3 = this->field_0xa8[idx_next + 1] - 0x20;
			if (v3 < 0) {
				v3 = 0;
			}
			if (v0 + v1 + v2 + v3 == 0x37C) {
				continue;
			}
			if (v0 == 0xDF) {
				v0 = delta;
			}
			if (v1 == 0xDF) {
				v1 = delta;
			}
			if (v2 == 0xDF) {
				v2 = delta;
			}
			if (v3 == 0xDF) {
				v3 = delta;
			}
			int32_t light = (v0 + v1 + v2 + v3) >> 4;
			if (light >= delta >> 2) {
				light = delta >> 2;
			}
			this->tile_light[ty * (this->view_tiles_x + 6) + tx] = light;
		}
	}
}

// 404DA3
void BigStruct2::sub_404DA3(CRect* rect, uint16_t unit_id) {
	this->field_0x10c.SetAtGrow(this->field_0x134, *rect);
	this->field_0x120.SetAtGrow(this->field_0x134, unit_id);
	this->field_0x134++;
}

// 41AB05
void BigStruct2::sub_41AB05() {
	int32_t mode = g_settings.AutoCasting & 0x38;
	if (mode == 8) {
		mode = 0x18;
	} else if (mode == 0x18) {
		mode = 0x38;
	} else if (mode == 0x38) {
		mode = 8;
	}
	g_settings.AutoCasting = (g_settings.AutoCasting & 7) | mode;
	this->FUN_0041ab74();
}

// 41A66E
void BigStruct2::sub_41A66E(uint16_t id) {
	POSITION it = this->field_0x9d0.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x24;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->param = id;
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr) {
		unit->PlayMoveVoice();
	}
}

// 41A7C7
void BigStruct2::sub_41A7C7(uint8_t arg1, uint16_t arg2, uint8_t arg3, uint16_t arg4, uint16_t arg5) {
	PacketItemTransfer* pkt = &PacketItemTransfer::Inst;
	pkt->id = 0x22;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->unit_id = this->field_0x138->unit_id;
	pkt->type = arg1;
	pkt->subtype = arg3;
	pkt->param1 = arg2;
	pkt->param2 = arg4;
	pkt->param3 = arg5;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41A5B4
void BigStruct2::sub_41A5B4() {
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x12;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr) {
		unit->PlayDefendVoice();
	}
}

// 41A33F
void BigStruct2::sub_41A33F(uint16_t x, uint16_t y) {
	POSITION it = this->field_0x9d0.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x21;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->x = x;
	pkt->y = y;
	pkt->y_high = y >> 8;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->param = obj->shopInventoryVisibleStart;
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr) {
		unit->PlayPickupVoice();
	}
}

// 41A425
void BigStruct2::sub_41A425() {
	POSITION it = this->field_0x9d0.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x14;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr) {
		unit->PlayRetreatVoice();
	}
}

// 41A4DF
void BigStruct2::sub_41A4DF() {
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = 0x13;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected() && (((CUnit*)obj)->unitFlags & 1)) {
			pkt->AppendWord(obj->unit_id);
			break;
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->sub_41DFDB();
	if (unit != nullptr) {
		unit->PlayPickupVoice();
	}
}

// 4184B8
int32_t BigStruct2::sub_4184B8(int32_t x, int32_t y) {
	int32_t tx = (x >> 5) + 3;
	int32_t frac = x & 0x1F;
	for (int32_t ty = -3; ty < this->view_tiles_y + 7; ty++) {
		int32_t idx6 = tx + (ty + 3) * (this->view_tiles_x + 6);
		int32_t h_min = this->tile_top_y[idx6];
		if (h_min > y) {
			continue;
		}
		int32_t h_max = this->tile_bottom_y[idx6];
		if (h_max < y) {
			continue;
		}
		int32_t idx7 = tx + (ty + 3) * (this->view_tiles_x + 7);
		int32_t h1 = this->tile_corner_y[idx7];
		int32_t h2 = this->tile_corner_y[idx7 + 1];
		int32_t top = h1 + (h2 - h1) * frac / 32;
		int32_t idx7b = tx + (ty + 4) * (this->view_tiles_x + 7);
		int32_t h3 = this->tile_corner_y[idx7b];
		int32_t h4 = this->tile_corner_y[idx7b + 1];
		int32_t bottom = h3 + (h4 - h3) * frac / 32;
		if (top > y) {
			continue;
		}
		if (bottom < y) {
			continue;
		}
		return ty;
	}
	return this->view_tiles_y + 4;
}

// 41864D
uint32_t BigStruct2::sub_41864D() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	uint32_t avail = 0;
	CUnit* old_unit = this->field_0x994;
	uint16_t old_id = (uint16_t)this->field_0x9a8;
	this->field_0x994 = nullptr;
	this->field_0x9a8 = 0;
	this->quest_some_id_2 = 0;
	this->quest_landmark_some_id = 0;
	this->quest_building_some_id = 0;
	this->quest_some_id = 0;

	CRect screen_rect;
	this->ClientRectToScreen(&screen_rect, &this->rect);
	CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
	if (!screen_rect.PtInRect(mouse_pt)) {
		if (this->field_0x994 != old_unit) {
			wnd->vis_charinfo->MsgProc(0x408, 0, 0);
			wnd->vis_sidestatus->MsgProc(0x408, 0, 0);
		}
		return avail;
	}

	CRect mouse_rect(g_mousept.x - 1, g_mousept.y - 1, g_mousept.x + 1, g_mousept.y + 1);
	for (int32_t i = this->field_0x134 - 1; i >= 0; i--) {
		CRect unit_rect = this->field_0x10c[i];
		unit_rect.NormalizeRect();
		CRect intersect_rect;
		if (!intersect_rect.IntersectRect(&unit_rect, &mouse_rect)) {
			continue;
		}
		CGameObject* obj;
		if (!this->field_0x9d0.Lookup(this->field_0x120[i], obj)) {
			break;
		}
		const char* class_name = obj->GetRuntimeClass()->m_lpszClassName;
		if (strcmp(class_name, "CUnit") == 0) {
			avail |= 1;
		}
		if (strcmp(class_name, "CAirUnit") == 0) {
			avail |= 2;
		}
		if (strcmp(class_name, "CStructure") == 0) {
			StructureInfo* info = g_StructuresInfo[obj->typeId];
			if (info->usable != 0) {
				avail |= 0x800;
			} else if (info->indestructible != 0) {
				continue;
			}
			avail |= 0x20;
		}
		if (this->my_main_unit->IsAllied(obj->map_player->index) != 0) {
			avail |= 4;
		}
		this->field_0x994 = (CUnit*)obj;
		if (wnd->sessionMode != 2) {
			for (POSITION it = this->field_0x4970->quests_map.GetStartPosition(); it != nullptr;) {
				uint32_t quest_id;
				Quest* quest;
				this->field_0x4970->quests_map.GetNextAssoc(it, quest_id, quest);
				if (this->field_0x994->unit_id == quest->GetObj() && quest->IsInWork() != 0) {
					if (quest->Kind() != 3 && quest->Kind() != 0xC) {
						this->quest_some_id_2 = quest->GetSomeId();
					}
				} else if (this->field_0x994->unit_id == (quest->GetLandmarkId() & 0xFFFF)) {
					this->quest_landmark_some_id = quest->GetSomeId();
				}
				if (this->field_0x994->unit_id == quest->GetBuildingId()) {
					this->quest_building_some_id = quest->GetSomeId();
				}
				if (this->field_0x994->IsKindOf(RUNTIME_CLASS(CUnit)) && quest->IsInWork() != 0) {
					if (this->field_0x994->questFlags == quest->GetObj() && (quest->Kind() == 3 || quest->Kind() == 0xC)) {
						this->quest_some_id = quest->GetSomeId();
					}
					if (this->field_0x994->typeId == (quest->GetObj() & 0xFF) && this->field_0x994->face == (quest->GetObj() >> 8) && quest->Kind() == 2) {
						this->quest_some_id_2 = quest->GetSomeId();
					}
					if (this->field_0x994->map_player->index == quest->GetObj() && quest->Kind() == 0xD) {
						this->quest_some_id_2 = quest->GetSomeId();
					}
				}
			}
		}
		this->field_0x9a8 = this->field_0x120[i];
		break;
	}

	CVisualObject* child2 = this->FindChild(2);
	CVisualObject* child3 = this->FindChild(3);
	if (child2 != nullptr) {
		CPoint pt(g_mousept.GetX(), g_mousept.GetY());
		if (child2->GetRect().PtInRect(pt)) {
			this->field_0x994 = nullptr;
			this->field_0x9a8 = 0;
		}
	}
	if (child3 != nullptr) {
		CPoint pt(g_mousept.GetX(), g_mousept.GetY());
		if (child3->GetRect().PtInRect(pt)) {
			this->field_0x994 = nullptr;
			this->field_0x9a8 = 0;
		}
	}

	int32_t tile_x = g_mousept.x >> 5;
	int32_t tile_y = this->sub_4184B8(g_mousept.x, g_mousept.y);
	uint16_t* land = this->field_0x80->GetLandscape();
	int32_t cell = tile_x + this->view_x + (tile_y + this->view_y) * this->map_width;
	uint16_t land_flags = (land[cell] & 0xC000) | (land[cell + 1] & 0xC000) |
		(land[cell + 1 + this->map_width] & 0xC000) | (land[cell + this->map_width] & 0xC000);
	if (land_flags == 0xC000 && this->tile_backpacks[tile_x + 3 + (tile_y + 3) * (this->view_tiles_x + 6)] != 0) {
		avail |= 0x40;
	}
	if (land_flags != 0xC000) {
		avail |= 0x400;
	}
	tile_x += this->view_x;
	tile_y += this->view_y;
	if (this->field_0x994 != old_unit) {
		wnd->vis_charinfo->MsgProc(0x408, 0, 0);
		wnd->vis_sidestatus->MsgProc(0x408, 0, 0);
		if (this->field_0x994 != nullptr) {
			this->field_0x994->m_bSelectionDirty = 1;
		}
		CGameObject* prev_obj = old_unit;
		if (this->field_0x9d0.Lookup(old_id, prev_obj)) {
			prev_obj->m_bSelectionDirty = 1;
		}
	}
	this->field_0x9ac = tile_x;
	this->field_0x9b0 = tile_y;
	return avail;
}

// 40B314
void BigStruct2::sub_40B314() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->dialogsMask != 1) {
		return;
	}
	CSprite256* cursor_sprite = g_mousept.GetCursorSprite();
	CRect screen_rect;
	this->ClientRectToScreen(&screen_rect, &this->rect);
	CCursor* cursor = nullptr;
	CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
	CRect select_frame = g_mousept.GetSelectFrame();
	select_frame.NormalizeRect();
	if (!screen_rect.PtInRect(mouse_pt)) {
		this->sub_41864D();
		return;
	}
	int32_t kb_control = g_kbControlState;
	int32_t kb_menu = g_kbMenuState;
	if (g_mousept.GetX() == 0) {
		if (g_mousept.GetY() == 0) {
			cursor = g_Cursors[CURSOR_ARROW7];
		} else if (g_mousept.GetY() < g_ScreenSize.bottom - 2) {
			cursor = g_Cursors[CURSOR_ARROW6];
		} else {
			cursor = g_Cursors[CURSOR_ARROW5];
		}
	} else if (g_mousept.GetX() < g_ScreenSize.right - 2) {
		if (g_mousept.GetY() == 0) {
			cursor = g_Cursors[CURSOR_ARROW0];
		} else if (g_mousept.GetY() < g_ScreenSize.bottom - 2) {
			if (screen_rect.PtInRect(mouse_pt)) {
				if (this->field_0x9b4 != 0) {
					switch (this->field_0x9b4) {
					case 1:
						cursor = g_Cursors[CURSOR_ATTACK];
						break;
					case 2:
						cursor = g_Cursors[CURSOR_MOVE];
						break;
					case 4:
						cursor = g_Cursors[CURSOR_DEFEND];
						break;
					case 5:
						cursor = g_Cursors[CURSOR_CAST];
						break;
					case 6:
						cursor = g_Cursors[CURSOR_SWARM];
						break;
					case 8:
						cursor = g_Cursors[CURSOR_PATROL];
						break;
					}
					uint32_t avail = this->sub_41864D();
					if ((avail & 0x400) != 0 && cursor == g_Cursors[CURSOR_CAST]) {
						cursor = g_Cursors[CURSOR_MOVE];
					}
					if (g_mousept.GetSelectState() != 0 && !select_frame.IsRectEmpty()) {
						cursor = g_Cursors[CURSOR_DEFAULT];
					}
				} else if (this->IsBookOpen() != 0 && wnd->vis_spellbook->sub_41F9B0() >= 0 &&
					(this->field_0x148 != 0 || this->field_0x150 != 0) && (this->field_0x144 & 4) == 0) {
					uint32_t avail = this->sub_41864D();
					cursor = g_Cursors[CURSOR_CAST];
					if ((avail & 0x400) != 0) {
						cursor = g_Cursors[CURSOR_MOVE];
					}
					if ((avail & 3) == 0 && wnd->vis_spellbook->sub_4CAAA7() != 0) {
						cursor = g_Cursors[CURSOR_MOVE];
					}
					int32_t pickup_flag = (this->field_0x140 == 1 && (this->field_0x144 & 1) != 0) ? 1 : 0;
					int32_t town_flag = pickup_flag;
					if (pickup_flag != 0) {
						pickup_flag = ((CUnit*)this->field_0x138)->unitFlags & 1;
						if (pickup_flag != 0) {
							pickup_flag = (avail == 0x40 || (this->field_0x994 == this->field_0x138 && (avail & 0x40) != 0)) ? 1 : 0;
						}
					}
					if (town_flag != 0) {
						town_flag = ((((CUnit*)this->field_0x138)->unitFlags & 1) != 0 && (avail & 0x800) != 0) ? 1 : 0;
					}
					if (kb_menu != 0) {
						cursor = g_Cursors[CURSOR_MOVE];
					} else if (kb_control != 0) {
						if ((avail & 3) != 0) {
							cursor = g_Cursors[CURSOR_ATTACK];
						} else {
							cursor = g_Cursors[CURSOR_SWARM];
						}
					} else if ((avail & 0x24) == 0 && g_kbShiftState != 0) {
						cursor = g_Cursors[CURSOR_SELECT];
					}
					if (pickup_flag != 0 && cursor == g_Cursors[CURSOR_MOVE]) {
						cursor = g_Cursors[CURSOR_PICKUP];
					}
					if (cursor == g_Cursors[CURSOR_CAST] && wnd->vis_spellbook->sub_4CAAA7() != 0 && (avail & 4) == 0 &&
						wnd->vis_spellbook->sub_41F9E0() == 0 && dword_62F908[wnd->vis_spellbook->sub_41F9B0()] != 0) {
						if ((avail & 3) != 0) {
							cursor = g_Cursors[CURSOR_SELECT];
						} else {
							cursor = g_Cursors[CURSOR_MOVE];
						}
					}
					if (town_flag != 0 && (avail & 4) == 0) {
						cursor = g_Cursors[CURSOR_TOWN];
					}
					this->sub_41864D();
				} else {
					uint32_t avail = this->sub_41864D();
					if (this->field_0x140 == 0) {
						if ((avail & 0x23) != 0) {
							cursor = g_Cursors[CURSOR_SELECT];
						} else {
							cursor = g_Cursors[CURSOR_DEFAULT];
						}
					} else if ((this->field_0x144 & 0x24) != 0) {
						if ((avail & 0x23) != 0) {
							cursor = g_Cursors[CURSOR_SELECT];
						} else {
							cursor = g_Cursors[CURSOR_DEFAULT];
						}
					} else {
						int32_t pickup_flag = (this->field_0x140 == 1 && (this->field_0x144 & 1) != 0) ? 1 : 0;
						int32_t town_flag = pickup_flag;
						if (pickup_flag != 0) {
							pickup_flag = ((CUnit*)this->field_0x138)->unitFlags & 1;
							if (pickup_flag != 0) {
								pickup_flag = (avail == 0x40 || (this->field_0x994 == this->field_0x138 && (avail & 0x40) != 0)) ? 1 : 0;
							}
						}
						if (town_flag != 0) {
							town_flag = ((((CUnit*)this->field_0x138)->unitFlags & 1) != 0 && (avail & 0x800) != 0) ? 1 : 0;
						}
						if ((avail & 4) != 0) {
							if (kb_menu != 0) {
								cursor = g_Cursors[CURSOR_MOVE];
							} else if ((avail & 0x20) == 0) {
								if (kb_control != 0) {
									cursor = g_Cursors[CURSOR_SWARM];
								} else {
									cursor = g_Cursors[CURSOR_ATTACK];
								}
							} else {
								cursor = g_Cursors[CURSOR_SELECT];
							}
						} else if ((avail & 0x23) != 0) {
							if (kb_menu != 0) {
								cursor = g_Cursors[CURSOR_MOVE];
							} else if (kb_control != 0) {
								cursor = g_Cursors[CURSOR_ATTACK];
							} else if (pickup_flag != 0) {
								cursor = g_Cursors[CURSOR_PICKUP];
							} else if (town_flag != 0) {
								cursor = g_Cursors[CURSOR_TOWN];
							} else {
								cursor = g_Cursors[CURSOR_SELECT];
							}
						} else {
							if (kb_control != 0) {
								cursor = g_Cursors[CURSOR_SWARM];
							} else if (kb_menu != 0) {
								cursor = g_Cursors[CURSOR_MOVE];
							} else if (pickup_flag != 0) {
								cursor = g_Cursors[CURSOR_PICKUP];
							} else {
								cursor = g_Cursors[CURSOR_MOVE];
							}
						}
					}
					if (g_mousept.GetSelectState() != 0 && !select_frame.IsRectEmpty()) {
						cursor = g_Cursors[CURSOR_DEFAULT];
					}
				}
			}
		} else {
			cursor = g_Cursors[CURSOR_ARROW4];
		}
	} else {
		if (g_mousept.GetY() == 0) {
			cursor = g_Cursors[CURSOR_ARROW1];
		} else if (g_mousept.GetY() < g_ScreenSize.bottom - 2) {
			cursor = g_Cursors[CURSOR_ARROW2];
		} else {
			cursor = g_Cursors[CURSOR_ARROW3];
		}
	}
	CVisualObject* child2 = this->FindChild(2);
	CVisualObject* child3 = this->FindChild(3);
	if (child2 != nullptr && child2->GetRect().PtInRect(mouse_pt)) {
		cursor = g_Cursors[CURSOR_DEFAULT];
	}
	if (child3 != nullptr && child3->GetRect().PtInRect(mouse_pt)) {
		cursor = g_Cursors[CURSOR_DEFAULT];
	}
	if (wnd->drag_entry != nullptr) {
		uint32_t avail = this->sub_41864D();
		cursor = wnd->item_cursor;
		if (wnd->dialogsMask == 1 && this->field_0x140 == 1 &&
			(((CUnit*)this->field_0x138)->unitFlags & 1) != 0 &&
			this->field_0x138->map_player == this->my_main_unit) {
			int32_t tile_x = (g_mousept.GetX() >> 5) + this->view_x;
			int32_t tile_y = this->sub_4184B8(g_mousept.GetX(), g_mousept.GetY()) + this->view_y;
			if (abs(tile_x - this->field_0x138->tileX) <= 2 &&
				abs(tile_y - this->field_0x138->tileY) <= 2 && (avail & 1) != 0) {
				cursor = g_Cursors[CURSOR_BACKPACK];
			}
		}
	}
	if (cursor != nullptr && cursor_sprite != cursor->GetSprite()) {
		cursor->Use();
	}
}

// 404E1A
void BigStruct2::sub_404E1A() {
	AfxGetMainWnd();
	timeGetTime();
	this->field_0x80->GetMapHeights();
	uint8_t* f18map = this->field_0x80->GetOverlayMap();
	uint16_t* landscape = this->field_0x80->GetLandscape();
	CRect rect;
	this->ClientRectToScreen(&rect, this->rect);
	for (int32_t y = 0; y < this->view_tiles_y + 4; y++) {
		for (int32_t x = this->view_tiles_x - 1; x >= 0; x--) {
			int32_t tile_x = x + this->view_x;
			int32_t tile_y = y + this->view_y;
			int32_t h_row = this->view_tiles_x + 7;
			int32_t cell = tile_x + tile_y * this->map_width;
			int32_t h_idx = (y + 3) * h_row + 3;
			int32_t cell2 = cell + this->map_width;
			int32_t h_idx2 = (y + 4) * h_row + 3;
			int32_t h1 = this->tile_corner_y[x + h_idx];
			int32_t h2 = this->tile_corner_y[x + h_idx + 1];
			int32_t h3 = this->tile_corner_y[x + h_idx2];
			int32_t h4 = this->tile_corner_y[x + h_idx2 + 1];
			uint16_t tile_id = landscape[cell] & 0x1FFF;
			int32_t ter_type = tile_id >> 6;
			int32_t ter_var = (tile_id >> 4) & 3;
			bool animated = ter_type >= 8 && ter_type < 12;
			if (animated) {
				int32_t frame = (ter_type + (tile_x + 1) * tile_y + (this->field_0xa88 >> 2)) & 3;
				if (g_Animation == 0) {
					frame = 0;
				}
				ter_type = frame + 8;
			}
			CGameBitmap* tile = g_ter_tiles(ter_var, ter_type);
			if ((landscape[cell] & 0x2000) != 0 && !animated) {
				memcpy(this->field_0x3f68->GetData(), (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400, 0x400);
				CopyIndexedSkip0((uint8_t*)this->field_0x3f68->GetData(),
					(uint8_t*)DAT_00665344->GetData() + ((tile_x + tile_y * 5) & 3) * 0x400, 0x400);
				tile = this->field_0x3f68;
				tile_id &= 0xFFF0;
			}
			int32_t l1 = f18map[cell];
			int32_t l2 = f18map[cell + 1];
			int32_t l3 = f18map[cell2];
			int32_t l4 = f18map[cell2 + 1];
			uint16_t* pal = DAT_00661018[ter_type]->GetPalette(0);
			uint8_t* tile_data = (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400;
			if (h1 == h2 && h3 == h4 && h1 + 0x20 == h3) {
				DrawFlatTile(x * 0x20, h1, l1, l2, l3, l4, tile_data, pal);
			} else {
				DrawDeformedTile(x * 0x20, (x + 1) * 0x20, h1, h2, h3, h4, l1, l2, l3, l4, tile_data, pal);
			}
			if (DAT_00660f68 != 0) {
				FillRectColor(x * 0x20, h1, (x + 1) * 0x20, h2, 0);
				FillRectColor(x * 0x20, h1, x * 0x20, h3, 0);
			}
		}
	}
}

// 405399
void BigStruct2::sub_405399(int32_t arg) {
	this->field_0x80->GetMapHeights();
	uint8_t* f18map = this->field_0x80->GetOverlayMap();
	uint16_t* landscape = this->field_0x80->GetLandscape();
	CRect rect;
	this->ClientRectToScreen(&rect, this->rect);
	int32_t y_off = (this->tile_scroll_y + arg) * 0x20;
	for (int32_t y = 0; y < this->view_tiles_y + 4; y++) {
		for (int32_t x = this->view_tiles_x - 1; x >= 0; x--) {
			int32_t map_x = x + this->tile_scroll_x;
			if (map_x >= this->view_tiles_x) {
				map_x -= this->view_tiles_x;
			}
			int32_t tile_x = x + this->view_x;
			int32_t tile_y = y + this->view_y;
			int32_t cell = tile_x + tile_y * this->map_width;
			if (((landscape[cell] & 0xC000) | (landscape[cell + 1] & 0xC000) |
				(landscape[cell + this->map_width] & 0xC000) |
				(landscape[cell + this->map_width + 1] & 0xC000)) != 0xC000) {
				continue;
			}
			int32_t h_idx = x + 3 + (y + 3) * (this->view_tiles_x + 7);
			int32_t h1 = this->tile_corner_y[h_idx] + y_off;
			int32_t h2 = this->tile_corner_y[h_idx + 1] + y_off;
			int32_t h_idx2 = h_idx + this->view_tiles_x + 7;
			int32_t h3 = this->tile_corner_y[h_idx2] + y_off;
			int32_t h4 = this->tile_corner_y[h_idx2 + 1] + y_off;
			uint16_t tile_id = landscape[cell] & 0x1FFF;
			int32_t ter_type = tile_id >> 6;
			int32_t ter_var = (tile_id >> 4) & 3;
			bool animated = ter_type >= 8 && ter_type < 12;
			if (!animated) {
				continue;
			}
			int32_t frame = (ter_type + tile_x + tile_x * tile_y + (this->field_0xa88 >> 2)) & 3;
			if (g_Animation == 0) {
				frame = 0;
			}
			ter_type = frame + 8;
			CGameBitmap* tile = g_ter_tiles(ter_var, ter_type);
			int32_t cell2 = cell + this->map_width;
			int32_t l1 = f18map[cell];
			int32_t l2 = f18map[cell + 1];
			int32_t l3 = f18map[cell2];
			int32_t l4 = f18map[cell2 + 1];
			uint16_t* pal = DAT_00661018[ter_type]->GetPalette(0);
			uint8_t* tile_data = (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400;
			if (h1 == h2 && h3 == h4 && h1 + 0x20 == h3) {
				DrawFlatTile(map_x * 0x20, h1, l1, l2, l3, l4, tile_data, pal);
			} else {
				DrawDeformedTile(map_x * 0x20, (map_x + 1) * 0x20, h1, h2, h3, h4, l1, l2, l3, l4, tile_data, pal);
			}
		}
	}
}

// 40587B
void BigStruct2::sub_40587B(int32_t arg) {
	AfxGetMainWnd();
	timeGetTime();
	this->field_0x80->GetMapHeights();
	uint8_t* f18map = this->field_0x80->GetOverlayMap();
	uint16_t* landscape = this->field_0x80->GetLandscape();
	CRect rect;
	this->ClientRectToScreen(&rect, this->rect);
	int32_t y_off = (this->tile_scroll_y + arg) * 0x20;
	for (int32_t y = 0; y < this->view_tiles_y + 4; y++) {
		for (int32_t x = this->view_tiles_x - 1; x >= 0; x--) {
			int32_t map_x = x + this->tile_scroll_x;
			if (map_x >= this->view_tiles_x) {
				map_x -= this->view_tiles_x;
			}
			int32_t tile_x = x + this->view_x;
			int32_t tile_y = y + this->view_y;
			int32_t h_row = this->view_tiles_x + 7;
			int32_t cell = tile_x + tile_y * this->map_width;
			int32_t h_idx = (y + 3) * h_row + 3;
			int32_t cell2 = cell + this->map_width;
			int32_t h_idx2 = (y + 4) * h_row + 3;
			int32_t h1 = this->tile_corner_y[x + h_idx] + y_off;
			int32_t h2 = this->tile_corner_y[x + h_idx + 1] + y_off;
			int32_t h3 = this->tile_corner_y[x + h_idx2] + y_off;
			int32_t h4 = this->tile_corner_y[x + h_idx2 + 1] + y_off;
			uint16_t tile_id = landscape[cell] & 0x1FFF;
			int32_t ter_type = tile_id >> 6;
			int32_t ter_var = (tile_id >> 4) & 3;
			bool animated = ter_type >= 8 && ter_type < 12;
			if (animated) {
				int32_t frame = (ter_type + (tile_x + 1) * tile_y + (this->field_0xa88 >> 2)) & 3;
				if (g_Animation == 0) {
					frame = 0;
				}
				ter_type = frame + 8;
			}
			CGameBitmap* tile = g_ter_tiles(ter_var, ter_type);
			if ((landscape[cell] & 0x2000) != 0 && !animated) {
				memcpy(this->field_0x3f68->GetData(), (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400, 0x400);
				CopyIndexedSkip0((uint8_t*)this->field_0x3f68->GetData(),
					(uint8_t*)DAT_00665344->GetData() + ((tile_x + tile_y * 5) & 3) * 0x400, 0x400);
				tile = this->field_0x3f68;
				tile_id &= 0xFFF0;
			}
			int32_t l1 = f18map[cell];
			int32_t l2 = f18map[cell + 1];
			int32_t l3 = f18map[cell2];
			int32_t l4 = f18map[cell2 + 1];
			uint16_t* pal = DAT_00661018[ter_type]->GetPalette(0);
			uint8_t* tile_data = (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400;
			if (h1 == h2 && h3 == h4 && h1 + 0x20 == h3) {
				DrawFlatTile(map_x * 0x20, h1, l1, l2, l3, l4, tile_data, pal);
			} else {
				DrawDeformedTile(map_x * 0x20, (map_x + 1) * 0x20, h1, h2, h3, h4, l1, l2, l3, l4, tile_data, pal);
			}
			if (DAT_00660f68 != 0) {
				FillRectColor(map_x * 0x20, h1, (map_x + 1) * 0x20, h2, 0);
				FillRectColor(map_x * 0x20, h1, map_x * 0x20, h3, 0);
			}
		}
	}
}

// 405D0E
void BigStruct2::sub_405D0E() {
	AfxGetMainWnd();
	this->field_0x80->GetMapHeights();
	uint8_t* f18map = this->field_0x80->GetOverlayMap();
	uint16_t* landscape = this->field_0x80->GetLandscape();
	CRect rect;
	this->ClientRectToScreen(&rect, this->rect);
	for (int32_t y = 0; y < this->view_tiles_y + 4; y++) {
		for (int32_t x = this->view_tiles_x - 1; x >= 0; x--) {
			int32_t vis_idx = x + 3 + (y + 3) * (this->view_tiles_x + 7);
			int32_t vis_idx2 = vis_idx + this->view_tiles_x + 7;
			int32_t l1 = this->field_0xa8[vis_idx];
			int32_t l2 = this->field_0xa8[vis_idx + 1];
			int32_t l3 = this->field_0xa8[vis_idx2];
			int32_t l4 = this->field_0xa8[vis_idx2 + 1];
			if (l1 + l2 + l3 + l4 == 0x3FC) {
				continue;
			}
			int32_t tile_x = x + this->view_x;
			int32_t tile_y = y + this->view_y;
			int32_t h_row = this->view_tiles_x + 7;
			int32_t cell = tile_x + tile_y * this->map_width;
			int32_t h_idx = (y + 3) * h_row + 3;
			int32_t cell2 = cell + this->map_width;
			int32_t h_idx2 = (y + 4) * h_row + 3;
			l1 = (std::min)(l1, (int32_t)f18map[cell]);
			l2 = (std::min)(l2, (int32_t)f18map[cell + 1]);
			l3 = (std::min)(l3, (int32_t)f18map[cell2]);
			l4 = (std::min)(l4, (int32_t)f18map[cell2 + 1]);
			int32_t h1 = this->tile_corner_y[x + h_idx];
			int32_t h2 = this->tile_corner_y[x + h_idx + 1];
			int32_t h3 = this->tile_corner_y[x + h_idx2];
			int32_t h4 = this->tile_corner_y[x + h_idx2 + 1];
			uint16_t tile_id = landscape[cell] & 0x1FFF;
			int32_t ter_type = tile_id >> 6;
			int32_t ter_var = (tile_id >> 4) & 3;
			bool animated = ter_type >= 8 && ter_type < 12;
			if (animated) {
				int32_t frame = (ter_type + (tile_x + 1) * tile_y + (this->field_0xa88 >> 2)) & 3;
				if (g_Animation == 0) {
					frame = 0;
				}
				ter_type = frame + 8;
			}
			CGameBitmap* tile = g_ter_tiles(ter_var, ter_type);
			if ((landscape[cell] & 0x2000) != 0 && !animated) {
				memcpy(this->field_0x3f68->GetData(), (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400, 0x400);
				CopyIndexedSkip0((uint8_t*)this->field_0x3f68->GetData(),
					(uint8_t*)DAT_00665344->GetData() + ((tile_x + tile_y * 5) & 3) * 0x400, 0x400);
				tile = this->field_0x3f68;
				tile_id &= 0xFFF0;
			}
			uint16_t* pal = DAT_00661018[ter_type]->GetPalette(0);
			uint8_t* tile_data = (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400;
			if (h1 == h2 && h3 == h4 && h1 + 0x20 == h3) {
				DrawFlatTile(x * 0x20, h1, l1, l2, l3, l4, tile_data, pal);
			} else {
				DrawDeformedTile(x * 0x20, (x + 1) * 0x20, h1, h2, h3, h4, l1, l2, l3, l4, tile_data, pal);
			}
			if (DAT_00660f68 != 0) {
				FillRectColor(x * 0x20, h1, (x + 1) * 0x20, h2, 0);
				FillRectColor(x * 0x20, h1, x * 0x20, h3, 0);
			}
		}
	}
}

// 406F7B
void BigStruct2::sub_406F7B() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->dialogsMask != 1 || this->field_0x80 == nullptr) {
		return;
	}

	CRect clip_rect;
	CRect local_rect;
	CDWordArray dummy_arr;

	this->field_0xa90++;
	this->ClientRectToScreen(&local_rect, this->rect);
	this->UpdateAmbientSounds();
	if (abs(this->scroll_dx) >= this->view_tiles_x / 2 || abs(this->scroll_dy) >= this->view_tiles_y / 2) {
		this->force_full_redraw = 1;
	}
	this->view_x += this->scroll_dx;
	this->view_y += this->scroll_dy;
	int32_t view_moved;
	if (this->view_x == this->prev_view_x && this->view_y == this->prev_view_y) {
		view_moved = 0;
	} else {
		view_moved = 1;
	}
	this->MakeHeightCells();
	this->sub_40403B();
	this->field_0xf4.IntersectRect(&this->field_0xf4, &local_rect);
	if (view_moved != 0) {
		this->field_0xf4 = local_rect;
	}
	if (this->field_0xe0 != 0) {
		this->field_0xf4 = local_rect;
		this->field_0xe0 = 0;
	}
	uint32_t anim_redraw = this->field_0xa88 - this->field_0xa8c > 3;
	if (anim_redraw != 0) {
		this->field_0xa8c = this->field_0xa88;
	}
	VisInvBase* child2 = (VisInvBase*)this->FindChild(2);
	CVisualObject* child3 = this->FindChild(3);
	CRect childs_rect(0, 0, 0, 0);
	int32_t childs_size = (int32_t)child2 + (int32_t)child3 + this->msglog.text.GetSize();
	if (child2 != nullptr) {
		childs_rect |= child2->GetRect();
	}
	if (child3 != nullptr) {
		childs_rect |= child3->GetRect();
	}
	if (childs_size != this->field_0x104) {
		this->field_0xf4 = local_rect;
	}
	if (child2 != nullptr && child2->sub_41F940() != 0) {
		this->field_0xf4 = local_rect;
	}
	g_mousept.Update();
	LockSurface3();
	GetClipRect(&clip_rect);
	if (this->force_full_redraw != 0) {
		this->sub_404E1A();
		this->tile_scroll_y = 0;
		this->tile_scroll_x = 0;
	} else if (this->scroll_dx != 0 || this->scroll_dy != 0) {
		int32_t old_tx = this->tile_scroll_x;
		int32_t old_ty = this->tile_scroll_y;
		this->tile_scroll_x += this->scroll_dx;
		if (this->tile_scroll_x < 0) {
			this->tile_scroll_x += this->view_tiles_x;
		}
		if (this->tile_scroll_x >= this->view_tiles_x) {
			this->tile_scroll_x -= this->view_tiles_x;
		}
		this->tile_scroll_y += this->scroll_dy;
		if (this->tile_scroll_y < 0) {
			this->tile_scroll_y += this->view_tiles_y;
		}
		if (this->tile_scroll_y >= this->view_tiles_y) {
			this->tile_scroll_y -= this->view_tiles_y;
		}
		CRect rc1;
		CRect rc2;
		CRect rc3;
		CRect rc4;
		if (this->scroll_dx != 0) {
			rc1.SetRect(old_tx << 5, 0, this->tile_scroll_x << 5, this->tile_scroll_y << 5);
			rc2.SetRect(old_tx << 5, this->tile_scroll_y << 5, this->tile_scroll_x << 5, this->view_tiles_y << 5);
			rc1.NormalizeRect();
			rc2.NormalizeRect();
			if (abs(this->tile_scroll_x - old_tx) < this->view_tiles_x / 2) {
				if (!rc1.IsRectEmpty()) {
					SetClipRect(rc1);
					this->sub_40587B(-this->view_tiles_y);
				}
				if (!rc2.IsRectEmpty()) {
					SetClipRect(rc2);
					this->sub_40587B(0);
				}
			} else {
				rc3.SetRect(0, rc1.TopLeft().y, rc1.TopLeft().x, rc1.BottomRight().y);
				if (!rc3.IsRectEmpty()) {
					SetClipRect(rc3);
					this->sub_40587B(-this->view_tiles_y);
				}
				rc3.SetRect(rc1.BottomRight().x, rc1.TopLeft().y, this->view_tiles_x << 5, rc1.BottomRight().y);
				if (!rc3.IsRectEmpty()) {
					SetClipRect(rc3);
					this->sub_40587B(-this->view_tiles_y);
				}
				rc3.SetRect(0, rc2.TopLeft().y, rc2.TopLeft().x, rc2.BottomRight().y);
				if (!rc3.IsRectEmpty()) {
					SetClipRect(rc3);
					this->sub_40587B(0);
				}
				rc3.SetRect(rc2.BottomRight().x, rc2.TopLeft().y, this->view_tiles_x << 5, rc2.BottomRight().y);
				if (!rc3.IsRectEmpty()) {
					SetClipRect(rc3);
					this->sub_40587B(0);
				}
			}
		}
		if (this->scroll_dy != 0) {
			rc1.SetRect(0, old_ty << 5, this->view_tiles_x << 5, this->tile_scroll_y << 5);
			rc1.NormalizeRect();
			if (abs(this->tile_scroll_y - old_ty) < this->view_tiles_y / 2) {
				SetClipRect(rc1);
				if (this->scroll_dy < 0) {
					this->sub_40587B(0);
				} else {
					this->sub_40587B(-this->view_tiles_y);
				}
			} else {
				rc2.SetRect(0, 0, this->view_tiles_x << 5, rc1.TopLeft().y);
				if (!rc2.IsRectEmpty()) {
					SetClipRect(rc2);
					this->sub_40587B(-this->view_tiles_y);
				}
				rc2.SetRect(0, rc1.BottomRight().y, this->view_tiles_x << 5, this->view_tiles_y << 5);
				if (!rc2.IsRectEmpty()) {
					SetClipRect(rc2);
					this->sub_40587B(0);
				}
			}
		}
	} else {
		if (anim_redraw != 0 && g_Animation != 0) {
			SetClipRect(0, this->tile_scroll_y << 5, this->view_tiles_x << 5, this->view_tiles_y << 5);
			this->sub_405399(0);
			SetClipRect(0, 0, this->view_tiles_x << 5, this->tile_scroll_y << 5);
			this->sub_405399(-this->view_tiles_y);
		}
	}
	SetClipRect(clip_rect);
	UnlockSurface3();
	g_mousept.Update();
	SetClipRect(this->field_0xf4);
	CopyScreenBlock(this->tile_scroll_x, this->tile_scroll_y, this->view_tiles_x << 5, this->view_tiles_y << 5);
	SetClipRect(clip_rect);
	g_mousept.Update();
	LockSurface2();
	GetClipRect(&clip_rect);
	SetClipRect(this->field_0xf4);
	if (this->field_0x108 != 0 && (g_DWORD_00659d18 & 2) == 0 && g_Lightning != 0) {
		this->sub_405D0E();
	}
	int32_t tick = this->field_0xa88;
	uint8_t* gfx_cells = this->field_0x80->GetObjectMap();
	uint8_t* f18map = this->field_0x80->GetOverlayMap();
	uint8_t* hmap = this->field_0x80->GetMapHeights();
	uint16_t* landscape = this->field_0x80->GetLandscape();
	this->field_0x134 = 0;

	if (g_Shadows != 0) {
		for (int32_t y = -4; y < this->view_tiles_y + 7; y++) {
			for (int32_t x = this->view_tiles_x + 3; x > -5; x--) {
				int32_t cell = x + this->view_x + this->map_width * (y + this->view_y);
				uint16_t lflags = (landscape[cell] & 0xC000) | (landscape[cell + 1] & 0xC000) |
					(landscape[cell + this->map_width] & 0xC000) | (landscape[cell + this->map_width + 1] & 0xC000);
				if (y > -4 && y < this->view_tiles_y + 7 && x > -4 && x < this->view_tiles_x + 3) {
					int32_t idx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
					CGameObject* obj = this->tile_structures[idx];
					uint32_t hgt = this->height_cells[x + 4 + (y + 4) * (this->view_tiles_x + 8)];
					if (obj != nullptr && obj->IsBlocked() < 2) {
						obj->DrawShadow(x, y, hgt);
					}
				}
			}
		}
	}

	for (int32_t y = -4; y < this->view_tiles_y + 7; y++) {
		for (int32_t x = this->view_tiles_x + 3; x > -5; x--) {
			int32_t cell = x + this->view_x + this->map_width * (y + this->view_y);
			uint16_t lflags = (landscape[cell] & 0xC000) | (landscape[cell + 1] & 0xC000) |
				(landscape[cell + this->map_width] & 0xC000) | (landscape[cell + this->map_width + 1] & 0xC000);
			if (y > -4 && y < this->view_tiles_y + 7 && x > -4 && x < this->view_tiles_x + 3) {
				int32_t idx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
				uint8_t light = this->tile_light[idx];
				CGameObject* obj9c = this->tile_special_units[idx];
				CGameObject* obj98 = this->tile_backpacks[idx];
				CGameObject* obj8c = this->tile_units[idx];
				CGameObject* obj94 = this->tile_structures[idx];
				uint32_t hgt = this->height_cells[x + 4 + (y + 4) * (this->view_tiles_x + 8)];
				if (obj94 != nullptr && obj94->IsBlocked() < 2 && g_StructuresInfo[obj94->typeId]->flat != 0) {
					obj94->Draw(x, y, light);
					StructureInfo* si = g_StructuresInfo[obj94->typeId];
					CRect sel = si->selection;
					sel.OffsetRect(obj94->mapBoundsLeft << 5,
						((obj94->mapBoundsTop + si->tile_height) - si->full_height) * 0x20 - hgt);
					if (obj94->IsBlocked() == 0) {
						this->sub_404DA3(&sel, obj94->unit_id);
					}
				}
			}
		}
	}

	for (int32_t y = -4; y < this->view_tiles_y + 7; y++) {
		for (int32_t x = this->view_tiles_x + 3; x > -5; x--) {
			int32_t cell = x + this->view_x + this->map_width * (y + this->view_y);
			uint16_t lflags = (landscape[cell] & 0xC000) | (landscape[cell + 1] & 0xC000) | (landscape[cell + this->map_width] & 0xC000) | (landscape[cell + this->map_width + 1] & 0xC000);
			if (y > -4 && y < this->view_tiles_y + 7 && x > -4 && x < this->view_tiles_x + 3) {
				int32_t idx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
				uint8_t light = this->tile_light[idx];
				CGameObject* obj9c = this->tile_special_units[idx];
				CGameObject* obj98 = this->tile_backpacks[idx];
				uint32_t hgt = this->height_cells[x + 4 + (y + 4) * (this->view_tiles_x + 8)];
				if (obj9c != nullptr && obj9c->IsBlocked() == 0) {
					if (g_Shadows != 0) {
						obj9c->DrawShadow(x, y, light);
					}
					obj9c->Draw(x, y, light);
				}
				if (obj98 != nullptr && obj98->IsBlocked() == 0) {
					if (g_Shadows != 0) {
						obj98->DrawShadow(x, y, light);
					}
					obj98->Draw(x, y, light);
				}
			}
		}
	}

	for (int32_t y = -4; y < this->view_tiles_y + 7; y++) {
		for (int32_t x = this->view_tiles_x + 3; x > -5; x--) {
			int32_t cell = x + this->view_x + this->map_width * (y + this->view_y);
			uint16_t lflags = (landscape[cell] & 0xC000) | (landscape[cell + 1] & 0xC000) |
				(landscape[cell + this->map_width] & 0xC000) | (landscape[cell + this->map_width + 1] & 0xC000);
			if (y > -4 && y < this->view_tiles_y + 7 && x > -4 && x < this->view_tiles_x + 3) {
				int32_t idx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
				uint8_t light = this->tile_light[idx];
				CUnit* unit = (CUnit*)this->tile_units[idx];
				CGameObject* bld = this->tile_structures[idx];
				uint32_t hgt = this->height_cells[x + 4 + (y + 4) * (this->view_tiles_x + 8)];
				if (bld != nullptr && bld->IsBlocked() < 2 && g_StructuresInfo[bld->typeId]->flat == 0) {
					bld->Draw(x, y, light);
					StructureInfo* si = g_StructuresInfo[bld->typeId];
					CRect sel = si->selection;
					sel.OffsetRect(bld->mapBoundsLeft << 5,
						((bld->mapBoundsTop + si->tile_height) - si->full_height) * 0x20 - hgt);
					if (bld->IsBlocked() == 0) {
						this->sub_404DA3(&sel, bld->unit_id);
					}
				}
				if (unit != nullptr && unit->IsBlocked() == 0 &&
					(unit->FindEffectTimer(0x20) < 0 || this->my_main_unit->IsVisibleTo(unit->map_player->index) != 0)) {
					UnitVFXUnfo* vfx = g_VFX_info[unit->typeId];
					CRect sel = vfx->selection;
					sel.OffsetRect(unit->centerScreenX - vfx->center_x,
						(unit->centerScreenY - vfx->center_y) - unit->terrainHeightOffset - unit->z_pos);
					if (unit->death_state < 2) {
						this->sub_404DA3(&sel, unit->unit_id);
					}
					if (g_Shadows != 0) {
						unit->DrawShadow(x, y, light);
					}
					unit->Draw(x, y, light);
				}
				if (g_Lightning != 0) {
					uint16_t lkey = (x + this->view_x) | ((y + this->view_y) << 8);
					uint32_t lval;
					if (this->field_0xa94.Lookup(lkey, lval)) {
						int32_t terrain = (uint16_t)(landscape[cell] & 0x1FFF) >> 6;
						if (lval < 0x12 && terrain > 7 && terrain < 0xC) {
							ProjectileInfo* proj = g_ProjectileInfos[8];
							proj->GetSprite()->Draw((x * 0x20 + 0x10) - proj->width / 2,
								((y * 0x20 + 0x10) - hgt) - proj->height / 2, lval >> 1, 0, 0);
						}
					}
				}
			}
			uint8_t gfxb = gfx_cells[cell];
			if (gfxb != 0 && x > -4 && y > -4 && x < this->view_tiles_x + 3 && y < this->view_tiles_y + 4) {
				int32_t gidx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
				uint8_t light2 = this->tile_light[gidx];
				int32_t tanval = (int32_t)(tan(this->field_0x80->GetRenderSunAngle()) * 65536.0);
				uint32_t hgt2 = this->height_cells[x + 4 + (y + 4) * (this->view_tiles_x + 8)];
				gfxb--;
				if (this->sub_40B173(gfxb, x, y, hgt2) == 0) {
					GfxObject* gobj = g_GfxObjects[gfxb];
					int32_t file = gobj->file;
					int32_t index = gobj->index;
					int32_t frame_count = gobj->frame_count;
					int32_t frame;
					if (frame_count == 0 || lflags != 0xC000) {
						if ((landscape[cell] & 0x2000) == 0 || gobj->dead_object == -1) {
							frame = index;
						} else {
							file = g_GfxObjects[gobj->dead_object]->file;
							index = 0;
							frame = 0;
						}
					} else if ((landscape[cell] & 0x2000) == 0 || gobj->dead_object == -1) {
						int32_t fidx = (tick + x + this->view_x + (y + this->view_y) * (x + this->view_x)) % frame_count;
						frame = index + gobj->frames[fidx];
					} else {
						file = g_GfxObjects[gobj->dead_object]->file;
						index = 0;
						frame = 0;
					}
					if (g_Animation == 0) {
						frame = 0;
					}
					CSprite256* spr = g_GfxFiles[file]->GetSprite();
					int32_t yoff = (int32_t)(tan(this->field_0x80->GetRenderSunAngle()) *
						(double)((spr->GetHeight(frame) / 2 + gobj->height / 2) - gobj->center_y));
					int32_t cx = (gobj->center_x - gobj->width / 2) + spr->GetWidth(0) / 2;
					int32_t cy = (gobj->center_y - gobj->height / 2) + spr->GetHeight(0) / 2;
					int32_t draw_x = ((x * 0x20 + 0x10) - cx) - yoff;
					int32_t draw_y = ((y * 0x20 + 0x10) - cy) - hgt2;
					CGamePalette* palette = &spr->palette;
					if (g_Shadows != 0) {
						spr->DrawShadowed(draw_x, draw_y, frame, g_DeltaCLR.shade_delta, tanval, 0);
					}
					CSprite256* spr_b = g_GfxFiles[file]->GetSpriteB();
					cx = (gobj->center_x - gobj->width / 2) + spr_b->GetWidth(0) / 2;
					cy = (gobj->center_y - gobj->height / 2) + spr_b->GetHeight(0) / 2;
					draw_x = ((x * 0x20 + 0x10) - cx) - yoff;
					draw_y = ((y * 0x20 + 0x10) - cy) - hgt2;
					if (g_settings.Smoothing != 0 && g_Shadows != 0) {
						spr_b->DrawShadowed(draw_x, draw_y, frame, g_DeltaCLR.lit_delta, tanval, 0);
					}
					cx = (gobj->center_x - gobj->width / 2) + spr->GetWidth(frame) / 2;
					cy = (gobj->center_y - gobj->height / 2) + spr->GetHeight(frame) / 2;
					draw_x = (x * 0x20 + 0x10) - cx;
					draw_y = ((y * 0x20 + 0x10) - cy) - hgt2;
					spr->DrawWithPalette(draw_x, draw_y, frame, light2, palette, 0);
					if (g_settings.Smoothing != 0) {
						cx = (gobj->center_x - gobj->width / 2) + spr_b->GetWidth(frame) / 2;
						cy = (gobj->center_y - gobj->height / 2) + spr_b->GetHeight(frame) / 2;
						draw_x = (x * 0x20 + 0x10) - cx;
						draw_y = ((y * 0x20 + 0x10) - cy) - hgt2;
						spr_b->DrawWithPaletteBlend(draw_x, draw_y, frame, light2, palette, 0);
					}
					uint16_t fkey = (x + this->view_x) | ((y + this->view_y) << 8);
					uint32_t fval;
					if (this->field_0xa94.Lookup(fkey, fval)) {
						ProjectileInfo* proj = g_ProjectileInfos[0xF];
						proj->GetSprite()->Draw((x * 0x20 + 0x10) - proj->width / 2,
							((y * 0x20 + 0x10) - hgt2) - proj->height / 2, (int32_t)fval / 2, 0, 0);
					}
				}
			}
			if (y > -4 && y < this->view_tiles_y + 7 && x > -4 && x < this->view_tiles_x + 3) {
				uint16_t tkey = (this->view_x + x) | ((this->view_y + y) << 8);
				uint32_t tval;
				if (this->field_0xa08.Lookup(tkey, tval) && tval != 0 && lflags == 0xC000) {
					int32_t tidx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
					this->sub_40AFC5(x * 0x20 + 0x10,
						(int32_t)(this->tile_top_y[tidx] + this->tile_bottom_y[tidx]) / 2, tval, 1);
				}
			}
		}
	}
	SetClipRect(clip_rect);
	UnlockSurface2();
	g_mousept.Update();
	LockSurface2();
	SetClipRect(this->field_0xf4);

	if (g_Shadows != 0) {
		for (int32_t y = -4; y < this->view_tiles_y + 7; y++) {
			for (int32_t x = this->view_tiles_x + 3; x > -5; x--) {
				int32_t cell = x + this->view_x + this->map_width * (y + this->view_y);
				uint16_t lflags = (landscape[cell] & 0xC000) | (landscape[cell + 1] & 0xC000) |
					(landscape[cell + this->map_width] & 0xC000) | (landscape[cell + this->map_width + 1] & 0xC000);
				if (y > -4 && y < this->view_tiles_y + 7 && x > -4 && x < this->view_tiles_x + 3) {
					int32_t idx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
					CGameObject* obj = this->tile_air_units[idx];
					uint32_t hgt = this->height_cells[x + 4 + (y + 4) * (this->view_tiles_x + 8)];
					if (obj != nullptr && obj->IsBlocked() == 0) {
						obj->DrawShadow(x, y, hgt);
					}
				}
			}
		}
	}

	POSITION it = this->field_0x9ec.GetStartPosition();
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9ec.GetNextAssoc(it, key, obj);
		obj->Draw(0, 0, 0);
	}

	for (int32_t y = -4; y < this->view_tiles_y + 7; y++) {
		for (int32_t x = this->view_tiles_x + 3; x > -5; x--) {
			int32_t cell = x + this->view_x + this->map_width * (y + this->view_y);
			uint16_t lflags = (landscape[cell] & 0xC000) | (landscape[cell + 1] & 0xC000) |
				(landscape[cell + this->map_width] & 0xC000) | (landscape[cell + this->map_width + 1] & 0xC000);
			if (y > -1 && y < this->view_tiles_y + 4 && x > -1 && x < this->view_tiles_x) {
				uint16_t tkey = (this->view_x + x) | ((this->view_y + y) << 8);
				uint32_t tval;
				if (this->field_0xa08.Lookup(tkey, tval) && tval != 0 && lflags == 0xC000) {
					int32_t tidx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
					this->sub_40AFC5(x * 0x20 + 0x10,
						(int32_t)(this->tile_top_y[tidx] + this->tile_bottom_y[tidx]) / 2, tval, 0);
				}
			}
		}
	}

	for (int32_t y = -4; y < this->view_tiles_y + 7; y++) {
		for (int32_t x = this->view_tiles_x + 3; x > -5; x--) {
			int32_t cell = x + this->view_x + this->map_width * (y + this->view_y);
			uint16_t lflags = (landscape[cell] & 0xC000) | (landscape[cell + 1] & 0xC000) |
				(landscape[cell + this->map_width] & 0xC000) | (landscape[cell + this->map_width + 1] & 0xC000);
			if (y > -4 && y < this->view_tiles_y + 7 && x > -4 && x < this->view_tiles_x + 3) {
				int32_t idx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
				uint8_t light = this->tile_light[idx];
				CGameObject* obj = this->tile_air_units[idx];
				uint32_t hgt = this->height_cells[x + 4 + (y + 4) * (this->view_tiles_x + 8)];
				if (obj != nullptr && obj->IsBlocked() == 0) {
					UnitVFXUnfo* vfx = g_VFX_info[obj->typeId];
					CRect sel = vfx->selection;
					sel.OffsetRect(obj->centerScreenX - vfx->center_x,
						(obj->centerScreenY - vfx->center_y) - obj->terrainHeightOffset - obj->z_pos);
					if (((CUnit*)obj)->death_state < 2) {
						this->sub_404DA3(&sel, obj->unit_id);
					}
					obj->Draw(x, y, light);
				}
			}
		}
	}

	for (int32_t y = -3; y < this->view_tiles_y + 7; y++) {
		for (int32_t x = this->view_tiles_x + 2; x > -4; x--) {
			int32_t idx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
			CGameObject* obj8c = this->tile_units[idx];
			CGameObject* obj90 = this->tile_air_units[idx];
			CGameObject* obj94 = this->tile_structures[idx];
			uint32_t hgt = this->height_cells[x + 4 + (y + 4) * (this->view_tiles_x + 8)];
			if (obj94 != nullptr && obj94->IsSelected()) {
				obj94->DrawHealthBar(x, y, hgt);
			}
			if (obj8c != nullptr && (obj8c->IsSelected() || (this->show_hp != 0 && obj8c->IsBlocked() == 0))) {
				obj8c->DrawHealthBar(x, y, hgt);
			}
			if (obj90 != nullptr && (obj90->IsSelected() || (this->show_hp != 0 && obj90->IsBlocked() == 0))) {
				obj90->DrawHealthBar(x, y, hgt);
			}
		}
	}
	SetClipRect(clip_rect);
	UnlockSurface2();
	g_mousept.Update();
	LockSurface2();
	SetClipRect(this->field_0xf4);

	for (int32_t x = 0; x < this->view_tiles_x; x++) {
		int32_t y = 0;
		while (y < this->view_tiles_y + 4) {
			int32_t stride = this->view_tiles_x + 7;
			int32_t idx = x + 3 + (y + 3) * stride;
			uint32_t t0 = this->field_0xa0[idx];
			uint32_t t1 = this->field_0xa0[idx + 1];
			uint32_t t2 = this->field_0xa0[idx + stride];
			uint32_t t3 = this->field_0xa0[idx + 1 + stride];
			uint32_t t4 = this->field_0xa0[idx + stride * 2];
			uint32_t t5 = this->field_0xa0[idx + 1 + stride * 2];
			uint32_t h0 = this->tile_corner_y[idx];
			uint32_t h1 = this->tile_corner_y[idx + 1];
			uint32_t h2 = this->tile_corner_y[idx + stride];
			uint32_t h3 = this->tile_corner_y[idx + 1 + stride];
			for (; t0 == t2 && t1 == t3 && t2 == t4 && t3 == t5 && y != this->view_tiles_y + 3; y++) {
				idx += stride;
				h2 = this->tile_corner_y[idx + stride];
				h3 = this->tile_corner_y[idx + 1 + stride];
				t4 = this->field_0xa0[idx + stride * 2];
				t5 = this->field_0xa0[idx + 1 + stride * 2];
			}
			if (t0 == t1 && t1 == t2 && t2 == t3) {
				if (t0 == 0) {
					y++;
				} else if (t0 == 0x10) {
					if (h0 == h1 && h2 == h3 && h0 + 0x20 == h2) {
						FillFlatTerrain(x << 5, h0, h2);
					} else {
						FillDeformedTerrain(x << 5, (x + 1) * 0x20, h0, h1, h2, h3);
					}
					y++;
				} else {
					if (t0 != 8) {
						if (h0 == h1 && h2 == h3 && h0 + 0x20 == h2) {
							FillFlatLight(x << 5, h0, t0, t1, t2, t3);
						} else {
							FillDeformedLight(x << 5, (x + 1) * 0x20, h0, h1, h2, h3, t0, t1, t2, t3);
						}
						y++;
						continue;
					}
					if (h0 == h1 && h2 == h3 && h0 + 0x20 == h2) {
						FillFlatGrey(x << 5, h0, h2);
					} else {
						FillDeformedGrey(x << 5, (x + 1) * 0x20, h0, h1, h2, h3);
					}
					y++;
				}
			} else {
				if (h0 == h1 && h2 == h3 && h0 + 0x20 == h2) {
					FillFlatLight(x << 5, h0, t0, t1, t2, t3);
				} else {
					FillDeformedLight(x << 5, (x + 1) * 0x20, h0, h1, h2, h3, t0, t1, t2, t3);
				}
				y++;
			}
		}
	}
	SetClipRect(clip_rect);
	UnlockSurface2();
	g_mousept.Update();
	LockSurface2();
	SetClipRect(this->field_0xf4);

	if (child2 == nullptr || child3 == nullptr) {
		if (child3 == nullptr) {
			if (child2 != nullptr && g_ScreenSize.right == 800) {
				FillRectColorSimple(local_rect.right - 0x20, child2->GetRect().top + 2, local_rect.right, local_rect.bottom, 0);
			}
		} else {
			FillRectColorSimple(local_rect.right - 0x20, child3->GetRect().top + 2, local_rect.right, local_rect.bottom, 0);
		}
	} else {
		FillRectColorSimple(local_rect.right - 0x20, child3->GetRect().top + 2, local_rect.right, child2->GetRect().bottom, 0);
		FillRectColorSimple(local_rect.left, child3->GetRect().bottom, local_rect.right, child3->GetRect().bottom + 4, 0);
	}
	g_bmp_crystall->DrawSubRectMasked(local_rect.right - 0x10, local_rect.top, 0, 0, 0x10, 0x9e);
	uint32_t top_panel_kind = (uint32_t)(this->field_0x140 == 0) | (this->field_0x144 & 0x24);
	if (top_panel_kind == 0) {
		g_bmp_cmdbarl->DrawSubRectMasked(local_rect.right - 0x10, local_rect.top + 0x9e, 0, 0, 0x10, 0x50);
	} else {
		g_bmp_headsl->DrawSubRectMasked(local_rect.right - 0x10, local_rect.top + 0x9e, 0, 0, 0x10, 0x50);
	}
	int32_t info_mode = wnd->vis_charinfo->info_mode;
	if (info_mode == 0) {
		g_bmp_textbackl->DrawSubRectMasked(local_rect.right - 0x10, local_rect.top + 0xee, 0, 0, 0x10, 0xf2);
	} else {
		g_bmp_humanbackl->DrawSubRectMasked(local_rect.right - 0x10, local_rect.top + 0xee, 0, 0, 0x10, 0xf2);
	}
	int32_t sidestatus_h = wnd->vis_sidestatus->GetRect().Height();
	int32_t charinfo_h = wnd->vis_charinfo->GetRect().Height();
	uint32_t panel_cmp = charinfo_h <= sidestatus_h;
	if (g_ScreenSize.bottom > 600) {
		g_bmp_extra1024l->DrawSubRectMasked(local_rect.right - 0x10, local_rect.top + 480, 0, 0, 0x10, g_bmp_extra1024l->GetHeight(0));
		g_bmp_textbackl->DrawSubRectMasked(local_rect.right - 0x10, local_rect.top + 480 + g_bmp_extra1024l->GetHeight(0), 0, 0, 0x10, 0xf2);
	} else if (g_ScreenSize.bottom > 480) {
		g_bmp_extra800l->DrawSubRectMasked(local_rect.right - 0x10, local_rect.top + 480, 0, 0, 0x10, g_bmp_extra800l->GetHeight(0));
	}
	SetClipRect(local_rect);
	TakeDamage::DrawDamages(&this->damage_labels);
	this->msglog.Draw();
	this->field_0xc4++;
	uint32_t now = timeGetTime();
	if (this->field_0xcc == 0) {
		this->field_0xcc = now;
	}
	this->field_0xc8 += now - this->field_0xcc;
	this->field_0xcc = now;
	if ((uint32_t)this->field_0xc8 > 1000) {
		this->field_0xd0 = ((double)(uint32_t)this->field_0xc4 * 1000.0) / (double)(uint32_t)this->field_0xc8;
		this->field_0xc8 -= 1000;
		this->field_0xc4 = 0;
	}
	if (DAT_00660f6c != 0) {
		FillRectColorSimple(local_rect.right - 0x78, local_rect.top, local_rect.right - 0x1e, local_rect.top + 0x18,
			GetColorRGB(8, 8, 8));
		char buf[32];
		sprintf(buf, "%3.1f fps", this->field_0xd0);
		g_font1->DrawTextWithShadow(local_rect.right - 0x26, local_rect.top, buf, 1, clrsh_ShockingBlack, 1);
	}
	if (wnd->sessionMode == 0 && DAT_00660f70 != 0) {
		NetStru2* client = g_NetStru1_local.GetClientByPlayerID(0);
		if (client != nullptr) {
			uint32_t uid = client->GetUid();
			ConnStatInfo* stat;
			if (g_NetStru1_local.client_stat.Lookup(uid, stat)) {
				FillRectColorSimple(local_rect.right - 0xb4, local_rect.top + 0x1e, local_rect.right - 0x1e, local_rect.top + 0x69,
					GetColorRGB(8, 8, 8));
				g_font1->DrawTextWithShadow(local_rect.right - 0x69, local_rect.top + 0x23, txt_patch.GetLine(74), 2, clrsh_ShockingBlack, 1);
				CString str;
				int32_t speed = 0;
				if (stat->time != 0) {
					speed = (int32_t)stat->total_bytes / (int32_t)stat->time;
				}
				str.Format("%s %d %s", txt_patch.GetLine(75), stat->cur_bs, txt_patch.GetLine(61));
				g_font1->DrawTextWithShadow(local_rect.right - 0xaf, local_rect.top + 0x34, (const char*)str, 0, clrsh_ShockingBlack, 1);
				str.Format("%s %d %s", txt_patch.GetLine(76), speed, txt_patch.GetLine(61));
				g_font1->DrawTextWithShadow(local_rect.right - 0xaf, local_rect.top + 0x45, (const char*)str, 0, clrsh_ShockingBlack, 1);
				str.Format("%s %d %s", txt_patch.GetLine(77), stat->max_bs, txt_patch.GetLine(61));
				g_font1->DrawTextWithShadow(local_rect.right - 0xaf, local_rect.top + 0x56, (const char*)str, 0, clrsh_ShockingBlack, 1);
			}
		}
	}
	SetClipRect(clip_rect);
	CVisualObject::VMethod7();
	UnlockSurface2();
	this->sub_40B314();
	if (childs_size == this->field_0x104) {
		this->field_0xf4.SubtractRect(&this->field_0xf4, &childs_rect);
		if (!childs_rect.IsRectEmpty()) {
			gfxFlushRect(childs_rect);
		}
	}
	this->field_0x104 = childs_size;
	gfxFlushRect(this->field_0xf4);
	CRect fps_area;
	CRect fps_flush(0x168, 0, 0x1c2, 0x18);
	CRect net_flush(300, 0x1e, 0x1c2, 0x69);
	if (DAT_00660f6c != 0) {
		gfxFlushRect(fps_flush);
	}
	if (DAT_00660f70 != 0 && wnd->sessionMode == 0) {
		gfxFlushRect(net_flush);
	}
	this->force_full_redraw = 0;
	this->scroll_dx = 0;
	this->scroll_dy = 0;
	this->prev_view_x = this->view_x;
	this->prev_view_y = this->view_y;
}

// 406F1A
void BigStruct2::VMethod7() {
	this->force_full_redraw = 1;
	this->sub_406F7B();
}

// 40C60F
int32_t BigStruct2::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
	return this->OnLButtonDown(wparam, pos);
}

// 402AF6
BigStruct2::BigStruct2(int32_t l, int32_t t, int32_t r, int32_t b) : CVisualObject(1, l, t, r, b, nullptr) {
	this->field_0x80 = nullptr;
	this->sub_402FB8();
	this->view_x = this->MapMinX();
	this->view_y = this->MapMinY();
	this->field_0xc4 = 0;
	this->field_0xc8 = 0;
	this->field_0xcc = 0;
	this->field_0xd0 = 0.0;
	this->field_0x10c.SetSize(0x40, 0x20);
	this->field_0x120.SetSize(0x40, 0x20);
	this->field_0x134 = 0;
	this->field_0x140 = 0;
	this->field_0x9b4 = 0;
	this->field_0xa88 = 0;
	this->field_0xa8c = 0;
	this->field_0xa90 = 0;
	this->field_0xd8 = 0;
	this->field_0xdc = 1;
	this->field_0xe0 = 1;
	this->field_0xa24 = 0;
	this->field_0x49b8 = 40000;
	this->field_0x49bc = 40000;
	this->field_0x49c0 = 0;
	this->tile_scroll_x = 0;
	this->tile_scroll_y = 0;
	this->prev_view_x = -1;
	this->prev_view_y = -1;
	this->field_0x104 = 1;
	this->field_0x148 = 0;
	this->field_0x150 = 0;
	this->field_0x14c = 0;
	this->wimpy = 0;
	this->formation = 1;
	this->show_hp = 0;
	this->flying_hp = 1;
	this->field_0x3f68 = new CBmp256(0x20, 0x20);
	this->field_0x80 = nullptr;
	this->field_0x3f50 = 7;
	this->field_0x3f4c = 1 << this->field_0x3f50;
	this->sub_403730();
	MapPlayerData* mpd = new MapPlayerData(0, 0xF);
	this->field_0x9b8.Add(mpd);
	this->field_0x9b8[0]->flags |= 1;
	this->field_0x3f6c = nullptr;
	this->my_main_unit = nullptr;
	this->field_0x994 = nullptr;
	this->field_0x9a8 = 0;
	this->quest_some_id_2 = 0;
	this->quest_landmark_some_id = 0;
	this->quest_building_some_id = 0;
	this->quest_some_id = 0;
	this->field_0x4970 = new QuestMap();
	for (int32_t i = 0; i < 12; i++) {
		this->field_0x4988[i] = nullptr;
	}
}

// 40AFC5
void BigStruct2::sub_40AFC5(int32_t x, int32_t y, uint32_t flags, int32_t flag) {
	if ((flags & 8) != 0 && flag != 0) {
		ProjectileInfo* proj = g_ProjectileInfos[0xF];
		int32_t phase = abs(this->field_0xa88 / 2 + x * y) % 5 + 3;
		proj->GetSprite()->Draw(x - proj->width / 2, y - proj->height / 2, phase, 0, 0);
	}
	if ((flags & 0x40) != 0 && flag == 0) {
		ProjectileInfo* proj = g_ProjectileInfos[0x15];
		int32_t phase = abs(this->field_0xa88 / 2 + x * y) % proj->phases;
		proj->GetSprite()->Draw(x - proj->width / 2, y - proj->height / 2, phase, 0, 0);
	} else if ((flags & 0x20000) != 0 && flag != 0) {
		ProjectileInfo* proj = g_ProjectileInfos[0x2B];
		int32_t phase = abs(this->field_0xa88 / 2 + x * y) % proj->phases;
		proj->GetSprite()->Draw(x - proj->width / 2, y - proj->height / 2, phase, 0, 0);
	}
}

// 40B173
int32_t BigStruct2::sub_40B173(int32_t gfx_id, int32_t x, int32_t y, int32_t height) {
	GfxObject* gobj = g_GfxObjects[gfx_id];
	int32_t min_tx = x - 1;
	int32_t max_tx = x + 2;
	if (min_tx < 0) {
		min_tx = 0;
	}
	if (max_tx > this->view_tiles_x) {
		max_tx = this->view_tiles_x;
	}
	int32_t cy = (gobj->center_y - gobj->height / 2) + g_GfxFiles[gobj->file]->GetSprite()->GetHeight(0) / 2;
	int32_t min_ty = this->sub_4184B8(x * 0x20 + 0x10, ((y * 0x20 + 0x10) - cy) - height);
	int32_t max_ty = this->sub_4184B8(x * 0x20 + 0x10,
		(((y * 0x20 + 0x10) - cy) - height) + g_GfxFiles[gobj->file]->GetSprite()->GetHeight(0)) + 1;
	if (min_ty < 0) {
		min_ty = 0;
	}
	if (max_ty > this->view_tiles_y + 4) {
		max_ty = this->view_tiles_y + 4;
	}
	for (int32_t tx = min_tx; tx <= max_tx; tx++) {
		for (int32_t ty = min_ty; ty <= max_ty; ty++) {
			if (this->field_0xa0[tx + 3 + (ty + 3) * (this->view_tiles_x + 7)] != 0x10) {
				return 0;
			}
		}
	}
	return 1;
}

// 402FB8
void BigStruct2::sub_402FB8() {
	this->view_tiles_x = this->rect.Width() / 32;
	this->view_tiles_y = this->rect.Height() / 32;
	if (this->field_0x80 != nullptr) {
		if (this->view_x + this->view_tiles_x >= this->field_0x80->GetWidth() - 8) {
			this->view_x = (this->field_0x80->GetWidth() - 8) - this->view_tiles_x;
		}
		if (this->view_y + this->view_tiles_y >= this->field_0x80->GetHeight() - 8) {
			this->view_y = (this->field_0x80->GetHeight() - 8) - this->view_tiles_y;
		}
	}
	this->rect.bottom = this->view_tiles_y * 0x20 + this->rect.top;
	this->tile_grid_bytes = (this->view_tiles_x + 6) * (this->view_tiles_y + 0xA) * 4;
	this->corner_grid_bytes = (this->view_tiles_x + 7) * (this->view_tiles_y + 0xB) * 4;
	this->tile_units = (CGameObject**)malloc(this->tile_grid_bytes);
	this->tile_air_units = (CGameObject**)malloc(this->tile_grid_bytes);
	this->tile_structures = (CGameObject**)malloc(this->tile_grid_bytes);
	this->tile_backpacks = (CGameObject**)malloc(this->tile_grid_bytes);
	this->tile_special_units = (CGameObject**)malloc(this->tile_grid_bytes);
	this->field_0xa0 = (uint32_t*)malloc(this->corner_grid_bytes);
	this->field_0xa4 = (uint32_t*)malloc(this->corner_grid_bytes);
	this->field_0xa8 = (uint8_t*)malloc(this->corner_grid_bytes / 4);
	this->field_0xac = (uint8_t*)malloc(this->corner_grid_bytes / 4);
	memset(this->field_0xac, 0, this->corner_grid_bytes / 4);
	this->tile_light = (uint8_t*)malloc(this->tile_grid_bytes / 4);
	this->tile_corner_y = (uint32_t*)malloc(this->corner_grid_bytes);
	this->tile_top_y = (uint32_t*)malloc(this->tile_grid_bytes);
	this->tile_bottom_y = (uint32_t*)malloc(this->tile_grid_bytes);
	this->height_cells = (uint32_t*)malloc((this->view_tiles_x + 8) * (this->view_tiles_y + 0xC) * 4);
	this->force_full_redraw = 1;
	this->scroll_dx = 0;
	this->scroll_dy = 0;
	this->msglog.SetRect(this->rect);
}

// 403730
void BigStruct2::sub_403730() {
	for (int32_t i = 0; i <= 0x14; i++) {
		for (int32_t j = 0; j <= 0x14; j++) {
			if (j < i / 2) {
				this->field_0xac0[i + 0x14][j + 0x14][0] = 0xFF;
				this->field_0xac0[i + 0x14][j + 0x14][1] = 0;
				this->field_0xac0[0x14 - i][0x14 - j][0] = 1;
				this->field_0xac0[0x14 - i][0x14 - j][1] = 0;
				this->field_0xac0[i + 0x14][0x14 - j][0] = 0xFF;
				this->field_0xac0[i + 0x14][0x14 - j][1] = 0;
				this->field_0xac0[0x14 - i][j + 0x14][0] = 1;
				this->field_0xac0[0x14 - i][j + 0x14][1] = 0;
			} else if (j > 2 * i) {
				this->field_0xac0[i + 0x14][j + 0x14][0] = 0;
				this->field_0xac0[i + 0x14][j + 0x14][1] = 0xFF;
				this->field_0xac0[0x14 - i][0x14 - j][0] = 0;
				this->field_0xac0[0x14 - i][0x14 - j][1] = 1;
				this->field_0xac0[i + 0x14][0x14 - j][0] = 0;
				this->field_0xac0[i + 0x14][0x14 - j][1] = 1;
				this->field_0xac0[0x14 - i][j + 0x14][0] = 0;
				this->field_0xac0[0x14 - i][j + 0x14][1] = 0xFF;
			} else {
				this->field_0xac0[i + 0x14][j + 0x14][0] = 0xFF;
				this->field_0xac0[i + 0x14][j + 0x14][1] = 0xFF;
				this->field_0xac0[0x14 - i][0x14 - j][0] = 1;
				this->field_0xac0[0x14 - i][0x14 - j][1] = 1;
				this->field_0xac0[i + 0x14][0x14 - j][0] = 0xFF;
				this->field_0xac0[i + 0x14][0x14 - j][1] = 1;
				this->field_0xac0[0x14 - i][j + 0x14][0] = 1;
				this->field_0xac0[0x14 - i][j + 0x14][1] = 0xFF;
			}
			int16_t val = (int16_t)(std::sqrt(i * i + j * j) * this->field_0x3f4c / (std::max)(i, j));
			this->field_3228[i + 0x14][j + 0x14] = val;
			this->field_3228[0x14 - i][0x14 - j] = val;
			this->field_3228[i + 0x14][0x14 - j] = val;
			this->field_3228[0x14 - i][j + 0x14] = val;
		}
	}
	this->field_0xac0[0x15][0x14][0] = 0xFF;
	this->field_0xac0[0x15][0x14][1] = 0;
	this->field_0xac0[0x13][0x14][0] = 1;
	this->field_0xac0[0x13][0x14][1] = 0;
}

// 403CA0
void BigStruct2::FUN_00403ca0(CGameObject* obj) {
	int32_t base_x = obj->tileX - 20;
	int32_t base_y = obj->tileY - 20;
	int32_t base_idx = base_y * this->map_width + base_x;
	memset(this->field_0x17e4, 0, 0x1A44);
	this->field_0x17e4[20][20] = (obj->scan_range >> (8 - this->field_0x3f50)) + (1 << (this->field_0x3f50 - 1));
	int8_t base_height = (int8_t)this->field_0x80->GetMapHeights()[obj->tileY * this->map_width + obj->tileX];
	for (int32_t r = 1; r < 0x14; r++) {
		int32_t ring_ok = 1;
		for (int32_t d = -r; d < r + 1; d++) {
			int32_t cx = d + 20;
			int32_t cy = 20 - r;
			if (this->field_0x80->IsInShadedBounds(base_x + cx, base_y + cy) != 0 && this->sub_403B8F(cx, cy, base_idx, base_height) == 0) {
				ring_ok = 0;
			}
			cx = d + 20;
			cy = r + 20;
			if (this->field_0x80->IsInShadedBounds(base_x + cx, base_y + cy) != 0 && this->sub_403B8F(cx, cy, base_idx, base_height) == 0) {
				ring_ok = 0;
			}
			cx = 20 - r;
			cy = d + 20;
			if (this->field_0x80->IsInShadedBounds(base_x + cx, base_y + cy) != 0 && this->sub_403B8F(cx, cy, base_idx, base_height) == 0) {
				ring_ok = 0;
			}
			cx = r + 20;
			cy = 20 - d;
			if (this->field_0x80->IsInShadedBounds(base_x + cx, base_y + cy) != 0 && this->sub_403B8F(cx, cy, base_idx, base_height) == 0) {
				ring_ok = 0;
			}
		}
		if (ring_ok != 0) {
			break;
		}
	}
}

// 41C74B
void BigStruct2::FUN_0041c74b(int32_t x, int32_t y, int32_t w, int32_t h, int32_t unk1, uint8_t* dat, int32_t unk2) {
	uint8_t* overlay = this->field_0x80->GetOverlayMap();
	for (int32_t i = 0; i < w; i++) {
		for (int32_t j = 0; j < h; j++) {
			int32_t bit_index = j * w + i;
			if ((dat[bit_index >> 3] & (1 << (bit_index & 7))) == 0) {
				continue;
			}
			uint16_t packed = (x + i) | ((y + j) << 8);
			uint32_t value;
			if (this->field_0xa08.Lookup(packed, value) == 0) {
				value = 0;
			}
			if (unk2 != 0) {
				value |= 1 << unk1;
				this->field_0xa08.SetAt(packed, value);
				int32_t idx = (x + i) + (y + j) * this->map_width;
				if (unk1 == 0xF) {
					overlay[idx] = 0;
					overlay[idx + 1] = 0;
					overlay[idx + this->map_width] = 0;
					overlay[idx + this->map_width + 1] = 0;
				} else if (unk1 == 0xE) {
					overlay[idx] = 0x50;
					overlay[idx + 1] = 0x50;
					overlay[idx + this->map_width] = 0x50;
					overlay[idx + this->map_width + 1] = 0x50;
				} else if (unk1 == 3) {
					uint32_t zero_val = 0;
					uint16_t* land = this->field_0x80->GetLandscape();
					if ((land[idx] & 0x2000) == 0) {
						uint8_t gfx_id = this->field_0x80->GetObjectMap()[idx];
						if (gfx_id != 0 && g_GfxObjects[gfx_id - 1]->in_map_editor != -1) {
							this->field_0xa94.SetAt(packed, zero_val);
						}
					}
					this->field_0x49b8++;
					land[idx] |= 0x2000;
				}
			} else {
				value &= ~(1 << unk1);
				if (value != 0) {
					this->field_0xa08.SetAt(packed, value);
				} else {
					this->field_0xa08.RemoveKey(packed);
				}
				if (unk1 == 3) {
					this->field_0x49b8++;
				} else if (unk1 == 0xE || unk1 == 0xF) {
					this->field_0x80->UpdateTerrainShading(x + i, y + j, 2, 2);
				}
			}
			this->force_full_redraw = 1;
		}
	}
}

// 41CB21
void BigStruct2::FUN_0041cb21() {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x39;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 40C56C
int32_t BigStruct2::OnRButtonDown(uint32_t wparam, CPoint pos) {
	if (this->TestFlags(FLAG_OVERCURSOR) == 0) {
		this->SetCursorOver(true);
	}
	return 1;
}

// 40D4B2
int32_t BigStruct2::OnChar(uint32_t wparam) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if ((wnd->dialogsMask & 8) != 0) {
		return 0;
	}
	return 1;
}

// 41CB67
void BigStruct2::FUN_0041cb67(int32_t val) {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = 0x3b;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = val;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 40C510
int32_t BigStruct2::OnLButtonDown(uint32_t wparam, CPoint pos) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->dialogsMask != 1) {
		return 0;
	}
	if (g_mousept.GetSelectState() == 0 && this->field_0x80 != nullptr) {
		wnd->UpdateMapRect();
		g_mousept.StartSelect(pos);
	}
	return 1;
}

// 40C59C
int32_t BigStruct2::OnRButtonUp(uint32_t wparam, CPoint pos) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (this->field_0xd8 == 0 && wnd->dialogsMask == 1) {
		this->MsgProc(0x405, 0, 0);
	}
	this->field_0xd8 = 0;
	if (this->TestFlags(FLAG_OVERCURSOR) != 0) {
		this->SetCursorOver(false);
	}
	return 1;
}

// 41da52
CGameBitmap* BigStruct2::FUN_0041da52(int32_t serv_id) {
	// 630e28: portrait top-left inside the 0xa0*0xf0 infowindow picture, indexed by typeId - 0x40
	static const POINT projectile_portrait_pos[39] = {
		{ 0x1e, 0x48 }, { 0x1a, 0x17 }, { 0x0a, 0x2c }, { -1, -1 },     { 0x0f, 0x15 },
		{ 0x20, 0x15 }, { -1, -1 },     { 0x07, 0x31 }, { -1, -1 },     { -1, -1 },
		{ 0x08, 0x48 }, { -1, -1 },     { -1, -1 },     { -1, -1 },     { -1, -1 },
		{ 0x1e, 0x48 }, { 0x17, 0x1f }, { 0x2b, 0x07 }, { 0x25, 0x1c }, { 0x25, 0x1c },
		{ 0x25, 0x1c }, { 0x25, 0x1c }, { 0x25, 0x1c }, { 0x25, 0x1c }, { 0x25, 0x1c },
		{ 0x25, 0x18 }, { 0x25, 0x18 }, { 0x25, 0x18 }, { 0x25, 0x18 }, { 0x25, 0x0f },
		{ 0x25, 0x0f }, { 0x25, 0x0f }, { 0x25, 0x0f }, { 0x23, 0x0f }, { 0x23, 0x0f },
		{ 0x23, 0x0e }, { 0x23, 0x15 }, { 0x20, 0x12 }, { 0x0f, 0x15 },
	};

	CBmp64* result = nullptr;
	CUnit* unit = this->FUN_0041dfa6(serv_id);
	if (unit == nullptr) {
		result = new CBmp64(0x58, 0x6c);
	} else {
		result = new CBmp64(0x58, 0x6c);
		CBmp64* pic = new CBmp64(0xa0, 0xf0);
		if ((unit->unitFlags & 0x11) == 0) {
			char buf[257];
			UnitVFXUnfo* info = g_VFX_info.GetAt(unit->typeId);
			sprintf(buf, "graphics\\infowindow\\%s%d", info->info_picture, unit->face);
			if (unit->face == 1) {
				buf[strlen(buf) - 1] = '\0';
			}
			strcat(buf, ".bmp");
			pic->LoadFile(buf);
			pic->FlipVertical();
		} else {
			unit->RenderHeroPortrait(nullptr, pic, nullptr);
		}
		result->SelectBitmapForDraw();
		memset(g_selDrawBitmap.lpSurface, 0, 0x4a40);
		CRect rect(-1, -1, -1, -1);
		if (unit->typeId >= 0x40 && unit->typeId <= 0x66) {
			rect.TopLeft() = projectile_portrait_pos[unit->typeId - 0x40];
		}
		g_bmp_t_back->FlipVertical();
		if (rect.top == -1) {
			g_bmp_t_back->DrawSubRect(8, 7, 0x24, 0x8c, 0x6c, 0xe8);
			pic->DrawSubRectMasked(8, 7, 0x24, 0x8c, 0x6c, 0xe8);
		} else {
			g_bmp_t_back->DrawSubRect(8, 7, rect.left, 0x90 - rect.top, rect.left + 0x48, 0xf0 - rect.top);
			pic->DrawSubRectMasked(8, 7, rect.left, 0x90 - rect.top, rect.left + 0x48, 0xf0 - rect.top);
		}
		g_bmp_t_back->FlipVertical();
		g_spr_t_border->Draw(0, 0, 0, 0, 0);
		delete pic;
		if (unit->unitFlags & 0x40) {
			delete unit;
		}
	}
	return result;
}

// 41DF23
CUnit* BigStruct2::FUN_0041df23(int32_t serv_id) {
	CUnit* result = nullptr;
	POSITION it = this->field_0x9d0.GetStartPosition();
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsKindOf(RUNTIME_CLASS(CUnit))) {
			CUnit* unit = (CUnit*)obj;
			if (unit->serverId == serv_id) {
				result = unit;
				break;
			}
		}
	}
	return result;
}

// 41DFDB
CUnit* BigStruct2::sub_41DFDB() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->sessionMode != 2) {
		return nullptr;
	}
	if (g_settings.Acknowledgement == 0) {
		return nullptr;
	}
	CArray<CUnit*> primary;
	CArray<CUnit*> secondary;
	CArray<CUnit*> tertiary;
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (!obj->IsSelected() || obj->hp <= 0) {
			continue;
		}
		CUnit* unit = (CUnit*)obj;
		if ((unit->unitFlags & 1) != 0) {
			primary.Add(unit);
		} else if (primary.GetSize() == 0 && (unit->unitFlags & 0x10) != 0) {
			if (unit->equipmentTokens[0] != nullptr) {
				secondary.Add(unit);
			} else if (secondary.GetSize() == 0) {
				tertiary.Add(unit);
			}
		} else if (unit->serverId != 0 && unit->serverId < 0x15) {
			secondary.Add(unit);
		}
	}
	if (primary.GetSize() != 0) {
		return primary[(primary.GetSize() * rand()) / 0x7FFF];
	}
	if (secondary.GetSize() != 0) {
		return secondary[(secondary.GetSize() * rand()) / 0x7FFF];
	}
	if (tertiary.GetSize() != 0) {
		return tertiary[(tertiary.GetSize() * rand()) / 0x7FFF];
	}
	return nullptr;
}

// 41D97E
void BigStruct2::FUN_0041d97e(int32_t arg) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if ((wnd->dialogsMask & 1) == 0) {
		return;
	}
	int32_t time_of_day = ((uint32_t)wnd->serverLoopCounter >> 4) + 0x168;
	if (((wnd->serverLoopCounter & 0xF) == 0 && time_of_day % 20 == 0 && g_settings.ShowTimeFlow != 0) || arg != 0) {
		UpdateDayNightLighting(time_of_day);
		this->field_0x80->UpdateTerrainShading(1, 1, 0, 0);
		this->sub_404912();
		ReloadPalettes();
		this->field_0xdc = 1;
		this->force_full_redraw = 1;
		wnd->vis_minimap->MsgProc(0x404, 0, 0);
	}
}

// 40C83F
int32_t BigStruct2::OnMouseMove(uint32_t wparam, CPoint pos) {
	if ((wparam & 2) != 0) {
		int32_t dx = pos.x - g_mousept.GetX();
		int32_t dy = pos.y - g_mousept.GetY();
		int32_t cell_dx = dx / 8;
		int32_t cell_dy = dy / 8;
		if (dx != 0 || dy != 0) {
			if (cell_dx != 0 || cell_dy != 0) {
				this->MsgProc(0x406, this->view_x + cell_dx, this->view_y + cell_dy);
				this->field_0xd8 = 1;
			}
			return 1;
		}
	}
	return CVisualObject::OnMouseMove(wparam, pos);
}

// 41B7B7
void BigStruct2::FUN_0041b7b7(int32_t xpos, int32_t ypos, int32_t* pvol, int32_t* ppan) {
	int32_t center_x = (this->view_x << 8) + (this->view_tiles_x << 7);
	int32_t center_y = (this->view_y << 8) + (this->view_tiles_y << 7);
	double dx = xpos - center_x;
	*ppan = (int32_t)(dx * 4000.0 / (this->view_tiles_x << 8));
	if (*ppan < -10000) {
		*ppan = -10000;
	}
	if (*ppan > 10000) {
		*ppan = 10000;
	}
	double dy = ypos - center_y;
	double dist = std::sqrt(dx * dx + dy * dy) / 256.0;
	double vol = (1.0 - std::exp(dist / 8.0)) * 100.0;
	if (vol < -10000.0) {
		vol = -10000.0;
	}
	*pvol = (int32_t)vol;
}

// 40C902
int32_t BigStruct2::OnKeyDown(uint32_t wparam) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->dialogsMask == 1 && this->field_0x80 != nullptr) {
		if (wnd->console_active != 0) {
			wnd->vis_1200->OnKeyDown(wparam);
			return 0;
		}
		if (wparam >= 0x30 && wparam <= 0x39) {
			if (g_kbControlState == 0) {
				if (g_kbMenuState == 0) {
					this->sub_416983(wparam - 0x30);
				} else {
					this->sub_416B4A(wparam - 0x30);
				}
			} else if (g_kbShiftState == 0) {
				this->sub_416A02(wparam - 0x30);
			} else {
				this->sub_416AA6(wparam - 0x30);
			}
			return 1;
		}
		switch (wparam) {
		case 8:
			this->msglog.Clear();
			break;
		case 0xD:
			wnd->OpenConsoleInput();
			break;
		case 0x25:
			if (wnd->dialogsMask == 1) {
				this->ScrollMapX(-1);
			}
			break;
		case 0x26:
			if (wnd->dialogsMask == 1) {
				this->ScrollMapY(-1);
			}
			break;
		case 0x27:
			if (wnd->dialogsMask == 1) {
				this->ScrollMapX(1);
			}
			break;
		case 0x28:
			if (wnd->dialogsMask == 1) {
				this->ScrollMapY(1);
			}
			break;
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B: {
			UserShortcut& shortcut = wnd->m_GameSession.shortcuts[wparam - 0x73];
			if ((shortcut.kind != 2 || g_kbControlState != 0) && g_kbShiftState == 0) {
				wnd->vis_spellbook->MsgProc(0x417, wparam - 0x73, g_kbControlState);
			}
			if ((shortcut.kind != 1 || g_kbShiftState != 0) && g_kbControlState == 0) {
				wnd->vis_invtype1->MsgProc(0x417, wparam - 0x73, g_kbShiftState);
			}
			if (shortcut.kind == 1) {
				if (this->IsBookOpen() == 0 && wnd->vis_spellbook->sub_4CA89B(shortcut.item_id) != 0) {
					this->sub_418F93(9);
				} else if (this->IsBookOpen() == 0 && wnd->vis_spellbook->sub_4CA8E0(shortcut.item_id) != 0) {
					this->sub_418F93(10);
				}
			}
			break;
		}
		default:
			if (wnd->dialogsMask == 1) {
				if (g_kbControlState == 0 && this->field_0x140 != 0 && (this->field_0x144 & 0x24) == 0) {
					switch (wparam) {
					case 0x41:
						this->sub_418F93(1);
						return 1;
					case 0x43:
						if (this->IsBookOpen() == 0) {
							this->sub_418F93(5);
						}
						return 1;
					case 0x44:
						this->sub_418F93(4);
						return 1;
					case 0x47:
						this->sub_418F93(3);
						return 1;
					case 0x4C:
						this->sub_41A5B4();
						return 1;
					case 0x4D:
						this->sub_418F93(2);
						return 1;
					case 0x50:
						this->sub_41A4DF();
						return 1;
					case 0x52:
						this->sub_418F93(8);
						return 1;
					case 0x53:
						this->sub_418F93(6);
						return 1;
					case 0x54:
						this->sub_418F93(7);
						return 1;
					}
				}
				switch (wparam) {
				case 0x20:
					if (wnd->dialogsMask == 1 && wnd->console_active == 0) {
						if (this->field_0x3f6c == nullptr || this->field_0x3f6c->hp > -40) {
							if (this->IsBagOpen() == 0 && this->IsBookOpen() == 0) {
								wnd->vis_right_panel->MsgProc(0x40E, 0, 0);
								this->MsgProc(0x40E, 0, 0);
								wnd->vis_right_panel->MsgProc(0x40F, 0, 0);
								this->MsgProc(0x40F, 0, 0);
							} else {
								if (this->IsBagOpen() != 0) {
									wnd->vis_right_panel->MsgProc(0x40E, 0, 0);
									this->MsgProc(0x40E, 0, 0);
								}
								if (this->IsBookOpen() != 0) {
									wnd->vis_right_panel->MsgProc(0x40F, 0, 0);
									this->MsgProc(0x40F, 0, 0);
								}
							}
						} else {
							this->sub_41B155();
						}
					}
					break;
				case 0x42:
				case 0x51:
					if (wnd->dialogsMask == 1) {
						wnd->vis_right_panel->MsgProc(0x40F, 0, 0);
						this->MsgProc(0x40F, 0, 0);
					}
					break;
				case 0x43:
					if (g_kbControlState != 0) {
						SetMessageColors(g_MessageColors == 0);
						this->msglog.Add(txt_patch.GetLine(g_MessageColors + 0x55), clr_log_sblack, 2000);
					}
					break;
				case 0x45:
					this->sub_416C4C();
					break;
				case 0x46:
					if (g_kbControlState != 0) {
						this->sub_41F6D0();
						this->msglog.Add(TxtFile::AllLines[this->sub_41F810() + 0x61], clr_log_sblack, 2000);
					}
					break;
				case 0x48:
					if (g_kbControlState != 0) {
						this->sub_41F770();
						this->msglog.Add(TxtFile::AllLines[this->show_hp + 0x64], clr_log_sblack, 2000);
					}
					break;
				case 0x49:
				case 0xC0:
					if (wnd->dialogsMask == 1) {
						wnd->vis_right_panel->MsgProc(0x40E, 0, 0);
						this->MsgProc(0x40E, 0, 0);
					}
					break;
				case 0x4B:
					if (g_kbControlState != 0 && wnd->sessionMode != 2) {
						g_settings.ClanNames = g_settings.ClanNames == 0;
						this->msglog.Add(TxtFile::AllLines[g_settings.ClanNames + 0x16F], clr_log_sblack, 2000);
					}
					break;
				case 0x4C:
					if (g_kbControlState != 0) {
						this->sub_41F7B0();
						this->msglog.Add(TxtFile::AllLines[this->flying_hp + 0x66], clr_log_sblack, 2000);
					}
					break;
				case 0x4E:
					if (g_kbControlState != 0) {
						g_settings.ShowTimeFlow = g_settings.ShowTimeFlow == 0;
						this->msglog.Add(TxtFile::AllLines[g_settings.ShowTimeFlow + 0x68], clr_log_sblack, 2000);
						this->FUN_0041d97e(1);
					}
					break;
				case 0x4F:
					if (g_kbControlState != 0) {
						g_settings.Smoothing = g_settings.Smoothing == 0;
						this->msglog.Add(TxtFile::AllLines[g_settings.Smoothing + 0x6A], clr_log_sblack, 2000);
					}
					break;
				case 0x54:
					if (g_kbControlState != 0) {
						DAT_00660f70 = DAT_00660f70 == 0;
					}
					break;
				case 0x55:
					if (g_kbControlState == 0) {
						if (wnd->dialogsMask == 1) {
							this->sub_41CE14();
						}
					} else {
						this->sub_41AB05();
						int32_t cast_state = (g_settings.AutoCasting & 0x10) != 0;
						if ((g_settings.AutoCasting & 0x20) != 0) {
							cast_state = 2;
						}
						this->msglog.Add(txt_patch.GetLine(cast_state + 0x73), clr_log_sblack, 2000);
					}
					break;
				case 0x57:
					if (g_kbControlState == 0) {
						if (wnd->sessionMode != 2) {
							wnd->PostMessage(0x466, 0, 0);
						}
					} else {
						this->sub_41F720();
						this->msglog.Add(TxtFile::AllLines[this->sub_41F7F0() + 0x5E], clr_log_sblack, 2000);
					}
					break;
				}
			}
			break;
		}
	} else if ((wnd->dialogsMask & 2) != 0 && (wparam == 0x42 || wparam == 0x51)) {
		wnd->vis_right_panel->MsgProc(0x40F, 0, 0);
		this->MsgProc(0x40F, 0, 0);
		wnd->vis_root->FindChild(1000)->MsgProc(0x40F, 0, 0);
	}
	return 0;
}

// 40BD34
void BigStruct2::sub_40BD34() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	uint32_t update_selection = 0;

	this->field_0xa88++;
	TakeDamage::UpdateDamages(&this->damage_labels);
	if (INT_00660f74 == 0 && (this->field_0xa88 & 0x1F) == 0) {
		this->sub_40C18B();
	}
	if ((this->field_0xa88 & 0xF) == 0 && g_SoundSettings.music_allowed != 0 && this->field_0x3f6c != nullptr) {
		UpdatePositionalMusic(this->field_0x3f6c->x_pos, this->field_0x3f6c->y_pos);
	}

	CDWordArray arr;
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->Update() == 0) {
			arr.Add(key);
		}
	}
	for (int32_t i = 0; i < arr.GetSize(); i++) {
		CGameObject* obj;
		this->field_0x9d0.Lookup(arr[i], obj);
		this->field_0x9d0.RemoveKey(arr[i]);
		if (obj->IsSelected() != 0) {
			this->field_0x9b4 = 0;
			wnd->vis_spellbook->FUN_004caa69();
			wnd->vis_ordertoolbar->MsgProc(0x40B, 0, 0);
		}
		delete obj;
	}
	if (arr.GetSize() != 0) {
		update_selection = 1;
	}

	this->sub_40C112();
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		update_selection = update_selection | obj->UpdateBlockedState();
	}
	if (update_selection != 0) {
		this->UpdateSelectionState();
	}

	arr.RemoveAll();
	for (POSITION it = this->field_0x9ec.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9ec.GetNextAssoc(it, key, obj);
		if (obj->Update() == 0) {
			arr.Add(key);
		}
	}
	for (int32_t i = 0; i < arr.GetSize(); i++) {
		CGameObject* obj;
		this->field_0x9ec.Lookup(arr[i], obj);
		this->field_0x9ec.RemoveKey(arr[i]);
		delete obj;
	}
}

// 417B42
void BigStruct2::sub_417B42(CPoint* pt) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	CRect select_rect = g_mousept.GetSelectFrame();
	select_rect.right = pt->x;
	select_rect.bottom = pt->y;
	select_rect.NormalizeRect();
	int32_t min_size = g_ScreenSize.right * 10 / 0x280;
	if (!select_rect.IsRectEmpty()) {
		select_rect.InflateRect(1, 1);
	}
	if (select_rect.Width() > min_size || select_rect.Height() > min_size) {
		this->sub_417F09(&select_rect);
		return;
	}
	CSprite256* cursor_sprite = g_mousept.GetCursorSprite();
	if (this->field_0x140 == 0) {
		this->sub_417F09(&select_rect);
		return;
	}
	uint16_t target_id = (uint16_t)this->field_0x9a8;
	if (cursor_sprite == g_Cursors[CURSOR_ATTACK]->GetSprite()) {
		if (target_id != 0 && this->field_0x994->IsKindOf(RUNTIME_CLASS(CUnit))) {
			this->sub_419246(target_id);
		} else {
			this->sub_419154(this->field_0x9ac, this->field_0x9b0);
		}
	} else if (cursor_sprite == g_Cursors[CURSOR_SWARM]->GetSprite()) {
		this->sub_41930D(this->field_0x9ac, this->field_0x9b0);
	} else if (cursor_sprite == g_Cursors[CURSOR_MOVE]->GetSprite()) {
		this->sub_419154(this->field_0x9ac, this->field_0x9b0);
	} else if (cursor_sprite == g_Cursors[CURSOR_PATROL]->GetSprite()) {
		this->sub_41965D(this->field_0x9ac, this->field_0x9b0);
	} else if (cursor_sprite == g_Cursors[CURSOR_DEFEND]->GetSprite()) {
		if (target_id != 0) {
			this->sub_41955C(target_id);
		}
	} else if (cursor_sprite == g_Cursors[CURSOR_SELECT]->GetSprite()) {
		this->sub_417F09(&select_rect);
	} else if (cursor_sprite == g_Cursors[CURSOR_PICKUP]->GetSprite()) {
		this->sub_41A33F(this->field_0x9ac, this->field_0x9b0);
	} else if (cursor_sprite == g_Cursors[CURSOR_TOWN]->GetSprite()) {
		this->sub_41A66E(target_id);
	} else if (cursor_sprite == g_Cursors[CURSOR_CAST]->GetSprite()) {
		int32_t spell_pos = wnd->vis_spellbook->sub_41F9B0();
		if (spell_pos >= 0) {
			if (target_id != 0) {
				if (this->field_0x994->IsKindOf(RUNTIME_CLASS(CUnit)) && wnd->vis_spellbook->sub_4CAAA7() != 0) {
					this->sub_419BA1(target_id, spell_pos + 1);
				} else {
					this->sub_41972F(this->field_0x9ac, this->field_0x9b0, spell_pos + 1);
				}
			} else if (wnd->vis_spellbook->sub_4CAAA7() == 0) {
				this->sub_41972F(this->field_0x9ac, this->field_0x9b0, spell_pos + 1);
			}
			if (this->field_0x9b4 == 5 && this->IsBookOpen() != 0) {
				this->FUN_0041b636();
			}
		}
	}
	if (this->field_0x9b4 != 0) {
		this->field_0x9b4 = 0;
		wnd->vis_spellbook->FUN_004caa69();
		wnd->vis_ordertoolbar->MsgProc(0x40B, 0, 0);
	}
}

// 417F09
void BigStruct2::sub_417F09(CRect* rect) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	int32_t min_size = g_ScreenSize.right * 10 / 0x280;
	int32_t small_rect = rect->Width() < min_size && rect->Height() < min_size;
	int32_t selected = g_kbShiftState == 0;
	if (small_rect == 0 && g_kbShiftState == 0) {
		selected = 0;
		for (int32_t i = this->field_0x134 - 1; i >= 0; i--) {
			CRect unit_rect = this->field_0x10c[i];
			CRect intersection;
			unit_rect.NormalizeRect();
			intersection.IntersectRect(&unit_rect, rect);
			if (intersection.Width() * intersection.Height() > 10) {
				CGameObject* obj;
				if (this->field_0x9d0.Lookup(this->field_0x120[i], obj) &&
					obj->map_player == this->my_main_unit && !obj->IsKindOf(RUNTIME_CLASS(CStructure))) {
					selected = 1;
				}
			}
		}
		if (selected == 0) {
			return;
		}
	}
	if (small_rect != 0 && g_kbShiftState == 0) {
		for (int32_t i = this->field_0x134 - 1; i >= 0; i--) {
			CRect unit_rect = this->field_0x10c[i];
			CRect intersection;
			unit_rect.NormalizeRect();
			intersection.IntersectRect(&unit_rect, rect);
			if (!intersection.IsRectEmpty()) {
				CGameObject* obj;
				if (this->field_0x9d0.Lookup(this->field_0x120[i], obj) && obj->IsKindOf(RUNTIME_CLASS(CStructure))) {
					selected = 0;
				}
			}
		}
	}
	if (selected != 0) {
		for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
			uint16_t key;
			CGameObject* obj;
			this->field_0x9d0.GetNextAssoc(it, key, obj);
			obj->SetSelected(0);
		}
	}
	for (int32_t i = this->field_0x134 - 1; i >= 0; i--) {
		CRect unit_rect = this->field_0x10c[i];
		CRect intersection;
		unit_rect.NormalizeRect();
		int32_t hit;
		if (small_rect != 0) {
			hit = intersection.IntersectRect(&unit_rect, rect);
		} else {
			intersection.IntersectRect(&unit_rect, rect);
			hit = intersection.Width() * intersection.Height() > 10;
		}
		if (hit == 0) {
			continue;
		}
		CGameObject* obj;
		if (!this->field_0x9d0.Lookup(this->field_0x120[i], obj)) {
			continue;
		}
		if (g_kbShiftState != 0) {
			if (obj->map_player == this->my_main_unit && (this->field_0x144 & 4) == 0) {
				obj->SetSelected(obj->IsSelected() == 0);
			}
		} else if (small_rect == 0) {
			if (obj->map_player == this->my_main_unit && !obj->IsKindOf(RUNTIME_CLASS(CStructure))) {
				obj->SetSelected(1);
			}
		} else if (!obj->IsKindOf(RUNTIME_CLASS(CStructure))) {
			obj->SetSelected(1);
		}
		if (small_rect != 0) {
			break;
		}
	}
	this->UpdateSelectionState();
	if (g_kbMenuState != 0 && this->field_0x140 == 1 && this->field_0x138->GetControlGroup() >= 0) {
		this->sub_416983(this->field_0x138->GetControlGroup());
	}
	this->field_0x9b4 = 0;
	wnd->vis_spellbook->FUN_004caa69();
	wnd->vis_ordertoolbar->MsgProc(0x40B, 0, 0);
	if (g_kbShiftState == 0 && this->field_0x140 != 0 && (this->field_0x144 & 1) != 0 && (this->field_0x144 & 4) == 0) {
		CUnit* unit = this->sub_41DFDB();
		if (unit != nullptr) {
			unit->PlaySelectVoice();
		}
	}
}

// 40C112
void BigStruct2::sub_40C112() {
	for (POSITION it = this->field_0xa94.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		uint32_t value;
		this->field_0xa94.GetNextAssoc(it, key, value);
		value++;
		if (value < 0x16) {
			this->field_0xa94.SetAt(key, value);
		} else {
			this->field_0xa94.RemoveKey(key);
		}
	}
}

// 40C18B
void BigStruct2::sub_40C18B() {
	this->field_0xdc = 1;
	this->field_0xe0 = 1;
	uint16_t* landscape = this->field_0x80->GetLandscape();
	int32_t count = this->map_width * this->map_height;
	for (int32_t i = 0; i < count; i++) {
		landscape[i] &= 0xBFFF;
	}
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		obj->MarkOccupancy();
	}
}

// 40C232
int32_t BigStruct2::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	int32_t result = CVisualObject::MsgProc(msg, wparam, lparam);
	if (result != 0) {
		return result;
	}
	switch (msg) {
	case 0x401:
		this->sub_40BD34();
		this->msglog.Update();
		this->FUN_0041d97e(0);
		break;
	case 0x402:
		this->sub_406F7B();
		break;
	case 0x405:
		this->sub_4168BD();
		result = 1;
		break;
	case 0x406:
		this->sub_4167F7(wparam, lparam);
		result = 1;
		break;
	case 0x40C:
		g_SfxArray[8]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xDC, 0);
		this->sub_418F93(wparam + 1);
		result = 1;
		break;
	case 0x40E:
		if (this->IsBagOpen()) {
			this->FUN_0041b40e();
		} else {
			this->FUN_0041b381();
		}
		if (g_mousept.GetSelectState() != 0) {
			g_mousept.ResetStates();
			wnd->ApplyCursorClip();
			this->sub_40B314();
			CPoint pt(g_mousept.GetX(), g_mousept.GetY());
			this->sub_417B42(&pt);
		}
		result = 1;
		break;
	case 0x40F:
		if (this->IsBookOpen()) {
			this->FUN_0041b636();
		} else {
			this->FUN_0041b509();
		}
		if (g_mousept.GetSelectState() != 0) {
			g_mousept.ResetStates();
			wnd->ApplyCursorClip();
			this->sub_40B314();
			CPoint pt(g_mousept.GetX(), g_mousept.GetY());
			this->sub_417B42(&pt);
		}
		result = 1;
		break;
	case 0x485:
		if ((wnd->dialogsMask & 1) != 0) {
			this->field_0x49b8 = -1;
			this->field_0x49bc = -1;
			this->UpdateAmbientSounds();
			result = 0;
		}
		break;
	}
	return result;
}

// 40C633
int32_t BigStruct2::OnLButtonUp(uint32_t wparam, CPoint pos) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->dialogsMask != 1) {
		return 0;
	}
	if (g_mousept.GetSelectState() != 0) {
		g_mousept.ResetStates();
		wnd->ApplyCursorClip();
		this->sub_40B314();
		this->sub_417B42(&pos);
	}
	if (wnd->drag_entry != nullptr) {
		if (this->field_0x140 == 1 && this->field_0x138 != nullptr) {
			int32_t tile_x = this->field_0x138->tileX;
			int32_t tile_y = this->field_0x138->tileY;
			if (this->field_0x994 != nullptr && this->field_0x994->IsKindOf(RUNTIME_CLASS(CUnit)) && (this->field_0x994->unitFlags & 1) != 0 &&
				abs(this->field_0x994->tileX - this->field_0x138->tileX) <= 2 && abs(this->field_0x994->tileY - this->field_0x138->tileY) <= 2) {
				tile_x = this->field_0x994->tileX;
				tile_y = this->field_0x994->tileY;
			}
			if (wnd->drag_entry->IsEmpty() != 0) {
				this->sub_41AC88(wnd->drag_entry->count, tile_x | (tile_y << 8));
			} else {
				this->sub_41A7C7(wnd->drag_src_tab, wnd->drag_src_slot, 3, tile_x | (tile_y << 8), wnd->drag_entry->count);
			}
		}
		delete wnd->drag_entry;
		g_Cursors[0]->Use();
		wnd->ResetItemCursor();
	}
	return 1;
}

// 41C630
void BigStruct2::FUN_0041c630(CWordArray* data) {
	uint16_t flag = (*data)[0];
	if (flag != 0) {
		flag = 0x2000;
	}
	data->RemoveAt(0, 1);
	uint16_t* land = this->field_0x80->GetLandscape();
	int32_t x = 8;
	int32_t y = 8;
	while (data->GetSize() != 0) {
		for (int32_t i = 0; i < (*data)[0]; i++) {
			uint16_t* cell = land + x + y * this->map_width;
			*cell = (*cell & 0xDFFF) | flag;
			x++;
			if (x == this->map_width - 8) {
				x = 8;
				y++;
			}
		}
		data->RemoveAt(0, 1);
		flag ^= 0x2000;
	}
}

// 41C4A1
void BigStruct2::FUN_0041c4a1(const char* name) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	delete this->field_0x80;
	CString str(name);
	if (wnd->sessionMode == 2) {
		str = "scenario\\" + str;
	}
	this->field_0x80 = new Scenario(str);
	this->field_0xdc = 1;
	this->sub_403F08(this->field_0x80);
	wnd->vis_right_panel->MsgProc(0x403, (uint32_t)this, 0);
	this->FUN_0041d97e(1);
	LoadTerrainTiles(this->field_0x80->terrain_tile_mask);
	this->field_0x4970->RemovePlayerQuests(0);
}

// 403395
BigStruct2::~BigStruct2() {
	delete this->field_0x3f68;
	this->sub_40328E();
	delete this->field_0x80;

	POSITION it = this->field_0x9d0.GetStartPosition();
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		delete obj;
	}

	it = this->field_0x9ec.GetStartPosition();
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9ec.GetNextAssoc(it, key, obj);
		delete obj;
	}

	for (int32_t i = 0; i < this->field_0x9b8.GetSize(); i++) {
		MapPlayerData* data = this->field_0x9b8[i];
		delete data;
	}

	delete this->field_0x4970;

	while (this->field_0x4974.GetSize() != 0) {
		Item* item = this->field_0x4974[0];
		delete item;
		this->field_0x4974.RemoveAt(0, 1);
	}

	for (int32_t i = 0; i < 12; i++) {
		delete this->field_0x4988[i];
	}
}

// 4764BC
uint8_t* __cdecl UpdateDayNightLighting(int32_t val) {
	if (g_settings.ShowTimeFlow == 0) {
		g_DeltaCLR.r = 0;
		g_DeltaCLR.g = 0;
		g_DeltaCLR.b = 0;
		g_DeltaCLR.brightness = 0xE;
		g_DeltaCLR.contrast = 0x20;
		g_DeltaCLR.shade_delta = 4;
		g_DeltaCLR.lit_delta = 2;
		g_DeltaCLR.sun_angle = 0.78539815;
		return &g_DeltaCLR.r;
	}

	int32_t hour = (val / 60) % 24;
	if (hour >= 6 && hour < 18) {
		g_DeltaCLR.r = 0;
		g_DeltaCLR.g = 0;
		g_DeltaCLR.b = 0;
		g_DeltaCLR.brightness = 0xE;
		g_DeltaCLR.contrast = 0x20;
		g_DeltaCLR.shade_delta = 4;
		g_DeltaCLR.lit_delta = 2;
		g_DeltaCLR.sun_angle = ((double)((val + 0x168) % 0x2D0) * 3.1415926) / 2.0 / 720.0 - 0.78539815;
	} else if (hour < 2 || hour >= 22) {
		g_DeltaCLR.r = 0;
		g_DeltaCLR.g = 12;
		g_DeltaCLR.b = 48;
		g_DeltaCLR.brightness = 0x20;
		g_DeltaCLR.contrast = 8;
		g_DeltaCLR.shade_delta = 8;
		g_DeltaCLR.lit_delta = 4;
		g_DeltaCLR.sun_angle = 0.78539815 - (((double)((val + 0x78) % 0xF0) * 3.1415926) / 2.0) / 240.0;
	} else {
		int32_t delta = val % 0x78;
		if (hour < 4) {
			g_DeltaCLR.r = (uint8_t)((delta * 0x18) / 0x78);
			g_DeltaCLR.g = 12;
			g_DeltaCLR.b = (uint8_t)(0x30 - (delta * 0x30) / 0x78);
			g_DeltaCLR.brightness = 0x20 - (delta * 4) / 0x78;
			g_DeltaCLR.contrast = (delta * 0xC) / 0x78 + 8;
			g_DeltaCLR.shade_delta = 6;
			g_DeltaCLR.lit_delta = 3;
			g_DeltaCLR.sun_angle = -0.78539815;
		} else if (hour < 6) {
			g_DeltaCLR.r = (uint8_t)(0x18 - (delta * 0x18) / 0x78);
			g_DeltaCLR.g = (uint8_t)(12 - (delta * 0xC) / 0x78);
			g_DeltaCLR.b = 0;
			g_DeltaCLR.brightness = 0x1C - (delta * 0xE) / 0x78;
			g_DeltaCLR.contrast = (delta * 0xC) / 0x78 + 0x14;
			g_DeltaCLR.shade_delta = 6;
			g_DeltaCLR.lit_delta = 3;
			g_DeltaCLR.sun_angle = -0.78539815;
		} else if (hour < 20) {
			g_DeltaCLR.r = (uint8_t)((delta * 0x18) / 0x78);
			g_DeltaCLR.g = 0;
			g_DeltaCLR.b = (uint8_t)((delta * 8) / 0x78);
			g_DeltaCLR.brightness = (delta * 10) / 0x78 + 0xE;
			g_DeltaCLR.contrast = 0x20 - (delta * 0xC) / 0x78;
			g_DeltaCLR.shade_delta = 6;
			g_DeltaCLR.lit_delta = 3;
			g_DeltaCLR.sun_angle = 0.78539815;
		} else {
			g_DeltaCLR.r = (uint8_t)(0x18 - (delta * 0x18) / 0x78);
			g_DeltaCLR.g = (uint8_t)((delta * 0xC) / 0x78);
			g_DeltaCLR.b = (uint8_t)((delta * 0x28) / 0x78 + 8);
			g_DeltaCLR.brightness = ((delta * 8) / 0x78) + 0x18;
			g_DeltaCLR.contrast = 0x14 - (delta * 0xC) / 0x78;
			g_DeltaCLR.shade_delta = 6;
			g_DeltaCLR.lit_delta = 3;
			g_DeltaCLR.sun_angle = 0.78539815;
		}
	}
	return &g_DeltaCLR.r;
}

// 47fa75
void ReloadPalettes() {
	g_mousept.Update();
	for (int32_t i = 0; i < g_VFX_info.GetSize(); i++) {
		UnitVFXUnfo* vfx = g_VFX_info[i];
		if (vfx != nullptr) {
			for (int32_t j = 0; j < vfx->palette; j++) {
				if (vfx->palettes[j] != nullptr) {
					vfx->palettes[j]->Free();
				}
			}
		}
	}
	g_mousept.Update();
	for (int32_t i = 0; i < 16; i++) {
		CGamePalette* pal = new CGamePalette();
		pal->SetPalette(g_human_pals_data[i], 0x10, 2, 1);
		if (g_Human_palettes[i] != nullptr) {
			delete g_Human_palettes[i];
		}
		g_Human_palettes.SetAt(i, pal);
		for (int32_t j = 0; j < 8; j++) {
			uint16_t clr = pal->GetPalette(15 - j)[0xa4];
			g_colors_human_pals[i][j * 2] = clr;
			g_colors_human_pals[i][j * 2 + 1] = clr;
		}
	}
	g_mousept.Update();
	for (int32_t i = 0; i < 32; i++) {
		if (g_ter_tiles(0, i) != nullptr) {
			g_ter_tiles(0, i)->ResetPalette(0x60, 3, 1);
			DAT_00661018[i] = &g_ter_tiles(0, i)->palette;
		}
	}
	g_mousept.Update();
	for (int32_t i = 0; i < g_GfxFiles.GetSize(); i++) {
		GfxFile* gfx = g_GfxFiles[i];
		if (gfx != nullptr && gfx->inited != 0) {
			gfx->spr->palette.Free();
			gfx->spr->ResetPalette(0x10, 2, 1);
		}
	}
	g_mousept.Update();
	for (int32_t i = 0; i < g_StructuresInfo.GetSize(); i++) {
		StructureInfo* structure = g_StructuresInfo[i];
		if (structure != nullptr && structure->inited != 0) {
			structure->spr->palette.Free();
			structure->spr->ResetPalette(0x10, 2, 1);
		}
	}
	g_mousept.Update();
	for (int32_t i = 0; i < g_ProjectileInfos.GetSize(); i++) {
		ProjectileInfo* projectile = g_ProjectileInfos[i];
		if (projectile != nullptr && projectile->inited != 0 && projectile->palette != 0 && projectile->a16 == 0) {
			projectile->sprite1->palette.Free();
			projectile->sprite1->ResetPalette(0x10, 2, 1);
		}
	}
	g_mousept.Update();
	g_spr_backpack->palette.Free();
	g_spr_backpack->ResetPalette(0x10, 2, 1);
}

// 477b59
void __cdecl LoadTerrainTiles(uint32_t val) {
	if ((g_DWORD_00659d18 & 2) != 0) {
		if (val != 0) {
			DAT_00665344 = new CBmp256("graphics\\terrain.3d\\dirt.bmp");
		}
		for (int32_t i = 0; i < 32; i++) {
			if ((val & (1 << i)) == 0) {
				for (int32_t j = 0; j < 4; j++) {
					if (g_ter_tiles(j, i) != nullptr) {
						delete g_ter_tiles(j, i);
						g_ter_tiles(j, i) = nullptr;
					}
				}
			} else {
				for (int32_t j = 0; j < 4; j++) {
					if (g_ter_tiles(j, i) != nullptr) {
						delete g_ter_tiles(j, i);
					}
					int32_t tile_group = (i >> 2) + 1;
					int32_t tile_num = j + (i & 3) * 4;
					char name[256];
					if (tile_num < 10) {
						sprintf(name, "graphics\\terrain.3d\\tile%d-0%d.bmp", tile_group, tile_num);
					} else {
						sprintf(name, "graphics\\terrain.3d\\tile%d-%d.bmp", tile_group, tile_num);
					}
					g_ter_tiles(j, i) = new CBmp256(name);
					g_mousept.Update();
				}
			}
		}
	}
	if (val != 0) {
		DAT_00665344 = new CBmp256("graphics\\terrain\\dirt.bmp");
	}
	for (int32_t i = 0; i < 32; i++) {
		if ((val & (1 << i)) == 0) {
			for (int32_t j = 0; j < 4; j++) {
				if (g_ter_tiles(j, i) != nullptr) {
					delete g_ter_tiles(j, i);
					g_ter_tiles(j, i) = nullptr;
				}
			}
		} else {
			for (int32_t j = 0; j < 4; j++) {
				if (g_ter_tiles(j, i) != nullptr) {
					delete g_ter_tiles(j, i);
				}
				int32_t tile_group = (i >> 2) + 1;
				int32_t tile_num = j + (i & 3) * 4;
				char name[256];
				if (tile_num < 10) {
					sprintf(name, "graphics\\terrain\\tile%d-0%d.bmp", tile_group, tile_num);
				} else {
					sprintf(name, "graphics\\terrain\\tile%d-%d.bmp", tile_group, tile_num);
				}
				g_ter_tiles(j, i) = new CBmp256(name);
				g_mousept.Update();
			}
		}
	}
	for (int32_t i = 0; i < 32; i++) {
		if (g_ter_tiles(0, i) != nullptr) {
			g_ter_tiles(0, i)->ResetPalette(0x60, 3, 1);
			DAT_00661018[i] = &g_ter_tiles(0, i)->palette;
		}
	}
}

void LoadVfxData() { //47b24e

	RegFile reg("graphics\\units\\units.reg");

	int32_t file_count = reg.GetInt("Global", "FileCount", 0);
	int32_t unit_count = reg.GetInt("Global", "UnitCount", 0);

	CString tmp;
	for (int32_t i = 0; i < file_count; i++) {
		tmp.Format("File%d", i);
		char buf[256];
		buf[0] = 0;

		reg.GetSizedString("Files", tmp, "", buf, sizeof(buf));
		g_UnitGfxFiles.Add(new UnitGfxFile(buf));
	}

	for (int32_t i = 0; i < unit_count; i++) {
		g_mousept.Update();

		tmp.Format("Unit%d", i);

		UnitVFXUnfo* inf = new UnitVFXUnfo();
		inf->ID = reg.GetInt(tmp, "ID", -1);
		inf->N = i;

		int32_t parentid = reg.GetInt(tmp, "Parent", -1);

		UnitVFXUnfo* pinfo = nullptr;
		CString ptmp;
		if (parentid != -1) {
			pinfo = g_VFX_info[parentid];
			ptmp.Format("Unit%d", pinfo->N);
		}

		int32_t t = -1;
		if (pinfo) {
			t = pinfo->file;
		}

		inf->file = reg.GetInt(tmp, "File", pinfo != nullptr ? pinfo->file : -1);
		inf->index = reg.GetInt(tmp, "Index", pinfo != nullptr ? pinfo->index : -1);
		inf->move_phases = reg.GetInt(tmp, "MovePhases", pinfo != nullptr ? pinfo->move_phases : -1);
		inf->move_begin_phases = reg.GetInt(tmp, "MoveBeginPhases", pinfo != nullptr ? pinfo->move_begin_phases : -1);
		inf->attack_phases = reg.GetInt(tmp, "AttackPhases", pinfo != nullptr ? pinfo->attack_phases : -1);
		inf->dying_phases = reg.GetInt(tmp, "DyingPhases", pinfo != nullptr ? pinfo->dying_phases : -1);
		inf->bone_phases = reg.GetInt(tmp, "BonePhases", pinfo != nullptr ? pinfo->bone_phases : -1);
		inf->idle_phases = reg.GetInt(tmp, "IdlePhases", pinfo != nullptr ? pinfo->idle_phases : 0);
		inf->width = reg.GetInt(tmp, "Width", pinfo != nullptr ? pinfo->width : -1);
		inf->height = reg.GetInt(tmp, "Height", pinfo != nullptr ? pinfo->height : -1);
		inf->center_x = reg.GetInt(tmp, "CenterX", pinfo != nullptr ? pinfo->center_x : -1);
		inf->center_y = reg.GetInt(tmp, "CenterY", pinfo != nullptr ? pinfo->center_y : -1);
		inf->selection.left = reg.GetInt(tmp, "SelectionX1", pinfo != nullptr ? pinfo->selection.left : -1);
		inf->selection.top = reg.GetInt(tmp, "SelectionY1", pinfo != nullptr ? pinfo->selection.top : -1);
		inf->selection.right = reg.GetInt(tmp, "SelectionX2", pinfo != nullptr ? pinfo->selection.right : -1);
		inf->selection.bottom = reg.GetInt(tmp, "SelectionY2", pinfo != nullptr ? pinfo->selection.bottom : -1);
		inf->dying = reg.GetInt(tmp, "Dying", pinfo != nullptr ? pinfo->dying : 0);
		inf->palette = reg.GetInt(tmp, "Palette", pinfo != nullptr ? pinfo->palette : 0);

		reg.GetSizedString(tmp, "DescText", "", inf->desc_text, sizeof(inf->desc_text));

		for (int j = 0; j < inf->palette; j++) {
			inf->palettes[j] = new CGamePalette();

			CString pfile = g_UnitGfxFiles[inf->file]->fname;
			pfile = pfile.Left(pfile.ReverseFind('\\'));
			if (j == 0) {
				pfile += "\\palette.pal";
			} else {
				CString pf;
				pf.Format("\\palette%d.pal", j + 1);
				pfile += pf;
			}

			File2 f;
			f.Open(pfile, CFile::modeRead);
			f.Seek(0x36, CFile::begin);

			inf->palette_datas[j] = new uint8_t[256 * 4];
			f.Read(inf->palette_datas[j], 256 * 4);
			f.Close();
		}
		
		inf->flip = reg.GetInt(tmp, "Flip", pinfo != nullptr ? pinfo->flip : 0);

		reg.GetInt32Array(tmp, "Sound", &inf->sound);
		if (inf->sound.GetSize() == 0 && pinfo) {
			inf->sound.Copy(pinfo->sound);
		}

		inf->tile_size = reg.GetInt(tmp, "TileSize", pinfo != nullptr ? pinfo->tile_size : 1);
		inf->projectile = reg.GetInt(tmp, "Projectile", pinfo != nullptr ? pinfo->projectile : 0);

		reg.GetSizedString(tmp, "InfoPicture", "", inf->info_picture, sizeof(inf->info_picture));
		if (strlen(inf->info_picture) == 0 && pinfo) {
			strcpy(inf->info_picture, pinfo->info_picture);
		}

		inf->shoot_delay = reg.GetInt(tmp, "ShootDelay", pinfo != nullptr ? pinfo->shoot_delay : 0);
		inf->attack_delay = reg.GetInt(tmp, "AttackDelay", pinfo != nullptr ? pinfo->attack_delay : 0);
		inf->z = reg.GetInt(tmp, "Z", pinfo != nullptr ? pinfo->z : 0);

		reg.GetInt32Array(tmp, "ShootOffset", &inf->shoot_offset);
		if (inf->shoot_offset.GetSize() == 0 && pinfo) {
			inf->shoot_offset.Copy(pinfo->shoot_offset);
		}

		CArray<uint32_t> local_2b4;
		CArray<uint32_t> local_4d0;

		reg.GetInt32Array(tmp, "MoveAnimTime", &local_2b4);
		if (local_2b4.GetSize() == 0 && pinfo) {
			reg.GetInt32Array(ptmp, "MoveAnimTime", &local_2b4);
		}


		reg.GetInt32Array(tmp, "MoveAnimFrame", &local_4d0);
		if (local_4d0.GetSize() == 0 && pinfo) {
			reg.GetInt32Array(ptmp, "MoveAnimFrame", &local_4d0);
		}

		for(int j = 0; j < local_4d0.GetSize() && j < local_2b4.GetSize(); j++) {
			int32_t count = local_2b4[j];
			int32_t frame = local_4d0[j];

			inf->move_anim_frames.InsertAt(inf->move_anim_frames.GetSize(), frame, count);
		}
		inf->move_anim_frame_cnt = inf->move_anim_frames.GetSize();


		reg.GetInt32Array(tmp, "IdleAnimTime", &local_2b4);
		if (local_2b4.GetSize() == 0 && pinfo) {
			reg.GetInt32Array(ptmp, "IdleAnimTime", &local_2b4);
		}


		reg.GetInt32Array(tmp, "IdleAnimFrame", &local_4d0);
		if (local_4d0.GetSize() == 0 && pinfo) {
			reg.GetInt32Array(ptmp, "IdleAnimFrame", &local_4d0);
		}

		for (int j = 0; j < local_4d0.GetSize() && j < local_2b4.GetSize(); j++) {
			int32_t count = local_2b4[j];
			int32_t frame = local_4d0[j];

			inf->idle_anim_frames.InsertAt(inf->idle_anim_frames.GetSize(), frame, count);
		}
		inf->idle_anim_frame_cnt = inf->idle_anim_frames.GetSize();



		reg.GetInt32Array(tmp, "AttackAnimTime", &local_2b4);
		if (local_2b4.GetSize() == 0 && pinfo) {
			reg.GetInt32Array(ptmp, "AttackAnimTime", &local_2b4);
		}


		reg.GetInt32Array(tmp, "AttackAnimFrame", &local_4d0);
		if (local_4d0.GetSize() == 0 && pinfo) {
			reg.GetInt32Array(ptmp, "AttackAnimFrame", &local_4d0);
		}

		for (int j = 0; j < local_4d0.GetSize() && j < local_2b4.GetSize(); j++) {
			int32_t count = local_2b4[j];
			int32_t frame = local_4d0[j];

			inf->attack_anim_frames.InsertAt(inf->attack_anim_frames.GetSize(), frame, count);
		}
		inf->attack_anim_frame_cnt = inf->attack_anim_frames.GetSize();

		g_VFX_info.SetAtGrow(inf->ID, inf);
	}

	File2 f;
	f.Open("graphics\\units\\humans\\human.pal", CFile::modeRead);
	f.Read(g_human_pals_data, 16 * 4 * 256);
	f.Close();

	for (int i = 0; i < 16; i++) {
		CGamePalette* pal = new CGamePalette();

		pal->SetPalette(g_human_pals_data[i], 16, 2, 1);
		g_Human_palettes.Add(pal);

		for (int j = 0; j < 8; j++) {
			g_colors_human_pals[i][j * 2] = pal->GetPalette(15 - j)[164];
			g_colors_human_pals[i][j * 2 + 1] = pal->GetPalette(15 - j)[164];
		}

		if (i == 15) {
			pal = new CGamePalette();
			pal->SetPalette(g_human_pals_data[0], 16, 5, 0);
			g_Human_palettes.Add(pal);
		}
	}
}




void BigStruct2::UpdateAmbientSounds() { //41b8d0
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	bool local_d8;
	if (this->view_x == this->field_0x49b8 && view_y == this->field_0x49bc) {
		local_d8 = false;
	} else {
		local_d8 = true;
	}

	bool local_10 = timeGetTime() > this->field_0x49c0;
	if (local_d8 || local_10) {
		if (local_d8) {
			this->field_0x49b8 = this->view_x;
			this->field_0x49bc = view_y;
		}

		SfxSample* smpl_50 = g_SfxArray[50];
		SfxSample* smpl_90 = g_SfxArray[90];
		SoundChannel* channel_50 = smpl_50->FindPlayingChannel();
		SoundChannel* channel_90 = smpl_90->FindPlayingChannel();

		int32_t vol_50 = -10000;
		int32_t vol_90 = -10000;
		int32_t pan_50 = 0;
		int32_t pan_90 = 0;
		int32_t local_28 = 0;
		int32_t local_2c = 0;

		int32_t minX = this->view_x - this->view_tiles_x;
		if (minX < 8) {
			minX = 8;
		}
		
		int32_t maxX = this->view_x + this->view_tiles_x * 2;
		if (maxX > this->map_width - 8) {
			maxX = this->map_width - 8;
		}
		
		int32_t minY = view_y - this->view_tiles_y;
		if (minY < 8) {
			minY = 8;
		}
		
		int32_t maxY = view_y + this->view_tiles_y * 2;
		if (maxY > this->map_height - 8) {
			maxY = this->map_height - 8;
		}
		
		uint16_t* landscape = this->field_0x80->GetLandscape();
		uint8_t* local_78 = this->field_0x80->GetObjectMap();
		int32_t count_50 = 0;
		int32_t count_90 = 0;
		int32_t local_34 = 0;
		int32_t local_58 = 0;
		int32_t local_44 = 0;
		int32_t centerX = this->view_x + (this->view_tiles_x / 2);
		int32_t centerY = view_y + (this->view_tiles_y / 2);

		for (int32_t yy = minY; yy <= maxY; yy++) {
			int32_t land_idx = minX + yy * this->map_width;
			int32_t dy = yy - centerY;

			for (int32_t xx = minX; xx <= maxX; xx++) {
				int32_t dx = xx - centerX;

				uint16_t tmp = (landscape[land_idx] & 0x1fff) >> 6;
				if (tmp > 7 && tmp < 12) {
					int32_t scale = (dx * 2000) / this->view_tiles_x;
					if (scale < -2000) {
						scale = -2000;
					}
					
					if (scale > 2000) {
						scale = 2000;
					}
				
					int32_t vol = 10000.0 - (exp(sqrt(dx * dx + dy * dy) / 8) - 1.0) * 100.0;
					if (vol < 0) {
						vol = 0;
					}
					
					if (vol_50 < vol - 10000) {
						vol_50 = vol - 10000;
					}
					
					pan_50 += (scale * vol) / 10000;
					count_50++;
				}

				uint32_t local_88;
				if (this->field_0xa08.Lookup(PosYX(xx, yy).val, local_88) != 0 && (local_88 & 8) != 0) {
					int32_t scale = (dx * 2000) / this->view_tiles_x;
					if (scale < -2000) {
						scale = -2000;
					}
					
					if (scale > 2000) {
						scale = 2000;					
					}

					int32_t vol = 10000.0 - (exp(sqrt(dx * dx + dy * dy) / 8) - 1.0) * 100.0;

					if (vol < 0) {
						vol = 0;
					}

					if (vol_90 < vol - 10000) {
						vol_90 = vol - 10000;
					}
					
					pan_90 += (scale * vol) / 10000;
					count_90++;
				}

				if (local_10 && local_78[land_idx] != 0) {
					uint8_t objid = local_78[land_idx] - 1;
					GfxObject* gobj = g_GfxObjects[objid];

					if (gobj->dead_object < 0) {
						if (gobj->fire_object == -2) {
							local_44++;

							int32_t scale = (dx * 2000) / this->view_tiles_x;
							if (scale < -2000) {
								scale = -2000;
							}
							if (scale > 2000) {
								scale = 2000;
							}

							int32_t vol = 10000.0 - (exp(sqrt(dx * dx + dy * dy) / 8) - 1.0) * 100.0;
							if (vol < 0) {
								vol = 0;
							}

							local_28 += (scale * vol) / 10000;
						}
					} else {
						if (gobj->phases < 2) {
							local_58++;
						} else {
							local_34++;
						}

						int32_t scale = (dx * 2000) / this->view_tiles_x;
						if (scale < -2000) {
							scale = -2000;
						}
						if (scale > 2000) {
							scale = 2000;
						}

						int32_t vol = 10000.0 - (exp(sqrt(dx * dx + dy * dy) / 8) - 1.0) * 100.0;
						if (vol < 0) {
							vol = 0;
						}

						local_28 += (scale * vol) / 10000;
					}
				}
				land_idx++;
			}
		}
		if (count_50 != 0) {
			pan_50 /= count_50;
		}

		if (count_90 != 0) {
			pan_90 /= count_90;
		}

		if (!channel_50) {
			if (count_50) {
				smpl_50->Play(vol_50 + g_SoundSettings.sfx_pos, pan_50, 1, 220, 0);
			}
		} else if (count_50 == 0) {
			channel_50->Stop();
		} else {
			channel_50->SetVolume(vol_50 + g_SoundSettings.sfx_pos);
			channel_50->SetPan(pan_50);
		}

		if (!channel_90) {
			if (count_90) {
				smpl_90->Play(vol_90 + g_SoundSettings.sfx_pos, pan_90, 1, 0xdc, 0);
			}
		} else if (count_90 == 0) {
			channel_90->Stop();
		} else {
			channel_90->SetVolume(vol_90 + g_SoundSettings.sfx_pos);
			channel_90->SetPan(pan_90);
		}

		if (local_34 + local_58 + local_44 != 0) {
			local_28 /= local_34 + local_58 + local_44;
			int32_t iVar4 = Random0N(local_34 + local_58 + local_44 - 1);
			
			int32_t base_idx = 600;

			if (local_34 <= iVar4) {
				base_idx += 20;
			}
			
			if (local_34 + local_58 <= iVar4) {
				base_idx += 20;
			}
			

			int32_t tmp = (((mwnd->serverLoopCounter / 16) + 360) / 60) % 24;
			if (tmp < 4 || tmp > 19) {
				base_idx += 10;
			}

			SfxSample* smpl = nullptr;
			while(!smpl) {
				smpl = g_SfxArray[base_idx + Random0N(10)];
			}

			smpl->Play(g_SoundSettings.sfx_pos, local_28, 0, 220);
			this->field_0x49c0 = timeGetTime() + 5000 + rand() / 7;
		}
	}
}

void BigStruct2::MakeHeightCells() { //4065d3
	uint8_t* hmap = this->field_0x80->GetMapHeights();
	for (int32_t y = -3; y <= this->view_tiles_y + 7; y++) {
		for (int32_t x = -3; x <= this->view_tiles_x + 3; x++) {
			this->tile_corner_y[x + 3 + (y + 3) * (this->view_tiles_x + 7)] = y * 32 - hmap[x + this->view_x + (y + this->view_y) * this->map_width];
		}
	}

	for (int32_t y = -3; y < this->view_tiles_y + 7; y++) {
		for (int32_t x = -3; x < this->view_tiles_x + 3; x++) {
			const int32_t idx = x + 3 + (y + 3) * (this->view_tiles_x + 7);
			const int32_t out_idx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
			if (this->tile_corner_y[idx] < this->tile_corner_y[idx + 1]) {
				this->tile_top_y[out_idx] = this->tile_corner_y[idx];
			} else {
				this->tile_top_y[out_idx] = this->tile_corner_y[idx + 1];
			}
			
			if (this->tile_corner_y[idx + this->view_tiles_x + 8] < this->tile_corner_y[idx + this->view_tiles_x + 7]) {
				this->tile_bottom_y[out_idx] = this->tile_corner_y[idx + this->view_tiles_x + 7];
			} else {
				this->tile_bottom_y[out_idx] = this->tile_corner_y[idx + this->view_tiles_x + 8];
			}
		}
	}

	for (int32_t y = -3; y < this->view_tiles_y + 9; y++) {
		for (int32_t x = -3; x < this->view_tiles_x + 5; x++) {
			int32_t idx = x - 1 + this->view_x + (y - 1 + this->view_y) * this->map_width;
			if (y + this->view_y < this->map_height) {
				this->height_cells[x + 3 + (y + 3) * (this->view_tiles_x + 8)] = (hmap[idx] + hmap[idx + 1] + hmap[idx + this->map_width] + hmap[idx + 1 + this->map_width]) / 4;
			}
		}
	}
}



void BigStruct2::ApplyDynamicLight(int32_t tx, int32_t ty, int32_t radius, int32_t brightness) { //406340
	if (g_Animation == 0) {
		CRect local_2c((tx - this->view_x - 1 - radius) * 32, 
			           (ty - this->view_y - 1 - radius) * 32,
			           (tx - this->view_x + 2 + radius) * 32, 
			           (ty - this->view_y + 2 + radius) * 32);
		this->field_0xf4.UnionRect(this->field_0xf4, local_2c);
	}

	if (g_Lightning != 0) {
		int32_t mdist = radius * (radius + 1);

		if (radius == 0) {
			mdist = 1;
		}
		
		int32_t maxX = this->view_tiles_x + 4;
		int32_t out_w = this->view_tiles_x + 7;
		int32_t maxY = this->view_tiles_y + 8;
		int32_t dx = tx - this->view_x;
		int32_t dy = ty - this->view_y;

		for (int32_t y = 0; y <= radius; y++) {
			for (int32_t x = 0; x <= radius; x++) {
				if (g_DistTable[y + 20][x + 20] < mdist) {
					int32_t out_x = dx + 4 + y;
					int32_t out_y = dy + 4 + x;
					int32_t out_x2 = (dx - y) + 3;
					int32_t out_y2 = (dy - x) + 3;

					if (out_x > 2 && out_x <= maxX) {
						if (out_y > 2 && out_y <= maxY) {
							this->field_0xa8[out_x + out_y * out_w] = brightness;
							this->field_0x108++;
						}
						if (out_y2 > 2 && out_y2 <= maxY) {
							this->field_0xa8[out_x + out_y2 * out_w] = brightness;
							this->field_0x108++;
						}
					}
					if (out_x2 > 2 && out_x2 <= maxX) {
						if (out_y > 2 && out_y <= maxY) {
							this->field_0xa8[out_x2 + out_y * out_w] = brightness;
							this->field_0x108++;
						}
						if (out_y2 > 2 && out_y2 <= maxY) {
							this->field_0xa8[out_x2 + out_y2 * out_w] = brightness;
							this->field_0x108++;
						}
					}
				}
			}
		}
	}
}

// 41a001
void BigStruct2::FUN_0041a001(int32_t idx) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	if (wnd->vis_spellbook->sub_4CA89B(idx - 1) != 0 && wnd->vis_spellbook->spell == -1) {
		POSITION it = this->field_0x9d0.GetStartPosition();
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->field_0x9d0.GetNextAssoc(it, key, obj);
			if (obj->IsSelected() && (obj->availableSpellMask & (1 << (idx - 1))) != 0) {
				pkt->from_player_id = this->my_main_unit->index;
				pkt->to_player_id = 0;
				pkt->id = 0x1E;
				pkt->slot = idx;
				pkt->count = 0;
				pkt->AppendWord(obj->unit_id);
				pkt->param = obj->unit_id;
				g_NetStru1_local.QueuePacketSend(pkt);
			}
		}
	}
	if (wnd->vis_spellbook->sub_4CA8E0(idx - 1) != 0) {
		POSITION it = this->field_0x9d0.GetStartPosition();
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->field_0x9d0.GetNextAssoc(it, key, obj);
			if (!obj->IsSelected() || (obj->activeSpellEffectMask & (1 << (idx - 1))) == 0) {
				continue;
			}
			pkt->count = 0;
			pkt->AppendWord(obj->unit_id);
			uint16_t slot = 0xFFFF;
			if (wnd->vis_spellbook->spell >= 0) {
				slot = wnd->vis_spellbook->spell;
			} else {
				for (int32_t i = 0; i < obj->tokenEntries.GetSize(); i++) {
					TokenEntry* entry = obj->tokenEntries[i];
					if (entry != nullptr && (entry->flg & 0x10) != 0 && entry->GetCastSpellId() == BOOK_POS_TO_SPELL_ID[idx]) {
						slot = i;
						break;
					}
				}
			}
			pkt->slot = slot;
			pkt->slot_high = slot >> 8;
			if (slot == 0xFFFF) {
				continue;
			}
			pkt->id = 0x25;
			g_NetStru1_local.QueuePacketSend(pkt);
			TokenEntry* entry = obj->tokenEntries[slot];
			if (entry->count == 1) {
				obj->tokenEntries.RemoveAt(slot, 1);
				delete entry;
			} else {
				entry->count--;
			}
			wnd->vis_spellbook->spell = -1;
			wnd->vis_spellbook->selected = -1;
		}
	}
	this->UpdateSelectionState();
}

// 41ae1c
void BigStruct2::FUN_0041ae1c(int32_t serv_id) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->sessionMode == 2) {
		CWordArray ids;
		for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
			uint16_t key;
			CGameObject* obj;
			this->field_0x9d0.GetNextAssoc(it, key, obj);
			if (obj->IsKindOf(RUNTIME_CLASS(CUnit)) == 0 || (((CUnit*)obj)->unitFlags & 1) != 0) {
				continue;
			}
			ids.Add(key);
		}
		while (ids.GetSize() != 0) {
			CGameObject* obj = nullptr;
			this->field_0x9d0.Lookup(ids[0], obj);
			this->field_0x9d0.RemoveKey(ids[0]);
			if (obj != nullptr) {
				delete obj;
			}
			ids.RemoveAt(0, 1);
		}
	} else {
		Packet2Dwords* pkt = &Packet2Dwords::Inst;
		pkt->from_player_id = this->my_main_unit->index;
		pkt->to_player_id = 0;
		pkt->id = 0x3A;
		pkt->param1 = this->GetUnit_3f6c()->unit_id;
		pkt->param2 = serv_id;
		g_NetStru1_local.QueuePacketSend(pkt);
	}
}

// 41ace2
void BigStruct2::FUN_0041ace2(int32_t spell_id) {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsSelected() && (obj->spells & (1 << spell_id)) != 0) {
			pkt->id = 0x46;
			pkt->from_player_id = this->my_main_unit->index;
			pkt->to_player_id = 0;
			pkt->param1 = 4;
			pkt->param2 = key | (spell_id << 16);
			g_NetStru1_local.QueuePacketSend(pkt);
			obj->active_spell = spell_id;
		}
	}
}

// 41e2af
int32_t BigStruct2::FUN_0041e2af(int32_t id, CUnit** out) {
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->IsKindOf(RUNTIME_CLASS(CUnit)) == 0) {
			continue;
		}
		*out = (CUnit*)obj;
		if ((*out)->questFlags == id) {
			return 1;
		}
	}
	return 0;
}

// 40c0a8
uint32_t BigStruct2::sub_40C0A8(int32_t x, int32_t y) {
	for (POSITION it = this->field_0x9d0.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->field_0x9d0.GetNextAssoc(it, key, obj);
		if (obj->bIsBlocked != 0 || obj->tileX != x || obj->tileY != y) {
			continue;
		}
		return key;
	}
	return 0;
}

// 41cda3
void BigStruct2::FUN_0041cda3(const char* mapname) {
	PacketJoin* pkt = &PacketJoin::Inst;
	pkt->id = 0xAE;
	if (this->my_main_unit != nullptr) {
		pkt->from_player_id = this->my_main_unit->index;
	}
	pkt->to_player_id = 0;
	strcpy(pkt->name, mapname);
	g_NetStru1_main.QueuePacketSend(pkt);
	g_NetStru1_main.SendAllData();
}

// 41a942
void BigStruct2::sub_41A942() {
	PacketWord* pkt = &PacketWord::Inst;
	pkt->id = 0x33;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->value = this->field_0x138->unit_id;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41dfa6
CUnit* BigStruct2::FUN_0041dfa6(int32_t serv_id) {
	CUnit* unit = this->FUN_0041df23(serv_id);
	if (unit == nullptr) {
		unit = this->sub_41DE80(serv_id);
	}
	return unit;
}

// 41a9f6
void BigStruct2::sub_41A9F6() {
	PacketWord* pkt = &PacketWord::Inst;
	pkt->id = 0x35;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->value = this->field_0x138->unit_id;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41a99c
void BigStruct2::sub_41A99C() {
	PacketWord* pkt = &PacketWord::Inst;
	pkt->id = 0x34;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->value = this->field_0x138->unit_id;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41de80
CUnit* BigStruct2::sub_41DE80(int32_t serv_id) {
	CUnit* unit = new CUnit();
	unit->unitFlags = 0x56;
	unit->typeId = 1;
	unit->face = serv_id - 0x50;
	unit->serverId = (uint16_t)serv_id;
	return unit;
}
