#include "item.h"
#include "effect.h"

// sub_548860
bool Item::IsSimilar(Item* other)
{
    if (item_id != other->item_id) {
        return false;
    }

    int v1 = VMethod16();
    int v2 = other->VMethod16();
    if (v1 != 0 && v2 != 0) {
        return true;
    }
    if (v1 != 0 || v2 != 0) {
        return false;
    }

    POSITION pos1 = this->_effects.GetHeadPosition();
    Effect* e1 = (pos1 != nullptr) ? this->_effects.GetNext(pos1) : nullptr;
    POSITION pos2 = other->_effects.GetHeadPosition();
    Effect* e2 = (pos2 != nullptr) ? other->_effects.GetNext(pos2) : nullptr;

    while (e1 != nullptr && e2 != nullptr) {
        if (!e1->sub_53EDB0(e2)) {
            return false;
        }
        e1 = (pos1 != nullptr) ? this->_effects.GetNext(pos1) : nullptr;
        e2 = (pos2 != nullptr) ? other->_effects.GetNext(pos2) : nullptr;
    }

    return e1 == nullptr && e2 == nullptr;
}
