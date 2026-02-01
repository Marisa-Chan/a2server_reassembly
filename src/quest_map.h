#ifndef QUEST_H
#define QUEST_H


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
public: // Virtual table at 0060f730.
    // virtual CRuntimeClass* GetRuntimeClass() const;
    // virtual ~QuestMap();
    // virtual void Serialize(CArchive& ar);
    // virtual CObject AssertValid();
    // virtual CObject Dump();
    // virtual void VMethod1(int param_2,int param_3,int param_4);
public:
    CMap<uint32_t, uint32_t, Quest*, Quest*> quests_map;
    CList<QuestSubstruct*> substructs;
    CMap<uint32_t, uint32_t, QuestInnGlue*, QuestInnGlue*> glues_map;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t> flags;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t>::CAssoc* assoc; // Not sure about the specific argument type, usage seems transient.
    Quest* quest;
    QuestInnGlue* glue;
    uint32_t building_id;
};

ASSERT_OFFSET(QuestMap, flags, 0x58);
ASSERT_SIZE(QuestMap, 0x84);

#endif
