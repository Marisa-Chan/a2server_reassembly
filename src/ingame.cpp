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
#include "world.h"
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

int g_ActionDirDx[16] = { 0, 0, 1, 0,   1, 0, 1, 0,   0, 0, -1, 0,   -1, 0, -1, 0 };
int g_ActionDirDy[16] = { -1, 0, -1, 0,   0, 0, 1, 0,   1, 0, 1, 0,   0, 0, -1, 0 };

// 62f908 - is the spell offensive, based on the spellbook position.
int32_t g_SpellOffensive[24] = {1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1};

CArray<UnitVFXUnfo*> g_VFX_info; //6610e0
CArray<GfxFile*> g_GfxFiles; //661100
CArray<GfxObject*> g_GfxObjects; //665330
CArray<StructureInfo*> g_StructuresInfo; //661098
CArray<ProjectileInfo*> g_ProjectileInfos; //6610b0
CArray<UnitGfxFile*> g_UnitGfxFiles; //665118

CGamePalette* g_pal_projectiles; //665490

int32_t g_TerrainHidden = 0; //660f68
int32_t g_ShowFps = 0; //660f6c
int32_t g_GameStatusCode = 0; //660f8c
int32_t g_JoinState = 0; //660f90
int32_t g_ShowNetStats = 0; //660f70
CGamePalette* g_pal_projectile_; //665494

CA16* g_spr_smoke[2]; //6610f8

CUnit g_CUnitStatic; //642a48
CStringArray g_WwwServerList; // 666a00



GM_a28::~GM_a28() = default; //4020ad

GM_a28::GM_a28() {
	//401f50
	this->elapsed_time = 0;
	this->dirty = 1;
}

