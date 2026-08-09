#ifndef QUEST_MAP_H
#define QUEST_MAP_H


#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"


class Quest;
class QuestInnGlue;


struct QuestSubstruct {
    uint32_t field_0x0;
    uint32_t id;
    uint32_t building_id;
    uint32_t some_id;
    uint32_t field_0x10;
    uint32_t field_0x14;
    uint32_t field_0x18;
};
ASSERT_OFFSET(QuestSubstruct, some_id, 0xc);
ASSERT_SIZE(QuestSubstruct, 0x1c);

class QuestMap : public CObject {
public: // VTable at 0060f730.
    virtual ~QuestMap(); // 580f70 / 55d915 
    virtual int32_t VMethod1(int32_t event_type, int32_t a, int32_t b); // 55e129
    
public:
    QuestMap(); // sub_55D579

    void sub_55D6F7(); // Reset quest state
    void sub_55E00C();
    void sub_55E027();
    void sub_55DE84();
    void sub_55DD10(int32_t event_type, int32_t a, int32_t b); // Fire a quest event
    void sub_55E24A(Quest* quest);
    void sub_55E5FB(Quest* quest); // Remove quest from quest map flags
    int32_t sub_55F441(int32_t building_id);
    void sub_55EA81(QuestInnGlue* glue); // Add QuestInnGlue to quest map
    void sub_55ECFE(int32_t player_id);

    Quest* FUN_0055ee42(int quest_kind);

    int32_t FUN_0041ec00() { return quests_map.GetCount(); }; //41ec00

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
