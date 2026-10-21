#include "net.h"
#include <WS2tcpip.h>

#include <cmath>
#include <cstring>
#include <process.h>

#include "player.h"
#include "unit.h"
#include "unit_list.h"
#include "group.h"
#include "game_app.h"
#include "effect.h"
#include "eye.h"
#include "packet.h"
#include "sack.h"
#include "server.h"
#include "spell.h"
#include "world.h"
#include "players_list.h"
#include "table.h"
#include "util.h"
#include "dplobby.h"
#include "building.h"
#include "buildings_list.h"
#include "spell_effect.h"
#include "quest_map.h"
#include "map_stuff.h"
#include "quest.h"
#include "inventory.h"
#include "shelf.h"
#include "shop.h"


uint32_t dword_665CFC;

GUID Allods2GUID = {0x65450caa, 0x57ca, 0x11d2, {0xbb, 0xcc,0x00, 0x60, 0x97, 0xd2, 0xee, 0x9f}};


NetStru1 NetStru1::HatConnector(0); // 6a6a98
NetStru1 g_NetStru1_main(1);  //6c3a08
NetStru1 g_NetStru1_local(0);  //6b16b0

CLlDriver g_CLlDriver(Allods2GUID, &g_NetStru1_main); //6d07a0
CLlDriver g_HatLLDriver(Allods2GUID, &NetStru1::HatConnector); //6cdbc0


void PackerDat::DeleteTail(PackerTail* node) {
    //526621
    if (node) {
        if (node->child0) {
            this->DeleteTail(node->child0);
        }
        if (node->child1) {
            this->DeleteTail(node->child1);
        }

        delete node;
    }
}

void PackerDat::Clear() {
    //5265f7
    this->DeleteTail(this->tail);
    this->tail = nullptr;
}

PackerDat::PackerDat() {
    //5260fa
    this->tail = nullptr;
}

PackerDat::~PackerDat() {
    //526115
    this->Clear();
}

void NetStru1::AnnouncePlayerName(const CString& name, Player* player) {
	PacketJoin& pkt = PacketJoin::Inst;
	pkt.id = 0x91;
	strcpy(pkt.name, name);

	if (player) {
		pkt.to_player_id = player->player_id;
	} else {
		pkt.to_player_id = 0;
	}

	pkt.player_id = 0;
	pkt.token_id = 0;
	pkt.color = 0;
	pkt.flags = 0;

	this->QueuePacketSend(&pkt);
}

// 51C8B1
void NetStru1::SendExistingPlayers(Player* player) {
    LogMessage("NetStru1::SendExistingPlayers");
	POSITION pos = g_PlayersList->list.GetHeadPosition();
	while (pos != nullptr) {
		Player* p = g_PlayersList->list.GetNext(pos);
		if (p == player) {
			continue;
		}

		PacketJoin& packet = PacketJoin::Inst;

		strcpy(packet.name, p->name);
		packet.id = 0x96;
		packet.to_player_id = player->player_id;
		packet.player_id = p->player_id;
		packet.token_id = p->token_id;
		packet.color = p->color_slot - 1;
		packet.flags = p->is_ai != 0;
		this->QueuePacketSend(&packet);

		if (p->main_unit != nullptr && p->main_unit->field_0x1bc != 0 && g_ServerConfig.gameType == 0) {
			g_NetStru1_main.SendTeamNotify(5, p->player_id, player);
		}
	}
}

// 51C61E
void NetStru1::UntrackToken(Token* token) {
    PacketWord& packet = PacketWord::Inst;

    packet.id = 0x6a;
    packet.value = token->building_id;
    POSITION pos = g_PlayersList->list.GetHeadPosition();
    while (pos != nullptr) {
        Player* player = g_PlayersList->list.GetNext(pos);
        if (token->sent_to_mask & player->vision_sharing_id) {
            packet.to_player_id = player->player_id;
            this->QueuePacketSend(&packet);
        }
    }
}

// 51c748
void NetStru1::SendTickPacket(Player* player) {
    PacketDword& packet = PacketDword::Inst;
    packet.to_player_id = 0;
    packet.id = 0x64;
    packet.value = g_Server->tick;
    if (player == nullptr) {
        NetStru2* client = this->GetClientByPlayerID(0);
        if (client != nullptr) {
            client = this->GetClientByPlayerID(0);
            packet.to_player_id = client->player_id;
        }
    } else {
        packet.to_player_id = player->player_id;
    }
    this->QueuePacketSend(&packet);
}

// 51ce86
void NetStru1::SendTeamNotify(uint32_t msg_type, uint32_t player_id, Player* recpt) {
    this->SendInfoPacket(0x92, msg_type, player_id, recpt);
}

// 51ceac
void NetStru1::SendBarePacket(uint8_t id, Player* player) {
    Packet& packet = Packet::Inst;
    packet.id = id; 
    packet.to_player_id = player ? player->player_id : 0;
    this->QueuePacketSend(&packet);
}

// 51d49b
void NetStru1::AnnouncePlayer(Player* player) {
    PacketJoin& packet = PacketJoin::Inst;
    packet.id = 0x93;
    strcpy(packet.name, player->name);
    packet.to_player_id = 0;
    packet.__field_0xa = player->player_id;
    this->QueuePacketSend(&packet);
}

// 51cefb
void NetStru1::SendInfoPacket(uint8_t id, int32_t value1, int32_t value2, Player* target) {
    Packet2Dwords& packet = Packet2Dwords::Inst;
    packet.id = id;
    packet.to_player_id = target ? target->player_id : 0;
    packet.param1 = value1;
    packet.param2 = value2;
    this->QueuePacketSend(&packet);
}

// 51C7CC
void NetStru1::SendLatency(int32_t latency, Player* player) {
    PacketDword& packet = PacketDword::Inst;
    packet.id = 0xc1;
    packet.value = latency;
    packet.to_player_id = player ? player->player_id : 0;
    this->QueuePacketSend(&packet);
}

// 51CD2A
void NetStru1::SendGameEvent(Player* player, int32_t event_id, int32_t value2) {
    Packet2Dwords& packet = Packet2Dwords::Inst;
    packet.id = 0xb6;
    packet.to_player_id = player ? player->player_id : 0;
    packet.param1 = event_id;
    packet.param2 = value2;
    this->QueuePacketSend(&packet);
}

// 51CDFB
void NetStru1::SendArenaScore(CString& name, uint32_t score, char type, Player* player) {
    PacketJoin& packet = PacketJoin::Inst;
    if (type != 0) {
        packet.id = type;
    } else {
        packet.id = 0x91;
    }
    strcpy(packet.name, name);
    packet.to_player_id = player ? player->player_id : 0;
    packet.__field_0xa = score;
    this->QueuePacketSend(&packet);
}

// 51CA5D
void NetStru1::SendServerState(Player* player) {
    POSITION pos = g_Server->srv_stru1->sack_list->list.GetHeadPosition();
    while (pos != nullptr) {
        Sack* sack = g_Server->srv_stru1->sack_list->list.GetNext(pos);
        this->BroadcastTokenState(sack, player, 0);
    }
}

// 51C601
void NetStru1::SendLevelUp(Unit* unit, int unused) {
    this->BroadcastTokenState(unit, nullptr, 0x73);
}

// 51B870
void NetStru1::BroadcastUnitState(Unit* unit) {
    POSITION pos = g_PlayersList->list.GetHeadPosition();
    while (pos != nullptr) {
        Player* player = g_PlayersList->list.GetNext(pos);
        if (player->is_active && unit->NeedsRefreshFor(player)) {
            unit->position->IsAtCellCenter(); // WAT: Useless call?
            if (unit->IsUnit()) {
                this->SendUnitUpdate(unit, player, 0xFFB, -1, 0, 0);
            } else {
                this->BroadcastTokenState(unit, player, 0);
            }
        }
    }
}

// 51B99E
void NetStru1::SendUnitActionPacket(Unit* unit, uint8_t angle, uint8_t speed_dir, uint8_t mode) {
    PacketUnitAction& packet = PacketUnitAction::Inst;
    if (mode == 0) {
        packet.id = 0x6B;
    } else {
        packet.id = mode;
    }
    packet.segments = speed_dir;
    packet.angle = angle;
    unit->GetTicksSinceAction();
    unit->last_action_tick = g_Server->tick + speed_dir;
    packet.unit_id = (uint16_t)unit->building_id;
    packet.to_player_id = 0;
    this->SendUnitAction(&packet, unit);
}

// 51BA2F
void NetStru1::SendRangedAttackPacket(Unit* unit) {
    PacketRangedAttack& packet = PacketRangedAttack::Inst;
    packet.id = 0x72;
    packet.unit_id = (uint16_t)unit->building_id;
    packet.target_id = (uint16_t)unit->cast_target->building_id;
    packet.to_player_id = 0;
    unit->last_action_tick = g_Server->tick + unit->charge + unit->relax;
    this->SendUnitAction(&packet, unit);
}

// 51C59C
void NetStru1::SendUnitTurn(Unit* unit, Unit* target) {
    if (unit->max_range < 2) {
        const uint8_t speed_dir = unit->charge + unit->relax;
        const uint8_t angle = unit->eye->GetFacingHalf();
        g_NetStru1_main.SendUnitActionPacket(unit, angle, speed_dir, 0x71);
    } else {
        g_NetStru1_main.SendRangedAttackPacket(unit);
    }
}

// 51BE0E
void NetStru1::BroadcastEffectMove(Effect* effect, uint8_t type) {
    PacketMoveBroadcast& packet = PacketMoveBroadcast::Inst;
    packet.id = 0x86;
    packet.x = effect->position->GetX();
    packet.y = effect->position->GetY();
    packet.segments = type;
    packet.unit_id = (uint16_t)effect->building_id;
    packet.type_id = (uint8_t)effect->typeId;
    packet.to_player_id = 0;
    this->BroadcastMovePacket(&packet, effect->position);
}

// 51BE8F
void NetStru1::SendAreaEffect(AreaEffect* obj, int32_t flag) {
    if (obj->itemDataID == 2) {
        PacketMoveBroadcast& packet = PacketMoveBroadcast::Inst;
        packet.id = 0x86;
        packet.x = obj->position->GetX();
        packet.y = obj->position->GetY();
        packet.segments = 0x16;
        packet.unit_id = (uint16_t)obj->building_id;
        packet.type_id = (uint8_t)obj->typeId;
        packet.to_player_id = 0;
        this->BroadcastMovePacket(&packet, obj->position);
    } else {
        PacketAoeZone& packet = PacketAoeZone::Inst;
        packet.id = 0x87;

        const int8_t radius = obj->radius;
        packet.field_0xb = obj->position->GetX() - radius;
        packet.field_0xc = obj->position->GetY() - radius;

        const int8_t size = radius * 2 + 1;
        packet.field_0xd = size;
        packet.field_0xe = size;

        for (int32_t i = 0; i < 12; i++) {
            packet.data[i] = 0;
        }
        for (int32_t dx = 0; dx <= radius * 2; ++dx) {
            for (int32_t dy = 0; dy <= radius * 2; ++dy) {
                int32_t bit_index = dx + dy * size;
                int32_t coordinate = (packet.field_0xb + dx) + ((packet.field_0xc + dy) << 8);
                AreaEffect* applicable = MapStuff_Instance->AreaEffectCovers(obj, coordinate);
                if (flag != 0) {
                    if (applicable != 0) {
                        packet.data[bit_index >> 3] |= (1 << (bit_index & 7));
                    }
                } else {
                    if (applicable == 0) {
                        packet.data[bit_index >> 3] |= (1 << (bit_index & 7));
                    }
                }
            }
        }
        packet.set_flag = (uint8_t)flag;
        packet.type_id = (uint8_t)obj->typeId;
        packet.to_player_id = 0;
        this->BroadcastAoeZone(&packet, obj);
    }
}

// 51C0F7
void NetStru1::SendPendingUnits(Player* player) {
    if (dword_6CDB3C != nullptr) {
        POSITION pos = dword_6CDB3C->unit_list.GetHeadPosition();
        while (pos != nullptr) {
            Unit* unit = dword_6CDB3C->unit_list.GetNext(pos);
            uint32_t flags = (unit->building_id < 0x6000) ? 0xA35FFFFF : 0x100013;
            if (unit->ownership_changed != 0) {
                flags |= 0x40;
            }
            if (g_Server->field4_0x74 == 0 && 0x16 <= unit->server_id && unit->server_id <= 0x17) {
                flags |= 0x200000;
            }
            g_NetStru1_main.SendUnitUpdate(unit, player, flags, 0xFFB, 0, 0);
        }
    }

    UnitList* units_list = g_Server->srv_stru1->units_list;
    if (units_list != nullptr) {
        POSITION pos = units_list->unit_list.GetHeadPosition();
        while (pos != nullptr) {
            Unit* unit = units_list->unit_list.GetNext(pos);
            uint32_t flags = (unit->building_id < 0x6000) ? 0xA35FFFFF : 0x13;
            if (unit->ownership_changed != 0) {
                flags |= 0x40;
            }
            if (unit->decay < 5) {
                g_NetStru1_main.SendUnitUpdate(unit, player, flags, 0xFFB, 0, 0);
            }
        }
    }

    BuildingsList* building_list = g_Server->srv_stru1->building_list;
    POSITION pos = building_list->GetHeadPosition();
    while (pos != nullptr) {
        Building* building = building_list->GetNext(pos);
        if (building->hp != building->hp_max) {
            g_NetStru1_main.BroadcastTokenState(building, player, 0);
        }
    }
}

// 51C46E
void NetStru1::ResendPlayerUnits(Player* player) {
    if (player->main_unit != nullptr) {
        this->SendUnitUpdate(player->main_unit, player, 0xFFB, -1, 0, 0);
    }
    if (player->unit_list != nullptr) {
        POSITION pos = player->unit_list->unit_list.GetHeadPosition();
        while (pos != nullptr) {
            Unit* unit = player->unit_list->unit_list.GetNext(pos);
            unit->sent_to_mask &= ~player->vision_sharing_id;
            g_NetStru1_main.SendUnitUpdate(unit, player, 0xFFB, -1, 0, 0);
        }
    }
    this->SendTickPacket(player);
}

// 51CB21
void NetStru1::SendVisibilityState(Player* player) {
    if (g_World == nullptr) {
        return;
    }
    PacketTerrain& packet = PacketTerrain::Inst;
    packet.id = 0xB9;
    packet.to_player_id = player->player_id;
    int32_t count = g_PlayersList->GetMaxPlayerId() + 1;
    packet.count = count;
    memset(packet.buf, 0, (size_t)count * 2);

    int32_t this_player_id = player->player_id;
    POSITION pos = g_PlayersList->list.GetHeadPosition();
    while (pos != nullptr) {
        Player* p = g_PlayersList->list.GetNext(pos);
        int32_t pid = p->player_id;
        packet.buf[pid] = g_World->diplomacy.diplomacy[this_player_id][pid];

        int8_t diplomacy = g_World->diplomacy.diplomacy[pid][this_player_id];
        if (diplomacy & 0x10) {
            packet.buf[pid] |= 8;
        }
        if (diplomacy & 2) {
            packet.buf[pid] |= 0x20;
        }
        if (diplomacy & 1) {
            packet.buf[pid] |= 0x40;
        }
    }

    this->QueuePacketSend(&packet);
}

// 51CF5C
void NetStru1::SendUnitVisibility(Unit* unit, int32_t flag, Player* player) {
    PacketWord& packet = PacketWord::Inst;
    packet.id = flag ? 0x74 : 0x69;
    packet.value = unit->building_id;

    if (player != nullptr) {
        packet.to_player_id = player->player_id;
        if (g_Server->field4_0x74 != 0) {
            if ((unit->seen_by_mask & player->vision_sharing_id) == 0) {
                int32_t pid = player->player_id;
                if (pid >= 0x10 && pid < 0x20) {
                    unit->something_per_player[pid - 0x10] |= 0x40000000;
                }
                if (flag == 0) {
                    return;
                }
            }
        }
        this->QueuePacketSend(&packet);
        return;
    }

    POSITION pos = g_PlayersList->list.GetHeadPosition();
    while (pos != nullptr) {
        Player* p = g_PlayersList->list.GetNext(pos);
        if (!p->is_active) {
            continue;
        }
        packet.to_player_id = p->player_id;
        if (g_Server->field4_0x74 != 0) {
            if ((unit->seen_by_mask & p->vision_sharing_id) == 0) {
                int32_t pid = p->player_id;
                if (pid >= 0x10 && pid < 0x20) {
                    unit->something_per_player[pid - 0x10] |= 0x40000000;
                }
                if (flag != 0) {
                    this->QueuePacketSend(&packet);
                }
                continue;
            }
        }
        this->QueuePacketSend(&packet);
    }
}

// 51D1A8
void NetStru1::SendKillStats(uint16_t player_id, Player* player) {
    if (player_id != 0) {
        PacketDword& packet = PacketDword::Inst;
        packet.id = 0xB3;
        packet.to_player_id = player ? player->player_id : 0;
        packet.value = (uint32_t)player_id | ((uint32_t)player->kill_stats[player_id] << 16);
        this->QueuePacketSend(&packet);
        return;
    }

    PacketData& packet = PacketData::Inst;
    packet.id = 0xBA;
    packet.to_player_id = player ? player->player_id : 0;

    // This encodes monster kill stats using a simple run-length encoding.
    // TODO: likely this needs to be put into a separate function.
    const int size = 0xA00;
    uint8_t* src = player->kill_stats;
    uint8_t* buf = (uint8_t*)operator new(size);
    *(uint32_t*)buf = size;
    uint8_t* dst = buf + 4;
    uint32_t written = 0;
    while (written < size) {
        if (src[0] == src[1]) {
            uint8_t* p2 = src + 1;
            uint8_t run = 1;
            while (p2[-1] == p2[0] && (run & 0xFF) < 0x7F && (written + run) < size) {
                run++;
                p2++;
            }
            *dst++ = (run & 0xFF) | 0x80;
            *dst++ = src[0];
            src += run;
            written += run;
        } else {
            uint8_t* p2 = src + 1;
            uint8_t run = 1;
            while (p2[-1] != p2[0] && (run & 0xFF) < 0x7F && (written + run) < size) {
                run++;
                p2++;
            }
            if ((written + (run & 0xFF)) == size) {
                run++;
            }
            *dst++ = (run - 1) & 0xFF;
            memcpy(dst, src, run - 1);
            dst += run - 1;
            src += run - 1;
            written += run - 1;
        }
    }
    uint32_t compSize = (uint32_t)(dst - buf);
    packet.count = compSize;
    memcpy(packet.data, buf, compSize);
    operator delete(buf);

    this->QueuePacketSend(&packet);
}

// 51D4F6
void NetStru1::SendSrvStru1State(QuestMap* quest_map, Player* player, int32_t flag) {
    PacketTerrain& packet = PacketTerrain::Inst;
    packet.id = flag ? 0xBC : 0xBB;
    packet.to_player_id = player->player_id;
    packet.count = 1;

    packet.buf[0] = 0;
    uint8_t* ptr = reinterpret_cast<uint8_t*>(&packet.buf[1]);

    POSITION pos = quest_map->quests_map.GetStartPosition();
    while (pos != nullptr) {
        Quest* quest;
        quest_map->quests_map.GetNextAssoc(pos, quest_map->building_id, quest);
        quest_map->quest = quest;
        if (quest->quest_data.player_id != player->player_id) {
            continue;
        }
        *reinterpret_cast<uint16_t*>(ptr) = (uint16_t)quest->Kind();
        ptr += 2;
        memcpy(ptr, &quest->quest_data.some_id, sizeof(QuestData));
        ptr += sizeof(QuestData);
        packet.count += 0xF;
        packet.buf[0] += 1;
    }
    this->QueuePacketSend(&packet);
}

