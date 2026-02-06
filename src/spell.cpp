#include "spell.h"

void SpellBook::RefreshForHumanoid(Humanoid* humanoid) {
    using Fn = void (__thiscall*)(SpellBook*, Humanoid*);
    auto sub_53DBC5 = reinterpret_cast<Fn>(0x53DBC5);

    sub_53DBC5(this, humanoid);
}
