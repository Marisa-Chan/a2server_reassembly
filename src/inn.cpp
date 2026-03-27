#include "inn.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "buildings_list.h"
#include "building.h"
#include "constants.h"
#include "game_app.h"
#include "group.h"
#include "player.h"
#include "players_list.h"
#include "quest.h"
#include "quest_map.h"
#include "server.h"
#include "spell.h"
#include "token.h"
#include "unit.h"
#include "unit_list.h"
#include "world.h"
#include "map_stuff.h"

// Vtable pointers for specific quest kinds (defined in Main.asm .rdata section)
extern "C" const void* off_60F9F8[];  // KillMob
extern "C" const void* off_60F9C0[];  // KillN
extern "C" const void* off_60F988[];  // KillGroup
extern "C" const void* off_60F950[];  // Escort
extern "C" const void* off_60F918[];  // DeliverItem
extern "C" const void* off_60F8E0[];  // DeliverMail
extern "C" const void* off_60F8A8[];  // RaiseZombies
extern "C" const void* off_60F870[];  // RaiseSkeletons
extern "C" const void* off_60F838[];  // RaiseGhosts
extern "C" const void* off_60F800[];  // InterceptUnit
extern "C" const void* off_60F7C8[];  // InterceptGroup
extern "C" const void* off_60F790[];  // KillInnDefenders

// Call Initialize via vtable slot 9 (offset +0x24 from vtable start)
// TODO: add `Quest` vtable. This is `q->Initialize(...)`.
static void QuestInitialize(Quest* q, int some_id, int player_id, int building_id, int obj, int landmark_id, int limit, int reward)
{
    auto vtable = *reinterpret_cast<void***>(q);
    reinterpret_cast<void(__thiscall*)(Quest*, int, int, int, int, int, int, int)>(vtable[9])(
        q, some_id, player_id, building_id, obj, landmark_id, limit, reward
    );
}

// Get quest kind via vtable slot 10 (offset +0x28 from vtable start)
// TODO: add `Quest` vtable. This is `q->Kind()`.
static int32_t QuestKind(Quest* q)
{
    auto vtable = *reinterpret_cast<void***>(q);
    return reinterpret_cast<int32_t(__thiscall*)(Quest*)>(vtable[10])(q);
}

// Helper: allocate a quest, set its vtable, and store as active_quest
// TODO: migrate `Quest` classes' vtables.
static Quest* MakeActiveQuest(Inn* inn, const void* vtable[])
{
    Quest* q = new Quest();
    q->quest_map = nullptr;
    *reinterpret_cast<const void**>(q) = vtable;
    inn->active_quest = q;
    return q;
}

// Helper: register quest (initialize, set state, register in quest_map)
static void RegisterQuest(Inn* inn, Quest* q, int some_id, int player_id, int building_id, int obj, int landmark_id, int limit, int reward)
{
    QuestInitialize(q, some_id, player_id, building_id, obj, landmark_id, limit, reward);
    q->state = 3;
    inn->quest_map->sub_55E24A(q);
}

