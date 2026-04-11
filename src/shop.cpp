#include "shop.h"

#include "shelf.h"

// 5446C7
Item* Shop::sub_5446C7(Humanoid* humanoid, int16_t src_slot, int32_t count) {
    return this->shop_template->sub_547C5A(humanoid, src_slot, count);
}
