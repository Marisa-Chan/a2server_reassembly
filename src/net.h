#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <array>
#include <cstdint>
#include <vector>

#include <afx.h>
#include "assert_offset.h"
#include <afxtempl.h>

#pragma warning(push)
#pragma warning(disable: 4005) // dplay.h redefines INTERFACE without a guard
#include "dplay.h"
#pragma warning(pop)


class AreaEffect;
class CMultiShopInstance;
class CMultiShopShelf;
class PacketAoeZone;
class PacketMoveBroadcast;
struct CLlDriver;
class Effect;
class CHlDriver;
struct NetClientPeer;
struct NetPacketBuffer;
class Packet;
class Player;
class Shop;
class Spell;
class Token;
struct TokenPos;
class Unit;
class QuestMap;
struct Inventory;


constexpr const bool IsDplayAvail = true;

#pragma pack(push, 1)
union HatCharId {
    uint64_t id;
    struct {
        int32_t id1;
        int32_t id2;
    };
};
#pragma pack(pop)

// ---------------------------------------------------------------------------
// Packet IDs - the first byte of every network packet. One shared ID space
// for game traffic (both directions) and the HAT-server protocol.
// Names annotated with (GM_Report*) come from the retail statistics logger
// (CHlDriver::WriteStatsLog); the rest are derived from the packet handlers.
// Entries named UNKNOWN_xx appear in CHlDriver::DecodePacket but have no known
// sender or handler in either binary.
// ---------------------------------------------------------------------------
enum PacketId : uint8_t {
    // Session / join handshake
    HAT_AUTH             = 0x00, // client->hat: v20 login + password + binary hashes for self-update
    ENTER_GAME           = 0x02, // client->server: join game (name, cd flag, hero params)
    MISSION_ENTERED      = 0x03, // server->client: bare "mission entered" event
    JOIN_MISSION         = 0x04, // client->server: player joins the running mission
    PLAYER_LEAVE         = 0x05, // client->server: player is leaving (is_active = 0)
    START_GAME           = 0x06, // server->client: load scenario <name> and start
    REQUEST_SAVE_GAME    = 0x07, // client->server: save game under name
    REQUEST_LOAD_GAME    = 0x08, // client->server: load game request ("not supported")
    CLIENT_SHUTDOWN      = 0x09, // client->server: client requests shutdown
    UNKNOWN_0A           = 0x0A,
    JOIN_ERROR           = 0x0B, // server->client: join rejected, param1 = error code
    JOIN_DONE            = 0x0E, // server->client: initial world sync finished
    UNKNOWN_0F           = 0x0F,
    GAME_SPEED           = 0x10, // server->client: set game speed

    // Group orders (client->server, PacketGroupOrder)
    GROUP_STOP           = 0x12,
    GROUP_GATHER         = 0x13, // gather nearby loot
    GROUP_GO_HOME        = 0x14,
    GROUP_MOVE           = 0x16,
    GROUP_GUARD          = 0x17,
    GROUP_AGGRESSIVE     = 0x18, // toggle aggressive stance
    GROUP_ATTACK         = 0x19, // attack unit/building target
    GROUP_ATTACK_MOVE    = 0x1A,
    GROUP_ATTACK_TARGET  = 0x1B, // attack building target
    GROUP_DEFEND         = 0x1C, // defend location
    GROUP_PATROL         = 0x1D,
    GROUP_CAST_UNIT      = 0x1E, // cast spell at unit/building
    GROUP_CAST_POS       = 0x1F, // cast spell at position
    GROUP_PICKUP         = 0x21, // swarm to sack and pick it up
    GROUP_ENTER_BUILDING = 0x24,
    SCROLL_CAST_UNIT     = 0x25, // cast scroll spell at unit/building
    SCROLL_CAST_POS      = 0x26, // cast scroll spell at position
    GROUP_STOP_AFK       = 0x4D, // stop + start AFK timer (singleplayer)

    // Items / shops / inns / map file transfer (client->server)
    ITEM_OPERATION       = 0x22, // equip/unequip/move/buy/sell/drop item
    DROP_GOLD            = 0x23, // create money sack on the ground
    ENTER_SHOP           = 0x32,
    SHOP_BUY             = 0x33, // confirm purchase
    SHOP_SELL            = 0x34, // confirm sale
    SHOP_TAKE_BACK       = 0x35, // take offered items back
    SHOP_CLOSE           = 0x36, // take items back and close shop
    ENTER_INN            = 0x38,
    REQUEST_MAP_INFO     = 0x39, // ask for map name + checksum
    EXIT_INN             = 0x3A,
    REQUEST_FILE_CHUNK   = 0x3B, // ask for next chunk of the map file
    UNKNOWN_3D           = 0x3D,
    CHEAT_MONEY          = 0x3E, // add money (only without fog of war)
    CHEAT_SHOP           = 0x3F, // regenerate default shop assortment
    UNKNOWN_40           = 0x40,

    // Player settings (client->server)
    SET_DIPLOMACY        = 0x45, // bulk diplomacy update
    SET_PARAMETER        = 0x46, // player parameter change (thresholds, formation, ...)
    JOIN_MAP             = 0x48, // join map (server) / create player character (client)
    CHEAT_SUMMON         = 0x49, // summon unit by index (only without fog of war)
    REQUEST_UNITS_RESEND = 0x4A, // resend all of the player's units
    REQUEST_RESPAWN      = 0x4B, // set respawn flag when hero is deeply dead
    HURT_SELF            = 0x4C, // "hurt myself" command
    REQUEST_UNIT_UPDATE  = 0xBE, // request full unit-ext update for own hero

    // HAT lobby extras
    HAT_JOIN_NAME        = 0x4E, // client->hat: player name for join
    HAT_SCORE_CLEAR      = 0x5A, // server->client: clear hat DM name/score lists
    HAT_SCORE_UPDATE     = 0x5B, // server->client: one hat DM name/score pair
    HAT_SCORE_LIST       = 0x5C, // server->client: show hat DM score list / arena round end