void GM_a28::Add(const char* msg, uint16_t* clr, uint32_t t) {
	//4021fe
	this->dirty = 1;

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	
	CGameFont* fnt = g_font1;
	if (wnd->sessionMode == SESSION_SERVER) {
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
	this->dirty = 1;

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

	if (wnd->sessionMode == SESSION_SERVER) {
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

	if (wnd->sessionMode == SESSION_SERVER) {
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


#ifdef A2CLIENT
// client: 41748d-4175da (packet 0x5c) / 417902-417a1b (packet 0xc3) — close shop/tavern/
// popup/dialog-1200 overlays before showing a hat DM window
void BigStruct2::CloseHatOverlays() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if ((wnd->dialogsMask & DIALOG_MASK_SHOP) != 0) {
		CVisualObject* shop = wnd->vis_root->FindChild(1000);
		if (shop != nullptr) {
			((VisShop*)shop)->CloseShopDialog();
			wnd->vis_root->RemoveChild(shop);
		}
	}
	if ((wnd->dialogsMask & DIALOG_MASK_TAVERN) != 0) {
		CVisualObject* tav = wnd->vis_root->FindChild(1100);
		if (tav != nullptr) {
			((VisTav*)tav)->CloseDialog();
			wnd->vis_root->RemoveChild(tav);
		}
	}
	if ((wnd->dialogsMask & DIALOG_MASK_POPUP) != 0) {
		CVisualObject* child = wnd->vis_root->FindChild(1);
		if (child != nullptr) {
			wnd->vis_root->RemoveChild(child);
			delete child;
		}
	}
	CVisualObject* child1200 = this->FindChild(MAP_CHILD_CHAT_LINE);
	if (child1200 != nullptr) {
		// TODO(client): the vanilla also calls the child's vtable+0x84 with 0x445 first
		this->RemoveChild(child1200);
	}
}
#endif

// 40da14
// client: 40dac9
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
			if (breakid != 0 || (wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
				continue;
			}
			return 0;
		}

		//printf("Client receive packet 0x%x\n", pkt->id);

		switch (pkt->id) {
		case MISSION_ENTERED:
			break;

		case START_GAME:
			if (!this->scenario) {
				this->LoadScenario(packet_join->name);
			}

			if (!this->scenario) {
				g_GameStatusCode = SESSION_ERROR_NO_MAP_FILE;
				return 0;
			}
			g_JoinState = 0;
			break;

		case JOIN_ERROR: {
			g_GameStatusCode = packet_2dwords->param1 | SESSION_ERROR_FLAG;

			if ((wnd->dialogsMask & DIALOG_MASK_INGAME) == 0) {
				while( g_NetStru1_local.ReceiveAnyPacket() != nullptr)
				{}
				g_CLlDriver.Close();
				return 0;
			}

			wnd->PostMessage(MAIN_MSG_LEAVE_TO_NETWORK_MENU, 0, 0);
			g_CLlDriver.Close();
			break;
		}

		case JOIN_DONE: {
			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{
			}
			return 1;
		} break;

#ifdef A2CLIENT
		case GAME_SPEED:
			// client: 417b81 — server-driven game speed change
			if (wnd->SetSpeed(packet_dword->value) != 0) {
				this->msglog.AddWithCheck(txt_patch.GetLine(wnd->game_speed + 0x6c), clr_log_sblack, 5000);
			}
			g_ServerConfig.game_speed = wnd->game_speed;
			break;

		case HAT_SCORE_CLEAR:
			// client: 4177e6 — clear the hat DM name/score lists
			this->hat_name_list.RemoveAll();
			this->hat_score_list.RemoveAll();
			break;

		case HAT_SCORE_UPDATE: {
			// client: 4176c7 — update one hat DM name/score pair
			CString name((char*)packet_data->data + 2);
			int i = 0;
			for (; i < this->hat_name_list.GetSize(); i++) {
				if (this->hat_name_list[i] == name) {
					this->hat_score_list[i] = packet_dword->value;
					break;
				}
			}
			if (i == this->hat_name_list.GetSize()) {
				this->hat_name_list.Add(name);
				this->hat_score_list.Add(packet_dword->value);
			}
			// client: 41ffe6 — sort pairs descending by score, keeping names in sync
			for (int j = 0; j < this->hat_score_list.GetSize(); j++) {
				for (int k = j + 1; k < this->hat_score_list.GetSize(); k++) {
					if (this->hat_score_list[j] < this->hat_score_list[k]) {
						uint32_t tmp_score = this->hat_score_list[j];
						this->hat_score_list[j] = this->hat_score_list[k];
						this->hat_score_list[k] = tmp_score;
						CString tmp_name = this->hat_name_list[j];
						this->hat_name_list[j] = this->hat_name_list[k];
						this->hat_name_list[k] = tmp_name;
					}
				}
			}
		} break;

		case HAT_SCORE_LIST:
			// client: 41745c — show the hat DM player list / close overlays
			this->hat_player_count = packet_dword->value;
			if (packet_2dwords->param2 != 0) {
				this->CloseHatOverlays();
				// client: 417624-4176a9
				HatPlayerListWindow* dlg = new HatPlayerListWindow(1, 0x14, 0, 0x26c, 0x1f4, &this->hat_name_list, packet_2dwords->param2);
				wnd->PopUpScreen(dlg);
			}
			this->ambient_view_x = 1;
			break;
#endif

		case SERVER_TICK:
			wnd->serverLoopCounter = packet_dword->value;
			break;

		case MONEY_UPDATE: {
			if (wnd->dialogsMask == DIALOG_MASK_INGAME && this->my_main_unit->gold < packet_2dwords->param1 && packet_2dwords->param2 == 0) {
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

		case UNIT_SHOWN:
		case UNIT_HIDDEN: {
			CUnit* unit;
			if (this->objects.Lookup(packet_word->value, *(CGameObject**)&unit)) {
				if (pkt->id == UNIT_HIDDEN) {
					unit->unitFlags |= UNIT_FLAG_HIDDEN;
				} else {
					unit->unitFlags &= ~UNIT_FLAG_HIDDEN;
				}
			}
		}
			break;
			

		case TOKEN_REMOVE: {
			CUnit* unit;
			if (this->objects.Lookup(packet_word->value, *(CGameObject**)&unit)) {
				this->objects.RemoveKey(packet_word->value);
				delete unit;
			}
		}
			break;

		case UNIT_MOVE: {
			CUnit* unit;
			if (!this->objects.Lookup(packet_action->unit_id, *(CGameObject**)&unit)) {
				sprintf(buf, "Invalid unit #%d. Command Move.", packet_action->unit_id);
				if (g_EnableTrace) {
					this->msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			} else {
				if (unit->action_segments) {
					switch (unit->action) {
					case ACTION_MOVE:
						sprintf(buf, "Overriding \'Move\' by \'Move\'. %d segments lost.", unit->action_segments);
						break;

					case ACTION_ATTACK:
						sprintf(buf, "Overriding \'Attack\' by \'Move\'. %d segments lost.", unit->action_segments);
						break;

					case ACTION_TURN:
						buf[0] = 0;
						break;

					case ACTION_SHOOT:
						sprintf(buf, "Overriding \'Shoot\' by \'Move\'. %d segments lost.", unit->action_segments);
						break;

					case ACTION_CAST:
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
					unit->action = ACTION_MOVE;
					unit->action_dir = packet_action->angle;
					unit->action_x = g_ActionDirDx[unit->action_dir] << 8;
					unit->action_y = g_ActionDirDy[unit->action_dir] << 8;
				}
			}
		}
			break;

		case UNIT_CHANGE_4:
		case UNIT_CHANGE_1:
		case UNIT_CHANGE_2:
		case UNIT_CHANGE_3: {

			if (packet_unit->unit_id == 0) {
				if (g_EnableTrace) {
					sprintf(buf, "Invalid index id=%d", packet_unit->unit_id);
					this->msglog.Add(buf, clrsh_TechBlack, 30000);
				}
			} else {
				uint8_t* pdata = packet_unit->data;
				if ((packet_unit->flags_mask & UNIT_UPD_HP) != 0) {
					g_CUnitStatic.hp = *(uint16_t*)pdata;
					pdata += 2;
				}
				if ((packet_unit->flags_mask & UNIT_UPD_MP) != 0) {
					g_CUnitStatic.mp = *(uint16_t*)pdata;
					pdata += 2;
				}
				if ((packet_unit->flags_mask & UNIT_UPD_SKILL_LEVELS) != 0) {
					g_CUnitStatic.skill_levels[0] = pdata[0];
					g_CUnitStatic.skill_levels[1] = pdata[1];
					g_CUnitStatic.skill_levels[2] = pdata[2];
					g_CUnitStatic.skill_levels[3] = pdata[3];
					g_CUnitStatic.skill_levels[4] = pdata[4];
					pdata += 5;
				}
				if ((packet_unit->flags_mask & UNIT_UPD_HP_MP_MAX) != 0) {
					g_CUnitStatic.hp_max = *(int16_t*)pdata;
					pdata += 2;
					g_CUnitStatic.mp_max = *(int16_t*)pdata;
					pdata += 2;
				}
				if ((packet_unit->flags_mask & UNIT_UPD_DIR_POS) != 0) {
					g_CUnitStatic.last_action = *pdata;
					pdata ++;
					g_CUnitStatic.x_pos = *pdata;
					pdata ++;
					g_CUnitStatic.y_pos = *pdata;
					pdata ++;
				}
				if ((packet_unit->flags_mask & UNIT_UPD_TYPE_FACE) != 0) {
					g_CUnitStatic.typeId = *pdata;
					pdata ++;
					g_CUnitStatic.face = *pdata;
					pdata ++;
				}

				uint8_t ownerid = 0;
				if ((packet_unit->flags_mask & UNIT_UPD_OWNER) != 0) {
					ownerid = *pdata;
					pdata ++;
				}

				if ((packet_unit->flags_mask & UNIT_UPD_SERVER_ID) != 0) {
					g_CUnitStatic.serverId = *(uint16_t*)pdata;
					pdata += 2;
				}

				if (ownerid >= this->map_players.GetSize()) {
					if (g_EnableTrace != 0) {
						sprintf(buf, "Invalid owner. Unit(ext) id=%d", packet_unit->unit_id);
						this->msglog.Add(buf, clrsh_TechBlack, 30000);
					}
				} else {
					MapPlayerData* mpl = this->map_players[ownerid];
					if (!mpl) {
						if (g_EnableTrace != 0) {
							sprintf(buf, "Invalid owner. Unit(ext) id=%d", packet_unit->unit_id);
							this->msglog.Add(buf, clrsh_TechBlack, 30000);
						}
					} else {
						bool groundunit = false;
						Unit* uni = nullptr;

						if (packet_unit->flags_mask & UNIT_UPD_TYPE_FACE) {
							if (g_CUnitStatic.typeId >= 0x20 && g_CUnitStatic.typeId < 0x40) {
								groundunit = true;
							}
						} else if (packet_unit->flags_mask & UNIT_UPD_SERVER_ID) {
							uni = new Unit();
							uni->itemDataID = g_GameDataRes.FindMonsterByServerId(g_CUnitStatic.serverId);
							uni->monster_info = &g_GameDataRes.monsters[uni->itemDataID];
							uni->ApplyMonsterInfo(uni->monster_info->Values());
							uni->server_id = g_CUnitStatic.serverId;

							if (wnd->sessionMode == SESSION_LOCAL_BATTLE && g_Server && g_Server->difficulty != DIFFICULTY_NORMAL) {
								if (g_Server->difficulty == DIFFICULTY_EASY) {
									uni->hp_max = uni->hp_max / 2.0;
								} else {
									uni->hit_values.attack += 50;
									uni->protections.defense += 50;
									uni->hp_max = uni->hp_max * 1.5;
								}
								uni->hp = uni->hp_max;
							}

							if (packet_unit->flags_mask & UNIT_UPD_HP_MP_MAX) {
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

						if (this->objects.Lookup(packet_unit->unit_id, *(CGameObject**)&ct) == 0) {
							ct_exist = false;

							if ((packet_unit->flags_mask & (UNIT_UPD_SERVER_ID | UNIT_UPD_TYPE_FACE)) == 0) {
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

							this->objects.SetAt(packet_unit->unit_id, ct);
							ct->unit_id = packet_unit->unit_id;
						}

						if (packet_unit->flags_mask & UNIT_UPD_OWNER) {
							ct->map_player = mpl;
						}

						if (packet_unit->flags_mask & UNIT_UPD_HP) {
							ct->hp = g_CUnitStatic.hp;
						}

						if (packet_unit->flags_mask & UNIT_UPD_MP) {
							ct->mp = g_CUnitStatic.mp;
						}

						if (packet_unit->flags_mask & UNIT_UPD_SKILL_LEVELS) {
							ct->skill_levels[0] = g_CUnitStatic.skill_levels[0];
							ct->skill_levels[1] = g_CUnitStatic.skill_levels[1];
							ct->skill_levels[2] = g_CUnitStatic.skill_levels[2];
							ct->skill_levels[3] = g_CUnitStatic.skill_levels[3];
							ct->skill_levels[4] = g_CUnitStatic.skill_levels[4];
						}

						if (packet_unit->flags_mask & UNIT_UPD_HP_MP_MAX) {
							ct->hp_max = g_CUnitStatic.hp_max;
							ct->mp_max = g_CUnitStatic.mp_max;
						}

						uint8_t sval = ct->death_state;

						if (packet_unit->flags_mask & UNIT_UPD_DIR_POS) {
							ct->dir = g_CUnitStatic.last_action;
							ct->x_pos = g_CUnitStatic.x_pos;
							ct->y_pos = g_CUnitStatic.y_pos;
							ct->x_pos = ct->x_pos * 0x100 + 0x80;
							ct->y_pos = ct->y_pos * 0x100 + 0x80;
							ct->x_pos2 = ct->x_pos;
							ct->y_pos2 = ct->y_pos;
							ct->z_pos = 0;
						}

						if (packet_unit->flags_mask & UNIT_UPD_SERVER_ID) {
							ct->serverId = g_CUnitStatic.serverId;
						}

						if (packet_unit->flags_mask & UNIT_UPD_ATTACK) {
							ct->attack = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & UNIT_UPD_HAND_DAMAGE) {
							ct->hand_dmg_min = pdata[0];
							ct->hand_dmg_spread = pdata[1];
							pdata += 2;
						}

						if (packet_unit->flags_mask & UNIT_UPD_DEFENSE) {
							ct->defence = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & UNIT_UPD_ABSORB) {
							ct->absorb = *pdata;
							pdata++;
						}

						if (packet_unit->flags_mask & UNIT_UPD_SCAN_RANGE) {
							ct->scan_range = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & UNIT_UPD_CARRIED_WEIGHT) {
							ct->carrying_weight_100g = *(uint16_t*)pdata;
							pdata += 2;
						}

						if (packet_unit->flags_mask & UNIT_UPD_MAGIC_PROTECT) {
							ct->magic_protect[0] = pdata[0];
							ct->magic_protect[1] = pdata[1];
							ct->magic_protect[2] = pdata[2];
							ct->magic_protect[3] = pdata[3];
							ct->magic_protect[4] = pdata[4];
							pdata += 5;
						}

						if (packet_unit->flags_mask & UNIT_UPD_KILL_STATS) {
							wnd->m_GameSession.monster_killed = *(uint32_t*)(pdata);
							wnd->m_GameSession.player_killed = *(uint32_t*)(pdata + 4);
							wnd->m_GameSession.death_count = *(uint32_t*)(pdata + 8);
							wnd->m_GameSession.fragCount = *(uint32_t*)(pdata + 12);

							pdata += 16;
						}

						if (packet_unit->flags_mask & UNIT_UPD_SPEED) {
							ct->speed = *pdata;
							pdata++;
						}

						if (packet_unit->flags_mask & UNIT_UPD_SPELLS) {
							ct->spells = *(int32_t*)pdata;
							pdata += 4;
						}

						if (packet_unit->flags_mask & UNIT_UPD_SPELLS) {
							ct->active_spell = *pdata;
							pdata++;
						}

						if (packet_unit->flags_mask & UNIT_UPD_EXP_SPHERE0) {
							ct->experience_per_sphere[0] = *(int32_t*)pdata;
							pdata += 4;
						}

						if (packet_unit->flags_mask & UNIT_UPD_EXP_SPHERE1) {
							ct->experience_per_sphere[1] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & UNIT_UPD_EXP_SPHERE2) {
							ct->experience_per_sphere[2] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & UNIT_UPD_EXP_SPHERE3) {
							ct->experience_per_sphere[3] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & UNIT_UPD_EXP_SPHERE4) {
							ct->experience_per_sphere[4] = *(int32_t*)pdata;
							pdata += 4;
						}
						if (packet_unit->flags_mask & UNIT_UPD_BASE_STATS) {
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

						if ((packet_unit->flags_mask & UNIT_UPD_DIR_POS) != 0 && ct->unit_id > 0x5fff) {
							ct->unitFlags &= ~UNIT_FLAG_HIDDEN;
						}

						if (packet_unit->flags_mask & UNIT_UPD_NAME) {
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

						if (wnd->sessionMode == SESSION_LOCAL_BATTLE) {
							if (ct->serverId == 0x15) {
								this->my_unit = ct;
								this->my_unit->unitFlags |= UNIT_FLAG_TOKEN_GOLD_ENTRY;
							}
						} else if (ct->map_player == this->my_main_unit && (ct->unitFlags & UNIT_FLAG_PLAYER_CLASS) != 0) {
							this->my_unit = ct;
							this->my_unit->unitFlags |= UNIT_FLAG_TOKEN_GOLD_ENTRY;
						}

						if (ct->str1[0] == 0 && ct->serverId != 0) {
							strcpy(ct->str1, txt_npcnames.GetLine(ct->serverId - 1));
						}

						if (packet_unit->flags_mask & UNIT_UPD_EFFECTS) {
							ct->effect_timers.RemoveAll();
						}

						if (ct_exist && (wnd->dialogsMask & DIALOG_MASK_CHARGEN) != 0) {
							if ((packet_unit->flags_mask & UNIT_UPD_TYPE_FACE) != 0) {
								ct->face = g_CUnitStatic.face;
							}

							this->my_unit->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
						}

						if ((packet_unit->flags_mask & UNIT_UPD_SPELLS) != 0) {
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

						if (wnd->sessionMode != SESSION_LOCAL_BATTLE && (ct->unitFlags & UNIT_FLAG_TOKEN_GOLD_ENTRY) != 0) {
							if ((packet_unit->flags_mask & (UNIT_UPD_BASE_STATS | UNIT_UPD_KILL_STATS | UNIT_UPD_MAGIC_PROTECT)) == 0) {
								if ((packet_unit->flags_mask & (UNIT_UPD_EXP_SPHERE0 | UNIT_UPD_EXP_SPHERE1 | UNIT_UPD_EXP_SPHERE2 | UNIT_UPD_EXP_SPHERE3 | UNIT_UPD_EXP_SPHERE4)) != 0) {
									wnd->m_GameSession.AutosaveTick();
								}
							} else {
								wnd->m_GameSession.AutosaveCharacter();
							}
						}

						switch (ct->death_state) {
						case 0:
							if (sval != 0) {
								ct->action = ACTION_NONE;
								if (ct->unitFlags & UNIT_FLAG_PLAYER_CLASS) {
									ct->ReloadSprite();
								}
							}
							break;

						case 1:
							if (sval == 0) {
								UnitVFXUnfo* dying = g_VFX_info[ g_VFX_info[ct->typeId]->dying ];
								ct->action_segments = dying->dying_phases * 2;
								ct->action = ACTION_DIE;
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
								ct->action = ACTION_DIE;
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
							if (wnd->sessionMode != SESSION_LOCAL_BATTLE && ct == this->my_unit && ct->death_state == 4 && sval < 4) {
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
								this->objects.RemoveKey(packet_unit->unit_id);
								if (ct) {
									delete ct;
								}
							}
							break;
						}
						if (g_CUnitStatic.death_state < 5) {
							if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0 && this->scenario) {
								ct->UpdateMapPosition();
							}

							if (this->primary_selected_object == ct) {
								wnd->vis_charinfo->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
								wnd->vis_sidestatus->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
							}

							if (ct->IsSelected() != 0 && ((ct->unitFlags & UNIT_FLAG_MAGE) != 0 || (packet_unit->flags_mask & UNIT_UPD_SPELLS) != 0)) {
								this->UpdateSelectionState();
							}

							ct->m_bSelectionDirty = 1;
						}
					}
				}
			}
		}
			break;

		case UNIT_TURN: {
			CUnit* ct = nullptr;
			if (this->objects.Lookup(packet_action->unit_id, *(CGameObject**)&ct) == 0) {
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
					ct->action = ACTION_TURN;
					ct->action_dir = packet_action->angle;
					ct->turn_angle = ct->dir * 16;
					ct->action_phase = 0;
					ct->move_accum_y = 0;
					ct->move_accum_x = 0;
				}
			}
		}
			break;

		case UNIT_ATTACK: {
			CUnit* ct = nullptr;
			if (this->objects.Lookup(packet_action->unit_id, *(CGameObject**)&ct) == 0) {
				if (g_EnableTrace != 0) {
					sprintf(buf, "Invalid unit #%d. Command Attack.", packet_action->unit_id);
					this->msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			} else {
				if (ct->action_segments == 0) {
					UnitVFXUnfo* vfx = g_VFX_info[ct->typeId];
					if (vfx->attack_phases != 0 && ct->hp > 0) {
						ct->action_segments = vfx->attack_anim_frame_cnt;
						ct->action = ACTION_ATTACK;
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

		case RANGED_ATTACK: {
			CUnit* ct = nullptr;
			if (this->objects.Lookup(packet_ranged_attack->unit_id, *(CGameObject**)&ct) == 0) {
				if (g_EnableTrace) {
					sprintf(buf, "Invalid unit #%d. Command Ranged Attack.", packet_ranged_attack->unit_id);
					this->msglog.Add(buf, clrsh_TechBlack, 5000);
				}
			} else if (ct->action_segments == 0 && ct->hp > 0) {
				ct->action_segments = g_VFX_info[ct->typeId]->attack_anim_frame_cnt;
				ct->action = ACTION_SHOOT;
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

		case UNIT_DAMAGED: {
			CUnit* ct = nullptr;
			if (this->objects.Lookup(packet_unit_hp->unit_id, *(CGameObject**)&ct) == 0) {
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
				if (this->primary_selected_object == ct) {
					wnd->vis_charinfo->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
					wnd->vis_sidestatus->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
				}
				ct->m_bSelectionDirty = 1;
			}
		}
			break;
		

		case ITEM_LIST: {
			if (wnd->drag_entry && packet_state->mode == wnd->drag_src_tab) {
				g_Cursors[CURSOR_DEFAULT]->Use();
				if (wnd->drag_entry) {
					delete wnd->drag_entry;
				}
				wnd->ResetItemCursor();
			}
			CUnit* ct = nullptr;
			switch (packet_state->mode & 0x7f) {
			case ITEM_LIST_EQUIPMENT:
				if (this->objects.Lookup(packet_state->building_id, *(CGameObject**)&ct) != 0) {
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

					ct->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;

					if ((wnd->dialogsMask & DIALOG_MASK_SHOP) != 0) {
						wnd->vis_root->FindChild(1000)->MsgProc(VIS_MSG_REFRESH_ITEM_GRIDS, packet_state->mode & 0x7f, 0);
					}

					ct->ReloadSprite();

					ct->m_bSelectionDirty = 1;

					if ((ct->unitFlags & UNIT_FLAG_MAGE) != 0 && wnd->sessionMode != SESSION_LOCAL_BATTLE) {
						uint8_t* pdata = packet_state->data;
						for (int i = 0; i < 12; i++) {
							if ((packet_state->start_index & (1 << i)) != 0) {
								if (this->hero_equipment[i]) {
									delete this->hero_equipment[i];
								}

								this->hero_equipment[i] = ReadItem(&pdata);
							}
						}

						packet_state->data_size = 0;

						for (int i = 0; i < 12; i++) {
							if (!this->hero_equipment[i]) {
								Item item;
								item.StoreToPacket(packet_state, 0);
							} else {
								this->hero_equipment[i]->StoreToPacket(packet_state, 0);
							}
						}

						CString fname = wnd->m_GameSession.GetSelectedRosterPath();
						WritePlayerFile(fname, nullptr, nullptr, nullptr, packet_state, nullptr, nullptr, 0);
					}
				}
				break;

			case ITEM_LIST_INVENTORY: {
				if (this->objects.Lookup(packet_state->building_id, *(CGameObject**)&ct) != 0) {
					if ((packet_state->mode & ITEM_LIST_PARTIAL_UPDATE) == 0) {
						ct->tokenEntries.RemoveAll();
					} else {
						int num = ct->tokenEntries.GetSize();

						if ((ct->unitFlags & UNIT_FLAG_TOKEN_GOLD_ENTRY) != 0) {
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

						if ((ct->unitFlags & UNIT_FLAG_TOKEN_GOLD_ENTRY) != 0) {
							ct->tokenEntries.RemoveAt(ct->tokenEntries.GetUpperBound());
						}
					}

					uint8_t* pdata = packet_state->data;
					for (int i = 0; i < packet_state->entry_count; i++) {
						TokenEntry* obj = new TokenEntry(&pdata, 0);
						obj->slot_code = 2;

						if (obj->HasKnownName()) {
							ct->tokenEntries.InsertAt(i + packet_state->start_index, obj, 1);
						} else if (g_EnableTrace) {
							// client 40dac9: only items without a known name are
							// reported as invalid (mode 1 does the same).
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
							g_ItemNameMap.Lookup(obj->item_id, *(void **)&itmname);

							if (obj->count < 2) {
								sprintf(buf, "%s %s", TxtFile::AllLines[85], itmname);
							} else {
								sprintf(buf, "%s %s (%s %d %s)", TxtFile::AllLines[85], itmname, TxtFile::AllLines[86], obj->count, TxtFile::AllLines[87]);
							}
							this->msglog.Add(buf, clr_log_tblack, 3000);
						}
					}

					if ((ct->unitFlags & UNIT_FLAG_TOKEN_GOLD_ENTRY) != 0) {
						if (wnd->sessionMode != SESSION_LOCAL_BATTLE) {
							uint8_t *pdata = packet_state->data;
							for (int i = 0; i < packet_state->entry_count; i++) {
								if (i + packet_state->start_index < this->hero_inventory.GetSize()) {
									Item* itm = this->hero_inventory[i + packet_state->start_index];
									if (itm) {
										delete itm;
									}
								}

								Item * itm = ReadItem(&pdata);
								this->hero_inventory.SetAtGrow(i + packet_state->start_index, itm);
							}

							if ((packet_state->mode & ITEM_LIST_PARTIAL_UPDATE) == 0) {
								this->hero_inventory.SetSize(packet_state->entry_count);
							}
							
							packet_state->data_size = 0;

							for (int i = 0; i < this->hero_inventory.GetSize(); i++) {
								this->hero_inventory[i]->StoreToPacket(packet_state, 0);
							}


							CString fname = wnd->m_GameSession.GetSelectedRosterPath();
							WritePlayerFile(fname, nullptr, nullptr, nullptr, packet_state, nullptr, nullptr, 0);
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

					if (wnd->dialogsMask & DIALOG_MASK_SHOP) {
						wnd->vis_root->FindChild(1000)->MsgProc(VIS_MSG_REFRESH_ITEM_GRIDS, packet_state->mode & 0x7f, 0);
					} else {
						if (this->selected_object_count == 1 && this->primary_selected_object == ct) {
							wnd->vis_invtype1->VMethod33(ct);
						}
					}
					
				}
			}
				break;

			case ITEM_LIST_SHOP_TABLE:
			case ITEM_LIST_SHELF_1:
			case ITEM_LIST_SHELF_2:
			case ITEM_LIST_SHELF_3:
			case ITEM_LIST_SHELF_4: {
				CArray<TokenEntry*> tmp;

				uint8_t* pdata = packet_state->data;

				for (int i = 0; i < packet_state->entry_count; i++) {
					TokenEntry* d0 = nullptr;
					if ((packet_state->mode & 0x7f) == ITEM_LIST_SHOP_TABLE) {
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

				if ((wnd->dialogsMask & DIALOG_MASK_SHOP) == 0) {
					for (int i = 0; i < tmp.GetSize(); i++) {
						if (tmp[i]) {
							delete tmp[i];
						}
					}
					tmp.RemoveAll();
				} else {
					wnd->vis_root->FindChild(1000)->MsgProc(VIS_MSG_REFRESH_ITEM_GRIDS, packet_state->mode & 0x7f, (uint32_t)&tmp);
				}
			}
				break;

			case ITEM_LIST_INN_REWARDS: {
				if ((wnd->dialogsMask & DIALOG_MASK_TAVERN) != 0) {
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


		case SACK_STATE: {
			CGameObject* ct = nullptr;
			if (this->objects.Lookup(packet_sack->unit_id, ct)) {
				ct->typeId = packet_sack->type_id;
				if (ct->typeId > 5) {
					ct->typeId = 5;
				}
			} else {
				CBackPack* pak = new CBackPack();
				this->objects[packet_sack->unit_id] = pak;

				pak->SetVals(packet_sack->unit_id, 1, packet_sack->xpos, packet_sack->ypos, 0, 0, 0, 0, 1);

				pak->map_player = this->my_main_unit;
				pak->typeId = packet_sack->type_id;
				if (pak->typeId > 5) {
					pak->typeId = 5;
				}

				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0 && this->scenario != nullptr) {
					pak->UpdateMapPosition();
				}
			}

		}
			break;

		case BUILDING_HP: {
			CUnit* ct = nullptr;
			if (this->objects.Lookup(packet_building_hp->building_id, *(CGameObject**)&ct)) {
				ct->hp = packet_building_hp->hp;
				if (this->primary_selected_object == ct) {
					wnd->vis_charinfo->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
					wnd->vis_sidestatus->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
				}
			}
		}
			break;

		case OPEN_SHOP:
			wnd->PostMessage(MAIN_MSG_SHOW_SHOP, packet_2dwords->param1, 0);
			break;

		case OPEN_INN:
			wnd->PostMessage(MAIN_MSG_SHOW_INN, packet_2dwords->param1, packet_2dwords->param2);
			break;

		case EFFECT_MOVE:
			// client 40dac9: the whole move-broadcast handler is gated on the
			// join being complete (scenario).
			if (this->scenario != 0) {
			if ((packet_move->type_id & 1) == 0 || packet_move->type_id >= g_ProjectileInfos.GetSize() || g_ProjectileInfos[packet_move->type_id] == nullptr) {
				CUnit* ct = nullptr;
				if (this->objects.Lookup(packet_move->unit_id, *(CGameObject**)&ct) == 0) {
					if (g_EnableTrace != 0) {
						sprintf(buf, "Invalid unit #%d. Command Cast spell.", packet_move->unit_id);
						this->msglog.Add(buf, clrsh_TechBlack, 5000);
					}
				} else if (ct->action_segments == 0 && g_VFX_info[ct->typeId]->attack_phases != 0) {
					ct->action_segments = g_VFX_info[ct->typeId]->attack_anim_frame_cnt;
					ct->action = ACTION_CAST;
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
				pj->action = ACTION_MOVE;
				pj->pMapObject = this;

				pj->UpdateMapPosition();

				this->projectiles[this->next_projectile_id] = pj;
				this->next_projectile_id++;

				SfxSample* snd = g_SfxArray[packet_move->type_id + 500];
				if (packet_move->type_id != 23 && snd) {
					int32_t vol = 0;
					int32_t pan = 0;
					this->ComputeSoundVolumePan(pj->x_pos, pj->y_pos, &vol, &pan);

					int priority = (10000 - ::abs(vol)) / 100;

				snd->Play(vol + g_SoundSettings.sfx_pos, pan, 0, priority, 0);
				}
			}
			}
			break;

		case AOE_ZONE:
			// client 40dac9: gated on field_0x80.
			if (this->scenario != 0) {
			this->ApplyTileLightMask(packet_aoe->x,
							packet_aoe->y,
							packet_aoe->width,
							packet_aoe->height,
							(packet_aoe->type_id / 2) - 4,
							packet_aoe->data,
							packet_aoe->set_flag);
			if (packet_aoe->set_flag == 0) {
				this->full_redraw_requested = 1;
			}
			}
			break;

		case EFFECT_HANG: {
			CGameObject* ct = nullptr;
			if (this->objects.Lookup(packet_effect->unit_id, ct) != 0) {
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

		case EFFECT_GONE: {
			CGameObject* ct = nullptr;
			if (this->objects.Lookup(packet_effect->unit_id, ct) != 0) {
				int32_t idx = ct->FindEffectTimer(packet_effect->effect_type);
				if (idx > -1) {
					ct->effect_timers.RemoveAt(idx);
				}

				ct->m_bSelectionDirty = 1;
			}
		}
		break;

		case EFFECT_MULTI_TARGET: {
			// client 40dac9: gated on field_0x80.
			if (this->scenario != 0) {
			CGameObject* ct = nullptr;
			this->objects.Lookup(packet_terrain->buf[0], ct);
			if (ct->action_segments == 0 && g_VFX_info[ct->typeId]->attack_phases != 0) {
				ct->action_segments = g_VFX_info[ct->typeId]->attack_anim_frame_cnt;
				ct->action = ACTION_CAST;
				ct->action_phase = 0;

				ct->actionTargets.RemoveAll();
				for (int i = 1; i < packet_terrain->count; i++) {
					ct->actionTargets.Add(packet_terrain->buf[i]);
				}

				ct->action_target = packet_terrain->buf[1];
				ct->action_spell = 0x1e;
			}
			}
		}
		break;

		case EFFECT_FROM: {
			// client 40dac9: gated on field_0x80.
			if (this->scenario != 0) {
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
			pj->action = ACTION_MOVE;
			pj->pMapObject = this;

			pj->UpdateMapPosition();
			
			this->projectiles[this->next_projectile_id] = pj;
			this->next_projectile_id++;

			SfxSample* snd = g_SfxArray[packet_move->type_id + 500];
			if (packet_move->type_id != 23 && snd) {
				int32_t vol = 0;
				int32_t pan = 0;
				this->ComputeSoundVolumePan(pj->x_pos, pj->y_pos, &vol, &pan);

				int priority = (10000 - ::abs(vol)) / 100;

				snd->Play(vol + g_SoundSettings.sfx_pos, pan, 0, priority, 0);
			}
			}
		}
		break;

		case EFFECT_MULTI_FROM: {
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
			pj->action = ACTION_MOVE;
			pj->pMapObject = this;

			pj->UpdateMapPosition();

			this->projectiles[this->next_projectile_id] = pj;
			this->next_projectile_id++;

			SfxSample* snd = g_SfxArray[pj->typeId + 500];
			if (pj->typeId != 23 && snd) {
				int32_t vol = 0;
				int32_t pan = 0;
				this->ComputeSoundVolumePan(pj->x_pos, pj->y_pos, &vol, &pan);

				int priority = (10000 - ::abs(vol)) / 100;

				snd->Play(vol + g_SoundSettings.sfx_pos, pan, 0, priority, 0);
			}
		}
		break;

		case CHAT: {
			if (packet_join->player_id == 0 || packet_join->player_id >= this->map_players.GetSize() || this->map_players[packet_join->player_id] == nullptr) {
				this->msglog.Add(packet_join->name, clr_log_sblack, 10000);
			} else if (this->my_main_unit->KnowsPlayer(packet_join->player_id) == 0 || packet_join->token_id == CHAT_TYPE_GLOBAL) {
				MapPlayerData* dat = this->map_players[packet_join->player_id];

				if (g_MessageColors == 0 || (wnd->dialogsMask & DIALOG_MASK_INGAME) == 0) {
					this->msglog.Add(CString(dat->name) + ": " + packet_join->name, g_colors_human_pals[dat->color], 10000);
				} else {
					uint16_t* clr = nullptr;
					switch (packet_join->token_id) {
					case CHAT_TYPE_REGULAR:
					case CHAT_TYPE_GLOBAL:
						clr = clr_log_tok0;
						break;
					case CHAT_TYPE_ALLIES:
						clr = clr_log_tok1;
						break;
					case CHAT_TYPE_PRIVATE:
						clr = clr_log_tok2;
						break;
					case CHAT_TYPE_YELL:
						clr = clr_log_tok3;
						break;
					}

					this->msglog.Add(CString(dat->name) + ": " + packet_join->name, clr, 10000);
				}

				g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				
			}
			wnd->vis_root->MsgProc(VIS_MSG_NETMAP_CHAT_SYNC, 0, 0);
		}
		break;

		case TEAM_NOTIFY: {
			switch (packet_2dwords->param1) {
			case TEAM_NOTIFY_UNREACHABLE:
				if (g_EnableTrace) {
					this->msglog.Add(TxtFile::AllLines[0x81], clr_log_sblack, 3000);
				}
				break;

			case TEAM_NOTIFY_SPHERE_LEVELUP: {
				int32_t texid = packet_2dwords->param2 & 0xffff;
				int32_t serv_id = (packet_2dwords->param2 >> 16) & 0xffff;

				CString txt;

				if ((this->my_unit->unitFlags & UNIT_FLAG_MAGE) == 0) {
					txt = TxtFile::AllLines[texid + 0x81];
				} else {
					txt = TxtFile::AllLines[texid + 0x86];
				}

				if (wnd->sessionMode == SESSION_LOCAL_BATTLE) {
					CUnit* ct = this->FindUnitByServerId(serv_id);
					if (ct) {
						if ((ct->unitFlags & UNIT_FLAG_MAGE) == 0) {
							txt = TxtFile::AllLines[texid + 0x81] + CString(" (") + ct->str1 + ")";
						} else {
							txt = TxtFile::AllLines[texid + 0x86] + CString(" (") + ct->str1 + ")";
						}
					}
				}

				this->msglog.Add(txt, clr_log_sblack, 3000);

				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
			}
				break;

			case TEAM_NOTIFY_PLAYER_JOINED_NEW:
				if (packet_2dwords->param2 != this->my_main_unit->index) {
					sprintf(buf, "%s %s %s", TxtFile::AllLines[0xcc], this->map_players[packet_2dwords->param2]->name, TxtFile::AllLines[0xcd]);
					this->msglog.Add(buf, clr_log_sblack, 5000);
				}
				break;

			case TEAM_NOTIFY_PLAYER_JOINED:
				if (packet_2dwords->param2 == this->my_main_unit->index) {
					sprintf(buf, "%s %s %s", TxtFile::AllLines[0xd0], this->map_players[packet_2dwords->param2]->name, TxtFile::AllLines[0xd1]);
				} else {
					sprintf(buf, "%s %s %s", TxtFile::AllLines[0xce], this->map_players[packet_2dwords->param2]->name, TxtFile::AllLines[0xcf]);
				}
				this->msglog.Add(buf, clr_log_sblack, 5000);
				break;

			case TEAM_NOTIFY_PLAYER_FLAGGED:
				sprintf(buf, "%s %s %s", TxtFile::AllLines[0xdd], this->map_players[packet_2dwords->param2]->name, TxtFile::AllLines[0xde]);
				this->msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case TEAM_NOTIFY_CHEAT_FAILED:
				sprintf(buf, "%s %s %s", TxtFile::AllLines[0xdf], this->map_players[packet_2dwords->param2]->name, TxtFile::AllLines[0xe0]);
				this->msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case TEAM_NOTIFY_CHEAT_OK:
				sprintf(buf, "%s %s %s", TxtFile::AllLines[0xe1], this->map_players[packet_2dwords->param2]->name, TxtFile::AllLines[0xe2]);
				this->msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case TEAM_NOTIFY_SHOUT_COOLDOWN:
				sprintf(buf, "%s %d %s", txt_patch.GetLine(0x53), packet_2dwords->param2, txt_patch.GetLine(0x54));
				this->msglog.Add(buf, clr_log_sblack, 5000);

				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case 0x10:
				this->msglog.Add(txt_patch.GetLine(0x58), clr_log_sblack, 5000);

				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case 0x20:
				this->msglog.Add(txt_patch.GetLine(0x59), clr_log_sblack, 5000);
				this->msglog.Add(txt_patch.GetLine(0x5a), clr_log_sblack, 5000);

				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case 0x40:
				this->msglog.Add(txt_patch.GetLine(0x5b), clr_log_sblack, 5000);

				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;

			case 0x80:
				this->msglog.Add(txt_patch.GetLine(0x5c), clr_log_sblack, 5000);

				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
					g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
				}
				break;
			}
		}
		break;

		case PLAYER_ANNOUNCED: {
			this->msglog.Add(txt_patch.GetLine(0x4e) + CString(" ") + packet_join->name + CString(" ") + txt_patch.GetLine(0x4f), clr_log_sblack, 5000);
			g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);

			if (packet_join->__as_dword == this->my_main_unit->index) {
				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) == 0) {
					while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
					{}
					g_CLlDriver.Close();
					return 0;
				}

				wnd->PostMessage(MAIN_MSG_LEAVE_TO_NETWORK_MENU, 0, 0);
				g_CLlDriver.Close();
			}
		}
			break;

		case KILL_NOTICE:
			if (packet_2dwords->param1 != 0 && 
				packet_2dwords->param1 < this->map_players.GetSize() && this->map_players[packet_2dwords->param1] != nullptr &&
				packet_2dwords->param2 != 0 &&
				packet_2dwords->param2 < this->map_players.GetSize() && this->map_players[packet_2dwords->param2] != nullptr) {
				CString str;
				str.Format("%s %s %s %s %s", txt_patch.GetLine(0x50), this->map_players[packet_2dwords->param1]->name, txt_patch.GetLine(0x51), this->map_players[packet_2dwords->param2]->name, txt_patch.GetLine(0x52));
				this->msglog.Add(str, clr_log_sblack, 5000);
			}
			break;

		case PLAYER_INFO: {
			bool isin = packet_join->player_id < this->map_players.GetSize();
			if (isin) {
				if (this->map_players[packet_join->player_id] == nullptr) {
					isin = false;
				}
			}
			MapPlayerData* mp = nullptr;
			if (!isin) {
				mp = new MapPlayerData(packet_join->player_id, packet_join->color);
				mp->token_id = packet_join->token_id;
			} else {
				mp = this->map_players[packet_join->player_id];
			}

			if ((packet_join->flags & 1) == 0) {
				mp->flags &= ~1;
			} else {
				mp->flags |= 1;
			}

			strncpy(mp->name, packet_join->name, 31);
			mp->name[31] = 0;

			if (this->map_players.GetSize() == 1) {
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

			this->map_players.SetAtGrow(mp->index, mp);

			wnd->ClearDiplomacyEntries();

			if (wnd->vis_msgbox) {
				wnd->vis_msgbox->ReadData(&wnd->diplomacy_entries);
				wnd->vis_msgbox->VMethod9();
			}

			wnd->vis_root->MsgProc(VIS_MSG_ROSTER_CHANGED, 0, 0);
		}
			break;

		case PLAYER_LEFT: {
			if (packet_2dwords->param1 == this->my_main_unit->index) {
				wnd->PostMessage(MAIN_MSG_LEAVE_TO_NETWORK_MENU, 0, 0);
				g_CLlDriver.Close();
				break;
			}

			CArray<uint16_t> tmp;
			for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
				uint16_t key;
				CGameObject* obj;
				this->objects.GetNextAssoc(it, key, obj);
				
				if (obj->map_player == this->map_players[packet_2dwords->param1]) {
					tmp.Add(key);
				}
			}

			for (int i = 0; i < tmp.GetSize(); i++) {
				CGameObject* obj;
				this->objects.Lookup(tmp[i], obj);
				this->objects.RemoveKey(tmp[i]);

				if (obj->IsSelected() != 0) {
					this->order_mode = 0;
					wnd->vis_spellbook->ClearSpellSelection();
					wnd->vis_ordertoolbar->MsgProc(VIS_MSG_ORDERS_CLEAR, 0, 0);
				}

				delete obj;
			}

			this->UpdateSelectionState();

			this->map_players[packet_2dwords->param1] = nullptr; //WAT ? not delete?
			wnd->vis_root->MsgProc(VIS_MSG_ROSTER_CHANGED, 0, 0);

			wnd->ClearDiplomacyEntries();

			if (wnd->vis_msgbox) {
				wnd->vis_msgbox->ReadData(&wnd->diplomacy_entries);
				wnd->vis_msgbox->VMethod9();
			}
		}
		break;

		case TERRAIN_DATA: {
			CWordArray tmp;
			tmp.SetSize(packet_terrain->count);
			memcpy(tmp.GetData(), packet_terrain->buf, packet_terrain->count * 2);
			this->ApplyLandscapeFlags(&tmp);
		}
			break;

		case ENEMY_EQUIPMENT: {
			CUnit* ct = nullptr;
			if (packet_equip->unit_id != 0 && this->objects.Lookup(packet_equip->unit_id, *(CGameObject**)&ct) != 0) {
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

				ct->unitFlags |= UNIT_FLAG_EQUIP_CHANGED;
				ct->ReloadSprite();
			}
		}
			break;

		case MAP_REVEAL: {
			switch(packet_2dwords->param1) {
			case 0:
				g_MapRevealed = 0;
				break;

			case 1: {
				g_MapRevealed = 1;
				uint16_t* landscape = this->scenario->GetLandscape();

				for (int i = 0; i < this->scenario->GetWidth(); i++) {
					for (int j = 0; j < this->scenario->GetHeight(); j++) {
						landscape[i + j * scenario->GetWidth()] |= 0xc000;
					}
				}
			}
				break;
			case 2:
				wnd->PostMessage(MAIN_MSG_FAMEHALL_MISSION_INTRO, 0, 0);
				break;
			}
		}
			break;

		case VIEW_POSITION:
			if (wnd->pending_game_mode == 0) {
				this->MsgProc(VIS_MSG_SCROLL_VIEW, packet_2dwords->param1 - (this->view_tiles_x / 2), packet_2dwords->param2 - (this->view_tiles_y / 2));
#ifdef A2CLIENT
				// client: 4164d8 — reselect the local player after the initial position
				if ((wnd->sessionMode == SESSION_SINGLEPLAYER || wnd->sessionMode == SESSION_CLIENT) && this->my_unit != nullptr) {
					this->my_unit->SetSelected(1);
					this->UpdateSelectionState();
				}
#endif
			}
			break;

		case ANNOUNCEMENT:
			this->MsgProc(VIS_MSG_NETMAP_FIND_MAP, (uint32_t)&(packet_join->__as_dword), 0); ///???????????
			break;

		case SERVER_SHUTDOWN:
			if (wnd->sessionMode == SESSION_SINGLEPLAYER) {
				if ((wnd->dialogsMask & DIALOG_MASK_INGAME) == 0) {
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

		case KILL_STAT:
			this->kill_stats[packet_dword->value & 0xffff] = (packet_dword->value >> 16) & 0xff;

			if (wnd->sessionMode != SESSION_LOCAL_BATTLE) {
				wnd->m_GameSession.SaveKillStats();
			}
			break;

		case MISSION_RESULT:
			wnd->PostMessage(MAIN_MSG_SHOW_EVENT_DIALOG, 255, packet_2dwords->param1);
			break;

		case MISSION_COMPLETE:
			wnd->PostMessage(MAIN_MSG_FAMEHALL_MISSION_INTRO, 0, 0);
			break;

		case GAME_EVENT:
			wnd->PostMessage(MAIN_MSG_SHOW_EVENT_DIALOG, packet_2dwords->param1, 0);
			break;

		case ENTER_BUILDING:
			if (wnd->sessionMode == SESSION_SINGLEPLAYER) {
				wnd->inside_building = 1;
				wnd->PostMessage(0x45e, 0, 0);
			}
			break;

		case LEAVE_BUILDING:
			if (wnd->sessionMode == SESSION_SINGLEPLAYER) {
				if ((wnd->dialogsMask & DIALOG_MASK_SHOP) != 0) {
					VisShop* shop = (VisShop*)wnd->vis_root->FindChild(1000);
					shop->CloseShopDialog();
					wnd->vis_root->RemoveChild(shop);
				}

				if ((wnd->dialogsMask & DIALOG_MASK_TAVERN) != 0) {
					VisTav* tavern = (VisTav*)wnd->vis_root->FindChild(1100);
					tavern->CloseDialog();
					wnd->vis_root->RemoveChild(tavern);
				}
				wnd->vis_root->MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
				wnd->inside_building = 0;
				wnd->PostMessage(MAIN_MSG_LEAVE_BUILDING, 0, 0);
			}
			break;

		case DIPLOMACY_STATE:
			for (int i = 16; i < packet_terrain->count; i++) {
				MapPlayerData* mp = this->map_players[i];
				if (mp && (mp->flags & 1) == 0 && i < this->my_main_unit->diplomacy.GetSize()) {
					buf[0] = 0;

					uint16_t& diplo = this->my_main_unit->diplomacy[i];

					if ((diplo & DIPLO_SHARED_VISION) != 0 && (packet_terrain->buf[i] & DIPLO_SHARED_VISION) == 0) {
						sprintf(buf, "%s %s %s", txt_patch.GetLine(0x48), mp->name, txt_patch.GetLine(0x49));
					} else if ((diplo & DIPLO_SHARED_VISION) == 0 && (packet_terrain->buf[i] & DIPLO_SHARED_VISION) != 0) {
						sprintf(buf, "%s %s %s", txt_patch.GetLine(0x46), mp->name, txt_patch.GetLine(0x47));
					}

					if (buf[0] != 0) {
						this->msglog.Add(buf, clr_log_sblack, 3000);
						g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
					}

					buf[0] = 0;

					if ((diplo & DIPLO_THEY_WAR) == 0 && (packet_terrain->buf[i] & DIPLO_THEY_WAR) != 0) {
						sprintf(buf, "%s %s %s", TxtFile::AllLines[0x94], mp->name, TxtFile::AllLines[0x95]);
					} else if ((diplo & DIPLO_THEY_WAR) != 0 && (packet_terrain->buf[i] & DIPLO_THEY_WAR) == 0) {
						sprintf(buf, "%s %s %s", TxtFile::AllLines[0x92], mp->name, TxtFile::AllLines[0x93]);
					}

					if (buf[0] != 0) {
						this->msglog.Add(buf, clr_log_sblack, 3000);
						g_SfxArray[15]->Play(g_SoundSettings.sfx_pos, 0, 0, 100, 0);
					}

					buf[0] = 0;

					if ((diplo & DIPLO_THEY_ALLIED) == 0 && (packet_terrain->buf[i] & DIPLO_THEY_ALLIED) != 0) {
						sprintf(buf, "%s %s %s", TxtFile::AllLines[0x8e], mp->name, TxtFile::AllLines[0x8f]);
					} else if ((diplo & DIPLO_THEY_ALLIED) != 0 && (packet_terrain->buf[i] & DIPLO_THEY_ALLIED) == 0) {
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

		case CREATURE_KNOWLEDGE: {
			uint8_t* stats = nullptr;
			int32_t stats_sz = 0;

			ReadKillStats(packet_data->data, packet_data->count, &stats, &stats_sz);
			memcpy(this->kill_stats.data(), stats, stats_sz);

			delete[] stats;

			if (wnd->sessionMode != SESSION_LOCAL_BATTLE) {
				wnd->m_GameSession.SaveKillStats();
			}
		}
			break;

		case PLAYER_QUESTS: {
			bool isnull = this->quest_map->GetQuestCount() == 0;

			this->quest_map->RemovePlayerQuests(0);

			uint16_t* dat = packet_terrain->buf + 1;

			for (int i = 0; i < packet_terrain->buf[0]; i++) {
				Quest* qwa = this->quest_map->CreateQuest(*dat);
				dat++;

				qwa->CopyFields((QuestData*)dat);
				dat = (uint16_t*)(((QuestData*)dat) + 1);

				this->quest_map->RegisterQuest(qwa);
			}

			if (isnull && quest_map->GetQuestCount() != 0) {
				wnd->PostMessage(MAIN_MSG_TOGGLE_QUEST_STATUS, 1, 0);
			}
		}
			break;

		case INN_QUESTS:
			if ((wnd->dialogsMask & DIALOG_MASK_TAVERN) != 0) {
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

		case MAP_INFO:
			if (packet_join->__as_dword == CalcFileChecksum(packet_join->name)) {
				wnd->enter_ack_received = 1;
				wnd->input_locked = 0;
			} else {
				this->SendRequestFileChunk(0);
				wnd->input_locked = 1;
			}
			break;

		case FILE_CHUNK: {
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
				this->SendRequestFileChunk(packet_file->offset + packet_file->count);
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

				this->SendRequestMapInfo();
			}
		}
			break;

		case LATENCY: {
			NetStru2* conn = g_NetStru1_local.GetFirstClient();
			if (conn) {
				g_CLlDriver.SetLatency(conn->GetUid(), packet_dword->value);
			}
		}
			break;

#ifdef A2CLIENT
		case TEAM_STATE: {
			// client: 41731d — bulk cursor/focus/hat state
			uint32_t* state = (uint32_t*)packet_data->data;
			this->ambient_view_x = *state++;
			this->hat_player_count = *state++;
			this->cursor_over_obj_last_index = *state++;
			this->last_focus_obj_index = *state++;
			this->ambient_view_y = *state++;
			this->ambient_sound_deadline = *state++;
			for (int i = 0; i < 16; i++) {
				this->hat_cursor_slots[i] = *state++;
			}
		} break;

		case GAME_START:
			// client: 41780d — hat session join. Both exits return 1 (never reach the breakid check).
			if (wnd->net_session.internet_game != 0 && wnd->hat_settings.ishat != 0 && wnd->hat_settings.deathmatch != 0) {
				if (this->FindChild(MAP_CHILD_HAT_SESSION) == nullptr) {
					AddChild(new VisHatSession(MAP_CHILD_HAT_SESSION, 10, 20, 370, 188));
					this->full_redraw_requested = 1;
					this->VMethod9();
				}
			}
			this->CloseHatOverlays();
			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{
			}
			if (wnd->net_session.internet_game != 0 && wnd->hat_settings.ishat != 0 && wnd->hat_settings.deathmatch == 3) {
				// client: 417aa7-417b27
				HatPlayerListWindow* dlg = new HatPlayerListWindow(1, 0x14, 0, 0x26c, 0x1f4, &this->hat_name_list, 3);
				wnd->PopUpScreen(dlg);
				g_CLlDriver.Close();
				g_NetStru1_local.ProcessConnections();
			}
			else {
				wnd->PostMessage(MAIN_MSG_SHOW_MISSION_END, packet_dword->value, packet_2dwords->param2);
				g_CLlDriver.Close();
				g_NetStru1_local.ProcessConnections();
			}
			return 1;
			break;
#endif

		case SERVER_LIST: {
			g_WwwServerList.RemoveAll();

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

				g_WwwServerList.Add( CString(bbuf) + '|');
			}


			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{}
			return 1;
		}
			break;

		case HAT_CHAR_LIST: {
			wnd->net_session.hat_id = *(int32_t*)packet_data->data;
			wnd->m_GameSession.hat_char_ids.RemoveAll();

			HatCharId* chars = (HatCharId*)(packet_data->data + 4);
			for (int i = 0; i < packet_data->count / 8; i++) {
				wnd->m_GameSession.hat_char_ids.Add(chars[i]);
			}
		}
			break;

		case HAT_CHAR_DATA: {
			HatCharId* chr = (HatCharId*)packet_data->data;

			CString fname;
			fname.Format("%u%u.a2c", chr->id1, chr->id2);

#ifndef A2CLIENT
			CFile fl;
			if (fl.Open(fname, CFile::modeCreate | CFile::modeWrite)) {
				fl.Write(packet_data->data + 8, packet_data->count - 8);
				fl.Close();
			} else
				g_GameStatusCode = SESSION_ERROR_HAT_CONNECTION_LOST;
#else
			CFile fl;
			if (packet_data->count < 0x18) {
				if (fl.Open(fname, CFile::modeRead)) {
					FileSectionBasicInfo* basic;
					FileSectionStats* stats;
					uint8_t* kstat;
					PacketUnitStateVec* equip;
					PacketUnitStateVec* state;
					uint8_t* a40;
					uint32_t a40_sz;
					int32_t res = ParsePlayerFile(&fl, &basic, &stats, &kstat, &equip, &state, &a40, &a40_sz);
					fl.Close();
					if (res == 0)
						wnd->m_GameSession.hat_char_ids.Add(*chr);
				}
			}
			else if (fl.Open(fname, CFile::modeRead)) {
				CMemFile memfile((BYTE*)packet_data->data + 8, packet_data->count - 8, 0);

				FileSectionBasicInfo* basic;
				FileSectionStats* stats;
				uint8_t* kstat;
				PacketUnitStateVec* equip;
				PacketUnitStateVec* state;
				uint8_t* a40;
				uint32_t a40_sz;
				int32_t res = ParsePlayerFile(&memfile, &basic, &stats, &kstat, &equip, &state, &a40, &a40_sz);

				memfile.Detach();

				if (res && basic && stats && equip) {
					FileSectionBasicInfo srv_basic = *basic;
					FileSectionStats srv_stats = *stats;

					std::array<uint8_t, 0xa00> srv_kstat;
					memcpy(srv_kstat.data(), kstat, 0xa00);

					PacketUnitStateVec srv_equip = *equip;
					PacketUnitStateVec srv_state = *state;

					res = ParsePlayerFile(&fl, &basic, &stats, &kstat, &equip, &state, &a40, &a40_sz);
					fl.Close();

					if (res) {
						strcpy(srv_basic.nick, basic->nick);
						srv_basic.flags = basic->flags;
						srv_basic.color = basic->color;
						if (!WritePlayerFile(fname, &srv_basic, &srv_stats, srv_kstat.data(), &srv_equip, &srv_state, nullptr, 0))
							g_GameStatusCode = SESSION_ERROR_HAT_CONNECTION_LOST;
					}
					else {
						if (fl.Open(fname, CFile::modeCreate | CFile::modeWrite)) {
							fl.Write(packet_data->data + 8, packet_data->count - 8);
							fl.Close();
						} else
							g_GameStatusCode = SESSION_ERROR_HAT_CONNECTION_LOST;
					}
				}
				else {
					fl.Close();
					g_GameStatusCode = SESSION_ERROR_HAT_CONNECTION_LOST;
				}
			} 
			else if (fl.Open(fname, CFile::modeCreate | CFile::modeWrite)) {
				fl.Write(packet_data->data + 8, packet_data->count - 8);
				fl.Close();
			}
			else
				g_GameStatusCode = SESSION_ERROR_HAT_CONNECTION_LOST;
#endif

			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{
			}
			return 1;
		}
			break;

		case HAT_TICKET:
			wnd->m_GameSession.hat_ticket.id1 = packet_2dwords->param1;
			wnd->m_GameSession.hat_ticket.id2 = packet_2dwords->param2;

			while (g_NetStru1_local.ReceiveAnyPacket() != nullptr)
			{}
			return 1;
			break;

#ifdef A2CLIENT
		case HAT_SESSION_SETUP: {
			// client: 41697d — hat session setup (client packet payload base = pkt+0xe)
			uint8_t* p = (uint8_t*)packet_data->data;
			wnd->net_session.hat_id = *(int32_t*)(p + 0);
			wnd->m_GameSession.hat_ticket.id1 = *(int32_t*)(p + 4);
			wnd->m_GameSession.hat_ticket.id2 = *(int32_t*)(p + 8);
			wnd->m_GameSession.sessionKeyPart1 = *(int32_t*)(p + 0xc);
			wnd->m_GameSession.sessionKeyPart2 = *(int32_t*)(p + 0x10);
			wnd->net_session.connect_address = CString((char*)p + 0x14, packet_data->count - 0x14);
		} break;

		case HAT_SESSION_FAIL:
			// client: 416948
			wnd->net_session.hat_id = -1;
			break;

		case HAT_ERROR:
			// client: 41695a
			g_GameStatusCode = (packet_dword->value + 0x18) | SESSION_ERROR_FLAG;
			break;
#endif
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

	this->drift_left = unk;
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
	this->burst_id = wnd->serverLoopCounter;
}



void TakeDamage::Update() {
	//45dd1b
	if (this->drift_left == 0) {
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

int32_t TakeDamage::GetScreenX() { //41f620
    return this->dx + this->cunit->centerScreenX;
}

int32_t TakeDamage::GetScreenY() { //41f640
    return this->dy + this->cunit->centerScreenY - this->cunit->terrainHeightOffset;
}


void __cdecl TakeDamage::AddDamage(CArray<TakeDamage>* arr, const TakeDamage& dmg) {
	//45dea1

	for (int i = 0; i < arr->GetSize(); i++) {
		TakeDamage& b = arr->ElementAt(i);
		if (b.cunit == dmg.cunit && b.burst_id == dmg.burst_id) {
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



void BigStruct2::SendChatPacket(const char* nam, int32_t token, int32_t playerid) {
	//41b2a4

	PacketJoin* join = &PacketJoin::Inst;
	join->id = CHAT;
	join->from_player_id = this->my_main_unit->index;
	join->to_player_id = 0;

	strcpy(join->name, nam);

	if (playerid == 0) {
		join->player_id = 0;
		join->token_id = CHAT_TYPE_REGULAR;
		join->color = 0;
		join->flags = 0;
	} else {
		join->__as_dword = this->map_players[playerid]->index;
	}

	join->__as_dword |= token << 8;

	if (g_NetStru1_local.IsActive() == 0) {
		if (g_NetStru1_main.IsActive() && g_Server) {
			g_NetStru1_main.QueuePacketSend(join);
			g_NetStru1_main.SendAllData();
		}
	} else {
		g_NetStru1_local.QueuePacketSend(join);
	}
}

void BigStruct2::SendShopClose() {
	//41a8cc
	PacketWord* pkt = &PacketWord::Inst;

	pkt->id = SHOP_CLOSE;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->value = this->GetUnit_3f6c()->unit_id;

	g_NetStru1_local.QueuePacketSend(pkt);

	((MainWindow*)AfxGetMainWnd())->dialogsMask &= ~DIALOG_MASK_SHOP;
}


int32_t BigStruct2::SendEnterGameAndWait() {
	//40d7f3  client: 40d87a
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	if (wnd->sessionMode == SESSION_SINGLEPLAYER && (wnd->net_session.internet_game == 0 || wnd->hat_settings.ishat == 0)) {
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

	this->objects.RemoveAll();

	if (this->my_unit) {
		delete this->my_unit;
	}

	this->my_unit = nullptr;

	this->UpdateSelectionState();

	g_NetStru1_local.QueuePacketSend(pkt);

	if (wnd->serverBootstrapEnabled != 0) {
		g_Server->PumpNetwork();
	}
	
	uint32_t stime = timeGetTime();
#ifndef A2CLIENT
	while (g_NetStru1_local.GetClientsPktNum() == 0) {

		MSG msg;
		if (PeekMessageA(&msg, NULL, 0, 0, 1) != 0) {
			if (msg.message == WM_QUIT) {
				return 0;
			}

			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		if (g_CmdTimeout < timeGetTime() - stime) {
			g_GameStatusCode = SESSION_ERROR_SERVER_NO_RESPONSE;
			return 0;
		}

		g_mousept.Update();
		g_NetStru1_local.ProcessConnections();
	}

	if (this->ProcessPackets(SERVER_TICK) == 0) {
		return 0;
	}

	if (this->map_players.GetSize() < 2) {
		g_GameStatusCode = SESSION_ERROR_NO_PLAYER_LIST;
		return 0;
	}
#else
	while (map_players.GetSize() < 2) {
		while (g_NetStru1_local.GetClientsPktNum() == 0) {

			MSG msg;
			if (PeekMessageA(&msg, NULL, 0, 0, 1) != 0) {
				if (msg.message == WM_QUIT) {
					return 0;
				}

				TranslateMessage(&msg);
				DispatchMessageA(&msg);
			}

			if (g_CmdTimeout < timeGetTime() - stime) {
				g_GameStatusCode = SESSION_ERROR_SERVER_NO_RESPONSE;
				return 0;
			}

			g_mousept.Update();
			g_NetStru1_local.ProcessConnections();
		}

		if (this->ProcessPackets(SERVER_TICK) == 0) {
			return 0;
		}
	}
#endif
	return 1;
}


int BigStruct2::ConnectAndJoinSession() {
	//40d4e2
	g_GameStatusCode = 0;

	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	if (wnd->sessionMode == SESSION_CLIENT || wnd->sessionMode == SESSION_LOCAL_BATTLE) {
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

	if (this->SendEnterGameAndWait()) {
		return 1;
	}

	if ((g_GameStatusCode & 0xff) < 11) {
		wnd->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, TxtFile::AllLines.GetAt((g_GameStatusCode & 0xff) + 0xc0), nullptr, 0));
	} else {
		wnd->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(g_GameStatusCode & 0xff), nullptr, 0));
	}
	return 0;
}



void BigStruct2::SendJoinMap() {
	//0041cbb8
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = JOIN_MAP;
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

void BigStruct2::SendRequestUnitUpdateFile(const CString& fname) {
	//41cc78
	PacketData* pkt = &PacketData::Inst;

	pkt->id = REQUEST_UNIT_UPDATE;
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



void BigStruct2::ResetWorldState(int32_t arg) { // 41d2da
	this->my_unit = nullptr;

	if (!this->objects.IsEmpty()) {
		POSITION it = this->objects.GetStartPosition();
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->objects.GetNextAssoc(it, key, obj);
			if (obj) {
				delete obj;
			}
		}

		this->objects.RemoveAll();
	}

	if (!this->projectiles.IsEmpty()) {
		POSITION it = this->projectiles.GetStartPosition();
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->projectiles.GetNextAssoc(it, key, obj);
			if (obj) {
				delete obj;
			}
		}

		this->projectiles.RemoveAll();
	}

	this->damage_labels.RemoveAll();

	if (arg == 0) {
		for (int i = 1; i < 17; i++) {
			MapPlayerData* mp = this->map_players[i];
			if (mp && (mp->flags & 1) != 0) {
				delete mp;
				this->map_players[i] = nullptr;
			}
		}
	} else {
		for (int i = 1; i < this->map_players.GetSize(); i++) {
			MapPlayerData* mp = this->map_players[i];
			if (mp) {
				delete mp;
			}
		}
		if (this->map_players.GetSize()) {
			this->map_players.SetSize(1);
		}
	}

	this->terrain_light_override_cells.RemoveAll();

	if (this->scenario) {
		delete this->scenario;
		this->scenario = nullptr;
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
	if (wnd->dialogsMask & DIALOG_MASK_SHOP) {
		return wnd->vis_root->FindChild(1000)->FindChild(MAP_CHILD_SPELL_BOOK) != nullptr;
	}

	return this->FindChild(MAP_CHILD_SPELL_BOOK) != nullptr;
}

int32_t BigStruct2::IsBagOpen() { // 41b495
	return this->FindChild(MAP_CHILD_BAG) != nullptr;
}

void BigStruct2::SendPlayerName(CString str) { //41afcf
	PacketJoin* pkt = &PacketJoin::Inst;
	pkt->id = 7;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	strcpy(pkt->name, str);

	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::SendDiplomacyState() { //41a735
	PacketTerrain* pkt = &PacketTerrain::Inst;
	pkt->id = SET_DIPLOMACY;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	memcpy(pkt->buf, this->my_main_unit->diplomacy.GetData(), this->my_main_unit->diplomacy.GetSize() * 2);
	pkt->count = this->my_main_unit->diplomacy.GetSize();
	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::StopAmbientSounds() { //41c39c
	this->ambient_view_x = -1;
	this->ambient_view_y = -1;
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
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);

		if (obj->IsKindOf(RUNTIME_CLASS(CUnit)) == 0) {
			arr.Add(key);
		} else {
			CUnit* uni = (CUnit*)obj;
			if ((uni->unitFlags & UNIT_FLAG_PLAYER_CLASS) != 0 && uni->hp >= -10 && uni->map_player == this->my_main_unit) {
				uni->death_state = 0;
				uni->action = ACTION_NONE;
				uni->action_segments = 0;
				uni->SetSelected(0);
			} else {
				arr.Add(key);
			}
		}
	}

	this->my_unit->SetSelected(1);

	//Remove it here or while iteration
	for (int i = 0; i < arr.GetSize(); i++) {
		delete this->objects[arr[i]];
		this->objects.RemoveKey(arr[i]);
	}
	arr.RemoveAll();

	for (POSITION it = this->projectiles.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->projectiles.GetNextAssoc(it, key, obj);

		delete obj;
		this->projectiles.RemoveKey(key);
	}

	this->damage_labels.RemoveAll();

	if (this->scenario) {
		delete this->scenario;
	}
	this->scenario = nullptr;

	while (this->map_players.GetSize() > 1) {
		MapPlayerData* mp = this->map_players[1];
		if (mp != nullptr && mp != this->my_main_unit) {
			delete mp;
		}
		this->map_players.RemoveAt(1, 1);
	}

	this->map_players.SetAtGrow(this->my_main_unit->index, this->my_main_unit);

	this->terrain_light_override_cells.RemoveAll();

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
	this->selected_object_count = 0;
	this->selection_flags = 0;
	this->selected_available_spell_mask = 0;
	this->active_spell_effect_mask = 0;
	this->selected_active_spell_mask = 0;

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

	for (POSITION it = this->objects.GetStartPosition(); it;) {
		POSITION curpos = it;
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);

		if (obj->IsSelected() == 0) {
			continue;

		// if selected
		}
		this->selected_object_count++;
		this->selected_available_spell_mask |= obj->availableSpellMask;
		if (obj->active_spell != 0) {
			this->selected_active_spell_mask |= SpellBtB[obj->active_spell];
		}

		if (!firstsel) {
			this->primary_selected_object = obj;
			this->primary_selected_assoc = curpos;
			firstsel = true;
		}

		if (mwnd->dialogsMask == DIALOG_MASK_INGAME || (mwnd->dialogsMask & DIALOG_MASK_SHOP) != 0) {
			if (obj->GetRuntimeClass() == RUNTIME_CLASS(CUnit)) {
				CUnit* unit = (CUnit*)obj;

				this->selection_flags |= 1;

				if ((unit->typeId == 0x17 || unit->typeId == 0x18) && unit->availableSpellMask != 0) {
					this->selection_flags |= 0x200;
					this->UpdateSpellModifiers(unit);
				}

				if (unit->unitFlags & UNIT_FLAG_PLAYER_CLASS) {
					this->selection_flags |= 8;
				}

				this->UpdateSpellEffects(unit);
			} else if (obj->GetRuntimeClass() == RUNTIME_CLASS(CAirUnit)) {
				this->selection_flags |= 2;
			} else if (obj->GetRuntimeClass() == RUNTIME_CLASS(CStructure)) {
				this->selection_flags |= 0x20;
			}
		}
		else {
			this->selection_flags = 8;
		}
	}

	if (this->selected_object_count > 0 && this->primary_selected_object->map_player != this->my_main_unit) {
		this->selection_flags |= 4;
	}
	
	if (this->selected_object_count == 0 || (this->selection_flags & 0x24) != 0) {
		mwnd->vis_right_panel->MsgProc(CVisualObject::MSG_40a, 0, 0);
	} else {
		mwnd->vis_right_panel->MsgProc(CVisualObject::MSG_409, this->AvailableOrderMask(), 0);
	}

	mwnd->vis_charinfo->MsgProc(CVisualObject::MSG_410, 0, 0);
	mwnd->vis_sidestatus->MsgProc(CVisualObject::MSG_410, 0, 0);
	if (this->selected_object_count == 1 && (this->selection_flags & 8) != 0) {
		mwnd->vis_invtype1->VMethod33(this->primary_selected_object);
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
		this->active_spell_effect_mask |= SpellBtB[cast_spell];
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
	if ((this->selection_flags & 4) == 0) {
		if (this->selected_available_spell_mask != 0 || this->active_spell_effect_mask != 0) {
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


void BigStruct2::SendGameMode(int32_t m) { //41cad0
	Packet2Dwords *pkt = &Packet2Dwords::Inst;
	pkt->id = 4;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = m;
	
	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::SendWimpyMode(int32_t wim) { //41aaaa
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = SET_PARAMETER;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = 1;
	pkt->param2 = wim;

	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::SendFormationMode(int32_t form) { // 41abd2
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = SET_PARAMETER;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = 2;
	pkt->param2 = form;

	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::SendAutoCastSetting() { //41ab74
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = SET_PARAMETER;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = 3;
	pkt->param2 = g_settings.AutoCasting;

	g_NetStru1_local.QueuePacketSend(pkt);
}


void BigStruct2::RecalcVisibleMapHeight() { //41b6e0
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	int32_t btm = this->rect.bottom & (~0x1f);
	CVisualObject* obj2 = this->FindChild(MAP_CHILD_BAG);
	CVisualObject* obj3 = this->FindChild(MAP_CHILD_SPELL_BOOK);

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

	if (CVisualObject::FindChild(MAP_CHILD_CHAT_LINE)) {
		mwnd->vis_1200->RepositionAboveDialogs();
	}

	this->force_full_redraw = 1;
}

void BigStruct2::CloseSpellBook() { //41b636
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if ((mwnd->dialogsMask & DIALOG_MASK_SHOP) == 0) {
		this->RemoveChild(mwnd->vis_spellbook);
	} else {
		mwnd->vis_root->FindChild(1000)->RemoveChild(mwnd->vis_spellbook);
	}

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);

	mwnd->vis_right_panel->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);

	this->RecalcVisibleMapHeight();
}


void BigStruct2::OpenSpellBook() { // 41b509

	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if (this->IsBagOpen() == 0) {
		mwnd->vis_spellbook->SetRect(0, this->rect.bottom - 85, this->rect.right, this->rect.bottom);
	} else {
		mwnd->vis_spellbook->SetRect(0, this->rect.bottom - 175, this->rect.right, this->rect.bottom - 90);
	}

	if ((mwnd->dialogsMask & DIALOG_MASK_SHOP) == 0) {
		this->AddChild(mwnd->vis_spellbook);
	} else {
		mwnd->vis_spellbook->SetRect(0, 305, 480, 390);
		mwnd->vis_root->FindChild(1000)->AddChild(mwnd->vis_spellbook);
	}

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);

	mwnd->vis_right_panel->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);

	this->RecalcVisibleMapHeight();
}

void BigStruct2::CloseBag() { //41b40e
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if (this->IsBookOpen()) {
		mwnd->vis_spellbook->SetRect(0, this->rect.bottom - 85, this->rect.right, this->rect.bottom);
	}

	this->RemoveChild(mwnd->vis_invtype1);

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);

	this->RecalcVisibleMapHeight();
}

void BigStruct2::OpenBag() { // 41b381
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if (this->IsBookOpen()) {
		mwnd->vis_spellbook->SetRect(0, this->rect.bottom - 175, this->rect.right, this->rect.bottom - 90);
	}
	
	this->AddChild(mwnd->vis_invtype1);

	g_SfxArray[7]->Play(g_SoundSettings.sfx_pos, 0, 0, 220, 0);
	this->RecalcVisibleMapHeight();
}

void BigStruct2::SendRequestUnitsResend() {
	Packet* pkt = &Packet::Inst;
	pkt->id = REQUEST_UNITS_RESEND;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::SendCheatShop(int32_t arg1, int32_t arg2) {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = CHEAT_SHOP;
	pkt->param1 = arg1;
	pkt->param2 = arg2;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}

void BigStruct2::SendAdjustPlayerGoldAction(int32_t goldDelta) const { //41b0be
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = CHEAT_MONEY;
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
	pkt->id = ENTER_SHOP;
	pkt->value = this->GetUnit_3f6c()->unit_id;
	g_NetStru1_local.QueuePacketSend(pkt);

	((MainWindow*)AfxGetMainWnd())->dialogsMask |= DIALOG_MASK_SHOP;
}

void BigStruct2::NetOnOpenInnDialog() { //41adbb
	PacketWord* pkt = &PacketWord::Inst;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->id = ENTER_INN;
	pkt->value = this->GetUnit_3f6c()->unit_id;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41F830
CUnit* BigStruct2::GetUnit_3f6c() {
	return this->my_unit;
}

// 41F6D0
void BigStruct2::CycleFormation() {
	this->formation = (this->formation + 1) % 3;
	this->SendFormationMode(this->formation);
}

// 41F720
void BigStruct2::CycleWimpyMode() {
	this->wimpy = (this->wimpy + 1) % 3;
	this->SendWimpyMode(this->wimpy);
}

// 41F770
void BigStruct2::ToggleShowHp() {
	this->show_hp = this->show_hp == 0;
}

// 41F7B0
void BigStruct2::ToggleFlyingHp() {
	this->flying_hp = this->flying_hp == 0;
}

// 41F7F0
int32_t BigStruct2::GetWimpyMode() {
	return this->wimpy;
}

// 41F810
int32_t BigStruct2::GetFormation() {
	return this->formation;
}

// 416983
void BigStruct2::SelectControlGroup(int32_t digit) {
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsInControlGroup(digit)) {
			obj->SetSelected(1);
		} else if (g_kbShiftState == 0) {
			obj->SetSelected(0);
		}
	}
	this->UpdateSelectionState();
}

// 416A02
void BigStruct2::AssignControlGroup(int32_t digit) {
	if (this->selection_flags & 4) {
		return;
	}
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
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
void BigStruct2::AddToControlGroup(int32_t digit) {
	if (this->selection_flags & 4) {
		return;
	}
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
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
void BigStruct2::SelectControlGroupAndCenter(int32_t digit) {
	int32_t sum_x = 0;
	int32_t sum_y = 0;
	int32_t count = 0;
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
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
		this->MsgProc(VIS_MSG_SCROLL_VIEW, avg_x - this->view_tiles_x / 2, avg_y - this->view_tiles_y / 2);
	}
}

// 416C4C
void BigStruct2::SelectAllOwnUnits() {
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsKindOf(RUNTIME_CLASS(CUnit)) && obj->map_player == this->my_main_unit &&
			((CUnit*)obj)->death_state <= 1 && !(((CUnit*)obj)->unitFlags & UNIT_FLAG_HIDDEN)) {
			obj->SetSelected(1);
		} else {
			obj->SetSelected(0);
		}
	}
	this->UpdateSelectionState();
}

// 4168BD
void BigStruct2::CancelOrderMode() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (this->order_mode != 0) {
		if (this->order_mode == 5 && this->IsBookOpen()) {
			this->CloseSpellBook();
		}
		wnd->vis_spellbook->ClearSpellSelection();
		this->order_mode = 0;
		wnd->vis_ordertoolbar->MsgProc(VIS_MSG_ORDERS_CLEAR, 0, 0);
	} else {
		for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
			uint16_t key;
			CGameObject* obj;
			this->objects.GetNextAssoc(it, key, obj);
			obj->SetSelected(0);
		}
		this->UpdateSelectionState();
	}
}

// 4167F7
void BigStruct2::ScrollViewTo(int32_t wparam, int32_t lparam) {
	if (this->scenario == nullptr) {
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
	wnd->vis_minimap->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
}

// 403F08
void BigStruct2::SetScenario(Scenario* scen) {
	this->scenario = scen;
	this->map_width = this->scenario->GetWidth();
	this->map_height = this->scenario->GetHeight();
}

// 418F93
void BigStruct2::SetOrderMode(int32_t arg) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	uint32_t avail = this->AvailableOrderMask();
	int32_t mode = arg;
	if (arg == 9) {
		mode = 5;
	}
	if (arg == 0xA) {
		mode = 5;
	}
	if (this->order_mode == 5 && mode != 5 && this->IsBookOpen()) {
		this->CloseSpellBook();
		wnd->vis_spellbook->ClearSpellSelection();
	}
	if ((avail & (1 << (mode - 1))) != 0 || arg == 0xA) {
		this->order_mode = mode;
		wnd->vis_ordertoolbar->MsgProc(VIS_MSG_TOOLBAR_ORDER_SELECT, mode - 1, 0);
		switch (arg) {
		case 3:
			this->SendGroupGuard();
			this->order_mode = 0;
			wnd->vis_ordertoolbar->MsgProc(VIS_MSG_ORDERS_CLEAR, 0, 0);
			break;
		case 7:
			this->SendGroupAggressive();
			this->order_mode = 0;
			wnd->vis_ordertoolbar->MsgProc(VIS_MSG_ORDERS_CLEAR, 0, 0);
			break;
		case 8:
			this->SendGroupGoHome();
			this->order_mode = 0;
			wnd->vis_ordertoolbar->MsgProc(VIS_MSG_ORDERS_CLEAR, 0, 0);
			break;
		case 5:
			if (!this->IsBookOpen()) {
				this->OpenSpellBook();
			}
			break;
		}
	}
}

// 419154
void BigStruct2::SendGroupMove(uint16_t x, uint16_t y) {
	POSITION it = this->objects.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_MOVE;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->x = x;
	int32_t tile_y = this->ScreenToTileRow(g_mousept.x, g_mousept.y) + this->view_y;
	pkt->y = tile_y;
	pkt->y_high = tile_y >> 8;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr && unit->last_action != ACTION_MOVE) {
		unit->PlayMoveVoice();
	}
}

// 419246
void BigStruct2::SendGroupAttack(uint16_t id) {
	POSITION it = this->objects.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_ATTACK;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param = id;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr) {
		unit->PlayAttackVoice();
	}
}

// 41930D
void BigStruct2::SendGroupAttackMove(uint16_t x, uint16_t y) {
	POSITION it = this->objects.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_ATTACK_MOVE;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->x = x;
	pkt->y = y;
	pkt->y_high = y >> 8;
	pkt->to_player_id = 0;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr && unit->last_action != ACTION_MOVE) {
		unit->PlaySwarmVoice();
	}
}

// 4193E8
void BigStruct2::SendGroupGuard() {
	POSITION it = this->objects.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_GUARD;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr) {
		unit->PlayDefendVoice();
	}
}

// 4194A2
void BigStruct2::SendGroupAggressive() {
	POSITION it = this->objects.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_AGGRESSIVE;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr) {
		unit->PlayDefendVoice();
	}
}

// 41955C
void BigStruct2::SendGroupAttackTarget(uint16_t id) {
	POSITION it = this->objects.GetStartPosition();
	CGameObject* obj;
	if (!this->objects.Lookup(id, obj) || !obj->IsKindOf(RUNTIME_CLASS(CUnit))) {
		return;
	}
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_ATTACK_TARGET;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param = id;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr) {
		unit->PlayDefendVoice();
	}
}

// 41965D
void BigStruct2::SendGroupPatrol(uint16_t x, uint16_t y) {
	POSITION it = this->objects.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_PATROL;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->x = x;
	pkt->y = y;
	pkt->y_high = y >> 8;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr) {
		unit->PlayMoveVoice();
	}
}

// 41972F
void BigStruct2::SendGroupCastAtPosition(uint16_t x, uint16_t y, int32_t spell) {
	POSITION it = this->objects.GetStartPosition();
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_CAST_POS;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->x = x;
	pkt->y = y;
	pkt->y_high = y >> 8;
	pkt->count = 0;
	if (wnd->vis_spellbook->IsSpellAvailable(spell - 1) != 0 && wnd->vis_spellbook->spell == -1) {
		pkt->id = GROUP_CAST_POS;
		pkt->slot = spell;
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->objects.GetNextAssoc(it, key, obj);
			if (obj->IsSelected() && (obj->availableSpellMask & (1 << (spell - 1))) != 0) {
				pkt->AppendWord(obj->unit_id);
			}
		}
		g_NetStru1_local.QueuePacketSend(pkt);
	}
	if (wnd->vis_spellbook->spell >= 0 && this->selected_object_count == 1 && this->primary_selected_object != nullptr) {
		pkt->id = SCROLL_CAST_POS;
		pkt->count = 0;
		pkt->AppendWord(this->primary_selected_object->unit_id);
		uint16_t slot = wnd->vis_spellbook->spell;
		pkt->slot = slot;
		pkt->slot_high = slot >> 8;
		g_NetStru1_local.QueuePacketSend(pkt);
		TokenEntry* entry = this->primary_selected_object->tokenEntries[slot];
		if (entry->count == 1) {
			this->primary_selected_object->tokenEntries.RemoveAt(slot, 1);
			delete entry;
		} else {
			entry->count--;
		}
		wnd->vis_spellbook->spell = -1;
		wnd->vis_spellbook->selected = -1;
	} else {
		if (wnd->vis_spellbook->IsSpellEffectActive(spell - 1) != 0) {
			for (it = this->objects.GetStartPosition(); it != nullptr;) {
				uint16_t key;
				CGameObject* obj;
				this->objects.GetNextAssoc(it, key, obj);
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
				pkt->id = SCROLL_CAST_POS;
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
void BigStruct2::SendGroupCastAtUnit(uint16_t id, int32_t spell) {
	POSITION it = this->objects.GetStartPosition();
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param = id;
	pkt->count = 0;
	if (wnd->vis_spellbook->IsSpellAvailable(spell - 1) != 0 && wnd->vis_spellbook->spell == -1) {
		pkt->id = GROUP_CAST_UNIT;
		pkt->slot = spell;
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->objects.GetNextAssoc(it, key, obj);
			if (obj->IsSelected() && (obj->availableSpellMask & (1 << (spell - 1))) != 0) {
				pkt->AppendWord(obj->unit_id);
			}
		}
		g_NetStru1_local.QueuePacketSend(pkt);
	}
	if (wnd->vis_spellbook->spell >= 0 && this->selected_object_count == 1 && this->primary_selected_object != nullptr) {
		pkt->count = 0;
		pkt->AppendWord(this->primary_selected_object->unit_id);
		uint16_t slot = wnd->vis_spellbook->spell;
		pkt->slot = slot;
		pkt->slot_high = slot >> 8;
		pkt->id = SCROLL_CAST_UNIT;
		g_NetStru1_local.QueuePacketSend(pkt);
		TokenEntry* entry = this->primary_selected_object->tokenEntries[slot];
		if (entry->count == 1) {
			this->primary_selected_object->tokenEntries.RemoveAt(slot, 1);
			delete entry;
		} else {
			entry->count--;
		}
		wnd->vis_spellbook->spell = -1;
		wnd->vis_spellbook->selected = -1;
	} else {
		if (wnd->vis_spellbook->IsSpellEffectActive(spell - 1) != 0) {
			for (it = this->objects.GetStartPosition(); it != nullptr;) {
				uint16_t key;
				CGameObject* obj;
				this->objects.GetNextAssoc(it, key, obj);
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
				pkt->id = SCROLL_CAST_UNIT;
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
void BigStruct2::FreeTileGrids() {
	free(this->tile_units);
	free(this->tile_air_units);
	free(this->tile_structures);
	free(this->tile_backpacks);
	free(this->tile_special_units);
	free(this->corner_tile_kind);
	free(this->corner_tile_kind_prev);
	free(this->corner_light);
	free(this->corner_light_prev);
	free(this->tile_light);
	free(this->tile_corner_y);
	free(this->tile_top_y);
	free(this->tile_bottom_y);
	free(this->height_cells);
}

// 40403B
void BigStruct2::RebuildWorldGrids() {
	this->dirty_rect.SetRectEmpty();
	memset(this->tile_units, 0, this->tile_grid_bytes);
	memset(this->tile_air_units, 0, this->tile_grid_bytes);
	memset(this->tile_structures, 0, this->tile_grid_bytes);
	memset(this->tile_backpacks, 0, this->tile_grid_bytes);
	memset(this->tile_special_units, 0, this->tile_grid_bytes);
	if (g_VideoOptionFlags == 2) {
		memcpy(this->corner_light_prev, this->corner_light, this->corner_grid_bytes / 4);
	}
	memset(this->corner_light, 0xFF, this->corner_grid_bytes / 4);
	this->lightning_active = 0;
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		obj->UpdateMapPosition();
		obj->UpdateTileLayer();
		obj->UpdateLight();
	}
	memset(this->corner_tile_kind, 0, this->corner_grid_bytes);
	uint16_t* landscape = this->scenario->GetLandscape();
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
					this->dirty_rect |= rect;
				} else if (this->minimap_dirty != 0) {
					CRect rect((x - 1) * 0x20, this->tile_top_y[h_idx] - 0x60, (x + 2) * 0x20, this->tile_bottom_y[h_idx] + 0x60);
					this->dirty_rect |= rect;
				}
				this->corner_tile_kind[out_idx] = 0;
			} else if (tile == 0x8000) {
				this->corner_tile_kind[out_idx] = 8;
			} else {
				this->corner_tile_kind[out_idx] = 0x10;
			}
			out_idx++;
		}
		src_idx += this->map_width - this->view_tiles_x - 7;
	}
	memcpy(this->corner_tile_kind_prev, this->corner_tile_kind, this->corner_grid_bytes);
	for (POSITION it = this->projectiles.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->projectiles.GetNextAssoc(it, key, obj);
		obj->UpdateMapPosition();
		obj->UpdateLight();
	}
	for (POSITION it = this->lightning_cells.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		uint32_t value;
		this->lightning_cells.GetNextAssoc(it, key, value);
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
			this->dirty_rect |= rect;
		}
	}
	for (POSITION it = this->terrain_light_override_cells.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		uint32_t value;
		this->terrain_light_override_cells.GetNextAssoc(it, key, value);
		int32_t lx = key & 0xFF;
		int32_t ly = key >> 8;
		if (this->view_x - 3 <= lx && lx < this->view_x + 3 + this->view_tiles_x &&
			this->view_y - 3 <= ly && ly < this->view_y + 7 + this->view_tiles_y && value != 0) {
			lx -= this->view_x;
			ly -= this->view_y;
			if (g_Lightning == 0) {
				CRect rect((lx - 1) * 0x20, (ly - 3) * 0x20, (lx + 2) * 0x20, (ly + 2) * 0x20);
				this->dirty_rect |= rect;
			} else {
				CRect rect((lx - 3) * 0x20, (ly - 3) * 0x20, (lx + 4) * 0x20, (ly + 4) * 0x20);
				this->dirty_rect |= rect;
			}
		}
	}
	for (int32_t i = 0; i < this->damage_labels.GetSize(); i++) {
		TakeDamage& dmg = this->damage_labels.ElementAt(i);
		int32_t left = (dmg.GetScreenX() - 0x30) & ~0x1F;
		int32_t top = dmg.GetScreenY() - 0x30;
		int32_t right = ((dmg.GetScreenX() + 0x30) & ~0x1F) + 0x20;
		int32_t bottom = dmg.GetScreenY() + 0x30;
		CRect rect(left, top, right, bottom);
		this->dirty_rect |= rect;
	}
	this->UpdateTileLight();
}

// 404912
void BigStruct2::ApplyTerrainLightOverrides() {
	uint8_t* buf = this->scenario->GetOverlayMap();
	for (POSITION it = this->terrain_light_override_cells.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		uint32_t value;
		this->terrain_light_override_cells.GetNextAssoc(it, key, value);
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
int32_t BigStruct2::StepShadowRay(int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4) {
	int32_t idx;
	if (arg2 > 0) {
		idx = arg2 * this->map_width + arg1;
	} else {
		idx = arg1 - (-arg2) * this->map_width;
	}
	int32_t nx = arg1 + this->shadow_step[arg1][arg2][0];
	int32_t ny = arg2 + this->shadow_step[arg1][arg2][1];
	int8_t* heights = this->scenario->GetMapHeights();
	int32_t base = heights[arg3 + idx] - arg4;
	int32_t val = this->shadow_light[nx][ny] - (this->shadow_ray_len[arg1][arg2] + base);
	this->shadow_light[arg1][arg2] = val;
	return val <= 0;
}

// 41B155
void BigStruct2::SendRequestRespawn() {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = REQUEST_RESPAWN;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41CE14
void BigStruct2::SendHurtSelf() {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = HURT_SELF;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41AC2D
void BigStruct2::SendHotkeyBroadcast(int32_t id) {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = SET_PARAMETER;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = 0x80;
	pkt->param2 = id;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41AC88
void BigStruct2::SendDropGold(int32_t arg1, int32_t arg2) {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = DROP_GOLD;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = arg1;
	pkt->param2 = arg2;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 404A2C
void BigStruct2::UpdateTileLight() {
	int32_t delta = g_DeltaCLR.brightness;
	memset(this->tile_light, delta >> 2, this->tile_grid_bytes / 4);
	for (POSITION it = this->terrain_light_override_cells.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		uint32_t value;
		this->terrain_light_override_cells.GetNextAssoc(it, key, value);
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
			int32_t brightness = (abs(this->anim_tick / 2 + x * y) % 5 & 1) != 0 ? 0xC : 0;
			this->ApplyDynamicLight(x, y, 1, brightness);
		}
	}
	for (int32_t ty = 0; ty < this->view_tiles_y + 10; ty++) {
		for (int32_t tx = 0; tx < this->view_tiles_x + 6; tx++) {
			int32_t idx = ty * (this->view_tiles_x + 7) + tx;
			int32_t idx_next = idx + this->view_tiles_x + 7;
			int32_t v0 = this->corner_light[idx] - 0x20;
			if (v0 < 0) {
				v0 = 0;
			}
			int32_t v1 = this->corner_light[idx + 1] - 0x20;
			if (v1 < 0) {
				v1 = 0;
			}
			int32_t v2 = this->corner_light[idx_next] - 0x20;
			if (v2 < 0) {
				v2 = 0;
			}
			int32_t v3 = this->corner_light[idx_next + 1] - 0x20;
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
void BigStruct2::AddHitRect(CRect* rect, uint16_t unit_id) {
	this->hit_rects.SetAtGrow(this->hit_count, *rect);
	this->hit_unit_ids.SetAtGrow(this->hit_count, unit_id);
	this->hit_count++;
}

// 41AB05
void BigStruct2::CycleAutoCastGroup() {
	int32_t mode = g_settings.AutoCasting & 0x38;
	if (mode == 8) {
		mode = 0x18;
	} else if (mode == 0x18) {
		mode = 0x38;
	} else if (mode == 0x38) {
		mode = 8;
	}
	g_settings.AutoCasting = (g_settings.AutoCasting & 7) | mode;
	this->SendAutoCastSetting();
}

// 41A66E
void BigStruct2::SendGroupEnterBuilding(uint16_t id) {
	POSITION it = this->objects.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_ENTER_BUILDING;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->param = id;
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr) {
		unit->PlayMoveVoice();
	}
}

// 41A7C7
void BigStruct2::SendItemOperation(uint8_t arg1, uint16_t arg2, uint8_t arg3, uint16_t arg4, uint16_t arg5) {
	PacketItemTransfer* pkt = &PacketItemTransfer::Inst;
	pkt->id = ITEM_OPERATION;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->unit_id = this->primary_selected_object->unit_id;
	pkt->type = arg1;
	pkt->subtype = arg3;
	pkt->param1 = arg2;
	pkt->param2 = arg4;
	pkt->param3 = arg5;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41A5B4
void BigStruct2::SendGroupStop() {
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_STOP;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr) {
		unit->PlayDefendVoice();
	}
}

// 41A33F
void BigStruct2::SendGroupPickup(uint16_t x, uint16_t y) {
	POSITION it = this->objects.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_PICKUP;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->x = x;
	pkt->y = y;
	pkt->y_high = y >> 8;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->param = obj->shopInventoryVisibleStart;
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr) {
		unit->PlayPickupVoice();
	}
}

// 41A425
void BigStruct2::SendGroupGoHome() {
	POSITION it = this->objects.GetStartPosition();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_GO_HOME;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected()) {
			pkt->AppendWord(obj->unit_id);
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr) {
		unit->PlayRetreatVoice();
	}
}

// 41A4DF
void BigStruct2::SendGroupGather() {
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_GATHER;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected() && (((CUnit*)obj)->unitFlags & UNIT_FLAG_PLAYER_CLASS)) {
			pkt->AppendWord(obj->unit_id);
			break;
		}
	}
	g_NetStru1_local.QueuePacketSend(pkt);
	CUnit* unit = this->PickVoiceUnit();
	if (unit != nullptr) {
		unit->PlayPickupVoice();
	}
}

// 4184B8
int32_t BigStruct2::ScreenToTileRow(int32_t x, int32_t y) {
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
uint32_t BigStruct2::UpdateMouseHover() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	uint32_t avail = 0;
	CUnit* old_unit = this->hovered_unit;
	uint16_t old_id = (uint16_t)this->hovered_unit_id;
	this->hovered_unit = nullptr;
	this->hovered_unit_id = 0;
	this->quest_some_id_2 = 0;
	this->quest_landmark_some_id = 0;
	this->quest_building_some_id = 0;
	this->quest_some_id = 0;

	CRect screen_rect;
	this->ClientRectToScreen(&screen_rect, &this->rect);
	CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
	if (!screen_rect.PtInRect(mouse_pt)) {
		if (this->hovered_unit != old_unit) {
			wnd->vis_charinfo->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
			wnd->vis_sidestatus->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
		}
		return avail;
	}

	CRect mouse_rect(g_mousept.x - 1, g_mousept.y - 1, g_mousept.x + 1, g_mousept.y + 1);
	for (int32_t i = this->hit_count - 1; i >= 0; i--) {
		CRect unit_rect = this->hit_rects[i];
		unit_rect.NormalizeRect();
		CRect intersect_rect;
		if (!intersect_rect.IntersectRect(&unit_rect, &mouse_rect)) {
			continue;
		}
		CGameObject* obj;
		if (!this->objects.Lookup(this->hit_unit_ids[i], obj)) {
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
		this->hovered_unit = (CUnit*)obj;
		if (wnd->sessionMode != SESSION_LOCAL_BATTLE) {
			for (POSITION it = this->quest_map->quests_map.GetStartPosition(); it != nullptr;) {
				uint32_t quest_id;
				Quest* quest;
				this->quest_map->quests_map.GetNextAssoc(it, quest_id, quest);
				if (this->hovered_unit->unit_id == quest->GetObj() && quest->IsInWork() != 0) {
					if (quest->Kind() != 3 && quest->Kind() != 0xC) {
						this->quest_some_id_2 = quest->GetSomeId();
					}
				} else if (this->hovered_unit->unit_id == (quest->GetLandmarkId() & 0xFFFF)) {
					this->quest_landmark_some_id = quest->GetSomeId();
				}
				if (this->hovered_unit->unit_id == quest->GetBuildingId()) {
					this->quest_building_some_id = quest->GetSomeId();
				}
				if (this->hovered_unit->IsKindOf(RUNTIME_CLASS(CUnit)) && quest->IsInWork() != 0) {
					if (this->hovered_unit->questFlags == quest->GetObj() && (quest->Kind() == 3 || quest->Kind() == 0xC)) {
						this->quest_some_id = quest->GetSomeId();
					}
					if (this->hovered_unit->typeId == (quest->GetObj() & 0xFF) && this->hovered_unit->face == (quest->GetObj() >> 8) && quest->Kind() == 2) {
						this->quest_some_id_2 = quest->GetSomeId();
					}
					if (this->hovered_unit->map_player->index == quest->GetObj() && quest->Kind() == 0xD) {
						this->quest_some_id_2 = quest->GetSomeId();
					}
				}
			}
		}
		this->hovered_unit_id = this->hit_unit_ids[i];
		break;
	}

	CVisualObject* child2 = this->FindChild(MAP_CHILD_BAG);
	CVisualObject* child3 = this->FindChild(MAP_CHILD_SPELL_BOOK);
	if (child2 != nullptr) {
		CPoint pt(g_mousept.GetX(), g_mousept.GetY());
		if (child2->GetRect().PtInRect(pt)) {
			this->hovered_unit = nullptr;
			this->hovered_unit_id = 0;
		}
	}
	if (child3 != nullptr) {
		CPoint pt(g_mousept.GetX(), g_mousept.GetY());
		if (child3->GetRect().PtInRect(pt)) {
			this->hovered_unit = nullptr;
			this->hovered_unit_id = 0;
		}
	}

	int32_t tile_x = g_mousept.x >> 5;
	int32_t tile_y = this->ScreenToTileRow(g_mousept.x, g_mousept.y);
	uint16_t* land = this->scenario->GetLandscape();
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
	if (this->hovered_unit != old_unit) {
		wnd->vis_charinfo->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
		wnd->vis_sidestatus->MsgProc(VIS_MSG_DATA_REFRESH, 0, 0);
		if (this->hovered_unit != nullptr) {
			this->hovered_unit->m_bSelectionDirty = 1;
		}
		CGameObject* prev_obj = old_unit;
		if (this->objects.Lookup(old_id, prev_obj)) {
			prev_obj->m_bSelectionDirty = 1;
		}
	}
	this->last_mouse_tile_x = tile_x;
	this->last_mouse_tile_y = tile_y;
	return avail;
}

// 40B314
// client: 40b35f
void BigStruct2::UpdateMapCursor() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->dialogsMask != DIALOG_MASK_INGAME) {
		return;
	}
	CSprite256* cursor_sprite = g_mousept.GetCursorSprite();
	CRect screen_rect;
	this->ClientRectToScreen(&screen_rect, &this->rect);
#ifdef A2CLIENT
	if (g_ScreenSize.bottom == 600) {
		screen_rect.bottom = 600;
	}
#endif
	CCursor* cursor = nullptr;
	CPoint mouse_pt(g_mousept.GetX(), g_mousept.GetY());
	CRect select_frame = g_mousept.GetSelectFrame();
	select_frame.NormalizeRect();
	if (!screen_rect.PtInRect(mouse_pt)) {
		this->UpdateMouseHover();
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
				if (this->order_mode != 0) {
					switch (this->order_mode) {
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
					uint32_t avail = this->UpdateMouseHover();
					if ((avail & 0x400) != 0 && cursor == g_Cursors[CURSOR_CAST]) {
						cursor = g_Cursors[CURSOR_MOVE];
					}
					if (g_mousept.GetSelectState() != 0 && !select_frame.IsRectEmpty()) {
						cursor = g_Cursors[CURSOR_DEFAULT];
					}
				} else if (this->IsBookOpen() != 0 && wnd->vis_spellbook->GetActiveSlot() >= 0 &&
					(this->selected_available_spell_mask != 0 || this->active_spell_effect_mask != 0) && (this->selection_flags & 4) == 0) {
					uint32_t avail = this->UpdateMouseHover();
					cursor = g_Cursors[CURSOR_CAST];
					if ((avail & 0x400) != 0) {
						cursor = g_Cursors[CURSOR_MOVE];
					}
					if ((avail & 3) == 0 && wnd->vis_spellbook->NeedsUnitTarget() != 0) {
						cursor = g_Cursors[CURSOR_MOVE];
					}
					int32_t pickup_flag = (this->selected_object_count == 1 && (this->selection_flags & 1) != 0) ? 1 : 0;
					int32_t town_flag = pickup_flag;
					if (pickup_flag != 0) {
						pickup_flag = ((CUnit*)this->primary_selected_object)->unitFlags & UNIT_FLAG_PLAYER_CLASS;
						if (pickup_flag != 0) {
							pickup_flag = (avail == 0x40 || (this->hovered_unit == this->primary_selected_object && (avail & 0x40) != 0)) ? 1 : 0;
						}
					}
					if (town_flag != 0) {
						town_flag = ((((CUnit*)this->primary_selected_object)->unitFlags & UNIT_FLAG_PLAYER_CLASS) != 0 && (avail & 0x800) != 0) ? 1 : 0;
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
					if (cursor == g_Cursors[CURSOR_CAST] && wnd->vis_spellbook->NeedsUnitTarget() != 0 && (avail & 4) == 0 &&
						wnd->vis_spellbook->HasSelectedSpell() == 0 && g_SpellOffensive[wnd->vis_spellbook->GetActiveSlot()] != 0) {
						if ((avail & 3) != 0) {
							cursor = g_Cursors[CURSOR_SELECT];
						} else {
							cursor = g_Cursors[CURSOR_MOVE];
						}
					}
					if (town_flag != 0 && (avail & 4) == 0) {
						cursor = g_Cursors[CURSOR_TOWN];
					}
					this->UpdateMouseHover();
				} else {
					uint32_t avail = this->UpdateMouseHover();
					if (this->selected_object_count == 0) {
						if ((avail & 0x23) != 0) {
							cursor = g_Cursors[CURSOR_SELECT];
						} else {
							cursor = g_Cursors[CURSOR_DEFAULT];
						}
					} else if ((this->selection_flags & 0x24) != 0) {
						if ((avail & 0x23) != 0) {
							cursor = g_Cursors[CURSOR_SELECT];
						} else {
							cursor = g_Cursors[CURSOR_DEFAULT];
						}
					} else {
						int32_t pickup_flag = (this->selected_object_count == 1 && (this->selection_flags & 1) != 0) ? 1 : 0;
						int32_t town_flag = pickup_flag;
						if (pickup_flag != 0) {
							pickup_flag = ((CUnit*)this->primary_selected_object)->unitFlags & UNIT_FLAG_PLAYER_CLASS;
							if (pickup_flag != 0) {
								pickup_flag = (avail == 0x40 || (this->hovered_unit == this->primary_selected_object && (avail & 0x40) != 0)) ? 1 : 0;
							}
						}
						if (town_flag != 0) {
							town_flag = ((((CUnit*)this->primary_selected_object)->unitFlags & UNIT_FLAG_PLAYER_CLASS) != 0 && (avail & 0x800) != 0) ? 1 : 0;
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
	CVisualObject* child2 = this->FindChild(MAP_CHILD_BAG);
	CVisualObject* child3 = this->FindChild(MAP_CHILD_SPELL_BOOK);
	if (child2 != nullptr && child2->GetRect().PtInRect(mouse_pt)) {
		cursor = g_Cursors[CURSOR_DEFAULT];
	}
	if (child3 != nullptr && child3->GetRect().PtInRect(mouse_pt)) {
		cursor = g_Cursors[CURSOR_DEFAULT];
	}
	if (wnd->drag_entry != nullptr) {
		uint32_t avail = this->UpdateMouseHover();
		cursor = wnd->item_cursor;
		if (wnd->dialogsMask == DIALOG_MASK_INGAME && this->selected_object_count == 1 &&
			(((CUnit*)this->primary_selected_object)->unitFlags & UNIT_FLAG_PLAYER_CLASS) != 0 &&
			this->primary_selected_object->map_player == this->my_main_unit) {
			int32_t tile_x = (g_mousept.GetX() >> 5) + this->view_x;
			int32_t tile_y = this->ScreenToTileRow(g_mousept.GetX(), g_mousept.GetY()) + this->view_y;
			if (abs(tile_x - this->primary_selected_object->tileX) <= 2 &&
				abs(tile_y - this->primary_selected_object->tileY) <= 2 && (avail & 1) != 0) {
				cursor = g_Cursors[CURSOR_BACKPACK];
			}
		}
	}
	if (cursor != nullptr && cursor_sprite != cursor->GetSprite()) {
		cursor->Use();
	}
}

// 404E1A
void BigStruct2::DrawTerrainFull() {
	AfxGetMainWnd();
	timeGetTime();
	//this->field_0x80->GetMapHeights();
	uint8_t* f18map = this->scenario->GetOverlayMap();
	uint16_t* landscape = this->scenario->GetLandscape();
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
				int32_t frame = (ter_type + (tile_x + 1) * tile_y + (this->anim_tick >> 2)) & 3;
				if (g_Animation == 0) {
					frame = 0;
				}
				ter_type = frame + 8;
			}
			CGameBitmap* tile = g_ter_tiles(ter_var, ter_type);
			if ((landscape[cell] & 0x2000) != 0 && !animated) {
				memcpy(this->scratch_tile->GetData(), (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400, 0x400);
				CopyIndexedSkip0((uint8_t*)this->scratch_tile->GetData(),
					(uint8_t*)g_TerDirtBmp->GetData() + ((tile_x + tile_y * 5) & 3) * 0x400, 0x400);
				tile = this->scratch_tile;
				tile_id &= 0xFFF0;
			}
			int32_t l1 = f18map[cell];
			int32_t l2 = f18map[cell + 1];
			int32_t l3 = f18map[cell2];
			int32_t l4 = f18map[cell2 + 1];
			uint16_t* pal = g_TerPalettes[ter_type]->GetPalette(0);
			uint8_t* tile_data = (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400;
			if (h1 == h2 && h3 == h4 && h1 + 0x20 == h3) {
				DrawFlatTile(x * 0x20, h1, l1, l2, l3, l4, tile_data, pal);
			} else {
				DrawDeformedTile(x * 0x20, (x + 1) * 0x20, h1, h2, h3, h4, l1, l2, l3, l4, tile_data, pal);
			}
			if (g_TerrainHidden != 0) {
				FillRectColor(x * 0x20, h1, (x + 1) * 0x20, h2, 0);
				FillRectColor(x * 0x20, h1, x * 0x20, h3, 0);
			}
		}
	}
}

// 405399
void BigStruct2::DrawTerrainAnimated(int32_t arg) {
	//this->field_0x80->GetMapHeights();
	uint8_t* f18map = this->scenario->GetOverlayMap();
	uint16_t* landscape = this->scenario->GetLandscape();
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
			int32_t frame = (ter_type + tile_x + tile_x * tile_y + (this->anim_tick >> 2)) & 3;
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
			uint16_t* pal = g_TerPalettes[ter_type]->GetPalette(0);
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
void BigStruct2::DrawTerrainScrolled(int32_t arg) {
	AfxGetMainWnd();
	timeGetTime();
	//this->field_0x80->GetMapHeights();
	uint8_t* f18map = this->scenario->GetOverlayMap();
	uint16_t* landscape = this->scenario->GetLandscape();
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
				int32_t frame = (ter_type + (tile_x + 1) * tile_y + (this->anim_tick >> 2)) & 3;
				if (g_Animation == 0) {
					frame = 0;
				}
				ter_type = frame + 8;
			}
			CGameBitmap* tile = g_ter_tiles(ter_var, ter_type);
			if ((landscape[cell] & 0x2000) != 0 && !animated) {
				memcpy(this->scratch_tile->GetData(), (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400, 0x400);
				CopyIndexedSkip0((uint8_t*)this->scratch_tile->GetData(),
					(uint8_t*)g_TerDirtBmp->GetData() + ((tile_x + tile_y * 5) & 3) * 0x400, 0x400);
				tile = this->scratch_tile;
				tile_id &= 0xFFF0;
			}
			int32_t l1 = f18map[cell];
			int32_t l2 = f18map[cell + 1];
			int32_t l3 = f18map[cell2];
			int32_t l4 = f18map[cell2 + 1];
			uint16_t* pal = g_TerPalettes[ter_type]->GetPalette(0);
			uint8_t* tile_data = (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400;
			if (h1 == h2 && h3 == h4 && h1 + 0x20 == h3) {
				DrawFlatTile(map_x * 0x20, h1, l1, l2, l3, l4, tile_data, pal);
			} else {
				DrawDeformedTile(map_x * 0x20, (map_x + 1) * 0x20, h1, h2, h3, h4, l1, l2, l3, l4, tile_data, pal);
			}
			if (g_TerrainHidden != 0) {
				FillRectColor(map_x * 0x20, h1, (map_x + 1) * 0x20, h2, 0);
				FillRectColor(map_x * 0x20, h1, map_x * 0x20, h3, 0);
			}
		}
	}
}

// 405D0E
void BigStruct2::RedrawLitTerrain() {
	AfxGetMainWnd();
	//this->field_0x80->GetMapHeights();
	uint8_t* f18map = this->scenario->GetOverlayMap();
	uint16_t* landscape = this->scenario->GetLandscape();
	CRect rect;
	this->ClientRectToScreen(&rect, this->rect);
	for (int32_t y = 0; y < this->view_tiles_y + 4; y++) {
		for (int32_t x = this->view_tiles_x - 1; x >= 0; x--) {
			int32_t vis_idx = x + 3 + (y + 3) * (this->view_tiles_x + 7);
			int32_t vis_idx2 = vis_idx + this->view_tiles_x + 7;
			int32_t l1 = this->corner_light[vis_idx];
			int32_t l2 = this->corner_light[vis_idx + 1];
			int32_t l3 = this->corner_light[vis_idx2];
			int32_t l4 = this->corner_light[vis_idx2 + 1];
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
				int32_t frame = (ter_type + (tile_x + 1) * tile_y + (this->anim_tick >> 2)) & 3;
				if (g_Animation == 0) {
					frame = 0;
				}
				ter_type = frame + 8;
			}
			CGameBitmap* tile = g_ter_tiles(ter_var, ter_type);
			if ((landscape[cell] & 0x2000) != 0 && !animated) {
				memcpy(this->scratch_tile->GetData(), (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400, 0x400);
				CopyIndexedSkip0((uint8_t*)this->scratch_tile->GetData(),
					(uint8_t*)g_TerDirtBmp->GetData() + ((tile_x + tile_y * 5) & 3) * 0x400, 0x400);
				tile = this->scratch_tile;
				tile_id &= 0xFFF0;
			}
			uint16_t* pal = g_TerPalettes[ter_type]->GetPalette(0);
			uint8_t* tile_data = (uint8_t*)tile->GetData() + (tile_id & 0xF) * 0x400;
			if (h1 == h2 && h3 == h4 && h1 + 0x20 == h3) {
				DrawFlatTile(x * 0x20, h1, l1, l2, l3, l4, tile_data, pal);
			} else {
				DrawDeformedTile(x * 0x20, (x + 1) * 0x20, h1, h2, h3, h4, l1, l2, l3, l4, tile_data, pal);
			}
			if (g_TerrainHidden != 0) {
				FillRectColor(x * 0x20, h1, (x + 1) * 0x20, h2, 0);
				FillRectColor(x * 0x20, h1, x * 0x20, h3, 0);
			}
		}
	}
}

// 406F7B
void BigStruct2::RenderWorldFrame() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->dialogsMask != DIALOG_MASK_INGAME || this->scenario == nullptr) {
		return;
	}

	CRect clip_rect;
	CRect local_rect;
	CDWordArray dummy_arr;

	this->gap_0xa90++;
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
	this->RebuildWorldGrids();
	this->dirty_rect.IntersectRect(&this->dirty_rect, &local_rect);
	if (view_moved != 0) {
		this->dirty_rect = local_rect;
	}
	if (this->full_redraw_requested != 0) {
		this->dirty_rect = local_rect;
		this->full_redraw_requested = 0;
	}
	uint32_t anim_redraw = this->anim_tick - this->anim_redraw_tick > 3;
	if (anim_redraw != 0) {
		this->anim_redraw_tick = this->anim_tick;
	}
	VisInvBase* child2 = (VisInvBase*)this->FindChild(MAP_CHILD_BAG);
	CVisualObject* child3 = this->FindChild(MAP_CHILD_SPELL_BOOK);
	CRect childs_rect(0, 0, 0, 0);
	int32_t childs_size = (int32_t)child2 + (int32_t)child3 + this->msglog.text.GetSize();
	if (child2 != nullptr) {
		childs_rect |= child2->GetRect();
	}
	if (child3 != nullptr) {
		childs_rect |= child3->GetRect();
	}
	if (childs_size != this->cached_msg_count) {
		this->dirty_rect = local_rect;
	}
	if (child2 != nullptr && child2->HasAnimatingCells() != 0) {
		this->dirty_rect = local_rect;
	}
	g_mousept.Update();
	LockSurface3();
	GetClipRect(&clip_rect);
	if (this->force_full_redraw != 0) {
		this->DrawTerrainFull();
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
					this->DrawTerrainScrolled(-this->view_tiles_y);
				}
				if (!rc2.IsRectEmpty()) {
					SetClipRect(rc2);
					this->DrawTerrainScrolled(0);
				}
			} else {
				rc3.SetRect(0, rc1.TopLeft().y, rc1.TopLeft().x, rc1.BottomRight().y);
				if (!rc3.IsRectEmpty()) {
					SetClipRect(rc3);
					this->DrawTerrainScrolled(-this->view_tiles_y);
				}
				rc3.SetRect(rc1.BottomRight().x, rc1.TopLeft().y, this->view_tiles_x << 5, rc1.BottomRight().y);
				if (!rc3.IsRectEmpty()) {
					SetClipRect(rc3);
					this->DrawTerrainScrolled(-this->view_tiles_y);
				}
				rc3.SetRect(0, rc2.TopLeft().y, rc2.TopLeft().x, rc2.BottomRight().y);
				if (!rc3.IsRectEmpty()) {
					SetClipRect(rc3);
					this->DrawTerrainScrolled(0);
				}
				rc3.SetRect(rc2.BottomRight().x, rc2.TopLeft().y, this->view_tiles_x << 5, rc2.BottomRight().y);
				if (!rc3.IsRectEmpty()) {
					SetClipRect(rc3);
					this->DrawTerrainScrolled(0);
				}
			}
		}
		if (this->scroll_dy != 0) {
			rc1.SetRect(0, old_ty << 5, this->view_tiles_x << 5, this->tile_scroll_y << 5);
			rc1.NormalizeRect();
			if (abs(this->tile_scroll_y - old_ty) < this->view_tiles_y / 2) {
				SetClipRect(rc1);
				if (this->scroll_dy < 0) {
					this->DrawTerrainScrolled(0);
				} else {
					this->DrawTerrainScrolled(-this->view_tiles_y);
				}
			} else {
				rc2.SetRect(0, 0, this->view_tiles_x << 5, rc1.TopLeft().y);
				if (!rc2.IsRectEmpty()) {
					SetClipRect(rc2);
					this->DrawTerrainScrolled(-this->view_tiles_y);
				}
				rc2.SetRect(0, rc1.BottomRight().y, this->view_tiles_x << 5, this->view_tiles_y << 5);
				if (!rc2.IsRectEmpty()) {
					SetClipRect(rc2);
					this->DrawTerrainScrolled(0);
				}
			}
		}
	} else {
		if (anim_redraw != 0 && g_Animation != 0) {
			SetClipRect(0, this->tile_scroll_y << 5, this->view_tiles_x << 5, this->view_tiles_y << 5);
			this->DrawTerrainAnimated(0);
			SetClipRect(0, 0, this->view_tiles_x << 5, this->tile_scroll_y << 5);
			this->DrawTerrainAnimated(-this->view_tiles_y);
		}
	}
	SetClipRect(clip_rect);
	UnlockSurface3();
	g_mousept.Update();
	SetClipRect(this->dirty_rect);
	CopyScreenBlock(this->tile_scroll_x, this->tile_scroll_y, this->view_tiles_x << 5, this->view_tiles_y << 5);
	SetClipRect(clip_rect);
	g_mousept.Update();
	LockSurface2();
	GetClipRect(&clip_rect);
	SetClipRect(this->dirty_rect);
	if (this->lightning_active != 0 && (g_VideoOptionFlags & 2) == 0 && g_Lightning != 0) {
		this->RedrawLitTerrain();
	}
	int32_t tick = this->anim_tick;
	uint8_t* gfx_cells = this->scenario->GetObjectMap();
	uint8_t* f18map = this->scenario->GetOverlayMap();
	int8_t* hmap = this->scenario->GetMapHeights();
	uint16_t* landscape = this->scenario->GetLandscape();
	this->hit_count = 0;

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
						this->AddHitRect(&sel, obj94->unit_id);
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
						this->AddHitRect(&sel, bld->unit_id);
					}
				}
				if (unit != nullptr && unit->IsBlocked() == 0 &&
					(unit->FindEffectTimer(0x20) < 0 || this->my_main_unit->IsVisibleTo(unit->map_player->index) != 0)) {
					UnitVFXUnfo* vfx = g_VFX_info[unit->typeId];
					CRect sel = vfx->selection;
					sel.OffsetRect(unit->centerScreenX - vfx->center_x,
						(unit->centerScreenY - vfx->center_y) - unit->terrainHeightOffset - unit->z_pos);
					if (unit->death_state < 2) {
						this->AddHitRect(&sel, unit->unit_id);
					}
					if (g_Shadows != 0) {
						unit->DrawShadow(x, y, light);
					}
					unit->Draw(x, y, light);
				}
				if (g_Lightning != 0) {
					uint16_t lkey = (x + this->view_x) | ((y + this->view_y) << 8);
					uint32_t lval;
					if (this->lightning_cells.Lookup(lkey, lval)) {
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
				int32_t tanval = (int32_t)(tan(this->scenario->GetRenderSunAngle()) * 65536.0);
				uint32_t hgt2 = this->height_cells[x + 4 + (y + 4) * (this->view_tiles_x + 8)];
				gfxb--;
				if (this->IsGfxObjectHidden(gfxb, x, y, hgt2) == 0) {
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
					int32_t yoff = (int32_t)(tan(this->scenario->GetRenderSunAngle()) *
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
					if (this->lightning_cells.Lookup(fkey, fval)) {
						ProjectileInfo* proj = g_ProjectileInfos[0xF];
						proj->GetSprite()->Draw((x * 0x20 + 0x10) - proj->width / 2,
							((y * 0x20 + 0x10) - hgt2) - proj->height / 2, (int32_t)fval / 2, 0, 0);
					}
				}
			}
			if (y > -4 && y < this->view_tiles_y + 7 && x > -4 && x < this->view_tiles_x + 3) {
				uint16_t tkey = (this->view_x + x) | ((this->view_y + y) << 8);
				uint32_t tval;
				if (this->terrain_light_override_cells.Lookup(tkey, tval) && tval != 0 && lflags == 0xC000) {
					int32_t tidx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
					this->DrawTileOverlayFx(x * 0x20 + 0x10,
						(int32_t)(this->tile_top_y[tidx] + this->tile_bottom_y[tidx]) / 2, tval, 1);
				}
			}
		}
	}
	SetClipRect(clip_rect);
	UnlockSurface2();
	g_mousept.Update();
	LockSurface2();
	SetClipRect(this->dirty_rect);

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

	POSITION it = this->projectiles.GetStartPosition();
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->projectiles.GetNextAssoc(it, key, obj);
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
				if (this->terrain_light_override_cells.Lookup(tkey, tval) && tval != 0 && lflags == 0xC000) {
					int32_t tidx = x + 3 + (y + 3) * (this->view_tiles_x + 6);
					this->DrawTileOverlayFx(x * 0x20 + 0x10,
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
						this->AddHitRect(&sel, obj->unit_id);
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
	SetClipRect(this->dirty_rect);

	for (int32_t x = 0; x < this->view_tiles_x; x++) {
		int32_t y = 0;
		while (y < this->view_tiles_y + 4) {
			int32_t stride = this->view_tiles_x + 7;
			int32_t idx = x + 3 + (y + 3) * stride;
			uint32_t t0 = this->corner_tile_kind[idx];
			uint32_t t1 = this->corner_tile_kind[idx + 1];
			uint32_t t2 = this->corner_tile_kind[idx + stride];
			uint32_t t3 = this->corner_tile_kind[idx + 1 + stride];
			uint32_t t4 = this->corner_tile_kind[idx + stride * 2];
			uint32_t t5 = this->corner_tile_kind[idx + 1 + stride * 2];
			uint32_t h0 = this->tile_corner_y[idx];
			uint32_t h1 = this->tile_corner_y[idx + 1];
			uint32_t h2 = this->tile_corner_y[idx + stride];
			uint32_t h3 = this->tile_corner_y[idx + 1 + stride];
			for (; t0 == t2 && t1 == t3 && t2 == t4 && t3 == t5 && y != this->view_tiles_y + 3; y++) {
				idx += stride;
				h2 = this->tile_corner_y[idx + stride];
				h3 = this->tile_corner_y[idx + 1 + stride];
				t4 = this->corner_tile_kind[idx + stride * 2];
				t5 = this->corner_tile_kind[idx + 1 + stride * 2];
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
	SetClipRect(this->dirty_rect);

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
	uint32_t top_panel_kind = (uint32_t)(this->selected_object_count == 0) | (this->selection_flags & 0x24);
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
	this->fps_frames++;
	uint32_t now = timeGetTime();
	if (this->fps_last_tick == 0) {
		this->fps_last_tick = now;
	}
	this->fps_time_ms += now - this->fps_last_tick;
	this->fps_last_tick = now;
	if ((uint32_t)this->fps_time_ms > 1000) {
		this->fps = ((double)(uint32_t)this->fps_frames * 1000.0) / (double)(uint32_t)this->fps_time_ms;
		this->fps_time_ms -= 1000;
		this->fps_frames = 0;
	}
	if (g_ShowFps != 0) {
		FillRectColorSimple(local_rect.right - 0x78, local_rect.top, local_rect.right - 0x1e, local_rect.top + 0x18,
			GetColorRGB(8, 8, 8));
		char buf[32];
		sprintf(buf, "%3.1f fps", this->fps);
		g_font1->DrawTextWithShadow(local_rect.right - 0x26, local_rect.top, buf, 1, clrsh_ShockingBlack, 1);
	}
	if (wnd->sessionMode == SESSION_SINGLEPLAYER && g_ShowNetStats != 0) {
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
	this->UpdateMapCursor();
	if (childs_size == this->cached_msg_count) {
		this->dirty_rect.SubtractRect(&this->dirty_rect, &childs_rect);
		if (!childs_rect.IsRectEmpty()) {
			gfxFlushRect(childs_rect);
		}
	}
	this->cached_msg_count = childs_size;
	gfxFlushRect(this->dirty_rect);
	CRect fps_area;
	CRect fps_flush(0x168, 0, 0x1c2, 0x18);
	CRect net_flush(300, 0x1e, 0x1c2, 0x69);
	if (g_ShowFps != 0) {
		gfxFlushRect(fps_flush);
	}
	if (g_ShowNetStats != 0 && wnd->sessionMode == SESSION_SINGLEPLAYER) {
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
	this->RenderWorldFrame();
}

// 40C60F
int32_t BigStruct2::OnLButtonDblClk(uint32_t wparam, CPoint pos) {
	return this->OnLButtonDown(wparam, pos);
}

// 402af6 (client: 402af6 — only the CVisualObject id differs, see MAP_CONTEXT_ID)
BigStruct2::BigStruct2(int32_t l, int32_t t, int32_t r, int32_t b) : CVisualObject(MAP_CONTEXT_ID, l, t, r, b, nullptr) {
	this->scenario = nullptr;
	this->AllocTileGrids();
	this->view_x = this->MapMinX();
	this->view_y = this->MapMinY();
	this->fps_frames = 0;
	this->fps_time_ms = 0;
	this->fps_last_tick = 0;
	this->fps = 0.0;
	this->hit_rects.SetSize(0x40, 0x20);
	this->hit_unit_ids.SetSize(0x40, 0x20);
	this->hit_count = 0;
	this->selected_object_count = 0;
	this->order_mode = 0;
	this->anim_tick = 0;
	this->anim_redraw_tick = 0;
	this->gap_0xa90 = 0;
	this->rbtn_drag = 0;
	this->minimap_dirty = 1;
	this->full_redraw_requested = 1;
	this->next_projectile_id = 0;
	this->ambient_view_x = 40000;
	this->ambient_view_y = 40000;
	this->ambient_sound_deadline = 0;
	this->tile_scroll_x = 0;
	this->tile_scroll_y = 0;
	this->prev_view_x = -1;
	this->prev_view_y = -1;
	this->cached_msg_count = 1;
	this->selected_available_spell_mask = 0;
	this->active_spell_effect_mask = 0;
	this->selected_active_spell_mask = 0;
	this->wimpy = 0;
	this->formation = 1;
	this->show_hp = 0;
	this->flying_hp = 1;
	this->scratch_tile = new CBmp256(0x20, 0x20);
	this->scenario = nullptr;
	this->shadow_shift = 7;
	this->shadow_scale = 1 << this->shadow_shift;
	this->BuildShadowTables();
	MapPlayerData* mpd = new MapPlayerData(0, 0xF);
	this->map_players.Add(mpd);
	this->map_players[0]->flags |= 1;
	this->my_unit = nullptr;
	this->my_main_unit = nullptr;
	this->hovered_unit = nullptr;
	this->hovered_unit_id = 0;
	this->quest_some_id_2 = 0;
	this->quest_landmark_some_id = 0;
	this->quest_building_some_id = 0;
	this->quest_some_id = 0;
	this->quest_map = new QuestMap();
	for (int32_t i = 0; i < 12; i++) {
		this->hero_equipment[i] = nullptr;
	}
#ifdef A2CLIENT
	// client: hat DM predefined item slots 12..31 (raw encoded ids, not pointers)
	this->hat_item_slot12 = 0;
	this->hat_item_slot13 = (int32_t)0xF0000001u;
	this->hat_item_slot14 = (int32_t)0xF0000001u;
	for (int32_t i = 0; i < 16; i++) {
		this->hat_cursor_slots[i] = (int32_t)0xF0000003u;
	}
	this->hat_player_count = (int32_t)0x7fffffffu;
#endif
}

// 40AFC5
void BigStruct2::DrawTileOverlayFx(int32_t x, int32_t y, uint32_t flags, int32_t flag) {
	if ((flags & 8) != 0 && flag != 0) {
		ProjectileInfo* proj = g_ProjectileInfos[0xF];
		int32_t phase = abs(this->anim_tick / 2 + x * y) % 5 + 3;
		proj->GetSprite()->Draw(x - proj->width / 2, y - proj->height / 2, phase, 0, 0);
	}
	if ((flags & 0x40) != 0 && flag == 0) {
		ProjectileInfo* proj = g_ProjectileInfos[0x15];
		int32_t phase = abs(this->anim_tick / 2 + x * y) % proj->phases;
		proj->GetSprite()->Draw(x - proj->width / 2, y - proj->height / 2, phase, 0, 0);
	} else if ((flags & 0x20000) != 0 && flag != 0) {
		ProjectileInfo* proj = g_ProjectileInfos[0x2B];
		int32_t phase = abs(this->anim_tick / 2 + x * y) % proj->phases;
		proj->GetSprite()->Draw(x - proj->width / 2, y - proj->height / 2, phase, 0, 0);
	}
}

// 40B173
int32_t BigStruct2::IsGfxObjectHidden(int32_t gfx_id, int32_t x, int32_t y, int32_t height) {
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
	int32_t min_ty = this->ScreenToTileRow(x * 0x20 + 0x10, ((y * 0x20 + 0x10) - cy) - height);
	int32_t max_ty = this->ScreenToTileRow(x * 0x20 + 0x10,
		(((y * 0x20 + 0x10) - cy) - height) + g_GfxFiles[gobj->file]->GetSprite()->GetHeight(0)) + 1;
	if (min_ty < 0) {
		min_ty = 0;
	}
	if (max_ty > this->view_tiles_y + 4) {
		max_ty = this->view_tiles_y + 4;
	}
	for (int32_t tx = min_tx; tx <= max_tx; tx++) {
		for (int32_t ty = min_ty; ty <= max_ty; ty++) {
			if (this->corner_tile_kind[tx + 3 + (ty + 3) * (this->view_tiles_x + 7)] != 0x10) {
				return 0;
			}
		}
	}
	return 1;
}

// 402FB8
void BigStruct2::AllocTileGrids() {
	this->view_tiles_x = this->rect.Width() / 32;
	this->view_tiles_y = this->rect.Height() / 32;
	if (this->scenario != nullptr) {
		if (this->view_x + this->view_tiles_x >= this->scenario->GetWidth() - 8) {
			this->view_x = (this->scenario->GetWidth() - 8) - this->view_tiles_x;
		}
		if (this->view_y + this->view_tiles_y >= this->scenario->GetHeight() - 8) {
			this->view_y = (this->scenario->GetHeight() - 8) - this->view_tiles_y;
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
	this->corner_tile_kind = (uint32_t*)malloc(this->corner_grid_bytes);
	this->corner_tile_kind_prev = (uint32_t*)malloc(this->corner_grid_bytes);
	this->corner_light = (uint8_t*)malloc(this->corner_grid_bytes / 4);
	this->corner_light_prev = (uint8_t*)malloc(this->corner_grid_bytes / 4);
	memset(this->corner_light_prev, 0, this->corner_grid_bytes / 4);
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
void BigStruct2::BuildShadowTables() {
	for (int32_t i = 0; i <= 0x14; i++) {
		for (int32_t j = 0; j <= 0x14; j++) {
			if (j < i / 2) {
				this->shadow_step[i + 0x14][j + 0x14][0] = -1;
				this->shadow_step[i + 0x14][j + 0x14][1] = 0;
				this->shadow_step[0x14 - i][0x14 - j][0] = 1;
				this->shadow_step[0x14 - i][0x14 - j][1] = 0;
				this->shadow_step[i + 0x14][0x14 - j][0] = -1;
				this->shadow_step[i + 0x14][0x14 - j][1] = 0;
				this->shadow_step[0x14 - i][j + 0x14][0] = 1;
				this->shadow_step[0x14 - i][j + 0x14][1] = 0;
			} else if (j > 2 * i) {
				this->shadow_step[i + 0x14][j + 0x14][0] = 0;
				this->shadow_step[i + 0x14][j + 0x14][1] = -1;
				this->shadow_step[0x14 - i][0x14 - j][0] = 0;
				this->shadow_step[0x14 - i][0x14 - j][1] = 1;
				this->shadow_step[i + 0x14][0x14 - j][0] = 0;
				this->shadow_step[i + 0x14][0x14 - j][1] = 1;
				this->shadow_step[0x14 - i][j + 0x14][0] = 0;
				this->shadow_step[0x14 - i][j + 0x14][1] = -1;
			} else {
				this->shadow_step[i + 0x14][j + 0x14][0] = -1;
				this->shadow_step[i + 0x14][j + 0x14][1] = -1;
				this->shadow_step[0x14 - i][0x14 - j][0] = 1;
				this->shadow_step[0x14 - i][0x14 - j][1] = 1;
				this->shadow_step[i + 0x14][0x14 - j][0] = -1;
				this->shadow_step[i + 0x14][0x14 - j][1] = 1;
				this->shadow_step[0x14 - i][j + 0x14][0] = 1;
				this->shadow_step[0x14 - i][j + 0x14][1] = -1;
			}
			uint16_t val = (std::sqrt(i * i + j * j) * (double)this->shadow_scale / (double)(std::max)(i, j));
			this->shadow_ray_len[i + 0x14][j + 0x14] = val;
			this->shadow_ray_len[0x14 - i][0x14 - j] = val;
			this->shadow_ray_len[i + 0x14][0x14 - j] = val;
			this->shadow_ray_len[0x14 - i][j + 0x14] = val;
		}
	}
	this->shadow_step[0x15][0x14][0] = -1;
	this->shadow_step[0x15][0x14][1] = 0;
	this->shadow_step[0x13][0x14][0] = 1;
	this->shadow_step[0x13][0x14][1] = 0;
}

// 403CA0
void BigStruct2::BuildShadowField(CGameObject* obj) {
	int32_t base_x = obj->tileX - 20;
	int32_t base_y = obj->tileY - 20;
	int32_t base_idx = base_y * this->map_width + base_x;
	memset(this->shadow_light, 0, 0x1A44);
	this->shadow_light[20][20] = (obj->scan_range >> (8 - this->shadow_shift)) + (1 << (this->shadow_shift - 1));
	int8_t base_height = this->scenario->GetMapHeights()[obj->tileY * this->map_width + obj->tileX];
	for (int32_t r = 1; r < 0x14; r++) {
		int32_t ring_ok = 1;
		for (int32_t d = -r; d < r + 1; d++) {
			int32_t cx = d + 20;
			int32_t cy = 20 - r;
			if (this->scenario->IsInShadedBounds(base_x + cx, base_y + cy) != 0 && this->StepShadowRay(cx, cy, base_idx, base_height) == 0) {
				ring_ok = 0;
			}
			cx = d + 20;
			cy = r + 20;
			if (this->scenario->IsInShadedBounds(base_x + cx, base_y + cy) != 0 && this->StepShadowRay(cx, cy, base_idx, base_height) == 0) {
				ring_ok = 0;
			}
			cx = 20 - r;
			cy = d + 20;
			if (this->scenario->IsInShadedBounds(base_x + cx, base_y + cy) != 0 && this->StepShadowRay(cx, cy, base_idx, base_height) == 0) {
				ring_ok = 0;
			}
			cx = r + 20;
			cy = 20 - d;
			if (this->scenario->IsInShadedBounds(base_x + cx, base_y + cy) != 0 && this->StepShadowRay(cx, cy, base_idx, base_height) == 0) {
				ring_ok = 0;
			}
		}
		if (ring_ok != 0) {
			break;
		}
	}
}

// 41C74B
void BigStruct2::ApplyTileLightMask(int32_t x, int32_t y, int32_t w, int32_t h, int32_t unk1, uint8_t* dat, int32_t unk2) {
	uint8_t* overlay = this->scenario->GetOverlayMap();
	for (int32_t i = 0; i < w; i++) {
		for (int32_t j = 0; j < h; j++) {
			int32_t bit_index = j * w + i;
			if ((dat[bit_index >> 3] & (1 << (bit_index & 7))) == 0) {
				continue;
			}
			uint16_t packed = (x + i) | ((y + j) << 8);
			uint32_t value;
			if (this->terrain_light_override_cells.Lookup(packed, value) == 0) {
				value = 0;
			}
			if (unk2 != 0) {
				value |= 1 << unk1;
				this->terrain_light_override_cells.SetAt(packed, value);
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
					uint16_t* land = this->scenario->GetLandscape();
					if ((land[idx] & 0x2000) == 0) {
						uint8_t gfx_id = this->scenario->GetObjectMap()[idx];
						if (gfx_id != 0 && g_GfxObjects[gfx_id - 1]->in_map_editor != -1) {
							this->lightning_cells.SetAt(packed, zero_val);
						}
					}
					this->ambient_view_x++;
					land[idx] |= 0x2000;
				}
			} else {
				value &= ~(1 << unk1);
				if (value != 0) {
					this->terrain_light_override_cells.SetAt(packed, value);
				} else {
					this->terrain_light_override_cells.RemoveKey(packed);
				}
				if (unk1 == 3) {
					this->ambient_view_x++;
				} else if (unk1 == 0xE || unk1 == 0xF) {
					this->scenario->UpdateTerrainShading(x + i, y + j, 2, 2);
				}
			}
			this->force_full_redraw = 1;
		}
	}
}

// 41CB21
void BigStruct2::SendRequestMapInfo() {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = REQUEST_MAP_INFO;
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
	if ((wnd->dialogsMask & DIALOG_MASK_POPUP) != 0) {
		return 0;
	}
	return 1;
}

// 41CB67
void BigStruct2::SendRequestFileChunk(int32_t val) {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	pkt->id = REQUEST_FILE_CHUNK;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->param1 = val;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 40C510
int32_t BigStruct2::OnLButtonDown(uint32_t wparam, CPoint pos) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->dialogsMask != DIALOG_MASK_INGAME) {
		return 0;
	}
	if (g_mousept.GetSelectState() == 0 && this->scenario != nullptr) {
		wnd->UpdateMapRect();
		g_mousept.StartSelect(pos);
	}
	return 1;
}

// 40C59C
int32_t BigStruct2::OnRButtonUp(uint32_t wparam, CPoint pos) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (this->rbtn_drag == 0 && wnd->dialogsMask == DIALOG_MASK_INGAME) {
		this->MsgProc(VIS_MSG_CANCEL_ORDER, 0, 0);
	}
	this->rbtn_drag = 0;
	if (this->TestFlags(FLAG_OVERCURSOR) != 0) {
		this->SetCursorOver(false);
	}
	return 1;
}

// 41da52
CGameBitmap* BigStruct2::BuildUnitPortrait(int32_t serv_id) {
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
	CUnit* unit = this->FindOrCreateUnitByServerId(serv_id);
	if (unit == nullptr) {
		result = new CBmp64(0x58, 0x6c);
	} else {
		result = new CBmp64(0x58, 0x6c);
		CBmp64* pic = new CBmp64(0xa0, 0xf0);
		if ((unit->unitFlags & (UNIT_FLAG_PLAYER_CLASS | UNIT_FLAG_CREATURE)) == 0) {
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
		if (unit->unitFlags & UNIT_FLAG_REMOTE_HERO) {
			delete unit;
		}
	}
	return result;
}

// 41DF23
CUnit* BigStruct2::FindUnitByServerId(int32_t serv_id) {
	CUnit* result = nullptr;
	POSITION it = this->objects.GetStartPosition();
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
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
CUnit* BigStruct2::PickVoiceUnit() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->sessionMode != SESSION_LOCAL_BATTLE) {
		return nullptr;
	}
	if (g_settings.Acknowledgement == 0) {
		return nullptr;
	}
	CArray<CUnit*> primary;
	CArray<CUnit*> secondary;
	CArray<CUnit*> tertiary;
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (!obj->IsSelected() || obj->hp <= 0) {
			continue;
		}
		CUnit* unit = (CUnit*)obj;
		if ((unit->unitFlags & UNIT_FLAG_PLAYER_CLASS) != 0) {
			primary.Add(unit);
		} else if (primary.GetSize() == 0 && (unit->unitFlags & UNIT_FLAG_CREATURE) != 0) {
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
void BigStruct2::UpdateDayNightCycle(int32_t arg) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if ((wnd->dialogsMask & DIALOG_MASK_INGAME) == 0) {
		return;
	}
	int32_t time_of_day = ((uint32_t)wnd->serverLoopCounter >> 4) + 0x168;
	if (((wnd->serverLoopCounter & 0xF) == 0 && time_of_day % 20 == 0 && g_settings.ShowTimeFlow != 0) || arg != 0) {
		UpdateDayNightLighting(time_of_day);
		this->scenario->UpdateTerrainShading(1, 1, 0, 0);
		this->ApplyTerrainLightOverrides();
		ReloadPalettes();
		this->minimap_dirty = 1;
		this->force_full_redraw = 1;
		wnd->vis_minimap->MsgProc(VIS_MSG_MINIMAP_REBUILD, 0, 0);
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
				this->MsgProc(VIS_MSG_SCROLL_VIEW, this->view_x + cell_dx, this->view_y + cell_dy);
				this->rbtn_drag = 1;
			}
			return 1;
		}
	}
	return CVisualObject::OnMouseMove(wparam, pos);
}

// 41B7B7
void BigStruct2::ComputeSoundVolumePan(int32_t xpos, int32_t ypos, int32_t* pvol, int32_t* ppan) {
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
	if (wnd->dialogsMask == DIALOG_MASK_INGAME && this->scenario != nullptr) {
		if (wnd->console_active != 0) {
			wnd->vis_1200->OnKeyDown(wparam);
			return 0;
		}
		if (wparam >= 0x30 && wparam <= 0x39) {
			if (g_kbControlState == 0) {
				if (g_kbMenuState == 0) {
					this->SelectControlGroup(wparam - 0x30);
				} else {
					this->SelectControlGroupAndCenter(wparam - 0x30);
				}
			} else if (g_kbShiftState == 0) {
				this->AssignControlGroup(wparam - 0x30);
			} else {
				this->AddToControlGroup(wparam - 0x30);
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
			if (wnd->dialogsMask == DIALOG_MASK_INGAME) {
				this->ScrollMapX(-1);
			}
			break;
		case 0x26:
			if (wnd->dialogsMask == DIALOG_MASK_INGAME) {
				this->ScrollMapY(-1);
			}
			break;
		case 0x27:
			if (wnd->dialogsMask == DIALOG_MASK_INGAME) {
				this->ScrollMapX(1);
			}
			break;
		case 0x28:
			if (wnd->dialogsMask == DIALOG_MASK_INGAME) {
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
				wnd->vis_spellbook->MsgProc(VIS_MSG_SHORTCUT_KEY, wparam - 0x73, g_kbControlState);
			}
			if ((shortcut.kind != 1 || g_kbShiftState != 0) && g_kbControlState == 0) {
				wnd->vis_invtype1->MsgProc(VIS_MSG_SHORTCUT_KEY, wparam - 0x73, g_kbShiftState);
			}
			if (shortcut.kind == 1) {
				if (this->IsBookOpen() == 0 && wnd->vis_spellbook->IsSpellAvailable(shortcut.item_id) != 0) {
					this->SetOrderMode(9);
				} else if (this->IsBookOpen() == 0 && wnd->vis_spellbook->IsSpellEffectActive(shortcut.item_id) != 0) {
					this->SetOrderMode(10);
				}
			}
			break;
		}
		default:
			if (wnd->dialogsMask == DIALOG_MASK_INGAME) {
				if (g_kbControlState == 0 && this->selected_object_count != 0 && (this->selection_flags & 0x24) == 0) {
					switch (wparam) {
					case 0x41:
						this->SetOrderMode(1);
						return 1;
					case 0x43:
						if (this->IsBookOpen() == 0) {
							this->SetOrderMode(5);
						}
						return 1;
					case 0x44:
						this->SetOrderMode(4);
						return 1;
					case 0x47:
						this->SetOrderMode(3);
						return 1;
					case 0x4C:
						this->SendGroupStop();
						return 1;
					case 0x4D:
						this->SetOrderMode(2);
						return 1;
					case 0x50:
						this->SendGroupGather();
						return 1;
					case 0x52:
						this->SetOrderMode(8);
						return 1;
					case 0x53:
						this->SetOrderMode(6);
						return 1;
					case 0x54:
						this->SetOrderMode(7);
						return 1;
					}
				}
				switch (wparam) {
				case 0x20:
					if (wnd->dialogsMask == DIALOG_MASK_INGAME && wnd->console_active == 0) {
						if (this->my_unit == nullptr || this->my_unit->hp > -40) {
							if (this->IsBagOpen() == 0 && this->IsBookOpen() == 0) {
								wnd->vis_right_panel->MsgProc(VIS_MSG_TOGGLE_SELECT_PANEL, 0, 0);
								this->MsgProc(VIS_MSG_TOGGLE_SELECT_PANEL, 0, 0);
								wnd->vis_right_panel->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
								this->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
							} else {
								if (this->IsBagOpen() != 0) {
									wnd->vis_right_panel->MsgProc(VIS_MSG_TOGGLE_SELECT_PANEL, 0, 0);
									this->MsgProc(VIS_MSG_TOGGLE_SELECT_PANEL, 0, 0);
								}
								if (this->IsBookOpen() != 0) {
									wnd->vis_right_panel->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
									this->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
								}
							}
						} else {
							this->SendRequestRespawn();
						}
					}
					break;
				case 0x42:
				case 0x51:
					if (wnd->dialogsMask == DIALOG_MASK_INGAME) {
						wnd->vis_right_panel->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
						this->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
					}
					break;
				case 0x43:
					if (g_kbControlState != 0) {
						SetMessageColors(g_MessageColors == 0);
						this->msglog.Add(txt_patch.GetLine(g_MessageColors + 0x55), clr_log_sblack, 2000);
					}
					break;
				case 0x45:
					this->SelectAllOwnUnits();
					break;
				case 0x46:
					if (g_kbControlState != 0) {
						this->CycleFormation();
						this->msglog.Add(TxtFile::AllLines[this->GetFormation() + 0x61], clr_log_sblack, 2000);
					}
					break;
				case 0x48:
					if (g_kbControlState != 0) {
						this->ToggleShowHp();
						this->msglog.Add(TxtFile::AllLines[this->show_hp + 0x64], clr_log_sblack, 2000);
					}
					break;
				case 0x49:
				case 0xC0:
					if (wnd->dialogsMask == DIALOG_MASK_INGAME) {
						wnd->vis_right_panel->MsgProc(VIS_MSG_TOGGLE_SELECT_PANEL, 0, 0);
						this->MsgProc(VIS_MSG_TOGGLE_SELECT_PANEL, 0, 0);
					}
					break;
				case 0x4B:
					if (g_kbControlState != 0 && wnd->sessionMode != SESSION_LOCAL_BATTLE) {
						g_settings.ClanNames = g_settings.ClanNames == 0;
						this->msglog.Add(TxtFile::AllLines[g_settings.ClanNames + 0x16F], clr_log_sblack, 2000);
					}
					break;
				case 0x4C:
					if (g_kbControlState != 0) {
						this->ToggleFlyingHp();
						this->msglog.Add(TxtFile::AllLines[this->flying_hp + 0x66], clr_log_sblack, 2000);
					}
					break;
				case 0x4E:
					if (g_kbControlState != 0) {
						g_settings.ShowTimeFlow = g_settings.ShowTimeFlow == 0;
						this->msglog.Add(TxtFile::AllLines[g_settings.ShowTimeFlow + 0x68], clr_log_sblack, 2000);
						this->UpdateDayNightCycle(1);
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
						g_ShowNetStats = g_ShowNetStats == 0;
					}
					break;
				case 0x55:
					if (g_kbControlState == 0) {
						if (wnd->dialogsMask == DIALOG_MASK_INGAME) {
							this->SendHurtSelf();
						}
					} else {
						this->CycleAutoCastGroup();
						int32_t cast_state = (g_settings.AutoCasting & 0x10) != 0;
						if ((g_settings.AutoCasting & 0x20) != 0) {
							cast_state = 2;
						}
						this->msglog.Add(txt_patch.GetLine(cast_state + 0x73), clr_log_sblack, 2000);
					}
					break;
				case 0x57:
					if (g_kbControlState == 0) {
						if (wnd->sessionMode != SESSION_LOCAL_BATTLE) {
							wnd->PostMessage(MAIN_MSG_TOGGLE_QUEST_STATUS, 0, 0);
						}
					} else {
						this->CycleWimpyMode();
						this->msglog.Add(TxtFile::AllLines[this->GetWimpyMode() + 0x5E], clr_log_sblack, 2000);
					}
					break;
				}
			}
			break;
		}
	} else if ((wnd->dialogsMask & DIALOG_MASK_SHOP) != 0 && (wparam == 0x42 || wparam == 0x51)) {
		wnd->vis_right_panel->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
		this->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
		wnd->vis_root->FindChild(1000)->MsgProc(VIS_MSG_TOGGLE_SPELL_PANEL, 0, 0);
	}
	return 0;
}

// 40BD34
void BigStruct2::UpdateWorld() {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	uint32_t update_selection = 0;

	this->anim_tick++;
	TakeDamage::UpdateDamages(&this->damage_labels);
	if (g_MapRevealed == 0 && (this->anim_tick & 0x1F) == 0) {
		this->RebuildLandscapeOccupancy();
	}
	if ((this->anim_tick & 0xF) == 0 && g_SoundSettings.music_allowed != 0 && this->my_unit != nullptr) {
		UpdatePositionalMusic(this->my_unit->x_pos, this->my_unit->y_pos);
	}

	CDWordArray arr;
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->Update() == 0) {
			arr.Add(key);
		}
	}
	for (int32_t i = 0; i < arr.GetSize(); i++) {
		CGameObject* obj;
		this->objects.Lookup(arr[i], obj);
		this->objects.RemoveKey(arr[i]);
		if (obj->IsSelected() != 0) {
			this->order_mode = 0;
			wnd->vis_spellbook->ClearSpellSelection();
			wnd->vis_ordertoolbar->MsgProc(VIS_MSG_ORDERS_CLEAR, 0, 0);
		}
		delete obj;
	}
	if (arr.GetSize() != 0) {
		update_selection = 1;
	}

	this->TickLightningCells();
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		update_selection = update_selection | obj->UpdateBlockedState();
	}
	if (update_selection != 0) {
		this->UpdateSelectionState();
	}

	arr.RemoveAll();
	for (POSITION it = this->projectiles.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->projectiles.GetNextAssoc(it, key, obj);
		if (obj->Update() == 0) {
			arr.Add(key);
		}
	}
	for (int32_t i = 0; i < arr.GetSize(); i++) {
		CGameObject* obj;
		this->projectiles.Lookup(arr[i], obj);
		this->projectiles.RemoveKey(arr[i]);
		delete obj;
	}
}

// 417B42
void BigStruct2::IssueOrderAtCursor(CPoint* pt) {
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
		this->SelectUnitsInRect(&select_rect);
		return;
	}
	CSprite256* cursor_sprite = g_mousept.GetCursorSprite();
	if (this->selected_object_count == 0) {
		this->SelectUnitsInRect(&select_rect);
		return;
	}
	uint16_t target_id = (uint16_t)this->hovered_unit_id;
	if (cursor_sprite == g_Cursors[CURSOR_ATTACK]->GetSprite()) {
		if (target_id != 0 && this->hovered_unit->IsKindOf(RUNTIME_CLASS(CUnit))) {
			this->SendGroupAttack(target_id);
		} else {
			this->SendGroupMove(this->last_mouse_tile_x, this->last_mouse_tile_y);
		}
	} else if (cursor_sprite == g_Cursors[CURSOR_SWARM]->GetSprite()) {
		this->SendGroupAttackMove(this->last_mouse_tile_x, this->last_mouse_tile_y);
	} else if (cursor_sprite == g_Cursors[CURSOR_MOVE]->GetSprite()) {
		this->SendGroupMove(this->last_mouse_tile_x, this->last_mouse_tile_y);
	} else if (cursor_sprite == g_Cursors[CURSOR_PATROL]->GetSprite()) {
		this->SendGroupPatrol(this->last_mouse_tile_x, this->last_mouse_tile_y);
	} else if (cursor_sprite == g_Cursors[CURSOR_DEFEND]->GetSprite()) {
		if (target_id != 0) {
			this->SendGroupAttackTarget(target_id);
		}
	} else if (cursor_sprite == g_Cursors[CURSOR_SELECT]->GetSprite()) {
		this->SelectUnitsInRect(&select_rect);
	} else if (cursor_sprite == g_Cursors[CURSOR_PICKUP]->GetSprite()) {
		this->SendGroupPickup(this->last_mouse_tile_x, this->last_mouse_tile_y);
	} else if (cursor_sprite == g_Cursors[CURSOR_TOWN]->GetSprite()) {
		this->SendGroupEnterBuilding(target_id);
	} else if (cursor_sprite == g_Cursors[CURSOR_CAST]->GetSprite()) {
		int32_t spell_pos = wnd->vis_spellbook->GetActiveSlot();
		if (spell_pos >= 0) {
			if (target_id != 0) {
				if (this->hovered_unit->IsKindOf(RUNTIME_CLASS(CUnit)) && wnd->vis_spellbook->NeedsUnitTarget() != 0) {
					this->SendGroupCastAtUnit(target_id, spell_pos + 1);
				} else {
					this->SendGroupCastAtPosition(this->last_mouse_tile_x, this->last_mouse_tile_y, spell_pos + 1);
				}
			} else if (wnd->vis_spellbook->NeedsUnitTarget() == 0) {
				this->SendGroupCastAtPosition(this->last_mouse_tile_x, this->last_mouse_tile_y, spell_pos + 1);
			}
			if (this->order_mode == 5 && this->IsBookOpen() != 0) {
				this->CloseSpellBook();
			}
		}
	}
	if (this->order_mode != 0) {
		this->order_mode = 0;
		wnd->vis_spellbook->ClearSpellSelection();
		wnd->vis_ordertoolbar->MsgProc(VIS_MSG_ORDERS_CLEAR, 0, 0);
	}
}

// 417F09
void BigStruct2::SelectUnitsInRect(CRect* rect) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	int32_t min_size = g_ScreenSize.right * 10 / 0x280;
	int32_t small_rect = rect->Width() < min_size && rect->Height() < min_size;
	int32_t selected = g_kbShiftState == 0;
	if (small_rect == 0 && g_kbShiftState == 0) {
		selected = 0;
		for (int32_t i = this->hit_count - 1; i >= 0; i--) {
			CRect unit_rect = this->hit_rects[i];
			CRect intersection;
			unit_rect.NormalizeRect();
			intersection.IntersectRect(&unit_rect, rect);
			if (intersection.Width() * intersection.Height() > 10) {
				CGameObject* obj;
				if (this->objects.Lookup(this->hit_unit_ids[i], obj) &&
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
		for (int32_t i = this->hit_count - 1; i >= 0; i--) {
			CRect unit_rect = this->hit_rects[i];
			CRect intersection;
			unit_rect.NormalizeRect();
			intersection.IntersectRect(&unit_rect, rect);
			if (!intersection.IsRectEmpty()) {
				CGameObject* obj;
				if (this->objects.Lookup(this->hit_unit_ids[i], obj) && obj->IsKindOf(RUNTIME_CLASS(CStructure))) {
					selected = 0;
				}
			}
		}
	}
	if (selected != 0) {
		for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
			uint16_t key;
			CGameObject* obj;
			this->objects.GetNextAssoc(it, key, obj);
			obj->SetSelected(0);
		}
	}
	for (int32_t i = this->hit_count - 1; i >= 0; i--) {
		CRect unit_rect = this->hit_rects[i];
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
		if (!this->objects.Lookup(this->hit_unit_ids[i], obj)) {
			continue;
		}
		if (g_kbShiftState != 0) {
			if (obj->map_player == this->my_main_unit && (this->selection_flags & 4) == 0) {
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
	if (g_kbMenuState != 0 && this->selected_object_count == 1 && this->primary_selected_object->GetControlGroup() >= 0) {
		this->SelectControlGroup(this->primary_selected_object->GetControlGroup());
	}
	this->order_mode = 0;
	wnd->vis_spellbook->ClearSpellSelection();
	wnd->vis_ordertoolbar->MsgProc(VIS_MSG_ORDERS_CLEAR, 0, 0);
	if (g_kbShiftState == 0 && this->selected_object_count != 0 && (this->selection_flags & 1) != 0 && (this->selection_flags & 4) == 0) {
		CUnit* unit = this->PickVoiceUnit();
		if (unit != nullptr) {
			unit->PlaySelectVoice();
		}
	}
}

// 40C112
void BigStruct2::TickLightningCells() {
	for (POSITION it = this->lightning_cells.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		uint32_t value;
		this->lightning_cells.GetNextAssoc(it, key, value);
		value++;
		if (value < 0x16) {
			this->lightning_cells.SetAt(key, value);
		} else {
			this->lightning_cells.RemoveKey(key);
		}
	}
}

// 40C18B
void BigStruct2::RebuildLandscapeOccupancy() {
	this->minimap_dirty = 1;
	this->full_redraw_requested = 1;
	uint16_t* landscape = this->scenario->GetLandscape();
	int32_t count = this->map_width * this->map_height;
	for (int32_t i = 0; i < count; i++) {
		landscape[i] &= 0xBFFF;
	}
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
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
	case VIS_MSG_WORLD_TICK:
		this->UpdateWorld();
		this->msglog.Update();
		this->UpdateDayNightCycle(0);
		break;
	case VIS_MSG_FRAME_TICK:
		this->RenderWorldFrame();
		break;
	case VIS_MSG_CANCEL_ORDER:
		this->CancelOrderMode();
		result = 1;
		break;
	case VIS_MSG_SCROLL_VIEW:
		this->ScrollViewTo(wparam, lparam);
		result = 1;
		break;
	case VIS_MSG_ORDER_MODE_CHANGED:
		g_SfxArray[8]->Play(g_SoundSettings.sfx_pos, 0, 0, 0xDC, 0);
		this->SetOrderMode(wparam + 1);
		result = 1;
		break;
	case VIS_MSG_TOGGLE_SELECT_PANEL:
		if (this->IsBagOpen()) {
			this->CloseBag();
		} else {
			this->OpenBag();
		}
		if (g_mousept.GetSelectState() != 0) {
			g_mousept.ResetStates();
			wnd->ApplyCursorClip();
			this->UpdateMapCursor();
			CPoint pt(g_mousept.GetX(), g_mousept.GetY());
			this->IssueOrderAtCursor(&pt);
		}
		result = 1;
		break;
	case VIS_MSG_TOGGLE_SPELL_PANEL:
		if (this->IsBookOpen()) {
			this->CloseSpellBook();
		} else {
			this->OpenSpellBook();
		}
		if (g_mousept.GetSelectState() != 0) {
			g_mousept.ResetStates();
			wnd->ApplyCursorClip();
			this->UpdateMapCursor();
			CPoint pt(g_mousept.GetX(), g_mousept.GetY());
			this->IssueOrderAtCursor(&pt);
		}
		result = 1;
		break;
	case VIS_MSG_AMBIENT_STOP:
		if ((wnd->dialogsMask & DIALOG_MASK_INGAME) != 0) {
			this->ambient_view_x = -1;
			this->ambient_view_y = -1;
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
	if (wnd->dialogsMask != DIALOG_MASK_INGAME) {
		return 0;
	}
	if (g_mousept.GetSelectState() != 0) {
		g_mousept.ResetStates();
		wnd->ApplyCursorClip();
		this->UpdateMapCursor();
		this->IssueOrderAtCursor(&pos);
	}
	if (wnd->drag_entry != nullptr) {
		if (this->selected_object_count == 1 && this->primary_selected_object != nullptr) {
			int32_t tile_x = this->primary_selected_object->tileX;
			int32_t tile_y = this->primary_selected_object->tileY;
			if (this->hovered_unit != nullptr && this->hovered_unit->IsKindOf(RUNTIME_CLASS(CUnit)) && (this->hovered_unit->unitFlags & UNIT_FLAG_PLAYER_CLASS) != 0 &&
				abs(this->hovered_unit->tileX - this->primary_selected_object->tileX) <= 2 && abs(this->hovered_unit->tileY - this->primary_selected_object->tileY) <= 2) {
				tile_x = this->hovered_unit->tileX;
				tile_y = this->hovered_unit->tileY;
			}
			if (wnd->drag_entry->IsEmpty() != 0) {
				this->SendDropGold(wnd->drag_entry->count, tile_x | (tile_y << 8));
			} else {
				this->SendItemOperation(wnd->drag_src_tab, wnd->drag_src_slot, TRANSFER_DST_GROUND, tile_x | (tile_y << 8), wnd->drag_entry->count);
			}
		}
		delete wnd->drag_entry;
		g_Cursors[0]->Use();
		wnd->ResetItemCursor();
	}
	return 1;
}

// 41C630
void BigStruct2::ApplyLandscapeFlags(CWordArray* data) {
	uint16_t flag = (*data)[0];
	if (flag != 0) {
		flag = 0x2000;
	}
	data->RemoveAt(0, 1);
	uint16_t* land = this->scenario->GetLandscape();
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
void BigStruct2::LoadScenario(const char* name) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	delete this->scenario;
	CString str(name);
	if (wnd->sessionMode == SESSION_LOCAL_BATTLE) {
		str = "scenario\\" + str;
	}
	this->scenario = new Scenario(str);
	this->minimap_dirty = 1;
	this->SetScenario(this->scenario);
	wnd->vis_right_panel->MsgProc(VIS_MSG_SET_MAP_CONTEXT, (uint32_t)this, 0);
	this->UpdateDayNightCycle(1);
	LoadTerrainTiles(this->scenario->terrain_tile_mask);
	this->quest_map->RemovePlayerQuests(0);
}

// 403395
BigStruct2::~BigStruct2() {
	delete this->scratch_tile;
	this->FreeTileGrids();
	delete this->scenario;

	POSITION it = this->objects.GetStartPosition();
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		delete obj;
	}

	it = this->projectiles.GetStartPosition();
	while (it != nullptr) {
		uint16_t key;
		CGameObject* obj;
		this->projectiles.GetNextAssoc(it, key, obj);
		delete obj;
	}

	for (int32_t i = 0; i < this->map_players.GetSize(); i++) {
		MapPlayerData* data = this->map_players[i];
		delete data;
	}

	delete this->quest_map;

	while (this->hero_inventory.GetSize() != 0) {
		Item* item = this->hero_inventory[0];
		delete item;
		this->hero_inventory.RemoveAt(0, 1);
	}

	for (int32_t i = 0; i < 12; i++) {
		delete this->hero_equipment[i];
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
			g_TerPalettes[i] = &g_ter_tiles(0, i)->palette;
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
	if ((g_VideoOptionFlags & 2) != 0) {
		if (val != 0) {
			g_TerDirtBmp = new CBmp256("graphics\\terrain.3d\\dirt.bmp");
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
		g_TerDirtBmp = new CBmp256("graphics\\terrain\\dirt.bmp");
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
			g_TerPalettes[i] = &g_ter_tiles(0, i)->palette;
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
	if (this->view_x == this->ambient_view_x && view_y == this->ambient_view_y) {
		local_d8 = false;
	} else {
		local_d8 = true;
	}

	bool local_10 = timeGetTime() > this->ambient_sound_deadline;
	if (local_d8 || local_10) {
		if (local_d8) {
			this->ambient_view_x = this->view_x;
			this->ambient_view_y = view_y;
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
		
		uint16_t* landscape = this->scenario->GetLandscape();
		uint8_t* local_78 = this->scenario->GetObjectMap();
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
				if (this->terrain_light_override_cells.Lookup(PosYX(xx, yy).val, local_88) != 0 && (local_88 & 8) != 0) {
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
			this->ambient_sound_deadline = timeGetTime() + 5000 + rand() / 7;
		}
	}
}

void BigStruct2::MakeHeightCells() { //4065d3
	int8_t* hmap = this->scenario->GetMapHeights();
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
		this->dirty_rect.UnionRect(this->dirty_rect, local_2c);
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
							this->corner_light[out_x + out_y * out_w] = brightness;
							this->lightning_active++;
						}
						if (out_y2 > 2 && out_y2 <= maxY) {
							this->corner_light[out_x + out_y2 * out_w] = brightness;
							this->lightning_active++;
						}
					}
					if (out_x2 > 2 && out_x2 <= maxX) {
						if (out_y > 2 && out_y <= maxY) {
							this->corner_light[out_x2 + out_y * out_w] = brightness;
							this->lightning_active++;
						}
						if (out_y2 > 2 && out_y2 <= maxY) {
							this->corner_light[out_x2 + out_y2 * out_w] = brightness;
							this->lightning_active++;
						}
					}
				}
			}
		}
	}
}

// 41a001
void BigStruct2::SendGroupSelfCast(int32_t idx) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	if (wnd->vis_spellbook->IsSpellAvailable(idx - 1) != 0 && wnd->vis_spellbook->spell == -1) {
		POSITION it = this->objects.GetStartPosition();
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->objects.GetNextAssoc(it, key, obj);
			if (obj->IsSelected() && (obj->availableSpellMask & (1 << (idx - 1))) != 0) {
				pkt->from_player_id = this->my_main_unit->index;
				pkt->to_player_id = 0;
				pkt->id = GROUP_CAST_UNIT;
				pkt->slot = idx;
				pkt->count = 0;
				pkt->AppendWord(obj->unit_id);
				pkt->param = obj->unit_id;
				g_NetStru1_local.QueuePacketSend(pkt);
			}
		}
	}
	if (wnd->vis_spellbook->IsSpellEffectActive(idx - 1) != 0) {
		POSITION it = this->objects.GetStartPosition();
		while (it != nullptr) {
			uint16_t key;
			CGameObject* obj;
			this->objects.GetNextAssoc(it, key, obj);
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
			pkt->id = SCROLL_CAST_UNIT;
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
void BigStruct2::LeaveInn(int32_t serv_id) {
	MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
	if (wnd->sessionMode == SESSION_LOCAL_BATTLE) {
		CWordArray ids;
		for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
			uint16_t key;
			CGameObject* obj;
			this->objects.GetNextAssoc(it, key, obj);
			if (obj->IsKindOf(RUNTIME_CLASS(CUnit)) == 0 || (((CUnit*)obj)->unitFlags & UNIT_FLAG_PLAYER_CLASS) != 0) {
				continue;
			}
			ids.Add(key);
		}
		while (ids.GetSize() != 0) {
			CGameObject* obj = nullptr;
			this->objects.Lookup(ids[0], obj);
			this->objects.RemoveKey(ids[0]);
			if (obj != nullptr) {
				delete obj;
			}
			ids.RemoveAt(0, 1);
		}
	} else {
		Packet2Dwords* pkt = &Packet2Dwords::Inst;
		pkt->from_player_id = this->my_main_unit->index;
		pkt->to_player_id = 0;
		pkt->id = EXIT_INN;
		pkt->param1 = this->GetUnit_3f6c()->unit_id;
		pkt->param2 = serv_id;
		g_NetStru1_local.QueuePacketSend(pkt);
	}
}

// 41ace2
void BigStruct2::SendAutoSpellSelect(int32_t spell_id) {
	Packet2Dwords* pkt = &Packet2Dwords::Inst;
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->IsSelected() && (obj->spells & (1 << spell_id)) != 0) {
			pkt->id = SET_PARAMETER;
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
int32_t BigStruct2::FindUnitByQuestFlag(int32_t id, CUnit** out) {
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
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
uint32_t BigStruct2::FindObjectAtTile(int32_t x, int32_t y) {
	for (POSITION it = this->objects.GetStartPosition(); it != nullptr;) {
		uint16_t key;
		CGameObject* obj;
		this->objects.GetNextAssoc(it, key, obj);
		if (obj->bIsBlocked != 0 || obj->tileX != x || obj->tileY != y) {
			continue;
		}
		return key;
	}
	return 0;
}

// 41cda3
void BigStruct2::SendMapAnnouncement(const char* mapname) {
	PacketJoin* pkt = &PacketJoin::Inst;
	pkt->id = ANNOUNCEMENT;
	if (this->my_main_unit != nullptr) {
		pkt->from_player_id = this->my_main_unit->index;
	}
	pkt->to_player_id = 0;
	strcpy(pkt->name, mapname);
	g_NetStru1_main.QueuePacketSend(pkt);
	g_NetStru1_main.SendAllData();
}

// 41a942
void BigStruct2::SendShopBuy() {
	PacketWord* pkt = &PacketWord::Inst;
	pkt->id = SHOP_BUY;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->value = this->primary_selected_object->unit_id;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41dfa6
CUnit* BigStruct2::FindOrCreateUnitByServerId(int32_t serv_id) {
	CUnit* unit = this->FindUnitByServerId(serv_id);
	if (unit == nullptr) {
		unit = this->CreateRemoteHeroStub(serv_id);
	}
	return unit;
}

// 41a9f6
void BigStruct2::SendShopTakeBack() {
	PacketWord* pkt = &PacketWord::Inst;
	pkt->id = SHOP_TAKE_BACK;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->value = this->primary_selected_object->unit_id;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41a99c
void BigStruct2::SendShopSell() {
	PacketWord* pkt = &PacketWord::Inst;
	pkt->id = SHOP_SELL;
	pkt->from_player_id = this->my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->value = this->primary_selected_object->unit_id;
	g_NetStru1_local.QueuePacketSend(pkt);
}

// 41de80
CUnit* BigStruct2::CreateRemoteHeroStub(int32_t serv_id) {
	CUnit* unit = new CUnit();
	unit->unitFlags = UNIT_FLAG_REMOTE_HERO | UNIT_FLAG_CREATURE | UNIT_FLAG_FEMALE | UNIT_FLAG_MAGE;
	unit->typeId = 1;
	unit->face = serv_id - 0x50;
	unit->serverId = (uint16_t)serv_id;
	return unit;
}

void BigStruct2::SendGroupStopAfk() { //client: 41be0f
	PacketGroupOrder* pkt = &PacketGroupOrder::Inst;
	pkt->id = GROUP_STOP_AFK;
	pkt->from_player_id = my_main_unit->index;
	pkt->to_player_id = 0;
	pkt->count = 0;
	pkt->AppendWord(my_unit->unit_id);
	g_NetStru1_local.QueuePacketSend(pkt);
}
