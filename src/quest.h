#ifndef QUEST_H
#define QUEST_H


#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"


class QuestMap;


// These fields are copied in Quest::CopyFields.
struct QuestData {
    uint32_t some_id;
    uint32_t player_id;
    uint32_t building_id;
    uint32_t state; // 0: started, 1: completed, 2: failed. (3: presented candidate in the inn?)
    uint32_t obj;
    uint32_t landmark_id;
    uint32_t limit; // Limit for a "kill N" quest.
};
ASSERT_SIZE(QuestData, 0x1c);

class Quest : public CObject {
public: // VTable at 60F6F8. Sub-classes have their own vtables, see FUN_0055ee42.
    virtual ~Quest();                                      // 57FF50
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4); // sub_55D25D
    virtual void MarkFailed();                             // 55D23F
    virtual void MarkCompleted();                          // 55D221
    virtual void CopyFields(QuestData* other);             // 55D1E1
    virtual void Initialize(int some_id, int player_id, int building_id, int obj, int landmark_id, int limit, int reward); // 55D177
    virtual int32_t Kind();                                // 57ff20
    virtual int32_t Something2();                          // 57ff30
    virtual int32_t Something3();                          // 57ff40

public:
    Quest(); // 55d0e0

public:
    QuestData quest_data;
    uint32_t progress; // Progress for a "kill N" quest.
    uint32_t reward;
    QuestMap* quest_map;
    void* field_0x2c; // Can be `Unit*`, `Group*` or `Player*`. Maybe it's quest target?
};
ASSERT_OFFSET(Quest, quest_data, 0x4);
ASSERT_OFFSET(Quest, progress, 0x20);
ASSERT_SIZE(Quest, 0x30);

// Quest kind 1.
class QuestKillMob : public Quest {
public: // VTable at 60F9F8
    virtual ~QuestKillMob();                                                         // 581630
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 55F736
    virtual int32_t Kind();                                                          // 5815F0
};

// Quest kind 2.
class QuestKillN : public Quest {
public: // VTable at 60F9C0
    virtual ~QuestKillN();                                                           // 581690
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 55F7E4
    virtual int32_t Kind();                                                          // 581650
};

// Quest kind 3.
class QuestKillGroup : public Quest {
public: // VTable at 60F988
    virtual ~QuestKillGroup();                                                       // 5816F0
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 55F896
    virtual int32_t Kind();                                                          // 5816B0
};

// Quest kind 4.
class QuestEscort : public Quest {
public: // VTable at 60F950
    virtual ~QuestEscort();                                                          // 581780
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 55F944
    virtual int32_t Kind();                                                          // 581710
    virtual int32_t Something2();                                                    // 581720
    virtual int32_t Something3();                                                    // 581730
};

// Quest kind 5.
class QuestDeliverItem : public Quest {
public: // VTable at 60F918
    virtual ~QuestDeliverItem();                                                     // 581810
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 55FA1D
    virtual int32_t Kind();                                                          // 5817A0
    virtual int32_t Something2();                                                    // 5817B0
    virtual int32_t Something3();                                                    // 5817C0
};

// Quest kind 6.
class QuestDeliverMail : public Quest {
public: // VTable at 60F8E0
    virtual ~QuestDeliverMail();                                                     // 5818A0
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 55FB32
    virtual int32_t Kind();                                                          // 581830
    virtual int32_t Something2();                                                    // 581840
    virtual int32_t Something3();                                                    // 581850
};

// Quest kind 7. Seems unused?
class QuestFindItem : public Quest {
};

// Quest kind 8.
class QuestRaiseZombies : public Quest {
public: // VTable at 60F8A8
    virtual ~QuestRaiseZombies();                                                    // 581A50
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 560111
    virtual int32_t Kind();                                                          // 5819E0
    virtual int32_t Something2();                                                    // 5819F0
    virtual int32_t Something3();                                                    // 581A00
};

// Quest kind 9.
class QuestRaiseSkeletons : public Quest {
public: // VTable at 60F870
    virtual ~QuestRaiseSkeletons();                                                  // 581AE0
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 5601C7
    virtual int32_t Kind();                                                          // 581A70
    virtual int32_t Something2();                                                    // 581A80
    virtual int32_t Something3();                                                    // 581A90
};

// Quest kind 10.
class QuestRaiseGhosts : public Quest {
public: // VTable at 60F838
    virtual ~QuestRaiseGhosts();                                                     // 581B70
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 56027D
    virtual int32_t Kind();                                                          // 581B00
    virtual int32_t Something2();                                                    // 581B10
    virtual int32_t Something3();                                                    // 581B20
};

// Quest kind 11.
class QuestInterceptUnit : public Quest {
public: // VTable at 60F800
    virtual ~QuestInterceptUnit();                                                   // 581930
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 55FC25
    virtual int32_t Kind();                                                          // 5818C0
    virtual int32_t Something2();                                                    // 5818D0
    virtual int32_t Something3();                                                    // 5818E0
};

// Quest kind 12.
class QuestInterceptGroup : public Quest {
public: // VTable at 60F7C8
    virtual ~QuestInterceptGroup();                                                  // 5819C0
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 55FE9B
    virtual int32_t Kind();                                                          // 581950
    virtual int32_t Something2();                                                    // 581960
    virtual int32_t Something3();                                                    // 581970
};

// Quest kind 13.
class QuestKillInnDefenders : public Quest {
public: // VTable at 60F790
    virtual ~QuestKillInnDefenders();                                                // 581BD0
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);     // 56032D
    virtual int32_t Kind();                                                          // 581B90
};

#endif