    // World sync (server->client)
    SERVER_TICK          = 0x64, // (GM_ReportNewSegment) server tick / frame counter
    UNKNOWN_65           = 0x65,
    MONEY_UPDATE         = 0x67, // (GM_ReportMoney) player gold
    UNIT_SHOWN           = 0x69, // (GM_ReportUnitShown) token entered vision
    TOKEN_REMOVE         = 0x6A, // remove token object on the client
    UNIT_MOVE            = 0x6B, // (GM_ReportUnitMove) move action with segments
    UNIT_CHANGE_4        = 0x6C, // (GM_ReportUnitChange) unit ext update, 4 flag bytes
    UNIT_TURN            = 0x6D, // (GM_ReportUnitTurn)
    UNIT_CHANGE_1        = 0x6E, // (GM_ReportUnitChange) unit ext update, 1 flag byte
    UNIT_CHANGE_2        = 0x6F, // (GM_ReportUnitChange) unit ext update, 2 flag bytes
    UNIT_CHANGE_3        = 0x70, // (GM_ReportUnitChange) unit ext update, 3 flag bytes
    UNIT_ATTACK          = 0x71, // (GM_ReportUnitAttack) melee attack animation
    RANGED_ATTACK        = 0x72, // (GM_ReportRangedAttack)
    UNIT_DAMAGED         = 0x73, // (GM_ReportUnitDamaged) hp update
    UNIT_HIDDEN          = 0x74, // (GM_ReportUnitHidden) token left vision
    ITEM_LIST            = 0x76, // (GM_ReportItemList) inventory/equipment/shop shelf contents
    UNIT_WEAR_ITEMS      = 0x79, // (GM_ReportUnitWearItems, stats log only)
    SACK_STATE           = 0x7A, // (GM_ReportSack) sack appeared/updated

    // Buildings & combat effects (server->client)
    BUILDING_HP          = 0x82, // building hp update
    OPEN_SHOP            = 0x83, // interaction with a shop building (opens shop UI)
    OPEN_INN             = 0x84, // interaction with an inn building (opens inn UI)
    EFFECT_MOVE          = 0x86, // (GM_ReportEffect) effect/projectile spawn, move or cast anim
    AOE_ZONE             = 0x87, // (GM_ReportAreaEffect) area-effect zone bitmap
    EFFECT_HANG          = 0x88, // (GM_ReportEffectHang) lasting effect applied to unit
    EFFECT_GONE          = 0x89, // (GM_ReportEffectGone) lasting effect removed from unit
    EFFECT_MULTI_TARGET  = 0x8A, // (GM_ReportEffectMultiTarget) multi-target cast
    EFFECT_FROM          = 0x8B, // (GM_ReportEffectFrom) cast from position (caster has no id)
    EFFECT_MULTI_FROM    = 0x8C, // (GM_ReportEffectMultiFrom) multi-target cast from position
    REPORT_MESSAGE       = 0x8D, // (GM_ReportMessage, stats log only)

    // Chat & players (server->client)
    CHAT                 = 0x91, // chat message / player name announcement
    TEAM_NOTIFY          = 0x92, // team notification message (param1 = msg type)
    PLAYER_ANNOUNCED     = 0x93, // player joined; own id means "replaced, disconnect"
    KILL_NOTICE          = 0x94, // notice about two players (arena kill text)
    PLAYER_INFO          = 0x96, // (GM_ReportNewPlayer) player roster entry upsert
    PLAYER_LEFT          = 0x97, // (GM_ReportDeletePlayer) player left, drop his objects
    ASH_MAP              = 0x98, // (GM_ReportAshMap, stats log only)
    TERRAIN_DATA         = 0x9B, // run-length encoded landscape
    ENEMY_EQUIPMENT      = 0x9C, // (GM_ReportEnemyEquipment) unit equipment slots

    // Game events / UI (server->client)
    MAP_REVEAL           = 0xAA, // 0 = hide map, 1 = reveal map, 2 = victory
    VIEW_POSITION        = 0xAB, // initial camera position
    UNKNOWN_AC           = 0xAC,
    UNKNOWN_AD           = 0xAD,
    ANNOUNCEMENT         = 0xAE, // server-wide announcement text
    SERVER_SHUTDOWN      = 0xAF, // server is going down, drop the connection
    KILL_STAT            = 0xB3, // single monster kill-count entry
    MISSION_RESULT       = 0xB4, // mission result code
    MISSION_COMPLETE     = 0xB5, // mission over, show result screen
    GAME_EVENT           = 0xB6, // generic game event (param1 = event id)
    ENTER_BUILDING       = 0xB7, // player entered building interior
    LEAVE_BUILDING       = 0xB8, // player left building interior
    DIPLOMACY_STATE      = 0xB9, // (GM_ReportDiplomacy) per-player diplomacy flags
    CREATURE_KNOWLEDGE   = 0xBA, // (GM_ReportCreatureKnowledge) run-length encoded kill stats
    PLAYER_QUESTS        = 0xBB, // (GM_ReportPlayerQuests) player quest list
    INN_QUESTS           = 0xBC, // (GM_ReportInnQuests) inn quest list (tavern open)
    MAP_INFO             = 0xBF, // map name + checksum (enter ack)
    FILE_CHUNK           = 0xC0, // chunk of a file being downloaded (map transfer)
    LATENCY              = 0xC1, // network latency setting (both directions)
    TEAM_STATE           = 0xC2, // team/CTF/frags bulk state
    GAME_START           = 0xC3, // game start signal (param1 = start kind)

    // HAT server protocol
    HAT_LOGOUT           = 0xC8,
    HAT_CREDENTIALS      = 0xC9, // login + password
    HAT_PICK_CHAR        = 0xCA, // selected character id
    HAT_CONNECT_SERVER   = 0xCB, // request to join a game server (hero params + name)
    HAT_CHAR_REQUEST     = 0xCC, // character id pair (client 50e70d, purpose unclear)
    SERVER_LIST          = 0xCD, // HAT server list page
    HAT_CHAR_LIST        = 0xCE, // character roster ids
    HAT_CHAR_DATA        = 0xCF, // character file transfer
    HAT_TICKET           = 0xD0, // session ticket (client->server: claim spectator slot)
    HAT_LOGIN            = 0xD1, // game server -> hat: login with name
    HAT_LOBBY_INFO       = 0xD2, // game server -> hat: player count, map level, game type
    HAT_CHAR_RETURN      = 0xD3, // hat -> game server: character return confirmed
    HAT_RAW_DATA         = 0xD4, // raw string data to HAT
    HAT_LOGIN_OK         = 0xD5, // hat -> game server: login successful
    UNKNOWN_D_6          = 0xD6,
    HAT_JOIN_OK          = 0xD8, // game server -> hat: player join accepted
    HAT_JOIN_FAIL        = 0xD9, // game server -> hat: player join rejected
    HAT_SESSION_SETUP    = 0xDA, // hat -> client: session keys + connect address
    HAT_SESSION_FAIL     = 0xDB, // hat -> client: no session (hat_id = -1)
    UNKNOWN_DC           = 0xDC,
    HAT_PLAYER_JOIN      = 0xDD, // hat -> game server: new player join request
    UNKNOWN_DE           = 0xDE,
    HAT_ERROR            = 0xDF, // hat -> client: error code
    HAT_MESSAGE          = 0xE0, // hat message / keepalive
    HAT_PLAYER_CHECK     = 0xE1, // player existence check (both directions)
};

