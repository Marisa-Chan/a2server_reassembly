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
    uint32_t state; // 0: started, 1: completed, 2: failed
    uint32_t obj;
    uint32_t landmark_id;
    uint32_t limit; // Limit for a "kill N" quest.
};
ASSERT_SIZE(QuestData, 0x1c);

class Quest : public CObject, public QuestData {
public: // There are different vtables for different quest kinds, see FUN_0055ee42.
    // virtual CRuntimeClass* GetRuntimeClass() const;
    // virtual ~Quest();
    // virtual void Serialize(CArchive& ar);
    // virtual CObject AssertValid();
    // virtual CObject Dump();
    // virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4);
    // virtual void MarkFailed();
    // virtual void MarkCompleted();
    // virtual void CopyFields(QuestData* other);
    // virtual void Initialize(int some_id,int player_id,int building_id,int obj,int landmark_id,int limit,int reward);
    // virtual BOOL IsSomething1();
    // virtual BOOL IsSomething2();
    // virtual BOOL IsSomething3();

public:
    uint32_t progress; // Progress for a "kill N" quest.
    uint32_t reward;
    QuestMap* quest_map;
    uint32_t field_0x2c;
};
ASSERT_OFFSET(Quest, some_id, 0x4);
ASSERT_OFFSET(Quest, progress, 0x20);
ASSERT_SIZE(Quest, 0x30);

// Quest kind 1.
class QuestKillMob : public Quest {
};

// Quest kind 2.
class QuestKillN : public Quest {
};

// Quest kind 3.
class QuestKillGroup : public Quest {
};

// Quest kind 4.
class QuestEscort : public Quest {
};

// Quest kind 5.
class QuestDeliverItem : public Quest {
};

// Quest kind 6.
class QuestDeliverMail : public Quest {
};

// Quest kind 7. Seems unused?
class QuestFindItem : public Quest {
};

// Quest kind 8.
class QuestRaiseZombies : public Quest {
};

// Quest kind 9.
class QuestRaiseSkeletons : public Quest {
};

// Quest kind 10.
class QuestRaiseGhosts : public Quest {
};

// Quest kind 11.
class QuestInterceptUnit : public Quest {
};

// Quest kind 12.
class QuestInterceptGroup : public Quest {
};

// Quest kind 13.
class QuestKillInnDefenders : public Quest {
};

#endif
