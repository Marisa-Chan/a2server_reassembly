#ifndef QUEST_GLUE_H
#define QUEST_GLUE_H

#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"
#include "quest.h"
#include "token.h"


class QuestMap;
struct World;


// Some object used to tie an inn and a quest.
class QuestInnGlue : public CObject {
public: // VTable at 60fde8.
    virtual ~QuestInnGlue();
    virtual void VMethod1();
    virtual void VMethod2(Quest* quest);
    virtual void VMethod3(Quest* quest);
    virtual void VMethod4(Quest* quest);
    virtual void VMethod5(Quest* quest);
    virtual void VMethod6(Quest* quest);
    virtual void VMethod7(Quest* quest);
    virtual uint8_t VMethod8();
    virtual uint8_t VMethod9();

public:
    QuestInnGlue(int32_t building_id, TokenPos* pos, World* world, int32_t field_0x3c); // sub_5BA8E0

public:
    uint32_t building_id;
    CMap<uint32_t, uint32_t, Quest*, Quest*> map;
    QuestMap* quest_map;
    CMap<uint32_t, uint32_t, Quest*, Quest*>::CAssoc* current_assoc;
    Quest* field_0x2c;
    uint32_t field_0x30;
    TokenPos* position;
    World* world;
    uint32_t field_0x3c;
};

ASSERT_OFFSET(QuestInnGlue, building_id, 0x4);
ASSERT_OFFSET(QuestInnGlue, field_0x2c, 0x2c);
ASSERT_SIZE(QuestInnGlue, 0x40);


#endif
