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
    virtual ~QuestInnGlue();  // client: 589050
    virtual void ProcessQuests();  // client: 5386f6
    virtual void ProcessEscortTarget(Quest* quest);  // client: 588620
    virtual void ProcessDeliverItem(Quest* quest);  // client: 588738
    virtual void ProcessInterceptGroup(Quest* quest);  // client: 5888aa
    virtual void SpawnZombieMobs(Quest* quest);  // client: 5889ff
    virtual void SpawnSkeletonMobs(Quest* quest);  // client: 588c04
    virtual void SpawnGhostMobs(Quest* quest);  // client: 588e0a
    virtual uint8_t GetX();  // client: 589010
    virtual uint8_t GetY();  // client: 58902b

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
