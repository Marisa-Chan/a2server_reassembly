#include "net.h"
#include "player.h"
#include "unit.h"
#include "unit_list.h"
#include "group.h"
#include "game_app.h"
#include "effect.h"
#include "eye.h"
#include "packet.h"
#include "server.h"
#include "spell.h"
#include "world.h"
#include "players_list.h"
#include "table.h"
#include <cstring>

Packet Packet::Inst;
PacketJoin PacketJoin::Inst;
PacketInfo PacketInfo::Inst;
PacketTerrain PacketTerrain::Inst;

// asm-side static instance at 6D1180 — exported so Main.asm can still access it
extern "C" PacketUnitUpdate unk_6D1180;


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

	FUN_005186cd(&pkt);
}

// 4FB4CA
void NetStru1::FUN_004fb4ca(Unit* unit, Player* new_owner)
{
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
        POSITION pos = g_PlayersList->GetHeadPosition();
        while (pos != nullptr) {
            Player* p = g_PlayersList->GetNext(pos);
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
        }
        return;
    }

    // Fill in packet header.
    PacketUnitUpdate* pkt = &unk_6D1180;

    pkt->field_0x10 = player->player_id;
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

            int32_t max_hp = unit->monster_info->values[0].health_max;

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
                if (g_World->diplomacy[unit->pOwner->player_id][player->player_id] & 0x10 == 0) {
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
                pkt->PutByte(0x4, unit->hit_values.skill_levels[i]);
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
            pkt->data_offset += 25; // 24 + terminator that already nulled
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
        this->FUN_005186cd(pkt);
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
