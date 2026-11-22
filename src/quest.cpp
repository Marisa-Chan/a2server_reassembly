#include "quest.h"
#include "quest_map.h"

#include <cstring>

// 55d0e0 (client: 5378c7)
Quest::Quest() {
    this->quest_data.state = Quest::STATE_IN_WORK;
    this->quest_data.quest_id = 0;
    this->quest_data.player_id = 0;
    this->quest_data.obj = 0;
    this->quest_data.landmark_id = 0;
    this->quest_data.limit = 0;
    this->progress = 0;
    this->quest_map = nullptr;
    this->target_object = nullptr;
}

// 55d15a (client: 537940)
Quest::~Quest() {
}

// 55d177 (client: 53795c)
void Quest::Initialize(int quest_id, int player_id, int building_id, int obj, int landmark_id, int limit, int reward) {
    this->quest_data.state = Quest::STATE_IN_WORK;
    this->quest_data.quest_id = quest_id;
    this->quest_data.player_id = player_id;
    this->quest_data.building_id = building_id;
    this->quest_data.obj = obj;
    this->quest_data.landmark_id = landmark_id;
    this->quest_data.limit = limit;
    this->progress = 0;
    this->reward = reward;
    this->target_object = nullptr;
}

// 55d1e1 (client: 5379c6)
void Quest::CopyFields(QuestData* other) {
    memcpy(&this->quest_data, other, sizeof(QuestData));
    this->progress = 0;
    this->target_object = nullptr;
    this->reward = 0;
}

// 55d221 (client: 537a06)
void Quest::MarkCompleted() {
    if (this->quest_data.state == Quest::STATE_IN_WORK) {
        this->quest_data.state = Quest::STATE_COMPLETED;
    }
}

// 55d23f (client: 537a24)
void Quest::MarkFailed() {
    if (this->quest_data.state != Quest::STATE_FAILED) {
        this->quest_data.state = Quest::STATE_FAILED;
    }
}

// 55d25d (client: 537a42)
int32_t Quest::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    int32_t result = 0;

    if (event_type == Quest::EVENT_FORCE_COMPLETE) {
        if (a == this->quest_data.quest_id && this->quest_data.state == Quest::STATE_IN_WORK) {
            this->MarkCompleted();
            uint32_t old_flags = 0;
            this->quest_map->flags.Lookup(this->quest_data.player_id, old_flags);
            this->quest_map->flags[this->quest_data.player_id] = old_flags | QuestMap::FLAG_COMPLETED;
        }
    } else if (event_type == Quest::EVENT_FORCE_FAIL) {
        if (a == this->quest_data.quest_id && this->quest_data.state == Quest::STATE_IN_WORK) {
            this->MarkFailed();
            uint32_t old_flags = 0;
            this->quest_map->flags.Lookup(this->quest_data.player_id, old_flags);
            this->quest_map->flags[this->quest_data.player_id] = old_flags | QuestMap::FLAG_FAILED;
        }
    } else if (event_type == Quest::EVENT_QUERY_PLAYER_INN_QUEST) {
        if (a == this->quest_data.player_id && b == this->quest_data.building_id && this->quest_data.state == Quest::STATE_IN_WORK) {
            result = this->quest_data.quest_id;
        }
    }

    return result;
}

// 57ff20 (client: 550a10)
int32_t Quest::Kind() {
    return Quest::KIND_NONE;
}

// 57ff30 (client: 550a20)
int32_t Quest::GetGlueEvent() {
    return 0;
}

// 57ff40 (client: 550a30)
int32_t Quest::GetGlueParam() {
    return 0;
}

// 4a4780 (client: 4a7e70)
uint32_t Quest::GetLimit() {
    return this->quest_data.limit;
}


// 581630 (client: 5524a0)
QuestKillMob::~QuestKillMob() {}

// 55f736 (client: 53912f)
int32_t QuestKillMob::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_UNIT_KILLED) {
        if (a == this->quest_data.obj && (b & 0xffff) == this->quest_data.player_id && this->quest_data.state != Quest::STATE_COMPLETED) {
            this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
        }
    } else if (event_type == Quest::EVENT_QUERY_UNIT_ACTIVE && a == this->quest_data.obj && this->quest_data.state == Quest::STATE_IN_WORK) {
        return this->quest_data.quest_id;
    }
    return Quest::OnEvent(event_type, a, b);
}