// 51D6B4
void NetStru1::SendTeamState(uint16_t to_player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = to_player_id;
    packet.id = 0xC2;
    packet.count = 0x58;
    uint32_t* ptr = reinterpret_cast<uint32_t*>(packet.data);
    int32_t teams_started = g_PlayersList->AreTeamsStarted();
    *ptr++ = teams_started;
    *ptr++ = g_ServerConfig.frag_limit;
    *ptr++ = g_Server->ctf_carrying[0];
    *ptr++ = g_Server->ctf_carrying[1];
    *ptr++ = teams_started ? g_Server->ctf_score[0] : 0xF0000001u;
    *ptr++ = teams_started ? g_Server->ctf_score[1] : 0xF0000001u;
    for (int32_t i = 0x10; i < 0x20; i++) {
        Player* p = g_PlayersList->FindById(i);
        if (p == nullptr) {
            *ptr++ = 0xF0000003u;
        } else {
            *ptr++ = teams_started ? p->frags : p->team_ready;
        }
    }
    this->QueuePacketSend(&packet);
}

// 51EEB7
void NetStru1::UpdateConnStats() {
    int32_t key;
    ConnStatInfo* info;
    POSITION pos = this->client_stat.GetStartPosition();
    while (pos != nullptr) {
        this->client_stat.GetNextAssoc(pos, key, info);
        info->time += 1;
        info->total_bytes += info->field1;
        if (info->field1 > info->max_bs) {
            info->max_bs = info->field1;
        }
        info->cur_bs = info->field1;
        info->field1 = 0;
    }
}

// 51E289
void NetStru1::SendHatLobbyInfo(int32_t player_count, CString source, int32_t map_level, int32_t game_type, int32_t map_range) {
	PacketJoin& pkt = PacketJoin::Inst;
	pkt.id = 0xd2;
	pkt.__field_0xa = player_count | (map_level << 8) | (game_type << 16) | (map_range << 24);
	strcpy(pkt.name, source);
	pkt.to_player_id = 0;
	this->QueuePacketSend(&pkt);
}

// 4FB4CA
void NetStru1::TransferUnitOwnership(Unit* unit, Player* new_owner) {
    #ifdef A2SERVER_PATCH
    if (unit == nullptr) {
        LogMessage("TransferUnitOwnership: unit is nullptr.");
        return;
    }
    #endif

    Player* old_owner = unit->pOwner;

    // If the unit belongs to a group, remove it from that group.
    if (unit->group != nullptr) {
        unit->group->RemoveUnit(unit);
    }

    // Remove unit from old owner's unit list.
    // ASM manually walks the CList structure to find and remove the node.
    UnitList* old_unit_list = old_owner->unit_list;
    POSITION pos = old_unit_list->unit_list.Find(unit);
    if (pos != nullptr) {
        old_unit_list->unit_list.RemoveAt(pos);
    }
    
    unit->pOwner = new_owner;
    new_owner->unit_list->AddTail(unit);
    
    // Create a new Group for the unit.
    Group* new_group = new Group();
    
    // Add the new group to the new owner's group list.
    // The assembly manually adds to the CList structure.
    new_owner->group_list->groups.AddTail(new_group);
    
    // Associate the unit with the new group.
    new_group->AddUnit(unit);

    g_World->ProcessAnimateDeadGroup(new_group);

    // Update unit's vision sharing flags.
    uint16_t old_vision_mask = ~old_owner->vision_sharing_id;
    unit->sent_to_mask = unit->sent_to_mask & old_vision_mask;
    
    uint16_t new_vision_mask = ~new_owner->vision_sharing_id;
    unit->sent_to_mask = unit->sent_to_mask & new_vision_mask;
    
    // Send network packet about unit ownership change.
    g_NetStru1_main.SendUnitUpdate(unit, 0, 0xFFFFFFFF, 0xFFB, 0, 0);
    
    // Mark unit as needing update?
    unit->ownership_changed = 1;
}

// SendUnitUpdate: send unit state to a player (or all players if player==nullptr)
// 519221
void NetStru1::SendUnitUpdate(Unit* unit, Player* player, uint32_t mask, int32_t props_flags, int32_t inv_start_index, int32_t inv_end_index) {
    if (player == nullptr) {
        // Broadcast to all players.
        POSITION pos = g_PlayersList->list.GetHeadPosition();
        while (pos != nullptr) {
            Player* p = g_PlayersList->list.GetNext(pos);
            if (p->is_active != 0 || unit->pOwner == p) {
                this->SendUnitUpdate(unit, p, mask, props_flags, inv_start_index, inv_end_index);
            }
        }
        return;
    }

    // Check unit visibility to this player (unit->field_0x1a4 & player->vision_sharing_id).
    if (!(unit->seen_by_mask & player->vision_sharing_id)) {
        if (g_Server->field4_0x74 != 0) {
            int16_t pid = player->player_id;
            if (pid >= 0x10 && pid < 0x20) {
                unit->something_per_player[pid - 16] |= mask;
            }
            return;
        }
        
    }

    // Fill in packet header.
    PacketUnitUpdate* pkt = &PacketUnitUpdate::Inst;

    pkt->to_player_id = player->player_id;
    pkt->unit_id = (uint16_t)unit->building_id;
    pkt->flags_mask = 0;
    pkt->data_offset = 0;

    int is_humanoid = unit->IsHumanoid();

    if (is_humanoid == 0) {
        if (unit->building_id < 0x6000) {
            if (unit->server_id > 20) {
                mask &= 0x800000db;
            }
        } else if (g_Server->field4_0x74 != 0) {
            mask &= ~0x80;
        }

        if (unit->pOwner != player) {
            mask &= 0xc08000fb;

            int32_t max_hp = unit->monster_info->Values()[0].health_max;

            if (g_Server->field4_0x74 == 0) {
                if (g_Server->field22_0xd8 == 1) {
                    max_hp = (int32_t)((double)max_hp * 0.5);
                } else if (g_Server->field22_0xd8 == 3) {
                    max_hp = (int32_t)((double)max_hp * 1.5);
                }
            }

            if (unit->hp_max == max_hp && unit->typeId != 0x1A && unit->typeId != 0x1B) {
                mask &= 0xc08000f3;
            }
        }
    } else { // Humanoid units
        if (unit->pOwner != player) {
            uint16_t tid = unit->typeId;
            if (tid < 0x21 || tid > 0x3F) {
                mask &= 0xc0c000fb;
            } else if (g_World != nullptr) {
                if ((g_World->diplomacy.diplomacy[unit->pOwner->player_id][player->player_id] & 0x10) == 0) {
                    mask &= 0xc0c000fb;
                }
            }
        }
        if (g_Server->field4_0x74 != 0) {
            mask &= ~0x80;
        }
    }

    if (unit->mp_max == 0) {
        mask &= ~0x2;
    }

    // Preserve original mask for the later calls.
    int32_t orig_mask = mask;

    // --- Pack fields into unk_6D1180 based on flags bits ---

    // bit 0x1: hp (uint16)
    if (mask & 0x1) {
        pkt->PutWord(0x1, unit->hp);
    }

    // bit 0x2: mp (uint16)
    if (mask & 0x2) {
        pkt->PutWord(0x2, unit->mp);
    }

    // bit 0x4: 5 skill bytes (humanoids only)
    if (is_humanoid != 0) {
        if (mask & 0x4) {
            for (int i = 0; i < 5; ++i) {
                pkt->PutByte(0x4, unit->hit_values.skill_levels[i+1]);
            }
        }
    }

    // bit 0x8: hp_max and mp_max
    if (mask & 0x8) {
        pkt->PutWord(0x8, unit->hp_max);
        pkt->PutWord(0x8, unit->mp_max);
    }

    // bit 0x10: position
    if (mask & 0x10) {
        uint8_t value = unit->eye->GetFacingHalf();
        pkt->PutByte(0x10, value);

        if (unit->position->IsAtCellCenter() == 0) {
            uint16_t position1 = unit->eye->position1;
            pkt->PutByte(0x10, static_cast<uint8_t>(position1));
            pkt->PutByte(0x10, static_cast<uint8_t>(position1 >> 8));
        } else {
            pkt->PutByte(0x10, unit->position->GetX());
            pkt->PutByte(0x10, unit->position->GetY());
        }
    }

    // bit 0x20: type ID and face.
    if (mask & 0x20) {
        pkt->PutByte(0x20, (uint8_t)unit->typeId);
        pkt->PutByte(0x20, (uint8_t)unit->face);
    }

    // bit 0x40: player ID (or 1 if no owner).
    if (mask & 0x40) {
        pkt->PutByte(0x40, unit->pOwner ? unit->pOwner->player_id : 1);
    }

    // bit 0x80: server ID.
    if (mask & 0x80) {
        pkt->PutWord(0x80, unit->server_id);
    }

    // bit 0x10000: attack.
    if (mask & 0x10000) {
        pkt->PutWord(0x10000, unit->hit_values.attack);
    }

    // bit 0x20000: physical damage.
    if (mask & 0x20000) {
        uint8_t s1 = unit->hit_values.hand_damage_min
                   + unit->hit_values.some_damage_min
                   + unit->hit_values.some_damage2_min;
        pkt->PutByte(0x20000, s1);

        uint8_t s2 = unit->hit_values.hand_damage_spread
                   + unit->hit_values.some_damage_spread
                   + unit->hit_values.some_damage2_spread;
        pkt->PutByte(0x20000, s2);
    }

    // bit 0x4000: defense.
    if (mask & 0x4000) {
        pkt->PutWord(0x4000, unit->protections.defense);
    }

    // bit 0x8000: absorption.
    if (mask & 0x8000) {
        pkt->PutByte(0x8000, static_cast<uint8_t>(unit->protections.absorption));
    }

    // bit 0x40000: scan_range.
    if (mask & 0x40000) {
        pkt->PutWord(0x40000, unit->scan_range);
    }

    // bit 0x2000: carrying weight.
    if (mask & 0x2000) {
        pkt->PutWord(0x2000, unit->carrying_weight_100g);
    }

    // bit 0x1000000: magic protection.
    if (mask & 0x1000000) {
        for (int i = 0; i < 5; ++i) {
            pkt->PutByte(0x1000000, static_cast<uint8_t>(unit->protections.magic_protections[i + 1]));
        }
    }
    
    // bit 0x2000000: battle stats (owner only).
    if (mask & 0x2000000) {
        Player* owner = unit->pOwner;
        if (g_Server->field4_0x74 == 0 || owner == nullptr || player != owner || owner->main_unit != unit) {
            mask &= ~0x2000000;
        } else {
            pkt->PutInt(0x2000000, owner->monster_kills);
            pkt->PutInt(0x2000000, owner->player_kills);
            pkt->PutInt(0x2000000, owner->deaths);
            pkt->PutInt(0x2000000, owner->frags);
        }
    }

    // bit 0x80000: speed.
    if (mask & 0x80000) {
        pkt->PutByte(0x80000, static_cast<uint8_t>(unit->speed));
    }

    // bit 0x100000: spellbook bitmask + eye2 calibration byte
    if ((mask & 0x100000) && unit->spell_book != nullptr) {
        pkt->PutInt(0x100000, unit->spell_book->CalcSpellMask());
        pkt->PutByte(0x100000, unit->eye2->spell_id);
    }

    // bits 0x100..0x1000: experience per sphere (humanoids only).
    if (is_humanoid) {
        Humanoid* h = static_cast<Humanoid*>(unit);
        if (mask & 0x100) {
            pkt->PutInt(0x100,  h->experience_per_sphere[0]);
        }
        if (mask & 0x200) {
            pkt->PutInt(0x200,  h->experience_per_sphere[1]);
        }
        if (mask & 0x400) {
            pkt->PutInt(0x400,  h->experience_per_sphere[2]);
        }
        if (mask & 0x800) {
            pkt->PutInt(0x800,  h->experience_per_sphere[3]);
        }
        if (mask & 0x1000) {
            pkt->PutInt(0x1000, h->experience_per_sphere[4]);
        }
    } else {
        mask &= 0xffffe0ff;
    }

    // bit 0x20000000: base stats.
    if (mask & 0x20000000) {
        // Effective body/reaction/mind/spirit.
        pkt->PutByte(0x20000000, static_cast<uint8_t>(unit->body));
        pkt->PutByte(0x20000000, static_cast<uint8_t>(unit->reaction));
        pkt->PutByte(0x20000000, static_cast<uint8_t>(unit->mind));
        pkt->PutByte(0x20000000, static_cast<uint8_t>(unit->spirit));
        // Base values: effective minus equipment bonus.
        pkt->PutByte(0x20000000, static_cast<uint8_t>(unit->body     - unit->equipment_extra.body));
        pkt->PutByte(0x20000000, static_cast<uint8_t>(unit->reaction - unit->equipment_extra.reaction));
        pkt->PutByte(0x20000000, static_cast<uint8_t>(unit->mind     - unit->equipment_extra.mind));
        pkt->PutByte(0x20000000, static_cast<uint8_t>(unit->spirit   - unit->equipment_extra.spirit));
    }

    // bit 0x80000000: unit name (24 chars)
    if (mask & 0x80000000u) {
        int nlen = unit->name.GetLength();
        if (nlen == 0) {
            mask &= ~0x80000000u;
        } else {
            char* dst = (char*)pkt->data + pkt->data_offset;
            strncpy(dst, unit->name, 24);
            dst[23] = 0;
            pkt->data_offset += 24; // 24 + terminator that already nulled
            pkt->flags_mask |= 0x80000000u;
        }
    }

    // bit 0x800000: enchantments.
    bool has_enchantments = mask & 0x800000;
    if (has_enchantments) {
        pkt->flags_mask |= 0x800000;
    }

    // Send the packet.
    if (mask & 0xbf1fffff) {
        this->QueuePacketSend(pkt);
    }

    if (orig_mask & 0x400000) {
        this->SendHumanoidProps(unit, player, props_flags);
    }

    // Owner-only: send inventory.
    if (orig_mask & 0x200000 && unit->pOwner == player) {
        this->SendMonsterInventory(unit, player, inv_start_index, inv_end_index);
    }

    bool needs_refresh = unit->NeedsRefreshFor(player);

    // Send effects.
    if (needs_refresh || has_enchantments) {
        POSITION iter = unit->_effects.GetHeadPosition();
        while (iter != nullptr) { 
            Effect* effect = unit->_effects.GetNext(iter);
            if (effect && effect->itemDataID != 0) {
                this->BroadcastEffect(effect, unit, 0);
            }
        }
    }

    if ((needs_refresh && (unit->unit_attrs & 8) != 0) || (mask & 0x40000000) != 0) {
        g_NetStru1_main.SendUnitVisibility(unit, (unit->unit_attrs & 8) ? 1 : 0, player);
    }

    // Mark unit as not needing the refresh in the "field_x18" bitmask.
    unit->ClearRefreshFor(player);

    // Clear per-player pending bits for players in the 0x10..0x1F range.
    if (0x10 <= player->player_id && player->player_id < 0x20) {
        unit->something_per_player[player->player_id - 16] = 0;
    }
}

// DecodePacket
Packet* NetStru1::DecodePacket(uint8_t cmd, NetStru2* ns2) {
    Packet* pkt;

    switch (cmd) {
    case 0x2:
        pkt = &PacketEnterGame::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x3: case 0xaf: case 0xb7: case 0xb8:
        pkt = &Packet::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x4: case 0xb: case 0x39: case 0x3b: case 0x5c: case 0xca:
    case 0xcc: case 0xd0: case 0xd5: case 0xd8: case 0xe1:
        pkt = &Packet2Dwords::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x5: case 0x9: case 0xa: case 0x4a: case 0x4b: case 0x4c:
    case 0x5a: case 0xc8: case 0xd6: case 0xdb: case 0xdc:
        pkt = &Packet::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x6: case 0x7: case 0x8: case 0x4e: case 0x5b: case 0x93:
    case 0xbf: case 0xc9: case 0xd1: case 0xd2: case 0xd3:
        pkt = &PacketJoin::Inst;
        pkt->ReadFrom(ns2);
        break;

    default:
        LogMessage(CString("Drv: unknown cmd type!"));
        pkt = &Packet::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0xe: case 0x10: case 0xb3: case 0xc1: case 0xdf:
        pkt = &PacketDword::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0xf: case 0x49: case 0x91: case 0x96: case 0xae:
        pkt = &PacketJoin::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x12: case 0x13: case 0x14: case 0x17: case 0x18: case 0x4d:
        pkt = &PacketGroupOrder::Inst;
        pkt->ReadFrom(ns2);
        pkt->order_mode = 1;
        break;

    case 0x16: case 0x1a: case 0x1c: case 0x1d: case 0x1f: case 0x21: case 0x24: case 0x26:
        pkt = &PacketGroupOrder::Inst;
        pkt->ReadFrom(ns2);
        pkt->order_mode = 2;
        break;

    case 0x19: case 0x1b: case 0x1e: case 0x25:
        pkt = &PacketGroupOrder::Inst;
        pkt->ReadFrom(ns2);
        pkt->order_mode = 3;
        break;

    case 0x22:
        pkt = &PacketItemTransfer::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x23: case 0x3a: case 0x3d: case 0x3e: case 0x3f: case 0x40:
    case 0x46: case 0x48: case 0x83: case 0x84: case 0x92: case 0x94:
    case 0xaa: case 0xc3:
        pkt = &Packet2Dwords::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x38:
    case 0x69: case 0x6a: case 0x74:
        pkt = &PacketWord::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x45: case 0x8a: case 0x8c: case 0x9b: case 0xb9: case 0xbb: case 0xbc:
        pkt = &PacketTerrain::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x64:
        pkt = &PacketDword::Inst;
        pkt->ReadFrom(ns2);
        ns2->ReleaseNetBuf();
        break;

    case 0x65: case 0x67: case 0x97: case 0xab: case 0xac: case 0xad:
    case 0xb4: case 0xb5: case 0xb6:
        pkt = &Packet2Dwords::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x6b: case 0x6d: case 0x71:
        pkt = &PacketUnitAction::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x6c: case 0x6e: case 0x6f: case 0x70:
        pkt = &PacketUnitUpdate::Inst;
        pkt->id = cmd;
        pkt->ReadFrom(ns2);
        break;

    case 0x72:
        pkt = &PacketRangedAttack::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x73:
        pkt = &PacketUnitHp::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x76:
        pkt = &PacketUnitStateVec::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x7a:
        pkt = &PacketSack::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x82:
        pkt = &PacketBuildingHp::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x86: case 0x8b:
        pkt = &PacketMoveBroadcast::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x87:
        pkt = &PacketAoeZone::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x88: case 0x89:
        pkt = &PacketEffect::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0x9c:
        pkt = &PacketUnitEquipment::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0xba: case 0xc2: case 0xcb: case 0xcd: case 0xce: case 0xcf:
    case 0xd4: case 0xd9: case 0xda: case 0xdd: case 0xde: case 0xe0:
        pkt = &PacketData::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0xbe:
        pkt = &PacketData::Inst;
        pkt->ReadFrom(ns2);
        break;

    case 0xc0:
        pkt = &PacketFileChunk::Inst;
        pkt->ReadFrom(ns2);
        break;
    }

    pkt->id = cmd;

    if (this->make_statistics == 0) {
        pkt->from_player_id = 0;
    } else if (ns2->player_id == 0) {
        pkt->from_player_id = static_cast<uint16_t>((ns2->uid & 0x3FFF) | 0x4000);
    } else {
        pkt->from_player_id = ns2->player_id;
    }

    return pkt;
}

