#include "eye.h"

#include <cstring>

#include "unit.h"


// 58BE86
UnitEye::UnitEye() {
    memset(this, 0, sizeof(UnitEye));
    this->path_mask = 65;
    this->rotation_speed = 16;
    this->static_age = 0xff;
    this->scan_radius_base = 5;
}

// 58C00E
uint8_t UnitEye::GetFacingHalf() {
    return (uint8_t)(this->facing + 8) >> 4;
}

// 5925C9
void UnitEye::SetPathMaskByMovement(Unit* unit) {
    uint8_t movement_type = unit->MovementType();
    if (movement_type == 1) {
        this->path_mask = 65;
    } else if (movement_type == 2) {
        this->path_mask = 68;
    } else if (movement_type == 3) {
        this->path_mask = 130;
    }
}

// 59367D
int UnitEye::CheckRotation(Unit* unit, uint8_t* out_dir, uint8_t eye0, uint8_t angle) {
    if (angle == this->target_facing) {
        return 0;
    }
    *out_dir = this->turn_steps;
    return 1;
}

// 5A4E83
UnitEye2::UnitEye2() {
    memset(this, 0, sizeof(UnitEye2));
    this->seeInvisible = 1;
    this->tick16 = 65500;
    this->positions_list = new CList<uint16_t>();
}

// 5A4F30
UnitEye2::~UnitEye2() {
    delete this->positions_list;
    this->positions_list = nullptr;
}

// 5B031B
void UnitEye2::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar.Write(this, 0xB8);
        this->positions_list->Serialize(ar);
    } else {
        if (this->positions_list != nullptr) {
            delete this->positions_list;
        }
        ar.Read(this, 0xB8);
        this->positions_list = new CList<uint16_t>();
        this->positions_list->Serialize(ar);
    }
}