// 5815f0 (client: 552460)
int32_t QuestKillMob::Kind() {
    return Quest::KIND_KILL_MOB;
}


// 581690 (client: 551ea0)
QuestKillN::~QuestKillN() {}

// 55f7e4 (client: 53895a)
int32_t QuestKillN::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_UNIT_KILLED && (b >> 16) == this->quest_data.obj && (b & 0xffff) == this->quest_data.player_id) {
        if (this->quest_data.state != Quest::STATE_COMPLETED && this->progress < this->quest_data.limit) {
            this->progress++;
            if (this->progress >= this->quest_data.limit) {
                this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
            }
        }
    }
    return Quest::OnEvent(event_type, a, b);
}

// 581650 (client: 551e60)
int32_t QuestKillN::Kind() {
    return Quest::KIND_KILL_N;
}


// 5816f0 (client: 551e20)
QuestKillGroup::~QuestKillGroup() {}

// 55f896 (client: 5388ac)
int32_t QuestKillGroup::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_GROUP_WIPED) {
        if (a == this->quest_data.obj && (b & 0xffff) == this->quest_data.player_id && this->quest_data.state != Quest::STATE_COMPLETED) {
            this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
        }
    } else if (event_type == Quest::EVENT_QUERY_GROUP_ACTIVE && a == this->quest_data.obj && this->quest_data.state == Quest::STATE_IN_WORK) {
        return this->quest_data.quest_id;
    }
    return Quest::OnEvent(event_type, a, b);
}

// 5816b0 (client: 551de0)
int32_t QuestKillGroup::Kind() {
    return Quest::KIND_KILL_GROUP;
}


// 581780 (client: 552210)
QuestEscort::~QuestEscort() {}

// 55f944 (client: 538df1)
int32_t QuestEscort::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_UNIT_KILLED) {
        if (a == this->quest_data.obj && this->quest_data.state != Quest::STATE_FAILED) {
            this->quest_map->QueueEvent(Quest::EVENT_FORCE_FAIL, this->quest_data.quest_id, 0);
        }
    } else if (event_type == Quest::EVENT_UNIT_REACHED_INN) {
        if (a == this->quest_data.obj && (b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_COMPLETED) {
            this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
        }
    } else if (event_type == Quest::EVENT_QUERY_UNIT_ACTIVE && a == this->quest_data.obj) {
        return this->quest_data.quest_id;
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581710 (client: 5521a0)
int32_t QuestEscort::Kind() {
    return Quest::KIND_ESCORT;
}

// 581720 (client: 5521b0)
int32_t QuestEscort::GetGlueEvent() {
    return Quest::EVENT_UNIT_REACHED_INN;
}

// 581730 (client: 5521c0)
int32_t QuestEscort::GetGlueParam() {
    return this->quest_data.obj;
}


// 581810 (client: 552160)
QuestDeliverItem::~QuestDeliverItem() {}

// 55fa1d (client: 538cc3)
int32_t QuestDeliverItem::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_ITEM_DELIVERED) {
        const uint16_t player_id = (b & 0xffff);
        const uint16_t landmark_id = (b >> 16);

        if (a == this->quest_data.obj && landmark_id == this->quest_data.landmark_id) {
            if (player_id == this->quest_data.player_id && this->quest_data.state != Quest::STATE_COMPLETED) {
                this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
            } else if (player_id != this->quest_data.player_id && this->quest_data.state != Quest::STATE_FAILED) {
                this->quest_map->QueueEvent(Quest::EVENT_FORCE_FAIL, this->quest_data.quest_id, 0);
            }
        }
    } else if (event_type == Quest::EVENT_QUERY_ITEM_ACTIVE && a == this->quest_data.obj && this->quest_data.state == Quest::STATE_IN_WORK) {
        return this->quest_data.quest_id;
    }

    return Quest::OnEvent(event_type, a, b);
}

// 5817a0 (client: 5520f0)
int32_t QuestDeliverItem::Kind() {
    return Quest::KIND_DELIVER_ITEM;
}

// 5817b0 (client: 552100)
int32_t QuestDeliverItem::GetGlueEvent() {
    return Quest::EVENT_ITEM_DELIVERED;
}

