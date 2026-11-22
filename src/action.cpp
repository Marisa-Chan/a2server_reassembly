#include "action.h"

#include <memory>

// 5b6c42 (client: 584db2)
Action::Action() {
    ::memset(this, 0, sizeof(Action));
}