Packet* NetStru1::ReceivePacket(NetStru2* cli) {
    //518a23
    if (cli->GetBuffersPktNum() == 0) {
        return nullptr;
    }

    uint8_t id = 0;
    cli->ReadData(&id, 1); //read pkt id
    return this->DecodePacket(id, cli);
}

Packet* NetStru1::ReceiveAnyPacket() {
    //518980
    for (POSITION it = this->active_connects.GetHeadPosition(); it != nullptr;) {
        NetStru2* cli = this->active_connects.GetNext(it);
        if (cli) {
            Packet* pkt = this->ReceivePacket(cli);
            if (pkt) {
                if (this->make_statistics == 0) { // ????? bug?  fixme?
                    this->AddStat1(cli, pkt->GetDataSize());
                    this->AddPacketStatistic(pkt);
                }
                return pkt;
            }
        }
    }
    return nullptr;
}




void NetStru1::AddTailNet2(NetStru2*& net2) {
    //517e91
    EnterCriticalSection(&this->critical_section);
    this->disconnect_list.AddTail(net2);
    LeaveCriticalSection(&this->critical_section);
}

void NetStru1::AddTailFreeNet3(NetStru3* net3) {
    //517b00
    EnterCriticalSection(&this->critical_section2);
    NetStru1* pool = this;
    if (this->linked_hl && make_statistics == 0) {
        pool = this->linked_hl;
    }
    pool->free_net3.AddHead(net3);
    LeaveCriticalSection(&this->critical_section2);
}

void NetStru1::ProcessNewConnects() {
    //51755c
    EnterCriticalSection(&this->critical_section);
    while (!this->new_connects.IsEmpty()) {
        NetStru2* cli = this->new_connects.RemoveHead();
        if (cli) {
            if (this->compression_mode != 0) {
                cli->compression_type = this->compression_type;
            } else {
                cli->compression_type = 0;
            }

            this->active_connects.AddTail(cli);

            this->OnClientConnect(cli);
        }
    }
    LeaveCriticalSection(&this->critical_section);
}

void NetStru1::ProcessDisconnectList() {
    //51725b
    EnterCriticalSection(&this->critical_section);
    while (!this->disconnect_list.IsEmpty()) {
        NetStru2* cli = this->disconnect_list.RemoveHead();
        if (cli) {
            POSITION pos = this->active_connects.Find(cli);
            if (pos) {
                this->OnClientDisconnect(cli);
                this->active_connects.RemoveAt(pos);
            }

            delete cli;
        }
    }
    LeaveCriticalSection(&this->critical_section);
}

void NetStru1::ProcessConnections() {
    // 51800f
    this->ProcessNewConnects();
    this->ProcessDisconnectList();
    if (this->lldriver) {
        this->lldriver->CleanupAllInvalid();
    }
}

void NetStru1::DisconnectClient(NetStru2* cli) {
    //5170b6
    if (cli == this->local_client) {
        this->AddTailNet2(this->local_client);
        this->local_client = nullptr;
    } else {
       this->lldriver->DisconnectClient(cli->uid);
    }
}

NetStru1::NetStru1(int param) {
    //51684b
    this->lldriver = nullptr;
    this->linked_hl = nullptr;
    this->local_client = nullptr;
    this->make_statistics = param;
    InitializeCriticalSection(&this->critical_section);
    InitializeCriticalSection(&this->critical_section2);
    this->field_0x18a4 = 0;
    this->field_0x18a0 = 0;
    this->field_0x189c = 0;
    this->field_0x1898 = 0;
    this->compression_mode = 0;
    this->compression_type = 1;
    memset(this->stat_pkt_num, 0, sizeof(this->stat_pkt_num));
    memset(this->stat_pkt_size, 0, sizeof(this->stat_pkt_size));
}

NetStru1::~NetStru1() {
    //516b44
    if (this->local_client != nullptr) {
        this->DisconnectClient(this->local_client);
    }

    this->ProcessNewConnects();
    this->ProcessDisconnectList();

    //inline 51710f 
    while (!this->free_net3.IsEmpty()) {
        NetStru3* buf = this->free_net3.RemoveHead();
        delete buf;
    }

    DeleteCriticalSection(&this->critical_section);
    DeleteCriticalSection(&this->critical_section2);

    this->lldriver = nullptr;
    this->linked_hl = nullptr;
    this->make_statistics = 0;
    this->field_0x18a4 = 0;
    this->field_0x18a0 = 0;
    this->field_0x189c = 0;
    this->field_0x1898 = 0;
    this->compression_mode = 0;
    this->compression_type = 1;
}


void NetStru1::OnClientConnect(NetStru2* cli) {
    //51fc72
    if (this != &HatConnector) {
        this->client_stat[cli->uid] = new ConnStatInfo;

        if (this->lldriver && lldriver->provider != 4) {
            cli->is_claimed = 1;
        } else {
            cli->is_claimed = 0;
        }
    }
}

void NetStru1::OnClientDisconnect(NetStru2* cli) {
    //51f561
    if (this == &HatConnector) {
        LogMessage("Connection with hat has been lost.");
        if (g_Server) {
            g_Server->field51_0x1d8 = GetTickCount();
        }
        return;
    }

    if (this->lldriver && lldriver->is_server != 0 && lldriver->provider == 1) {
        lldriver->RestartModemServerDp();
    }

    if (g_Server) {
        Player* pl = g_PlayersList->FindById(cli->player_id);
        if (pl && pl->drop_deadline_tick16 == 0) {
            if (pl->is_ai == 1 || pl->player_id <= 15) {
                LogMessage("Warning: trying to disconnect AI player " + pl->name);
            } else {
                LogMessage("Player " + pl->name + " has disconnected.");

                pl->Deactivate();

                if (lldriver && lldriver->provider == 4 &&
                    g_ServerConfig.gameType != 1 &&
                    g_ServerConfig.gameType != 3 &&
                    g_Server->field59_0x208 == 0) {
                    if (g_ServerConfig.gameType == 2) {
                        if (g_PlayersList->AreTeamsStarted()) {
                            pl->drop_deadline_tick16 = MAXINT32;
                        }
                    } else {
                        pl->drop_deadline_tick16 = g_Server->tick16 + 300;
                    }
                }
                if (!MapStuff_Instance && g_Server->field4_0x74) {
                    this->SendInfoPacket(0x97, pl->player_id, 0, nullptr);
                    LogMessage("Player " + pl->name + " has disconnected and leaved game.");

                    POSITION pos = g_PlayersList->list.Find(pl);
                    if (pos != nullptr) {
                        g_PlayersList->list.RemoveAt(pos);
                    }

                    delete pl;

                    g_Server->UpdateHatAnnounce();
                }
            }
        }
    }

    ConnStatInfo* stat = nullptr;
    if (this->client_stat.Lookup(cli->uid, stat)) {
        this->client_stat.RemoveKey(cli->uid);
        if (stat) {
            delete stat;
        }
    }
}


NetStru2* NetStru1::AllocClientBufManager(uint32_t uid) {
    //517c99
    EnterCriticalSection(&this->critical_section);

    NetStru2* mgr = new NetStru2();
    mgr->uid = uid;
    mgr->SetDrivers(this, this->lldriver);

    this->new_connects.AddTail(mgr);
    LeaveCriticalSection(&this->critical_section);

    return mgr;
}


NetStru3* NetStru1::GetFreeNet3() {
    //5177f5
    EnterCriticalSection(&this->critical_section2);
    NetStru1* pool = this;
    if (this->linked_hl && make_statistics == 0) {
        pool = this->linked_hl;
    }

    if (!pool->free_net3.IsEmpty() && pool->free_net3.GetHeadPosition() == NULL) { //Seems HACK
        new(&pool->free_net3) CList<NetStru3*>(); //Here was memcpy with stack allocated Clist in Nival code >_<
    }

    NetStru3* buf = nullptr;
    if (pool->free_net3.IsEmpty()) {
        buf = new NetStru3();
    } else {
        buf = pool->free_net3.RemoveHead();
    }
    LeaveCriticalSection(&this->critical_section2);
    return buf;
}

NetStru2* NetStru1::GetClientByPlayerID(uint16_t player_id) {
    //518544
    for (POSITION it = this->active_connects.GetHeadPosition(); it != nullptr;) {
        NetStru2* cli = this->active_connects.GetNext(it);
        if (cli->player_id == player_id) {
            return cli;
        }
    }

    if (this->make_statistics == 0 && this->local_client != nullptr) {
        return this->local_client;
    }

    return nullptr;
}

NetStru2* NetStru1::GetClientByLowUid(uint32_t low_uid) {
    //5185d5
    uint32_t luid = low_uid & 0x3fff;
    for (POSITION it = this->active_connects.GetHeadPosition(); it != nullptr;) {
        NetStru2* cli = this->active_connects.GetNext(it);
        if ((cli->uid & 0x3fff) == luid) {
            return cli;
        }
    }
    return nullptr;
}

void NetStru1::AddStat1(NetStru2* client, int32_t value) {
    //51eff8
    ConnStatInfo* stat = nullptr;
    this->client_stat.Lookup(client->uid, stat);

    if (stat) {
        stat->field1 += value;
    }
}

void NetStru1::AddPacketStatistic(Packet* pkt) {
    //51f0bb
    this->stat_pkt_num[pkt->id]++;
    this->stat_pkt_size[pkt->id] += pkt->GetDataSize();
}

void NetStru1::SendAllData() {
    //5188db
    //518927
    for (POSITION it = this->active_connects.GetHeadPosition(); it != nullptr;) {
        NetStru2* cli = this->active_connects.GetNext(it);
        if (cli) {
            cli->SendData();
        }
    }
}

void NetStru1::SendPacket_64(uint32_t val, uint16_t player_id) {
    //51d88d
    PacketDword& pkt = PacketDword::Inst;
    pkt.id = 0x64;
    pkt.value = val;
    pkt.to_player_id = player_id;
    this->QueuePacketSend(&pkt);

#ifdef A2CLIENT
    this->SendAllData();
#else
    if ((val & 3) == 1)
        SendAllData();
#endif
}

void NetStru1::SetLLDriver(CLlDriver* drv) {
    //517ff9
    this->lldriver = drv;
}

void NetStru1::SetLinkedHLDriver(NetStru1* hl) {
    //51703e
    this->linked_hl = hl;
    if (this->local_client) {
        this->DisconnectClient(this->local_client);
    }
    this->local_client = this->AllocClientBufManager(-1);
    this->local_client->is_local_player = 1;
    this->local_client->compression_type = 0;
}

void NetStru1::QueuePacketSend(Packet* pkt) {
    //5186cd

    //uint32_t stime = GetTickCount();
    //printf("Send packet 0x%x\n", pkt->id);
    if (pkt->to_player_id == 0) {
        for (POSITION it = this->active_connects.GetHeadPosition(); it != nullptr;) {
            NetStru2* cli = this->active_connects.GetNext(it);
            if (cli && (this->make_statistics == 0 || cli->player_id != 0)) {
                pkt->WriteTo(cli);
                if (this->make_statistics != 0) {
                    this->AddStat1(cli, pkt->GetDataSize());
                    this->AddPacketStatistic(pkt);
                }
                if (pkt->id == 0x64) {
                    cli->out_buffers[cli->out_buff_id].pkt_num++;
                }
            }
        }
    } else {
        NetStru2* cli = this->GetClientByPlayerID(pkt->to_player_id);
        if (cli) {
            pkt->WriteTo(cli);
            if (this->make_statistics != 0) {
                this->AddStat1(cli, pkt->GetDataSize());
                this->AddPacketStatistic(pkt);
            }
            if (pkt->id == 0x64) {
                cli->out_buffers[cli->out_buff_id].pkt_num++;
            }
        }
    }

    if (this->make_statistics != 0 && pkt->id == 0x76) {
        //DAT_0070b410 += (GetTickCount() - stime);
    }

    if (this->make_statistics == 0 && pkt->id != 0x64) {
        this->SendPacket_64(1, 0);
    }
}


uint32_t NetStru1::GetClientsPktNum() {
    //518a6b
    uint32_t sum = 0;
    for (POSITION it = this->active_connects.GetHeadPosition(); it != nullptr;) {
        NetStru2* cli = this->active_connects.GetNext(it);
        if (cli) {
            sum += cli->GetBuffersPktNum();
        }
    }
    return sum;
}

int NetStru1::IsActive() {
    //518ac9
    if (!this->lldriver) {
        return this->linked_hl != nullptr;
    } else if (this->lldriver->listen_socket.is_in_use == 1 || this->linked_hl != nullptr) {
        return 1;
    }
    return 0;
}


NetStru2* NetStru1::GetFirstClient() {
    if (this->active_connects.IsEmpty()) {
        return nullptr;
    }

    return this->active_connects.GetHead();
}




int NetStru2::WriteData(void* buf, uint32_t size) {
    //515ef3
    while(true) {
        int32_t avail = 0x8e - this->out_buffers[this->out_buff_id].datasz;
        if (size <= avail) {
            break;
        }

        this->out_buffers[this->out_buff_id].WriteToBuffer(buf, avail);
        this->SendData();

        buf = (char*)buf + avail;
        size -= avail;
    }
    
    this->out_buffers[this->out_buff_id].WriteToBuffer(buf, size);
    return 1;
}

int NetStru2::ReadData(void* buf, uint32_t size) {
    //515f9c
    EnterCriticalSection(&this->critical_section);
    while (!this->received_buffers.IsEmpty()) {
        NetStru3* buffer = this->received_buffers.GetHead();
        int32_t remain = buffer->datasz - buffer->readpos;
        
        if (size <= remain) {
            buffer->ReadFromBuffer(buf, size);
            LeaveCriticalSection(&this->critical_section);
            return 1;
        }

        buffer->ReadFromBuffer(buf, remain);

        size -= remain;
        buf = (char*)buf + remain;
        
        this->received_buffers.RemoveHead();
        this->hldriver->AddTailFreeNet3(buffer);
    }
    LeaveCriticalSection(&this->critical_section);
    return 0;
}



void NetStru2::SetDrivers(NetStru1* HlDriver, CLlDriver* LlDriver) {
    //51682d
    this->hldriver = HlDriver;
    this->lldriver = LlDriver;
}

int NetStru2::ReceiveData(NetStru3* buffer) {
    //5161bf
    EnterCriticalSection(&this->critical_section);

    /*if (buffer->cmode != 0)
    {
        NetStru3* tbuf = hldriver->GetFreeNet3();
        tbuf->Clear();
        int num = hldriver->Unpack(buffer->cmode, buffer->buf, buffer->csize, tbuf->buf, 0x8e);
        if (num > 0x8e)
        {
            ReportWarning("CBufferManager::ReceiveData().\nError during unpacking buffer.\n");
            hldriver->AddTailFreeNet3(buffer);
            hldriver->AddTailFreeNet3(tbuf);
            LeaveCriticalSection(&critical_section);
            return 0;
        }
        tbuf->datasz = num;
        tbuf->pkt_num = buffer->pkt_num;
        hldriver->AddTailFreeNet3(buffer);
        buffer = tbuf;
    }*/

    this->received_buffers.AddTail(buffer);

    LeaveCriticalSection(&this->critical_section);
    return 1;
}

int NetStru2::SendData() {
    //518408
    if (this->out_buffers[this->out_buff_id].datasz < 1) {
        if (this->lldriver) {
            this->lldriver->CleanupInvalidTcpClient(this->uid);
        }
        return 1;
    }

    NetStru3* buffer = this->hldriver->GetFreeNet3();
    buffer->Clear();
    buffer->cmode = this->compression_type;

    //here must be compression_type check and pack, but we just drop it

    buffer->CopyData(this->out_buffers + this->out_buff_id);
    buffer->pkt_num = this->out_buffers[this->out_buff_id].pkt_num;

    // inline FUN_00515eb2();
    this->out_buff_id = (this->out_buff_id + 1) & 1;
    this->out_buffers[this->out_buff_id].Clear();

    if (this->is_local_player == 0) {
        if (!this->lldriver) {
            this->hldriver->AddTailFreeNet3(buffer);
            return 0;
        }

        this->lldriver->SendData(this->uid, buffer);
    } else {
        if (!this->hldriver) {
            this->hldriver->AddTailFreeNet3(buffer);
            return 0;
        }

        NetStru2* recv_cli = nullptr;
        if (this->hldriver->linked_hl) {
            recv_cli = this->hldriver->linked_hl->local_client;
        }
        
        if (!recv_cli) {
            this->hldriver->AddTailFreeNet3(buffer);
            return 0;
        }

        recv_cli->ReceiveData(buffer);
    }
    return 1;
}

NetStru2::NetStru2() {
    //515831
    this->field_0x2a0 = 0;
    this->field_0x2a4 = 0;
    this->is_claimed = 0;
    this->field_0x2ac = 0;
    this->field_0x2b4 = -1;
    this->str.Empty();
    this->field_0x2b8 = 0;
    this->hldriver = nullptr;
    this->lldriver = nullptr;
    this->out_buff_id = 0;
    this->out_buffers[0].Clear();
    this->out_buffers[1].Clear();
    this->compression_type = 0;
    this->player_id = 0;
    this->uid = 0;
    this->is_local_player = 0;
    this->buf[0] = 0;
    InitializeCriticalSection(&this->critical_section);
}

NetStru2::~NetStru2() {
    //515c27
    this->ReturnBuffers();
    DeleteCriticalSection(&this->critical_section);
    this->is_local_player = 0;
    this->player_id = 0;
    this->uid = 0;
    this->hldriver = nullptr;
    this->lldriver = nullptr;
    this->buf[0] = 0;
}

void NetStru2::ReturnBuffers() {
    //515d9d
    if (!this->hldriver) {
        return;
    }

    EnterCriticalSection(&this->critical_section);

    while (!this->received_buffers.IsEmpty()) {
        this->hldriver->AddTailFreeNet3( this->received_buffers.RemoveHead() );
    }

    LeaveCriticalSection(&this->critical_section);
}


void NetStru2::ReleaseNetBuf() {
    //5167A5
    EnterCriticalSection(&this->critical_section);
    for (POSITION pos = this->received_buffers.GetHeadPosition(); pos != nullptr;) {
        NetStru3* buffer = this->received_buffers.GetNext(pos);
        if (buffer->pkt_num != 0) {
            buffer->pkt_num--;
            break;
        }
    }
    LeaveCriticalSection(&this->critical_section);
}

