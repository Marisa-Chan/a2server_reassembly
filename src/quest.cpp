#include "quest.h"
#include "quest_map.h"

#include <cstring>

// 55d0e0
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

// 57FF50 / 55d15a
Quest::~Quest() {
}

// 55D177
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

// 55D1E1
void Quest::CopyFields(QuestData* other) {
    memcpy(&this->quest_data, other, sizeof(QuestData));
    this->progress = 0;
    this->target_object = nullptr;
    this->reward = 0;
}

// 55D221
void Quest::MarkCompleted() {
    if (this->quest_data.state == Quest::STATE_IN_WORK) {
        this->quest_data.state = Quest::STATE_COMPLETED;
    }
}

// 55D23F
void Quest::MarkFailed() {
    if (this->quest_data.state != Quest::STATE_FAILED) {
        this->quest_data.state = Quest::STATE_FAILED;
    }
}

// 55D25D
int32_t Quest::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    int32_t result = 0;

    if (event_type == 1) {
        if (a == this->quest_data.quest_id && this->quest_data.state == Quest::STATE_IN_WORK) {
            this->MarkCompleted();
            uint32_t old_flags = 0;
            this->quest_map->flags.Lookup(this->quest_data.player_id, old_flags);
            this->quest_map->flags[this->quest_data.player_id] = old_flags | 0x20;
        }
    } else if (event_type == 2) {
        if (a == this->quest_data.quest_id && this->quest_data.state == Quest::STATE_IN_WORK) {
            this->MarkFailed();
            uint32_t old_flags = 0;
            this->quest_map->flags.Lookup(this->quest_data.player_id, old_flags);
            this->quest_map->flags[this->quest_data.player_id] = old_flags | 0x40;
        }
    } else if (event_type == 0x11) {
        if (a == this->quest_data.player_id && b == this->quest_data.building_id && this->quest_data.state == Quest::STATE_IN_WORK) {
            result = this->quest_data.quest_id;
        }
    }

    return result;
}

// 57FF20
int32_t Quest::Kind() {
    return Quest::KIND_NONE;
}

// 57FF30
int32_t Quest::GetGlueEvent() {
    return 0;
}

// 57FF40
int32_t Quest::GetGlueParam() {
    return 0;
}

// 4a4780
uint32_t Quest::GetLimit() {
    return this->quest_data.limit;
}


// 581630
QuestKillMob::~QuestKillMob() {}

// 55F736
int32_t QuestKillMob::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 4) {
        if (a == this->quest_data.obj && (b & 0xffff) == this->quest_data.player_id && this->quest_data.state != Quest::STATE_COMPLETED) {
            this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
        }
    } else if (event_type == 0xc && a == this->quest_data.obj && this->quest_data.state == Quest::STATE_IN_WORK) {
        return this->quest_data.quest_id;
    }
    return Quest::OnEvent(event_type, a, b);
}

// 5815F0
int32_t QuestKillMob::Kind() {
    return Quest::KIND_KILL_MOB;
}


// 581690
QuestKillN::~QuestKillN() {}

// 55F7E4
int32_t QuestKillN::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 4 && (b >> 16) == this->quest_data.obj && (b & 0xffff) == this->quest_data.player_id) {
        if (this->quest_data.state != Quest::STATE_COMPLETED && this->progress < this->quest_data.limit) {
            this->progress++;
            if (this->progress >= this->quest_data.limit) {
                this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
            }
        }
    }
    return Quest::OnEvent(event_type, a, b);
}

// 581650
int32_t QuestKillN::Kind() {
    return Quest::KIND_KILL_N;
}


// 5816F0
QuestKillGroup::~QuestKillGroup() {}

// 55F896
int32_t QuestKillGroup::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 5) {
        if (a == this->quest_data.obj && (b & 0xffff) == this->quest_data.player_id && this->quest_data.state != Quest::STATE_COMPLETED) {
            this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
        }
    } else if (event_type == 0xd && a == this->quest_data.obj && this->quest_data.state == Quest::STATE_IN_WORK) {
        return this->quest_data.quest_id;
    }
    return Quest::OnEvent(event_type, a, b);
}

// 5816B0
int32_t QuestKillGroup::Kind() {
    return Quest::KIND_KILL_GROUP;
}


// 581780
QuestEscort::~QuestEscort() {}

