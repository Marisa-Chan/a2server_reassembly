#ifndef SERVER_H
#define SERVER_H

#include <array>
#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"


class Building;
class Sack;
struct ScriptSettings;
class Spell;
class VirtualCaster;
class Unit;
class UnitList;

struct CowardActivation {
    char key[100];
    BOOL enabled;
};
ASSERT_OFFSET(CowardActivation, enabled, 0x64);
ASSERT_SIZE(CowardActivation, 0x68);

class SrvStru1Lst;

struct SrvStru1 {
    CList<Building*>* building_list;
    SrvStru1Lst* effects_list;
    CList<Sack*>* sack_list;
    UnitList* units_list;
    CList<VirtualCaster*> virtual_casters_list;
    CList<Unit*> some_unit_list;
};
ASSERT_SIZE(SrvStru1, 0x48);

class SrvStru1Lst {
public: // VTable at 0060f6dc.
    virtual void VMethod1();

public:
    CList<SrvStru1*> list;
};
ASSERT_SIZE(SrvStru1Lst, 0x20);

class Srv1 : public CObject { // Aka `A2Srv_1`.
public: // Virtual table at 0060ec18.
    // virtual ~Srv1();

public:
    CMap<uint32_t, uint32_t, uint32_t, uint32_t> field1_0x4;
    uint32_t field2_0x20;
    SrvStru1 *srv_stru;
    CArray<uint16_t> field4_0x28;
};
ASSERT_OFFSET(Srv1, srv_stru, 0x24);
ASSERT_SIZE(Srv1, 0x3c);


class Player;

struct Server {
public:
    void sub_4FBB79();
    void sub_4F1E2A();
    void sub_4FA5C4();
    void sub_4F94C0(int arg);
    void sub_4FA78E(int arg);
    void sub_4F8831();

public:
    int tick16; // This value seems to be advanced every 16 ticks
    int tick;
    CowardActivation coward_activation;
    int field3_0x70;
    int field4_0x74;
    CString field5_0x78;
    SrvStru1* srv_stru1;
    uint8_t gap_0x80[8];
    HANDLE field15_0x88;
    int field16_0x8c;
    CString current_map_name;
    uint32_t field18_0x94;
    Srv1 field19_0x98;
    uint32_t field21_0xd4;
    uint32_t field22_0xd8;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t> field23_0xdc;
    std::array<Spell*, 30> spells;
    ScriptSettings* script_settings;
    uint32_t field26_0x174;
    uint32_t field27_0x178;
    uint32_t field28_0x17c;
    uint32_t field29_0x180;
    uint32_t field30_0x184;
    uint32_t field31_0x188;
    uint32_t field32_0x18c;
    uint32_t field33_0x190;
    uint32_t field34_0x194;
    uint32_t field35_0x198;
    uint32_t field36_0x19c;
    uint32_t field37_0x1a0;
    uint32_t field38_0x1a4;
    uint32_t field39_0x1a8;
    uint32_t field40_0x1ac;
    uint32_t field41_0x1b0;
    int field42_0x1b4;
    int field43_0x1b8;
    int field44_0x1bc;
    int tic16;
    int field46_0x1c4;
    CString current_map_title;
    uint32_t MapLevel;
    uint32_t field49_0x1d0;
    uint32_t field50_0x1d4;
    uint32_t field51_0x1d8;
    CStringArray FileList;
    uint32_t field53_0x1f0;
    uint32_t field54_0x1f4;
    uint32_t field55_0x1f8;
    uint32_t field56_0x1fc;
    uint32_t field57_0x200;
    uint32_t field58_0x204;
    uint32_t field59_0x208;
    uint32_t field60_0x20c;
    uint32_t field61_0x210;
    uint32_t field62_0x214;
    uint32_t field63_0x218;
    uint32_t field64_0x21c;
    uint32_t field65_0x220;
    uint32_t field66_0x224;
    CStringArray field67_0x228;
    CArray<uint32_t> field68_0x23c;
    int32_t map_elapsed_time;
    int32_t map_elapsed_time2;



    void FUN_004ff439(Player* player, int32_t arg4);
};
ASSERT_OFFSET(Server, srv_stru1, 0x7c);
ASSERT_OFFSET(Server, script_settings, 0x170);
ASSERT_SIZE(Server, 0x258);

extern int32_t g_IsServer; // 1 for server mode, 0 for client code. 665D00.
extern int32_t g_ShutdownIn; // Server shutdown (in milliseconds?). 0x7FFFFFFF means no shutdown scheduled. 63623c.
extern int32_t g_GameType; // 0: co-op, 1: deathmatch, 2: softcore, 3: arena. 6D1648.
extern int32_t g_CurrentMapIndex; // Current map index in map array. 6D1634.
extern CStringArray g_MapNames; // Array of map names. 6D15F0.
extern CArray<int32_t> g_MapDurations; // Array of map durations. 0x7FFFFFFF means no limit. 6D1618.

#endif
