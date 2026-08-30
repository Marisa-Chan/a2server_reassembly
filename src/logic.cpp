#include "logic.h"

#include <cstring>

// 5afc9c
TriggerAction::TriggerAction() {
    memset(this, 0, sizeof(TriggerAction));
}

// 5afc1d
TriggerCheck::TriggerCheck() {
    memset(this, 0, sizeof(TriggerCheck));
    this->exec_once = 1;
}

// 5afc6f
TriggerCheck& TriggerCheck::operator=(const TriggerCheck& other) {
    memcpy(this, &other, sizeof(TriggerCheck));
    return *this;
}

// 5afd79
Trigger::Trigger() {
    memset(this, 0, sizeof(Trigger));
    this->checks = new CList<Check>();
    this->actions = new CList<int32_t>();
}

// 5afe3d
Trigger::~Trigger() {
    delete this->checks;
    delete this->actions;
}

// 5b006e
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

// 5B681A
void ScriptSettings::sub_5B681A(CArchive& ar) {
    if (ar.IsStoring()) {
        ar.Write(this, sizeof(ScriptSettings));
    } else {
        ar.Read(this, sizeof(ScriptSettings));
    }
}
