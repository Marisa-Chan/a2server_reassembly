#include "logic.h"

#include <cstring>

// 5afc9c (client: 57de0c)
TriggerAction::TriggerAction() {
    memset(this, 0, sizeof(TriggerAction));
}

// 5afc1d (client: 57dd8d)
TriggerCheck::TriggerCheck() {
    memset(this, 0, sizeof(TriggerCheck));
    this->exec_once = 1;
}

// 5afc6f (client: 57dddf)
TriggerCheck& TriggerCheck::operator=(const TriggerCheck& other) {
    memcpy(this, &other, sizeof(TriggerCheck));
    return *this;
}

// 5afd79 (client: 57dee9)
Trigger::Trigger() {
    memset(this, 0, sizeof(Trigger));
    this->checks = new CList<Check>();
    this->actions = new CList<int32_t>();
}

// 5afe3d (client: 57dfad)
Trigger::~Trigger() {
    delete this->checks;
    delete this->actions;
}

// 5b006e (client: 57e1de)
Trigger& Trigger::operator=(const Trigger& other) {
    memcpy(this, &other, sizeof(Trigger));
    this->checks = new CList<Check>();
    this->actions = new CList<int32_t>();
    for (POSITION it = other.checks->GetHeadPosition(); it != nullptr;) {
        Check& check = other.checks->GetNext(it);
        this->checks->AddTail(check);
    }
    for (POSITION it = other.actions->GetHeadPosition(); it != nullptr;) {
        int32_t action = other.actions->GetNext(it);
        this->actions->AddTail(action);
    }
    return *this;
}

// 5b681a (client: 58498a) - Serialize
void ScriptSettings::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar.Write(this, sizeof(ScriptSettings));
    } else {
        ar.Read(this, sizeof(ScriptSettings));
    }
}
