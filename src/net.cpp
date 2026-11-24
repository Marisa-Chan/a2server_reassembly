#include "net.h"
#include <WS2tcpip.h>

#include <cmath>
#include <cstring>
#include <process.h>
#include <wininet.h>

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
#ifdef A2CLIENT_PATCH
#include "patch/hat_client.h"
#include "patch/flags.h"
#endif
#ifdef A2SERVER_PATCH
#include <cerrno>
#include <string>
#include "patch/flags.h"
#include "patch/tangar_log.h"
#endif
#ifdef A2TANGAR_HAT
#include "patch/config_new.h"
#include "patch/flags.h"
#include "patch/hat_net.h"
#endif

#if defined(A2CLIENT_PATCH) || defined(A2SERVER_PATCH)
#include "patch/netlog.h"

namespace {
// Netlog direction tag for a CLlDriver TCP socket, by peer port: the HAT
// listens on 8000 (clients) / 7999 (servers), game servers on 8001+.
long netlog_peer_port(SOCKET s) {
    sockaddr_in peer;
    int sl = sizeof(peer);
    if (getpeername(s, (sockaddr*)&peer, &sl) == 0) {
        return ntohs(peer.sin_port);
    }
    return 0;
}
} // namespace
#endif


uint32_t g_SpectatorState;

GUID Allods2GUID = {0x65450caa, 0x57ca, 0x11d2, {0xbb, 0xcc,0x00, 0x60, 0x97, 0xd2, 0xee, 0x9f}};


NetStru1 NetStru1::HatConnector(0); // 6a6a98
NetStru1 g_NetStru1_main(1);  //6c3a08
NetStru1 g_NetStru1_local(0);  //6b16b0

CLlDriver g_CLlDriver(Allods2GUID, &g_NetStru1_main); //6d07a0
CLlDriver g_HatLLDriver(Allods2GUID, &NetStru1::HatConnector); //6cdbc0

#ifdef A2SERVER_PATCH
namespace {

const char* TcpDriverLabel(const CLlDriver* drv) {
    if (drv == &g_CLlDriver) {
        return "server";
    }
    if (drv == &g_HatLLDriver) {
        return "hat";
    }
    return "other";
}

std::string FormatWinSockError(int32_t err) {
    char msg_buf[256] = {};
    DWORD len = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        static_cast<DWORD>(err),
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        msg_buf,
        sizeof(msg_buf),
        nullptr);
    if (len == 0) {
        len = FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            static_cast<DWORD>(err),
            0,
            msg_buf,
            sizeof(msg_buf),
            nullptr);
    }
    while (len > 0 && (msg_buf[len - 1] == '\r' || msg_buf[len - 1] == '\n' || msg_buf[len - 1] == ' ')) {
        msg_buf[--len] = '\0';
    }
    char out[320];
    if (len > 0) {
        _snprintf(out, sizeof(out) - 1, "%d (%s)", err, msg_buf);
    } else {
        _snprintf(out, sizeof(out) - 1, "%d", err);
    }
    out[sizeof(out) - 1] = '\0';
    return out;
}

std::string FormatSockAddrIn(const sockaddr_in* sa) {
    if (sa == nullptr) {
        return "<null>";
    }
    in_addr addr_copy = sa->sin_addr;
    char ip_buf[INET_ADDRSTRLEN] = {};
    const char* ip_str = InetNtopA(AF_INET, &addr_copy, ip_buf, sizeof(ip_buf));
    char out[64];
    _snprintf(out, sizeof(out) - 1, "%s:%u", ip_str != nullptr ? ip_str : "<invalid>", static_cast<unsigned int>(ntohs(sa->sin_port)));
    out[sizeof(out) - 1] = '\0';
    return out;
}

} // namespace
#endif


