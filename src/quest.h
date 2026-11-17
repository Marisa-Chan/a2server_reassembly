#ifndef QUEST_H
#define QUEST_H


#include <cstdint>

#include <afx.h>
#include "assert_offset.h"
#include <afxtempl.h>


class QuestMap;

struct QuestId {
    union {
        uint32_t raw;
        struct {
            uint32_t kind        : 8;  // Quest::KindId
            uint32_t player_id   : 8;  // Player::player_id
            uint32_t building_id : 16; // Originating Inn::building_id
        };
    };

    constexpr QuestId(uint32_t v = 0) : raw(v) {}
    constexpr operator uint32_t() const { return this->raw; }

    static constexpr QuestId Make(uint16_t bld_id, uint8_t pid, uint8_t k) {
        return QuestId((static_cast<uint32_t>(bld_id) << 16) | (static_cast<uint32_t>(pid) << 8) | k);
    }
};
ASSERT_SIZE(QuestId, 4);


// These fields are copied in Quest::CopyFields.
struct QuestData {
    QuestId quest_id;
    uint32_t player_id;
    uint32_t building_id;
    uint32_t state; // 0: started, 1: completed, 2: failed. (3: presented candidate in the inn?)
    uint32_t obj;
    uint32_t landmark_id;
    uint32_t limit; // Limit for a "kill N" quest.
};
ASSERT_SIZE(QuestData, 0x1c);

class Quest : public CObject {
public: // VTable at 60F6F8. Sub-classes have their own vtables, see CreateQuest.
    enum KindId : uint8_t {
        KIND_NONE               = 0,
        KIND_KILL_MOB           = 1,
        KIND_KILL_N             = 2,
        KIND_KILL_GROUP         = 3,
        KIND_ESCORT             = 4,
        KIND_DELIVER_ITEM       = 5,
        KIND_DELIVER_MAIL       = 6,
        KIND_FIND_ITEM          = 7,
        KIND_RAISE_ZOMBIES      = 8,
        KIND_RAISE_SKELETONS    = 9,
        KIND_RAISE_GHOSTS       = 10,
        KIND_INTERCEPT_UNIT     = 11,
        KIND_INTERCEPT_GROUP    = 12,
        KIND_KILL_INN_DEFENDERS = 13,
    };

    enum State : uint32_t {
        STATE_IN_WORK       = 0,
        STATE_COMPLETED     = 1,
        STATE_FAILED        = 2,
        STATE_INN_CANDIDATE = 3,
    };

    enum Event : int32_t {
        EVENT_FORCE_COMPLETE         = 1,
        EVENT_FORCE_FAIL             = 2,
        EVENT_UNIT_KILLED            = 4,
        EVENT_GROUP_WIPED            = 5,
        EVENT_UNIT_REACHED_INN       = 6,
        EVENT_ITEM_DELIVERED         = 7,
        EVENT_ZOMBIES_RAISED         = 8,
        EVENT_SKELETONS_RAISED       = 9,
        EVENT_GHOSTS_RAISED          = 10,
        EVENT_INTERCEPT_ESCAPED      = 11,
        EVENT_QUERY_UNIT_ACTIVE      = 12,
        EVENT_QUERY_GROUP_ACTIVE     = 13,
        EVENT_QUERY_ITEM_ACTIVE      = 14,
        EVENT_INN_DEFENDERS_WIPED    = 15,
        EVENT_START_MAIL_TIMER       = 16,
        EVENT_QUERY_PLAYER_INN_QUEST = 17,
    };

    virtual ~Quest();                                      // 57FF50 (client: 550a40)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b); // 55D25D (client: 537a42)
    virtual void MarkFailed();                             // 55D23F (client: 537a24)
    virtual void MarkCompleted();                          // 55D221 (client: 537a06)
    virtual void CopyFields(QuestData* other);             // 55D1E1 (client: 5379c6)
    virtual void Initialize(int quest_id, int player_id, int building_id, int obj, int landmark_id, int limit, int reward); // 55D177 (client: 53795c)
    virtual int32_t Kind();                                // 57ff20 (client: 550a10)
    virtual int32_t GetGlueEvent();                        // 57ff30 (client: 550a20)
    virtual int32_t GetGlueParam();                        // 57ff40 (client: 550a30)

    int32_t IsInWork() { return this->quest_data.state == STATE_IN_WORK; } // 41eb90 (client: 420f40)
    uint32_t GetObj() { return this->quest_data.obj; }         // 41eb70 (client: 420f20)
    QuestId GetQuestId() { return this->quest_data.quest_id; }          // 41eb10 (client: 420ec0)
    uint32_t GetBuildingId() { return this->quest_data.building_id; }   // 41eb30 (client: 420ee0)
    uint32_t GetLandmarkId() { return this->quest_data.landmark_id; }   // 41eb50 (client: 420f00)
    uint32_t GetLimit(); //4a4780 (client: 4a7e70) in asm

public:
    Quest(); // 55d0e0 (client: 5378c7)

public:
    QuestData quest_data;
    uint32_t progress; // Progress for a "kill N" quest.
    uint32_t reward;
    QuestMap* quest_map;
    void* target_object; // Can be `Unit*`, `Group*` or `Player*`. Maybe it's quest target?
};
ASSERT_OFFSET(Quest, quest_data, 0x4);
ASSERT_OFFSET(Quest, progress, 0x20);
ASSERT_SIZE(Quest, 0x30);