uint32_t NetStru2::GetBuffersPktNum() {
    //51670a  Get received_buffers summary of pkt_num
    uint32_t sum = 0;
    EnterCriticalSection(&this->critical_section);
    for (POSITION pos = this->received_buffers.GetHeadPosition(); pos != nullptr;) {
       sum += this->received_buffers.GetNext(pos)->pkt_num;        
    }
    LeaveCriticalSection(&this->critical_section);
    return sum;
}




void NetStru3::Clear() {
    //5156f6
    this->datasz = 0;
    this->readpos = 0;
    this->pkt_num = 0;
    this->timestamp = 0;
    this->timestamp2 = 0;
    this->csize = 0;
}

NetStru3::NetStru3() {
    //5155c0
    memset(this->full_data, 0, sizeof(this->full_data));
    this->pktid = 0;
    this->cmode = 0;
    this->pkt_num = 0;
    this->datasz = -1;
    this->readpos = -1;
    this->timestamp = 0;
    this->timestamp2 = 0;
}

NetStru3::~NetStru3() {
    //515694
    this->datasz = -1;
    this->readpos = -1;
    this->timestamp = 0;
    this->timestamp2 = 0;
    this->pktid = 0;
    this->cmode = 0;
    this->pkt_num = 0;
    memset(this->full_data, 0, sizeof(this->full_data));
}

void NetStru3::CopyData(const NetStru3* src) {
    //515625
    memcpy(this->full_data, &src->full_data, src->datasz + 8);
    this->datasz = src->datasz;
    this->readpos = src->readpos;
    this->timestamp = src->timestamp;
    this->timestamp2 = src->timestamp2;
}

int NetStru3::WriteToBuffer(void* data, uint32_t sz) {
    //51573e
    uint32_t newsz = this->datasz + sz;
    if (newsz >= 143) {
        return false;
    }

    memcpy(this->buf + this->datasz, data, sz);
    this->datasz = newsz;
    return true;
}

int NetStru3::ReadFromBuffer(void* out, uint32_t sz) {
    //51579e
    uint32_t newpos = this->readpos + sz;
    if (newpos > this->datasz) {
        return false;
    }

    memcpy(out, this->buf + this->readpos, sz);
    this->readpos = newpos;
    return true;
}


A2NetSock::A2NetSock() {
    //520510
    this->is_in_use = 0;
    this->uid = 0;
    this->socket = INVALID_SOCKET;
    this->player_dpid = 0;
    this->manager = nullptr;
    this->next_in_pktid = 0;
    this->next_out_pktid = 0;
    this->current_buffer = nullptr;
    this->copy_num = -1;
    this->wait_obj = nullptr;
    this->latency = 0;
    this->stat_pkts_queued = 0;
    this->stat_pkts_sent = 0;
}

A2NetSock::~A2NetSock() {
    //520684
    this->is_in_use = 0;
    this->uid = 0;
    this->socket = INVALID_SOCKET;
    this->wait_obj = nullptr;
    this->player_dpid = 0;
    this->manager = nullptr;
    this->next_in_pktid = 0;
    this->next_out_pktid = 0;
    this->current_buffer = nullptr;
    this->copy_num = -1;
    this->latency = 0;
    this->stat_pkts_queued = 0;
    this->stat_pkts_sent = 0;
}

void NetSockLatency::AddLatency(uint32_t lat_time) {
    //51fdf7
    static std::array<uint32_t, 128> storage;

    this->lat_times[this->num] = lat_time;
    this->num++;

    std::array<uint32_t, 256> count1;
    std::array<uint32_t*, 256> arrPos1;
    std::array<uint32_t, 256> count2;
    std::array<uint32_t*, 256> arrPos2;
    std::array<uint32_t, 256> count3;
    std::array<uint32_t*, 256> arrPos3;
    std::array<uint32_t, 256> count4;
    std::array<uint32_t*, 256> arrPos4;

    if (this->num >= 128) {
        this->num = 0;

        count1.fill(0);
        for (int i = 0; i < 128; i++) {
            count1[ShiftToByte(this->lat_times[i], 0)]++;
        }

        uint32_t* ppos = storage.data();
        for (int i = 0; i < 256; i++) {
            arrPos1[i] = ppos;
            ppos += count1[i];
        }

        for (int i = 0; i < 128; i++) {
            int idx = ShiftToByte(this->lat_times[i], 0);
            *arrPos1[idx] = this->lat_times[i];
            arrPos1[idx]++;
        }


        count2.fill(0);
        for (int i = 0; i < 128; i++) {
            count2[ShiftToByte(storage[i], 8)]++;
        }

        ppos = this->lat_times;
        for (int i = 0; i < 256; i++) {
            arrPos2[i] = ppos;
            ppos += count2[i];
        }

        for (int i = 0; i < 128; i++) {
            int idx = ShiftToByte(storage[i], 8);
            *arrPos2[idx] = storage[i];
            arrPos2[idx]++;
        }


        count3.fill(0);
        for (int i = 0; i < 128; i++) {
            count3[ShiftToByte(this->lat_times[i], 16)]++;
        }

        ppos = storage.data();
        for (int i = 0; i < 256; i++) {
            arrPos3[i] = ppos;
            ppos += count3[i];
        }

        for (int i = 0; i < 128; i++) {
            int idx = ShiftToByte(this->lat_times[i], 16);
            *arrPos3[idx] = this->lat_times[i];
            arrPos3[idx]++;
        }


        count4.fill(0);
        for (int i = 0; i < 128; i++) {
            count4[ShiftToByte(storage[i], 24)]++;
        }

        ppos = this->lat_times;
        for (int i = 0; i < 256; i++) {
            arrPos4[i] = ppos;
            ppos += count4[i];
        }

        for (int i = 0; i < 128; i++) {
            int idx = ShiftToByte(storage[i], 24);
            *arrPos4[idx] = storage[i];
            arrPos4[idx]++;
        }

        this->calc_latency = this->lat_times[64];
    }
    if (this->calc_latency < 25) {
        this->calc_latency = 25;
    }
    if (this->calc_latency > 1000) {
        this->calc_latency = 1000;
    }
}




CLlDriver::CLlDriver(GUID _appid, NetStru1* net1) {
    //5208ec
    this->application_guid = _appid;
    this->hl_driver = net1;
    this->is_server = 0;
    this->guaranteed = 0;
    this->latency = 0xffffffff;
    this->max_connections = -1;
    this->session_lost = 0;
    this->provider = 5;
    this->connection_sockets = nullptr;
    this->connections_info = nullptr;
    this->enum_addresses = nullptr;
    this->enum_addresses_num = 0;
    this->enum_sessions = nullptr;
    this->enum_sessions_num = 0;
    this->enum_conns = nullptr;
    this->enum_conns_num = 0;
    this->enum_players = nullptr;
    this->enum_players_num = 0;
    this->address_str[0] = 0;
    this->next_uid = 0x3bef0000;
    this->unused = 0;
    this->keepalive = 0;
    this->dplay_is_4 = 0;
    this->timeout = 12000;
    InitializeCriticalSection(&this->critical_section);
    this->ev_create_player = nullptr;
    this->ev_close = nullptr;
    this->ev_new_session = nullptr;
    this->dplay4 = nullptr;
    CoInitialize(NULL);
}

CLlDriver::~CLlDriver() {
    //520ad3

    this->Close();
    this->FreeEnumAddresses();
    this->FreeEnumSessions();
    this->FreeEnumConns();
    this->FreeEnumPlayers();

    if (this->connection_sockets) {
        delete[] this->connection_sockets;
        this->connection_sockets = nullptr;
    }

    if (this->connections_info) {
        delete[] this->connections_info;
        this->connections_info = nullptr;
    }

    DeleteCriticalSection(&this->critical_section);

    this->Free();

    CoUninitialize();
}

void CLlDriver::SetHlDriver(NetStru1* drv) {
    //496e50
    this->hl_driver = drv;
}


void CLlDriver::Close() {
    //5225d0
    if (this->listen_socket.is_in_use) {
        if (this->provider == 4) {
            this->CloseTcp();
        } else {
            this->CloseDp();
        }
    }
    this->listen_socket.is_in_use = 0;
    this->is_server = 0;
}



void CLlDriver::CloseTcp() {
    //523b01
    this->CloseTcpSocket(&this->listen_socket);
    this->CloseTcpSocket(&this->ping_socket);
    if (this->is_server) {
        for (uint32_t i = 0; i < this->max_connections; i++) {
            this->CloseTcpSocket(this->connection_sockets + i);
        }
    }
}

void CLlDriver::CloseTcpSocket(A2NetSock* sock) {
    //523a16
    if (sock->is_in_use) {
        this->num_connections--;
    }

    sock->is_in_use = 0;
    if (sock->socket != INVALID_SOCKET) {
        closesocket(sock->socket);
        sock->socket = INVALID_SOCKET;
    }

    if (sock->wait_obj) {
        WaitForSingleObject(sock->wait_obj, -1);
        CloseHandle(sock->wait_obj);
        sock->wait_obj = nullptr;
    }

    if (sock->current_buffer) {
        this->hl_driver->AddTailFreeNet3(sock->current_buffer);
        sock->current_buffer = nullptr;
        sock->copy_num = -1;
    }

    if (sock->manager) {
        this->hl_driver->AddTailNet2(sock->manager);
        sock->manager = nullptr;
    }
}


void CLlDriver::CloseDp() {
    //525a09
    if (this->listen_socket.wait_obj) {
        SetEvent(this->ev_close);
        WaitForSingleObject(this->listen_socket.wait_obj, -1);
        CloseHandle(this->listen_socket.wait_obj);
        this->listen_socket.wait_obj = nullptr;
    }

    if (this->is_server) {
        for (uint32_t i = 0; i < this->max_connections; i++) {
            this->CloseDpSock(this->connection_sockets + i, -1);
        }
    } else {
        this->listen_socket.is_in_use = 0;
        if (this->listen_socket.manager) {
            this->hl_driver->AddTailNet2(this->listen_socket.manager);
            this->listen_socket.manager = nullptr;
        }
    }

    if (this->ev_create_player) {
        CloseHandle(this->ev_create_player);
        this->ev_create_player = nullptr;
    }

    if (this->ev_close) {
        CloseHandle(this->ev_close);
        this->ev_close = nullptr;
    }

    if (this->dplay4) {
        if (this->listen_socket.player_dpid != DPID_UNKNOWN) {
            this->dplay4->DestroyPlayer(this->listen_socket.player_dpid);
            this->listen_socket.player_dpid = DPID_UNKNOWN;
        }
        this->dplay4->Close();
    }
}

void CLlDriver::CloseDpSock(A2NetSock* sock, int idx) {
    //525bcd
    if (this->IsInUse(sock) == 0) {
        return;
    }

    if (this->is_server == 0) {
        this->Close();
        return;
    }

    if (this->dplay_is_4) {
        this->dplay4->DestroyPlayer(sock->player_dpid);
    }

    if (sock->manager) {
        this->hl_driver->AddTailNet2(sock->manager);
        sock->manager = nullptr;
    }

    sock->is_in_use = 0;

    while (!sock->list_0x14.IsEmpty()) {
        this->hl_driver->AddTailFreeNet3( sock->list_0x14.RemoveHead() );
    }
    sock->list_0x14.RemoveAll();

    while (!sock->list_0x30.IsEmpty()) {
        this->hl_driver->AddTailFreeNet3(sock->list_0x30.RemoveHead());
    }
    sock->list_0x30.RemoveAll();

    if (idx == -1) {
        for (uint32_t i = 0; i < this->num_connections; i++) {
            if (this->connections_info[i].sock == sock) {
                idx = i;
                break;
            }
        }
    }

    memmove(this->connections_info + idx, this->connections_info + idx + 1, (this->num_connections - idx - 1) * sizeof(SocketNm));
}


CLlAddress* CLlDriver::AllocAddress() {
    //520d8c
    if (this->enum_addresses == nullptr) {
        this->enum_addresses = (CLlAddress*)malloc(sizeof(CLlAddress));
        this->enum_addresses_num = 1;
        return this->enum_addresses;
    } else {
        this->enum_addresses_num++;
        this->enum_addresses = (CLlAddress*)realloc(this->enum_addresses, sizeof(CLlAddress) * this->enum_addresses_num);
        return this->enum_addresses + (this->enum_addresses_num - 1);
    }
}

void CLlDriver::FreeEnumAddresses() {
    //520c80
    if (this->enum_addresses) {
        free(this->enum_addresses);
        this->enum_addresses = nullptr;
        this->enum_addresses_num = 0;
    }
}


int CLlDriver::EnumAddresses(CLlAddress** addrs, int* num) {
    //52182f
    this->FreeEnumAddresses();
    if (this->provider == 0) {
        CLlAddress* addr = this->AllocAddress();
        strcpy(addr->name, "COM1");
        addr->com.index = 1;
        addr = this->AllocAddress();
        strcpy(addr->name, "COM2");
        addr->com.index = 2;
        addr = this->AllocAddress();
        strcpy(addr->name, "COM3");
        addr->com.index = 3;
        addr = this->AllocAddress();
        strcpy(addr->name, "COM4");
        addr->com.index = 4;
    } else if (this->provider == 1) {
        this->EnumAddressesDp();
    } else if (this->provider == 4) {
        this->EnumAddressesTcp();
    } else {
        return 0;
    }

    *addrs = this->enum_addresses;
    *num = this->enum_addresses_num;
    return 1;
}


void CLlDriver::EnumAddressesTcp() {
    //5214c5
    OSVERSIONINFOA osver;
    osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    GetVersionExA(&osver);

    char path[260];
    memset(path, 0, sizeof(path));
    uint32_t len = 0;

    if (osver.dwPlatformId == 1) {
        len = GetWindowsDirectoryA(path, sizeof(path));
    } else {
        len = GetSystemDirectoryA(path, sizeof(path));
    }

    if (path[len] != '\\') { //??? path[len-1] ???
        strcat(path, "\\");
    }

    if (osver.dwPlatformId == 1) {
        strcat(path, "WinIpCfg.exe");
    } else {
        strcat(path, "IpConfig.exe");
    }

    _spawnlp(P_WAIT, path, path, "/ALL", "/BATCH", "IPINFO.$$$$$$", NULL);


    CLlAddress* addr = nullptr;
    char buf[2048];

    FILE* f = fopen("IPINFO.$$$$$$", "r");
    if (f) {
        while (true) {
            if (fgets(buf, sizeof(buf), f) == NULL) {
                break;
            }

            char* p = strchr(buf, ':');
            if (p) {
                *p = 0;
                p++;
                while (isspace(*p)) {
                    p++;
                }
                for (char* t = p + strlen(p) - 1; t != p && isspace(*t); t--) {
                    *t = 0;
                }

                if (strstr(buf, "Description") != NULL) {
                    addr = this->AllocAddress();
                    strcpy(addr->name, p);
                    strcpy(addr->address, "0.0.0.0");
                } else if (addr && strstr(buf, "IP Address") != NULL) {
                    strcpy(addr->address, p);
                    addr = nullptr;
                }
            }
        }

        fclose(f);
        _unlink("IPINFO.$$$$$$");
    }

    if (this->enum_addresses_num == 0) {
        if (!addr) {
            addr = this->AllocAddress();
        }

        strcpy(addr->name, "Default adapter");
        strcpy(addr->address, "0.0.0.0");
    }
}

void CLlDriver::EnumAddressesDp() {
    //521200
    IDirectPlayLobby2A* lobby = nullptr;
    if (CoCreateInstance(CLSID_DirectPlayLobby, NULL, 1, IID_IDirectPlayLobby2A, (LPVOID*)&lobby) != 0) {
        ReportWarning("CLlDriver::GetPossibleModems.\nUnable to create DirectPlayLobby.\n");
        return;
    }

    IDirectPlay3A* dplay = nullptr;
    if (CoCreateInstance(CLSID_DirectPlay, NULL, 1, IID_IDirectPlayLobby3A, (LPVOID*)&dplay) != 0) {
        ReportWarning("CLlDriver::GetPossibleModems.\nUnable to create DirectPlay.\n");
        lobby->Release();
        return;
    }

    char buf[4] = { 0 };
    DPCOMPOUNDADDRESSELEMENT elem[2];
    elem[0].guidDataType = DPAID_ServiceProvider;
    elem[0].dwDataSize = sizeof(GUID);
    elem[0].lpData = (VOID*)&DPSPGUID_MODEM;
    elem[1].guidDataType = DPAID_Phone;
    elem[1].dwDataSize = 1;
    elem[1].lpData = buf;

    DWORD AddrSize;
    if (lobby->CreateCompoundAddress(elem, 2, NULL, &AddrSize) != DPERR_BUFFERTOOSMALL) {
        lobby->Release();
        dplay->Release();
        return;
    }

    uint8_t* dpaddr = new uint8_t[AddrSize];
    if (lobby->CreateCompoundAddress(elem, 2, dpaddr, &AddrSize) != 0) {
        delete[] dpaddr;

        lobby->Release();
        dplay->Release();
        return;
    }

    if (dplay->InitializeConnection(dpaddr, 0) != 0) {
        delete[] dpaddr;
        ReportWarning("CLlDriver::GetPossibleModems.\nUnable to initialize DirectPlay.\n");
        lobby->Release();
        dplay->Release();
        return;
    }

    delete[] dpaddr;

    if (dplay->GetPlayerAddress(0, NULL, &AddrSize) != DPERR_BUFFERTOOSMALL) {
        ReportWarning("CLlDriver::GetPossibleModems.\nUnable to enum modems.\n");
        lobby->Release();
        dplay->Release();
        return;
    }

    dpaddr = new uint8_t[AddrSize];

    if (dplay->GetPlayerAddress(0, dpaddr, &AddrSize) != 0) {
        delete[] dpaddr;
        ReportWarning("CLlDriver::GetPossibleModems.\nUnable to enum modems(2).\n");
        lobby->Release();
        dplay->Release();
        return;
    }

    lobby->EnumAddress(cbEnumAddress, dpaddr, AddrSize, this);

    delete[] dpaddr;

    lobby->Release();
    dplay->Release();
}

BOOL __stdcall CLlDriver::cbEnumAddress(REFGUID guidDataType, DWORD dwDataSize, LPCVOID lpData, LPVOID lpContext) {
    //5211b2
    CLlDriver* drv = (CLlDriver*)lpContext;
    if (memcmp(&guidDataType, &DPAID_Modem, sizeof(GUID)) == 0) {
        CLlAddress* addr = drv->AllocAddress();
        strcpy(addr->name, (const char*)lpData);
    }
    return TRUE;
}




CLlNetSession* CLlDriver::AllocSession() {
    //520e2e
    if (this->enum_sessions == nullptr) {
        this->enum_sessions = (CLlNetSession*)malloc(sizeof(CLlNetSession));
        this->enum_sessions_num = 1;
        return this->enum_sessions;
    } else {
        this->enum_sessions_num++;
        this->enum_sessions = (CLlNetSession*)realloc(this->enum_sessions, sizeof(CLlNetSession) * this->enum_sessions_num);
        return this->enum_sessions + (this->enum_sessions_num - 1);
    }
}

void CLlDriver::FreeEnumSessions() {
    //520cc3
    if (this->enum_sessions) {
        free(this->enum_sessions);
        this->enum_sessions = nullptr;
        this->enum_sessions_num = 0;
    }
}

