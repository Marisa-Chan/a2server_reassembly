#include "quest.h"
#include "quest_map.h"

#include <cstring>

// 55d0e0
Quest::Quest() {
    this->state = 0;
    this->some_id = 0;
    this->player_id = 0;
    this->obj = 0;
    this->landmark_id = 0;
    this->limit = 0;
    this->progress = 0;
    this->quest_map = nullptr;
    this->field_0x2c = nullptr;
}

// 57FF50 / 55d15a
Quest::~Quest() {
}

// 55D177
void Quest::Initialize(int some_id, int player_id, int building_id, int obj, int landmark_id, int limit, int reward) {
    this->state = 0;
    this->some_id = some_id;
    this->player_id = player_id;
    this->building_id = building_id;
    this->obj = obj;
    this->landmark_id = landmark_id;
    this->limit = limit;
    this->progress = 0;
    this->reward = reward;
    this->field_0x2c = nullptr;
}

// 55D1E1
void Quest::CopyFields(QuestData* other) {
    memcpy(&this->some_id, other, sizeof(QuestData));
    this->progress = 0;
    this->field_0x2c = nullptr;
    this->reward = 0;
}

// 55D221
void Quest::MarkCompleted() {
    if (this->state == 0) {
        this->state = 1;
    }
}

// 55D23F
void Quest::MarkFailed() {
    if (this->state != 2) {
        this->state = 2;
    }
}

// 55D25D
int32_t Quest::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    int32_t result = 0;

    if (param_2 == 1) {
        if (param_3 == this->some_id && this->state == 0) {
            this->MarkCompleted();
            uint32_t old_flags = 0;
            this->quest_map->flags.Lookup(this->player_id, old_flags);
            this->quest_map->flags[this->player_id] = old_flags | 0x20;
        }
    } else if (param_2 == 2) {
        if (param_3 == this->some_id && this->state == 0) {
            this->MarkFailed();
            uint32_t old_flags = 0;
            this->quest_map->flags.Lookup(this->player_id, old_flags);
            this->quest_map->flags[this->player_id] = old_flags | 0x40;
        }
    } else if (param_2 == 0x11) {
        if (param_3 == this->player_id && param_4 == this->building_id && this->state == 0) {
            result = this->some_id;
        }
    }

    return result;
}

// 57FF20
int32_t Quest::Kind() {
    return 0;
}

// 57FF30
int32_t Quest::Something2() {
    return 0;
}

// 57FF40
int32_t Quest::Something3() {
    return 0;
}


// 581630
QuestKillMob::~QuestKillMob() {}

// 55F736
int32_t QuestKillMob::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 4) {
        if (param_3 == this->obj && (param_4 & 0xffff) == this->player_id && this->state != 1) {
            this->quest_map->sub_55DD10(1, this->some_id, 0);
        }
    } else if (param_2 == 0xc && param_3 == this->obj && this->state == 0) {
        return this->some_id;
    }
    return Quest::VMethod1(param_2, param_3, param_4);
}

// 5815F0
int32_t QuestKillMob::Kind() {
    return 1;
}


// 581690
QuestKillN::~QuestKillN() {}

// 55F7E4
int32_t QuestKillN::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 4 && (param_4 >> 16) == this->obj && (param_4 & 0xffff) == this->player_id) {
        if (this->state != 1 && this->progress < this->limit) {
            this->progress++;
            if (this->progress >= this->limit) {
                this->quest_map->sub_55DD10(1, this->some_id, 0);
            }
        }
    }
    return Quest::VMethod1(param_2, param_3, param_4);
}

// 581650
int32_t QuestKillN::Kind() {
    return 2;
}


// 5816F0
QuestKillGroup::~QuestKillGroup() {}

// 55F896
int32_t QuestKillGroup::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 5) {
        if (param_3 == this->obj && (param_4 & 0xffff) == this->player_id && this->state != 1) {
            this->quest_map->sub_55DD10(1, this->some_id, 0);
        }
    } else if (param_2 == 0xd && param_3 == this->obj && this->state == 0) {
        return this->some_id;
    }
    return Quest::VMethod1(param_2, param_3, param_4);
}

// 5816B0
int32_t QuestKillGroup::Kind() {
    return 3;
}


