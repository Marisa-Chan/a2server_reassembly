#include "net.h"
#include "player.h"



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