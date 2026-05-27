#include "quest_map.h"

#include "quest.h"

// 55D579
QuestMap::QuestMap() {
	this->assoc = nullptr;
	this->quest = nullptr;
	this->glue = nullptr;
	this->building_id = 0;
}

// 580F70 / 55D915
QuestMap::~QuestMap() {
	POSITION quest_pos = this->quests_map.GetStartPosition();
	while (quest_pos != nullptr) {
		uint32_t quest_id = 0;
		Quest* quest_value = nullptr;
		this->quests_map.GetNextAssoc(quest_pos, quest_id, quest_value);
		delete quest_value;
	}

	POSITION sub_pos = this->substructs.GetHeadPosition();
	while (sub_pos != nullptr) {
		QuestSubstruct* sub = this->substructs.GetNext(sub_pos);
		delete sub;
	}

	this->assoc = nullptr;
	this->quest = nullptr;
	this->glue = nullptr;
	this->building_id = 0;
}

// 55e129
int32_t QuestMap::VMethod1(int32_t event_type, int32_t a, int32_t b) {
	POSITION quest_pos = this->quests_map.GetStartPosition();
	while (quest_pos != nullptr) {
		uint32_t quest_id = 0;
		Quest* quest = nullptr;
		this->quests_map.GetNextAssoc(quest_pos, quest_id, quest);

		if (quest != nullptr && quest->VMethod1(event_type, a, b) != 0) {
			return static_cast<int32_t>(quest_id);
		}
	}

	return 0;
}

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
