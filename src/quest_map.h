#ifndef QUEST_MAP_H
#define QUEST_MAP_H


#include <cstdint>

#include <afx.h>
#include "assert_offset.h"
#include <afxtempl.h>


class Quest;
class QuestInnGlue;


struct DeferredQuestEvent {
    uint32_t gap_0x0;
    uint32_t event_type;
    uint32_t param_a;
    uint32_t param_b;
    uint32_t gap_0x10;
    uint32_t gap_0x14;
    uint32_t gap_0x18;
};
ASSERT_OFFSET(DeferredQuestEvent, param_b, 0xc);
ASSERT_SIZE(DeferredQuestEvent, 0x1c);

class QuestMap : public CObject {
public: // VTable at 0060f730.
    enum PlayerFlags : uint32_t {
        FLAG_UNREGISTERED    = 0x01,
        FLAG_ACCEPTED        = 0x10,
        FLAG_COMPLETED       = 0x20,
        FLAG_FAILED          = 0x40,
        FLAG_KILLED_BY_OTHER = 0x80,
    };

    virtual ~QuestMap(); // 55d915 (client: 537c83)
    virtual int32_t OnEvent(int32_t event_type, int32_t a, int32_t b); // 55e129 (client: 537ed5)
    
public:
    QuestMap(); // 55d579 (client: 537b72)

    void DeleteGlues(); // 55d6f7 (client: 537bfe) - Reset quest state
    void ProcessDeferredEvents(); // 55e00c (client: 537e5f)
    void ProcessInnGlues(); // 55e027 (client: 537e7a)
    void ProcessQueuedEvents(); // 55de84 (client: 537e01)
    void QueueEvent(int32_t event_type, int32_t a, int32_t b); // 55dd10 (client: 537dbb) - Fire a quest event
    void RegisterQuest(Quest* quest); // 55e24a (client: 537f4f)
    void UnregisterQuest(Quest* quest); // 55e5fb (client: 537fba) - Remove quest from quest map flags
    int32_t PickRandomOtherInn(int32_t building_id); // 55f441 (client: 53865b)
    int32_t FindQuest(uint32_t id, Quest** out); // 4a47a0 (client: 4a7e90)
    void AddInnGlue(QuestInnGlue* glue); // 55ea81 (client: 53805d) - Add QuestInnGlue to quest map
    void RemovePlayerQuests(int32_t player_id); // 55ecfe (client: 538089)

    Quest* CreateQuest(int quest_kind); // 55ee42 (client: 538128)

    int32_t GetQuestCount() { return this->quests_map.GetCount(); }; // 41ec00 (client: 4206e0)

public:
    CMap<uint32_t, uint32_t, Quest*, Quest*> quests_map;
    CList<DeferredQuestEvent*> substructs;
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