// 526621 (client: 515453)
void PackerDat::DeleteTail(PackerTail* node) {
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

// 5265f7 (client: 515429)
void PackerDat::Clear() {
    this->DeleteTail(this->tail);
    this->tail = nullptr;
}

// 5260fa (client: 514efa)
PackerDat::PackerDat() {
    this->tail = nullptr;
}

// 526115 (client: 514f15)
PackerDat::~PackerDat() {
    this->Clear();
}

// 51cd89 (client: 50de98)
void NetStru1::AnnouncePlayerName(const CString& name, Player* player) {
    PacketJoin& pkt = PacketJoin::Inst;
    pkt.id = CHAT;
#ifdef A2SERVER_PATCH
    strncpy(pkt.name, name, sizeof(pkt.name) - 1);
    pkt.name[sizeof(pkt.name) - 1] = '\0';
#else
    strcpy(pkt.name, name);
#endif

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

// 51c8b1 (client: 50d96e) - Send all existing players (PacketJoin) to new player
void NetStru1::SendExistingPlayers(Player* player) {
    LogMessage("NetStru1::SendExistingPlayers");
    POSITION pos = g_PlayersList->list.GetHeadPosition();
    while (pos != nullptr) {
        Player* p = g_PlayersList->list.GetNext(pos);
        if (p == player) {
            continue;
        }
#ifdef A2SERVER_PATCH
        // adm_invis hook (51C901): invisible GMs are hidden from the joining
        // player's view of the world
        if ((p->flags & GMF_INVISIBLE) == GMF_INVISIBLE) {
            continue;
        }
#endif

        PacketJoin& packet = PacketJoin::Inst;

#ifdef A2SERVER_PATCH
        strncpy(packet.name, p->name, sizeof(packet.name) - 1);
        packet.name[sizeof(packet.name) - 1] = '\0';
#else
        strcpy(packet.name, p->name);
#endif
        packet.id = PLAYER_INFO;
        packet.to_player_id = player->player_id;
        packet.player_id = p->player_id;
        packet.token_id = p->token_id;
        packet.color = p->color_slot - 1;
        packet.flags = p->is_ai != 0;
        this->QueuePacketSend(&packet);

        if (p->main_unit != nullptr && p->main_unit->has_flagged_item != 0
#ifndef A2CLIENT
            && g_ServerConfig.gameType == GAME_TYPE_COOPERATIVE
#endif
        ) {
            g_NetStru1_main.SendTeamNotify(TEAM_NOTIFY_PLAYER_FLAGGED, p->player_id, player);
        }
    }
}

// 51c61e (client: 50d7b2) - Remove unit/sack from network tracking
void NetStru1::UntrackToken(Token* token) {
    PacketWord& packet = PacketWord::Inst;

    packet.id = TOKEN_REMOVE;
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

// 51c748 (client: 50d82f)
void NetStru1::SendTickPacket(Player* player) {
    PacketDword& packet = PacketDword::Inst;
    packet.to_player_id = 0;
    packet.id = SERVER_TICK;
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

// 51ce86 (client: 50df07)
void NetStru1::SendTeamNotify(uint32_t msg_type, uint32_t player_id, Player* recpt) {
    this->SendInfoPacket(TEAM_NOTIFY, msg_type, player_id, recpt);
}

// 51ceac (client: 50df2d)
void NetStru1::SendBarePacket(uint8_t id, Player* player) {
    Packet& packet = Packet::Inst;
    packet.id = id; 
    packet.to_player_id = player ? player->player_id : 0;
    this->QueuePacketSend(&packet);
}

// 51d49b (client: 50e2cc)
void NetStru1::AnnouncePlayer(Player* player) {
    PacketJoin& packet = PacketJoin::Inst;
    packet.id = PLAYER_ANNOUNCED;
#ifdef A2SERVER_PATCH
    strncpy(packet.name, player->name, sizeof(packet.name) - 1);
    packet.name[sizeof(packet.name) - 1] = '\0';
#else
    strcpy(packet.name, player->name);
#endif
    packet.to_player_id = 0;
    packet.__as_dword = player->player_id;
    this->QueuePacketSend(&packet);
}

// 51cefb (client: 50df7c)
void NetStru1::SendInfoPacket(uint8_t id, int32_t value1, int32_t value2, Player* target) {
    Packet2Dwords& packet = Packet2Dwords::Inst;
    packet.id = id;
    packet.to_player_id = target ? target->player_id : 0;
    packet.param1 = value1;
    packet.param2 = value2;
    this->QueuePacketSend(&packet);
}

// 51c7cc (client: 50d8ab) - Send latency update to player
void NetStru1::SendLatency(int32_t latency, Player* player) {
    PacketDword& packet = PacketDword::Inst;
    packet.id = LATENCY;
    packet.value = latency;
    packet.to_player_id = player ? player->player_id : 0;
    this->QueuePacketSend(&packet);
}

// 51cd2a (client: 50de39) - Send in-game event trigger
void NetStru1::SendGameEvent(Player* player, int32_t event_id, int32_t value2) {
    Packet2Dwords& packet = Packet2Dwords::Inst;
    packet.id = GAME_EVENT;
    packet.to_player_id = player ? player->player_id : 0;
    packet.param1 = event_id;
    packet.param2 = value2;
    this->QueuePacketSend(&packet);
}

// 51cdfb (client: none) - send arena score
void NetStru1::SendArenaScore(CString& name, uint32_t score, char type, Player* player) {
    PacketJoin& packet = PacketJoin::Inst;
    if (type != 0) {
        packet.id = type;
    } else {
        packet.id = CHAT;
    }
#ifdef A2SERVER_PATCH
    strncpy(packet.name, name, sizeof(packet.name) - 1);
    packet.name[sizeof(packet.name) - 1] = '\0';
#else
    strcpy(packet.name, name);
#endif
    packet.to_player_id = player ? player->player_id : 0;
    packet.__as_dword = score;
    this->QueuePacketSend(&packet);
}

// 51ca5d (client: 50da69) - Send server state to player
void NetStru1::SendServerState(Player* player) {
    POSITION pos = g_Server->srv_stru1->sack_list->list.GetHeadPosition();
    while (pos != nullptr) {
        Sack* sack = g_Server->srv_stru1->sack_list->list.GetNext(pos);
        this->BroadcastTokenState(sack, player, 0);
    }
}

// 51c601 (client: 50d795) - Send unit-level-up packet
void NetStru1::SendLevelUp(Unit* unit, int unused) {
    this->BroadcastTokenState(unit, nullptr, UNIT_DAMAGED);
}

// 51b870 (client: 50cc7f) - Broadcast unit to all players.
void NetStru1::BroadcastUnitState(Unit* unit) {
    POSITION pos = g_PlayersList->list.GetHeadPosition();
    while (pos != nullptr) {
        Player* player = g_PlayersList->list.GetNext(pos);
        if (player->is_active && unit->NeedsRefreshFor(player)) {
            unit->position->IsAtCellCenter(); // WAT: Useless call?
            if (unit->IsUnit()) {
                this->SendUnitUpdate(unit, player, UNIT_UPD_ALL, EQUIP_SLOTS_ALL, 0, 0);
            } else {
                this->BroadcastTokenState(unit, player, 0);
            }
        }
    }
}

// 51b99e (client: 50cd3c)
void NetStru1::SendUnitActionPacket(Unit* unit, uint8_t angle, uint8_t speed_dir, uint8_t mode) {
    PacketUnitAction& packet = PacketUnitAction::Inst;
    if (mode == 0) {
        packet.id = UNIT_MOVE;
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

// 51ba2f (client: 50cdcd)
void NetStru1::SendRangedAttackPacket(Unit* unit) {
    PacketRangedAttack& packet = PacketRangedAttack::Inst;
    packet.id = RANGED_ATTACK;
    packet.unit_id = (uint16_t)unit->building_id;
    packet.target_id = (uint16_t)unit->cast_target->building_id;
    packet.to_player_id = 0;
    unit->last_action_tick = g_Server->tick + unit->charge + unit->relax;
    this->SendUnitAction(&packet, unit);
}

// 51c59c (client: 50d730) - Notify clients about unit turning to face its target
void NetStru1::SendUnitTurn(Unit* unit, Unit* target) {
    if (unit->max_range < 2) {
        const uint8_t speed_dir = unit->charge + unit->relax;
        const uint8_t angle = unit->eye->GetFacingHalf();
        g_NetStru1_main.SendUnitActionPacket(unit, angle, speed_dir, UNIT_ATTACK);
    } else {
        g_NetStru1_main.SendRangedAttackPacket(unit);
    }
}

// 51be0e (client: 50d19c) - Broadcast move packet for an effect token
void NetStru1::BroadcastEffectMove(Effect* effect, uint8_t type) {
    PacketMoveBroadcast& packet = PacketMoveBroadcast::Inst;
    packet.id = EFFECT_MOVE;
    packet.x = effect->position->GetX();
    packet.y = effect->position->GetY();
    packet.segments = type;
    packet.unit_id = (uint16_t)effect->building_id;
    packet.type_id = (uint8_t)effect->typeId;
    packet.to_player_id = 0;
    this->BroadcastMovePacket(&packet, effect->position);
}

// 51be8f (client: 50d21d) - Send area-effect packet
void NetStru1::SendAreaEffect(AreaEffect* obj, int32_t flag) {
    if (obj->itemDataID == 2) {
        PacketMoveBroadcast& packet = PacketMoveBroadcast::Inst;
        packet.id = EFFECT_MOVE;
        packet.x = obj->position->GetX();
        packet.y = obj->position->GetY();
        packet.segments = 0x16;
        packet.unit_id = (uint16_t)obj->building_id;
        packet.type_id = (uint8_t)obj->typeId;
        packet.to_player_id = 0;
        this->BroadcastMovePacket(&packet, obj->position);
    } else {
        PacketAoeZone& packet = PacketAoeZone::Inst;
        packet.id = AOE_ZONE;

        const int8_t radius = obj->radius;
        packet.x = obj->position->GetX() - radius;
        packet.y = obj->position->GetY() - radius;

        const int8_t size = radius * 2 + 1;
        packet.width = size;
        packet.height = size;

        for (int32_t i = 0; i < 12; i++) {
            packet.data[i] = 0;
        }
        for (int32_t dx = 0; dx <= radius * 2; ++dx) {
            for (int32_t dy = 0; dy <= radius * 2; ++dy) {
                int32_t bit_index = dx + dy * size;
                int32_t coordinate = (packet.x + dx) + ((packet.y + dy) << 8);
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

// 51c0f7 (client: 50d485) - Send units from pending unit list to player
void NetStru1::SendPendingUnits(Player* player) {
    if (g_ServerUnits != nullptr) {
        POSITION pos = g_ServerUnits->unit_list.GetHeadPosition();
        while (pos != nullptr) {
            Unit* unit = g_ServerUnits->unit_list.GetNext(pos);
            uint32_t flags = (unit->building_id < 0x6000) ? UNIT_UPD_ALL_STATS_EQUIPMENT : (UNIT_UPD_HP | UNIT_UPD_MP | UNIT_UPD_DIR_POS | UNIT_UPD_SPELLS);
            if (unit->ownership_changed != 0) {
                flags |= UNIT_UPD_OWNER;
            }
            if (g_Server->multiplayer == 0 && 0x16 <= unit->server_id && unit->server_id <= 0x17) {
                flags |= UNIT_UPD_SEND_INVENTORY;
            }
            g_NetStru1_main.SendUnitUpdate(unit, player, flags, EQUIP_SLOTS_ALL, 0, 0);
        }
    }

    UnitList* units_list = g_Server->srv_stru1->units_list;
    if (units_list != nullptr) {
        POSITION pos = units_list->unit_list.GetHeadPosition();
        while (pos != nullptr) {
            Unit* unit = units_list->unit_list.GetNext(pos);
            uint32_t flags = (unit->building_id < 0x6000) ? UNIT_UPD_ALL_STATS_EQUIPMENT : (UNIT_UPD_HP | UNIT_UPD_MP | UNIT_UPD_DIR_POS);
            if (unit->ownership_changed != 0) {
                flags |= UNIT_UPD_OWNER;
            }
            if (unit->decay < Unit::DECAY_REMOVED) {
                g_NetStru1_main.SendUnitUpdate(unit, player, flags, EQUIP_SLOTS_ALL, 0, 0);
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

// 51c46e (client: 50d66d)
void NetStru1::ResendPlayerUnits(Player* player) {
    if (player->main_unit != nullptr) {
        this->SendUnitUpdate(player->main_unit, player, UNIT_UPD_ALL, EQUIP_SLOTS_ALL, 0, 0);
    }
    if (player->unit_list != nullptr) {
        POSITION pos = player->unit_list->unit_list.GetHeadPosition();
        while (pos != nullptr) {
            Unit* unit = player->unit_list->unit_list.GetNext(pos);
            unit->sent_to_mask &= ~player->vision_sharing_id;
            g_NetStru1_main.SendUnitUpdate(unit, player, UNIT_UPD_ALL, EQUIP_SLOTS_ALL, 0, 0);
        }
    }
    this->SendTickPacket(player);
}

// 51cb21 (client: 50dabc) - Send terrain/diplomacy visibility state to player
void NetStru1::SendVisibilityState(Player* player) {
    if (g_World == nullptr) {
        return;
    }
    PacketTerrain& packet = PacketTerrain::Inst;
    packet.id = DIPLOMACY_STATE;
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
        if (diplomacy & DIPLO_VISION_SHARE) {
            packet.buf[pid] |= DIPLO_SHARED_VISION;
        }
        if (diplomacy & DIPLO_ALLIANCE) {
            packet.buf[pid] |= DIPLO_THEY_ALLIED;
        }
        if (diplomacy & DIPLO_WAR) {
            packet.buf[pid] |= DIPLO_THEY_WAR;
        }

        uint16_t crossd = g_World->diplomacy.diplomacy[this_player_id][pid] | (g_World->diplomacy.diplomacy[pid][this_player_id] << 8);
#ifdef A2CLIENT
        if (p->is_ai == 0 && Server::somewords[this_player_id][pid] != crossd && this_player_id != pid) {
            CString str;
            str.Format("Diplomacy changed: %s -> %s =", (const char*)p->name, (const char*)player->name);

            if ((g_World->diplomacy.diplomacy[pid][this_player_id] & DIPLO_ALLIANCE)) {
                str += " Alliance";
            }
            else if ((g_World->diplomacy.diplomacy[pid][this_player_id] & DIPLO_WAR)) {
                str += " War";
            }
            else
                str += " Neutrality";

            if ((g_World->diplomacy.diplomacy[pid][this_player_id] & DIPLO_VISION_SHARE) != 0) {
                str += " + Vision";
            }
            LogMessage(str);
        }

        Server::somewords[this_player_id][pid] = crossd;
        Server::somewords[pid][this_player_id] = g_World->diplomacy.diplomacy[pid][this_player_id] | (g_World->diplomacy.diplomacy[this_player_id][pid] << 8);
#endif
    }

    this->QueuePacketSend(&packet);
}

// 51cf5c (client: 50dfdd) - Send unit visibility packet (retn 0Ch)
void NetStru1::SendUnitVisibility(Unit* unit, int32_t flag, Player* player) {
    PacketWord& packet = PacketWord::Inst;
    packet.id = flag ? UNIT_HIDDEN : UNIT_SHOWN;
    packet.value = unit->building_id;

    if (player != nullptr) {
        packet.to_player_id = player->player_id;
        if (g_Server->multiplayer != 0) {
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
        if (g_Server->multiplayer != 0) {
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

// 51d1a8 (client: 50e1b7) - Send kill stats (pass 0,nullptr for all)
void NetStru1::SendKillStats(uint16_t player_id, Player* player) {
    if (player_id != 0) {
        PacketDword& packet = PacketDword::Inst;
        packet.id = KILL_STAT;
        packet.to_player_id = player ? player->player_id : 0;
        packet.value = (uint32_t)player_id | ((uint32_t)player->kill_stats[player_id] << 16);
        this->QueuePacketSend(&packet);
        return;
    }

    PacketData& packet = PacketData::Inst;
    packet.id = CREATURE_KNOWLEDGE;
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

// 51d4f6 (client: 50e326) - Send SrvStru1 state list from packet
void NetStru1::SendSrvStru1State(QuestMap* quest_map, Player* player, int32_t flag) {
    PacketTerrain& packet = PacketTerrain::Inst;
    packet.id = flag ? INN_QUESTS : PLAYER_QUESTS;
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
        memcpy(ptr, &quest->quest_data, sizeof(QuestData));
        ptr += sizeof(QuestData);
        packet.count += 0xF;
        packet.buf[0] += 1;
    }
    this->QueuePacketSend(&packet);
}

// 51d6b4 (client: none)
void NetStru1::SendTeamState(uint16_t to_player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = to_player_id;
    packet.id = TEAM_STATE;
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

// 51eeb7 (client: 50f6ef)
void NetStru1::UpdateConnStats() {
    int32_t key;
    ConnStatInfo* info;
    POSITION pos = this->client_stat.GetStartPosition();
    while (pos != nullptr) {
        this->client_stat.GetNextAssoc(pos, key, info);
        info->time += 1;
        info->total_bytes += info->window_bytes;
        if (info->window_bytes > info->max_bs) {
            info->max_bs = info->window_bytes;
        }
        info->cur_bs = info->window_bytes;
        info->window_bytes = 0;
    }
}

// 51e289 (client: 50ec97) - Send hat lobby info packet
void NetStru1::SendHatLobbyInfo(int32_t player_count, CString source, int32_t map_level, int32_t game_type, int32_t map_range) {
#ifdef A2TANGAR_HAT
    // imp_UpdateInfo hook (51E289): push server status over the HAT control
    // layer instead of legacy 0xD2 HAT_LOBBY_INFO so HAT preserves SERVER_CAP_DETAILED_INFO.
    if ((Config::ServerCaps & SVC_DETAILED_INFO) != 0) {
        tangar_hat::NetHatUpdateInfo(static_cast<uint8_t>(player_count), static_cast<const char*>(source),
                                     static_cast<uint8_t>(map_level), static_cast<uint8_t>(game_type),
                                     static_cast<uint8_t>(map_range));
        return;
    }
#endif
    PacketJoin& pkt = PacketJoin::Inst;
    pkt.id = HAT_LOBBY_INFO;
    pkt.__as_dword = player_count | (map_level << 8) | (game_type << 16) | (map_range << 24);
#ifdef A2SERVER_PATCH
    strncpy(pkt.name, source, sizeof(pkt.name) - 1);
    pkt.name[sizeof(pkt.name) - 1] = '\0';
#else
    strcpy(pkt.name, source);
#endif
    pkt.to_player_id = 0;
    this->QueuePacketSend(&pkt);
}

// SendUnitUpdate: send unit state to a player (or all players if player==nullptr)
// 519221 (client: 50ac91)
void NetStru1::SendUnitUpdate(Unit* unit, Player* player, uint32_t mask, int32_t props_flags, int32_t inv_start_index, int32_t inv_end_index) {
#ifdef A2SERVER_PATCH
    if (unit == nullptr) {
        return;
    }
#endif
    if (!g_PlayersList && !player)
        return;

    if (player == nullptr) {
        if (!g_PlayersList) {
            return;
        }

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
        if (g_Server->multiplayer != 0) {
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
                mask &= UNIT_UPD_SUMMON_MASK;
            }
        } else if (g_Server->multiplayer != 0) {
            mask &= ~UNIT_UPD_SERVER_ID;
        }

        if (unit->pOwner != player) {
            mask &= UNIT_UPD_MONSTER_MASK;

#ifdef A2SERVER_PATCH
            // fix_0051946B hook (51946B): unit->monster_info may be null on non-humanoid units.
            if (unit->monster_info != nullptr && unit->monster_info->Values().GetSize() > 0)
#endif
            {
                int32_t max_hp = unit->monster_info->Values()[0].health_max;

                if (g_Server->multiplayer == 0) {
                    if (g_Server->difficulty == DIFFICULTY_EASY) {
                        max_hp = (int32_t)((double)max_hp * 0.5);
                    } else if (g_Server->difficulty == DIFFICULTY_HARD) {
                        max_hp = (int32_t)((double)max_hp * 1.5);
                    }
                }

                if (unit->hp_max == max_hp && unit->typeId != unit_type::boss_a && unit->typeId != unit_type::boss_b) {
                    mask &= UNIT_UPD_MONSTER_STD_HP_MASK;
                }
            }
        }
    } else { // Humanoid units
        if (unit->pOwner != player) {
            uint16_t tid = unit->typeId;
            if (tid < unit_type::hero_fighter_male || tid > unit_type::humanoid_last) {
                mask &= UNIT_UPD_OTHER_HERO_MASK;
            }
            #ifdef A2SERVER_PATCH
            // bug_0001: unit->pOwner may be null here - skip the vision-share
            // check (keep the full mask) instead of crashing
            else if (g_World != nullptr && unit->pOwner != nullptr) {
            #else
            else if (g_World != nullptr) {
            #endif
                if ((g_World->diplomacy.diplomacy[unit->pOwner->player_id][player->player_id] & DIPLO_VISION_SHARE) == 0) {
                    mask &= UNIT_UPD_OTHER_HERO_MASK;
                }
            }
        }
        if (g_Server->multiplayer != 0) {
            mask &= ~UNIT_UPD_SERVER_ID;
        }
    }

    if (unit->mp_max == 0) {
        mask &= ~UNIT_UPD_MP;
    }

    // --- Pack fields into 6D1180 based on flags bits ---

    // bit 0x1: hp (uint16)
    if (mask & UNIT_UPD_HP) {
        pkt->PutWord(UNIT_UPD_HP, unit->hp);
    }

    // bit 0x2: mp (uint16)
    if (mask & UNIT_UPD_MP) {
        pkt->PutWord(UNIT_UPD_MP, unit->mp);
    }

    // bit 0x4: 5 skill bytes (humanoids only)
    if (is_humanoid != 0) {
        if (mask & UNIT_UPD_SKILL_LEVELS) {
            for (int i = 0; i < 5; ++i) {
                pkt->PutByte(UNIT_UPD_SKILL_LEVELS, unit->hit_values.skill_levels[i+1]);
            }
        }
    }

    // bit 0x8: hp_max and mp_max
    if (mask & UNIT_UPD_HP_MP_MAX) {
        pkt->PutWord(UNIT_UPD_HP_MP_MAX, unit->hp_max);
        pkt->PutWord(UNIT_UPD_HP_MP_MAX, unit->mp_max);
    }

    // bit 0x10: position
    if (mask & UNIT_UPD_DIR_POS) {
        uint8_t value = unit->eye->GetFacingHalf();
        pkt->PutByte(UNIT_UPD_DIR_POS, value);

        if (unit->position->IsAtCellCenter() == 0) {
            uint16_t position1 = unit->eye->position1;
            pkt->PutByte(UNIT_UPD_DIR_POS, static_cast<uint8_t>(position1));
            pkt->PutByte(UNIT_UPD_DIR_POS, static_cast<uint8_t>(position1 >> 8));
        } else {
            pkt->PutByte(UNIT_UPD_DIR_POS, unit->position->GetX());
            pkt->PutByte(UNIT_UPD_DIR_POS, unit->position->GetY());
        }
    }

    // bit 0x20: type ID and face.
    if (mask & UNIT_UPD_TYPE_FACE) {
        pkt->PutByte(UNIT_UPD_TYPE_FACE, (uint8_t)unit->typeId);
        pkt->PutByte(UNIT_UPD_TYPE_FACE, (uint8_t)unit->face);
    }

    // bit 0x40: player ID (or 1 if no owner).
    if (mask & UNIT_UPD_OWNER) {
        pkt->PutByte(UNIT_UPD_OWNER, unit->pOwner ? unit->pOwner->player_id : 1);
    }

    // bit 0x80: server ID.
    if (mask & UNIT_UPD_SERVER_ID) {
        pkt->PutWord(UNIT_UPD_SERVER_ID, unit->server_id);
    }

    // bit 0x10000: attack.
    if (mask & UNIT_UPD_ATTACK) {
        pkt->PutWord(UNIT_UPD_ATTACK, unit->hit_values.attack);
    }

    // bit 0x20000: physical damage.
    if (mask & UNIT_UPD_HAND_DAMAGE) {
        uint8_t s1 = unit->hit_values.hand_damage_min
                   + unit->hit_values.some_damage_min
                   + unit->hit_values.some_damage2_min;
        pkt->PutByte(UNIT_UPD_HAND_DAMAGE, s1);

        uint8_t s2 = unit->hit_values.hand_damage_spread
                   + unit->hit_values.some_damage_spread
                   + unit->hit_values.some_damage2_spread;
        pkt->PutByte(UNIT_UPD_HAND_DAMAGE, s2);
    }

    // bit 0x4000: defense.
    if (mask & UNIT_UPD_DEFENSE) {
        pkt->PutWord(UNIT_UPD_DEFENSE, unit->protections.defense);
    }

    // bit 0x8000: absorption.
    if (mask & UNIT_UPD_ABSORB) {
        pkt->PutByte(UNIT_UPD_ABSORB, static_cast<uint8_t>(unit->protections.absorption));
    }

    // bit 0x40000: scan_range.
    if (mask & UNIT_UPD_SCAN_RANGE) {
        pkt->PutWord(UNIT_UPD_SCAN_RANGE, unit->scan_range);
    }

    // bit 0x2000: carrying weight.
    if (mask & UNIT_UPD_CARRIED_WEIGHT) {
        pkt->PutWord(UNIT_UPD_CARRIED_WEIGHT, unit->carrying_weight_100g);
    }

    // bit 0x1000000: magic protection.
    if (mask & UNIT_UPD_MAGIC_PROTECT) {
        for (int i = 0; i < 5; ++i) {
            pkt->PutByte(UNIT_UPD_MAGIC_PROTECT, static_cast<uint8_t>(unit->protections.magic_protections[i + 1]));
        }
    }
    
    // bit 0x2000000: battle stats (owner only).
    if (mask & UNIT_UPD_KILL_STATS) {
        Player* owner = unit->pOwner;
        if (g_Server->multiplayer == 0 || owner == nullptr || player != owner || owner->main_unit != unit) {
            mask &= ~UNIT_UPD_KILL_STATS;
        } else {
            pkt->PutInt(UNIT_UPD_KILL_STATS, owner->monster_kills);
            pkt->PutInt(UNIT_UPD_KILL_STATS, owner->player_kills);
            pkt->PutInt(UNIT_UPD_KILL_STATS, owner->deaths);
            pkt->PutInt(UNIT_UPD_KILL_STATS, owner->frags);
        }
    }

    // bit 0x80000: speed.
    if (mask & UNIT_UPD_SPEED) {
        pkt->PutByte(UNIT_UPD_SPEED, static_cast<uint8_t>(unit->speed));
    }

    // bit 0x100000: spellbook bitmask + eye2 calibration byte
    if ((mask & UNIT_UPD_SPELLS) && unit->spell_book != nullptr) {
        pkt->PutInt(UNIT_UPD_SPELLS, unit->spell_book->CalcSpellMask());
        pkt->PutByte(UNIT_UPD_SPELLS, unit->eye2->spell_id);
    }

    // bits 0x100..0x1000: experience per sphere (humanoids only).
    if (is_humanoid) {
        Humanoid* h = static_cast<Humanoid*>(unit);
        if (mask & UNIT_UPD_EXP_SPHERE0) {
            pkt->PutInt(UNIT_UPD_EXP_SPHERE0,  h->experience_per_sphere[0]);
        }
        if (mask & UNIT_UPD_EXP_SPHERE1) {
            pkt->PutInt(UNIT_UPD_EXP_SPHERE1,  h->experience_per_sphere[1]);
        }
        if (mask & UNIT_UPD_EXP_SPHERE2) {
            pkt->PutInt(UNIT_UPD_EXP_SPHERE2,  h->experience_per_sphere[2]);
        }
        if (mask & UNIT_UPD_EXP_SPHERE3) {
            pkt->PutInt(UNIT_UPD_EXP_SPHERE3,  h->experience_per_sphere[3]);
        }
        if (mask & UNIT_UPD_EXP_SPHERE4) {
            pkt->PutInt(UNIT_UPD_EXP_SPHERE4, h->experience_per_sphere[4]);
        }
    } else {
        mask &= 0xffffe0ff;
    }

    // bit 0x20000000: base stats.
    if (mask & UNIT_UPD_BASE_STATS) {
        // Effective body/reaction/mind/spirit.
        pkt->PutByte(UNIT_UPD_BASE_STATS, static_cast<uint8_t>(unit->body));
        pkt->PutByte(UNIT_UPD_BASE_STATS, static_cast<uint8_t>(unit->reaction));
        pkt->PutByte(UNIT_UPD_BASE_STATS, static_cast<uint8_t>(unit->mind));
        pkt->PutByte(UNIT_UPD_BASE_STATS, static_cast<uint8_t>(unit->spirit));
        // Base values: effective minus equipment bonus.
        pkt->PutByte(UNIT_UPD_BASE_STATS, static_cast<uint8_t>(unit->body     - unit->equipment_extra.body));
        pkt->PutByte(UNIT_UPD_BASE_STATS, static_cast<uint8_t>(unit->reaction - unit->equipment_extra.reaction));
        pkt->PutByte(UNIT_UPD_BASE_STATS, static_cast<uint8_t>(unit->mind     - unit->equipment_extra.mind));
        pkt->PutByte(UNIT_UPD_BASE_STATS, static_cast<uint8_t>(unit->spirit   - unit->equipment_extra.spirit));
    }

    // bit 0x80000000: unit name (24 chars)
    if (mask & UNIT_UPD_NAME) {
        int nlen = unit->name.GetLength();
        if (nlen == 0) {
            mask &= ~UNIT_UPD_NAME;
        } else {
            char* dst = (char*)pkt->data + pkt->data_offset;
            strncpy(dst, unit->name, 24);
            dst[23] = 0;
            pkt->data_offset += 24; // 24 + terminator that already nulled
            pkt->flags_mask |= 0x80000000u;
        }
    }

    // bit 0x800000: enchantments.
    bool has_enchantments = mask & UNIT_UPD_EFFECTS;
    if (has_enchantments) {
        pkt->flags_mask |= UNIT_UPD_EFFECTS;
    }

    // Send the packet.
    if (mask & UNIT_UPD_SEND_GATE_MASK) {
        this->QueuePacketSend(pkt);
    }

    if (mask & UNIT_UPD_SEND_EQUIPMENT) {
        this->SendHumanoidProps(unit, player, props_flags);
    }

    // Owner-only: send inventory.
    if (mask & UNIT_UPD_SEND_INVENTORY && unit->pOwner == player) {
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

    if ((needs_refresh && (unit->unit_attrs & Unit::ATTR_PHASED_OUT) != 0) || (mask & 0x40000000) != 0) {
        g_NetStru1_main.SendUnitVisibility(unit, (unit->unit_attrs & Unit::ATTR_PHASED_OUT) ? 1 : 0, player);
    }

    // Mark unit as not needing the refresh in the "field_x18" bitmask.
    unit->ClearRefreshFor(player);

    // Clear per-player pending bits for players in the 0x10..0x1F range.
    if (0x10 <= player->player_id && player->player_id < 0x20) {
        unit->something_per_player[player->player_id - 16] = 0;
    }
}

#if defined(A2SERVER_PATCH) && !defined(A2CLIENT)
// Ported from srvmgr/srvmgr.cpp:1192 (check_sig_c_s).
bool IsAllowedClientPacketId(uint8_t c) {
    static const PacketId kAllowedClientPackets[] = {
        ENTER_GAME, JOIN_MISSION, PLAYER_LEAVE, REQUEST_SAVE_GAME, REQUEST_LOAD_GAME, CLIENT_SHUTDOWN,
        GROUP_STOP, GROUP_GATHER, GROUP_GO_HOME, GROUP_MOVE, GROUP_GUARD, GROUP_AGGRESSIVE,
        GROUP_ATTACK, GROUP_ATTACK_MOVE, GROUP_ATTACK_TARGET, GROUP_DEFEND, GROUP_PATROL,
        GROUP_CAST_UNIT, GROUP_CAST_POS, GROUP_PICKUP, ITEM_OPERATION, DROP_GOLD,
        GROUP_ENTER_BUILDING, SCROLL_CAST_UNIT, SCROLL_CAST_POS,
        ENTER_SHOP, SHOP_BUY, SHOP_SELL, SHOP_TAKE_BACK, SHOP_CLOSE,
        ENTER_INN, REQUEST_MAP_INFO, EXIT_INN, REQUEST_FILE_CHUNK,
        CHEAT_MONEY, CHEAT_SHOP,
        SET_DIPLOMACY, SET_PARAMETER, JOIN_MAP, CHEAT_SUMMON,
        REQUEST_UNITS_RESEND, REQUEST_RESPAWN, HURT_SELF, GROUP_STOP_AFK,
        CHAT, ANNOUNCEMENT, REQUEST_UNIT_UPDATE, LATENCY, HAT_TICKET, SERVER_TICK,
    };
    for (size_t i = 0; i < sizeof(kAllowedClientPackets) / sizeof(kAllowedClientPackets[0]); ++i) {
        if (c == kAllowedClientPackets[i]) {
            return true;
        }
    }
    return false;
}
#endif

// DecodePacket
// 51e7fc (client: 50f04e)
Packet* NetStru1::DecodePacket(uint8_t cmd, NetStru2* ns2) {
#if defined(A2SERVER_PATCH) && !defined(A2CLIENT)
    if (this != &HatConnector && ns2 != nullptr && ns2->is_local_player == 0 && !IsAllowedClientPacketId(cmd)) {
        Player* bad_player = (g_PlayersList != nullptr && ns2->player_id != 0)
            ? g_PlayersList->FindById(ns2->player_id)
            : nullptr;
        CString inv_msg;
        inv_msg.Format("Invalid packet '%.2X' from '%s' (disconnecting)",
                       static_cast<unsigned int>(cmd),
                       bad_player != nullptr ? (const char*)bad_player->login : (const char*)ns2->buf);
        LogMessage(inv_msg);
        if (bad_player != nullptr) {
            g_NetStru1_main.AnnouncePlayer(bad_player);
        }
        ns2->ReturnBuffers();
        this->DisconnectClient(ns2);
        return nullptr;
    }
#endif
    Packet* pkt;

    switch (cmd) {
    case ENTER_GAME:
        pkt = &PacketEnterGame::Inst;
        pkt->ReadFrom(ns2);
        break;

    case MISSION_ENTERED: case SERVER_SHUTDOWN: case ENTER_BUILDING: case LEAVE_BUILDING:
        pkt = &Packet::Inst;
        pkt->ReadFrom(ns2);
        break;

    case JOIN_MISSION: case JOIN_ERROR: case REQUEST_MAP_INFO: case REQUEST_FILE_CHUNK: case HAT_SCORE_LIST: case HAT_PICK_CHAR:
    case HAT_CHAR_REQUEST: case HAT_TICKET: case HAT_LOGIN_OK: case HAT_JOIN_OK: case HAT_PLAYER_CHECK:
        pkt = &Packet2Dwords::Inst;
        pkt->ReadFrom(ns2);
        break;

    case PLAYER_LEAVE: case CLIENT_SHUTDOWN: case UNKNOWN_0A: case REQUEST_UNITS_RESEND: case REQUEST_RESPAWN: case HURT_SELF:
    case HAT_SCORE_CLEAR: case HAT_LOGOUT: case UNKNOWN_D_6: case HAT_SESSION_FAIL: case UNKNOWN_DC:
        pkt = &Packet::Inst;
        pkt->ReadFrom(ns2);
        break;

    case START_GAME: case REQUEST_SAVE_GAME: case REQUEST_LOAD_GAME: case HAT_JOIN_NAME: case HAT_SCORE_UPDATE: case PLAYER_ANNOUNCED:
    case MAP_INFO: case HAT_CREDENTIALS: case HAT_LOGIN: case HAT_LOBBY_INFO: case HAT_CHAR_RETURN:
        pkt = &PacketJoin::Inst;
        pkt->ReadFrom(ns2);
        break;
        
#if defined(A2CLIENT_PATCH) || defined(A2SERVER_PATCH)
    case HAT_AUTH:
        pkt = &PacketJoin::Inst;
        pkt->id = cmd;
        pkt->ReadFrom(ns2);
        break;
#endif

    default:
        LogMessage(CString("Drv: unknown cmd type!"));
        pkt = &Packet::Inst;
        pkt->ReadFrom(ns2);
        break;

    case JOIN_DONE: case GAME_SPEED: case KILL_STAT: case LATENCY: case HAT_ERROR:
        pkt = &PacketDword::Inst;
        pkt->ReadFrom(ns2);
        break;

    case UNKNOWN_0F: case CHEAT_SUMMON: case CHAT: case PLAYER_INFO: case ANNOUNCEMENT:
        pkt = &PacketJoin::Inst;
        pkt->id = cmd;
        pkt->ReadFrom(ns2);
        break;

    case GROUP_STOP: case GROUP_GATHER: case GROUP_GO_HOME: case GROUP_GUARD: case GROUP_AGGRESSIVE: case GROUP_STOP_AFK:
        pkt = &PacketGroupOrder::Inst;
        pkt->ReadFrom(ns2);
        pkt->order_mode = 1;
        break;

    case GROUP_MOVE: case GROUP_ATTACK_MOVE: case GROUP_DEFEND: case GROUP_PATROL: case GROUP_CAST_POS: case GROUP_PICKUP: case GROUP_ENTER_BUILDING: case SCROLL_CAST_POS:
        pkt = &PacketGroupOrder::Inst;
        pkt->ReadFrom(ns2);
        pkt->order_mode = 2;
        break;

    case GROUP_ATTACK: case GROUP_ATTACK_TARGET: case GROUP_CAST_UNIT: case SCROLL_CAST_UNIT:
        pkt = &PacketGroupOrder::Inst;
        pkt->ReadFrom(ns2);
        pkt->order_mode = 3;
        break;

    case ITEM_OPERATION:
        pkt = &PacketItemTransfer::Inst;
        pkt->ReadFrom(ns2);
        break;

    case DROP_GOLD: case EXIT_INN: case UNKNOWN_3D: case CHEAT_MONEY: case CHEAT_SHOP: case UNKNOWN_40:
    case SET_PARAMETER: case JOIN_MAP: case OPEN_SHOP: case OPEN_INN: case TEAM_NOTIFY: case KILL_NOTICE:
    case MAP_REVEAL: case GAME_START:
        pkt = &Packet2Dwords::Inst;
        pkt->ReadFrom(ns2);
        break;

    case ENTER_SHOP: case SHOP_BUY: case SHOP_SELL: case SHOP_TAKE_BACK: case SHOP_CLOSE: case ENTER_INN:
    case UNIT_SHOWN: case TOKEN_REMOVE: case UNIT_HIDDEN:
        pkt = &PacketWord::Inst;
        pkt->ReadFrom(ns2);
        break;

    case SET_DIPLOMACY: case EFFECT_MULTI_TARGET: case EFFECT_MULTI_FROM: case TERRAIN_DATA: case DIPLOMACY_STATE: case PLAYER_QUESTS: case INN_QUESTS:
        pkt = &PacketTerrain::Inst;
        pkt->ReadFrom(ns2);
        break;

    case SERVER_TICK:
        pkt = &PacketDword::Inst;
        pkt->ReadFrom(ns2);
        ns2->ReleaseNetBuf();
        break;

    case UNKNOWN_65: case MONEY_UPDATE: case PLAYER_LEFT: case VIEW_POSITION: case UNKNOWN_AC: case UNKNOWN_AD:
    case MISSION_RESULT: case MISSION_COMPLETE: case GAME_EVENT:
        pkt = &Packet2Dwords::Inst;
        pkt->ReadFrom(ns2);
        break;

    case UNIT_MOVE: case UNIT_TURN: case UNIT_ATTACK:
        pkt = &PacketUnitAction::Inst;
        pkt->ReadFrom(ns2);
        break;

    case UNIT_CHANGE_4: case UNIT_CHANGE_1: case UNIT_CHANGE_2: case UNIT_CHANGE_3:
        pkt = &PacketUnitUpdate::Inst;
        pkt->id = cmd;
        pkt->ReadFrom(ns2);
        break;

    case RANGED_ATTACK:
        pkt = &PacketRangedAttack::Inst;
        pkt->ReadFrom(ns2);
        break;

    case UNIT_DAMAGED:
        pkt = &PacketUnitHp::Inst;
        pkt->ReadFrom(ns2);
        break;

    case ITEM_LIST:
        pkt = &PacketUnitStateVec::Inst;
        pkt->ReadFrom(ns2);
        break;

    case SACK_STATE:
        pkt = &PacketSack::Inst;
        pkt->ReadFrom(ns2);
        break;

    case BUILDING_HP:
        pkt = &PacketBuildingHp::Inst;
        pkt->ReadFrom(ns2);
        break;

    case EFFECT_MOVE: case EFFECT_FROM:
        pkt = &PacketMoveBroadcast::Inst;
        pkt->ReadFrom(ns2);
        break;

    case AOE_ZONE:
        pkt = &PacketAoeZone::Inst;
        pkt->ReadFrom(ns2);
        break;

    case EFFECT_HANG: case EFFECT_GONE:
        pkt = &PacketEffect::Inst;
        pkt->ReadFrom(ns2);
        break;

    case ENEMY_EQUIPMENT:
        pkt = &PacketUnitEquipment::Inst;
        pkt->ReadFrom(ns2);
        break;

    case CREATURE_KNOWLEDGE: case TEAM_STATE: case HAT_CONNECT_SERVER: case SERVER_LIST: case HAT_CHAR_LIST: case HAT_CHAR_DATA:
    case HAT_RAW_DATA: case HAT_JOIN_FAIL: case HAT_SESSION_SETUP: case HAT_PLAYER_JOIN: case UNKNOWN_DE: case HAT_MESSAGE:
        pkt = &PacketData::Inst;
        pkt->ReadFrom(ns2);
        break;

    case REQUEST_UNIT_UPDATE:
        pkt = &PacketData::Inst;
        pkt->ReadFrom(ns2);
        break;

    case FILE_CHUNK:
        pkt = &PacketFileChunk::Inst;
        pkt->ReadFrom(ns2);
        break;
    }

    pkt->id = cmd;

    if (this->make_statistics == 0) {
        pkt->from_player_id = 0;
    } else if (ns2->player_id == 0) {
        pkt->from_player_id = static_cast<uint16_t>((ns2->uid & UNCLAIMED_CLIENT_UID_MASK) | UNCLAIMED_CLIENT_FLAG);
    } else {
        pkt->from_player_id = ns2->player_id;
    }

    return pkt;
}

// 518a23 (client: 50a498) - receive packet from client
Packet* NetStru1::ReceivePacket(NetStru2* cli) {
    if (cli->GetBuffersPktNum() == 0) {
        return nullptr;
    }

    uint8_t id = 0;
#ifdef A2SERVER_PATCH
    if (!cli->ReadData(&id, 1)) {
        return nullptr;
    }
#else
    cli->ReadData(&id, 1); //read pkt id
#endif
    return this->DecodePacket(id, cli);
}

// 518980 (client: 50a3fc) - iterate over active connections and try receive packet
Packet* NetStru1::ReceiveAnyPacket() {
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




// 517e91 (client: 509b9b)
void NetStru1::AddTailNet2(NetStru2*& net2) {
    EnterCriticalSection(&this->critical_section);
    this->disconnect_list.AddTail(net2);
    LeaveCriticalSection(&this->critical_section);
}

// 517b00 (client: 509a73)
void NetStru1::AddTailFreeNet3(NetStru3* net3) {
    EnterCriticalSection(&this->critical_section2);
    NetStru1* pool = this;
    if (this->linked_hl && make_statistics == 0) {
        pool = this->linked_hl;
    }
    pool->free_net3.AddHead(net3);
    LeaveCriticalSection(&this->critical_section2);
}

// 51755c (client: 509889)
void NetStru1::ProcessNewConnects() {
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

// 51725b (client: 509799)
void NetStru1::ProcessDisconnectList() {
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

// 51800f (client: 509be7)
void NetStru1::ProcessConnections() {
    this->ProcessNewConnects();
    this->ProcessDisconnectList();
    if (this->lldriver) {
        this->lldriver->CleanupAllInvalid();
    }
}

// 5170b6 (client: 5096a2)
void NetStru1::DisconnectClient(NetStru2* cli) {
    if (cli == this->local_client) {
        this->AddTailNet2(this->local_client);
        this->local_client = nullptr;
    } else {
#ifdef A2SERVER_PATCH
        if (this->lldriver != nullptr) {
            this->lldriver->DisconnectClient(cli->uid);
        }
#else
       this->lldriver->DisconnectClient(cli->uid);
#endif
    }
}

// 51684b (client: 509343)
NetStru1::NetStru1(int param) {
    this->lldriver = nullptr;
    this->linked_hl = nullptr;
    this->local_client = nullptr;
    this->make_statistics = param;
    InitializeCriticalSection(&this->critical_section);
    InitializeCriticalSection(&this->critical_section2);
    this->packer2_packed_bytes = 0;
    this->packer2_trained_bytes = 0;
    this->packer1_packed_bytes = 0;
    this->packer1_trained_bytes = 0;
    this->compression_mode = 0;
    this->compression_type = 1;
    memset(this->stat_pkt_num, 0, sizeof(this->stat_pkt_num));
    memset(this->stat_pkt_size, 0, sizeof(this->stat_pkt_size));
}

// 516b44 (client: 5094ca)
NetStru1::~NetStru1() {
    if (this->local_client != nullptr) {
        this->DisconnectClient(this->local_client);
    }

    this->ProcessNewConnects();
    this->ProcessDisconnectList();

    //inline 51710f
    EnterCriticalSection(&this->critical_section2);
    while (!this->free_net3.IsEmpty()) {
        NetStru3* buf = this->free_net3.RemoveHead();
        delete buf;
    }
    LeaveCriticalSection(&this->critical_section2);

    DeleteCriticalSection(&this->critical_section);
    DeleteCriticalSection(&this->critical_section2);

    this->lldriver = nullptr;
    this->linked_hl = nullptr;
    this->make_statistics = 0;
    this->packer2_packed_bytes = 0;
    this->packer2_trained_bytes = 0;
    this->packer1_packed_bytes = 0;
    this->packer1_trained_bytes = 0;
    this->compression_mode = 0;
    this->compression_type = 1;
}


// 51fc72 (client: 50fe58)
void NetStru1::OnClientConnect(NetStru2* cli) {
    if (this != &HatConnector) {
        this->client_stat[cli->uid] = new ConnStatInfo;

        if (this->lldriver && lldriver->provider != PROVIDER_HAT_TCP) {
            cli->is_claimed = 1;
        } else {
            cli->is_claimed = 0;
        }
    }
}

// 51f561 (client: 50fc92)
void NetStru1::OnClientDisconnect(NetStru2* cli) {
    if (this == &HatConnector) {
        LogMessage("Connection with hat has been lost.");
        if (g_Server) {
            g_Server->hat_disconnect_tick = GetTickCount();
        }
        return;
    }

    if (this->lldriver && lldriver->is_server != 0 && lldriver->provider == PROVIDER_MODEM) {
        lldriver->RestartModemServerDp();
    }

#ifdef A2SERVER_PATCH
    if (g_Server && g_PlayersList) {
#else
    if (g_Server) {
#endif
        Player* pl = g_PlayersList->FindById(cli->player_id);
        if (pl && pl->drop_deadline_tick16 == 0) {
#ifdef A2CLIENT
            // The client variant has no AI-warning branch and no gameType/teams
            // deadline conditions (50fc92).
            pl->Deactivate();

            if (this->lldriver && this->lldriver->provider == PROVIDER_HAT_TCP) {
                pl->drop_deadline_tick16 = g_Server->tick16 + 300;
            }

            if (!MapStuff_Instance && g_Server->multiplayer) {
                this->SendInfoPacket(PLAYER_LEFT, pl->player_id, 0, nullptr);
                LogMessage("Player " + pl->name + " disconnected.");

                POSITION pos = g_PlayersList->list.Find(pl);
                if (pos != nullptr) {
                    g_PlayersList->list.RemoveAt(pos);
                }

                delete pl;

                g_Server->UpdateHatAnnounce();
            }
#else
            if (pl->is_ai == 1 || pl->player_id <= 15) {
                LogMessage("Warning: trying to disconnect AI player " + pl->name);
            } else {
                LogMessage("Player " + pl->name + " has disconnected.");

                pl->Deactivate();

                if (lldriver && lldriver->provider == PROVIDER_HAT_TCP &&
                    g_ServerConfig.gameType != GAME_TYPE_DEATHMATCH &&
                    g_ServerConfig.gameType != GAME_TYPE_ARENA &&
                    g_Server->game_started == 0) {
                    if (g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
                        if (g_PlayersList->AreTeamsStarted()) {
                            pl->drop_deadline_tick16 = MAXINT32;
                        }
                    } else {
                        pl->drop_deadline_tick16 = g_Server->tick16 + 300;
                    }
                }
                if (!MapStuff_Instance && g_Server->multiplayer) {
                    this->SendInfoPacket(PLAYER_LEFT, pl->player_id, 0, nullptr);
                    LogMessage("Player " + pl->name + " has disconnected and leaved game.");

                    POSITION pos = g_PlayersList->list.Find(pl);
                    if (pos != nullptr) {
                        g_PlayersList->list.RemoveAt(pos);
                    }

                    delete pl;

                    g_Server->UpdateHatAnnounce();
                }
            }
#endif
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


// 517c99 (client: 509ad7)
NetStru2* NetStru1::AllocClientBufManager(uint32_t uid) {
    EnterCriticalSection(&this->critical_section);

    NetStru2* mgr = new NetStru2();
    mgr->uid = uid;
    mgr->SetDrivers(this, this->lldriver);

    this->new_connects.AddTail(mgr);
    LeaveCriticalSection(&this->critical_section);

    return mgr;
}


// 5177f5 (client: 50994f)
NetStru3* NetStru1::GetFreeNet3() {
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

// 518544 (client: 50a128)
NetStru2* NetStru1::GetClientByPlayerID(uint16_t player_id) {
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

// 5185d5 (client: 50a1ab)
NetStru2* NetStru1::GetClientByLowUid(uint32_t low_uid) {
    uint32_t luid = low_uid & UNCLAIMED_CLIENT_UID_MASK;
    for (POSITION it = this->active_connects.GetHeadPosition(); it != nullptr;) {
        NetStru2* cli = this->active_connects.GetNext(it);
        if ((cli->uid & UNCLAIMED_CLIENT_UID_MASK) == luid) {
            return cli;
        }
    }
    return nullptr;
}

// 51eff8 (client: 50f79c)
void NetStru1::AddStat1(NetStru2* client, int32_t value) {
    ConnStatInfo* stat = nullptr;
    this->client_stat.Lookup(client->uid, stat);

    if (stat) {
        stat->window_bytes += value;
    }
}

// 51f0bb (client: 50f7dd)
void NetStru1::AddPacketStatistic(Packet* pkt) {
    this->stat_pkt_num[pkt->id]++;
    this->stat_pkt_size[pkt->id] += pkt->GetDataSize();
}

// 5188db (client: 50a3b1) - 518927 variant without flag check because in original it's used once and not optimal, because iterate all and do not do anything
void NetStru1::SendAllData() {
    for (POSITION it = this->active_connects.GetHeadPosition(); it != nullptr;) {
        NetStru2* cli = this->active_connects.GetNext(it);
        if (cli) {
            cli->SendData();
        }
    }
}

// 51d88d (client: 50e446)
void NetStru1::SendPacket_64(uint32_t val, uint16_t player_id) {
    PacketDword& pkt = PacketDword::Inst;
    pkt.id = SERVER_TICK;
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

// 517ff9 (client: 509bd1)
void NetStru1::SetLLDriver(CLlDriver* drv) {
    this->lldriver = drv;
}

// 51703e (client: 50963b)
void NetStru1::SetLinkedHLDriver(NetStru1* hl) {
    this->linked_hl = hl;
    if (this->local_client) {
        this->DisconnectClient(this->local_client);
    }
    this->local_client = this->AllocClientBufManager(-1);
    this->local_client->is_local_player = 1;
    this->local_client->compression_type = 0;
}

// 5186cd (client: 50a218)
void NetStru1::QueuePacketSend(Packet* pkt) {

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
                if (pkt->id == SERVER_TICK) {
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
            if (pkt->id == SERVER_TICK) {
                cli->out_buffers[cli->out_buff_id].pkt_num++;
            }
        }
    }

    if (this->make_statistics != 0 && pkt->id == ITEM_LIST) {
        //DAT_0070b410 += (GetTickCount() - stime);
    }

    if (this->make_statistics == 0 && pkt->id != SERVER_TICK) {
        this->SendPacket_64(1, 0);
    }
}


// 518a6b (client: 50a4de)
uint32_t NetStru1::GetClientsPktNum() {
    uint32_t sum = 0;
    for (POSITION it = this->active_connects.GetHeadPosition(); it != nullptr;) {
        NetStru2* cli = this->active_connects.GetNext(it);
        if (cli) {
            sum += cli->GetBuffersPktNum();
        }
    }
    return sum;
}

// 518ac9 (client: 50a53b)
int NetStru1::IsActive() {
    if (!this->lldriver) {
        return this->linked_hl != nullptr;
    } else if (this->lldriver->listen_socket.is_in_use == 1 || this->linked_hl != nullptr) {
        return 1;
    }
    return 0;
}


// 41f520 (client: 420af0)
NetStru2* NetStru1::GetFirstClient() {
    if (this->active_connects.IsEmpty()) {
        return nullptr;
    }

    return this->active_connects.GetHead();
}




// 515ef3 (client: 508c6a)
int NetStru2::WriteData(void* buf, uint32_t size) {
    while(true) {
        int32_t avail = MAX_SEGMENT_PAYLOAD - this->out_buffers[this->out_buff_id].datasz;
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

// 515f9c (client: 508d13)
int NetStru2::ReadData(void* buf, uint32_t size) {
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
#ifdef A2SERVER_PATCH
    if (size > 0) {
        memset(buf, 0, size);
    }
#endif
    LeaveCriticalSection(&this->critical_section);
    return 0;
}



// 51682d (client: 509325)
void NetStru2::SetDrivers(NetStru1* HlDriver, CLlDriver* LlDriver) {
    this->hldriver = HlDriver;
    this->lldriver = LlDriver;
}

// 5161bf (client: 508e04)
int NetStru2::ReceiveData(NetStru3* buffer) {
    EnterCriticalSection(&this->critical_section);
#if defined(A2SERVER_PATCH) && !defined(A2CLIENT)
    constexpr INT_PTR kMaxQueuedRecvBuffers = 4096;
    if (this->is_local_player == 0 && this->hldriver != &NetStru1::HatConnector &&
        this->received_buffers.GetCount() >= kMaxQueuedRecvBuffers) {
        LeaveCriticalSection(&this->critical_section);
        if (this->hldriver != nullptr) {
            this->hldriver->AddTailFreeNet3(buffer);
        }
        ReportWarning("CBufferManager::ReceiveData().\nClient receive queue overflow.\n");
        return 0;
    }
#endif

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

// 516408 (client: 508f33)
int NetStru2::SendData() {
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

    // inline 515eb2
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

// 515831 (client: 508871)
NetStru2::NetStru2() {
    this->gap_0x2a0 = 0;
    this->gap_0x2a4 = 0;
    this->is_claimed = 0;
    this->gap_0x2ac = 0;
    this->gap_0x2b4 = -1;
    this->str.Empty();
    this->gap_0x2b8 = 0;
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

// 515c27 (client: 508acc)
NetStru2::~NetStru2() {
    this->ReturnBuffers();
    DeleteCriticalSection(&this->critical_section);
    this->is_local_player = 0;
    this->player_id = 0;
    this->uid = 0;
    this->hldriver = nullptr;
    this->lldriver = nullptr;
    this->buf[0] = 0;
}

// 515d9d (client: 508b9a) - return buffers to High-level driver
void NetStru2::ReturnBuffers() {
    if (!this->hldriver) {
        return;
    }

    EnterCriticalSection(&this->critical_section);

    while (!this->received_buffers.IsEmpty()) {
        this->hldriver->AddTailFreeNet3( this->received_buffers.RemoveHead() );
    }

    LeaveCriticalSection(&this->critical_section);
}


// 5167a5 (client: 5092a4)
void NetStru2::ReleaseNetBuf() {
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

// 51670a (client: 509208) - Get received_buffers summary of pkt_num
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




// 5156f6 (client: 508736)
void NetStru3::Clear() {
    this->datasz = 0;
    this->readpos = 0;
    this->pkt_num = 0;
    this->timestamp = 0;
    this->timestamp2 = 0;
    this->csize = 0;
}

// 5155c0 (client: 50863e)
NetStru3::NetStru3() {
    memset(this->full_data, 0, sizeof(this->full_data));
    this->pktid = 0;
    this->cmode = 0;
    this->pkt_num = 0;
    this->datasz = -1;
    this->readpos = -1;
    this->timestamp = 0;
    this->timestamp2 = 0;
}

// 515694 (client: 5086f3)
NetStru3::~NetStru3() {
    this->datasz = -1;
    this->readpos = -1;
    this->timestamp = 0;
    this->timestamp2 = 0;
    this->pktid = 0;
    this->cmode = 0;
    this->pkt_num = 0;
    memset(this->full_data, 0, sizeof(this->full_data));
}

// 515625 (client: 508684)
void NetStru3::CopyData(const NetStru3* src) {
    memcpy(this->full_data, &src->full_data, src->datasz + 8);
    this->datasz = src->datasz;
    this->readpos = src->readpos;
    this->timestamp = src->timestamp;
    this->timestamp2 = src->timestamp2;
}

// 51573e (client: 50877e)
int NetStru3::WriteToBuffer(void* data, uint32_t sz) {
    uint32_t newsz = this->datasz + sz;
    if (newsz >= 143) {
        return false;
    }

    memcpy(this->buf + this->datasz, data, sz);
    this->datasz = newsz;
    return true;
}

// 51579e (client: 5087de)
int NetStru3::ReadFromBuffer(void* out, uint32_t sz) {
    uint32_t newpos = this->readpos + sz;
    if (newpos > this->datasz) {
        return false;
    }

    memcpy(out, this->buf + this->readpos, sz);
    this->readpos = newpos;
    return true;
}


// 520510 (client: 51002a)
A2NetSock::A2NetSock() {
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

// 520684 (client: 510121)
A2NetSock::~A2NetSock() {
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

// 51fdf7 (client: 50ff67)
void NetSockLatency::AddLatency(uint32_t lat_time) {
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




// 5208ec (client: 510241)
CLlDriver::CLlDriver(GUID _appid, NetStru1* net1) {
    this->application_guid = _appid;
    this->hl_driver = net1;
    this->is_server = 0;
    this->guaranteed = 0;
    this->latency = 0xffffffff;
    this->max_connections = -1;
    this->session_lost = 0;
    this->provider = PROVIDER_NONE;
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

// 520ad3 (client: 510429)
CLlDriver::~CLlDriver() {

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

// 496e50 (client: 49a920)
void CLlDriver::SetHlDriver(NetStru1* drv) {
    this->hl_driver = drv;
}


// 5225d0 (client: 511d0c)
void CLlDriver::Close() {
    if (this->listen_socket.is_in_use) {
        if (this->provider == PROVIDER_HAT_TCP) {
            this->CloseTcp();
        } else {
            this->CloseDp();
        }
    }
    this->listen_socket.is_in_use = 0;
    this->is_server = 0;
}



// 523b01 (client: 5131ad)
void CLlDriver::CloseTcp() {
    this->CloseTcpSocket(&this->listen_socket);
    this->CloseTcpSocket(&this->ping_socket);
    if (this->is_server) {
        for (uint32_t i = 0; i < this->max_connections; i++) {
            this->CloseTcpSocket(this->connection_sockets + i);
        }
    }
}

// 523a16 (client: 5130cf)
void CLlDriver::CloseTcpSocket(A2NetSock* sock) {
    if (sock->is_in_use) {
        this->num_connections--;
    }

    sock->is_in_use = 0;
    if (sock->socket != INVALID_SOCKET) {
#ifdef A2TANGAR_HAT
        tangar_hat::CloseServerSocket(sock->socket);
#else
        closesocket(sock->socket);
#endif
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


// 525a09 (client: 514b64)
void CLlDriver::CloseDp() {
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

// 525bcd (client: 514d27)
void CLlDriver::CloseDpSock(A2NetSock* sock, int idx) {
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


// 520d8c (client: 510646)
CLlAddress* CLlDriver::AllocAddress() {
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

// 520c80 (client: 51053a)
void CLlDriver::FreeEnumAddresses() {
    if (this->enum_addresses) {
        free(this->enum_addresses);
        this->enum_addresses = nullptr;
        this->enum_addresses_num = 0;
    }
}


// 52182f (client: 5110c6)
int CLlDriver::EnumAddresses(CLlAddress** addrs, int* num) {
    this->FreeEnumAddresses();
    if (this->provider == PROVIDER_SERIAL) {
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
    } else if (this->provider == PROVIDER_MODEM) {
        this->EnumAddressesDp();
    } else if (this->provider == PROVIDER_HAT_TCP) {
        this->EnumAddressesTcp();
    } else {
        return 0;
    }

    *addrs = this->enum_addresses;
    *num = this->enum_addresses_num;
    return 1;
}


// 5214c5 (client: 510d5c)
void CLlDriver::EnumAddressesTcp() {
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

// 521200 (client: 510a97)
void CLlDriver::EnumAddressesDp() {
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

// 5211b2 (client: 510a50)
BOOL __stdcall CLlDriver::cbEnumAddress(REFGUID guidDataType, DWORD dwDataSize, LPCVOID lpData, LPVOID lpContext) {
    CLlDriver* drv = (CLlDriver*)lpContext;
    if (memcmp(&guidDataType, &DPAID_Modem, sizeof(GUID)) == 0) {
        CLlAddress* addr = drv->AllocAddress();
        strcpy(addr->name, (const char*)lpData);
    }
    return TRUE;
}




// 520e2e (client: 5106e8)
CLlNetSession* CLlDriver::AllocSession() {
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

// 520cc3 (client: 51057d)
void CLlDriver::FreeEnumSessions() {
    if (this->enum_sessions) {
        free(this->enum_sessions);
        this->enum_sessions = nullptr;
        this->enum_sessions_num = 0;
    }
}

// 52210d (client: 51184d)
int CLlDriver::EnumSessions(CLlNetSession** sessions, int* num, CBUPDATECALLBACK cbUpdate, uint32_t timeout) {
    this->FreeEnumSessions();

    if (this->provider == PROVIDER_HAT_TCP) {
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

// 522020 (client: 51176c)
BOOL __stdcall CLlDriver::cbEnumSessions(LPCDPSESSIONDESC2 lpThisSD, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext) {
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

    if (drv->provider == PROVIDER_MODEM || drv->provider == PROVIDER_SERIAL) {
        drv->SetEventNewSession();
    }

    return TRUE;
}






// 520ed0 (client: 51078a)
CLlConn* CLlDriver::AllocEnumConns() {
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

// 520d06 (client: 5105c0)
void CLlDriver::FreeEnumConns() {
    if (this->enum_conns) {
        free(this->enum_conns);
        this->enum_conns = nullptr;
        this->enum_conns_num = 0;
    }
}

// 5210ea (client: 510988)
void CLlDriver::EnumConnections(CLlConn** conns, int* num) {
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

// 52100e (client: 5108c8)
BOOL __stdcall CLlDriver::cbEnumConnections(LPCGUID lpguidSP, LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext) {
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



// 520f72 (client: 51082c)
CLlName* CLlDriver::AllocEnumPlayers() {
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

// 520d49 (client: 510603)
void CLlDriver::FreeEnumPlayers() {
    if (this->enum_players) {
        free(this->enum_players);
        this->enum_players = nullptr;
        this->enum_players_num = 0;
    }
}

// 52238e (client: 511ace)
void CLlDriver::EnumPlayers(CLlNetSession* ses, CLlName** names, int* num) {
    this->FreeEnumPlayers();
    if (this->dplay4) {
        this->dplay4->EnumPlayers(&ses->guid, cbEnumPlayes, this, DPENUMPLAYERS_SESSION);
    }
    *names = this->enum_players;
    *num = this->enum_players_num;
}

// 52233f (client: 511a7f)
BOOL __stdcall CLlDriver::cbEnumPlayes(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext) {
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


// 5219af (client: 511246)
void CLlDriver::Free() {
    if (this->listen_socket.is_in_use == 1) {
        this->Close();
    }

    if (this->provider != PROVIDER_NONE) {
        if (this->provider == PROVIDER_HAT_TCP) {
            this->FreeTcp();
        } else {
            this->FreeDp();
        }
        this->provider = PROVIDER_NONE;
    }
}

// 523611 (client: 420b70)
void CLlDriver::FreeTcp() {
    WSACleanup();
}

// 523d5a (client: 513406)
void CLlDriver::FreeDp() {
    if (this->dplay4) {
        this->dplay4->Release();
        this->dplay4 = nullptr;
    }
}





// 52194f (client: 5111e6)
int CLlDriver::ResetProvider(int prov) {
    this->Free();

    this->provider = (NetProvider)prov;
    if (prov == PROVIDER_HAT_TCP) {
        if (!this->CreateTcp()) {
            return 0;
        }
    } else {
        if (!this->CreateDp()) {
            return 0;
        }
    }
    this->provider = (NetProvider)prov;
    return 1;
}


// 52359b (client: 512c58)
int CLlDriver::CreateTcp() {
    WSADATA wsa;
    int32_t wsa_err = WSAStartup(0x101, &wsa);
    if (wsa_err != 0) {
#ifdef A2SERVER_PATCH
        Printf("[tcp:%s] CreateTcp: WSAStartup(0x101) failed: %s",
               TcpDriverLabel(this), FormatWinSockError(wsa_err).c_str());
#endif
        return 0;
    }

    if (wsa.wVersion == 0x101) {
        return 1;
    }

#ifdef A2SERVER_PATCH
    Printf("[tcp:%s] CreateTcp: unexpected Winsock version wVersion=0x%04x (wHighVersion=0x%04x, expected 0x0101)",
           TcpDriverLabel(this), wsa.wVersion, wsa.wHighVersion);
#endif
    WSACleanup();
    return 0;
}

// 523ccf (client: 51337b)
int CLlDriver::CreateDp() {
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


// 5224c4 (client: 511c00)
A2NetSock* CLlDriver::GetClientBySocketId(uint32_t uid) {
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


// 522318 (client: 511a58)
void CLlDriver::SetEventNewSession() {
    if (this->ev_new_session) {
        SetEvent(this->ev_new_session);
    }
}

// 52293a (client: 512078)
int CLlDriver::IsInUse(A2NetSock* sock) {
    if (this->listen_socket.is_in_use == 1) {
        return sock->is_in_use;
    } else {
        return 0;
    }
}

// 522966 (client: 5120a2) - GetClientBySocketId + IsInUse
int CLlDriver::IsUidInUse(uint32_t uid) {
    A2NetSock* sock = this->GetClientBySocketId(uid);
    if (sock == nullptr) {
        return 0;
    }
    return this->IsInUse(sock);
}



// 52343e (client: 512afd)
int CLlDriver::MakeSockAddr(const char* address, sockaddr_in* out, int port, int resolve) {
    char buf[1024];
    strcpy(buf, address);

    if (buf[0] == 0) {
#ifdef A2SERVER_PATCH
        Printf("[tcp:%s] MakeSockAddr: empty address string (default_port=%d, resolve=%d)",
               TcpDriverLabel(this), port, resolve);
#endif
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
    if (InetPton(AF_INET, buf, &out->sin_addr.S_un.S_addr) != 1) {
        out->sin_addr.S_un.S_addr = INADDR_NONE;
#ifdef A2SERVER_PATCH
        if (resolve == 0) {
            Printf("[tcp:%s] MakeSockAddr: InetPton failed for host=\"%s\" (raw=\"%s\", port=%u) with resolve=0; sin_addr left as INADDR_NONE",
                   TcpDriverLabel(this), buf, address, static_cast<unsigned int>(ntohs(out->sin_port)));
        }
#endif
    }

    if (out->sin_addr.S_un.S_addr == INADDR_NONE && resolve != 0) {
        addrinfo hints = {};
        hints.ai_family = AF_INET;
        addrinfo* res = nullptr;
        int32_t gai_err = GetAddrInfo(buf, nullptr, &hints, &res);
        if (gai_err != 0 || res == nullptr) {
#ifdef A2SERVER_PATCH
            int32_t wsa_err = WSAGetLastError();
            Printf("[tcp:%s] MakeSockAddr: GetAddrInfo failed for host=\"%s\" (raw=\"%s\", port=%u): gai=%s, wsa=%s",
                   TcpDriverLabel(this), buf, address, static_cast<unsigned int>(ntohs(out->sin_port)),
                   FormatWinSockError(gai_err).c_str(), FormatWinSockError(wsa_err).c_str());
#endif
            if (res != nullptr) {
                FreeAddrInfo(res);
            }
            out->sin_addr.S_un.S_addr = 0;
            out->sin_port = 0;
            return 0;
        }

        out->sin_addr.S_un.S_addr = ((sockaddr_in*)res->ai_addr)->sin_addr.S_un.S_addr;
        FreeAddrInfo(res);
    }
    memset(out->sin_zero, 0, sizeof(out->sin_zero));
    return 1;
}



// 523621 (client: 512cdc)
int CLlDriver::StartServerTcp() {
    this->listen_socket.socket = INVALID_SOCKET;
    this->listen_socket.wait_obj = nullptr;
    this->num_connections = 0;

    this->listen_socket.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (this->listen_socket.socket == INVALID_SOCKET) {
#ifdef A2SERVER_PATCH
        int32_t err = WSAGetLastError();
        Printf("[tcp:%s] StartServerTcp: socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) failed: %s",
               TcpDriverLabel(this), FormatWinSockError(err).c_str());
#endif
        this->CloseTcp();
        return 0;
    }
    u_long nonblock = 1;
    if (ioctlsocket(this->listen_socket.socket, FIONBIO, &nonblock) != 0) {
#ifdef A2SERVER_PATCH
        int32_t err = WSAGetLastError();
        Printf("[tcp:%s] StartServerTcp: ioctlsocket(FIONBIO) warning on socket=0x%llx: %s",
               TcpDriverLabel(this), static_cast<unsigned long long>(this->listen_socket.socket),
               FormatWinSockError(err).c_str());
#endif
    }

#ifdef A2SERVER_PATCH
    if (this->server_start_addr == nullptr) {
        Printf("[tcp:%s] StartServerTcp: server_start_addr is null", TcpDriverLabel(this));
        this->CloseTcp();
        return 0;
    }
#endif

    sockaddr addr;
    if (this->MakeSockAddr(this->server_start_addr->address, (sockaddr_in*)&addr, 8000, 0) == 0) {
#ifdef A2SERVER_PATCH
        Printf("[tcp:%s] StartServerTcp: MakeSockAddr failed for address=\"%s\"",
               TcpDriverLabel(this), this->server_start_addr->address);
#endif
        this->CloseTcp();
        return 0;
    }

    if (bind(this->listen_socket.socket, &addr, sizeof(addr)) != 0) {
#ifdef A2SERVER_PATCH
        int32_t err = WSAGetLastError();
        Printf("[tcp:%s] StartServerTcp: bind(%s) (raw=\"%s\", socket=0x%llx) failed: %s",
               TcpDriverLabel(this),
               FormatSockAddrIn((const sockaddr_in*)&addr).c_str(),
               this->server_start_addr->address,
               static_cast<unsigned long long>(this->listen_socket.socket),
               FormatWinSockError(err).c_str());
#endif
        this->CloseTcp();
        return 0;
    }

    if (listen(this->listen_socket.socket, this->max_connections) != 0) {
#ifdef A2SERVER_PATCH
        int32_t err = WSAGetLastError();
        Printf("[tcp:%s] StartServerTcp: listen(%s, backlog=%u, socket=0x%llx) failed: %s",
               TcpDriverLabel(this),
               FormatSockAddrIn((const sockaddr_in*)&addr).c_str(),
               this->max_connections,
               static_cast<unsigned long long>(this->listen_socket.socket),
               FormatWinSockError(err).c_str());
#endif
        this->CloseTcp();
        return 0;
    }

    this->listen_socket.wait_obj = CreateEventA(NULL, 0, 0, NULL);
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
    if (this->listen_socket.wait_obj == nullptr) {
#ifdef A2SERVER_PATCH
        int32_t err = static_cast<int32_t>(GetLastError());
        Printf("[tcp:%s] StartServerTcp: CreateEventA failed: %s",
               TcpDriverLabel(this), FormatWinSockError(err).c_str());
#endif
        this->CloseTcp();
        return 0;
    }

    this->guaranteed = 1;
    this->listen_socket.is_in_use = 1;

    if (_beginthread(AcceptThreadTcp, 0, this) == -1L) {
#ifdef A2SERVER_PATCH
        int32_t crt_err = errno;
        uint32_t dos_err = _doserrno;
        Printf("[tcp:%s] StartServerTcp: _beginthread(AcceptThreadTcp) failed: errno=%d (%s), doserrno=%u",
               TcpDriverLabel(this), crt_err, std::strerror(crt_err), dos_err);
#endif
        this->listen_socket.is_in_use = 0;
        CloseHandle(this->listen_socket.wait_obj);
        this->listen_socket.wait_obj = nullptr;
        this->CloseTcp();
        return 0;
    }
#else
    if (_beginthread(AcceptThreadTcp, 0, this) == -1L || this->listen_socket.wait_obj == nullptr) {
        CloseHandle(this->listen_socket.wait_obj);
        this->listen_socket.wait_obj = nullptr;
        this->CloseTcp();
        return 0;
    }

    this->guaranteed = 1;
#endif

#ifdef A2SERVER_PATCH
    {
        sockaddr_in bound = *(const sockaddr_in*)&addr;
        int32_t bound_len = sizeof(bound);
        getsockname(this->listen_socket.socket, (sockaddr*)&bound, &bound_len);
        Printf("[tcp:%s] StartServerTcp: listening on %s (socket=0x%llx, max_connections=%u)",
               TcpDriverLabel(this),
               FormatSockAddrIn(&bound).c_str(),
               static_cast<unsigned long long>(this->listen_socket.socket),
               this->max_connections);
    }
#endif

    return 1;
}


// 522e8f (client: 51258b)
void __cdecl CLlDriver::AcceptThreadTcp(void* context) {
    CLlDriver* drv = (CLlDriver*)context;

#if !defined(A2SERVER_PATCH) && !defined(A2CLIENT_PATCH)
    // setthreadp hook (522EA1) / SYSTEM_setThreadPriority (client 51259D):
    // patched server and client skip raising the accept thread's priority
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif

    while (true) {
        if (drv->listen_socket.is_in_use != 1) {
            SetEvent(drv->listen_socket.wait_obj);
            _endthread();
            return;
        }

        fd_set fd;
        FD_ZERO(&fd);
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
        if (drv->listen_socket.socket != INVALID_SOCKET) {
            FD_SET(drv->listen_socket.socket, &fd);
        }

        if (drv->is_server) {
            for (uint32_t i = 0; i < drv->max_connections; i++) {
                A2NetSock* sock = drv->connection_sockets + i;
                if (sock->is_in_use == 1 && sock->socket != INVALID_SOCKET && fd.fd_count < FD_SETSIZE) {
                    FD_SET(sock->socket, &fd);
                }
            }
        }

        if (fd.fd_count == 0) {
            Sleep(10);
            continue;
        }
#else
        FD_SET(drv->listen_socket.socket, &fd);

        if (drv->is_server) {
            for (int i = 0; i < drv->max_connections; i++) {
                A2NetSock* sock = drv->connection_sockets + i;
                if (sock->is_in_use == 1) {
                    FD_SET(sock->socket, &fd);
                }
            }
        }
#endif

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
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
            Sleep(1);
#endif
            continue;
        }

        if (drv->is_server == 0) {
            if (drv->listen_socket.is_in_use == 1 &&
                FD_ISSET(drv->listen_socket.socket, &fd) &&
                drv->RecvThreadTcp(&drv->listen_socket) == 0) {
#ifdef A2TANGAR_HAT
                tangar_hat::CloseServerSocket(drv->listen_socket.socket);
#else
                closesocket(drv->listen_socket.socket);
#endif
                drv->listen_socket.socket = INVALID_SOCKET;
            }
            continue;
        }

        if (FD_ISSET(drv->listen_socket.socket, &fd)) {
            sockaddr_in newcon;
            int newconln = sizeof(newcon);
#ifdef A2TANGAR_HAT
            SOCKET nsock = tangar_hat::AcceptServerSocket(drv->listen_socket.socket, (sockaddr*)&newcon, &newconln);
#else
            SOCKET nsock = accept(drv->listen_socket.socket, (sockaddr*)&newcon, &newconln);
#endif
            if (nsock == INVALID_SOCKET) {
                if (drv->listen_socket.is_in_use == 1) {
                    ReportWarning("CLlDriverAcceptThreadTcp().\nUnable to accept new connection.\n");
                }
                continue;
            }
            
            if (drv->num_connections >= drv->max_connections) {
#ifdef A2TANGAR_HAT
                tangar_hat::CloseServerSocket(nsock);
#else
                closesocket(nsock);
#endif
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
#ifdef A2TANGAR_HAT
                tangar_hat::CloseServerSocket(nsock);
#else
                closesocket(nsock);
#endif
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
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
            if (conn->is_in_use == 1 && conn->socket != INVALID_SOCKET &&
#else
            if (conn->is_in_use == 1 &&
#endif
                FD_ISSET(conn->socket, &fd) &&
                drv->RecvThreadTcp(conn) == 0) {
#ifdef A2TANGAR_HAT
                tangar_hat::CloseServerSocket(conn->socket);
#else
                closesocket(conn->socket);
#endif
                conn->socket = INVALID_SOCKET;
            }
        }

    }
}

// 522af4 (client: 512230)
void __cdecl CLlDriver::SetSockOptions(SOCKET s, int enlinger) {
    int32_t flag = 1;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, 4);
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
    int32_t bufsz = 0x40000; // 256 KiB
#else
    int32_t bufsz = 0x8000;
#endif
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



// 522b95 (client: 5122d1)
int CLlDriver::RecvThreadTcp(A2NetSock* sock) {
#ifdef A2CLIENT_PATCH
    static uint8_t databuf[51200];
    int rsz = tangar_client::RecvClientFramed(sock->socket, reinterpret_cast<char*>(databuf), sizeof(databuf), 0);
#elif defined(A2TANGAR_HAT)
    uint8_t databuf[0x2000];
    int rsz = tangar_hat::RecvServerFramed(sock->socket, reinterpret_cast<char*>(databuf), sizeof(databuf), 0);
    if (rsz == -2) {
        return 1;
    }
#else
    uint8_t databuf[0x2000];
    int rsz = recv(sock->socket, (char *)databuf, 0x2000, 0);
#endif
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
#if defined(A2TANGAR_HAT) || defined(A2CLIENT_PATCH)
                {
                    long peer_port = netlog_peer_port(sock->socket);
#ifdef A2CLIENT
                    netlog::frame(peer_port == 8000 ? "HAT->C" : "SRV->C", peer_port,
                                  &sock->current_buffer->full_data,
                                  (size_t)sock->current_buffer->datasz + 8, "rcv");
#else
                    netlog::frame(peer_port == 7999 ? "HAT->SRV" : "CLI->SRV", peer_port,
                                  &sock->current_buffer->full_data,
                                  (size_t)sock->current_buffer->datasz + 8, "rcv");
#endif
                }
#endif
#if defined(A2SERVER_PATCH) && !defined(A2CLIENT)
                int recv_ok = sock->manager->ReceiveData(sock->current_buffer);
                sock->copy_num = 0;
                sock->current_buffer = nullptr;
                if (!recv_ok) {
                    return 0;
                }
#else
                sock->manager->ReceiveData(sock->current_buffer);
                sock->copy_num = 0;
                sock->current_buffer = nullptr;
#endif
            }
        }
    }
}




// 523d96 (client: 513442)
int CLlDriver::PrepareAddressDp(CLlAddress* addr) {
    IDirectPlayLobby2A* lobby = nullptr;
    if (CoCreateInstance(CLSID_DirectPlayLobby, NULL, 1, IID_IDirectPlayLobby2A, (LPVOID *) &lobby) != DP_OK) {
        return 0;
    }

    DPCOMPOUNDADDRESSELEMENT elem[3];
    int numelem = 0;

    char addrbuf[100];

    switch (this->provider) {
    case PROVIDER_SERIAL:
        elem[0].guidDataType = DPAID_ServiceProvider;
        elem[0].dwDataSize = sizeof(GUID);
        elem[0].lpData = (LPVOID) &DPSPGUID_SERIAL;
        elem[1].guidDataType = DPAID_ComPort;
        elem[1].dwDataSize = sizeof(DPCOMPORTADDRESS);
        elem[1].lpData = &addr->com;
        numelem = 2;
        break;
    case PROVIDER_MODEM:
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
    case PROVIDER_IPX:
        elem[0].guidDataType = DPAID_ServiceProvider;
        elem[0].dwDataSize = sizeof(GUID);
        elem[0].lpData = (LPVOID)&DPSPGUID_IPX;
        numelem = 1;
        break;
    case PROVIDER_DP_TCPIP:
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

// 5241ea (client: 513896)
int CLlDriver::StartServerDp() {
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
    
    if (this->provider == PROVIDER_MODEM) {
        this->latency = 1000;
        if (this->timeout < 16000) {
            this->timeout = 16000;
        }
    } else if (this->provider == PROVIDER_DP_TCPIP) {
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






// 523b7a (client: 513226)
void __cdecl CLlDriver::RecvThreadDp(void* context) {
    CLlDriver* drv = (CLlDriver*)context;

#ifndef A2CLIENT_PATCH
    // SYSTEM_setThreadPriority hook (client 51259D): patched client skips
    // raising the recv thread priority
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif

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

// 524abd (client: 514141)
void CLlDriver::HandleMessageDp(uint32_t from, uint32_t to, void* data, uint32_t datasz) {
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

// 524986 (client: 51400a)
A2NetSock* CLlDriver::FindDpSock(uint32_t id, int* out_idx) {
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


// 521a09 (client: 51129c)
int CLlDriver::StartServer(int maxconn, const char* name, CLlAddress* addr) {
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
#ifdef A2SERVER_PATCH
        Printf("[tcp:%s] StartServer: closing previously active listener before starting new session",
               TcpDriverLabel(this));
#endif
        this->Close();
    }

    this->num_connections = 0;

    if (this->provider == PROVIDER_HAT_TCP) {
        if (this->StartServerTcp() == 0) {
#ifdef A2SERVER_PATCH
            Printf("[tcp:%s] StartServer: StartServerTcp failed (addr=\"%s\", max_connections=%u, comp_name=\"%s\")",
                   TcpDriverLabel(this),
                   this->server_start_addr != nullptr ? this->server_start_addr->address : "<null>",
                   this->max_connections,
                   this->comp_name);
#endif
            ReportWarning("CLlDriver::StartServer().\nUnable to connect over TCP/IP.\n");
            return 0;
        }
    } else {
        if (this->StartServerDp() == 0) {
#ifdef A2SERVER_PATCH
            Printf("[tcp:%s] StartServer: StartServerDp failed (provider=%d, max_connections=%u)",
                   TcpDriverLabel(this), static_cast<int>(this->provider), this->max_connections);
#endif
            ReportWarning("CLlDriver::StartServer().\nUnable to connect over DirectPlay.\n");
            return 0;
        }
    }
    this->listen_socket.is_in_use = 1;

    return 1;
}


// 521d2a (client: 51151c)
int CLlDriver::PrepareForConnect(const char* name, CLlAddress* addr) {
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

    if (this->provider == PROVIDER_HAT_TCP) {
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

    if (this->provider == PROVIDER_MODEM) {
        this->latency = 1000;
        if (this->timeout < 16000) {
            this->timeout = 16000;
        }
    } else if (this->provider == PROVIDER_DP_TCPIP) {
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


// 52465f (client: 513d0b)
int CLlDriver::ConnectDp(const char* name, CLlNetSession* session) {
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


// 523794 (client: 512e4f)
int CLlDriver::ConnectTcp() {
    this->num_connections = 0;
    this->listen_socket.socket = INVALID_SOCKET;
    this->listen_socket.wait_obj = nullptr;
    this->listen_socket.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (this->listen_socket.socket == INVALID_SOCKET) {
#ifdef A2SERVER_PATCH
        int32_t err = WSAGetLastError();
        Printf("[tcp:%s] ConnectTcp: socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) failed: %s",
               TcpDriverLabel(this), FormatWinSockError(err).c_str());
#endif
        this->CloseTcp();
        return 0;
    }

    sockaddr connaddr;
    sockaddr bindaddr;
    if (this->MakeSockAddr(this->address_str, (sockaddr_in*)&connaddr, 8000, 1) == 0 ||
        this->MakeSockAddr(this->cur_address.address, (sockaddr_in*)&bindaddr, 0, 0) == 0) {
#ifdef A2SERVER_PATCH
        Printf("[tcp:%s] ConnectTcp: MakeSockAddr failed (remote=\"%s\", local=\"%s\")",
               TcpDriverLabel(this), this->address_str, this->cur_address.address);
#endif
        this->CloseTcp();
        return 0;
    }

    if (bind(this->listen_socket.socket, &bindaddr, sizeof(bindaddr)) != 0) {
#ifdef A2SERVER_PATCH
        int32_t err = WSAGetLastError();
        Printf("[tcp:%s] ConnectTcp: bind(%s) (raw_local=\"%s\", socket=0x%llx) failed: %s",
               TcpDriverLabel(this),
               FormatSockAddrIn((const sockaddr_in*)&bindaddr).c_str(),
               this->cur_address.address,
               static_cast<unsigned long long>(this->listen_socket.socket),
               FormatWinSockError(err).c_str());
#endif
        this->CloseTcp();
        return 0;
    }

    if (connect(this->listen_socket.socket, &connaddr, sizeof(connaddr)) != 0) {
#ifdef A2SERVER_PATCH
        int32_t err = WSAGetLastError();
        Printf("[tcp:%s] ConnectTcp: connect(%s -> %s) (raw_remote=\"%s\", socket=0x%llx) failed: %s",
               TcpDriverLabel(this),
               FormatSockAddrIn((const sockaddr_in*)&bindaddr).c_str(),
               FormatSockAddrIn((const sockaddr_in*)&connaddr).c_str(),
               this->address_str,
               static_cast<unsigned long long>(this->listen_socket.socket),
               FormatWinSockError(err).c_str());
#endif
        this->CloseTcp();
        return 0;
    }

    SetSockOptions(this->listen_socket.socket, 1);

    this->listen_socket.uid = this->next_uid;
    this->listen_socket.manager = this->hl_driver->AllocClientBufManager(this->listen_socket.uid);
    this->listen_socket.wait_obj = CreateEventA(NULL, 0, 0, NULL);
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
    if (this->listen_socket.wait_obj == nullptr) {
#ifdef A2SERVER_PATCH
        int32_t err = static_cast<int32_t>(GetLastError());
        Printf("[tcp:%s] ConnectTcp: CreateEventA failed: %s",
               TcpDriverLabel(this), FormatWinSockError(err).c_str());
#endif
        this->CloseTcp();
        return 0;
    }

    this->num_connections = 1;
    this->guaranteed = 1;
    this->listen_socket.is_in_use = 1;

    if (_beginthread(AcceptThreadTcp, 0x400, this) == -1L) {
#ifdef A2SERVER_PATCH
        int32_t crt_err = errno;
        uint32_t dos_err = _doserrno;
        Printf("[tcp:%s] ConnectTcp: _beginthread(AcceptThreadTcp) failed: errno=%d (%s), doserrno=%u",
               TcpDriverLabel(this), crt_err, std::strerror(crt_err), dos_err);
#endif
        this->listen_socket.is_in_use = 0;
        this->num_connections = 0;
        CloseHandle(this->listen_socket.wait_obj);
        this->listen_socket.wait_obj = nullptr;
        this->CloseTcp();
        return 0;
    }
#else
    if (_beginthread(AcceptThreadTcp, 0x400, this) == -1L || this->listen_socket.wait_obj == nullptr) {
        CloseHandle(this->listen_socket.wait_obj);
        this->listen_socket.wait_obj = nullptr;
        this->CloseTcp();
        return 0;
    }

    this->num_connections = 1;
    this->guaranteed = 1;
#endif

#ifdef A2SERVER_PATCH
    {
        sockaddr_in local_bound = *(const sockaddr_in*)&bindaddr;
        int32_t bound_len = sizeof(local_bound);
        getsockname(this->listen_socket.socket, (sockaddr*)&local_bound, &bound_len);
        Printf("[tcp:%s] ConnectTcp: connected %s -> %s (socket=0x%llx)",
               TcpDriverLabel(this),
               FormatSockAddrIn(&local_bound).c_str(),
               FormatSockAddrIn((const sockaddr_in*)&connaddr).c_str(),
               static_cast<unsigned long long>(this->listen_socket.socket));
    }
#endif

    return 1;
}

// 5223fb (client: 511b3b)
int CLlDriver::Connect(const char* name, CLlNetSession* session) {
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

    if (this->provider == PROVIDER_HAT_TCP) {
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

// 52394a (client: 513005)
int CLlDriver::SendDataTcp(A2NetSock* sock, NetStru3* buffer) {
    int len = buffer->datasz + 8;
    int sendlen = -1;
    if (sock->socket != INVALID_SOCKET) {
#if defined(A2TANGAR_HAT) || defined(A2CLIENT_PATCH)
        long peer_port = netlog_peer_port(sock->socket);
#ifdef A2CLIENT
        netlog::frame(peer_port == 8000 ? "C->HAT" : "C->SRV", peer_port,
                      &buffer->full_data, len, "snd");
#else
        netlog::frame(peer_port == 7999 ? "SRV->HAT" : "SRV->CLI", peer_port,
                      &buffer->full_data, len, "snd");
#endif
#endif
#ifdef A2TANGAR_HAT
        tangar_hat::FlushEnqueuedPackets(sock->socket);
#endif
        sendlen = send(sock->socket, (char*)&buffer->full_data, len, 0);
    }

    this->hl_driver->AddTailFreeNet3(buffer);

    if (sendlen == -1 || sendlen != len) {
        this->CloseTcpSocket(sock);
        return 0;
    }

    return 1;
}



// 5255c6 (client: 514795)
int CLlDriver::SendDataDp(A2NetSock* sock, NetStru3* buffer) {
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


// 5226c4 (client: 511dfc)
int CLlDriver::SendData(uint32_t uid, NetStru3* buffer) {
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

    if (this->provider == PROVIDER_HAT_TCP) {
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

// 525b86 (client: 514ce1)
void CLlDriver::SendAllPendingDp(A2NetSock* sock) {
    if (this->guaranteed == 0) {
        while (!sock->list_0x14.IsEmpty()) {
            if (this->SendDataDp(sock, nullptr) == 0) {
                break;
            }

            Sleep(10);
        }
    }
}

// 522526 (client: 511c62)
void CLlDriver::DisconnectClient(uint32_t uid) {
    EnterCriticalSection(&this->critical_section);
    A2NetSock* sock = this->GetClientBySocketId(uid);
    if (sock) {
        if (this->IsInUse(sock)) {
            if (this->provider == PROVIDER_HAT_TCP) {
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

// 52262a (client: 511d62)
void CLlDriver::CleanupInvalidTcpClient(uint32_t uid) {
    if (this->provider == PROVIDER_HAT_TCP) {
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

// 522809 (client: 511f4d)
void CLlDriver::CleanupAllInvalid() {
    if (this->listen_socket.is_in_use != 1) {
        return;
    }
        
    EnterCriticalSection(&this->critical_section);
    if (this->is_server == 0) {
        if (this->provider == PROVIDER_HAT_TCP) {
            if (this->listen_socket.socket == INVALID_SOCKET) {
                this->CloseTcp();
            }
        } else {
            this->SendDataDp(&this->listen_socket, nullptr);
        }
    } else {
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
        for (uint32_t i = 0; i < this->max_connections; i++) {
#else
        for (int i = 0; i < this->num_connections; i++) {
#endif
            A2NetSock* sock = this->connection_sockets + i;
            if (this->IsInUse(sock)) {
                if (this->provider == PROVIDER_HAT_TCP) {
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


// 52460e (client: 513cba)
int CLlDriver::RestartModemServerDp() {
    if (this->provider != PROVIDER_MODEM) {
        return 1;
    }

    this->CloseDp();
    this->CreateDp();
    return this->StartServerDp();
}

// 524644 (client: 513cf0)
void CLlDriver::RecreateDp() {
    this->CloseDp();
    this->CreateDp();
}

// 5229cd (client: 512109) - Set connection latency limit
void CLlDriver::SetLatency(uint32_t conn_uid, int32_t latency_ms) {
    A2NetSock* sock = this->GetClientBySocketId(conn_uid);
    if (sock) {
        sock->latency = latency_ms;
    }
}

// 5229fd (client: 512139) - Get current measured latency (ms)
int32_t CLlDriver::GetLatency(uint32_t conn_uid) {
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

// 522a51 (client: 51218d) - Get current packet-loss figure
int32_t CLlDriver::GetPacketLoss(uint32_t conn_uid) {
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
// split from 51b370 (client: 50c867)
bool IsVisibleTo(TokenPos* pos, Player* player) {
    if (MapStuff_Instance == nullptr || g_Server->multiplayer == 0) {
        return true;
    }

    uint32_t x = pos->GetX() >> 3;
    uint32_t y = pos->GetY() >> 3;
    return MapStuff_Instance->scan_presence_grid.sector_grid[x+1][y+1] & player->vision_sharing_id;
}

// Helper: handle "unit hidden" case for effect packets (ids 0x88/0x89).
// split from 51b370 (client: 50c867)
void MarkUnitHiddenForPlayer(PacketMoveBroadcast* packet, Player* player) {
    if (packet->id < EFFECT_HANG || packet->id > EFFECT_GONE) {
        return;
    }

    Unit* affected_unit = g_ServerUnits->FindById(packet->unit_id);
    if (affected_unit == nullptr) {
        return;
    }

    int32_t pid = player->player_id;
    if (0x10 <= pid && pid < 0x20) {
        affected_unit->something_per_player[pid - 0x10] |= 0x800000;
    }
}

// 51b0f0 (client: 50c65a) - send unit action packet to relevant players
void NetStru1::SendUnitAction(Packet* packet, Unit* unit) {
    if (packet->to_player_id != 0) {
        Player* player = g_PlayersList->FindById(packet->to_player_id);
        if ((unit->seen_by_mask & player->vision_sharing_id) == 0 && g_Server->multiplayer != 0) {
            int32_t pid = player->player_id;
            if (pid >= 0x10 && pid < 0x20 && packet->id == UNIT_MOVE &&
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
        if ((unit->seen_by_mask & player->vision_sharing_id) == 0 && g_Server->multiplayer != 0) {
            int32_t pid = player->player_id;
            if (pid >= 0x10 && pid < 0x20 && packet->id == UNIT_MOVE &&
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

// 51b370 (client: 50c867) - Broadcast move packet to all players in visibility
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

// 51b638 (client: 50cab9) - Broadcast AoE-zone packet to all players in visibility.
void NetStru1::BroadcastAoeZone(PacketAoeZone* packet, AreaEffect* area_effect) {
    if (packet->to_player_id != 0) {
        Player* player = g_PlayersList->FindById(packet->to_player_id);
        if ((area_effect->seen_by_mask & player->vision_sharing_id) != 0 || g_Server->multiplayer == 0) {
            this->QueuePacketSend(packet);
        } else {
            auto pid = player->player_id;
            if (pid >= 0x10 && pid < 0x20) {
                if (packet->set_flag != 0) {
                    area_effect->notified_mask |= player->vision_sharing_id;
                } else if ((area_effect->notified_mask & player->vision_sharing_id) == 0) {
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
        if ((area_effect->seen_by_mask & player->vision_sharing_id) != 0 || g_Server->multiplayer == 0) {
            packet->to_player_id = player->player_id;
            this->QueuePacketSend(packet);
        } else {
            auto pid = player->player_id;
            if (pid >= 0x10 && pid < 0x20) {
                if (packet->set_flag != 0) {
                    area_effect->notified_mask |= player->vision_sharing_id;
                } else if ((area_effect->notified_mask & player->vision_sharing_id) == 0) {
                    packet->to_player_id = player->player_id;
                    this->QueuePacketSend(packet);
                }
            }
        }
    }
}

// 51bab0 (client: 50ce4e) - Send targeted-unit spell packet
void NetStru1::SendSpellCastAtUnit(Unit* caster, Spell* spell, Unit* target, int16_t delay) {
    PacketMoveBroadcast& packet = PacketMoveBroadcast::Inst;
    packet.id = EFFECT_MOVE;
    packet.unit_id = caster->building_id;
    packet.type_id = 0;
    packet.x = 0;
    packet.y = 0;
    if (caster->typeId == 0) {
        packet.id = EFFECT_FROM;
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

// 51bb94 (client: 50cf32) - Send point/area spell packet
void NetStru1::SendSpellCastAtPos(Unit* caster, Spell* spell, TokenPos* pos, int16_t delay) {
    PacketMoveBroadcast& packet = PacketMoveBroadcast::Inst;
    packet.id = EFFECT_MOVE;
    packet.unit_id = caster->building_id;
    if (caster->typeId == 0) {
        packet.id = EFFECT_FROM;
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

// 51bc6b (client: 50d009) - Send multi-target spell packet
void NetStru1::SendSpellCastMulti(Unit* caster, Spell* spell, CList<Unit*>* units, int32_t flag) {
    PacketTerrain& packet = PacketTerrain::Inst;
    packet.id = EFFECT_MULTI_TARGET;
    packet.count = units->GetCount() + 1;
    packet.buf[0] = caster->building_id;
    if (caster->typeId == 0) {
        packet.id = EFFECT_MULTI_FROM;
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

// 51bda4 (client: 50d132)
void NetStru1::BroadcastEffect(Effect* effect, Unit* unit, uint8_t packet_id) {
    PacketEffect& packet = PacketEffect::Inst;
    packet.id = packet_id ? packet_id : EFFECT_HANG;
    packet.unit_id = unit->building_id;
    packet.effect_type = effect->typeId;
    packet.to_player_id = 0;
    // This reinterpret_cast is kinda weird, but it works, because `BroadcastMovePacket` only reads the 2-byte 0xa field from non-Packet fields.
    this->BroadcastMovePacket(reinterpret_cast<PacketMoveBroadcast*>(&packet), unit->position);
}

// 51c822 (client: 50d901) - Send spectator/alliance state
void NetStru1::SendSpectatorState(NetStru2* ns2) {
    PacketDword& packet = PacketDword::Inst;
    packet.id = JOIN_DONE;
    packet.value = g_SpectatorState;
    packet.WriteTo(ns2);
    packet.id = SERVER_TICK;
    packet.value = (uint32_t)g_Server->tick;
    packet.WriteTo(ns2);
    ns2->out_buffers[ns2->out_buff_id].pkt_num += 1;
    this->SendAllData();
}

// 51d837 (client: none) - Send server state / player list to player
void NetStru1::SendGameInfo(int32_t game_speed, Player* player) {
    PacketDword& packet = PacketDword::Inst;
    packet.id = GAME_SPEED;
    if (player != nullptr) {
        packet.to_player_id = player->player_id;
    } else {
        packet.to_player_id = 0;
    }
    packet.value = game_speed;
    this->QueuePacketSend(&packet);
}

// 51dfa7 (client: 50eb05)
void NetStru1::SendHatCharData(int32_t hat_char_id1, int32_t hat_char_id2, CString name, void* data, uint32_t size, uint16_t player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = player_id;
    packet.id = HAT_CHAR_DATA;
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

// 51e0b7 (client: none)
void NetStru1::SendHatMessage(int32_t hat_char_id1, int32_t hat_char_id2, CString name, void* data, uint32_t size, uint16_t player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = player_id;
    packet.id = HAT_MESSAGE;
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

// 51e1c7 (client: none)
void NetStru1::SendHatKeepalive(uint16_t player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = player_id;
    packet.id = HAT_MESSAGE;
    packet.count = 1;
    this->QueuePacketSend(&packet);
}

// 51e205 (client: 50ec0e) - Send hat login packet
void NetStru1::SendHatLogin(CString name) {
    PacketJoin& packet = PacketJoin::Inst;
    packet.id = HAT_LOGIN;
    packet.__as_dword = 0;
    strcpy(packet.name, name);
    packet.to_player_id = 0;
    this->QueuePacketSend(&packet);
}

// 51e40c (client: 50ede9) - Send raw string data to HAT
void NetStru1::SendHatRawString(const char* str, int32_t len) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = 0;
    packet.id = HAT_RAW_DATA;
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

// 51e5fb (client: 50efc6) - Send HAT join-success reply (pkt 0xD8)
void NetStru1::SendHatJoinOk(int32_t a, int32_t b) {
    Packet2Dwords& packet = Packet2Dwords::Inst;
    packet.id = HAT_JOIN_OK;
    packet.param1 = a;
    packet.param2 = b;
    packet.to_player_id = 0;
#if defined(A2TANGAR_HAT) || defined(A2CLIENT_PATCH)
    netlog::logf("EVT", "HAT_JOIN_OK id1=%08X id2=%08X", (unsigned)a, (unsigned)b);
#endif
    this->QueuePacketSend(&packet);
}

// 51e63f (client: none) - Send HAT join-failure reply (pkt 0xD9)
void NetStru1::SendHatJoinFail(int32_t a, int32_t b, int32_t error_code) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = 0;
    packet.id = HAT_JOIN_FAIL;
    packet.count = 0xC;
    uint32_t* p = (uint32_t*)packet.data;
    *p++ = a;
    *p++ = b;
    *p = error_code;
#if defined(A2TANGAR_HAT) || defined(A2CLIENT_PATCH)
    netlog::logf("EVT", "HAT_JOIN_FAIL id1=%08X id2=%08X error=%d", (unsigned)a, (unsigned)b,
                 error_code);
#endif
    this->QueuePacketSend(&packet);
}

// 51e794 (client: none) - Send HAT player-check reply (pkt 0xE1)
void NetStru1::SendHatPlayerCheck(int32_t a, int32_t b, uint8_t status, int16_t player_id) {
    PacketData& packet = PacketData::Inst;
    packet.to_player_id = player_id;
    packet.id = HAT_PLAYER_CHECK;
    packet.count = 0xC;
    uint32_t* p = (uint32_t*)packet.data;
    *p++ = a;
    *p++ = b;
    *p = status;
    this->QueuePacketSend(&packet);
}

// 51a6d5 (client: 50beae)
void NetStru1::SendMonsterInventory(Unit* unit, Player* player, int32_t start_index, int32_t end_index) {
    if (unit->typeId < unit_type::hero_fighter_male || unit->typeId >= unit_type::monster_first) {
        return;
    }

    if (start_index + end_index == 0) {
        start_index = 0;
        end_index = unit->inventory->items.GetSize();
    }

    PacketUnitStateVec& packet = PacketUnitStateVec::Inst;
    packet.id = ITEM_LIST;
    packet.entry_count = (uint16_t)(end_index - start_index);
    packet.start_index = (uint16_t)start_index;
    packet.data_size = 0;

    packet.mode = ITEM_LIST_INVENTORY;
    if (start_index != 0 || end_index != unit->inventory->items.GetSize()) {
        packet.mode |= ITEM_LIST_PARTIAL_UPDATE;
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

// 51a0ef (client: 50b9df)
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
                this->SendHumanoidProps(unit, p, EQUIP_SLOTS_ALL);
            }
        }
        return;
    }

    if (unit->typeId < unit_type::hero_fighter_male || unit->typeId >= unit_type::monster_first) {
        if (unit->sent_to_mask & player->vision_sharing_id) {
            return;
        }

        if (unit->pOwner != player) {
            PacketUnitEquipment& packet = PacketUnitEquipment::Inst;
            packet.id = ENEMY_EQUIPMENT;
            packet.unit_id = (uint16_t)unit->building_id;
            packet.flags = (uint16_t)flags;
            packet.to_player_id = player->player_id;

            int32_t i = 0;
            if (flags & 1) {
                packet.prop[i++] = unit->weapon ? unit->weapon->item_id.raw : 0;
            }
            if (flags & 2) {
                packet.prop[i++] = unit->shield ? unit->shield->item_id.raw : 0;
            }
            for (int32_t slot = 3; slot <= 12; slot++) {
                if (flags & (1 << (slot - 1))) {
                    Item* equip = static_cast<Humanoid*>(unit)->equipment[slot];
                    packet.prop[i++] = equip ? equip->item_id.raw : 0;
                }
            }

            this->QueuePacketSend(&packet);
            return;
        }
    }

    PacketUnitStateVec& packet = PacketUnitStateVec::Inst;
    packet.id = ITEM_LIST;
    packet.entry_count = 0;
    packet.start_index = (uint16_t)flags;
    packet.data_size = 0;
    packet.mode = ITEM_LIST_EQUIPMENT;
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

// 51a8b2 (client: 50c006)
void NetStru1::SendShopTable(Token* token, Inventory* inventory, Player* player, int32_t mode) {
    PacketUnitStateVec& packet = PacketUnitStateVec::Inst;
    packet.id = ITEM_LIST;
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
    packet.mode = (ItemListMode)mode;

    POSITION it = inventory->items.GetHeadPosition();
    while (it != nullptr) {
        Item* item = inventory->items.GetNext(it);
        item->StoreToPacket(&packet, mode == ITEM_LIST_SHOP_TABLE);
        packet.entry_count++;
    }
    this->QueuePacketSend(&packet);
}

// 51aa26 (client: 50c10b) - send single shop shelf contents
void NetStru1::SendShopShelf(Shop* shop, CMultiShopShelf* shelf, Player* player, uint8_t slot) {
    PacketUnitStateVec& packet = PacketUnitStateVec::Inst;
    packet.id = ITEM_LIST;
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
    packet.mode = (ItemListMode)slot;
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

// 51ab99 (client: 50c25c) - send shop shelf contents
void NetStru1::SendShopShelves(CMultiShopInstance* inst, Player* player, int32_t shelf_id) {
    if (shelf_id == -1) {
        for (int32_t i = 0; i < 4; i++) {
            Shop* shop = nullptr;
            if (inst->shop_template != nullptr) {
                shop = inst->shop_template->shop;
            }
            this->SendShopShelf(shop, &inst->shelves[i], player, i + ITEM_LIST_SHELF_1);
        }
    } else if (shelf_id > 0) {
        Shop* shop = nullptr;
        if (inst->shop_template != nullptr) {
            shop = inst->shop_template->shop;
        }
        this->SendShopShelf(shop, &inst->shelves[shelf_id - 1], player, shelf_id - 1 + ITEM_LIST_SHELF_1);
    }
}

// 51ac77 (client: 50c2ea) - Broadcast token state to players
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
        if (flag == UNIT_DAMAGED) {
            if (!(unit->seen_by_mask & player->vision_sharing_id) && g_Server->multiplayer != 0) {
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
        packet.id = SACK_STATE;
        packet.to_player_id = player->player_id;
        packet.xpos = sack->position->GetXx();
        packet.ypos = sack->position->GetYy();
        packet.unit_id = (uint16_t)sack->building_id;
        SackVisualType sack_type((uint8_t)log10(sack->_exp));

#ifndef A2CLIENT
        if (g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
            if (sack->inventory->FindItemByName("Quest RuneF")) {
                sack_type.has_rune_f = 1;
            }
            if (sack->inventory->FindItemByName("Quest RuneA")) {
                sack_type.has_rune_a = 1;
            }
        }
#endif
        packet.type_id = sack_type;

        if (g_Server->multiplayer != 0) {
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
            packet.id = BUILDING_HP;
            packet.to_player_id = player ? player->player_id : 0;
            packet.building_id = (uint16_t)building->building_id;
            packet.hp = building->hp;
            this->QueuePacketSend(&packet);
        }
    }
}



// 51f124 (client: 50f846)
void NetStru1::WriteStatsLog(CString fname) {
    CStdioFile f(fname, CFile::modeCreate | CFile::modeWrite);

    f.WriteString("Cmd|Packets|Avg Size| Total Size |Comment\n");
    f.WriteString("-------------------------------------------\n");

    for (int32_t i = 0; i < 0x100; i++) {
        if (this->stat_pkt_num[i] != 0) {
            CString pname;
            switch (i) {
            case SERVER_TICK:
                pname = "GM_ReportNewSegment";
                break;
            case MONEY_UPDATE:
                pname = "GM_ReportMoney";
                break;
            case UNIT_SHOWN:
                pname = "GM_ReportUnitShown";
                break;
            case UNIT_MOVE:
                pname = "GM_ReportUnitMove";
                break;
            case UNIT_CHANGE_4:
            case UNIT_CHANGE_1:
            case UNIT_CHANGE_2:
            case UNIT_CHANGE_3:
                pname = "GM_ReportUnitChange";
                break;
            case UNIT_TURN:
                pname = "GM_ReportUnitTurn";
                break;
            case UNIT_ATTACK:
                pname = "GM_ReportUnitAttack";
                break;
            case RANGED_ATTACK:
                pname = "GM_ReportRangedAttack";
                break;
            case UNIT_DAMAGED:
                pname = "GM_ReportUnitDamaged";
                break;
            case UNIT_HIDDEN:
                pname = "GM_ReportUnitHidden";
                break;
            case ITEM_LIST:
                pname = "GM_ReportItemList";
                break;
            case UNIT_WEAR_ITEMS:
                pname = "GM_ReportUnitWearItems";
                break;
            case SACK_STATE:
                pname = "GM_ReportSack";
                break;
            case EFFECT_MOVE:
                pname = "GM_ReportEffect";
                break;
            case AOE_ZONE:
                pname = "GM_ReportAreaEffect";
                break;
            case EFFECT_HANG:
                pname = "GM_ReportEffectHang";
                break;
            case EFFECT_GONE:
                pname = "GM_ReportEffectGone";
                break;
            case EFFECT_MULTI_TARGET:
                pname = "GM_ReportEffectMultiTarget";
                break;
            case EFFECT_FROM:
                pname = "GM_ReportEffectFrom";
                break;
            case EFFECT_MULTI_FROM:
                pname = "GM_ReportEffectMultiFrom";
                break;
            case REPORT_MESSAGE:
                pname = "GM_ReportMessage";
                break;
            case PLAYER_INFO:
                pname = "GM_ReportNewPlayer";
                break;
            case PLAYER_LEFT:
                pname = "GM_ReportDeletePlayer";
                break;
            case ASH_MAP:
                pname = "GM_ReportAshMap";
                break;
            case ENEMY_EQUIPMENT:
                pname = "GM_ReportEnemyEquipment";
                break;
            case DIPLOMACY_STATE:
                pname = "GM_ReportDiplomacy";
                break;
            case CREATURE_KNOWLEDGE:
                pname = "GM_ReportCreatureKnowledge";
                break;
            case PLAYER_QUESTS:
                pname = "GM_ReportPlayerQuests";
                break;
            case INN_QUESTS:
                pname = "GM_ReportInnQuests";
                break;
            }

            CString msg;
            msg.Format("%3d|%7d|%8d|%11d|%s\n", i, this->stat_pkt_num[i], this->stat_pkt_size[i] / this->stat_pkt_num[i], this->stat_pkt_size[i], (const char*)pname);
            
            f.WriteString(msg);
        }
    }
    f.Close();
}

// 51d8d6 (client: 50e48b)
void NetStru1::SendHatCredentials(const CString& login, const CString& password, uint32_t flags) {
    PacketJoin* pkt = &PacketJoin::Inst;
#ifdef A2CLIENT_PATCH
    // v20 HAT_AUTH (0x00) is client-only; dedicated servers authenticate via SendHatLogin (0xD1).
    tangar_client::BuildAuthorizationPacket(pkt, login, password, flags);
#else
    CString namepwd = login + password;

    strcpy(pkt->name, namepwd);

    pkt->id = HAT_CREDENTIALS;

    pkt->player_id = login.GetLength();
    pkt->token_id = 0;
    pkt->color = flags;
    pkt->flags = 7;
    pkt->to_player_id = 0;
#endif

    this->QueuePacketSend(pkt);
}

// 51d9d0 (client: 50e58b)
void NetStru1::SendHatCharId(HatCharId chid) {
    Packet2Dwords* pkt = &Packet2Dwords::Inst;
    pkt->id = HAT_PICK_CHAR;
    pkt->param1 = chid.id1;
    pkt->param2 = chid.id2;
    pkt->to_player_id = 0;
    this->QueuePacketSend(pkt);
}

// 51d9a0 (client: 50e55b)
void NetStru1::SendHatLogout() {
    Packet* pkt = &Packet::Inst;
    pkt->id = HAT_LOGOUT;
    pkt->to_player_id = 0;
    this->QueuePacketSend(pkt);
}


// 51da14 (client: 50e5cf)
void NetStru1::SendHatConnectServer(HatCharId key, uint8_t body, uint8_t reaction, uint8_t mind, uint8_t spirit, uint8_t sphere, uint8_t type_face, uint8_t color, const CString& name, const CString& srv) {
    PacketData* pkt = &PacketData::Inst;
    pkt->to_player_id = 0;
    pkt->Inst.id = HAT_CONNECT_SERVER;
    pkt->count = name.GetLength() + 16 + srv.GetLength();
    memcpy(pkt->data, &key, 8);
    pkt->data[8] = body;
    pkt->data[9] = reaction;
    pkt->data[10] = mind;
    pkt->data[11] = spirit;
    pkt->data[12] = sphere;
    pkt->data[13] = type_face;
    pkt->data[14] = color;
    pkt->data[15] = name.GetLength();
    memcpy(pkt->data + 16, (const char*)name, name.GetLength());
    memcpy(pkt->data + 16 + name.GetLength(), (const char*)srv, srv.GetLength());
    QueuePacketSend(pkt);
}

// 51dce5 (client: 50e8c0)
void NetStru1::SendServerConnectHatTicket(HatCharId ticket, uint16_t player_id) {
    Packet2Dwords* pkt = &Packet2Dwords::Inst;
    pkt->id = HAT_TICKET;
    pkt->param1 = ticket.id1;
    pkt->param2 = ticket.id2;
    pkt->to_player_id = player_id;
    QueuePacketSend(pkt);
}

// 51e513 (client: 50eede) - client-only helper
void NetStru1::SendUnknownDC() {
    Packet* pkt = &Packet::Inst;
    pkt->id = UNKNOWN_DC;
    pkt->to_player_id = 0;
    QueuePacketSend(pkt);
}

// 51db5c (client: 50e70d) - client-only helper
void NetStru1::SendHatCharRequest(HatCharId id) {
    Packet2Dwords* pkt = &Packet2Dwords::Inst;
    pkt->id = HAT_CHAR_REQUEST;
    pkt->to_player_id = 0;
    pkt->param1 = id.id1;
    pkt->param2 = id.id2;
    QueuePacketSend(pkt);
}




// 4e505f (client: 4ea58f)
void SendWWWReport(const CString& surl, const CString& data, int val3) {
    HINTERNET agent = InternetOpenA("RageOfMages2", 0, 0, 0, 0);
    if (!agent) {
        return;
    }

    CString tmp;
    for (int i = 0; i < data.GetLength(); i++) {
        char chr = data[i];
        if (chr == ' ') {
            tmp += '+';
        }
        else {
            tmp += chr;
        }
    }

    tmp = surl + '?' + tmp;

    CString proto = tmp.Left(7);
    proto.MakeLower();

    if (proto != "http://") {
        tmp = "http://" + tmp;
    }

    HINTERNET url = InternetOpenUrlA(agent, tmp, nullptr, 0, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 0);
    if (!url == 0) {
        InternetCloseHandle(agent);
        return;
    }

    static char buf[1000];
    DWORD readed = 0;

    char* buf_pos = buf;
    if (val3) {
        if (!InternetReadFile(url, buf_pos, 1000, &readed)) {
            InternetCloseHandle(agent);
            return;
        }
    }
    InternetCloseHandle(agent);
}
