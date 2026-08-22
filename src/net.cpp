#include "net.h"

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


extern "C" uint32_t dword_665CFC;

GUID Allods2GUID = {0x65450caa, 0x57ca, 0x11d2, {0xbb, 0xcc,0x00, 0x60, 0x97, 0xd2, 0xee, 0x9f}};


NetStru1 NetStru1::HatConnector(0); // 6a6a98
NetStru1 g_NetStru1_main(1);  //6c3a08
NetStru1 g_NetStru1_local(0);  //6b16b0

CLlDriver g_CLlDriver(Allods2GUID, &g_NetStru1_main); //6d07a0
CLlDriver g_HatLLDriver(Allods2GUID, &NetStru1::HatConnector); //6cdbc0


void PackerDat::DeleteTail(PackerTail* node)
{
    //526621
    if (node)
    {
        if (node->field_0x0)
            DeleteTail(node->field_0x0);
        if (node->field_0x4)
            DeleteTail(node->field_0x4);

        delete node;
    }
}

void PackerDat::Clear()
{
    //5265f7
    DeleteTail(tail);
    tail = nullptr;
}

PackerDat::PackerDat()
{
    //5260fa
    tail = nullptr;
}

PackerDat::~PackerDat()
{
    //526115
    Clear();
}

void NetStru1::FUN_0051cd89(const CString& name, Player* player)
{
	PacketJoin& pkt = PacketJoin::Inst;
	pkt.id = 0x91;
	strcpy(pkt.name, name);

	if (player)
		pkt.to_player_id = player->player_id;
	else
		pkt.to_player_id = 0;

	pkt.player_id = 0;
	pkt.token_id = 0;
	pkt.field_0xc = 0;
	pkt.flags = 0;

	QueuePacketSend(&pkt);
}

// 51C8B1
void NetStru1::sub_51C8B1(Player* player) {
    LogMessage("NetStru1::sub_51C8B1");
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
		packet.field_0xc = p->field_0xa44 - 1;
		packet.flags = p->is_ai != 0;
		this->QueuePacketSend(&packet);

		if (p->main_unit != nullptr && p->main_unit->field_0x1bc != 0 && g_ServerConfig.gameType == 0) {
			g_NetStru1_main.FUN_0051ce86(5, p->player_id, player);
		}
	}
}

// 51C61E
void NetStru1::sub_51C61E(Token* token) {
    PacketWord& packet = PacketWord::Inst;

    packet.id = 0x6a;
    packet.value = token->building_id;
    POSITION pos = g_PlayersList->list.GetHeadPosition();
    while (pos != nullptr) {
        Player* player = g_PlayersList->list.GetNext(pos);
        if (token->field_x18 & player->vision_sharing_id) {
            packet.to_player_id = player->player_id;
            this->QueuePacketSend(&packet);
        }
    }
}

