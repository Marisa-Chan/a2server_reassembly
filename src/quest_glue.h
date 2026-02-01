#ifndef QUEST_GLUE_H
#define QUEST_GLUE_H

#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"
#include "token.h"


class QuestMap;
class World;


// Some object used to tie an inn and a quest.
class QuestInnGlue : public CObject {
public: // See the vtable at 0060fde8.
    // virtual CRuntimeClass* GetRuntimeClass() const;
    // virtual ~QuestInnGlue();
    // virtual void Serialize(CArchive& ar);
    // virtual CObject AssertValid();
    // virtual CObject Dump();
    // virtual void VMethod1();
    // virtual void VMethod2(int32_t param_2);
    // virtual void VMethod3(int32_t param_2);
    // virtual void VMethod4(int32_t param_2);
    // virtual void VMethod5(int32_t param_2);
    // virtual void VMethod6(int32_t param_2);
    // virtual void VMethod7(int32_t param_2);
    // virtual uint8_t VMethod8(); // returns position->x
    // virtual uint8_t VMethod9(); // returns position->y
public:
    uint32_t building_id;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t> map; // Don't know the key/value types.
    QuestMap* quest_map;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t>::CAssoc* current_assoc;
    void* field_0x2c; // Same type as the value in `map`.
    uint32_t field_0x30;
    TokenPos* position;
    World* world;
    uint32_t field_0x3c;
};

ASSERT_OFFSET(QuestInnGlue, building_id, 0x4);
ASSERT_OFFSET(QuestInnGlue, field_0x2c, 0x2c);
ASSERT_SIZE(QuestInnGlue, 0x40);


#endif
