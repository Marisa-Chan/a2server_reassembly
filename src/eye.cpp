#include "eye.h"

#include <cstring>

#include "map_stuff.h"
#include "unit.h"


// 58be86 (client: 55b336)
UnitEye::UnitEye() {
    memset(this, 0, sizeof(UnitEye));
    this->path_mask = path_mask::land;
    this->rotation_speed = 16;
    this->static_age = 0xff;
    this->scan_radius_base = 5;
}

// 58c00e (client: 55b4be)
uint8_t UnitEye::GetFacingHalf() {
    return (uint8_t)(this->facing + 8) >> 4;
}

// 5925c9 (client: 561a79)
void UnitEye::SetPathMaskByMovement(Unit* unit) {
    uint8_t movement_type = unit->MovementType();
    if (movement_type == 1) {
        this->path_mask = path_mask::land;
    } else if (movement_type == 2) {
        this->path_mask = path_mask::amphibious;
    } else if (movement_type == 3) {
        this->path_mask = path_mask::flying;
    }
}

// 59367d (client: 562b2d)
int UnitEye::CheckRotation(Unit* unit, uint8_t* out_dir, uint8_t eye0, uint8_t angle) {
    if (angle == this->target_facing) {
        return 0;
    }
    *out_dir = this->turn_steps;
    return 1;
}

// 5a4e83 (client: 572ff3)
UnitAiState::UnitAiState() {
    memset(this, 0, sizeof(UnitAiState));
    this->seeInvisible = 1;
    this->tick16 = 65500;
    this->positions_list = new CList<uint16_t>();
}

// 5a4f30 (client: 5730a0)
UnitAiState::~UnitAiState() {
    delete this->positions_list;
    this->positions_list = nullptr;
}

// 5b031b (client: 57e48b)
void UnitAiState::Serialize(CArchive& ar) {
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