// 51c748
void NetStru1::FUN_0051c748(Player* player) {
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
void NetStru1::FUN_0051ce86(uint32_t msg_type, uint32_t player_id, Player* recpt) {
    this->FUN_0051cefb(0x92, msg_type, player_id, recpt);
}

// 51ceac
void NetStru1::FUN_0051ceac(uint8_t id, Player* player) {
    Packet& packet = Packet::Inst;
    packet.id = id; 
    packet.to_player_id = player ? player->player_id : 0;
    this->QueuePacketSend(&packet);
}

// 51d49b
void NetStru1::FUN_0051d49b(Player* player) {
    PacketJoin& packet = PacketJoin::Inst;
    packet.id = 0x93;
    strcpy(packet.name, player->name);
    packet.to_player_id = 0;
    packet.__field_0xa = player->player_id;
    this->QueuePacketSend(&packet);
}

// 51cefb
void NetStru1::FUN_0051cefb(uint8_t param_1, int32_t param_2, int32_t param_3, Player* param_4) {
    PacketInfo& packet = PacketInfo::Inst;
    packet.id = param_1;
    packet.to_player_id = param_4 ? param_4->player_id : 0;
    packet.field_0xa = param_2;
    packet.field_0xe = param_3;
    this->QueuePacketSend(&packet);
}

// 51C7CC
void NetStru1::sub_51C7CC(int32_t latency, Player* player) {
    PacketDword& packet = PacketDword::Inst;
    packet.id = 0xc1;
    packet.value = latency;
    packet.to_player_id = player ? player->player_id : 0;
    this->QueuePacketSend(&packet);
}

// 51CD2A
void NetStru1::sub_51CD2A(Player* player, int32_t event_id, int32_t arg3) {
    PacketInfo& packet = PacketInfo::Inst;
    packet.id = 0xb6;
    packet.to_player_id = player ? player->player_id : 0;
    packet.field_0xa = event_id;
    packet.field_0xe = arg3;
    this->QueuePacketSend(&packet);
}

// 51CA5D
void NetStru1::sub_51CA5D(Player* player) {
    POSITION pos = g_Server->srv_stru1->sack_list->list.GetHeadPosition();
    while (pos != nullptr) {
        Sack* sack = g_Server->srv_stru1->sack_list->list.GetNext(pos);
        this->sub_51AC77(sack, player, 0);
    }
}

// 51C601
void NetStru1::sub_51C601(Unit* unit, int unused) {
    this->sub_51AC77(unit, nullptr, 0x73);
}

// 51B870
void NetStru1::sub_51B870(Unit* unit) {
    POSITION pos = g_PlayersList->list.GetHeadPosition();
    while (pos != nullptr) {
        Player* player = g_PlayersList->list.GetNext(pos);
        if (player->field_0x43 && unit->sub_5287c4(player)) {
            unit->position->sub_58bec3(); // WAT: Useless call?
            if (unit->VMethod7()) {
                this->sub_519221(unit, player, 0xFFB, -1, 0, 0);
            } else {
                this->sub_51AC77(unit, player, 0);
            }
        }
    }
}

// 51BE8F
void NetStru1::sub_51BE8F(AreaEffect* obj, int32_t flag) {
    if (obj->itemDataID == 2) {
        PacketMoveCmd& packet = PacketMoveCmd::Inst;
        packet.id = 0x86;
        packet.field_0xd = obj->position->GetX();
        packet.field_0xe = obj->position->GetY();
        packet.field_0xf = 0x16;
        packet.field_0xa = (uint16_t)obj->building_id;
        packet.field_0xc = (uint8_t)obj->typeId;
        packet.to_player_id = 0;
        this->sub_51B370(&packet, obj->position);
    } else {
        PacketAoeZone& packet = PacketAoeZone::Inst;
        packet.id = 0x87;

        const int8_t radius = obj->field_0x4d;
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
                AreaEffect* applicable = MapStuff_Instance->sub_5953CB(obj, coordinate);
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
        packet.field_0xf = (uint8_t)flag;
        packet.field_0xa = (uint8_t)obj->typeId;
        packet.to_player_id = 0;
        this->sub_51B638(&packet, obj);
    }
}

// 51C0F7
void NetStru1::sub_51C0F7(Player* player) {
    if (dword_6CDB3C != nullptr) {
        POSITION pos = dword_6CDB3C->unit_list.GetHeadPosition();
        while (pos != nullptr) {
            Unit* unit = dword_6CDB3C->unit_list.GetNext(pos);
            uint32_t flags = (unit->building_id < 0x6000) ? 0xA35FFFFF : 0x100013;
            if (unit->field_0x204 != 0) {
                flags |= 0x40;
            }
            if (g_Server->field4_0x74 == 0 && 0x16 <= unit->server_id && unit->server_id <= 0x17) {
                flags |= 0x200000;
            }
            g_NetStru1_main.sub_519221(unit, player, flags, 0xFFB, 0, 0);
        }
    }

    UnitList* units_list = g_Server->srv_stru1->units_list;
    if (units_list != nullptr) {
        POSITION pos = units_list->unit_list.GetHeadPosition();
        while (pos != nullptr) {
            Unit* unit = units_list->unit_list.GetNext(pos);
            uint32_t flags = (unit->building_id < 0x6000) ? 0xA35FFFFF : 0x13;
            if (unit->field_0x204 != 0) {
                flags |= 0x40;
            }
            if (unit->decay < 5) {
                g_NetStru1_main.sub_519221(unit, player, flags, 0xFFB, 0, 0);
            }
        }
    }

    BuildingsList* building_list = g_Server->srv_stru1->building_list;
    POSITION pos = building_list->GetHeadPosition();
    while (pos != nullptr) {
        Building* building = building_list->GetNext(pos);
        if (building->hp != building->hp_max) {
            g_NetStru1_main.sub_51AC77(building, player, 0);
        }
    }
}

// 51C46E
void NetStru1::sub_51C46E(Player* player) {
    if (player->main_unit != nullptr) {
        this->sub_519221(player->main_unit, player, 0xFFB, -1, 0, 0);
    }
    if (player->unit_list != nullptr) {
        POSITION pos = player->unit_list->unit_list.GetHeadPosition();
        while (pos != nullptr) {
            Unit* unit = player->unit_list->unit_list.GetNext(pos);
            unit->field_x18 &= ~player->vision_sharing_id;
            g_NetStru1_main.sub_519221(unit, player, 0xFFB, -1, 0, 0);
        }
    }
    this->FUN_0051c748(player);
}

// 51CB21
void NetStru1::sub_51CB21(Player* player) {
    if (g_World == nullptr) {
        return;
    }
    PacketTerrain& packet = PacketTerrain::Inst;
    packet.id = 0xB9;
    packet.to_player_id = player->player_id;
    int32_t count = g_PlayersList->sub_535F97() + 1;
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
void NetStru1::sub_51CF5C(Unit* unit, int32_t flag, Player* player) {
    PacketWord& packet = PacketWord::Inst;
    packet.id = flag ? 0x74 : 0x69;
    packet.value = unit->building_id;

    if (player != nullptr) {
        packet.to_player_id = player->player_id;
        if (g_Server->field4_0x74 != 0) {
            if ((unit->field_0x1a4 & player->vision_sharing_id) == 0) {
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
        if (!p->field_0x43) {
            continue;
        }
        packet.to_player_id = p->player_id;
        if (g_Server->field4_0x74 != 0) {
            if ((unit->field_0x1a4 & p->vision_sharing_id) == 0) {
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
void NetStru1::sub_51D1A8(uint16_t player_id, Player* player) {
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
void NetStru1::sub_51D4F6(QuestMap* quest_map, Player* player, int32_t flag) {
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
void NetStru1::FUN_0051d6b4(uint16_t arg) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = arg;
    packet.id = 0xC2;
    packet.count = 0x58;
    uint32_t* ptr = reinterpret_cast<uint32_t*>(packet.data);
    int32_t teams_started = g_PlayersList->sub_53636E();
    *ptr++ = teams_started;
    *ptr++ = g_ServerConfig.chat_range;
    *ptr++ = g_Server->ctf_carrying[0];
    *ptr++ = g_Server->ctf_carrying[1];
    *ptr++ = teams_started ? g_Server->ctf_score[0] : 0xF0000001u;
    *ptr++ = teams_started ? g_Server->ctf_score[1] : 0xF0000001u;
    for (int32_t i = 0x10; i < 0x20; i++) {
        Player* p = g_PlayersList->sub_535B50(i);
        if (p == nullptr) {
            *ptr++ = 0xF0000003u;
        } else {
            *ptr++ = teams_started ? p->frags : p->field_0xa6c;
        }
    }
    this->QueuePacketSend(&packet);
}

// 51EEB7
void NetStru1::sub_51EEB7() {
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
void NetStru1::sub_51E289(int32_t arg0, CString source, int32_t arg8, int32_t argC, int32_t arg10) {
	PacketJoin& pkt = PacketJoin::Inst;
	pkt.id = 0xd2;
	pkt.__field_0xa = arg0 | (arg8 << 8) | (argC << 16) | (arg10 << 24);
	strcpy(pkt.name, source);
	pkt.to_player_id = 0;
	this->QueuePacketSend(&pkt);
}

// 4FB4CA
void NetStru1::FUN_004fb4ca(Unit* unit, Player* new_owner)
{
    #ifdef A2SERVER_PATCH
    if (unit == nullptr) {
        LogMessage("FUN_004fb4ca: unit is nullptr.");
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
    
    // Update unit's vision sharing flags.
    uint16_t old_vision_mask = ~old_owner->vision_sharing_id;
    unit->field_x18 = unit->field_x18 & old_vision_mask;
    
    uint16_t new_vision_mask = ~new_owner->vision_sharing_id;
    unit->field_x18 = unit->field_x18 & new_vision_mask;
    
    // Send network packet about unit ownership change.
    g_NetStru1_main.sub_519221(unit, 0, 0xFFFFFFFF, 0xFFB, 0, 0);
    
    // Mark unit as needing update?
    unit->field_0x204 = 1;
}

// sub_519221: send unit state to a player (or all players if player==nullptr)
// 519221
void NetStru1::sub_519221(Unit* unit, Player* player, uint32_t mask, int32_t param5, int32_t param6, int32_t param7)
{
    if (player == nullptr) {
        // Broadcast to all players.
        POSITION pos = g_PlayersList->list.GetHeadPosition();
        while (pos != nullptr) {
            Player* p = g_PlayersList->list.GetNext(pos);
            if (p->field_0x43 != 0 || unit->pOwner == p) {
                this->sub_519221(unit, p, mask, param5, param6, param7);
            }
        }
        return;
    }

    // Check unit visibility to this player (unit->field_0x1a4 & player->vision_sharing_id).
    if (!(unit->field_0x1a4 & player->vision_sharing_id)) {
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

    int is_humanoid = unit->VMethod8();

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
                if (g_World->diplomacy.diplomacy[unit->pOwner->player_id][player->player_id] & 0x10 == 0) {
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
        uint8_t value = unit->eye->sub_58c00e();
        pkt->PutByte(0x10, value);

        if (unit->position->sub_58bec3() == 0) {
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
    if ((mask & 0x100000) && unit->spell_book != nullptr)
    {
        pkt->PutInt(0x100000, unit->spell_book->sub_53DD3D());
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
        this->sub_51A0EF(unit, player, param5);
    }

    // Owner-only: send inventory.
    if (orig_mask & 0x200000 && unit->pOwner == player) {
        this->sub_51A6D5(unit, player, param6, param7);
    }

    bool needs_refresh = unit->sub_5287c4(player);

    // Send effects.
    if (needs_refresh || has_enchantments) {
        POSITION iter = unit->_effects.GetHeadPosition();
        while (iter != nullptr) { 
            Effect* effect = unit->_effects.GetNext(iter);
            if (effect && effect->itemDataID != 0) {
                this->sub_51BDA4(effect, unit, 0);
            }
        }
    }

    if ((needs_refresh && (unit->unit_attrs & 8) != 0) || (mask & 0x40000000) != 0) {
        g_NetStru1_main.sub_51CF5C(unit, (unit->unit_attrs & 8) ? 1 : 0, player);
    }

    // Mark unit as not needing the refresh in the "field_x18" bitmask.
    unit->sub_5287ec(player);

    // Clear per-player pending bits for players in the 0x10..0x1F range.
    if (0x10 <= player->player_id && player->player_id < 0x20) {
        unit->something_per_player[player->player_id - 16] = 0;
    }
}

// DecodePacket
Packet* NetStru1::DecodePacket(uint8_t cmd, NetStru2* ns2)
{
    Packet* pkt;

    switch (cmd) {
    case 0x2:
        pkt = &Packet3Dwords::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x3: case 0xaf: case 0xb7: case 0xb8:
        pkt = &Packet::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x4: case 0xb: case 0x39: case 0x3b: case 0x5c: case 0xca:
    case 0xcc: case 0xd0: case 0xd5: case 0xd8: case 0xe1:
        pkt = &PacketInfo::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x5: case 0x9: case 0xa: case 0x4a: case 0x4b: case 0x4c:
    case 0x5a: case 0xc8: case 0xd6: case 0xdb: case 0xdc:
        pkt = &Packet::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x6: case 0x7: case 0x8: case 0x4e: case 0x5b: case 0x93:
    case 0xbf: case 0xc9: case 0xd1: case 0xd2: case 0xd3:
        pkt = &PacketJoin::Inst;
        pkt->VMethod4(ns2);
        break;

    default:
        LogMessage(CString("Drv: unknown cmd type!"));
        pkt = &Packet::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0xe: case 0x10: case 0xb3: case 0xc1: case 0xdf:
        pkt = &PacketDword::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0xf: case 0x49: case 0x91: case 0x96: case 0xae:
        pkt = &PacketJoin::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x12: case 0x13: case 0x14: case 0x17: case 0x18: case 0x4d:
        pkt = &PacketItemOperation::Inst;
        pkt->VMethod4(ns2);
        pkt->field_0x4 = 1;
        break;

    case 0x16: case 0x1a: case 0x1c: case 0x1d: case 0x1f: case 0x21: case 0x24: case 0x26:
        pkt = &PacketItemOperation::Inst;
        pkt->VMethod4(ns2);
        pkt->field_0x4 = 2;
        break;

    case 0x19: case 0x1b: case 0x1e: case 0x25:
        pkt = &PacketItemOperation::Inst;
        pkt->VMethod4(ns2);
        pkt->field_0x4 = 3;
        break;

    case 0x22:
        pkt = &PacketCmd::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x23: case 0x3a: case 0x3d: case 0x3e: case 0x3f: case 0x40:
    case 0x46: case 0x48: case 0x83: case 0x84: case 0x92: case 0x94:
    case 0xaa: case 0xc3:
        pkt = &PacketInfo::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x38:
    case 0x69: case 0x6a: case 0x74:
        pkt = &PacketWord::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x45: case 0x8a: case 0x8c: case 0x9b: case 0xb9: case 0xbb: case 0xbc:
        pkt = &PacketTerrain::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x64:
        pkt = &PacketDword::Inst;
        pkt->VMethod4(ns2);
        ns2->sub_5167A5();
        break;

    case 0x65: case 0x67: case 0x97: case 0xab: case 0xac: case 0xad:
    case 0xb4: case 0xb5: case 0xb6:
        pkt = &PacketInfo::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x6b: case 0x6d: case 0x71:
        pkt = &PacketAbility::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x6c: case 0x6e: case 0x6f: case 0x70:
        pkt = &PacketUnitUpdate::Inst;
        pkt->id = cmd;
        pkt->VMethod4(ns2);
        break;

    case 0x72:
        pkt = &PacketMount::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x73:
        pkt = &PacketPing::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x76:
        pkt = &PacketUnitStateVec::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x7a:
        pkt = &PacketEight::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x82:
        pkt = &PacketSync::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x86: case 0x8b:
        pkt = &PacketMoveCmd::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x87:
        pkt = &PacketAoeZone::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x88: case 0x89:
        pkt = &PacketEffect::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0x9c:
        pkt = &PacketUnitProperties::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0xba: case 0xc2: case 0xcb: case 0xcd: case 0xce: case 0xcf:
    case 0xd4: case 0xd9: case 0xda: case 0xdd: case 0xde: case 0xe0:
        pkt = &PacketData::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0xbe:
        pkt = &PacketData::Inst;
        pkt->VMethod4(ns2);
        break;

    case 0xc0:
        pkt = &PacketPlayerInfo::Inst;
        pkt->VMethod4(ns2);
        break;
    }

    pkt->id = cmd;

    if (make_statistics == 0) {
        pkt->field_0x5 = 0;
    } else if (ns2->player_id == 0) {
        pkt->field_0x5 = static_cast<uint16_t>((ns2->uid & 0x3FFF) | 0x4000);
    } else {
        pkt->field_0x5 = ns2->player_id;
    }

    return pkt;
}

Packet* NetStru1::ReceivePacket(NetStru2* cli)
{
    //518a23
    if (cli->GetBuffersPktNum() == 0)
        return nullptr;

    uint8_t id = 0;
    cli->ReadData(&id, 1); //read pkt id
    return DecodePacket(id, cli);
}

Packet* NetStru1::ReceiveAnyPacket()
{
    //518980
    for (POSITION it = active_connects.GetHeadPosition(); it != nullptr;)
    {
        NetStru2* cli = active_connects.GetNext(it);
        if (cli)
        {
            Packet* pkt = ReceivePacket(cli);
            if (pkt)
            {
                if (make_statistics == 0) // ????? bug?  fixme?
                {
                    AddStat1(cli, pkt->GetDataSize());
                    AddPacketStatistic(pkt);
                }
                return pkt;
            }
        }
    }
    return nullptr;
}




void NetStru1::AddTailNet2(NetStru2*& net2)
{
    //517e91
    EnterCriticalSection(&critical_section);
    disconnect_list.AddTail(net2);
    LeaveCriticalSection(&critical_section);
}

void NetStru1::AddTailFreeNet3(NetStru3* net3)
{
    //517b00
    EnterCriticalSection(&critical_section2);
    free_net3.AddTail(net3);
    LeaveCriticalSection(&critical_section2);
}

void NetStru1::ProcessNewConnects()
{
    //51755c
    EnterCriticalSection(&critical_section);
    while (!new_connects.IsEmpty())
    {
        NetStru2* cli = new_connects.RemoveHead();
        if (cli)
        {
            if (compression_mode != 0)
                cli->compression_type = compression_type;
            else
                cli->compression_type = 0;

            active_connects.AddTail(cli);

            OnClientConnect(cli);
        }
    }
    LeaveCriticalSection(&critical_section);
}

void NetStru1::ProcessDisconnectList()
{
    //51725b
    EnterCriticalSection(&critical_section);
    while (!disconnect_list.IsEmpty())
    {
        NetStru2* cli = disconnect_list.RemoveHead();
        if (cli)
        {
            POSITION pos = active_connects.Find(cli);
            if (pos)
            {
                OnClientDisconnect(cli);
                active_connects.RemoveAt(pos);
            }

            delete cli;
        }
    }
    LeaveCriticalSection(&critical_section);
}

void NetStru1::ProcessConnections()
{
    // 51800f
    ProcessNewConnects();
    ProcessDisconnectList();
    if (lldriver)
        lldriver->CleanupAllInvalid();
}

void NetStru1::DisconnectClient(NetStru2* cli)
{
    //5170b6
    if (cli == local_client)
    {
        AddTailNet2(local_client);
        local_client = nullptr;
    }
    else
       lldriver->DisconnectClient(cli->uid);
}

NetStru1::NetStru1(int param)
{
    //51684b
    lldriver = nullptr;
    linked_hl = nullptr;
    local_client = nullptr;
    make_statistics = param;
    InitializeCriticalSection(&critical_section);
    InitializeCriticalSection(&critical_section2);
    field_0x18a4 = 0;
    field_0x18a0 = 0;
    field_0x189c = 0;
    field_0x1898 = 0;
    compression_mode = 0;
    compression_type = 1;
    memset(stat_pkt_num, 0, sizeof(stat_pkt_num));
    memset(stat_pkt_size, 0, sizeof(stat_pkt_size));
}

NetStru1::~NetStru1()
{
    //516b44
    if (local_client != nullptr)
        DisconnectClient(local_client);

    ProcessNewConnects();
    ProcessDisconnectList();

    //inline 51710f 
    while (!free_net3.IsEmpty())
    {
        NetStru3* buf = free_net3.RemoveHead();
        delete buf;
    }

    DeleteCriticalSection(&critical_section);
    DeleteCriticalSection(&critical_section2);

    lldriver = nullptr;
    linked_hl = nullptr;
    make_statistics = 0;
    field_0x18a4 = 0;
    field_0x18a0 = 0;
    field_0x189c = 0;
    field_0x1898 = 0;
    compression_mode = 0;
    compression_type = 1;
}


void NetStru1::OnClientConnect(NetStru2* cli)
{
    //51fc72
    if (this != &HatConnector)
    {
        client_stat[cli->uid] = new ConnStatInfo;

        if (lldriver && lldriver->provider != 4)
            cli->field_0x2a8 = 1;
        else
            cli->field_0x2a8 = 0;
    }
}

void NetStru1::OnClientDisconnect(NetStru2* cli)
{
    //51f561
    if (this == &HatConnector)
    {
        LogMessage("Connection with hat has been lost.");
        if (g_Server)
            g_Server->field51_0x1d8 = GetTickCount();
        return;
    }

    if (lldriver && lldriver->is_server != 0 && lldriver->provider == 1)
        lldriver->RestartModemServerDp();

    if (g_Server)
    {
        Player* pl = g_PlayersList->sub_535B50(cli->player_id);
        if (pl && pl->field_0xa50 == 0)
        {
            if (pl->is_ai == 1 || pl->player_id <= 15)
                LogMessage("Warning: trying to disconnect AI player " + pl->name);
            else
            {
                LogMessage("Player " + pl->name + " has disconnected.");

                pl->FUN_00534778();

                if (lldriver && lldriver->provider == 4 &&
                    g_ServerConfig.gameType != 1 &&
                    g_ServerConfig.gameType != 3 &&
                    g_Server->field59_0x208 == 0)
                {
                    if (g_ServerConfig.gameType == 2)
                    {
                        if (g_PlayersList->sub_53636E())
                            pl->field_0xa50 = MAXINT32;
                    }
                    else
                        pl->field_0xa50 = g_Server->tick16 + 300;
                }
                if (!MapStuff_Instance && g_Server->field4_0x74)
                {
                    FUN_0051cefb(0x97, pl->player_id, 0, nullptr);
                    LogMessage("Player " + pl->name + " has disconnected and leaved game.");

                    POSITION pos = g_PlayersList->list.Find(pl);
                    if (pos != nullptr)
                        g_PlayersList->list.RemoveAt(pos);

                    g_Server->sub_4F4570();
                }
            }
        }
    }

    ConnStatInfo* stat = nullptr;
    if (client_stat.Lookup(cli->uid, stat))
    {
        client_stat.RemoveKey(cli->uid);
        if (stat)
            delete stat;
    }
}


NetStru2* NetStru1::AllocClientBufManager(uint32_t uid)
{
    //517c99
    EnterCriticalSection(&critical_section);

    NetStru2* mgr = new NetStru2();
    mgr->uid = uid;
    mgr->SetDrivers(this, lldriver);

    new_connects.AddTail(mgr);
    LeaveCriticalSection(&critical_section);

    return mgr;
}


NetStru3* NetStru1::GetFreeNet3()
{
    //5177f5
    EnterCriticalSection(&critical_section2);
    NetStru1* pool = this;
    if (linked_hl && make_statistics == 0)
        pool = linked_hl;

    if (!pool->free_net3.IsEmpty() && pool->free_net3.GetHeadPosition() == NULL) //Seems HACK
        new(&pool->free_net3) CList<NetStru3*>(); //Here was memcpy with stack allocated Clist in Nival code >_<

    NetStru3* buf = nullptr;
    if (pool->free_net3.IsEmpty())
        buf = new NetStru3();
    else
        buf = pool->free_net3.RemoveHead();
    LeaveCriticalSection(&critical_section2);
    return buf;
}

NetStru2* NetStru1::GetClientByPlayerID(uint16_t player_id)
{
    //518544
    for (POSITION it = active_connects.GetHeadPosition(); it != nullptr;)
    {
        NetStru2* cli = active_connects.GetNext(it);
        if (cli->player_id == player_id)
            return cli;
    }

    if (make_statistics == 0 && local_client != nullptr)
        return local_client;

    return nullptr;
}

NetStru2* NetStru1::GetClientByLowUid(uint32_t low_uid)
{
    //5185d5
    uint32_t luid = low_uid & 0x3fff;
    for (POSITION it = active_connects.GetHeadPosition(); it != nullptr;)
    {
        NetStru2* cli = active_connects.GetNext(it);
        if ((cli->uid & 0x3fff) == luid)
            return cli;
    }
    return nullptr;
}

void NetStru1::AddStat1(NetStru2* client, int32_t value)
{
    //51eff8
    ConnStatInfo* stat = nullptr;
    client_stat.Lookup(client->uid, stat);

    if (stat)
        stat->field1 += value;
}

void NetStru1::AddPacketStatistic(Packet* pkt)
{
    //51f0bb
    stat_pkt_num[pkt->id]++;
    stat_pkt_size[pkt->id] += pkt->GetDataSize();
}

void NetStru1::SendAllData()
{
    //5188db
    //518927
    for (POSITION it = active_connects.GetHeadPosition(); it != nullptr;)
    {
        NetStru2* cli = active_connects.GetNext(it);
        if (cli)
            cli->SendData();
    }
}

void NetStru1::SendPacket_64(uint32_t val, uint16_t player_id)
{
    //51d88d
    PacketDword& pkt = PacketDword::Inst;
    pkt.id = 0x64;
    pkt.value = val;
    pkt.to_player_id = player_id;
    QueuePacketSend(&pkt);

#ifdef A2CLIENT
    SendAllData();
#else
    if ((val & 3) == 1)
        SendAllData();
#endif
}

void NetStru1::SetLLDriver(CLlDriver* drv)
{
    //517ff9
    lldriver = drv;
}

void NetStru1::SetLinkedHLDriver(NetStru1* hl)
{
    //51703e
    linked_hl = hl;
    if (local_client)
        DisconnectClient(local_client);
    local_client = AllocClientBufManager(-1);
    local_client->is_local_player = 1;
    local_client->compression_type = 0;
}

void NetStru1::QueuePacketSend(Packet* pkt)
{
    //5186cd

    //uint32_t stime = GetTickCount();
    //printf("Send packet 0x%x\n", pkt->id);
    if (pkt->to_player_id == 0)
    {
        for (POSITION it = active_connects.GetHeadPosition(); it != nullptr;)
        {
            NetStru2* cli = active_connects.GetNext(it);
            if (cli && (make_statistics == 0 || cli->player_id != 0))
            {
                pkt->VMethod3(cli);
                if (make_statistics != 0)
                {
                    AddStat1(cli, pkt->GetDataSize());
                    AddPacketStatistic(pkt);
                }
                if (pkt->id == 0x64)
                    cli->out_buffers[cli->out_buff_id].pkt_num++;
            }
        }
    }
    else
    {
        NetStru2* cli = GetClientByPlayerID(pkt->to_player_id);
        if (cli)
        {
            pkt->VMethod3(cli);
            if (make_statistics != 0)
            {
                AddStat1(cli, pkt->GetDataSize());
                AddPacketStatistic(pkt);
            }
            if (pkt->id == 0x64)
                cli->out_buffers[cli->out_buff_id].pkt_num++;
        }
    }

    if (make_statistics != 0 && pkt->id == 0x76)
    {
        //DAT_0070b410 += (GetTickCount() - stime);
    }

    if (make_statistics == 0 && pkt->id != 0x64)
        SendPacket_64(1, 0);
}


uint32_t NetStru1::GetClientsPktNum()
{
    //518a6b
    uint32_t sum = 0;
    for (POSITION it = active_connects.GetHeadPosition(); it != nullptr;)
    {
        NetStru2* cli = active_connects.GetNext(it);
        if (cli)
            sum += cli->GetBuffersPktNum();
    }
    return sum;
}

int NetStru1::IsActive()
{
    //518ac9
    if (!lldriver)
        return linked_hl != nullptr;
    else if (lldriver->listen_socket.is_in_use == 1 || linked_hl != nullptr)
        return 1;
    return 0;
}


NetStru2* NetStru1::FUN_0041f520()
{
    if (active_connects.IsEmpty())
        return nullptr;

    return active_connects.GetHead();
}




int NetStru2::WriteData(void* buf, uint32_t size)
{
    //515ef3
    while(true)
    {
        int32_t avail = 0x8e - out_buffers[out_buff_id].datasz;
        if (size <= avail)
            break;

        out_buffers[out_buff_id].WriteToBuffer(buf, avail);
        SendData();

        buf = (char*)buf + avail;
        size -= avail;
    }
    
    out_buffers[out_buff_id].WriteToBuffer(buf, size);
    return 1;
}

int NetStru2::ReadData(void* buf, uint32_t size)
{
    //515f9c
    EnterCriticalSection(&critical_section);
    while (!received_buffers.IsEmpty())
    {
        NetStru3* buffer = received_buffers.GetHead();
        int32_t remain = buffer->datasz - buffer->readpos;
        
        if (size <= remain)
        {
            buffer->ReadFromBuffer(buf, size);
            LeaveCriticalSection(&critical_section);
            return 1;
        }

        buffer->ReadFromBuffer(buf, remain);

        size -= remain;
        buf = (char*)buf + remain;
        
        received_buffers.RemoveHead();
        hldriver->AddTailFreeNet3(buffer);
    }
    LeaveCriticalSection(&critical_section);
    return 0;
}



void NetStru2::SetDrivers(NetStru1* HlDriver, CLlDriver* LlDriver)
{
    //51682d
    hldriver = HlDriver;
    lldriver = LlDriver;
}

int NetStru2::ReceiveData(NetStru3* buffer)
{
    //5161bf
    EnterCriticalSection(&critical_section);

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

    received_buffers.AddTail(buffer);

    LeaveCriticalSection(&critical_section);
    return 1;
}

int NetStru2::SendData()
{
    //518408
    if (out_buffers[out_buff_id].datasz < 1)
    {
        if (lldriver)
            lldriver->CleanupInvalidTcpClient(uid);
        return 1;
    }

    NetStru3* buffer = hldriver->GetFreeNet3();
    buffer->Clear();
    buffer->cmode = compression_type;

    //here must be compression_type check and pack, but we just drop it

    buffer->CopyData(out_buffers + out_buff_id);
    buffer->pkt_num = out_buffers[out_buff_id].pkt_num;

    // inline FUN_00515eb2();
    out_buff_id = (out_buff_id + 1) & 1;
    out_buffers[out_buff_id].Clear();

    if (is_local_player == 0)
    {
        if (!lldriver)
        {
            hldriver->AddTailFreeNet3(buffer);
            return 0;
        }

        lldriver->SendData(uid, buffer);
    }
    else
    {
        if (!hldriver)
        {
            hldriver->AddTailFreeNet3(buffer);
            return 0;
        }

        NetStru2* recv_cli = nullptr;
        if (hldriver->linked_hl)
            recv_cli = hldriver->linked_hl->local_client;
        
        if (!recv_cli)
        {
            hldriver->AddTailFreeNet3(buffer);
            return 0;
        }

        recv_cli->ReceiveData(buffer);
    }
    return 1;
}

NetStru2::NetStru2()
{
    //515831
    field_0x2a0 = 0;
    field_0x2a4 = 0;
    field_0x2a8 = 0;
    field_0x2ac = 0;
    field_0x2b4 = -1;
    str.Empty();
    field_0x2b8 = 0;
    hldriver = nullptr;
    lldriver = nullptr;
    out_buff_id = 0;
    out_buffers[0].Clear();
    out_buffers[1].Clear();
    compression_type = 0;
    player_id = 0;
    uid = 0;
    is_local_player = 0;
    buf[0] = 0;
    InitializeCriticalSection(&critical_section);
}

NetStru2::~NetStru2()
{
    //515c27
    ReturnBuffers();
    DeleteCriticalSection(&critical_section);
    is_local_player = 0;
    player_id = 0;
    uid = 0;
    hldriver = nullptr;
    lldriver = nullptr;
    buf[0] = 0;
}

void NetStru2::ReturnBuffers()
{
    //515d9d
    if (!hldriver)
        return;

    EnterCriticalSection(&critical_section);

    while (!received_buffers.IsEmpty())
        hldriver->AddTailFreeNet3( received_buffers.RemoveHead() );

    LeaveCriticalSection(&critical_section);
}


void NetStru2::sub_5167A5()
{
    //5167A5
    EnterCriticalSection(&critical_section);
    for (POSITION pos = received_buffers.GetHeadPosition(); pos != nullptr;)
    {
        NetStru3* buffer = received_buffers.GetNext(pos);
        if (buffer->pkt_num != 0)
        {
            buffer->pkt_num--;
            break;
        }
    }
    LeaveCriticalSection(&critical_section);
}

uint32_t NetStru2::GetBuffersPktNum()
{
    //51670a  Get received_buffers summary of pkt_num
    uint32_t sum = 0;
    EnterCriticalSection(&critical_section);
    for (POSITION pos = received_buffers.GetHeadPosition(); pos != nullptr;)
    {
       sum += received_buffers.GetNext(pos)->pkt_num;        
    }
    LeaveCriticalSection(&critical_section);
    return sum;
}




void NetStru3::Clear()
{
    //5156f6
    datasz = 0;
    readpos = 0;
    pkt_num = 0;
    timestamp = 0;
    timestamp2 = 0;
    csize = 0;
}

NetStru3::NetStru3()
{
    //5155c0
    memset(full_data, 0, sizeof(full_data));
    datasz = -1;
    readpos = -1;
    timestamp = 0;
    timestamp2 = 0;
}

NetStru3::~NetStru3()
{
    //515694
    datasz = -1;
    readpos = -1;
    timestamp = 0;
    timestamp2 = 0;
    memset(full_data, 0, sizeof(full_data));
}

void NetStru3::CopyData(const NetStru3* src)
{
    //515625
    memcpy(full_data, &src->full_data, src->datasz + 8);
    datasz = src->datasz;
    readpos = src->readpos;
    timestamp = src->timestamp;
    timestamp2 = src->timestamp2;
}

int NetStru3::WriteToBuffer(void* data, uint32_t sz)
{
    //51573e
    uint32_t newsz = datasz + sz;
    if (newsz >= 143)
        return false;

    memcpy(buf + datasz, data, sz);
    datasz = newsz;
    return true;
}

int NetStru3::ReadFromBuffer(void* out, uint32_t sz)
{
    //51579e
    uint32_t newpos = readpos + sz;
    if (newpos > datasz)
        return false;

    memcpy(out, buf + readpos, sz);
    readpos = newpos;
    return true;
}


A2NetSock::A2NetSock()
{
    //520510
    is_in_use = 0;
    uid = 0;
    socket = INVALID_SOCKET;
    player_dpid = 0;
    manager = nullptr;
    field_0x50 = 0;
    field_0x4c = 0;
    current_buffer = nullptr;
    copy_num = -1;
    wait_obj = nullptr;
    latency = 0;
    field_0x260 = 0;
    field_0x264 = 0;
}

A2NetSock::~A2NetSock()
{
    //520684
    is_in_use = 0;
    uid = 0;
    socket = INVALID_SOCKET;
    wait_obj = nullptr;
    player_dpid = 0;
    manager = nullptr;
    field_0x50 = 0;
    field_0x4c = 0;
    current_buffer = nullptr;
    copy_num = -1;
    latency = 0;
    field_0x260 = 0;
    field_0x264 = 0;
}

void NetSockLatency::AddLatency(uint32_t lat_time)
{
    //51fdf7
    static std::array<uint32_t, 128> storage;

    lat_times[num] = lat_time;
    num++;

    std::array<uint32_t, 256> count1;
    std::array<uint32_t*, 256> arrPos1;
    std::array<uint32_t, 256> count2;
    std::array<uint32_t*, 256> arrPos2;
    std::array<uint32_t, 256> count3;
    std::array<uint32_t*, 256> arrPos3;
    std::array<uint32_t, 256> count4;
    std::array<uint32_t*, 256> arrPos4;

    if (num >= 128)
    {
        num = 0;

        count1.fill(0);
        for (int i = 0; i < 128; i++)
            count1[ShiftToByte(lat_times[i], 0)]++;

        uint32_t* ppos = storage.data();
        for (int i = 0; i < 256; i++)
        {
            arrPos1[i] = ppos;
            ppos += count1[i];
        }

        for (int i = 0; i < 128; i++)
        {
            int idx = ShiftToByte(lat_times[i], 0);
            *arrPos1[idx] = lat_times[i];
            arrPos1[idx]++;
        }


        count2.fill(0);
        for (int i = 0; i < 128; i++)
            count2[ShiftToByte(storage[i], 8)]++;

        ppos = lat_times;
        for (int i = 0; i < 256; i++)
        {
            arrPos2[i] = ppos;
            ppos += count2[i];
        }

        for (int i = 0; i < 128; i++)
        {
            int idx = ShiftToByte(storage[i], 8);
            *arrPos2[idx] = storage[i];
            arrPos2[idx]++;
        }


        count3.fill(0);
        for (int i = 0; i < 128; i++)
            count3[ShiftToByte(lat_times[i], 16)]++;

        ppos = storage.data();
        for (int i = 0; i < 256; i++)
        {
            arrPos3[i] = ppos;
            ppos += count3[i];
        }

        for (int i = 0; i < 128; i++)
        {
            int idx = ShiftToByte(lat_times[i], 16);
            *arrPos3[idx] = lat_times[i];
            arrPos3[idx]++;
        }


        count4.fill(0);
        for (int i = 0; i < 128; i++)
            count4[ShiftToByte(storage[i], 24)]++;

        ppos = lat_times;
        for (int i = 0; i < 256; i++)
        {
            arrPos4[i] = ppos;
            ppos += count4[i];
        }

        for (int i = 0; i < 128; i++)
        {
            int idx = ShiftToByte(storage[i], 24);
            *arrPos4[idx] = storage[i];
            arrPos4[idx]++;
        }

        calc_latency = lat_times[64];
    }
    if (calc_latency < 25)
        calc_latency = 25;
    if (calc_latency > 1000)
        calc_latency = 1000;
}




CLlDriver::CLlDriver(GUID _appid, NetStru1* net1)
{
    //5208ec
    application_guid = _appid;
    hl_driver = net1;
    is_server = 0;
    guaranteed = 0;
    latency = 0xffffffff;
    max_connections = -1;
    session_lost = 0;
    provider = 5;
    connection_sockets = nullptr;
    connections_info = nullptr;
    enum_addresses = nullptr;
    enum_addresses_num = 0;
    enum_sessions = nullptr;
    enum_sessions_num = 0;
    enum_conns = nullptr;
    enum_conns_num = 0;
    enum_players = nullptr;
    enum_players_num = 0;
    address_str[0] = 0;
    next_uid = 0x3bef0000;
    unused = 0;
    keepalive = 0;
    dplay_is_4 = 0;
    timeout = 12000;
    InitializeCriticalSection(&critical_section);
    ev_create_player = nullptr;
    ev_close = nullptr;
    ev_new_session = nullptr;
    dplay4 = nullptr;
    CoInitialize(NULL);
}

CLlDriver::~CLlDriver()
{
    //520ad3

    Close();
    FreeEnumAddresses();
    FreeEnumSessions();
    FreeEnumConns();
    FreeEnumPlayers();

    if (connection_sockets)
    {
        delete[] connection_sockets;
        connection_sockets = nullptr;
    }

    if (connections_info)
    {
        delete[] connections_info;
        connections_info = nullptr;
    }

    DeleteCriticalSection(&critical_section);

    Free();

    CoUninitialize();
}

void CLlDriver::SetHlDriver(NetStru1* drv)
{
    //496e50
    hl_driver = drv;
}


void CLlDriver::Close()
{
    //5225d0
    if (listen_socket.is_in_use)
    {
        if (provider == 4)
            CloseTcp();
        else
            CloseDp();
    }
    listen_socket.is_in_use = 0;
    is_server = 0;
}



void CLlDriver::CloseTcp()
{
    //523b01
    CloseTcpSocket(&listen_socket);
    CloseTcpSocket(&ping_socket);
    if (is_server)
    {
        for (uint32_t i = 0; i < max_connections; i++)
            CloseTcpSocket(connection_sockets + i);
    }
}

void CLlDriver::CloseTcpSocket(A2NetSock* sock)
{
    //523a16
    if (sock->is_in_use)
        num_connections--;

    sock->is_in_use = 0;
    if (sock->socket != INVALID_SOCKET)
    {
        closesocket(sock->socket);
        sock->socket = INVALID_SOCKET;
    }

    if (sock->wait_obj)
    {
        WaitForSingleObject(sock->wait_obj, -1);
        CloseHandle(sock->wait_obj);
        sock->wait_obj = nullptr;
    }

    if (sock->current_buffer)
    {
        hl_driver->AddTailFreeNet3(sock->current_buffer);
        sock->current_buffer = nullptr;
        sock->copy_num = -1;
    }

    if (sock->manager)
    {
        hl_driver->AddTailNet2(sock->manager);
        sock->manager = nullptr;
    }
}


void CLlDriver::CloseDp()
{
    //525a09
    if (listen_socket.wait_obj)
    {
        SetEvent(ev_close);
        WaitForSingleObject(listen_socket.wait_obj, -1);
        CloseHandle(listen_socket.wait_obj);
        listen_socket.wait_obj = nullptr;
    }

    if (is_server)
    {
        for (uint32_t i = 0; i < max_connections; i++)
            CloseDpSock(connection_sockets + i, -1);
    }
    else
    {
        listen_socket.is_in_use = 0;
        if (listen_socket.manager)
        {
            hl_driver->AddTailNet2(listen_socket.manager);
            listen_socket.manager = nullptr;
        }
    }

    if (ev_create_player)
    {
        CloseHandle(ev_create_player);
        ev_create_player = nullptr;
    }

    if (ev_close)
    {
        CloseHandle(ev_close);
        ev_close = nullptr;
    }

    if (dplay4)
    {
        if (listen_socket.player_dpid != DPID_UNKNOWN)
        {
            dplay4->DestroyPlayer(listen_socket.player_dpid);
            listen_socket.player_dpid = DPID_UNKNOWN;
        }
        dplay4->Close();
    }
}

void CLlDriver::CloseDpSock(A2NetSock* sock, int idx)
{
    //525bcd
    if (IsInUse(sock) == 0)
        return;

    if (is_server == 0)
    {
        Close();
        return;
    }

    if (dplay_is_4)
        dplay4->DestroyPlayer(sock->player_dpid);

    if (sock->manager)
    {
        hl_driver->AddTailNet2(sock->manager);
        sock->manager = nullptr;
    }

    sock->is_in_use = 0;

    while (!sock->list_0x14.IsEmpty())
        hl_driver->AddTailFreeNet3( sock->list_0x14.RemoveHead() );
    sock->list_0x14.RemoveAll();

    while (!sock->list_0x30.IsEmpty())
        hl_driver->AddTailFreeNet3(sock->list_0x30.RemoveHead());
    sock->list_0x30.RemoveAll();

    if (idx == -1)
    {
        for (uint32_t i = 0; i < num_connections; i++)
        {
            if (connections_info[i].sock == sock)
            {
                idx = i;
                break;
            }
        }
    }

    memmove(connections_info + idx, connections_info + idx + 1, (num_connections - idx - 1) * sizeof(SocketNm));
}


CLlAddress* CLlDriver::AllocAddress()
{
    //520d8c
    if (enum_addresses == nullptr)
    {
        enum_addresses = (CLlAddress*)malloc(sizeof(CLlAddress));
        enum_addresses_num = 1;
        return enum_addresses;
    }
    else
    {
        enum_addresses_num++;
        enum_addresses = (CLlAddress*)realloc(enum_addresses, sizeof(CLlAddress) * enum_addresses_num);
        return enum_addresses + (enum_addresses_num - 1);
    }
}

void CLlDriver::FreeEnumAddresses()
{
    //520c80
    if (enum_addresses)
    {
        free(enum_addresses);
        enum_addresses = nullptr;
        enum_addresses_num = 0;
    }
}


int CLlDriver::EnumAddresses(CLlAddress** addrs, int* num)
{
    //52182f
    FreeEnumAddresses();
    if (provider == 0)
    {
        CLlAddress* addr = AllocAddress();
        strcpy(addr->name, "COM1");
        addr->com.index = 1;
        addr = AllocAddress();
        strcpy(addr->name, "COM2");
        addr->com.index = 2;
        addr = AllocAddress();
        strcpy(addr->name, "COM3");
        addr->com.index = 3;
        addr = AllocAddress();
        strcpy(addr->name, "COM4");
        addr->com.index = 4;
    }
    else if (provider == 1)
    {
        EnumAddressesDp();
    }
    else if (provider == 4)
    {
        EnumAddressesTcp();
    }
    else
        return 0;

    *addrs = enum_addresses;
    *num = enum_addresses_num;
    return 1;
}


void CLlDriver::EnumAddressesTcp()
{
    //5214c5
    OSVERSIONINFOA osver;
    osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    GetVersionExA(&osver);

    char path[260];
    memset(path, 0, sizeof(path));
    uint32_t len = 0;

    if (osver.dwPlatformId == 1)
        len = GetWindowsDirectoryA(path, sizeof(path));
    else
        len = GetSystemDirectoryA(path, sizeof(path));

    if (path[len] != '\\') //??? path[len-1] ???
        strcat(path, "\\");

    if (osver.dwPlatformId == 1)
        strcat(path, "WinIpCfg.exe");
    else
        strcat(path, "IpConfig.exe");

    _spawnlp(P_WAIT, path, path, "/ALL", "/BATCH", "IPINFO.$$$$$$", NULL);


    CLlAddress* addr = nullptr;
    char buf[2048];

    FILE* f = fopen("IPINFO.$$$$$$", "r");
    if (f)
    {
        while (true)
        {
            if (fgets(buf, sizeof(buf), f) == NULL)
                break;

            char* p = strchr(buf, ':');
            if (p)
            {
                *p = 0;
                p++;
                while (isspace(*p))
                    p++;
                for (char* t = p + strlen(p) - 1; t != p && isspace(*t); t--)
                    *t = 0;

                if (strstr(buf, "Description") != NULL)
                {
                    addr = AllocAddress();
                    strcpy(addr->name, p);
                    strcpy(addr->address, "0.0.0.0");
                }
                else if (addr && strstr(buf, "IP Address") != NULL)
                {
                    strcpy(addr->address, p);
                    addr = nullptr;
                }
            }
        }

        fclose(f);
        _unlink("IPINFO.$$$$$$");
    }

    if (enum_addresses_num == 0)
    {
        if (!addr)
            addr = AllocAddress();

        strcpy(addr->name, "Default adapter");
        strcpy(addr->address, "0.0.0.0");
    }
}

void CLlDriver::EnumAddressesDp()
{
    //521200
    IDirectPlayLobby2A* lobby = nullptr;
    if (CoCreateInstance(CLSID_DirectPlayLobby, NULL, 1, IID_IDirectPlayLobby2A, (LPVOID*)&lobby) != 0)
    {
        ReportWarning("CLlDriver::GetPossibleModems.\nUnable to create DirectPlayLobby.\n");
        return;
    }

    IDirectPlay3A* dplay = nullptr;
    if (CoCreateInstance(CLSID_DirectPlay, NULL, 1, IID_IDirectPlayLobby3A, (LPVOID*)&dplay) != 0)
    {
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
    if (lobby->CreateCompoundAddress(elem, 2, NULL, &AddrSize) != DPERR_BUFFERTOOSMALL)
    {
        lobby->Release();
        dplay->Release();
        return;
    }

    uint8_t* dpaddr = new uint8_t[AddrSize];
    if (lobby->CreateCompoundAddress(elem, 2, dpaddr, &AddrSize) != 0)
    {
        delete[] dpaddr;

        lobby->Release();
        dplay->Release();
        return;
    }

    if (dplay->InitializeConnection(dpaddr, 0) != 0)
    {
        delete[] dpaddr;
        ReportWarning("CLlDriver::GetPossibleModems.\nUnable to initialize DirectPlay.\n");
        lobby->Release();
        dplay->Release();
        return;
    }

    delete[] dpaddr;

    if (dplay->GetPlayerAddress(0, NULL, &AddrSize) != DPERR_BUFFERTOOSMALL)
    {
        ReportWarning("CLlDriver::GetPossibleModems.\nUnable to enum modems.\n");
        lobby->Release();
        dplay->Release();
        return;
    }

    dpaddr = new uint8_t[AddrSize];

    if (dplay->GetPlayerAddress(0, dpaddr, &AddrSize) != 0)
    {
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

BOOL __stdcall CLlDriver::cbEnumAddress(REFGUID guidDataType, DWORD dwDataSize, LPCVOID lpData, LPVOID lpContext)
{
    //5211b2
    CLlDriver* drv = (CLlDriver*)lpContext;
    if (memcmp(&guidDataType, &DPSPGUID_MODEM, sizeof(GUID)) == 0)
    {
        CLlAddress* addr = drv->AllocAddress();
        strcpy(addr->name, (const char*)lpData);
    }
    return TRUE;
}




CLlNetSession* CLlDriver::AllocSession()
{
    //520e2e
    if (enum_sessions == nullptr)
    {
        enum_sessions = (CLlNetSession*)malloc(sizeof(CLlNetSession));
        enum_sessions_num = 1;
        return enum_sessions;
    }
    else
    {
        enum_sessions_num++;
        enum_sessions = (CLlNetSession*)realloc(enum_sessions, sizeof(CLlNetSession) * enum_sessions_num);
        return enum_sessions + (enum_sessions_num - 1);
    }
}

void CLlDriver::FreeEnumSessions()
{
    //520cc3
    if (enum_sessions)
    {
        free(enum_sessions);
        enum_sessions = nullptr;
        enum_sessions_num = 0;
    }
}

int CLlDriver::EnumSessions(CLlNetSession** sessions, int* num, CBUPDATECALLBACK cbUpdate, uint32_t timeout)
{
    //52210d
    FreeEnumSessions();

    if (provider == 4)
        return 0;

    DPSESSIONDESC2 desc;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.guidApplication = application_guid;
    ev_new_session = CreateEventA(NULL, FALSE, FALSE, NULL);

    if (timeout == 0)
    {
        DPCAPS caps;
        caps.dwSize = sizeof(caps);
        if (dplay4->GetCaps(&caps, (guaranteed != 0 ? DPGETCAPS_GUARANTEED : 0)) != 0)
        {
            ReportWarning("CLlDriver::GetActiveServers().\nUnable to get caps.\n");
            return 1;
        }
        timeout = caps.dwTimeout;
    }

    uint32_t t1 = GetTickCount();
    while (true)
    {
        uint32_t t2 = GetTickCount();
        if (t2 - t1 >= timeout)
            break;

        HRESULT res = dplay4->EnumSessions(&desc, 0, cbEnumSessions, this, DPENUMSESSIONS_AVAILABLE | DPENUMSESSIONS_ASYNC | DPENUMSESSIONS_RETURNSTATUS);
        if (res != 0 && res != DPERR_CONNECTING)
            break;

        if (WaitForSingleObject(ev_new_session, 10) == 0)
            break;

        if (cbUpdate && cbUpdate() == 0)
        {
            dplay4->EnumSessions(&desc, 0, cbEnumSessions, this, DPENUMSESSIONS_STOPASYNC);
            break;
        }
    }

    CloseHandle(ev_new_session);
    ev_new_session = nullptr;
    
    *sessions = enum_sessions;
    *num = enum_sessions_num;

    return 1;
}

BOOL __stdcall CLlDriver::cbEnumSessions(LPCDPSESSIONDESC2 lpThisSD, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext)
{
    //522020
    CLlDriver* drv = (CLlDriver*)lpContext;

    if ((dwFlags & DPENUMSESSIONS_AVAILABLE) == 0)
        return FALSE;

    if (!lpThisSD)
        return FALSE;

    for (uint32_t i = 0; i < drv->enum_sessions_num; i++)
    {
        if (memcmp(&drv->enum_sessions[i].guid, &lpThisSD->guidInstance, sizeof(GUID)) == 0)
            return TRUE;
    }

    CLlNetSession* ses = drv->AllocSession();
    strcpy(ses->name, lpThisSD->lpszSessionNameA);
    ses->guid = lpThisSD->guidInstance;

    if (drv->provider == 1 || drv->provider == 0)
        drv->SetEventNewSession();

    return TRUE;
}






CLlConn* CLlDriver::AllocEnumConns()
{
    //520ed0
    if (enum_conns == nullptr)
    {
        enum_conns = (CLlConn*)malloc(sizeof(CLlConn));
        enum_conns_num = 1;
        return enum_conns;
    }
    else
    {
        enum_conns_num++;
        enum_conns = (CLlConn*)realloc(enum_conns, sizeof(CLlConn) * enum_conns_num);
        return enum_conns + (enum_conns_num - 1);
    }
}

void CLlDriver::FreeEnumConns()
{
    //520d06
    if (enum_conns)
    {
        free(enum_conns);
        enum_conns = nullptr;
        enum_conns_num = 0;
    }
}

void CLlDriver::EnumConnections(CLlConn** conns, int* num)
{
    //5210ea
    FreeEnumConns();

    CLlConn* conn = AllocEnumConns();
    strcpy(conn->name, "TCP/IP");
    conn->typ = 4;

    bool hasntdp = dplay4 == nullptr;

    if (hasntdp)
        CreateDp();

    if (dplay4)
        dplay4->EnumConnections(&application_guid, cbEnumConnections, this, DPCONNECTION_DIRECTPLAY);

    if (hasntdp)
        FreeDp();

    *conns = enum_conns;
    *num = enum_conns_num;
}

BOOL __stdcall CLlDriver::cbEnumConnections(LPCGUID lpguidSP, LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext)
{
    //52100e
    CLlDriver* drv = (CLlDriver*)lpContext;

    int typ = -1;
    if (memcmp(lpguidSP, &DPSPGUID_MODEM, sizeof(GUID)) == 0)
        typ = 1;
    else if (memcmp(lpguidSP, &DPSPGUID_TCPIP, sizeof(GUID)) == 0)
        typ = 3;
    else if (memcmp(lpguidSP, &DPSPGUID_IPX, sizeof(GUID)) == 0)
        typ = 2;
    else if (memcmp(lpguidSP, &DPSPGUID_SERIAL, sizeof(GUID)) == 0)
        typ = 0;
    else
        return TRUE;

    CLlConn* conn = drv->AllocEnumConns();
    strcpy(conn->name, lpName->lpszShortNameA);
    conn->typ = typ;
    return TRUE;
}



CLlName* CLlDriver::AllocEnumPlayers()
{
    //520f72
    if (enum_players == nullptr)
    {
        enum_players = (CLlName*)malloc(sizeof(CLlName));
        enum_players_num = 1;
        return enum_players;
    }
    else
    {
        enum_players_num++;
        enum_players = (CLlName*)realloc(enum_players, sizeof(CLlName) * enum_players_num);
        return enum_players + (enum_players_num - 1);
    }
}

void CLlDriver::FreeEnumPlayers()
{
    //520d49
    if (enum_players)
    {
        free(enum_players);
        enum_players = nullptr;
        enum_players_num = 0;
    }
}

void CLlDriver::EnumPlayers(CLlNetSession* ses, CLlName** names, int* num)
{
    //52238e
    FreeEnumPlayers();
    if (dplay4)
        dplay4->EnumPlayers(&ses->guid, cbEnumPlayes, this, DPENUMPLAYERS_SESSION);
    *names = enum_players;
    *num = enum_players_num;
}

BOOL __stdcall CLlDriver::cbEnumPlayes(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext)
{
    //52233f
    CLlDriver* drv = (CLlDriver*)lpContext;

    if (!lpName->lpszShortNameA)
        return FALSE;

    if (dwPlayerType != DPPLAYERTYPE_PLAYER)
        return TRUE;

    CLlName *pl = drv->AllocEnumPlayers();
    strcpy(pl->name, lpName->lpszShortNameA);
    return TRUE;
}


void CLlDriver::Free()
{
    //5219af
    if (listen_socket.is_in_use == 1)
        Close();

    if (provider == 5)
    {
        if (provider == 4)
            FreeTcp();
        else
            FreeDp();
        provider = 5;
    }
}

void CLlDriver::FreeTcp()
{
    //523611
    WSACleanup();
}

void CLlDriver::FreeDp()
{
    //523d5a
    if (dplay4)
    {
        dplay4->Release();
        dplay4 = nullptr;
    }
}





int CLlDriver::ResetProvider(int prov)
{
    //52194f
    Free();

    provider = prov;
    if (prov == 4)
    {
        if (!CreateTcp())
            return 0;
    }
    else
    {
        if (!CreateDp())
            return 0;
    }
    provider = prov;
    return 1;
}


int CLlDriver::CreateTcp()
{
    //52359b
    WSADATA wsa;
    if (WSAStartup(0x101, &wsa) != 0)
        return 0;

    if (wsa.wVersion == 0x101)
        return 1;

    WSACleanup();
    return 0;
}

int CLlDriver::CreateDp()
{
    //523ccf
    if (dplay4)
        return 1;

    dplay_is_4 = 1;
    if (CoCreateInstance(CLSID_DirectPlay, NULL, 1, IID_IDirectPlay4A, (LPVOID*)&dplay4) == 0)
        return 1;

    dplay_is_4 = 0;
    if (CoCreateInstance(CLSID_DirectPlay, NULL, 1, IID_IDirectPlay3A, (LPVOID*)&dplay3) == 0)
        return 1;

    dplay4 = nullptr;
    return 0;
}


A2NetSock* CLlDriver::GetClientBySocketId(uint32_t uid)
{
    //5224c4
    if (!is_server)
        return &listen_socket;
    
    A2NetSock* sock = connection_sockets + (uid & 0xFFFF);
    if (sock->uid != uid)
    {
        ReportWarning("CLlDriver::GetClientBySocketId().\nConnection with client lost.\n");
        return nullptr;
    }
    return sock;
}


void CLlDriver::SetEventNewSession()
{
    //522318
    if (ev_new_session)
        SetEvent(ev_new_session);
}

int CLlDriver::IsInUse(A2NetSock* sock)
{
    //52293a
    if (listen_socket.is_in_use == 1)
        return sock->is_in_use;
    else
        return 0;
}



int CLlDriver::MakeSockAddr(const char* address, sockaddr_in* out, int port, int resolve)
{
    //52343e
    char buf[1024];
    strcpy(buf, address);

    if (buf[0] == 0)
    {
        out->sin_addr.S_un.S_addr = 0;
        out->sin_port = 0;
        return 0;
    }

    out->sin_port = htons(port);

    for (char* p = buf + strlen(buf) - 1; p != buf; p--)
    {
        if (*p == ':')
        {
            *p = 0;
            out->sin_port = htons(atoi(p + 1) + (port == 8001));
            break;
        }
    }

    out->sin_family = AF_INET;
    out->sin_addr.S_un.S_addr = inet_addr(buf);

    if (out->sin_addr.S_un.S_addr == INADDR_NONE && resolve != 0)
    {
        hostent *host = gethostbyname(buf);
        if (host == nullptr)
        {
            out->sin_addr.S_un.S_addr = 0;
            out->sin_port = 0;
            return 0;
        }
        out->sin_addr.S_un.S_addr = *(uint32_t *)(host->h_addr_list);
    }
    memset(out->sin_zero, 0, sizeof(out->sin_zero));
    return 1;
}



int CLlDriver::StartServerTcp()
{
    //523621
    listen_socket.socket = INVALID_SOCKET;
    listen_socket.wait_obj = nullptr;
    num_connections = 0;

    listen_socket.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket.socket == INVALID_SOCKET)
    {
        CloseTcp();
        return 0;
    }
    u_long nonblock = 1;
    ioctlsocket(listen_socket.socket, FIONBIO, &nonblock);

    sockaddr addr;
    if (MakeSockAddr(server_start_addr->address, (sockaddr_in*)&addr, 8000, 0) == 0)
    {
        CloseTcp();
        return 0;
    }

    if (bind(listen_socket.socket, &addr, sizeof(addr)) != 0)
    {
        CloseTcp();
        return 0;
    }

    if (listen(listen_socket.socket, max_connections) != 0)
    {
        CloseTcp();
        return 0;
    }

    listen_socket.wait_obj = CreateEventA(NULL, 0, 0, NULL);

    if (_beginthread(AcceptThreadTcp, 0, this) == -1L || listen_socket.wait_obj == nullptr)
    {
        CloseHandle(listen_socket.wait_obj);
        listen_socket.wait_obj = nullptr;
        CloseTcp();
        return 0;
    }
    
    guaranteed = 1;

    return 1;
}


void __cdecl CLlDriver::AcceptThreadTcp(void* context)
{
    //522e8f
    CLlDriver* drv = (CLlDriver*)context;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    while (true)
    {
        if (drv->listen_socket.is_in_use != 1)
        {
            SetEvent(drv->listen_socket.wait_obj);
            _endthread();
            return;
        }

        fd_set fd;
        FD_ZERO(&fd);
        FD_SET(drv->listen_socket.socket, &fd);

        if (drv->is_server)
        {
            for (int i = 0; i < drv->max_connections; i++)
            {
                A2NetSock* sock = drv->connection_sockets + i;
                if (sock->is_in_use == 1)
                {
                    FD_SET(sock->socket, &fd);
                }
            }
        }

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 250000;

        int numfd = select(0, &fd, NULL, NULL, &timeout);

        if (numfd == 0)
            continue;

        if (numfd == SOCKET_ERROR)
        {
            if (drv->listen_socket.is_in_use == 1)
                ReportWarning("CLlDriverAcceptThreadTcp().\nSelect error.\n");
            continue;
        }

        if (drv->is_server == 0)
        {
            if (drv->listen_socket.is_in_use == 1 &&
                FD_ISSET(drv->listen_socket.socket, &fd) &&
                drv->RecvThreadTcp(&drv->listen_socket) == 0)
            {
                closesocket(drv->listen_socket.socket);
                drv->listen_socket.socket = INVALID_SOCKET;
            }
            continue;
        }

        if (FD_ISSET(drv->listen_socket.socket, &fd))
        {
            sockaddr_in newcon;
            int newconln = sizeof(newcon);
            SOCKET nsock = accept(drv->listen_socket.socket, (sockaddr*)&newcon, &newconln);
            if (nsock == INVALID_SOCKET)
            {
                if (drv->listen_socket.is_in_use == 1)
                    ReportWarning("CLlDriverAcceptThreadTcp().\nUnable to accept new connection.\n");
                continue;
            }
            
            if (drv->num_connections >= drv->max_connections)
            {
                closesocket(nsock);
                ReportWarning("CLlDriverAcceptThreadTcp().\nClient connection refused.\n");
                continue;
            }

            SetSockOptions(nsock, 1);

            EnterCriticalSection(&drv->critical_section);

            int32_t idx = -1;
            for (int i = 0; i < drv->max_connections; i++)
            {
                if (!drv->IsInUse(drv->connection_sockets + i))
                {
                    idx = i;
                    break;
                }
            }

            if (idx == -1)
            {
                closesocket(nsock);
                ReportWarning("CLlDriverAcceptThreadTcp().\n No free slot.\n");
            }
            else
            {
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

        for (uint32_t i = 0; i < drv->max_connections; i++)
        {
            A2NetSock* conn = drv->connection_sockets + i;
            if (conn->is_in_use == 1 &&
                FD_ISSET(conn->socket, &fd) &&
                drv->RecvThreadTcp(conn) == 0)
            {
                closesocket(conn->socket);
                conn->socket = INVALID_SOCKET;
            }
        }

    }
}

void __cdecl CLlDriver::SetSockOptions(SOCKET s, int enlinger)
{
    //522af4
    int32_t flag = 1;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, 4);
    int32_t bufsz = 0x8000;
    setsockopt(s, SOL_SOCKET, SO_SNDBUF, (const char*)&bufsz, 4);
    setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char*)&bufsz, 4);

    if (enlinger)
    {
        linger lngr;
        lngr.l_linger = 0;
        lngr.l_onoff = 1;
        setsockopt(s, SOL_SOCKET, SO_LINGER, (const char*)&lngr, sizeof(linger));
    }
    u_long nonblock = 1;
    ioctlsocket(s, FIONBIO, &nonblock);
}



int CLlDriver::RecvThreadTcp(A2NetSock* sock)
{
    //522b95
    uint8_t databuf[0x2000];
    int rsz = recv(sock->socket, (char *)databuf, 0x2000, 0);
    if (rsz == 0 || rsz == -1)
        return 0;

    uint8_t *pos = databuf;
    while (true)
    {
        if (rsz == 0)
            return 1;

        if (sock->copy_num < 8)
        {
            if (sock->current_buffer == nullptr)
            {
                sock->copy_num = 0;
                sock->current_buffer = hl_driver->GetFreeNet3();
                sock->current_buffer->Clear();
            }
            int32_t copynum = 8 - sock->copy_num;
            if (rsz < copynum)
                copynum = rsz;
            
            memcpy(sock->current_buffer->buf + sock->copy_num - 8, pos, copynum);
            pos += copynum;
            rsz -= copynum;
            sock->copy_num += copynum;

            if (sock->copy_num >= 8 && (sock->current_buffer->pos == 0 || sock->current_buffer->pos >= 143))
            {
                ReportWarning("CLlDriverRecvThreadTcp().\nReceived invalid message.\n");
                hl_driver->AddTailFreeNet3(sock->current_buffer);
                sock->copy_num = 0;
                sock->current_buffer = nullptr;
                return 0;
            }
        }
        else
        {
            int32_t cpysz = (sock->current_buffer->pos + 8) - sock->copy_num;
            int32_t cpynum = cpysz;
            if (cpynum > rsz)
                cpynum = rsz;
            
            memcpy(sock->current_buffer->buf + sock->copy_num - 8, pos, cpynum);
            pos += cpynum;
            rsz -= cpynum;
            sock->copy_num += cpynum;

            if (cpysz == cpynum)
            {
                sock->current_buffer->datasz = sock->current_buffer->pos;
                XorData(sock->current_buffer->buf, sock->current_buffer->datasz);
                sock->manager->ReceiveData(sock->current_buffer);
                sock->copy_num = 0;
                sock->current_buffer = nullptr;
            }
        }
    }
}




int CLlDriver::PrepareAddressDp(CLlAddress* addr)
{
    //523d96
    IDirectPlayLobby2A* lobby = nullptr;
    if (CoCreateInstance(CLSID_DirectPlayLobby, NULL, 1, IID_IDirectPlayLobby2A, (LPVOID *) &lobby) != DP_OK)
        return 0;

    DPCOMPOUNDADDRESSELEMENT elem[3];
    int numelem = 0;

    char addrbuf[100];

    switch (provider)
    {
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
        elem[2].dwDataSize = strlen(address_str) + 1;
        elem[2].lpData = address_str;
        numelem = 3;
        break;
    case 2:
        elem[0].guidDataType = DPAID_ServiceProvider;
        elem[0].dwDataSize = sizeof(GUID);
        elem[0].lpData = (LPVOID)&DPSPGUID_IPX;
        numelem = 1;
        break;
    case 3:
        strcpy(addrbuf, address_str);
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
    if (lobby->CreateCompoundAddress(elem, numelem, nullptr, &sz) != DPERR_BUFFERTOOSMALL)
    {
        lobby->Release();
        return 0;
    }

    uint8_t *dat = new uint8_t[sz];
    if (lobby->CreateCompoundAddress(elem, numelem, dat, &sz) != DP_OK)
    {
        delete[] dat;
        lobby->Release();
        return 0;
    }

    HRESULT res = dplay4->InitializeConnection(dat, 0);
    if (res == DPERR_ALREADYINITIALIZED)
    {
        FreeDp();
        CreateDp();
        if (dplay4)
            res = dplay4->InitializeConnection(dat, 0);
    }

    delete[] dat;
    lobby->Release();
    return res == DP_OK;
}

int CLlDriver::StartServerDp()
{
    //5241ea
    ev_close = nullptr;
    ev_create_player = nullptr;
    listen_socket.player_dpid = -1;

    if (PrepareAddressDp(server_start_addr) == 0)
    {
        ReportWarning("CLlDriver::StartServerDP.\nUnable to initialize DirectPlay connection.\n");
        CloseDp();
        return 0;
    }

    DPCAPS caps;
    caps.dwSize = sizeof(DPCAPS);
    if (dplay4->GetCaps(&caps, DPGETCAPS_GUARANTEED) != DP_OK)
    {
        ReportWarning("CLlDriver::StartServerDp.\nUnable to get caps.\n");
        return 0;
    }

    if (caps.dwMaxPlayers < num_connections)
    {
        ReportWarning("CLlDriver::StartServerDp().\nUnsupported number of connections.\n");
        num_connections = caps.dwMaxPlayers;
    }

    if ((caps.dwFlags & (DPCAPS_GUARANTEEDSUPPORTED | DPCAPS_GUARANTEEDOPTIMIZED)) == (DPCAPS_GUARANTEEDSUPPORTED | DPCAPS_GUARANTEEDOPTIMIZED) )
        guaranteed = 1;
    else
        guaranteed = 0;
    
    if (provider == 1)
    {
        latency = 1000;
        if (timeout < 16000)
            timeout = 16000;
    }
    else if (provider == 3)
    {
        guaranteed = 0;
        latency = 500;
        if (timeout < 16000)
            timeout = 16000;
    }
    else
    {
        latency = caps.dwLatency;
    }

    ev_create_player = CreateEventA(NULL, 0, 0, NULL);
    ev_close = CreateEventA(NULL, 0, 0, NULL);

    if (ev_create_player == nullptr || ev_close == nullptr)
    {
        ReportWarning("CLlDriver::StartServerDp().\nUnable to create events.\n");
        CloseDp();
        return 0;
    }

    DPSESSIONDESC2 session;
    memset(&session, 0, sizeof(DPSESSIONDESC2));
    session.dwSize = sizeof(DPSESSIONDESC2);
    session.dwFlags = DPSESSION_NODATAMESSAGES;
    if (keepalive || guaranteed)
        session.dwFlags |= DPSESSION_KEEPALIVE;

    session.guidApplication = application_guid;
    session.dwMaxPlayers = max_connections;
    session.lpszSessionNameA = comp_name;
    session.lpszPasswordA = (char*)"";

    if (dplay4->Open(&session, DPOPEN_CREATE) != DP_OK)
    {
        ReportWarning("CLlDriver::StartServerDp().\nUnable to create DirectPlay session.\n");
        CloseDp();
        return 0;
    }

    DPNAME pname;
    memset(&pname, 0, sizeof(DPNAME));
    pname.lpszShortNameA = comp_name;
    pname.lpszLongNameA = comp_name;

    if (dplay4->CreatePlayer((LPDPID)&listen_socket.player_dpid, &pname, ev_create_player, nullptr, 0, DPPLAYER_SERVERPLAYER) != DP_OK)
    {
        ReportWarning("CLlDriver::StartServerDp().\nUnable to create DirectPlay server player.\n");
        CloseDp();
        return 0;
    }

    listen_socket.field_0x50 = 0;
    listen_socket.field_0x4c = 0;
    listen_socket.wait_obj = CreateEventA(NULL, 0, 0, NULL);
    if (_beginthread(RecvThreadDp, 0, this) == -1L || listen_socket.wait_obj == nullptr)
    {
        CloseDp();
        return 0;
    }

    return 1;
}






void __cdecl CLlDriver::RecvThreadDp(void* context)
{
    //523b7a
    CLlDriver* drv = (CLlDriver*)context;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    HANDLE handles[2] = { drv->ev_close, drv->ev_create_player };

    void* mem = malloc(0x96);
    DWORD memsz = 0x96;
    while (true)
    {
        if (WaitForMultipleObjects(2, handles, 0, -1) != 1)
        {
            free(mem);
            SetEvent(drv->listen_socket.wait_obj);
            _endthread();
            return;
        }
        EnterCriticalSection(&drv->critical_section);
        while (true)
        {
            DPID from;
            DPID to;
            HRESULT res = drv->dplay4->Receive(&from, &to, DPRECEIVE_ALL, mem, &memsz);
            if (res == DPERR_BUFFERTOOSMALL)
            {
                free(mem);
                mem = malloc(memsz);
            }
            else if (res == DPERR_NOMESSAGES)
                break;
            else if (res == DP_OK)
                drv->HandleMessageDp(from, to, mem, memsz);
            else
                ReportWarning("CLlDriverRecvThreadDp().\nError during receiving messages.\n");
        }
        LeaveCriticalSection(&drv->critical_section);
    }
}

void CLlDriver::HandleMessageDp(uint32_t from, uint32_t to, void* data, uint32_t datasz)
{
    //524abd
    if (from == 0)
    {
        int dwType = ((DPMSG_GENERIC*)data)->dwType;
        if (dwType == DPSYS_CREATEPLAYERORGROUP)
        {
            DPMSG_CREATEPLAYERORGROUP* createmsg = (DPMSG_CREATEPLAYERORGROUP*)data;
            if (is_server != 0 && createmsg->dwPlayerType == DPPLAYERTYPE_PLAYER)
            {
                int32_t idx = 0;
                while (idx < max_connections)
                {
                    if (!IsInUse(connection_sockets + idx))
                        break;
                    idx++;
                }

                int infid;
                FindDpSock(createmsg->dpId, &infid);
                memmove(connections_info + infid + 1, connections_info + infid, (num_connections - infid) * sizeof(SocketNm)); //make hole for insert
                
                SocketNm* inf = connections_info + infid;
                A2NetSock* sock = connection_sockets + idx;
                inf->dpid = createmsg->dpId;
                inf->sock = sock;
                sock->field_0x50 = 0;
                sock->field_0x4c = 0;
                sock->uid = next_uid | idx;

                next_uid += 0x10000;

                sock->player_dpid = createmsg->dpId;
                sock->is_in_use = 1;
                sock->latency_check.num = 0;
                sock->latency_check.calc_latency = sock->latency;
                sock->latency = 0;
                sock->field_0x260 = 0;
                sock->field_0x264 = 0;
                sock->manager = hl_driver->AllocClientBufManager(sock->uid);

                num_connections++;
            }
        }
        else if (dwType == DPSYS_DESTROYPLAYERORGROUP)
        {
            DPMSG_DESTROYPLAYERORGROUP* destroymsg = (DPMSG_DESTROYPLAYERORGROUP*)data;
            if (is_server != 0 && destroymsg->dwPlayerType == DPPLAYERTYPE_PLAYER)
            {
                int infid;
                A2NetSock* sock = FindDpSock(destroymsg->dpId, &infid);
                CloseDpSock(sock, infid);
                num_connections--;
            }
        }
        else if (dwType == DPSYS_SESSIONLOST)
        {
            session_lost = 1;
        }
        return;
    }
    
    if (datasz <= 7 || datasz >= 0x97)
    {
        ReportWarning("CLlDriver::HandleMessageDp().\nReceived invalid message.\n");
        return;
    }

    A2NetSock* sock = FindDpSock(from, nullptr);
    if (!sock || !IsInUse(sock))
    {
        ReportWarning("CLlDriver::HandleMessageDp().\nMessage from disconnected client.\n");
        return;
    }

    if (datasz == 8)
    {
        if (guaranteed != 0)
        {
            ReportWarning("CLlDriver::HandleMessageDp().\nAcknoweledgement on guaranteed media.\n");
            return;
        }

        if (sock->list_0x14.IsEmpty())
            return;
        
        for (POSITION pos = sock->list_0x14.GetHeadPosition(); pos != nullptr;)
        {
            POSITION curpos = pos;
            NetStru3* buf = sock->list_0x14.GetNext(pos);
            if (buf->pktid == *(uint32_t*)data)
            {
                uint32_t latency = 0;
                const uint32_t ticks = GetTickCount();
                if (buf->timestamp < ticks)
                    latency = ticks - buf->timestamp;

                sock->latency_check.AddLatency(latency);

                sock->list_0x14.RemoveAt(curpos);

                buf->Clear();

                hl_driver->AddTailFreeNet3(buf);
                break;
            }
        }
        return;
    }

    NetStru3* buf = hl_driver->GetFreeNet3();
    buf->Clear();

    memcpy(&buf->full_data, data, datasz);

    buf->datasz = datasz - 8;
    
    if (guaranteed)
    {
        sock->manager->ReceiveData(buf);
        return;
    }

    while (dplay4->Send(listen_socket.player_dpid, from, 0, data, 8) == DPERR_BUSY)
    {
        ReportWarning("CLlDriver::HandleMessageDp().\nDirectPlay is busy, I go to sleep for 10ms.\n");
        Sleep(10);
    }

    uint32_t pktid = buf->pktid;
    if (pktid == sock->field_0x50)
    {
        sock->manager->ReceiveData(buf);
        sock->field_0x50++;
        if (!sock->list_0x30.IsEmpty())
        {
            for (POSITION pos = sock->list_0x30.GetHeadPosition(); pos != nullptr;)
            {
                POSITION curpos = pos;
                buf = sock->list_0x30.GetNext(pos);
                if (buf->pktid == sock->field_0x50)
                {
                    sock->manager->ReceiveData(buf);
                    sock->field_0x50++;

                    sock->list_0x30.RemoveAt(curpos);
                    return;
                }
            }
        }
    }
    else if (pktid < sock->field_0x50)
    {
        buf->Clear();
        hl_driver->AddTailFreeNet3(buf);
    }
    else
    {
        if (sock->list_0x30.IsEmpty())
        {
            sock->list_0x30.AddTail(buf);
        }
        else
        {
            for (POSITION pos = sock->list_0x30.GetHeadPosition(); pos != nullptr;)
            {
                POSITION curpos = pos;
                NetStru3 *cbuf = sock->list_0x30.GetNext(pos);
                if (cbuf->pktid == pktid)
                {
                    buf->Clear();
                    hl_driver->AddTailFreeNet3(buf);
                    return;
                }
            }

            sock->list_0x30.AddTail(buf);
        }
    }
}

A2NetSock* CLlDriver::FindDpSock(uint32_t id, int* out_idx)
{
    //524986
    if (is_server == 0)
        return &listen_socket;

    if (num_connections < 1)
    {
        if (out_idx)
            *out_idx = 0;
        return nullptr;
    }

    int32_t lb = 0;
    int32_t rb = num_connections - 1;
    while (lb >= rb)
    {
        int32_t cur = lb + (rb - lb) / 2;
        SocketNm* info = connections_info + cur;

        if (info->dpid == id)
        {
            if (out_idx)
                *out_idx = cur;

            return info->sock;
        }

        if (info->dpid <= id)
            lb = cur + 1;
        else
            rb = cur;
    }

    //if lb >= rb
    SocketNm* info = connections_info + rb;
    if (info->dpid == id)
    {
        if (out_idx)
            *out_idx = rb;
        return info->sock;
    }

    if (out_idx)
    {
        if (info->dpid > id)
            *out_idx = rb;
        else
            *out_idx = rb + 1;
    }
    return nullptr;
}


int CLlDriver::StartServer(int maxconn, const char* name, CLlAddress* addr)
{
    //521a09
    is_server = 1;
    max_connections = maxconn;
    server_start_addr = addr;
    if (!name)
    {
        DWORD sz = sizeof(comp_name);
        GetComputerNameA(comp_name, &sz);
    }
    else
        strcpy(comp_name, name);

    if (connection_sockets)
    {
        delete[] connection_sockets;
        connection_sockets = nullptr;
    }

    if (connections_info)
    {
        delete[] connections_info;
        connections_info = nullptr;
    }

    connection_sockets = new A2NetSock[max_connections]();
    connections_info = new SocketNm[max_connections]();

    if (listen_socket.is_in_use == 1)
        Close();

    num_connections = 0;

    if (provider == 4)
    {
        if (StartServerTcp() == 0)
        {
            ReportWarning("CLlDriver::StartServer().\nUnable to connect over TCP/IP.\n");
            return 0;
        }
    }
    else
    {
        if (StartServerDp() == 0)
        {
            ReportWarning("CLlDriver::StartServer().\nUnable to connect over DirectPlay.\n");
            return 0;
        }
    }
    listen_socket.is_in_use = 1;

    return 1;
}


int CLlDriver::PrepareForConnect(const char* name, CLlAddress* addr)
{
    //521d2a
    is_server = 0;
    max_connections = 1;

    if (connection_sockets)
    {
        delete[] connection_sockets;
        connection_sockets = nullptr;
    }

    if (connections_info)
    {
        delete[] connections_info;
        connections_info = nullptr;
    }

    strcpy(address_str, name);

    if (addr)
        cur_address = *addr;
    else
        memset(&cur_address, 0, sizeof(cur_address));

    if (provider == 4)
    {
        guaranteed = 1;
        return 1;
    }

    if (PrepareAddressDp(addr) == 0)
    {
        ReportWarning("CLlDriver::PrepareForConnect().\nUnable to initialize DirectPlay connection.\n");
        CloseDp();
        return 0;
    }

    DPCAPS caps;
    caps.dwSize = sizeof(DPCAPS);

    if (dplay4->GetCaps(&caps, DPGETCAPS_GUARANTEED) != DP_OK)
    {
        ReportWarning("CLlDriver::PrepareForConnect().\nUnable to get caps.\n");
        return 0;
    }

    if ((caps.dwFlags & (DPCAPS_GUARANTEEDSUPPORTED | DPCAPS_GUARANTEEDOPTIMIZED)) == (DPCAPS_GUARANTEEDSUPPORTED | DPCAPS_GUARANTEEDOPTIMIZED))
        guaranteed = 1;
    else
        guaranteed = 0;

    if (provider == 1)
    {
        latency = 1000;
        if (timeout < 16000)
            timeout = 16000;
    }
    else if (provider == 3)
    {
        guaranteed = 0;
        latency = 500;
        if (timeout < 16000)
            timeout = 16000;
    }
    else
    {
        latency = caps.dwLatency;
    }
    return 1;
}


int CLlDriver::ConnectDp(const char* name, CLlNetSession* session)
{
    //52465f
    ev_close = nullptr;
    ev_create_player = nullptr;

    listen_socket.player_dpid = -1;

    ev_create_player = CreateEventA(NULL, 0, 0, NULL);
    ev_close = CreateEventA(NULL, 0, 0, NULL);

    if (ev_create_player == nullptr || ev_close == nullptr)
    {
        ReportWarning("CLlDriver::ConnectDp().\nUnable to create events.\n");
        CloseDp();
        return 0;
    }

    DPSESSIONDESC2 desc;
    memset(&desc,0,0x50);
    desc.dwSize = sizeof(DPSESSIONDESC2);
    desc.guidInstance = session->guid;
    desc.lpszPasswordA = (char *)"";

    if (dplay4->Open(&desc, DPOPEN_JOIN) != DP_OK)
    {
        ReportWarning("CLlDriver::ConnectDp().\nUnable to join DirectPlay session.\n");
        CloseDp();
        return 0;
    }

    DPSESSIONDESC2 tmp;
    memset(&tmp, 0, 0x50);
    tmp.dwSize = sizeof(DPSESSIONDESC2);
    dplay4->EnumSessions(&tmp, 0, cbEnumSessions, this, DPENUMSESSIONS_STOPASYNC);

    DPNAME pname;
    memset(&pname, 0, sizeof(DPNAME));
    pname.dwSize = sizeof(DPNAME);
    pname.lpszShortNameA = (char *)name;
    pname.lpszLongNameA = (char*)name;
    if (dplay4->CreatePlayer((LPDPID)&listen_socket.player_dpid, &pname, ev_create_player, nullptr, 0, 0) != DP_OK)
    {
        ReportWarning("CLlDriver::ConnectDp().\nUnable to create DirectPlay player.\n");
        CloseDp();
        return 0;
    }

    listen_socket.field_0x50 = 0;
    listen_socket.field_0x4c = 0;

    listen_socket.uid = next_uid;

    listen_socket.latency_check.num = 0;
    listen_socket.latency_check.calc_latency = latency;
    
    listen_socket.manager = hl_driver->AllocClientBufManager(listen_socket.uid);
    listen_socket.wait_obj = CreateEventA(NULL, 0, 0, NULL);

    if (_beginthread(RecvThreadDp, 0, this) == -1L || listen_socket.wait_obj == nullptr)
    {
        CloseDp();
        return 0;
    }

    return 1;
}


int CLlDriver::ConnectTcp()
{
    //523794
    num_connections = 0;
    listen_socket.socket = INVALID_SOCKET;
    listen_socket.wait_obj = nullptr;
    listen_socket.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (listen_socket.socket == INVALID_SOCKET)
    {
        CloseTcp();
        return 0;
    }

    sockaddr connaddr;
    sockaddr bindaddr;
    if (MakeSockAddr(address_str, (sockaddr_in*)&connaddr, 8000, 1) == 0 ||
        MakeSockAddr(cur_address.address, (sockaddr_in*)&bindaddr, 0, 0) == 0)
    {
        CloseTcp();
        return 0;
    }

    if (bind(listen_socket.socket, &bindaddr, sizeof(bindaddr)) != 0)
    {
        CloseTcp();
        return 0;
    }

    if (connect(listen_socket.socket, &connaddr, sizeof(connaddr)) != 0)
    {
        CloseTcp();
        return 0;
    }

    SetSockOptions(listen_socket.socket, 1);

    listen_socket.uid = next_uid;
    listen_socket.manager = hl_driver->AllocClientBufManager(listen_socket.uid);
    listen_socket.wait_obj = CreateEventA(NULL, 0, 0, NULL);

    if (_beginthread(AcceptThreadTcp, 0x400, this) == -1L && listen_socket.wait_obj != nullptr)
    {
        CloseHandle(listen_socket.wait_obj);
        listen_socket.wait_obj = nullptr;
        CloseTcp();
        return 0;
    }
    
    num_connections = 1;
    guaranteed = 1;
    return 1;
}

int CLlDriver::Connect(const char* name, CLlNetSession* session)
{
    //5223fb
    char buf[16];
    memset(buf, 0, sizeof(buf));

    if (!name)
    {
        DWORD sz = sizeof(buf);
        GetComputerNameA(buf, &sz);
        name = buf;
    }

    if (listen_socket.is_in_use == 1)
        Close();

    if (provider == 4)
    {
        if (ConnectTcp() == 0)
        {
            ReportWarning("CLlDriver::Connect().\nUnable to connect over TCP/IP.\n");
            return 0;
        }
    }
    else
    {
        if (ConnectDp(name, session) == 0)
        {
            ReportWarning("CLlDriver::Connect().\nUnable to connect over DirectPlay.\n");
            return 0;
        }
    }

    listen_socket.is_in_use = 1;
    return 1;
}

int CLlDriver::SendDataTcp(A2NetSock* sock, NetStru3* buffer)
{
    //52394a
    int len = buffer->datasz + 8;
    int sendlen = -1;
    if (sock->socket != INVALID_SOCKET)
        sendlen = send(sock->socket, (char*)&buffer->full_data, len, 0);

    hl_driver->AddTailFreeNet3(buffer);

    if (sendlen == -1 || sendlen != len)
    {
        CloseTcpSocket(sock);
        return 0;
    }

    return 1;
}



int CLlDriver::SendDataDp(A2NetSock* sock, NetStru3* buffer)
{
    //5255c6
    if (session_lost != 0)
    {
        if (buffer)
            hl_driver->AddTailFreeNet3(buffer);

        CloseDp();
        session_lost = 0;
        return 0;
    }

    uint32_t currentTick = GetTickCount();
    if (guaranteed == 0)
    {
        if (buffer)
        {
            buffer->timestamp = currentTick;
            buffer->timestamp2 = 0;
            buffer->pktid = sock->field_0x4c;
            sock->field_0x4c++;
            sock->list_0x14.AddTail(buffer);

            sock->field_0x260++;
            sock->field_0x264--;
        }
    }
    else
    {
        if (!buffer)
            return 1;
    }

    uint32_t latency = 0;
    if (sock->latency)
        latency = sock->latency;
    else
        latency = sock->latency_check.GetLatency();
    
    if (guaranteed == 0)
    {
        uint32_t unsent_size = 0;

        for (POSITION it = sock->list_0x14.GetHeadPosition(); it != nullptr;)
        {
            NetStru3* dat = sock->list_0x14.GetNext(it);
            unsent_size += 8 + dat->datasz;
        }

        if (unsent_size > 0x10000)
        {
            ReportWarning("CLlDriver::SendDataDp().\nOut of bandwidth.\n");
            CloseDpSock(sock, -1);
            return 0;
        }

        latency = std::max<uint32_t>(latency, (unsent_size * 1000) / 1024);
    }

    POSITION iter = nullptr;
    if (guaranteed == 0)
        iter = sock->list_0x14.GetHeadPosition();

    bool loop = true;
    while (loop)
    {
        if (guaranteed == 0) 
        {
            buffer = sock->list_0x14.GetNext(iter);

            if (iter == nullptr) //not more nodes;
                loop = false;

            uint32_t mxticks = ((buffer->datasz + 8) * 1000) / 1024;
            if (currentTick < mxticks)
                mxticks = 0;
            else
                mxticks = currentTick - mxticks;

            uint32_t maxtime = 0;
            if (buffer->timestamp < mxticks)
                maxtime = mxticks - buffer->timestamp;
            
            if (timeout < maxtime)
            {
                ReportWarning("CLlDriver::SendDataDp().\nCannot receive acknowledgement.\n");
                CloseDpSock(sock, -1);
                return 0;
            }

            uint32_t t2time = 0;
            if (buffer->timestamp2 < mxticks)
                t2time = mxticks - buffer->timestamp2;

            if (latency <= t2time)
                buffer->timestamp2 = currentTick;
        }
        else
            loop = false;

        uint32_t to = DPID_SERVERPLAYER;
        if (is_server != 0)
            to = sock->player_dpid;


        uint32_t tosentsz = buffer->datasz + 8;

        sock->field_0x264++;

        HRESULT res = dplay4->Send(listen_socket.player_dpid, to, (guaranteed != 0 ? DPSEND_GUARANTEED : 0), &buffer->full_data, tosentsz);
        while (guaranteed != 0 && res == DPERR_BUSY) //if guaranteed try and try if BUSY
        {
            ReportWarning("CLlDriver::SendDataDp().\nDirectPlay is busy, I go to sleep for 10ms.\n");
            Sleep(10);
            res = dplay4->Send(listen_socket.player_dpid, to, (guaranteed != 0 ? DPSEND_GUARANTEED : 0), &buffer->full_data, tosentsz);
        }

        if (res != DP_OK && res != DPERR_BUSY) // not send and not BUSY -> exit
        {
            if (guaranteed != 0)
                hl_driver->AddTailFreeNet3(buffer);

            CloseDpSock(sock, -1);
            return 0;
        }

        if (guaranteed != 0)
            hl_driver->AddTailFreeNet3(buffer);
    }
    return 1;
}


int CLlDriver::SendData(uint32_t uid, NetStru3* buffer)
{
    //5226c4
    EnterCriticalSection(&critical_section);

    A2NetSock* sock = GetClientBySocketId(uid);

    if ((sock == NULL) || (sock->is_in_use != 1)) {
        if (buffer)
            hl_driver->AddTailFreeNet3(buffer);
        LeaveCriticalSection(&critical_section);
        return 0;
    }

    buffer->pos = buffer->datasz;

    if (provider == 4)
    {
        XorData(buffer->buf, buffer->datasz);

        if (SendDataTcp(sock, buffer) != 0)
        {
            LeaveCriticalSection(&critical_section);
            return 1;
        }
    }
    else {
        if (SendDataDp(sock, buffer) != 0)
        {
            LeaveCriticalSection(&critical_section);
            return 1;
        }
    }

    if (listen_socket.is_in_use == 1 && is_server != 0)
        ReportWarning("CLlDriver::SendData().\nConnection with client lost.\n");
    else
        ReportWarning("CLlDriver::SendData().\nConnection lost.\n");

    LeaveCriticalSection(&critical_section);
    return 0;
}

void CLlDriver::SendAllPendingDp(A2NetSock* sock)
{
    //525b86
    if (guaranteed == 0)
    {
        while (!sock->list_0x14.IsEmpty())
        {
            if (SendDataDp(sock, nullptr) == 0)
                break;

            Sleep(10);
        }
    }
}

void CLlDriver::DisconnectClient(uint32_t uid)
{
    //522526
    EnterCriticalSection(&critical_section);
    A2NetSock* sock = GetClientBySocketId(uid);
    if (sock)
    {
        if (IsInUse(sock))
        {
            if (provider == 4)
                CloseTcpSocket(sock);
            else
            {
                SendAllPendingDp(sock);
                CloseDpSock(sock, -1);
            }
            sock->is_in_use = 0;
        }
    }
    LeaveCriticalSection(&critical_section);
}

void CLlDriver::CleanupInvalidTcpClient(uint32_t uid)
{
    //52262a
    if (provider == 4)
    {
        EnterCriticalSection(&critical_section);
        A2NetSock* sock = GetClientBySocketId(uid);
        if (sock && IsInUse(sock))
        {
            if (sock->socket == INVALID_SOCKET)
                CloseTcpSocket(sock);
        }
        LeaveCriticalSection(&critical_section);
    }
}

void CLlDriver::CleanupAllInvalid()
{
    //522809
    if (listen_socket.is_in_use != 1)
        return;
        
    EnterCriticalSection(&critical_section);
    if (is_server == 0)
    {
        if (provider == 4)
        {
            if (listen_socket.socket == INVALID_SOCKET)
                CloseTcp();
        }
        else
            SendDataDp(&listen_socket, nullptr);
    }
    else
    {
        for (int i = 0; i < num_connections; i++)
        {
            A2NetSock* sock = connection_sockets + i;
            if (IsInUse(sock))
            {
                if (provider == 4)
                {
                    if (sock->socket == INVALID_SOCKET)
                        CloseTcpSocket(sock);
                }
                else
                    SendDataDp(sock, nullptr);
            }
        }
    }
    LeaveCriticalSection(&critical_section);
}


int CLlDriver::RestartModemServerDp()
{
    //52460e
    if (provider != 1)
        return 1;

    CloseDp();
    CreateDp();
    return StartServerDp();
}

void CLlDriver::RecreateDp()
{
    //524644
    CloseDp();
    CreateDp();
}

void CLlDriver::SetLatency(uint32_t conn_uid, int32_t latency_ms)
{
    //5229cd
    A2NetSock* sock = GetClientBySocketId(conn_uid);
    if (sock)
        sock->latency = latency_ms;
}

int32_t CLlDriver::GetLatency(uint32_t conn_uid)
{
    //5229fd
    A2NetSock* sock = GetClientBySocketId(conn_uid);
    if (!sock)
        return -1;

    if (sock->latency == 0)
        return sock->latency_check.GetLatency();
    else
        return sock->latency;
}

int32_t CLlDriver::GetPacketLoss(uint32_t conn_uid)
{
    //522a51
    A2NetSock* sock = GetClientBySocketId(conn_uid);
    if (!sock)
        return 0;

    if (sock->field_0x260 > 1000)
    {
        sock->field_0x260 /= 10;
        sock->field_0x264 /= 10;
    }

    if (sock->field_0x264 == 0)
        return 0;

    return ((float)sock->field_0x264 * 100000.0) / (float)sock->field_0x260;
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
void MarkUnitHiddenForPlayer(PacketMoveCmd* packet, Player* player) {
    if (packet->id < 0x88 || packet->id > 0x89) {
        return;
    }

    Unit* affected_unit = dword_6CDB3C->sub_5560D2(packet->field_0xa);
    if (affected_unit == nullptr) {
        return;
    }

    int32_t pid = player->player_id;
    if (0x10 <= pid && pid < 0x20) {
        affected_unit->something_per_player[pid - 0x10] |= 0x800000;
    }
}

// 51B370
void NetStru1::sub_51B370(PacketMoveCmd* packet, TokenPos* pos) {
    if (packet->to_player_id != 0) {
        Player* player = g_PlayersList->sub_535B50(packet->to_player_id);
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
        if (!player->field_0x43) {
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
void NetStru1::sub_51B638(PacketAoeZone* packet, AreaEffect* area_effect) {
    if (packet->to_player_id != 0) {
        Player* player = g_PlayersList->sub_535B50(packet->to_player_id);
        if ((area_effect->field4_0x42 & player->vision_sharing_id) != 0 || g_Server->field4_0x74 == 0) {
            this->QueuePacketSend(packet);
        } else {
            auto pid = player->player_id;
            if (pid >= 0x10 && pid < 0x20) {
                if (packet->field_0xf != 0) {
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
        if (!player->field_0x43) {
            continue;
        }
        if ((area_effect->field4_0x42 & player->vision_sharing_id) != 0 || g_Server->field4_0x74 == 0) {
            packet->to_player_id = player->player_id;
            this->QueuePacketSend(packet);
        } else {
            auto pid = player->player_id;
            if (pid >= 0x10 && pid < 0x20) {
                if (packet->field_0xf != 0) {
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
void NetStru1::sub_51BAB0(Unit* caster, Spell* spell, Unit* target, int16_t delay) {
    PacketMoveCmd& packet = PacketMoveCmd::Inst;
    packet.id = 0x86;
    packet.field_0xa = caster->building_id;
    packet.field_0xc = 0;
    packet.field_0xd = 0;
    packet.field_0xe = 0;
    if (caster->typeId == 0) {
        packet.id = 0x8B;
        // Original code was writing the X and Y separately into +0xa and +0xb. I hope it's the same.
        packet.field_0xa = caster->position->GetX() | (caster->position->GetY() << 8);
    }
    packet.field_0xc = (spell->spell_id * 2 + 8);
    if (target != nullptr) {
        *(uint16_t*)&packet.field_0xd = (uint16_t)target->building_id;
        packet.field_0xf = delay;
    }
    if (caster->VMethod8()) {
        caster->last_action_tick = g_Server->tick;
    }
    packet.to_player_id = 0;
    this->sub_51B370(&packet, caster->position);
}

// 51BB94
void NetStru1::sub_51BB94(Unit* caster, Spell* spell, TokenPos* pos, int16_t delay) {
    PacketMoveCmd& packet = PacketMoveCmd::Inst;
    packet.id = 0x86;
    packet.field_0xa = caster->building_id;
    if (caster->typeId == 0) {
        packet.id = 0x8B;
        // Original code was writing the X and Y separately into +0xa and +0xb. I hope it's the same.
        packet.field_0xa = caster->position->GetX() | (caster->position->GetY() << 8);
    }
    packet.field_0xc = spell->spell_id * 2 + 8;
    packet.field_0xd = pos->GetX();
    packet.field_0xe = pos->GetY();
    packet.field_0xf = delay;
    if (caster->VMethod8()) {
        caster->last_action_tick = g_Server->tick;
    }
    packet.to_player_id = 0;
    this->sub_51B370(&packet, caster->position);
}

// 51BDA4
void NetStru1::sub_51BDA4(Effect* effect, Unit* unit, uint8_t arg) {
    PacketEffect& packet = PacketEffect::Inst;
    packet.id = arg ? arg : 0x88;
    packet.field_0xa = unit->building_id;
    packet.effect_type = effect->typeId;
    packet.to_player_id = 0;
    // This reinterpret_cast is kinda weird, but it works, because `sub_51B370` only reads the 2-byte 0xa field from non-Packet fields.
    this->sub_51B370(reinterpret_cast<PacketMoveCmd*>(&packet), unit->position);
}

// 51C822
void NetStru1::sub_51C822(NetStru2* ns2) {
    PacketDword& packet = PacketDword::Inst;
    packet.id = 0x0E;
    packet.value = dword_665CFC;
    packet.VMethod3(ns2);
    packet.id = 0x64;
    packet.value = (uint32_t)g_Server->tick;
    packet.VMethod3(ns2);
    ns2->out_buffers[ns2->out_buff_id].pkt_num += 1;
    this->SendAllData();
}

// 51D837
void NetStru1::sub_51D837(int32_t param1, Player* player) {
    PacketDword& packet = PacketDword::Inst;
    packet.id = 0x10;
    if (player != nullptr) {
        packet.to_player_id = player->player_id;
    } else {
        packet.to_player_id = 0;
    }
    packet.value = param1;
    this->QueuePacketSend(&packet);
}

// 51DFA7
void NetStru1::sub_51DFA7(int32_t param1, int32_t param2, CString name, void* data, uint32_t size, uint16_t player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = player_id;
    packet.id = 0xCF;
    int32_t name_len = name.GetLength();
    packet.count = name_len + size + 0x10;
    uint32_t* p = (uint32_t*)packet.data;
    *p++ = param1;
    *p++ = param2;
    *p++ = name_len;
    *p++ = size;
    memcpy(p, data, size);
    memcpy(packet.data + 0x10 + size, (const char*)name, name_len);
    this->QueuePacketSend(&packet);
}

// 51E0B7
void NetStru1::sub_51E0B7(int32_t param1, int32_t param2, CString name, void* data, uint32_t size, uint16_t player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = player_id;
    packet.id = 0xE0;
    int32_t name_len = name.GetLength();
    packet.count = name_len + size + 0x10;
    uint32_t* p = (uint32_t*)packet.data;
    *p++ = param1;
    *p++ = param2;
    *p++ = name_len;
    *p++ = size;
    memcpy(p, data, size);
    memcpy(packet.data + 0x10 + size, (const char*)name, name_len);
    this->QueuePacketSend(&packet);
}

// 51E1C7
void NetStru1::sub_51E1C7(uint16_t player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = player_id;
    packet.id = 0xE0;
    packet.count = 1;
    this->QueuePacketSend(&packet);
}

// 51E205
void NetStru1::sub_51E205(CString name) {
    PacketJoin& packet = PacketJoin::Inst;
    packet.id = 0xD1;
    packet.__field_0xa = 0;
    strcpy(packet.name, name);
    packet.to_player_id = 0;
    this->QueuePacketSend(&packet);
}

// 51E5FB
void NetStru1::sub_51E5FB(int32_t a, int32_t b) {
    PacketInfo& packet = PacketInfo::Inst;
    packet.id = 0xD8;
    packet.field_0xa = a;
    packet.field_0xe = b;
    packet.to_player_id = 0;
    this->QueuePacketSend(&packet);
}

// 51E63F
void NetStru1::sub_51E63F(int32_t a, int32_t b, int32_t error_code) {
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
void NetStru1::sub_51E794(int32_t a, int32_t b, uint8_t status, int16_t player_id) {
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
void NetStru1::sub_51A6D5(Unit* unit, Player* player, int32_t param5, int32_t param6) {
    if (unit->typeId < 0x21 || unit->typeId >= 0x40) {
        return;
    }

    if (param5 + param6 == 0) {
        param5 = 0;
        param6 = unit->inventory->items.m_nCount;
    }

    PacketUnitStateVec& packet = PacketUnitStateVec::Inst;
    packet.id = 0x76;
    packet.entry_count = (uint16_t)(param6 - param5);
    packet.field_0xf = (uint16_t)param5;
    packet.data_size = 0;

    packet.field_0xc = 2;
    if (param5 != 0 || param6 != unit->inventory->items.m_nCount) {
        packet.field_0xc |= 0x80;
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
        if (index >= param5 && index < param6) {
            item->StoreToPacket(&packet, 0);
        }
        index++;
    }

    this->QueuePacketSend(&packet);
}

// 51A0EF
void NetStru1::sub_51A0EF(Unit* unit, Player* player, int32_t flags) {
    if (!unit->VMethod8()) {
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
            if (p->field_0x43 || p == unit->pOwner) {
                this->sub_51A0EF(unit, p, 0xFFB);
            }
        }
        return;
    }

    if (unit->typeId < 0x21 || unit->typeId >= 0x40) {
        if (unit->field_x18 & player->vision_sharing_id) {
            return;
        }

        if (unit->pOwner != player) {
            PacketUnitProperties& packet = PacketUnitProperties::Inst;
            packet.id = 0x9C;
            packet.field_0xa = (uint16_t)unit->building_id;
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
    packet.field_0xf = (uint16_t)flags;
    packet.data_size = 0;
    packet.field_0xc = 1;
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
    if (unit->VMethod8()) {
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

// 51AC77
void NetStru1::sub_51AC77(Token* token, Player* player, uint8_t flag) {
    if (player == nullptr) {
        POSITION pos = g_PlayersList->list.GetHeadPosition();
        while (pos != nullptr) {
            Player* p = g_PlayersList->list.GetNext(pos);
            if (p->field_0x43 || p == token->pOwner) {
                this->sub_51AC77(token, p, flag);
            }
        }
        return;
    }

    if (token == nullptr) {
        // Original code here constructs a string "Token Change with NULL\n" and then discards it :) Guess it's from debugging.
        return;
    }

    if (token->VMethod7()) {
        Unit* unit = static_cast<Unit*>(token);
        if (flag == 0x73) {
            if (!(unit->field_0x1a4 & player->vision_sharing_id) && g_Server->field4_0x74 != 0) {
                int32_t pid = player->player_id;
                if (0x10 <= pid && pid < 0x20) {
                    unit->something_per_player[pid - 0x10] |= 1;
                }
                return;
            }
            PacketPing& packet = PacketPing::Inst;
            packet.id = flag;
            packet.to_player_id = player->player_id;
            packet.field_0xa = (uint16_t)unit->building_id;
            packet.field_0xc = unit->hp;
            this->QueuePacketSend(&packet);
        }
        return;
    }

    if (token->IsKindOf(RUNTIME_CLASS(Sack))) {
        Sack* sack = static_cast<Sack*>(token);
        PacketEight& packet = PacketEight::Inst;
        packet.id = 0x7A;
        packet.to_player_id = player->player_id;
        packet.xpos = sack->position->GetXx();
        packet.ypos = sack->position->GetYy();
        packet.unit_id = (uint16_t)sack->building_id;
        packet.type_id = (uint8_t)log10(sack->_exp);

        if (g_ServerConfig.gameType == 2) {
            if (sack->inventory->sub_5530A2("Quest RuneF")) {
                packet.type_id |= 0x40;
            }
            if (sack->inventory->sub_5530A2("Quest RuneA")) {
                packet.type_id |= 0x80;
            }
        }

        if (g_Server->field4_0x74 != 0) {
            if (!IsVisibleTo(sack->position, player)) {
                return;
            }
        }

        sack->field_x18 |= player->vision_sharing_id;
        sack->field_0x4c &= ~player->vision_sharing_id;
        this->QueuePacketSend(&packet);
        return;
    }

    if (token->VMethod9()) {
        Building* building = static_cast<Building*>(token);
        building->field_x18 |= (player ? player->vision_sharing_id : 0xFFFF);
        if (building->typeId != 0 && building->hp_max != 0) {
            PacketSync& packet = PacketSync::Inst;
            packet.id = 0x82;
            packet.to_player_id = player ? player->player_id : 0;
            packet.field_0xa = (uint16_t)building->building_id;
            packet.field_0xc = building->hp;
            this->QueuePacketSend(&packet);
        }
    }
}
