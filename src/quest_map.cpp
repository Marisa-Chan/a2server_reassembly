#include "quest_map.h"

#include "quest.h"
#include "quest_glue.h"

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

	this->sub_55D6F7();

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

// 55D6F7
void QuestMap::sub_55D6F7() {
	POSITION pos = this->glues_map.GetStartPosition();
	while (pos != nullptr) {
		uint32_t building_id = 0;
		QuestInnGlue* glue = nullptr;
		this->glues_map.GetNextAssoc(pos, building_id, glue);
		delete glue;
	}
	this->glues_map.RemoveAll();
}

// 55DD10
void QuestMap::sub_55DD10(int32_t event_type, int32_t a, int32_t b) {
	QuestSubstruct* sub = new QuestSubstruct;
	sub->id = event_type;
	sub->building_id = a;
	sub->some_id = b;
	this->substructs.AddTail(sub);
}

// 55E24A
void QuestMap::sub_55E24A(Quest* quest) {
	this->quests_map[quest->quest_data.some_id] = quest;
	quest->quest_map = this;

	if (quest->quest_data.landmark_id != 0) {
		QuestInnGlue* glue = nullptr;
		if (this->glues_map.Lookup(quest->quest_data.landmark_id, glue)) {
			this->glue = glue;
			glue->map[quest->quest_data.some_id] = quest;
		}
	}
}

// 55E5FB
void QuestMap::sub_55E5FB(Quest* quest) {
	this->flags[quest->quest_data.player_id] |= 1;
	this->quests_map.RemoveKey(quest->quest_data.some_id);
	quest->quest_map = nullptr;

	if (quest->quest_data.landmark_id != 0) {
		QuestInnGlue* glue = nullptr;
		if (this->glues_map.Lookup(quest->quest_data.landmark_id, glue)) {
			this->glue = glue;
			glue->map.RemoveKey(quest->quest_data.some_id);
			if (glue->map.GetCount() == 0) {
				glue->map.RemoveAll();
			}
		}
	}
}

// 55EA81
void QuestMap::sub_55EA81(QuestInnGlue* glue) {
	this->glues_map.SetAt(glue->building_id, glue);
	glue->quest_map = this;
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
