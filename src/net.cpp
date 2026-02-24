#include "net.h"
#include "player.h"
#include "unit.h"
#include "unit_list.h"
#include "group.h"
#include "game_app.h"
#include <cstring>

void NetStru1::FUN_0051cd89(const CString& name, Player* player)
{
	PacketJoin& pkt = PacketJoin::Inst;
	pkt.id = 0x91;
	strcpy(pkt.name, name);

	if (player)
		pkt.to_player_id = player->player_id;
	else
		pkt.to_player_id = 0;

	pkt.player_id = 0;
	pkt.token_id = 0;
	pkt.field_0xc = 0;
	pkt.flags = 0;

	FUN_005186cd(&pkt);
}

// 4FB4CA
void NetStru1::FUN_004fb4ca(Unit* unit, Player* new_owner)
{
    Player* old_owner = unit->pOwner;

    // If the unit belongs to a group, remove it from that group.
    if (unit->group != nullptr) {
        unit->group->RemoveUnit(unit);
    }

    // Remove unit from old owner's unit list.
    // ASM manually walks the CList structure to find and remove the node.
    UnitList* old_unit_list = old_owner->unit_list;
    POSITION pos = old_unit_list->unit_list.Find(unit);
    if (pos != nullptr) {
        old_unit_list->unit_list.RemoveAt(pos);
    }
    
    unit->pOwner = new_owner;
    new_owner->unit_list->AddTail(unit);
    
    // Create a new Group for the unit.
    Group* new_group = new Group();
    
    // Add the new group to the new owner's group list.
    // The assembly manually adds to the CList structure.
    new_owner->group_list->AddTail(new_group);
    
    // Associate the unit with the new group.
    new_group->AddUnit(unit);
    
    // Update unit's vision sharing flags.
    uint16_t old_vision_mask = ~old_owner->vision_sharing_id;
    unit->field_x18 = unit->field_x18 & old_vision_mask;
    
    uint16_t new_vision_mask = ~new_owner->vision_sharing_id;
    unit->field_x18 = unit->field_x18 & new_vision_mask;
    
    // Send network packet about unit ownership change.
    g_NetStru1_main.sub_519221(unit, 0, 0xFFFFFFFF, 0xFFB, 0, 0);
    
    // Mark unit as needing update?
    unit->field_0x204 = 1;
}
