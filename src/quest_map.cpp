#include "quest_map.h"

#include "quest.h"

// 55EE42
Quest* QuestMap::FUN_0055ee42(int quest_kind) {
	switch (quest_kind) {
	case 1:
		return new QuestKillMob();
	case 2:
		return new QuestKillN();
	case 3:
		return new QuestKillGroup();
	case 4:
		return new QuestEscort();
	case 5:
		return new QuestDeliverItem();
	case 6:
		return new QuestDeliverMail();
	case 8:
		return new QuestRaiseZombies();
	case 9:
		return new QuestRaiseSkeletons();
	case 10:
		return new QuestRaiseGhosts();
	case 11:
		return new QuestInterceptUnit();
	case 12:
		return new QuestInterceptGroup();
	case 13:
		return new QuestKillInnDefenders();
	default: // 0 and 7 are covered here.
		return new Quest();
	}
}