int CLlDriver::EnumSessions(CLlNetSession** sessions, int* num, CBUPDATECALLBACK cbUpdate, uint32_t timeout) {
    //52210d
    this->FreeEnumSessions();

    if (this->provider == 4) {
        return 0;
    }

    DPSESSIONDESC2 desc;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.guidApplication = this->application_guid;
    this->ev_new_session = CreateEventA(NULL, FALSE, FALSE, NULL);

    if (timeout == 0) {
        DPCAPS caps;
        caps.dwSize = sizeof(caps);
        if (this->dplay4->GetCaps(&caps, (this->guaranteed != 0 ? DPGETCAPS_GUARANTEED : 0)) != 0) {
            ReportWarning("CLlDriver::GetActiveServers().\nUnable to get caps.\n");
            return 1;
        }
        timeout = caps.dwTimeout;
    }

    uint32_t t1 = GetTickCount();
    while (true) {
        uint32_t t2 = GetTickCount();
        if (t2 - t1 >= timeout) {
            break;
        }

        HRESULT res = this->dplay4->EnumSessions(&desc, 0, cbEnumSessions, this, DPENUMSESSIONS_AVAILABLE | DPENUMSESSIONS_ASYNC | DPENUMSESSIONS_RETURNSTATUS);
        if (res != 0 && res != DPERR_CONNECTING) {
            break;
        }

        if (WaitForSingleObject(this->ev_new_session, 10) == 0) {
            break;
        }

        if (cbUpdate && cbUpdate() == 0) {
            this->dplay4->EnumSessions(&desc, 0, cbEnumSessions, this, DPENUMSESSIONS_STOPASYNC);
            break;
        }
    }

    CloseHandle(this->ev_new_session);
    this->ev_new_session = nullptr;
    
    *sessions = this->enum_sessions;
    *num = this->enum_sessions_num;

    return 1;
}

BOOL __stdcall CLlDriver::cbEnumSessions(LPCDPSESSIONDESC2 lpThisSD, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext) {
    //522020
    CLlDriver* drv = (CLlDriver*)lpContext;

    if ((dwFlags & DPENUMSESSIONS_AVAILABLE) != 0) {
        return FALSE;
    }

    if (!lpThisSD) {
        return FALSE;
    }

    for (uint32_t i = 0; i < drv->enum_sessions_num; i++) {
        if (memcmp(&drv->enum_sessions[i].guid, &lpThisSD->guidInstance, sizeof(GUID)) == 0) {
            return TRUE;
        }
    }

    CLlNetSession* ses = drv->AllocSession();
    strcpy(ses->name, lpThisSD->lpszSessionNameA);
    ses->guid = lpThisSD->guidInstance;

    if (drv->provider == 1 || drv->provider == 0) {
        drv->SetEventNewSession();
    }

    return TRUE;
}






CLlConn* CLlDriver::AllocEnumConns() {
    //520ed0
    if (this->enum_conns == nullptr) {
        this->enum_conns = (CLlConn*)malloc(sizeof(CLlConn));
        this->enum_conns_num = 1;
        return this->enum_conns;
    } else {
        this->enum_conns_num++;
        this->enum_conns = (CLlConn*)realloc(this->enum_conns, sizeof(CLlConn) * this->enum_conns_num);
        return this->enum_conns + (this->enum_conns_num - 1);
    }
}

void CLlDriver::FreeEnumConns() {
    //520d06
    if (this->enum_conns) {
        free(this->enum_conns);
        this->enum_conns = nullptr;
        this->enum_conns_num = 0;
    }
}

void CLlDriver::EnumConnections(CLlConn** conns, int* num) {
    //5210ea
    this->FreeEnumConns();

    CLlConn* conn = this->AllocEnumConns();
    strcpy(conn->name, "TCP/IP");
    conn->typ = 4;

    bool hasntdp = this->dplay4 == nullptr;

    if (hasntdp) {
        this->CreateDp();
    }

    if (this->dplay4) {
        this->dplay4->EnumConnections(&this->application_guid, cbEnumConnections, this, DPCONNECTION_DIRECTPLAY);
    }

    if (hasntdp) {
        this->FreeDp();
    }

    *conns = this->enum_conns;
    *num = this->enum_conns_num;
}

BOOL __stdcall CLlDriver::cbEnumConnections(LPCGUID lpguidSP, LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext) {
    //52100e
    CLlDriver* drv = (CLlDriver*)lpContext;

    int typ = -1;
    if (memcmp(lpguidSP, &DPSPGUID_MODEM, sizeof(GUID)) == 0) {
        typ = 1;
    } else if (memcmp(lpguidSP, &DPSPGUID_TCPIP, sizeof(GUID)) == 0) {
        typ = 3;
    } else if (memcmp(lpguidSP, &DPSPGUID_IPX, sizeof(GUID)) == 0) {
        typ = 2;
    } else if (memcmp(lpguidSP, &DPSPGUID_SERIAL, sizeof(GUID)) == 0) {
        typ = 0;
    } else {
        return TRUE;
    }

    CLlConn* conn = drv->AllocEnumConns();
    strcpy(conn->name, lpName->lpszShortNameA);
    conn->typ = typ;
    return TRUE;
}



CLlName* CLlDriver::AllocEnumPlayers() {
    //520f72
    if (this->enum_players == nullptr) {
        this->enum_players = (CLlName*)malloc(sizeof(CLlName));
        this->enum_players_num = 1;
        return this->enum_players;
    } else {
        this->enum_players_num++;
        this->enum_players = (CLlName*)realloc(this->enum_players, sizeof(CLlName) * this->enum_players_num);
        return this->enum_players + (this->enum_players_num - 1);
    }
}

void CLlDriver::FreeEnumPlayers() {
    //520d49
    if (this->enum_players) {
        free(this->enum_players);
        this->enum_players = nullptr;
        this->enum_players_num = 0;
    }
}

void CLlDriver::EnumPlayers(CLlNetSession* ses, CLlName** names, int* num) {
    //52238e
    this->FreeEnumPlayers();
    if (this->dplay4) {
        this->dplay4->EnumPlayers(&ses->guid, cbEnumPlayes, this, DPENUMPLAYERS_SESSION);
    }
    *names = this->enum_players;
    *num = this->enum_players_num;
}

BOOL __stdcall CLlDriver::cbEnumPlayes(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext) {
    //52233f
    CLlDriver* drv = (CLlDriver*)lpContext;

    if (!lpName->lpszShortNameA) {
        return FALSE;
    }

    if (dwPlayerType != DPPLAYERTYPE_PLAYER) {
        return TRUE;
    }

    CLlName *pl = drv->AllocEnumPlayers();
    strcpy(pl->name, lpName->lpszShortNameA);
    return TRUE;
}


void CLlDriver::Free() {
    //5219af
    if (this->listen_socket.is_in_use == 1) {
        this->Close();
    }

    if (this->provider != 5) {
        if (this->provider == 4) {
            this->FreeTcp();
        } else {
            this->FreeDp();
        }
        this->provider = 5;
    }
}

void CLlDriver::FreeTcp() {
    //523611
    WSACleanup();
}

void CLlDriver::FreeDp() {
    //523d5a
    if (this->dplay4) {
        this->dplay4->Release();
        this->dplay4 = nullptr;
    }
}





int CLlDriver::ResetProvider(int prov) {
    //52194f
    this->Free();

    this->provider = prov;
    if (prov == 4) {
        if (!this->CreateTcp()) {
            return 0;
        }
    } else {
        if (!this->CreateDp()) {
            return 0;
        }
    }
    this->provider = prov;
    return 1;
}


int CLlDriver::CreateTcp() {
    //52359b
    WSADATA wsa;
    if (WSAStartup(0x101, &wsa) != 0) {
        return 0;
    }

    if (wsa.wVersion == 0x101) {
        return 1;
    }

    WSACleanup();
    return 0;
}

int CLlDriver::CreateDp() {
    //523ccf
    if (this->dplay4) {
        return 1;
    }

    this->dplay_is_4 = 1;
    if (CoCreateInstance(CLSID_DirectPlay, NULL, 1, IID_IDirectPlay4A, (LPVOID*)&this->dplay4) == 0) {
        return 1;
    }

    this->dplay_is_4 = 0;
    if (CoCreateInstance(CLSID_DirectPlay, NULL, 1, IID_IDirectPlay3A, (LPVOID*)&this->dplay3) == 0) {
        return 1;
    }

    this->dplay4 = nullptr;
    return 0;
}


A2NetSock* CLlDriver::GetClientBySocketId(uint32_t uid) {
    //5224c4
    if (!this->is_server) {
        return &this->listen_socket;
    }
    
    A2NetSock* sock = this->connection_sockets + (uid & 0xFFFF);
    if (sock->uid != uid) {
        ReportWarning("CLlDriver::GetClientBySocketId().\nConnection with client lost.\n");
        return nullptr;
    }
    return sock;
}


void CLlDriver::SetEventNewSession() {
    //522318
    if (this->ev_new_session) {
        SetEvent(this->ev_new_session);
    }
}

int CLlDriver::IsInUse(A2NetSock* sock) {
    //52293a
    if (this->listen_socket.is_in_use == 1) {
        return sock->is_in_use;
    } else {
        return 0;
    }
}



int CLlDriver::MakeSockAddr(const char* address, sockaddr_in* out, int port, int resolve) {
    //52343e
    char buf[1024];
    strcpy(buf, address);

    if (buf[0] == 0) {
        out->sin_addr.S_un.S_addr = 0;
        out->sin_port = 0;
        return 0;
    }

    out->sin_port = htons(port);

    for (char* p = buf + strlen(buf) - 1; p != buf; p--) {
        if (*p == ':') {
            *p = 0;
            out->sin_port = htons(atoi(p + 1) + (port == 8001));
            break;
        }
    }

    out->sin_family = AF_INET;
    InetPton(AF_INET, buf, &out->sin_addr.S_un.S_addr); //out->sin_addr.S_un.S_addr = inet_addr(buf);

    if (out->sin_addr.S_un.S_addr == INADDR_NONE && resolve != 0) {
        addrinfo* res = nullptr;
        if (GetAddrInfo(buf, nullptr, nullptr, &res) != 0 && res != nullptr) {
            out->sin_addr.S_un.S_addr = 0;
            out->sin_port = 0;
            return 0;
        }

        out->sin_addr.S_un.S_addr = ((sockaddr_in*)res->ai_addr)->sin_addr.S_un.S_addr;

        /*hostent* host = gethostbyname(buf);
        if (host == nullptr)
        {
            out->sin_addr.S_un.S_addr = 0;
            out->sin_port = 0;
            return 0;
        }
        out->sin_addr.S_un.S_addr = *(uint32_t *)(host->h_addr_list);*/
    }
    memset(out->sin_zero, 0, sizeof(out->sin_zero));
    return 1;
}



int CLlDriver::StartServerTcp() {
    //523621
    this->listen_socket.socket = INVALID_SOCKET;
    this->listen_socket.wait_obj = nullptr;
    this->num_connections = 0;

    this->listen_socket.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (this->listen_socket.socket == INVALID_SOCKET) {
        this->CloseTcp();
        return 0;
    }
    u_long nonblock = 1;
    ioctlsocket(this->listen_socket.socket, FIONBIO, &nonblock);

    sockaddr addr;
    if (this->MakeSockAddr(this->server_start_addr->address, (sockaddr_in*)&addr, 8000, 0) == 0) {
        this->CloseTcp();
        return 0;
    }

    if (bind(this->listen_socket.socket, &addr, sizeof(addr)) != 0) {
        this->CloseTcp();
        return 0;
    }

    if (listen(this->listen_socket.socket, this->max_connections) != 0) {
        this->CloseTcp();
        return 0;
    }

    this->listen_socket.wait_obj = CreateEventA(NULL, 0, 0, NULL);

    if (_beginthread(AcceptThreadTcp, 0, this) == -1L || this->listen_socket.wait_obj == nullptr) {
        CloseHandle(this->listen_socket.wait_obj);
        this->listen_socket.wait_obj = nullptr;
        this->CloseTcp();
        return 0;
    }
    
    this->guaranteed = 1;

    return 1;
}


void __cdecl CLlDriver::AcceptThreadTcp(void* context) {
    //522e8f
    CLlDriver* drv = (CLlDriver*)context;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    while (true) {
        if (drv->listen_socket.is_in_use != 1) {
            SetEvent(drv->listen_socket.wait_obj);
            _endthread();
            return;
        }

        fd_set fd;
        FD_ZERO(&fd);
        FD_SET(drv->listen_socket.socket, &fd);

        if (drv->is_server) {
            for (int i = 0; i < drv->max_connections; i++) {
                A2NetSock* sock = drv->connection_sockets + i;
                if (sock->is_in_use == 1) {
                    FD_SET(sock->socket, &fd);
                }
            }
        }

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 250000;

        int numfd = select(0, &fd, NULL, NULL, &timeout);

        if (numfd == 0) {
            continue;
        }

        if (numfd == SOCKET_ERROR) {
            if (drv->listen_socket.is_in_use == 1) {
                ReportWarning("CLlDriverAcceptThreadTcp().\nSelect error.\n");
            }
            continue;
        }

        if (drv->is_server == 0) {
            if (drv->listen_socket.is_in_use == 1 &&
                FD_ISSET(drv->listen_socket.socket, &fd) &&
                drv->RecvThreadTcp(&drv->listen_socket) == 0) {
                closesocket(drv->listen_socket.socket);
                drv->listen_socket.socket = INVALID_SOCKET;
            }
            continue;
        }

        if (FD_ISSET(drv->listen_socket.socket, &fd)) {
            sockaddr_in newcon;
            int newconln = sizeof(newcon);
            SOCKET nsock = accept(drv->listen_socket.socket, (sockaddr*)&newcon, &newconln);
            if (nsock == INVALID_SOCKET) {
                if (drv->listen_socket.is_in_use == 1) {
                    ReportWarning("CLlDriverAcceptThreadTcp().\nUnable to accept new connection.\n");
                }
                continue;
            }
            
            if (drv->num_connections >= drv->max_connections) {
                closesocket(nsock);
                ReportWarning("CLlDriverAcceptThreadTcp().\nClient connection refused.\n");
                continue;
            }

            SetSockOptions(nsock, 1);

            EnterCriticalSection(&drv->critical_section);

            int32_t idx = -1;
            for (int i = 0; i < drv->max_connections; i++) {
                if (!drv->IsInUse(drv->connection_sockets + i)) {
                    idx = i;
                    break;
                }
            }

            if (idx == -1) {
                closesocket(nsock);
                ReportWarning("CLlDriverAcceptThreadTcp().\n No free slot.\n");
            } else {
                A2NetSock* cs = drv->connection_sockets + idx;
                cs->socket = nsock;
                cs->uid = idx | drv->next_uid;
                drv->next_uid += 0x10000;
                cs->wait_obj = nullptr;
                cs->copy_num = 0;
                cs->manager = drv->hl_driver->AllocClientBufManager(cs->uid);
                sprintf((char*)cs->manager->buf, "%u.%u.%u.%u:%u", newcon.sin_addr.S_un.S_un_b.s_b1, newcon.sin_addr.S_un.S_un_b.s_b2, newcon.sin_addr.S_un.S_un_b.s_b3, newcon.sin_addr.S_un.S_un_b.s_b4, ntohs(newcon.sin_port));
                cs->is_in_use = 1;
                drv->num_connections++;
            }

            LeaveCriticalSection(&drv->critical_section);
        }

        for (uint32_t i = 0; i < drv->max_connections; i++) {
            A2NetSock* conn = drv->connection_sockets + i;
            if (conn->is_in_use == 1 &&
                FD_ISSET(conn->socket, &fd) &&
                drv->RecvThreadTcp(conn) == 0) {
                closesocket(conn->socket);
                conn->socket = INVALID_SOCKET;
            }
        }

    }
}

void __cdecl CLlDriver::SetSockOptions(SOCKET s, int enlinger) {
    //522af4
    int32_t flag = 1;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, 4);
    int32_t bufsz = 0x8000;
    setsockopt(s, SOL_SOCKET, SO_SNDBUF, (const char*)&bufsz, 4);
    setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char*)&bufsz, 4);

    if (enlinger) {
        linger lngr;
        lngr.l_linger = 0;
        lngr.l_onoff = 1;
        setsockopt(s, SOL_SOCKET, SO_LINGER, (const char*)&lngr, sizeof(linger));
    }
    u_long nonblock = 1;
    ioctlsocket(s, FIONBIO, &nonblock);
}



int CLlDriver::RecvThreadTcp(A2NetSock* sock) {
    //522b95
    uint8_t databuf[0x2000];
    int rsz = recv(sock->socket, (char *)databuf, 0x2000, 0);
    if (rsz == 0 || rsz == -1) {
        return 0;
    }

    uint8_t *pos = databuf;
    while (true) {
        if (rsz == 0) {
            return 1;
        }

        if (sock->copy_num < 8) {
            if (sock->current_buffer == nullptr) {
                sock->copy_num = 0;
                sock->current_buffer = this->hl_driver->GetFreeNet3();
                sock->current_buffer->Clear();
            }
            int32_t copynum = 8 - sock->copy_num;
            if (rsz < copynum) {
                copynum = rsz;
            }
            
            memcpy(sock->current_buffer->buf + sock->copy_num - 8, pos, copynum);
            pos += copynum;
            rsz -= copynum;
            sock->copy_num += copynum;

            if (sock->copy_num >= 8 && (sock->current_buffer->pos == 0 || sock->current_buffer->pos >= 143)) {
                ReportWarning("CLlDriverRecvThreadTcp().\nReceived invalid message.\n");
                this->hl_driver->AddTailFreeNet3(sock->current_buffer);
                sock->copy_num = 0;
                sock->current_buffer = nullptr;
                return 0;
            }
        } else {
            int32_t cpysz = (sock->current_buffer->pos + 8) - sock->copy_num;
            int32_t cpynum = cpysz;
            if (cpynum > rsz) {
                cpynum = rsz;
            }
            
            memcpy(sock->current_buffer->buf + sock->copy_num - 8, pos, cpynum);
            pos += cpynum;
            rsz -= cpynum;
            sock->copy_num += cpynum;

            if (cpysz == cpynum) {
                sock->current_buffer->datasz = sock->current_buffer->pos;
                XorData(sock->current_buffer->buf, sock->current_buffer->datasz);
                sock->manager->ReceiveData(sock->current_buffer);
                sock->copy_num = 0;
                sock->current_buffer = nullptr;
            }
        }
    }
}