// 562148
void Inn::InnCreateQuests(Player* player)
{
    // ---- QuestMap lookup / creation ----
    if (!this->quest_map_per_player.Lookup(player->player_id, this->quest_map)) {
        this->quest_map = new QuestMap();
        this->quest_map_per_player[player->player_id] = this->quest_map;
    }

    // ---- Early-out conditions ----
    if (g_ServerConfig.gameType == 3) {
        return;
    }
    if (g_QuestMap.sub_55E129(0x11, player->player_id, this->building_id) != 0) {
        return;
    }
    if (this->sub_567B4D(player)) {
        return;
    }

    // ---- Player level info ----
    int32_t exp = player->main_unit->_exp;
    int max_skill = 0;
    for (int i = 1; i < 6; i++) {
        max_skill = (std::max)(max_skill, (int)player->main_unit->hit_values.skill_levels[i]);
    }

    int def_min = max_skill * 3 - 50;
    int def_max = max_skill * 3 + 75;

    // ---- Map width and distance thresholds ----
    uint8_t map_width = MapStuff_Instance->map_width;
    uint8_t dist_threshold  = (map_width - 16 < 128) ? 64 : map_width / 2;
    uint8_t dist_threshold2 = 64;

    BuildingsList* building_list = g_Server->srv_stru1->building_list;

    // =====================================================================
    //  KillN (vtable off_60F9C0)
    // =====================================================================
    {
        // Collect type/face histogram: candidates[face][typeId]
        // typeId in 0x40..0x80 (actually, 0x40..0x6a, but the loop below goes through 0x40 values), face in 0..4
        static const int TYPE_MIN = 0x40;
        static const int TYPE_MAX = 0x80;
        static const int FACE_MAX = 5;
        int counts[FACE_MAX][TYPE_MAX] = {};

        POSITION pos = g_PlayersList->GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->GetNext(pos);
            if (!p->is_ai) {
                continue;
            }

            POSITION upos = p->unit_list->unit_list.GetHeadPosition();
            while (upos) {
                Unit* u = p->unit_list->unit_list.GetNext(upos);
                if (u->summon_id) {
                    continue;
                }

                int tid = (uint8_t)u->typeId;
                int fid = (uint8_t)u->face;
                if (fid < 0 || fid >= FACE_MAX) {
                    continue;
                }
                if (tid < TYPE_MIN || tid >= TYPE_MAX) {
                    continue;
                }

                // Type ID filter
                if (0x3f < tid && tid < 0x52 && this->has_kill_all_monsters) {
                } else if (0x66 < tid && tid < 0x6b && this->has_kill_all_monsters) {
                } else if (0x51 < tid && tid < 0x63 && this->has_kill_all_undead) {
                } else {
                    continue;
                }

                // Defense range filter
                int32_t def = u->sub_52D904();
                if (def <= def_min || def >= def_max) {
                    continue;
                }

                counts[fid][tid]++;
            }
        }

        // Build candidate list of (face, typeId) pairs with nonzero counts
        struct Candidate { int face; int typeId; int count; };
        // Vanilla code allocates a fixed array of 256 entries. std::vector is cleaner.
        std::vector<Candidate> candidates;
        for (int f = 0; f < FACE_MAX; f++) {
            for (int t = TYPE_MIN; t < TYPE_MAX; t++) {
                if (counts[f][t] > 0) {
                    candidates.push_back({f, t, counts[f][t]});
                }
            }
        }

        if (candidates.size() > 0) {
            int pick = sub_542216(candidates.size() - 1);
            Candidate& candidate = candidates[pick];

            int type_id = g_GameDataRes.sub_5126A2(candidate.typeId, candidate.face);
            if (type_id == 0xA3) { // Replace campaign orc-shamans with regular ones?
                type_id = 0x51;
            }

            int amount = 0;
            if (candidate.count < 2) {
                amount = sub_542216(3) + 1;
            } else if (candidate.count < 4) {
                amount = sub_542216(4) + 2;
            } else if (candidate.count < 8) {
                amount = sub_542216(5) + 2;
            } else {
                amount = sub_542216(8) + 2;
            }

            int reward = 0;
            if (type_id >= 0 && type_id < g_GameDataRes.monsters.GetSize()) {
                reward = g_GameDataRes.monsters[type_id].values[0].experience * amount;
            }

            Quest* q = MakeActiveQuest(this, off_60F9C0);
            int32_t kind = QuestKind(q);
            int32_t encoded_unit_id = candidate.typeId | (candidate.face << 8);
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, encoded_unit_id, 0, amount, reward);
        }
    }

    // =====================================================================
    //  KillMob (vtable off_60F9F8)
    // =====================================================================
    {
        std::vector<Unit*> candidates;

        POSITION pos = g_PlayersList->GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->GetNext(pos);
            if (!p->is_ai || p == this->pOwner) {
                continue;
            }

            POSITION upos = p->unit_list->unit_list.GetHeadPosition();
            while (upos) {
                Unit* u = p->unit_list->unit_list.GetNext(upos);
                if (!u || !u->field_0x194) {
                    continue;
                }

                uint8_t tid = (uint8_t)u->typeId;

                // Type ID filter
                if (0x3f < u->typeId && u->typeId < 0x52 && this->has_kill_all_monsters) {
                } else if (0x66 < u->typeId && u->typeId < 0x6b && this->has_kill_all_monsters) {
                } else if (0x51 < u->typeId && u->typeId < 0x64 && this->has_kill_all_undead) {
                } else {
                    continue;
                }
                if (this->position->Distance(u->position) >= dist_threshold) {
                    continue;
                }

                int def = u->sub_52D904();
                if (def <= def_min || def >= def_max) {
                    continue;
                }

                candidates.push_back(u);
            }
        }

        if (candidates.size() > 0) {
            int pick = sub_542216(candidates.size() - 1);
            Unit* unit = candidates[pick];
            Building* near_building = building_list->sub_557F69(unit->position);

            int32_t obj = unit->building_id;
            int32_t near_id = near_building ? near_building->building_id : 0;
            int32_t landmark_id = near_id | 0xffff0000;

            Quest* q = MakeActiveQuest(this, off_60F9F8);
            int32_t kind = QuestKind(q);
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, obj, landmark_id, 0, unit->_exp * 2);
        }
    }

    // =====================================================================
    //  KillGroup (vtable off_60F988)
    // =====================================================================
    {
        std::vector<Group*> candidates;

        POSITION pos = g_PlayersList->GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->GetNext(pos);
            if (!p->is_ai || p == this->pOwner) {
                continue;
            }

            POSITION gpos = p->group_list->groups.GetHeadPosition();
            while (gpos) {
                Group* grp = p->group_list->groups.GetNext(gpos);
                if (!grp->has_quest_kill) {
                    continue;
                }

                POSITION upos = grp->unit_list.GetHeadPosition();
                while (upos) {
                    Unit* u = grp->unit_list.GetNext(upos);
                    int def = u->sub_52D904();
                    if (def <= def_min || def >= def_max) {
                        continue;
                    }
                    if (this->position->Distance(u->position) >= dist_threshold) {
                        continue;
                    }
                    if (g_QuestMap.sub_55E129(0xd, grp->group_id, 0)) {
                        continue;
                    }
                    
                    candidates.push_back(grp);
                    break;
                }
            }
        }

        if (candidates.size() > 0) {
            int pick = sub_542216(candidates.size() - 1);
            Group* grp = candidates[pick];

            int avg_x = 0, avg_y = 0, count = 0, max_exp = 0;

            POSITION upos = grp->unit_list.GetHeadPosition();
            while (upos) {
                Unit* u = grp->unit_list.GetNext(upos);
                avg_x += u->position->x;
                avg_y += u->position->y;
                count++;
                if (u->_exp > max_exp) {
                    max_exp = u->_exp;
                }
            }
            if (count == 0) {
                count = 1;
            }

            avg_x /= count;
            avg_y /= count;

            TokenPos pos(avg_x, avg_y, g_World->field24_0xa50);
            Building* near_building = building_list->sub_557F69(&pos);

            int32_t obj = grp->group_id;
            int32_t near_id = near_building ? near_building->building_id : 0;
            int32_t landmark_id = near_id | 0xffff0000;

            Quest* q = MakeActiveQuest(this, off_60F988);
            int32_t kind = QuestKind(q);
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, obj, landmark_id, 0, max_exp * 5);
        }
    }

    // =====================================================================
    //  InterceptUnit (vtable off_60F800)
    // =====================================================================
    {
        std::vector<Unit*> candidates;

        POSITION pos = g_PlayersList->GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->GetNext(pos);
            if (!p->is_ai) {
                continue;
            }
            if (!(g_World->diplomacy[player->player_id][p->player_id] & 1)) {
                continue;
            }

            POSITION upos = p->unit_list->unit_list.GetHeadPosition();
            while (upos) {
                Unit* u = p->unit_list->unit_list.GetNext(upos);
                if (!u->field_0x19c) {
                    continue;
                }

                if (0x3f < u->typeId && u->typeId < 0x52 && this->has_kill_all_monsters) {
                } else if (0x66 < u->typeId && u->typeId < 0x6b && this->has_kill_all_monsters) {
                } else if (0x51 < u->typeId && u->typeId < 0x64 && this->has_kill_all_undead) {
                } else {
                    continue;
                }

                if (this->position->Distance(u->position) >= dist_threshold2) {
                    continue;
                }

                int32_t def = u->sub_52D904();
                if (def <= def_min || def >= def_max) {
                    continue;
                }
                if (g_QuestMap.sub_55E129(0xc, u->building_id, 0)) {
                    continue;
                }

                candidates.push_back(u);
            }
        }

        if (candidates.size() > 0) {
            int pick = sub_542216(candidates.size() - 1);
            Unit* unit = candidates[pick];
            Building* near_building = building_list->sub_557F69(unit->position);

            int32_t obj = unit->building_id;
            int32_t landmark_id = this->building_id;

            Quest* q = MakeActiveQuest(this, off_60F800);
            int32_t kind = QuestKind(q);
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, obj, landmark_id, 0, unit->_exp * 3);
        }
    }

    // =====================================================================
    //  InterceptGroup (vtable off_60F7C8)
    // =====================================================================
    {
        std::vector<Group*> candidates;

        POSITION pos = g_PlayersList->GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->GetNext(pos);
            if (!p->is_ai) {
                continue;
            }
            if (!(g_World->diplomacy[player->player_id][p->player_id] & 1)) {
                continue;
            }

            POSITION gpos = p->group_list->groups.GetHeadPosition();
            while (gpos) {
                Group* grp = p->group_list->groups.GetNext(gpos);
                if (!grp->has_quest_intercept) {
                    continue;
                }

                // Check units in the group
                int32_t max_exp = 0;
                bool ok = false;
                POSITION upos = grp->unit_list.GetHeadPosition();
                while (upos) {
                    Unit* u = grp->unit_list.GetNext(upos);
                    int32_t def = u->sub_52D904();
                    if (def <= def_min || def >= def_max) {
                        continue;
                    }
                    if (this->position->Distance(u->position) >= dist_threshold2) {
                        continue;
                    }
                    if (g_QuestMap.sub_55E129(0xd, grp->group_id, 0)) {
                        continue;
                    }
                    candidates.push_back(grp);
                    break;
                }
            }
        }

        if (candidates.size() > 0) {
            int pick = sub_542216(candidates.size() - 1);
            Group* grp = candidates[pick];

            int max_exp = 0;

            POSITION upos = grp->unit_list.GetHeadPosition();
            while (upos) {
                Unit* u = grp->unit_list.GetNext(upos);
                if (u->_exp > max_exp) {
                    max_exp = u->_exp;
                }
            }

            int32_t obj = grp->group_id;
            int32_t landmark_id = this->building_id;

            Quest* q = MakeActiveQuest(this, off_60F7C8);
            int32_t kind = QuestKind(q);
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, obj, landmark_id, 0, max_exp * 6);
        }
    }

    // =====================================================================
    //  Escort (vtable off_60F950)
    // =====================================================================
    {
        std::vector<Unit*> candidates;

        int escort_dest = g_QuestMap.sub_55F441(this->building_id);
        if (escort_dest != 0) {
            POSITION upos = this->pOwner ? this->pOwner->unit_list->unit_list.GetHeadPosition() : nullptr;
            while (upos) {
                Unit* u = this->pOwner->unit_list->unit_list.GetNext(upos);
                if (!u->field_0x198 || u->decay != 0) {
                    continue;
                }
                if (g_QuestMap.sub_55E129(0xc, u->building_id, 0)) {
                    continue;
                }
                candidates.push_back(u);
            }
        }
        if (candidates.size() > 0) {
            int pick = sub_542216(candidates.size() - 1);
            Unit* unit = candidates[pick];

            int32_t reward = player->main_unit->_exp / 33;
            int32_t obj = 0;

            Quest* q = MakeActiveQuest(this, off_60F950);
            int32_t kind = QuestKind(q);
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, unit->building_id, escort_dest, 0, reward);
        }
    }

    // =====================================================================
    //  Raise undeads (Ghosts / Skeletons / Zombies --- off_60F838 / off_60F870 / off_60F8A8)
    // =====================================================================
    if ((player->main_unit->unit_attrs & 4) != 0 && this->has_raise_dead != 0) {
        SpellBook* sb = player->main_unit->spell_book;
        if (sb && (sb->sub_53DD3D() & (1 << spell::animate_dead)) != 0) {
            int raise = sub_542216(2);
            const void** vtbl = (raise == 0) ? off_60F838 : (raise == 1) ? off_60F870 : off_60F8A8;
            Quest* q = MakeActiveQuest(this, vtbl);
            int amount = sub_542216(6) + 3;
            int32_t kind = QuestKind(q);
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, 0, this->building_id, amount, 0);
        }
    }

    // =====================================================================
    //  KillInnDefenders (vtable off_60F790)
    // =====================================================================
    {
        // Find an AI enemy player that owns a building and has field_0xa60 != 0,
        // and diplomacy with inn owner says hostile.
        struct Candidate { Player* p; Building* b; };
        std::vector<Candidate> candidates;

        POSITION pos = g_PlayersList->GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->GetNext(pos);
            if (!p->is_ai || !this->pOwner) {
                continue;
            }
            if (!(g_World->diplomacy[this->pOwner->player_id][p->player_id] & 1)) {
                continue;
            }
            if (!p->field_0xa60) {
                continue;
            }

            Building* found_building = building_list->sub_558055(p);
            if (!found_building) {
                continue;
            }

            candidates.push_back({p, found_building});
        }

        if (candidates.size() > 0) {
            int32_t pick = sub_542216(candidates.size() - 1);
            const Candidate& candidate = candidates[pick];

            POSITION upos = candidate.p->unit_list->unit_list.GetHeadPosition();
            int32_t max_exp = 0;
            while (upos) {
                Unit* u = candidate.p->unit_list->unit_list.GetNext(upos);
                if (u->_exp > max_exp) {
                    max_exp = u->_exp;
                }
            }

            int32_t obj = candidate.p->player_id;
            int32_t landmark_id = candidate.b->building_id | 0xffff0000;
            int32_t reward = max_exp * 10;

            Quest* q = MakeActiveQuest(this, off_60F790);
            int32_t kind = QuestKind(q);
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, obj, landmark_id, 0, reward);
        }
    }

    // =====================================================================
    //  DeliverMail (vtable off_60F8E0)
    // =====================================================================
    {
        int32_t dest_id = g_QuestMap.sub_55F441(this->building_id);
        if (dest_id != 0) {
            Building* target_building = building_list->sub_557DB2(dest_id);
            if (target_building) {
                // Compute limit: distance * (1.5 + rand/32767.0) / speed
                int32_t dx = (int32_t)this->position->GetX() - target_building->position->GetX();
                int32_t dy = (int32_t)this->position->GetY() - target_building->position->GetY();
                double dist = std::sqrt((double)(dx * dx + dy * dy)) * 256.0;
                double speed_d = (player->main_unit->speed > 0) ? (double)player->main_unit->speed : 1.0;
                int32_t limit = (int32_t)(dist * (1.5 + (double)rand() / 32767.0) / speed_d);

                Quest* q = MakeActiveQuest(this, off_60F8E0);
                int32_t kind = QuestKind(q);
                int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
                int32_t obj = player->main_unit->building_id;
                int32_t reward = player->main_unit->_exp / 50;
                RegisterQuest(this, q, some_id, player->player_id, this->building_id, obj, dest_id, limit, reward);
            }
        }
    }

    // =====================================================================
    //  DeliverItem (vtable off_60F918)
    // =====================================================================
    {
        int32_t dest_id = g_QuestMap.sub_55F441(this->building_id);
        if (dest_id != 0 && this->delivery_item_id != 0) {
            Quest* q = MakeActiveQuest(this, off_60F918);
            int32_t reward = player->main_unit->_exp / 100;
            int32_t kind = QuestKind(q);
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            int32_t obj = this->delivery_item_id;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, obj, dest_id, 0, reward);
        }
    }
}
