#include "quest_glue.h"

#include "building.h"
#include "buildings_list.h"
#include "game_app.h"
#include "group.h"
#include "inventory.h"
#include "item.h"
#include "net.h"
#include "player.h"
#include "players_list.h"
#include "quest_map.h"
#include "server.h"
#include "unit.h"
#include "world.h"

// 5BA8E0
QuestInnGlue::QuestInnGlue(int32_t building_id, TokenPos* pos, World* world, int32_t max_dist) {
    this->building_id = building_id;
    this->position = pos;
    this->world = world;
    this->max_dist = max_dist;
}

// 5BB350 / 5BB380
QuestInnGlue::~QuestInnGlue() {
}

// 55F589
void QuestInnGlue::ProcessQuests() {
    POSITION pos = this->map.GetStartPosition();
    while (pos != nullptr) {
        uint32_t key;
        Quest* quest;
        this->map.GetNextAssoc(pos, key, quest);
        this->current_quest_key = key;
        this->current_quest = quest;
        switch (quest->Something2()) {
        case 6:  this->ProcessEscortTarget(this->current_quest); break;
        case 7:  this->ProcessDeliverItem(this->current_quest); break;
        case 8:  this->SpawnZombieMobs(this->current_quest); break;
        case 9:  this->SpawnSkeletonMobs(this->current_quest); break;
        case 10: this->SpawnGhostMobs(this->current_quest); break;
        case 11: this->ProcessInterceptGroup(this->current_quest); break;
        }
    }
}

// 5BA920
void QuestInnGlue::ProcessEscortTarget(Quest* quest) {
    int32_t inn_id = quest->Something3();
    if (quest->field_0x2c == 0) {
        Unit* unit = this->world->field24_0xa50->field69_0xa456c->FindById(inn_id);
        quest->field_0x2c = unit;
    }
    Unit* unit = (Unit*)quest->field_0x2c;
    if (unit == nullptr) {
        return;
    }
    uint16_t pos2 = this->position->GetYX();
    uint16_t pos1 = unit->position->GetYX();
    uint8_t dist = this->world->field24_0xa50->ChebyshevDistance(pos1, pos2);
    if (dist > this->max_dist) {
        return;
    }
    if (quest->Kind() == 4) {
        Building* building = g_Server->srv_stru1->building_list->FindClosest(this->position);
        if (building != nullptr) {
            g_NetStru1_main.TransferUnitOwnership(unit, building->pOwner);
        }
    }
    this->quest_map->QueueEvent(6, inn_id, this->building_id << 16);
}

// 5BAA38
void QuestInnGlue::ProcessDeliverItem(Quest* quest) {
    uint16_t item_type = (uint16_t)quest->Something3() | 0xE00;
    bool found = false;
    int32_t player_id = 0;
    for (POSITION pos = this->world->players_list->list.GetHeadPosition(); pos != nullptr;) {
        Player* player = this->world->players_list->list.GetNext(pos);
        if (player->is_ai != 0 || player->main_unit == nullptr) {
            continue;
        }
        uint16_t pos2 = this->position->GetYX();
        uint16_t pos1 = player->main_unit->position->GetYX();
        uint8_t dist = this->world->field24_0xa50->ChebyshevDistance(pos1, pos2);
        if (dist > this->max_dist) {
            continue;
        }
        if (!player->main_unit->inventory->FindItemById(item_type)) {
            continue;
        }
        found = true;
        Item* item = player->main_unit->inventory->TakeItemById(item_type);
        if (item != nullptr) {
            delete item;
        }
        g_NetStru1_main.SendUnitUpdate(player->main_unit, player, 0x202000, 0xFFB, 0, 0);
        player_id = (int32_t)player->player_id;
    }

    if (found) {
        int32_t item_id = quest->Something3();
        this->quest_map->QueueEvent(7, item_id, (this->building_id << 16) | player_id);
    }
}