// ChatType: relay mode of a CHAT packet, carried in PacketJoin::token_id.
// Server relays in the CHAT handler (server.cpp), client colors by it
// (ingame.cpp CHAT handler).
enum ChatType : uint8_t {
    CHAT_TYPE_REGULAR = 0, // everyone within chat_range
    CHAT_TYPE_ALLIES  = 1, // players with diplomacy bit 2 toward sender
    CHAT_TYPE_PRIVATE = 2, // direct message, recipient in player_id byte
    CHAT_TYPE_YELL    = 3, // global, shout_cooldown applies
    CHAT_TYPE_GLOBAL  = 4, // global broadcast, no cooldown
};

// Packet2Dwords::param1 for SET_PARAMETER (0x46).
namespace set_param_op {
enum : int32_t {
    wimpy_mode       = 1,    // withdraw / wimpy retreat thresholds
    formation        = 2,    // group movement formation mode
    autobuff         = 3,    // AutoCastSettings mask
    active_spell     = 4,    // toggle default/active spell on unit (param2 = unit_id | spell_id << 16)
    hotkey_broadcast = 0x80, // client hotkey broadcast (>= 0x80 rejected by retail server)
};
} // namespace set_param_op



// TEAM_NOTIFY param1 (SendTeamNotify msg_type). The client switch also has
// cases 0x10/0x20/0x40/0x80 (QuestMap::FLAG_*) and 0x100..0x107 (CTF rune notifications).
enum TeamNotifyType : uint32_t {
    TEAM_NOTIFY_UNREACHABLE       = 1,  // move destination unreachable (client shows only with g_EnableTrace)
    TEAM_NOTIFY_SPHERE_LEVELUP    = 2,  // skill sphere leveled up (param2 = sphere | server_id << 16)
    TEAM_NOTIFY_PLAYER_JOINED_NEW = 3,  // player joined, character was not in the save
    TEAM_NOTIFY_PLAYER_JOINED     = 4,  // player joined, character loaded from save
    TEAM_NOTIFY_PLAYER_FLAGGED    = 5,  // player flagged: coop flagged item / cheat enabled / inn item destroyed
    TEAM_NOTIFY_CHEAT_FAILED      = 6,  // cheat command rejected (bad latency, unknown item, dead unit, ...)
    TEAM_NOTIFY_CHEAT_OK          = 7,  // cheat command applied
    TEAM_NOTIFY_SHOUT_COOLDOWN    = 8,  // shout rejected, param2 = remaining cooldown seconds
    TEAM_NOTIFY_STAT_BOOST        = 9,  // rune stat boost applied, param2 = boost
    TEAM_NOTIFY_TEAMS_READY       = 10, // all teams ready, match starting
    TEAM_NOTIFY_TEAM_READY_PROMPT = 11, // prompt player to ready up in teamplay
    TEAM_NOTIFY_MAP_CHANGE_IN     = 12, // map change countdown, param2 = remaining seconds
    TEAM_NOTIFY_SHUTDOWN_IN       = 13, // server shutdown countdown, param2 = remaining seconds
    TEAM_NOTIFY_CAMPING_IN        = 14, // camping countdown, param2 = remaining seconds
    TEAM_NOTIFY_CAMPING_CANCELLED = 15, // camping cancelled
};

// PacketUnitUpdate::flags_mask bits: the SendUnitUpdate update_mask selects
// which fields the sender packs (net.cpp), the client handler parses them
// (ingame.cpp). Side-effect bits trigger extra sends instead of a field.
enum UnitUpdateField : uint32_t {
    UNIT_UPD_HP             = 0x00000001, // int16 hp
    UNIT_UPD_MP             = 0x00000002, // int16 mp
    UNIT_UPD_SKILL_LEVELS   = 0x00000004, // 5 skill bytes (humanoids)
    UNIT_UPD_HP_MP_MAX      = 0x00000008, // int16 hp_max + int16 mp_max
    UNIT_UPD_DIR_POS        = 0x00000010, // facing byte + x byte + y byte
    UNIT_UPD_TYPE_FACE      = 0x00000020, // type id + face byte
    UNIT_UPD_OWNER          = 0x00000040, // owner player id
    UNIT_UPD_SERVER_ID      = 0x00000080, // uint16 server id
    UNIT_UPD_EXP_SPHERE0    = 0x00000100, // int32 experience_per_sphere[0..4] (humanoids)
    UNIT_UPD_EXP_SPHERE1    = 0x00000200,
    UNIT_UPD_EXP_SPHERE2    = 0x00000400,
    UNIT_UPD_EXP_SPHERE3    = 0x00000800,
    UNIT_UPD_EXP_SPHERE4    = 0x00001000,
    UNIT_UPD_CARRIED_WEIGHT = 0x00002000, // uint16 carrying weight (100g units)
    UNIT_UPD_DEFENSE        = 0x00004000, // uint16 defence
    UNIT_UPD_ABSORB         = 0x00008000, // uint8 absorption
    UNIT_UPD_ATTACK         = 0x00010000, // uint16 attack
    UNIT_UPD_HAND_DAMAGE    = 0x00020000, // hand damage min + spread bytes
    UNIT_UPD_SCAN_RANGE     = 0x00040000, // uint16 scan range
    UNIT_UPD_SPEED          = 0x00080000, // speed byte
    UNIT_UPD_SPELLS         = 0x00100000, // spellbook mask (int32) + active spell byte
    UNIT_UPD_SEND_INVENTORY = 0x00200000, // side effect: send bag contents (owner only)
    UNIT_UPD_SEND_EQUIPMENT = 0x00400000, // side effect: send equipment list (props slot flags)
    UNIT_UPD_EFFECTS        = 0x00800000, // side effect: refresh effect list
    UNIT_UPD_MAGIC_PROTECT  = 0x01000000, // 5 magic protection bytes
    UNIT_UPD_KILL_STATS     = 0x02000000, // monster/player kills, deaths, frags (owner main unit)
    UNIT_UPD_BASE_STATS     = 0x20000000, // body/reaction/mind/spirit + base values
    UNIT_UPD_NAME           = 0x80000000, // unit name string (24 bytes)