int CLlDriver::PrepareAddressDp(CLlAddress* addr) {
    //523d96
    IDirectPlayLobby2A* lobby = nullptr;
    if (CoCreateInstance(CLSID_DirectPlayLobby, NULL, 1, IID_IDirectPlayLobby2A, (LPVOID *) &lobby) != DP_OK) {
        return 0;
    }

    DPCOMPOUNDADDRESSELEMENT elem[3];
    int numelem = 0;

    char addrbuf[100];

    switch (this->provider) {
    case 0:
        elem[0].guidDataType = DPAID_ServiceProvider;
        elem[0].dwDataSize = sizeof(GUID);
        elem[0].lpData = (LPVOID) &DPSPGUID_SERIAL;
        elem[1].guidDataType = DPAID_ComPort;
        elem[1].dwDataSize = sizeof(DPCOMPORTADDRESS);
        elem[1].lpData = &addr->com;
        numelem = 2;
        break;
    case 1:
        elem[0].guidDataType = DPAID_ServiceProvider;
        elem[0].dwDataSize = sizeof(GUID);
        elem[0].lpData = (LPVOID)&DPSPGUID_MODEM;
        elem[1].guidDataType = DPAID_Modem;
        elem[1].dwDataSize = strlen(addr->name) + 1;
        elem[1].lpData = addr->name;
        elem[2].guidDataType = DPAID_Phone;
        elem[2].dwDataSize = strlen(this->address_str) + 1;
        elem[2].lpData = this->address_str;
        numelem = 3;
        break;
    case 2:
        elem[0].guidDataType = DPAID_ServiceProvider;
        elem[0].dwDataSize = sizeof(GUID);
        elem[0].lpData = (LPVOID)&DPSPGUID_IPX;
        numelem = 1;
        break;
    case 3:
        strcpy(addrbuf, this->address_str);
        elem[0].guidDataType = DPAID_ServiceProvider;
        elem[0].dwDataSize = sizeof(GUID);
        elem[0].lpData = (LPVOID)&DPSPGUID_TCPIP;
        elem[1].guidDataType = DPAID_INet;
        elem[1].dwDataSize = strlen(addrbuf) + 1;
        elem[1].lpData = addrbuf;
        numelem = 2;
        break;
    }
    DWORD sz = 0;
    if (lobby->CreateCompoundAddress(elem, numelem, nullptr, &sz) != DPERR_BUFFERTOOSMALL) {
        lobby->Release();
        return 0;
    }

    uint8_t *dat = new uint8_t[sz];
    if (lobby->CreateCompoundAddress(elem, numelem, dat, &sz) != DP_OK) {
        delete[] dat;
        lobby->Release();
        return 0;
    }

    HRESULT res = this->dplay4->InitializeConnection(dat, 0);
    if (res == DPERR_ALREADYINITIALIZED) {
        this->FreeDp();
        this->CreateDp();
        if (this->dplay4) {
            res = this->dplay4->InitializeConnection(dat, 0);
        }
    }

    delete[] dat;
    lobby->Release();
    return res == DP_OK;
}

int CLlDriver::StartServerDp() {
    //5241ea
    this->ev_close = nullptr;
    this->ev_create_player = nullptr;
    this->listen_socket.player_dpid = -1;

    if (this->PrepareAddressDp(this->server_start_addr) == 0) {
        ReportWarning("CLlDriver::StartServerDP.\nUnable to initialize DirectPlay connection.\n");
        this->CloseDp();
        return 0;
    }

    DPCAPS caps;
    caps.dwSize = sizeof(DPCAPS);
    if (this->dplay4->GetCaps(&caps, DPGETCAPS_GUARANTEED) != DP_OK) {
        ReportWarning("CLlDriver::StartServerDp.\nUnable to get caps.\n");
        return 0;
    }

    if (caps.dwMaxPlayers < this->num_connections) {
        ReportWarning("CLlDriver::StartServerDp().\nUnsupported number of connections.\n");
        this->num_connections = caps.dwMaxPlayers;
    }

    if ((caps.dwFlags & (DPCAPS_GUARANTEEDSUPPORTED | DPCAPS_GUARANTEEDOPTIMIZED)) == (DPCAPS_GUARANTEEDSUPPORTED | DPCAPS_GUARANTEEDOPTIMIZED) ) {
        this->guaranteed = 1;
    } else {
        this->guaranteed = 0;
    }
    
    if (this->provider == 1) {
        this->latency = 1000;
        if (this->timeout < 16000) {
            this->timeout = 16000;
        }
    } else if (this->provider == 3) {
        this->guaranteed = 0;
        this->latency = 500;
        if (this->timeout < 16000) {
            this->timeout = 16000;
        }
    } else {
        this->latency = caps.dwLatency;
    }

    this->ev_create_player = CreateEventA(NULL, 0, 0, NULL);
    this->ev_close = CreateEventA(NULL, 0, 0, NULL);

    if (this->ev_create_player == nullptr || this->ev_close == nullptr) {
        ReportWarning("CLlDriver::StartServerDp().\nUnable to create events.\n");
        this->CloseDp();
        return 0;
    }

    DPSESSIONDESC2 session;
    memset(&session, 0, sizeof(DPSESSIONDESC2));
    session.dwSize = sizeof(DPSESSIONDESC2);
    session.dwFlags = DPSESSION_NODATAMESSAGES;
    if (this->keepalive || this->guaranteed) {
        session.dwFlags |= DPSESSION_KEEPALIVE;
    }

    session.guidApplication = this->application_guid;
    session.dwMaxPlayers = this->max_connections;
    session.lpszSessionNameA = this->comp_name;
    session.lpszPasswordA = (char*)"";

    if (this->dplay4->Open(&session, DPOPEN_CREATE) != DP_OK) {
        ReportWarning("CLlDriver::StartServerDp().\nUnable to create DirectPlay session.\n");
        this->CloseDp();
        return 0;
    }

    DPNAME pname;
    memset(&pname, 0, sizeof(DPNAME));
    pname.lpszShortNameA = this->comp_name;
    pname.lpszLongNameA = this->comp_name;

    if (this->dplay4->CreatePlayer((LPDPID)&this->listen_socket.player_dpid, &pname, this->ev_create_player, nullptr, 0, DPPLAYER_SERVERPLAYER) != DP_OK) {
        ReportWarning("CLlDriver::StartServerDp().\nUnable to create DirectPlay server player.\n");
        this->CloseDp();
        return 0;
    }

    this->listen_socket.next_in_pktid = 0;
    this->listen_socket.next_out_pktid = 0;
    this->listen_socket.wait_obj = CreateEventA(NULL, 0, 0, NULL);
    if (_beginthread(RecvThreadDp, 0, this) == -1L || this->listen_socket.wait_obj == nullptr) {
        this->CloseDp();
        return 0;
    }

    return 1;
}






void __cdecl CLlDriver::RecvThreadDp(void* context) {
    //523b7a
    CLlDriver* drv = (CLlDriver*)context;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    HANDLE handles[2] = { drv->ev_close, drv->ev_create_player };

    void* mem = malloc(0x96);
    DWORD memsz = 0x96;
    while (true) {
        if (WaitForMultipleObjects(2, handles, 0, -1) != 1) {
            free(mem);
            SetEvent(drv->listen_socket.wait_obj);
            _endthread();
            return;
        }
        EnterCriticalSection(&drv->critical_section);
        while (true) {
            DPID from;
            DPID to;
            HRESULT res = drv->dplay4->Receive(&from, &to, DPRECEIVE_ALL, mem, &memsz);
            if (res == DPERR_BUFFERTOOSMALL) {
                free(mem);
                mem = malloc(memsz);
            } else if (res == DPERR_NOMESSAGES) {
                break;
            } else if (res == DP_OK) {
                drv->HandleMessageDp(from, to, mem, memsz);
            } else {
                ReportWarning("CLlDriverRecvThreadDp().\nError during receiving messages.\n");
            }
        }
        LeaveCriticalSection(&drv->critical_section);
    }
}

void CLlDriver::HandleMessageDp(uint32_t from, uint32_t to, void* data, uint32_t datasz) {
    //524abd
    if (from == 0) {
        int dwType = ((DPMSG_GENERIC*)data)->dwType;
        if (dwType == DPSYS_CREATEPLAYERORGROUP) {
            DPMSG_CREATEPLAYERORGROUP* createmsg = (DPMSG_CREATEPLAYERORGROUP*)data;
            if (this->is_server != 0 && createmsg->dwPlayerType == DPPLAYERTYPE_PLAYER) {
                int32_t idx = 0;
                while (idx < this->max_connections) {
                    if (!this->IsInUse(this->connection_sockets + idx)) {
                        break;
                    }
                    idx++;
                }

                int infid;
                this->FindDpSock(createmsg->dpId, &infid);
                memmove(this->connections_info + infid + 1, this->connections_info + infid, (this->num_connections - infid) * sizeof(SocketNm)); //make hole for insert
                
                SocketNm* inf = this->connections_info + infid;
                A2NetSock* sock = this->connection_sockets + idx;
                inf->dpid = createmsg->dpId;
                inf->sock = sock;
                sock->next_in_pktid = 0;
                sock->next_out_pktid = 0;
                sock->uid = this->next_uid | idx;

                this->next_uid += 0x10000;

                sock->player_dpid = createmsg->dpId;
                sock->is_in_use = 1;
                sock->latency_check.num = 0;
                sock->latency_check.calc_latency = sock->latency;
                sock->latency = 0;
                sock->stat_pkts_queued = 0;
                sock->stat_pkts_sent = 0;
                sock->manager = this->hl_driver->AllocClientBufManager(sock->uid);

                this->num_connections++;
            }
        } else if (dwType == DPSYS_DESTROYPLAYERORGROUP) {
            DPMSG_DESTROYPLAYERORGROUP* destroymsg = (DPMSG_DESTROYPLAYERORGROUP*)data;
            if (this->is_server != 0 && destroymsg->dwPlayerType == DPPLAYERTYPE_PLAYER) {
                int infid;
                A2NetSock* sock = this->FindDpSock(destroymsg->dpId, &infid);
                this->CloseDpSock(sock, infid);
                this->num_connections--;
            }
        } else if (dwType == DPSYS_SESSIONLOST) {
            this->session_lost = 1;
        }
        return;
    }
    
    if (datasz <= 7 || datasz >= 0x97) {
        ReportWarning("CLlDriver::HandleMessageDp().\nReceived invalid message.\n");
        return;
    }

    A2NetSock* sock = this->FindDpSock(from, nullptr);
    if (!sock || !this->IsInUse(sock)) {
        ReportWarning("CLlDriver::HandleMessageDp().\nMessage from disconnected client.\n");
        return;
    }

    if (datasz == 8) {
        if (this->guaranteed != 0) {
            ReportWarning("CLlDriver::HandleMessageDp().\nAcknoweledgement on guaranteed media.\n");
            return;
        }

        if (sock->list_0x14.IsEmpty()) {
            return;
        }
        
        for (POSITION pos = sock->list_0x14.GetHeadPosition(); pos != nullptr;) {
            POSITION curpos = pos;
            NetStru3* buf = sock->list_0x14.GetNext(pos);
            if (buf->pktid == *(uint32_t*)data) {
                uint32_t latency = 0;
                const uint32_t ticks = GetTickCount();
                if (buf->timestamp < ticks) {
                    latency = ticks - buf->timestamp;
                }

                sock->latency_check.AddLatency(latency);

                sock->list_0x14.RemoveAt(curpos);

                buf->Clear();

                this->hl_driver->AddTailFreeNet3(buf);
                break;
            }
        }
        return;
    }

    NetStru3* buf = this->hl_driver->GetFreeNet3();
    buf->Clear();

    memcpy(&buf->full_data, data, datasz);

    buf->datasz = datasz - 8;
    
    if (this->guaranteed) {
        sock->manager->ReceiveData(buf);
        return;
    }

    while (this->dplay4->Send(this->listen_socket.player_dpid, from, 0, data, 8) == DPERR_BUSY) {
        ReportWarning("CLlDriver::HandleMessageDp().\nDirectPlay is busy, I go to sleep for 10ms.\n");
        Sleep(10);
    }

    uint32_t pktid = buf->pktid;
    if (pktid == sock->next_in_pktid) {
        sock->manager->ReceiveData(buf);
        sock->next_in_pktid++;
        if (!sock->list_0x30.IsEmpty()) {
            for (POSITION pos = sock->list_0x30.GetHeadPosition(); pos != nullptr;) {
                POSITION curpos = pos;
                buf = sock->list_0x30.GetNext(pos);
                if (buf->pktid == sock->next_in_pktid) {
                    sock->manager->ReceiveData(buf);
                    sock->next_in_pktid++;

                    sock->list_0x30.RemoveAt(curpos);
                    return;
                }
            }
        }
    } else if (pktid < sock->next_in_pktid) {
        buf->Clear();
        this->hl_driver->AddTailFreeNet3(buf);
    } else {
        if (sock->list_0x30.IsEmpty()) {
            sock->list_0x30.AddTail(buf);
        } else {
            for (POSITION pos = sock->list_0x30.GetHeadPosition(); pos != nullptr;) {
                POSITION curpos = pos;
                NetStru3 *cbuf = sock->list_0x30.GetNext(pos);
                if (cbuf->pktid == pktid) {
                    buf->Clear();
                    this->hl_driver->AddTailFreeNet3(buf);
                    return;
                }
            }

            sock->list_0x30.AddTail(buf);
        }
    }
}

A2NetSock* CLlDriver::FindDpSock(uint32_t id, int* out_idx) {
    //524986
    if (this->is_server == 0) {
        return &this->listen_socket;
    }

    if (this->num_connections < 1) {
        if (out_idx) {
            *out_idx = 0;
        }
        return nullptr;
    }

    int32_t lb = 0;
    int32_t rb = this->num_connections - 1;
    while (lb < rb) {
        int32_t cur = lb + (rb - lb) / 2;
        SocketNm* info = this->connections_info + cur;

        if (info->dpid == id) {
            if (out_idx) {
                *out_idx = cur;
            }

            return info->sock;
        }

        if (info->dpid <= id) {
            lb = cur + 1;
        } else {
            rb = cur;
        }
    }

    //if lb >= rb
    SocketNm* info = this->connections_info + rb;
    if (info->dpid == id) {
        if (out_idx) {
            *out_idx = rb;
        }
        return info->sock;
    }

    if (out_idx) {
        if (info->dpid > id) {
            *out_idx = rb;
        } else {
            *out_idx = rb + 1;
        }
    }
    return nullptr;
}


int CLlDriver::StartServer(int maxconn, const char* name, CLlAddress* addr) {
    //521a09
    this->is_server = 1;
    this->max_connections = maxconn;
    this->server_start_addr = addr;
    if (!name) {
        DWORD sz = sizeof(this->comp_name);
        GetComputerNameA(this->comp_name, &sz);
    } else {
        strcpy(this->comp_name, name);
    }

    if (this->connection_sockets) {
        delete[] this->connection_sockets;
        this->connection_sockets = nullptr;
    }

    if (this->connections_info) {
        delete[] this->connections_info;
        this->connections_info = nullptr;
    }

    this->connection_sockets = new A2NetSock[this->max_connections]();
    this->connections_info = new SocketNm[this->max_connections]();

    if (this->listen_socket.is_in_use == 1) {
        this->Close();
    }

    this->num_connections = 0;

    if (this->provider == 4) {
        if (this->StartServerTcp() == 0) {
            ReportWarning("CLlDriver::StartServer().\nUnable to connect over TCP/IP.\n");
            return 0;
        }
    } else {
        if (this->StartServerDp() == 0) {
            ReportWarning("CLlDriver::StartServer().\nUnable to connect over DirectPlay.\n");
            return 0;
        }
    }
    this->listen_socket.is_in_use = 1;

    return 1;
}


int CLlDriver::PrepareForConnect(const char* name, CLlAddress* addr) {
    //521d2a
    this->is_server = 0;
    this->max_connections = 1;

    if (this->connection_sockets) {
        delete[] this->connection_sockets;
        this->connection_sockets = nullptr;
    }

    if (this->connections_info) {
        delete[] this->connections_info;
        this->connections_info = nullptr;
    }

    strcpy(this->address_str, name);

    if (addr) {
        this->cur_address = *addr;
    } else {
        memset(&this->cur_address, 0, sizeof(this->cur_address));
    }

    if (this->provider == 4) {
        this->guaranteed = 1;
        return 1;
    }

    if (this->PrepareAddressDp(addr) == 0) {
        ReportWarning("CLlDriver::PrepareForConnect().\nUnable to initialize DirectPlay connection.\n");
        this->CloseDp();
        return 0;
    }

    DPCAPS caps;
    caps.dwSize = sizeof(DPCAPS);

    if (this->dplay4->GetCaps(&caps, DPGETCAPS_GUARANTEED) != DP_OK) {
        ReportWarning("CLlDriver::PrepareForConnect().\nUnable to get caps.\n");
        return 0;
    }

    if ((caps.dwFlags & (DPCAPS_GUARANTEEDSUPPORTED | DPCAPS_GUARANTEEDOPTIMIZED)) == (DPCAPS_GUARANTEEDSUPPORTED | DPCAPS_GUARANTEEDOPTIMIZED)) {
        this->guaranteed = 1;
    } else {
        this->guaranteed = 0;
    }

    if (this->provider == 1) {
        this->latency = 1000;
        if (this->timeout < 16000) {
            this->timeout = 16000;
        }
    } else if (this->provider == 3) {
        this->guaranteed = 0;
        this->latency = 500;
        if (this->timeout < 16000) {
            this->timeout = 16000;
        }
    } else {
        this->latency = caps.dwLatency;
    }
    return 1;
}


int CLlDriver::ConnectDp(const char* name, CLlNetSession* session) {
    //52465f
    this->ev_close = nullptr;
    this->ev_create_player = nullptr;

    this->listen_socket.player_dpid = -1;

    this->ev_create_player = CreateEventA(NULL, 0, 0, NULL);
    this->ev_close = CreateEventA(NULL, 0, 0, NULL);

    if (this->ev_create_player == nullptr || this->ev_close == nullptr) {
        ReportWarning("CLlDriver::ConnectDp().\nUnable to create events.\n");
        this->CloseDp();
        return 0;
    }

    DPSESSIONDESC2 desc;
    memset(&desc,0,0x50);
    desc.dwSize = sizeof(DPSESSIONDESC2);
    desc.guidInstance = session->guid;
    desc.lpszPasswordA = (char *)"";

    if (this->dplay4->Open(&desc, DPOPEN_JOIN) != DP_OK) {
        ReportWarning("CLlDriver::ConnectDp().\nUnable to join DirectPlay session.\n");
        this->CloseDp();
        return 0;
    }

    DPSESSIONDESC2 tmp;
    memset(&tmp, 0, 0x50);
    tmp.dwSize = sizeof(DPSESSIONDESC2);
    this->dplay4->EnumSessions(&tmp, 0, cbEnumSessions, this, DPENUMSESSIONS_STOPASYNC);

    DPNAME pname;
    memset(&pname, 0, sizeof(DPNAME));
    pname.dwSize = sizeof(DPNAME);
    pname.lpszShortNameA = (char *)name;
    pname.lpszLongNameA = (char*)name;
    if (this->dplay4->CreatePlayer((LPDPID)&this->listen_socket.player_dpid, &pname, this->ev_create_player, nullptr, 0, 0) != DP_OK) {
        ReportWarning("CLlDriver::ConnectDp().\nUnable to create DirectPlay player.\n");
        this->CloseDp();
        return 0;
    }

    this->listen_socket.next_in_pktid = 0;
    this->listen_socket.next_out_pktid = 0;

    this->listen_socket.uid = this->next_uid;

    this->listen_socket.latency_check.num = 0;
    this->listen_socket.latency_check.calc_latency = this->latency;
    
    this->listen_socket.manager = this->hl_driver->AllocClientBufManager(this->listen_socket.uid);
    this->listen_socket.wait_obj = CreateEventA(NULL, 0, 0, NULL);

    if (_beginthread(RecvThreadDp, 0, this) == -1L || this->listen_socket.wait_obj == nullptr) {
        this->CloseDp();
        return 0;
    }

    return 1;
}