// 581780
QuestEscort::~QuestEscort() {}

// 55F944
int32_t QuestEscort::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 4) {
        if (param_3 == this->obj && this->state != 2) {
            this->quest_map->sub_55DD10(2, this->some_id, 0);
        }
    } else if (param_2 == 6) {
        if (param_3 == this->obj && (param_4 >> 16) == this->landmark_id && this->state != 1) {
            this->quest_map->sub_55DD10(1, this->some_id, 0);
        }
    } else if (param_2 == 12 && param_3 == this->obj) {
        return this->some_id;
    }

    return Quest::VMethod1(param_2, param_3, param_4);
}

// 581710
int32_t QuestEscort::Kind() {
    return 4;
}

// 581720
int32_t QuestEscort::Something2() {
    return 6;
}

// 581730
int32_t QuestEscort::Something3() {
    return this->obj;
}


// 581810
QuestDeliverItem::~QuestDeliverItem() {}

// 55FA1D
int32_t QuestDeliverItem::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 7) {
        if (param_3 == this->obj && (param_4 & 0xffff) == this->player_id && (param_4 >> 16) == this->landmark_id) {
            if (this->state != 1) {
                this->quest_map->sub_55DD10(1, this->some_id, 0);
            } else if (this->state != 2) {
                this->quest_map->sub_55DD10(2, this->some_id, 0);
            }
        }
    } else if (param_2 == 14 && param_3 == this->obj && this->state == 0) {
        return this->some_id;
    }

    return Quest::VMethod1(param_2, param_3, param_4);
}

// 5817A0
int32_t QuestDeliverItem::Kind() {
    return 5;
}

// 5817B0
int32_t QuestDeliverItem::Something2() {
    return 7;
}

// 5817C0
int32_t QuestDeliverItem::Something3() {
    return this->obj;
}


// 5818A0
QuestDeliverMail::~QuestDeliverMail() {}

// 55FB32
int32_t QuestDeliverMail::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 6) {
        if (param_3 == this->obj && (param_4 >> 16) == this->landmark_id && this->state != 1 && this->state != 2) {
            DWORD tick = GetTickCount();
            if (tick < ((this->limit * 1000) >> 4) + this->progress) {
                this->quest_map->sub_55DD10(1, this->some_id, 0);
            } else {
                this->quest_map->sub_55DD10(2, this->some_id, 0);
            }
        }
    } else if (param_2 == 16 && param_3 == this->some_id) {
        this->progress = GetTickCount();
    }

    return Quest::VMethod1(param_2, param_3, param_4);
}

// 581830
int32_t QuestDeliverMail::Kind() {
    return 6;
}

// 581840
int32_t QuestDeliverMail::Something2() {
    return 6;
}

// 581850
int32_t QuestDeliverMail::Something3() {
    return this->obj;
}


// 581A50
QuestRaiseZombies::~QuestRaiseZombies() {}

// 560111
int32_t QuestRaiseZombies::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 8 && this->limit <= (param_3 & 0xffff) && (param_4 & 0xffff) == this->player_id) {
        if ((param_4 >> 16) == this->landmark_id && this->state != 2) {
            this->reward = ((param_3 >> 16) * (param_3 >> 16)) / 5;
            this->quest_map->sub_55DD10(1, this->some_id, 0);
        }
    }

    return Quest::VMethod1(param_2, param_3, param_4);
}

// 5819E0
int32_t QuestRaiseZombies::Kind() {
    return 8;
}

// 5819F0
int32_t QuestRaiseZombies::Something2() {
    return 8;
}

// 581A00
int32_t QuestRaiseZombies::Something3() {
    return this->player_id | (this->limit << 16);
}


// 581AE0
QuestRaiseSkeletons::~QuestRaiseSkeletons() {}

// 5601C7
int32_t QuestRaiseSkeletons::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 9 && this->limit <= (param_3 & 0xffff) && (param_4 & 0xffff) == this->player_id) {
        if ((param_4 >> 16) == this->landmark_id && this->state != 2) {
            this->reward = ((param_3 >> 16) * (param_3 >> 16)) / 5;
            this->quest_map->sub_55DD10(1, this->some_id, 0);
        }
    }

    return Quest::VMethod1(param_2, param_3, param_4);
}

