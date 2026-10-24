#ifndef QUEST_GLUE_H
#define QUEST_GLUE_H

#include <cstdint>

#include <afx.h>
#include "assert_offset.h"
#include <afxtempl.h>
#include "quest.h"
#include "token.h"


class QuestMap;
struct World;


// Some object used to tie an inn and a quest.
class QuestInnGlue : public CObject {
public: // VTable at 60fde8.
    virtual ~QuestInnGlue();
    virtual void ProcessQuests();
    virtual void ProcessEscortTarget(Quest* quest);
    virtual void ProcessDeliverItem(Quest* quest);
    virtual void ProcessInterceptGroup(Quest* quest);
    virtual void SpawnZombieMobs(Quest* quest);
    virtual void SpawnSkeletonMobs(Quest* quest);
    virtual void SpawnGhostMobs(Quest* quest);
    virtual uint8_t GetX();
    virtual uint8_t GetY();

public:
    QuestInnGlue(int32_t building_id, TokenPos* pos, World* world, int32_t max_dist); // sub_5BA8E0 (client: 5885e0)

public:
    uint32_t building_id;
    CMap<uint32_t, uint32_t, Quest*, Quest*> map;
    QuestMap* quest_map;
    POSITION current_assoc;
    Quest* current_quest;
    uint32_t current_quest_key;
    TokenPos* position;
    World* world;
    uint32_t max_dist;
};

ASSERT_OFFSET(QuestInnGlue, building_id, 0x4);
// MFC140 CMap is 4 bytes larger than VC6's, so chain off the previous field
ASSERT_OFFSET(QuestInnGlue, current_quest_key, offsetof(QuestInnGlue, current_quest) + 4);
ASSERT_SIZE(QuestInnGlue, 0x40);


#endif