int CLlDriver::ConnectTcp() {
    //523794
    this->num_connections = 0;
    this->listen_socket.socket = INVALID_SOCKET;
    this->listen_socket.wait_obj = nullptr;
    this->listen_socket.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (this->listen_socket.socket == INVALID_SOCKET) {
        this->CloseTcp();
        return 0;
    }

    sockaddr connaddr;
    sockaddr bindaddr;
    if (this->MakeSockAddr(this->address_str, (sockaddr_in*)&connaddr, 8000, 1) == 0 ||
        this->MakeSockAddr(this->cur_address.address, (sockaddr_in*)&bindaddr, 0, 0) == 0) {
        this->CloseTcp();
        return 0;
    }

    if (bind(this->listen_socket.socket, &bindaddr, sizeof(bindaddr)) != 0) {
        this->CloseTcp();
        return 0;
    }

    if (connect(this->listen_socket.socket, &connaddr, sizeof(connaddr)) != 0) {
        this->CloseTcp();
        return 0;
    }

    SetSockOptions(this->listen_socket.socket, 1);

    this->listen_socket.uid = this->next_uid;
    this->listen_socket.manager = this->hl_driver->AllocClientBufManager(this->listen_socket.uid);
    this->listen_socket.wait_obj = CreateEventA(NULL, 0, 0, NULL);

    if (_beginthread(AcceptThreadTcp, 0x400, this) == -1L && this->listen_socket.wait_obj != nullptr) {
        CloseHandle(this->listen_socket.wait_obj);
        this->listen_socket.wait_obj = nullptr;
        this->CloseTcp();
        return 0;
    }
    
    this->num_connections = 1;
    this->guaranteed = 1;
    return 1;
}

int CLlDriver::Connect(const char* name, CLlNetSession* session) {
    //5223fb
    char buf[16];
    memset(buf, 0, sizeof(buf));

    if (!name) {
        DWORD sz = sizeof(buf);
        GetComputerNameA(buf, &sz);
        name = buf;
    }

    if (this->listen_socket.is_in_use == 1) {
        this->Close();
    }

    if (this->provider == 4) {
        if (this->ConnectTcp() == 0) {
            ReportWarning("CLlDriver::Connect().\nUnable to connect over TCP/IP.\n");
            return 0;
        }
    } else {
        if (this->ConnectDp(name, session) == 0) {
            ReportWarning("CLlDriver::Connect().\nUnable to connect over DirectPlay.\n");
            return 0;
        }
    }

    this->listen_socket.is_in_use = 1;
    return 1;
}

int CLlDriver::SendDataTcp(A2NetSock* sock, NetStru3* buffer) {
    //52394a
    int len = buffer->datasz + 8;
    int sendlen = -1;
    if (sock->socket != INVALID_SOCKET) {
        sendlen = send(sock->socket, (char*)&buffer->full_data, len, 0);
    }

    this->hl_driver->AddTailFreeNet3(buffer);

    if (sendlen == -1 || sendlen != len) {
        this->CloseTcpSocket(sock);
        return 0;
    }

    return 1;
}



int CLlDriver::SendDataDp(A2NetSock* sock, NetStru3* buffer) {
    //5255c6
    if (this->session_lost != 0) {
        if (buffer) {
            this->hl_driver->AddTailFreeNet3(buffer);
        }

        this->CloseDp();
        this->session_lost = 0;
        return 0;
    }

    uint32_t currentTick = GetTickCount();
    if (this->guaranteed == 0) {
        if (buffer) {
            buffer->timestamp = currentTick;
            buffer->timestamp2 = 0;
            buffer->pktid = sock->next_out_pktid;
            sock->next_out_pktid++;
            sock->list_0x14.AddTail(buffer);

            sock->stat_pkts_queued++;
            sock->stat_pkts_sent--;
        }
    } else {
        if (!buffer) {
            return 1;
        }
    }

    uint32_t latency = 0;
    if (sock->latency) {
        latency = sock->latency;
    } else {
        latency = sock->latency_check.GetLatency();
    }
    
    if (this->guaranteed == 0) {
        uint32_t unsent_size = 0;

        for (POSITION it = sock->list_0x14.GetHeadPosition(); it != nullptr;) {
            NetStru3* dat = sock->list_0x14.GetNext(it);
            unsent_size += 8 + dat->datasz;
        }

        if (unsent_size > 0x10000) {
            ReportWarning("CLlDriver::SendDataDp().\nOut of bandwidth.\n");
            this->CloseDpSock(sock, -1);
            return 0;
        }

        latency = std::max<uint32_t>(latency, (unsent_size * 1000) / 1024);
    }

    POSITION iter = nullptr;
    if (this->guaranteed == 0) {
        iter = sock->list_0x14.GetHeadPosition();
    }

    bool loop = true;
    while (loop) {
        if (this->guaranteed == 0) {
            buffer = sock->list_0x14.GetNext(iter);

            if (iter == nullptr) { //not more nodes;
                loop = false;
            }

            uint32_t mxticks = ((buffer->datasz + 8) * 1000) / 1024;
            if (currentTick < mxticks) {
                mxticks = 0;
            } else {
                mxticks = currentTick - mxticks;
            }

            uint32_t maxtime = 0;
            if (buffer->timestamp < mxticks) {
                maxtime = mxticks - buffer->timestamp;
            }
            
            if (this->timeout < maxtime) {
                ReportWarning("CLlDriver::SendDataDp().\nCannot receive acknowledgement.\n");
                this->CloseDpSock(sock, -1);
                return 0;
            }

            uint32_t t2time = 0;
            if (buffer->timestamp2 < mxticks) {
                t2time = mxticks - buffer->timestamp2;
            }

            if (latency <= t2time) {
                buffer->timestamp2 = currentTick;
            }
        } else {
            loop = false;
        }

        uint32_t to = DPID_SERVERPLAYER;
        if (this->is_server != 0) {
            to = sock->player_dpid;
        }


        uint32_t tosentsz = buffer->datasz + 8;

        sock->stat_pkts_sent++;

        HRESULT res = this->dplay4->Send(this->listen_socket.player_dpid, to, (this->guaranteed != 0 ? DPSEND_GUARANTEED : 0), &buffer->full_data, tosentsz);
        while (this->guaranteed != 0 && res == DPERR_BUSY) { //if guaranteed try and try if BUSY
            ReportWarning("CLlDriver::SendDataDp().\nDirectPlay is busy, I go to sleep for 10ms.\n");
            Sleep(10);
            res = this->dplay4->Send(this->listen_socket.player_dpid, to, (this->guaranteed != 0 ? DPSEND_GUARANTEED : 0), &buffer->full_data, tosentsz);
        }

        if (res != DP_OK && res != DPERR_BUSY) { // not send and not BUSY -> exit
            if (this->guaranteed != 0) {
                this->hl_driver->AddTailFreeNet3(buffer);
            }

            this->CloseDpSock(sock, -1);
            return 0;
        }

        if (this->guaranteed != 0) {
            this->hl_driver->AddTailFreeNet3(buffer);
        }
    }
    return 1;
}


int CLlDriver::SendData(uint32_t uid, NetStru3* buffer) {
    //5226c4
    EnterCriticalSection(&this->critical_section);

    A2NetSock* sock = this->GetClientBySocketId(uid);

    if ((sock == NULL) || (sock->is_in_use != 1)) {
        if (buffer) {
            this->hl_driver->AddTailFreeNet3(buffer);
        }
        LeaveCriticalSection(&this->critical_section);
        return 0;
    }

    buffer->pos = buffer->datasz;

    if (this->provider == 4) {
        XorData(buffer->buf, buffer->datasz);

        if (this->SendDataTcp(sock, buffer) != 0) {
            LeaveCriticalSection(&this->critical_section);
            return 1;
        }
    } else {
        if (this->SendDataDp(sock, buffer) != 0) {
            LeaveCriticalSection(&this->critical_section);
            return 1;
        }
    }

    if (this->listen_socket.is_in_use == 1 && this->is_server != 0) {
        ReportWarning("CLlDriver::SendData().\nConnection with client lost.\n");
    } else {
        ReportWarning("CLlDriver::SendData().\nConnection lost.\n");
    }

    LeaveCriticalSection(&this->critical_section);
    return 0;
}

void CLlDriver::SendAllPendingDp(A2NetSock* sock) {
    //525b86
    if (this->guaranteed == 0) {
        while (!sock->list_0x14.IsEmpty()) {
            if (this->SendDataDp(sock, nullptr) == 0) {
                break;
            }

            Sleep(10);
        }
    }
}

void CLlDriver::DisconnectClient(uint32_t uid) {
    //522526
    EnterCriticalSection(&this->critical_section);
    A2NetSock* sock = this->GetClientBySocketId(uid);
    if (sock) {
        if (this->IsInUse(sock)) {
            if (this->provider == 4) {
                this->CloseTcpSocket(sock);
            } else {
                this->SendAllPendingDp(sock);
                this->CloseDpSock(sock, -1);
            }
            sock->is_in_use = 0;
        }
    }
    LeaveCriticalSection(&this->critical_section);
}

void CLlDriver::CleanupInvalidTcpClient(uint32_t uid) {
    //52262a
    if (this->provider == 4) {
        EnterCriticalSection(&this->critical_section);
        A2NetSock* sock = this->GetClientBySocketId(uid);
        if (sock && IsInUse(sock)) {
            if (sock->socket == INVALID_SOCKET) {
                this->CloseTcpSocket(sock);
            }
        }
        LeaveCriticalSection(&this->critical_section);
    }
}

void CLlDriver::CleanupAllInvalid() {
    //522809
    if (this->listen_socket.is_in_use != 1) {
        return;
    }
        
    EnterCriticalSection(&this->critical_section);
    if (this->is_server == 0) {
        if (this->provider == 4) {
            if (this->listen_socket.socket == INVALID_SOCKET) {
                this->CloseTcp();
            }
        } else {
            this->SendDataDp(&this->listen_socket, nullptr);
        }
    } else {
        for (int i = 0; i < this->num_connections; i++) {
            A2NetSock* sock = this->connection_sockets + i;
            if (this->IsInUse(sock)) {
                if (this->provider == 4) {
                    if (sock->socket == INVALID_SOCKET) {
                        this->CloseTcpSocket(sock);
                    }
                } else {
                    this->SendDataDp(sock, nullptr);
                }
            }
        }
    }
    LeaveCriticalSection(&this->critical_section);
}


int CLlDriver::RestartModemServerDp() {
    //52460e
    if (this->provider != 1) {
        return 1;
    }

    this->CloseDp();
    this->CreateDp();
    return this->StartServerDp();
}

void CLlDriver::RecreateDp() {
    //524644
    this->CloseDp();
    this->CreateDp();
}

void CLlDriver::SetLatency(uint32_t conn_uid, int32_t latency_ms) {
    //5229cd
    A2NetSock* sock = this->GetClientBySocketId(conn_uid);
    if (sock) {
        sock->latency = latency_ms;
    }
}

int32_t CLlDriver::GetLatency(uint32_t conn_uid) {
    //5229fd
    A2NetSock* sock = this->GetClientBySocketId(conn_uid);
    if (!sock) {
        return -1;
    }

    if (sock->latency == 0) {
        return sock->latency_check.GetLatency();
    } else {
        return sock->latency;
    }
}

int32_t CLlDriver::GetPacketLoss(uint32_t conn_uid) {
    //522a51
    A2NetSock* sock = this->GetClientBySocketId(conn_uid);
    if (!sock) {
        return 0;
    }

    if (sock->stat_pkts_queued > 1000) {
        sock->stat_pkts_queued /= 10;
        sock->stat_pkts_sent /= 10;
    }

    if (sock->stat_pkts_sent == 0) {
        return 0;
    }

    return ((float)sock->stat_pkts_sent * 100000.0) / (float)sock->stat_pkts_queued;
}

// Helper: check if a map visibility grid cell is visible to a player.
// Returns true if player can see the position, or fog is disabled, or map is null.
bool IsVisibleTo(TokenPos* pos, Player* player) {
    if (MapStuff_Instance == nullptr || g_Server->field4_0x74 == 0) {
        return true;
    }

    uint32_t x = pos->GetX() >> 3;
    uint32_t y = pos->GetY() >> 3;
    return MapStuff_Instance->scan_presence_grid.sector_grid[x+1][y+1] & player->vision_sharing_id;
}

// Helper: handle "unit hidden" case for effect packets (ids 0x88/0x89).
void MarkUnitHiddenForPlayer(PacketMoveBroadcast* packet, Player* player) {
    if (packet->id < 0x88 || packet->id > 0x89) {
        return;
    }

    Unit* affected_unit = dword_6CDB3C->FindById(packet->unit_id);
    if (affected_unit == nullptr) {
        return;
    }

    int32_t pid = player->player_id;
    if (0x10 <= pid && pid < 0x20) {
        affected_unit->something_per_player[pid - 0x10] |= 0x800000;
    }
}

// 51B0F0
void NetStru1::SendUnitAction(Packet* packet, Unit* unit) {
    if (packet->to_player_id != 0) {
        Player* player = g_PlayersList->FindById(packet->to_player_id);
        if ((unit->seen_by_mask & player->vision_sharing_id) == 0 && g_Server->field4_0x74 != 0) {
            int32_t pid = player->player_id;
            if (pid >= 0x10 && pid < 0x20 && packet->id == 0x6B &&
                (unit->something_per_player[pid - 0x10] & 0x40000000) == 0) {
                unit->something_per_player[pid - 0x10] |= 0x40000010;
                g_NetStru1_main.SendUnitVisibility(unit, 1, player);
            }
        } else {
            this->QueuePacketSend(packet);
        }
        return;
    }

    POSITION it = g_PlayersList->list.GetHeadPosition();
    while (it != nullptr) {
        Player* player = g_PlayersList->list.GetNext(it);
        if (!player->is_active) {
            continue;
        }
        if ((unit->seen_by_mask & player->vision_sharing_id) == 0 && g_Server->field4_0x74 != 0) {
            int32_t pid = player->player_id;
            if (pid >= 0x10 && pid < 0x20 && packet->id == 0x6B &&
                (unit->something_per_player[pid - 0x10] & 0x40000000) == 0) {
                unit->something_per_player[pid - 0x10] |= 0x40000010;
                g_NetStru1_main.SendUnitVisibility(unit, 1, player);
            }
        } else {
            packet->to_player_id = player->player_id;
            this->QueuePacketSend(packet);
        }
    }
}

// 51B370
void NetStru1::BroadcastMovePacket(PacketMoveBroadcast* packet, TokenPos* pos) {
    if (packet->to_player_id != 0) {
        Player* player = g_PlayersList->FindById(packet->to_player_id);
        if (IsVisibleTo(pos, player)) {
            this->QueuePacketSend(packet);
        } else {
            MarkUnitHiddenForPlayer(packet, player);
        }
        return;
    }

    POSITION iter = g_PlayersList->list.GetHeadPosition();
    while (iter != nullptr) {
        Player* player = g_PlayersList->list.GetNext(iter);
        if (!player->is_active) {
            continue;
        }
        packet->to_player_id = player->player_id;
        if (IsVisibleTo(pos, player)) {
            this->QueuePacketSend(packet);
        } else {
            MarkUnitHiddenForPlayer(packet, player);
        }
    }
}

// 51B638
void NetStru1::BroadcastAoeZone(PacketAoeZone* packet, AreaEffect* area_effect) {
    if (packet->to_player_id != 0) {
        Player* player = g_PlayersList->FindById(packet->to_player_id);
        if ((area_effect->field4_0x42 & player->vision_sharing_id) != 0 || g_Server->field4_0x74 == 0) {
            this->QueuePacketSend(packet);
        } else {
            auto pid = player->player_id;
            if (pid >= 0x10 && pid < 0x20) {
                if (packet->set_flag != 0) {
                    area_effect->field6_0x46 |= player->vision_sharing_id;
                } else if ((area_effect->field6_0x46 & player->vision_sharing_id) == 0) {
                    this->QueuePacketSend(packet);
                }
            }
        }
        return;
    }

    POSITION iter = g_PlayersList->list.GetHeadPosition();
    while (iter != nullptr) {
        Player* player = g_PlayersList->list.GetNext(iter);
        if (!player->is_active) {
            continue;
        }
        if ((area_effect->field4_0x42 & player->vision_sharing_id) != 0 || g_Server->field4_0x74 == 0) {
            packet->to_player_id = player->player_id;
            this->QueuePacketSend(packet);
        } else {
            auto pid = player->player_id;
            if (pid >= 0x10 && pid < 0x20) {
                if (packet->set_flag != 0) {
                    area_effect->field6_0x46 |= player->vision_sharing_id;
                } else if ((area_effect->field6_0x46 & player->vision_sharing_id) == 0) {
                    packet->to_player_id = player->player_id;
                    this->QueuePacketSend(packet);
                }
            }
        }
    }
}

// 51BAB0
void NetStru1::SendSpellCastAtUnit(Unit* caster, Spell* spell, Unit* target, int16_t delay) {
    PacketMoveBroadcast& packet = PacketMoveBroadcast::Inst;
    packet.id = 0x86;
    packet.unit_id = caster->building_id;
    packet.type_id = 0;
    packet.x = 0;
    packet.y = 0;
    if (caster->typeId == 0) {
        packet.id = 0x8B;
        // Original code was writing the X and Y separately into +0xa and +0xb. I hope it's the same.
        packet.unit_id = caster->position->GetX() | (caster->position->GetY() << 8);
    }
    packet.type_id = (spell->spell_id * 2 + 8);
    if (target != nullptr) {
        *(uint16_t*)&packet.x = (uint16_t)target->building_id;
        packet.segments = delay;
    }
    if (caster->IsUnit()) {
        caster->last_action_tick = g_Server->tick;
    }
    packet.to_player_id = 0;
    this->BroadcastMovePacket(&packet, caster->position);
}

// 51BB94
void NetStru1::SendSpellCastAtPos(Unit* caster, Spell* spell, TokenPos* pos, int16_t delay) {
    PacketMoveBroadcast& packet = PacketMoveBroadcast::Inst;
    packet.id = 0x86;
    packet.unit_id = caster->building_id;
    if (caster->typeId == 0) {
        packet.id = 0x8B;
        // Original code was writing the X and Y separately into +0xa and +0xb. I hope it's the same.
        packet.unit_id = caster->position->GetX() | (caster->position->GetY() << 8);
    }
    packet.type_id = spell->spell_id * 2 + 8;
    packet.x = pos->GetX();
    packet.y = pos->GetY();
    packet.segments = delay;
    if (caster->IsUnit()) {
        caster->last_action_tick = g_Server->tick;
    }
    packet.to_player_id = 0;
    this->BroadcastMovePacket(&packet, caster->position);
}