    // Recurring combinations (named at their call sites).
    UNIT_UPD_WEIGHT_SPEED           = UNIT_UPD_CARRIED_WEIGHT | UNIT_UPD_SPEED,                              // 0x82000: item dropped/sold
    UNIT_UPD_INVENTORY_WEIGHT       = UNIT_UPD_SEND_INVENTORY | UNIT_UPD_CARRIED_WEIGHT,                     // 0x202000
    UNIT_UPD_INVENTORY_WEIGHT_SPEED = UNIT_UPD_SEND_INVENTORY | UNIT_UPD_CARRIED_WEIGHT | UNIT_UPD_SPEED,    // 0x282000: bag changed
    UNIT_UPD_EQUIP_WEIGHT_SPEED     = UNIT_UPD_SEND_EQUIPMENT | UNIT_UPD_CARRIED_WEIGHT | UNIT_UPD_SPEED,    // 0x482000: equipment changed
    UNIT_UPD_COMBAT_STATS           = UNIT_UPD_SKILL_LEVELS | UNIT_UPD_HP_MP_MAX | UNIT_UPD_ATTACK | UNIT_UPD_HAND_DAMAGE, // 0x3000C: weapon damage recalc
    UNIT_UPD_STAT_BOOST             = UNIT_UPD_HP | UNIT_UPD_MP | UNIT_UPD_SKILL_LEVELS | UNIT_UPD_HP_MP_MAX | UNIT_UPD_EXP_SPHERE0
                                    | UNIT_UPD_EXP_SPHERE1 | UNIT_UPD_EXP_SPHERE2 | UNIT_UPD_EXP_SPHERE3 | UNIT_UPD_EXP_SPHERE4
                                    | UNIT_UPD_ATTACK | UNIT_UPD_HAND_DAMAGE, // 0x31f0f: rune boost
    UNIT_UPD_ALL_STATS              = 0xA31FFFFF, // every stat field, no side-effect bits
    UNIT_UPD_ALL_STATS_EQUIPMENT    = UNIT_UPD_ALL_STATS | UNIT_UPD_SEND_EQUIPMENT, // 0xA35FFFFF
    UNIT_UPD_SEND_GATE_MASK         = 0xBF1FFFFF, // fields carried inside PacketUnitUpdate itself
    UNIT_UPD_SUMMON_MASK            = 0x800000DB, // summoned non-humanoid privacy filter
    UNIT_UPD_MONSTER_MASK           = 0xC08000FB, // non-owner monster privacy filter
    UNIT_UPD_MONSTER_STD_HP_MASK    = 0xC08000F3, // non-owner monster with standard hp_max (omits hp_max/mp_max)
    UNIT_UPD_OTHER_HERO_MASK        = 0xC0C000FB, // non-allied humanoid privacy filter
    UNIT_UPD_ALL                    = 0xFFFFFFFF,
};

enum : uint32_t {
    UNCLAIMED_CLIENT_UID_MASK = 0x3FFF,
    UNCLAIMED_CLIENT_FLAG     = 0x4000,
    MAX_SEGMENT_PAYLOAD       = 0x8E,
};

// SendUnitUpdate props_flags / SendHumanoidProps slot mask: INDEPENDENT bit
// space. Bit i = equipment slot i+1 present (weapon = slot 1, shield = slot 2,
// slots 3..12 = equipment[]). Consumed by SendHumanoidProps (ITEM_LIST mode 1)
// and the slot math in the ITEM_OPERATION handler.
enum EquipSlotFlag : uint32_t {
    EQUIP_SLOT_WEAPON = 0x001, // slot 1
    EQUIP_SLOT_SHIELD = 0x002, // slot 2
    EQUIP_SLOT_3      = 0x004,
    EQUIP_SLOT_4      = 0x008,
    EQUIP_SLOT_5      = 0x010,
    EQUIP_SLOT_6      = 0x020,
    EQUIP_SLOT_7      = 0x040,
    EQUIP_SLOT_8      = 0x080,
    EQUIP_SLOT_9      = 0x100,
    EQUIP_SLOT_10     = 0x200,
    EQUIP_SLOT_11     = 0x400,
    EQUIP_SLOT_12     = 0x800,
    EQUIP_SLOTS_ALL   = 0xFFB,  // all slots except slot 3 (mask every caller uses)
};

struct PackerTail {
    PackerTail* child0;
    PackerTail* child1;
    int32_t gap_0x8;
    int32_t gap_0xc;
    int32_t gap_0x10;
    int32_t gap_0x14;
};
ASSERT_SIZE(PackerTail, 0x18);

// HAT server browser / ICMP ping client. Read-only view of the fields the
// a2mgr-installed browser object exposes to the dialog (client accessors
// 4540d0/4540f0/454110/454130/454170/4541d0; CS installed at 0x204). Full
// winsock types live here, hence the definition; implementation is in visual.cpp.



struct PackerDat {
    std::array<int32_t, 256> bitMap;
    std::array<int32_t, 256> bitLength;
    std::array<int32_t, 256> freq;
    PackerTail* tail;

    void DeleteTail(PackerTail* node); // 526621 (client: 515453)
    void Clear(); // 5265f7 (client: 515429)

    PackerDat(); // 5260fa (client: 514efa)
    ~PackerDat(); // 526115 (client: 514f15)
};
ASSERT_OFFSET(PackerDat, freq, 0x800);
ASSERT_SIZE(PackerDat, 0xc04);

struct ConnStatInfo {
    uint32_t window_bytes;
    uint32_t cur_bs;
    uint32_t total_bytes;
    uint32_t max_bs;
    uint32_t time;

    ConnStatInfo() {
        memset(this, 0, sizeof(ConnStatInfo));
    }
};
ASSERT_SIZE(ConnStatInfo, 0x14);

class CHlDriver {
public: // VTable at 0060ecc0.
    virtual void OnClientConnect(NetClientPeer* client); // 51fc72 (client: 50fe58)
    virtual void OnClientDisconnect(NetClientPeer* client); // 51f561 (client: 50fc92)

public:
    CLlDriver* lldriver; //low-level net driver
    CHlDriver* linked_hl;
    CList<NetClientPeer*> disconnect_list;
    CList<NetClientPeer*> new_connects;
    CList<NetPacketBuffer*> free_net3;
    CRITICAL_SECTION critical_section;
    CRITICAL_SECTION critical_section2;
    PackerDat packer_dat1; //huffman tables for Pack/Unpack channel 1
    PackerDat packer_dat2; //huffman tables for Pack/Unpack channel 2
    uint32_t packer1_trained_bytes; //packer1 trained-bytes statistic
    uint32_t packer1_packed_bytes; //packer1 packed-bytes statistic
    uint32_t packer2_trained_bytes; //packer2 trained-bytes statistic
    uint32_t packer2_packed_bytes; //packer2 packed-bytes statistic
    uint32_t compression_type;
    uint32_t compression_mode; // 0 - disable, 1 - make statistic, 2 - compression
    uint32_t make_statistics; // calculate statistic
    NetClientPeer* local_client;
    CList<NetClientPeer*> active_connects;
    CMap<int32_t, int32_t, ConnStatInfo*, ConnStatInfo*> client_stat;
    uint32_t stat_pkt_num[256];
    uint32_t stat_pkt_size[256];