// 5BABAA
void QuestInnGlue::ProcessInterceptGroup(Quest* quest) {
    int32_t group_id = quest->Something3();
    bool found = false;
    if (quest->field_0x2c == nullptr) {
        POSITION pos = this->world->players_list->list.GetHeadPosition();
        while (pos != nullptr && quest->field_0x2c == nullptr) {
            Player* player = this->world->players_list->list.GetNext(pos);
            if (player->is_ai == 0) {
                continue;
            }

            POSITION gpos = player->group_list->groups.GetHeadPosition();
            while (gpos != nullptr) {
                Group* group = player->group_list->groups.GetNext(gpos);
                if (group->group_id == group_id) {
                    quest->field_0x2c = group;
                    break;
                }
            }
        }
    }

    Group* group = (Group*)quest->field_0x2c;
    if (group == nullptr) {
        return;
    }

    POSITION pos = group->unit_list.GetHeadPosition();
    while (pos != nullptr) {
        Unit* unit = group->unit_list.GetNext(pos);
        uint16_t pos2 = this->position->GetYX();
        uint16_t pos1 = unit->position->GetYX();
        uint8_t dist = this->world->field24_0xa50->ChebyshevDistance(pos1, pos2);
        if (dist <= this->max_dist) {
            found = true;
        }
    }

    if (found) {
        this->quest_map->QueueEvent(11, group_id, this->building_id << 16);
    }
}

// 5BACFF
void QuestInnGlue::SpawnZombieMobs(Quest* quest) {
    const uint16_t mob_type_id = 0x52; // F_Zombie.1

    int32_t player_id = quest->Something3() & 0xFFFF;
    int32_t count_needed = quest->Something3() >> 16;
    if (quest->field_0x2c == nullptr) {
        quest->field_0x2c = g_PlayersList->FindById(player_id);
    }

    Player* player = (Player*)quest->field_0x2c;
    int32_t count = 0;
    int16_t hp_sum = 0;
    for (POSITION gpos = player->group_list->groups.GetHeadPosition(); gpos != nullptr;) {
        Group* group = player->group_list->groups.GetNext(gpos);
        for (POSITION upos = group->unit_list.GetHeadPosition(); upos != nullptr;) {
            Unit* unit = group->unit_list.GetNext(upos);
            if (unit->typeId != mob_type_id) {
                continue;
            }
            uint16_t pos2 = this->position->GetYX();
            uint16_t pos1 = unit->position->GetYX();
            uint8_t dist = this->world->field24_0xa50->ChebyshevDistance(pos1, pos2);
            if (dist > this->max_dist) {
                continue;
            }
            count++;
            hp_sum += unit->hp_max;
        }
    }

    if (count < count_needed) {
        return;
    }

    this->quest_map->QueueEvent(8, (hp_sum << 16) | count, (this->building_id << 16) | player_id);

    for (POSITION gpos = player->group_list->groups.GetHeadPosition(); gpos != nullptr;) {
        Group* group = player->group_list->groups.GetNext(gpos);
        for (POSITION upos = group->unit_list.GetHeadPosition(); upos != nullptr;) {
            Unit* unit = group->unit_list.GetNext(upos);
            if (unit->typeId != mob_type_id) {
                continue;
            }
            uint16_t pos2 = this->position->GetYX();
            uint16_t pos1 = unit->position->GetYX();
            uint8_t dist = this->world->field24_0xa50->ChebyshevDistance(pos1, pos2);
            if (dist <= this->max_dist) {
                unit->hp = -50;
            }
        }
    }
}