// 51BC6B
void NetStru1::SendSpellCastMulti(Unit* caster, Spell* spell, CList<Unit*>* units, int32_t flag) {
    PacketTerrain& packet = PacketTerrain::Inst;
    packet.id = 0x8A;
    packet.count = units->GetCount() + 1;
    packet.buf[0] = caster->building_id;
    if (caster->typeId == 0) {
        packet.id = 0x8C;
        packet.buf[0] = caster->position->GetYX();
    }
    int32_t i = 1;
    POSITION it = units->GetHeadPosition();
    while (it != nullptr) {
        Unit* unit = units->GetNext(it);
        packet.buf[i] = unit->building_id;
        i++;
    }
    if (caster->IsUnit()) {
        caster->last_action_tick = g_Server->tick;
    }
    packet.to_player_id = 0;
    // This reinterpret_cast is kinda weird, but it works, because `BroadcastMovePacket` only reads the 2-byte 0xa field from non-Packet fields.
    this->BroadcastMovePacket(reinterpret_cast<PacketMoveBroadcast*>(&packet), caster->position);
}

// 51BDA4
void NetStru1::BroadcastEffect(Effect* effect, Unit* unit, uint8_t packet_id) {
    PacketEffect& packet = PacketEffect::Inst;
    packet.id = packet_id ? packet_id : 0x88;
    packet.unit_id = unit->building_id;
    packet.effect_type = effect->typeId;
    packet.to_player_id = 0;
    // This reinterpret_cast is kinda weird, but it works, because `BroadcastMovePacket` only reads the 2-byte 0xa field from non-Packet fields.
    this->BroadcastMovePacket(reinterpret_cast<PacketMoveBroadcast*>(&packet), unit->position);
}

// 51C822
void NetStru1::SendSpectatorState(NetStru2* ns2) {
    PacketDword& packet = PacketDword::Inst;
    packet.id = 0x0E;
    packet.value = dword_665CFC;
    packet.WriteTo(ns2);
    packet.id = 0x64;
    packet.value = (uint32_t)g_Server->tick;
    packet.WriteTo(ns2);
    ns2->out_buffers[ns2->out_buff_id].pkt_num += 1;
    this->SendAllData();
}

// 51D837
void NetStru1::SendGameInfo(int32_t game_speed, Player* player) {
    PacketDword& packet = PacketDword::Inst;
    packet.id = 0x10;
    if (player != nullptr) {
        packet.to_player_id = player->player_id;
    } else {
        packet.to_player_id = 0;
    }
    packet.value = game_speed;
    this->QueuePacketSend(&packet);
}

// 51DFA7
void NetStru1::SendHatCharData(int32_t hat_char_id1, int32_t hat_char_id2, CString name, void* data, uint32_t size, uint16_t player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = player_id;
    packet.id = 0xCF;
    int32_t name_len = name.GetLength();
    packet.count = name_len + size + 0x10;
    uint32_t* p = (uint32_t*)packet.data;
    *p++ = hat_char_id1;
    *p++ = hat_char_id2;
    *p++ = name_len;
    *p++ = size;
    memcpy(p, data, size);
    memcpy(packet.data + 0x10 + size, (const char*)name, name_len);
    this->QueuePacketSend(&packet);
}

// 51E0B7
void NetStru1::SendHatMessage(int32_t hat_char_id1, int32_t hat_char_id2, CString name, void* data, uint32_t size, uint16_t player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = player_id;
    packet.id = 0xE0;
    int32_t name_len = name.GetLength();
    packet.count = name_len + size + 0x10;
    uint32_t* p = (uint32_t*)packet.data;
    *p++ = hat_char_id1;
    *p++ = hat_char_id2;
    *p++ = name_len;
    *p++ = size;
    memcpy(p, data, size);
    memcpy(packet.data + 0x10 + size, (const char*)name, name_len);
    this->QueuePacketSend(&packet);
}

// 51E1C7
void NetStru1::SendHatKeepalive(uint16_t player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = player_id;
    packet.id = 0xE0;
    packet.count = 1;
    this->QueuePacketSend(&packet);
}

// 51E205
void NetStru1::SendHatLogin(CString name) {
    PacketJoin& packet = PacketJoin::Inst;
    packet.id = 0xD1;
    packet.__field_0xa = 0;
    strcpy(packet.name, name);
    packet.to_player_id = 0;
    this->QueuePacketSend(&packet);
}

// 51E40C
void NetStru1::SendHatRawString(const char* str, int32_t len) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = 0;
    packet.id = 0xD4;
    while (len != 0) {
        if (len <= 0x10000) {
            packet.count = len;
        } else {
            packet.count = 0x10000;
        }
        len -= packet.count;
        memcpy(packet.data, str, packet.count);
        str += packet.count;
        this->QueuePacketSend(&packet);
    }
}

// 51E5FB
void NetStru1::SendHatJoinOk(int32_t a, int32_t b) {
    Packet2Dwords& packet = Packet2Dwords::Inst;
    packet.id = 0xD8;
    packet.param1 = a;
    packet.param2 = b;
    packet.to_player_id = 0;
    this->QueuePacketSend(&packet);
}

// 51E63F
void NetStru1::SendHatJoinFail(int32_t a, int32_t b, int32_t error_code) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = 0;
    packet.id = 0xD9;
    packet.count = 0xC;
    uint32_t* p = (uint32_t*)packet.data;
    *p++ = a;
    *p++ = b;
    *p = error_code;
    this->QueuePacketSend(&packet);
}

// 51E794
void NetStru1::SendHatPlayerCheck(int32_t a, int32_t b, uint8_t status, int16_t player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = player_id;
    packet.id = 0xE1;
    packet.count = 0xC;
    uint32_t* p = (uint32_t*)packet.data;
    *p++ = a;
    *p++ = b;
    *p = status;
    this->QueuePacketSend(&packet);
}

// 51A6D5
void NetStru1::SendMonsterInventory(Unit* unit, Player* player, int32_t start_index, int32_t end_index) {
    if (unit->typeId < 0x21 || unit->typeId >= 0x40) {
        return;
    }

    if (start_index + end_index == 0) {
        start_index = 0;
        end_index = unit->inventory->items.GetSize();
    }

    PacketUnitStateVec& packet = PacketUnitStateVec::Inst;
    packet.id = 0x76;
    packet.entry_count = (uint16_t)(end_index - start_index);
    packet.start_index = (uint16_t)start_index;
    packet.data_size = 0;

    packet.mode = 2;
    if (start_index != 0 || end_index != unit->inventory->items.GetSize()) {
        packet.mode |= 0x80;
    }

    packet.building_id = unit->building_id;
    if (player != nullptr) {
        packet.to_player_id = player->player_id;
    } else {
        packet.to_player_id = unit->pOwner->player_id;
    }

    int32_t index = 0;
    POSITION pos = unit->inventory->items.GetHeadPosition();
    while (pos != nullptr) {
        Item* item = unit->inventory->items.GetNext(pos);
        if (index >= start_index && index < end_index) {
            item->StoreToPacket(&packet, 0);
        }
        index++;
    }

    this->QueuePacketSend(&packet);
}

// 51A0EF
void NetStru1::SendHumanoidProps(Unit* unit, Player* player, int32_t flags) {
    if (!unit->IsHumanoid()) {
        return;
    }

    flags &= 0xFFF;
    if (!flags) {
        return;
    }

    if (player == nullptr) {
        POSITION pos = g_PlayersList->list.GetHeadPosition();
        while (pos != nullptr) {
            Player* p = g_PlayersList->list.GetNext(pos);
            if (p->is_active || p == unit->pOwner) {
                this->SendHumanoidProps(unit, p, 0xFFB);
            }
        }
        return;
    }

    if (unit->typeId < 0x21 || unit->typeId >= 0x40) {
        if (unit->sent_to_mask & player->vision_sharing_id) {
            return;
        }

        if (unit->pOwner != player) {
            PacketUnitEquipment& packet = PacketUnitEquipment::Inst;
            packet.id = 0x9C;
            packet.unit_id = (uint16_t)unit->building_id;
            packet.flags = (uint16_t)flags;
            packet.to_player_id = player->player_id;

            int32_t i = 0;
            if (flags & 1) {
                packet.prop[i++] = unit->weapon ? unit->weapon->item_id : 0;
            }
            if (flags & 2) {
                packet.prop[i++] = unit->shield ? unit->shield->item_id : 0;
            }
            for (int32_t slot = 3; slot <= 12; slot++) {
                if (flags & (1 << (slot - 1))) {
                    Item* equip = static_cast<Humanoid*>(unit)->equipment[slot];
                    packet.prop[i++] = equip ? equip->item_id : 0;
                }
            }

            this->QueuePacketSend(&packet);
            return;
        }
    }

    PacketUnitStateVec& packet = PacketUnitStateVec::Inst;
    packet.id = 0x76;
    packet.entry_count = 0;
    packet.start_index = (uint16_t)flags;
    packet.data_size = 0;
    packet.mode = 1;
    packet.building_id = unit->building_id;
    packet.to_player_id = player->player_id;
    if (flags & 1) {
        if (unit->weapon) {
            unit->weapon->StoreToPacket(&packet, 0);
        } else {
            Item{}.StoreToPacket(&packet, 0);
        }
        packet.entry_count++;
    }
    if (flags & 2) {
        if (unit->shield) {
            unit->shield->StoreToPacket(&packet, 0);
        } else {
            Item{}.StoreToPacket(&packet, 0);
        }
        packet.entry_count++;
    }
    if (unit->IsHumanoid()) {
        Humanoid* h = static_cast<Humanoid*>(unit);
        for (int32_t slot = 3; slot <= 12; slot++) {
            if (flags & (1 << (slot - 1))) {
                if (h->equipment[slot]) {
                    h->equipment[slot]->StoreToPacket(&packet, 0);
                } else {
                    Item{}.StoreToPacket(&packet, 0);
                }
                packet.entry_count++;
            }
        }
    }

    this->QueuePacketSend(&packet);
}

// 51A8B2
void NetStru1::SendShopTable(Token* token, Inventory* inventory, Player* player, int32_t mode) {
    PacketUnitStateVec& packet = PacketUnitStateVec::Inst;
    packet.id = 0x76;
    packet.entry_count = 0;
    packet.start_index = 0;
    packet.data_size = 0;
    packet.building_id = token->building_id;
    if (player == nullptr) {
        // Original code logged "Error - notify about shop table contents to all" (compiles to a no-op)
        packet.to_player_id = 0;
    } else {
        packet.to_player_id = player->player_id;
    }
    packet.mode = (uint8_t)mode;
    // ASM also calls a no-op library helper (unknown_libname_724) with inventory->items.m_nCount here.
    POSITION it = inventory->items.GetHeadPosition();
    while (it != nullptr) {
        Item* item = inventory->items.GetNext(it);
        item->StoreToPacket(&packet, mode == 4);
        packet.entry_count++;
    }
    this->QueuePacketSend(&packet);
}

// 51AA26
void NetStru1::SendShopShelf(Shop* shop, CMultiShopShelf* shelf, Player* player, uint8_t slot) {
    PacketUnitStateVec& packet = PacketUnitStateVec::Inst;
    packet.id = 0x76;
    packet.entry_count = 0;
    packet.start_index = 0;
    packet.data_size = 0;
    packet.building_id = shop->building_id;
    if (player == nullptr) {
        // Original code logged "Error - notify about shop contents to all" (compiles to a no-op)
        packet.to_player_id = 0;
    } else {
        packet.to_player_id = player->player_id;
    }
    packet.mode = slot;
    Item empty_item;
    int32_t count = shelf->items.GetSize();
    for (int32_t i = 0; i < count; i++) {
        Item* item = shelf->items[i];
        if (item != nullptr) {
            item->StoreToPacket(&packet, 0);
        } else {
            empty_item.StoreToPacket(&packet, 0);
        }
        packet.entry_count++;
    }
    this->QueuePacketSend(&packet);
}

// 51AB99
void NetStru1::SendShopShelves(CMultiShopInstance* inst, Player* player, int32_t shelf_id) {
    if (shelf_id == -1) {
        for (int32_t i = 0; i < 4; i++) {
            Shop* shop = nullptr;
            if (inst->shop_template != nullptr) {
                shop = inst->shop_template->shop;
            }
            this->SendShopShelf(shop, &inst->shelves[i], player, i + 5);
        }
    } else if (shelf_id > 0) {
        Shop* shop = nullptr;
        if (inst->shop_template != nullptr) {
            shop = inst->shop_template->shop;
        }
        this->SendShopShelf(shop, &inst->shelves[shelf_id - 1], player, shelf_id + 4);
    }
}

// 51AC77
void NetStru1::BroadcastTokenState(Token* token, Player* player, uint8_t flag) {
    if (player == nullptr) {
        POSITION pos = g_PlayersList->list.GetHeadPosition();
        while (pos != nullptr) {
            Player* p = g_PlayersList->list.GetNext(pos);
            if (p->is_active || p == token->pOwner) {
                this->BroadcastTokenState(token, p, flag);
            }
        }
        return;
    }

    if (token == nullptr) {
        // Original code here constructs a string "Token Change with NULL\n" and then discards it :) Guess it's from debugging.
        return;
    }

    if (token->IsUnit()) {
        Unit* unit = static_cast<Unit*>(token);
        if (flag == 0x73) {
            if (!(unit->seen_by_mask & player->vision_sharing_id) && g_Server->field4_0x74 != 0) {
                int32_t pid = player->player_id;
                if (0x10 <= pid && pid < 0x20) {
                    unit->something_per_player[pid - 0x10] |= 1;
                }
                return;
            }
            PacketUnitHp& packet = PacketUnitHp::Inst;
            packet.id = flag;
            packet.to_player_id = player->player_id;
            packet.unit_id = (uint16_t)unit->building_id;
            packet.hp = unit->hp;
            this->QueuePacketSend(&packet);
        }
        return;
    }

    if (token->IsKindOf(RUNTIME_CLASS(Sack))) {
        Sack* sack = static_cast<Sack*>(token);
        PacketSack& packet = PacketSack::Inst;
        packet.id = 0x7A;
        packet.to_player_id = player->player_id;
        packet.xpos = sack->position->GetXx();
        packet.ypos = sack->position->GetYy();
        packet.unit_id = (uint16_t)sack->building_id;
        packet.type_id = (uint8_t)log10(sack->_exp);

        if (g_ServerConfig.gameType == 2) {
            if (sack->inventory->FindItemByName("Quest RuneF")) {
                packet.type_id |= 0x40;
            }
            if (sack->inventory->FindItemByName("Quest RuneA")) {
                packet.type_id |= 0x80;
            }
        }

        if (g_Server->field4_0x74 != 0) {
            if (!IsVisibleTo(sack->position, player)) {
                return;
            }
        }

        sack->sent_to_mask |= player->vision_sharing_id;
        sack->resend_mask &= ~player->vision_sharing_id;
        this->QueuePacketSend(&packet);
        return;
    }

    if (token->IsBuilding()) {
        Building* building = static_cast<Building*>(token);
        building->sent_to_mask |= (player ? player->vision_sharing_id : 0xFFFF);
        if (building->typeId != 0 && building->hp_max != 0) {
            PacketBuildingHp& packet = PacketBuildingHp::Inst;
            packet.id = 0x82;
            packet.to_player_id = player ? player->player_id : 0;
            packet.building_id = (uint16_t)building->building_id;
            packet.hp = building->hp;
            this->QueuePacketSend(&packet);
        }
    }
}



void NetStru1::WriteStatsLog(CString fname) { //51f124
    CStdioFile f(fname, CFile::modeCreate | CFile::modeWrite);

    f.WriteString("Cmd|Packets|Avg Size| Total Size |Comment\n");
    f.WriteString("-------------------------------------------\n");

    for (int32_t i = 0; i < 0x100; i++) {
        if (this->stat_pkt_num[i] != 0) {
            CString pname;
            switch (i) {
            case 0x64:
                pname = "GM_ReportNewSegment";
                break;
            case 0x67:
                pname = "GM_ReportMoney";
                break;
            case 0x69:
                pname = "GM_ReportUnitShown";
                break;
            case 0x6b:
                pname = "GM_ReportUnitMove";
                break;
            case 0x6c:
            case 0x6e:
            case 0x6f:
            case 0x70:
                pname = "GM_ReportUnitChange";
                break;
            case 0x6d:
                pname = "GM_ReportUnitTurn";
                break;
            case 0x71:
                pname = "GM_ReportUnitAttack";
                break;
            case 0x72:
                pname = "GM_ReportRangedAttack";
                break;
            case 0x73:
                pname = "GM_ReportUnitDamaged";
                break;
            case 0x74:
                pname = "GM_ReportUnitHidden";
                break;
            case 0x76:
                pname = "GM_ReportItemList";
                break;
            case 0x79:
                pname = "GM_ReportUnitWearItems";
                break;
            case 0x7a:
                pname = "GM_ReportSack";
                break;
            case 0x86:
                pname = "GM_ReportEffect";
                break;
            case 0x87:
                pname = "GM_ReportAreaEffect";
                break;
            case 0x88:
                pname = "GM_ReportEffectHang";
                break;
            case 0x89:
                pname = "GM_ReportEffectGone";
                break;
            case 0x8a:
                pname = "GM_ReportEffectMultiTarget";
                break;
            case 0x8b:
                pname = "GM_ReportEffectFrom";
                break;
            case 0x8c:
                pname = "GM_ReportEffectMultiFrom";
                break;
            case 0x8d:
                pname = "GM_ReportMessage";
                break;
            case 0x96:
                pname = "GM_ReportNewPlayer";
                break;
            case 0x97:
                pname = "GM_ReportDeletePlayer";
                break;
            case 0x98:
                pname = "GM_ReportAshMap";
                break;
            case 0x9c:
                pname = "GM_ReportEnemyEquipment";
                break;
            case 0xb9:
                pname = "GM_ReportDiplomacy";
                break;
            case 0xba:
                pname = "GM_ReportCreatureKnowledge";
                break;
            case 0xbb:
                pname = "GM_ReportPlayerQuests";
                break;
            case 0xbc:
                pname = "GM_ReportInnQuests";
                break;
            }

            CString msg;
            msg.Format("%3d|%7d|%8d|%11d|%s\n", i, this->stat_pkt_num[i], this->stat_pkt_size[i] / this->stat_pkt_num[i], this->stat_pkt_size[i], pname);
            
            f.WriteString(msg);
        }
    }
    f.Close();
}

void NetStru1::SendHatCredentials(const CString& login, const CString& password, uint32_t flags) { //51d8d6
    PacketJoin* pkt = &PacketJoin::Inst;
    CString namepwd = login + password;

    strcpy(pkt->name, namepwd);

    pkt->id = 0xc9;

    pkt->player_id = login.GetLength();
    pkt->token_id = 0;
    pkt->color = flags;
    pkt->flags = 7;
    pkt->to_player_id = 0;

    this->QueuePacketSend(pkt);
}

void NetStru1::SendHatCharId(HatCharId chid) { //51d9d0
    Packet2Dwords* pkt = &Packet2Dwords::Inst;
    pkt->id = 0xca;
    pkt->param1 = chid.id1;
    pkt->param2 = chid.id2;
    pkt->to_player_id = 0;
    this->QueuePacketSend(pkt);
}

void NetStru1::SendHatLogout() { //51d9a0
    Packet* pkt = &Packet::Inst;
    pkt->id = 0xc8;
    pkt->to_player_id = 0;
    this->QueuePacketSend(pkt);
}