    static CHlDriver HatConnector;

public:
    CHlDriver(int stats); // 51684b (client: 509343)
    ~CHlDriver(); // 516b44 (client: 5094ca)

public:
    void SendTeamNotify(uint32_t msg_type, uint32_t player_id, Player* recpt); // 51ce86 (client: 50df07)
    void SendBarePacket(uint8_t id, Player* recpt); // 51ceac (client: 50df2d)
    void SendTickPacket(Player* recpt); // 51c748 (client: 50d82f)
    void UntrackToken(Token* token); // 51c61e (client: 50d7b2) - Remove unit/sack from network tracking
    void AnnouncePlayer(Player* recpt); // 51d49b (client: 50e2cc)

    void AnnouncePlayerName(const CString& name, Player* player); // 51cd89 (client: 50de98)

    NetClientPeer* GetClientByPlayerID(uint16_t player_id); // 518544 (client: 50a128)
    NetClientPeer* GetClientByLowUid(uint32_t low_uid); // 5185d5 (client: 50a1ab)

    void AddClientWindowBytes(NetClientPeer* client, int32_t value); // 51eff8 (client: 50f79c)
    void AddPacketStatistic(Packet* pkt); // 51f0bb (client: 50f7dd)
    void SendAllData(); // 5188db (client: 50a3b1) - 518927 variant without flag check because in original it's used once and not optimal, because iterate all and do not do anything

    void QueuePacketSend(Packet* pkt); // 5186cd (client: 50a218)
    void SendServerTickFlush(uint32_t val, uint16_t player_id); // 51d88d (client: 50e446)
    void SetLLDriver(CLlDriver* drv); // 517ff9 (client: 509bd1)
    void SetLinkedHLDriver(CHlDriver* hl); // 51703e (client: 50963b)

    uint32_t GetClientsPktNum(); // 518a6b (client: 50a4de)
    int IsActive(); // 518ac9 (client: 50a53b)

    void SendTeamState(uint16_t to_player_id); // 51d6b4 (client: none)
    void SendInfoPacket(uint8_t id, int32_t value1, int32_t value2, Player* target); // 51cefb (client: 50df7c)
    
    void SendUnitActionPacket(Unit* unit, uint8_t angle, uint8_t speed_dir, uint8_t mode); // 51b99e (client: 50cd3c)
    void SendRangedAttackPacket(Unit* unit); // 51ba2f (client: 50cdcd)
    void SendUnitTurn(Unit* unit, Unit* target); // 51c59c (client: 50d730) - Notify clients about unit turning to face its target
    void SendUnitUpdate(Unit* unit, Player* player, uint32_t mask, int32_t props_flags, int32_t inv_start_index, int32_t inv_end_index); // 519221 (client: 50ac91)

    void SendHumanoidProps(Unit* unit, Player* player, int32_t flags); // 51a0ef (client: 50b9df)
    void SendMonsterInventory(Unit* unit, Player* player, int32_t start_index, int32_t end_index); // 51a6d5 (client: 50beae)
    void BroadcastEffect(Effect* effect, Unit* unit, uint8_t packet_id); // 51bda4 (client: 50d132)

    void SendGameInfo(int32_t game_speed, Player* player); // 51d837 (client: none) - Send server state / player list to player
    void SendExistingPlayers(Player* player); // 51c8b1 (client: 50d96e) - Send all existing players (PacketJoin) to new player
    void SendVisibilityState(Player* player); // 51cb21 (client: 50dabc) - Send terrain/diplomacy visibility state to player
    void SendPendingUnits(Player* player); // 51c0f7 (client: 50d485) - Send units from pending unit list to player
    void SendServerState(Player* player); // 51ca5d (client: 50da69) - Send server state to player
    void SendKillStats(uint16_t player_id, Player* player); // 51d1a8 (client: 50e1b7) - Send kill stats (pass 0,nullptr for all)
    void BroadcastUnitState(Unit* unit); // 51b870 (client: 50cc7f) - Broadcast unit to all players.
    void SendUnitVisibility(Unit* unit, int flag, Player* player); // 51cf5c (client: 50dfdd) - Send unit visibility packet (retn 0Ch)
    void SendAreaEffect(AreaEffect* obj, int flag); // 51be8f (client: 50d21d) - Send area-effect packet
    void BroadcastEffectMove(Effect* effect, uint8_t type); // 51be0e (client: 50d19c) - Broadcast move packet for an effect token
    void SendPlayerQuests(QuestMap* quest_map, Player* player, int flag); // 51d4f6 (client: 50e326) - Send player or inn quest list packet
    void SendShopTable(Token* token, Inventory* inventory, Player* player, int32_t mode); // 51a8b2 (client: 50c006)
    void SendShopShelf(Shop* shop, CMultiShopShelf* shelf, Player* player, uint8_t slot); // 51aa26 (client: 50c10b) - send single shop shelf contents
    void SendShopShelves(CMultiShopInstance* inst, Player* player, int32_t shelf_id); // 51ab99 (client: 50c25c) - send shop shelf contents
    void UpdateConnStats(); // 51eeb7 (client: 50f6ef)

    void SendLatency(int32_t latency, Player* player); // 51c7cc (client: 50d8ab) - Send latency update to player
    void SendGameEvent(Player* player, int32_t event_id, int32_t value2); // 51cd2a (client: 50de39) - Send in-game event trigger
    void SendArenaScore(CString& name, uint32_t score, char type, Player* player); // 51cdfb (client: none) - send arena score

    void ResendPlayerUnits(Player* player); // 51c46e (client: 50d66d)
    void SendLevelUp(Unit* unit, int unused); // 51c601 (client: 50d795) - Send unit-level-up packet
    void SendSpectatorState(NetClientPeer* ns2); // 51c822 (client: 50d901) - Send spectator/alliance state

    // DecodePacket – decode an incoming network message into the appropriate Packet singleton,
    // populate it via ReadFrom, and stamp it with the sender id.
    Packet* DecodePacket(uint8_t cmd, NetClientPeer* ns2); // 51e7fc (client: 50f04e)
    Packet* ReceivePacket(NetClientPeer* cli); // 518a23 (client: 50a498) - receive packet from client
    Packet* ReceiveAnyPacket(); // 518980 (client: 50a3fc) - iterate over active connections and try receive packet

