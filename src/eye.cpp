#include "eye.h"

#include "unit.h"


// 58C00E
uint8_t UnitEye::sub_58c00e() {
    return (uint8_t)(this->field0_0x0 + 8) >> 4;
}

// 5925C9
void UnitEye::sub_5925C9(Unit* unit) {
    uint8_t movement_type = unit->sub_59A030();
    if (movement_type == 1) {
        this->field5_0x5 = 65;
    } else if (movement_type == 2) {
        this->field5_0x5 = 68;
    } else if (movement_type == 3) {
        this->field5_0x5 = 130;
    }
}

// 59367D
int UnitEye::sub_59367D(Unit* unit, uint8_t* out_dir, uint8_t eye0, uint8_t angle) {
    if (angle == this->field1_0x1) {
        return 0;
    }
    *out_dir = this->field148_0xa4;
    return 1;
}

// 5A4F30
UnitEye2::~UnitEye2()
{
    delete positions_list;
    positions_list = nullptr;
}