// 55F944
int32_t QuestEscort::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 4) {
        if (a == this->quest_data.obj && this->quest_data.state != Quest::STATE_FAILED) {
            this->quest_map->QueueEvent(2, this->quest_data.quest_id, 0);
        }
    } else if (event_type == 6) {
        if (a == this->quest_data.obj && (b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_COMPLETED) {
            this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
        }
    } else if (event_type == 12 && a == this->quest_data.obj) {
        return this->quest_data.quest_id;
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581710
int32_t QuestEscort::Kind() {
    return Quest::KIND_ESCORT;
}

// 581720
int32_t QuestEscort::GetGlueEvent() {
    return 6;
}

// 581730
int32_t QuestEscort::GetGlueParam() {
    return this->quest_data.obj;
}


// 581810
QuestDeliverItem::~QuestDeliverItem() {}

// 55FA1D
int32_t QuestDeliverItem::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 7) {
        const uint16_t player_id = (b & 0xffff);
        const uint16_t landmark_id = (b >> 16);

        if (a == this->quest_data.obj && landmark_id == this->quest_data.landmark_id) {
            if (player_id == this->quest_data.player_id && this->quest_data.state != Quest::STATE_COMPLETED) {
                this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
            } else if (player_id != this->quest_data.player_id && this->quest_data.state != Quest::STATE_FAILED) {
                this->quest_map->QueueEvent(2, this->quest_data.quest_id, 0);
            }
        }
    } else if (event_type == 14 && a == this->quest_data.obj && this->quest_data.state == Quest::STATE_IN_WORK) {
        return this->quest_data.quest_id;
    }

    return Quest::OnEvent(event_type, a, b);
}

// 5817A0
int32_t QuestDeliverItem::Kind() {
    return Quest::KIND_DELIVER_ITEM;
}

// 5817B0
int32_t QuestDeliverItem::GetGlueEvent() {
    return 7;
}

// 5817C0
int32_t QuestDeliverItem::GetGlueParam() {
    return this->quest_data.obj;
}


// 5818A0
QuestDeliverMail::~QuestDeliverMail() {}

