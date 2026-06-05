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
