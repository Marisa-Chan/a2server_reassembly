#include "visibility.h"

#include <cmath>
#include <cstring>
#include <memory>

#include "file.h"
#include "map_stuff.h"
#include "resource.h"
#include "unit.h"

// 58E1C1
void Visibility::sub_58E1C1() {
    memset(this->field_0x2a008, 0, sizeof(this->field_0x2a008));
}

// 58DE69
void Visibility::sub_58DE69(Unit* unit, uint16_t yx) {
    this->sub_58D768(unit, yx);
}

// 58CD84
void Visibility::sub_58CD84() {
    for (int32_t y = 0; y <= Visibility::CENTER; y++) {
        for (int32_t x = 0; x <= Visibility::CENTER; x++) {
            int16_t dist_sq = static_cast<int16_t>(y * y + x * x);
            this->field_0x20000[y + Visibility::CENTER][x + Visibility::CENTER] = dist_sq;
            this->field_0x20000[y + Visibility::CENTER][Visibility::CENTER - x] = dist_sq;
            this->field_0x20000[Visibility::CENTER - y][x + Visibility::CENTER] = dist_sq;
            this->field_0x20000[Visibility::CENTER - y][Visibility::CENTER - x] = dist_sq;
        }
    }
}

// 58CE74
void Visibility::sub_58CE74() {
    for (int32_t y = 0; y <= Visibility::CENTER; y++) {
        for (int32_t x = 0; x <= Visibility::CENTER; x++) {
            uint16_t p1;
            uint16_t p2;
            uint16_t p3;
            uint16_t p4;
            if (x < (y / 2)) {
                p1 = 0x00FF;
                p2 = 0x0001;
                p3 = 0x00FF;
                p4 = 0x0001;
            } else if (x > (y * 2)) {
                p1 = 0xFF00;
                p2 = 0x0100;
                p3 = 0x0100;
                p4 = 0xFF00;
            } else {
                p1 = 0xFFFF;
                p2 = 0x0101;
                p3 = 0x01FF;
                p4 = 0xFF01;
            }
            this->field_0x22000[y + Visibility::CENTER][x + Visibility::CENTER] = p1;
            this->field_0x22000[Visibility::CENTER - y][Visibility::CENTER - x] = p2;
            this->field_0x22000[y + Visibility::CENTER][Visibility::CENTER - x] = p3;
            this->field_0x22000[Visibility::CENTER - y][x + Visibility::CENTER] = p4;

            int32_t max_val = (y > x) ? y : x;
            int16_t value = 0;
            if (max_val != 0) {
                value = std::sqrt(y * y + x * x) * this->field_0x2a000 / max_val;
            }
            this->field_0x28000[y + Visibility::CENTER][x + Visibility::CENTER] = value;
            this->field_0x28000[Visibility::CENTER - y][Visibility::CENTER - x] = value;
            this->field_0x28000[y + Visibility::CENTER][Visibility::CENTER - x] = value;
            this->field_0x28000[Visibility::CENTER - y][x + Visibility::CENTER] = value;
        }
    }

    this->field_0x22000[Visibility::CENTER + 1][Visibility::CENTER] = 0x00FF;
    this->field_0x22000[Visibility::CENTER - 1][Visibility::CENTER] = 0x0001;
}

// 58E1E4
Visibility::Visibility() {
    File2 f;
    f.Open("World\\Data\\map.reg", 0, nullptr);

    RegFile reg;
    reg.ReadFromFile(&f, -1);

    int32_t scan_shift = reg.GetInt("Scanning", "ScanShift", 7);

    this->scan_shift = scan_shift;
    this->field_0x2a000 = 1 << scan_shift;

    this->sub_58CD84();
    this->sub_58CE74();

    memset(this->field_0x0, 0, sizeof(this->field_0x0));
}

// 58D638
int32_t Visibility::sub_58D638(int32_t x, int32_t y, int32_t base_yx, int32_t base_height) {
    PosYX pos = PosYX(x, y);
    PosYX direction = PosYX(this->field_0x22000[x][y]);
    PosYX with_offset = PosYX(base_yx + pos.val);
    int32_t height = this->map_stuff->height_map[with_offset.val];

    PosYX with_direction = PosYX(x + direction.x, y + direction.y);

    this->field_0x24000[x][y] = this->field_0x24000[with_direction.x][with_direction.y] - (this->field_0x28000[x][y] + height - base_height);

    if (this->field_0x24000[x][y] >= 1) {
        this->field_0x2a008[with_offset.val] = 1;
        return 0;
    }

    return 1;
}

// 58D768
void Visibility::sub_58D768(Unit* unit, uint16_t yx) {
    PosYX pos = PosYX(yx);
    int32_t base_yx = PosYX(pos.x - Visibility::CENTER, pos.y - Visibility::CENTER).val;

    memset(this->field_0x24000, 0, sizeof(this->field_0x24000));
    this->field_0x24000[Visibility::CENTER][Visibility::CENTER] = ((unit->scan_range >> 8) << this->scan_shift) + (1 << (this->scan_shift - 1));

    uint16_t unit_yx = unit->position->CompatGetYX();
    this->field_0x2a008[unit_yx] = 1;

    uint8_t base_height = this->map_stuff->height_map[unit_yx];

    bool keep_going = true;
    for (int32_t radius = 1; radius < Visibility::CENTER && keep_going; radius++) {
        keep_going = false;
        for (int32_t offset = -radius; offset <= radius; offset++) {
            int32_t x1 = Visibility::CENTER + offset;
            int32_t y1 = Visibility::CENTER - radius;
            if (this->map_stuff->IsInMapBounds(pos.x + x1, pos.y + y1) && !this->sub_58D638(x1, y1, base_yx, base_height)) {
                keep_going = true;
            }

            x1 = Visibility::CENTER + offset;
            y1 = Visibility::CENTER + radius;
            if (this->map_stuff->IsInMapBounds(pos.x + x1, pos.y + y1) && !this->sub_58D638(x1, y1, base_yx, base_height)) {
                keep_going = true;
            }

            x1 = Visibility::CENTER - radius;
            y1 = Visibility::CENTER + offset;
            if (this->map_stuff->IsInMapBounds(pos.x + x1, pos.y + y1) && !this->sub_58D638(x1, y1, base_yx, base_height)) {
                keep_going = true;
            }

            x1 = Visibility::CENTER + radius;
            y1 = Visibility::CENTER - offset;
            if (this->map_stuff->IsInMapBounds(pos.x + x1, pos.y + y1) && !this->sub_58D638(x1, y1, base_yx, base_height)) {
                keep_going = true;
            }
        }
    }
}