// 55FB32
int32_t QuestDeliverMail::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 6) {
        if (a == this->quest_data.obj && (b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_COMPLETED && this->quest_data.state != Quest::STATE_FAILED) {
            DWORD tick = GetTickCount();
            if (tick < ((this->quest_data.limit * 1000) >> 4) + this->progress) {
                this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
            } else {
                this->quest_map->QueueEvent(2, this->quest_data.quest_id, 0);
            }
        }
    } else if (event_type == 16 && a == this->quest_data.quest_id) {
        this->progress = GetTickCount();
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581830
int32_t QuestDeliverMail::Kind() {
    return Quest::KIND_DELIVER_MAIL;
}

// 581840
int32_t QuestDeliverMail::GetGlueEvent() {
    return 6;
}

// 581850
int32_t QuestDeliverMail::GetGlueParam() {
    return this->quest_data.obj;
}


// 581A50
QuestRaiseZombies::~QuestRaiseZombies() {}

// 560111
int32_t QuestRaiseZombies::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 8 && this->quest_data.limit <= (a & 0xffff) && (b & 0xffff) == this->quest_data.player_id) {
        if ((b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_FAILED) {
            this->reward = ((a >> 16) * (a >> 16)) / 5;
            this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
        }
    }

    return Quest::OnEvent(event_type, a, b);
}

// 5819E0
int32_t QuestRaiseZombies::Kind() {
    return Quest::KIND_RAISE_ZOMBIES;
}

// 5819F0
int32_t QuestRaiseZombies::GetGlueEvent() {
    return 8;
}

// 581A00
int32_t QuestRaiseZombies::GetGlueParam() {
    return this->quest_data.player_id | (this->quest_data.limit << 16);
}


// 581AE0
QuestRaiseSkeletons::~QuestRaiseSkeletons() {}

// 5601C7
int32_t QuestRaiseSkeletons::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 9 && this->quest_data.limit <= (a & 0xffff) && (b & 0xffff) == this->quest_data.player_id) {
        if ((b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_FAILED) {
            this->reward = ((a >> 16) * (a >> 16)) / 5;
            this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
        }
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581A70
int32_t QuestRaiseSkeletons::Kind() {
    return Quest::KIND_RAISE_SKELETONS;
}

// 581A80
int32_t QuestRaiseSkeletons::GetGlueEvent() {
    return 9;
}

// 581A90
int32_t QuestRaiseSkeletons::GetGlueParam() {
    return this->quest_data.player_id | (this->quest_data.limit << 16);
}


// 581B70
QuestRaiseGhosts::~QuestRaiseGhosts() {}

// 56027D
int32_t QuestRaiseGhosts::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 10 && this->quest_data.limit <= (a & 0xffff) && (b & 0xffff) == this->quest_data.player_id) {
        if ((b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_FAILED) {
            this->reward = ((a >> 16) * (a >> 16)) / 8;
            this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
        }
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581B00
int32_t QuestRaiseGhosts::Kind() {
    return Quest::KIND_RAISE_GHOSTS;
}

// 581B10
int32_t QuestRaiseGhosts::GetGlueEvent() {
    return 10;
}

// 581B20
int32_t QuestRaiseGhosts::GetGlueParam() {
    return this->quest_data.player_id | (this->quest_data.limit << 16);
}


// 581930
QuestInterceptUnit::~QuestInterceptUnit() {}

// 55FC25
int32_t QuestInterceptUnit::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 4) {
        if (a == this->quest_data.obj && this->quest_data.state != Quest::STATE_COMPLETED && this->quest_data.state != Quest::STATE_FAILED) {
            if ((b & 0xffff) != this->quest_data.player_id) {
                this->quest_map->UnregisterQuest(this);
                this->quest_map->flags[this->quest_data.player_id] = 0x80;
                return 0;
            }
            this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
        }
    } else if (event_type == 6) {
        if (a == this->quest_data.obj && (b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_FAILED) {
            this->quest_map->QueueEvent(2, this->quest_data.quest_id, 0);
        }
    } else if (event_type == 0xc && a == this->quest_data.obj) {
        return this->quest_data.quest_id;
    }

    return Quest::OnEvent(event_type, a, b);
}

// 5818C0
int32_t QuestInterceptUnit::Kind() {
    return Quest::KIND_INTERCEPT_UNIT;
}

// 5818D0
int32_t QuestInterceptUnit::GetGlueEvent() {
    return 6;
}

// 5818E0
int32_t QuestInterceptUnit::GetGlueParam() {
    return this->quest_data.obj;
}


// 5819C0
QuestInterceptGroup::~QuestInterceptGroup() {}

// 55FE9B
int32_t QuestInterceptGroup::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 5) {
        if (a == this->quest_data.obj && this->quest_data.state != Quest::STATE_COMPLETED && this->quest_data.state != Quest::STATE_FAILED) {
            if ((b & 0xffff) != this->quest_data.player_id) {
                this->quest_map->UnregisterQuest(this);
                this->quest_map->flags[this->quest_data.player_id] = 0x80;
                return 0;
            }
            this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
        }
    } else if (event_type == 11) {
        if (a == this->quest_data.obj && (b >> 16) == this->quest_data.landmark_id && this->quest_data.state != Quest::STATE_FAILED) {
            this->quest_map->QueueEvent(2, this->quest_data.quest_id, 0);
        }
    } else if (event_type == 13 && a == this->quest_data.obj) {
        return this->quest_data.quest_id;
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581950
int32_t QuestInterceptGroup::Kind() {
    return Quest::KIND_INTERCEPT_GROUP;
}

// 581960
int32_t QuestInterceptGroup::GetGlueEvent() {
    return 11;
}

// 581970
int32_t QuestInterceptGroup::GetGlueParam() {
    return this->quest_data.obj;
}


// 581BD0
QuestKillInnDefenders::~QuestKillInnDefenders() {}

// 56032D
int32_t QuestKillInnDefenders::OnEvent(int32_t event_type, int32_t a, int32_t b) {
    if (event_type == 15 && a == this->quest_data.obj && (b & 0xffff) == this->quest_data.player_id && this->quest_data.state != Quest::STATE_FAILED) {
        this->quest_map->QueueEvent(1, this->quest_data.quest_id, 0);
    }

    return Quest::OnEvent(event_type, a, b);
}

// 581B90
int32_t QuestKillInnDefenders::Kind() {
    return Quest::KIND_KILL_INN_DEFENDERS;
}