// 5817c0 (client: 552110)
int32_t QuestDeliverItem::GetGlueParam() {
    return this->quest_data.obj;
}


// 5818a0 (client: 5520b0)
QuestDeliverMail::~QuestDeliverMail() {}

// 55fb32 (client: 538bd8)
int32_t QuestDeliverMail::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_UNIT_REACHED_INN) {
        if (a == this->quest_data.obj && (b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_COMPLETED && this->quest_data.state != Quest::STATE_FAILED) {
            DWORD tick = GetTickCount();
            if (tick < ((this->quest_data.limit * 1000) >> 4) + this->progress) {
                this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
            } else {
                this->quest_map->QueueEvent(Quest::EVENT_FORCE_FAIL, this->quest_data.quest_id, 0);
            }
        }
    } else if (event_type == Quest::EVENT_START_MAIL_TIMER && a == this->quest_data.quest_id) {
        this->progress = GetTickCount();
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581830 (client: 552040)
int32_t QuestDeliverMail::Kind() {
    return Quest::KIND_DELIVER_MAIL;
}

// 581840 (client: 552050)
int32_t QuestDeliverMail::GetGlueEvent() {
    return Quest::EVENT_UNIT_REACHED_INN;
}

// 581850 (client: 552060)
int32_t QuestDeliverMail::GetGlueParam() {
    return this->quest_data.obj;
}


// 581a50 (client: 552420)
QuestRaiseZombies::~QuestRaiseZombies() {}

// 560111 (client: 539083)
int32_t QuestRaiseZombies::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_ZOMBIES_RAISED && this->quest_data.limit <= (a & 0xffff) && (b & 0xffff) == this->quest_data.player_id) {
        if ((b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_FAILED) {
            this->reward = ((a >> 16) * (a >> 16)) / 5;
            this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
        }
    }

    return Quest::OnEvent(event_type, a, b);
}

// 5819e0 (client: 5523b0)
int32_t QuestRaiseZombies::Kind() {
    return Quest::KIND_RAISE_ZOMBIES;
}

// 5819f0 (client: 5523c0)
int32_t QuestRaiseZombies::GetGlueEvent() {
    return Quest::EVENT_ZOMBIES_RAISED;
}

// 581a00 (client: 5523d0)
int32_t QuestRaiseZombies::GetGlueParam() {
    return this->quest_data.player_id | (this->quest_data.limit << 16);
}


// 581ae0 (client: 552370)
QuestRaiseSkeletons::~QuestRaiseSkeletons() {}

// 5601c7 (client: 538fd1)
int32_t QuestRaiseSkeletons::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_SKELETONS_RAISED && this->quest_data.limit <= (a & 0xffff) && (b & 0xffff) == this->quest_data.player_id) {
        if ((b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_FAILED) {
            this->reward = ((a >> 16) * (a >> 16)) / 5;
            this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
        }
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581a70 (client: 552300)
int32_t QuestRaiseSkeletons::Kind() {
    return Quest::KIND_RAISE_SKELETONS;
}

// 581a80 (client: 552310)
int32_t QuestRaiseSkeletons::GetGlueEvent() {
    return Quest::EVENT_SKELETONS_RAISED;
}

// 581a90 (client: 552320)
int32_t QuestRaiseSkeletons::GetGlueParam() {
    return this->quest_data.player_id | (this->quest_data.limit << 16);
}


// 581b70 (client: 5522c0)
QuestRaiseGhosts::~QuestRaiseGhosts() {}

// 56027d (client: 538f1f)
int32_t QuestRaiseGhosts::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_GHOSTS_RAISED && this->quest_data.limit <= (a & 0xffff) && (b & 0xffff) == this->quest_data.player_id) {
        if ((b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_FAILED) {
            this->reward = ((a >> 16) * (a >> 16)) / 8;
            this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
        }
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581b00 (client: 552250)
int32_t QuestRaiseGhosts::Kind() {
    return Quest::KIND_RAISE_GHOSTS;
}

// 581b10 (client: 552260)
int32_t QuestRaiseGhosts::GetGlueEvent() {
    return Quest::EVENT_GHOSTS_RAISED;
}

// 581b20 (client: 552270)
int32_t QuestRaiseGhosts::GetGlueParam() {
    return this->quest_data.player_id | (this->quest_data.limit << 16);
}


// 581930 (client: 552000)
QuestInterceptUnit::~QuestInterceptUnit() {}

// 55fc25 (client: 538acf)
int32_t QuestInterceptUnit::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_UNIT_KILLED) {
        if (a == this->quest_data.obj && this->quest_data.state != Quest::STATE_COMPLETED && this->quest_data.state != Quest::STATE_FAILED) {
            if ((b & 0xffff) != this->quest_data.player_id) {
                QuestMap* quest_map = this->quest_map;
                quest_map->UnregisterQuest(this);
                quest_map->flags[this->quest_data.player_id] = QuestMap::FLAG_KILLED_BY_OTHER;
                return 0;
            }
            this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
        }
    } else if (event_type == Quest::EVENT_UNIT_REACHED_INN) {
        if (a == this->quest_data.obj && (b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_FAILED) {
            this->quest_map->QueueEvent(Quest::EVENT_FORCE_FAIL, this->quest_data.quest_id, 0);
        }
    } else if (event_type == Quest::EVENT_QUERY_UNIT_ACTIVE && a == this->quest_data.obj) {
        return this->quest_data.quest_id;
    }

    return Quest::OnEvent(event_type, a, b);
}

// 5818c0 (client: 551f90)
int32_t QuestInterceptUnit::Kind() {
    return Quest::KIND_INTERCEPT_UNIT;
}

// 5818d0 (client: 551fa0)
int32_t QuestInterceptUnit::GetGlueEvent() {
    return Quest::EVENT_UNIT_REACHED_INN;
}

// 5818e0 (client: 551fb0)
int32_t QuestInterceptUnit::GetGlueParam() {
    return this->quest_data.obj;
}


// 5819c0 (client: 551f50)
QuestInterceptGroup::~QuestInterceptGroup() {}

// 55fe9b (client: 538a01)
int32_t QuestInterceptGroup::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_GROUP_WIPED) {
        if (a == this->quest_data.obj && this->quest_data.state != Quest::STATE_COMPLETED && this->quest_data.state != Quest::STATE_FAILED) {
            if ((b & 0xffff) != this->quest_data.player_id) {
                // Retail caches the quest_map pointer before the call (55ff33): UnregisterQuest
                // nulls quest->quest_map (55e850), and the flags write still targets the old map.
                QuestMap* quest_map = this->quest_map;
                quest_map->UnregisterQuest(this);
                quest_map->flags[this->quest_data.player_id] = QuestMap::FLAG_KILLED_BY_OTHER;
                return 0;
            }
            this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
        }
    } else if (event_type == Quest::EVENT_INTERCEPT_ESCAPED) {
        if (a == this->quest_data.obj && (b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_FAILED) {
            this->quest_map->QueueEvent(Quest::EVENT_FORCE_FAIL, this->quest_data.quest_id, 0);
        }
    } else if (event_type == Quest::EVENT_QUERY_GROUP_ACTIVE && a == this->quest_data.obj) {
        return this->quest_data.quest_id;
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581950 (client: 551ee0)
int32_t QuestInterceptGroup::Kind() {
    return Quest::KIND_INTERCEPT_GROUP;
}

// 581960 (client: 551ef0)
int32_t QuestInterceptGroup::GetGlueEvent() {
    return Quest::EVENT_INTERCEPT_ESCAPED;
}

// 581970 (client: 551f00)
int32_t QuestInterceptGroup::GetGlueParam() {
    return this->quest_data.obj;
}


// 581bd0 (client: 551da0)
QuestKillInnDefenders::~QuestKillInnDefenders() {}

// 56032d (client: 538805)
int32_t QuestKillInnDefenders::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == Quest::EVENT_INN_DEFENDERS_WIPED && a == this->quest_data.obj && (b & 0xffff) == this->quest_data.player_id && this->quest_data.state != Quest::STATE_FAILED) {
        this->quest_map->QueueEvent(Quest::EVENT_FORCE_COMPLETE, this->quest_data.quest_id, 0);
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581b90 (client: 551d60)
int32_t QuestKillInnDefenders::Kind() {
    return Quest::KIND_KILL_INN_DEFENDERS;
}
