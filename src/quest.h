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

class Quest : public CObject, public QuestData {
public: // 60F6F8. Sub-classes have their own vtables, see FUN_0055ee42.
    // virtual CRuntimeClass* GetRuntimeClass() const;
    virtual ~Quest();                                      // 57FF50
    // virtual void Serialize(CArchive& ar);
    // virtual void AssertValid() const;
    // virtual void Dump(CDumpContext&) const;
    virtual int32_t VMethod1(int32_t param_2, int32_t param_3, int32_t param_4); // sub_55D25D
    virtual void MarkFailed();                             // 55D23F
    virtual void MarkCompleted();                          // 55D221
    virtual void CopyFields(QuestData* other);             // 55D1E1
    virtual void Initialize(int some_id, int player_id, int building_id, int obj, int landmark_id, int limit, int reward); // 55D177
    virtual int32_t Kind();                                // 57ff20
    virtual BOOL IsSomething2();                           // 57ff30
    virtual BOOL IsSomething3();                           // 57ff40

public:
    Quest(); // 55d0e0

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