// 5BAF04
void QuestInnGlue::SpawnSkeletonMobs(Quest* quest) {
    const uint16_t mob_type_id = 0x59; // F_Skeleton.1
    
    int32_t player_id = quest->Something3() & 0xFFFF;
    int32_t count_needed = quest->Something3() >> 16;
    if (quest->field_0x2c == nullptr) {
        quest->field_0x2c = g_PlayersList->FindById(player_id);
    }

    Player* player = (Player*)quest->field_0x2c;
    int32_t count = 0;
    int16_t hp_sum = 0;
    for (POSITION gpos = player->group_list->groups.GetHeadPosition(); gpos != nullptr;) {
        Group* group = player->group_list->groups.GetNext(gpos);
        for (POSITION upos = group->unit_list.GetHeadPosition(); upos != nullptr;) {
            Unit* unit = group->unit_list.GetNext(upos);
            if (unit->typeId != mob_type_id) {
                continue;
            }
            uint16_t pos2 = this->position->GetYX();
            uint16_t pos1 = unit->position->GetYX();
            uint8_t dist = this->world->field24_0xa50->ChebyshevDistance(pos1, pos2);
            if (dist > this->max_dist) {
                continue;
            }
            count++;
            hp_sum += unit->hp_max;
        }
    }

    if (count < count_needed) {
        return;
    }

    for (POSITION gpos = player->group_list->groups.GetHeadPosition(); gpos != nullptr;) {
        Group* group = player->group_list->groups.GetNext(gpos);
        for (POSITION upos = group->unit_list.GetHeadPosition(); upos != nullptr;) {
            Unit* unit = group->unit_list.GetNext(upos);
            if (unit->typeId != mob_type_id) {
                continue;
            }
            uint16_t pos2 = this->position->GetYX();
            uint16_t pos1 = unit->position->GetYX();
            uint8_t dist = this->world->field24_0xa50->ChebyshevDistance(pos1, pos2);
            if (dist <= this->max_dist) {
                unit->hp = -50;
            }
        }
    }
    this->quest_map->QueueEvent(9, (hp_sum << 16) | count, (this->building_id << 16) | player_id);
}

// 5BB10A
void QuestInnGlue::SpawnGhostMobs(Quest* quest) {
    const uint16_t mob_type_id = 0x45; // Ghost

    int32_t player_id = quest->Something3() & 0xFFFF;
    int32_t count_needed = quest->Something3() >> 16;
    if (quest->field_0x2c == nullptr) {
        quest->field_0x2c = g_PlayersList->FindById(player_id);
    }

    Player* player = (Player*)quest->field_0x2c;
    int32_t count = 0;
    int16_t hp_sum = 0;
    for (POSITION gpos = player->group_list->groups.GetHeadPosition(); gpos != nullptr;) {
        Group* group = player->group_list->groups.GetNext(gpos);
        for (POSITION upos = group->unit_list.GetHeadPosition(); upos != nullptr;) {
            Unit* unit = group->unit_list.GetNext(upos);
            if (unit->typeId != mob_type_id) {
                continue;
            }
            uint16_t pos2 = this->position->GetYX();
            uint16_t pos1 = unit->position->GetYX();
            uint8_t dist = this->world->field24_0xa50->ChebyshevDistance(pos1, pos2);
            if (dist > this->max_dist) {
                continue;
            }
            count++;
            hp_sum += unit->hp_max;
        }
    }

    if (count < count_needed) {
        return;
    }

    for (POSITION gpos = player->group_list->groups.GetHeadPosition(); gpos != nullptr;) {
        Group* group = player->group_list->groups.GetNext(gpos);
        for (POSITION upos = group->unit_list.GetHeadPosition(); upos != nullptr;) {
            Unit* unit = group->unit_list.GetNext(upos);
            if (unit->typeId != mob_type_id) {
                continue;
            }
            uint16_t pos2 = this->position->GetYX();
            uint16_t pos1 = unit->position->GetYX();
            uint8_t dist = this->world->field24_0xa50->ChebyshevDistance(pos1, pos2);
            if (dist <= this->max_dist) {
                unit->hp = -50;
            }
        }
    }

    this->quest_map->QueueEvent(10, (hp_sum << 16) | count, (this->building_id << 16) | player_id);
}

// 5BB310
uint8_t QuestInnGlue::GetX() {
    return this->position->GetX();
}

// 5BB32B
uint8_t QuestInnGlue::GetY() {
    return this->position->GetY();
}
