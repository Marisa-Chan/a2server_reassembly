#include "inn.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "buildings_list.h"
#include "building.h"
#include "constants.h"
#include "effect.h"
#include "game_app.h"
#include "group.h"
#include "inventory.h"
#include "player.h"
#include "players_list.h"
#include "quest.h"
#include "quest_map.h"
#include "server.h"
#include "shop_assortment.h"
#include "spell.h"
#include "token.h"
#include "unit.h"
#include "unit_list.h"
#include "world.h"
#include "map_stuff.h"

IMPLEMENT_SERIAL(Inn, Building, 1); // 637330.

// 56046B
Inn::Inn() {
    this->has_kill_all_humans = 0;
    this->has_kill_all_monsters = 0;
    this->has_kill_all_undead = 0;
    this->has_raise_dead = 0;
    this->delivery_item_id = 0;
    this->quest_roll_counter = 0;
}

// 5605CC
Inn::Inn(uint8_t type_id, TokenPos* pos) : Building(type_id, pos, 0, 0) {
    this->has_kill_all_humans = 0;
    this->has_kill_all_monsters = 0;
    this->has_kill_all_undead = 0;
    this->has_raise_dead = 0;
    this->delivery_item_id = 0;
    this->quest_roll_counter = 0;
}

// 56073E
Inn::~Inn() {
    POSITION pos = this->quest_map_per_player.GetStartPosition();
    while (pos) {
        uint32_t player_id;
        QuestMap* qmap;
        this->quest_map_per_player.GetNextAssoc(pos, player_id, qmap);
        delete qmap;
    }
}

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
    q->Initialize(some_id, player_id, building_id, obj, landmark_id, limit, reward);
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

        POSITION pos = g_PlayersList->list.GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->list.GetNext(pos);
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
            int pick = Random0N(candidates.size() - 1);
            Candidate& candidate = candidates[pick];

            int type_id = g_GameDataRes.sub_5126A2(candidate.typeId, candidate.face);
            if (type_id == 0xA3) { // Replace campaign orc-shamans with regular ones?
                type_id = 0x51;
            }

            int amount = 0;
            if (candidate.count < 2) {
                amount = Random0N(3) + 1;
            } else if (candidate.count < 4) {
                amount = Random0N(4) + 2;
            } else if (candidate.count < 8) {
                amount = Random0N(5) + 2;
            } else {
                amount = Random0N(8) + 2;
            }

            int reward = 0;
            if (type_id >= 0 && type_id < g_GameDataRes.monsters.GetSize()) {
                reward = g_GameDataRes.monsters[type_id].Values()[0].experience * amount;
            }

            Quest* q = MakeActiveQuest(this, off_60F9C0);
            int32_t kind = q->Kind();
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

        POSITION pos = g_PlayersList->list.GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->list.GetNext(pos);
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
            int pick = Random0N(candidates.size() - 1);
            Unit* unit = candidates[pick];
            Building* near_building = building_list->sub_557F69(unit->position);

            int32_t obj = unit->building_id;
            int32_t near_id = near_building ? near_building->building_id : 0;
            int32_t landmark_id = near_id | 0xffff0000;

            Quest* q = MakeActiveQuest(this, off_60F9F8);
            int32_t kind = q->Kind();
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, obj, landmark_id, 0, unit->_exp * 2);
        }
    }

    // =====================================================================
    //  KillGroup (vtable off_60F988)
    // =====================================================================
    {
        std::vector<Group*> candidates;

        POSITION pos = g_PlayersList->list.GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->list.GetNext(pos);
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
            int pick = Random0N(candidates.size() - 1);
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
            int32_t kind = q->Kind();
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, obj, landmark_id, 0, max_exp * 5);
        }
    }

    // =====================================================================
    //  InterceptUnit (vtable off_60F800)
    // =====================================================================
    {
        std::vector<Unit*> candidates;

        POSITION pos = g_PlayersList->list.GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->list.GetNext(pos);
            if (!p->is_ai) {
                continue;
            }
            if (!(g_World->diplomacy.diplomacy[player->player_id][p->player_id] & 1)) {
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
            int pick = Random0N(candidates.size() - 1);
            Unit* unit = candidates[pick];
            Building* near_building = building_list->sub_557F69(unit->position);

            int32_t obj = unit->building_id;
            int32_t landmark_id = this->building_id;

            Quest* q = MakeActiveQuest(this, off_60F800);
            int32_t kind = q->Kind();
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, obj, landmark_id, 0, unit->_exp * 3);
        }
    }

    // =====================================================================
    //  InterceptGroup (vtable off_60F7C8)
    // =====================================================================
    {
        std::vector<Group*> candidates;

        POSITION pos = g_PlayersList->list.GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->list.GetNext(pos);
            if (!p->is_ai) {
                continue;
            }
            if (!(g_World->diplomacy.diplomacy[player->player_id][p->player_id] & 1)) {
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
            int pick = Random0N(candidates.size() - 1);
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
            int32_t kind = q->Kind();
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
            int pick = Random0N(candidates.size() - 1);
            Unit* unit = candidates[pick];

            int32_t reward = player->main_unit->_exp / 33;
            int32_t obj = 0;

            Quest* q = MakeActiveQuest(this, off_60F950);
            int32_t kind = q->Kind();
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
            int raise = Random0N(2);
            const void** vtbl = (raise == 0) ? off_60F838 : (raise == 1) ? off_60F870 : off_60F8A8;
            Quest* q = MakeActiveQuest(this, vtbl);
            int amount = Random0N(6) + 3;
            int32_t kind = q->Kind();
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

        POSITION pos = g_PlayersList->list.GetHeadPosition();
        while (pos) {
            Player* p = g_PlayersList->list.GetNext(pos);
            if (!p->is_ai || !this->pOwner) {
                continue;
            }
            if (!(g_World->diplomacy.diplomacy[this->pOwner->player_id][p->player_id] & 1)) {
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
            int32_t pick = Random0N(candidates.size() - 1);
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
            int32_t kind = q->Kind();
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
                int32_t kind = q->Kind();
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
            int32_t kind = q->Kind();
            int32_t some_id = (this->building_id << 16) | (player->player_id << 8) | kind;
            int32_t obj = this->delivery_item_id;
            RegisterQuest(this, q, some_id, player->player_id, this->building_id, obj, dest_id, 0, reward);
        }
    }
}

// 567D74
void Inn::VMethod1() {
    // Phase 1: Remove visitors whose owner has disconnected (field_0x43 == 0).
    POSITION pos = this->unit_list.unit_list.GetHeadPosition();
    while (pos) {
        Unit* unit = this->unit_list.unit_list.GetNext(pos);
        if (unit->pOwner->field_0x43 == 0) {
            this->sub_560DC2(static_cast<Humanoid*>(unit), 0xFFFF);
        }
    }

    // Phase 2: Tick the quest-roll counter; when it exceeds 120 and the inn is
    // empty, refresh quests for each player and reset the counter.
    this->quest_roll_counter++;
    if (this->quest_roll_counter > 120 && this->unit_list.unit_list.m_nCount == 0) {
        POSITION qpos = this->quest_map_per_player.GetStartPosition();
        while (qpos) {
            uint32_t player_id;
            QuestMap* qmap;
            this->quest_map_per_player.GetNextAssoc(qpos, player_id, qmap);
            qmap->sub_55ECFE(player_id);
        }
        this->quest_roll_counter = 0;
    }
}

// Data arrays for InnReward scroll/book/spell generation.
static const std::array<const char*, 34> ScrollsList = {
    "Scroll Fire Ball",
    "Scroll Fire Wall",
    "Scroll Protection from Fire",
    "Scroll Poison Cloud",
    "Scroll Blizzard",
    "Scroll Protection from Water",
    "Scroll Acid Stream",
    "Scroll Prismatic Spray",
    "Scroll Invisibility",
    "Scroll Protection from Air",
    "Scroll Wall of Earth",
    "Scroll Stone Curse",
    "Scroll Protection from Earth",
    "Scroll Bless",
    "Scroll Haste",
    "Scroll Teleport",
    "Scroll Heal",
    "SuperScroll Fire Ball",
    "SuperScroll Fire Wall",
    "SuperScroll Protection from Fire",
    "SuperScroll Poison Cloud",
    "SuperScroll Blizzard",
    "SuperScroll Protection from Water",
    "SuperScroll Acid Stream",
    "SuperScroll Prismatic Spray",
    "SuperScroll Invisibility",
    "SuperScroll Protection from Air",
    "SuperScroll Wall of Earth",
    "SuperScroll Stone Curse",
    "SuperScroll Protection from Earth",
    "SuperScroll Bless",
    "SuperScroll Haste",
    "SuperScroll Teleport",
    "SuperScroll Heal",
};

static const std::array<const char*, 22> TeachSpellBooks = {
    "Book Fire", "Book Fire", "Book Fire", "Book Fire",
    "Book Water", "Book Water", "Book Water", "Book Water",
    "Book Air", "Book Air", "Book Air", "Book Air",
    "Book Earth", "Book Earth", "Book Earth", "Book Earth",
    "Book Astral", "Book Astral", "Book Astral", "Book Astral", "Book Astral", "Book Astral",
};

static const std::array<const char*, 22> TeachSpellMagics = {
    "teachSpell=Fire_Arrow", "teachSpell=Fire_Ball", "teachSpell=Wall_of_Fire", "teachSpell=Protection_from_Fire",
    "teachSpell=Ice_Missile", "teachSpell=Poison_Cloud", "teachSpell=Blizzard", "teachSpell=Protection_from_Water",
    "teachSpell=Lightning", "teachSpell=Prismatic_Spray", "teachSpell=Invisibility", "teachSpell=Protection_from_Air",
    "teachSpell=Diamond_Dust", "teachSpell=Wall_of_Earth", "teachSpell=Stone_Curse", "teachSpell=Protection_from_Earth",
    "teachSpell=Bless", "teachSpell=Haste", "teachSpell=Control_Spirit", "teachSpell=Teleport", "teachSpell=Drain_Life", "teachSpell=Summon",
};

static const std::array<int, 22> TeachSpellIDs = {
    spell::fire_arrow, spell::fire_ball, spell::wall_of_fire, spell::protection_from_fire,
    spell::ice_missile, spell::poison_cloud, spell::blizzard, spell::protection_from_water,
    spell::lightning, spell::prismatic_spray, spell::invisibility, spell::protection_from_air,
    spell::stone_missile, spell::wall_of_earth, spell::stone_curse, spell::protection_from_earth,
    spell::bless, spell::haste, spell::animate_dead, spell::teleport, spell::drain_life, spell::summon,
};

// Helper to delete all items in a CArray<Item*> and clear it.
static void DeleteItemArray(CArray<Item*>& arr) {
    for (int i = 0; i < arr.GetSize(); i++) {
        if (arr[i]) {
            delete arr[i];
        }
    }
    arr.SetSize(0, -1);
}

// Helper to pick one random item from candidates, add to results, delete the rest.
static bool PickOneRandom(CArray<Item*>& candidates, CArray<Item*>& results) {
    if (candidates.GetSize() == 0) {
        return false;
    }
    int pick = Random0N(candidates.GetSize() - 1);
    for (int i = 0; i < candidates.GetSize(); i++) {
        if (i == pick) {
            results.Add(candidates[i]);
        } else {
            if (candidates[i]) {
                delete candidates[i];
            }
        }
    }
    return true;
}

// 565307
void Inn::InnReward(Player* player) {
    int reward = 0;

    // Find completed quest for this player at this inn.
    POSITION pos = g_QuestMap.quests_map.GetStartPosition();
    while (pos) {
        uint32_t key;
        Quest* quest;
        g_QuestMap.quests_map.GetNextAssoc(pos, key, quest);
        if (quest && quest->player_id == player->player_id && quest->building_id == this->building_id && quest->state == 1) {
            reward = quest->reward;
            break;
        }
    }

    if (reward == 0 && player->field_0xa98 <= 50) {
        return;
    }

    if (reward > 16383000) {
        // The reward can't be much higher: gold is sent in a uint16_t multiple of 250,
        // and 65535 * 250 = 16_383_750, so the max reward is just below that.
        reward = 16383000;
    }
    if (reward < 250) {
        reward = 250;
    }

    // Check if player already has a reward stored.
    Inventory* existing;
    if (this->rewards_per_player.Lookup(player->player_id, existing)) {
        return;
    }

    CArray<Item*> temp_shelf;
    CArray<Item*> result_items;
    CArray<Item*> filtered_by_class;
    CArray<Item*> temp_candidates;

    // Determine player class filter: 1 = fighter, 2 = wizard.
    uint8_t item_type_filter;
    if (player->main_unit->unit_attrs & 4) {
        item_type_filter = 2;
    } else {
        item_type_filter = 1;
    }

    // --- Phase 1: Non-magic item ---
    ShopAssortment assortment;
    AssortGenParams params;
    params.max_cost = reward * 2;
    params.min_cost = reward;
    params.max_count = 100;
    params.max_same_count = 1;
    params.flags = 0x1b7fffff;
    assortment.GenerateAssortment(&params);
    assortment.ArrangeShelfs(100, 1, params.min_cost, params.max_cost, &temp_shelf);

    for (int i = 0; i < temp_shelf.GetSize(); i++) {
        Item* item = temp_shelf[i];
        EquipData* equip_data = &item->world_equip->Values()[0];
        if (equip_data->other_param & item_type_filter) {
            filtered_by_class.Add(item);
        } else {
            delete item;
        }
    }
    temp_shelf.SetSize(0, -1);

    // --- Phase 2: Magic item ---
    params.max_cost = (reward * 3) / 2;
    params.min_cost = (reward * 3) / 4;
    params.flags = 0x2bffffff;
    assortment.GenerateAssortment(&params);
    assortment.ArrangeShelfs(100, 1, params.min_cost, params.max_cost, &temp_shelf);

    for (int i = 0; i < temp_shelf.GetSize(); i++) {
        Item* item = temp_shelf[i];
        EquipData* equip_data = &item->world_equip->Values()[0];
        if (equip_data->other_param & item_type_filter) {
            temp_candidates.Add(item);
        } else {
            delete item;
        }
    }
    temp_shelf.SetSize(0, -1);

    // --- Phase 3: Pick one random from each equipment group ---
    if (PickOneRandom(filtered_by_class, result_items)) {
        result_items[result_items.GetSize() - 1]->count = 1; // Set count to 1 for the picked item.
    }

    filtered_by_class.SetSize(0, -1);

    if (PickOneRandom(temp_candidates, result_items)) {
        result_items[result_items.GetSize() - 1]->count = 1; // Set count to 1 for the picked item.
    }

    temp_candidates.SetSize(0, -1);

    // --- Phase 4: Scrolls ---
    for (int i = 0; i < ScrollsList.size(); i++) {
        CString scroll_name(ScrollsList[i]);
        Item* scroll = new Item(scroll_name);

        int32_t exp_800 = scroll->_exp * 800;
        if (scroll->_exp < reward && reward < exp_800) {
            scroll->count = reward / scroll->_exp;
            if (scroll->count > 10) {
                scroll->count = (scroll->count - 10) / 2 + 10;
            }
            if (scroll->count > 20) {
                scroll->count = (scroll->count - 20) / 2 + 20;
            }
            if (scroll->count > 50) {
                scroll->count = (scroll->count - 50) / 2 + 50;
            }
            if (scroll->count > 100) {
                scroll->count = (scroll->count - 100) / 2 + 100;
            }
            temp_candidates.Add(scroll);
        } else {
            delete scroll;
        }
    }

    // --- Phase 5: Pick one random scroll ---
    PickOneRandom(temp_candidates, result_items);

    // --- Phase 6: Spell books (wizards only) ---
    if (player->main_unit->unit_attrs & 4) {
        temp_candidates.SetSize(0, -1);
        for (int i = 0; i < TeachSpellIDs.size(); i++) {
            int32_t known_spells = player->main_unit->spell_book->sub_53DD3D();
            if ((known_spells & (1 << TeachSpellIDs[i])) == 0) {
                CString book_name(TeachSpellBooks[i]);
                Item* book = new Item(book_name);

                book->sub_548F3F(TeachSpellMagics[i]);
                book->VMethod15();

                if (reward / 2 < book->_exp && book->_exp < reward * 5 / 4) {
                    temp_candidates.Add(book);
                } else {
                    delete book;
                }
            }
        }
        PickOneRandom(temp_candidates, result_items);
    }

    // --- Phase 7: Stat potions ---
    if ((player->main_unit->experience > 18000000 && reward >= 1000000) || player->field_0xa98 > 50) {
        bool is_fighter = (player->main_unit->unit_attrs & 4) == 0;
        bool is_female = (player->main_unit->typeId == 0x22 || player->main_unit->typeId == 0x24);

        uint8_t max_body, max_reaction, max_mind, max_spirit;

        if (is_fighter && !is_female) {
            max_body = 52;
            max_reaction = 50;
            max_mind = 48;
            max_spirit = 46;
        } else if (is_fighter && is_female) {
            max_body = 50;
            max_reaction = 52;
            max_mind = 46;
            max_spirit = 48;
        } else if (!is_fighter && !is_female) {
            max_body = 48;
            max_reaction = 46;
            max_mind = 52;
            max_spirit = 50;
        } else { // !is_fighter && is_female
            max_body = 46;
            max_reaction = 48;
            max_mind = 50;
            max_spirit = 52;
        }

        // Note: vanilla logic used a retrying loop.
        std::vector<const char*> upgradable;
        upgradable.reserve(4);

        if (player->main_unit->body - player->main_unit->equipment_extra.body < max_body) {
            upgradable.push_back("Potion Body");
        }
        if (player->main_unit->mind - player->main_unit->equipment_extra.mind < max_mind) {
            upgradable.push_back("Potion Mind");
        }
        if (player->main_unit->spirit - player->main_unit->equipment_extra.spirit < max_spirit) {
            upgradable.push_back("Potion Spirit");
        }
        if (player->main_unit->reaction - player->main_unit->equipment_extra.reaction < max_reaction) {
            upgradable.push_back("Potion Reaction");
        }

        if (!upgradable.empty()) {
            const char* name = upgradable[Random0N(upgradable.size() - 1)];
            result_items.Add(new Item(name));
        }
    }

    // --- Phase 8: Gold ---
    {
        Item* gold = new Item();
        gold->item_id = 0xFFFF;
        gold->count = reward / 250;
        result_items.Add(gold);
    }

    // --- Phase 9: Experience ---
    if (g_ServerConfig.gameType == 0) {
        Item* experience = new Item();
        experience->item_id = 0xFFFE;
        experience->count = reward / 250;
        result_items.Add(experience);
    }

    // --- Phase 10: Mob reward ---
    if (player->unit_list->sub_557AB0() == 0) {
        int mob_idx = g_GameDataRes.sub_50DF19(reward / 16);
        if (mob_idx != 0) {
            Item* mob = new Item();
            mob->item_id = 0xFFFD;
            MonsterInfoData* data = g_GameDataRes.monsters[mob_idx].Values().GetData();
            mob->count = (data->face << 8) | data->type_id;
            result_items.Add(mob);
        }
    }

    // --- Phase 11: Upgrade item ---
    {
        int high_skills = 0;
        for (int i = 1; i < 6; i++) {
            if (player->main_unit->hit_values.skill_levels[i] > 75) {
                high_skills++;
            }
        }

        if ((high_skills > 3 && reward >= 1000000) || player->field_0xa98 > 50) {
            temp_candidates.SetSize(0, -1);

            for (int slot = 1; slot < 13; slot++) {
                Item* equip;
                if (slot == 1) {
                    equip = player->main_unit->weapon;
                } else if (slot == 2) {
                    equip = player->main_unit->shield;
                } else {
                    equip = player->main_unit->equipment[slot];
                }

                if (equip) {
                    equip->sub_54A0BE();
                }

                if (!equip || equip->VMethod16() != 0 || equip->magic_volume <= 0) {
                    continue;
                }

                // Get the first effect on the item.
                Effect* effect = nullptr;
                if (equip->_effects.m_nCount != 0) {
                    effect = equip->_effects.GetHead();
                }
                if (!effect) {
                    continue;
                }

                int mana_cost = g_GameDataRes.magics[effect->effect_id].Values()[0].mana_cost;
                if (mana_cost > equip->magic_volume) {
                    continue;
                }

                uint32_t capacity = 0;
                int32_t effect_max = g_GameDataRes.magics[effect->effect_id].Values()[0].affect_max;
                int32_t diff = 0;

                switch (effect->effect_id) {
                    case 2: case 3: case 4: case 5:
                    case 0x10: case 0x11: case 0x12: case 0x13:
                        capacity = (effect->full_magic_value < effect_max) ? 1 : 0;
                        break;
                    case 6: case 7: case 9: case 10:
                    case 0xC: case 0xD: case 0xF: case 0x2B:
                        diff = effect_max - effect->full_magic_value;
                        capacity = (diff < 9) ? diff : 8;
                        break;
                    case 8: case 0xB:
                        diff = effect_max - effect->full_magic_value;
                        capacity = (diff < 21) ? diff : 20;
                        break;
                    case 0x14: case 0x15: case 0x16: case 0x17: case 0x18: case 0x19:
                    case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
                    case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25:
                        diff = effect_max - effect->full_magic_value;
                        capacity = (diff < 6) ? diff : 5;
                        break;
                    case 0x29:
                        effect_max = 100;
                        diff = 100 - (int)effect->spell_value;
                        capacity = (diff < 9) ? diff : 8;
                        break;
                    case 0x2C: case 0x2D: case 0x2E: case 0x2F: case 0x30: case 0x31:
                        diff = effect_max - effect->damage_min - effect->damage_spread;
                        capacity = (diff < 9) ? diff : 8;
                        break;
                }

                if (capacity == 0) {
                    continue;
                }

                uint32_t upgrade_points = reward * capacity / 8000000;
                if (upgrade_points == 0) {
                    upgrade_points = 1;
                }
                if (upgrade_points > capacity) {
                    upgrade_points = capacity;
                }
                if (upgrade_points * mana_cost > equip->magic_volume) {
                    upgrade_points = equip->magic_volume / mana_cost;
                }

                equip->field15_0x54 = 1;

                // Create a copy of the item.
                Item* copy;
                if (slot == 1) {
                    copy = new Weapon((Weapon*)equip);
                } else if (slot == 2) {
                    copy = new Shield((Shield*)equip);
                } else {
                    copy = new Armor((Armor*)equip);
                }

                // Get the first effect on the copy.
                effect = copy->_effects.GetHead();

                // Reduce magic volume by the cost and apply upgrade.
                copy->magic_volume = copy->magic_volume - mana_cost * upgrade_points;

                if (effect->effect_id == 0x29) {
                    effect->spell_value = effect->spell_value + upgrade_points;
                } else if (effect->effect_id >= 0x2C && effect->effect_id <= 0x31) {
                    effect->damage_min = effect->damage_min + upgrade_points;
                } else {
                    effect->full_magic_value = effect->full_magic_value + upgrade_points;
                }

                temp_candidates.Add(copy);
            }

            PickOneRandom(temp_candidates, result_items);
        }
    }

    // --- Phase 12: Create inventory and store reward ---
    Inventory* inv = new Inventory();
    for (int i = 0; i < result_items.GetSize(); i++) {
        inv->PutItemIntoBagAtDefault(result_items[i]);
    }
    this->rewards_per_player[player->player_id] = inv;
}

// sub_5306EA: Convert experience to skill level (inverse of sub_530726).
extern "C" int __cdecl sub_5306EA(int experience);

// sub_530726: Get experience required for given skill level.
extern "C" uint32_t __cdecl sub_530726(int32_t skill_level);

// sub_560DC2 вЂ” Handle inn quest/reward interaction.
// Called when a humanoid finishes interacting with the inn.
// `id`: < 0x100 = reward item index, >= 0x100 = accepted quest's `some_id`.
void Inn::sub_560DC2(Humanoid* humanoid, int32_t id) {
    // Look up the unit by the building id
    Unit* found = this->unit_list.sub_5560D2(humanoid->building_id);
    if (!found) {
        return;
    }

    // Find and remove humanoid from the inn's unit list
    {
        POSITION pos = this->unit_list.unit_list.GetHeadPosition();
        while (pos) {
            POSITION cur_pos = pos;
            Unit* u = this->unit_list.unit_list.GetNext(pos);
            if (u == (Unit*)humanoid) {
                this->unit_list.unit_list.RemoveAt(cur_pos);
                break;
            }
        }
    }

    // Notify if server field is set
    if (g_Server->field4_0x74 != 0) {
        humanoid->sub_52C409();
    }

    // Special cancel sentinel?
    if (id == 0xAAAAAAAA) {
        int32_t result = g_QuestMap.sub_55E129(0x11, humanoid->pOwner->player_id, this->building_id);
        g_QuestMap.sub_55E129(2, result, 0);
        return;
    }

    if (id >= 0x100) { // Accepting quest.
        int32_t player_id = humanoid->pOwner->player_id;

        // Look up QuestMap for this player
        QuestMap* qm = nullptr;
        if (this->quest_map_per_player.Lookup(player_id, qm)) {
            this->quest_map = qm;

            // Look up quest by id in the quest map
            Quest* quest = nullptr;
            if (qm->quests_map.Lookup(id, quest)) {
                this->active_quest = quest;

                // Process the quest
                qm->sub_55E5FB(this->active_quest);
                this->active_quest->state = 0;
                g_QuestMap.sub_55E24A(this->active_quest);
                g_NetStru1_main.sub_51D4F6(&g_QuestMap, humanoid->pOwner, 0);
                g_NetStru1_main.FUN_0051ce86(0x10, 0, humanoid->pOwner);

                int32_t kind = this->active_quest->Kind();
                switch (kind) {
                    case 4: // Escort
                    {
                        Unit* escort_unit = dword_6CDB3C->sub_5560D2(this->active_quest->obj);
                        if (escort_unit == nullptr || humanoid == nullptr) {
                            g_QuestMap.sub_55E5FB(this->active_quest);
                        } else {
                            g_NetStru1_main.FUN_004fb4ca(escort_unit, humanoid->pOwner);
                        }
                        break;
                    }
                    case 5: // DeliverItem
                    {
                        uint8_t subtype = this->active_quest->obj;
                        Item* new_item = new Item(0xE, subtype);
                        humanoid->inventory->PutItemIntoBagAtDefault(new_item);
                        g_NetStru1_main.sub_519221(humanoid, humanoid->pOwner, 0x202000, 0xFFB, 0, 0);
                        break;
                    }
                    case 6: // DeliverMail
                    {
                        this->active_quest->progress = GetTickCount();
                        break;
                    }
                    case 11: // Intercept unit
                    {
                        uint16_t unit_id = this->active_quest->obj;
                        Unit* target_unit = dword_6CDB3C->sub_5560D2(unit_id);
                        if (target_unit == nullptr) {
                            g_QuestMap.sub_55E5FB(this->active_quest);
                        } else {
                            target_unit->group->group_sub->active = 1;
                            uint8_t y = this->position->GetY();
                            uint8_t x = this->position->GetX();
                            g_World->sub_5AC785(target_unit->group, x, y);
                        }
                        break;
                    }
                    case 12: // Intercept group
                    {
                        POSITION ppos = g_PlayersList->list.GetHeadPosition();
                        Player* pl = nullptr;
                        while (ppos) {
                            pl = g_PlayersList->list.GetNext(ppos);
                            if (pl->is_ai) {
                                POSITION gpos = pl->group_list->groups.GetHeadPosition();
                                bool found_group = false;
                                while (gpos) {
                                    Group* grp = pl->group_list->groups.GetNext(gpos);
                                    if (grp->group_id == this->active_quest->obj) {
                                        grp->group_sub->active = 1;
                                        uint8_t y = this->position->GetY();
                                        uint8_t x = this->position->GetX();
                                        g_World->sub_5AC785(grp, x, y);
                                        pl = nullptr;
                                        found_group = true;
                                        break;
                                    }
                                }
                                if (found_group) {
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
    } else {
        // ---- Reward item path (id < 256) ----
        int32_t player_id = humanoid->pOwner->player_id;

        // Look up Inventory from rewards_per_player
        Inventory* item_list = nullptr;
        if (this->rewards_per_player.Lookup(player_id, item_list)) {
            int32_t inv_count = item_list->items.GetCount();
            Item* item = item_list->sub_552E42(id, 0xFFFF);
            if (item != nullptr) {
                uint16_t item_hash = item->item_id;
                if (item_hash == 0xFFFD) {
                    // ---- Monster reward ----
                    uint8_t low_byte = item->count & 0xFF;
                    uint8_t high_byte = (item->count >> 8) & 0xFF;
                    int monster_index = g_GameDataRes.sub_50DDAE(low_byte, high_byte);

                    Unit* new_unit = new Unit(g_GameDataRes.monsters[monster_index].name);

                    if (new_unit != nullptr) {
                        uint8_t y = humanoid->position->GetY();
                        uint8_t x = humanoid->position->GetX();
                        int placed = new_unit->sub_52BF3D(x, y, 5);
                        if (placed != 0) {
                            dword_6CDB3C->AddTailAllocId(new_unit);
                            new_unit->pOwner = humanoid->pOwner;
                            humanoid->pOwner->unit_list->AddTail(new_unit);

                            Group* new_group = new Group();
                            CList<Group*>& grp_list = humanoid->pOwner->group_list->groups;
                            POSITION node = grp_list.AddTail(new_group);

                            new_group->AddUnit(new_unit);
                            g_World->sub_5A9A6A(new_unit);
                            g_World->sub_5ACDF4(new_group);
                            g_NetStru1_main.sub_519221(new_unit, nullptr, 0xFFFFFFFF, 0xFFB, 0, 0);
                        } else {
                            // Placement failed вЂ” clean up
                            delete new_unit;
                        }
                    }
                } else if (item_hash == 0xFFFE) {
                    // ---- Experience reward ----
                    uint32_t base_exp = ((uint32_t)item->count) * 250;
                    int32_t carry = 0;
                    int32_t max_skill_exp = sub_530726(100);

                    for (int32_t sphere = 1; sphere <= 5; sphere++) {
                        if (sphere == 5 || sphere == humanoid->main_sphere) {
                            carry += base_exp / 5;
                        } else {
                            carry = carry / 8 + base_exp / 40;
                        }

                        if (humanoid->experience_per_sphere[sphere - 1] >= max_skill_exp) {
                            // Already at max
                            humanoid->experience_per_sphere[sphere - 1] = max_skill_exp;
                        } else if (humanoid->experience_per_sphere[sphere - 1] + carry >= max_skill_exp) {
                            // Would exceed max вЂ” clamp
                            carry -= (max_skill_exp - humanoid->experience_per_sphere[sphere - 1]);
                            humanoid->experience_per_sphere[sphere - 1] = max_skill_exp;
                        } else {
                            // Add carry to this sphere
                            humanoid->experience_per_sphere[sphere - 1] += carry;
                            carry = 0;

                            int32_t new_level = sub_5306EA(humanoid->experience_per_sphere[sphere - 1]);
                            if (new_level > humanoid->hit_values2.skill_levels[sphere]) {
                                g_NetStru1_main.FUN_0051ce86(2, sphere, humanoid->pOwner);

                                int16_t clamped_level;
                                humanoid->hit_values2.skill_levels[sphere] = new_level;
                                if (humanoid->hit_values2.skill_levels[sphere] > 100) {
                                    clamped_level = 100;
                                } else {
                                    clamped_level = humanoid->hit_values2.skill_levels[sphere];
                                }
                                humanoid->hit_values.skill_levels[sphere] = clamped_level + humanoid->equipment_extra.hit_values.skill_levels[sphere];
                            }
                        }
                    }

                    // Recalculate total experience
                    humanoid->experience = 0;
                    for (int32_t sphere = 1; sphere <= 5; sphere++) {
                        humanoid->experience += humanoid->experience_per_sphere[sphere - 1];
                    }
                    g_NetStru1_main.sub_519221(humanoid, nullptr, 0xA31FFFFF, 0xFFB, 0, 0);
                } else if (item_hash == 0xFFFF) {
                    // ---- Money reward ----
                    humanoid->pOwner->sub_534AC1(((uint32_t)item->count) * 250, 0);
                } else {
                    // ---- Regular item ----
                    bool is_item_improvement = (id >= inv_count - 1);

                    if (!is_item_improvement) {
                        // Not last item вЂ” just give it to the humanoid
                        item->TokenID = 1;
                        humanoid->inventory->PutItemIntoBagAtDefault(item);
                        g_NetStru1_main.sub_519221(humanoid, nullptr, 0x282000, 0xFFB, 0, 0);
                    } else {
                        // Last item вЂ” try to equip it, so `equipped` would be the item that was on the player before.
                        Item* equipped = item->VMethod10(humanoid);
                        if (equipped == nullptr || equipped->field15_0x54 == 0) {
                            // Failed --- burn the item.
                            item->VMethod11(humanoid);
                            delete item;
                            if (equipped != nullptr) {
                                delete equipped;
                            }
                            g_NetStru1_main.FUN_0051ce86(5, humanoid->pOwner->player_id, humanoid->pOwner);
                        } else {
                            // Use succeeded вЂ” clean up equipped placeholder
                            delete equipped;
                        }
                        g_NetStru1_main.sub_519221(humanoid, nullptr, 0x482000, 0xFFB, 0, 0);
                    }
                }
            }

            // Clean up the rewards_per_player entry
            this->rewards_per_player.RemoveKey(player_id);

            // Delete all remaining items from the inventory
            while (item_list->items.GetCount() > 0) {
                Item* remaining = item_list->items.GetHead();
                POSITION pos = item_list->items.Find(remaining);
                if (pos) {
                    item_list->items.RemoveAt(pos);
                }
                delete remaining;
            }
            delete item_list;
        }
    }

    // Find and clean up any pending quest for this player
    this->active_quest = this->sub_567A25(humanoid->pOwner);
    if (this->active_quest != nullptr) {
        g_QuestMap.sub_55E5FB(this->active_quest);
        if (this->active_quest != nullptr) {
            delete this->active_quest;
        }
    }
}