    void BroadcastTokenState(Token* token, Player* player, uint8_t flag); // 51ac77 (client: 50c2ea) - Broadcast token state to players
    void SendSpellCastAtUnit(Unit* caster, Spell* spell, Unit* target, int16_t delay); // 51bab0 (client: 50ce4e) - Send targeted-unit spell packet
    void SendSpellCastAtPos(Unit* caster, Spell* spell, TokenPos* pos, int16_t delay); // 51bb94 (client: 50cf32) - Send point/area spell packet
    void SendSpellCastMulti(Unit* caster, Spell* spell, CList<Unit*>* units, int32_t flag); // 51bc6b (client: 50d009) - Send multi-target spell packet
    void BroadcastMovePacket(PacketMoveBroadcast* pkt, TokenPos* pos); // 51b370 (client: 50c867) - Broadcast move packet to all players in visibility
    void BroadcastAoeZone(PacketAoeZone* pkt, AreaEffect* area_effect); // 51b638 (client: 50cab9) - Broadcast AoE-zone packet to all players in visibility.
    void SendUnitAction(Packet* pkt, Unit* unit); // 51b0f0 (client: 50c65a) - send unit action packet to relevant players


    void EnqueueDisconnectClient(NetClientPeer*& net2); // 517e91 (client: 509b9b)
    void ReleasePacketBuffer(NetPacketBuffer* net3); // 517b00 (client: 509a73)

    NetClientPeer* AllocClientBufManager(uint32_t uid); // 517c99 (client: 509ad7)

    NetPacketBuffer* AcquirePacketBuffer(); // 5177f5 (client: 50994f)

    void ProcessNewConnects(); // 51755c (client: 509889)
    void ProcessDisconnectList(); // 51725b (client: 509799)
    void ProcessConnections(); // 51800f (client: 509be7)
    void DisconnectClient(NetClientPeer* cli); // 5170b6 (client: 5096a2)

    void SendHatJoinOk(int32_t a, int32_t b); // 51e5fb (client: 50efc6) - Send HAT join-success reply (pkt 0xD8)
    void SendHatJoinFail(int32_t a, int32_t b, int32_t error_code); // 51e63f (client: none) - Send HAT join-failure reply (pkt 0xD9)
    void SendHatPlayerCheck(int32_t a, int32_t b, uint8_t status, int16_t player_id); // 51e794 (client: none) - Send HAT player-check reply (pkt 0xE1)

    void SendHatLogin(CString name); // 51e205 (client: 50ec0e) - Send hat login packet
    void SendHatLobbyInfo(int32_t player_count, CString source, int32_t map_level, int32_t game_type, int32_t map_range); // 51e289 (client: 50ec97) - Send hat lobby info packet
    void SendHatCharData(int32_t hat_char_id1, int32_t hat_char_id2, CString name, void* data, uint32_t size, uint16_t player_id); // 51dfa7 (client: 50eb05)
    void SendHatMessage(int32_t hat_char_id1, int32_t hat_char_id2, CString name, void* data, uint32_t size, uint16_t player_id); // 51e0b7 (client: none)
    void SendHatKeepalive(uint16_t player_id); // 51e1c7 (client: none)
    void SendHatRawString(const char* str, int32_t len); // 51e40c (client: 50ede9) - Send raw string data to HAT

    void SendHatCredentials(const CString& login, const CString& password, uint32_t flags); // 51d8d6 (client: 50e48b)
    void SendHatCharId(HatCharId charid); // 51d9d0 (client: 50e58b)
    void SendHatLogout(); // 51d9a0 (client: 50e55b)

    void WriteStatsLog(CString fname); // 51f124 (client: 50f846)

    void SendHatConnectServer(HatCharId key, uint8_t body, uint8_t reaction, uint8_t mind, uint8_t spirit, uint8_t sphere, uint8_t type_face, uint8_t color, const CString& name, const CString& srv); // 51da14 (client: 50e5cf)

    void SendServerConnectHatTicket(HatCharId ticket, uint16_t player_id); // 51dce5 (client: 50e8c0)
    void SendHatSessionConfirm(); // 51e513 (client: 50eede) - client-only helper
    void SendHatCharRequest(HatCharId id); // 51db5c (client: 50e70d) - client-only helper

    NetClientPeer* GetFirstClient(); // 41f520 (client: 420af0)
};
#if defined(A2SERVER_PATCH) && !defined(A2CLIENT)
bool IsAllowedClientPacketId(uint8_t c);
#endif
ASSERT_OFFSET(CHlDriver, packer_dat1, 0x90);
ASSERT_OFFSET(CHlDriver, packer1_trained_bytes, 0x1898);
ASSERT_SIZE(CHlDriver, 0x20f0);

__pragma(pack(push, 1))
struct NetPacketBuffer {
    uint32_t timestamp;
    uint32_t timestamp2;
    union {
        struct {
            union {
                struct {
                    uint16_t pos;
                    uint16_t __padding_0xa;
                };
                uint32_t pktid;
            };
            
            uint8_t cmode; //compression mode
            uint16_t csize;
            uint8_t pkt_num; //number of packets in buffer?
        };
        uint8_t full_data[8];
    };
    uint8_t buf[144];
    int32_t datasz;
    int32_t readpos;

    void Clear(); // 5156f6 (client: 508736)
    NetPacketBuffer(); // 5155c0 (client: 50863e)
    ~NetPacketBuffer(); // 515694 (client: 5086f3)

    void CopyData(const NetPacketBuffer* src); // 515625 (client: 508684)

    int WriteToBuffer(void* data, uint32_t sz); // 51573e (client: 50877e)
    int ReadFromBuffer(void* out, uint32_t sz); // 51579e (client: 5087de)
};
__pragma(pack(pop))
ASSERT_OFFSET(NetPacketBuffer, datasz, 0xa0);
ASSERT_SIZE(NetPacketBuffer, 0xa8);

__pragma(pack(push, 1))
struct NetClientPeer {
    CHlDriver* hldriver;
    CLlDriver* lldriver;
    uint8_t buf[256];
    uint16_t player_id;
    uint8_t gap_0x10a[2];
    uint32_t uid;
    NetPacketBuffer out_buffers[2]; // buffers with send data
    int32_t out_buff_id; // currenct send buffer
    CList<NetPacketBuffer*> received_buffers; //buffers with received data
    CRITICAL_SECTION critical_section;
    uint32_t compression_type; // 0 - disabled, 1,2 - compression types
    uint32_t is_local_player;
    uint32_t gap_0x2a0;
    uint32_t gap_0x2a4;
    uint32_t is_claimed;
    uint32_t gap_0x2ac;
    CString str;
    int32_t gap_0x2b4;
    uint32_t gap_0x2b8;
    uint32_t gap_0x2bc;


public:
    int WriteData(void* buf, uint32_t size); // 515ef3 (client: 508c6a)
    int ReadData(void* buf, uint32_t size); // 515f9c (client: 508d13)

