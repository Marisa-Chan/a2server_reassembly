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
QuestInnGlue::QuestInnGlue(int32_t building_id, TokenPos* pos, World* world, int32_t field_0x3c) {
    this->building_id = building_id;
    this->position = pos;
    this->world = world;
    this->field_0x3c = field_0x3c;
}

// 5BB350 / 5BB380
QuestInnGlue::~QuestInnGlue() {
}

// 55F589
void QuestInnGlue::VMethod1() {
    POSITION pos = this->map.GetStartPosition();
    while (pos != nullptr) {
        uint32_t key;
        Quest* quest;
        this->map.GetNextAssoc(pos, key, quest);
        this->field_0x30 = key;
        this->field_0x2c = quest;
        switch (quest->Something2()) {
        case 6:  this->VMethod2(this->field_0x2c); break;
        case 7:  this->VMethod3(this->field_0x2c); break;
        case 8:  this->VMethod5(this->field_0x2c); break;
        case 9:  this->VMethod6(this->field_0x2c); break;
        case 10: this->VMethod7(this->field_0x2c); break;
        case 11: this->VMethod4(this->field_0x2c); break;
        }
    }
}

// 5BA920
void QuestInnGlue::VMethod2(Quest* quest) {
    int32_t inn_id = quest->Something3();
    if (quest->field_0x2c == 0) {
        Unit* unit = this->world->field24_0xa50->field69_0xa456c->sub_5560D2(inn_id);
        quest->field_0x2c = unit;
    }
    Unit* unit = (Unit*)quest->field_0x2c;
    if (unit == nullptr) {
        return;
    }
    uint16_t pos2 = this->position->GetYX();
    uint16_t pos1 = unit->position->GetYX();
    uint8_t dist = this->world->field24_0xa50->sub_593B29(pos1, pos2);
    if (dist > this->field_0x3c) {
        return;
    }
    if (quest->Kind() == 4) {
        Building* building = g_Server->srv_stru1->building_list->sub_557F69(this->position);
        if (building != nullptr) {
            g_NetStru1_main.FUN_004fb4ca(unit, building->pOwner);
        }
    }
    this->quest_map->sub_55DD10(6, inn_id, this->building_id << 16);
}

// 5BAA38
void QuestInnGlue::VMethod3(Quest* quest) {
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
        uint8_t dist = this->world->field24_0xa50->sub_593B29(pos1, pos2);
        if (dist > this->field_0x3c) {
            continue;
        }
        if (!player->main_unit->inventory->sub_552B6B(item_type)) {
            continue;
        }
        found = true;
        Item* item = player->main_unit->inventory->sub_552C37(item_type);
        if (item != nullptr) {
            delete item;
        }
        g_NetStru1_main.sub_519221(player->main_unit, player, 0x202000, 0xFFB, 0, 0);
        player_id = (int32_t)player->player_id;
    }

    if (found) {
        int32_t item_id = quest->Something3();
        this->quest_map->sub_55DD10(7, item_id, (this->building_id << 16) | player_id);
    }
}

// 5BABAA
void QuestInnGlue::VMethod4(Quest* quest) {
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
        uint8_t dist = this->world->field24_0xa50->sub_593B29(pos1, pos2);
        if (dist <= this->field_0x3c) {
            found = true;
        }
    }

    if (found) {
        this->quest_map->sub_55DD10(11, group_id, this->building_id << 16);
    }
}

// 5BACFF
void QuestInnGlue::VMethod5(Quest* quest) {
    const uint16_t mob_type_id = 0x52; // F_Zombie.1

    int32_t player_id = quest->Something3() & 0xFFFF;
    int32_t count_needed = quest->Something3() >> 16;
    if (quest->field_0x2c == nullptr) {
        quest->field_0x2c = g_PlayersList->sub_535B50(player_id);
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
            uint8_t dist = this->world->field24_0xa50->sub_593B29(pos1, pos2);
            if (dist > this->field_0x3c) {
                continue;
            }
            count++;
            hp_sum += unit->hp_max;
        }
    }

    if (count < count_needed) {
        return;
    }

    this->quest_map->sub_55DD10(8, (hp_sum << 16) | count, (this->building_id << 16) | player_id);

    for (POSITION gpos = player->group_list->groups.GetHeadPosition(); gpos != nullptr;) {
        Group* group = player->group_list->groups.GetNext(gpos);
        for (POSITION upos = group->unit_list.GetHeadPosition(); upos != nullptr;) {
            Unit* unit = group->unit_list.GetNext(upos);
            if (unit->typeId != mob_type_id) {
                continue;
            }
            uint16_t pos2 = this->position->GetYX();
            uint16_t pos1 = unit->position->GetYX();
            uint8_t dist = this->world->field24_0xa50->sub_593B29(pos1, pos2);
            if (dist <= this->field_0x3c) {
                unit->hp = -50;
            }
        }
    }
}

// 5BAF04
void QuestInnGlue::VMethod6(Quest* quest) {
    const uint16_t mob_type_id = 0x59; // F_Skeleton.1
    
    int32_t player_id = quest->Something3() & 0xFFFF;
    int32_t count_needed = quest->Something3() >> 16;
    if (quest->field_0x2c == nullptr) {
        quest->field_0x2c = g_PlayersList->sub_535B50(player_id);
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
            uint8_t dist = this->world->field24_0xa50->sub_593B29(pos1, pos2);
            if (dist > this->field_0x3c) {
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
            uint8_t dist = this->world->field24_0xa50->sub_593B29(pos1, pos2);
            if (dist <= this->field_0x3c) {
                unit->hp = -50;
            }
        }
    }
    this->quest_map->sub_55DD10(9, (hp_sum << 16) | count, (this->building_id << 16) | player_id);
}

// 5BB10A
void QuestInnGlue::VMethod7(Quest* quest) {
    const uint16_t mob_type_id = 0x45; // Ghost

    int32_t player_id = quest->Something3() & 0xFFFF;
    int32_t count_needed = quest->Something3() >> 16;
    if (quest->field_0x2c == nullptr) {
        quest->field_0x2c = g_PlayersList->sub_535B50(player_id);
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
            uint8_t dist = this->world->field24_0xa50->sub_593B29(pos1, pos2);
            if (dist > this->field_0x3c) {
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
            uint8_t dist = this->world->field24_0xa50->sub_593B29(pos1, pos2);
            if (dist <= this->field_0x3c) {
                unit->hp = -50;
            }
        }
    }

    this->quest_map->sub_55DD10(10, (hp_sum << 16) | count, (this->building_id << 16) | player_id);
}

// 5BB310
uint8_t QuestInnGlue::VMethod8() {
    return this->position->GetX();
}

// 5BB32B
uint8_t QuestInnGlue::VMethod9() {
    return this->position->GetY();
}