// Quest kind 1.
class QuestKillMob : public Quest {
public: // VTable at 60F9F8
    virtual ~QuestKillMob();                                                         // 581630 (client: 5524a0)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 55F736 (client: 53912f)
    virtual int32_t Kind();                                                          // 5815F0 (client: 552460)
};

// Quest kind 2.
class QuestKillN : public Quest {
public: // VTable at 60F9C0
    virtual ~QuestKillN();                                                           // 581690 (client: 551ea0)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 55F7E4 (client: 53895a)
    virtual int32_t Kind();                                                          // 581650 (client: 551e60)
};

// Quest kind 3.
class QuestKillGroup : public Quest {
public: // VTable at 60F988
    virtual ~QuestKillGroup();                                                       // 5816F0 (client: 551e20)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 55F896 (client: 5388ac)
    virtual int32_t Kind();                                                          // 5816B0 (client: 551de0)
};

// Quest kind 4.
class QuestEscort : public Quest {
public: // VTable at 60F950
    virtual ~QuestEscort();                                                          // 581780 (client: 552210)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 55F944 (client: 538df1)
    virtual int32_t Kind();                                                          // 581710 (client: 5521a0)
    virtual int32_t GetGlueEvent();                                                  // 581720 (client: 5521b0)
    virtual int32_t GetGlueParam();                                                  // 581730 (client: 5521c0)
};

// Quest kind 5.
class QuestDeliverItem : public Quest {
public: // VTable at 60F918
    virtual ~QuestDeliverItem();                                                     // 581810 (client: 552160)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 55FA1D (client: 538cc3)
    virtual int32_t Kind();                                                          // 5817A0 (client: 5520f0)
    virtual int32_t GetGlueEvent();                                                  // 5817B0 (client: 552100)
    virtual int32_t GetGlueParam();                                                  // 5817C0 (client: 552110)
};

// Quest kind 6.
class QuestDeliverMail : public Quest {
public: // VTable at 60F8E0
    virtual ~QuestDeliverMail();                                                     // 5818A0 (client: 5520b0)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 55FB32 (client: 538bd8)
    virtual int32_t Kind();                                                          // 581830 (client: 552040)
    virtual int32_t GetGlueEvent();                                                  // 581840 (client: 552050)
    virtual int32_t GetGlueParam();                                                  // 581850 (client: 552060)
};

// Quest kind 7. Seems unused?
class QuestFindItem : public Quest {
};

// Quest kind 8.
class QuestRaiseZombies : public Quest {
public: // VTable at 60F8A8
    virtual ~QuestRaiseZombies();                                                    // 581A50 (client: 552420)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 560111 (client: 539083)
    virtual int32_t Kind();                                                          // 5819E0 (client: 5523b0)
    virtual int32_t GetGlueEvent();                                                  // 5819F0 (client: 5523c0)
    virtual int32_t GetGlueParam();                                                  // 581A00 (client: 5523d0)
};

// Quest kind 9.
class QuestRaiseSkeletons : public Quest {
public: // VTable at 60F870
    virtual ~QuestRaiseSkeletons();                                                  // 581AE0 (client: 552370)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 5601C7 (client: 538fd1)
    virtual int32_t Kind();                                                          // 581A70 (client: 552300)
    virtual int32_t GetGlueEvent();                                                  // 581A80 (client: 552310)
    virtual int32_t GetGlueParam();                                                  // 581A90 (client: 552320)
};

// Quest kind 10.
class QuestRaiseGhosts : public Quest {
public: // VTable at 60F838
    virtual ~QuestRaiseGhosts();                                                     // 581B70 (client: 5522c0)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 56027D (client: 538f1f)
    virtual int32_t Kind();                                                          // 581B00 (client: 552250)
    virtual int32_t GetGlueEvent();                                                  // 581B10 (client: 552260)
    virtual int32_t GetGlueParam();                                                  // 581B20 (client: 552270)
};

// Quest kind 11.
class QuestInterceptUnit : public Quest {
public: // VTable at 60F800
    virtual ~QuestInterceptUnit();                                                   // 581930 (client: 552000)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 55FC25 (client: 538acf)
    virtual int32_t Kind();                                                          // 5818C0 (client: 551f90)
    virtual int32_t GetGlueEvent();                                                  // 5818D0 (client: 551fa0)
    virtual int32_t GetGlueParam();                                                  // 5818E0 (client: 551fb0)
};

// Quest kind 12.
class QuestInterceptGroup : public Quest {
public: // VTable at 60F7C8
    virtual ~QuestInterceptGroup();                                                  // 5819C0 (client: 551f50)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 55FE9B (client: 538a01)
    virtual int32_t Kind();                                                          // 581950 (client: 551ee0)
    virtual int32_t GetGlueEvent();                                                  // 581960 (client: 551ef0)
    virtual int32_t GetGlueParam();                                                  // 581970 (client: 551f00)
};

// Quest kind 13.
class QuestKillInnDefenders : public Quest {
public: // VTable at 60F790
    virtual ~QuestKillInnDefenders();                                                // 581BD0 (client: 551da0)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b);     // 56032D (client: 538805)
    virtual int32_t Kind();                                                          // 581B90 (client: 551d60)
};

#endif