// 581A70
int32_t QuestRaiseSkeletons::Kind() {
    return 9;
}

// 581A80
int32_t QuestRaiseSkeletons::Something2() {
    return 9;
}

// 581A90
int32_t QuestRaiseSkeletons::Something3() {
    return this->player_id | (this->limit << 16);
}


// 581B70
QuestRaiseGhosts::~QuestRaiseGhosts() {}

// 56027D
int32_t QuestRaiseGhosts::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 10 && this->limit <= (param_3 & 0xffff) && (param_4 & 0xffff) == this->player_id) {
        if ((param_4 >> 16) == this->landmark_id && this->state != 2) {
            this->reward = ((param_3 >> 16) * (param_3 >> 16)) / 8;
            this->quest_map->sub_55DD10(1, this->some_id, 0);
        }
    }

    return Quest::VMethod1(param_2, param_3, param_4);
}

// 581B00
int32_t QuestRaiseGhosts::Kind() {
    return 10;
}

// 581B10
int32_t QuestRaiseGhosts::Something2() {
    return 10;
}

// 581B20
int32_t QuestRaiseGhosts::Something3() {
    return this->player_id | (this->limit << 16);
}


// 581930
QuestInterceptUnit::~QuestInterceptUnit() {}

// 55FC25
int32_t QuestInterceptUnit::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 4) {
        if (param_3 == this->obj && this->state != 1 && this->state != 2) {
            if ((param_4 & 0xffff) != this->player_id) {
                this->quest_map->sub_55E5FB(this);
                this->quest_map->flags[this->player_id] = 0x80;
                return 0;
            }
            this->quest_map->sub_55DD10(1, this->some_id, 0);
        }
    } else if (param_2 == 6) {
        if (param_3 == this->obj && (param_4 >> 16) == this->landmark_id && this->state != 2) {
            this->quest_map->sub_55DD10(2, this->some_id, 0);
        }
    } else if (param_2 == 0xc && param_3 == this->obj) {
        return this->some_id;
    }

    return Quest::VMethod1(param_2, param_3, param_4);
}

// 5818C0
int32_t QuestInterceptUnit::Kind() {
    return 11;
}

// 5818D0
int32_t QuestInterceptUnit::Something2() {
    return 6;
}

// 5818E0
int32_t QuestInterceptUnit::Something3() {
    return this->obj;
}


// 5819C0
QuestInterceptGroup::~QuestInterceptGroup() {}

// 55FE9B
int32_t QuestInterceptGroup::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 5) {
        if (param_3 == this->obj && this->state != 1 && this->state != 2) {
            if ((param_4 & 0xffff) != this->player_id) {
                this->quest_map->sub_55E5FB(this);
                this->quest_map->flags[this->player_id] = 0x80;
                return 0;
            }
            this->quest_map->sub_55DD10(1, this->some_id, 0);
        }
    } else if (param_2 == 11) {
        if (param_3 == this->obj && (param_4 >> 16) == this->landmark_id && this->state != 2) {
            this->quest_map->sub_55DD10(2, this->some_id, 0);
        }
    } else if (param_2 == 13 && param_3 == this->obj) {
        return this->some_id;
    }

    return Quest::VMethod1(param_2, param_3, param_4);
}

// 581950
int32_t QuestInterceptGroup::Kind() {
    return 12;
}

// 581960
int32_t QuestInterceptGroup::Something2() {
    return 11;
}

// 581970
int32_t QuestInterceptGroup::Something3() {
    return this->obj;
}


// 581BD0
QuestKillInnDefenders::~QuestKillInnDefenders() {}

// 56032D
int32_t QuestKillInnDefenders::VMethod1(int32_t param_2, int32_t param_3, int32_t param_4) {
    if (param_2 == 15 && param_3 == this->obj && (param_4 & 0xffff) == this->player_id && this->state != 2) {
        this->quest_map->sub_55DD10(1, this->some_id, 0);
    }

    return Quest::VMethod1(param_2, param_3, param_4);
}

// 581B90
int32_t QuestKillInnDefenders::Kind() {
    return 13;
}
