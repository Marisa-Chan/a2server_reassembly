#include "action.h"

#include <memory>

Action::Action() {
    ::memset(this, 0, sizeof(Action));
}
