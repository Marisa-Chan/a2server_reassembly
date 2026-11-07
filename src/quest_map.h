#ifndef QUEST_MAP_H
#define QUEST_MAP_H


#include <cstdint>

#include <afx.h>
#include "assert_offset.h"
#include <afxtempl.h>


class Quest;
class QuestInnGlue;


struct QuestSubstruct {
    uint32_t gap_0x0;
    uint32_t id;
    uint32_t building_id;
    uint32_t some_id;
    uint32_t gap_0x10;
    uint32_t gap_0x14;
    uint32_t gap_0x18;
};
ASSERT_OFFSET(QuestSubstruct, some_id, 0xc);
ASSERT_SIZE(QuestSubstruct, 0x1c);

class QuestMap : public CObject {
public: // VTable at 0060f730.
    virtual ~QuestMap(); // 580f70 (client: 537c83) / 55d915 
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b); // 55e129 (client: 537ed5)
    
public:
    QuestMap(); // sub_55D579 (client: 537b72)

    void DeleteGlues(); // Reset quest state (client: 537bfe)
    void ProcessDeferredEvents();  // client: 537e5f
    void ProcessInnGlues();  // client: 537e7a
    void ProcessQueuedEvents();  // client: 537e01
    void QueueEvent(int32_t event_type, int32_t a, int32_t b); // Fire a quest event (client: 537dbb)
    void RegisterQuest(Quest* quest);  // client: 537f4f
    void UnregisterQuest(Quest* quest); // Remove quest from quest map flags (client: 537fba)
    int32_t PickRandomOtherInn(int32_t building_id);  // client: 53865b
    int32_t FindQuest(uint32_t id, Quest** out); //4a47a0 (client: 4a7e90) in asm
    void AddInnGlue(QuestInnGlue* glue); // Add QuestInnGlue to quest map (client: 53805d)
    void RemovePlayerQuests(int32_t player_id);  // client: 538089

    Quest* CreateQuest(int quest_kind);  // client: 538128

    int32_t GetQuestCount() { return this->quests_map.GetCount(); }; //41ec00 (client: 4206e0)

public:
    CMap<uint32_t, uint32_t, Quest*, Quest*> quests_map;
    CList<QuestSubstruct*> substructs;
    CMap<uint32_t, uint32_t, QuestInnGlue*, QuestInnGlue*> glues_map;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t> flags;
    POSITION assoc; // Not sure about the specific argument type, usage seems transient.
    Quest* quest;
    QuestInnGlue* glue;
    uint32_t building_id;
};

ASSERT_OFFSET(QuestMap, flags, 0x58);
ASSERT_SIZE(QuestMap, 0x84);

extern QuestMap g_QuestMap; // Global QuestMap instance (at 0x6CE4D8)

#endif
