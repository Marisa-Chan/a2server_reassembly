#include "players_list.h"
#include "player.h"


int PlayersList::CountHumanPlayers()
{
	//53618f

	if (IsEmpty())
		return 0;

	int count = 0;

	for (CNode* pNode = m_pNodeHead; pNode ; pNode = pNode->pNext)
	{
		if (pNode->data->is_ai == 0)
			count++;
	}
	return count;
}