    // ReleaseNetBuf – walks the free_net3 linked list and decrements the
    // ref-count byte (field_0xF) of the first live entry it finds.
    void ReleaseNetBuf(); // 5167a5 (client: 5092a4)

    void SetDrivers(CHlDriver* HlDriver, CLlDriver* LlDriver); // 51682d (client: 509325)
    int ReceiveData(NetPacketBuffer* buffer); // 5161bf (client: 508e04)

    int SendData(); // 516408 (client: 508f33)

    NetClientPeer(); // 515831 (client: 508871)
    ~NetClientPeer(); // 515c27 (client: 508acc)
    void ReturnBuffers(); // 515d9d (client: 508b9a) - return buffers to High-level driver

    uint32_t GetBuffersPktNum(); // 51670a (client: 509208) - Get received_buffers summary of pkt_num

    uint32_t GetUid() { return this->uid; }; // 41f500 (client: 420700)
};
__pragma(pack(pop))

ASSERT_OFFSET(NetClientPeer, player_id, 0x108);
ASSERT_OFFSET(NetClientPeer, critical_section, 0x280);
ASSERT_SIZE(NetClientPeer, 0x2c0);

struct NetSockLatency {
    uint32_t lat_times[128];
    uint32_t num;
    uint32_t calc_latency;

    void AddLatency(uint32_t lat_time); // 51fdf7 (client: 50ff67)
    static inline uint32_t ShiftToByte(uint32_t val, int shift) { return (val >> shift) & 0xFF; };
    uint32_t GetLatency() const { return this->calc_latency * 2; }; // 5204fa (client: 510014)
};

struct A2NetSock {
    uint32_t is_in_use;
    uint32_t uid;
    SOCKET socket;
    uint32_t player_dpid;
    NetClientPeer* manager;
    CList<NetPacketBuffer*> list_0x14;
    CList<NetPacketBuffer*> list_0x30;
    uint32_t next_out_pktid;
    uint32_t next_in_pktid;
    NetSockLatency latency_check;
    uint32_t latency;
    int32_t stat_pkts_queued;
    int32_t stat_pkts_sent;
    HANDLE wait_obj;
    NetPacketBuffer* current_buffer;
    int32_t copy_num;

    A2NetSock(); // 520510 (client: 51002a)
    ~A2NetSock(); // 520684 (client: 510121)
};
ASSERT_OFFSET(A2NetSock, list_0x30, 0x30);
ASSERT_OFFSET(A2NetSock, wait_obj, 0x268);
ASSERT_SIZE(A2NetSock, 0x274);

struct SocketNm {
    uint32_t dpid;
    A2NetSock *sock;

    SocketNm() {
        this->dpid = 0;
        this->sock = nullptr;
    };

    ~SocketNm() {
        this->dpid = 0;
        this->sock = nullptr;
    };
};

struct ComSettings { //DPCOMPORTADDRESS
    int32_t index;
    uint32_t speed;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t flow_control;

    void Reset() { // 43cff2 (client: 43f402)
        this->index = 1;
        this->speed = 14400;
        this->stop_bits = 0;
        this->parity = 0;
        this->flow_control = 4;
    }

    void Load(HKEY& rkey) { // 43d02e (client: 43f43e)
        this->Reset();
        DWORD sz = 0x14;
        RegQueryValueExA(rkey, "comportsettings", nullptr, nullptr, (LPBYTE)this, &sz);
    }
};
ASSERT_SIZE(ComSettings, 0x14);

struct CLlName {
    char name[256];
};

struct CLlAddress : CLlName {
    char address[256];
    union {
        struct {
            uint32_t gap_0x200;
            uint32_t gap_0x204;
            uint32_t gap_0x208;
            uint32_t gap_0x20c;
            uint32_t gap_0x210;
        };
        GUID guid;
        ComSettings com;
    };
};
ASSERT_SIZE(CLlAddress, 0x214);

struct CLlNetSession : CLlName {
    GUID guid;
};
ASSERT_SIZE(CLlNetSession, 0x110);

struct CLlConn : CLlName {
    int32_t typ;
};
ASSERT_SIZE(CLlConn, 0x104);

enum NetProvider : int32_t {
    PROVIDER_SERIAL   = 0, // DirectPlay DPSPGUID_SERIAL
    PROVIDER_MODEM    = 1, // DirectPlay DPSPGUID_MODEM
    PROVIDER_IPX      = 2, // DirectPlay DPSPGUID_IPX
    PROVIDER_DP_TCPIP = 3, // DirectPlay DPSPGUID_TCPIP
    PROVIDER_HAT_TCP  = 4, // custom hat TCP stack (client/server internet play)
    PROVIDER_NONE     = 5, // no provider initialized (ctor default; restored by Free())
};

struct CLlDriver {
    CHlDriver* hl_driver;
    A2NetSock listen_socket;
    A2NetSock ping_socket;
    int32_t is_server;
    NetProvider provider;
    int32_t guaranteed;
    int32_t latency;
    char address_str[128];
    A2NetSock* connection_sockets;
    SocketNm* connections_info;
    CLlAddress* enum_addresses;
    uint32_t enum_addresses_num;
    CLlNetSession* enum_sessions;
    uint32_t enum_sessions_num;
    CLlName* enum_players;
    uint32_t enum_players_num;
    CLlConn* enum_conns;
    uint32_t enum_conns_num;
    uint32_t max_connections;
    uint32_t num_connections;
    uint32_t next_uid;
    uint32_t unused;
    GUID application_guid;
    union {
        IDirectPlay4A* dplay4;
        IDirectPlay3A* dplay3;
    };
    HANDLE ev_create_player;
    HANDLE ev_close;
    HANDLE ev_new_session;
    uint32_t session_lost;
    CLlAddress cur_address;
    CRITICAL_SECTION critical_section;
    uint32_t timeout;
    uint32_t keepalive;
    uint32_t dplay_is_4;
    char comp_name[256];
    CLlAddress* server_start_addr;

public:
    A2NetSock* GetClientBySocketId(uint32_t uid); // 5224c4 (client: 511c00)

