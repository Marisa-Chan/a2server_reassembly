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
        if ((uint32_t)param_3 == this->some_id && this->state == 0) {
            this->MarkCompleted();
            uint32_t old_flags = 0;
            this->quest_map->flags.Lookup(this->player_id, old_flags);
            this->quest_map->flags[this->player_id] = old_flags | 0x20;
        }
    } else if (param_2 == 2) {
        if ((uint32_t)param_3 == this->some_id && this->state == 0) {
            this->MarkFailed();
            uint32_t old_flags = 0;
            this->quest_map->flags.Lookup(this->player_id, old_flags);
            this->quest_map->flags[this->player_id] = old_flags | 0x40;
        }
    } else if (param_2 == 0x11) {
        if ((uint32_t)param_3 == this->player_id && (uint32_t)param_4 == this->building_id && this->state == 0) {
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
