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