    CLlDriver(GUID _appid, CHlDriver *net1); // 5208ec (client: 510241)
    ~CLlDriver(); // 520ad3 (client: 510429)

    void SetHlDriver(CHlDriver* drv); // 496e50 (client: 49a920)

    void Close(); // 5225d0 (client: 511d0c)
    void CloseTcp(); // 523b01 (client: 5131ad)
    void CloseDp(); // 525a09 (client: 514b64)
    void CloseTcpSocket(A2NetSock* sock); // 523a16 (client: 5130cf)
    void CloseDpSock(A2NetSock* sock, int idx = -1); // 525bcd (client: 514d27)

    CLlAddress* AllocAddress(); // 520d8c (client: 510646)
    void FreeEnumAddresses(); // 520c80 (client: 51053a)

    int EnumAddresses(CLlAddress** addrs, int* num); // 52182f (client: 5110c6)
    void EnumAddressesTcp(); // 5214c5 (client: 510d5c)
    void EnumAddressesDp(); // 521200 (client: 510a97)
    static BOOL __stdcall cbEnumAddress(REFGUID guidDataType, DWORD dwDataSize, LPCVOID lpData, LPVOID lpContext); // 5211b2 (client: 510a50)

    CLlNetSession* AllocSession(); // 520e2e (client: 5106e8)
    void FreeEnumSessions(); // 520cc3 (client: 51057d)

    typedef int(*CBUPDATECALLBACK)();
    int EnumSessions(CLlNetSession**, int* num, CBUPDATECALLBACK cbUpdate = nullptr, uint32_t timeout = 0); // 52210d (client: 51184d)
    static BOOL __stdcall cbEnumSessions(LPCDPSESSIONDESC2 lpThisSD, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext); // 522020 (client: 51176c)


    CLlConn* AllocEnumConns(); // 520ed0 (client: 51078a)
    void FreeEnumConns(); // 520d06 (client: 5105c0)
    void EnumConnections(CLlConn** conns, int* num); // 5210ea (client: 510988)
    static BOOL __stdcall cbEnumConnections(LPCGUID lpguidSP, LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext); // 52100e (client: 5108c8)


    CLlName* AllocEnumPlayers(); // 520f72 (client: 51082c)
    void FreeEnumPlayers(); // 520d49 (client: 510603)
    void EnumPlayers(CLlNetSession* ses, CLlName** names, int* num); // 52238e (client: 511ace)
    static BOOL __stdcall cbEnumPlayes(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext); // 52233f (client: 511a7f)


    void Free(); // 5219af (client: 511246)
    void FreeTcp(); // 523611 (client: 420b70)
    void FreeDp(); // 523d5a (client: 513406)

    int ResetProvider(int provider); // 52194f (client: 5111e6)

    int CreateTcp(); // 52359b (client: 512c58)
    int CreateDp(); // 523ccf (client: 51337b)

    int IsListen() { return this->listen_socket.is_in_use == 1; }; // 496ed0 (client: 49a810)

    int IsInUse(A2NetSock* sock); // 52293a (client: 512078)
    int IsUidInUse(uint32_t uid); // 522966 (client: 5120a2) - GetClientBySocketId + IsInUse
    void SetEventNewSession(); // 522318 (client: 511a58)

    int MakeSockAddr(const char* address, sockaddr_in* out, int port, int resolve); // 52343e (client: 512afd)
    int StartServerTcp(); // 523621 (client: 512cdc)

    static void __cdecl SetSockOptions(SOCKET s, int linger); // 522af4 (client: 512230)

    static void __cdecl AcceptThreadTcp(void* context); // 522e8f (client: 51258b)
    int RecvThreadTcp(A2NetSock* sock); // 522b95 (client: 5122d1)

    int PrepareAddressDp(CLlAddress* addr); // 523d96 (client: 513442)
    int StartServerDp(); // 5241ea (client: 513896)
    static void __cdecl RecvThreadDp(void* context); // 523b7a (client: 513226)
    void HandleMessageDp(uint32_t from, uint32_t to, void* data, uint32_t datasz); // 524abd (client: 514141)

    A2NetSock* FindDpSock(uint32_t id, int* out_idx); // 524986 (client: 51400a)

    int StartServer(int maxconn, const char* name, CLlAddress* addr); // 521a09 (client: 51129c)

    int PrepareForConnect(const char* name, CLlAddress* addr); // 521d2a (client: 51151c)


    int ConnectDp(const char* name, CLlNetSession* session); // 52465f (client: 513d0b)
    int ConnectTcp(); // 523794 (client: 512e4f)

    int Connect(const char* name, CLlNetSession* session); // 5223fb (client: 511b3b)

    int SendDataTcp(A2NetSock* sock, NetPacketBuffer* buf); // 52394a (client: 513005)
    int SendDataDp(A2NetSock* sock, NetPacketBuffer* buffer); // 5255c6 (client: 514795)
    int SendData(uint32_t uid, NetPacketBuffer* buffer); // 5226c4 (client: 511dfc)

    void SendAllPendingDp(A2NetSock* sock); // 525b86 (client: 514ce1)
    void DisconnectClient(uint32_t uid); // 522526 (client: 511c62)
    void CleanupInvalidTcpClient(uint32_t uid); // 52262a (client: 511d62)

    void CleanupAllInvalid(); // 522809 (client: 511f4d)

    int RestartModemServerDp(); // 52460e (client: 513cba)
    void RecreateDp(); // 524644 (client: 513cf0)

    void SetLatency(uint32_t conn_uid, int32_t latency_ms); // 5229cd (client: 512109) - Set connection latency limit
    int32_t GetLatency(uint32_t conn_uid); // 5229fd (client: 512139) - Get current measured latency (ms)
    int32_t GetPacketLoss(uint32_t conn_uid); // 522a51 (client: 51218d) - Get current packet-loss figure

    void SetTimeout(uint32_t t) { this->timeout = t; } // 496f20 (client: 49ab40)
    int GetProvider() { return this->provider; } // 451610 (client: 453580)
};
ASSERT_OFFSET(CLlDriver, connection_sockets, 0x57c);
ASSERT_OFFSET(CLlDriver, critical_section, 0x7ec);
ASSERT_SIZE(CLlDriver, 0x914);





extern CHlDriver g_NetStru1_main;  //6c3a08
extern CHlDriver g_NetStru1_local; //6b16b0

extern CLlDriver g_CLlDriver; //6d07a0
extern CLlDriver g_HatLLDriver; //6cdbc0




void SendWWWReport(const CString& surl, const CString& data, int val3); // 4e505f (client: 4ea58f)
